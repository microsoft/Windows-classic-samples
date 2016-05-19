//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1996-2000 Microsoft Corporation.
//
// @doc 
//
// @module itrnjoin.CPP | Test Module for Joined Transactions
// 
#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "itrnjoin.h"
#include "txnbase.hpp"		//Base classes for transacted rowsets
//#ifdef TXNJOIN
	#include "XOLEHLP.H"
	#include "txdtc.h"
//#endif  //TXNJOIN


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x561a4720, 0x9dbd, 0x11d1, { 0x87, 0x30, 0x00, 0xc0, 0x4f, 0xd6, 0x58, 0xf5 }};
DECLARE_MODULE_NAME("ITransactionJoin");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for Joined Transactions");
DECLARE_MODULE_VERSION(839560026);
// }}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This must be defined so that the base class constructors 
//in txnbase.hpp know what to use for this test module's name.
LPWSTR	gwszThisModuleName	= L"itrnjoin";	
BOOL	gfDTCStarted		= FALSE;
BOOL	gfBlocked			= FALSE;


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule *pThisTestModule)
{
	HRESULT					hr;
	ITransactionJoin		*pITxnJoin				= NULL;
	
	WCHAR		lpService[10]	= L"MSDTC";
	BOOL		fCPPass			= FALSE;
	SC_HANDLE	hService;
	SC_HANDLE	hServiceDTC;
	WCHAR		*pwszDBMS;
	WCHAR		*pwszDBMSVer;



	if (ModuleCreateDBSession(pThisTestModule))
	{
		//make sure DTC is started
		//this will fail if dtc is already started. no big
		//the only concern here is that dtc is started
		hService=OpenSCManager (NULL,NULL,SC_MANAGER_ALL_ACCESS);

		//if this fails, let the test proceed, this could be a win98 machine with a remote DTC which doesn't
		//appear as a service
		if (hService)
		{
			hServiceDTC=OpenServiceW(hService, lpService,SERVICE_START);
			fCPPass=StartService(hServiceDTC,NULL,NULL);
			//if this fails remember it was because the service was already running
			//or DTC is not working
			if(!fCPPass)
			{
				if (ERROR_SERVICE_ALREADY_RUNNING==GetLastError())
				{
					gfDTCStarted=TRUE;
				}
				else
				{
					odtLog <<L"A useable MSDTC is not on this machine.\n";
					return TEST_SKIPPED;
				}
			}
			CloseServiceHandle(hServiceDTC);
			CloseServiceHandle(hService);
		}

		//Fail gracefully and quit module if we don't support local transactions
		if (SUCCEEDED(hr = pThisTestModule->m_pIUnknown2->QueryInterface(
			IID_ITransactionJoin, (void **)&pITxnJoin)))
		{
			pITxnJoin->Release();
			pITxnJoin	=	NULL;
		}
		else
		{
			//Make sure we returned E_NOINTERFACE if we've failed
			if (pThisTestModule->m_pError->Validate(hr,	
								LONGSTRING(__FILE__), __LINE__, E_NOINTERFACE))
			{
				odtLog <<L"ITransactionJoin is not supported.\n";
			}
			return TEST_SKIPPED;
		}

		//Create a table we'll use for the whole test module,
		//store it in pVoid for now		
		pThisTestModule->m_pVoid = new CTable(
												(IUnknown *)pThisTestModule->m_pIUnknown2, 
												(LPWSTR)gwszModuleName
											);
			
		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}		    

		//Start with a table with rows		
		if (FAILED(((CTable *)pThisTestModule->m_pVoid)->CreateTable(NUM_ROWS)))
		{
			return FALSE;
		}

		//Init last actual insert number to last row we inserted
		g_ulLastActualInsert = NUM_ROWS;

		//First row in table is 1
		g_ulFirstRowInTable		= 1;

		g_ulLastActualDelete	= 0;
		
	
		GetProperty(DBPROP_DBMSVER ,DBPROPSET_DATASOURCEINFO, (IUnknown *)pThisTestModule->m_pIUnknown,&pwszDBMSVer);
		GetProperty(DBPROP_DBMSNAME ,DBPROPSET_DATASOURCEINFO, (IUnknown *)pThisTestModule->m_pIUnknown,&pwszDBMS);

		//if this is SQL Server with a version less than 8 flag is so some variations can be skipped
		//becasue they will block.
		if (!wcscmp(pwszDBMS,L"Microsoft SQL Server"))
		{
			if(wcscmp(pwszDBMSVer,L"08.00.0000") < 0)
			{
					gfBlocked	= TRUE;
			}
		}
		PROVIDER_FREE(pwszDBMS);
		PROVIDER_FREE(pwszDBMSVer);

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
	WCHAR		lpService[20]	= L"MSDTC";
	BOOL		fCPPass			= FALSE;
	SC_HANDLE	hService;
	SC_HANDLE	hServiceDTC;

	//if DTC was not started before the test then make sure it is stopped
	if (!gfDTCStarted)
	{
		//make sure DTC is in the state it was before the test started
		hService=OpenSCManager (NULL,NULL,SC_MANAGER_ALL_ACCESS);
		hServiceDTC=OpenServiceW(hService, lpService,SERVICE_STOP);
		fCPPass=ControlService(hServiceDTC,SERVICE_CONTROL_STOP,NULL);
		CloseServiceHandle(hServiceDTC);
		CloseServiceHandle(hService);
	}
	
	//We still own the table since all of our testcases
	//have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{	
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Declarations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//--------------------------------------------------------------------
// @class CTxnChgUpdRowset | Transactable updateable buffered mode rowset
// @base public | CTxnChgRowset
//--------------------------------------------------------------------
class CTxnChgUpdRowset : public CTxnChgRowset
{
// @access public
public:
	
	//@cmember IRowsetUpdate Interface for this rowset
	IRowsetUpdate	*m_pIRowsetUpdate;
	//@cmember IRowset interface for this rowset
	IRowset			*m_pIRowset;
	//@cmember Count of pending rows on update
	DBCOUNTITEM		m_cPendingRows;
	//@cmember Array of pending rows
	HROW			*m_rgPendingRows;
	//@cmember Array of pending rows' status
	DBPENDINGSTATUS	*m_rgPendingStatus;


	//@cmember CTOR
	CTxnChgUpdRowset(LPWSTR tcName) : CTxnChgRowset(tcName){};	
	//@cmember Initialization - Creates an updateable buffered mode rowset
	virtual BOOL Init();
	//@cmember Termination - Does termination for an updateable buffered mode rowset
	virtual BOOL Terminate();
	//@cmember Creates an updateable buffered rowset and gets IRowsetUpdate interface
	virtual HRESULT	MakeRowset();	
	//@cmember Frees an updateable buffered rowset and frees IRowsetUpdate interface
	virtual HRESULT	FreeRowset();	
	//@cmember make sure properties are set
	virtual HRESULT	ReSetProps();	
	//@cmember Verifies pending change after a transaction commit or abort
	BOOL CheckPendingRow(ULONG cPendingRows);	
};


//--------------------------------------------------------------------
// @class CUpdateTxn | Base Class for IRowsetUpdate / Transaction testing.
// @base public | CTxn
//--------------------------------------------------------------------
class CTxnUpdate : public CTxn
{
// @access public
public:
	//@cmember	Pointer to Rowset object that supports Changes in buffered mode
	CTxnChgUpdRowset	*m_pChgUpdRowset1;
	//@cmember	Pointer to Rowset object that supports Changes in buffered mode
	CTxnChgUpdRowset	*m_pChgUpdRowset2;
	//@cmember CTOR
	CTxnUpdate(LPWSTR tcName) : CTxn(tcName){};
	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();

	//@cmember Releases all rowsets associated with any encapsulated objects
	virtual void ReleaseAllRowsetsAndTxns();
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Function Definitions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//--------------------------------------------------------------------
// Does init for buffered mode rowset.
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnChgUpdRowset::Init()
{
	const ULONG		cProps	= 5;
	DBPROP			DBProp[cProps];	
	DBPROPSET		RowsetPropSet;
	BOOL			fInit	= FALSE;
	ULONG			i		= 0;

		m_pIRowsetUpdate = NULL;
		m_pIRowset = NULL;
		m_cPendingRows = 0;
		m_rgPendingRows = NULL;
		m_rgPendingStatus = NULL;

		//Set properties for change ability and determine support
		fInit = CTxnChgRowset::Init();

		if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
		{
			//Now reset all properties and include IRowsetUpdate
			DBProp[i].dwPropertyID = DBPROP_IRowsetChange;
			DBProp[i].dwOptions=0;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			DBProp[i].colid	= DB_NULLID;
			i++;

			DBProp[i].dwPropertyID= DBPROP_IRowsetUpdate;
			DBProp[i].dwOptions=0;
			DBProp[i].vValue.vt=VT_BOOL;
			DBProp[i].colid	= DB_NULLID;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;

			DBProp[i].dwPropertyID= DBPROP_UPDATABILITY;
			DBProp[i].dwOptions=0;
			DBProp[i].vValue.vt=VT_I4;	
			DBProp[i].colid	= DB_NULLID;
			DBProp[i].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;
			i++;
		}

		if (g_rgDBPrpt[IDX_OtherInsert].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERINSERT;
			DBProp[i].dwOptions=0;
			DBProp[i].vValue.vt=VT_BOOL;
			DBProp[i].colid	= DB_NULLID;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}

		if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
			DBProp[i].dwOptions=0;
			DBProp[i].vValue.vt=VT_BOOL;
			DBProp[i].colid	= DB_NULLID;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}

		//Build one set of rowset properties
		RowsetPropSet.rgProperties		= DBProp;
		RowsetPropSet.cProperties		= i;
		RowsetPropSet.guidPropertySet	= DBPROPSET_ROWSET;
	
		//Now we'll be ready to execute and get a buffered mode rowset
		SetRowsetProperties(&RowsetPropSet, 1);	

		return fInit;
}


//--------------------------------------------------------------------
// makes sure props are set.
// @mfunc 
//--------------------------------------------------------------------
HRESULT	CTxnChgUpdRowset::ReSetProps()
{	
	const ULONG		cProps			= 3;
	DBPROP			DBProp[cProps];	
	DBPROPSET		RowsetPropSet;
	ULONG			i				= 0;
	
	if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
	{	//Now reset all properties and include IRowsetUpdate
		DBProp[0].dwPropertyID = DBPROP_IRowsetChange;
		DBProp[0].dwOptions=0;
		DBProp[0].vValue.vt=VT_BOOL;
		DBProp[0].colid	= DB_NULLID;
		V_BOOL(&(DBProp[0].vValue))=VARIANT_TRUE;

		DBProp[1].dwPropertyID= DBPROP_IRowsetUpdate;
		DBProp[1].dwOptions=0;
		DBProp[1].vValue.vt=VT_BOOL;
		DBProp[1].colid	= DB_NULLID;
		V_BOOL(&(DBProp[1].vValue))=VARIANT_TRUE;

		DBProp[2].dwPropertyID= DBPROP_UPDATABILITY;
		DBProp[2].dwOptions=0;
		DBProp[2].vValue.vt=VT_I4;	
		DBProp[2].colid	= DB_NULLID;
		DBProp[2].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;

		//Build one set of rowset properties
		RowsetPropSet.rgProperties = DBProp;
		RowsetPropSet.cProperties = cProps;
		RowsetPropSet.guidPropertySet = DBPROPSET_ROWSET;
		
		//Now we'll be ready to execute and get a buffered mode rowset
		return SetRowsetProperties(&RowsetPropSet, 1);	
	}
	else
	{
		return S_OK;
	}
	
}


//--------------------------------------------------------------------
// Does init for buffered mode rowset.
// @mfunc CreateRowset
//--------------------------------------------------------------------
HRESULT	CTxnChgUpdRowset::MakeRowset()
{	
	m_hr = NOERROR;

	//We may already have an update and rowset interface, in which
	//case we are assuming that MakeRowset above did not create a new
	//rowset, and our interfaces will still be valid, so we'll just return
	if (!m_pIRowsetUpdate)
	{
		//Create the rowset and get IRowsetUpdate interface to it
		if (CHECK(m_hr = ReSetProps(), S_OK))			{	
			//Create the rowset and get IRowsetUpdate interface to it
			if (CHECK(m_hr = CTxnChgRowset::MakeRowset(), S_OK))	
			{
				//If our Init passed, we know IRowsetUpdate should always be available
				if (VerifyInterface(m_pIAccessor, IID_IRowsetUpdate, ROWSET_INTERFACE,
					(IUnknown **)&m_pIRowsetUpdate))
				{
					if (VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
						(IUnknown **)&m_pIRowset))
					{
						return NOERROR;	
					}
					else
					{
						m_hr = ResultFromScode(E_NOINTERFACE);	
					}
				}
				else
				{
					m_hr = ResultFromScode(E_NOINTERFACE);
				}
			}
		}
	}
	return m_hr;
}

//--------------------------------------------------------------------
// Does init for buffered mode rowset.
// @mfunc CreateRowset
//--------------------------------------------------------------------
HRESULT	CTxnChgUpdRowset::FreeRowset()
{	
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowset);

	ReleaseRowsetObject();
	
	return NOERROR;
}

//--------------------------------------------------------------------
// Verifies pending row from change, if rowset isn't zombied
// @mfunc CheckPendingRow
//--------------------------------------------------------------------
BOOL CTxnChgUpdRowset::CheckPendingRow(ULONG cPendingRows)
{
	BOOL	fResults = FALSE;
	ULONG	i;
	ULONG	cPending;


	if (!cPendingRows)
	{
		cPending=S_FALSE;//no rows are pending
	}
	else
	{
		cPending=S_OK;
	}

	//We know the rowset should be in tact, so we can check PendingRows
	if (CHECK(m_pIRowsetUpdate->GetPendingRows(	NULL,
												DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED, 
												&m_cPendingRows, 
												&m_rgPendingRows, 
												&m_rgPendingStatus), cPending))
	{
		//THESE ARE COMMENTED OUT BECAUSE THESE >CAN< BE NULL & ZERO IF NO ROWS ARE PENDING
		//Make sure our pointers are valid
		//if (!m_rgPendingRows || !m_rgPendingStatus)
		//	goto CLEANUP;

		//Make sure we have the right number of pending changes
		if (COMPARE(m_cPendingRows, cPendingRows))
		{			
			//For each row, check the status
			for (i=0; i<cPendingRows; i++)
			{
				COMPARE(m_rgPendingStatus[i], DBPENDINGSTATUS_CHANGED);						
			}
			
			fResults = TRUE;		
		}
	}
//CLEANUP:
	//Release the row
	if (m_rgPendingRows)		
		CHECK(m_pIRowset->ReleaseRows(m_cPendingRows, m_rgPendingRows, 
		NULL, NULL, NULL), S_OK);
	
	if (m_rgPendingStatus)
		m_pIMalloc->Free(m_rgPendingStatus);
	return fResults;
}

//--------------------------------------------------------------------
// Does all termination, including releasing m_pIRowsetUpdate if needed.
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnChgUpdRowset::Terminate()
{
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowset);
	
	return CTxnChgRowset::Terminate();
}


//--------------------------------------------------------------------
// Inits the CTxnUpdate Object.  It inits the encapsulated objects
// which are unique to CTxnUpdate
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnUpdate::Init()
{
	if (CTxn::Init())
	{
		//Create our encapsulated ChgUpdRowset objects
		m_pChgUpdRowset1 = new CTxnChgUpdRowset((LPWSTR)gwszModuleName);
		m_pChgUpdRowset2 = new CTxnChgUpdRowset((LPWSTR)gwszModuleName);

		if (m_pChgUpdRowset1 && m_pChgUpdRowset2)
		{
			//Initialize pointers
			m_pChgUpdRowset1->m_pIRowsetUpdate	= NULL;
			m_pChgUpdRowset1->m_pIRowset		= NULL;
			m_pChgUpdRowset1->m_cPendingRows	= 0;
			m_pChgUpdRowset1->m_rgPendingRows	= NULL;
			m_pChgUpdRowset1->m_rgPendingStatus	= NULL;

			m_pChgUpdRowset2->m_pIRowsetUpdate	= NULL;
			m_pChgUpdRowset2->m_pIRowset		= NULL;
			m_pChgUpdRowset2->m_cPendingRows	= 0;
			m_pChgUpdRowset2->m_rgPendingRows	= NULL;
			m_pChgUpdRowset2->m_rgPendingStatus	= NULL;

			//Copy the stuff which normally gets initialized at
			//testcase initialization time, but didn't for these
			//objects since they are encapsulated rather than
			//inherited by the testcase object.
			CopyTestCaseInfo(m_pChgUpdRowset1);
			CopyTestCaseInfo(m_pChgUpdRowset2);
			
			//Set the same DSO and table for our encapsulated rowset objects
			//also create each rowset
			if (m_pChgUpdRowset1->Init() &&
				m_pChgUpdRowset2->Init())
				return TRUE;
		}
	}
	return FALSE;
}


//--------------------------------------------------------------------
// Releases all rowsets associated with encapsulated rowset objects
// and does an abort of all open transactions
// @mfunc ReleaseAllRowsetsAndTxns
//--------------------------------------------------------------------
void CTxnUpdate::ReleaseAllRowsetsAndTxns()
{
	//Clean up all rowset interfaces
	m_pChgUpdRowset1->ReleaseRowsetObject();
	SAFE_RELEASE(m_pChgUpdRowset1->m_pIRowset);
	SAFE_RELEASE(m_pChgUpdRowset1->m_pIRowsetUpdate);

	m_pChgUpdRowset2->ReleaseRowsetObject();
	SAFE_RELEASE(m_pChgUpdRowset2->m_pIRowset);
	SAFE_RELEASE(m_pChgUpdRowset2->m_pIRowsetUpdate);

	//We don't have a return value, since its just cleanup, 
	//which may not succeed because its not necessary
	m_pChgUpdRowset1->Abort(FALSE, FALSE);
	m_pChgUpdRowset2->Abort(FALSE, FALSE);
}


//--------------------------------------------------------------------
// Cleans up the CTxn Object				
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnUpdate::Terminate()		
{	
	//Clean up encapsulated objects
	if (m_pChgUpdRowset1)
	{
		m_pChgUpdRowset1->Terminate();
		delete m_pChgUpdRowset1;
	}
		
	if (m_pChgUpdRowset2)
	{
		m_pChgUpdRowset2->Terminate();
		delete m_pChgUpdRowset2;
	}

	return CTxn::Terminate();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Miscellaneous Function Definitions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//--------------------------------------------------------------------
// Does a non retaining commit on the given rowset.  This function is 
// intended to be envoked by CreateThread or ResumeThread, so the commit 
// will be done on a second thread.
//
// @mfunc ThreadCommit
//--------------------------------------------------------------------
LRESULT     ThreadCommit(LPVOID pChgRowset)
{		
	ULONG	ul = 0;
	
	//We want to make sure our other operation has a chance to start before we continue.
	//Specifying zero will immediately turn our time slice over to another thread.
	Sleep(0);

	if (FAILED(CoInitialize(NULL)))
		return ResultFromScode(E_FAIL);

	ul = (ULONG)((CTxnChgRowset *)pChgRowset)->m_pITxnLocal->Commit(FALSE, 0, 0);		
	CoUninitialize();

	return ul;
}

//--------------------------------------------------------------------
// Does a non retaining abort on the given rowset.  This function is 
// intended to be envoked by CreateThread or ResumeThread, so the abort 
// will be done on a second thread.
//
// @mfunc ThreadAbort
//--------------------------------------------------------------------
LRESULT     ThreadAbort(LPVOID pChgRowset)
{			
	ULONG	ul	= 0;
	
	//We want to make sure our other operation has a chance to start before we continue.
	//Specifying zero will immediately turn our time slice over to another thread.
	Sleep(0);

	if (FAILED(CoInitialize(NULL)))
		return ResultFromScode(E_FAIL);

	ul = (ULONG)((CTxnChgRowset *)pChgRowset)->m_pITxnLocal->Abort(NULL, FALSE, FALSE);		
	CoUninitialize();

	return ul;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_TEST_CASE_MAP(TCIsoLevel)
//--------------------------------------------------------------------
// @class Isolation Level Testing
//
class TCIsoLevel : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIsoLevel,CTxnImmed);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Positive ISOLEVEL
	int Variation_1();
	// @cmember Chaos
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCIsoLevel)
#define THE_CLASS TCIsoLevel
BEG_TEST_CASE(TCIsoLevel, CTxnImmed, L"Isolation Level Testing")
	TEST_VARIATION(1,		L"Positive ISOLEVEL")
	TEST_VARIATION(2,		L"Chaos")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCEnListmentNULL)
//--------------------------------------------------------------------
// @class Enlistment Testing
//
class TCEnListmentNULL : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCEnListmentNULL,CTxnImmed);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Join-Commit-UnEnlist-reEnlist
	int Variation_1();
	// @cmember Join-Abort-UnEnlist-reEnlist
	int Variation_2();
	// @cmember Back to back UnEnlists, no pending work
	int Variation_3();
	// @cmember UnEnList but not enlisted
	int Variation_4();
	// @cmember Join Committed session with new MTSTxn
	int Variation_5();
	// @cmember Join Aborted session with new MTSTxn
	int Variation_6();
	// @cmember UnEnlists, pending work
	int Variation_7();
// }}
};
// {{ TCW_TESTCASE(EnListment)
#define THE_CLASS TCEnListmentNULL
BEG_TEST_CASE(TCEnListmentNULL, CTxnImmed, L"Enlistment Testing")
	TEST_VARIATION(1,		L"Join-Commit-UnEnlist-reEnlist")
	TEST_VARIATION(2,		L"Join-Abort-UnEnlist-reEnlist")
	TEST_VARIATION(3,		L"Back to back UnEnlists, no pending work")
	TEST_VARIATION(4,		L"UnEnList but no txn")
	TEST_VARIATION(5,		L"Join Committed session with new MTSTxn")
	TEST_VARIATION(6,		L"Join Aborted session with new MTSTxn")	
	TEST_VARIATION(7,		L"UnEnlists, pending work")	
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCEnListment)
//--------------------------------------------------------------------
// @class Enlistment Testing
//
class TCEnListment : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCEnListment,CTxnImmed);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Join-Commit-reEnlist
	int Variation_1();
	// @cmember Join-Abort-reEnlist
	int Variation_2();
	// @cmember Back to back Enlists
	int Variation_3();
	// @cmember Test MTS after Commit
	int Variation_4();
	// @cmember Test MTS after Abort
	int Variation_5();
	// @cmember Back to back Enlists - different txns
	int Variation_6();
	// @cmember Join-Commit-unEnlist-reEnlist
	int Variation_7();
	// @cmember Join before firehose mode
	int Variation_8();
	// @cmember Join after firehose mode
	int Variation_9();
	// @cmember commit with 2 commands open
	int Variation_10();
	// @cmember DBPROP_RESETDATASOURCE
	int Variation_11();
// }}
};
// {{ TCW_TESTCASE(EnListment)
#define THE_CLASS TCEnListment
BEG_TEST_CASE(TCEnListment, CTxnImmed, L"Enlistment Testing")
	TEST_VARIATION(1,		L"Join-Commit-reEnlist")
	TEST_VARIATION(2,		L"Join-Abort-reEnlist")
	TEST_VARIATION(3,		L"Back to back Enlists")
	TEST_VARIATION(4,		L"Test MTS after Commit")
	TEST_VARIATION(5,		L"Test MTS after Abort")	
	TEST_VARIATION(6,		L"Back to back Enlists - different txns")
	TEST_VARIATION(7,		L"Join-Commit-unEnlist-reEnlist")
	TEST_VARIATION(8,		L"Join before firehose mode")
	TEST_VARIATION(9,		L"Join after firehose mode")
	TEST_VARIATION(10,		L"commit with 2 commands open")
	TEST_VARIATION(11,		L"DBPROP_RESETDATASOURCE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(TCRetainPreserve)
//--------------------------------------------------------------------
// @class Retaining/Preserving Behavior
//
class TCRetainPreserve : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRetainPreserve,CTxnImmed);
	// }}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	//@cmember Function to test Commit with fRetaining = TRUE
	BOOL CommitRetain(ISOLEVEL fIsoLevel);
	//@cmember Function to test Commit with fRetaining = FALSE
	BOOL CommitNonRetain(ISOLEVEL fIsoLevel);
	//@cmember Function to test Abort with fRetaining = TRUE
	BOOL AbortRetain(ISOLEVEL fIsoLevel);
	//@cmember Function to test Abort with fRetaining = FALSE
	BOOL AbortNonRetain(ISOLEVEL fIsoLevel);	

	// {{ TCW_TESTVARS()
	// @cmember Commit(fRetaining = TRUE
	int Variation_1();
	// @cmember Commit(fRetaining = FALSE
	int Variation_2();
	// @cmember Abort(fRetaining = TRUE
	int Variation_3();
	// @cmember Abort(fRetaining = FALSE
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(TCRetainPreserve)
#define THE_CLASS TCRetainPreserve
BEG_TEST_CASE(TCRetainPreserve, CTxnImmed, L"Retaining/Preserving Behavior")
	TEST_VARIATION(1,		L"Commit(fRetaining = TRUE")
	TEST_VARIATION(2,		L"Commit(fRetaining = FALSE")
	TEST_VARIATION(3,		L"Abort(fRetaining = TRUE")
	TEST_VARIATION(4,		L"Abort(fRetaining = FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCIRowsetUpdate)
//--------------------------------------------------------------------
// @class Transacted rowsets in Buffered Update mode
//
class TCIRowsetUpdate : public CTxnUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowsetUpdate,CTxnUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Change, Abort with fRetaining = FALSE, Update
	int Variation_1();
	// @cmember Change, Update, Abort with fRetaining = FALSE
	int Variation_2();
	// @cmember Change, Commit with fRetaining = FALSE, Update
	int Variation_3();
	// @cmember Change, Update, Commit with fRetaining = FALSE
	int Variation_4();
	// @cmember Open read only rowset
	int Variation_5();
	// @cmember Change, Join Txn, Update, Abort with fRetaining = FALSE
	int Variation_6();
	// @cmember Change, Update, Join Txn, Abort with fRetaining = FALSE
	int Variation_7();
	// @cmember Change, Join Txn, Update, Commit with fRetaining = FALSE
	int Variation_8();
	// @cmember Change, Update, Join Txn, Commit with fRetaining = FALSE
	int Variation_9();
// }}
};
// {{ TCW_TESTCASE(TCIRowsetUpdate)
#define THE_CLASS TCIRowsetUpdate
BEG_TEST_CASE(TCIRowsetUpdate, CTxnUpdate, L"Transacted rowsets in Buffered Update mode")
	TEST_VARIATION(1,		L"Change, Abort with fRetaining = FALSE, Update")
	TEST_VARIATION(2,		L"Change, Update, Abort with fRetaining = FALSE")
	TEST_VARIATION(3,		L"Change, Commit with fRetaining = FALSE, Update")
	TEST_VARIATION(4,		L"Change, Update, Commit with fRetaining = FALSE")
	TEST_VARIATION(5,		L"Open read only rowset")
	TEST_VARIATION(6,		L"Change, Join Txn, Update, Abort with fRetaining = FALSE")
	TEST_VARIATION(7,		L"Change, Update, Join Txn, Abort with fRetaining = FALSE")
	TEST_VARIATION(8,		L"Change, Join Txn, Update, Commit with fRetaining = FALSE")
	TEST_VARIATION(9,		L"Change, Update, Join Txn, Commit with fRetaining = FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CNoTxn)
//--------------------------------------------------------------------
// @class Abort and Commit called before StartTransaction
//
class CNoTxn : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	//ITransactionLocal interface
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CNoTxn,CTxnImmed);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining = FALSE before coordinated txn is joined  
	int Variation_1();
	// @cmember Commit with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_2();
	// @cmember GetTransactionInfo before StartTransaction - XACT_E_NOTRANSACTION
	int Variation_3();
	// @cmember GetOptionsObject before StartTransaction - S_OK
	int Variation_4();
	// @cmember Abort with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_5();
	// }}
};
// {{ TCW_TESTCASE(CNoTxn)
#define THE_CLASS CNoTxn
BEG_TEST_CASE(CNoTxn, CSessionObject, L"Abort and Commit after local changes")
	TEST_VARIATION(1,		L"Start a coordinated txn, make a change, abort it, look for change")
	TEST_VARIATION(2,		L"Make a change, start a coordinated txn, join it, abort it, look for change")
	TEST_VARIATION(3,		L"Make a change, start a coordinated txn, join it, commit it, look for change")
	TEST_VARIATION(4,		L"GetOptionsObject before StartTransaction - S_OK")
	TEST_VARIATION(5,		L"Start a coordinated txn, make a change, join the coord txn, abort it, look for change")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CMultipleTxns)
//--------------------------------------------------------------------
// @class Multiple Transactions
//
class CMultipleTxns : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CMultipleTxns,CTxnImmed);
	// }}
 
	//@cmember Second table for second rowset
	CTable *	m_pTable;
	//@cemember Actual delete count for second table
	ULONG		m_ulCurrentDelete;
	//@cemember Actual insert count for second table
	ULONG		m_ulCurrentInsert;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Abort two transactions on same DSO
	int Variation_1();
	// @cmember Commit two transactions on same DSO
	int Variation_2();
	// @cmember Abort one transaction, commit the other, on same DSO
	int Variation_3();
	// @cmember Abort and Commit with multiple commands/rowsets active
	int Variation_4();
	// @cmember Multiple Sessions, Transaction on only one
	int Variation_5();
	// @cmember Abort and Commit from 2 session, each to it's own coord txn
	int Variation_6();	
	// @cmember Two sessions, UnEnlist from one
	int Variation_7();	
// }}
};
// {{ TCW_TESTCASE(CMultipleTxns)
#define THE_CLASS CMultipleTxns
BEG_TEST_CASE(CMultipleTxns, CTxnImmed, L"Multiple Transactions")
	TEST_VARIATION(1,		L"Abort two transactions on same DSO")
	TEST_VARIATION(2,		L"Commit two transactions on same DSO")
	TEST_VARIATION(3,		L"Abort one transaction, commit the other, on same DSO")
	TEST_VARIATION(4,		L"Abort and Commit with multiple commands/rowsets active")
	TEST_VARIATION(5,		L"Multiple Sessions, Transaction on only one")
	TEST_VARIATION(6,		L"Abort and Commit from 2 session, each to it's own coord txn")
	TEST_VARIATION(7,		L"Two sessions, UnEnlist from one")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CSequence)
//--------------------------------------------------------------------
// @class Sequence testing for StartTransaction and ITransactionOptions
//
class CSequence : public CTxn {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CSequence,CTxn);
	// }}

	//@cmember Coordinated Transaction interface
	ITransaction		*m_pITransaction;
	//@cmember Joined Transaction interface
	ITransactionJoin	*m_pITxnJoin;
	//@cmember Transaction Options interface	
	ITransactionOptions *m_pITxnOptions;
	//@cmember Options struct
 	XACTOPT				m_TxnOptions;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	//@cmember Start DTC, coordinated transaction
	BOOL	StartCoordTxn(	ITransaction	**ppITransaction,ISOLEVEL		isolevel);
	//@cmember gwt txnjoin object
	HRESULT	GetTxnJoin();		// @parm [OUT] Session Pointer)
	//@cmember free txn join session object
	BOOL	FreeJoinTxn()		;
	//@cmember free DTC, coordinated transaction
	BOOL	FreeCoordTxn(ITransaction	*pITransaction);
	//@cmember Cleanup function for this derived class
	virtual void ReleaseAllRowsetsAndTxns()
	{
		//Cleanup our transaction, if we have one
		if (m_pITransaction)
		{
			m_pITransaction->Commit(FALSE, 0, 0);
		}
	}
	
	// {{ TCW_TESTVARS()
	// @cmember GetOptionsObject before StartTransaction
	int Variation_1();
	// @cmember GetOptionsObject after StartTransaction
	int Variation_2();
	// @cmember SetOptions twice
	int Variation_3();
	// }}
};
// {{ TCW_TESTCASE(CSequence)
#define THE_CLASS CSequence
BEG_TEST_CASE(CSequence, CTxn, L"Sequence testing for StartTransaction and ITransactionOptions")
	TEST_VARIATION(1,		L"GetOptionsObject before StartTransaction")
	TEST_VARIATION(2,		L"GetOptionsObject after StartTransaction")
	TEST_VARIATION(3,		L"SetOptions twice")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	// @cmember Txn Object interface for session
	ITransactionOptions * m_pITxnOptions;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CTxnImmed);
	// }}
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid ITransactionLocal calls with previous error object existing.
	int Variation_1();
	// @cmember Valid Abort call with previous error object existing.
	int Variation_2();
	// @cmember Valid SetOptions call with previous error object existing.
	int Variation_3();
	// @cmember Valid GetOptions call with previous error object existing.
	int Variation_4();
	// @cmember XACT_E_NOTSUPPORTED Commit call with previous error object existing
	int Variation_5();
	// @cmember XACT_E_NOISORETAIN JoinTransaction call with previous error object existing
	int Variation_6();
	// @cmember E_INVALIDARG GetTransactionInfo call with previous error object existing
	int Variation_7();
	// @cmember E_INVALIDARG GetOptions call with previous error object existing
	int Variation_8();
	// @cmember E_INVALIDARG SetOptions call with previous error object existing
	int Variation_9();
	// @cmember XACT_E_ISOLATIONLEVEL NULL & INVALID ISOLEVEL
	int Variation_10();
	// @cmember E_INVALIDARG GetOptionObject calls no with previous error object existing
	int Variation_11();
	// @cmember E_INVALIDARG SetOptions calls with no previous error object existing
	int Variation_12();
	// @cmember E_INVALIDARG GetOptions calls with no previous error object existing
	int Variation_13();
	// @cmember Valid Abort calls with previous error object existing, check error on ITransaction
	int Variation_14();
	// @cmember Valid TransactionLocal calls with previous error object existing, check error on ITransaction
	int Variation_15();
	// @cmember punkTransactionCoord NULL isoflag ignored
	int Variation_16();
	// @cmember punkTransactionCoord NULL isoflag ignored
	int Variation_17();
	// @cmember punkTransactionCoord NULL isolevel ignored
	int Variation_18();
	// @cmember punkTransactionCoord NULL isolevel ignored
	int Variation_19();
	// @cmember XACT_E_ISOLATIONLEVEL INVALID ISOLEVEL
	int Variation_20();
	// @cemeber Start/Join/XACT_E_XTIONEXISTS
	int Variation_21();
	// }}
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CTxnImmed, L"Extended Errors")
	TEST_VARIATION(1,		L"Valid ITransactionLocal calls with previous error object existing.")
	TEST_VARIATION(2,		L"Valid Abort call with previous error object existing.")
	TEST_VARIATION(3,		L"Valid SetOptions call with previous error object existing.")
	TEST_VARIATION(4,		L"Valid GetOptions call with previous error object existing.")
	TEST_VARIATION(5,		L"XACT_E_NOTSUPPORTED Commit call with previous error object existing")
	TEST_VARIATION(6,		L"XACT_E_NOISORETAIN JoinTransaction call with previous error object existing")
	TEST_VARIATION(7,		L"E_INVALIDARG GetTransactionInfo call with previous error object existing")
	TEST_VARIATION(8,		L"E_INVALIDARG GetOptions call with previous error object existing")
	TEST_VARIATION(9,		L"E_INVALIDARG SetOptions call with previous error object existing")
	TEST_VARIATION(10,		L"XACT_E_ISOLATIONLEVEL NULL & INVALID ISOLEVEL")
	TEST_VARIATION(11,		L"E_INVALIDARG GetOptionObject calls no with previous error object existing")
	TEST_VARIATION(12,		L"E_INVALIDARG SetOptions calls with no previous error object existing")
	TEST_VARIATION(13,		L"E_INVALIDARG GetOptions calls with no previous error object existing")
	TEST_VARIATION(14,		L"Valid Abort calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(15,		L"Valid TransactionLocal calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(16,		L"punkTransactionCoord NULL isoflag ignored")
	TEST_VARIATION(17,		L"punkTransactionCoord NULL isoflag ignored")
	TEST_VARIATION(18,		L"punkTransactionCoord NULL isolevel ignored")
	TEST_VARIATION(19,		L"punkTransactionCoord NULL isolevel ignored")
	TEST_VARIATION(20,		L"XACT_E_ISOLATIONLEVEL INVALID ISOLEVEL")
	TEST_VARIATION(21,		L"Start/Join/XACT_E_XTIONEXISTS")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCRetainPreserve)
//--------------------------------------------------------------------
// @class Retaining/Preserving Behavior
//
class TCNestedTransactions : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNestedTransactions,CTxnImmed);
	// }}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @Legal - Nested Transaction Supported?
	int Variation_1();
	// @Illegal Nested Transaction - 2 ITransactionJoins
	int Variation_2();
	// @Illegal Nested Transaction - Local before Join
	int Variation_3();

// }}
};

// {{ TCW_TESTCASE(TCRetainPreserve)
#define THE_CLASS TCNestedTransactions
BEG_TEST_CASE(TCNestedTransactions, CTxnImmed, L"Nested Transactions")
	TEST_VARIATION(1,		L"Legal - Nested Transaction Supported?")
	TEST_VARIATION(2,		L"Illegal Nested Transaction - 2 ITransactionJoins")
	TEST_VARIATION(3,		L"Illegal Nested Transaction - Local before Join")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}
 

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(10, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIsoLevel)
	TEST_CASE(2, TCEnListmentNULL)
	TEST_CASE(3, TCRetainPreserve)
	TEST_CASE(4, TCIRowsetUpdate)
	TEST_CASE(5, CNoTxn)
	TEST_CASE(6, CMultipleTxns)
	TEST_CASE(7, CSequence)
	TEST_CASE(8, TCExtendedErrors)
	TEST_CASE(9, TCNestedTransactions)
	TEST_CASE(10, TCEnListment)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(TCIsoLevel)
//*-----------------------------------------------------------------------
//|	Test Case:		TCIsoLevel - Isolation Level Testing
//|	Created:			04/16/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIsoLevel::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{				
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Positive ISOLEVEL
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_1()
{
	BOOL			fResults		= FALSE;
	XACTUOW			uow1;
	XACTUOW			uow2;
	ITransaction	*pITransaction	= NULL;
	const ULONG		ulIsoNum		= 9;
	ISOLATIONLEVEL	rgIsoLevel[ulIsoNum];
	ULONG			ulIndex			= 0;
	ISOLATIONLEVEL	IsoLevel;


	rgIsoLevel[0]	= ISOLATIONLEVEL_CHAOS;
	rgIsoLevel[1]	= ISOLATIONLEVEL_READUNCOMMITTED;
	rgIsoLevel[2]	= ISOLATIONLEVEL_BROWSE;
	rgIsoLevel[3]	= ISOLATIONLEVEL_CURSORSTABILITY;
	rgIsoLevel[4]	= ISOLATIONLEVEL_READCOMMITTED;
	rgIsoLevel[5]	= ISOLATIONLEVEL_REPEATABLEREAD;
	rgIsoLevel[6]	= ISOLATIONLEVEL_SERIALIZABLE;
	rgIsoLevel[7]	= ISOLATIONLEVEL_ISOLATED;
	rgIsoLevel[8]	= ISOLATIONLEVEL_UNSPECIFIED;

	for (ulIndex=0;ulIndex<ulIsoNum;ulIndex++)
	{
		//begin transaction can not handle ISOLATIONLEVEL_UNSPECIFIED so don't try it
		if (rgIsoLevel[ulIndex]==ISOLATIONLEVEL_UNSPECIFIED)
		{
			IsoLevel = ISOLATIONLEVEL_READCOMMITTED;
		}
		else
		{
			IsoLevel = rgIsoLevel[ulIndex];
		}

		//start a coordinated txn through dtc & join it
		if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,IsoLevel), TRUE))
		{	
			//get transactionjoin transaction object.
			if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
			{
				if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			rgIsoLevel[ulIndex],
																			0,
																			NULL
																		), S_OK))
				{		
					m_pChgRowset1->m_fTxnStarted	= TRUE;
					//Make sure the Txn Info is correct
					VerifyTxnInfo(pITransaction, rgIsoLevel[ulIndex], &uow1);

					//get transactionjoin transaction object.
					if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
					{
						if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					ISOLATIONLEVEL_UNSPECIFIED,
																					0,
																					NULL
																				), S_OK))
						{
							m_pChgRowset2->m_fTxnStarted	= TRUE;
							//Make sure the Txn Info is correct
							VerifyTxnInfo(pITransaction, ISOLATIONLEVEL_UNSPECIFIED, &uow2);

							//Make sure the Unit of Work Identifiers are idenitical (same coord txn)
							if (memcmp(uow1.rgb, uow2.rgb, sizeof(uow1.rgb)))				
							{
								goto CLEANUP;
							}
						}
					}
				}
			}
		}

		if (pITransaction)
		{
			m_pChgRowset1->FreeCoordTxn(pITransaction);
			pITransaction = NULL;
		}
		//free objects;
		m_pChgRowset1->FreeJoinTxn();
		m_pChgRowset2->FreeJoinTxn();

		//Release each rowset and end all transactions
		ReleaseAllRowsetsAndTxns();
	}
							
	fResults = TRUE;
CLEANUP:
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Chaos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_2()
{
	HROW			hRow			= DB_NULL_HROW;
	BOOL			fResults		= FALSE;
	IRowset			*pIRowset		= NULL;
	BOOL			fReadCommitted	= TRUE;
	HRESULT			hr				= S_OK;
	ITransaction	*pITransaction	= NULL;

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos)
	{
	 	odtLog << wszNoProviderSupport << L"CHAOS" << wszNewLine;
	 	return TEST_PASS;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS at Chaos Isolation Level
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		ISOLATIONLEVEL_CHAOS,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				
				//Make sure the Txn Info is correct
				VerifyTxnInfo(pITransaction, ISOLATIONLEVEL_CHAOS);

				//get 2nd transactionjoin transaction object.
				if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
				{
					//join the  coordinated txn from a 2nd session, ISOLATIONLEVEL_READCOMMITTED
					hr=m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		ISOLATIONLEVEL_READCOMMITTED,
																		0,
																		NULL
																	);

					//if ISOLATIONLEVEL_READCOMMITTED unsupported
					if (XACT_E_ISOLATIONLEVEL==hr)
					{
						//join the  coordinated txn from a 2nd session, ISOLATIONLEVEL_READUNCOMMITTED
						if(CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					ISOLATIONLEVEL_READUNCOMMITTED,
																					0,
																					NULL
																				),S_OK))
						{
							fReadCommitted = FALSE;
						}
					}
					else
					{
						//We should have succeeded if we support it
						CHECK(m_hr, S_OK);	
					}
					m_pChgRowset2->m_fTxnStarted	= TRUE;

					//if provider does not support IRowsetChange
					if (!m_pChgRowset2->m_fChange)
					{
						if(!CHECK((m_pChgRowset2->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
						{
							goto CLEANUP;
						}
					}
					else
					{
						//Do something in txn	
						if(!COMPARE(Insert(m_pChgRowset2), TRUE))
						{
							goto CLEANUP;
						}
					}

					//But we shouldn't be able to see the change or touch it
					//if we are at read committed on the second txn
					if (fReadCommitted)
					{
						if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
							goto CLEANUP;
					}
					//We will be able to see the change if we are in read uncommitted mode
					else
					{
						if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
							goto CLEANUP;
					
					}
				}
			}
		}
	}
	//Everything went OK if we got to here
	fResults = TRUE;
CLEANUP:
	m_pChgRowset1->FreeJoinTxn();
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	
	SAFE_RELEASE(pIRowset);
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}

// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIsoLevel::Terminate()
{
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(TCEnListmentNULL)
//*-----------------------------------------------------------------------
//|	Test Case:		TCEnListmentNULL - Enlistment Testing
//|	Created:			04/16/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnListmentNULL::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{				
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Join-Commit-UnEnlist-reEnlist
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_1()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, commit it, do not retain commit, zombies session
				if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//unelinst from MTS
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																	);
					if(S_OK==hr)
					{	
						//rowset should be functional
						if (!COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
						{
							goto CLEANUP;
						}

						//clean up any open objects
						m_pChgRowset1->ReleaseRowsetObject();

						//re-join MTS, pITransaction is dead should get XACT_E_NOENLIST
						if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					m_fIsoLevel,
																					0,
																					NULL
																				), XACT_E_NOENLIST))
						{	
							fResults = TRUE;
						}
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Join-Abort-UnEnlist-reEnlist
// @rdesc TEST_PASS or TEST_FAIL
// SESF_IN_TXN
int TCEnListmentNULL::Variation_2()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				//end the mts coordinated txn, abort it, txn not retained, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//unelinst from MTS
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																	);
					if(S_OK==hr)
					{	
						//rowset should be functional

						if (!COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
						{
							goto CLEANUP;
						}
						//clean up any open objects
						m_pChgRowset1->ReleaseRowsetObject();

						//re-join MTS, pITransaction is dead should get XACT_E_NOENLIST
						if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					m_fIsoLevel,
																					0,
																					NULL
																				), XACT_E_NOENLIST))
						{	
							fResults = TRUE;
						}
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Back to back UnEnlists
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_3()
{
	BOOL				fResults			= TEST_FAIL;
	HRESULT				hr					= S_OK;
	ITransaction		*pITransaction		= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, abort it, do not retain abort, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//unelinst from MTS
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																	);
					if(S_OK==hr)
					{	
						//no pending work, should return S_OK
						hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																			m_fIsoLevel,
																			0,
																			NULL
																		);
						if(S_OK==hr	)
						{	
							fResults = TRUE;
						}
					}
				}
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc UnEnList but no txn
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_4()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//no pending work, should return S_OK
			hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																m_fIsoLevel,
																0,
																NULL
															);
			if(S_OK==hr	)
			{
				fResults=TRUE;
			}
		}
	}

	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Join Committed session with new MTSTxn
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_5()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				//end the mts coordinated txn, commit it, do not retain commit, zombies session
				if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;

					//clean up any open objects
					m_pChgRowset1->ReleaseRowsetObject();

					//free the MTS Txn pointer
					if (pITransaction)
					{
						m_pChgRowset1->FreeCoordTxn(pITransaction);
						pITransaction = NULL;
					}
					
					//start another coordinated txn
					if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
					{	
						//Rejoin MTS
						if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					m_fIsoLevel,
																					0,
																					NULL
																				), S_OK))
						{	
							m_pChgRowset1->m_fTxnStarted	= TRUE;

							if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
							{
								//end the mts coordinated txn, commit it, do not retain commit, zombies session
								if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
								{
									m_pChgRowset1->m_fTxnStarted	= FALSE;
									//unelinst from MTS
									hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																						m_fIsoLevel,
																						0,
																						NULL
																					);
									if(S_OK==hr)
									{	
										fResults	= TRUE;
									}
								}
							}
						}
					}
				}				
			}
		}
	}
//CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	

	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Join Aborted session with new MTSTxn
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_6()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, abort it, do not retain abort, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;

					//clean up any open objects
					m_pChgRowset1->ReleaseRowsetObject();

					//free the MTS Txn pointer
					if (pITransaction)
					{
						m_pChgRowset1->FreeCoordTxn(pITransaction);
						pITransaction = NULL;
					}
					//start another coordinated txn
					if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
					{	
						//Rejoin MTS
						if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					m_fIsoLevel,
																					0,
																					NULL
																				), S_OK))
						{	
							m_pChgRowset1->m_fTxnStarted	= TRUE;

							if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
							{
								//end the mts coordinated txn, abort it, do not retain abort, zombies session
								if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
								{
									//unelinst from MTS
									hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																						m_fIsoLevel,
																						0,
																						NULL
																					);
									if(S_OK==hr)
									{	
										fResults	= TRUE;
									}
								}
							}
						}
					}
				}				
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	

	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc UnEnlists with pending work
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListmentNULL::Variation_7()
{
	BOOL				fResults			= TEST_SKIPPED;
	HRESULT				hr					= S_OK;
	ITransaction		*pITransaction		= NULL;

//this variation just blocks on sql server so it is commented out so the test does not
//hang when run against sql server

	if(gfBlocked)
	{
		return fResults;
	}
	fResults	= TEST_PASS;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
	 
				//make something pending on the txn
				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					//Do something in txn	
					if(!CHECK((m_pChgRowset1->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					//Do something in txn	
					if(!COMPARE(Insert(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}
						
				//pending work, should return S_OK but might return XACT_E_XTIONEXISTS
				hr	= m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																	);

				if (S_OK!=hr && XACT_E_XTIONEXISTS!=hr)
				{
					goto CLEANUP;
				}

				//end the mts coordinated txn, abort it, do not retain abort, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//unenlist again from MTS in case above unenlistment returned XACT_E_XTIONEXISTS
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																	);
					if(S_OK==hr)
					{	
						fResults	= TRUE;
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;	
}
// }}


BOOL TCEnListmentNULL::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCEnListment)
//*-----------------------------------------------------------------------
//|	Test Case:		TCEnListment - Enlistment Testing
//|	Created:			04/16/98
//*-----------------------------------------------------------------------
																																
//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnListment::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{				
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Join-Commit-reEnlist
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_1()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, commit it, zombies session
				if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
				{

					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//pending work, should return XACT_E_XTIONEXISTS
					if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		0,
																		NULL
																		),S_OK)){}

					//clean up any open objects
					m_pChgRowset1->ReleaseRowsetObject();

					//re-elinst from MTS
					if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), XACT_E_NOENLIST))
					{	
						fResults = TRUE;
					}
				}
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Join-Abort-reEnlist
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_2()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, abort it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//unelinst from MTS
					if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), XACT_E_NOENLIST))
					{	
						fResults = TRUE;
					}
				}
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Back to back Enlists
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_3()
{
	BOOL					fResults				= TEST_FAIL;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;

	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{ 	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				//unelinst from MTS
				if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), XACT_E_XTIONEXISTS))
				{	
						fResults = TRUE;
				}
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc mfunc Test MTS after Commit
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_4()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction			= NULL;
	XACTTRANSINFO			TXNInfo;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				//Now commit it, zombies session
				if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), XACT_E_NOTRANSACTION))
					{
						if (CHECK(pITransaction->GetTransactionInfo(&TXNInfo), XACT_E_NOTRANSACTION))
						{
							fResults = TRUE;
						}
					}				
				}				
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc  Test MTS after Abort
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_5()
{
	BOOL					fResults			= FALSE;
	HRESULT					hr					= S_OK;
	ITransaction			*pITransaction		= NULL;
	XACTTRANSINFO			TXNInfo;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;
				//Now commit it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					hr=pITransaction->Abort(NULL, FALSE, FALSE);
					if (XACT_E_NOTRANSACTION==hr || XACT_S_ABORTING==hr)
					{
						if (CHECK(pITransaction->GetTransactionInfo(&TXNInfo), XACT_E_NOTRANSACTION))
						{
							fResults = TRUE;
						}
					}
				}				
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Back to back Enlists (different Txns)
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_6()
{
	BOOL					fResults				= TEST_FAIL;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction1			= NULL;
	ITransaction			*pITransaction2			= NULL;


	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction1,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//start another coordinated txn 
			if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction2,m_fIsoLevel), TRUE))
			{	
				//join MTS
				if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction1,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
				{	
					//unelinst from MTS
					if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction2,
																				m_fIsoLevel,
																				0,
																				NULL
																			), XACT_E_XTIONEXISTS))
					{	
							fResults = TRUE;
					}
				}

			}
		}
	}

	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	ReleaseAllRowsetsAndTxns();	
	if (pITransaction1)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction1);
		pITransaction1 = NULL;
	}
	if (pITransaction2)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction2);
		pITransaction2 = NULL;
	}

	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Join-Commit-unEnlist-reEnlist
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_7()
{
	BOOL					fResults				= FALSE;
	HRESULT					hr						= S_OK;
	ITransaction			*pITransaction1			= NULL;
	ITransaction			*pITransaction2			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction1,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction1,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//end the mts coordinated txn, commit it, zombies session
				if (CHECK(pITransaction1->Commit(FALSE, FALSE, FALSE), S_OK))
				{

					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//UnEnlist into MTS
					if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																				m_fIsoLevel,
																				0,
																				NULL
																			),S_OK)){}

					//clean up any open objects
					m_pChgRowset1->ReleaseRowsetObject();

					//start 2nd coordinated (DTC) txn
					if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction2,m_fIsoLevel), TRUE))
					{	
						//re-elinst session in new DTC txn
						if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction2,
																					m_fIsoLevel,
																					0,
																					NULL
																				), S_OK))
						{	
							m_pChgRowset1->m_fTxnStarted	= TRUE;

							if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
							{
								//end the mts coordinated txn, commit it, do not retain commit, zombies session
								if (CHECK(pITransaction2->Commit(FALSE, FALSE, FALSE), S_OK))
								{
									//clean up any open objects, can't have any open statements when 
									//unenlisting because some provider might disconnect on unenlistment
									m_pChgRowset1->ReleaseRowsetObject();

									m_pChgRowset1->m_fTxnStarted	= FALSE;
									//unelinst from MTS
									hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																						m_fIsoLevel,
																						0,
																						NULL
																					);
//why is XACT_E_XTIONEXISTS acceptable here?
//					if(XACT_E_XTIONEXISTS==hr)
//					{
//						fResults=TRUE;
//						goto CLEANUP;
//					}
									if(S_OK==hr)
									{	
										fResults						= TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction1)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction1);
		pITransaction1 = NULL;
	}
	if (pITransaction2)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction2);
		pITransaction2 = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Join before firehose mode
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_8()
{
	BOOL				fResults				= FALSE;
	HRESULT				hr						= S_OK;
	ITransaction		*pITransaction			= NULL;
	ICommand			*pICommand2				= NULL;
	DBPROPSET			DBPropSet;
	DBPROP				DBProp;


	//set a prop to insure a forward only curosr
	DBPropSet.rgProperties		= &DBProp;
	DBPropSet.cProperties		= 1;
	DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;

	DBProp.dwPropertyID			= DBPROP_CANSCROLLBACKWARDS;
	DBProp.dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBProp.colid				= DB_NULLID;
	DBProp.vValue.vt			= VT_BOOL;
	V_BOOL(&(DBProp.vValue))	= VARIANT_FALSE;

	//set props
	hr = m_pChgRowset1->SetRowsetProperties(&DBPropSet, 1);	
	
	//get a rowset through the command
	m_pChgRowset1->CreateRowsetObject();

	//Another hstmt should work, either on the same hdbc or a new one
	if (!CHECK(m_pChgRowset1->m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&pICommand2), S_OK))
	{
		goto CLEANUP;
	}		

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			TEST2C_(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), XACT_E_CONNECTION_REQUEST_DENIED, E_FAIL);
			m_pChgRowset1->m_fTxnStarted	= TRUE;
		
			//end the mts coordinated txn, commit it, zombies session
			if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
			{
				fResults = TRUE;
			}
		}
	}
CLEANUP:
	//free objects;
	ReleaseAllRowsetsAndTxns();	
	SAFE_RELEASE(pICommand2);
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Join after firehose mode
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_9()
{
	BOOL				fResults			= TEST_FAIL;
	HRESULT				hr					= S_OK;
	ITransaction		*pITransaction		= NULL;
	ICommand			*pICommand2			= NULL;
	DBPROPSET			DBPropSet;
	DBPROP				DBProp;
	IRowsetChange		*pIRowsetChange		= NULL;
	IRowsetChange		*pIRowsetChange2	= NULL;


	//set a prop to insure a forward only curosr
	DBPropSet.rgProperties		= &DBProp;
	DBPropSet.cProperties		= 1;
	DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;

	DBProp.dwPropertyID			= DBPROP_CANSCROLLBACKWARDS;
	DBProp.dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBProp.colid				= DB_NULLID;
	DBProp.vValue.vt			= VT_BOOL;
	V_BOOL(&(DBProp.vValue))	= VARIANT_FALSE;

	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}

	//set props
	hr = m_pChgRowset1->SetRowsetProperties(&DBPropSet, 1);	
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//get a rowset through the command
				m_pChgRowset1->CreateRowsetObject();

				//Another command here might or might not work
				//some providers might not allow another command here if the only way to get the command
				//is by implicitly opening another connection because that under-the-covers connection
				//will not be transacted
				//if the command is allowed then that means the second command object is now also enlisted
				//in the transactoin
				hr	= m_pChgRowset1->m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&pICommand2);
				
				m_pChgRowset1->ReleaseRowsetObject();

				//if the second command object is not allowed then skip;
				if (S_OK==hr)
				{
					//end the mts coordinated txn, commit it, zombies session
					if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
					{
						//test to see if both command objects are under the global transaction
						//since the CreateCommand passed, if we are here both QI should zombie on the commit
//						CHECK(VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,(IUnknown **)&pIRowsetChange),E_FAIL);
//						CHECK(VerifyInterface(pICommand2, IID_IRowsetChange, ROWSET_INTERFACE,(IUnknown **)&pIRowsetChange2),E_FAIL);
						fResults = TEST_PASS;
					}
				}
				else
				{
					fResults	= TEST_SKIPPED;
				}
			}
		}
	}

	//free objects;
	ReleaseAllRowsetsAndTxns();	
    SAFE_RELEASE(pICommand2);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetChange2);
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc commit with 2 commands open
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_10()
{
	BOOL				fResults			= TEST_FAIL;
	HRESULT				hr					= S_OK;
	ITransaction		*pITransaction		= NULL;
	ICommand			*pICommand2			= NULL;
	IRowsetChange		*pIRowsetChange		= NULL;
	IRowsetChange		*pIRowsetChange2	= NULL;
	HACCESSOR			hAccessor			= NULL;
	void*				pData				= NULL;
	DBCOUNTITEM			cBindings			= 0;
	DBBINDING			*rgBindings			= NULL;

	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				//get a rowset through the command
				m_pChgRowset1->CreateRowsetObject();

				//Make a change
				if (!COMPARE(Change(m_pChgRowset1), TRUE))
				{
					goto CLEANUP;
				}	

				//Another command here might or might not work
				//some providers might not allow another command here if the only way to get the command
				//is by implicitly opening another connection because that under-the-covers connection
				//will not be transacted
				//if the command is allowed then that means the second command object is now also enlisted
				//in the transaction
				CHECK(m_pChgRowset1->m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&pICommand2),S_OK);

				//if the second command object is not allowed then skip;
				if (S_OK==hr)
				{
					//create a rowset off the new command and make a change on it
					CHECK((m_pChgRowset1->m_pTable)->Select(NULL,1,IID_IRowsetChange,m_pChgRowset1->m_cPropSets, 
						m_pChgRowset1->m_rgPropSets, NULL,(IUnknown **)&pIRowsetChange2,&pICommand2),S_OK);
					//get rowet interface to fetch a row

					//Get an accessor with all the writeable columns
					CHECK(GetAccessorAndBindings(pIRowsetChange2, 
						DBACCESSOR_ROWDATA,
						&hAccessor, &rgBindings, &cBindings, NULL,
						DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL,
						NULL, NULL,	DBTYPE_EMPTY, 0, NULL, NULL,
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

					// Fill buffer with appropriate data for insert of this row number
					CHECK(FillInputBindings(
							m_pChgRowset1->m_pTable,
							DBACCESSOR_ROWDATA,		
							cBindings,
							rgBindings,	
							(BYTE**)&pData,		
							99,
							0,
							NULL),S_OK);

					CHECK(pIRowsetChange2->InsertRow(NULL, hAccessor, pData, NULL),S_OK);

					//end the mts coordinated txn, abort it, zombies session
					//this should abort change from both command objects
//					if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
					if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
					{
						g_ulLastActualDelete--;
						g_ulLastActualInsert--;
						//test to see if both command objects are under the global transaction
						//since the CreateCommand passed, if we are here both objects should zombie on the commit
						//so the QI should fail
//						CHECK(VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,(IUnknown **)&pIRowsetChange),E_FAIL);
//						CHECK(VerifyInterface(pICommand2, IID_IRowsetChange, ROWSET_INTERFACE,(IUnknown **)&pIRowsetChange2),E_FAIL);
						fResults = TEST_PASS;
					}
				}
				else
				{
					fResults	= TEST_SKIPPED;
				}
			}
		}
	}
CLEANUP:
	//free objects;
	//Cleanup any out of line memory allocated in FillInputBindings and pData

    hAccessor = DB_NULL_HACCESSOR;
    SAFE_RELEASE(pICommand2);

	if(pData)
	{
		ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE*)pData, TRUE);
	}
	ReleaseAllRowsetsAndTxns();	
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetChange2);
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_RESETDATASOURCE
// this class/variation is purposely put last.  DO NOT ADD varaiton after this!
// this variaiton deletes the table, there will be no more table
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnListment::Variation_11()
{
	BOOL				fResults		= TEST_SKIPPED;
	HRESULT				hr				= S_OK;
	ITransaction		*pITransaction	= NULL;
	IDBInitialize		*pIDBInitialize	= NULL;	
	IDBProperties		*pIDBProperties	= NULL;
	const ULONG			ulProps			= 1;
	DBPROP				DBProp[ulProps];
	DBPROPSET			rgPropSets[1];
	VARIANT_BOOL		bValue;
	VARIANT_BOOL		bValue2;
	ULONG				ulRD			= 99;	
	ULONG_PTR			ulRD2			= 99;	
	
	//this code may only work if there is one session open so...
	//delete table
	if (m_pThisTestModule->m_pVoid)
	{	
		((CTable *)m_pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)m_pThisTestModule->m_pVoid;
		m_pThisTestModule->m_pVoid = NULL;
	}
	//Clean up encapsulated objects
	ReleaseAllRowsets();
	if (m_pRORowset1)
	{
		m_pRORowset1->Terminate();
		delete m_pRORowset1;
		m_pRORowset1=NULL;
	}
	if (m_pChgRowset2)
	{
		m_pChgRowset2->Terminate();
		delete m_pChgRowset2;
		m_pChgRowset2=NULL;
	}
	
	//grab the DSO interface pointers
	if (!VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize))
	{
		goto CLEANUP;
	}
	if (!VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
	{
		goto CLEANUP;
	}

	CHECK(GetProperty(DBPROP_RESETDATASOURCE,DBPROPSET_DATASOURCE, (IUnknown *)m_pThisTestModule->m_pIUnknown,&ulRD2),S_OK);
	COMPARE(0,ulRD2);

	//get the default for DBPROP_MULTIPLECONNECTIONS
	GetProperty(DBPROP_MULTIPLECONNECTIONS,DBPROPSET_DATASOURCE, (IUnknown *)m_pThisTestModule->m_pIUnknown,&bValue);
	
	//set 2 dso props cause 2 is more fun than 1
	rgPropSets->rgProperties	= &DBProp[0];
	rgPropSets->cProperties		= ulProps;
	rgPropSets->guidPropertySet	= DBPROPSET_DATASOURCE;

	//set this prop to the opposite of its default
	if (bValue==VARIANT_TRUE)
	{
		DBProp[0].dwPropertyID		= DBPROP_MULTIPLECONNECTIONS;
		DBProp[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
		DBProp[0].colid				= DB_NULLID;
		DBProp[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[0].vValue))	= VARIANT_FALSE;
	}
	else
	{
		DBProp[0].dwPropertyID		= DBPROP_MULTIPLECONNECTIONS;
		DBProp[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
		DBProp[0].colid				= DB_NULLID;
		DBProp[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[0].vValue))	= VARIANT_TRUE;
	}

	//Set the DBPROP_MULTIPLECONNECTIONS to opposite of the default
	hr	= pIDBProperties->SetProperties(1, rgPropSets);
	//make sure were ok here
	GetProperty(DBPROP_MULTIPLECONNECTIONS,DBPROPSET_DATASOURCE, (IUnknown *)m_pThisTestModule->m_pIUnknown,&bValue2);

	if (DB_E_ERRORSOCCURRED == hr)
	{
		//if this prop can not be set, pretend like it passed so the test goes on
		if (bValue2==VARIANT_TRUE)
		{
			bValue2=VARIANT_FALSE;
		}
		else
		{
			bValue2=VARIANT_TRUE;
		}
	}

	//if the set failed, something is wrong so end here
	if (S_OK != hr)
	{
		goto CLEANUP;
	}

	//if the prop was not set the test changes the 2nd of these
	//if the prop was set then the 2nd of these was changed by the provider
	if (bValue==bValue2)
	{
		goto CLEANUP;
	}

	fResults		= TEST_FAIL;

	//set DBPROP_RESETDATASOURCE to re-initialize the DSO
	DBProp[0].dwPropertyID		= DBPROP_RESETDATASOURCE;
	DBProp[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBProp[0].colid				= DB_NULLID;
	DBProp[0].vValue.vt			= VT_I4;
	DBProp[0].vValue.lVal		= DBPROPVAL_RD_RESETALL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			TESTC_(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL
																), S_OK);
			m_pChgRowset1->m_fTxnStarted	= TRUE;
		
			//Set the DBPROP_RESETDATASOURCE to reset all
			CHECK(pIDBProperties->SetProperties(1, rgPropSets),S_OK);

			//DSO should still be initialized
			CHECK(pIDBInitialize->Initialize(),DB_E_ALREADYINITIALIZED);
			
			//make sure session is still in dtc/txn
			if (!CHECK(m_pChgRowset1->AbortCoord(FALSE, pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
			{
				goto CLEANUP;
			}

			//get the value for DBPROP_MULTIPLECONNECTIONS again
			GetProperty(DBPROP_MULTIPLECONNECTIONS,DBPROPSET_DATASOURCE, (IUnknown *)m_pThisTestModule->m_pIUnknown,&bValue2);

			//this should be the default now
			COMPARE(bValue,bValue2);

			//open a rowset w/new props
			m_pChgRowset1->CreateRowsetObject();

			//Setting DBPROP_RESETDATASOURCE with an open rowset should fail
			TESTC_(pIDBProperties->SetProperties(1, rgPropSets),DB_E_ERRORSOCCURRED);
			if (rgPropSets->rgProperties->dwStatus!=DBPROPSTATUS_NOTSET &&
				rgPropSets->rgProperties->dwStatus!=DBPROPSTATUS_NOTSETTABLE)
			{
				goto CLEANUP;
			}

			//clean up any open objects
			m_pChgRowset1->ReleaseRowsetObject();

//add setting multiple DSO props w/RESETDATASOURCE here once 42987 is resloved

			fResults	= TEST_PASS;
		}
	}
CLEANUP:
	//free objects;
//	ReleaseAllRowsetsAndTxns();	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);
	return fResults;	
}
// }}


BOOL TCEnListment::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRetainPreserve)
//*-----------------------------------------------------------------------
//|	Test Case:		TCRetainPreserve - Retaining/Preserving Behavior
//|	Created:			04/16/96
//*-----------------------------------------------------------------------
 
//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{					
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::CommitRetain(ISOLEVEL fIsoLevel)
{
	BOOL					fResults				= FALSE;
	HACCESSOR				hAccessor				= DB_NULL_HACCESSOR;
	HROW 					hRow					= DB_NULL_HROW;
	HRESULT					hr;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}

				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					if(!CHECK((m_pChgRowset1->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					//Do something in txn	
					if(!COMPARE(Insert(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}

				//Now commit it with retaining TRUE.  this should fail, dtc does not support retaining
				if (!CHECK(m_hr = pITransaction->Commit(TRUE, FALSE, FALSE), XACT_E_CANTRETAIN))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					if (S_OK==m_hr)
					{
						//Our last insert was mistakenly committed, so increment row count
						m_pChgRowset1->m_fTxnPendingInsert = FALSE;
						g_InsertIncrement();
					}
					fResults=FALSE;
					goto CLEANUP;
				}
	
				//Make sure the Txn Info is correct
				// Because transaction did not start so no need to check this.
				//VerifyTxnInfo(pITransaction, m_fIsoLevel);

				fResults = TRUE;
			}
		}
	}
CLEANUP:
	ReleaseAllRowsetsAndTxns();	

	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;	
}

//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::CommitNonRetain(ISOLEVEL fIsoLevel)
{
	BOOL			fResults		= FALSE;
	HACCESSOR		hAccessor		= DB_NULL_HACCESSOR;
	HROW 			hRow			= DB_NULL_HROW;
	HRESULT			hr				= S_OK;
	ITransaction	*pITransaction	= NULL;


	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}

				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					if(!CHECK((m_pChgRowset1->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					//Do something in txn	
					if(!COMPARE(Insert(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}
				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
				{
					Sleep(SLEEP_TIME);	//Takes milliseconds as param
				}

				//Now commit it
				if (CHECK(pITransaction->Commit(FALSE, FALSE, FALSE), S_OK))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					//Our last insert was mistakenly committed, so increment row count
					m_pChgRowset1->m_fTxnPendingInsert = FALSE;
					g_InsertIncrement();
					fResults	= TRUE;
				}
				else
				{
					//commit failed, no row inserted
					g_ulLastActualInsert--;
				}
			}
		}
	}
CLEANUP:
	ReleaseAllRowsetsAndTxns();	

	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::AbortRetain(ISOLEVEL fIsoLevel)
{
	BOOL			fResults		= FALSE;
	HACCESSOR		hAccessor		= DB_NULL_HACCESSOR;
	HROW 			hRow			= DB_NULL_HROW;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}

				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					if(!CHECK((m_pChgRowset1->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					//Do something in txn	
					if(!COMPARE(Insert(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}

				//Now abort it with retaining TRUE.  this should fail, dtc does not support retaining
				if (!CHECK(m_hr = pITransaction->Abort(NULL, FALSE, FALSE), XACT_E_CANTRETAIN))
				{
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					if (S_OK==m_hr)
					{
						//Our last insert was mistakenly committed, so increment row count
						m_pChgRowset1->m_fTxnPendingInsert = FALSE;
					}
					fResults=FALSE;
					goto CLEANUP;
				}

				//Make sure the Txn Info is correct
				VerifyTxnInfo(pITransaction, m_fIsoLevel);
				
				fResults = TRUE;
			}
		}
	}
CLEANUP:	
	ReleaseAllRowsetsAndTxns();	

	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::AbortNonRetain(ISOLEVEL fIsoLevel)
{
	BOOL			fResults		= FALSE;
	HACCESSOR		hAccessor		= DB_NULL_HACCESSOR;
	HROW 			hRow			= DB_NULL_HROW;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				m_pChgRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}

				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					if(!CHECK((m_pChgRowset1->m_pTable)->Insert(GetNextRowToInsert()), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					//Do something in txn	
					if(!COMPARE(Insert(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}
				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
				{
					Sleep(SLEEP_TIME);	//Takes milliseconds as param
				}

				//Make sure the Txn Info is correct
				VerifyTxnInfo(pITransaction, fIsoLevel);
					
				//Now abort it
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//Our last insert was just aborted, so it's no longer pending
					m_pChgRowset1->m_fTxnStarted	= FALSE;
					m_pChgRowset1->m_fTxnPendingInsert = FALSE;
					fResults	= TRUE;
				}
			}
		}
	}	
CLEANUP:
	ReleaseAllRowsetsAndTxns();	

	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	return fResults;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit(fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRetainPreserve::Variation_1()
{
	IDBInitialize	*pIDBInitialize = NULL;
	int				nResult			= TEST_FAIL;	
	ULONG_PTR		ulSupportedIso	= 0,
					i;

	// Get IDBInitialize interface
	TESTC(VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize));

	// Determine if this is a supported property
	TESTC_PROVIDER(SupportedProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize));

	// Now get the property
	TESTC(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &ulSupportedIso));

	for(i=0; i<g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;
		
		if ((g_IsolationList[i].ulIsolation & ulSupportedIso) == g_IsolationList[i].ulIsolation) {
			odtLog << "\n";
			CHECK(CommitRetain(g_IsolationList[i].ulIsolation), true);
		}
		else
			odtLog << "\t-> Not Supported\n";
	}

	nResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);

	return nResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit(fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRetainPreserve::Variation_2()
{															  
	IDBInitialize	*pIDBInitialize = NULL;
	int				nResult			= TEST_FAIL;	
	ULONG_PTR		ulSupportedIso	= 0,
					i;

	// Get IDBInitialize interface
	TESTC(VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize));

	// Determine if this is a supported property
	TESTC_PROVIDER(SupportedProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize));

	// Now get the property
	TESTC(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &ulSupportedIso));

	for(i=0; i<g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;
		
		if ((g_IsolationList[i].ulIsolation & ulSupportedIso) == g_IsolationList[i].ulIsolation) {
			odtLog << "\n";
			CHECK(CommitNonRetain(g_IsolationList[i].ulIsolation), true);
		}
		else
			odtLog << "\t-> Not Supported\n";
	}

	nResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);

	return nResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort(fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRetainPreserve::Variation_3()
{
	IDBInitialize	*pIDBInitialize = NULL;
	int				nResult			= TEST_FAIL;	
	ULONG_PTR		ulSupportedIso	= 0,
					i;

	// Get IDBInitialize interface
	TESTC(VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize));

	// Determine if this is a supported property
	TESTC_PROVIDER(SupportedProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize));

	// Now get the property
	TESTC(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &ulSupportedIso));

	for(i=0; i<g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;
		
		if ((g_IsolationList[i].ulIsolation & ulSupportedIso) == g_IsolationList[i].ulIsolation) {
			odtLog << "\n";
			CHECK(AbortRetain(g_IsolationList[i].ulIsolation), true);
		}
		else
			odtLog << "\t-> Not Supported\n";
	}

	nResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);

	return nResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort(fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRetainPreserve::Variation_4()
{
	IDBInitialize	*pIDBInitialize = NULL;
	int				nResult			= TEST_FAIL;	
	ULONG_PTR		ulSupportedIso	= 0,
					i;

	// Get IDBInitialize interface
	TESTC(VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize));

	// Determine if this is a supported property
	TESTC_PROVIDER(SupportedProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize));

	// Now get the property
	TESTC(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &ulSupportedIso));

	for(i=0; i<g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;
		
		if ((g_IsolationList[i].ulIsolation & ulSupportedIso) == g_IsolationList[i].ulIsolation) {
			odtLog << "\n";
			CHECK(AbortNonRetain(g_IsolationList[i].ulIsolation), true);
		}
		else
			odtLog << "\t-> Not Supported\n";
	}

	nResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);

	return nResult;
}

// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowsetUpdate)
//*-----------------------------------------------------------------------
//|	Test Case:		TCIRowsetUpdate - Transacted rowsets in Buffered Update mode
//|	Created:			05/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowsetUpdate::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnUpdate::Init())
	// }}
	{
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Change, Abort with fRetaining = FALSE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_1()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;
	HRESULT			hExpectedHR;
	DBCOUNTITEM		cRows			= 0;
	DBROWSTATUS		*prgRowStatus	= NULL;
	HROW			*pHRows			= NULL;
	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgUpdRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}	

				//Make a change
				if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
				{
					goto CLEANUP;
				}

				//Abort txn, change was pending but the rowset is zombied so it's not anymore
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}

				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Verify pending row (or lack thereof)
				if (!m_pChgUpdRowset1->CheckPendingRow(1))
				{
					goto CLEANUP;
				}
	
				//Use second rowset to verify we can't see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
				{
					goto CLEANUP;
				}

				//delete second rowset in case it will be used again in this variation
				if (!CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK))
				{
					goto CLEANUP;
				}
		
//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//				if (m_pChgUpdRowset1->m_fAbortPreserve)
//				{
					hExpectedHR = S_OK;
//				}
//				else
//				{
//					hExpectedHR = E_UNEXPECTED;
//				}

				//Call update for all pending changes

//if the session zombied then this should return and error
//if the session didn't zombie then this is ok and change goto back end (autocommit mode at this point)
				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
																		0, 
																		NULL, 
																		&cRows, 
																		&pHRows, 
																		&prgRowStatus), hExpectedHR))
				{
					goto CLEANUP;
				}

				//if this commits and then Updates the pending rows look like a delete then 
				//insert as far as the back end data is concerened
				g_ulLastActualInsert++;
				g_ulLastActualDelete++;

				//Commit the update (no matter if it succedded or not) 
				//shouldn't work, retaining is FALSE, there is no current transaction
				if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), XACT_E_NOTRANSACTION))
				{
					goto CLEANUP;
				}
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), XACT_E_NOTRANSACTION))
				{
					goto CLEANUP;
				}

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//				if (m_pChgUpdRowset1->m_fAbortPreserve)
//				{
//					//We should now be able to see the change (if we are here Update should have worked) 
					//since we updated and our change from before the abort should have been
					//preserved
					if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
					{
						goto CLEANUP;
					}
//				}
//				else
//				{
//					//We shouldn't see a change because the rowset
//					//should have zomibied on abort, losing the pending change
//					if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
//					{
//						goto CLEANUP;	
//					}
//				}
			}
		}
	}	
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	if(pHRows)
	{
		CHECK(m_pChgUpdRowset1->m_pIRowset->ReleaseRows(cRows,pHRows,NULL,NULL,NULL),S_OK);
	}

	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	PROVIDER_FREE(pHRows);

	PROVIDER_FREE(prgRowStatus);
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_2()
{
	BOOL			fResults		= FALSE;		
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgUpdRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}	
				//Make a change
				if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
				{
					goto CLEANUP;
				}	

				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
																		0, 
																		NULL, 
																		NULL, 
																		NULL, 
																		NULL), S_OK))
				{
					goto CLEANUP;
				}	

				//Abort the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row (or lack thereof) after the abort
				//since we already called update
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can't see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Change, Commit with fRetaining = FALSE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_3()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;
	HRESULT			hExpectedHR;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgUpdRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}	

				//Make a change
				if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
				{
					goto CLEANUP;
				}	
				
				//Commit with fRetaining = FALSE, we should then be in autocommit 
				if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify pending row after commit
				if (!m_pChgUpdRowset1->CheckPendingRow(1))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can't see 
				//the change since we haven't updated yet
				if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
				{
					goto CLEANUP;
				}	

				//delete second rowset in case it will be used again in this variation
				if (!CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK))
				{
					goto CLEANUP;
				}	
					
//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//				if (m_pChgUpdRowset1->m_fCommitPreserve)
//				{
					hExpectedHR = S_OK;
//				}
//				else
//				{
//					hExpectedHR = E_UNEXPECTED;
//				}

				//Call update for all pending changes
				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
																		0, 
																		NULL, 
																		NULL, 
																		NULL, 
																		NULL), hExpectedHR))
				{
					goto CLEANUP;
				}

//if the session zombied then this should return an error
//if the session didn't zombie then this is ok and change goto back end (autocommit mode at this point)
				g_ulLastActualInsert++;
				g_ulLastActualDelete++;//g_DeleteAndInsertIncrement();

				//Commit the update (no matter if it succedded or not) 
				//shouldn't work, retaining is FALSE, there is no current transaction
				if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), XACT_E_NOTRANSACTION))
				{
					goto CLEANUP;
				}
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), XACT_E_NOTRANSACTION))
				{
					goto CLEANUP;
				}

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//				if (m_pChgUpdRowset1->m_fCommitPreserve)
//				{
					//We should now be able to see the change since we updated
					//and our change from before the commit should have been
					//preserved
					if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
					{
						goto CLEANUP;
					}
//				}
//				else
//				{
//					//We shouldn't see a change because the rowset
//					//should have zomibied on commit, losing the pending change
//					if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
//					{
//						goto CLEANUP;
//					}	
//				}
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// }}
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Commit with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_4()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgUpdRowset1->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}		

				//Make a change
				if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
				{
					goto CLEANUP;
				}	

				//Call update for all pending changes
				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
					NULL, NULL, NULL, NULL), S_OK))
				{
					goto CLEANUP;
				}	
				
				//Commit the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				g_ulLastActualInsert++;
				g_ulLastActualDelete++;//g_DeleteAndInsertIncrement();

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row 
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
				{
					goto CLEANUP;
				}
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Open read only rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_5()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				hr=m_pChgUpdRowset1->MakeRowsetReadOnly();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Commit the change to end the dist txn
	if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
	{
		goto CLEANUP;
	}	
	m_pChgUpdRowset1->m_fTxnStarted	= FALSE;
	fResults	= TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Change, Join Txn, Update, Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_6()
{
	BOOL			fResults		= FALSE;		
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	hr=m_pChgUpdRowset1->MakeRowset();		
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	//pretend the txn is started here so the change is not recored yet by this test
	m_pChgUpdRowset1->m_fTxnStarted	= TRUE;
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
	{
		goto CLEANUP;
	}	

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
																		0, 
																		NULL, 
																		NULL, 
																		NULL, 
																		NULL), S_OK))
				{
					goto CLEANUP;
				}	

				//Abort the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row (or lack thereof) after the abort
				//since we already called update
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can't see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Join Txn, Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_7()
{
	BOOL			fResults		= FALSE;		
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	hr=m_pChgUpdRowset1->MakeRowset();		
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
	{
		goto CLEANUP;
	}	

	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
															0, 
															NULL, 
															NULL, 
															NULL, 
															NULL), S_OK))
	{
		goto CLEANUP;
	}	

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				//Abort the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row (or lack thereof) after the abort
				//since we already called update
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Change, Join Txn, Update, Commit with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_8()
{
	BOOL			fResults		= FALSE;		
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	hr=m_pChgUpdRowset1->MakeRowset();		
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	//pretend the txn is started here so the change is not recored yet by this test
	m_pChgUpdRowset1->m_fTxnStarted	= TRUE;
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
	{
		goto CLEANUP;
	}	

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
																		0, 
																		NULL, 
																		NULL, 
																		NULL, 
																		NULL), S_OK))
				{
					goto CLEANUP;
				}	

				//Abort the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->CommitCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;
				g_ulLastActualInsert++;
				g_ulLastActualDelete++;//g_DeleteAndInsertIncrement();

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row (or lack thereof) after the abort
				//since we already called update
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Join Txn, Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_9()
{
	BOOL			fResults		= FALSE;		
	HRESULT			hr;
	ITransaction	*pITransaction	= NULL;

	//if provider does not support IRowsetChange
	if (!m_pChgUpdRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	hr=m_pChgUpdRowset1->MakeRowset();		
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
	{
		goto CLEANUP;
	}	

	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(	NULL, 
															0, 
															NULL, 
															NULL, 
															NULL, 
															NULL), S_OK))
	{
		goto CLEANUP;
	}	

	//start coordinated txn & join it
	if (CHECK(m_pChgUpdRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgUpdRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgUpdRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
			{
				m_pChgUpdRowset1->m_fTxnStarted	= TRUE;

				//Abort the change, with fRetaining = FALSE
				if (!CHECK(m_pChgUpdRowset1->AbortCoord(FALSE, pITransaction,m_pChgUpdRowset1->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}	
				m_pChgUpdRowset1->m_fTxnStarted	= FALSE;

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Verify no pending row (or lack thereof) after the abort
				//since we already called update
				if (!m_pChgUpdRowset1->CheckPendingRow(0))
				{
					goto CLEANUP;
				}	

				//Use second rowset to verify we can see the change 
				if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
				{
					goto CLEANUP;
				}	
			}
		}
	}
	//Everything went OK 
	fResults = TRUE;
CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	//free objects;
	m_pChgUpdRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	m_pChgUpdRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgUpdRowset2->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}	
	ReleaseAllRowsetsAndTxns();

	if (fResults)
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
BOOL TCIRowsetUpdate::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CNoTxn)
//*-----------------------------------------------------------------------
//|	Test Case:		CNoTxn - Abort and Commit called before StartTransaction
//|	Created:			05/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CNoTxn::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Start a coordinated txn, make a change, abort it, look for change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_1()
{   
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr;
	IRowset				*pIRowset		= NULL;
	BOOL				fResults		= TEST_FAIL;

	//start coordinated txn & join it
	if (!CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		goto CLEANUP;
	}

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	
	
	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
		if(!CHECK((m_pChgRowset1->m_pTable)->Update(GetNextRowToInsert()), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		//Do something in txn	
		if(!COMPARE(Change(m_pChgRowset1), TRUE))
		{
			goto CLEANUP;
		}
	}

	//Try to abort when the session never joined the coordinated Transaction
	hr=pITransaction->Abort(NULL, FALSE, FALSE);

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//	if (!m_pChgRowset1->m_fAbortPreserve)
//	{
//		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&pIRowset))
//		{
//			goto CLEANUP;
//		}
//		hr=pIRowset->RestartPosition(NULL);
//		if(DB_S_COMMANDREEXECUTED!=hr&&S_OK!=hr)//rowset should not be zombied, transaction not on session
//		{
//			goto CLEANUP;
//		}
//		//Cleanup rowsets and start with new ones, in case the first ones
//		//were zombied
//	}

	//if provider does not support IRowsetChange
	if (m_pChgRowset1->m_fChange)
	{
		//verify we can see the change, this session is not enlisted
		if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
		{
			goto CLEANUP;
		}
	}
	fResults	= TEST_PASS;
CLEANUP:
	SAFE_RELEASE(pIRowset);
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Make a change, start a coordinated txn, join it, abort it, look for change
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_2()
{
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr;
	IRowset				*pIRowset		= NULL;
	BOOL				fResults		= FALSE;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	
	
	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
		if(!CHECK((m_pChgRowset1->m_pTable)->Update(GetNextRowToInsert()), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		//Do something in txn	
		if(!COMPARE(Change(m_pChgRowset1), TRUE))
		{
			goto CLEANUP;
		}
	}


	//start coordinated txn & join it
	if (!CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		goto CLEANUP;
	}

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		m_hr = m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL
																);
		//some providers do not allow joining a txn with open rowsets
		if (m_hr==XACT_E_XTIONEXISTS)
		{
			fResults = TEST_PASS;
			goto CLEANUP;
		}
		if (m_hr != S_OK)
		{
			goto CLEANUP;
		}
			
		//Try to abort the joined and the coordinated Transaction
		hr=pITransaction->Abort(NULL, FALSE, FALSE);

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//			if (!m_pChgRowset1->m_fAbortPreserve)
//			{
//				if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&pIRowset))
//				{
//					goto CLEANUP;
//				}
//				m_hr=pIRowset->RestartPosition(NULL);
//				COMPARE(m_hr,E_UNEXPECTED);//expecting E_UNEXPECTED if the is zombied
//				//Cleanup rowsets and start with new ones, in case the first ones
//				//were zombied
//				m_pChgRowset1->ReleaseRowsetObject();
//				hr=m_pChgRowset1->MakeRowset();
//				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
//				{
//					goto CLEANUP;
//				}
//			}

			
//session might be zombied here, should it be???????
//best fix it till we know for sure
			//unenlist the  session from the dead dist txn
			if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_UNSPECIFIED,
																		0,
																		NULL
																	),S_OK))
			{
				goto CLEANUP;
			}
			ReleaseAllRowsets();
			hr=m_pChgRowset1->MakeRowset();
			if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}	

			//if provider does not support IRowsetChange
			if (m_pChgRowset1->m_fChange)
			{
				//verify we can see the change, this session is not enlisted
				if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
				{
					goto CLEANUP;
				}
			}
			else
			{
				if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
				{
					goto CLEANUP;
				}
			}
	}
	fResults	= TEST_PASS;
CLEANUP:
	ReleaseAllRowsetsAndTxns();
	SAFE_RELEASE(pIRowset);	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Make a change, start a coordinated txn, join it, commit it, look for change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_3()
{
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr;
	IRowset				*pIRowset		= NULL;
	BOOL				fResults		= TEST_FAIL;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	
	
	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
		if(!CHECK((m_pChgRowset1->m_pTable)->Update(GetNextRowToInsert()), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		//Do something in txn	
		if(!COMPARE(Change(m_pChgRowset1), TRUE))
		{
			goto CLEANUP;
		}
	}

	//start coordinated txn & join it
	if (!CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		goto CLEANUP;
	}

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL
																), S_OK))
		{
			//Try to Commit the joined and the coordinated Transaction, shouldn't 
			//affect change no matter
			hr=pITransaction->Commit(FALSE, FALSE, FALSE);

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//			if (!m_pChgRowset1->m_fCommitPreserve)
//			{
//				if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&pIRowset))
//				{
//					goto CLEANUP;
//				}
//				COMPARE(pIRowset->RestartPosition(NULL),E_UNEXPECTED);//expecting E_UNEXPECTED if the is zombied
//				//Cleanup rowsets and start with new ones, in case the first ones
//				//were zombied
//				m_pChgRowset1->ReleaseRowsetObject();
//				hr=m_pChgRowset1->MakeRowset();
//				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
//				{
//					goto CLEANUP;
//				}
//			}

//session might be zombied here, should it be???????
//best fix it till we know for sure
			//unenlist the  session from the dead dist txn
			if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_UNSPECIFIED,
																		0,
																		NULL
																	),S_OK))
			{
				goto CLEANUP;
			}
			ReleaseAllRowsets();
			hr=m_pChgRowset1->MakeRowset();
			if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}	

			
			//if provider does not support IRowsetChange
			if (m_pChgRowset1->m_fChange)
			{
				//verify we can see the change, this session is not enlisted
				if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
				{
					goto CLEANUP;
				}
			}
			else
			{
				if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
				{
					goto CLEANUP;
				}
			}
		}
	}
	fResults	= TEST_PASS;
CLEANUP:
	SAFE_RELEASE(pIRowset);	
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetOptionsObject before StartTransaction - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_4()
{	
	BOOL					fResults		= FALSE;
	XACTOPT					TxnOptions;
	ITransactionOptions		*pITxnOptions	= NULL;		

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		//Try to GetOptionsObject when we've never called StartTransaction
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{
			//Make sure we can also get the options
			if (CHECK(pITxnOptions->GetOptions(&TxnOptions), S_OK))
			{
				fResults = TRUE;
			}
			if(pITxnOptions){pITxnOptions->Release();}
		}
	}	
	m_pChgRowset1->FreeJoinTxn();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Start a coordinated txn, make a change, join the coord txn, abort it, look for change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_5()
{
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr;
	IRowset				*pIRowset		= NULL;
	BOOL				fResults		= FALSE;

	//start coordinated txn & join it
	if (!CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		goto CLEANUP;
	}

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	
	
	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
		if(!CHECK((m_pChgRowset1->m_pTable)->Update(GetNextRowToInsert()), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		//Do something in txn	
		if(!COMPARE(Change(m_pChgRowset1), TRUE))
		{
			goto CLEANUP;
		}
	}

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL
																), S_OK))
		{
			//Try to abort the joined and the coordinated Transaction
			hr=pITransaction->Abort(NULL, FALSE, FALSE);

//the following is commented out because preserve does not apply to distributed txns	
//it has yet to be decided whether sessions will be preserved at commit/abort
//until then it is provider specific where the session is preserved or not with no way to determine this before doing it
//			if (!m_pChgRowset1->m_fAbortPreserve)
//			{
//				if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&pIRowset))
//				{
//					goto CLEANUP;
//				}
//				COMPARE(pIRowset->RestartPosition(NULL),E_UNEXPECTED);//expecting E_UNEXPECTED if the is zombied
//				//Cleanup rowsets and start with new ones, in case the first ones
//				//were zombied
//				m_pChgRowset1->ReleaseRowsetObject();
//				hr=m_pChgRowset1->MakeRowset();
//				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
//				{
///					goto CLEANUP;
//				}
//			}

//session might be zombied here, should it be???????
//best fix it till we know for sure
			//unenlist the  session from the dead dist txn
			if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_UNSPECIFIED,
																		0,
																		NULL
																	),S_OK))
			{
				goto CLEANUP;
			}
			ReleaseAllRowsets();
			hr=m_pChgRowset1->MakeRowset();
			if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}	

			//if provider does not support IRowsetChange
			if (m_pChgRowset1->m_fChange)
			{
				//verify we can see the change, this session is not enlisted
				if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
				{
					goto CLEANUP;
				}
			}
			else
			{
				if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
				{
					goto CLEANUP;
				}
			}
		}
	}
	fResults	= TEST_PASS;
CLEANUP:
	SAFE_RELEASE(pIRowset);
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	return fResults;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CNoTxn::Terminate()
{
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CMultipleTxns)
//*-----------------------------------------------------------------------
//|	Test Case:		CMultipleTxns - Multiple Transactions
//|	Created:			05/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CMultipleTxns::Init()
{
	m_pTable			= NULL;
	m_ulCurrentDelete	= 1;
	m_ulCurrentInsert	= NUM_ROWS+1;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			CreateDBSession();
			//Create a second table				
			m_pTable = new CTable(m_pIOpenRowset,(LPWSTR)gwszModuleName);
				
			if (!m_pTable)
			{
				odtLog << wszMemoryAllocationError;
				return FALSE;
			}
		    					
			//Start with a table with rows	
			if (FAILED(m_pTable->CreateTable(NUM_ROWS)))
				return FALSE;
		
			//Point second rowset at a second table so their are
			//no conflicting locks -- we are only testing
			//concurrent OLE DB objects here, not concurrent
			//access to the same data.
			//m_pChgRowset2 now owns the table in the db and will drop it.
			m_pChgRowset2->SetTable(m_pTable, DELETETABLE_YES);

			return TRUE;
		}
	}
	return FALSE;
}

		 
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort two sessions joined to a coord transactions on same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_1()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	
	ITransaction	*pITransaction	= NULL;
	IRowset			*pIRowset		= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				//get 2nd transactionjoin transaction object.
				if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
				{
					//join the  coordinated txn from a 2nd session
					if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), S_OK))
					{
						hr=m_pChgRowset1->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
						hr=m_pChgRowset2->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
	
						/////////////////////////////////////////////////////////////////////
						//Do something in each txn, abort 1, and verify change was aborted
						//check 2 is then working normally
						/////////////////////////////////////////////////////////////////////
						
						//Rowset 1
						//if provider does not support IRowsetChange
						if (!m_pChgRowset1->m_fChange)
						{
							m_ulCurrentChangeRow	= GetNextRowToInsert();
							if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}

							if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}
						}
						else
						{
							//Make a change 1st session
							if(!COMPARE(Change(m_pChgRowset1), TRUE))
							{
								goto CLEANUP;
							}
							//Make a change, 2nd session
							if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
							{
								goto CLEANUP;
							}
						}
						//Now abort the whole thing (both sessions)
						if (!CHECK(m_hr = m_pChgRowset1->AbortCoord(FALSE, pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
						{
							goto CLEANUP;
						}
											
						//unenlist the second session from the dead dist txn
						if (!CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	NULL,
																					ISOLATIONLEVEL_UNSPECIFIED,
																					0,
																					NULL
																				),S_OK))
						{
							goto CLEANUP;
						}

						//AbortCoord takes care of zombied rowsets on session 1, now do it for session 2
						//So close rowsets
						m_pChgRowset2->ReleaseRowsetObject();
						//Now recreate the rowsets
						m_pChgRowset2->MakeRowset();
					
						//Shouldn't be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
						{
							goto CLEANUP;
						}
						//Shouldn't be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset2), FALSE))
						{
							goto CLEANUP;
						}
					}
				}
			}
		}
	}
	fResults = TRUE;	
CLEANUP:
	m_pChgRowset1->FreeJoinTxn();
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	
	SAFE_RELEASE(pIRowset);
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit two sessions joined to a coord transactions on same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_2()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;

	ITransaction	*pITransaction	= NULL;
	IRowset			*pIRowset		= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				//get 2nd transactionjoin transaction object.
				if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
				{
					//join the  coordinated txn from a 2nd session
					if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), S_OK))
					{
						hr=m_pChgRowset1->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
						hr=m_pChgRowset2->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}

						/////////////////////////////////////////////////////////////////////
						//Do something in each txn, commit it, and verify change was committed
						/////////////////////////////////////////////////////////////////////
						
						//Rowset 1
						//if provider does not support IRowsetChange
						if (!m_pChgRowset1->m_fChange)
						{
							m_ulCurrentChangeRow	= GetNextRowToInsert();
							if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}

							if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}
						}
						else
						{
							//Make a change
							if(!COMPARE(Change(m_pChgRowset1), TRUE))
							{
								goto CLEANUP;
							}
							//Make a change
							if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
							{
								goto CLEANUP;
							}
				 		}
						//Now commit (both sessions should be committed)
						if (!CHECK(m_hr = m_pChgRowset1->CommitCoord(FALSE,pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
						{
							goto CLEANUP;
						}

						//So close rowset
						m_pChgRowset2->ReleaseRowsetObject();

						//need to unenlist 2nd session, CommitCoord only unenlist the session that calls it 
						if (!CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	NULL,
																					m_fIsoLevel,
																					0,
																					NULL
																				), S_OK))
						{
							goto CLEANUP;
						}

						//Now recreate the rowsets
						m_pChgRowset1->MakeRowset();
						m_pChgRowset2->MakeRowset();

						//Sleep for a few seconds; this is to ensure that the back end has had
						//time to make this update visible to other transactions which
						//should see it.  This is only necessary for Access, which only does
						//this every few seconds.
						if (m_fOnAccess)
							Sleep(SLEEP_TIME);	//Takes milliseconds as param

						//Should be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
						{
							goto CLEANUP;
						}

						//Rowset 2, have to figure the count manually since we 
						//aren't on the same table as m_pChgRowset1 anymore.
													
						if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), TRUE))
						{
							//Increment our count since we already committed the delete/insert
							//but won't hit the increment before CLEANUP
							m_ulCurrentInsert++;
							m_ulCurrentDelete++;
							goto CLEANUP;
						}
					}
				}
			}
		}
	}
	//Increment our count since we committed the delete/insert
	m_ulCurrentInsert++;
	m_ulCurrentDelete++;

	fResults = TRUE;
	
CLEANUP:	
	SAFE_RELEASE(pIRowset);
	m_pChgRowset1->FreeJoinTxn();
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort two sessions joined to a coord txn on same DSO, check the 2nd after
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_3()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;

	ITransaction	*pITransaction	= NULL;
	IRowset			*pIRowset		= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				//get 2nd transactionjoin transaction object.
				if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
				{
					//join the  coordinated txn from a 2nd session
					if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), S_OK))
					{
						hr=m_pChgRowset1->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
						hr=m_pChgRowset2->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
	
						/////////////////////////////////////////////////////////////////////
						//Do something in each txn, abort 1, and verify change was aborted
						//check 2 is then working normally
						/////////////////////////////////////////////////////////////////////
						
						//Rowset 1
						//if provider does not support IRowsetChange
						if (!m_pChgRowset1->m_fChange)
						{
							m_ulCurrentChangeRow	= GetNextRowToInsert();
							if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}
						}
						else
						{
							//Make a change
							if(!COMPARE(Change(m_pChgRowset1), TRUE))
							{
								goto CLEANUP;
							}
						}
						//Now abort the whole thing
						if (!CHECK(m_hr = m_pChgRowset1->AbortCoord(FALSE, pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
						{
							goto CLEANUP;
						}		

						//unenlist the second session from the dead dist txn
						if (!CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	NULL,
																					ISOLATIONLEVEL_UNSPECIFIED,
																					0,
																					NULL
																				),S_OK))
						{
							goto CLEANUP;
						}

						//Commit-AbortCoord takes care of zombied rowsets on session 1, now do it for session 2
						//So close rowsets
						m_pChgRowset2->ReleaseRowsetObject();
						//Now recreate the rowsets
						m_pChgRowset2->MakeRowset();

						//Shouldn't be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
						{
							goto CLEANUP;
						}						
						
						//if provider does not support IRowsetChange
						if (!m_pChgRowset2->m_fChange)
						{
							m_ulCurrentChangeRow	= GetNextRowToInsert();
							if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}
						}
						else
						{
							//Make a change
							if(!COMPARE(Change(m_pChgRowset2), TRUE))
							{
								goto CLEANUP;
							}
						}

						//Should be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset2), TRUE))
						{
							goto CLEANUP;
						}
					}
				}
			}
		}
	}
	fResults = TRUE;	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	m_pChgRowset1->FreeJoinTxn();
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort and Commit with multiple commands/rowsets active, same session
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_4()
{
	BOOL			fResults = FALSE;
	IUnknown*		pIUnknown = NULL;
	HRESULT			hr;

	ITransaction	*pITransaction	= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//Make first rowset
			hr=m_pChgRowset1->MakeRowset();
			if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}

			if (!CHECK(m_pChgRowset1->GetSessionObject(IID_IUnknown, (IUnknown**)&pIUnknown), S_OK))
				goto CLEANUP;

			///////////////////////////////////////////////////////
			//Create a second command/rowset on the same DB Session
			///////////////////////////////////////////////////////

			//First we must release original session that the second object was set with
			m_pChgRowset2->ReleaseRowsetObject();
			m_pChgRowset2->ReleaseCommandObject();
			m_pChgRowset2->ReleaseDBSession();
			if (m_pChgRowset2->m_pITxnJoin)
			{
				m_pChgRowset2->m_pITxnJoin->Release();
				m_pChgRowset2->m_pITxnJoin = NULL;
			}

			//Now set session to that of our first rowset	
			m_pChgRowset2->SetDBSession(pIUnknown);

			//Make sure we use the same table so row numbering is right --
			//m_pChgRowset2 won't own this table, just use it in this variation
			m_pChgRowset2->SetTable(m_pChgRowset1->m_pTable, DELETETABLE_NO);
			
			//Make second rowset
			m_pChgRowset2->SetTxnRowsetProperties();
			hr=m_pChgRowset2->MakeRowset();
			if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}
			
			//Start Txn on our DB Session
			m_hr = m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL
											);
			//Some providers such as ODBC Provider to SQL Server will not allow this
			//because they can't have mutliple commands per transaction 
			if (m_hr == XACT_E_CONNECTION_DENIED)
			{
				odtLog << L"This provider does not support multiple commands in the same transaction, this is expected behavior for ODBC Provider to SQL Server. \n";		
				fResults = TRUE;
				goto CLEANUP;
			}
			else
			{
				//Providers such as ODBC Provider to Access can't support 
				//starting a transaction with a rowset open
				if (m_hr == E_FAIL)
				{
					//So close rowsets before creating txn
					m_pChgRowset1->ReleaseRowsetObject();
					m_pChgRowset2->ReleaseRowsetObject();

					m_hr = m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		);
					//Now recreate the rowsets
					m_pChgRowset1->MakeRowset();
					m_pChgRowset2->MakeRowset();

					//StartTxn should always work since rowsets were closed
					if (!CHECK(m_hr, S_OK))
					{
						goto CLEANUP;
					}

				}
				else	
				{
					if (!CHECK(m_hr, S_OK))
					{
						goto CLEANUP;
					}
				}
			}
			///////////////////////////////////////////////////////////////
			//Make changes to both rowsets and verify they both get aborted
			///////////////////////////////////////////////////////////////
			
			//if provider does not support IRowsetChange
			if (!m_pChgRowset1->m_fChange)
			{
				m_ulCurrentChangeRow	= GetNextRowToInsert();
				if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
				{
					goto CLEANUP;
				}
			}
			else
			{
				if(!COMPARE(Change(m_pChgRowset1), TRUE))
				{
					goto CLEANUP;
				}
			}

			//Record all the txn related things that are now true about
			//rowset 2 because it shares a session with rowset 1
			m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);

			//if provider does not support IRowsetChange
			if (!m_pChgRowset2->m_fChange)
			{
				m_ulCurrentChangeRow	= GetNextRowToInsert();
				if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
				{
					goto CLEANUP;
				}
			}
			else
			{
				if(!COMPARE(Change(m_pChgRowset2), TRUE))
				{
					goto CLEANUP;
				}
			}
			
			//Now abort the whole session
			if (!CHECK(m_hr = m_pChgRowset1->AbortCoord(FALSE, pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
			{
				goto CLEANUP;
			}

			//Commit-AbortCoord takes care of zombied rowsets on sessin 1, now do it for session 2
			//So close rowsets
			m_pChgRowset2->ReleaseRowsetObject();
			//Now recreate the rowsets
			m_pChgRowset2->MakeRowset();

			//Record all the txn related things that are now true about
			//rowset 2 because it shares a session with rowset 1
			m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);
				
			//Shouldn't be able to see the changes
			if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
				goto CLEANUP;	
			if (!COMPARE(FindChange(m_pChgRowset2), FALSE))
				goto CLEANUP;

			//the coord txn was aborted-nonretaining so start a new one in order to get a valid coord txn pointer
			if (pITransaction)
			{
				m_pChgRowset1->FreeCoordTxn(pITransaction);
				pITransaction = NULL;
			}
			//start coordinated txn & join it
			if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
			{
				//Start a new Txn since we aborted the last one non retaining
				//join Txn from our DB Session, ITransactoinJoin pointer should still be valid
				if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	),S_OK))
				{
					goto CLEANUP;
				}

				/////////////////////////////////////////////////////////////////
				//Make changes to both rowsets and verify they both get committed
				/////////////////////////////////////////////////////////////////

				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					m_ulCurrentChangeRow	= GetNextRowToInsert();
					if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					if(!COMPARE(Change(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
				}
				
				//Record all the txn related things that are now true about
				//rowset 2 because it shares a session with rowset 1
				m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);
				
				//if provider does not support IRowsetChange
				if (!m_pChgRowset2->m_fChange)
				{
					m_ulCurrentChangeRow	= GetNextRowToInsert();
					if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					if(!COMPARE(Change(m_pChgRowset2), TRUE))
					{
						goto CLEANUP;
					}
				}
				
				//Now commit the whole session
				if (!CHECK(m_hr = m_pChgRowset1->CommitCoord(FALSE, pITransaction,m_pChgRowset1->m_pITxnJoin), S_OK))
					goto CLEANUP;

				//Commit-AbortCoord takes care of zombied rowsets on sessin 1, now do it for session 2
				//So close rowsets
				m_pChgRowset2->ReleaseRowsetObject();
				//Now recreate the rowsets
				m_pChgRowset2->MakeRowset();

				//Sleep for a few seconds; this is to ensure that the back end has had
				//time to make this update visible to other transactions which
				//should see it.  This is only necessary for Access, which only does
				//this every few seconds.
				if (m_fOnAccess)
					Sleep(SLEEP_TIME);	//Takes milliseconds as param

				//Since we are sharing one session for the commit of two updates, and that
				//commit only assumes there is one change per commit, increment
				//our count for the second update on the same table
				//g_ulLastActualDelete++;
				//g_ulLastActualInsert;
				g_DeleteAndInsertIncrement();

				//Record all the txn related things that are now true about
				//rowset 2 because it shares a session with rowset 1
				m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);
				
				//Should be able to see the changes
				if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
					goto CLEANUP;	
				if (!COMPARE(FindChange(m_pChgRowset2), TRUE))
					goto CLEANUP;
			}
		}
	}
	fResults = TRUE;
CLEANUP:
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	if(pIUnknown)
		pIUnknown->Release();

	//Give m_pChgRowset2 back the ownership of the table it got
	//in Init() so that it will clean it up in it's destructor
	m_pChgRowset2->SetTable(m_pTable, DELETETABLE_YES);
	ReleaseAllRowsetsAndTxns();	
					 
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}

 
// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Multiple Sessions, only one joins the coord Transaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_5()
{
	BOOL		fResults = FALSE;
	IUnknown	*pIUnknown = NULL;
	ULONG		cPropSets = 0;
	DBPROPSET	*rgPropSets = NULL;	
	HRESULT		hr;
	
	ITransaction	*pITransaction	= NULL;
	IRowset			*pIRowset		= NULL;

	//Make first rowset
	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
	
	///////////////////////////////////////////////////////
	//Create a second command/rowset on a different DB Session
	///////////////////////////////////////////////////////
	
	//Clean up all objects existing
	m_pChgRowset2->Terminate();

	//Make a new session
	if (!COMPARE(m_pChgRowset2->Init(), TRUE))
		goto CLEANUP;	

	//Set the second different table back into m_pChgRowset2 
	//since the call to m_pChgRowset2->Terminate wiped it out
	m_pChgRowset2->SetTable(m_pTable, DELETETABLE_YES);	
		
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get 2nd transactionjoin transaction(session) object.
		if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
		{
			//join the  coordinated txn from a 2nd session
			if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{
				hr=m_pChgRowset2->MakeRowset();
				if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}
	
				///////////////////////////////////////////////////////////////
				//Make changes to both rowsets and verify only one gets aborted
				///////////////////////////////////////////////////////////////
	
				//if provider does not support IRowsetChange
				if (!m_pChgRowset1->m_fChange)
				{
					m_ulCurrentChangeRow	= GetNextRowToInsert();
					if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
					{
						goto CLEANUP;
					}

					if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
					{
						goto CLEANUP;
					}
				}
				else
				{
					if(!COMPARE(Change(m_pChgRowset1), TRUE))
					{
						goto CLEANUP;
					}
					if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
					{
						goto CLEANUP;
					}
				}

				
				//Now abort the coord txn
				if (!CHECK(m_hr = m_pChgRowset2->AbortCoord(FALSE, pITransaction,m_pChgRowset2->m_pITxnJoin), S_OK))
				{
					goto CLEANUP;
				}

				//Shouldn't be able to see the changes here
				if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), FALSE))
				{
					goto CLEANUP;
				}	
				
				//But should be able to see them on non transacted rowset
				//Note that we don't want to use FindChange() here because
				//that looks for the last changed row, which would have
				//been incremented for the second change we did, and
				//we want the first change we did (the one that actually 
				//wasn't aborted).
				if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
				{
					goto CLEANUP;
				}
			}
		}
	}
	fResults = TRUE;
CLEANUP:
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Abort and Commit from 2 session, each to it's own coord txn
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_6()
{
	BOOL			fResults	= FALSE;
	IUnknown*		pIUnknown	= NULL;
	HRESULT			hr;

	ITransaction	*pITransaction1	= NULL;
	ITransaction	*pITransaction2	= NULL;

	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction1,m_fIsoLevel), TRUE))
	{
		if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction2,m_fIsoLevel), TRUE))
		{
			//get transactionjoin transaction object.
			if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
			{
				if(!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction1,
																		m_fIsoLevel,
																		0,
																		NULL
																	),S_OK))
				{
					goto CLEANUP;
				}
				//can't have same session joined to 2 different global(coord) txns
				if(!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction2,
																			m_fIsoLevel,
																			0,
																			NULL
																		),XACT_E_XTIONEXISTS))
				{
					goto CLEANUP;
				}
				//get 2nd transactionjoin transaction object.
				if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
				{
					if(CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction2,
																				m_fIsoLevel,
																				0,
																				NULL
																			),S_OK))
					{
						hr=m_pChgRowset1->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
						hr=m_pChgRowset2->MakeRowset();
						if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
						{
							goto CLEANUP;
						}
		
						/////////////////////////////////////////////////////////////////////
						//Do something in each txn, abort 1, and verify change was aborted
						//check 2 is then working normally
						/////////////////////////////////////////////////////////////////////						
						//Rowset 1
						//if provider does not support IRowsetChange
						if (!m_pChgRowset1->m_fChange)
						{
							m_ulCurrentChangeRow	= GetNextRowToInsert();
							if(!CHECK((m_pChgRowset1->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}

							if(!CHECK((m_pChgRowset2->m_pTable)->Update(m_ulCurrentChangeRow,PRIMARY,TRUE,NULL,FALSE,g_ulFirstRowInTable), S_OK))
							{
								goto CLEANUP;
							}
						}
						else
						{
							//Make a change 1st session
							if(!COMPARE(Change(m_pChgRowset1), TRUE))
							{
								goto CLEANUP;
							}
							//Make a change, 2nd session
							if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
							{
								goto CLEANUP;
							}
						}
						//Now abort the whole thing (1 session)
						if (!CHECK(m_hr = m_pChgRowset1->AbortCoord(FALSE, pITransaction1,m_pChgRowset1->m_pITxnJoin), S_OK))
						{
							goto CLEANUP;
						}

						//no txn, abort should fail
						m_hr = m_pChgRowset1->AbortCoord(FALSE, pITransaction1,m_pChgRowset1->m_pITxnJoin);
						if (XACT_E_NOTRANSACTION!= m_hr)
						{
							COMPARE(1,0);
							goto CLEANUP;
						}
						//no txn, commit should fail
						m_hr = m_pChgRowset1->CommitCoord(FALSE, pITransaction1,m_pChgRowset1->m_pITxnJoin);
						if (XACT_E_NOTRANSACTION!= m_hr)
						{
							COMPARE(1,0);
							goto CLEANUP;
						}
						//Now commit the whole thing (1 session)
						if (!CHECK(m_hr = m_pChgRowset2->CommitCoord(FALSE, pITransaction2,m_pChgRowset2->m_pITxnJoin), S_OK))
						{
							goto CLEANUP;
						}
						//no txn, abort should fail
						m_hr = m_pChgRowset2->AbortCoord(FALSE, pITransaction2,m_pChgRowset2->m_pITxnJoin);
						if (XACT_E_NOTRANSACTION!= m_hr)
						{
							COMPARE(1,0);
							goto CLEANUP;
						}
						//no txn, commit should fail
						m_hr = m_pChgRowset2->CommitCoord(FALSE, pITransaction2,m_pChgRowset2->m_pITxnJoin);
						if (XACT_E_NOTRANSACTION!= m_hr)
						{
							COMPARE(1,0);
							goto CLEANUP;
						}

						//Commit-AbortCoord takes care of zombied rowsets on sessin 1, now do it for session 2
						//So close rowsets
						m_pChgRowset2->ReleaseRowsetObject();
						//Now recreate the rowsets
						m_pChgRowset2->MakeRowset();

						//Shouldn't be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
						{
							goto CLEANUP;
						}
						//Should be able to see the change
						if (!COMPARE(FindChange(m_pChgRowset2), TRUE))
						{
							goto CLEANUP;
						}
					}
				}
			}
		}
	}
	fResults = TRUE;
CLEANUP:
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction1)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction1);
		pITransaction1 = NULL;
	}
	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction2)
	{
		m_pChgRowset2->FreeCoordTxn(pITransaction2);
		pITransaction2 = NULL;
	}
	ReleaseAllRowsetsAndTxns();	
					 
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc two sessions, UnEnlist from one
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_7()
{
	BOOL			fResults		= FALSE;
	HRESULT			hr;
	
	ITransaction	*pITransaction	= NULL;
	IRowset			*pIRowset		= NULL;

	for (ULONG i=0;i<100;i++)
	{
		//start coordinated txn & join it
		if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
		{	
			//get transactionjoin transaction object.
			if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
			{
				if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), S_OK))
				{
					//get 2nd transactionjoin transaction object.
					if (CHECK(m_pChgRowset2->GetTxnJoin(), S_OK))
					{
						//join the  coordinated txn from a 2nd session
						if (CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	pITransaction,
																					m_fIsoLevel,
																					0,
																					NULL
																				), S_OK))
						{
							//OpenRowset in each session
							hr=m_pChgRowset1->MakeRowset();
							if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
							{
								goto CLEANUP;
							}
							hr=m_pChgRowset2->MakeRowset();
							if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
							{
								goto CLEANUP;
							}
		
							//commit the global txn (both sessin get committed)
							if (!CHECK(pITransaction->Commit(	FALSE, 
																XACTTC_ASYNC_PHASEONE, 
																0),XACT_S_ASYNC))
							{
								goto CLEANUP;
							}

							//close both rowsets
							m_pChgRowset1->ReleaseRowsetObject();
							m_pChgRowset2->ReleaseRowsetObject();

							//UnEnlist one session
							if (!CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																						m_fIsoLevel,
																						0,
																						NULL
																					), S_OK))
							{
								goto CLEANUP;
							}
							//UnEnlist the other session
							if (!CHECK(m_pChgRowset2->m_pITxnJoin->JoinTransaction	(	NULL,
																						m_fIsoLevel,
																						0,
																						NULL
																					), S_OK))
							{
								goto CLEANUP;
							}
						}
					}
				}
			}
		}

		m_pChgRowset1->FreeJoinTxn();
		m_pChgRowset2->FreeJoinTxn();
		if (pITransaction)
		{
			m_pChgRowset1->FreeCoordTxn(pITransaction);
			pITransaction = NULL;
		}
	}
	fResults = TRUE;	
CLEANUP:
	m_pChgRowset1->FreeJoinTxn();
//	m_pChgRowset2->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	ReleaseAllRowsetsAndTxns();	
	SAFE_RELEASE(pIRowset);
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }} 
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CMultipleTxns::Terminate()
{
	if (!m_fOnAccess)
	{
		ReleaseDBSession();
	}
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CSequence)
//*-----------------------------------------------------------------------
//|	Test Case:		CSequence - Sequence testing for StartTransaction and ITransactionOptions
//|	Created:			05/17/96
//*-----------------------------------------------------------------------
//--------------------------------------------------------------------
// @mfunc Start DTC, coordinated transaction
//
// @rdesc TRUE or FALSE
//
BOOL CSequence::StartCoordTxn(	ITransaction	**ppITransaction,
								ISOLEVEL		isolevel)
{
	ITransactionDispenser	*pTransactionDispenser	= NULL;
	HRESULT					hr;

	if(ppITransaction)
	{
		*ppITransaction = NULL;
	}

	// Obtain the ITransactionDispenser Interface pointer
	// by calling DtcGetTransactionManager()
	hr = DtcGetTransactionManager(
				NULL,								// LPTSTR	 pszHost,
				NULL,								// LPTSTR	 pszTmName,
				IID_ITransactionDispenser,			// /* in  */ REFIID rid,
				0,									// /* in  */ DWORD	dwReserved1,
				0, 									// /* in  */ WORD	wcbReserved2,
				NULL,								// /* in  */ void FAR * pvReserved2,
				(void**)&pTransactionDispenser 				// /* out */ void** ppvObject
				);
	if (FAILED (hr))
	{	
		return FALSE;
	}
	
	hr = pTransactionDispenser->BeginTransaction( 
					NULL,							//	/* [in]  */ IUnknown __RPC_FAR *punkOuter,
					isolevel,						//	/* [in]  */ ISOLEVEL isoLevel,
					0,								// 	/* [in]  */ ULONG isoFlags,
					NULL,							//	/* [in]  */ ITransactionOptions *pOptions 
					(ITransaction**)ppITransaction	//	/* [out] */ ITransaction **ppTransaction
					);
	
	SAFE_RELEASE(pTransactionDispenser);

	if (FAILED (hr))
	{	
		return FALSE;
	}
	
	return TRUE;
}
///--------------------------------------------------------------------
// get TxnJoin
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT	CSequence::GetTxnJoin()		// @parm [OUT] Session Pointer)
{
	if(m_pITxnJoin)
		m_pITxnJoin = NULL;

	if(m_pIOpenRowset)
	{
		if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionJoin, SESSION_INTERFACE, (IUnknown**)&m_pITxnJoin))
			return E_NOINTERFACE;

		return S_OK;
	}

	return E_FAIL;
}

//--------------------------------------------------------------------
// Free Txn Join object
// @mfunc 
//--------------------------------------------------------------------
BOOL	CSequence::FreeJoinTxn()		
{
	SAFE_RELEASE(m_pITxnJoin);
	return TRUE;
}

//--------------------------------------------------------------------
// Free Coordinated Txn object
// @mfunc 
//--------------------------------------------------------------------
BOOL	CSequence::FreeCoordTxn(ITransaction	*pITransaction)		
{
	//end any open transaction
	pITransaction->Abort(NULL,FALSE,FALSE);
	SAFE_RELEASE(pITransaction);
	return TRUE;
}

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CSequence::Init()
{		
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxn::Init())
	// }}
	{		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			if (CHECK(CreateDBSession(), S_OK))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetOptionsObject before StartTransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CSequence::Variation_1()
{
	BOOL			fResults		= FALSE;

	//start coordinated txn & join it
	if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pITxnJoin->JoinTransaction	(	m_pITransaction,
														m_fIsoLevel,
														0,
														NULL
													), S_OK))
			{
				if (CHECK(m_pITxnJoin->GetOptionsObject(&m_pITxnOptions), S_OK))
				{
					if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
					{			
						COMPARE(m_TxnOptions.ulTimeout, 0);
						COMPARE(m_TxnOptions.szDescription[0], '\0');

						//Now try setting them to something new
						m_TxnOptions.ulTimeout = 1;
						strcpy((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION);

						//Set should also work
						if (CHECK(m_pITxnOptions->SetOptions(&m_TxnOptions), S_OK))
						{
							//End Txn and start again
							CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);	
							if (m_pITransaction)
							{
								FreeCoordTxn(m_pITransaction);
								m_pITransaction = NULL;
							}							
							//unenlist the session from the dead dist txn
							if (!CHECK(m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_UNSPECIFIED,
																		0,
																		NULL
																	),S_OK))
							{
								goto CLEANUP;
							}
							//start coordinated txn & join it
							if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
							{	
								//get transactionjoin transaction object.
								if (CHECK(GetTxnJoin(), S_OK))
								{
									//start coordinated txn & join it
									if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
									{	
										m_hr = m_pITxnJoin->JoinTransaction	(	m_pITransaction,
																				m_fIsoLevel,
																				0,
																				m_pITxnOptions
																			);

										//Do Get one more time, this time while txn is active
										if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
										{
											//Should now have new values
											COMPARE(m_TxnOptions.ulTimeout, 1);
											COMPARE(strcmp((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION), 0);
										}
										//Either of these is acceptable for JoinTransaction
										if (m_hr == S_OK)
										{ 
											//Clean up txn
											CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);
											fResults = TRUE;
										}
										else
										{
											//This is OK, too
											if (m_hr == XACT_E_NOTIMEOUT)
											{
												fResults = TRUE;			
											}
										}
									}
								}	
							}
							if(m_pITxnOptions){m_pITxnOptions->Release();}
							m_pITxnOptions = NULL;
						}	
					}
				}
			}
		}
	}
CLEANUP:
	FreeJoinTxn();
	if (m_pITransaction)
	{
		FreeCoordTxn(m_pITransaction);
		m_pITransaction = NULL;
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetOptionsObject after StartTransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CSequence::Variation_2()
{
	BOOL			fResults		= FALSE;

	//start coordinated txn & join it
	if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pITxnJoin->JoinTransaction	(	m_pITransaction,
														m_fIsoLevel,
														0,
														NULL
													), S_OK))
			{
				if (CHECK(m_pITxnJoin->GetOptionsObject(&m_pITxnOptions), S_OK))
				{
					if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
					{
						//Now try setting them to something new
						m_TxnOptions.ulTimeout = TIMEOUT;
						strcpy((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION);

						//Set should also work
						if (CHECK(m_pITxnOptions->SetOptions(&m_TxnOptions), S_OK))
						{
							//End Txn and start again
							CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);	
							if (m_pITransaction)
							{
								FreeCoordTxn(m_pITransaction);
								m_pITransaction = NULL;
							}							
							//unenlist the session from the dead dist txn
							if (!CHECK(m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_UNSPECIFIED,
																		0,
																		NULL
																	),S_OK))
							{
								goto CLEANUP;
							}
							//start coordinated txn & join it
							if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
							{	
								//get transactionjoin transaction object.
								if (CHECK(GetTxnJoin(), S_OK))
								{
									m_hr=m_pITxnJoin->JoinTransaction	(	m_pITransaction,
																			m_fIsoLevel,
																			0,
																			m_pITxnOptions
																		);
									{
										//Either of these is acceptable
										if (m_hr == S_OK)
										{
											//Clean up txn
											CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);
											fResults = TRUE;
										}
										else
										{
											//This is OK, too
											if (m_hr == XACT_E_NOTIMEOUT)
											{
												fResults = TRUE;
											}
										}

										//Do Get one more time
										if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
										{
											//Should now have new values
											COMPARE(m_TxnOptions.ulTimeout, TIMEOUT);
											COMPARE(strcmp((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION), 0);
										}
									}
								}
							}
						}	
						else
						{
							CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);
						}
					}
					if(m_pITxnOptions){m_pITxnOptions->Release();}
					m_pITxnOptions = NULL;
				}	
			}
		}
	}
CLEANUP:
	FreeJoinTxn();
	if (m_pITransaction)
	{
		FreeCoordTxn(m_pITransaction);
		m_pITransaction = NULL;
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetOptions twice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CSequence::Variation_3()
{
	BOOL			fResults		= FALSE;

	//start coordinated txn & join it
	if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pITxnJoin->JoinTransaction	(	m_pITransaction,
														m_fIsoLevel,
														0,
														NULL
													), S_OK))
			{
				if (CHECK(m_pITxnJoin->GetOptionsObject(&m_pITxnOptions), S_OK))
				{
					//Set options
					m_TxnOptions.ulTimeout = TIMEOUT;
					strcpy((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION);
					
					if (CHECK(m_pITxnOptions->SetOptions(&m_TxnOptions), S_OK))
					{
						//Now try setting them to something new
						m_TxnOptions.ulTimeout = 0;
						m_TxnOptions.szDescription[0] = '\0';			
						
						if (CHECK(m_pITxnOptions->SetOptions(&m_TxnOptions), S_OK))	
						{
							if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
							{
								//Should now have latest values
								COMPARE(m_TxnOptions.ulTimeout, 0);
								COMPARE(strcmp((CHAR *)m_TxnOptions.szDescription, ""), 0);
							}
						}

						//End Txn and start again
						CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);	
						if (m_pITransaction)
						{
							FreeCoordTxn(m_pITransaction);
							m_pITransaction = NULL;
						}							
						//unenlist the session from the dead dist txn
						if (!CHECK(m_pITxnJoin->JoinTransaction	(	NULL,
																	ISOLATIONLEVEL_UNSPECIFIED,
																	0,
																	NULL
																),S_OK))
						{
							goto CLEANUP;
						}

						//start coordinated txn & join it
						if (CHECK(StartCoordTxn(&m_pITransaction,m_fIsoLevel), TRUE))
						{	
							//get transactionjoin transaction object.
							if (CHECK(GetTxnJoin(), S_OK))
							{
								m_hr=m_pITxnJoin->JoinTransaction	(	m_pITransaction,
																		m_fIsoLevel,
																		0,
																		m_pITxnOptions
																	);
								//Either of these is acceptable
								if (m_hr == S_OK)
								{
									//Clean up txn
									CHECK(m_pITransaction->Commit(FALSE, 0, 0), S_OK);
									fResults = TRUE;
								}
								else
								{
									//This is OK, too
									if (m_hr == XACT_E_NOTIMEOUT)
									{
										fResults = TRUE;
									}
									//Do Get one more time
									if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
									{
										//Should now have new values
										COMPARE(m_TxnOptions.ulTimeout, TIMEOUT);
										COMPARE(strcmp((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION), 0);
									}
								}
							}	
							if(m_pITxnOptions){m_pITxnOptions->Release();}
							m_pITxnOptions = NULL;	
						}
					}
				}
			}
		}
	}
CLEANUP:
	FreeJoinTxn();
	if (m_pITransaction)
	{
		FreeCoordTxn(m_pITransaction);
		m_pITransaction = NULL;
	}

	if (fResults)
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
BOOL CSequence::Terminate()
{	
	if (!m_fOnAccess)
	{
		ReleaseDBSession();
		SAFE_RELEASE(m_pITxnJoin);
		SAFE_RELEASE(m_pITransaction);
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxn::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//|	Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			07/31/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{
	BOOL			fResults		= FALSE;

	m_pITxnOptions	= NULL;
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	{
		//}}
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			//get transactionjoin transaction object.
			if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
			{	
				if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&m_pITxnOptions),S_OK))
				{
					fResults = TRUE;
				}
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	fResults = TRUE;
	return fResults;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid ITransactionJoin calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	ULONG				ulTxnLvl1		= 0;
	ULONG				ulTxnLvl2		= 0;
	BOOL				fResults		= FALSE;
	ITransactionOptions *pITxnOptions	= NULL;
	XACTTRANSINFO		TxnInfo;
	ITransaction		*pITransaction	= NULL;


	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionJoin method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//create an error object
				m_pExtError->CauseError();

				//Try to get the txn options interface
				if (CHECK(m_hr=m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions),S_OK))
				{
					//Do extended check following GetOptionsObject
					fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, m_hr);
				}
				//create an error object
				m_pExtError->CauseError();

				//call GeTransactionInfo
				if (CHECK(m_hr=pITransaction->GetTransactionInfo(&TxnInfo), S_OK))
				{
					//Do extended check following GetTransactionInfo
					fResults &= XCHECK(pITransaction, IID_ITransaction, m_hr);
				}

				//create an error object
				m_pExtError->CauseError();

				//End the txn
				if(CHECK(m_hr=pITransaction->Commit(FALSE, 0, 0), S_OK))	
				{
					//Do extended check following Commit
					fResults &= XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
				//Try to get the txn options interface
				if (CHECK(m_hr=pITransaction->GetTransactionInfo(&TxnInfo),XACT_E_NOTRANSACTION))
				{
					//Do extended check following GetOptionsObject
					fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();

	if(pITxnOptions){pITxnOptions->Release();}
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid Abort call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL			fResults		= FALSE;
	BOID			boidReason		= BOID_NULL;	//Just so we have some value
	ITransaction	*pITransaction	= NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionJoin method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//create an error object
				m_pExtError->CauseError();

				if(CHECK(m_hr=pITransaction->Abort(&boidReason, FALSE, FALSE), S_OK))
				{
					//Do extended check following Abort
					fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
			}
		}
	}

	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Valid SetOptions call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
  	XACTOPT			TxnOptions;
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;
	
	//For each method of the interface, first create an error object on

	//the current thread, then try get S_OK from the ITransactionOptions method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//Set time out to non zero
	TxnOptions.ulTimeout = MAX_ULONG;
	TxnOptions.szDescription[0] = '\0';

 	//create an error object
	m_pExtError->CauseError();
	
	if (m_pITxnOptions)
	{
		//Set our time out option
		if (CHECK(m_hr=m_pITxnOptions->SetOptions(&TxnOptions), S_OK))
		{
			//Do extended check following SetOptions
			fResults = XCHECK(m_pITxnOptions, IID_ITransactionOptions, m_hr);
			
			//start coordinated txn & join it
			if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
			{	
				//get transactionjoin transaction object.
				if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
				{
					//enlist in coordinated txn
					m_hr = m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			m_pITxnOptions
																		);
					//free objects;
					m_pChgRowset1->FreeJoinTxn();
					ReleaseAllRowsetsAndTxns();

					//Either of these is acceptable
					if (m_hr == S_OK || m_hr == XACT_E_NOTIMEOUT)
					{
						fResults = TRUE;
					}
				}
			}
		}	
	}
	else
	{
		return TEST_SKIPPED;
	}
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	if(fResults)   
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}
  

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid GetOptions call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	ITransactionOptions		*pITxnOptions	= NULL;
	BOOL					fResults		= FALSE;
	XACTOPT					TxnOptions;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionOptions method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		//enlist in coordinated txn
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{
			//create an error object
			m_pExtError->CauseError();
	
			//Make sure we can also get the options with
			//TxnOptions allocated on stack (so we know
			//provider isn't trying to free the memory)
			if (CHECK(m_hr=pITxnOptions->GetOptions(&TxnOptions), S_OK))
			{
				//Do extended check following GetOptions
				fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
			}

			if(pITxnOptions){pITxnOptions->Release();}
		}
	}
	m_pChgRowset1->FreeJoinTxn();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

  
// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTSUPPORTED Commit call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
   	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
  
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
			   	//create an error object
				m_pExtError->CauseError();

				//Check for proper return code for given parameters
				if( CHECK(m_hr=pITransaction->Commit(FALSE, (XACTTC_SYNC_PHASETWO + 1), 0), XACT_E_NOTSUPPORTED))
				{
					//Do extended check following Commit
					fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
			}
		}
	}
	//Cleanup
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOISORETAIN JoinTransaction call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
  	
	//create an error object
	m_pExtError->CauseError();

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		1,
																		NULL
																	), XACT_E_NOISORETAIN))
			{	
				//Do extended check following JoinTransaction
				fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, XACT_E_NOISORETAIN);
			}
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		-1,
																		NULL
																	), XACT_E_NOISORETAIN))
			{	
				//Do extended check following JoinTransaction
				fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, XACT_E_NOISORETAIN);
			}
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//Do extended check following JoinTransaction
				fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, S_OK);
				fResults=TRUE;
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;	
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetTransactionInfo call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_7()
{
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
  
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//create an error object
				m_pExtError->CauseError();

				//Check for invalid arg on pinfo = NULL
				if (CHECK(m_hr=pITransaction->GetTransactionInfo(NULL), E_INVALIDARG))
				{
					//Do extended check following GetTransactionInfo
					fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
			}
		}
	}
	//Cleanup
	ReleaseAllRowsetsAndTxns();
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptions call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_8()
{
	BOOL				fResults		= FALSE;
	ITransactionOptions *pITxnOptions	= NULL;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransitionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  
	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{	
			//create an error object
			m_pExtError->CauseError();
	
			//Expect invalid arg for NULL
			if (CHECK(m_hr=pITxnOptions->SetOptions(NULL), E_INVALIDARG))
			{
				//Do extended check following SetOptions
				fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
			}
		}
		if(pITxnOptions){pITxnOptions->Release();}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG SetOptions call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_9()
{
	ITransactionOptions *pITxnOptions	= NULL;
	BOOL				fResults		= FALSE;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  
	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{	
		  	//create an error object
			m_pExtError->CauseError();
	
			//Make sure we can also get the options
			if (CHECK(m_hr=pITxnOptions->GetOptions(NULL), E_INVALIDARG))
			{
				//Do extended check following GetOptions
				fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
			}
			if(pITxnOptions){pITxnOptions->Release();}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc  XACT_E_ISOLATIONLEVEL NULL & INVALID ISOLEVEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_10()
{
	BOOL				fResults		= FALSE;
	ITransaction		*pITransaction	= NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		ISOLATIONLEVEL_CHAOS | 
																			ISOLATIONLEVEL_READUNCOMMITTED | 
																			ISOLATIONLEVEL_READCOMMITTED | 
																			ISOLATIONLEVEL_SERIALIZABLE,
																		0,
																		NULL
																	), XACT_E_ISOLATIONLEVEL))
			{		
				//Do extended check following JoinTransaction
				fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, XACT_E_ISOLATIONLEVEL);
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptionObject calls no with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_11()
{
	BOOL				fResults		= FALSE;
	ITransactionJoin	*pITransactionJoin	= NULL;
	
	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(NULL), E_INVALIDARG))
		{
			//Do extended check following GetOptionsObject
			fResults = XCHECK(pITransactionJoin, IID_ITransactionJoin, m_hr);
		}
	}

	//free objects;
	m_pChgRowset1->FreeJoinTxn();

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG SetOptions calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_12()
{
	BOOL				fResults		= FALSE;
	ITransactionOptions *pITxnOptions	= NULL;	

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{
			//Expect invalid arg for NULL
			if (CHECK(m_hr=pITxnOptions->SetOptions(NULL), E_INVALIDARG))
			{
				//Do extended check following SetOptions
				fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
			}
		}
		if(pITxnOptions){pITxnOptions->Release();}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptions calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_13()
{
	ITransactionOptions *pITxnOptions	= NULL;
	BOOL				fResults		= FALSE;

	//get transactionjoin transaction object.
	if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
	{
		if (CHECK(m_pChgRowset1->m_pITxnJoin->GetOptionsObject(&pITxnOptions), S_OK))
		{
			//Make sure we can also get the options
			if (CHECK(m_hr=pITxnOptions->GetOptions(NULL), E_INVALIDARG))
			{
				//Do extended check following GetOptions
				fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
			}
		}
		if(pITxnOptions){pITxnOptions->Release();}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();

	if (fResults)  
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
  

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid Abort calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_14()
{
	BOOL			fResults		= FALSE;
	BOID			boidReason		= BOID_NULL;	//Just so we have some value
	ITransaction	*pITransaction	= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{			
		//For each method of the interface, first create an error object on
		//the current thread, then try get S_OK from the ITransactionJoin method.
		//We then check extended errors on ITransaction interface to verify nothing
		//is set since an error object shouldn't exist following a successful call.
   	
		//Check for proper return code for given parameters
		//create an error object
		m_pExtError->CauseError();

		if(CHECK(m_hr=pITransaction->Abort(&boidReason, FALSE, FALSE), S_OK))
		{
			//Do extended check on ITransaction following Abort
			fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
		}
	}

	//free objects;
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Valid TransactionLocal calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_15()
{
	ULONG			ulTxnLvl1		= 0;
	BOOL			fResults		= FALSE;
	XACTTRANSINFO	TxnInfo;
	ITransaction	*pITransaction	= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{			
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//For each method of the interface, first create an error object on
				//the current thread, then try get S_OK from the ITransactionJoin method.
				//We then check extended errors on ITransaction interface to verify nothing
				//is set since an error object shouldn't exist following a successful call.
   	
				//create an error object
				m_pExtError->CauseError();

				//call GeTransactionInfo
				if (CHECK(m_hr=pITransaction->GetTransactionInfo(&TxnInfo), S_OK))
				{
					//Do extended check following JoinTransaction
					fResults = XCHECK(pITransaction, IID_ITransaction, m_hr);
				}

				//This will succeed if nested txns are supported, else return
				//XACT_E_XTIONEXISTS
				m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTxnLvl1);
		
				if (m_hr == S_OK)
				{
					//Make sure the nesting worked
					COMPARE(ulTxnLvl1, 2);	
				}
				else
				{
					//Make sure we got the right return value
					CHECK(m_hr, XACT_E_XTIONEXISTS);
				}

				//create an error object
				m_pExtError->CauseError();

				//End the txn -- this should end both levels, if applicable
				if(CHECK(m_hr=pITransaction->Commit(FALSE, 0, 0), S_OK))	
				{
					//Do extended check following Commit
					fResults &= XCHECK(pITransaction, IID_ITransaction, m_hr);
				}
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc punkTransactionCoord NULL isoflag ignored
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_16()
{
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;
	HRESULT			hr				= S_OK;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
  	
	//create an error object
	m_pExtError->CauseError();

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//end the mts coordinated txn, abort it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//unelinst from MTS, punkTransactionCoord NULL isoflag ignored
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		5,
																		NULL
																	);
					if(XACT_E_XTIONEXISTS==hr || S_OK==hr)
					{
						fResults=TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;	
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc punkTransactionCoord NULL isoflag ignored
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_17()
{
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;
	HRESULT			hr				= S_OK;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
  	
	//create an error object
	m_pExtError->CauseError();

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//end the mts coordinated txn, abort it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//unelinst from MTS, punkTransactionCoord NULL isoflag ignored
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		m_fIsoLevel,
																		-1,
																		NULL
																	);
					if(XACT_E_XTIONEXISTS == hr || S_OK == hr)
					{
						fResults=TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;	
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc punkTransactionCoord NULL isolevel ignored
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_18()
{
	BOOL				fResults		= FALSE;
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr				= S_OK;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//end the mts coordinated txn, abort it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//punkTransactionCoord NULL isolevel ignored
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		ISOLATIONLEVEL_CHAOS | 
																					ISOLATIONLEVEL_READUNCOMMITTED | 
																					ISOLATIONLEVEL_READCOMMITTED | 
																					ISOLATIONLEVEL_SERIALIZABLE,
																		0,
																		NULL
																	);
					if(XACT_E_XTIONEXISTS==hr || S_OK==hr)
					{
						fResults=TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}



// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc  punkTransactionCoord NULL isolevel ignored
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_19()
{
	BOOL				fResults		= FALSE;
	ITransaction		*pITransaction	= NULL;
	HRESULT				hr				= S_OK;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//end the mts coordinated txn, abort it, zombies session
				if (CHECK(pITransaction->Abort(NULL, FALSE, FALSE), S_OK))
				{
					//punkTransactionCoord NULL isolevel ignored
					hr=m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
																		0,
																		0,
																		NULL
																	);
					if(XACT_E_XTIONEXISTS==hr || hr==S_OK)
					{
						fResults=TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}
CLEANUP:
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc  XACT_E_ISOLATIONLEVEL INVALID ISOLEVEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_20()
{
	BOOL				fResults		= FALSE;
	ITransaction		*pITransaction	= NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionJoin method.
	//We then check extended errors to verify the right extended error behavior.
	
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//join MTS, invalid isolevel
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		0,
																		0,
																		NULL
																	), XACT_E_ISOLATIONLEVEL))
			{	
				//Do extended check following JoinTransaction
				fResults=TRUE;
				fResults = XCHECK(m_pChgRowset1->m_pITxnJoin, IID_ITransactionJoin, XACT_E_ISOLATIONLEVEL);
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Start/Join/XACT_E_XTIONEXISTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_21()
{
	ULONG			ulTxnLvl1		= 0;
	BOOL			fResults		= FALSE;
	ITransaction	*pITransaction	= NULL;
	HRESULT			hr;


	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{			
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//enlist in coordinated txn
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//This will succeed if nested txns are supported, else return
				//XACT_E_XTIONEXISTS
				hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTxnLvl1);	

				if (S_OK==hr||XACT_E_XTIONEXISTS==hr)
				{
					//enlist in coordinated txn
					if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																				m_fIsoLevel,
																				0,
																				NULL
																			), XACT_E_XTIONEXISTS))
					{	
						fResults	= TRUE;
					}
				}
			}
		}
	}
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();

	if(fResults)
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
BOOL TCExtendedErrors::Terminate()
{
	if (!m_fOnAccess)
	{
		// Release the Objects
		if( m_pITxnOptions )
			if(m_pITxnOptions){m_pITxnOptions->Release();}
	}
	//if (m_pExtError)
	//	delete m_pExtError;
	//m_pExtError = NULL;
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCNestedTransactions)
//*-----------------------------------------------------------------------
//|	Test Case:		TCNestedTransactions 
//|	Created:			02/14/98
//*-----------------------------------------------------------------------
 
//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNestedTransactions::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{
		//the jet engine (access) does not support distributed transactions
		if (m_fOnAccess)
		{
			return TEST_SKIPPED;
		}
		else
		{
			return TEST_PASS;
		}
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Legal - Nested Transaction Supported?
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNestedTransactions::Variation_1()
{
	BOOL					fResults				= FALSE;
	ITransaction			*pITransaction			= NULL;

	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				//this local transaction should start a nested transaction
				m_hr	= m_pChgRowset1->StartTxn(m_fIsoLevel);
				//S_OK	if nesting is supported
				//XACT_EXTIONEXISTS if nesting is not supported
				if (S_OK==m_hr || XACT_E_XTIONEXISTS==m_hr)	
				{
					fResults = TRUE;
				}
			}
		}
	}
	
	//clean up these txns
	//end the mts coordinated txn, abort it, zombies session
	pITransaction->Abort(NULL, FALSE, FALSE);

	//unenlist the  session from the dead dist txn
	m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
													ISOLATIONLEVEL_UNSPECIFIED,
													0,
													NULL
												);
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();	

	return fResults;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Illegal Nested Transaction - 2 ITransactionJoins
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNestedTransactions::Variation_2()
{
	BOOL					fResults				= FALSE;
	ITransaction			*pITransaction			= NULL;
	HRESULT					hr						= S_OK;

	if(gfBlocked)
	{
		return TEST_SKIPPED;
	}
	//start coordinated txn & join it
	if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
	{	
		//get transactionjoin transaction object.
		if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
		{
			//coordinated txns are not allowed if local txns are current
			if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																		m_fIsoLevel,
																		0,
																		NULL
																	), S_OK))
			{	
				// Online Books
				// Microsoft OLE DB\OLE DB Programmer's Reference\Part 1 Introduction to OLE DB\
				// Chapter 15: Transactions\Distributed Transactions
				// It is an error to call ITransactionJoin::JoinTransaction if the session is already enlisted in a 
				// local transaction. In addition, the provider might require that any existing distributed transactions 
				// be committed or aborted before enlisting in a new distributed transaction. 

	
				hr = m_pChgRowset1->m_pITxnJoin->JoinTransaction (	pITransaction,
																	m_fIsoLevel,
																	0,
																	NULL);
				if (CHECK(hr, XACT_E_XTIONEXISTS))
				{	
					fResults = TRUE;
				}
			}
		}
	}

  //clean up these txns
	//end the mts coordinated txn, abort it, zombies session
	pITransaction->Abort(NULL, FALSE, FALSE);

	//unenlist the  session from the dead dist txn
	m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
													ISOLATIONLEVEL_UNSPECIFIED,
													0,
													NULL
												);
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();	

	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Illegal Nested Transaction - Local before Join
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNestedTransactions::Variation_3()
{
	BOOL					fResults				= FALSE;
	ITransaction			*pITransaction			= NULL;

	//this local transaction should start a local transaction
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//start coordinated txn & join it
		if (CHECK(m_pChgRowset1->StartCoordTxn(&pITransaction,m_fIsoLevel), TRUE))
		{	
			//get transactionjoin transaction object.
			if (CHECK(m_pChgRowset1->GetTxnJoin(), S_OK))
			{
				//coordinated txns are not allowed if local txns are current
				if (CHECK(m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	pITransaction,
																			m_fIsoLevel,
																			0,
																			NULL
																		), XACT_E_XTIONEXISTS))
				{	
					fResults = TRUE;
				}
			}
		}
	}

	//clean up these txns
	//end the mts coordinated txn, abort it, zombies session
	pITransaction->Abort(NULL, FALSE, FALSE);

	//unenlist the  session from the dead dist txn
	m_pChgRowset1->m_pITxnJoin->JoinTransaction	(	NULL,
													ISOLATIONLEVEL_UNSPECIFIED,
													0,
													NULL
												);
	//free objects;
	m_pChgRowset1->FreeJoinTxn();
	if (pITransaction)
	{
		m_pChgRowset1->FreeCoordTxn(pITransaction);
		pITransaction = NULL;
	}
	
	ReleaseAllRowsetsAndTxns();	

	return fResults;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNestedTransactions::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}

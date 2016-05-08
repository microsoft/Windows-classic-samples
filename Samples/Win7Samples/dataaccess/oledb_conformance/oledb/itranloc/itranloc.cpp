//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1996-2000 Microsoft Corporation.
//
// @doc 
//
// @module itranloc.CPP | Test Module for Local (Non Coordinated) Transactions
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "itranloc.h"
#include "txnbase.hpp"		//Base classes for transacted rowsets

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xc9066071, 0x97e4, 0x11cf, { 0x97, 0x77, 0x00, 0xaa, 0x00, 0xbd, 0xf9, 0x52 }};
DECLARE_MODULE_NAME("ITransactionLocal");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for Local Transactions");
DECLARE_MODULE_VERSION(839560026);
// }}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This must be defined so that the base class constructors 
//in txnbase.hpp know what to use for this test module's name.
LPWSTR	gwszThisModuleName = L"itranloc";	


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	ITransactionLocal			*pITxnLocal = NULL;
	HRESULT						hr;
	IConnectionPointContainer	*pIConnectionPointContainer	= NULL;

	if (ModuleCreateDBSession(pThisTestModule))
	{
		//Fail gracefully and quit module if we don't support local transactions
		if (SUCCEEDED(hr = pThisTestModule->m_pIUnknown2->QueryInterface(
			IID_ITransactionLocal, (void **)&pITxnLocal)))
		{
			SAFE_RELEASE(pITxnLocal);
		}
		else
		{
			//Make sure we returned E_NOINTERFACE if we've failed
			if (pThisTestModule->m_pError->Validate(hr,	
								LONGSTRING(__FILE__), __LINE__, E_NOINTERFACE))
				odtLog <<L"ITransactionLocal is not supported.\n";
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
			return FALSE;

		//Init last actual insert number to last row we inserted
		g_ulLastActualInsert = NUM_ROWS;

		//First row in table is 1
		g_ulFirstRowInTable = 1;

		g_ulLastActualDelete = 0;

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
	//@cmember Creates an readonly rowset
//	virtual HRESULT	MakeRowsetReadOnly();
	//@cmember Frees an updateable buffered rowset and frees IRowsetUpdate interface
	virtual HRESULT	FreeRowset();	
	//@cmember make sure properties are set
	virtual HRESULT	ReSetProps();	
	//@cmember Commits current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the commit.
	//Also makes sure we have a current IRowsetUpdate on the preserved rowset
	virtual HRESULT	Commit(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);
	//@cmember Aborts current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the abort.
	//Also makes sure we have a current IRowsetUpdate on the preserved rowset
	virtual HRESULT Abort(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);		
	//@cmember Verifies pending change after a transaction commit or abort
	BOOL CheckPendingRow(ETXN_END_TYPE eTxnEndType, ULONG cPendingRows);	
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
	CTxnChgUpdRowset *	m_pChgUpdRowset1;
	//@cmember	Pointer to Rowset object that supports Changes in buffered mode
	CTxnChgUpdRowset *	m_pChgUpdRowset2;
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
	const ULONG		cProps = 5;
	DBPROP			DBProp[cProps];	
	DBPROPSET		RowsetPropSet;
	BOOL			fInit	=	FALSE;
	ULONG			i		= 0;

	m_pIRowsetUpdate = NULL;
	m_pIRowset = NULL;
	m_cPendingRows = 0;
	m_rgPendingRows = NULL;
	m_rgPendingStatus = NULL;

	//Set properties for change ability and determine support
	fInit = CTxnChgRowset::Init();

//	if (!m_rgPropSets)
//	{
		if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
		{
			//Now reset all properties and include IRowsetUpdate
			DBProp[0].dwPropertyID = DBPROP_IRowsetChange;
			DBProp[0].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[0].colid=DB_NULLID;
			DBProp[0].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[0].vValue))=VARIANT_TRUE;

			DBProp[1].dwPropertyID= DBPROP_IRowsetUpdate;
			DBProp[1].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[1].colid=DB_NULLID;
			DBProp[1].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[1].vValue))=VARIANT_TRUE;

			DBProp[2].dwPropertyID= DBPROP_UPDATABILITY;
			DBProp[2].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[2].colid=DB_NULLID;
			DBProp[2].vValue.vt=VT_I4;	
			DBProp[2].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;
			i=3;
		}

		if (g_rgDBPrpt[IDX_OtherInsert].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERINSERT;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}

		if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}

		//Build one set of rowset properties
		RowsetPropSet.rgProperties		= DBProp;
		RowsetPropSet.cProperties		= i;
		RowsetPropSet.guidPropertySet	= DBPROPSET_ROWSET;
	
		//Now we'll be ready to execute and get a buffered mode rowset
		SetRowsetProperties(&RowsetPropSet, 1);	
//	}
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
		DBProp[0].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBProp[0].colid=DB_NULLID;
		DBProp[0].vValue.vt=VT_BOOL;
		V_BOOL(&(DBProp[0].vValue))=VARIANT_TRUE;

		DBProp[1].dwPropertyID= DBPROP_IRowsetUpdate;
		DBProp[1].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBProp[1].colid=DB_NULLID;
		DBProp[1].vValue.vt=VT_BOOL;
		V_BOOL(&(DBProp[1].vValue))=VARIANT_TRUE;

		DBProp[2].dwPropertyID= DBPROP_UPDATABILITY;
		DBProp[2].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBProp[2].colid=DB_NULLID;
		DBProp[2].vValue.vt=VT_I4;	
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
// Makes sure we have a current IRowsetUpdate even if we recreate the rowset
// @mfunc Commit
//--------------------------------------------------------------------
HRESULT	CTxnChgUpdRowset::Commit(BOOL fPreserveRowset, BOOL fNewUnitOfWork)
{
	//If the rowset won't last but we expect it to, release and 
	//NULL out our interfaces, we'll create a new ones later
	if (!m_fCommitPreserve && fPreserveRowset)
	{
		SAFE_RELEASE(m_pIRowsetUpdate);
		SAFE_RELEASE(m_pIRowset);
	}

	//Do the commit
	return (CTxnChgRowset::Commit(fPreserveRowset, fNewUnitOfWork));
	
	return m_hr;
}

//--------------------------------------------------------------------
// Makes sure we have a current IRowsetUpdate even if we recreate the rowset
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT	CTxnChgUpdRowset::Abort(BOOL fPreserveRowset, BOOL fNewUnitOfWork)
{
	//If the rowset won't last but we expect it to, release and 
	//NULL out IRowsetUpdate, we'll create a new one later
	if (!m_fAbortPreserve && fPreserveRowset)
	{
		SAFE_RELEASE(m_pIRowsetUpdate);
		SAFE_RELEASE(m_pIRowset);
	}
		
	//Do the abort
	return (CTxnChgRowset::Abort(fPreserveRowset, fNewUnitOfWork));
}


//--------------------------------------------------------------------
// Verifies pending row from change, if rowset isn't zombied
// @mfunc CheckPendingRow
//--------------------------------------------------------------------
BOOL CTxnChgUpdRowset::CheckPendingRow(ETXN_END_TYPE eTxnType, ULONG cPendingRows)
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
	//Check the correct property to see if the end of the txn
	//zombied the rowset, in which case we can't check pending rows
	if (eTxnType == EABORT && !m_fAbortPreserve)
		return TRUE;
	if (eTxnType == ECOMMIT && !m_fCommitPreserve)
		return TRUE;

	//We know the rowset should be in tact, so we can check PendingRows
	if (CHECK(m_pIRowsetUpdate->GetPendingRows(NULL,
		DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED, 
		&m_cPendingRows, &m_rgPendingRows, 
		&m_rgPendingStatus), cPending))
	{
		//THESE ARE COMMENTED OUT BECAUSE THESE >CAN< BE NULL & ZERO IF NO ROW ARE PENDING
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
			m_pChgUpdRowset1->m_pIRowsetUpdate = NULL;
			m_pChgUpdRowset1->m_pIRowset = NULL;
			m_pChgUpdRowset1->m_cPendingRows = 0;
			m_pChgUpdRowset1->m_rgPendingRows = NULL;
			m_pChgUpdRowset1->m_rgPendingStatus	= NULL;

			m_pChgUpdRowset2->m_pIRowsetUpdate = NULL;
			m_pChgUpdRowset2->m_pIRowset = NULL;
			m_pChgUpdRowset2->m_cPendingRows = 0;
			m_pChgUpdRowset2->m_rgPendingRows = NULL;
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
	ULONG	ul;
	
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
	ULONG	ul;
	
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
	// @cmember Read Uncommitted
	int Variation_1();
	// @cmember Read Committed
	int Variation_2();
	// @cmember Repeatable Read
	int Variation_3();
	// @cmember Serializable
	int Variation_4();
	// @cmember Chaos
	int Variation_5();
	// @cmember Unspecified
	int Variation_6();
	// }}
};
// {{ TCW_TESTCASE(TCIsoLevel)
#define THE_CLASS TCIsoLevel
BEG_TEST_CASE(TCIsoLevel, CTxnImmed, L"Isolation Level Testing")
	TEST_VARIATION(1,		L"Read Uncommitted")
	TEST_VARIATION(2,		L"Read Committed")
	TEST_VARIATION(3,		L"Repeatable Read")
	TEST_VARIATION(4,		L"Serializable")
	TEST_VARIATION(5,		L"Chaos")
	TEST_VARIATION(6,		L"Unspecified")
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
 

// {{ TCW_TEST_CASE_MAP(TCITxnReturnVals)
//--------------------------------------------------------------------
// @class ITransaction Return Values
//
class TCITxnReturnVals : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCITxnReturnVals,CTxnImmed);
	// }}
 
	// @cmember Txn Object interface for session
	ITransactionOptions * m_pITxnOptions;
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();	
	
	// {{ TCW_TESTVARS()
	// @cmember Abort with pboidReason = NULL - S_OK
	int Variation_1();
	// @cmember Abort with pboidReason = BOID_NULL - S_OK
	int Variation_2();
	// @cmember Abort with fAsync=TRUE - XACT_E_NOTSUPPORTED or S_OK
	int Variation_3();
	// @cmember Abort with fAsync=FALSE - S_OK
	int Variation_4();
	// @cmember Call commit on one thread, Abort on another - XACT_E_ALREADYINPROGRESS
	int Variation_5();
	// @cmember Call abort  on one thread and then on another - XACT_S_ABORTING
	int Variation_6();
	// @cmember Commit with grfTC =XACTC_ASYNC - XACT_E_NOTSUPPORTED or S_OK
	int Variation_7();
	// @cmember Commit with grfTC =XACTC_SYNC_PHASEONE -  XACT_E_NOTSUPPORTED or S_OK
	int Variation_8();
	// @cmember Commit with grfRM!=0 -  XACT_E_NOTSUPPORTED
	int Variation_9();
	// @cmember Call Abort on one thread, Commit on another - XACT_E_ALREADYINPROGRESS
	int Variation_10();
	// @cmember Call commit on one thread, and then on another - XACT_E_ALREADYINPROGRESS
	int Variation_11();
	// @cmember Commit with reported supported grfTC values - S_OK
	int Variation_12();
	// @cmember GetTransactionInfo with pinfo = NULL
	int Variation_13();
	// @cmember GetOptionsObject with ppOptions = NULL - E_INVALIDARG
	int Variation_14();
	// @cmember StartTransaction twice in a row - S_OK or XACT_E_XTIONEXISTS
	int Variation_15();
	// @cmember StartTransaction with a non zero timeout - S_OK or XACT_E_NOTIMEOUT
	int Variation_16();
	// @cmember StartTransaction with zero timeout - S_OK
	int Variation_17();
	// @cmember StartTransaction with pulTransactionLevel = NULL - S_OK
	int Variation_18();
	// @cmember StartTransaction with isoFlags != 0 - XACT_E_NOISORETAIN
	int Variation_19();
	// @cmember StartTransaction with isoLevel = invalid - XACT_E_ISOLATIONLEVEL
	int Variation_20();
	// @cmember GetOptions with pOptions = NULL - E_INVALIDARG
	int Variation_21();
	// @cmember GetOptions with pOptions allocated on stack - S_OK
	int Variation_22();
	// @cmember SetOptions with max szDescription
	int Variation_23();
	// @cmember SetOptions with ppOptions = NULL - E_INVALIDARG
	int Variation_24();
	// @cmember Commit with grfTC =XACTC_SYNC_PHASETWO -   S_OK
	int Variation_25();
	// }}
};

// {{ TCW_TESTCASE(TCITxnReturnVals)
#define THE_CLASS TCITxnReturnVals
BEG_TEST_CASE(TCITxnReturnVals, CTxnImmed, L"ITransaction Return Values")
	TEST_VARIATION(1,		L"Abort with pboidReason = NULL - S_OK")
	TEST_VARIATION(2,		L"Abort with pboidReason = BOID_NULL - S_OK")
	TEST_VARIATION(3,		L"Abort with fAsync=TRUE - XACT_E_NOTSUPPORTED or S_OK")
	TEST_VARIATION(4,		L"Abort with fAsync=FALSE - S_OK")
	TEST_VARIATION(5,		L"Call commit on one thread, Abort on another - XACT_E_ALREADYINPROGRESS")
	TEST_VARIATION(6,		L"Call abort  on one thread and then on another - XACT_S_ABORTING")
	TEST_VARIATION(7,		L"Commit with grfTC =XACTC_ASYNC - XACT_E_NOTSUPPORTED or S_OK")
	TEST_VARIATION(8,		L"Commit with grfTC =XACTC_SYNC_PHASEONE -  XACT_E_NOTSUPPORTED or S_OK")
	TEST_VARIATION(9,		L"Commit with grfRM!=0 -  XACT_E_NOTSUPPORTED")
	TEST_VARIATION(10,		L"Call Abort on one thread, Commit on another - XACT_E_ALREADYINPROGRESS")
	TEST_VARIATION(11,		L"Call commit on one thread, and then on another - XACT_E_ALREADYINPROGRESS")
	TEST_VARIATION(12,		L"Commit with reported supported grfTC values - S_OK")
	TEST_VARIATION(13,		L"GetTransactionInfo with pinfo = NULL")
	TEST_VARIATION(14,		L"GetOptionsObject with ppOptions = NULL - E_INVALIDARG")
	TEST_VARIATION(15,		L"StartTransaction twice in a row - S_OK or XACT_E_XTIONEXISTS")
	TEST_VARIATION(16,		L"StartTransaction with a non zero timeout - S_OK or XACT_E_NOTIMEOUT")
	TEST_VARIATION(17,		L"StartTransaction with zero timeout - S_OK")
	TEST_VARIATION(18,		L"StartTransaction with pulTransactionLevel = NULL - S_OK")
	TEST_VARIATION(19,		L"StartTransaction with isoFlags != 0 - XACT_E_NOISORETAIN")
	TEST_VARIATION(20,		L"StartTransaction with isoLevel = invalid - XACT_E_ISOLATIONLEVEL")
	TEST_VARIATION(21,		L"GetOptions with pOptions = NULL - E_INVALIDARG")
	TEST_VARIATION(22,		L"GetOptions with pOptions allocated on stack - S_OK")
	TEST_VARIATION(23,		L"SetOptions with max szDescription")
	TEST_VARIATION(24,		L"SetOptions with ppOptions = NULL - E_INVALIDARG")
	TEST_VARIATION(25,		L"Commit with grfTC =XACTC_SYNC_PHASETWO -   S_OK")
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
	// @cmember Change, Abort with fRetaining = TRUE, Update
	int Variation_1();
	// @cmember Change, Abort with fRetaining = FALSE, Update
	int Variation_2();
	// @cmember Change, Update, Abort with fRetaining = TRUE
	int Variation_3();
	// @cmember Change, Update, Abort with fRetaining = FALSE
	int Variation_4();
	// @cmember Change, Commit with fRetaining = TRUE, Update
	int Variation_5();
	// @cmember Change, Commit with fRetaining = FALSE, Update
	int Variation_6();
	// @cmember Change, Update, Commit with fRetaining = TRUE
	int Variation_7();
	// @cmember Change, Update, Commit with fRetaining = FALSE
	int Variation_8();
	// @cmember Open read only rowset
	int Variation_9();	// }}
};
// {{ TCW_TESTCASE(TCIRowsetUpdate)
#define THE_CLASS TCIRowsetUpdate
BEG_TEST_CASE(TCIRowsetUpdate, CTxnUpdate, L"Transacted rowsets in Buffered Update mode")
	TEST_VARIATION(1,		L"Change, Abort with fRetaining = TRUE, Update")
	TEST_VARIATION(2,		L"Change, Abort with fRetaining = FALSE, Update")
	TEST_VARIATION(3,		L"Change, Update, Abort with fRetaining = TRUE")
	TEST_VARIATION(4,		L"Change, Update, Abort with fRetaining = FALSE")
	TEST_VARIATION(5,		L"Change, Commit with fRetaining = TRUE, Update")
	TEST_VARIATION(6,		L"Change, Commit with fRetaining = FALSE, Update")
	TEST_VARIATION(7,		L"Change, Update, Commit with fRetaining = TRUE")
	TEST_VARIATION(8,		L"Change, Update, Commit with fRetaining = FALSE")
	TEST_VARIATION(9,		L"Open read only rowset")
	END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CNoTxn)
//--------------------------------------------------------------------
// @class Abort and Commit called before StartTransaction
//
class CNoTxn : public CSessionObject {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	//ITransactionLocal interface
	ITransactionLocal * m_pITxnLocal;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CNoTxn,CSessionObject);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_1();
	// @cmember Commit with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_2();
	// @cmember GetTransactionInfo before StartTransaction - XACT_E_NOTRANSACTION
	int Variation_3();
	// @cmember GetOptionsObject before StartTransaction - S_OK
	int Variation_4();
	// @cmember Abort with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_5();
	// @cmember Commit with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION 
	int Variation_6();
	// }}
};
// {{ TCW_TESTCASE(CNoTxn)
#define THE_CLASS CNoTxn
BEG_TEST_CASE(CNoTxn, CSessionObject, L"Abort and Commit called before StartTransaction")
	TEST_VARIATION(1,		L"Abort with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION ")
	TEST_VARIATION(2,		L"Commit with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION ")
	TEST_VARIATION(3,		L"GetTransactionInfo before StartTransaction - XACT_E_NOTRANSACTION")
	TEST_VARIATION(4,		L"GetOptionsObject before StartTransaction - S_OK")
	TEST_VARIATION(5,		L"Abort with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION ")
	TEST_VARIATION(6,		L"Commit with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION ")
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

	//@cmember Local Transaction interface
	ITransactionLocal * m_pITxnLocal;
	//@cmember Transaction Options interface	
	ITransactionOptions * m_pITxnOptions;
	//@cmember Options struct
 	XACTOPT				m_TxnOptions;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	//@cmember Cleanup function for this derived class
	virtual void ReleaseAllRowsetsAndTxns()
	{
		//Cleanup our transaction, if we have one
		if (m_pITxnLocal)
			m_pITxnLocal->Commit(FALSE, 0, 0);
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
	// @cmember XACT_E_NOISORETAIN StartTransaction call with previous error object existing
	int Variation_6();
	// @cmember E_INVALIDARG GetTransactionInfo call with previous error object existing
	int Variation_7();
	// @cmember E_INVALIDARG GetOptionObject call with previous error object existing
	int Variation_8();
	// @cmember XACT_E_NOTRANSACTION Abort call with previous error object existing
	int Variation_9();
	// @cmember E_INVALIDARG GetOptions call with previous error object existing
	int Variation_10();
	// @cmember E_INVALIDARG SetOptions call with previous error object existing
	int Variation_11();
	// @cmember XACT_E_ISOLATIONLEVEL StartTransaction calls no with previous error object existing
	int Variation_12();
	// @cmember E_INVALIDARG GetOptionObject calls no with previous error object existing
	int Variation_13();
	// @cmember XACT_E_NOTRANSACTION Abort calls with no previous error object existing
	int Variation_14();
	// @cmember XACT_E_NOTRANSACTION Commit calls with no previous error object existing
	int Variation_15();
	// @cmember XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing
	int Variation_16();
	// @cmember E_INVALIDARG SetOptions calls with no previous error object existing
	int Variation_17();
	// @cmember E_INVALIDARG GetOptions calls with no previous error object existing
	int Variation_18();
	// @cmember Valid Abort calls with previous error object existing, check error on ITransaction
	int Variation_19();
	// @cmember Valid TransactionLocal calls with previous error object existing, check error on ITransaction
	int Variation_20();
	// @cmember XACT_E_NOTSUPPORTED Commit calls with previous error object existing, check error on ITransaction
	int Variation_21();
	// @cmember E_INVALIDARG GetTransactionInfo calls with previous error object existing, check error on ITransaction
	int Variation_22();
	// @cmember XACT_E_NOTRANSACTION Abort calls with previous error object existing, check error on ITransaction
	int Variation_23();
	// @cmember XACT_E_NOTRANSACTION Abort calls with no previous error object existing, check error on ITransaction
	int Variation_24();
	// @cmember XACT_E_NOTRANSACTION Commit calls with no previous error object existing, check error on ITransaction
	int Variation_25();
	// @cmember XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing, check error on ITransaction
	int Variation_26();
	// @cmember DB_E_OBJECTOPEN the provider does not support starting a new transaction with existing open rowset objects
	int Variation_27();
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
	TEST_VARIATION(6,		L"XACT_E_NOISORETAIN StartTransaction call with previous error object existing")
	TEST_VARIATION(7,		L"E_INVALIDARG GetTransactionInfo call with previous error object existing")
	TEST_VARIATION(8,		L"E_INVALIDARG GetOptionObject call with previous error object existing")
	TEST_VARIATION(9,		L"XACT_E_NOTRANSACTION Abort call with previous error object existing")
	TEST_VARIATION(10,		L"E_INVALIDARG GetOptions call with previous error object existing")
	TEST_VARIATION(11,		L"E_INVALIDARG SetOptions call with previous error object existing")
	TEST_VARIATION(12,		L"XACT_E_ISOLATIONLEVEL StartTransaction calls no with previous error object existing")
	TEST_VARIATION(13,		L"E_INVALIDARG GetOptionObject calls no with previous error object existing")
	TEST_VARIATION(14,		L"XACT_E_NOTRANSACTION Abort calls with no previous error object existing")
	TEST_VARIATION(15,		L"XACT_E_NOTRANSACTION Commit calls with no previous error object existing")
	TEST_VARIATION(16,		L"XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing")
	TEST_VARIATION(17,		L"E_INVALIDARG SetOptions calls with no previous error object existing")
	TEST_VARIATION(18,		L"E_INVALIDARG GetOptions calls with no previous error object existing")
	TEST_VARIATION(19,		L"Valid Abort calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(20,		L"Valid TransactionLocal calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(21,		L"XACT_E_NOTSUPPORTED Commit calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(22,		L"E_INVALIDARG GetTransactionInfo calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(23,		L"XACT_E_NOTRANSACTION Abort calls with previous error object existing, check error on ITransaction")
	TEST_VARIATION(24,		L"XACT_E_NOTRANSACTION Abort calls with no previous error object existing, check error on ITransaction")
	TEST_VARIATION(25,		L"XACT_E_NOTRANSACTION Commit calls with no previous error object existing, check error on ITransaction")
	TEST_VARIATION(26,		L"XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing, check error on ITransaction")
	TEST_VARIATION(27,		L"DB_E_OBJECTOPEN the provider does not support starting a new transaction with existing open rowset objects")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCMultipleCommands)
//--------------------------------------------------------------------
// @class Tests mutliple commands with and without transactions
//
class TCMultipleCommands : public CRowsetObject {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMultipleCommands,CRowsetObject);
	// }}
 
	//@cmember Flag used if we are running ODBC Provider, indicating whether we 
	//are using the  Microsoft SQL Server ODBC driver	
	BOOL	m_fMSSQLServer;

	//@cmember Flag used if we are running ODBC Provider, indicating whether our ODBC driver
	//supports multiple hstmts
	BOOL	m_fMultipleHstmts;

	//@cmember Interface to second command object
	ICommand *	m_pICommand2;

	//@cmember Flag telling if current command is in firehose mode
	BOOL	m_fFireHoseMode;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	//@cmember Turns backwards scrolling on or off to disable/enable firehose mode
	HRESULT	SetFirehoseMode(ICommand * pICommand, BOOL fOn);

	//@cmember Verifies second command can be use to get data, then releases command
	HRESULT	VerifyAndReleaseCommand(ICommand ** pICommand, BOOL fCanScrollBackwards);

	//@cmember Starts true firehose mode by getting a row, which ties up the TDS connection
	HRESULT GetRows(IUnknown * pRowset);

	// {{ TCW_TESTVARS()
	// @cmember First rowset in Firehose mode with no transaction
	int Variation_1();
	// @cmember First rowset in Firehose mode with a transaction
	int Variation_2();
	// @cmember Second rowset in firehose mode with no transaction
	int Variation_3();
	// @cmember Second rowset in firehose mode with a transaction
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(TCMultipleCommands)
#define THE_CLASS TCMultipleCommands
BEG_TEST_CASE(TCMultipleCommands, CRowsetObject, L"Tests mutliple commands with and without transactions")
	TEST_VARIATION(1,		L"First rowset in Firehose mode with no transaction")
	TEST_VARIATION(2,		L"First rowset in Firehose mode with a transaction")
	TEST_VARIATION(3,		L"Second rowset in firehose mode with no transaction")
	TEST_VARIATION(4,		L"Second rowset in firehose mode with a transaction")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors2)
//--------------------------------------------------------------------
// @class Check extended errors on multipla commands
//
class TCExtendedErrors2 : public TCMultipleCommands {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors2,TCMultipleCommands);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember open a txn in firehose mode -- E_FAIL
	int Variation_1();
	// @cmember second rowset with the first rowset in firehose mode within a txn -- E_FAIL
	int Variation_2();
	// @cmember create two connection with one in firehose, start txn -- XACT_E_CONNECTION_DENIED
	int Variation_3();
	// }}
};
// {{ TCW_TESTCASE(TCExtendedErrors2)
#define THE_CLASS TCExtendedErrors2
BEG_TEST_CASE(TCExtendedErrors2, TCMultipleCommands, L"Check extended errors on multipla commands")
	TEST_VARIATION(1,		L"open a txn in firehose mode -- E_FAIL")
	TEST_VARIATION(2,		L"second rowset with the first rowset in firehose mode within a txn -- E_FAIL")
	TEST_VARIATION(3,		L"create two connection with one in firehose, start txn -- XACT_E_CONNECTION_DENIED")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(TCRetainPreserve)
//--------------------------------------------------------------------
// @class Retaining/Preserving Behavior
//
class TCITransactionObject : public CTxnImmed {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCITransactionObject,CTxnImmed);
	// }}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @Legal - Legal - Nested and ITransactionObject
	int Variation_1();
	// @Illegal Illegal ulTransactionLevel arg GetTransactionObject
	int Variation_2();
	// @Illegal Illegal ppITransaction arg GetTransactionObject
	int Variation_3();
	// @Illegal just Nested
	int Variation_4();
// }}
};
// {{ TCW_TESTCASE(TCITransactionObject)
#define THE_CLASS TCITransactionObject
BEG_TEST_CASE(TCITransactionObject, CTxnImmed, L"ITransactionObject/Nested Transactions")
	TEST_VARIATION(1,		L"Legal - Nested and ITransactionObject")
	TEST_VARIATION(2,		L"Illegal ulTransactionLevel arg GetTransactionObject")
	TEST_VARIATION(3,		L"Illegal ppITransaction arg GetTransactionObject")
	TEST_VARIATION(4,		L"just Nested")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}
 
// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(11, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIsoLevel)
	TEST_CASE(2, TCRetainPreserve)
	TEST_CASE(3, TCITxnReturnVals)
	TEST_CASE(4, TCIRowsetUpdate)
	TEST_CASE(5, CNoTxn)
	TEST_CASE(6, CMultipleTxns)
	TEST_CASE(7, CSequence)
	TEST_CASE(8, TCExtendedErrors)
	TEST_CASE(9, TCMultipleCommands)
	TEST_CASE(10, TCExtendedErrors2)
	TEST_CASE(11, TCITransactionObject)
	
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
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read Uncommitted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_1()
{
	BOOL	fResults = FALSE;
	XACTUOW	uow1, uow2;

	//If no provider support for this isolation level, just return pass
	if (!m_fReadUncommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READ UNCOMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}

	//Start txn we're testing at Read Uncommitted Isolation Level
	if (CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_READUNCOMMITTED), S_OK))
	{
		//Make sure the Txn Info is correct
		VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, ISOLATIONLEVEL_READUNCOMMITTED, &uow1);

		//We'll verify by using a second txn in Read Committed Isolation Level
		if (CHECK(m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READCOMMITTED), S_OK))
		{
			//Make sure the Txn Info is correct
			VerifyTxnInfo(m_pChgRowset2->m_pITxnLocal, ISOLATIONLEVEL_READCOMMITTED, &uow2);
 
			//Make sure the Unit of Work Identifiers are unique
			if (memcmp(uow1.rgb, uow2.rgb, sizeof(uow1.rgb)))				
				fResults = TRUE;
				
		}
	}
	
	//Release each rowset and end all transactions
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Read Committed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_2()
{	
	
	BOOL	fResults = FALSE;
	XACTUOW	uow1, uow2;

	//If no provider support for this isolation level, just return pass
	if (!m_fReadCommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READ COMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}

	//Start txn we're testing at Read Committed Isolation Level
	if (CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_READCOMMITTED), S_OK))
	{
		//Make sure the Txn Info is correct
		VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, ISOLATIONLEVEL_READCOMMITTED, &uow1);

		//We'll verify by using a second txn in Read Committed Isolation Level
		if (CHECK(m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READCOMMITTED), S_OK))
		{
			//Make sure the Txn Info is correct
			VerifyTxnInfo(m_pChgRowset2->m_pITxnLocal, ISOLATIONLEVEL_READCOMMITTED, &uow2);

			//Make sure the Unit of Work Identifiers are unique
			if (memcmp(uow1.rgb, uow2.rgb, sizeof(uow1.rgb)))
				fResults = TRUE;
		}
	}
	
	//Release each rowset and end all transactions
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;		

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Repeatable Read
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;
	BOOL	fResults = FALSE;
	IRowset * pIRowset = NULL;
	XACTUOW	uow1, uow2;
	
	//If no provider support for this isolation level, just return pass
	if (!m_fRepeatableRead)
	{
	 	odtLog << wszNoProviderSupport << L"REPEATABLE READ" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Start txn we're testing at Repeatable Read Isolation Level	
	if (CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_REPEATABLEREAD), S_OK))		
	{
		//Make sure the Txn Info is correct
		VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, ISOLATIONLEVEL_REPEATABLEREAD, &uow1);

		//We'll verify by using a second txn in Read Committed Isolation Level
		if (CHECK(m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READCOMMITTED), S_OK))
		{
			//Make sure the Txn Info is correct
			VerifyTxnInfo(m_pChgRowset2->m_pITxnLocal, ISOLATIONLEVEL_READCOMMITTED, &uow2);

			//Make sure the Unit of Work Identifiers are unique
			if (memcmp(uow1.rgb, uow2.rgb, sizeof(uow1.rgb)))				
				 fResults = TRUE;	
		}
	}

//CLEANUP:
	
	//Get rid of hRow and pIRowset if we haven't already
	if (hRow != DB_NULL_HROW && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);

	SAFE_RELEASE(pIRowset);
	
	//Release each rowset and end all transactions
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Serializable
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_4()
{
	HROW	hRow = DB_NULL_HROW;
	BOOL	fResults = FALSE;
	IRowset * pIRowset = NULL;
	XACTUOW	uow1, uow2;
	
	//If no provider support for this isolation level, just return pass
	if (!m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"SERIALIZABLE" << wszNewLine;
	 	return TEST_PASS;
	}		

	//Start txn we're testing at Repeatable Read Isolation Level	
	if (CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_SERIALIZABLE), S_OK))		
	{
		//Make sure the Txn Info is correct
		VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, ISOLATIONLEVEL_SERIALIZABLE, &uow1);

		//We'll verify by using a second txn in Read Committed Isolation Level
		if (CHECK(m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READCOMMITTED), S_OK))
		{
			//Make sure the Txn Info is correct
			VerifyTxnInfo(m_pChgRowset2->m_pITxnLocal, ISOLATIONLEVEL_READCOMMITTED, &uow2);

			//Make sure the Unit of Work Identifiers are unique
			if (memcmp(uow1.rgb, uow2.rgb, sizeof(uow1.rgb)))				
				fResults = TRUE;
		}
	}

//CLEANUP:
	
	//Get rid of hRow and pIRowset if we haven't already
	if (hRow != DB_NULL_HROW && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	
	SAFE_RELEASE(pIRowset);

	//Release each rowset and end all transactions
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Chaos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_5()
{
	HROW	hRow = DB_NULL_HROW;
	BOOL	fResults = FALSE;
	IRowset * pIRowset = NULL;
	BOOL	fReadCommitted = TRUE;

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos)
	{
	 	odtLog << wszNoProviderSupport << L"CHAOS" << wszNewLine;
	 	return TEST_PASS;
	}

	//Start txn we're testing at Chaos Isolation Level	
	if (!CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_CHAOS), S_OK))		
		goto CLEANUP;

	//Make sure the Txn Info is correct
	VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, ISOLATIONLEVEL_CHAOS);

	
	//We'll verify by using a second txn in Read Committed Isolation Level
	if ((m_hr = m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READCOMMITTED))
		== XACT_E_ISOLATIONLEVEL)
	{
		//Try read uncommitted if that wasn't supported.  We didn't try read uncommited 
		//right away because SQL Server does not support updateable read uncommitted
		if (CHECK(m_pChgRowset2->StartTxn(ISOLATIONLEVEL_READUNCOMMITTED), S_OK))
			fReadCommitted = FALSE;
	}
	else
		//We should have succeeded if we support it
		CHECK(m_hr, S_OK);	

	//Rowset 2 should be able to change a row
	if(!COMPARE(Change(m_pChgRowset2), TRUE))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

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

	//Everything went OK if we got to here
	fResults = TRUE;

CLEANUP:

	//Release each rowset and end all transactions
	ReleaseAllRowsetsAndTxns();
 
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Unspecified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIsoLevel::Variation_6()
{
	//Isolation level UNSPECIFIED is not supported for any interface
	//other than ITransctionJoin, so try it on ITransactionLocal
	if (CHECK(m_pChgRowset1->StartTxn(ISOLATIONLEVEL_UNSPECIFIED), XACT_E_ISOLATIONLEVEL))
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
BOOL TCIsoLevel::Terminate()
{
	//Try to set autocommit isolevel.  This may or may not be supported,
	//but if it is, we want to set it here to make sure we are in a
	//low level isolation so that we see everything when in autocommit
	m_pChgRowset1->SetAutoCommitIsoLevel(DBPROPVAL_TI_READUNCOMMITTED);

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
		return TRUE;
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

	BOOL		fResults	= FALSE;
	HACCESSOR	hAccessor	= DB_NULL_HACCESSOR;
	IRowset		*pIRowset	= NULL;
	HROW 		hRow		= DB_NULL_HROW;
	HRESULT		hr;
IRowset		*pIRowset2	= NULL;

	//Start Txn
	if (!CHECK(m_pChgRowset1->StartTxn(fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Do something in txn	
	if(!COMPARE(Insert(m_pChgRowset1), TRUE))
		goto CLEANUP;
	
	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//If we are supposed to be in a zombie state after commit, get hRow, accessor 
	//and pIRowset now so we can verify they can be released later
	if (!m_pChgRowset1->m_fCommitPreserve)
	{
		//Get hRow to hang onto of a row already existing
		if(!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, &hRow), TRUE))
		 	goto CLEANUP;
		
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
			(IUnknown **)&pIRowset))
			goto CLEANUP;

		if (!CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_pChgRowset1->m_cReadBindings,
			m_pChgRowset1->m_rgReadBindings, m_pChgRowset1->m_cbReadRowSize, &hAccessor, NULL), S_OK))
			goto CLEANUP;

	}

	//Now commit it
	if(XACT_E_NOTSUPPORTED==m_pChgRowset1->m_pITxnLocal->Commit(TRUE, XACTTC_ASYNC_PHASEONE, 0))
	{
		if(XACT_E_NOTSUPPORTED==m_pChgRowset1->m_pITxnLocal->Commit(TRUE, XACTTC_SYNC_PHASEONE, 0))
		{
			if(XACT_E_NOTSUPPORTED==m_pChgRowset1->m_pITxnLocal->Commit(TRUE, XACTTC_SYNC_PHASETWO, 0))
			{
				if(XACT_E_NOTSUPPORTED==m_pChgRowset1->m_pITxnLocal->Commit(TRUE, XACTTC_SYNC, 0))
				{
					if(XACT_E_NOTSUPPORTED==m_pChgRowset1->m_pITxnLocal->Commit(TRUE, 0, 0))
					{
						//commit retaining not supported
						odtLog << L"Retaining Commit not supported, returning TEST_PASS. \n";
						fResults = TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}	
	//Our last insert was just committed, so increment row count
	m_pChgRowset1->m_fTxnPendingInsert = FALSE;
	g_InsertIncrement();	

	//Make sure the Txn Info is correct
	VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, fIsoLevel);
	
	//If we can't retain, its because its not supported, so just pass
	if (m_hr == XACT_E_CANTRETAIN)
	{
		odtLog << L"Retaining Commit not supported, returning TEST_PASS. \n";
		fResults = TRUE;
		goto CLEANUP;
	}

	//Check commit return value
	if (CHECK(m_hr, S_OK))
	
		//Verify the correct behavior, based on CommitPreserve property
	
		if (m_pChgRowset1->m_fCommitPreserve)
		{
			if (COMPARE(NewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
				if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
					fResults = TRUE;

		}
		else
		{
			
			if (COMPARE(RowsetZombied(m_pChgRowset1, &pIRowset, &hRow, hAccessor), TRUE))
				if (COMPARE(NewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
					fResults = TRUE;
		}
CLEANUP:
	ReleaseAllRowsetsAndTxns();	

	//Get rid of hRow, accessor and pIRowset if we haven't already
	if ((hRow != DB_NULL_HROW) && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	
	SAFE_RELEASE(pIRowset);

	return fResults;

}

//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::CommitNonRetain(ISOLEVEL fIsoLevel)
{
	BOOL	fResults = FALSE;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	IRowset * pIRowset = NULL;
	HROW 		hRow = DB_NULL_HROW;
	HRESULT		hr;

	//Start Txn
	if (!CHECK(m_pChgRowset1->StartTxn(fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Do something in txn	
	if(!COMPARE(Insert(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//If we are supposed to be in a zombie state after commit, get hRow, accessor 
	//and pIRowset now so we can verify they can be released later
	if (!m_pChgRowset1->m_fCommitPreserve)
	{
		//Get hRow to hang on to of a row already existing
		if(!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, &hRow), TRUE))
		 	goto CLEANUP;
		
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
			(IUnknown **)&pIRowset))
			goto CLEANUP;

		if (!CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_pChgRowset1->m_cReadBindings,
			m_pChgRowset1->m_rgReadBindings, m_pChgRowset1->m_cbReadRowSize, &hAccessor, NULL), S_OK))
			goto CLEANUP;

	}

	//Make sure the Txn Info is correct
	VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, fIsoLevel);	

	//Now commit it
	if (CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK))
	{
		//We could successfully delete the row, return TRUE
		fResults = TRUE;			
	}

	//Our last insert was just committed, so increment row count
	m_pChgRowset1->m_fTxnPendingInsert = FALSE;
	m_pChgRowset1->m_fTxnStarted = FALSE;
	g_InsertIncrement();
	
	//Verify the correct behavior, based on CommitPreserve property	
	if (m_pChgRowset1->m_fCommitPreserve)
	{
		if (COMPARE(NoNewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))		
			if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
		
				fResults = TRUE;		
	}
	else
	{		
		if (COMPARE(RowsetZombied(m_pChgRowset1, &pIRowset, &hRow, hAccessor), TRUE))
		{	
			if (COMPARE(NoNewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))

				fResults = TRUE;
		}
	}


CLEANUP:
	
	ReleaseAllRowsetsAndTxns();	

	//Get rid of hRow, accessor and pIRowset if we haven't already
	if ((hRow != DB_NULL_HROW) && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	
	SAFE_RELEASE(pIRowset);

	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::AbortRetain(ISOLEVEL fIsoLevel)
{

	BOOL		fResults = FALSE;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	IRowset		*pIRowset = NULL;
	HROW 		hRow = DB_NULL_HROW;
	HRESULT		hr;
	//Start Txn
	if (!CHECK(m_pChgRowset1->StartTxn(fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Do something in txn	
	if(!COMPARE(Insert(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//If we are supposed to be in a zombie state after commit, get hRow, accessor 
	//and pIRowset now so we can verify they can be released later
	if (!m_pChgRowset1->m_fAbortPreserve)
	{
		//Get hRow to hang on to of a row already existing
		if(!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, &hRow), TRUE))
		 	goto CLEANUP;	
		
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
			(IUnknown **)&pIRowset))
			goto CLEANUP;

		if (!CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_pChgRowset1->m_cReadBindings,
			m_pChgRowset1->m_rgReadBindings, m_pChgRowset1->m_cbReadRowSize, &hAccessor, NULL), S_OK))
			goto CLEANUP;

	}

	//Now abort it
	m_hr = m_pChgRowset1->m_pITxnLocal->Abort(NULL, TRUE, FALSE);
	//Our last insert was just aborted, so it's no longer pending
	m_pChgRowset1->m_fTxnPendingInsert = FALSE;


	//Make sure the Txn Info is correct
	VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, fIsoLevel);
	
	//If we can't retain, its because its not supported, so just pass
	if (m_hr == XACT_E_CANTRETAIN)
	{
		odtLog << L"Retaining Abort not supported, returning TEST_PASS. \n";
		fResults = TRUE;
		goto CLEANUP;
	}

	//Check Abort return code
	if (CHECK(m_hr, S_OK))
	
		//Verify the correct behavior, based on CommitPreserve property
	
		if (m_pChgRowset1->m_fAbortPreserve)
		{
			if (COMPARE(NewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
				if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
					fResults = TRUE;

		}
		else
		{			
			if (COMPARE(RowsetZombied(m_pChgRowset1, &pIRowset, &hRow, hAccessor), TRUE))
				if (COMPARE(NewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
					fResults = TRUE;
		}
	
CLEANUP:
	
	ReleaseAllRowsetsAndTxns();	

	//Get rid of hRow, accessor and pIRowset if we haven't already
	if ((hRow != DB_NULL_HROW) && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	
	SAFE_RELEASE(pIRowset);

	return fResults;
}


//--------------------------------------------------------------------
// @mfunc Tests retaining semantics for a specific isolation level
//
// @rdesc TRUE or FALSE
//
BOOL TCRetainPreserve::AbortNonRetain(ISOLEVEL fIsoLevel)
{

	BOOL		fResults = FALSE;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	IRowset		*pIRowset = NULL;
	HROW 		hRow = DB_NULL_HROW;
	HRESULT		hr;

	//Start Txn
	if (!CHECK(m_pChgRowset1->StartTxn(fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Do something in txn	
	if(!COMPARE(Insert(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//If we are supposed to be in a zombie state after commit, get hRow, accessor 
	//and pIRowset now so we can verify they can be released later
	if (!m_pChgRowset1->m_fAbortPreserve)
	{
		//Get hRow to hang on to of a row already existing
		if(!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, &hRow), TRUE))
		 	goto CLEANUP;
		
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
			(IUnknown **)&pIRowset))
			goto CLEANUP;

		if (!CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_pChgRowset1->m_cReadBindings,
			m_pChgRowset1->m_rgReadBindings, m_pChgRowset1->m_cbReadRowSize, &hAccessor, NULL), S_OK))
			goto CLEANUP;

	}

	//Make sure the Txn Info is correct
	VerifyTxnInfo(m_pChgRowset1->m_pITxnLocal, fIsoLevel);

	//Now abort it
	if (CHECK(m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), S_OK))
	//Our last insert was just aborted, so it's no longer pending
	m_pChgRowset1->m_fTxnPendingInsert = FALSE;
	m_pChgRowset1->m_fTxnStarted = FALSE;

	//Verify the correct behavior, based on CommitPreserve property

	if (m_pChgRowset1->m_fAbortPreserve)
	{
		if (COMPARE(NoNewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
			if (COMPARE(RowsetFunctional(m_pChgRowset1), TRUE))
				fResults = TRUE;

	}
	else
	{		
		if (COMPARE(RowsetZombied(m_pChgRowset1, &pIRowset, &hRow, hAccessor), TRUE))
			if (COMPARE(NoNewUnitOfWork(m_pChgRowset1, m_pChgRowset2), TRUE))
				fResults = TRUE;
	}

	
CLEANUP:
	
	ReleaseAllRowsetsAndTxns();	

	//Get rid of hRow, accessor and pIRowset if we haven't already
	if ((hRow != DB_NULL_HROW) && pIRowset)
		CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	if ((hAccessor != DB_NULL_HACCESSOR) && m_pChgRowset1->m_pIAccessor)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	
	SAFE_RELEASE(pIRowset);

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
	TBEGIN

	ISOLEVEL		ulSupported			=	0;
	DWORD			i					=	0;
	
	TESTC_(GetIsoLevels(&ulSupported), S_OK);

	for(i=0;	i<g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;

		if ((ulSupported & g_IsolationList[i].ulIsolation) != g_IsolationList[i].ulIsolation)
			odtLog	<<	"\t->\tNot Supported";
		else
			CHECK(CommitRetain(g_IsolationList[i].ulIsolation), true);
		odtLog << "\n";
	}

CLEANUP:
	TRETURN
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
	TBEGIN

	ISOLEVEL	ulSupported					=	0;
	DWORD		i							=	0;

	TESTC_(GetIsoLevels(&ulSupported), S_OK);

	for(i = 0; i < g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;

		if ((ulSupported & g_IsolationList[i].ulIsolation) == g_IsolationList[i].ulIsolation)
			CHECK(CommitNonRetain(g_IsolationList[i].ulIsolation), true);
		else
			odtLog << "\t->\tNot Supported";
		odtLog << "\n";
	}
CLEANUP:
	TRETURN
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
	TBEGIN

	ISOLEVEL	ulSupported					=	0;
	DWORD		i							=	0;

	TESTC_(GetIsoLevels(&ulSupported), S_OK);

	for(i = 0; i < g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;

		if ((ulSupported & g_IsolationList[i].ulIsolation) == g_IsolationList[i].ulIsolation)
			CHECK(AbortRetain(g_IsolationList[i].ulIsolation), true);
		else
			odtLog << "\t->\tNot Supported";
		odtLog << "\n";
	}
CLEANUP:
	TRETURN
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
	TBEGIN

	ISOLEVEL	ulSupported					=	0;
	DWORD		i							=	0;

	TESTC_(GetIsoLevels(&ulSupported), S_OK);

	for(i = 0; i < g_ulTotalIsolations; i++) {
		odtLog << g_IsolationList[i].pszDesc;

		if ((ulSupported & g_IsolationList[i].ulIsolation) == g_IsolationList[i].ulIsolation)
			CHECK(AbortNonRetain(g_IsolationList[i].ulIsolation), true);
		else
			odtLog << "\t->\tNot Supported";
		odtLog << "\n";
	}
CLEANUP:
	TRETURN
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


// {{ TCW_TC_PROTOTYPE(TCITxnReturnVals)
//*-----------------------------------------------------------------------
//|	Test Case:		TCITxnReturnVals - ITransaction Return Values
//|	Created:			04/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCITxnReturnVals::Init()
{
	HRESULT					hr			= NULL;
	ITransactionObject		*pITranObj	= NULL;
	
	m_pITxnOptions	= NULL;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{		
		//Try to get the txn options interface
		hr	= m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&m_pITxnOptions);
			
		if (S_OK==hr)
		{
			return TEST_PASS;		
		}
		else
		{
			if (DB_E_NOTSUPPORTED==hr)
			{
				//test for IID_ITransactionObject OI here
				if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionObject,SESSION_INTERFACE, (IUnknown **)&pITranObj))
				{
					SAFE_RELEASE(pITranObj);
					return TEST_SKIPPED;
				}
				//free memory that was incorrectly allocated
				SAFE_RELEASE(pITranObj);
			}
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with pboidReason = NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_1()
{
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
		fResults = CHECK(m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), S_OK);
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with pboidReason = BOID_NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_2()
{
	BOOL	fResults = FALSE;
	BOID	boidReason = BOID_NULL;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
		fResults = CHECK(m_pChgRowset1->m_pITxnLocal->Abort(&boidReason, FALSE, FALSE), S_OK);
	
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
// @mfunc Abort with fAsync=TRUE - XACT_E_NOTSUPPORTED or S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_3()
{
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//Check for proper return code for given parameters
		m_hr = m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, TRUE);
					
		switch (m_pChgRowset1->m_eAsyncAbort)
		{
			case EASYNCTRUE:			
				fResults = CHECK(m_hr, S_OK);		
				break;
			case  EASYNCNOTSUPPORTED:				
				odtLog << L"DBPROP_ASYNCTXNABORT is returning NOT SUPPORTED.\n";				
				//Don't break here as we want to fall through
				//to also check that we get XACT_E_NOSUPPORTED
			case EASYNCFALSE:		
				//Async is not supported
				fResults = CHECK(m_hr, XACT_E_NOTSUPPORTED);
				break;
			default:
				//We have to add code for another ENUM Value
				ASSERT(FALSE);
		}
	}
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort with fAsync=FALSE - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_4()
{
	BOOL	fResults = FALSE;
	BOID	boidReason = BOID_NULL;	//Just so we have some value

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
		fResults = CHECK(m_pChgRowset1->m_pITxnLocal->Abort(&boidReason, FALSE, FALSE), S_OK);
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Call commit on one thread, Abort on another - XACT_E_ALREADYINPROGRESS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_5()
{	
	HANDLE hThread;
	ULONG  ulThreadID;
	BOOL	fResults = FALSE;
	HRESULT	hr; 
	
	//Start our transaction
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{	
		
		//Create a thread to do the abort
		hThread = CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)&ThreadAbort,
						m_pChgRowset1,
						CREATE_SUSPENDED,
						&ulThreadID);		
		
		if (hThread == NULL)
			goto CLEANUP;				

		//Start the abort thread.  Inside ThreadAbort we immediately
		//give the time slice back, which hopefully means commit will occur
		//before the abort in ThreadAbort
		ResumeThread(hThread);

		//Do the commit, this should occur first
		m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0);
		
		//Wait for abort to finish, and get its return value
		WaitForSingleObject(hThread, INFINITE);			
		if (!GetExitCodeThread(hThread, (DWORD *)&hr))
			goto CLEANUP;
		//Cleanup second thread
		if (hThread)
			CloseHandle(hThread);  

		//Our commit should have succeeded
		CHECK(m_hr, S_OK);

		//Either we got the abort done in time, in which case we expect
		//XACT_E_ALREADYINPROGRESS; or the commit had finished before 
		//we could do the abort, in which case we expect XACT_E_NOTRANSACTION		
		if ((hr == XACT_E_ALREADYINPROGRESS) || (hr == XACT_E_NOTRANSACTION))		
			fResults = TRUE;					
		else
			//Make sure we log a message that our hr didn't match anything we expected
			CHECK(hr, XACT_E_ALREADYINPROGRESS);
	}	 
		

CLEANUP:
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Call abort  on one thread and then on another - XACT_S_ABORTING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_6()
{
	HANDLE hThread;
	ULONG  ulThreadID;
	BOOL	fResults = FALSE;
	HRESULT	hr; 
	
	//Start our transaction
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{			
		//Create a thread to do the second abort
		hThread = CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)&ThreadAbort,
						m_pChgRowset1,
						CREATE_SUSPENDED,
						&ulThreadID);		
		
		if (hThread == NULL)
			goto CLEANUP;				

		//Start the second thread.  Inside ThreadAbort we immediately
		//give the time slice back, which hopefully means first Abort will occur
		//before the abort in ThreadAbort
		ResumeThread(hThread);

		//Do the abort, this should occur first
		m_hr = m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE);
		
		//Wait for abort to finish, and get its return value
		WaitForSingleObject(hThread, INFINITE);			
		if (!GetExitCodeThread(hThread, (DWORD *)&hr))
			goto CLEANUP;
		//Cleanup second thread
		if (hThread)
			CloseHandle(hThread);  

		//Our first abort should have succeeded
		CHECK(m_hr, S_OK);

		//Either we got the second abort done in time, in which case we expect
		//XACT_S_ABORTING; or the first abort had finished before we could
		//do the second abort, in which case we expect XACT_E_NOTRANSACTION		
		if ((hr == XACT_S_ABORTING) || (hr == XACT_E_NOTRANSACTION))		
			fResults = TRUE;					
		else
			//Make sure we log a message that our hr didn't match anything we expected
			CHECK(hr, XACT_S_ABORTING);
	}	 
		

CLEANUP:
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Commit with grfTC =XACTC_ASYNC - XACT_E_NOTSUPPORTED or S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_7()
{	
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//Check for proper return code for given parameters
		m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_ASYNC_PHASEONE, 0);

		//Make sure the right thing was returned per our property
		switch (m_pChgRowset1->m_eAsyncCommit)
		{
			case EASYNCTRUE:			
				fResults = CHECK(m_hr, S_OK);		
				break;

			case  EASYNCNOTSUPPORTED:
				odtLog << L"DBPROP_ASYNCTXNCOMMIT is returning NOT SUPPORTED.\n";
				//Don't break here as we want to fall through
				//and check that the right return code was generated
			case EASYNCFALSE:		
				//Async is not supported
				fResults = CHECK(m_hr, XACT_E_NOTSUPPORTED);
				break;
			default:
				//We have to add code for another ENUM Value
				ASSERT(FALSE);
		}

	}
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
	{
		//Log a message telling what the return code was
		CHECK(m_hr, S_OK);
		return TEST_FAIL;
	}

}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Commit with grfTC =XACTC_SYNC_PHASEONE -  XACT_E_NOTSUPPORTED or S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_8()
{
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//Check for proper return code for given parameters		
		m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASEONE, 0);
		fResults = (m_hr == S_OK) || (m_hr == XACT_E_NOTSUPPORTED);
	}
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
	{
		//Log a message telling what the return code was
		CHECK(m_hr, S_OK);
		return TEST_FAIL;
	}

}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Commit with grfRM!=0 -  XACT_E_NOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_9()
{
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
		fResults = CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 1),  
			XACT_E_NOTSUPPORTED);
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Call Abort on one thread, Commit on another - XACT_E_ALREADYINPROGRESS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_10()
{
	HANDLE hThread;
	ULONG  ulThreadID;
	BOOL	fResults = FALSE;
	HRESULT	hr; 
	
	//Start our transaction
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{	
		
		//Create a thread to do the commit 
		hThread = CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)&ThreadCommit,
						m_pChgRowset1,
						CREATE_SUSPENDED,
						&ulThreadID);		
		
		if (hThread == NULL)
			goto CLEANUP;				

		//Start the commit thread.  Inside ThreadCommit we immediately
		//give the time slice back, which hopefully means Abort will occur
		//before the commit in ThreadCommit
		ResumeThread(hThread);

		//Do the abort, this should occur first
		m_hr = m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE);
		
		//Wait for commit to finish, and get its return value
		WaitForSingleObject(hThread, INFINITE);			
		if (!GetExitCodeThread(hThread, (DWORD *)&hr))
			goto CLEANUP;
		//Cleanup second thread
		if (hThread)
			CloseHandle(hThread);  

		//Our abort should have succeeded
		CHECK(m_hr, S_OK);

		//Either we got the commit done in time, in which case we expect
		//XACT_E_ALREADYINPROGRESS; or the abort had finished before 
		//we could do the commit, in which case we expect XACT_E_NOTRANSACTION		
		if ((hr == XACT_E_ALREADYINPROGRESS) || (hr == XACT_E_NOTRANSACTION))		
			fResults = TRUE;					
		else
			//Make sure we log a message that our hr didn't match anything we expected
			CHECK(hr, XACT_E_ALREADYINPROGRESS);
	}	 
		

CLEANUP:
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Call commit on one thread, and then on another - XACT_E_ALREADYINPROGRESS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_11()
{
	HANDLE hThread;
	ULONG  ulThreadID;
	BOOL	fResults = FALSE;
	HRESULT	hr; 
	
	//Start our transaction
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{	
		
		//Create a thread to do the second commit
		hThread = CreateThread(NULL,
						0,
						(LPTHREAD_START_ROUTINE)&ThreadCommit,
						m_pChgRowset1,
						CREATE_SUSPENDED,
						&ulThreadID);		
		
		if (hThread == NULL)
			goto CLEANUP;				

		//Start the second thread.  Inside ThreadCommit we immediately
		//give the time slice back, which hopefully means our first commit will occur
		//before the commit in ThreadCommit
		ResumeThread(hThread);

		//Do the commit, this should occur first
		m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0 , 0);
		
		//Wait for commit to finish, and get its return value
		WaitForSingleObject(hThread, INFINITE);			
		if (!GetExitCodeThread(hThread, (DWORD *)&hr))
			goto CLEANUP;
		//Cleanup second thread
		if (hThread)
			CloseHandle(hThread);  

		//Our first commit should have succeeded
		CHECK(m_hr, S_OK);

		//Either we got the second commit done in time, in which case we expect
		//XACT_E_ALREADYINPROGRESS; or the first commit had finished before we could
		//do the second commit, in which case we expect XACT_E_NOTRANSACTION		
		if ((hr == XACT_E_ALREADYINPROGRESS) || (hr == XACT_E_NOTRANSACTION))		
			fResults = TRUE;					
		else
			//Make sure we log a message that our hr didn't match anything we expected
			CHECK(hr, XACT_E_ALREADYINPROGRESS);
	}	 
		

CLEANUP:
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Commit with reported supported grfTC values - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_12()
{
	BOOL	fResults = FALSE;
	XACTTRANSINFO	TxnInfo;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//Check which grfTC values are supported
		if (CHECK(m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(&TxnInfo), S_OK))
		{
			//If both values are supported, try ASYNC first (since they are mutually
			//exclusive when called with Commit, we can only do one at a time)
			if ((TxnInfo.grfTCSupported & XACTTC_ASYNC_PHASEONE) &&
				(TxnInfo.grfTCSupported & XACTTC_SYNC_PHASEONE))
			{
				   CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_ASYNC_PHASEONE, 0), S_OK);
				   //Now remove this option from our flags so we don't try to do it again
					TxnInfo.grfTCSupported &= ~(XACTTC_ASYNC_PHASEONE);
					//Reset another transaction
					CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK);
			}
			//Any remaining flags should also work on commit
			fResults = CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 
				TxnInfo.grfTCSupported, 0), S_OK);
		}
	}
	
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc GetTransactionInfo with pinfo = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_13()
{

	BOOL	fResults = FALSE;

	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for invalid arg on pinfo = NULL
		if (CHECK(m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(NULL), E_INVALIDARG))
			fResults = TRUE;

	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc GetOptionsObject with ppOptions = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_14()
{
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(NULL), E_INVALIDARG))
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction twice in a row - S_OK or XACT_E_XTIONEXISTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_15()
{
	ULONG	ulTxnLvl1 = 0;
	ULONG	ulTxnLvl2 = 0;

	if (CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		0, NULL, &ulTxnLvl1), S_OK))
	{
		//This will succeed if nested txns are supported, else return
		//XACT_E_XTIONEXISTS
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
			0, NULL, &ulTxnLvl2);
		
		//First level must always be 1
		COMPARE(ulTxnLvl1, 1);
		
		if (m_hr == S_OK)
		{
			//Make sure the nesting worked
			COMPARE(ulTxnLvl2, 2);	
		}
		else
		{
			//Make sure we got the right return value
			CHECK(m_hr, XACT_E_XTIONEXISTS);
		}
		//End the txn -- this should end both levels, if applicable
		CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
			
		return TEST_PASS;
	}

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction with a non zero timeout - S_OK or XACT_E_NOTIMEOUT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_16()
{
	XACTOPT	TxnOptions;
	BOOL	fResults = FALSE;
	
	//Set time out to non zero
	TxnOptions.ulTimeout = MAX_ULONG;
	TxnOptions.szDescription[0] = '\0';


	//Set our time out option
	if (CHECK(m_pITxnOptions->SetOptions(&TxnOptions), S_OK))
	{
			
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
			0, m_pITxnOptions, NULL);

		ReleaseAllRowsetsAndTxns();

		//Either of these is acceptable
		if (m_hr == S_OK || m_hr == XACT_E_NOTIMEOUT)
			return TEST_PASS;
	}	
	
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction with zero timeout - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_17()
{
	XACTOPT	TxnOptions;
	BOOL	fResults = FALSE;
	
	//Set time out to non zero
	TxnOptions.ulTimeout = 0;
	TxnOptions.szDescription[0] = '\0';

	//Set our time out option
	if (CHECK(m_pITxnOptions->SetOptions(&TxnOptions), S_OK))
	{			
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
			0, m_pITxnOptions, NULL);

		ReleaseAllRowsetsAndTxns();
		
		//Time out of zero should always work
		if (CHECK(m_hr, S_OK))
			return TEST_PASS;
	}	
	
	return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction with pulTransactionLevel = NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_18()
{	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		0, NULL, NULL), S_OK))
	{
		ReleaseAllRowsetsAndTxns();
		return TEST_PASS;
	}
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction with isoFlags != 0 - XACT_E_NOISORETAIN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_19()
{	
	//Make isoFlags 1, nothing but 0 is valid
	if (CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		1, NULL,NULL), XACT_E_NOISORETAIN))		
		return TEST_PASS;	
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc StartTransaction with isoLevel = invalid - XACT_E_ISOLATIONLEVEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_20()
{
	//Make isoLevel invalid -- since these values are mutually exclusive,
	//or'ing them together will create an invalid value
	if (m_hr = CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(
		ISOLATIONLEVEL_CHAOS | ISOLATIONLEVEL_READUNCOMMITTED | 
		ISOLATIONLEVEL_READCOMMITTED | ISOLATIONLEVEL_SERIALIZABLE,
		0, NULL, NULL), XACT_E_ISOLATIONLEVEL))		
		return TEST_PASS;	
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc GetOptions with pOptions = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_21()
{
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//Make sure we can also get the options
		if (CHECK(pITxnOptions->GetOptions(NULL), E_INVALIDARG))
			fResults = TRUE;
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc GetOptions with pOptions allocated on stack - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_22()
{
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	XACTOPT	TxnOptions;
	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//Make sure we can also get the options with
		//TxnOptions allocated on stack (so we know
		//provider isn't trying to free the memory)
		if (CHECK(pITxnOptions->GetOptions(&TxnOptions), S_OK))
			fResults = TRUE;
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc SetOptions with max szDescription
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_23()
{
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	XACTOPT	TxnOptions;
	CHAR	szDesc[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";	//39 chars, plus null terminator 

	//Make sure we are using exactly the whole portion of the struct
	ASSERT(sizeof(szDesc) == MAX_TRAN_DESC);

	strcpy((CHAR *)TxnOptions.szDescription, szDesc);
	TxnOptions.ulTimeout = 0;
	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		if (CHECK(pITxnOptions->SetOptions(&TxnOptions), S_OK))
			if (CHECK(pITxnOptions->GetOptions(&TxnOptions), S_OK))
				//Make sure we didn't loose any characters or forget null terminator
				if (COMPARE(strcmp((CHAR *)TxnOptions.szDescription, szDesc), 0))
					fResults = TRUE;
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc SetOptions with ppOptions = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_24()
{
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//Expect invalid arg for NULL
		if (CHECK(pITxnOptions->SetOptions(NULL), E_INVALIDARG))
			fResults = TRUE;
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Commit with grfTC =XACTC_SYNC_PHASETWO -   S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITxnReturnVals::Variation_25()
{
	// TO DO:  Add your own code here
	
	BOOL	fResults = FALSE;

	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		//Check for proper return code for given parameters		
		//XACTTC_SYNC_PHASETWO should be supported by ODBC Provider
		if(CHECK(m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK))
			fResults = TRUE;
	}
	
	//Cleanup
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
BOOL TCITxnReturnVals::Terminate()
{
	// Release the Objects
	SAFE_RELEASE(m_pITxnOptions)

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
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Change, Abort with fRetaining = TRUE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_1()
{
	BOOL	fResults					= FALSE;
	HRESULT	hr;
	
	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	
	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	//Abort the change
	if (!CHECK(m_pChgUpdRowset1->Abort(TRUE, TRUE), S_OK))
		goto CLEANUP;	

	//Verify pending row (or lack thereof) after Update() 
	if (!m_pChgUpdRowset1->CheckPendingRow(EABORT, 1))
		goto CLEANUP;

	
	//Use second rowset to verify we can't see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;

	//delete second rowset in case it will be used again in this variation
	if (!CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK))
		goto CLEANUP;
		
	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

	//Commit the update
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, TRUE), S_OK))
		goto CLEANUP;


	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	if (m_pChgUpdRowset1->m_fAbortPreserve)
	{
		//We should now be able to see the change since we updated
		//and our change from before the abort should have been
		//preserved
		if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	}
	else
	{
		//We shouldn't see a change because the rowset
		//should have zomibied on abort, losing the pending change
		if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
			goto CLEANUP;	
	}
	

	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Change, Abort with fRetaining = FALSE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_2()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
	
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	//Abort the change, with fRetaining = FALSE, we should then be in autocommit
	if (!CHECK(m_pChgUpdRowset1->Abort(TRUE, FALSE), S_OK))
		goto CLEANUP;	

	//Verify pending row (or lack thereof) after abort
	if (!m_pChgUpdRowset1->CheckPendingRow(EABORT, 1))
		goto CLEANUP;
	
	//Use second rowset to verify we can't see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;

	//delete second rowset in case it will be used again in this variation
	if (!CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK))
		goto CLEANUP;
		
	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;


	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	if (m_pChgUpdRowset1->m_fAbortPreserve)
	{
		//If the rowset was preserved, our change should have occured
		if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	}
	else
	{
		//We still shouldn't be able to see the change since we aborted
		//and our rowset wasn't preserved
		if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
			goto CLEANUP;
	}

	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Abort with fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_3()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;
		
	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

	//Abort the change, with fRetaining = TRUE
	if (!CHECK(m_pChgUpdRowset1->Abort(TRUE, TRUE), S_OK))
		goto CLEANUP;	

	//Verify there are no pending rows after the abort since 
	//we've already called update
	if (!m_pChgUpdRowset1->CheckPendingRow(EABORT, 0))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Use second rowset to verify we can't see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;

	
	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_4()
{
	BOOL	fResults = FALSE;		
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
		
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;	

	//Abort the change, with fRetaining = FALSE
	if (!CHECK(m_pChgUpdRowset1->Abort(TRUE, FALSE), S_OK))
		goto CLEANUP;	

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Verify no pending row (or lack thereof) after the abort
	//since we already called update
	if (!m_pChgUpdRowset1->CheckPendingRow(EABORT, 0))
		goto CLEANUP;

	//Use second rowset to verify we can't see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;
	
	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Change, Commit with fRetaining = TRUE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_5()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	//Commit 
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, TRUE), S_OK))
		goto CLEANUP;	

	//Verify a possible pending row after commit 
	if (!m_pChgUpdRowset1->CheckPendingRow(ECOMMIT, 1))
		goto CLEANUP;

	//Use second rowset to verify we can't see 
	//the change since we haven't updated yet
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;

	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;
		
	//Commit our update	 
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, TRUE), S_OK))
		goto CLEANUP;	

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	if (m_pChgUpdRowset1->m_fCommitPreserve)
	{
		//We should now be able to see the change since we updated
		//and our change from before the commit should have been
		//preserved
		if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	}
	else
	{
		//We shouldn't see a change because the rowset
		//should have zomibied on commit, losing the pending change
		if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
			goto CLEANUP;	
	}

	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Change, Commit with fRetaining = FALSE, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_6()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;
	
	//Commit with fRetaining = FALSE, we should then be in autocommit 
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, FALSE), S_OK))
		goto CLEANUP;	

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Verify pending row after commit
	if (!m_pChgUpdRowset1->CheckPendingRow(ECOMMIT, 1))
		goto CLEANUP;

	//Use second rowset to verify we can't see 
	//the change since we haven't updated yet
	if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
		goto CLEANUP;

	//delete second rowset in case it will be used again in this variation
	if (!CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK))
		goto CLEANUP;
		
	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param
	
	if (m_pChgUpdRowset1->m_fCommitPreserve)
	{
		//We should now be able to see the change since we updated
		//and our change from before the commit should have been
		//preserved
		if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	}
	else
	{
		//We shouldn't see a change because the rowset
		//should have zomibied on commit, losing the pending change
		if (!COMPARE(FindChange(m_pChgUpdRowset2), FALSE))
			goto CLEANUP;	
	}
	
	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Commit with fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_7()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;

	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
		
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;


	//Commit the change, with fRetaining = TRUE 
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, TRUE), S_OK))
		goto CLEANUP;	

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Verify no pending row 
	if (!m_pChgUpdRowset1->CheckPendingRow(ECOMMIT, 0))
		goto CLEANUP;
	
	//Use second rowset to verify we can see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	
	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Change, Update, Commit with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_8()
{
	BOOL	fResults = FALSE;
	HRESULT	hr;
	
	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	hr=m_pChgUpdRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
		
	//Make a change
	if (!COMPARE(Change(m_pChgUpdRowset1), TRUE))
		goto CLEANUP;

	//Call update for all pending changes
	if (!CHECK(m_pChgUpdRowset1->m_pIRowsetUpdate->Update(NULL, 0, 
		NULL, NULL, NULL, NULL), S_OK))
		goto CLEANUP;
	

	//Commit the change, with fRetaining = TRUE
	if (!CHECK(m_pChgUpdRowset1->Commit(TRUE, FALSE), S_OK))
		goto CLEANUP;	

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Verify no pending row 
	if (!m_pChgUpdRowset1->CheckPendingRow(ECOMMIT, 0))
		goto CLEANUP;

	//Use second rowset to verify we can see the change 
	if (!COMPARE(FindChange(m_pChgUpdRowset2), TRUE))
		{
			goto CLEANUP;
		}
		else
		{
			//if this commits then the Update of the pending rows looks like a delete then 
			//insert as far as the back end data is concerened
			g_ulLastActualInsert++;
			g_ulLastActualDelete++;
		}
	
	//Everything went OK 
	fResults = TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);
	CHECK(m_pChgUpdRowset2->FreeRowset(), S_OK);

	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Open read only rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowsetUpdate::Variation_9()
{
	BOOL	fResults	= FALSE;
	HRESULT	hr;
	
	
	//if provider does not support IRowsetChange
	//if provider does not support IRowsetUpdate
	if (!m_pChgUpdRowset1->m_fChange	||	!m_pChgUpdRowset1->m_fUpdate)
	{
	 	odtLog << L"IRowsetChange or IRowsetUpdate not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}

	//Start Txn
	if (!CHECK(m_pChgUpdRowset1->StartTxn(ISOLATIONLEVEL_READUNCOMMITTED), S_OK))
	{
		goto CLEANUP;
	}

	hr=m_pChgUpdRowset1->MakeRowsetReadOnly();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}	

	fResults	= TRUE;

CLEANUP:
	CHECK(m_pChgUpdRowset1->FreeRowset(), S_OK);

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
	m_pITxnLocal = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CSessionObject::Init())
	// }}
	{
		//We won't do a SetDataSource object here, just for variety --
		//we'll create a new DSO
		if (CHECK(CreateDBSession(), S_OK))
			//Txns must be supported if we passed ModuleInit
			if(CHECK(GetSessionObject(IID_ITransactionLocal, (IUnknown**)&m_pITxnLocal), S_OK))
					return TRUE;

	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_1()
{
	//Try to abort when we've never called StartTransaction
	if (CHECK(m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining = FALSE before StartTransaction - XACT_E_NOTRANSACTION 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_2()
{
	//Try to commmit when we've never called StartTransaction
	if (CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetTransactionInfo before StartTransaction - XACT_E_NOTRANSACTION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_3()
{
	XACTTRANSINFO	TxnInfo;

	//Try to GetTransactionInfo when we've never called StartTransaction
	if (CHECK(m_pITxnLocal->GetTransactionInfo(&TxnInfo), XACT_E_NOTRANSACTION))
		return TEST_PASS;
	else
		return TEST_FAIL;
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
	BOOL					fResults		= TEST_FAIL;
	XACTOPT					TxnOptions;
	ITransactionOptions		*pITxnOptions	= NULL;
	HRESULT					hr;

	//Try to get the txn options interface
	hr	= m_pITxnLocal->GetOptionsObject(&pITxnOptions);
		
	if (S_OK==hr)
	{
		//Make sure we can also get the options
		if (CHECK(pITxnOptions->GetOptions(&TxnOptions), S_OK))
		{
			fResults = TEST_PASS;
		}
			
		SAFE_RELEASE(pITxnOptions);
	}	
	else
	{
		if (DB_E_NOTSUPPORTED==hr)
		{
			//test for IID_ITransactionObject OI here
			if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionObject,SESSION_INTERFACE, (IUnknown **)&pITxnOptions))
			{
				SAFE_RELEASE(pITxnOptions);
				return TEST_SKIPPED;
			}
			//free memory that was incorrectly allocated
			SAFE_RELEASE(pITxnOptions);
		}
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_5()
{
	//Try to abort when we've never called StartTransaction
	if (CHECK(m_pITxnLocal->Abort(NULL, TRUE, FALSE), XACT_E_NOTRANSACTION))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining = TRUE before StartTransaction - XACT_E_NOTRANSACTION 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CNoTxn::Variation_6()
{
	//Try to commmit when we've never called StartTransaction
	if (CHECK(m_pITxnLocal->Commit(TRUE, 0, 0), XACT_E_NOTRANSACTION))
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
BOOL CNoTxn::Terminate()
{
	SAFE_RELEASE(m_pITxnLocal);

	ReleaseDBSession();
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSessionObject::Terminate());
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
	m_pTable = NULL;
	m_ulCurrentDelete = 1;
	m_ulCurrentInsert = NUM_ROWS+1;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{
		CreateDBSession();
		//Create a second table				
		m_pTable = new CTable( 			
			m_pIOpenRowset, 
			(LPWSTR)gwszModuleName);
			
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
	return FALSE;
}

		 
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort two transactions on same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_1()
{
	BOOL		fResults = FALSE;
	HRESULT		hr;
	CLSID		clsidMSDASQL	=	CLSID_MSDASQL;
	//Start Txns
	if (!CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pChgRowset2->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;
	
	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}


	// Hack: Sleep for 5 seconds, so changes are visible
	// MSDASQL/JET needs this time to make changes visible
	if (GetModInfo()->GetThisTestModule()->m_ProviderClsid==clsidMSDASQL)
		::Sleep(5000);

	hr=m_pChgRowset2->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	/////////////////////////////////////////////////////////////////////
	//Do something in each txn, abort it, and verify change was aborted	
	/////////////////////////////////////////////////////////////////////
	
	//Rowset 1

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	
	//Make a change
	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;
	//Now abort
	if (!CHECK(m_hr = m_pChgRowset1->Abort(TRUE, FALSE), S_OK))
		goto CLEANUP;
	
	//Shouldn't be able to see the change
	if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
		goto CLEANUP;


	//Rowset 2, have to figure the count manually since we 
	//aren't on the same table as m_pChgRowset1 anymore.
	
	//Make a change
	if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
		goto CLEANUP;
	//Now abort, preserving rowset
	if (!CHECK(m_hr = m_pChgRowset2->Abort(TRUE, FALSE), S_OK))
		goto CLEANUP;

	//Shouldn't be able to see the change
	if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), FALSE))
		goto CLEANUP;

	fResults = TRUE;
	
CLEANUP:
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit two transactions on same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_2()
{
	BOOL		fResults = FALSE;
	HRESULT		hr;

	//Start Txns
	if (!CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;
	if (!CHECK(m_pChgRowset2->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;
	
	
	//Make rowsets
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
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	
	//Make a change
	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;
	//Now commit
	if (!CHECK(m_hr = m_pChgRowset1->Commit(TRUE, FALSE), S_OK))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Should be able to see the change
	if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Rowset 2, have to figure the count manually since we 
	//aren't on the same table as m_pChgRowset1 anymore.
	
	//Make a change
	if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
		goto CLEANUP;
	
	//Now commit, preserving rowset
	if (!CHECK(m_hr = m_pChgRowset2->Commit(TRUE, FALSE), S_OK))
		goto CLEANUP;
	
	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param
		
	//Should be able to see the change
	if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), TRUE))
	{
		//Increment our count since we already committed the delete/insert
		//but won't hit the increment before CLEANUP
		m_ulCurrentInsert++;
		m_ulCurrentDelete++;
		goto CLEANUP;
	}

	//Increment our count since we committed the delete/insert
	m_ulCurrentInsert++;
	m_ulCurrentDelete++;

	fResults = TRUE;
	
CLEANUP:
	
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort one transaction, commit the other, on same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_3()
{
	BOOL		fResults = FALSE;
	HRESULT		hr;

	//Start Txns
	if (!CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;
	if (!CHECK(m_pChgRowset2->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;
		
	//Make rowsets
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
	//Do something in each txn, commit one, abort the other and verify
	/////////////////////////////////////////////////////////////////////
	
	//Rowset 1

	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	
	//Make a change
	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;
	//Now commit
	if (!CHECK(m_hr = m_pChgRowset1->Commit(TRUE, FALSE), S_OK))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Should be able to see the change
	if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Rowset 2, have to figure the count manually since we 
	//aren't on the same table as m_pChgRowset1 anymore.
	
	//Make a change
	if (!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
		goto CLEANUP;
	//Now abort, preserving rowset
	if (!CHECK(m_hr = m_pChgRowset2->Abort(TRUE, FALSE), S_OK))
		goto CLEANUP;

	//Sleep for a few seconds; this is to ensure that the back end has had
	//time to make this update visible to other transactions which
	//should see it.  This is only necessary for Access, which only does
	//this every few seconds.
	if (m_fOnAccess)
		Sleep(SLEEP_TIME);	//Takes milliseconds as param

	//Shouldn't be able to see the change
	if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), FALSE))
		goto CLEANUP;

	fResults = TRUE;
	
CLEANUP:	
	ReleaseAllRowsetsAndTxns();	

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort and Commit with multiple commands/rowsets active
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CMultipleTxns::Variation_4()
{
	BOOL			fResults = FALSE;
	IUnknown*		pIUnknown = NULL;
	HRESULT			hr;

	//Make first rowset
	hr=m_pChgRowset1->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
	
	//Get Session from first object
	if (!CHECK(m_pChgRowset1->GetSessionObject(IID_IUnknown, (IUnknown**)&pIUnknown), S_OK))
		goto CLEANUP;

	///////////////////////////////////////////////////////
	//Create a second command/rowset on the same DB Session
	///////////////////////////////////////////////////////

	//First we must release original session our second object was set with
	m_pChgRowset2->ReleaseRowsetObject();
	m_pChgRowset2->ReleaseCommandObject();
	m_pChgRowset2->ReleaseDBSession();
	
	SAFE_RELEASE(m_pChgRowset2->m_pITxnLocal);

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
	m_hr = m_pChgRowset1->StartTxn(m_fIsoLevel);

	//Some providers such as ODBC Provider to SQL Server will not allow this
	//because they can't have mutliple commands per transaction 
	if (m_hr == XACT_E_CONNECTION_DENIED)
	{
		odtLog << L"This provider does not support multiple commands in the same transaction, this is expected behavior for ODBC Provider to SQL Server. \n";		
		fResults = TRUE;
		goto CLEANUP;
	}
	else
		//Providers such as ODBC Provider to Access can't support 
		//starting a transaction with a rowset open
		if (m_hr == E_FAIL)
		{
			//So close rowsets before creating txn
			m_pChgRowset1->ReleaseRowsetObject();
			m_pChgRowset2->ReleaseRowsetObject();

			m_hr = m_pChgRowset1->StartTxn(m_fIsoLevel);

			//Now recreate the rowsets
			m_pChgRowset1->MakeRowset();
			m_pChgRowset2->MakeRowset();

			//StartTxn should always work since rowsets were closed
			if (!CHECK(m_hr, S_OK))
				goto CLEANUP;

		}
		else	
			if (!CHECK(m_hr, S_OK))
				goto CLEANUP;
	
	///////////////////////////////////////////////////////////////
	//Make changes to both rowsets and verify they both get aborted
	///////////////////////////////////////////////////////////////
	
	//if provider does not support IRowsetChange
	if (!m_pChgRowset1->m_fChange)
	{
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;
	
	//Record all the txn related things that are now true about
	//rowset 2 because it shares a session with rowset 1
	m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);

	if(!COMPARE(Change(m_pChgRowset2), TRUE))
		goto CLEANUP;
	
	//Now abort the whole session
	if (!CHECK(m_hr = m_pChgRowset1->Abort(FALSE, FALSE), S_OK))
		goto CLEANUP;

	//Cleanup rowsets and start with new ones, in case the first ones
	//were zombied.  Do this before StartTxn in case we don't support
	//StartTransaction after rowsets are open
	m_pChgRowset1->ReleaseRowsetObject();
	m_pChgRowset2->ReleaseRowsetObject();

	//Start a new Txn since we aborted the last one non retaining
	CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK);

	//Record all the txn related things that are now true about
	//rowset 2 because it shares a session with rowset 1
	m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);

	
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
		

	//Shouldn't be able to see the changes
	if (!COMPARE(FindChange(m_pChgRowset1), FALSE))
		goto CLEANUP;	
	if (!COMPARE(FindChange(m_pChgRowset2), FALSE))
		goto CLEANUP;

	/////////////////////////////////////////////////////////////////
	//Make changes to both rowsets and verify they both get committed
	/////////////////////////////////////////////////////////////////

	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;
	
	//Record all the txn related things that are now true about
	//rowset 2 because it shares a session with rowset 1
	m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);
	
	if(!COMPARE(Change(m_pChgRowset2), TRUE))
		goto CLEANUP;
	
	//Now commit the whole session
	if (!CHECK(m_hr = m_pChgRowset1->Commit(FALSE, FALSE), S_OK))
		goto CLEANUP;

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
	//g_ulLastActualInsert++;
	g_DeleteAndInsertIncrement();

	//Record all the txn related things that are now true about
	//rowset 2 because it shares a session with rowset 1
	m_pChgRowset2->CopyTxnInfo(m_pChgRowset1);

	//Cleanup rowsets and start with new ones, in case the first ones
	//were zombied
	m_pChgRowset1->ReleaseRowsetObject();
	m_pChgRowset2->ReleaseRowsetObject();
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
	
	//Should be able to see the changes
	if (!COMPARE(FindChange(m_pChgRowset1), TRUE))
		goto CLEANUP;	
	if (!COMPARE(FindChange(m_pChgRowset2), TRUE))
		goto CLEANUP;
	
	fResults = TRUE;

CLEANUP:
	
	SAFE_RELEASE(pIUnknown);

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
// @mfunc Multiple Sessions, Transaction on only one
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
		
	//Start Txn on our second DB Session
	if (!CHECK(m_pChgRowset2->StartTxn(m_fIsoLevel), S_OK))
		goto CLEANUP;	
	
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
	 	odtLog << L"IRowsetChange not supported, Variation skipped" << wszNewLine;
		fResults = TRUE;
		goto CLEANUP;
	}
	
	if(!COMPARE(Change(m_pChgRowset1), TRUE))
		goto CLEANUP;

	//Release rowset so that weave any locks 
	//on the table for the next execute don't h
	//m_pChgRowset1->Terminate();
	
	if(!COMPARE(m_pChgRowset2->Change(m_ulCurrentDelete, m_ulCurrentInsert), TRUE))
		goto CLEANUP;
	
	//Now abort the second session
	if (!CHECK(m_hr = m_pChgRowset2->Abort(TRUE, TRUE), S_OK))
		goto CLEANUP;

	//Shouldn't be able to see the changes here
	if (!COMPARE(m_pChgRowset2->FindRow(m_ulCurrentInsert, NULL), FALSE))
		goto CLEANUP;	
	
	//Get us back to a good rowset
	//if (!COMPARE(m_pChgRowset1->Init(), TRUE))
	//	goto CLEANUP;

	//But should be able to see them on non transacted rowset
	//Note that we don't want to use FindChange() here because
	//that looks for the last changed row, which would have
	//been incremented for the second change we did, and
	//we want the first change we did (the one that actually 
	//wasn't aborted).
	if (!COMPARE(m_pChgRowset1->FindRow(g_ulLastActualInsert, NULL), TRUE))
		goto CLEANUP;

	fResults = TRUE;

CLEANUP:
	
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
BOOL CMultipleTxns::Terminate()
{
	
	ReleaseDBSession();

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
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CSequence::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxn::Init())
	// }}
	{
		if (CHECK(CreateDBSession(), S_OK))
			if (CHECK(GetSessionObject(IID_ITransactionLocal, (IUnknown**)&m_pITxnLocal), S_OK))					
				return TRUE;
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
	BOOL	fResults = FALSE;

	if (CHECK(m_pITxnLocal->GetOptionsObject(&m_pITxnOptions), S_OK))
	{
		if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
		{			
			COMPARE(m_TxnOptions.ulTimeout, 0);
			COMPARE(m_TxnOptions.szDescription[0], '\0');

			//Now try setting them to something new
			m_TxnOptions.ulTimeout = TIMEOUT;
			strcpy((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION);

			//Set should also work
			if (CHECK(m_pITxnOptions->SetOptions(&m_TxnOptions), S_OK))
			{
				m_hr = m_pITxnLocal->StartTransaction(m_fIsoLevel,
				0, m_pITxnOptions, NULL);		

				//Do Get one more time, this time while txn is active
				if (CHECK(m_pITxnOptions->GetOptions(&m_TxnOptions), S_OK))
				{
					//Should now have new values
					COMPARE(m_TxnOptions.ulTimeout, TIMEOUT);
					COMPARE(strcmp((CHAR *)m_TxnOptions.szDescription, szDESCRIPTION), 0);
				}

				//Either of these is acceptable for StartTransaction
				if (m_hr == S_OK)
				{ 
					//Clean up txn
					CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
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
	
		SAFE_RELEASE(m_pITxnOptions);
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
	BOOL				fResults	= FALSE;
	ITransactionObject	*pITranObj	= NULL;
	HRESULT				hr;

	if (CHECK(m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, NULL), S_OK))		
	{
		//Try to get the txn options interface
		hr	= m_pITxnLocal->GetOptionsObject(&m_pITxnOptions);
			
		if (S_OK==hr)
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
					CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
					m_hr = m_pITxnLocal->StartTransaction(m_fIsoLevel,0, m_pITxnOptions, NULL);		

					//Either of these is acceptable
					if (m_hr == S_OK)
					{
						//Clean up txn
						CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
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
				else
				{
					CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
				}
				}
			SAFE_RELEASE(m_pITxnOptions);
		}
		else
		{
			if (DB_E_NOTSUPPORTED==hr)
			{
				//test for IID_ITransactionObject OI here
				if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionObject,SESSION_INTERFACE, (IUnknown **)&pITranObj))
				{
						SAFE_RELEASE(pITranObj);
						return TEST_SKIPPED;
				}
				//free memory that was incorrectly allocated
				SAFE_RELEASE(pITranObj);
			}
		}
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
	BOOL				fResults	= FALSE;
	ITransactionObject	*pITranObj	= NULL;
	HRESULT				hr;

	if (CHECK(m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, NULL), S_OK))		
	{
		//Try to get the txn options interface
		hr	= m_pITxnLocal->GetOptionsObject(&m_pITxnOptions);

		if (S_OK==hr)
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
				CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
				m_hr = m_pITxnLocal->StartTransaction(m_fIsoLevel,0, m_pITxnOptions, NULL);		

				//Either of these is acceptable
				if (m_hr == S_OK)
				{
					//Clean up txn
					CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
					fResults = TRUE;
				}
				else
				{
					//This is OK, too
					if (m_hr == XACT_E_NOTIMEOUT)
					{
						fResults = TRUE;
					}
			        //Clean up txn
			        CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), S_OK);
				}
			}		
			SAFE_RELEASE(m_pITxnOptions);
		}
		else
		{
			if (DB_E_NOTSUPPORTED==hr)
			{
				//test for IID_ITransactionObject OI here
				if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionObject,SESSION_INTERFACE, (IUnknown **)&pITranObj))
				{
					SAFE_RELEASE(pITranObj);
					return TEST_SKIPPED;
				}
				//free memory that was incorrectly allocated
				SAFE_RELEASE(pITranObj);
			}
		}
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
	ReleaseDBSession();

	SAFE_RELEASE(m_pITxnLocal);

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
	HRESULT					hr			= NULL;
	ITransactionObject		*pITranObj	= NULL;

	m_pITxnOptions = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	//}}
	{
		//Try to get the txn options interface
		hr = m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&m_pITxnOptions);
			
		if (S_OK==hr)
		{
			return TEST_PASS;		
		}	
		else
		{
			if (DB_E_NOTSUPPORTED==hr)
			{
				//test for IID_ITransactionObject OI here
				if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionObject,SESSION_INTERFACE, (IUnknown **)&pITranObj))
				{
					SAFE_RELEASE(pITranObj);
					return TEST_SKIPPED;
				}
				//free memory that was incorrectly allocated
				SAFE_RELEASE(pITranObj);
			}
		}
	}
	return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid ITransactionLocal calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	ULONG	ulTxnLvl1 = 0;
	ULONG	ulTxnLvl2 = 0;
	BOOL	fResults = FALSE;
	ITransactionOptions * pITxnOptions = NULL;
	XACTTRANSINFO	TxnInfo;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionLocal method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//create an error object
	m_pExtError->CauseError();

	//Try to get the txn options interface
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions),
		S_OK))
		//Do extended check following GetOptionsObject
		fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);

	//create an error object
	m_pExtError->CauseError();
	
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		0, NULL, &ulTxnLvl1), S_OK))
	{
		//Do extended check following StartTransaction
		fResults &= XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);

		//create an error object
		m_pExtError->CauseError();

		//call GeTransactionInfo
		if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(&TxnInfo), S_OK))
			//Do extended check following GetTransactionInfo
			fResults &= XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);

		//This will succeed if nested txns are supported, else return
		//XACT_E_XTIONEXISTS
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTxnLvl2);
		
		//First level must always be 1
		COMPARE(ulTxnLvl1, 1);
		
		if (m_hr == S_OK)
		{
			//Make sure the nesting worked
			COMPARE(ulTxnLvl2, 2);	
		}
		else
		{
			//Make sure we got the right return value
			CHECK(m_hr, XACT_E_XTIONEXISTS);
		}

		//create an error object
		m_pExtError->CauseError();

		//End the txn -- this should end both levels, if applicable
		if(CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0), S_OK))	
			//Do extended check following Commit
			fResults &= XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	}
	
	ReleaseAllRowsetsAndTxns();

	SAFE_RELEASE(pITxnOptions);
	
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
	BOOL	fResults = FALSE;
	BOID	boidReason = BOID_NULL;	//Just so we have some value

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionLocal method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
	{
		//create an error object
		m_pExtError->CauseError();

		if(CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(&boidReason, FALSE, FALSE), S_OK))
			//Do extended check following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
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
  	XACTOPT	TxnOptions;
	BOOL	fResults = FALSE;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionOptions method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	//Set time out to non zero
	TxnOptions.ulTimeout = MAX_ULONG;
	TxnOptions.szDescription[0] = '\0';

 	//create an error object
	m_pExtError->CauseError();
	
	//Set our time out option
	if (CHECK(m_hr=m_pITxnOptions->SetOptions(&TxnOptions), S_OK))
	{
			//Do extended check following SetOptions
			fResults = XCHECK(m_pITxnOptions, IID_ITransactionOptions, m_hr);
		
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
			0, m_pITxnOptions, NULL);
	    
		ReleaseAllRowsetsAndTxns();

		//Either of these is acceptable
		if (m_hr == S_OK || m_hr == XACT_E_NOTIMEOUT)
			fResults &= TRUE;
	}	
	if(fResults)   
		return TEST_PASS;
	else
		return TEST_FAIL;

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
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	XACTOPT	TxnOptions;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionOptions method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
   	
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//create an error object
		m_pExtError->CauseError();
	
		//Make sure we can also get the options with
		//TxnOptions allocated on stack (so we know
		//provider isn't trying to free the memory)
		if (CHECK(m_hr=pITxnOptions->GetOptions(&TxnOptions), S_OK))
			//Do extended check following GetOptions
			fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
		
		SAFE_RELEASE(pITxnOptions);
	}

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
   	BOOL	fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))
	{	
	   	//create an error object
		m_pExtError->CauseError();

		//Check for proper return code for given parameters
		if( CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 1),  
			XACT_E_NOTSUPPORTED))
			//Do extended check following Commit
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	}
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOISORETAIN StartTransaction call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	BOOL fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  	
	//create an error object
	m_pExtError->CauseError();

	//Make isoFlags 1, nothing but 0 is valid
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		1, NULL,NULL), XACT_E_NOISORETAIN))		
			//Do extended check following StartTransaction
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
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
	BOOL	fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		 //create an error object
		m_pExtError->CauseError();

		//Check for invalid arg on pinfo = NULL
		if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(NULL), E_INVALIDARG))
			//Do extended check following GetTransactionInfo
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	}
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

	
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptionObject call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_8()
{
	BOOL fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
  	//create an error object
	m_pExtError->CauseError();
	
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetOptionsObject(NULL), E_INVALIDARG))
			//Do extended check following GetOptionsObject
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Abort call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_9()
{
	BOOL fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
  	//create an error object
	m_pExtError->CauseError();
	
	
	//Try to abort when we've never called StartTransaction
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
			//Do extended check following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	 
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

 // CNoTxn


}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptions call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_10()
{
	BOOL	fResults = FALSE;
	ITransactionOptions * pITxnOptions = NULL;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransitionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
	  	//create an error object
		m_pExtError->CauseError();
	
		//Expect invalid arg for NULL
		if (CHECK(m_hr=pITxnOptions->SetOptions(NULL), E_INVALIDARG))
			//Do extended check following SetOptions
			fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG SetOptions call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_11()
{
  
  ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
	  	//create an error object
		m_pExtError->CauseError();
	
		//Make sure we can also get the options
		if (CHECK(m_hr=pITxnOptions->GetOptions(NULL), E_INVALIDARG))
			//Do extended check following GetOptions
			fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_ISOLATIONLEVEL StartTransaction calls no with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_12()
{
	BOOL fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
	
  	//Make isoLevel invalid -- since these values are mutually exclusive,
	//or'ing them together will create an invalid value
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(
		ISOLATIONLEVEL_CHAOS | ISOLATIONLEVEL_READUNCOMMITTED | 
		ISOLATIONLEVEL_READCOMMITTED | ISOLATIONLEVEL_SERIALIZABLE,
		0, NULL, NULL), XACT_E_ISOLATIONLEVEL))		
			//Do extended check following StartTransaction
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptionObject calls no with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_13()
{
	BOOL	fResults = FALSE;
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetOptionsObject(NULL), E_INVALIDARG))
			//Do extended check following GetOptionsObject
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Abort calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_14()
{
	BOOL fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  
	//Try to abort when we've never called StartTransaction
//	if (CHECK(m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
			//Do extended check following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

  //CNoTxn


}
// }}

// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Commit calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_15()
{
	BOOL fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetResynch method.
	//We then check extended errors to verify the right extended error behavior.
  
 	//Try to commmit when we've never called StartTransaction
	//if (CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION))
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION))
		//Do extended check following Commit
		fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
  
	
	//  CNoTxn
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_16()
{
  	XACTTRANSINFO	TxnInfo;
	BOOL	fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors to verify the right extended error behavior.
  

	//Try to GetTransactionInfo when we've never called StartTransaction
	//if (CHECK(m_hr=m_pITxnLocal->GetTransactionInfo(&TxnInfo), XACT_E_NOTRANSACTION))
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(&TxnInfo), XACT_E_NOTRANSACTION))
		//Do extended check following GetTransactionInfo
		fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransactionLocal, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;


 // CNoTxn

}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG SetOptions calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_17()
{

	BOOL	fResults = FALSE;
	ITransactionOptions * pITxnOptions = NULL;
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  

	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//Expect invalid arg for NULL
		if (CHECK(m_hr=pITxnOptions->SetOptions(NULL), E_INVALIDARG))
			//Do extended check following SetOptions
			fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}

// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetOptions calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_18()
{
	
	ITransactionOptions * pITxnOptions = NULL;
	BOOL	fResults = FALSE;
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionOptions method.
	//We then check extended errors to verify the right extended error behavior.
  
	if (CHECK(m_pChgRowset1->m_pITxnLocal->GetOptionsObject(&pITxnOptions), S_OK))
	{
		//Make sure we can also get the options
		if (CHECK(m_hr=pITxnOptions->GetOptions(NULL), E_INVALIDARG))
			//Do extended check following GetOptions
			fResults = XCHECK(pITxnOptions, IID_ITransactionOptions, m_hr);
		
		SAFE_RELEASE(pITxnOptions);
	}

	if (fResults)  
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
  

// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid Abort calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_19()
{
	BOOL	fResults = FALSE;
	BOID	boidReason = BOID_NULL;	//Just so we have some value

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify nothing
	//is set since an error object shouldn't exist following a successful call.
   	
	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
		//Check for proper return code for given parameters
	{
		//create an error object
		m_pExtError->CauseError();

		if(CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(&boidReason, FALSE, FALSE), S_OK))
			//Do extended check on ITransaction following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	}

	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid TransactionLocal calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_20()
{
	
	ULONG	ulTxnLvl1 = 0;
	ULONG	ulTxnLvl2 = 0;
	BOOL	fResults = FALSE;
	XACTTRANSINFO	TxnInfo;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify nothing
	//is set since an error object shouldn't exist following a successful call.
   	
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
		0, NULL, &ulTxnLvl1), S_OK))
	{
		//create an error object
		m_pExtError->CauseError();

		//call GeTransactionInfo
		if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(&TxnInfo), S_OK))
			//Do extended check following StartTransaction
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);

		//This will succeed if nested txns are supported, else return
		//XACT_E_XTIONEXISTS
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,
			0, NULL, &ulTxnLvl2);
		
		//First level must always be 1
		COMPARE(ulTxnLvl1, 1);
		
		if (m_hr == S_OK)
		{
			//Make sure the nesting worked
			COMPARE(ulTxnLvl2, 2);	
		}
		else
		{
			//Make sure we got the right return value
			CHECK(m_hr, XACT_E_XTIONEXISTS);
		}

		//create an error object
		m_pExtError->CauseError();

		//End the txn -- this should end both levels, if applicable
		if(CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0), S_OK))	
			//Do extended check following Commit
			fResults &= XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	}
	
	ReleaseAllRowsetsAndTxns();

	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTSUPPORTED Commit calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_21()
{
   
   BOOL	fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify the right
	//extended error behavior.
  
	//Start Txn
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))
	{	
	   	//create an error object
		m_pExtError->CauseError();

		//Check for proper return code for given parameters
		if( CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 1),  
			XACT_E_NOTSUPPORTED))
			//Do extended check following Commit
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	}
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults)  
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetTransactionInfo calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_22()
{

	BOOL	fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify the right
	//extended error behavior.
  
	if (CHECK(m_pChgRowset1->StartTxn(m_fIsoLevel), S_OK))	
	{
		 //create an error object
		m_pExtError->CauseError();

		//Check for invalid arg on pinfo = NULL
		if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(NULL), E_INVALIDARG))
			//Do extended check following GetTransactionInfo
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	}
	//Cleanup
	ReleaseAllRowsetsAndTxns();

	if (fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;

	
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Abort calls with previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_23()
{
	
	BOOL fResults = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify the right
	//extended error behavior.
  
  	//create an error object
	m_pExtError->CauseError();
	
	
	//Try to abort when we've never called StartTransaction
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
			//Do extended check following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	 
	
	if(fResults)  
		return TEST_PASS;
	else
		return TEST_FAIL;

 // CNoTxn


}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Abort calls with no previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_24()
{
	
	BOOL fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify the right
	//extended error behavior.
  
	//Try to abort when we've never called StartTransaction
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE), XACT_E_NOTRANSACTION))
			//Do extended check following Abort
			fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	
	if(fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

  //CNoTxn


}
// }}

// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION Commit calls with no previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_25()
{
	
	BOOL fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetResynch method.
	//We then check extended errors to verify the right extended error behavior.
  
 	//Try to commmit when we've never called StartTransaction
	//if (CHECK(m_pITxnLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION))
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION))
		//Do extended check following Commit
		fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	
	if(fResults) 
		return TEST_PASS;
	else
		return TEST_FAIL;
  
	
	//  CNoTxn

}
// }}

// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc XACT_E_NOTRANSACTION GetTransactionInfo calls with no previous error object existing, check error on ITransaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_26()
{
	
  	XACTTRANSINFO	TxnInfo;
	BOOL	fResults = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ITransactionLocal method.
	//We then check extended errors on ITransaction interface to verify the 
	//right extended error behavior.
  

	//Try to GetTransactionInfo when we've never called StartTransaction
	//if (CHECK(m_hr=m_pITxnLocal->GetTransactionInfo(&TxnInfo), XACT_E_NOTRANSACTION))
	if (CHECK(m_hr=m_pChgRowset1->m_pITxnLocal->GetTransactionInfo(&TxnInfo), XACT_E_NOTRANSACTION))
		//Do extended check following GetTransactionInfo
		fResults = XCHECK(m_pChgRowset1->m_pITxnLocal, IID_ITransaction, m_hr);
	
	if(fResults)  
		return TEST_PASS;
	else
		return TEST_FAIL;


  //CNoTxn

}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN the provider does not support starting a new transaction with existing open rowset objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_27()
{
	HRESULT		hr			= S_OK;
	ULONG		ulTxnLvl1	= 0;
	BOOL		fResults	= FALSE;

	hr=m_pChgRowset2->MakeRowset();
	if (!(hr==S_OK||hr==DB_S_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}
	
	//Start Txn on our second DB Session
	hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTxnLvl1);

	if (!(hr==S_OK||hr==DB_E_OBJECTOPEN))
	{
		goto CLEANUP;
	}
	fResults	= TRUE;
CLEANUP:		
	ReleaseAllRowsetsAndTxns();	

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


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	// Release the Objects
	SAFE_RELEASE(m_pITxnOptions);

	//if (m_pExtError)
	//	delete m_pExtError;
	//m_pExtError = NULL;
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());

}	// }}
// }}
// }}


//////////////////////////////////////////////////////////////////////////////
// NOTE: This testcase is targeted towards testing ODBC Provider
// thus comments are assuming that is our provider; If that
// is not our provider, we will fail the Init.  Firehose mode
// refers to the mode which SQL Server uses to implement a read only
// forward only cursor.  This implementation ties up the TDS stream
// to the server, and thus only one hstmt is allowed.  ODBC Provider needs
// to allocate a new hstmt for every new command, thus we will envoke
// firehose mode (by disabling backwards scrolling and setting no other
// properties) and then try to get additional commands.
//////////////////////////////////////////////////////////////////////////////


// {{ TCW_TC_PROTOTYPE(TCMultipleCommands)
//*-----------------------------------------------------------------------
//|	Test Case:		TCMultipleCommands - Tests mutliple commands with and without transactions
//|	Created:			08/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMultipleCommands::Init()
{
	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetObject::Init())
	// }}
	{
		m_pICommand2 = NULL;		
		m_fFireHoseMode = FALSE;

		//This will create a table under the covers
		//Use order by so that if we change any rows in the table,
		//we still know what the first row in the rowset is
		CreateRowsetObject();

		//this part of the test is kagera specific and ok to run with commands
		//since privlib now makes a rowset off of IOpenRowset we no longer 
		//have  a class member  command object after calling CreateRowsetObject
		//so make one here
		if (VerifyInterface(m_pIAccessor, IID_IAccessor, COMMAND_INTERFACE,
			(IUnknown **)&m_pICommand))
	
		//If we don't support commands, fail now as we
		//intend on stressing multiple commands
		//Find out if PASS BY REF Accessors are supported		
		if (!m_pICommand)
			return FALSE;		

		//Record if we are running against the Microsoft SQL Server ODBC Driver
		LPWSTR wszProviderName = NULL;
		GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &wszProviderName);
		if (!wcscmp((LPWSTR)wszProviderName, L"Microsoft SQL Server"))
		{
			m_fMSSQLServer = TRUE;
			m_fMultipleHstmts = FALSE;
		}
		else
		{
			m_fMSSQLServer = FALSE;
			m_fMultipleHstmts = TRUE;
			odtLog << L"Assuming that the ODBC Driver supports multiple Hstmts per connection.\n";
		}

		PROVIDER_FREE(wszProviderName);

		//Now release the rowset and command, and use the
		//command which CTable generates automatically
		//so that only one total command exists on the session
		ReleaseRowsetObject();
		ReleaseCommandObject();
		SetCommandObject(m_pTable->m_pICommand);
		
		return TRUE;
	}
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc Enables/Disables Firehose mode by turning scrolling
// backwards off or on.
//
// @rdesc TRUE or FALSE
//
HRESULT TCMultipleCommands::SetFirehoseMode(ICommand * pICommand, BOOL	fOn)
{
	ICommandProperties * pICmdProps = NULL;
	DBPROPSET	DBPropSet;
	DBPROP		DBProp;
	BOOL		fResults = FALSE;
	HRESULT		hr = ResultFromScode(S_OK);

	DBPropSet.rgProperties = &DBProp;
	DBPropSet.cProperties = 1;
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;

	DBProp.dwPropertyID = DBPROP_CANSCROLLBACKWARDS;
	DBProp.dwOptions = DBPROPOPTIONS_REQUIRED;
	DBProp.colid = DB_NULLID;
	DBProp.vValue.vt = VT_BOOL;
	//Set property on or off, depending on requested behavior
	//Note that backwards scrolling is OPPOSITE of firehose mode
	if (!fOn)
		V_BOOL(&(DBProp.vValue)) = VARIANT_TRUE;
	else
		V_BOOL(&(DBProp.vValue)) = VARIANT_FALSE;

		//set props
	hr = SetRowsetProperties(&DBPropSet, 1);	
	
	//If everything went OK, we know we have backwards scrolling,
	//thus we can't be in firehose mode
	if (SUCCEEDED(hr) && !fOn)
		m_fFireHoseMode = FALSE;
	else
		//We assume we are in read only forward only (firehose) mode
		m_fFireHoseMode = TRUE;
	return hr;
}


//--------------------------------------------------------------------
// @mfunc Verifies you can get data on second command and then
// releases this command.
//
// @rdesc TRUE or FALSE
//
HRESULT TCMultipleCommands::VerifyAndReleaseCommand(ICommand **ppICommand, BOOL fFireHoseMode)
{	
	ICommand	*pTempCmd = NULL;
	IAccessor	*pTempAcc = NULL;
	DB_LORDINAL *pTmprgTC = NULL;
	
	//Don't want to give out E_FAIL for any reason beyond that the TDS line is blocked
	HRESULT		hr = ResultFromScode(E_INVALIDARG);	
	
	//If we don't have a command, something went wrong
	if (!m_pICommand)
	{	
		hr = ResultFromScode(E_INVALIDARG);
		goto DONE;
	}

	//Put us in the correct mode.  
	SetFirehoseMode(*ppICommand, fFireHoseMode);

	//If we wanted non firehose mode but that isn't supported,	
	//Just continue gracefully since its just a lack of support.
	//We don't care if we failed on setting firehose mode
	//because that will be the default anyway if setting 
	//CANSCROLLBACKWARDS to false fails.
	if (!fFireHoseMode &&(fFireHoseMode!= m_fFireHoseMode))
	{			
		
		hr = NOERROR;
		goto DONE;
	}

	//Use our rowset framework for this command, so save what
	//we've got in there currently, and then replace it
	pTempCmd = m_pICommand;
	pTempAcc = m_pIAccessor;
	pTmprgTC = m_rgTableColOrds;

	m_pICommand = *ppICommand;
	m_pIAccessor = NULL;
	
	//Create a rowset object on this command
	hr = CreateRowsetObject();
	if (SUCCEEDED(hr))
	{
		hr = GetRows(m_pIAccessor);
	}
	
	//Now get rid of this command/rowset and replace the old ones		
	ReleaseRowsetObject();
	m_pICommand		 = pTempCmd;
	m_pIAccessor	 = pTempAcc;
	m_rgTableColOrds = pTmprgTC;

DONE:

	//Now release the command the user passed us, since
	//they expect us to clean it up
	SAFE_RELEASE(*ppICommand);

	return hr;

}

//--------------------------------------------------------------------
// @mfunc Starts real firehose mode by getting a row.  This function
// cleans up after itself and releases the row.
//
// @rdesc TRUE or FALSE
//
HRESULT TCMultipleCommands::GetRows(IUnknown * pRowset)
{
	IRowset		*pIRowset		= NULL;
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW		hRow;
	HROW		*phRow			= &hRow;
	HRESULT		hr				= ResultFromScode(E_FAIL);

	//Now make sure we can GetNextRows
	if (VerifyInterface(pRowset, IID_IRowset, ROWSET_INTERFACE,
		(IUnknown **)&pIRowset))
	{
		hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow);
		if (SUCCEEDED(hr))
			pIRowset->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL);
		
		SAFE_RELEASE(pIRowset);
	}
	else
	{
		hr = E_NOINTERFACE;	
	}

	return hr;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc First rowset in Firehose mode with no transaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleCommands::Variation_1()
{
	BOOL	fResults = FALSE;

	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	//We should be able to get a second command by getting another hstmt
	//if the rowset isn't in firehose mode, or if need be by allocating 
	//another hdbc since we aren't in a transaction
	

	//Try to Set Firehose mode OFF.
	SetFirehoseMode(m_pICommand, FALSE);

	if (!CHECK(CreateRowsetObject(), S_OK))	
		goto CLEANUP;
	
	//Another hstmt should work, either on the same hdbc or a new one
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown **)&m_pICommand2), S_OK))
		goto CLEANUP;

	if (m_fFireHoseMode)
	{
		//Initiate tying up the server with command 1
		if (FAILED(GetRows(m_pIAccessor)))
			goto CLEANUP;

		//Command 1 is in firehose mode, so Command2 should 
		//be able to produce a rowset in non firehose mode
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, FALSE)))
			goto CLEANUP;
	}
	else
	{	
		//Command 1 is not in firehose mode, so command 2 should
		//be able to produce a rowset in firehose mode
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	}

	ReleaseRowsetObject();

	//Now try to SetFirehose mode ON. 
	SetFirehoseMode(m_pICommand, TRUE);

	if (!CHECK(CreateRowsetObject(), S_OK))
		goto CLEANUP;

	//This should still succeed since we can get a new hdbc
	//if our active hstmts are limited to 1 by the odbc driver
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown **)&m_pICommand2), S_OK))
		goto CLEANUP;

	if (m_fFireHoseMode)
	{
		//Initiate tying up the server with command 1
		if (FAILED(GetRows(m_pIAccessor)))
			goto CLEANUP;

		//Command 1 is in firehose mode, so Command2 should 
		//be able to produce a rowset in firehose mode by some means
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	}
	else
	{	
		//Command 1 is not in firehose mode, so command 2 should
		//be able to produce a rowset in firehose mode
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	}

	//Go back to just a command and try again
	ReleaseRowsetObject();
	
	//We are no longer in active firehose mode, so this should work
	if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown **)&m_pICommand2), S_OK))
		goto CLEANUP;

	//Command2 should be able to produce a firehose mode rowset
	//since no rowset is open on command 1		
	if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
		goto CLEANUP;	
	
	fResults = TRUE;

CLEANUP:
	//Cleanup for subsequent variations
	ReleaseRowsetObject();

	//Release the Command object
	SAFE_RELEASE(m_pICommand2)

	if (fResults)
		return TEST_PASS;
	else 
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc First rowset in Firehose mode with a transaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleCommands::Variation_2()
{
	BOOL						fResults			= FALSE;	
	ITransactionLocal		*pITxnLocal		= NULL;

	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	if (!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal,SESSION_INTERFACE, (IUnknown **)&pITxnLocal))
	{
		return TEST_PASS;
	}
	//Put us in a txn
	if (!CHECK(pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, NULL), S_OK))
	{
		goto CLEANUP;
	}
/*
	//Set Firehose mode OFF if it is supported
	SetFirehoseMode(m_pICommand, FALSE);
	//If setting fire hose mode off worked
	if (!m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))	
			goto CLEANUP;
		
		//This should not tie up the server since we
		//shouldn't be in firehose mode
		if (FAILED(GetRows(m_pIAccessor)))
			goto CLEANUP;
		
		//If multiple hstmts on an hdbc are allowed, this will work.  This
		//will also work on MS SQL Server since they should be clever about
		//knowing  that we aren't in firehose mode, thus the 1 hstmt restriction
		//isn't necessary
		if (m_fMultipleHstmts || m_fMSSQLServer)
		{	
				if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), S_OK))
					goto CLEANUP;
		}
		//Otherwise we should fail since we can't get more hstmts on the current hdbc
		//and another hdbc isn't allowed in a txn
		else
		{
			if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), E_FAIL))
				goto CLEANUP;
		}

		ReleaseRowsetObject();
	}

	//Now SetFirehose mode ON.  If this fails, it just means we
	//are in firehose mode by default since we can't scroll backwards
*/
	SetFirehoseMode(m_pICommand, TRUE);

	if (!CHECK(CreateRowsetObject(), S_OK))
		goto CLEANUP;
	
	//Initiate tying up the server with command 1
	if (FAILED(GetRows(m_pIAccessor)))
		goto CLEANUP;

	//MS SQL Server should not let us to get an hstmt while in active firehose
	//mode.  Also, if the driver does not allow multiple hstmts, this
	//should fail since we can't get another hdbc while in a txn
	if (!m_fMultipleHstmts || m_fMSSQLServer)
	{
		if (!CHECK(m_pIDBCreateCommand->CreateCommand	(	NULL, 
															IID_ICommand,
															(IUnknown **)&m_pICommand2), 
															E_FAIL
														))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&m_pICommand2), S_OK))
			goto CLEANUP;

		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	}		

	//Go back to just a command and try again
	ReleaseRowsetObject();

	//If multiple hstmts on an hdbc are allowed, this will work.  This
	//will also work on MS SQL Server since they should be clever about
	//knowing  that we aren't in firehose mode, thus the 1 hstmt restriction
	//isn't necessary
	if (m_fMultipleHstmts || m_fMSSQLServer)
	{
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&m_pICommand2), S_OK))
			goto CLEANUP;

		//We should be able to get another connection in firehose mode
		//since the first one is not in firehose mode
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	}
	//Otherwise we should fail since we can't get more hstmts on the current hdbc
	//and another hdbc isn't allowed in a txn
	else
	{
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&m_pICommand2), E_FAIL))
			goto CLEANUP;
	}

	fResults = TRUE;
	

CLEANUP:
	//Cleanup for subsequent variations
	ReleaseRowsetObject();

	//Release the Command object
	SAFE_RELEASE(m_pICommand2);

	pITxnLocal->Abort(NULL, false, false);

	SAFE_RELEASE(pITxnLocal);

	if (fResults)
		return TEST_PASS;
	else 
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Second rowset in firehose mode with no transaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleCommands::Variation_3()
{
	BOOL				fResults = FALSE;	
	
	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	//Set Firehose mode OFF. 
	SetFirehoseMode(m_pICommand, FALSE);
	//If fire hose mode can't get turned off, we have nothing to test
	if (!m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))	
			goto CLEANUP;
			
		//This should not tie up the server since we shouldn't be
		//in firehose mode with command 1
		if (FAILED(GetRows(m_pIAccessor)))
			goto CLEANUP;
		
		//This should always work because we can use another hdbc if we
		//need to since we aren't in a transaction		
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&m_pICommand2), S_OK))
			goto CLEANUP;
		
		//Firehose mode for the second rowset should be OK because
		//ODBC Provider should be smart enough to do it on a second hdbc
		//if it needs to
		
		if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
			goto CLEANUP;
	
	}						
		fResults = TRUE;
	
CLEANUP:
	//Cleanup for subsequent variations
	ReleaseRowsetObject();

	//Release the Command object
	SAFE_RELEASE(m_pICommand2)

	if (fResults)
		return TEST_PASS;
	else 
		return TEST_FAIL;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Second rowset in firehose mode with a transaction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleCommands::Variation_4()
{
	BOOL				fResults = FALSE;	
	ITransactionLocal * pITxnLocal = NULL;

	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	if (!VerifyInterface(m_pIDBCreateCommand, IID_ITransactionLocal,
		SESSION_INTERFACE, (IUnknown **)&pITxnLocal))
		return TEST_PASS;

	//Put us in a txn
	if (!CHECK(pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,
		0, NULL, NULL), S_OK))
		goto CLEANUP;

	//Set Firehose mode OFF. 
	SetFirehoseMode(m_pICommand, FALSE);
	//If fire hose mode can't get turned off, we have nothing to test
	if (!m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))	
			goto CLEANUP;
			
		//This should not tie up the server since we shouldn't be
		//in firehose mode with command 1
		if (FAILED(GetRows(m_pIAccessor)))
			goto CLEANUP;
		
		//If multiple hstmts on an hdbc are allowed, this will work.  This
		//will also work on MS SQL Server since they should be clever about
		//knowing  that we aren't in firehose mode, thus the 1 hstmt restriction
		//isn't necessary
		if (m_fMultipleHstmts || m_fMSSQLServer)
		{
			if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), S_OK))
				goto CLEANUP;
			
			//When we ask for a second rowset in firehose mode, this should 
			//succeed, since ODBC Provider should increase the rowset size
			//to prevent it, even though that would normally
			//be the default mode in this situation (with no properties set)
			if (m_fMSSQLServer)
			{
				if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
					goto CLEANUP;
			}
			else
			{
				//Firehose mode does not apply for multiple hstmt drivers
				//so this should succeed
				if (FAILED(VerifyAndReleaseCommand(&m_pICommand2, TRUE)))
					goto CLEANUP;
			}

		}
		//Otherwise we should fail since we can't get more hstmts on the current hdbc
		//and another hdbc isn't allowed in a txn
		else
		{
			if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), E_FAIL))
				goto CLEANUP;
		}

	}			
		fResults = TRUE;
	
CLEANUP:
	//Cleanup for subsequent variations
	ReleaseRowsetObject();

	//Release the Command object
	SAFE_RELEASE(m_pICommand2)
	SAFE_RELEASE(pITxnLocal);

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
BOOL TCMultipleCommands::Terminate()
{

	ReleaseRowsetObject();
	ReleaseCommandObject(1);
	//Expect the command to have a ref count of 1 after
	//we've released it since this same command is still 
	//in use by the CTable object in m_pTable

	// Drop the table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	ReleaseDBSession();
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetObject::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors2)
//*-----------------------------------------------------------------------
//|	Test Case:		TCExtendedErrors2 - Check extended errors on multipla commands
//|	Created:			11/26/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors2::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMultipleCommands::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc open a txn in firehose mode -- E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors2::Variation_1()
{
	BOOL				fResults = FALSE;	
	ITransactionLocal * pITxnLocal = NULL;
	HRESULT				hr;


	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	if (!VerifyInterface(m_pIDBCreateCommand, IID_ITransactionLocal,SESSION_INTERFACE, (IUnknown **)&pITxnLocal))
	{
		return TEST_PASS;
	}
	//SetFirehose mode ON.  If this fails, it just means we
	//are in firehose mode by default since we can't scroll backwards
	SetFirehoseMode(m_pICommand, TRUE);

	if (m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))
			goto CLEANUP;
	
		//We can not start a txn under firehose mode
		if (m_fMSSQLServer)
		{
			if (CHECK(hr=pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,
				0, NULL, NULL), E_FAIL))
				fResults = XCHECK(pITxnLocal, IID_ITransactionLocal, hr);
			else
			{
				pITxnLocal->Commit(FALSE, 0, 0);
				goto CLEANUP;
			}
		}
		//I am assuming other driver will not be affected by firehose mode
		else
		{
			if (!CHECK(hr=pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,
				0, NULL, NULL), S_OK))
				goto CLEANUP;
			pITxnLocal->Commit(FALSE, 0, 0);
		}
	}
	
	//if we cannot set firehose mode, it is fine.
	fResults = TRUE;

CLEANUP:

    pITxnLocal->Commit(FALSE, 0, 0);

	ReleaseRowsetObject();

	SAFE_RELEASE(pITxnLocal);

	if (fResults)
		return TEST_PASS;
	else 
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc second rowset with the first rowset in firehose mode within a txn -- E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors2::Variation_2()
{
	// TO DO:  Add your own code here
	BOOL				fResults = FALSE;	
	ITransactionLocal * pITxnLocal = NULL;
	HRESULT				hr;

	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	if (!VerifyInterface(m_pIDBCreateCommand, IID_ITransactionLocal,SESSION_INTERFACE, (IUnknown **)&pITxnLocal))
		return TEST_PASS;

	//Put us in a txn
	if (!CHECK(pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, NULL), S_OK))
		goto CLEANUP;

	//Now SetFirehose mode ON.  If this fails, it just means we
	//are in firehose mode by default since we can't scroll backwards
	SetFirehoseMode(m_pICommand, TRUE);

	if (m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))
			goto CLEANUP;
	

		//MS SQL Server should not let us to get an hstmt while in active firehose
		//mode.  Also, if the driver does not allow multiple hstmts, this
		//should fail since we can't get another hdbc while in a txn
		if (!m_fMultipleHstmts || m_fMSSQLServer)
		{
			if (CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), E_FAIL))
				fResults = XCHECK(m_pIDBCreateCommand, IID_IDBCreateCommand, hr);
			else
				goto CLEANUP;

		}
		else
		{
			if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
				(IUnknown **)&m_pICommand2), S_OK))
				goto CLEANUP;
			else
				fResults = TRUE;

		}		
	}
	//if we cannot set firehose on, it is okay
	else
	{
		fResults = TRUE;
	}
CLEANUP:
	ReleaseRowsetObject();
	
	//Release the Command object
	SAFE_RELEASE(m_pICommand2)

	if (pITxnLocal)
	{
		pITxnLocal->Commit(FALSE, 0, 0);
		SAFE_RELEASE(pITxnLocal);
	}

	if (fResults)
		return TEST_PASS;
	else 
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc create two connection with one in firehose, start txn -- XACT_E_CONNECTION_DENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors2::Variation_3()
{
	BOOL				fResults	= FALSE;	
	ITransactionLocal	*pITxnLocal = NULL;
	HRESULT				hr;

	//Firehose mode only applies to ODBC Provider
	if (m_pThisTestModule->m_ProviderClsid != CLSID_MSDASQL)
	{
		odtLog << L"This test case is ODBC Provider specific, and will not continue.\n";
		return TRUE;
	}
	
	if (!VerifyInterface(m_pIDBCreateCommand, IID_ITransactionLocal,SESSION_INTERFACE, (IUnknown **)&pITxnLocal))
	{
		return TEST_PASS;
	}
	//Now SetFirehose mode ON.  If this fails, it just means we
	//are in firehose mode by default since we can't scroll backwards
	SetFirehoseMode(m_pICommand, TRUE);
	
	if (m_fFireHoseMode)
	{
		if (!CHECK(CreateRowsetObject(), S_OK))
		{
			goto CLEANUP;
		}
		//This should still succeed since we can get a new hdbc
		if (!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand2), S_OK))
		{
			goto CLEANUP;
		}
		//Put us in a txn
		hr=pITxnLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, NULL); 
		if(hr==XACT_E_CONNECTION_DENIED)
		{
			fResults = XCHECK(pITxnLocal, IID_ITransactionLocal, hr);
		}
		else	
		{
			if (hr==E_FAIL)
			{
				fResults = XCHECK(pITxnLocal, IID_ITransactionLocal, hr);
			}
			else
			{
				pITxnLocal->Commit(FALSE, 0, 0);
			}
		}
		fResults=TRUE;
		goto CLEANUP;
	}
	//if we cannot set firehose on, it is okay
	else
		fResults = TRUE;


CLEANUP:

	ReleaseRowsetObject();
	
	//Release the Command object
	SAFE_RELEASE(m_pICommand2)
	SAFE_RELEASE(pITxnLocal);

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
BOOL TCExtendedErrors2::Terminate()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCMultipleCommands::Terminate())//Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCITransactionObject)
//*-----------------------------------------------------------------------
//|	Test Case:		TCITransactionObject 
//|	Created:			08/14/98
//*-----------------------------------------------------------------------
 
//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCITransactionObject::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTxnImmed::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Legal - Nested and ITransactionObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITransactionObject::Variation_1()
{
	BOOL				fResults						= TEST_FAIL;
	const	ULONG		ulNestedTnxs					= 10;
	ITransactionObject	*pITransactionObject			= NULL;
	ITransaction		*rgpITransaction[ulNestedTnxs];
	ULONG				i								= 0;
	ULONG				ulTransactionLevel				= 0;

	for (i=0;i<ulNestedTnxs;i++)
	{
		rgpITransaction[i]	= NULL;
	}

	//try to get an interface to ITransactionObject
	//this should work if test is past init
	if (!VerifyInterface(m_pChgRowset1->m_pITxnLocal, IID_ITransactionObject, SESSION_INTERFACE,(IUnknown **)&pITransactionObject))
	{
		//ITransactionObject not supported so skip this test class
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}

	for (i=0;i<ulNestedTnxs;i++)
	{
		if (!CHECK(m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTransactionLevel),S_OK))
		{
			goto CLEANUP;
		}
		if (!CHECK(pITransactionObject->GetTransactionObject(ulTransactionLevel,&(rgpITransaction[i])),S_OK))
		{
			goto CLEANUP;
		}
	}
	//commit all the levels 
	//hard coding is ulgy but this shows best what variation is trying to do
	if (CHECK((rgpITransaction[8])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	//since 8 was committed 9 is implicity committed as well
	if (CHECK((rgpITransaction[9])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), XACT_E_NOTRANSACTION)){goto CLEANUP;}

	if (CHECK((rgpITransaction[7])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[6])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[5])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[4])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[3])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[0])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	//since 0 was committed 1 and 2 are implicity committed as well
	if (CHECK((rgpITransaction[2])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}
	if (CHECK((rgpITransaction[1])->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK)){goto CLEANUP;}

	fResults = TEST_PASS;	
CLEANUP:
	ReleaseAllRowsetsAndTxns();	
	PROVIDER_FREE(pITransactionObject);
	for (i=0;i<ulNestedTnxs;i++)
	{
		if (rgpITransaction[i])
		{
			SAFE_RELEASE(rgpITransaction[i]);
			PROVIDER_FREE(rgpITransaction[i]);
		}
	}
	PROVIDER_FREE(pITransactionObject);
	return fResults;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Illegal ulTransactionLevel arg GetTransactionObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITransactionObject::Variation_2()
{
	BOOL				fResults						= TEST_FAIL;
	ITransactionObject	*pITransactionObject			= NULL;
	ITransaction		**ppITransaction				= NULL;

	//try to get an interface to ITransactionObject
	//this should work if test is past init
	if (!VerifyInterface(m_pChgRowset1->m_pITxnLocal, IID_ITransactionObject, SESSION_INTERFACE,(IUnknown **)&pITransactionObject))
	{
		//ITransactionObject not supported so skip this test class
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}
	//ulTransactionLevel can not be zero
	if (!CHECK(pITransactionObject->GetTransactionObject(0,ppITransaction),E_INVALIDARG))
	{
		goto CLEANUP;
	}
	fResults	 = TEST_PASS;
CLEANUP:
	PROVIDER_FREE(pITransactionObject);
	if (ppITransaction)
	{
		PROVIDER_FREE(*ppITransaction);
	}
	ReleaseAllRowsetsAndTxns();	
	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Illegal ppITransaction arg GetTransactionObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITransactionObject::Variation_3()
{
	BOOL				fResults						= TEST_FAIL;
	ITransactionObject	*pITransactionObject			= NULL;

	//try to get an interface to ITransactionObject
	//this should work if test is past init
	if (!VerifyInterface(m_pChgRowset1->m_pITxnLocal, IID_ITransactionObject, SESSION_INTERFACE,(IUnknown **)&pITransactionObject))
	{
		//ITransactionObject not supported so skip this test class
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}
	//ulTransactionLevel can not be zero
	if (!CHECK(pITransactionObject->GetTransactionObject(1,NULL),E_INVALIDARG))
	{
		goto CLEANUP;
	}
	fResults	 = TEST_PASS;
CLEANUP:
	PROVIDER_FREE(pITransactionObject);
	ReleaseAllRowsetsAndTxns();	
	return fResults;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc just Nested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCITransactionObject::Variation_4()
{
	BOOL				fResults						= TEST_SKIPPED;
	const	ULONG		ulNestedTnxs					= 100;
	ULONG				i								= 0;
	ULONG				j								= 0;
	ULONG				k								= 0;
	ULONG				ulTransactionLevel[ulNestedTnxs];

	//test to see if nested txns are supported, as of now there is no prop to tell whether nested txns are supported
	//so if the second StartTxns returns XACT_E_XTIONEXISTS then nested txns are not supported
	if (CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTransactionLevel[i]),S_OK))
	{
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTransactionLevel[i]);
		if (XACT_E_XTIONEXISTS==m_hr)
		{
			goto CLEANUP;
		}
		fResults = TEST_FAIL;
		if (m_hr!=S_OK)
		{
			COMPARE(S_OK,m_hr);
			goto CLEANUP;
		}
	}
	else
	{
		fResults = TEST_FAIL;
		goto CLEANUP;
	}

	//nest x number of txns + 2 from above
	for (i=0;i<ulNestedTnxs;i++)
	{
		m_hr = m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fIsoLevel,0, NULL, &ulTransactionLevel[i]);
		//when XACT_E_XTIONEXISTS is returned we've reached the nesting limit on the provider
		if (XACT_E_XTIONEXISTS == m_hr)
		{
			break;
		}
		if (S_OK!=m_hr)
		{
			COMPARE(S_OK,m_hr);
			goto CLEANUP;
		}

		//skip the check the first time since there will only the one entry in the list
		for (j=0;j<i;j++)
		{
			//if this level was already in the array then is was obtained from previous StartTxn call
			//levels must be unique
			if (ulTransactionLevel[i]==ulTransactionLevel[j])
			{
				goto CLEANUP;
			}
		}
	}	

	//Now commit all the levels 
	for (k=0;k<i;k++)
	{
		if (!CHECK(m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK))
		{
			COMPARE(S_OK,m_hr);
			goto CLEANUP;
		}
	}
	if (!CHECK(m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK))
	{
		COMPARE(S_OK,m_hr);
		goto CLEANUP;
	}
	if (!CHECK(m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK))
	{
		COMPARE(S_OK,m_hr);
		goto CLEANUP;
	}
	if (!CHECK(m_hr = m_pChgRowset1->m_pITxnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), XACT_E_NOTRANSACTION))
	{
		COMPARE(XACT_E_NOTRANSACTION,m_hr);
		goto CLEANUP;
	}

	fResults = TEST_PASS;
CLEANUP:
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
BOOL TCITransactionObject::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTxnImmed::Terminate());
}	// }}
// }}
// }}

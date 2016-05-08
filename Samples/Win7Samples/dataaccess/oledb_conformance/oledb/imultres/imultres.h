//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IMULTRES.H | Header file for IMultipleResults test module.
//
// @rev 01 | 09-19-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _IMULTRES_H_
#define _IMULTRES_H_

#define MAX_STMT_LEN		256
						  
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// OLE DB Test Private Library

//-----------------------------------------------------------------------------
// Global function declarations
//-----------------------------------------------------------------------------
LONG g_GetDBPROP_MULTILPLERESULTS(IUnknown * pDSO);

#define	ISROWSET	(m_eTestCase==TC_Rowset)
#define	ISROW		(m_eTestCase==TC_Row)

#define DECL_FLAGSANDIIDS(x)	const ULONG	ulIIDCount = x;				\
								ULONG i;								\
								DBRESULTFLAG	rgFlag[ulIIDCount];		\
								IID				rgIID[ulIIDCount];		\
								for(i=0; i<ulIIDCount; i++)		\
								{										\
									if(m_eTestCase == TC_Rowset)		\
									{									\
										rgFlag[i] = DBRESULTFLAG_DEFAULT;	\
										rgIID[i] = IID_IRowset;			\
									}									\
									else								\
									{									\
										rgFlag[i] = DBRESULTFLAG_ROW;	\
										rgIID[i] = IID_IRow;			\
									}									\
								}

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------
//This enum is used to serve as an index into our m_rgpwszSQL string table array.
//In this array, the select statement is at index 0, the insert statment is at
//index 1, etc.
enum ESQLSTMT
{	
	ESELECT = 0,
	EINSERT = 1,
	EUPDATE = 2,
	EDELETE = 3,
	ECREATE = 4,	
	EDROP = 5,
	EEMPTYSELECT = 6,
	ECOMPUTESUM = 7,
	ESELECTWAIT = 8,
	ERPCSELECTOUT,
	ESTMT_LAST		// Must remain the last enum value
};


//Used for TestTxn to determine how to end the transaction
enum ETXN	
{
	ETXN_COMMIT, 
	ETXN_ABORT
};

//This enumeration represents various objects that can be 
//obtained from GetResult. For e.g., TC_Row represents the Test 
//Case which will test getting a ROW object (mostly) from GetResult.
enum ETESTCASE
{
	TC_Rowset = 1,				//Getting Rowset
	TC_Row,						//Getting Row
};

enum EABORTSTATE
{
	EABORT_NONE,
	EABORT_BEFORE_GETRESULT,
	EABORT_AFTER_GETRESULT,
	EABORT_AFTER_GETRESULT_IMULTRES_RELEASED,
	EABORT_AFTER_PARTIAL_GETNEXTROWS,
	EABORT_AFTER_GETNEXTROWS,
	EABORT_AFTER_GETDATA,
	EABORT_AFTER_RESULTS_READ,
	EABORT_AFTER_DB_S_NORESULT,
	EABORT_AFTER_DB_S_ASYNCHRONOUS,
	EABORT_AFTER_RELEASEIMULTRES,
	EABORT_DURING_ISEQSTREAM,
	EABORT_AFTER_GETCOLUMNS,
	EABORT_AFTER_INSERT
};

enum EABORTTYPE
{
	EABORT_SYNCH = 0,
	EABORT_THREAD = 0x1000,
	EABORT_ASYNCH = 0x2000
};


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const ULONG	NUM_SQLSTMTS = 8;		//Number of values in SQLSTMT enum
const ULONG CHANGE_ROW_NUM = 10;	//Minimum number of rows in Change table
const ULONG SELECT_ROW_NUM = 30;	//Minumum number of rows in Select table
const ULONG	ROW_SEED = 3;			//Our change table will only have rows with this seed
const ULONG MAX_PARAM_NUM = 400;		//Max number of parameters, used for ordinal array
const ULONG SQLBUFSIZE = 500000;	//Max SQL buf size for special SQLOLEDB test


const WCHAR	wszCrtTblSuffix[] = L"_A";						//Used to differentiate our change 
															//table and newly created tables
const WCHAR wszCrtTbl[] = L"Create table %s (col1 int)";	//Use simple, standard type for create table tests
const WCHAR wszDropTbl[] = L"Drop table %s";
const WCHAR wszNegOneWarning[] = L"WARNING:  -1 was returned for *pcRowsAffected.  This is legal, but the provider may be able to return an accurate row count.\n";
const CHAR szOrdersFile[] = "sql2000_orders.sql";

typedef struct tagProcInfo
{
	LPWSTR pwszProcName;
	DBORDINAL cParams;
	DBORDINAL cParamIO;
	DBPARAMIO	eParamIO;
	DB_LORDINAL * prgParamColMap;
} ProcInfo;

typedef struct tagStmtInfo
{
	BOOL fHasStreamBinding;
	BOOL fOutParamsProcessed;
	BOOL fCombinedResult;
	DBORDINAL cParams;
} StmtInfo;

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------
BOOL IsServerCursorProperty(DBPROPID dwPropID);

//-----------------------------------------------------------------------------
// Class definitions
//-----------------------------------------------------------------------------

//Forward declaration
class CMultResults;


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
		

// CTestBase ---------------------------------------------------------------
//
// Performs common test functionality needed by variations testing
// IMultipleResults.
// 
//
class CTestBase : public CCommandObject, public TCBase
{

////////////////////////////////////////////////////
public:
	DBORDINAL m_cSelectTableRows;

	//@cmember Interface to command object
	ICommandText *			m_pICommandText;
	
	//@cmember CTOR
	CTestBase(LPWSTR wszTestCaseName) 
		: CCommandObject(wszTestCaseName),	//Call base class CTOR
		m_pICommandText(NULL)				//Init only member var
	{
		//Do nothing
	};

	//@cmember Pointer to table object used for all selects
	CTable *	m_pSelectTable;

	//@cmember Pointer to table object used for all inserts, updates, deletes
	CTable *	m_pChangeTable;

	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();


	//@cmember Creates a CMultResult object and puts an IMultipleResults pointer
	//in (*ppMultResults)->m_pIMultResults. User must release this pointer and
	//delete *ppMultResult after a successful call to this function.
	//If ceSQLStmts is 0, this function uses it's own fixed SQL Stmts for the object.
	//This can be used if the user doesn't care what kind of SQL Stmts are used.
	HRESULT SetUpGetResult(CMultResults ** ppMultResult, ULONG ceSQLStmts = 0, ESQLSTMT * rgeSqlStmts = NULL, DBRESULTFLAG* rgResFlags = NULL);
	
	//@cmember Uses IRowsetLocate to ensure it is functional on this rowset
	HRESULT TryLocate(IRowsetLocate * pIRowLoc);

	//@cmember Holds rows to verify it is possible
	HRESULT TryHoldRows(IRowset * pIRowset);

	//@cmember Sets the given rowset property using ICommandProperties to VARIANT_TRUE
	HRESULT SetRowsetPropertyOn(DBPROPID DBPropID, VARTYPE vt = VT_BOOL, void * pValue = (void *)VARIANT_TRUE);

	//@cmember Sets the given rowset property using ICommandProperties to the default value
	HRESULT SetRowsetPropertyDefault(DBPROPID DBPropID);
};


// CMultResults ---------------------------------------------------------------
//
// This class provides the base functionality for putting the OLE DB Provider
// in a multiple results state and keeping track of the expected behavior
// of the provider for these results
//
class CMultResults
{

///////////////////////////////////////////////////////////
private:

	//@cmember Name of table for all create table and drop table statements
	WCHAR *					m_pwszNewTableName;
	//@cmember Helper function to build all SQL Strings
	BOOL					BuildSQLStringTable();	


///////////////////////////////////////////////////////////
protected:

	//@cmember Interface to command from which all results statements are executed
	ICommandText *			m_pICommandText;
	//@cmember Interface to session which command is built on
	IDBCreateCommand *		m_pIDBCreateCommand;	
	//@cmember Count of SQL Stmts in one execution statement
	ULONG					m_ceSQLStmts;
	//@cmember Array of enums specifying SQL statements used in execution
	ESQLSTMT *				m_rgeSQLStmts;
	//@cmember Array of result flags used in GetResult
	DBRESULTFLAG *			m_rgResFlags;
	//cmember Array of rows each SQL statement affects on execution.
	//It's elements correspond to those of m_rgeSQLStmts
	DBROWCOUNT *			m_rgStmtRowsAffected;	
	//@cmember Pointer to array of ordinals in select table
	DB_LORDINAL *			m_rgSelectOrds;
	//@cmember Pointer to array of ordinals used in comparing data
	DB_LORDINAL *			m_rgCompOrds;
	//@cmember SQL Statement executed
	WCHAR *					m_pwszSQLStmt;
	//Our String Table -- An array of pointers to SQL Stmt Strings.  Note,
	//we use the ESQLSTMT enum values as the indexes into this array.
	WCHAR *					m_rgpwszSQL[ESTMT_LAST]; 

	// Array of stored proc names, one for each stored proc created and associated with the execute statement for the 
	// stored proc.
	ProcInfo				m_prgProcInfo[ESTMT_LAST]; 

	//@cmember Concats all SQL Strings in our array into 
	// one space separated string.
	HRESULT BuildConcatSQL();

	//@cmember Count of rows expected from select stmt.
	DBCOUNTITEM m_cRowsInSelect;

///////////////////////////////////////////////////////////
public:


	//@cmember Array of ordinals in consecutive order, to be used for Parameter ordinals
	static	DBORDINAL *	s_rgParamOrdinals;
	//@cmember Task memory allocator interface
	static	IMalloc *	s_pIMalloc;	


	//@cmember Pure virtual function for derived classes to set the
	//correct command text and execute it
	virtual HRESULT SetAndExecute(REFIID riid)=0;	
	//@cmember Checks all output parameters for correctness given the result index	
	BOOL AreGetResultOutParamsRight(ULONG iResult, IUnknown * pUnkRowset, REFIID riid, DBRESULTFLAG resFlag);
	//@cmember Checks all output parameters for correctness against the first expected result
	BOOL AreExecuteOutParamsRight(REFIID riid, IUnknown * pUnkRowset);
	//@cmember Executes the text set on this object's command -- can
	//be called by derived classes implementations of SetAndExecute
	//to perform the ICommand::Execute
	HRESULT	Execute(REFIID riid, DBPARAMS * pParams);
	//@cmember Init function to be called immediately after construction
	//for every object of this class type.  Function does NOT assume
	//ownership of memory arrays, it alloc's it's own and copies, thus
	//caller must free it's own arrays.
	BOOL	FInit(const ULONG cSQLStmts,
							ESQLSTMT * rgSQLStmts, 
							DBRESULTFLAG* rgResFlags,
							DBCOUNTITEM * rgStmtRowsAffected);
	
	//@cmember Compares m_cRowsGetResultAffected with the correct 
	//number to verify whether or not m_cRowsGetResultAffected is right
	virtual BOOL IsGetResultRowsAffectedRight(ULONG iResult)
	{
		//Default implementation is to check the row count
		//for the current result.
		//NOTE:  Technically this could also be -1, we may need to
		//change in the future, but for now expect the provider to
		//always be able to give us the correct row count and not -1		

		//-1 is always valid, but print a warning if it's
		//received for an insert, update or delete, since
		//usually these can be given a real row count
		if (m_cRowsGetResultAffected == -1)
		{
			if (m_rgeSQLStmts[iResult] == EINSERT ||
				m_rgeSQLStmts[iResult] == EUPDATE ||
				m_rgeSQLStmts[iResult] == EDELETE)
			{
				odtLog << wszNegOneWarning;
			
			}
			
			return TRUE;

		}
		else
			return m_cRowsGetResultAffected == m_rgStmtRowsAffected[iResult];		
	};

	//@cmember Compares m_cRowsExecuteAffected with the correct 
	//number to verify whether or not m_cRowsGetResultAffected is right
	virtual BOOL IsExecuteRowsAffectedRight()
	{
		//Default implementation is to check the row count
		//for the first result.
		//-1 is always valid, but print a warning for it
		//since sometimes a provider should be able to
		//return more than just -1
		if (m_cRowsExecuteAffected == -1)
		{			
			odtLog << wszNegOneWarning;									
			return TRUE;

		}
		else
			return m_cRowsExecuteAffected == m_rgStmtRowsAffected[0];
	};

	BOOL IsResultProducing(ULONG iStmt)
	{
		// Don't over-run array
		if (!COMPARE(iStmt <= m_ceSQLStmts-1, TRUE))
			return FALSE;

		switch (m_rgeSQLStmts[iStmt])
		{
			// Insert stmts always return a result (count of rows affected) if there are not multiple
			// paramsets or if they are the only statement in the batch, otherwise they do not return a result.
			case EINSERT:
			case EUPDATE:
			case EDELETE:
			case ECREATE:
			case EDROP:
				return FALSE;
			case ESELECT:
			case ESELECTWAIT:
			case EEMPTYSELECT:
				return TRUE;
			default:
				//Need to add code to deal with any other enum values
				ASSERT(FALSE);
		}

		return FALSE;

	}

	//@cmember Count of results expected from execution
	DB_UPARAMS				m_cTotalResultsExpected;

	//@cmember Count of rows currently in table we are changing with our SQL stmts
	DBCOUNTITEM				m_cRowsInChgTable;
	//@cmember Interface to IMultipleResults from Execute
	IMultipleResults *		m_pIMultResults;
	//@cmember Interface to rowset from Execute 
	IRowset *				m_pIRowset;	
	//@cmember Interface to row from Execute 
	IRow *					m_pIRow;	
	//@cmember Number of rows affected by ICommand::Execute
	DBROWCOUNT				m_cRowsExecuteAffected;
	//@cmember Number of rows affected by result retrieved by 
	//IMultipleResults::GetResult
	DBROWCOUNT				m_cRowsGetResultAffected;
	//@cmember Table for all selects
	CTable *				m_pSelectTable;
	//@cmember Table for all inserts, updates and deletes
	CTable *				m_pChangeTable;
	//@cmember Place to put any HRESULTS
	HRESULT					m_hr;
	
	//@cmember CTOR
	CMultResults(IDBCreateCommand * pIDBCreateCommand,
				ICommandText * pICommandText,
				CTable * pSelectTable,
				CTable * pChangeTable);
	//@cmember DTOR
	~CMultResults();
	//@cmember Verifies that the given rowset has the correct data in it
	HRESULT	VerifySelectRowset(REFIID riid, IUnknown * pUnkRowset, BOOL fIsRowset);
	//@cmember Verifies that the output params are populated with correct data
	virtual BOOL VerifyOutParams(ULONG iResult){return FALSE;}

	//@cmember Takes m_pIMultResults and verifies all remaining results
	virtual HRESULT ProcessAllResults(ULONG cResultsAlreadyProcessed,	//Number of times called GetResults already
							REFIID  riidExecute, // riid that was passed to Execute.
							IID*  rgRiidGetResult);		//riid to pass to GetResults
						
	//@cmember verifies that m_cRowsGetResultAffected and pRowset are 
	//properly set after an error on GetResult
	BOOL	CheckErrorOutParams(IUnknown * pRowset);

	//@cmember Releases our IRowset interface if it hasn't
	// been released, and sets it to null.
	void	ReleaseIRowset()
	{ 
		SAFE_RELEASE(m_pIRowset);
		SAFE_RELEASE(m_pIRow);
	}

	//@cmember Releases our IMultipleResults interface if it hasn't
	// been released, and sets it to null.
	void	ReleaseIMultipleResults()
	{ 
		if (m_pIMultResults)
		{
			m_pIMultResults->Release();
			m_pIMultResults = NULL;
		}
	}

	// @cmember Allows non-class members to use concatenated statements created
	LPWSTR GetSQLStmt() {return m_pwszSQLStmt;}							
};

	
// CBatch ---------------------------------------------------------------
//
//  Provides multiple results generated via batched SQL
//
class CBatch : public CMultResults
{

///////////////////////////////////////////////////////////
public:
	
	//@cmember CTOR
	CBatch(IDBCreateCommand * pIDBCreateCommand,
				ICommandText * pICommandText,
				CTable * pSelectTable,
				CTable * pChangeTable)
				: CMultResults(pIDBCreateCommand,
								pICommandText,
								pSelectTable,
								pChangeTable){};

	
	//@cmember Sets and executes
	virtual HRESULT SetAndExecute(REFIID riid);

	//@cmember Init function to be called immediately after construction
	//for every object of this class type.  Function does NOT assume
	//ownership of memory arrays, it alloc's it's own and copies, thus
	//caller must free it's own arrays.
	//This function overrides CMultResults::FInit, and internally
	//calls this base class function.
	BOOL	FInit(const ULONG cSQLStmts, ESQLSTMT * rgSQLStmts, DBRESULTFLAG* rgResFlags);
	
};


// CParamSets ---------------------------------------------------------------
//
//  Provides multiple results generated via multiple parameter sets
//
class CParamSets : public CMultResults
{

////////////////////////////////////////////////////
protected:
	//@cmember Total count of rows affected by a complete execution
	DB_UPARAMS				m_cTotalRowsAffected;	
	//@cmember Param structure used on Execute
	DBPARAMS				m_dbParams;
	//@cmember Used for creating accessors on command object
	IAccessor *				m_pIAccessor;
	//@cmember Array of bindings for input parameters
	DBBINDING *				m_rgBindings;
	//@cmember Count of bindings for input parameters
	DBCOUNTITEM				m_cBindings;
	//@cmember RowSize for one set of input parameters
	DBLENGTH				m_cbRowSize;
	//@cmember Book-keeping information for each statement
	StmtInfo *			m_prgStmtInfo;
	//@cmember Map results to underlying statement
	ULONG *				m_prgResultMap;

	//@cmember Calculates and sets m_cTotalRowsAffected
	void CalcTotalRowsAffected();
	//@cmember Calculates and sets m_cTotalResultsExpected
	BOOL CalcTotalResultsExpected();

	BOOL VerifyOutParams(ULONG iResult);	

	HRESULT ComputeGetResultHR(ULONG iResult);

	ULONG CalcResultGroups();

	BOOL IsResultProducing(ULONG iStmt);

	BOOL AreGetResultOutParamsRight(ULONG iResult, IUnknown * pUnkRowset, REFIID riid, DBRESULTFLAG resFlag);

///////////////////////////////////////////////////////////
public:
	
	//@cmember Sets and executes
	virtual HRESULT SetAndExecute(REFIID riid);

	//@cmember Takes m_pIMultResults and verifies all remaining results
	virtual HRESULT ProcessAllResults(ULONG cResultsAlreadyProcessed,	//Number of times called GetResults already
							REFIID  riidExecute, // riid that was passed to Execute.
							IID*  rgRiidGetResult);		//riid to pass to GetResults
	
	//@cmember Init function to be called immediately after construction
	//for every object of this class type.  Function does NOT assume
	//ownership of memory arrays, it alloc's it's own and copies, thus
	//caller must free it's own arrays.
	//This function overrides CMultResults::FInit, and internally
	//calls this base class function.
	BOOL	FInit(const ULONG cSQLStmts, ESQLSTMT * rgSQLStmts, DBRESULTFLAG* rgResFlags, ULONG cResFlags, ULONG ulRowNum = ROW_SEED);	


	//@cmember CTOR
	CParamSets::CParamSets(ULONG cParamSets, 
							IDBCreateCommand * pIDBCreateCommand,
							ICommandText * pICommandText,
							CTable * pSelectTable,
							CTable * pChangeTable);

	//@cmember DTOR
	~CParamSets();

	//@cmember Create m_dbParams.pData  and m_dbParams.hAccessor for execution
	HRESULT BuildParamAccessorAndData(const ULONG cSQLStmts);

	BOOL SetOutParamBindType(ULONG iStmt, BOOL fHasStreamBinding);

};



// CFakeUnk ---------------------------------------------------------------
//
// Dummy implementation of IUnknown used for agregation test

class CFakeUnk : public IUnknown
{

	//Purely dummy implementation of all IUnknown methods
	STDMETHODIMP_(ULONG) AddRef(){return 0;};
	STDMETHODIMP_(ULONG) Release(){return 0;};
	STDMETHODIMP QueryInterface(REFIID, void ** pUnk)
	{	*pUnk =  this;
		return NOERROR;
	};
};

#endif 	//_IMULTRES_H_

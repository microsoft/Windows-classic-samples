//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IGetRow.cpp | This module tests the OLE DB IGetRow interface 
//

#include "MODStandard.hpp"		// Standard headers			
#include "IGetRow.h"			// IGetRow header
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x910ec7b0, 0x2b1e, 0x11d2, { 0xa9, 0x8d, 0x00, 0xc0, 0x4f, 0x94, 0xa7, 0x17} };
DECLARE_MODULE_NAME("IGetRow");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IGetRow interface test");
DECLARE_MODULE_VERSION(838086926);
// TCW_WizardVersion(2)
// TCW_Automation(False)
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
    return CommonModuleInit(pThisTestModule, IID_IGetRow);
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



////////////////////////////////////////////////////////////////////////////
//  TCIGetRow
//
////////////////////////////////////////////////////////////////////////////
class TCIGetRow : public CRowset
{
public:
	//constructors
	TCIGetRow(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIGetRow();

	//methods
	virtual BOOL		Init();
	virtual BOOL		Terminate();

	//Interface
	virtual IGetRow*	const pIGetRow();

	//IGetRow::GetRowFromHROW
	virtual BOOL		VerifyGetRowFromHROW(DBCOUNTITEM cRows, HROW* rghRows, DBCOUNTITEM iRow, BOOL fGetURL = FALSE);
	virtual BOOL		VerifyGetRowFromAllRows();

	//IGetRow::GetURLFromHROW
	virtual BOOL		VerifyGetURLFromHROW(DBCOUNTITEM cRows, HROW* rghRows, DBCOUNTITEM iRow);
	virtual BOOL		VerifyGetURLFromAllRows();

	//Row Object
	virtual BOOL		VerifyRowObject(IUnknown* pIUnkRow, DBCOUNTITEM iRow);

	//Thread Methods
	static ULONG WINAPI Thread_VerifyGetFromHROW(LPVOID pv);

private:
	//Data
	IGetRow*			m_pIGetRow;
};




////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::TCIGetRow
//
////////////////////////////////////////////////////////////////////////////
TCIGetRow::TCIGetRow(WCHAR * wstrTestCaseName)	: CRowset(wstrTestCaseName) 
{
	m_pIGetRow	=	NULL;
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::~TCIGetRow
//
////////////////////////////////////////////////////////////////////////////
TCIGetRow::~TCIGetRow()
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::Init()
{
	TBEGIN
	
	TESTC(CRowset::Init());

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);
	SetProperty(DBPROP_CANHOLDROWS);
	
	//Create the Rowset object
	TESTC_(CreateRowset(), S_OK);
	
	//Obtain the IGetRow interface
	TESTC_(QI(pIRowset(), IID_IGetRow, (void**)&m_pIGetRow),S_OK);

CLEANUP:
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::Terminate()
{
	SAFE_RELEASE(m_pIGetRow);
	return CRowset::Terminate();
}



////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::pIGetRow
//
////////////////////////////////////////////////////////////////////////////
IGetRow* const TCIGetRow::pIGetRow()
{
	ASSERT(m_pIGetRow);
	return m_pIGetRow;
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::VerifyGetRowFromHROW
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::VerifyGetRowFromHROW
(
	DBCOUNTITEM	cRows,
	HROW*	rghRows,
	DBCOUNTITEM	iRow,
	BOOL	fGetURL
)
{
	TBEGIN
	IRow* pIRow = NULL;

	//Obtain the row object.
	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		if(fGetURL)
		{
			//NOTE: The helper verifies the URL returned
			TESTC_(GetURLFromHROW(rghRows[i], NULL),S_OK);
		}
		else
		{
			TESTC_(GetRowFromHROW(NULL, rghRows[i], IID_IRow, (IUnknown**)&pIRow),S_OK);

			//Verify this row object
			TESTC(VerifyRowObject(pIRow, iRow + i));
			SAFE_RELEASE(pIRow);
		}
	}

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::VerifyGetURLFromHROW
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::VerifyGetURLFromHROW
(
	DBCOUNTITEM	cRows,
	HROW*	rghRows,
	DBCOUNTITEM	iRow
)
{
	//Delegate (so we don't have to maintain two similar functions)
	return VerifyGetRowFromHROW(cRows, rghRows, iRow, TRUE/*fGetURL*/);
}



////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::VerifyGetRowFromAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::VerifyGetRowFromAllRows()
{
	TBEGIN
	DBCOUNTITEM i,cRowsObtained = 0;
	HROW* rghRows = NULL;

	//get the number of rows in the table
	DBCOUNTITEM ulRowCount = pTable()->CountRowsOnTable();

	//Make sure the cursor is at the start
	TESTC_(RestartPosition(), S_OK);

	//loop through the rowset, retrieve one row at a time
	for(i=0; i<ulRowCount; i++)
	{
		//GetNextRow 
		TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//IGetRow::GetRowFromHROW
		TESTC(VerifyGetRowFromHROW(cRowsObtained, rghRows, i+1));

		//release the row handle
		TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::VerifyGetURLFromAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::VerifyGetURLFromAllRows()
{
	TBEGIN
	DBCOUNTITEM iRow,cRowsObtained = 0;
	HROW* rghRows = NULL;
	WCHAR** rgpwszURLs = NULL;
	
	//get the number of rows in the table
	DBCOUNTITEM ulRowCount = pTable()->CountRowsOnTable();
	SAFE_ALLOC(rgpwszURLs, WCHAR*, ulRowCount);
	memset(rgpwszURLs, 0, (size_t) (sizeof(WCHAR*)*ulRowCount));

	//loop through the rowset, retrieve one row at a time
	for(iRow=0; iRow<ulRowCount; iRow++)
	{
		//GetNextRows
		TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//IGetRow::GetURLFromHROW
		TESTC_(GetURLFromHROW(rghRows[0], &rgpwszURLs[iRow]),S_OK);
		TESTC(VerifyGetURLFromHROW(1, &rghRows[0], iRow+1));

		//Make sure this URL is not the same as any of the 
		//previously returned URLs, (ie: to provide a URL back to this preticular row
		//the URL must be unique - at least unique accross this rowset - all row different)
		for(DBCOUNTITEM iPrev=0; iPrev<iRow; iPrev++)
		{
			TESTC(rgpwszURLs[iRow] && rgpwszURLs[iPrev]);
			TESTC(wcscmp(rgpwszURLs[iRow], rgpwszURLs[iPrev])!=0);
		}

		//release the row handle
		TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	//Free all URLs...
	for(iRow=0; iRow<ulRowCount; iRow++)
		SAFE_FREE(rgpwszURLs[iRow]);
	SAFE_FREE(rgpwszURLs);

	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::Thread_VerifyGetFromHROW
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIGetRow::Thread_VerifyGetFromHROW(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIGetRow* pThis = (TCIGetRow*)THREAD_FUNC;
	ULONG		cRows	= (ULONG)THREAD_ARG1;
	HROW*		rghRows = (HROW*)THREAD_ARG2;
	ULONG		iRow	= (ULONG)THREAD_ARG3;
	BOOL		fGetURL	= (ULONG)THREAD_ARG4;
	ASSERT(pThis && cRows && rghRows && iRow);

	ThreadSwitch(); //Let the other thread(s) catch up

	//IGetRow::GetRowFromHROW
	TESTC(pThis->VerifyGetRowFromHROW(cRows, rghRows, iRow, fGetURL));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIGetRow::VerifyRowObject
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIGetRow::VerifyRowObject(IUnknown* pIUnkRow, DBCOUNTITEM iRow)
{
	TBEGIN
	CRowObject RowObject;

	//Create our helper object
	TESTC_(RowObject.SetRowObject(pIUnkRow),S_OK);

	//Verify the data associated with the row object...
	if(!RowObject.VerifyGetColumns(iRow, m_pTable, ALL_COLS_BOUND, BLOB_LONG))
	{
		//Data incorrect for this row!
		TERROR("Row Object Data was incorrect for row " << iRow);
		QTESTC(FALSE);
	}

CLEANUP:
	TRETURN
}

	
// {{ TCW_TEST_CASE_MAP(TCIGetRow_Unknown)
//*-----------------------------------------------------------------------
// @class IGetRow IUnknown testing
//
class TCIGetRow_Unknown : public TCIGetRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIGetRow_Unknown,TCIGetRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IUnknown - QI Mandatory interfaces
	int Variation_1();
	// @cmember IUnknown - QI Optional interfaces
	int Variation_2();
	// @cmember IUnknown - QI Invalid
	int Variation_3();
	// @cmember IUnknown - AddRef Release
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIGetRow_Unknown)
#define THE_CLASS TCIGetRow_Unknown
BEG_TEST_CASE(TCIGetRow_Unknown, TCIGetRow, L"IGetRow IUnknown testing")
	TEST_VARIATION(1, 		L"IUnknown - QI Mandatory interfaces")
	TEST_VARIATION(2, 		L"IUnknown - QI Optional interfaces")
	TEST_VARIATION(3, 		L"IUnknown - QI Invalid")
	TEST_VARIATION(4, 		L"IUnknown - AddRef Release")
	TEST_VARIATION(5, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(TCIGetRow_GetRowFromHROW)
//*-----------------------------------------------------------------------
// @class IGetRow::GetRowFromHROW testing
//
class TCIGetRow_GetRowFromHROW : public TCIGetRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIGetRow_GetRowFromHROW,TCIGetRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get row object for every row
	int Variation_1();
	// @cmember General - Get row multiple times for same row
	int Variation_2();
	// @cmember General - Get row over different rows from the rowset
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Aggregation - non-IUnknown
	int Variation_6();
	// @cmember Aggregation - Agg IUnknown
	int Variation_7();
	// @cmember Aggregation - Agg Row -> GetSourceRowset
	int Variation_8();
	// @cmember Aggregation - Agg Rowset -> Row -> GetSourceRowset
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember riid - IOpenRowset with IID_IGetRow
	int Variation_11();
	// @cmember riid - Execute with IID_IGetRow
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Properties - DBPROP_IGetRow - IOpenRowset
	int Variation_14();
	// @cmember Properties - DBPROP_IGetRow - Execute
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember Properties - CANHOLDROWS = TRUE - obtain multiple rows with outstanding row objects
	int Variation_17();
	// @cmember Properties - CANHOLDROWS = FALSE - try to obtain multiple rows with outstanding row objects
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Threads - GetRowFromHROW
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Boundary - DB_E_DELETEDROW
	int Variation_22();
	// @cmember Boundary - DB_E_DELETEDROW - Bufferred Mode
	int Variation_23();
	// @cmember Empty
	int Variation_24();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIGetRow_GetRowFromHROW)
#define THE_CLASS TCIGetRow_GetRowFromHROW
BEG_TEST_CASE(TCIGetRow_GetRowFromHROW, TCIGetRow, L"IGetRow::GetRowFromHROW testing")
	TEST_VARIATION(1, 		L"General - Get row object for every row")
	TEST_VARIATION(2, 		L"General - Get row multiple times for same row")
	TEST_VARIATION(3, 		L"General - Get row over different rows from the rowset")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Aggregation - non-IUnknown")
	TEST_VARIATION(7, 		L"Aggregation - Agg IUnknown")
	TEST_VARIATION(8, 		L"Aggregation - Agg Row -> GetSourceRowset")
	TEST_VARIATION(9, 		L"Aggregation - Agg Rowset -> Row -> GetSourceRowset")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"riid - IOpenRowset with IID_IGetRow")
	TEST_VARIATION(12, 		L"riid - Execute with IID_IGetRow")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Properties - DBPROP_IGetRow - IOpenRowset")
	TEST_VARIATION(15, 		L"Properties - DBPROP_IGetRow - Execute")
	TEST_VARIATION(16, 		L"Empty")
	TEST_VARIATION(17, 		L"Properties - CANHOLDROWS = TRUE - obtain multiple rows with outstanding row objects")
	TEST_VARIATION(18, 		L"Properties - CANHOLDROWS = FALSE - try to obtain multiple rows with outstanding row objects")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Threads - GetRowFromHROW")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Boundary - DB_E_DELETEDROW")
	TEST_VARIATION(23, 		L"Boundary - DB_E_DELETEDROW - Bufferred Mode")
	TEST_VARIATION(24, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIGetRow_GetURLFromHROW)
//*-----------------------------------------------------------------------
// @class IGetRow::GetURLFomHROW testing
//
class TCIGetRow_GetURLFromHROW : public TCIGetRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIGetRow_GetURLFromHROW,TCIGetRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Obtain URL for every row in the rowset - verify all different URLs
	int Variation_1();
	// @cmember General - Obtain for the same row - verify same URL but different string pointers
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Boundary - E_INVALIDARG
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Threads - GetURLFromHROW from seperate threads
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Boundary - DB_E_DELETEDROW - Immediate mode
	int Variation_8();
	// @cmember Boundary - DB_E_DELETEDROW - Buffered mode
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIGetRow_GetURLFromHROW)
#define THE_CLASS TCIGetRow_GetURLFromHROW
BEG_TEST_CASE(TCIGetRow_GetURLFromHROW, TCIGetRow, L"IGetRow::GetURLFomHROW testing")
	TEST_VARIATION(1, 		L"General - Obtain URL for every row in the rowset - verify all different URLs")
	TEST_VARIATION(2, 		L"General - Obtain for the same row - verify same URL but different string pointers")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Boundary - E_INVALIDARG")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Threads - GetURLFromHROW from seperate threads")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Boundary - DB_E_DELETEDROW - Immediate mode")
	TEST_VARIATION(9, 		L"Boundary - DB_E_DELETEDROW - Buffered mode")
	TEST_VARIATION(10, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCTransactions)
//*-----------------------------------------------------------------------
// @class Transaction senarios
//
class TCTransactions : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransactions,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	//Helpers
	virtual BOOL VerifyTransaction(BOOL fCommit, BOOL fRetaining);


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
// {{ TCW_TESTCASE(TCTransactions)
#define THE_CLASS TCTransactions
BEG_TEST_CASE(TCTransactions, CTransaction, L"Transaction senarios")
	TEST_VARIATION(1, 		L"ABORT with fRetaining TRUE")
	TEST_VARIATION(2, 		L"ABORT with fRetaining FALSE")
	TEST_VARIATION(3, 		L"COMMIT with fRetaining TRUE")
	TEST_VARIATION(4, 		L"COMMIT with fRetaining FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIGetRow_Unknown)
	TEST_CASE(2, TCIGetRow_GetRowFromHROW)
	TEST_CASE(3, TCIGetRow_GetURLFromHROW)
	TEST_CASE(4, TCTransactions)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END




////////////////////////////////////////////////////////////////////////////
//  TCTransactionsVerifyTransaction
//
////////////////////////////////////////////////////////////////////////////
BOOL TCTransactions::VerifyTransaction(BOOL fCommit, BOOL fRetaining)
{
	HROW		hRow;
	CRowset		RowsetA;
	IGetRow*	pIGetRow = NULL;
	IRow*		pIRow = NULL;
	WCHAR*		pwszURL = NULL;
	BOOL		fPreserving = FALSE;

	//start a transaction
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, (IUnknown**)&pIGetRow, 0, NULL));
	TESTC_(RowsetA.CreateRowset(pIGetRow),S_OK);

	//Obtain a row handle - before we commit/abort the transaction
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//commit the transaction with fRetaining==TRUE
	if(fCommit)
	{
		TESTC(GetCommit(fRetaining))
		fPreserving = m_fCommitPreserve;
	}
	else
	{
		TESTC(GetAbort(fRetaining))
		fPreserving = m_fAbortPreserve;
	}
	
	if(fPreserving)
	{
		//fully functional

		//GetRowFromHROW
		TEST2C_(pIGetRow->GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
		TESTC(pIRow != NULL);
		TESTC(DefaultObjectTesting(pIRow, ROW_INTERFACE));

		//GetURLFromHROW
		TESTC_(pIGetRow->GetURLFromHROW(hRow, &pwszURL), S_OK);
		TESTC(pwszURL != NULL);
	}
	else
	{
		//zombie
		
		//GetRowFromHROW
		TESTC_(pIGetRow->GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow), E_UNEXPECTED);
		TESTC(pIRow == NULL);

		//GetURLFromHROW
		TESTC_(pIGetRow->GetURLFromHROW(hRow, &pwszURL), E_UNEXPECTED);
		TESTC(pwszURL == NULL);
	}

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszURL);
	 
	//clean up.
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);
	TRETURN
}
		



// {{ TCW_TC_PROTOTYPE(TCIGetRow_Unknown)
//*-----------------------------------------------------------------------
//| Test Case:		TCIGetRow_Unknown - IGetRow IUnknown testing
//| Created:  	8/24/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetRow_Unknown::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIGetRow::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Mandatory interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_Unknown::Variation_1()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIGetRow(), ROWSET_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Optional interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_Unknown::Variation_2()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIGetRow(), ROWSET_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_Unknown::Variation_3()
{ 
	TBEGIN
		
	//Do some invalid testing of IUnknown
	//NOTE: Our helper (QI) veries the result pointer...
	TESTC_(QI(pIGetRow(), IID_IRow),				E_NOINTERFACE);	//Row
	TESTC_(QI(pIGetRow(), IID_ICommand),			E_NOINTERFACE);	//Session
	TESTC_(QI(pIGetRow(), IID_IOpenRowset),			E_NOINTERFACE);	//Session
	TESTC_(QI(pIGetRow(), IID_IDBProperties),		E_NOINTERFACE); //DataSource
	TESTC_(QI(pIGetRow(), IID_ISourcesRowset),		E_NOINTERFACE); //Enumerator
	TESTC_(QI(pIGetRow(), IID_IBindResource),		E_NOINTERFACE); //Binder
	TESTC_(QI(pIGetRow(), IID_ISequentialStream),	E_NOINTERFACE); //Stream
	TESTC_(QI(pIGetRow(), IID_IRowPosition),		E_NOINTERFACE); //RowPosition
	TESTC_(QI(pIGetRow(), IID_IRowsetNotify),		E_NOINTERFACE); //ConnectionPoint
	TESTC_(QI(pIGetRow(), IID_IDBAsynchNotify),		E_NOINTERFACE); //ConnectionPoint
	TESTC_(QI(pIGetRow(), IID_IDataInitialize),		E_NOINTERFACE); //ServiceComponents
	TESTC_(QI(pIGetRow(), IID_IDBPromptInitialize),	E_NOINTERFACE); //DataLinks

	//Note: DefaultObjectTesting (above) covers numerous senarios, it covers
	//IID_NULL, NULL output pointer, etc...

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - AddRef Release
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_Unknown::Variation_4()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIGetRow(), ROWSET_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_Unknown::Variation_5()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIGetRow_Unknown::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIGetRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCIGetRow_GetRowFromHROW)
//*-----------------------------------------------------------------------
//| Test Case:		TCIGetRow_GetRowFromHROW - IGetRow::GetRowFromHROW testing
//| Created:  	8/24/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetRow_GetRowFromHROW::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIGetRow::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get row object for every row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_1()
{ 
	TBEGIN

	//Get the row object for every row and verify
	TESTC(VerifyGetRowFromAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get row multiple times for same row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_2()
{ 
	TBEGIN
	HROW hRow = NULL;
	IRow* pIRow = NULL;
	IColumnsInfo* pIColumnsInfo = NULL;

	//Obtain the second row
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
		
	//Get the row object for this row
	TESTC(VerifyGetRowFromHROW(ONE_ROW, &hRow, SECOND_ROW));

	//Obtain and verify this row object again
	TESTC(VerifyGetRowFromHROW(ONE_ROW, &hRow, SECOND_ROW));

	//Obtain the row object and leave open...
	TESTC_(GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow),S_OK);
	TESTC_(GetRowFromHROW(NULL, hRow, IID_IColumnsInfo, (IUnknown**)&pIColumnsInfo),S_OK);

	//Verify the row object...
	TESTC(VerifyRowObject(pIRow, SECOND_ROW));
	TESTC(VerifyRowObject(pIColumnsInfo, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get row over different rows from the rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_3()
{ 
	TBEGIN
	HROW hRow = NULL;
	IRow* pIRow = NULL;
	IColumnsInfo* pIColumnsInfo = NULL;

	//Obtain the second row
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
	TESTC_(GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow),S_OK);
	ReleaseRows(hRow);

	//Obtain the third row
	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(GetRowFromHROW(NULL, hRow, IID_IColumnsInfo, (IUnknown**)&pIColumnsInfo),S_OK);
	ReleaseRows(hRow);

	//Verify the row objects...
	TESTC(VerifyRowObject(pIRow, SECOND_ROW));
	TESTC(VerifyRowObject(pIColumnsInfo, THIRD_ROW));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_4()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_5()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_6()
{ 
	TBEGIN
	HROW hRow = NULL;
    CAggregate Aggregate(pIRowset());
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Obtain the second row
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
	
	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(GetRowFromHROW(&Aggregate, hRow, IID_IRow, (IUnknown**)&pIUnkInner), DB_E_NOAGGREGATION);
	
CLEANUP:
	ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Agg IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_7()
{ 
	TBEGIN
	HROW hRow = NULL;
    CAggregate Aggregate(pIRowset());
	IUnknown* pIUnkInner = NULL;
	HRESULT hr = S_OK;

	//Obtain the second row
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
	
	//Aggregation
	hr = GetRowFromHROW(&Aggregate, hRow, IID_IUnknown, &pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRow));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Agg Row -> GetSourceRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_8()
{ 
	TBEGIN
	HROW hRow = NULL;
    CAggregate Aggregate(pIRowset());
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;
	IGetRow* pIGetRow = INVALID(IGetRow*);
	IUnknown* pIUnkInner = NULL;

	//Obtain the second row
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
	
	//Aggregation
	hr = GetRowFromHROW(&Aggregate, hRow, IID_IUnknown, &pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRow, (IUnknown**)&pIRow));

	//GetSourceRowset (invalid interface)
	TESTC_(hr = pIRow->GetSourceRowset(IID_IAggregate, (IUnknown**)&pIGetRow, NULL),E_NOINTERFACE);
	TESTC(pIGetRow == NULL);

	//GetSourceRowset (valid interface)
	TEST2C_(hr = pIRow->GetSourceRowset(IID_IGetRow, (IUnknown**)&pIGetRow, NULL),S_OK,DB_E_NOSOURCEOBJECT);
	TESTC(DefaultObjectTesting(pIGetRow, ROWSET_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIRow);
	if(pIGetRow != INVALID(IGetRow*))
		SAFE_RELEASE(pIGetRow);
	ReleaseRows(hRow);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Agg Rowset -> Row -> GetSourceRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_9()
{ 
	TBEGIN
	HROW hRow = NULL;
    CAggregate Aggregate(pIRowset());
	IRow* pIRow = NULL;
	IGetRow* pIGetRow = NULL;
	IUnknown* pIAggregate  = NULL;
	COpenRowset OpenRowsetA;
	CRowset RowsetA;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;

	//Create a rowset that is Aggregated...
	HRESULT hr = OpenRowsetA.CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IGetRow, (IUnknown**)&pIGetRow));

	//Obtain the second row
	TESTC_(RowsetA.CreateRowset(pIGetRow),S_OK);
	TESTC_(RowsetA.GetRow(SECOND_ROW, &hRow),S_OK);
	
	//Now Create the Row (non-aggregated)
	ulRefCountBefore = Aggregate.GetRefCount();
	TEST2C_(hr = pIGetRow->GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow),S_OK,DB_S_NOROWSPECIFICCOLUMNS);
	ulRefCountAfter = Aggregate.GetRefCount();
	
	//IRow::GetSourceRow
	TEST2C_(hr = pIRow->GetSourceRowset(IID_IAggregate, (IUnknown**)&pIAggregate, NULL),S_OK,DB_E_NOSOURCEOBJECT);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pIGetRow));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRow::GetSourceRowset unable to retrieve Parent object!");
	}


CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIGetRow);
	ReleaseRows(hRow);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_10()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc riid - IOpenRowset with IID_IGetRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_11()
{ 
	TBEGIN
	IGetRow* pIGetRow = NULL;

	//Create a row object from IOpenRowset
	TESTC_(pTable()->CreateRowset(USE_OPENROWSET, IID_IGetRow, 0, NULL, (IUnknown**)&pIGetRow),S_OK);
	TESTC(DefaultObjectTesting(pIGetRow, ROWSET_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc riid - Execute with IID_IGetRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_12()
{ 
	TBEGIN
	IGetRow* pIGetRow = NULL;

	//Create a row object from IOpenRowset
	TESTC_(pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IGetRow, 0, NULL, (IUnknown**)&pIGetRow),S_OK);
	TESTC(DefaultObjectTesting(pIGetRow, ROWSET_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_13()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_IGetRow - IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_14()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//DBPROP_IGetRow
	::SetProperty(DBPROP_IGetRow, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	//Create a row object from IOpenRowset asking for IID_IUnknown and DBPROP_IGetRow.
	TESTC_(pTable()->CreateRowset(USE_OPENROWSET, IID_IUnknown, cPropSets, rgPropSets, &pIUnknown),S_OK);

	//Verify a Rowset object is returned...
	TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_IGetRow - Execute
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_15()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//DBPROP_IGetRow
	::SetProperty(DBPROP_IGetRow, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	//Create a row object from ICommand::Execute asking for IID_IUnknown and DBPROP_IGetRow
	TESTC_(pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IUnknown, cPropSets, rgPropSets, &pIUnknown),S_OK);

	//Verify a Rowset object is returned...
	TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_16()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Properties - CANHOLDROWS = TRUE - obtain multiple rows with outstanding row objects
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_17()
{ 
	TBEGIN
	DBCOUNTITEM iRow,ulRowCount = 0;
	HROW  hRow = NULL;
	IUnknown** rgpIUnk = NULL;
	IGetRow* pIGetRow = NULL;
	CRowset RowsetA;

	//Create the rowset
	RowsetA.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(),S_OK);
	TESTC_(QI(RowsetA.pIRowset(), IID_IGetRow, (void**)&pIGetRow),S_OK);

	//get the number of rows in the table
	ulRowCount = pTable()->CountRowsOnTable();
	SAFE_ALLOC(rgpIUnk, IUnknown*, ulRowCount);
	memset(rgpIUnk, 0, (size_t) (sizeof(IUnknown*)*ulRowCount));

	//RestartPosition
	TESTC_(RowsetA.RestartPosition(),S_OK);

	//loop through the rowset, retrieve one row at a time
	for(iRow=0; iRow<ulRowCount; iRow++)
	{
		//GetNextRow 
		TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
		
		//IGetRow::GetRowFromHROW
		TEST2C_(pIGetRow->GetRowFromHROW(NULL, hRow, IID_IUnknown, &rgpIUnk[iRow]),S_OK,DB_S_NOROWSPECIFICCOLUMNS);
		TESTC(VerifyRowObject(rgpIUnk[iRow], iRow+1));

		//Make sure this row object is not the same as any of the 
		//previously returned row object pointers
		for(DBCOUNTITEM iPrev=0; iPrev<iRow; iPrev++)
			TESTC(!VerifyEqualInterface(rgpIUnk[iRow], rgpIUnk[iPrev]));

		//release the row handle
		TESTC_(RowsetA.ReleaseRows(hRow),S_OK);
	}

CLEANUP:
	//Release all the row objects...
	for(iRow=0; iRow<ulRowCount; iRow++)
		SAFE_RELEASE(rgpIUnk[iRow]);
	SAFE_FREE(rgpIUnk);

	RowsetA.ReleaseRows(hRow);
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Properties - CANHOLDROWS = FALSE - try to obtain multiple rows with outstanding row objects
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_18()
{ 
	TBEGIN
	DBCOUNTITEM iRow,ulRowCount = 0;
	HROW  hRow = NULL;
	IUnknown** rgpIUnk = NULL;
	IGetRow* pIGetRow = NULL;
	CRowset RowsetA;
	HRESULT hr = S_OK;
	
	//Create the rowset
	RowsetA.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(),S_OK);
	TESTC_(QI(RowsetA.pIRowset(), IID_IGetRow, (void**)&pIGetRow),S_OK);

	//get the number of rows in the table
	ulRowCount = pTable()->CountRowsOnTable();
	SAFE_ALLOC(rgpIUnk, IUnknown*, ulRowCount);
	memset(rgpIUnk, 0, (size_t) (sizeof(IUnknown*)*ulRowCount));

	//RestartPosition
	TESTC_(RowsetA.RestartPosition(),S_OK);

	//loop through the rowset, retrieve one row at a time
	for(iRow=0; iRow<ulRowCount; iRow++)
	{
		//GetNextRow 
		//TODO: Spec issue
		//An outstanding row object may keep a refcount on the rowset row handle
		//so even though the consumer has released all refcounts the user may not
		//be able to obtain another row handle.  
		TEST2C_(hr = RowsetA.GetNextRows(&hRow),S_OK,DB_E_ROWSNOTRELEASED);
	
		//May need work if the provider cannot support obtaining more rows
		//while their is an outstanding row object...
		if(FAILED(hr))
		{
			//We should at least be able to obtain the first row (without error)
			TESTC(iRow != 0);

			//Make sure we can obtain the row once all outstanding row objects
			//are released.  To do this we will just release our list and
			//continue with the fall through verification logic
			for(DBCOUNTITEM iPrev=0; iPrev<iRow; iPrev++)
				SAFE_RELEASE(rgpIUnk[iPrev]);

			//Now we should be able to obtain another row handle
			TESTC_(hr = RowsetA.GetNextRows(&hRow),S_OK);
		}

		//IGetRow::GetRowFromHROW
		TEST2C_(pIGetRow->GetRowFromHROW(NULL, hRow, IID_IUnknown, &rgpIUnk[iRow]),S_OK,DB_S_NOROWSPECIFICCOLUMNS);
		TESTC(VerifyRowObject(rgpIUnk[iRow], iRow+1));

		//Make sure this row object is not the same as any of the 
		//previously returned row object pointers
		for(DBCOUNTITEM iPrev=0; iPrev<iRow; iPrev++)
			TESTC(!VerifyEqualInterface(rgpIUnk[iRow], rgpIUnk[iPrev]));
		
		//release the row handle
		TESTC_(RowsetA.ReleaseRows(hRow),S_OK);
	}

CLEANUP:
	//Release all the row objects...
	for(iRow=0; iRow<ulRowCount; iRow++)
		SAFE_RELEASE(rgpIUnk[iRow]);
	SAFE_FREE(rgpIUnk);

	RowsetA.ReleaseRows(hRow);
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END








// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_19()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Threads - GetRowFromHROW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_20()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	DBCOUNTITEM cRowsObtained = 0;
	HROW rghRows[FIVE_ROWS];
	HRESULT hr = S_OK;

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)FIVE_ROWS, rghRows, (void*)FIRST_ROW, FALSE/*fGetURL*/ };

	//Get the first five rows
	TEST2C_(hr = GetRow(FIRST_ROW, FIVE_ROWS, &cRowsObtained, rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);
	T1Arg.pArg1 = (void*)cRowsObtained;

	//Create Threads
	CREATE_THREADS(Thread_VerifyGetFromHROW, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_21()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_DELETEDROW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_22()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowsetChange RowsetA;
	CRowObject RowObjectA;
	IRow* pIRow = NULL;

	//Create a new rowset, (this requires IRowsetChange)
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Delete the first row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//DB_E_DELETEDROW - Now call GetRowFromHROW on a deleted row handle
	//NOTE: The helper verifies both the positive and negative output params...
	TESTC_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), DB_E_DELETEDROW);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_DELETEDROW - Bufferred Mode
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_23()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowsetUpdate RowsetA;
	CRowObject RowObjectA;
	IRow* pIRow = NULL;

	//Create a new rowset, (this requires IRowsetUpdate)
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Delete the first row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//DB_E_DELETEDROW - Now call GetRowFromHROW on a deleted row handle
	//NOTE: The helper verifies both the positive and negative output params...
	TESTC_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), DB_E_DELETEDROW);

	//Update the pending change
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);
	TESTC_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), DB_E_DELETEDROW);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetRowFromHROW::Variation_24()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIGetRow_GetRowFromHROW::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIGetRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCIGetRow_GetURLFromHROW)
//*-----------------------------------------------------------------------
//| Test Case:		TCIGetRow_GetURLFromHROW - IGetRow::GetURLFomHROW testing
//| Created:  	8/18/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIGetRow_GetURLFromHROW::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIGetRow::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Obtain URL for every row in the rowset - verify all different URLs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_1()
{ 
	TBEGIN

	//Get the row object for every row and verify
	TESTC(VerifyGetURLFromAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Obtain for the same row - verify same URL but different string pointers
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_2()
{ 
	TBEGIN
	HROW   hRow1 = NULL;
	HROW   hRow2 = NULL;
	WCHAR* pwszURL1 = NULL;
	WCHAR* pwszURL2 = NULL;
	
	//Obtain the thrid row URL
	TESTC_(GetRow(THIRD_ROW, &hRow1),S_OK);
	TESTC_(GetURLFromHROW(hRow1, &pwszURL1),S_OK);

	//Obtain another row (in between - just to mess it up)
	TESTC_(GetRow(SECOND_ROW, &hRow2),S_OK);
	TESTC_(GetURLFromHROW(hRow2, &pwszURL2),S_OK);
	TESTC(wcscmp(pwszURL1, pwszURL2)!=0);
	SAFE_FREE(pwszURL2);

	//Now Obtain the thrid row URL (again)
	TESTC_(GetRow(THIRD_ROW, &hRow2),S_OK);
	TESTC_(GetURLFromHROW(hRow2, &pwszURL2),S_OK);

	//Verify URLs are the same - but different string pointers
	TESTC(wcscmp(pwszURL1, pwszURL2)==0);
	TESTC(pwszURL1 != pwszURL2);

CLEANUP:
	SAFE_FREE(pwszURL1);
	SAFE_FREE(pwszURL2);
	ReleaseRows(hRow1);
	ReleaseRows(hRow2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_3()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_4()
{ 
	TBEGIN
	HROW   hRow = NULL;

	//Obtain the first row handle
	//NOTE: Even for this invalidarg senario we still have to send in a "good"
	//row handle since the provider is not required to valid that param (for performance
	//reasons), and may actually use it before validating the other required one...
	TESTC_(GetRow(FIFTH_ROW, &hRow),S_OK);

	//NOTE: We have to call the method directory since our helper
	//automatically supplies the output pointer if neccessary
	TESTC_(pIGetRow()->GetURLFromHROW(hRow, NULL), E_INVALIDARG);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_5()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Threads - GetURLFromHROW from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_6()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	DBCOUNTITEM cRowsObtained = 0;
	HROW rghRows[FIVE_ROWS];
	HRESULT hr = S_OK;

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)FIVE_ROWS, rghRows, (void*)FIRST_ROW, (void*)TRUE/*fGetURL*/ };

	//Get the first five rows
	TEST2C_(hr = GetRow(FIRST_ROW, FIVE_ROWS, &cRowsObtained, rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);
	T1Arg.pArg1 = (void*)cRowsObtained;

	//Create Threads
	CREATE_THREADS(Thread_VerifyGetFromHROW, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_7()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_DELETEDROW - Immediate mode
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_8()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowsetChange RowsetA;
	WCHAR* pwszURL = NULL;

	//Create a new rowset, (this requires IRowsetChange)
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Delete the first row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//DB_E_DELETEDROW - Now call GetURLFromHROW on a deleted row handle
	//NOTE: This helper takes care of validating arguments...
	TESTC_(RowsetA.GetURLFromHROW(hRow, &pwszURL), DB_E_DELETEDROW);

CLEANUP:
	ReleaseRows(hRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_DELETEDROW - Buffered mode
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_9()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowsetUpdate RowsetA;
	WCHAR* pwszURL = NULL;

	//Create a new rowset, (this requires IRowsetUpdate)
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Delete the first row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//DB_E_DELETEDROW - Now call GetURLFromHROW on a deleted row handle
	//NOTE: This helper takes care of validating arguments...
	TESTC_(RowsetA.GetURLFromHROW(hRow, &pwszURL), DB_E_DELETEDROW);

	//Update the pending change
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);
	TESTC_(RowsetA.GetURLFromHROW(hRow, &pwszURL), DB_E_DELETEDROW);

CLEANUP:
	ReleaseRows(hRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIGetRow_GetURLFromHROW::Variation_10()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END








// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIGetRow_GetURLFromHROW::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIGetRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(TCTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransactions - Transaction senarios
//| Created:  	8/24/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransactions::Init()
{ 
	if(CTransaction::Init())
	{
	   	//register interface to be tested
		if(RegisterInterface(ROWSET_INTERFACE, IID_IGetRow, 0, NULL)) 
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would have been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ABORT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_1()
{ 
	return VerifyTransaction(FALSE/*fCommit*/, TRUE/*fRetaining*/);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ABORT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_2()
{ 
	return VerifyTransaction(FALSE/*fCommit*/, FALSE/*fRetaining*/);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc COMMIT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_3()
{ 
	return VerifyTransaction(TRUE/*fCommit*/, TRUE/*fRetaining*/);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc COMMIT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_4()
{ 
	return VerifyTransaction(FALSE/*fCommit*/, FALSE/*fRetaining*/);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCTransactions::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END







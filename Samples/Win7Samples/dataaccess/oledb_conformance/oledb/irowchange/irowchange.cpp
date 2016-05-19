//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IRowChange.cpp | This module tests the OLEDB IRowChange interface 
//

#include "MODStandard.hpp"		// Standard headers			
#include "IRowChange.h"			// IRowChange header
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x7dbea520, 0x3ec1, 0x11d2, { 0xa9, 0x8f, 0x00, 0xc0, 0x4f, 0x94, 0xa7, 0x17} };
DECLARE_MODULE_NAME("IRowChange");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRowChange interface test");
DECLARE_MODULE_VERSION(838086926);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END


//////////////////////////////////////////////////////////////////////////
// Globals
//
//////////////////////////////////////////////////////////////////////////
ULONG cInterfaceIIDs = 0;
INTERFACEMAP* rgInterfaceIIDs = NULL;



//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{	
	//Obtain the Interface IIDs for the Row object
	if(GetInterfaceArray(ROW_INTERFACE, &cInterfaceIIDs, &rgInterfaceIIDs))
		return CommonModuleInit(pThisTestModule, IID_IRowChange, SIZEOF_TABLE, ROW_INTERFACE);

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
    return CommonModuleTerminate(pThisTestModule);
}	




////////////////////////////////////////////////////////////////////////////
//  TCIRowChange
//
////////////////////////////////////////////////////////////////////////////
class TCIRowChange : public CRowsetChange
{
public:
	//constructors
	TCIRowChange(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIRowChange();

	//methods
	virtual BOOL		Init();
	virtual BOOL		Terminate();

	//IRowChange
	virtual BOOL		VerifySetColumns
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							DBORDINAL				cColAccess,
							DBCOLUMNACCESS*		rgColAccess
						);

	virtual BOOL		VerifySetColumns
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							ECOLS_BOUND			eColsToBind		= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL,
							DBPART				dwPart			= DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH
						);

	virtual BOOL		VerifySetColumnsAllRows
						(
							CRowsetChange*		pCRowset,
							ECOLS_BOUND			eColsToBind		= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL,
							DBPART				dwPart			= DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH
						);

	//Thread Methods
	static ULONG WINAPI Thread_VerifySetColumns(LPVOID pv);

	//Interface
	virtual IRowChange*	const pIRowChange();

	//Data
	CRowObject*			m_pCRowObject;
	IRowChange*			m_pIRowChange;
};




////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::TCIRowChange
//
////////////////////////////////////////////////////////////////////////////
TCIRowChange::TCIRowChange(WCHAR * wstrTestCaseName)	: CRowsetChange(wstrTestCaseName) 
{
	m_pCRowObject	= NULL;
	m_pIRowChange	= NULL;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::~TCIRowChange
//
////////////////////////////////////////////////////////////////////////////
TCIRowChange::~TCIRowChange()
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowChange::Init()
{
	TBEGIN
	HROW hRow = NULL;
	
	//Create the new row object
	m_pCRowObject = new CRowObject;
	TESTC(m_pCRowObject != NULL);

	TESTC(CRowsetChange::Init());

	//Create the Rowset object
	//CANHOLDROWS is a requireed level 0 property
	TESTC_(CreateRowset(DBPROP_CANHOLDROWS), S_OK);
	
	//Obtain the First row...
	TESTC_(GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(m_pCRowObject->CreateRowObject(pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Now obtain our IRowChange interface.
	TESTC(VerifyInterface(m_pCRowObject->pIRow(), IID_IRowChange, ROW_INTERFACE, (IUnknown**)&m_pIRowChange));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowChange::Terminate()
{
	SAFE_RELEASE(m_pIRowChange);
	SAFE_DELETE(m_pCRowObject);
	return CRowsetChange::Terminate();
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::VerifySetColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowChange::VerifySetColumns
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	DBORDINAL				cColAccess,
	DBCOLUMNACCESS*		rgColAccess
)
{
	TBEGIN
	HRESULT hr = S_OK;

	//Create the Data for SetColumns
	TESTC_(hr = pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, iRow),S_OK);

	//IRowChange::SetColumns
	TESTC_(hr = pCRowObject->SetColumns(cColAccess, rgColAccess),S_OK);

	//IRow::GetColumns
	TESTC_(hr = pCRowObject->GetColumns(cColAccess, rgColAccess),S_OK);

	//Compare Data for this row object
	if(!pCRowObject->CompareColAccess(cColAccess, rgColAccess, iRow, pTable()))
	{
		//Data incorrect for this row!
		TERROR("Data was incorrect for row " << iRow);
		QTESTC(FALSE);
	}
	
	FreeColAccess(cColAccess, rgColAccess, FALSE);

CLEANUP:
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::VerifySetColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowChange::VerifySetColumns
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	ECOLS_BOUND			eColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind,
	DBPART				dwPart
)
{
	TBEGIN
	HRESULT hr = S_OK;
	
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures...
	TESTC_(hr = pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, eColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind, dwPart),S_OK);

	//Verify SetColumns
	QTESTC(VerifySetColumns(pCRowObject, iRow, cColAccess, rgColAccess));

CLEANUP:
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::VerifySetColumnsAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowChange::VerifySetColumnsAllRows
(
	CRowsetChange*		pCRowset,
	ECOLS_BOUND			eColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind,
	DBPART				dwPart
)
{
	TBEGIN
	HRESULT hr = S_OK;

	DBCOUNTITEM iRow,cRowsObtained = 0;
	HROW* rghRows = NULL;
	CRowsetChange RowsetA;
	IRowsetUpdate* pIRowsetUpdate = NULL;

	//Restart the position.
	if(pCRowset)
	{
		TEST2C_(hr = pCRowset->RestartPosition(),S_OK, DB_E_CANNOTRESTART);
	}

	//Default rowset
	//Some providers may not be able to restart due to live data stream...
	if(pCRowset == NULL || hr==DB_E_CANNOTRESTART)
	{
		pCRowset = &RowsetA;

		//May require IRowsetLocate to position on Blobs
		if(dwBlobType != NO_BLOB_COLS)
			pCRowset->SetSettableProperty(DBPROP_IRowsetLocate);
		TESTC_(pCRowset->CreateRowset(),S_OK);
	}

	//See if we are in bufferred mode
	VerifyInterface(pCRowset->pIRowset(), IID_IRowsetUpdate, ROWSET_INTERFACE, (IUnknown**)&pIRowsetUpdate);

	//loop through the rowset, retrieve one row at a time
	for(iRow=1; iRow<=pCRowset->m_ulTableRows; iRow++)	
	{
		//GetNextRow 
		CRowObject RowObjectA;
		TESTC_(pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//Create the row object from this row
		TEST2C_(RowObjectA.CreateRowObject(pCRowset->pIRowset(), rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

		//Verify Row Object
		QTESTC(VerifySetColumns(&RowObjectA, iRow, eColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind, dwPart));

		//release the row handle
		TESTC_(pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);

		//If the rowset is in buffered mode, it will require an Update, or Undo
		//before the change really takes effect.
		if(pIRowsetUpdate)
			TESTC_(pIRowsetUpdate->Update(NULL, 0, NULL, NULL, NULL, NULL),S_OK);
	}

CLEANUP:
	if(pCRowset)
		pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowsetUpdate);
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::Thread_VerifySetColumns
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIRowChange::Thread_VerifySetColumns(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIRowChange* pThis		= (TCIRowChange*)THREAD_FUNC;
	CRowObject* pCRowObject = (CRowObject*)THREAD_ARG1;
	ASSERT(pThis && pCRowObject);

	ThreadSwitch(); //Let the other thread(s) catch up

	//IRowChange::SetColumns
	QTESTC(pThis->VerifySetColumns(pCRowObject, FIRST_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowChange::pIRowChange
//
////////////////////////////////////////////////////////////////////////////
IRowChange* const TCIRowChange::pIRowChange()
{
	ASSERT(m_pIRowChange);
	return m_pIRowChange;
}


// {{ TCW_TEST_CASE_MAP(TCIRowChange_IUnknown)
//*-----------------------------------------------------------------------
// @class IRowChange IUnknown scenarios
//
class TCIRowChange_IUnknown : public TCIRowChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowChange_IUnknown,TCIRowChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IUnknown - QI Mandatory Interfaces
	int Variation_1();
	// @cmember IUnknown - QI Optional Interfaces
	int Variation_2();
	// @cmember IUnknown - AddRef / Release
	int Variation_3();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIRowChange_IUnknown)
#define THE_CLASS TCIRowChange_IUnknown
BEG_TEST_CASE(TCIRowChange_IUnknown, TCIRowChange, L"IRowChange IUnknown scenarios")
	TEST_VARIATION(1, 		L"IUnknown - QI Mandatory Interfaces")
	TEST_VARIATION(2, 		L"IUnknown - QI Optional Interfaces")
	TEST_VARIATION(3, 		L"IUnknown - AddRef / Release")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCIRowChange_SetColumns)
//*-----------------------------------------------------------------------
// @class IRowChange::SetColumns
//
class TCIRowChange_SetColumns : public TCIRowChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowChange_SetColumns,TCIRowChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember SetColumns - All columns - no BLOBs
	int Variation_1();
	// @cmember SetColumns - All Columns - BLOBs
	int Variation_2();
	// @cmember SetColumns - 0 columns - no-op
	int Variation_3();
	// @cmember SetColumns - same column bound numerous times
	int Variation_4();
	// @cmember SetColumns - BLOB Columns only
	int Variation_5();
	// @cmember SetColumns - Each Column Seperatly
	int Variation_6();
	// @cmember SetColumns - Asking for just Rowset columns
	int Variation_7();
	// @cmember SetColumns - Asking for just Extra Row Columns
	int Variation_8();
	// @cmember SetColumns - IUnknown Columns - native
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember SetColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED
	int Variation_11();
	// @cmember SetColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED
	int Variation_12();
	// @cmember SetColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK
	int Variation_13();
	// @cmember SetColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK
	int Variation_14();
	// @cmember SetColumns - Binding all columns, even read-only columns - DB_S_ERRORSOCCURRED
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED
	int Variation_17();
	// @cmember Boundary - All non-existent columns - DB_E_ERRORSOCCURRED
	int Variation_18();
	// @cmember Boundary - No Vector Columns - S_OK
	int Variation_19();
	// @cmember Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED
	int Variation_20();
	// @cmember Boundary - Only Vector Columns - S_OK
	int Variation_21();
	// @cmember Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED
	int Variation_22();
	// @cmember Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED
	int Variation_23();
	// @cmember Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Status - DBSTATUS_S_ISNULL
	int Variation_26();
	// @cmember Status - DBSTATUS_S_DEFAULT
	int Variation_27();
	// @cmember Status - DBSTATUS_S_IGNORE
	int Variation_28();
	// @cmember Empty
	int Variation_29();
	// @cmember Buffered Mode - All Columns - no BLOBs
	int Variation_30();
	// @cmember Buffered Mode - All Columns - BLOBs
	int Variation_31();
	// @cmember Buffered Mode - All Columns - Just extra columns
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember Multiple Row Object - same row
	int Variation_34();
	// @cmember Multiple Row Object - different rows
	int Variation_35();
	// @cmember Empty
	int Variation_36();
	// @cmember Threads - SetColumns seperate threads [same row object]
	int Variation_37();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCIRowChange_SetColumns)
#define THE_CLASS TCIRowChange_SetColumns
BEG_TEST_CASE(TCIRowChange_SetColumns, TCIRowChange, L"IRowChange::SetColumns")
	TEST_VARIATION(1, 		L"SetColumns - All columns - no BLOBs")
	TEST_VARIATION(2, 		L"SetColumns - All Columns - BLOBs")
	TEST_VARIATION(3, 		L"SetColumns - 0 columns - no-op")
	TEST_VARIATION(4, 		L"SetColumns - same column bound numerous times")
	TEST_VARIATION(5, 		L"SetColumns - BLOB Columns only")
	TEST_VARIATION(6, 		L"SetColumns - Each Column Seperatly")
	TEST_VARIATION(7, 		L"SetColumns - Asking for just Rowset columns")
	TEST_VARIATION(8, 		L"SetColumns - Asking for just Extra Row Columns")
	TEST_VARIATION(9, 		L"SetColumns - IUnknown Columns - native")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"SetColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(12, 		L"SetColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(13, 		L"SetColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK")
	TEST_VARIATION(14, 		L"SetColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK")
	TEST_VARIATION(15, 		L"SetColumns - Binding all columns, even read-only columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(16, 		L"Empty")
	TEST_VARIATION(17, 		L"Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(18, 		L"Boundary - All non-existent columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(19, 		L"Boundary - No Vector Columns - S_OK")
	TEST_VARIATION(20, 		L"Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(21, 		L"Boundary - Only Vector Columns - S_OK")
	TEST_VARIATION(22, 		L"Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(23, 		L"Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(24, 		L"Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"Status - DBSTATUS_S_ISNULL")
	TEST_VARIATION(27, 		L"Status - DBSTATUS_S_DEFAULT")
	TEST_VARIATION(28, 		L"Status - DBSTATUS_S_IGNORE")
	TEST_VARIATION(29, 		L"Empty")
	TEST_VARIATION(30, 		L"Buffered Mode - All Columns - no BLOBs")
	TEST_VARIATION(31, 		L"Buffered Mode - All Columns - BLOBs")
	TEST_VARIATION(32, 		L"Buffered Mode - All Columns - Just extra columns")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"Multiple Row Object - same row")
	TEST_VARIATION(35, 		L"Multiple Row Object - different rows")
	TEST_VARIATION(36, 		L"Empty")
	TEST_VARIATION(37, 		L"Threads - SetColumns seperate threads [same row object]")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCTransactions)
//*-----------------------------------------------------------------------
// @class IRowChange inside Transactions
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
BEG_TEST_CASE(TCTransactions, CTransaction, L"IRowChange inside Transactions")
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
TEST_MODULE(3, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIRowChange_IUnknown)
	TEST_CASE(2, TCIRowChange_SetColumns)
	TEST_CASE(3, TCTransactions)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END



////////////////////////////////////////////////////////////////////////////
//  TCTransactionsVerifyTransaction
//
////////////////////////////////////////////////////////////////////////////
BOOL TCTransactions::VerifyTransaction(BOOL fCommit, BOOL fRetaining)
{
	HROW				hRow;
	IGetRow*			pIGetRow = NULL;
	BOOL				fPreserving = FALSE;
	CRowset				RowsetA;
	CRowObject			RowObjectA;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIGetRow, 0, NULL));
	TESTC_(RowsetA.CreateRowset(pIGetRow),S_OK);

	//Obtain a row object - before we commit/abort the transaction
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TESTC_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK);

	//FillColAccess (before we commit/abort the transaction
	TESTC_(RowObjectA.FillColAccess(RowsetA.pTable(), RowObjectA.m_cColAccess, RowObjectA.m_rgColAccess, FIRST_ROW),S_OK);

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

		//IRow::SetColumns
		TESTC(RowObjectA.SetColumns(RowObjectA.m_cColAccess, RowObjectA.m_rgColAccess));

		//IRow::GetColumns
		TESTC_(RowObjectA.GetColumns(RowObjectA.m_cColAccess, RowObjectA.m_rgColAccess),S_OK);

		//Compare Data for this row object
		TESTC(RowObjectA.CompareColAccess(RowObjectA.m_cColAccess, RowObjectA.m_rgColAccess, FIRST_ROW, RowsetA.pTable()));
	}
	else
	{
		//zombie

		//IRow::SetColumns
		TESTC_(RowObjectA.SetColumns(RowObjectA.m_cColAccess, RowObjectA.m_rgColAccess),E_UNEXPECTED);
	}

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	 
	//clean up.
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);
	TRETURN
}


// {{ TCW_TC_PROTOTYPE(TCIRowChange_IUnknown)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowChange_IUnknown - IRowChange IUnknown scenarios
//| Created:  	8/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowChange_IUnknown::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowChange::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Mandatory Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_IUnknown::Variation_1()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIRowChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_IUnknown::Variation_2()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIRowChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - AddRef / Release
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_IUnknown::Variation_3()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pIRowChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIRowChange_IUnknown::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCIRowChange_SetColumns)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowChange_SetColumns - IRowChange::SetColumns
//| Created:  	8/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowChange_SetColumns::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowChange::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - All columns - no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_1()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, UPDATEABLE_NONINDEX_COLS_BOUND));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - All Columns - BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_2()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - 0 columns - no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_3()
{ 
	TBEGIN
	
	//SetColumns - with (0 NULL)
	TESTC(VerifySetColumnsAllRows(this, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
				NO_COLS_BY_REF,	DBTYPE_EMPTY, 0, NULL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - same column bound numerous times
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_4()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL i,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;
	DBORDINAL  cColOrds = 0;
	DBORDINAL* rgColOrds = NULL;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.
	CRowsetChange RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_OTHERUPDATEDELETE)==S_OK);

	//Get the ColumnInfo
	TESTC(VerifyInterface(pIRowChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);
	SAFE_ALLOC(rgColOrds, DBORDINAL, cColumns);

	//Loop through each column seperatly...
	for(i=0; i<cColumns; i++)
	{
		//Fill in the Col Ordinals with numerous duplicates
		cColOrds = 0;
		for(ULONG iDup=0; iDup<i; iDup++)
		{
			if(rgColumnInfo[i].dwFlags & DBCOLUMNFLAGS_WRITE)
			{
				rgColOrds[iDup] = rgColumnInfo[i].iOrdinal;
				cColOrds++;	
			}
		}
		
		//Loop through all the rows in the rowset, verify the columns...
		TESTC(VerifySetColumnsAllRows(&RowsetA, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
					NO_COLS_BY_REF,	DBTYPE_EMPTY, cColOrds, rgColOrds));
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(rgColOrds);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - BLOB Columns only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_5()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, BLOB_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Each Column Seperatly
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_6()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL iCol,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	//Get the ColumnInfo
	TESTC(VerifyInterface(pIRowChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);

	//Loop through each column seperatly...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		//Loop through all the rows in the rowset, verify the columns...
		if(rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITE)
		{
			if(!VerifySetColumnsAllRows(this, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,NO_COLS_BY_REF,	DBTYPE_EMPTY, 1, &rgColumnInfo[iCol].iOrdinal))
			{
				//Data incorrect for this column!
				TERROR("Data was incorrect for this column Ordinal " << rgColumnInfo[iCol].iOrdinal);
			}
		}
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Asking for just Rowset columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_7()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	HROW hRow = NULL;

	CRowsetChange RowsetA;
	CRowObject RowObjectA;
	TESTC_(RowsetA.CreateRowset(),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create ColAccess structures from the Rowset bindings
	TESTC_(hr = RowObjectA.BindingsToColAccess(RowsetA.m_cBindings, RowsetA.m_rgBinding, RowsetA.m_pData, &cColAccess, &rgColAccess),S_OK);
	
	//Verify SetColumns
	TESTC(VerifySetColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Asking for just Extra Row Columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_8()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	HROW hRow = NULL;

	DBORDINAL cColumns = 0;
	DBORDINAL* rgColOrdinals = NULL;

	CRowsetChange RowsetA;
	CRowObject RowObjectA;
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Obtain the just the Extra columns
	TESTC_(hr = RowObjectA.GetExtraColumnInfo(&cColumns, NULL, NULL, &rgColOrdinals),S_OK);

	//Create the ColAccess Structures for just the extra columns...
	//Including BLOB columns bound as objects...
	TESTC_(hr = RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, USE_COLS_TO_BIND_ARRAY | UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cColumns, rgColOrdinals),S_OK);
							 
	//Verify SetColumns
	TESTC(VerifySetColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_FREE(rgColOrdinals);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - IUnknown Columns - native
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_9()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, BLOB_COLS_BOUND, BLOB_IID_IUNKNOWN));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_10()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_11()
{ 
	TBEGIN
	DBORDINAL i,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	//NOTE: We are not bindings any BLOB columns, since this maybe allowed is pass a length
	//only accessor for SetData on providers that need this info for BLOBs (this is covered seperatly...)
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, 
		ECOLS_BOUND(UPDATEABLE_NONINDEX_COLS_BOUND | NONNULLABLE_COLS_BOUND), NO_BLOB_COLS, 
		FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, 0, NULL, DBPART_LENGTH|DBPART_STATUS),S_OK);

	//FillColAccess
	//NOTE: We bound only the non-nullable columns, so we don't have to worry about
	//DBSTATUS_S_ISNULL passing, since value is not looked at...
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, SECOND_ROW),S_OK);

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColAccess, rgColAccess), cColAccess ? DB_E_ERRORSOCCURRED : S_OK);

	//Verify Status'
	//Length only bindings for SetData/SetColumns are not allowed, (only for BLOBs)
	for(i=0; i<cColAccess; i++)
		TESTC(rgColAccess[i].dwStatus == DBSTATUS_E_UNAVAILABLE);

	FreeColAccess(cColAccess, rgColAccess);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_12()
{ 
	TBEGIN
	DBORDINAL i,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	//NOTE: We are not bindings any BLOB columns, since this maybe allowed is pass a length
	//only accessor for SetData on providers that need this info for BLOBs (this is covered seperatly...)
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, 
		ECOLS_BOUND(UPDATEABLE_NONINDEX_COLS_BOUND | NONNULLABLE_COLS_BOUND), NO_BLOB_COLS),S_OK);

	//FillColAccess
	//NOTE: We bound only the non-nullable columns, so we don't have to worry about
	//DBSTATUS_S_ISNULL passing, since value is not looked at...
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, SECOND_ROW),S_OK);

	//For some columns, don't bind the value
	for(i=0; i<cColAccess; i++)
	{
		//Don't bind value for every other column...
		//NOTE: We have one allocated pData that all the structures point into, so there
		//is nothing to free, we just set this pointer to NULL...
		if(i%2)
			rgColAccess[i].pData = NULL;
	}

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColAccess, rgColAccess), cColAccess>1 ? DB_S_ERRORSOCCURRED : S_OK);

	//Make sure on output, that our pData's are unchanged!
	for(i=0; i<cColAccess; i++)
	{
		if(i%2)
		{
			TESTC(rgColAccess[i].pData == NULL);
			TESTC(rgColAccess[i].dwStatus == DBSTATUS_E_UNAVAILABLE);
		}
		else
		{
			//NOTE: Some providers maybe a "all-or-nothing" provider.  So if one
			//column was unable to be set they all contain UNAVAILABLE...
			TESTC(rgColAccess[i].pData != NULL);
			TESTC(rgColAccess[i].dwStatus == DBSTATUS_S_OK || rgColAccess[i].dwStatus == DBSTATUS_E_UNAVAILABLE);
		}
	}

	FreeColAccess(cColAccess, rgColAccess);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_13()
{ 
	TBEGIN
	DBORDINAL i,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	//NOTE: Only bind the LENGTH and STATUS, and only bind the NULLABLE columns.
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, 
		ECOLS_BOUND(UPDATEABLE_NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND), NO_BLOB_COLS, 
		FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, 0, NULL, DBPART_LENGTH|DBPART_STATUS),S_OK);

	//FillColAccess
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, SECOND_ROW),S_OK);

	//Bind all columns as ISNULL (so the value is ignored)
	for(i=0; i<cColAccess; i++)
	{
		rgColAccess[i].dwStatus = DBSTATUS_S_ISNULL;
		rgColAccess[i].pData	= NULL;
	}

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColAccess, rgColAccess), S_OK);

	//Verify all the columns (come back as ISNULL)
	TESTC_(m_pCRowObject->GetColumns(cColAccess, rgColAccess), S_OK);
	for(i=0; i<cColAccess; i++)
		TESTC(rgColAccess[i].dwStatus == DBSTATUS_S_ISNULL);

	FreeColAccess(cColAccess, rgColAccess);
	
CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_14()
{ 
	TBEGIN

	//Use a new rowset, and ask for a non-forward-only cursor, so we can obtain the data multiple times.
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4); 
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Loop through all the rows in the rowset, verify the columns...
	//Only bind the LENGTH and STATUS for BLOB columns.  This should succeed as this is the only
	//allowed senario where SetData is allowed without the value bound...
	TESTC(VerifySetColumnsAllRows(&RowsetA, BLOB_COLS_BOUND, BLOB_LONG, FORWARD,
				NO_COLS_BY_REF,	DBTYPE_EMPTY, 0, NULL, DBPART_LENGTH | DBPART_STATUS));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc SetColumns - Binding all columns, even read-only columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_15()
{ 
	TBEGIN
	DBORDINAL i,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	HRESULT hr = S_OK;

	//Before we do anything set the data to be the first row's data.
	//So later when we change it to the second row we know it changed or didn't change
	TESTC(m_pCRowObject->VerifySetColumns(FIRST_ROW, pTable()));

	//Create the ColAccess Structures (for all columns - except index)...
	//NOTE: This includes any potentially readonly, or other non-updateable columns
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, ECOLS_BOUND(NONINDEX_COLS_BOUND | NOBOOKMARK_COLS_BOUND)),S_OK);

	//FillColAccess
	//NOTE: We have to tell FillInputBindings to actually binding the readonly columns
	//since it normally just puts IGNORE in those places...
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, SECOND_ROW, PRIMARY, NONINDEX_COLS_BOUND),S_OK);

	//IRowChange::SetColumns
	//NOTE: our helper verifies the hr to the status consensus...
	TEST3C_(hr = m_pCRowObject->SetColumns(cColAccess, rgColAccess), S_OK, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);

	//Verify all the columns
	for(i=0; i<cColAccess; i++)
	{
		DBCOLUMNACCESS* pColAccess = &rgColAccess[i];

		//First obtain the ColumnInfo for this column for greater verfication
		DBCOLUMNINFO dbColInfo;
		TESTC(::FindColInfo(m_pCRowObject->pIRow(), &pColAccess->columnid, 0, &dbColInfo, NULL));

		//Now verify the data and status
		switch(pColAccess->dwStatus)
		{
			case DBSTATUS_S_OK:
			case DBSTATUS_S_ISNULL:
				//Verify the Data is CHANGED (should the the second row's data)
				TESTC_(m_pCRowObject->GetColumns(1, pColAccess), S_OK);
				TESTC(m_pCRowObject->CompareColAccess(1, pColAccess, SECOND_ROW, pTable()));
				
				//Make sure the column was updateable
				TESTC(dbColInfo.dwFlags & DBCOLUMNFLAGS_WRITE || dbColInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN);
				break;

			default:
				//Save the status before overwritting it with GetColumns
				DBSTATUS dbSetColumnsStatus = pColAccess->dwStatus;
				
				//Otherwise verify the data is UNCHANGED (should the the first row's data)
				TESTC_(m_pCRowObject->GetColumns(1, pColAccess), S_OK);
				if(dbColInfo.dwFlags & (DBCOLUMNFLAGS_ISROWID|DBCOLUMNFLAGS_ISROWVER))
				{
					//For RowIDs we can't compare against original value since its altered on
					//on every setdata...
				}
				else
				{
					//Otherwise verify the data is UNCHANGED (should the the first row's data)
					if(!m_pCRowObject->CompareColAccess(1, pColAccess, FIRST_ROW, pTable()))
						TERROR("Data was incorrect for column (" << i << ")");
				}
				
				if(dbSetColumnsStatus == DBSTATUS_E_PERMISSIONDENIED)
					TESTC(!(dbColInfo.dwFlags & DBCOLUMNFLAGS_WRITE || dbColInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN));

				//The only error we should be expecting for this senario is PERMISSIONDENIED.
				//We don't have a seperate case, since if some other error is returned for some
				//reason (a bug) its more important that the data is not updated (first) then
				//verification of the status takes second place...
				TESTC(dbSetColumnsStatus == DBSTATUS_E_PERMISSIONDENIED || 
					dbSetColumnsStatus == DBSTATUS_E_UNAVAILABLE);
				break;
		};
	}

	FreeColAccess(cColAccess, rgColAccess);
	
CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_16()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_17()
{ 
	TBEGIN
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBORDINAL cColAccess2 = 0;
	DBCOLUMNACCESS* rgColAccess2 = NULL;
	void* pData = NULL;
	void* pData2 = NULL;
	DBORDINAL cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;

	//Create the ColAccess Structures (for all columns)...
	//NOTE: We do this twice, since we want to test invalid columnid within valid ids.
	//A memcpy is not sufficient, since ColAccess contains ColIds which have to be allocated,
	//as well as memory location buffers, etc.  Easier to just call twice...
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG),S_OK);
	TESTC_(m_pCRowObject->CreateColAccess(&cColAccess2, &rgColAccess2, &pData2, NULL, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG),S_OK);
	
	//Now create complete ColAccess array, containing:
	//Format = 1 Invalid, All Valid, 2 Invalid, All Valid, 1 Invalid
	cColInvalid = 1 + cColAccess + 2 + cColAccess2 + 1;
	SAFE_ALLOC(rgColInvalid, DBCOLUMNACCESS, cColInvalid);
	//Create "Invalid" ColIDs
	memset(rgColInvalid, 0, (size_t)(sizeof(DBCOLUMNACCESS) * cColInvalid));
	//Create First set of All Valid
	memcpy(&rgColInvalid[1], rgColAccess, (size_t)(sizeof(DBCOLUMNACCESS)*cColAccess));
	//Create Second set of All Valid
	memcpy(&rgColInvalid[1 + cColAccess + 2], rgColAccess2, (size_t)(sizeof(DBCOLUMNACCESS)*cColAccess2));

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess, &rgColInvalid[1], SECOND_ROW),S_OK);
	TESTC_(m_pCRowObject->FillColAccess(pTable(), cColAccess2, &rgColInvalid[1 + cColAccess + 2], SECOND_ROW),S_OK);
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid),DB_S_ERRORSOCCURRED);

	//Verify Status'
	TESTC(rgColInvalid[0].dwStatus == DBSTATUS_E_DOESNOTEXIST);
	TESTC(rgColInvalid[1 + cColAccess + 0].dwStatus == DBSTATUS_E_DOESNOTEXIST);
	TESTC(rgColInvalid[1 + cColAccess + 1].dwStatus == DBSTATUS_E_DOESNOTEXIST);
	TESTC(rgColInvalid[1 + cColAccess + 2 + cColAccess2].dwStatus == DBSTATUS_E_DOESNOTEXIST);

	//Compare First All Valid Columns
	TESTC_(m_pCRowObject->GetColumns(cColAccess, &rgColInvalid[1]),S_OK);
	TESTC(m_pCRowObject->CompareColAccess(cColAccess, &rgColInvalid[1], SECOND_ROW, pTable()));

	//Compare Second All Valid Columns
	TESTC_(m_pCRowObject->GetColumns(cColAccess2, &rgColInvalid[1 + cColAccess + 2]),S_OK);
	TESTC(m_pCRowObject->CompareColAccess(cColAccess2, &rgColInvalid[1 + cColAccess + 2], SECOND_ROW, pTable()));

	FreeColAccess(cColAccess, rgColAccess);
	FreeColAccess(cColAccess2, rgColAccess2);

CLEANUP:
	SAFE_FREE(pData);
	SAFE_FREE(pData2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - All non-existent columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_18()
{ 
	TBEGIN
	DBORDINAL i,cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	TESTC_(m_pCRowObject->CreateColAccess(&cColInvalid, &rgColInvalid, &pData, NULL, ALL_COLS_BOUND, BLOB_LONG),S_OK);
	
	//Now replace the Array with (unique/nonexistent) columns
	for(i=0; i<cColInvalid; i++)
	{
		//Create a unique ColID name
		TESTC_(CreateUniqueDBID(&rgColInvalid[i].columnid),S_OK);
	}

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid),DB_E_ERRORSOCCURRED);

	//Verify Status'
	for(i=0; i<cColInvalid; i++)
		TESTC(rgColInvalid[i].dwStatus == DBSTATUS_E_DOESNOTEXIST);

	FreeColAccess(cColInvalid, rgColInvalid);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Vector Columns - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_19()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, ECOLS_BOUND(NOVECTOR_COLS_BOUND | UPDATEABLE_NONINDEX_COLS_BOUND)));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_20()
{ 
	TBEGIN
	DBORDINAL i,cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	TESTC_(m_pCRowObject->CreateColAccess(&cColInvalid, &rgColInvalid, &pData, NULL, NOVECTOR_COLS_BOUND, BLOB_LONG),S_OK);
	
	//Now replace the Array with (unique/nonexistent) columns
	for(i=0; i<cColInvalid; i++)
	{
		//Create a unique ColID name
		TESTC_(CreateUniqueDBID(&rgColInvalid[i].columnid),S_OK);
	}

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid),DB_E_ERRORSOCCURRED);

	//Verify Status'
	for(i=0; i<cColInvalid; i++)
		TESTC(rgColInvalid[i].dwStatus == DBSTATUS_E_DOESNOTEXIST);

	FreeColAccess(cColInvalid, rgColInvalid);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Only Vector Columns - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_21()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, ECOLS_BOUND(VECTOR_COLS_BOUND | UPDATEABLE_NONINDEX_COLS_BOUND)));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_22()
{ 
	TBEGIN
	DBORDINAL i,cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures (for all columns)...
	TESTC_(m_pCRowObject->CreateColAccess(&cColInvalid, &rgColInvalid, &pData, NULL, VECTOR_COLS_BOUND),S_OK);
	
	//Now replace the Array with (unique/nonexistent) columns
	for(i=0; i<cColInvalid; i++)
	{
		//Create a unique ColID name
		TESTC_(CreateUniqueDBID(&rgColInvalid[i].columnid),S_OK);
	}

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid), cColInvalid ? DB_E_ERRORSOCCURRED : S_OK);

	//Verify Status'
	for(i=0; i<cColInvalid; i++)
		TESTC(rgColInvalid[i].dwStatus == DBSTATUS_E_DOESNOTEXIST);

	FreeColAccess(cColInvalid, rgColInvalid);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_23()
{ 
	TBEGIN
	DBORDINAL i,cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;
	void* pData = NULL;
	ULONG cNonVectors = 0;
	HRESULT hrExpected = S_OK;

	//Create the ColAccess Structures (for all columns)...
	TESTC_(m_pCRowObject->CreateColAccess(&cColInvalid, &rgColInvalid, &pData, NULL, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG),S_OK);
	
	//Now for all non-vectors, create a new invalid colid
	for(i=0; i<cColInvalid; i++)
	{
		//Create a unique ColID name (for all non-vectors)
		if(rgColInvalid[i].wType & DBTYPE_VECTOR)
		{
			TESTC_(m_pCRowObject->FillColAccess(pTable(), 1, &rgColInvalid[i], THIRD_ROW),S_OK);
		}
		else
		{
			TESTC_(CreateUniqueDBID(&rgColInvalid[i].columnid),S_OK);
			cNonVectors++;
		}
	}

	if(cColInvalid)
		hrExpected = (cColInvalid == cNonVectors) ? DB_E_ERRORSOCCURRED : DB_S_ERRORSOCCURRED;

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid), hrExpected);

	//Verify Status'
	for(i=0; i<cColInvalid; i++)
	{
		if(rgColInvalid[i].wType & DBTYPE_VECTOR)
		{
			TESTC(rgColInvalid[i].dwStatus == DBSTATUS_S_OK || rgColInvalid[i].dwStatus == DBSTATUS_S_ISNULL);
			TESTC_(m_pCRowObject->GetColumns(1, &rgColInvalid[i]),S_OK);
			TESTC(m_pCRowObject->CompareColAccess(1, &rgColInvalid[i], THIRD_ROW, pTable()));
		}
		else
		{
			//Only non-vectors where changed to non-existent columnids
			TESTC(rgColInvalid[i].dwStatus == DBSTATUS_E_DOESNOTEXIST);
		}
	}

	FreeColAccess(cColInvalid, rgColInvalid);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
}
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_24()
{ 
	TBEGIN
	DBORDINAL i,cColInvalid = 0;
	DBCOLUMNACCESS* rgColInvalid = NULL;
	void* pData = NULL;
	ULONG cVectors = 0;
	HRESULT hrExpected = S_OK;

	//Create the ColAccess Structures (for all columns)...
	TESTC_(m_pCRowObject->CreateColAccess(&cColInvalid, &rgColInvalid, &pData, NULL, UPDATEABLE_NONINDEX_COLS_BOUND, NO_BLOB_COLS),S_OK);
	
	//Now for all non-vectors, create a new invalid colid
	for(i=0; i<cColInvalid; i++)
	{
		//Create a unique ColID name (for all non-vectors)
		if(rgColInvalid[i].wType & DBTYPE_VECTOR)
		{
			TESTC_(CreateUniqueDBID(&rgColInvalid[i].columnid),S_OK);
			cVectors++;
		}
		else
		{
			TESTC_(m_pCRowObject->FillColAccess(pTable(), 1, &rgColInvalid[i], THIRD_ROW),S_OK);
		}
	}

	if(cColInvalid)
		hrExpected = (cColInvalid == cVectors) ? DB_E_ERRORSOCCURRED : DB_S_ERRORSOCCURRED;

	//IRowChange::SetColumns
	TESTC_(m_pCRowObject->SetColumns(cColInvalid, rgColInvalid), hrExpected);

	//Verify Status'
	for(i=0; i<cColInvalid; i++)
	{
		if(rgColInvalid[i].wType & DBTYPE_VECTOR)
		{
			//Only vectors where changed to non-existent columnids
			TESTC(rgColInvalid[i].dwStatus == DBSTATUS_E_DOESNOTEXIST);
		}
		else
		{
			TESTC(rgColInvalid[i].dwStatus == DBSTATUS_S_OK || rgColInvalid[i].dwStatus == DBSTATUS_S_ISNULL || rgColInvalid[i].dwStatus == DBSTATUS_S_IGNORE);
			TESTC_(m_pCRowObject->GetColumns(1, &rgColInvalid[i]),S_OK);
			TESTC(m_pCRowObject->CompareColAccess(1, &rgColInvalid[i], THIRD_ROW, pTable()));
		}
	}

	FreeColAccess(cColInvalid, rgColInvalid);

CLEANUP:
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_25()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Status - DBSTATUS_S_ISNULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_26()
{ 
	TBEGIN

	//ISNULL is tested basically for free.  The privlib table, will contain a NULL diagonal,
	//or if using the INI file, somewhere within the file its pretty likely there is a NULL 
	//value somewhere.  This way we can just loop through all rows in the table and insert data
	//for that row to test SetColumns with a NULL value.

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(this, UPDATEABLE_NONINDEX_COLS_BOUND));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Status - DBSTATUS_S_DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_27()
{ 
	//NOTE: This senario is already tested in the IRowsetChange test against
	//a provider that supports Row Objects it uses SetColumns under the covers so we don't
	//have to reinvent every new variation senario.

	//We could duplicate the senario just for completeness, but with the current time
	//constraints and the complexity of the verification we'll not duplciate efforts
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Status - DBSTATUS_S_IGNORE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_28()
{ 
	//NOTE: This senario is already tested in the IRowsetChange test against
	//a provider that supports Row Objects it uses SetColumns under the covers so we don't
	//have to reinvent every new variation senario.

	//We could duplicate the senario just for completeness, but with the current time
	//constraints and the complexity of the verification we'll not duplciate efforts
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_29()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Buffered Mode - All Columns - no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_30()
{ 
	TBEGIN

	//Create a buffered mode rowset
	CRowsetUpdate RowsetA;

	//Not all providers will support IRowsetUpdate
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(&RowsetA, UPDATEABLE_NONINDEX_COLS_BOUND));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Buffered Mode - All Columns - BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_31()
{ 
	TBEGIN
	
	//Create a buffered mode rowset
	CRowsetUpdate RowsetA;

	//Not all providers will support IRowsetUpdate
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifySetColumnsAllRows(&RowsetA, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Buffered Mode - All Columns - Just extra columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_32()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	DBORDINAL cColumns = 0;
	DBORDINAL* rgColOrdinals = NULL;
	HROW hRow = NULL;

	//Create a buffered mode rowset
	CRowsetUpdate RowsetA;
	CRowObject RowObjectA;

	//Not all providers will support IRowsetUpdate
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the First row...
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Obtain the just the Extra columns
	TESTC_(hr = RowObjectA.GetExtraColumnInfo(&cColumns, NULL, NULL, &rgColOrdinals),S_OK);

	//Create the ColAccess Structures for just the extra columns...
	TESTC_(hr = RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cColumns, rgColOrdinals),S_OK);

	//Verify SetColumns
	TESTC(VerifySetColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess));

CLEANUP:
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_FREE(rgColOrdinals);
	RowsetA.ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_33()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Multiple Row Object - same row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_34()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowsetChange	RowsetA;
	CRowObject RowObjectA;
	CRowObject RowObjectB;

	//Create the rowset
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain the First row...
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TESTC(VerifySetColumns(&RowObjectA, THIRD_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

	//Now verify the data as seen from a new row object over the same row
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TESTC(RowObjectB.VerifyGetColumns(THIRD_ROW, pTable(), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Multiple Row Object - different rows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_35()
{ 
	TBEGIN
	HROW rghRows[TWO_ROWS];
	CRowsetChange	RowsetA;
	CRowObject RowObjectA;
	CRowObject RowObjectB;

	//Create the rowset
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain the First row...
	TESTC_(RowsetA.GetNextRows(TWO_ROWS, rghRows),S_OK);

	//Now create the row objects.
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_TWO]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Verify SetColumns (create and set data, using the third row data into this row)
	TESTC(VerifySetColumns(&RowObjectA, THIRD_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	TESTC(VerifySetColumns(&RowObjectB, FOURTH_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

	//Now verify the data as seen from a new row object over the same row
	TESTC(RowObjectB.VerifyGetColumns(FOURTH_ROW, pTable(), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	TESTC(RowObjectA.VerifyGetColumns(THIRD_ROW, pTable(), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS, rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_36()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Threads - SetColumns seperate threads [same row object]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowChange_SetColumns::Variation_37()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowsetChange RowsetA;
	CRowObject RowObjectA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &RowObjectA };
	
	//Create Rowset object
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create Threads
	CREATE_THREADS(Thread_VerifySetColumns, &T1Arg);

	START_THREADS();
	END_THREADS();	

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
BOOL TCIRowChange_SetColumns::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransactions - IRowChange inside Transactions
//| Created:  	8/5/98
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
	return VerifyTransaction(TRUE/*fCommit*/, FALSE/*fRetaining*/);
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



//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IRUpdate.cpp | This module tests the OLEDB IRowsetUpdate interface 
//

#include "MODStandard.hpp"		// Standard headers			
#include "IRUpdate.h"			// IRowsetUpdate header
#include "msdasql.h"			// KAGPROP_QUERYBASEDUPDATES
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x0bb5a150, 0x8cce, 0x11cf, { 0xaa, 0x41, 0x00, 0xaa, 0x00, 0x3e, 0x77, 0x8a }};
DECLARE_MODULE_NAME("IRowsetUpdate");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRowsetUpdate interface test");
DECLARE_MODULE_VERSION(838070366);
// }}


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{	
    return CommonModuleInit(pThisTestModule, IID_IRowsetUpdate, FOURTEEN_ROWS);
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
//  TCIRowsetUpdate
//
////////////////////////////////////////////////////////////////////////////
class TCIRowsetUpdate : public CRowsetUpdate
{
public:
	//constructors
	TCIRowsetUpdate(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIRowsetUpdate();

	//methods
	virtual BOOL	Init();
	virtual BOOL	Terminate();

	virtual BOOL	VerifyAddRefRows
						(	
							ULONG		cRows, 
							HROW*		rghRows, 
							ULONG		ulRefCount = 0,
							DBROWSTATUS dwRowStatus = DBROWSTATUS_S_OK
						);

	virtual BOOL	VerifyReleaseRows
						(	
							ULONG		cRows, 
							HROW*		rghRows, 
							ULONG		ulRefCount = 0,
							DBROWSTATUS dwRowStatus = DBROWSTATUS_S_OK
						);

	//@mfunc: verify the reference counts for an array of RefCounts
	virtual BOOL	VerifyRowStatus
						(	
							ULONG			cRows,							//[in] cRows
							DBROWSTATUS*	rgRowStatus,					//[in] rgRowStatus
							DBROWSTATUS		dwRowStatus = DBROWSTATUS_S_OK	//[in] Expected RowStatus 
						);

};


////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::TCIRowsetUpdate
//
////////////////////////////////////////////////////////////////////////////
TCIRowsetUpdate::TCIRowsetUpdate(WCHAR * wstrTestCaseName)	: CRowsetUpdate(wstrTestCaseName) 
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::~TCIRowsetUpdate
//
////////////////////////////////////////////////////////////////////////////
TCIRowsetUpdate::~TCIRowsetUpdate()
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowsetUpdate::Init()
{
	return CRowsetUpdate::Init();
}

////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowsetUpdate::Terminate()
{
	return CRowsetUpdate::Terminate();
}

	
////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::VerifyReleaseRows
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowsetUpdate::VerifyReleaseRows
(	
	ULONG		cRows, 
	HROW*		rghRows, 
	ULONG		ulRefCount,
	DBROWSTATUS dwRowStatus
)
{
	TBEGIN
	ULONG* rgRefCounts = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	if(cRows && rghRows)
	{
		//Release the rows
		TESTC_(ReleaseRows(cRows, rghRows, &rgRefCounts, &rgRowStatus),S_OK);
		TESTC(rgRefCounts != NULL && rgRowStatus != NULL);

		//Verify the ref counts and status
		QTESTC(VerifyRefCounts(cRows, rgRefCounts, ulRefCount));
		QTESTC(VerifyRowStatus(cRows, rgRowStatus, dwRowStatus));
	}

CLEANUP:
	PROVIDER_FREE(rgRefCounts);
	PROVIDER_FREE(rgRowStatus);
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::VerifyAddRefRows
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowsetUpdate::VerifyAddRefRows
(	
	ULONG		cRows, 
	HROW*		rghRows, 
	ULONG		ulRefCount,
	DBROWSTATUS dwRowStatus
)
{	
	TBEGIN
	ULONG*	rgRefCounts = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	if(cRows && rghRows)
	{
		rgRefCounts = PROVIDER_ALLOC_(cRows, ULONG);
		rgRowStatus = PROVIDER_ALLOC_(cRows, DBROWSTATUS);
		TESTC(rgRefCounts != NULL);
		TESTC(rgRowStatus != NULL);

		//AddRef the rows
		TESTC_(pIRowset()->AddRefRows(cRows, rghRows, rgRefCounts, rgRowStatus),S_OK);

		//Verify the ref count / row status
		TESTC(VerifyRefCounts(cRows, rgRefCounts, ulRefCount));
		TESTC(VerifyRowStatus(cRows, rgRowStatus, dwRowStatus));
	}

CLEANUP:
	PROVIDER_FREE(rgRefCounts);
	PROVIDER_FREE(rgRowStatus);
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowsetUpdate::VerifyRowStatus
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowsetUpdate::VerifyRowStatus
(	
	ULONG			cRows,			//[in] cRows
	DBROWSTATUS*	rgRowStatus,	//[in] rgRowStatus
	DBROWSTATUS		dwRowStatus 	//[in] Expected RowStatus 
)	
{
	//every element in the ref count array should be the same as cExpected
	for(ULONG i=0; i<cRows; i++)
	{
		if(rgRowStatus[i] != dwRowStatus)
			return FALSE;
	}
	return TRUE;
}


// {{ TCW_TEST_CASE_MAP(TCGetOriginalRows)
//--------------------------------------------------------------------
// @class IRowsetUpdate::GetOriginalRows test case
//
class TCGetOriginalRows : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetOriginalRows,CRowsetUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - IID_IRowsetUpdate on a read-only rowset
	int Variation_1();
	// @cmember General - IID_IRowsetUpdate on a read-only rowset, created with OuterJoin
	int Variation_2();
	// @cmember General - IID_IRowsetChange implicitly set on IID_IRowsetUpdate
	int Variation_3();
	// @cmember General - IID_IRowsetUpdate is not exposed on IRowsetChange
	int Variation_4();
	// @cmember General - IID_IRowsetLocate exposed on IRowsetUpdate
	int Variation_5();
	// @cmember General - QueryI(invalid, valid
	int Variation_6();
	// @cmember General - QueryI(valid, NULL
	int Variation_7();
	// @cmember General - QueryI(IID_IRowsetUpdate, valid
	int Variation_8();
	// @cmember General - QueryI(IID_IRowsetChange, valid
	int Variation_9();
	// @cmember General - IRowsetChange->QueryI(IID_IRowsetUpdate, valid
	int Variation_10();
	// @cmember General - Verify AddRef / Release
	int Variation_11();
	// @cmember General - Verify RefCount or row handles
	int Variation_12();
	// @cmember Boundary - [NULL Accessor]
	int Variation_13();
	// @cmember Boundary - [PASSBYREF NULL Accessor]
	int Variation_14();
	// @cmember Boundary - [valid, valid, NULL]
	int Variation_15();
	// @cmember Boundary - [DB_INVALID_ROW, valid, valid]
	int Variation_16();
	// @cmember Boundary - [valid, DB_NULL_HACCESSOR, valid]
	int Variation_17();
	// @cmember Boundary - Call GetOriginalData before any other method
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Parameters - Hard deleted row
	int Variation_20();
	// @cmember Parameters - Soft Deleted row, No update
	int Variation_21();
	// @cmember Parameters - Newly inserted row, No update
	int Variation_22();
	// @cmember Parameters - Newly inserted row - NULL Accessor, No update
	int Variation_23();
	// @cmember Parameters - A modified row, No update
	int Variation_24();
	// @cmember Parameters - An inserted, modified row
	int Variation_25();
	// @cmember Parameters - A newly inserted row, update
	int Variation_26();
	// @cmember Parameters - A newly inserted row - NULL Accessor, update
	int Variation_27();
	// @cmember Parameters - A modified row, update
	int Variation_28();
	// @cmember Parameters - An inserted/modified row, update
	int Variation_29();
	// @cmember Parameters - Insert/Modify/Delete all rows
	int Variation_30();
	// @cmember Empty
	int Variation_31();
	// @cmember Empty
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember Empty
	int Variation_34();
	// @cmember Empty
	int Variation_35();
	// @cmember Empty
	int Variation_36();
	// @cmember Empty
	int Variation_37();
	// @cmember Accessor  - BLOB / Long columns - SetPos
	int Variation_38();
	// @cmember Accessor  - BLOB / Long columns - QBU
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember Parameters - DB_E_BADACCESSORTYPE
	int Variation_41();
	// @cmember Parameters - DB_E_SEC_E_PERMISSIONDENIED
	int Variation_42();
	// @cmember Parameters - DB_E_BADBINDINFO
	int Variation_43();
	// @cmember Parameters - DB_E_COLUMNUNAVAILABLE
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember Sequence - Call GetOriginalData 3 times
	int Variation_46();
	// }}
};
// {{ TCW_TESTCASE(TCGetOriginalRows)
#define THE_CLASS TCGetOriginalRows
BEG_TEST_CASE(TCGetOriginalRows, CRowsetUpdate, L"IRowsetUpdate::GetOriginalRows test case")
	TEST_VARIATION(1,		L"General - IID_IRowsetUpdate on a read-only rowset")
	TEST_VARIATION(2,		L"General - IID_IRowsetUpdate on a read-only rowset, created with OuterJoin")
	TEST_VARIATION(3,		L"General - IID_IRowsetChange implicitly set on IID_IRowsetUpdate")
	TEST_VARIATION(4,		L"General - IID_IRowsetUpdate is not exposed on IRowsetChange")
	TEST_VARIATION(5,		L"General - IID_IRowsetLocate exposed on IRowsetUpdate")
	TEST_VARIATION(6,		L"General - QueryI(invalid, valid")
	TEST_VARIATION(7,		L"General - QueryI(valid, NULL")
	TEST_VARIATION(8,		L"General - QueryI(IID_IRowsetUpdate, valid")
	TEST_VARIATION(9,		L"General - QueryI(IID_IRowsetChange, valid")
	TEST_VARIATION(10,		L"General - IRowsetChange->QueryI(IID_IRowsetUpdate, valid")
	TEST_VARIATION(11,		L"General - Verify AddRef / Release")
	TEST_VARIATION(12,		L"General - Verify RefCount or row handles")
	TEST_VARIATION(13,		L"Boundary - [NULL Accessor]")
	TEST_VARIATION(14,		L"Boundary - [PASSBYREF NULL Accessor]")
	TEST_VARIATION(15,		L"Boundary - [valid, valid, NULL]")
	TEST_VARIATION(16,		L"Boundary - [DB_INVALID_ROW, valid, valid]")
	TEST_VARIATION(17,		L"Boundary - [valid, DB_NULL_HACCESSOR, valid]")
	TEST_VARIATION(18,		L"Boundary - Call GetOriginalData before any other method")
	TEST_VARIATION(19,		L"Empty")
	TEST_VARIATION(20,		L"Parameters - Hard deleted row")
	TEST_VARIATION(21,		L"Parameters - Soft Deleted row, No update")
	TEST_VARIATION(22,		L"Parameters - Newly inserted row, No update")
	TEST_VARIATION(23,		L"Parameters - Newly inserted row - NULL Accessor, No update")
	TEST_VARIATION(24,		L"Parameters - A modified row, No update")
	TEST_VARIATION(25,		L"Parameters - An inserted, modified row")
	TEST_VARIATION(26,		L"Parameters - A newly inserted row, update")
	TEST_VARIATION(27,		L"Parameters - A newly inserted row - NULL Accessor, update")
	TEST_VARIATION(28,		L"Parameters - A modified row, update")
	TEST_VARIATION(29,		L"Parameters - An inserted/modified row, update")
	TEST_VARIATION(30,		L"Parameters - Insert/Modify/Delete all rows")
	TEST_VARIATION(31,		L"Empty")
	TEST_VARIATION(32,		L"Empty")
	TEST_VARIATION(33,		L"Empty")
	TEST_VARIATION(34,		L"Empty")
	TEST_VARIATION(35,		L"Empty")
	TEST_VARIATION(36,		L"Empty")
	TEST_VARIATION(37,		L"Empty")
	TEST_VARIATION(38,		L"Accessor  - BLOB / Long columns - SetPos")
	TEST_VARIATION(39,		L"Accessor  - BLOB / Long columns - QBU")
	TEST_VARIATION(40,		L"Empty")
	TEST_VARIATION(41,		L"Parameters - DB_E_BADACCESSORTYPE")
	TEST_VARIATION(42,		L"Parameters - DB_E_SEC_E_PERMISSIONDENIED")
	TEST_VARIATION(43,		L"Parameters - DB_E_BADBINDINFO")
	TEST_VARIATION(44,		L"Parameters - DB_E_COLUMNUNAVAILABLE")
	TEST_VARIATION(45,		L"Empty")
	TEST_VARIATION(46,		L"Sequence - Call GetOriginalData 3 times")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCGetPendingRows)
//--------------------------------------------------------------------
// @class IRowsetUpdate::GetPendingRows test case
//
class TCGetPendingRows : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetPendingRows,CRowsetUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Verify GetPendingRows alters row handles correctly
	int Variation_1();
	// @cmember Boundary - N Changes [NULL, _ALL, NULL, NULL]
	int Variation_2();
	// @cmember Boundary - N Changes [invalid, _ALL, NULL, invalid, invalid]
	int Variation_3();
	// @cmember Boundary - N Changes [NULL, _ALL, valid, NULL, NULL]
	int Variation_4();
	// @cmember Boundary - N Changes [NULL, _ALL, valid, NULL, valid]
	int Variation_5();
	// @cmember Boundary - N Changes [NULL, _ALL, valid, valid, NULL]
	int Variation_6();
	// @cmember Boundary - No Changes [NULL, _ALL, NULL, NULL, NULL]
	int Variation_7();
	// @cmember Boundary - No Changes [NULL, _ALL, valid, NULL, NULL]
	int Variation_8();
	// @cmember Boundary - No Changes [NULL, _ALL, valid, valid, NULL]
	int Variation_9();
	// @cmember Boundary - No Changes [NULL, _ALL, valid, valid, valid]
	int Variation_10();
	// @cmember Boundary - No Changes (NULL, _ALL, valid, NULL, valid
	int Variation_11();
	// @cmember Boundary - Empty Rowset [NULL, _ALL, valid, valid, valid]
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Parameters - DBROWSTATUS seperately, before any changes
	int Variation_14();
	// @cmember Parameters - DBPENDINGSTATUS_INVALIDROW
	int Variation_15();
	// @cmember Parameters - DBROWSTATUS _NEW | _CHANGED | _SOFTDELETED
	int Variation_16();
	// @cmember Parameters - DBPENDINGSTATUS_NEW, w/ no new rows
	int Variation_17();
	// @cmember Parameters - DBPENDINGSTATUS_CHANGED, w/ no changed rows
	int Variation_18();
	// @cmember Parameters - DBPENDINGSTATUS_UNCHANGED, w/ all changed rows
	int Variation_19();
	// @cmember Parameters - DBPENDINGSTATUS_DELETED, w/ no soft-deleted rows
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Parameters - _NEW | _UNCHANGED
	int Variation_22();
	// @cmember Parameters - _NEW | _SOFTDELETED
	int Variation_23();
	// @cmember Empty
	int Variation_24();
	// @cmember Parameters - Insert/Modify/Delete a single row
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Sequence - Call GetPendingRows 3 times
	int Variation_27();
	// @cmember Sequence - Modify the same row twice
	int Variation_28();
	// @cmember Sequence - Modify the same row twice, undo, GetPendingRows
	int Variation_29();
	// }}
};
// {{ TCW_TESTCASE(TCGetPendingRows)
#define THE_CLASS TCGetPendingRows
BEG_TEST_CASE(TCGetPendingRows, CRowsetUpdate, L"IRowsetUpdate::GetPendingRows test case")
	TEST_VARIATION(1,		L"General - Verify GetPendingRows alters row handles correctly")
	TEST_VARIATION(2,		L"Boundary - N Changes [NULL, _ALL, NULL, NULL]")
	TEST_VARIATION(3,		L"Boundary - N Changes [invalid, _ALL, NULL, invalid, invalid]")
	TEST_VARIATION(4,		L"Boundary - N Changes [NULL, _ALL, valid, NULL, NULL]")
	TEST_VARIATION(5,		L"Boundary - N Changes [NULL, _ALL, valid, NULL, valid]")
	TEST_VARIATION(6,		L"Boundary - N Changes [NULL, _ALL, valid, valid, NULL]")
	TEST_VARIATION(7,		L"Boundary - No Changes [NULL, _ALL, NULL, NULL, NULL]")
	TEST_VARIATION(8,		L"Boundary - No Changes [NULL, _ALL, valid, NULL, NULL]")
	TEST_VARIATION(9,		L"Boundary - No Changes [NULL, _ALL, valid, valid, NULL]")
	TEST_VARIATION(10,		L"Boundary - No Changes [NULL, _ALL, valid, valid, valid]")
	TEST_VARIATION(11,		L"Boundary - No Changes (NULL, _ALL, valid, NULL, valid")
	TEST_VARIATION(12,		L"Boundary - Empty Rowset [NULL, _ALL, valid, valid, valid]")
	TEST_VARIATION(13,		L"Empty")
	TEST_VARIATION(14,		L"Parameters - DBROWSTATUS seperately, before any changes")
	TEST_VARIATION(15,		L"Parameters - DBPENDINGSTATUS_INVALIDROW")
	TEST_VARIATION(16,		L"Parameters - DBROWSTATUS _NEW | _CHANGED | _SOFTDELETED")
	TEST_VARIATION(17,		L"Parameters - DBPENDINGSTATUS_NEW, w/ no new rows")
	TEST_VARIATION(18,		L"Parameters - DBPENDINGSTATUS_CHANGED, w/ no changed rows")
	TEST_VARIATION(19,		L"Parameters - DBPENDINGSTATUS_UNCHANGED, w/ all changed rows")
	TEST_VARIATION(20,		L"Parameters - DBPENDINGSTATUS_DELETED, w/ no soft-deleted rows")
	TEST_VARIATION(21,		L"Empty")
	TEST_VARIATION(22,		L"Parameters - _NEW | _UNCHANGED")
	TEST_VARIATION(23,		L"Parameters - _NEW | _SOFTDELETED")
	TEST_VARIATION(24,		L"Empty")
	TEST_VARIATION(25,		L"Parameters - Insert/Modify/Delete a single row")
	TEST_VARIATION(26,		L"Empty")
	TEST_VARIATION(27,		L"Sequence - Call GetPendingRows 3 times")
	TEST_VARIATION(28,		L"Sequence - Modify the same row twice")
	TEST_VARIATION(29,		L"Sequence - Modify the same row twice, undo, GetPendingRows")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCUndo)
//--------------------------------------------------------------------
// @class IRowsetUpdate::Undo test case
//
class TCUndo : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCUndo,CRowsetUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Boundary - No Changes [NULL, 0, NULL, NULL, NULL]
	int Variation_1();
	// @cmember Boundary - No Changes [NULL, N, valid, NULL, NULL]
	int Variation_2();
	// @cmember Boundary - No Changes [NULL,0,NULL, valid, valid]
	int Variation_3();
	// @cmember Boundary - No Changes [invalid, 1, NULL, valid, valid]
	int Variation_4();
	// @cmember Boundary - N Changes [NULL, 0, NULL, NULL, valid]
	int Variation_5();
	// @cmember Boundary - N Changes [NULL, 0, valid, valid, valid]
	int Variation_6();
	// @cmember Boundary - N Changes [NULL, N, valid, NULL, NULL]
	int Variation_7();
	// @cmember Boundary - N Changes [NULL, N, valid, valid, valid]
	int Variation_8();
	// @cmember Boundary - N Changes [invalid, N, NULL, valid, valid]
	int Variation_9();
	// @cmember Boundary - N Changes [invalid, N, valid, valid, NULL]
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Parameters - 3 invalid / 2 modified
	int Variation_12();
	// @cmember Parameters - 1 hard del / 1 invalid / 1 modified
	int Variation_13();
	// @cmember Parameters - 1 hard del / 1 invalid
	int Variation_14();
	// @cmember Parameters - 3 soft del / 2 orginal
	int Variation_15();
	// @cmember Parameters - 3 new rows / 1 orginal
	int Variation_16();
	// @cmember Parameters - 4 modified / 3 orginal
	int Variation_17();
	// @cmember Parameters - 3 soft del / 2 orginal, update
	int Variation_18();
	// @cmember Parameters - 3 new rows / 1 orginal, update
	int Variation_19();
	// @cmember Parameters - 4 modified / 3 orginal, update
	int Variation_20();
	// @cmember Parameters - 3 hard del / 2 modified
	int Variation_21();
	// @cmember Parameters - Insert/Modify/Delete a single row
	int Variation_22();
	// @cmember Parameters - Insert/Modify/Delete a single row, Update
	int Variation_23();
	// @cmember Parameters - Duplicate entries in the array
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Sequence - Fetch every other row, modify every even, undo all odd.
	int Variation_26();
	// @cmember Sequence - Call Undo 3 times
	int Variation_27();
	// @cmember Empty
	int Variation_28();
	// @cmember Boundary - Empty Rowset [NULL, 0 ,NULL, valid, valid]
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember Accessor  - BLOB / Long columns - SetPos
	int Variation_31();
	// @cmember Accessor  - BLOB / Long columns - QBU
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember General - Soft-Deleted/Undo verify valid row handle
	int Variation_34();
	// @cmember General - Inserted/Undo verify invalid row handle
	int Variation_35();
	// @cmember General - Modified/Undo verify valid row handle, and OriginalData
	int Variation_36();
	// @cmember General - Verify Undo alters row handle refcount correctly
	int Variation_37();
	// }}
};
// {{ TCW_TESTCASE(TCUndo)
#define THE_CLASS TCUndo
BEG_TEST_CASE(TCUndo, CRowsetUpdate, L"IRowsetUpdate::Undo test case")
	TEST_VARIATION(1,		L"Boundary - No Changes [NULL, 0, NULL, NULL, NULL]")
	TEST_VARIATION(2,		L"Boundary - No Changes [NULL, N, valid, NULL, NULL]")
	TEST_VARIATION(3,		L"Boundary - No Changes [NULL,0,NULL, valid, valid]")
	TEST_VARIATION(4,		L"Boundary - No Changes [invalid, 1, NULL, valid, valid]")
	TEST_VARIATION(5,		L"Boundary - N Changes [NULL, 0, NULL, NULL, valid]")
	TEST_VARIATION(6,		L"Boundary - N Changes [NULL, 0, valid, valid, valid]")
	TEST_VARIATION(7,		L"Boundary - N Changes [NULL, N, valid, NULL, NULL]")
	TEST_VARIATION(8,		L"Boundary - N Changes [NULL, N, valid, valid, valid]")
	TEST_VARIATION(9,		L"Boundary - N Changes [invalid, N, NULL, valid, valid]")
	TEST_VARIATION(10,		L"Boundary - N Changes [invalid, N, valid, valid, NULL]")
	TEST_VARIATION(11,		L"Empty")
	TEST_VARIATION(12,		L"Parameters - 3 invalid / 2 modified")
	TEST_VARIATION(13,		L"Parameters - 1 hard del / 1 invalid / 1 modified")
	TEST_VARIATION(14,		L"Parameters - 1 hard del / 1 invalid")
	TEST_VARIATION(15,		L"Parameters - 3 soft del / 2 orginal")
	TEST_VARIATION(16,		L"Parameters - 3 new rows / 1 orginal")
	TEST_VARIATION(17,		L"Parameters - 4 modified / 3 orginal")
	TEST_VARIATION(18,		L"Parameters - 3 soft del / 2 orginal, update")
	TEST_VARIATION(19,		L"Parameters - 3 new rows / 1 orginal, update")
	TEST_VARIATION(20,		L"Parameters - 4 modified / 3 orginal, update")
	TEST_VARIATION(21,		L"Parameters - 3 hard del / 2 modified")
	TEST_VARIATION(22,		L"Parameters - Insert/Modify/Delete a single row")
	TEST_VARIATION(23,		L"Parameters - Insert/Modify/Delete a single row, Update")
	TEST_VARIATION(24,		L"Parameters - Duplicate entries in the array")
	TEST_VARIATION(25,		L"Empty")
	TEST_VARIATION(26,		L"Sequence - Fetch every other row, modify every even, undo all odd.")
	TEST_VARIATION(27,		L"Sequence - Call Undo 3 times")
	TEST_VARIATION(28,		L"Empty")
	TEST_VARIATION(29,		L"Boundary - Empty Rowset [NULL, 0 ,NULL, valid, valid]")
	TEST_VARIATION(30,		L"Empty")
	TEST_VARIATION(31,		L"Accessor  - BLOB / Long columns - SetPos")
	TEST_VARIATION(32,		L"Accessor  - BLOB / Long columns - QBU")
	TEST_VARIATION(33,		L"Empty")
	TEST_VARIATION(34,		L"General - Soft-Deleted/Undo verify valid row handle")
	TEST_VARIATION(35,		L"General - Inserted/Undo verify invalid row handle")
	TEST_VARIATION(36,		L"General - Modified/Undo verify valid row handle, and OriginalData")
	TEST_VARIATION(37,		L"General - Verify Undo alters row handle refcount correctly")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCUpdate)
//--------------------------------------------------------------------
// @class IRowsetUpdate::Update test case
//
class TCUpdate : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCUpdate,CRowsetUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Boundary - No Changes [NULL, 0, NULL,NULL,NULL,NULL,NULL]
	int Variation_1();
	// @cmember Boundary - No Changes (NULL, 0, NULL, valid, valid, valid, valid
	int Variation_2();
	// @cmember Boundary - N Changes (NULL, N, valid, NULL, NULL, valid, valid
	int Variation_3();
	// @cmember Boundary - N Changes (invalid, N, NULL, valid, valid, valid, valid
	int Variation_4();
	// @cmember Boundary - N Changes (invalid, N, valid, valid, NULL, valid, valid
	int Variation_5();
	// @cmember Boundary - N Changes (NULL, 0, NULL, valid, valid, valid, NULL
	int Variation_6();
	// @cmember Boundary - N Changes (NULL, 0, NULL, NULL, NULL, NULL, NULL
	int Variation_7();
	// @cmember Boundary - N Changes (NULL, 0, NULL, valid, valid,valid,valid,
	int Variation_8();
	// @cmember Boundary - Empty Rowset (NULL, 0, NULL, valid, valid,valid,valid,
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Parameters - 3 hard deleted / 2 modified
	int Variation_11();
	// @cmember Parameters - 3 invalid / 2 inserted
	int Variation_12();
	// @cmember Parameters - 1 hard deleted / 1 invalid / 1 soft-del
	int Variation_13();
	// @cmember Parameters - 1 hard deleted / 1 invalid
	int Variation_14();
	// @cmember Parameters - 1 col with non-schema info
	int Variation_15();
	// @cmember Parameters - 3 col with metacolumn non-schema info
	int Variation_16();
	// @cmember Parameters - 2 inserted rows, no value specified / not nullable
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember Sequence - Update a non-updateable column
	int Variation_19();
	// @cmember Sequence - Delete a non-updateable column
	int Variation_20();
	// @cmember Sequence - Deferred only
	int Variation_21();
	// @cmember Sequence - Cache-Deferred only
	int Variation_22();
	// @cmember Sequence - Cache-Deferred / Deferred / Non-deferred col
	int Variation_23();
	// @cmember Empty
	int Variation_24();
	// @cmember Parameters - Duplicate hrow entries in the array
	int Variation_25();
	// @cmember Parameters - Fetch/Modify/Release
	int Variation_26();
	// @cmember Parameters - Fetch/Modify/don't Release
	int Variation_27();
	// @cmember Empty
	int Variation_28();
	// @cmember Accessor  - BLOB / Long columns - SetPos
	int Variation_29();
	// @cmember Accessor  - BLOB / Long columns - QBU
	int Variation_30();
	// @cmember Empty
	int Variation_31();
	// @cmember Related - Resync / GetOriginalData same data
	int Variation_32();
	// @cmember Related - ResynchRows
	int Variation_33();
	// @cmember Related - Modify / Resync 
	int Variation_34();
	// @cmember Related - Modify / Resync / Undo
	int Variation_35();
	// @cmember Empty
	int Variation_36();
	// @cmember Properties - NONE
	int Variation_37();
	// @cmember Properties - OWNINSERT
	int Variation_38();
	// @cmember Properties - OWNINSERT & CANSCROLL
	int Variation_39();
	// @cmember Properties - OWNUPDATEDELETE
	int Variation_40();
	// @cmember Properties - OWNUPDATEDELETE & CANFETCH
	int Variation_41();
	// @cmember Properties - OTHERINSERT
	int Variation_42();
	// @cmember Properties - OTHERINSERT & CANSCROLL
	int Variation_43();
	// @cmember Properties - OTHERUPDATEDELETE
	int Variation_44();
	// @cmember Properties - OTHERUPDATEDELETE & CANSCROLL
	int Variation_45();
	// @cmember Properties - OWNUPDATEDELETE & OTHERINSERT
	int Variation_46();
	// @cmember Properties - OWNUPDATEDELETE & OTHERINSERT & CANFETCH
	int Variation_47();
	// @cmember Properties - OTHERUPDATEDELETE & OWNINSERT
	int Variation_48();
	// @cmember Properties - OTHERUPDATEDELETE & OWNINSERT & CANSCROLL
	int Variation_49();
	// @cmember Properties - verify property dependencies
	int Variation_50();
	// @cmember Empty
	int Variation_51();
	// @cmember Related - Qualified Table Name
	int Variation_52();
	// @cmember Empty
	int Variation_53();
	// @cmember Properties - MAXPENDINGCHANGEROWS
	int Variation_54();
	// @cmember Properties - CANHOLDROWS
	int Variation_55();
	// @cmember Properties - ~CANHOLDROWS
	int Variation_56();
	// @cmember Properties - BOOKMARKS
	int Variation_57();
	// @cmember Empty
	int Variation_58();
	// @cmember Transactions - Insert/Commit
	int Variation_59();
	// @cmember Transactions - Insert/Commit/Update
	int Variation_60();
	// @cmember Transactions - Modify/Update/Commit
	int Variation_61();
	// @cmember Transactions - Modify/Update/Commit/Undo
	int Variation_62();
	// @cmember Transactions - Delete/Update/Abort
	int Variation_63();
	// @cmember Transactions - Delete/Update/Abort/Undo
	int Variation_64();
	// @cmember Transactions - concurrency - DB_E_CONCURRENCYVILOATION
	int Variation_65();
	// @cmember Transactions - ABORTRETAINING
	int Variation_66();
	// @cmember Transactions - ~ABORTRETAINING
	int Variation_67();
	// @cmember Transactions - COMMITRETAINING
	int Variation_68();
	// @cmember Transactions - ~COMMITRETAINING
	int Variation_69();
	// @cmember Transactions - NOTENTRANT
	int Variation_70();
	// @cmember Empty
	int Variation_71();
	// @cmember Sequence - Insert/Modify/Delete a single row
	int Variation_72();
	// @cmember Sequence - Delete all rows/Undo 1/Modify 1/Update
	int Variation_73();
	// @cmember Sequence - 1 Row rowset/Delete row/Undo/Delete/Update/Undo/Update
	int Variation_74();
	// @cmember Sequence - Insert 3 rows/Undo all/Insert 4/Modify
	int Variation_75();
	// @cmember Empty
	int Variation_76();
	// @cmember Sequence - Call Update 3 times
	int Variation_77();
	// @cmember Sequence - Call Update on seperate columns
	int Variation_78();
	// @cmember Sequence - Update multilple rows, covering all types
	int Variation_79();
	// @cmember Sequence - Multiple Table Update
	int Variation_80();
	// }}
};
// {{ TCW_TESTCASE(TCUpdate)
#define THE_CLASS TCUpdate
BEG_TEST_CASE(TCUpdate, CRowsetUpdate, L"IRowsetUpdate::Update test case")
	TEST_VARIATION(1,		L"Boundary - No Changes [NULL, 0, NULL,NULL,NULL,NULL,NULL]")
	TEST_VARIATION(2,		L"Boundary - No Changes (NULL, 0, NULL, valid, valid, valid, valid")
	TEST_VARIATION(3,		L"Boundary - N Changes (NULL, N, valid, NULL, NULL, valid, valid")
	TEST_VARIATION(4,		L"Boundary - N Changes (invalid, N, NULL, valid, valid, valid, valid")
	TEST_VARIATION(5,		L"Boundary - N Changes (invalid, N, valid, valid, NULL, valid, valid")
	TEST_VARIATION(6,		L"Boundary - N Changes (NULL, 0, NULL, valid, valid, valid, NULL")
	TEST_VARIATION(7,		L"Boundary - N Changes (NULL, 0, NULL, NULL, NULL, NULL, NULL")
	TEST_VARIATION(8,		L"Boundary - N Changes (NULL, 0, NULL, valid, valid,valid,valid,")
	TEST_VARIATION(9,		L"Boundary - Empty Rowset (NULL, 0, NULL, valid, valid,valid,valid,")
	TEST_VARIATION(10,		L"Empty")
	TEST_VARIATION(11,		L"Parameters - 3 hard deleted / 2 modified")
	TEST_VARIATION(12,		L"Parameters - 3 invalid / 2 inserted")
	TEST_VARIATION(13,		L"Parameters - 1 hard deleted / 1 invalid / 1 soft-del")
	TEST_VARIATION(14,		L"Parameters - 1 hard deleted / 1 invalid")
	TEST_VARIATION(15,		L"Parameters - 1 col with non-schema info")
	TEST_VARIATION(16,		L"Parameters - 3 col with metacolumn non-schema info")
	TEST_VARIATION(17,		L"Parameters - 2 inserted rows, no value specified / not nullable")
	TEST_VARIATION(18,		L"Empty")
	TEST_VARIATION(19,		L"Sequence - Update a non-updateable column")
	TEST_VARIATION(20,		L"Sequence - Delete a non-updateable column")
	TEST_VARIATION(21,		L"Sequence - Deferred only")
	TEST_VARIATION(22,		L"Sequence - Cache-Deferred only")
	TEST_VARIATION(23,		L"Sequence - Cache-Deferred / Deferred / Non-deferred col")
	TEST_VARIATION(24,		L"Empty")
	TEST_VARIATION(25,		L"Parameters - Duplicate hrow entries in the array")
	TEST_VARIATION(26,		L"Parameters - Fetch/Modify/Release")
	TEST_VARIATION(27,		L"Parameters - Fetch/Modify/don't Release")
	TEST_VARIATION(28,		L"Empty")
	TEST_VARIATION(29,		L"Accessor  - BLOB / Long columns - SetPos")
	TEST_VARIATION(30,		L"Accessor  - BLOB / Long columns - QBU")
	TEST_VARIATION(31,		L"Empty")
	TEST_VARIATION(32,		L"Related - Resync / GetOriginalData same data")
	TEST_VARIATION(33,		L"Related - ResynchRows")
	TEST_VARIATION(34,		L"Related - Modify / Resync ")
	TEST_VARIATION(35,		L"Related - Modify / Resync / Undo")
	TEST_VARIATION(36,		L"Empty")
	TEST_VARIATION(37,		L"Properties - NONE")
	TEST_VARIATION(38,		L"Properties - OWNINSERT")
	TEST_VARIATION(39,		L"Properties - OWNINSERT & CANSCROLL")
	TEST_VARIATION(40,		L"Properties - OWNUPDATEDELETE")
	TEST_VARIATION(41,		L"Properties - OWNUPDATEDELETE & CANFETCH")
	TEST_VARIATION(42,		L"Properties - OTHERINSERT")
	TEST_VARIATION(43,		L"Properties - OTHERINSERT & CANSCROLL")
	TEST_VARIATION(44,		L"Properties - OTHERUPDATEDELETE")
	TEST_VARIATION(45,		L"Properties - OTHERUPDATEDELETE & CANSCROLL")
	TEST_VARIATION(46,		L"Properties - OWNUPDATEDELETE & OTHERINSERT")
	TEST_VARIATION(47,		L"Properties - OWNUPDATEDELETE & OTHERINSERT & CANFETCH")
	TEST_VARIATION(48,		L"Properties - OTHERUPDATEDELETE & OWNINSERT")
	TEST_VARIATION(49,		L"Properties - OTHERUPDATEDELETE & OWNINSERT & CANSCROLL")
	TEST_VARIATION(50,		L"Properties - verify property dependencies")
	TEST_VARIATION(51,		L"Empty")
	TEST_VARIATION(52,		L"Related - Qualified Table Name")
	TEST_VARIATION(53,		L"Empty")
	TEST_VARIATION(54,		L"Properties - MAXPENDINGCHANGEROWS")
	TEST_VARIATION(55,		L"Properties - CANHOLDROWS")
	TEST_VARIATION(56,		L"Properties - ~CANHOLDROWS")
	TEST_VARIATION(57,		L"Properties - BOOKMARKS")
	TEST_VARIATION(58,		L"Empty")
	TEST_VARIATION(59,		L"Transactions - Insert/Commit")
	TEST_VARIATION(60,		L"Transactions - Insert/Commit/Update")
	TEST_VARIATION(61,		L"Transactions - Modify/Update/Commit")
	TEST_VARIATION(62,		L"Transactions - Modify/Update/Commit/Undo")
	TEST_VARIATION(63,		L"Transactions - Delete/Update/Abort")
	TEST_VARIATION(64,		L"Transactions - Delete/Update/Abort/Undo")
	TEST_VARIATION(65,		L"Transactions - concurrency - DB_E_CONCURRENCYVILOATION")
	TEST_VARIATION(66,		L"Transactions - ABORTRETAINING")
	TEST_VARIATION(67,		L"Transactions - ~ABORTRETAINING")
	TEST_VARIATION(68,		L"Transactions - COMMITRETAINING")
	TEST_VARIATION(69,		L"Transactions - ~COMMITRETAINING")
	TEST_VARIATION(70,		L"Transactions - NOTENTRANT")
	TEST_VARIATION(71,		L"Empty")
	TEST_VARIATION(72,		L"Sequence - Insert/Modify/Delete a single row")
	TEST_VARIATION(73,		L"Sequence - Delete all rows/Undo 1/Modify 1/Update")
	TEST_VARIATION(74,		L"Sequence - 1 Row rowset/Delete row/Undo/Delete/Update/Undo/Update")
	TEST_VARIATION(75,		L"Sequence - Insert 3 rows/Undo all/Insert 4/Modify")
	TEST_VARIATION(76,		L"Empty")
	TEST_VARIATION(77,		L"Sequence - Call Update 3 times")
	TEST_VARIATION(78,		L"Sequence - Call Update on seperate columns")
	TEST_VARIATION(79,		L"Sequence - Update multilple rows, covering all types")
	TEST_VARIATION(80,		L"Sequence - Multiple Table Update")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCGetRowStatus)
//--------------------------------------------------------------------
// @class IRowsetUpdate::GetRowStatus
//
class TCGetRowStatus : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRowStatus,CRowsetUpdate);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Verify GetRowStatus alters row handles RefCount correctly
	int Variation_1();
	// @cmember Boundary - No Changes [invalid, 0, NULL, valid]
	int Variation_2();
	// @cmember Boundary - No Changes [NULL, 1, valid, NULL]
	int Variation_3();
	// @cmember Boundary - No Changes [NULL, 1, NULL, valid]
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Boundary - N Changes [NULL, 0, NULL, NULL]
	int Variation_7();
	// @cmember Boundary - N Changes [NULL, N, NULL, NULL]
	int Variation_8();
	// @cmember Boundary - N Changes [NULL, N, valid, valid]
	int Variation_9();
	// @cmember Boundary - Empty Rowset [NULL, N, valid, valid]
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Empty
	int Variation_12();
	// @cmember Parameters - 3 hard deleted 2 modified
	int Variation_13();
	// @cmember Parameters - 3 invalid
	int Variation_14();
	// @cmember Parameters - 1 hard deleted 1 invalid 1 modified
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember Parameters - All types
	int Variation_17();
	// @cmember Parameters - All Unchanged
	int Variation_18();
	// @cmember Parameters - All Inserted
	int Variation_19();
	// @cmember Parameters - All Soft-Deleted
	int Variation_20();
	// @cmember Parameters - Insert/Modify
	int Variation_21();
	// @cmember Parameters - Modify Delete
	int Variation_22();
	// @cmember Parameters - Insert/Modify/Delete
	int Variation_23();
	// @cmember Empty
	int Variation_24();
	// @cmember Sequence - Insert/Delete/Modify - Update
	int Variation_25();
	// @cmember Sequence - Make all type changes - Undo half - Update
	int Variation_26();
	// @cmember Sequence - Change N rows, GetPendingRows, GetRowStatus
	int Variation_27();
	// @cmember Sequence - Call GetRowStatus 3 times
	int Variation_28();
	// }}
};
// {{ TCW_TESTCASE(TCGetRowStatus)
#define THE_CLASS TCGetRowStatus
BEG_TEST_CASE(TCGetRowStatus, CRowsetUpdate, L"IRowsetUpdate::GetRowStatus")
	TEST_VARIATION(1,		L"General - Verify GetRowStatus alters row handles RefCount correctly")
	TEST_VARIATION(2,		L"Boundary - No Changes [invalid, 0, NULL, valid]")
	TEST_VARIATION(3,		L"Boundary - No Changes [NULL, 1, valid, NULL]")
	TEST_VARIATION(4,		L"Boundary - No Changes [NULL, 1, NULL, valid]")
	TEST_VARIATION(5,		L"Empty")
	TEST_VARIATION(6,		L"Empty")
	TEST_VARIATION(7,		L"Boundary - N Changes [NULL, 0, NULL, NULL]")
	TEST_VARIATION(8,		L"Boundary - N Changes [NULL, N, NULL, NULL]")
	TEST_VARIATION(9,		L"Boundary - N Changes [NULL, N, valid, valid]")
	TEST_VARIATION(10,		L"Boundary - Empty Rowset [NULL, N, valid, valid]")
	TEST_VARIATION(11,		L"Empty")
	TEST_VARIATION(12,		L"Empty")
	TEST_VARIATION(13,		L"Parameters - 3 hard deleted 2 modified")
	TEST_VARIATION(14,		L"Parameters - 3 invalid")
	TEST_VARIATION(15,		L"Parameters - 1 hard deleted 1 invalid 1 modified")
	TEST_VARIATION(16,		L"Empty")
	TEST_VARIATION(17,		L"Parameters - All types")
	TEST_VARIATION(18,		L"Parameters - All Unchanged")
	TEST_VARIATION(19,		L"Parameters - All Inserted")
	TEST_VARIATION(20,		L"Parameters - All Soft-Deleted")
	TEST_VARIATION(21,		L"Parameters - Insert/Modify")
	TEST_VARIATION(22,		L"Parameters - Modify Delete")
	TEST_VARIATION(23,		L"Parameters - Insert/Modify/Delete")
	TEST_VARIATION(24,		L"Empty")
	TEST_VARIATION(25,		L"Sequence - Insert/Delete/Modify - Update")
	TEST_VARIATION(26,		L"Sequence - Make all type changes - Undo half - Update")
	TEST_VARIATION(27,		L"Sequence - Change N rows, GetPendingRows, GetRowStatus")
	TEST_VARIATION(28,		L"Sequence - Call GetRowStatus 3 times")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CRowsetUpdate {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CRowsetUpdate);
	// }}
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid GetOriginalData calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid GetOriginalData calls with previous error object existing
	int Variation_2();
	// @cmember Invalid GetOriginalData calls with no previous error object existing
	int Variation_3();
	// @cmember Valid GetPendingRows calls with previous error object existing.
	int Variation_4();
	// @cmember Invalid GetPendingRows calls with previous error object existing
	int Variation_5();
	// @cmember Invalid GetPendingRows calls with no previous error object existing
	int Variation_6();
	// @cmember Valid GetRowStatus calls with previous error object existing.
	int Variation_7();
	// @cmember Invalid GetRowStatus calls with previous error object existing
	int Variation_8();
	// @cmember Invalid GetRowStatus calls with no previous error object existing
	int Variation_9();
	// @cmember Valid Undo calls with previous error object existing.
	int Variation_10();
	// @cmember Invalid Undo calls with previous error object existing
	int Variation_11();
	// @cmember Invalid Undo calls with no previous error object existing
	int Variation_12();
	// @cmember Valid Update calls with previous error object existing.
	int Variation_13();
	// @cmember Invalid Update calls with previous error object existing
	int Variation_14();
	// @cmember Invalid Update calls with no previous error object existing
	int Variation_15();
	// @cmember S_FALSE GetPendingRows call with no previous error object existing
	int Variation_16();
	// @cmember DB_E_DELETEDROW GetOriginalData call with no previous error object existing
	int Variation_17();
	// @cmember DB_E_BADACCESSORTYPE GetOriginal call with no previous error object existing
	int Variation_18();
	// }}
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CRowsetUpdate, L"Extended Errors")
	TEST_VARIATION(1,		L"Valid GetOriginalData calls with previous error object existing.")
	TEST_VARIATION(2,		L"Invalid GetOriginalData calls with previous error object existing")
	TEST_VARIATION(3,		L"Invalid GetOriginalData calls with no previous error object existing")
	TEST_VARIATION(4,		L"Valid GetPendingRows calls with previous error object existing.")
	TEST_VARIATION(5,		L"Invalid GetPendingRows calls with previous error object existing")
	TEST_VARIATION(6,		L"Invalid GetPendingRows calls with no previous error object existing")
	TEST_VARIATION(7,		L"Valid GetRowStatus calls with previous error object existing.")
	TEST_VARIATION(8,		L"Invalid GetRowStatus calls with previous error object existing")
	TEST_VARIATION(9,		L"Invalid GetRowStatus calls with no previous error object existing")
	TEST_VARIATION(10,		L"Valid Undo calls with previous error object existing.")
	TEST_VARIATION(11,		L"Invalid Undo calls with previous error object existing")
	TEST_VARIATION(12,		L"Invalid Undo calls with no previous error object existing")
	TEST_VARIATION(13,		L"Valid Update calls with previous error object existing.")
	TEST_VARIATION(14,		L"Invalid Update calls with previous error object existing")
	TEST_VARIATION(15,		L"Invalid Update calls with no previous error object existing")
	TEST_VARIATION(16,		L"S_FALSE GetPendingRows call with no previous error object existing")
	TEST_VARIATION(17,		L"DB_E_DELETEDROW GetOriginalData call with no previous error object existing")
	TEST_VARIATION(18,		L"DB_E_BADACCESSORTYPE GetOriginal call with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCZombie)
//--------------------------------------------------------------------
// @class Zombie testing of IRowsetUpdate
//
class TCZombie : public CTransaction {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombie,CTransaction);
	// }}
 
	ULONG m_cPropSets;
	DBPROPSET* m_rgPropSets;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Zombie - ABORT with fRetaining == TRUE
	int Variation_1();
	// @cmember Zombie - ABORT with fRetaining == FALSE
	int Variation_2();
	// @cmember Zombie - COMMIT with fRetaining == TRUE
	int Variation_3();
	// @cmember Zombie - COMMIT with fRetaining == TRUE
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(TCZombie)
#define THE_CLASS TCZombie
BEG_TEST_CASE(TCZombie, CTransaction, L"Zombie testing of IRowsetUpdate")
	TEST_VARIATION(1,		L"Zombie - ABORT with fRetaining == TRUE")
	TEST_VARIATION(2,		L"Zombie - ABORT with fRetaining == FALSE")
	TEST_VARIATION(3,		L"Zombie - COMMIT with fRetaining == TRUE")
	TEST_VARIATION(4,		L"Zombie - COMMIT with fRetaining == TRUE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// }} END_DECLARE_TEST_CASES();

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetOriginalRows)
	TEST_CASE(2, TCGetPendingRows)
	TEST_CASE(3, TCUndo)
	TEST_CASE(4, TCUpdate)
	TEST_CASE(5, TCGetRowStatus)
	TEST_CASE(6, TCExtendedErrors)
	TEST_CASE(7, TCZombie)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(TCGetOriginalRows)
//*-----------------------------------------------------------------------
//|	Test Case:		TCGetOriginalRows - IRowsetUpdate::GetOriginalRows test case
//|	Created:			03/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetOriginalRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - IID_IRowsetUpdate on a read-only rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_1()
{
	TBEGIN
	HROW hRow = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate ReadOnlyRowset;

	//Try to ask for IID_IRowsetUpdate on a readonly rowset 
	TESTC_PROVIDER(ReadOnlyRowset.CreateRowset(SELECT_COUNT)==S_OK);

	//Now that IRowsetUpdate is allowed on a read-only rowset, verify 
	//DBPROP_COLUMNRESTICT is VARIANT_TRUE
	TESTC_PROVIDER(ReadOnlyRowset.GetProperty(DBPROP_COLUMNRESTRICT));

	//Also Verify that we can't make any changes to this rowset
	TESTC_(ReadOnlyRowset.GetRow(FIRST_ROW,&hRow),S_OK);
	TESTC_(ReadOnlyRowset.DeleteRow(hRow),S_OK);
	TESTC_(ReadOnlyRowset.UpdateRow(ONE_ROW,&hRow,NULL,NULL,&rgRowStatus),DB_E_ERRORSOCCURRED);

	//Verify not updatable
	TESTC(rgRowStatus != NULL);

	//PERMISSIONDENIED should only be returned if ROWRESTRICT is on
	if(ReadOnlyRowset.GetProperty(DBPROP_ROWRESTRICT))
	{
		TESTC(rgRowStatus[0] == DBROWSTATUS_E_PERMISSIONDENIED);
	}
	else
	{
		TESTC(rgRowStatus[0] == DBROWSTATUS_E_INTEGRITYVIOLATION);
	}


CLEANUP:
	ReadOnlyRowset.ReleaseRows(hRow);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - IID_IRowsetUpdate on a read-only rowset, created with OuterJoin
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_2()
{
	TBEGIN
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	IUnknown* pIUnknown = NULL;
	
	CRowset Rowset1;
	Rowset1.SetProperty(DBPROP_IRowsetChange);

	CRowset Rowset2;
	Rowset2.SetProperty(DBPROP_IRowsetUpdate);

	//Obtain IDBSchemaRowset interface
	HRESULT hr = QI(pISession(), IID_IDBSchemaRowset,(void**)&pIDBSchemaRowset);

	//Not all providers will support IDBSchemaRowset
	TESTC_PROVIDER(hr==S_OK);

	//Try to ask for DBPROP_IRowsetChange on a read-only schema rowset	
	TESTC_(pIDBSchemaRowset->GetRowset(NULL,DBSCHEMA_TABLES,0,NULL,IID_IRowset,Rowset1.m_cPropSets,Rowset1.m_rgPropSets,&pIUnknown),DB_E_ERRORSOCCURRED);
	TESTC_(pIDBSchemaRowset->GetRowset(NULL,DBSCHEMA_TABLES,0,NULL,IID_IRowsetChange,0,NULL,&pIUnknown),E_NOINTERFACE);
	
	//Try to ask for DBPROP_IRowsetUpdate on a read-only schema rowset	
	TESTC_(pIDBSchemaRowset->GetRowset(NULL,DBSCHEMA_TABLES,0,NULL,IID_IRowset,Rowset2.m_cPropSets,Rowset2.m_rgPropSets,&pIUnknown),DB_E_ERRORSOCCURRED);
	TESTC_(pIDBSchemaRowset->GetRowset(NULL,DBSCHEMA_TABLES,0,NULL,IID_IRowsetUpdate,0,NULL,&pIUnknown),E_NOINTERFACE);
	   
CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}                
// }}

             
// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - IID_IRowsetChange implicitly set on IID_IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_3()
{
	TBEGIN
	CRowset RowsetA;

	//Set Properties
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Just as a sanity check, make sure IID_IRowseUpdate is still set  
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_IRowsetUpdate));

	//Verify IID_IRowsetChange is implicitly set 
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_IRowsetChange));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - IID_IRowsetUpdate is not exposed on IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_4()
{
	TBEGIN
	CRowsetChange RowsetChange;
	TESTC_PROVIDER(RowsetChange.CreateRowset()==S_OK);

	//Just as a sanity check, make sure IID_IRowseChange is still set
	TESTC_PROVIDER(RowsetChange.GetProperty(DBPROP_IRowsetChange));

	//Verify IID_IRowsetUpdate is not implicitly set
	TESTC_PROVIDER(!(RowsetChange.GetProperty(DBPROP_IRowsetUpdate)));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - IID_IRowsetLocate exposed on IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_5()
{
	TBEGIN
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IRowsetLocate)==S_OK);

	//Just as a sanity check, make sure properties are still set
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_IRowsetUpdate));
	
	//Verify IID_IRowsetLocate is implicitly set
	if(SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET))
		TESTC(RowsetA.GetProperty(DBPROP_IRowsetLocate));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - QueryI(invalid, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_6()
{
	TBEGIN
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//QI for a non-rowset Interface
	TESTC_(QI(RowsetA.pIRowsetUpdate(),IID_IConnectionPoint),E_NOINTERFACE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - QueryI(valid, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_7()
{
	TBEGIN
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->QueryInterface(IID_IRowsetUpdate, NULL),E_INVALIDARG);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - QueryI(IID_IRowsetUpdate, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_8()
{
	TBEGIN
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(QI(RowsetA.pIRowsetUpdate(),IID_IRowsetUpdate),S_OK);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - QueryI(IID_IRowsetChange, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_9()
{
	TBEGIN
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(QI(RowsetA.pIRowsetUpdate(),IID_IRowsetChange),S_OK);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - IRowsetChange->QueryI(IID_IRowsetUpdate, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_10()
{
	TBEGIN
	//Create a IRowsetChange
	CRowsetChange RowsetChange; 
	TESTC_PROVIDER(RowsetChange.CreateRowset(DBPROP_IRowsetChange)==S_OK);
	
	// IRowsetChange->QI(IID_IRowsetUpdate)
	TESTC_(QI(RowsetChange.pIRowsetChange(),IID_IRowsetUpdate),E_NOINTERFACE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - Verify AddRef / Release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_11()
{
	TBEGIN
	ULONG cOrgRefCount;
		
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the current reference count
	cOrgRefCount = GetRefCount(RowsetA.pIRowsetUpdate());
	
	//Increment ref count a few times
	RowsetA.pIRowsetUpdate()->AddRef();
	RowsetA.pIRowsetUpdate()->AddRef();
	RowsetA.pIRowsetUpdate()->AddRef();

	//Release a couple of times
	RowsetA.pIRowsetUpdate()->Release();
	RowsetA.pIRowsetUpdate()->Release();
	
	//Addref 
	RowsetA.pIRowsetUpdate()->AddRef();

	//Release
	RowsetA.pIRowsetUpdate()->Release();	
	RowsetA.pIRowsetUpdate()->Release();	

	TESTC(GetRefCount(RowsetA.pIRowsetUpdate())==cOrgRefCount);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc General - Verify RefCount or row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_12()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	void* rgpOriginalData[NROWS] = {NULL,NULL,NULL};
	
	ULONG i;
	ULONG rgBeforeRefCounts[NROWS] = {0,0,0};
	ULONG rgAfterRefCounts[NROWS] = {0,0,0};
		
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Get the current ref count for all row handles, before the call
	TESTC_(RowsetA()->AddRefRows(NROWS,rghRow,rgBeforeRefCounts,NULL),S_OK);
	TESTC_(RowsetA.ReleaseRows(NROWS,rghRow),S_OK);
	
	//Call GetOriginalData on the above rows
	TESTC_(RowsetA.GetOriginalData(NROWS, rghRow, rgpOriginalData),S_OK);
	
	//Get the current ref count for all row handles, after the call
	TESTC_(RowsetA()->AddRefRows(NROWS,rghRow,rgAfterRefCounts,NULL),S_OK);
	TESTC_(RowsetA.ReleaseRows(NROWS,rghRow),S_OK);
	
	//Verify the refcount of the row handles have not increased
	for(i=0; i<NROWS; i++)
		COMPC(rgBeforeRefCounts[i],rgAfterRefCounts[i])


CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
    PROVIDER_FREE2(NROWS, rgpOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [NULL Accessor]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_13()
{
	TBEGIN
	HACCESSOR hNullAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	HROW hRow = DB_NULL_HROW;
	void* pData = INVALID(void*);

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Create a NULL Accessor
	TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hNullAccessor,NULL),S_OK);
	 
	//Obtain the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hNullAccessor,pData),S_OK);
	TESTC(pData==INVALID(void*));

	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(INVALID(HACCESSOR),NULL),DB_E_BADACCESSORHANDLE);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(DB_NULL_HACCESSOR,NULL),DB_E_BADACCESSORHANDLE);

CLEANUP:
	RowsetA.ReleaseAccessor(hNullAccessor);
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [PASSBYREF NULL Accessor]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_14()
{
	TBEGIN
	HACCESSOR hNullAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	HROW hRow = DB_NULL_HROW;
	void* pData = INVALID(void*);
	BOOL fByrefAccessorSupported = FALSE;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Create a NULL BYREF Accessor (if supported)
    if(RowsetA.GetProperty(DBPROP_BYREFACCESSORS))
	{
		TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF ,0,NULL,0,&hNullAccessor,NULL),S_OK);
	}
	//Otherwsie just a NULL Accessor is fine
	else
	{
		TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA ,0,NULL,0,&hNullAccessor,NULL),S_OK);
	}

	//Obtain the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hNullAccessor,pData),S_OK);
	TESTC(pData==INVALID(void*));

    TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(hNullAccessor,NULL),S_OK);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(hAccessor,NULL),DB_E_BADACCESSORHANDLE);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(INVALID(HACCESSOR),NULL),DB_E_BADACCESSORHANDLE);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(DB_NULL_HACCESSOR,NULL),DB_E_BADACCESSORHANDLE);
	
CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
    RowsetA.ReleaseAccessor(hNullAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [valid, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_15()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Get the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Call GetOriginalData with NULL pDdata
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,NULL),E_INVALIDARG);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [DB_INVALID_ROW, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_16()
{
	CRowsetUpdate RowsetA;  
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
  
	//Call GetOriginalData with invalid row - should fail gracefully
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(DB_NULL_HROW,RowsetA.m_hAccessor,RowsetA.m_pData),DB_E_BADROWHANDLE);
	TESTC(RowsetA.m_pData!=NULL) //provider shouldn't touch my alloced buffer;

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [valid, DB_NULL_HACCESSOR, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_17()
{
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Should be able to pass DB_NUL_HROW to release rows
	TESTC_(RowsetA.ReleaseRows(ONE_ROW,&hRow),DB_E_ERRORSOCCURRED);

	//Get the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

	//Call GetOriginalData with a NULL Accessor
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,DB_NULL_HACCESSOR,RowsetA.m_pData),DB_E_BADACCESSORHANDLE);
	TESTC(RowsetA.m_pData!=NULL) //provider shouldn't touch my alloced buffer;

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Call GetOriginalData before any other method
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_18()
{
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Get the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

	//GetOriginalData, before any other modifying methods
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK);
	TESTC(RowsetA.m_pData!=NULL) //provider shouldn't touch my alloced buffer;

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_19()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Hard deleted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_20()
{
	HROW hRow = DB_NULL_HROW;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the first row	
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

	//Hard-delete the row
	TESTC_(RowsetA.HardDeleteRow(hRow),S_OK);
	
	//Call GetOriginalData on the hard deleted row  
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),DB_E_DELETEDROW);
	TESTC(RowsetA.m_pData!=NULL) //provider shouldn't touch my alloced buffer;

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Soft Deleted row, No update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_21()
{
	void* pOriginalData = NULL;
	HROW hRow = DB_NULL_HROW;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	//Obtain the first row	
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(hRow, &pOriginalData),S_OK);

	//Soft-delete the row
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
		
	//Call GetOriginalData on the soft deleted row
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK);

	//Compare with the orginal data, should match
	TESTC(RowsetA.CompareRowData(pOriginalData,RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Newly inserted row, No update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_22()
{
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Soft-Insert the row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
		
	//Call GetOriginalData on the soft inserted row
	//This may produce DB_S_ERRORSOCCURRED since if the driver has non-NULL
	//columns, Some providers cannot return ISNULL, and has no idea what valid 
	//defaults would be, so DBSTATUS_E_UNAVAILABLE will be returned for those 
	//columns
	TEST3C_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

	//GetOriginalData should return defaults, therefore it should not match
    TESTC(!RowsetA.CompareRowData(hRow,RowsetA.m_pData));

	//Compare with defaults
	TESTC(RowsetA.CompareWithDefaults(RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//--------------------------------------------------------------------
// @mfunc Parameters - Newly inserted row - NULL Accessor, No update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_23()
{
	HROW hRow = DB_NULL_HROW;
	HACCESSOR hAccessor = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Create NULL Accessor
	TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA, 0, NULL, 0, &hAccessor, NULL),S_OK);

	//Soft-Insert the row (using NULL Accessor)
	TESTC_(RowsetA.pIRowsetUpdate()->InsertRow(NULL, hAccessor, NULL, &hRow),S_OK);

	//Call GetOriginalData on the soft inserted row
	//This may produce DB_S_ERRORSOCCURRED since if the driver has non-NULL
	//columns, Some providers cannot return ISNULL, and has no idea what valid 
	//defaults would be, so DBSTATUS_E_UNAVAILABLE will be returned for those 
	//columns
	TEST3C_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

	//Get the Data
	TEST3C_(RowsetA.GetRowData(hRow, &pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

	//Compare with defaults
	TESTC(RowsetA.CompareWithDefaults(RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - A modified row, No update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_24()
{
	void* pOriginalData = NULL;
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Save the orginal data
	TESTC_(RowsetA.GetRowData(hRow, &pOriginalData),S_OK);

	//Modify the row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
		
	//Call GetOriginalData on the modified row 
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK);

	//Compare data with orginal data, should match
	TESTC(RowsetA.CompareRowData(pOriginalData,RowsetA.m_pData));

	//Compare data with modified data, should not match
	TESTC(!RowsetA.CompareRowData(hRow,RowsetA.m_pData));


CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - An inserted, modified row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_25()
{
	void* pModifiedData = NULL;
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the first row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	
	//Modify the row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//Save the modified data
	TESTC_(RowsetA.GetRowData(hRow, &pModifiedData),S_OK);
			
	//Call GetOriginalData on the modified row 
	TEST3C_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

	//Compare data with modified data, should not match
	TESTC(!RowsetA.CompareRowData(pModifiedData,RowsetA.m_pData));

	//Compare data with defaults, should match
	TESTC(RowsetA.CompareWithDefaults(RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pModifiedData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - A newly inserted row, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_26()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	HACCESSOR hAccessor = NULL;
	void* pOriginalData = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Soft-Insert the row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	
	//Save the OriginalData
	TEST3C_(RowsetA.GetOriginalData(hRow, &pOriginalData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
	
	//Save the Data
	TESTC_(RowsetA.GetRowData(hRow, &pData),S_OK);
	
	//Update the row - (Hard Insert)
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);

	//Call GetOriginalData on the updated row 
	TESTC_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK);

	//Compare data with original data, should not match (updated)
	TESTC(!RowsetA.CompareRowData(pOriginalData, RowsetA.m_pData));

	//Compare data with row data, should match (updated)
	TESTC(RowsetA.CompareRowData(pData, RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//--------------------------------------------------------------------
// @mfunc Parameters - A newly inserted row - NULL Accessor, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_27()
{
	TBEGIN
	HRESULT hr;

	HROW hRow = DB_NULL_HROW;
	HACCESSOR hAccessor = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_SERVERDATAONINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	pData = PROVIDER_ALLOC(RowsetA.m_cRowSize*sizeof(void*));

	//Create NULL Accessor
	TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA, 0, NULL, 0, &hAccessor, NULL),S_OK);

	//Soft-Insert the row (using NULL Accessor)
	TESTC_(RowsetA.pIRowsetUpdate()->InsertRow(NULL, hAccessor, NULL, &hRow),S_OK);
	
	//This may produce DB_E_ERRORSOCCURRED since if the table has non-NULL
	//columns, the driver may fail to insert the row, since I have bound no values
	hr = RowsetA.UpdateRow(hRow);
	TESTC(hr==S_OK || hr==DB_E_ERRORSOCCURRED);
	
	//Call GetOriginalData on the soft inserted row
	//Since there really is no "OrginalData" on a newly inserted row
	//that hasn't been updated yet, the provider should return defaults 
	if(hr == S_OK)
	{
		//Note: Since the update succeeded, the provider used NULLs for nullable columns
		//and Defaults for all columns that were non-nullable that had defaults.  Weither they return
		//the defaults (ie: make a roundtrip to obtain the defaults inserted), is dependent upon the 
		//DBPROP_SERVERDATAONINSERT property.  If this is not supported, then we have to allow
		//errors returned since the defaults may not have been brought back from the server...
		if(SettableProperty(DBPROP_SERVERDATAONINSERT, DBPROPSET_ROWSET))
		{
			TESTC_(hr = RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK);
		}
		else
		{
			TEST3C_(hr = RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
		}

		//GetData for row in the rowset (should return the same result as GetOriginalData)
		TESTC_(RowsetA.pIRowset()->GetData(hRow,RowsetA.m_hAccessor,pData), hr);
	
		//Both GetOriginalData and GetData should match...
		//The row was successfully inserted, so now there is no pending insert
		TESTC(RowsetA.CompareRowData(RowsetA.m_pData, pData));
	}
	else
	{
		//This may produce DB_S_ERRORSOCCURRED since if the driver has non-NULL
		//columns, some providers cannot return ISNULL, and has no idea what valid 
		//defaults would be, so DBSTATUS_E_UNAVAILABLE will be returned for those columns
		TEST3C_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

		//GetData for row in the rowset
		TEST3C_(RowsetA.pIRowset()->GetData(hRow,RowsetA.m_hAccessor,pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
	
		//Compare data with defaults, should match
		TESTC(RowsetA.CompareWithDefaults(RowsetA.m_pData));
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - A modified row, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_28()
{
	void* pOriginalData = NULL;
	HROW hRow = DB_NULL_HROW;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(hRow, &pOriginalData),S_OK);

	//Hard-Modify the row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);

	//Call GetOriginalData on the modified row
	TESTC_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK);

	//Compare data with orginal data, should not equal, updated was performed
	TESTC(!RowsetA.CompareRowData(pOriginalData,RowsetA.m_pData));

	//Compare data with modified data, should be equal, update was performed
	TESTC(RowsetA.CompareRowData(hRow,RowsetA.m_pData));


CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - An inserted/modified row, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_29()
{
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the first row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	
	//Save the OriginalData
	TESTC_(RowsetA.GetRowData(hRow, &pOriginalData),S_OK);
	
	//Modify the row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//Save the modified data
	TESTC_(RowsetA.GetRowData(hRow, &pModifiedData),S_OK);
			
	//Update the row  
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);

	//Call GetOriginalData on the updated row 
	TESTC_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),S_OK);

	//Compare data with modified data, should match, (updated)
	TESTC(RowsetA.CompareRowData(pModifiedData,RowsetA.m_pData));

	//Compare data with original data, should not match (updated)
	TESTC(!RowsetA.CompareRowData(pOriginalData,RowsetA.m_pData));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify/Delete all rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_30()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
    DBCOUNTITEM i,cRows = NROWS;

	//Create a rowset with no rows
	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable)==S_OK);
	cRows = RowsetA.GetMaxPendingRows();
	if(cRows==0 || cRows>NROWS)
		cRows = NROWS;
    
	//Now insert all rows
	TESTC_(RowsetA.InsertRow(cRows, rghRow),S_OK);
    
    //Now modify all rows
	TESTC_(RowsetA.ModifyRow(cRows, rghRow),S_OK);

	//Now delete all rows
	TESTC_(RowsetA.DeleteRow(cRows, rghRow),S_OK);
    
    //Now call GetOriginalData
    for(i=0; i<cRows; i++) 
		TESTC_(RowsetA.GetOriginalData(rghRow[i], &RowsetA.m_pData),DB_E_DELETEDROW);
	
CLEANUP:
    RowsetA.ReleaseRows(NROWS,rghRow);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_31()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_32()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_33()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_34()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_35()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_36()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_37()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - SetPos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_38()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Insert a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->InsertRow(NULL,hAccessor, pInsertData, &hNewRow),S_OK);
	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Delete a row (BLOB data)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//GetOrginalData should produce the orginal results	
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData1, pData2, hAccessor));
	
CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - QBU
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_39()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Insert a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->InsertRow(NULL,hAccessor, pInsertData, &hNewRow),S_OK);
	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Delete a row (BLOB data)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//GetOrginalData should produce the orginal results	
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData1, pData2, hAccessor));
	
CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_40()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DB_E_BADACCESSORTYPE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_41()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;

	//Create the rowset
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateCommand()==S_OK);

	//Create an invalid Accessor for use with GetOriginalData
	//Must be done on the CommandObject, ParameterAccessors are not allowed
	//to be created on the RowsetObject
	TEST2C_(hr = GetAccessorAndBindings(RowsetA.pICommand(),DBACCESSOR_PARAMETERDATA,&hAccessor,
		NULL,NULL,NULL,DBPART_ALL,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT), S_OK, DB_E_BADACCESSORFLAGS);

	if(hr == S_OK)
	{
		//Now that we have the Accessor Created, now create the rowset
		//Must be a command query, since the accessor was created on the command
		TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_REVCOLLIST)==S_OK);
		
		//Obtain the first row
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Call GetOriginalData with an invalid accessor type
		TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hAccessor,RowsetA.m_pData),DB_E_BADACCESSORTYPE);
		TESTC(RowsetA.m_pData!=NULL);
	}
	else
	{
		//Make sure the provider doesn't support parameters (validated immediately)
		TESTC_(QI(RowsetA.pICommand(), IID_ICommandWithParameters), E_NOINTERFACE);
	}

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DB_E_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_42()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;

	//Create the rowset
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateCommand()==S_OK);
	
	//Create an invalid Accessor for use with GetOriginalData
	//Must be done on the CommandObject, ParameterAccessors are not allowed
	//to be created on the RowsetObject
	TEST2C_(hr = GetAccessorAndBindings(RowsetA.pICommand(),DBACCESSOR_PARAMETERDATA,&hAccessor,
		NULL,NULL,NULL,DBPART_ALL,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT), S_OK, DB_E_BADACCESSORFLAGS);

	if(hr == S_OK)
	{
		//Now that we have the Accessor Created, now create the rowset
		//Must be a command query, since the accessor was created on the command
		TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_REVCOLLIST)==S_OK);

		//Obtain the first row
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Call GetOriginalData with an invalid accessor type
		TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hAccessor,RowsetA.m_pData),DB_E_BADACCESSORTYPE);
		TESTC(RowsetA.m_pData!=NULL);
	}
	else
	{
		//Make sure the provider doesn't support parameters (validated immediately)
		TESTC_(QI(RowsetA.pICommand(), IID_ICommandWithParameters), E_NOINTERFACE);
	}

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DB_E_BADBINDINFO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_43()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;

	//Create the rowset
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateCommand()==S_OK);
	
	//Create an invalid Accessor for use with GetOriginalData
	//Must be done on the CommandObject, ParameterAccessors are not allowed
	//to be created on the RowsetObject
	TEST2C_(hr = GetAccessorAndBindings(RowsetA.pICommand(),DBACCESSOR_PARAMETERDATA,&hAccessor,
		NULL,NULL,NULL,DBPART_ALL,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT), S_OK, DB_E_BADACCESSORFLAGS);

	if(hr == S_OK)
	{
		//Now that we have the Accessor Created, now create the rowset
		//Must be a command query, since the accessor was created on the command
		TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_REVCOLLIST)==S_OK);

		//Obtain the first row
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Call GetOriginalData with an invalid accessor type
		TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hAccessor,RowsetA.m_pData),DB_E_BADACCESSORTYPE) ;
		TESTC(RowsetA.m_pData!=NULL);
	}
	else
	{
		//Make sure the provider doesn't support parameters (validated immediately)
		TESTC_(QI(RowsetA.pICommand(), IID_ICommandWithParameters), E_NOINTERFACE);
	}

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DB_E_COLUMNUNAVAILABLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_44()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;

	//Create the rowset
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateCommand()==S_OK);
	
	//Create an invalid Accessor for use with GetOriginalData
	//Must be done on the CommandObject, ParameterAccessors are not allowed
	//to be created on the RowsetObject
	TEST2C_(hr = GetAccessorAndBindings(RowsetA.pICommand(),DBACCESSOR_PARAMETERDATA,&hAccessor,
		NULL,NULL,NULL,DBPART_ALL,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT), S_OK, DB_E_BADACCESSORFLAGS);

	if(hr == S_OK)
	{
		//Now that we have the Accessor Created, now create the rowset
		//Must be a command query, since the accessor was created on the command
		TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_REVCOLLIST)==S_OK);

		//Obtain the first row
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Call GetOriginalData with an invalid accessor type
		TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hAccessor,RowsetA.m_pData),DB_E_BADACCESSORTYPE);
		TESTC(RowsetA.m_pData!=NULL);
	}
	else
	{
		//Make sure the provider doesn't support parameters (validated immediately)
		TESTC_(QI(RowsetA.pICommand(), IID_ICommandWithParameters), E_NOINTERFACE);
	}

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_45()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call GetOriginalData 3 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetOriginalRows::Variation_46()
{
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;

	void* pFirstCall    = NULL;
	void* pSecondCall   = NULL;
	void* pThirdCall    = NULL;
	
	HROW hRow = DB_NULL_HROW;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	pFirstCall    = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);
	pSecondCall   = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);
	pThirdCall    = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);
	
	//Obtain the row handle(s);
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
			
	//Save the OriginalData
	TESTC_(RowsetA.GetRowData(hRow, &pOriginalData),S_OK);
	
	//Modify the row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//Save the modified data
	TESTC_(RowsetA.GetRowData(hRow, &pModifiedData),S_OK);
			
	//Call GetOriginalData the first time 
	TESTC_(RowsetA.GetOriginalData(hRow, &pFirstCall),S_OK);

	//Compare data with modified data, should not match
	TESTC(!RowsetA.CompareRowData(pModifiedData,pFirstCall));

	//Compare data with original data, should match
	TESTC(RowsetA.CompareRowData(pOriginalData,pFirstCall));
    
	//Modify the row again
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//Call GetOriginalData the Second time 
	TESTC_(RowsetA.GetOriginalData(hRow, &pSecondCall),S_OK);
    
	//Modify the row again
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//Call GetOriginalData the Third time 
	TESTC_(RowsetA.GetOriginalData(hRow, &pThirdCall),S_OK);

	//All three GetOriginalData calls should match, no update was performed
	TESTC(RowsetA.CompareRowData(pFirstCall, pSecondCall));
	TESTC(RowsetA.CompareRowData(pSecondCall, pThirdCall));
    
    
CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(pOriginalData);

	PROVIDER_FREE(pFirstCall);
	PROVIDER_FREE(pSecondCall);
	PROVIDER_FREE(pThirdCall);

	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetOriginalRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetPendingRows)
//*-----------------------------------------------------------------------
//|	Test Case:		TCGetPendingRows - IRowsetUpdate::GetPendingRows test case
//|	Created:			03/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPendingRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify GetPendingRows alters row handles correctly
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_1()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW rghRow[THREE_ROWS] = {NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows = 0;
	HROW* rgPendingRows = NULL;
	ULONG* rgRefCounts = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	ULONG ulRefCount = 0;
	DBSTATUS dwRowStatus = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, &cRowsObtained, rghRow),S_OK);
	
	//Make change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK) //modify row 1;

	//Release row handles
	//DB_S_ERRORSOCCURRED - PendingChanges
	TESTC_(RowsetA.ReleaseRows(cRowsObtained, rghRow, &rgRefCounts, &rgRowStatus),S_OK);
	TESTC(rgRowStatus[ROW_ONE] == DBROWSTATUS_S_PENDINGCHANGES);

	//Verify now invalid row handles
	TESTC_(RowsetA.ReleaseRows(ONE_ROW, rghRow, &ulRefCount, &dwRowStatus), rgRefCounts[ROW_ONE] == 0 ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC_(RowsetA.ReleaseRows(ONE_ROW, rghRow, &ulRefCount, &dwRowStatus), ulRefCount == 0 ? DB_E_ERRORSOCCURRED : S_OK);
	
	//Call GetPendingRows, and have row handles retuned	
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,NULL),S_OK);
	TESTC(cPendingRows==ONE_ROW);
	TESTC(rgPendingRows[0]==rghRow[ROW_ONE]);
	
 	//verify row handles have been refcounted, should be valid
	TESTC_(RowsetA.ReleaseRows(cPendingRows, rgPendingRows, &ulRefCount, &dwRowStatus),S_OK);
    TESTC_(RowsetA.ReleaseRows(cPendingRows, rgPendingRows), ulRefCount == 0 ? DB_E_ERRORSOCCURRED : S_OK);
          
CLEANUP:
	RowsetA.ReleaseRows(cPendingRows, rgPendingRows);
	PROVIDER_FREE(rgRefCounts);
	PROVIDER_FREE(rgRowStatus);
    PROVIDER_FREE(rgPendingRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, _ALL, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_2()
{
	TBEGIN
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;
	HRESULT hr = S_OK;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	//Grab all the rows in the rowset, provider alloced array
	hr = RowsetA()->GetNextRows(NULL, 0, LONG_MAX, &cRowsObtained, &rghRows);
	TESTC(hr==DB_S_ENDOFROWSET || hr==DB_S_ROWLIMITEXCEEDED || hr==E_OUTOFMEMORY);
	if(hr==E_OUTOFMEMORY)
	{
		TESTC(cRowsObtained == 0);
		TESTC(rghRows == NULL);
		goto CLEANUP;
	}
	if(hr==DB_S_ROWLIMITEXCEEDED)
	{
		TESTC(RowsetA.GetMaxOpenRows() != 0);
		TESTC(cRowsObtained && cRowsObtained<LONG_MAX);
		TESTC(rghRows!=NULL);
	}
	else
	{
		TESTC(cRowsObtained && cRowsObtained<LONG_MAX && cRowsObtained >= 3);
		TESTC(rghRows!=NULL);
	}

	//Modify Row 1
	TESTC_(RowsetA.ModifyRow(rghRows[ROW_ONE]),S_OK)    
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),S_OK);

	//Delete Row 2
	TESTC_PROVIDER(cRowsObtained >=2);
	if(!RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.UpdateAll(),S_OK);
	TESTC_(RowsetA.DeleteRow(rghRows[ROW_TWO]),S_OK) 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),S_OK);

	//Insert Row3
	TESTC_PROVIDER(cRowsObtained >=3);
	if(!RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.UpdateAll(),S_OK);
	TESTC_(RowsetA.InsertRow(&rghRows[ROW_THREE]),S_OK) 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(cRowsObtained,rghRows); 
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [invalid, _ALL, NULL, invalid, invalid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_3()
{
	TBEGIN
	HROW rghRow[THREE_ROWS] = {NULL,NULL,NULL};

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//NOTE: We pass Invalid pointers to prgPendingRows and prgPendingStauts on input, since the
	//spec states prgPendingRows and prgPendingStatus are ignored if pcPendingRows is NULL on input.

	//insert row 1;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_ONE]),S_OK)   
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(INVALID(HCHAPTER),DBPENDINGSTATUS_ALL,NULL,INVALID(HROW**),INVALID(ULONG**)),S_OK);

	//insert row 2;
	if(!RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.UpdateAll(),S_OK);
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_TWO]),S_OK)   
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(INVALID(HCHAPTER),DBPENDINGSTATUS_ALL,NULL,INVALID(HROW**),INVALID(ULONG**)),S_OK);

	//insert row 3;
	if(!RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.UpdateAll(),S_OK);
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK) 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(INVALID(HCHAPTER),DBPENDINGSTATUS_ALL,NULL,INVALID(HROW**),INVALID(ULONG**)),S_OK);
		

CLEANUP:
	RowsetA.ReleaseRows(THREE_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, _ALL, valid, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_4()
{
	TBEGIN
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBCOUNTITEM cPendingRows = 0;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);
	
	//Make change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_OK);
	COMPC(cPendingRows,ONE_ROW);

	TESTC_PROVIDER(RowsetA.AllowPendingRows(2));
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)    //delete row 2
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_OK);
	COMPC(cPendingRows,TWO_ROWS);

	TESTC_PROVIDER(RowsetA.AllowPendingRows(3));
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK) //Insert row 3
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)  //modify row 3
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_OK);
	COMPC(cPendingRows,THREE_ROWS);

	TESTC_PROVIDER(RowsetA.AllowPendingRows(4));
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //insert row 4
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK)   //modify row 4
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)   //delete row 4
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_OK);
	COMPC(cPendingRows,THREE_ROWS);
	
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, _ALL, valid, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_5()
{
	TBEGIN
	const int NROWS = SEVEN_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL}; 
	DBCOUNTITEM cPendingRows = 0;
	DBPENDINGSTATUS* rgPendingStatus = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK);
		
	//Make change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	cModifiedRows++;
		
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)  //modify row 3;
		cModifiedRows++;
	}
	else
	{			
		//Otherwise make sure the Data is unchanged...
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),DB_E_MAXPENDCHANGESEXCEEDED);
		TESTC(RowsetA.CompareOrgRowData(rghRow[ROW_THREE]));
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //modify row 4;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)   //delete row 4;
	}

	if(RowsetA.AllowPendingRows(4))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_SIX]),S_OK)   //insert row 6;
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_SIX]),S_OK)    //modify row 6;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_SIX]),S_OK)    //delete row 6;
	}	
	
	if(RowsetA.AllowPendingRows(5))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_SEVEN]),S_OK) //insert row 7;
		cModifiedRows++;
	}

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,&rgPendingStatus),S_OK);
	COMPC(cPendingRows, cModifiedRows)
	COMPC(rgPendingStatus[0], DBPENDINGSTATUS_CHANGED)
	if(cModifiedRows >= 2)
		COMPC(rgPendingStatus[1], DBPENDINGSTATUS_CHANGED)
	if(cModifiedRows >= 3)
		COMPC(rgPendingStatus[2], DBPENDINGSTATUS_NEW)

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgPendingStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, _ALL, valid, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_6()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	DBCOUNTITEM cPendingRows = 0;
	HROW* rgPendingRows = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Make change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1

	TESTC_PROVIDER(RowsetA.AllowPendingRows(2));
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)    //delete row 2

	TESTC_PROVIDER(RowsetA.AllowPendingRows(3));
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK) //insert row 3
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)  //delete row 3

	TESTC_PROVIDER(RowsetA.AllowPendingRows(4));
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK)   //modify row 4
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)   //delete row 4

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,NULL),S_OK);
	RowsetA.ReleaseRows(cPendingRows,rgPendingRows); //GetPendingRows refcounts

	COMPC(cPendingRows,THREE_ROWS)
	TESTC(rgPendingRows!=NULL);
	COMPC(rgPendingRows[0],rghRow[ROW_ONE])
	COMPC(rgPendingRows[1],rghRow[ROW_TWO])
	COMPC(rgPendingRows[2],rghRow[ROW_FOUR])
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgPendingRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, _ALL, NULL, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_7()
{
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),S_FALSE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, _ALL, valid, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_8()
{
	DBCOUNTITEM cPendingRows = 1;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_FALSE);
	COMP(cPendingRows,NO_ROWS);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, _ALL, valid, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_9()
{
	DBCOUNTITEM cPendingRows = 1;
	HROW* rgPendingRows = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,NULL),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL) ;
	
CLEANUP:
	PROVIDER_FREE(rgPendingRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, _ALL, valid, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_10()
{
	DBCOUNTITEM cPendingRows = 1;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
CLEANUP:
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes (NULL, _ALL, valid, NULL, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_11()
{
	DBCOUNTITEM cPendingRows = 1;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,&rgPendingStatus),S_FALSE);
	TESTC(cPendingRows==NO_ROWS && rgPendingStatus==NULL);
	
CLEANUP:
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Empty Rowset [NULL, _ALL, valid, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_12()
{
	DBCOUNTITEM cPendingRows = 1;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	
	TESTC_(EmptyRowset.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
CLEANUP:
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_13()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS seperately, before any changes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_14()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	HROW hNewRow       = NULL;
	ULONG cModifiedRows = 0;
	ULONG cDeletedRows = 0;

	CRowsetUpdate RowsetA; 
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//BEFORE CHANGES

	//GetPendingRows DBPENDINGSTATUS_NEW
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_NEW, NO_ROWS),S_FALSE);
	
	//GetPendingRows DBPENDINGSTATUS_CHANGED
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_CHANGED, NO_ROWS),S_FALSE);

	//GetPendingRows DBPENDINGSTATUS_DELETED
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED, NO_ROWS),S_FALSE);

	//GetPendingRows DBPENDINGSTATUS_ALL
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_ALL, NO_ROWS),S_FALSE);

	//AFTER CHANGES
	
	//Insert a row
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK)		    //Insert row;
	
	//Modify a row
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)		//modify row 1;
		cModifiedRows++;
	}
	
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)	//modify row 3;
		cModifiedRows++;
	}

	//Delete a row
	if(RowsetA.AllowPendingRows(4))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)	//delete row 4;
		cDeletedRows++;
	}

	//GetPendingRows DBPENDINGSTATUS_NEW
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_NEW, ONE_ROW, DBPENDINGSTATUS_NEW),S_OK);
	
	//GetPendingRows DBPENDINGSTATUS_CHANGED
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_CHANGED, cModifiedRows, DBPENDINGSTATUS_CHANGED),cModifiedRows ? S_OK : S_FALSE);

	//GetPendingRows DBPENDINGSTATUS_DELETED
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED, cDeletedRows, DBPENDINGSTATUS_DELETED), cDeletedRows ? S_OK : S_FALSE);

	//GetPendingRows DBPENDINGSTATUS_ALL
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_ALL, 1 + cModifiedRows + cDeletedRows), S_OK);

	//GetPendingRows DBPENDINGSTATUS_UNCHANGED
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_UNCHANGED,NULL,NULL,NULL),E_INVALIDARG);

	//GetPendingRows DBPENDINGSTATUS_INVALIDROW
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_INVALIDROW,NULL,NULL,NULL),E_INVALIDARG);
	
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS_INVALIDROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_15()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;

	//GetPendingRows DBPENDINGSTATUS_INVALIDROW
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_INVALIDROW,&cPendingRows,&rgPendingRows,&rgPendingStatus),E_INVALIDARG);
	COMPC(cPendingRows, NO_ROWS);
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS _NEW | _CHANGED | _SOFTDELETED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_16()
{
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)  //modify row 2;

	//GetPendingRows NEW | CHANGED | SOFTDELETED
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_NEW |DBPENDINGSTATUS_CHANGED |DBPENDINGSTATUS_DELETED,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_OK);
	RowsetA.ReleaseRows(cPendingRows,rgPendingRows); //GetPendingRows refcounts

	COMPC(cPendingRows,ONE_ROW)
	COMPC(rgPendingRows[0],rghRow[ROW_TWO])
	COMPC(rgPendingStatus[0],DBPENDINGSTATUS_CHANGED)
	
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);

	//GetPendingRows NEW | CHANGED | SOFTDELETED | UNCHANGED
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_NEW |DBPENDINGSTATUS_CHANGED |DBPENDINGSTATUS_DELETED|DBPENDINGSTATUS_UNCHANGED,&cPendingRows,&rgPendingRows,&rgPendingStatus),E_INVALIDARG);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);	
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS_NEW, w/ no new rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_17()
{
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Delete row(s)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)	 //modify row 2;

	//GetPendingRows NEW, with no new rows     
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_NEW,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,	NO_ROWS)  //No NEW rows
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);


CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS_CHANGED, w/ no changed rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_18()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK)        //insert row;

	//GetPendingRows CHANGED, with no modified rows
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_CHANGED,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)  //No changed rows
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);


CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);	
	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS_UNCHANGED, w/ all changed rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_19()
{
	const int NROWS = SEVEN_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};  	
	
	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s) 
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Modify row(s)
	cModifiedRows = RowsetA.GetMaxPendingRows();
	if(cModifiedRows==0 || cModifiedRows > NROWS)
		cModifiedRows = NROWS;
	TESTC_(RowsetA.ModifyRow(cModifiedRows, rghRow),S_OK)  //modify all rows;

	//GetPendingRows DBPENDINGSTATUS_UNCHANGED
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_UNCHANGED,&cPendingRows,&rgPendingRows,&rgPendingStatus),E_INVALIDARG);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPENDINGSTATUS_DELETED, w/ no soft-deleted rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_20()
{
	HROW hNewRow = NULL;

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(hNewRow),S_OK);

	//GetPendingRows SOFTDELETED, with no deleted rows
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_DELETED,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)  //No soft-Deleted rows
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);

	//According to the 2.0 spec, InsertRow - SetData counts as NEW row not CHANGED
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_NEW, ONE_ROW),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_CHANGED, NO_ROWS),S_FALSE);

CLEANUP:
	RowsetA.ReleaseRows(hNewRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_21()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - _NEW | _UNCHANGED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_22()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;	
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_UNCHANGED,&cPendingRows,&rgPendingRows,&rgPendingStatus),E_INVALIDARG);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);


CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - _NEW | _SOFTDELETED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_23()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
	//Modify row(s)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK);

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_DELETED,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_OK);
	RowsetA.ReleaseRows(cPendingRows,rgPendingRows); //GetPendingRows refcounts

	TESTC(cPendingRows == ONE_ROW);
	TESTC(rgPendingRows!=NULL && rgPendingStatus!=NULL);
	TESTC(rgPendingRows[0]==hNewRow);
	TESTC(rgPendingStatus[0]==DBPENDINGSTATUS_NEW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);	
	RowsetA.ReleaseRows(hNewRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_24()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify/Delete a single row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_25()
{
	HROW hRow = DB_NULL_HROW;

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;	
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	//Delete row(s)
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);


CLEANUP:
	RowsetA.ReleaseRows(hRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_26()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call GetPendingRows 3 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_27()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;
	ULONG cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
	cModifiedRows++;
	
	//Modify row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
		cModifiedRows++;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK);
		cModifiedRows++;
	}

	//GetPendingRows #1
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_ALL, cModifiedRows),S_OK);
	
	//GetPendingRows #2
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_ALL, cModifiedRows),S_OK);

	//Update
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//GetPendingRows #3
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_ALL, NO_ROWS),S_FALSE);


CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Modify the same row twice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_28()
{
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);
	//Make a duplicate HROW, HROW3 == HROW1
	rghRow[ROW_THREE] = rghRow[ROW_ONE];

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)	//modify row 1, duplicate;

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_OK);
	RowsetA.ReleaseRows(cPendingRows,rgPendingRows); //GetPendingRows refcounts

	COMPC(cPendingRows,ONE_ROW)
	COMPC(rgPendingRows[0],rghRow[ROW_ONE])
	COMPC(rgPendingStatus[0],DBPENDINGSTATUS_CHANGED)


CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Modify the same row twice, undo, GetPendingRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPendingRows::Variation_29()
{
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);
	//Make a duplicate HROW, HROW3 == HROW1
	rghRow[ROW_THREE] = rghRow[ROW_ONE];

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)	//delete row 1, duplicate;

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_OK);
	RowsetA.ReleaseRows(cPendingRows,rgPendingRows); //GetPendingRows refcounts

	COMPC(cPendingRows,ONE_ROW)
	COMPC(rgPendingRows[0],rghRow[ROW_ONE])
	COMPC(rgPendingStatus[0],DBPENDINGSTATUS_DELETED)

	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);

	//Undo all
	TESTC_(RowsetA.UndoAll(),S_OK);

	//GetPendingRows 
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);


CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPendingRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCUndo)
//*-----------------------------------------------------------------------
//|	Test Case:		TCUndo - IRowsetUpdate::Undo test case
//|	Created:			03/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUndo::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, 0, NULL, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_1()
{
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,NULL,NULL,NULL),S_OK);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, N, valid, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_2()
{
	HROW hInvalidRow = INVALID(HROW);

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,&hInvalidRow,NULL,NULL,NULL),S_OK);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL,0,NULL, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_3()
{
	DBCOUNTITEM cRowsUndone  = 1;  //should be 0 on error
	HROW* rgRowsUndone;      //should be NULL on error
	DBROWSTATUS* rgRowStatus;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus),S_OK);
	COMPC(cRowsUndone,NO_ROWS)
	TESTC(rgRowsUndone==NULL && rgRowStatus==NULL);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [invalid, 1, NULL, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_4()
{
	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone;
	DBROWSTATUS* rgRowStatus;
		
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(RowsetA.pIRowsetUpdate()->Undo(INVALID(HCHAPTER),ONE_ROW,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus),E_INVALIDARG);
	COMPC(cRowsUndone,NO_ROWS)
	TESTC(rgRowsUndone==NULL && rgRowStatus==NULL);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, 0, NULL, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_5()
{
	TBEGIN
	const int NROWS = SEVEN_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL}; 
	HROW* rgRowsUndone = INVALID(HROW*);
	DBROWSTATUS* rgRowStatus = INVALID(DBROWSTATUS*);

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_FOUR]),S_OK);
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_FIVE]),S_OK);
	
	//Make change(s)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK)  //insert row 3;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)   //modify row 3;

	//Undo (0,NULL)
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,NULL,&rgRowsUndone,&rgRowStatus),S_OK);

	//Since pcRowsUndone==NULL, there can be no out arrays
	//Should just ingore rgRowsUndone / rgRowStatus
 	TESTC(rgRowsUndone==INVALID(HROW*) && rgRowStatus==INVALID(DBROWSTATUS*));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, 0, valid, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_6()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK) ;
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_TWO]),S_OK)	  ;
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_FOUR]),S_OK)	  ;
	
	//Make 2 changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)			//modify row 1;
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)		//delete row 2;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK)	//insert row 3;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)	//delete row 3;
	}

	//Call Undo, (all)
	TESTC_(RowsetA.UndoRow(0,rghRow),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, N, valid, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_7()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL}; 
	DBROWSTATUS* rgRowStatus = NULL;

	void* pOriginalData[NROWS] = {NULL,NULL,NULL};
	void* pModifiedData[NROWS] = {NULL,NULL,NULL};

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK) ;
	TESTC_(RowsetA.GetNextRows(SECOND_ROW,&rghRow[ROW_TWO]),S_OK);
	rghRow[ROW_THREE] = rghRow[ROW_TWO]; //2 instances of the same hrow with changes
	
	//Save Orginal Data
	TESTC_(RowsetA.GetRowData(NROWS, rghRow, pOriginalData),S_OK);
	
	//Make 2 changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)			//modify row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)		//modify row 2;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)	//delete row 3;
	
	//Save Modified Data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_ONE],&pModifiedData[0]),S_OK);
	
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),S_OK);
	//rgRowStatus should be filled in	
	TESTC(rgRowStatus!=NULL);
	TESTC(VerifyArray(NROWS,rgRowStatus,DBROWSTATUS_S_OK));

	//Verify undo matched orginal data
	TESTC(RowsetA.CompareRowData(NROWS, rghRow,pOriginalData));
	//Verify undo does not match modified data
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_ONE],pModifiedData[0]));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS, pOriginalData);
	PROVIDER_FREE2(NROWS, pModifiedData);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, N, valid, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_8()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	DBROWSTATUS* rgRowStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK) ;
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_TWO]),S_OK)	;
	rghRow[ROW_FOUR] = rghRow[ROW_TWO]; //2 instances of the same hrow with changes
	
	//Make  changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK)		//insert row 3;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)		//delete row 3;
	
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)		//modify row 1;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)		//delete row 2;
	
	//Undo NROWS 
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus!=NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[3]==DBROWSTATUS_S_OK);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow); 
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [invalid, N, NULL, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_9()
{
	TBEGIN
	DBCOUNTITEM cRowsUndone  = 1; //should be 0 on error
	HROW* rgRowsUndone;     //should be NULL on error
	DBROWSTATUS* rgRowStatus;     //should be NULL on error
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	
	//Make 2 changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)     //modify row 1;
	
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(INVALID(HCHAPTER),NROWS,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus),E_INVALIDARG);
	COMPC(cRowsUndone,NO_ROWS)
	TESTC(rgRowsUndone==NULL && rgRowStatus==NULL);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [invalid, N, valid, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_10()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	DBCOUNTITEM cRowsUndone = 1;
	DBROWSTATUS* rgRowStatus = INVALID(DBROWSTATUS*);

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1	
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK)  ;
		
	//Make 2 changes  
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,&cRowsUndone,NULL,&rgRowStatus),E_INVALIDARG);
	COMPC(cRowsUndone,NO_ROWS);
	TESTC(rgRowStatus==NULL);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_11()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 invalid / 2 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_12()
{
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	
	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	
	DBROWSTATUS* rgRowStatus = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	
	rghRow[ROW_ONE] = DB_NULL_HROW;   //invalid
	TESTC_(RowsetA.GetRow(FIRST_ROW,TWO_ROWS,&rghRow[ROW_TWO]),S_OK) ;
	rghRow[ROW_FOUR] = INVALID(HROW); //invalid
	rghRow[ROW_FIVE] = DB_NULL_HROW;  //invalid
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE], &pOriginalData),S_OK);
	
	//Delete/Modify the row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK) //modify row 3;
	
	//Save the modified row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE], &pModifiedData),S_OK);

	//Call Undo on the modified/invlaid row(s) 
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus!=NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[3]==DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[4]==DBROWSTATUS_E_INVALID);

	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData));

	//Compare data with modified data, should not be equal, undo was called
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_THREE], pModifiedData));

CLEANUP:
	//Release the row handles
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 hard del / 1 invalid / 1 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_13()
{
	TBEGIN
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL}; 
	
	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	rghRow[ROW_ONE] = DB_NULL_HROW;		    
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK) ;
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_THREE]),S_OK);
	
	//Save the orginal modified row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_TWO], &pOriginalData),S_OK);
	
	//Hard delete the row
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK) //hard-delete row 3;

	//Modify/delete row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)       //modify row 2;
	TESTC_(RowsetA.GetRowData(rghRow[ROW_TWO], &pModifiedData),S_OK);

	//Call Undo on the modified/invlaid row(s)
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus!=NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_E_DELETED);
	
	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(rghRow[ROW_TWO],pOriginalData));
	
	//Compare data with modified data, should not be equal, undo was called
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_TWO],pModifiedData));


CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 hard del / 1 invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_14()
{
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL}; 

	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	rghRow[ROW_ONE] = DB_NULL_HROW;		
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK) ;
	rghRow[ROW_THREE] = rghRow[ROW_TWO];           //row 3 == row 2 duplicate

	//Hard-Delete row(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_TWO]),S_OK)  //hard-delete row 2;
	
	//Call Undo on the modified/invlaid row(s)
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgRowStatus!=NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[1]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[2]==DBROWSTATUS_E_DELETED);

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 soft del / 2 orginal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_15()
{
	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	void* pOriginalData[NROWS] = {NULL,NULL,NULL,NULL,NULL};

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(NROWS, rghRow, pOriginalData),S_OK);
		
	//Delete the row(s)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_ONE]),S_OK)			//delete row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)	//delete row 3;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FIVE]),S_OK)	//delete row 5;

	//Call Undo on the deleted/orginal row(s)
	TESTC_(RowsetA.UndoRow(NROWS,rghRow),S_OK);

	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(NROWS, rghRow, pOriginalData));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS, pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 new rows / 1 orginal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_16()
{
	TBEGIN
	void* pOriginalData = NULL;
	void* pData = NULL;
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	pData = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(SECOND_ROW, &rghRow[ROW_THREE]),S_OK)		//get row 3;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_ONE]),S_OK)				//insert row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_TWO]),S_OK)            //insert row 2;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)           //insert row 4;
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE], &pOriginalData),S_OK) //orginal;

	//Call Undo on the inserted/orginal row(s)
	TESTC_(RowsetA.UndoRow(NROWS,rghRow),RowsetA.AllowPendingRows(3) ? S_OK : DB_S_ERRORSOCCURRED);

	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData));

    //Inserted rows should no longer exists...
	//TESTC(!ValidRow(rghRow[ROW_ONE]));
	//TESTC(!ValidRow(rghRow[ROW_TWO]));
	//TESTC(!ValidRow(rghRow[ROW_FOUR]));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);  
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 4 modified / 3 orginal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_17()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	void* pOriginalData[NROWS] = {NULL,NULL,NULL,NULL};
	void* pModifiedData = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(NROWS, rghRow, pOriginalData),S_OK);
		
	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK) //modify row 3;

	//Save the modified row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE], &pModifiedData),S_OK);

	//Call Undo on the deleted/orginal row(s)
	TESTC_(RowsetA.UndoRow(NROWS,rghRow),S_OK);

	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(NROWS, rghRow, pOriginalData));

	//Compare data with modified data, should not be equal, undo was called
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_THREE], pModifiedData));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS, pOriginalData);
	PROVIDER_FREE(pModifiedData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 soft del / 2 orginal, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_18()
{
	const int NROWS = FIVE_ROWS;

	void* pOriginalData[NROWS] = {NULL,NULL,NULL,NULL};
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	ULONG cDeletedRows = 0;
	
	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(NROWS, rghRow, pOriginalData),S_OK);
		
	//Delete the row(s)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_ONE]),S_OK)			//delete row 1;
	cDeletedRows++;

	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)	//delete row 3;
		cDeletedRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FIVE]),S_OK)	//delete row 5;
		cDeletedRows++;
	}

	//Update
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow),S_OK);

	//Call Undo on the deleted/orginal row(s)
	//I could just Undo all, but since I want to pound on the array that is 
	//holding the pending rows, try undoing every other row in the array

	//deleted
	TESTC_(RowsetA.UndoRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	COMPC(rgRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPC(rgRowStatus[1], DBROWSTATUS_S_OK);
	COMPC(rgRowStatus[2], (DBROWSTATUS)((cDeletedRows >= 2) ? DBROWSTATUS_E_DELETED : DBROWSTATUS_S_OK));
	COMPC(rgRowStatus[3], DBROWSTATUS_S_OK);
	COMPC(rgRowStatus[4], (DBROWSTATUS)((cDeletedRows >= 3) ? DBROWSTATUS_E_DELETED : DBROWSTATUS_S_OK));

	//Compare orginal rows with orginal data, should be equal, no change
	TESTC(RowsetA.CompareRowData(rghRow[ROW_TWO], pOriginalData[1]));
	TESTC(RowsetA.CompareRowData(rghRow[ROW_FOUR], pOriginalData[3]));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS, pOriginalData);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(THREE_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 new rows / 1 orginal, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_19()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	ULONG cRows = 0;

	void* pOriginalData[NROWS] = {NULL,NULL,NULL,NULL};
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(SECOND_ROW, &rghRow[ROW_ONE]),S_OK);
	cRows++;

	TESTC_(RowsetA.InsertRow(&rghRow[ROW_TWO]),S_OK);
	cRows++;
	
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK);
		cRows++;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK);
		cRows++;
	}
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(cRows, rghRow, pOriginalData),S_OK);

	//Update all row(s)
	TESTC_(RowsetA.UpdateRow(cRows, rghRow),S_OK);

	//Call Undo on the inserted/orginal row(s)
	TESTC_(RowsetA.UndoRow(cRows, rghRow),S_OK);
	
	//Compare data with orginal data, should be equal, undo was called
	//Inserted rows should exists, since update was called
	TESTC(RowsetA.CompareRowData(cRows, rghRow, pOriginalData));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS, pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 4 modified / 3 orginal, update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_20()
{
	const int NROWS = FOUR_ROWS;
	void* pOriginalData[NROWS] = {NULL,NULL,NULL,NULL};
	void* pModifiedData = NULL;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	DBCOUNTITEM cRowsUndone = 1;
		
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save the orginal row(s) data
	TESTC_(RowsetA.GetRowData(NROWS, rghRow, pOriginalData),S_OK);
		
	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK) //modify row 3;

	//Save the modified row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE], &pModifiedData),S_OK);

	//Update
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow),S_OK);

	//Call Undo
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);

	//Compare with orginal data, should not be equal, update was called
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData[2]));

	//Compare with modified data, should be equal, update was called
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE], pModifiedData));

	//Compare orginal rows, should be equal to prginal data, no change
	TESTC(RowsetA.CompareRowData(rghRow[ROW_ONE], pOriginalData[0]));
	TESTC(RowsetA.CompareRowData(rghRow[ROW_TWO], pOriginalData[1]));
	TESTC(RowsetA.CompareRowData(rghRow[ROW_FOUR], pOriginalData[3]));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE2(NROWS,pOriginalData);
	PROVIDER_FREE(pModifiedData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 hard del / 2 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_21()
{
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;

	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain handle(s), starting at row 1, get 5 handles
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK);
	
	//Save the orginal row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_TWO], &pOriginalData),S_OK);
	
	//Hard-Delete the row(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK);
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FIVE]),S_OK);

	//Modify the row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK) //modified;
	
	//Save the modified row data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_TWO], &pModifiedData),S_OK);

	//Call Undo on the modified/deleted row(s)
	//I could just Undo all, but since I want to pound on the array that is 
	//holding the pending rows, try undoing every other row in the array

	//deleted
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	
	//Verify output
	TESTC(rgRowStatus!=NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[3]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[4]==DBROWSTATUS_E_DELETED);

	//Compare data with orginal data, should be equal, undo was called
	TESTC(RowsetA.CompareRowData(rghRow[ROW_TWO],pOriginalData));

	//Compare data with modified data, should not be equal, undo was called
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_TWO],pModifiedData));

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(THREE_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify/Delete a single row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_22()
{
	HROW hRow = DB_NULL_HROW; 
	
	DBCOUNTITEM cRowsUndone = 1;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Insert row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	//Modify row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	//Delete row
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Call Undo
	//An inserted row has been deleted.  This row is now an invalid state, and is no
	//no longer pending (see GetPendingRows), and IRowsetUpdate::Undo with (0,NULL) 
	//only undoes pending rows, thus there are no pending rows to undo, thus S_OK...
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);

	//Row should no longer exists
	TESTC_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),DB_E_DELETEDROW);

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify/Delete a single row, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_23()
{
	HROW hRow = DB_NULL_HROW; 
	
	DBCOUNTITEM cRowsUndone = 1;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Insert row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	//Modify row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	//Delete row
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Update
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//Call Undo
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);

	//Row should not exists, update was called
	TESTC_(RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),DB_E_DELETEDROW);

CLEANUP:
	//Release the row handle
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Duplicate entries in the array
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_24()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	
	ULONG cRowsUndone = 1;
	HROW* rgRowsUndone = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);
	
	//Make a duplicate HROW, 
	rghRow[ROW_THREE] = rghRow[ROW_ONE];  // HROW3 == HROW1
	rghRow[ROW_FOUR]  = rghRow[ROW_TWO];  // HROW4 == HROW2

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)		//modify row 1;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)	//modify row 1, duplicate;

	//Call Undo
	TESTC_(RowsetA.UndoRow(NROWS,rghRow),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgRowsUndone);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_25()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Fetch every other row, modify every even, undo all odd.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_26()
{
	TBEGIN
	const int NROWS = SIX_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL};
	HROW hNewRow = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Modify every even row
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK);
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_SIX]),S_OK);

	//Now internally there should be an array of every other row
	//Just to pound on that array, will insert delete a couple of rows from the middle
	if(RowsetA.AllowPendingRows(4))
		TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
	if(RowsetA.AllowPendingRows(5))
		TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FOUR]),S_OK);
	
	//Now Undo every even
	TESTC_(RowsetA.UndoRow(rghRow[ROW_TWO]),S_OK);
	TESTC_(RowsetA.UndoRow(rghRow[ROW_FOUR]), RowsetA.AllowPendingRows(5) ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC_(RowsetA.UndoRow(rghRow[ROW_SIX]),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call Undo 3 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_27()
{
	TBEGIN
	const int NROWS = SIX_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL};
	HROW hNewRow = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	DBCOUNTITEM cRowsUndone = 0;
	HROW* rgRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = PROVIDER_ALLOC_(1,DBROWSTATUS);
	ULONG ulRefCount = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK)			//insert row 7;
	cModifiedRows++;

	//Release some of the Rows, just to make sure that the changes
	//are not lost, and that internally the provider doesn't have a problem
	//trying to undo pending changes where the row refount is released...
	TESTC_(RowsetA.ReleaseRows(1, &hNewRow, &ulRefCount),S_OK);
	RowsetA.ReleaseRows(1, &hNewRow, &ulRefCount);

	//Modify row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)	//modify row 1;
		cModifiedRows++;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)	//modify row 2;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)	//delete row 2;
		cModifiedRows++;
	}
	if(RowsetA.AllowPendingRows(4))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_SIX]),S_OK)	//modify row 6;
		TESTC_(RowsetA.ReleaseRows(rghRow[ROW_SIX]),S_OK);
		cModifiedRows++;
	}
	
	//Call Undo #1
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone,&rgRowsUndone),S_OK);
	TESTC(cRowsUndone==cModifiedRows && rgRowsUndone!=NULL);
	
	//Since 0, NULL was passed the returned array is not required to be ordered
	//In the same order of the changes.
	TESTC(FindValue(hNewRow, cRowsUndone, rgRowsUndone));
	if(cModifiedRows >= 2)
		TESTC(FindValue(rghRow[ROW_ONE], cRowsUndone, rgRowsUndone));
	if(cModifiedRows >= 3)
		TESTC(FindValue(rghRow[ROW_TWO], cRowsUndone, rgRowsUndone));
	if(cModifiedRows >= 4)
		TESTC(FindValue(rghRow[ROW_SIX], cRowsUndone, rgRowsUndone));

	//Call Undo #2
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);
	
	//Delete row(s) (row was already released) 
    TESTC_(RowsetA.pIRowsetUpdate()->DeleteRows(NULL,ONE_ROW,&hNewRow,rgRowStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0] == (DBROWSTATUS)(ulRefCount ? DBROWSTATUS_E_DELETED : DBROWSTATUS_E_INVALID));	
	
	//Call Undo #3
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);
		

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);
	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(rgRowsUndone);   
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_28()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Empty Rowset [NULL, 0 ,NULL, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_29()
{
	DBCOUNTITEM cRowsUndone  = 1;
	
	CRowsetUpdate EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	
	TESTC_(EmptyRowset.UndoRow(0,NULL,&cRowsUndone),S_OK);
	TESTC(cRowsUndone==NO_ROWS);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_30()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - SetPos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_31()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Insert a row (BLOB data)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.pIRowsetChange()->InsertRow(NULL,hAccessor, pInsertData, &hNewRow),S_OK);
	//Delete a row (BLOB data)
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//Before undo, should be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData2, pModifyData, hAccessor));

	//Now call Undo
	TESTC_(RowsetA.UndoAll(),S_OK);
	
	//After undo, should not be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(!RowsetA.CompareRowData(pData2, pModifyData, hAccessor));


CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - QBU
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_32()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Insert a row (BLOB data)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.pIRowsetChange()->InsertRow(NULL,hAccessor, pInsertData, &hNewRow),S_OK);
	//Delete a row (BLOB data)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//Before undo, should be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData2, pModifyData, hAccessor));

	//Now call Undo
	TESTC_(RowsetA.UndoAll(),S_OK);
	
	//After undo, should not be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(!RowsetA.CompareRowData(pData2, pModifyData, hAccessor));


CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_33()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc General - Soft-Deleted/Undo verify valid row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_34()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Delete row(s)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);
	cModifiedRows++;
	
	//Insert row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
		cModifiedRows++;
	}

	//Modify row(s)
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		cModifiedRows++;
	}
	
	//Call Undo
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone,&rgRowsUndone),S_OK);
	COMPC(cRowsUndone, cModifiedRows);

	//Since 0, NULL was passed the returned array is not required to be ordered
	//In the same order of the changes.
	TESTC(FindValue(rghRow[ROW_THREE], cRowsUndone, rgRowsUndone));
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_THREE], &RowsetA.m_pData),S_OK);
	if(cModifiedRows >= 2)
	{
		TESTC(FindValue(hNewRow, cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(hNewRow, &RowsetA.m_pData),DB_E_DELETEDROW);
	}
	if(cModifiedRows >= 3)
	{
		TESTC(FindValue(rghRow[ROW_ONE], cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &RowsetA.m_pData),S_OK);
	}
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);
	PROVIDER_FREE(rgRowsUndone);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc General - Inserted/Undo verify invalid row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_35()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Insert row(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
	cModifiedRows++;

	//Modify row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		cModifiedRows++;
	}
	//Delete row(s)
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);
		cModifiedRows++;
	}
	
	//Call Undo
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone,&rgRowsUndone),S_OK);
	COMPC(cRowsUndone, cModifiedRows)

	//Since 0, NULL was passed the returned array is not required to be ordered
	//In the same order of the changes.
	TESTC(FindValue(hNewRow, cRowsUndone, rgRowsUndone));
	TESTC_(RowsetA.GetOriginalData(hNewRow, &RowsetA.m_pData),DB_E_DELETEDROW);
	if(cModifiedRows >= 2)
	{
		TESTC(FindValue(rghRow[ROW_ONE], cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &RowsetA.m_pData),S_OK);
	}
	if(cModifiedRows >= 3)
	{
		TESTC(FindValue(rghRow[ROW_THREE], cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(rghRow[ROW_THREE], &RowsetA.m_pData),S_OK);
	}	

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);
	PROVIDER_FREE(rgRowsUndone);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc General - Modified/Undo verify valid row handle, and OriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_36()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;
	
	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	pData = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Save OriginalData    
	TESTC_(RowsetA.GetRowData(rghRow[ROW_ONE],&pOriginalData),S_OK);

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
	cModifiedRows++;

	//Insert row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
		cModifiedRows++;
	}

	//Delete row(s)
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);
		cModifiedRows++;
	}
	
	//Save ModifiedData
	TESTC_(RowsetA.GetRowData(rghRow[ROW_ONE],&pModifiedData),S_OK);

	//Call Undo
	TESTC_(RowsetA.UndoRow(0,NULL,&cRowsUndone,&rgRowsUndone),S_OK);
	COMPC(cRowsUndone, cModifiedRows)

	//Since 0, NULL was passed the returned array is not required to be ordered
	//In the same order of the changes.
	TESTC(FindValue(rghRow[ROW_ONE], cRowsUndone, rgRowsUndone));
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pData),S_OK);

	if(cModifiedRows >= 2)
	{
		TESTC(FindValue(hNewRow, cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(hNewRow, &pData),DB_E_DELETEDROW);
	}
	if(cModifiedRows >= 3)
	{
		TESTC(FindValue(rghRow[ROW_THREE], cRowsUndone, rgRowsUndone));
		TESTC_(RowsetA.GetOriginalData(rghRow[ROW_THREE], &pData),S_OK);
	}

	//Verify modified/undo data
	TESTC(RowsetA.CompareRowData(rghRow[ROW_ONE],pOriginalData));
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_ONE],pModifiedData));

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	PROVIDER_FREE(rgRowsUndone);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc General - Verify Undo alters row handle refcount correctly
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUndo::Variation_37()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	HROW hNewRow = NULL;
	
	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Delete row(s)
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);
	cModifiedRows++;

	//Insert row(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.InsertRow(&hNewRow),S_OK);
		cModifiedRows++;
	}

	//Modify row(s)
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		cModifiedRows++;
	}
	
	//Release all the rows
	TESTC_(RowsetA.ReleaseRows(NROWS, rghRow, NULL, &rgRowStatus),S_OK)	//Will return DBROWSTATUS_S_PENDINGCHANGES;
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[ROW_ONE]		== (DBROWSTATUS)(cModifiedRows>=3 ? DBROWSTATUS_S_PENDINGCHANGES : DBROWSTATUS_S_OK));
	TESTC(rgRowStatus[ROW_TWO]		== DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[ROW_THREE]	== DBROWSTATUS_S_PENDINGCHANGES);
	
	if(hNewRow)
	{
		PROVIDER_FREE(rgRowStatus);
		TESTC_(RowsetA.ReleaseRows(ONE_ROW, &hNewRow, NULL, &rgRowStatus),S_OK)	//Will return DBROWSTATUS_S_PENDINGCHANGES;
		TESTC(rgRowStatus != NULL);
		TESTC(rgRowStatus[0]		== DBROWSTATUS_S_PENDINGCHANGES);
	}

	//Call Undo
	PROVIDER_FREE(rgRowStatus);
	TESTC_(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus),S_OK);
	COMPC(cRowsUndone, cModifiedRows)

	//Since 0, NULL was passed the returned array is not required to be ordered
	//In the same order of the changes.
	TESTC(FindValue(rghRow[ROW_THREE], cRowsUndone, rgRowsUndone));
	VerifyArray(cRowsUndone, rgRowStatus, DBROWSTATUS_S_OK);
	if(cModifiedRows >= 2)
		TESTC(FindValue(hNewRow, cRowsUndone, rgRowsUndone));
	if(cModifiedRows >= 3)
		TESTC(FindValue(rghRow[ROW_ONE], cRowsUndone, rgRowsUndone));
	
	//Release again, should be fine 
	TESTC_(RowsetA.ReleaseRows(cRowsUndone,rgRowsUndone),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	RowsetA.ReleaseRows(hNewRow);
	PROVIDER_FREE(rgRowsUndone);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUndo::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCUpdate)
//*-----------------------------------------------------------------------
//|	Test Case:		TCUpdate - IRowsetUpdate::Update test case
//|	Created:			03/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUpdate::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}

					   
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, 0, NULL,NULL,NULL,NULL,NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_1()
{
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	TESTC_(RowsetA.pIRowsetUpdate()->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes (NULL, 0, NULL, valid, valid, valid, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_2()
{
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	TESTC_(RowsetA.UpdateRow(0,NULL),S_OK);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (NULL, N, valid, NULL, NULL, valid, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_3()
{
	DBROWSTATUS* rgRowStatus = NULL;
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK);
	
	//Make changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)   //delete row 2;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK)  //modify row 4;
	if(RowsetA.AllowPendingRows(4))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)  //delete row 4;

	TESTC_(RowsetA.pIRowsetUpdate()->Update(NULL,NROWS,rghRow,NULL,NULL,&rgRowStatus),S_OK);
	TESTC(rgRowStatus != NULL);
	TESTC(VerifyArray(NROWS,rgRowStatus,DBROWSTATUS_S_OK));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (invalid, N, NULL, valid, valid, valid, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_4()
{
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS,rghRow),S_OK);
	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;

	TESTC_(RowsetA.pIRowsetUpdate()->Update(INVALID(HCHAPTER),NROWS,NULL,&cUpdatedRows,&rgUpdatedRows,&rgRowStatus),E_INVALIDARG);
	TESTC(cUpdatedRows==0 && rgUpdatedRows==NULL && rgRowStatus==NULL);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (invalid, N, valid, valid, NULL, valid, valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_5()
{											   
	TBEGIN
	HROW* rgUpdatedRows = INVALID(HROW*);
	DBROWSTATUS* rgRowStatus = INVALID(DBROWSTATUS*);

	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS,rghRow),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)    //delete row 2;
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK) //insert row 3;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK)  //delete row 3;
	}

	TESTC_(RowsetA.pIRowsetUpdate()->Update(NULL,0,NULL,NULL,&rgUpdatedRows,&rgRowStatus),S_OK);
	//rgUpdatedRows / rgRowStatus should just be ignored since, pcUpdaedRows==NULL
	TESTC(rgUpdatedRows==INVALID(HROW*) && rgRowStatus==INVALID(DBROWSTATUS*));
	        
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (NULL, 0, NULL, valid, valid, valid, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_6()
{
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	DBROWSTATUS* rgRowStatus = NULL;

	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
		
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS,rghRow),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK) //insert row 3;

	TESTC_(RowsetA.pIRowsetUpdate()->Update(NULL,0,NULL,&cUpdatedRows,NULL,&rgRowStatus),E_INVALIDARG);
	TESTC(cUpdatedRows==0 && rgRowStatus==NULL);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (NULL, 0, NULL, NULL, NULL, NULL, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_7()
{
	HROW hNewRow = NULL; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Make changes(s)
	TESTC_(RowsetA.InsertRow(&hNewRow),S_OK)  //insert row;
		
	TESTC_(RowsetA.UpdateRow(hNewRow),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hNewRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes (NULL, 0, NULL, valid, valid,valid,valid,
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_8()
{
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,TWO_ROWS,rghRow),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)     //modify row 1;
	cModifiedRows++;

	//Release some of the rows, so we know that the provider doesn't have a 
	//problem trying to update pending rows, that have been released...
	TESTC_(RowsetA.ReleaseRows(rghRow[ROW_ONE]),S_OK);
	RowsetA.ReleaseRows(rghRow[ROW_ONE]);

	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)     //delete row 2;
		TESTC_(RowsetA.ReleaseRows(rghRow[ROW_TWO]),S_OK);
		cModifiedRows++;	
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)   //insert row 4;
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)    //delete row 4;
	}

	//Update (0,NULL)
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==cModifiedRows);
	COMPC(rgUpdatedRows[0],rghRow[ROW_ONE]) 
	if(cModifiedRows >= 2)
		COMPC(rgUpdatedRows[1],rghRow[ROW_TWO]) 
	 
	
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);  
	PROVIDER_FREE(rgUpdatedRows);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Empty Rowset (NULL, 0, NULL, valid, valid,valid,valid,
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_9()
{
	DBCOUNTITEM cRowsUndone  = 1;
	HROW* rgRowsUndone = NULL;
	
	CRowsetUpdate EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	
	TESTC_(EmptyRowset.UpdateRow(0,NULL,&cRowsUndone,&rgRowsUndone),S_OK);
	TESTC(cRowsUndone==0 && rgRowsUndone==NULL);

CLEANUP:
	PROVIDER_FREE(rgRowsUndone)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_10()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 hard deleted / 2 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_11()
{					 
	const int NROWS = FIVE_ROWS;

	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBROWSTATUS* rgRowStatus = NULL;

	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save orginal data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_ONE],&pOriginalData),S_OK)    //save row 1 old data;

	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_TWO]),S_OK) //delete row 2;
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FOUR]),S_OK)//delete row 4;
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FIVE]),S_OK)//delete row 5;

	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)     //modify row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)   //modify row 3;

	//Save modified data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_ONE], &pModifiedData),S_OK)	//save row 1 new data;
			 
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[1]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[2]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[3]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[4]==DBROWSTATUS_E_DELETED);

	//Verify Update did update the data at the backend
//	TESTC(RowsetA.CompareTableData(FIRST_ROW, rghRow[ROW_ONE]));
//	TESTC(RowsetA.CompareTableData(THIRD_ROW, rghRow[ROW_THREE]));

	//Verify Update did update the data in the rowset 
	TESTC(RowsetA.CompareRowData(rghRow[ROW_ONE],pModifiedData));

	//Verify Updated data does not equal orginal data
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_ONE],pOriginalData));
	

CLEANUP:
 	RowsetA.ReleaseRows(NROWS,rghRow);

	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	TableInsert(THREE_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 invalid / 2 inserted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_12()
{
	const int NROWS = FIVE_ROWS;

	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBROWSTATUS* rgRowStatus = NULL;
	
	void* pOriginalData = NULL;
	void* pModifiedData = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	//Save orginal data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE],&pOriginalData),S_OK) //save row 3 old data;

	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FIVE]),S_OK)//delete row 5;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)     //modify row 3;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;

	//Save modified data
	TESTC_(RowsetA.GetRowData(rghRow[ROW_THREE],&pModifiedData),S_OK)	//save row 3 new data;
	
	//Update 
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[3]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[4]==DBROWSTATUS_E_DELETED);

	//Verify Update did update the data at the backend
//	TESTC(RowsetA.CompareTableData(FIRST_ROW, rghRow[ROW_ONE]));
//	TESTC(RowsetA.CompareTableData(THIRD_ROW, rghRow[ROW_THREE]));

	//Verify Update did update the data in the rowset 
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE],pModifiedData));

	//Verify Updated data does not equal orginal data
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_THREE],pOriginalData));
	

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);

	PROVIDER_FREE(rgRowStatus);

	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pModifiedData);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 hard deleted / 1 invalid / 1 soft-del
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_13()
{
	DBROWSTATUS* rgRowStatus = NULL;

	const int NROWS =  FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(SECOND_ROW, TWO_ROWS, rghRow),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK) //hard-delete row 1;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)     //soft-delete row 2;
	rghRow[ROW_THREE] = DB_NULL_HROW;					//invalid row 3 
	rghRow[ROW_FOUR]  = rghRow[ROW_ONE];				//duplicate row 4 == row 1

	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[3]==DBROWSTATUS_E_DELETED);
		
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 hard deleted / 1 invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_14()
{
	DBROWSTATUS* rgRowStatus = NULL;

	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(SECOND_ROW, &rghRow[ROW_ONE]),S_OK);
	rghRow[ROW_TWO] = DB_NULL_HROW;           //invalid row 2
		
	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK) //hard-delete row 1;
	 
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[1]==DBROWSTATUS_E_INVALID);
		
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);

	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 col with non-schema info
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_15()
{
	DBROWSTATUS* rgRowStatus = NULL;

	const int NROWS = ONE_ROW;
	HROW rghRow[NROWS] = {NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(SECOND_ROW, &rghRow[ROW_ONE]),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK) //hard-delete row 1;
	 
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,&rgRowStatus),DB_E_ERRORSOCCURRED);
		
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 col with metacolumn non-schema info
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_16()
{
	const int NROWS = ONE_ROW;
	HROW rghRow[NROWS] = {NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRow[ROW_ONE]),S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK) //hard-delete row 1;
	 
	TESTC_(RowsetA.UpdateRow(NROWS,rghRow,NULL,NULL,NULL),DB_E_ERRORSOCCURRED);
		
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 2 inserted rows, no value specified / not nullable
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_17()
{
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBCOUNTITEM cInsertedRows = 0;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_ONE]),S_OK);
	cInsertedRows++;
	
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK);
		cInsertedRows++;
	}
	
	//These are valid inserted rows, with defaults
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_TWO]),S_OK);
		cInsertedRows++;
	}
	if(RowsetA.AllowPendingRows(4))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK);
		cInsertedRows++;
	}

	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==cInsertedRows);
	TESTC(rgUpdatedRows[0]==rghRow[ROW_ONE]);
	if(cInsertedRows >= 2)
		TESTC(rgUpdatedRows[1]==rghRow[ROW_THREE]);
	if(cInsertedRows >= 3)
		TESTC(rgUpdatedRows[2]==rghRow[ROW_TWO]);
	if(cInsertedRows >= 4)
		TESTC(rgUpdatedRows[3]==rghRow[ROW_FOUR]);
			
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);

	PROVIDER_FREE(rgUpdatedRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_18()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Update a non-updateable column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_19()
{
	HACCESSOR hAccessorAllCol = NULL;
	DBLENGTH     cRowSize = 0;
	DBORDINAL     cAllBindings = 0;
	DBORDINAL     cUpdBindings = 0;
	HRESULT   hr = S_OK;

	HROW  hRow = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the hAccessor bindings for all col, even non-updateable columns  
	TESTC_(GetAccessorAndBindings(RowsetA(), DBACCESSOR_ROWDATA,
			&hAccessorAllCol, NULL, &cAllBindings, &cRowSize, DBPART_ALL, ALL_COLS_BOUND),S_OK);

	//Obtain the hAccessor bindings for all updateable columns  
	TESTC_(GetAccessorAndBindings(RowsetA(), DBACCESSOR_ROWDATA,
			NULL, NULL, &cUpdBindings, NULL, DBPART_ALL, UPDATEABLE_COLS_BOUND),S_OK);

	//If all columns are updateable, there is nothing to test for this variation
	if(cUpdBindings == cAllBindings)
		goto CLEANUP;

	//Alloc buffer large enough for all columns
	pData = PROVIDER_ALLOC(sizeof(void*)*cRowSize);

	//Get row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	//Get Data
	TESTC_(RowsetA()->GetData(hRow,hAccessorAllCol,pData),S_OK);
	//Release the row handle
	TESTC_(RowsetA.ReleaseRows(hRow),S_OK);

	//Insert a new row, containing non-updateable columns
	//Its provider specific wither can can insert any other the other columns
	hRow = NULL;
	hr = RowsetA.pIRowsetUpdate()->InsertRow(NULL,hAccessorAllCol,pData,&hRow);
	TEST2C_(hr, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);

	//Try Updating the row, containing non-updateable columns
	TESTC_(RowsetA.UpdateRow(ONE_ROW,&hRow,NULL,NULL,&rgRowStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	if(hr==DB_S_ERRORSOCCURRED)
	{
		//If we got a success code from InsertRow, then the hRow returned should be valid
		//And passing a valid hRow to Update should fail since not all are updatable
		TESTC(hRow != NULL);
		TESTC(rgRowStatus[0]==DBROWSTATUS_E_INTEGRITYVIOLATION);
	}
	else
	{
		//If we got a failure code from InsertRow, then the hRow returned should be NULL
		//And passing a NULL hRow to Update should fail
		TESTC(hRow == NULL);
		TESTC(rgRowStatus[0]==DBROWSTATUS_E_INVALID);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);

	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Delete a non-updateable column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_20()
{
	HACCESSOR hAccessorAllCol = NULL;
	DBLENGTH     cRowSize = 0;
	DBORDINAL     cAllBindings = 0;
	DBORDINAL     cUpdBindings = 0;
	HRESULT   hr = S_OK;

	HROW  rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBROWSTATUS* rgRowStatus = NULL;
	void* pData = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain the hAccessor bindings for all col, even non-updateable columns 
	TESTC_(GetAccessorAndBindings(RowsetA(), DBACCESSOR_ROWDATA,
			&hAccessorAllCol, NULL, &cAllBindings, &cRowSize, DBPART_ALL, ALL_COLS_BOUND),S_OK);

	//Obtain the hAccessor bindings for all updateable columns  
	TESTC_(GetAccessorAndBindings(RowsetA(), DBACCESSOR_ROWDATA,
			NULL, NULL, &cUpdBindings, NULL, DBPART_ALL, UPDATEABLE_COLS_BOUND),S_OK);

	//Depending upon the DataSource all columns might be updateable
	//If all columns are updateable, there is nothing to test for this variation
	if(cUpdBindings == cAllBindings)
		goto CLEANUP;

	//Alloc buffer large enough for all columns
	pData = PROVIDER_ALLOC(sizeof(void*)*cRowSize);

	//Get row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK);
	
	//Get Data
	TESTC_(RowsetA()->GetData(rghRow[ROW_TWO],hAccessorAllCol,pData),S_OK);
	//Insert a new row, containing non-updateable columns
	//Its provider specific wither can can insert any other the other columns
	hr = RowsetA.pIRowsetUpdate()->InsertRow(NULL,hAccessorAllCol,pData,&rghRow[ROW_FOUR]);
	TEST2C_(hr, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
	//Now update backend
	TESTC_(RowsetA.UpdateRow(rghRow[ROW_FOUR]),DB_E_ERRORSOCCURRED);

	//Try deleting non-updateable row
	TESTC_(RowsetA.pIRowsetUpdate()->DeleteRows(NULL,ONE_ROW,&rghRow[ROW_FOUR],NULL), hr==DB_S_ERRORSOCCURRED ? S_OK : DB_E_ERRORSOCCURRED);

	//Now Try Updating the rows, containing non-updateable columns
	TESTC_(RowsetA.UpdateRow(FOUR_ROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[1]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2]==DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[3]==(DBROWSTATUS)(hr==DB_S_ERRORSOCCURRED ? DBROWSTATUS_E_DELETED : DBROWSTATUS_E_INVALID));

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	RowsetA.ReleaseAccessor(hAccessorAllCol);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Deferred only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_21()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	void* pOriginalData = NULL;
	void* pBackEndData  = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_DEFERRED);
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Make changes to the backend
	//TODO

	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Verify Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(RowsetA.CompareRowData(&pOriginalData, &pBackEndData));
	
	//Make changes to the backend again
	//TODO

	//Verify Not Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(!RowsetA.CompareRowData(&pOriginalData, &pBackEndData));


CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pBackEndData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Cache-Deferred only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_22()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	void* pOriginalData = NULL;
	void* pBackEndData  = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_CACHEDEFERRED);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Make changes to the backend
	//TODO

	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Verify Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(RowsetA.CompareRowData(&pOriginalData, &pBackEndData));
	
	//Make changes to the backend again
	//TODO

	//Verify Not Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(!RowsetA.CompareRowData(&pOriginalData, &pBackEndData));


CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pBackEndData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Cache-Deferred / Deferred / Non-deferred col
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_23()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	
	void* pOriginalData = NULL;
	void* pBackEndData  = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_DEFERRED);
	RowsetA.SetSettableProperty(DBPROP_CACHEDEFERRED);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Make changes to the backend
	//TODO

	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Verify Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(RowsetA.CompareRowData(&pOriginalData, &pBackEndData));
	
	//Make changes to the backend again
	//TODO

	//Verify Not Seen with GetOriginalData
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_ONE], &pOriginalData),S_OK);
	//TESTC(!RowsetA.CompareRowData(&pOriginalData, &pBackEndData));


CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pBackEndData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_24()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Duplicate hrow entries in the array
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_25()
{
	TBEGIN
	HROW rghRow[FIVE_ROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	
	ULONG cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);
	rghRow[ROW_THREE] = rghRow[ROW_ONE];  // Duplicate hRow, hRow1 == hRow3 
	rghRow[ROW_FOUR]  = rghRow[ROW_TWO];  // Duplicate hRow, hRow2 == hRow4 
	
	//Now make some changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //delete row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)  //delete row 1 //Duplicate...;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)    //delete row 3;
	if(RowsetA.AllowPendingRows(4))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)   //delete row 3 //Duplicate...;
	if(RowsetA.AllowPendingRows(5))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FIVE]),S_OK)  //insert row 5;

	TESTC_(RowsetA.UpdateRow(FIVE_ROWS,rghRow), RowsetA.AllowPendingRows(5) ? S_OK : DB_S_ERRORSOCCURRED);

CLEANUP:
	RowsetA.ReleaseRows(FIVE_ROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Fetch/Modify/Release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_26()
{
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)			//modify row 1;
	cModifiedRows++;
		
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)		//delete row 2;
		cModifiedRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)	//insert row 4;
		cModifiedRows++;
	}

	//Now release handles
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);

	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==cModifiedRows && rgUpdatedRows!=NULL);
	COMPC(rgUpdatedRows[0],rghRow[ROW_ONE])
	if(cModifiedRows >=2 )
		COMPC(rgUpdatedRows[1],rghRow[ROW_TWO])
	if(cModifiedRows >=3 )
		COMPC(rgUpdatedRows[2],rghRow[ROW_FOUR])

					   
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Fetch/Modify/don't Release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_27()
{												
	TBEGIN
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;

	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL}; 
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //insert row 4;

	//Don't release handles
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==ONE_ROW);
	COMPC(rgUpdatedRows[0],rghRow[ROW_FOUR])

					   
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_28()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - SetPos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_29()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Delete a row (BLOB data)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//Before update, should not be equal
	TESTC(!RowsetA.CompareRowData(pData1, pModifyData, hAccessor));

	//Now call Update
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
	//After update, should be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData2, pModifyData, hAccessor));


CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Accessor  - BLOB / Long columns - QBU
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_30()
{
	TBEGIN
	TRETURN  //TODO need to work on blob support once supported

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL};
	HROW hNewRow = DB_NULL_HROW;
	DBLENGTH cRowSize = 0;

	void* pInsertData  = NULL;
	void* pModifyData  = NULL;

	void* pData1     = NULL;
	void* pData2     = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Create Accessor binding BLOB/Long data (last param TRUE)
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,&cRowSize,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pInsertData,hAccessor));
	TESTC(RowsetA.MakeRowData(&pModifyData,hAccessor));
	pData1 = PROVIDER_ALLOC(sizeof(pInsertData));
	pData2 = PROVIDER_ALLOC(sizeof(pInsertData));

	//Get Rows
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_TWO]),S_OK);

	//Get the Data
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData1),S_OK);

	//Modify a row (BLOB data)
	TESTC_(RowsetA.pIRowsetChange()->SetData(rghRow[ROW_ONE],hAccessor, pModifyData),S_OK);
	//Delete a row (BLOB data)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);
	//Insert a row (BLOB data)
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.pIRowsetChange()->InsertRow(NULL,hAccessor, pInsertData, &hNewRow),S_OK);

	//Before update, should not be equal
	TESTC(!RowsetA.CompareRowData(pData1, pModifyData, hAccessor));

	//Now call Update
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
	//After update, should be equal
	TESTC_(RowsetA.pIRowset()->GetData(rghRow[ROW_ONE], hAccessor, pData2),S_OK);
	TESTC(RowsetA.CompareRowData(pData2, pModifyData, hAccessor));


CLEANUP:
	//Free Data
	RowsetA.ReleaseRowData(pInsertData,hAccessor);
	RowsetA.ReleaseRowData(pModifyData,hAccessor);

	//Release the Accesssor
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hNewRow);
	RowsetA.ReleaseRows(NROWS, rghRow);
	
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_31()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Related - Resync / GetOriginalData same data
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_32()
{
	TBEGIN
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL}; 
	void* pOriginalData = NULL;
	void* pResynchData  = NULL;
	IRowsetResynch* pIRowsetResynch = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);//Requires RESYNCH 
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Alloc pData
	pResynchData = PROVIDER_ALLOC(sizeof(void*)*RowsetA.m_cRowSize);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)  //modify row 3;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //insert row 4;

	//Call GetOriginalData for row 3
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_THREE], &pOriginalData),S_OK);

	//Obtain the Resynch Interface
	TESTC_(QI(RowsetA(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK);

	//Call Resynch::GetVisisableData for Row 3  
	TESTC_(pIRowsetResynch->GetVisibleData(rghRow[ROW_THREE],RowsetA.m_hAccessor,pResynchData),S_OK);

	//Verify GetOriginalData == GetVisibleData
	TESTC(RowsetA.CompareRowData(pOriginalData, pResynchData));

	//Call Update
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//Verify GetOriginalData == GetVisibleData after update
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.GetOriginalData(rghRow[ROW_TWO], &pOriginalData),S_OK);
		TESTC_(pIRowsetResynch->GetVisibleData(rghRow[ROW_TWO],RowsetA.m_hAccessor,pResynchData),S_OK);
		TESTC(RowsetA.CompareRowData(pOriginalData, pResynchData));
	}
	
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	SAFE_RELEASE(pIRowsetResynch);
	PROVIDER_FREE(pResynchData);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Related - ResynchRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_33()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	void* pOriginalData = NULL;
	IRowsetResynch* pIRowsetResynch = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);//Requires RESYNCH 
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)  //modify row 3;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //insert row 4;

	//Call GetOriginalData for row 3
	TESTC_(RowsetA.GetOriginalData(rghRow[ROW_THREE], &pOriginalData),S_OK);

	//Verify GetOriginalData != Rowset after changes
	TESTC(!RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData));

	//Obtain the Resynch Interface
	TESTC_(QI(RowsetA(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK);

	//Call Resynch::ResynchRows for all rows 
	TESTC_(pIRowsetResynch->ResynchRows(0,NULL,NULL,NULL,NULL),DB_S_ERRORSOCCURRED);

	//Verify GetOriginalData == GetVisibleData
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData));

	//Call Update
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//Verify GetOriginalData == GetVisibleData after update
	TESTC(RowsetA.CompareRowData(rghRow[ROW_THREE], pOriginalData));

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	SAFE_RELEASE(pIRowsetResynch);
	PROVIDER_FREE(pOriginalData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Related - Modify / Resync 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_34()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 
	void* pOriginalData = NULL;
	IRowsetResynch* pIRowsetResynch = NULL;

	DBCOUNTITEM cRowsResynched = 1;
	HROW* rgRowsResynched = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch); //Requires RESYNCH 
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)		//insert row 4;
	cModifiedRows++;
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)	//modify row 3;
		cModifiedRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)			//modify row 1;
		cModifiedRows++;
	}

	//GetPendingRows == 3
	TESTC_(RowsetA.GetPendingRows(cModifiedRows),S_OK);

	//Obtain the Resynch Interface
	TESTC_(QI(RowsetA(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK);

	//Call Resynch::ResynchRows for all rows
	//Should return DB_S for newly inserted rows
	TESTC_(pIRowsetResynch->ResynchRows(0,NULL,&cRowsResynched,&rgRowsResynched,&rgRowStatus),DB_S_ERRORSOCCURRED);

	//Verify Resynch output params
	TESTC(cRowsResynched==FOUR_ROWS) //All "Active" rows;
	TESTC(rgRowsResynched!=NULL && rgRowStatus!=NULL);
	
	//ResynchRows resycnhs all active rows, so any retrieved or intered row is game
	//Thats why even though only 3 rows have been changed, 4 are returned...
	TESTC(rgRowsResynched[3] == rghRow[ROW_FOUR]);
	TESTC(rgRowStatus[3] == DBROWSTATUS_E_PENDINGINSERT);
	if(cModifiedRows >= 2)
	{
		TESTC(rgRowsResynched[1] == rghRow[ROW_TWO]);
		TESTC(rgRowStatus[1] == DBROWSTATUS_S_OK);
	}
	if(cModifiedRows >= 3)
	{
		TESTC(rgRowsResynched[2] == rghRow[ROW_THREE]);
		TESTC(rgRowStatus[2] == DBROWSTATUS_S_OK);
	}
	if(cModifiedRows >= 4)
	{
		TESTC(rgRowsResynched[0] == rghRow[ROW_ONE]);
		TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);
	}


	//GetPendingRows, should be ONE after resync, there was a newly inserted row
	TESTC_(RowsetA.GetPendingRows(ONE_ROW),S_OK);

	//Update All, no updates
	TESTC_(RowsetA.UpdateAll(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	RowsetA.ReleaseRows(cRowsResynched,rgRowsResynched);
	SAFE_RELEASE(pIRowsetResynch);
	PROVIDER_FREE(pOriginalData);

	PROVIDER_FREE(rgRowsResynched);
	PROVIDER_FREE(rgRowStatus);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Related - Modify / Resync / Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_35()
{
	TBEGIN
	const int NROWS = ONE_ROW;
	HROW rghRow[NROWS] = {NULL}; 
	IRowsetResynch* pIRowsetResynch = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);//Requires RESYNCH 
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, ONE_ROW, rghRow),S_OK);
	
	//Now make some changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)  //insert row 4;

	//GetPendingRows == 1
	TESTC_(RowsetA.GetPendingRows(ONE_ROW),S_OK);

	//Obtain the Resynch Interface
	TESTC_(QI(RowsetA(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK);

	//Call Resynch::ResynchRows for all rows
	//Should return DB_S for newly inserted rows
	TESTC_(pIRowsetResynch->ResynchRows(0,NULL,NULL,NULL,NULL),DB_S_ERRORSOCCURRED);
	
	//GetPendingRows, should be ONE after resync, there was a newly inserted row
	TESTC_(RowsetA.GetPendingRows(ONE_ROW),S_OK);

	//Undo all
	TESTC_(RowsetA.UndoAll(),S_OK);

	//GetPendingRows
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);
	
	//Update All
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//GetPendingRows
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	SAFE_RELEASE(pIRowsetResynch);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_36()
{
	return TEST_PASS;
}
// }}

// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Properties - NONE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_37()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set no OWN/OTHER properties
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
		
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNINSERT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_38()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OWNINSERT properties
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
 	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNINSERT & CANSCROLL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_39()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OWNINSERTT & CANSCROLL properties
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetA.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetB.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNUPDATEDELETE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_40()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OWNUPDATEDELETE properties
	RowsetA.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNUPDATEDELETE & CANFETCH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_41()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OWNUPDATEDELETE & CANFETCH properties
	RowsetA.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	RowsetA.SetSettableProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	RowsetB.SetSettableProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERINSERT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_42()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERINSERT properties
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERINSERT & CANSCROLL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_43()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERINSERT &	CANSCROLL properties
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetA.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetB.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	RowsetB.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERUPDATEDELETE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_44()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERUPDATEDELETE properties
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERUPDATEDELETE & CANSCROLL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_45()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERUPDATEDELETE & CANSCROLL properties
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetB.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	RowsetB.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNUPDATEDELETE & OTHERINSERT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_46()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERUPDATEDELETE & OTHRINSERT properties
	RowsetA.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Properties - OWNUPDATEDELETE & OTHERINSERT & CANFETCH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_47()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OWNUPDATEDELETE & OTHRINSERT & CANFETCH properties
	RowsetA.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetA.SetSettableProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OWNUPDATEDELETE);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetB.SetSettableProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERUPDATEDELETE & OWNINSERT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_48()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERUPDATEDELETE & OWNINSERT properties
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Properties - OTHERUPDATEDELETE & OWNINSERT & CANSCROLL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_49()
{
	TBEGIN
	HROW hNewRowA  = NULL;
	HROW hFirstRowA= NULL;

	HROW hNewRowB  = NULL;
	HROW hFirstRowB= NULL;
	
	CRowsetUpdate RowsetA, RowsetB;
	
	//Set OTHERUPDATEDELETE & OWNINSERT & CANSCROLL properties
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	if(!MSDASQL) RowsetA.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetA.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	RowsetB.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	if(!MSDASQL) RowsetB.SetSettableProperty(DBPROP_OTHERINSERT);
	RowsetB.SetSettableProperty(DBPROP_CANSCROLLBACKWARDS);
	RowsetB.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetB.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//2 Rowsets, A and B
	//A fetches/deletes the first row
	//B modifies last row
	//A inserts a new row
	//B inserts a new row, Updates
	//A Updates

	//A fetches and deletes the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hFirstRowA),S_OK);
	TESTC_(RowsetA.DeleteRow(hFirstRowA),S_OK);
	TESTC_(RowsetA.ReleaseRows(hFirstRowA),S_OK); //Will Return DBROWSTATUS_S_PENDINGCHANGES;

	//B modifies the first row, make sure no conflict between A's row
	TESTC_(RowsetB.GetRow(FIRST_ROW,&hFirstRowB),S_OK);
	TESTC_(RowsetB.DeleteRow(hFirstRowB),S_OK);
	
	//A inserts a new row
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&hNewRowA),S_OK);

	//B inserts a new row, Updates
	if(RowsetB.AllowPendingRows(2))
		TESTC_(RowsetB.InsertRow(&hNewRowB),S_OK);
	TESTC_(RowsetB.UndoRow(hFirstRowB),S_OK);
	TESTC_(RowsetB.UpdateAll(),S_OK);
	
	//A Updates, row should already been deleted
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hNewRowA);
	RowsetA.ReleaseRows(hFirstRowA);
	RowsetB.ReleaseRows(hNewRowB);
	RowsetB.ReleaseRows(hFirstRowB);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Properties - verify property dependencies
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_50()
{
	TBEGIN
	CRowset RowsetA;

	//Set Properties
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	RowsetA.SetProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetProperty(DBPROP_OWNUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Just as a sanity check, make sure IID_IRowseUpdate is still set  
	TESTC(RowsetA.GetProperty(DBPROP_IRowsetUpdate));

	//Verify IID_IRowsetChange is implicitly set 
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_IRowsetChange));

	//Just as a sanity check, make sure still set  
	TESTC(RowsetA.GetProperty(DBPROP_OTHERUPDATEDELETE));

	//Just as a sanity check, make sure still set  
	TESTC(RowsetA.GetProperty(DBPROP_OWNUPDATEDELETE));
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_51()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Related - Qualified Table Name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_52()
{
	CRowsetUpdate RowsetA;
	HROW hRow = DB_NULL_HROW;

    //Use the Qualified table name,
    TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the row handle
    TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

    //Make change(s)
    TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	    
    //Update the row
    TESTC_(RowsetA.UpdateRow(hRow),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_53()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Properties - MAXPENDINGCHANGEROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_54()
{
	TBEGIN
	const ULONG NROWS = 7;
	HROW rghRow[NROWS];
	HROW rghNewRow[NROWS];

	ULONG_PTR ulMaxPendingRows = 0;
	ULONG_PTR i, ulMaxOpenRows = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	RowsetA.GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, &ulMaxPendingRows);
	RowsetA.GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &ulMaxOpenRows);
	TESTC_PROVIDER(RowsetA.AllowPendingRows(NROWS));

	//Besides just stress testing the number of pendingrows and open rows,
	//I'm also going to stress test the internal imp of the update array.
	//Their keeping an internal array of the modified rows, and upon deleting
	//some they will compact the array.  So this should be stress tested here...

	//Grab all the rows of the table, starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	
	//Modify every even row
	for(i=0; i<NROWS; i+=2)
		TESTC_(RowsetA.ModifyRow(rghRow[i]),S_OK);
	//GetPendingRows, should be half pending
	TESTC_(RowsetA.GetPendingRows(NROWS/2+1),S_OK);

	//Update just those even rows, should be no change
	for(i=0; i<NROWS; i+=2)
		TESTC_(RowsetA.UpdateRow(rghRow[i]),S_OK);
	//GetPendingRows, should be no pending
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);
	
	//Now delete all the odd rows
	for(i=1; i<NROWS; i+=2)
		TESTC_(RowsetA.DeleteRow(rghRow[i]),S_OK);
	//GetPendingRows, should be half pending
	TESTC_(RowsetA.GetPendingRows(NROWS/2),S_OK);

	//Now Undo all the odd rows
	for(i=1; i<NROWS; i+=2)
		TESTC_(RowsetA.UndoRow(rghRow[i]),S_OK);
	//GetPendingRows, should be no pending
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

	
	//Now Insert a bunch of rows
	for(i=0; i<NROWS; i++)
		TESTC_(RowsetA.InsertRow(&rghNewRow[i]),S_OK);

	//Now Modify all the odd inserted rows
	for(i=1; i<NROWS; i+=2)
		TESTC_(RowsetA.ModifyRow(rghNewRow[i]),S_OK);
	//GetPendingRows, should be all pending
	TESTC_(RowsetA.GetPendingRows(NROWS),S_OK);

	//Now delete all the even inserted rows
	for(i=0; i<NROWS; i+=2)
		TESTC_(RowsetA.DeleteRow(rghNewRow[i]),S_OK);
	//GetPendingRows, should be all (half modified / half deleted)
	TESTC_(RowsetA.GetPendingRows(NROWS/2),S_OK);

	//Now Update all odd 
	for(i=1; i<NROWS; i+=2)
		TESTC_(RowsetA.UpdateRow(rghNewRow[i]),S_OK);
	//GetPendingRows, should be half pending
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

	//Update all
	TESTC_(RowsetA.UpdateAll(),S_OK);
	//GetPendingRows, should be no pending
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	RowsetA.ReleaseRows(NROWS, rghNewRow);
	TableInsert(NROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Properties - CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_55()
{
	TBEGIN
	HROW rghRow[TWO_ROWS] = {NULL,NULL};

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Fetch/modify row 1
	//Fetch last row
	//Delete row 1 and Undo
	//Update all

	//Fetch Row 1, and modify
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);

	//Fetch Next row
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_TWO]),S_OK);
	
	//Delete Row 1, and Undo
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.UndoRow(rghRow[ROW_ONE]),S_OK);

	//Modify other row and release
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
	TESTC_(RowsetA.ReleaseRows(rghRow[ROW_ONE]),S_OK);

	//Update all
	TESTC_(RowsetA.UpdateAll(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Properties - ~CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_56()
{
	DBCOUNTITEM cRowsObtained=1;
	HROW* rgRowsObtained = NULL;
	HRESULT hr = S_OK;

	HROW hFirstRow = NULL;
	HROW hLastRow  = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET,(void*)VARIANT_FALSE,DBTYPE_BOOL);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Setting CANHOLROWS=FALSE, According to the spec indicates that holding rows may or 
	//may not be allowed, since you didn't request the ability to hold rows.  Prevents the 
	//provider from having artifical limiting functionality.

	//Verify ~CANHOLDROWS
	TESTC(!RowsetA.GetProperty(DBPROP_CANHOLDROWS));

	//Fetch Row 1, and modify
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hFirstRow),S_OK);
	TESTC_(RowsetA.ModifyRow(hFirstRow),S_OK);

	//Fetch Second row, "may" cause an error
	TEST3C_(hr = RowsetA()->GetNextRows(NULL, SECOND_ROW, ONE_ROW, &cRowsObtained, &rgRowsObtained), S_OK, DB_S_ROWLIMITEXCEEDED, DB_E_ROWSNOTRELEASED);
	
	if(hr == S_OK)
	{
		//Able to hold row when CANHOLDROWS = FALSE
		TESTC(cRowsObtained==1 && rgRowsObtained!=NULL && rgRowsObtained[0]!=NULL);
	}
	else if(hr == DB_S_ROWLIMITEXCEEDED)
	{
		//Unable to have more than 1 row open
		TESTC(cRowsObtained==0 && rgRowsObtained==NULL);
		TESTC(RowsetA.m_ulMaxOpenRows == 1);
	}
	else
	{
		//Unable to hold rows when CANHOLDROWS = FALSE
		TESTC(cRowsObtained==0 && rgRowsObtained==NULL);
	}
	
CLEANUP:
	RowsetA.ReleaseRows(hFirstRow);
	RowsetA.ReleaseRows(hLastRow);
	RowsetA.ReleaseRows(cRowsObtained, rgRowsObtained);
	PROVIDER_FREE(rgRowsObtained);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Properties - BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_57()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_58()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Insert/Commit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_59()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Insert/Commit/Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_60()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Modify/Update/Commit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_61()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Modify/Update/Commit/Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_62()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Delete/Update/Abort
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_63()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Delete/Update/Abort/Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_64()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Transactions - concurrency - DB_E_CONCURRENCYVILOATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_65()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Transactions - ABORTRETAINING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_66()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Transactions - ~ABORTRETAINING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_67()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Transactions - COMMITRETAINING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_68()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Transactions - ~COMMITRETAINING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_69()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Transactions - NOTENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_70()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_71()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert/Modify/Delete a single row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_72()
{
	HROW hRow = DB_NULL_HROW; 
	
	DBCOUNTITEM cPendingRows = 1;
	HROW* rgPendingRows;
	DBROWSTATUS* rgPendingStatus = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Make changes(s)
	TESTC_(RowsetA.InsertRow(&hRow),S_OK) //insert row X;
	TESTC_(RowsetA.ModifyRow(hRow),S_OK)  //modify row X;
	TESTC_(RowsetA.DeleteRow(hRow),S_OK)  //delete row X;
	
	//inv: Should be no changes that need updating.
	TESTC_(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE);
	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);

	//inv: Should not be any updates to the backend
	TESTC_(RowsetA.UpdateRow(ONE_ROW, &hRow, NULL, NULL, &rgRowStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0] == DBROWSTATUS_E_DELETED);
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(rgPendingStatus);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Delete all rows/Undo 1/Modify 1/Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_73()
{
	const ULONG NROWS = 7;
	HROW rghRow[NROWS];
	DBCOUNTITEM cModifiedRows = 0;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Grab all the rows in the table, starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);
	cModifiedRows = RowsetA.GetMaxPendingRows();
	if(cModifiedRows==0 || cModifiedRows>NROWS)
		cModifiedRows = NROWS;
	
	//Delete all the rows in the table
	TESTC_(RowsetA.DeleteRow(cModifiedRows,rghRow),S_OK);
	
	//GetPendingRows, all rows should have pending changes
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,cModifiedRows),S_OK);
	
	//Now undo last row
	TESTC_(RowsetA.UndoRow(rghRow[cModifiedRows-1]),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,cModifiedRows-1), (cModifiedRows-1) ? S_OK : S_FALSE);
	
	//Now Delete that row
	TESTC_(RowsetA.DeleteRow(rghRow[cModifiedRows-1]),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,cModifiedRows), S_OK);

	//Call update
	TESTC_(RowsetA.UpdateRow(cModifiedRows,rghRow),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TableInsert(NROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Sequence - 1 Row rowset/Delete row/Undo/Delete/Update/Undo/Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_74()
{
	HROW hRow = DB_NULL_HROW;
	CRowsetUpdate RowsetA;

	//Create a rowset from a 1 row table
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable)==S_OK);
		
	//Grab the only row in the table, row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Delete the row in the table
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//GetPendingRows, all rows should have pending changes
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,ONE_ROW),S_OK);
	
	//Now undo last row
	TESTC_(RowsetA.UndoRow(hRow),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,NO_ROWS),S_FALSE);
	
	//Now Delete that row
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED,ONE_ROW),S_OK);

	//Call update
	TESTC_(RowsetA.UpdateRow(hRow),S_OK);
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert 3 rows/Undo all/Insert 4/Modify
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_75()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	DBROWSTATUS* rgRowStatus = NULL;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	TESTC_PROVIDER(RowsetA.AllowPendingRows(NROWS));
	
 	//Insert 3 rows into the table
	TESTC_(RowsetA.InsertRow(THREE_ROWS, rghRow),S_OK);
	
	//Undo all of them
	TESTC_(RowsetA.UndoRow(THREE_ROWS, rghRow),S_OK);

	//Insert 4 rows into the table
	TESTC_(RowsetA.InsertRow(FOUR_ROWS, rghRow),S_OK);

	//Modify those rows
	TESTC_(RowsetA.ModifyRow(FOUR_ROWS, rghRow),S_OK);

	//Update 2 of them
 	TESTC_(RowsetA.UpdateRow(rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.UpdateRow(rghRow[ROW_TWO]),S_OK);

	//InsertRow->SetData - is still considered a NEW Row according to the spec
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_NEW,TWO_ROWS),S_OK);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_CHANGED, NO_ROWS),S_FALSE);
	TESTC_(RowsetA.GetPendingRows(DBPENDINGSTATUS_DELETED, NO_ROWS),S_FALSE);
	
	//Undo the other two
	TESTC_(RowsetA.UndoRow(rghRow[ROW_THREE]),S_OK);
	TESTC_(RowsetA.UndoRow(rghRow[ROW_FOUR]),S_OK);
	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

	//Update All, shouldn't be any updates, row 3, row 4 are invalid
	TESTC_(RowsetA.UpdateRow(FOUR_ROWS,rghRow,NULL,NULL,&rgRowStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus != NULL);
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[1] == DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2] == DBROWSTATUS_E_DELETED);
	TESTC(rgRowStatus[3] == DBROWSTATUS_E_DELETED);

	TESTC_(RowsetA.GetPendingRows(NO_ROWS),S_FALSE);

									   	
CLEANUP:
	PROVIDER_FREE(rgRowStatus);
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_76()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call Update 3 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_77()
{
	TBEGIN
	const int NROWS = SIX_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL};
	DBCOUNTITEM cModifiedRows = 0;
	
	DBCOUNTITEM cUpdatedRows = 0;
	HROW* rgUpdatedRows = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//NOTE:  
	//With this variation (mutliple Updates) it is important that 
	//we don't modify the same row twice in different updates
	//Since I have bound the index column, modifiying the data will cause
	//the row to be repositioned, (SQLServer) will be unable to reposition on 
	//that row...

	//Obtain row handle(s)	
	TESTC_(RowsetA.GetRow(SECOND_ROW, &rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_TWO]),S_OK);
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_THREE]),S_OK);
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_FOUR]),S_OK);
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FIVE]),S_OK);
	cModifiedRows++;

	//Make change(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
		cModifiedRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK);
		cModifiedRows++;
	}
	
	//Update #1
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==cModifiedRows);
	
	//Since 0,NULL was passed in, you can't assume the rows returned 
	// Are in the same order as modifed...
	TESTC(FindValue(rghRow[ROW_FIVE], cUpdatedRows, rgUpdatedRows));
	if(cModifiedRows >=2)
		TESTC(FindValue(rghRow[ROW_TWO], cUpdatedRows, rgUpdatedRows));
	if(cModifiedRows >=3)
		TESTC(FindValue(rghRow[ROW_FOUR], cUpdatedRows, rgUpdatedRows));
	PROVIDER_FREE(rgUpdatedRows);
	
	//Update #2
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==NO_ROWS);
	
	//Make a few of changes, to the previously updated rows
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_SIX]),S_OK);
	cModifiedRows = 1;
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		cModifiedRows++;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);
		cModifiedRows++;
	}

	//Update #3
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC(cUpdatedRows==cModifiedRows);

	//Since 0,NULL was passed in, you can't assume the rows returned 
	//Are in the same order as modifed...
	TESTC(FindValue(rghRow[ROW_SIX], cUpdatedRows, rgUpdatedRows));
	if(RowsetA.AllowPendingRows(2))
		TESTC(FindValue(rghRow[ROW_ONE], cUpdatedRows, rgUpdatedRows));
	if(RowsetA.AllowPendingRows(3))
		TESTC(FindValue(rghRow[ROW_THREE], cUpdatedRows, rgUpdatedRows));

CLEANUP:
	RowsetA.ReleaseRows(SIX_ROWS, rghRow);	
	PROVIDER_FREE(rgUpdatedRows);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call Update on seperate columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_78()
{
	TBEGIN
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Update multilple rows, covering all types
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_79()
{
	TBEGIN
	const int NROWS = FOURTEEN_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}; 
	
	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	TESTC_PROVIDER(RowsetA.AllowPendingRows(NROWS));
	
	//Row types
	// 1. DELETEDROW   (hard-deleted)			g e
	// 2. SOFT-DELETED							g u
	// 3. MODIFIED								g u
	// 4. MODIFIED/DELETED						g u
	// 5. MODIFIED/MODIFIED						g u
	// 6. UNDONE ROW							g  
	// 7. BADROWHANDLE (invalid)				  e
	// 8. INSERTED								i u
	// 9. INSERTED/MODIFIED						i u
	//10. INSERTED/MODIFIED/DELETED				i u
	//11. INSERTED/DELETED						i u
	//12. INSERTED/MODFIFIED/MODIFIED			i u
	//13. INSERTED/MODFIFIED/MODIFIED/DELETED	i u
	//14. UNCHANGED								g

	//g - indicates which rows require fetching/getting
	//i - indicates which rows require inserting
	//e - indicates which rows will produce an error
	//u - indicates which rows should be reported as updated

	//Obtain handle(s) of fetched rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, SIX_ROWS, rghRow),S_OK) //rows 1-6;
	
	//Now obtain other handles
	rghRow[ROW_SEVEN] = DB_NULL_HROW;			//row  7
	
	//Now obtain the inserted row handles
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_EIGHT]),S_OK)	//row  8;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_NINE]),S_OK)	    //row  9;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_TEN]),S_OK)	    //row 10;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_ELEVEN]),S_OK)	//row 11;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_TWELVE]),S_OK)	//row 12;
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THIRTEEN]),S_OK)	//row 13;

	//Now modify to get the above descriptions
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK)  //hard-delete row 1;
	
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)      //soft-delete row 2;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)    //modify row 3;

	TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK)     //modify row 4;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]),S_OK)     //delete row 4;

	TESTC_(RowsetA.ModifyRow(rghRow[ROW_FIVE]),S_OK)     //modify row 5;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_FIVE]),S_OK)     //modify row 5;

	TESTC_(RowsetA.ModifyRow(rghRow[ROW_SIX]),S_OK)      //modify row 6;
	TESTC_(RowsetA.UndoRow(rghRow[ROW_SIX]),S_OK)  //undo row 6;

	// row 7 is already bad
	// row 8 is already inserted
											     
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_NINE]),S_OK)     //modify row 9;

	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TEN]),S_OK)      //modify row 10;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TEN]),S_OK)      //delete row 10;

	TESTC_(RowsetA.DeleteRow(rghRow[ROW_ELEVEN]),S_OK)   //delete row 11;

	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWELVE]),S_OK)   //modify row 12;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWELVE]),S_OK)   //modify row 12;
	
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THIRTEEN]),S_OK) //modify row 13;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THIRTEEN]),S_OK) //modify row 13;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THIRTEEN]),S_OK) //delete row 13;

	//Now update the beast...	
	TESTC_(RowsetA.UpdateRow(0,NULL,&cUpdatedRows,&rgUpdatedRows),S_OK);
	TESTC_(RowsetA.ReleaseRows(cUpdatedRows,rgUpdatedRows),S_OK);

	//Verify array
	TESTC(cUpdatedRows==SEVEN_ROWS);

	//Since 0,NULL was passed in, you can't assume the rows returned 
	//Are in the same order as modifed...
	TESTC(FindValue(rghRow[ROW_EIGHT], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_NINE], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_TWELVE], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_TWO], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_THREE], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_FOUR], cUpdatedRows, rgUpdatedRows));
	TESTC(FindValue(rghRow[ROW_FIVE], cUpdatedRows, rgUpdatedRows));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);  
	PROVIDER_FREE(rgUpdatedRows);
	TableInsert(SIX_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple Table Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUpdate::Variation_80()
{
	TBEGIN
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUpdate::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRowStatus)
//*-----------------------------------------------------------------------
//|	Test Case:		TCGetRowStatus - IRowsetUpdate::GetRowStatus
//|	Created:			04/16/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowStatus::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify GetRowStatus alters row handles RefCount correctly
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_1()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBROWSTATUS rgPendingStatus[1] = {ULONG_MAX};

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
    //Release rows
    TESTC_(RowsetA.ReleaseRows(FOUR_ROWS,rghRow),S_OK);

	//GetRowStatus [invalid, 0, NULL, valid], should no nothing
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER),0,NULL,rgPendingStatus),S_OK);
    TESTC(rgPendingStatus[0] == ULONG_MAX);
	
    //Release again, should cause error
    TESTC_(RowsetA.ReleaseRows(FOUR_ROWS,rghRow),DB_E_ERRORSOCCURRED);


CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [invalid, 0, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_2()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBROWSTATUS rgPendingStatus[1] = {ULONG_MAX};

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//GetRowStatus [invalid, 0, NULL, NULL], do nothing
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER), 0, NULL, NULL),S_OK);

	//GetRowStatus [invalid, 0, NULL, valid], should ignore rgPendingStatus
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER), 0, NULL, rgPendingStatus),S_OK);
	TESTC(rgPendingStatus[0] == ULONG_MAX);

	//GetRowStatus [invalid, 0, NULL, valid], should ignore rghRows and do nothing
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER), 0, rghRow, NULL), S_OK);

	//GetRowStatus [invalid, 0, valid, valid], should ignore rgPendingStatus
	rgPendingStatus[0] = ULONG_MAX;
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER), 0, rghRow, rgPendingStatus), S_OK);
	TESTC(rgPendingStatus[0] == ULONG_MAX);

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, 1, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_3()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//GetRowStatus
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, ONE_ROW, rghRow, NULL),E_INVALIDARG);
	
CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Changes [NULL, 1, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_4()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBROWSTATUS rgPendingStatus[1] = {ULONG_MAX};

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//GetRowStatus
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, ONE_ROW, NULL, rgPendingStatus),E_INVALIDARG);
	TESTC(rgPendingStatus[0] == ULONG_MAX);

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_5()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_6()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, 0, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_7()
{
	HROW rghRow[SIX_ROWS] = {NULL,NULL,NULL,NULL,NULL,NULL};
	DBROWSTATUS rgPendingStatus[1] = {ULONG_MAX};

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//Modify row(s)
 	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
 	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK);

	//Insert row(s)
 	if(RowsetA.AllowPendingRows(3))
	 	TESTC_(RowsetA.InsertRow(&rghRow[ROW_SIX]),S_OK);
 	if(RowsetA.AllowPendingRows(4))
	 	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FIVE]),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,0,NULL,NULL),S_OK);
	 
CLEANUP:
	RowsetA.ReleaseRows(SIX_ROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, N, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_8()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[FOUR_ROWS] = {ULONG_MAX,ULONG_MAX,ULONG_MAX,ULONG_MAX};

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//Modify row(s)
 	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);
 	if(RowsetA.AllowPendingRows(2))
	 	TESTC_(RowsetA.ModifyRow(rghRow[ROW_FOUR]),S_OK);

	//Delete row(s)
 	if(RowsetA.AllowPendingRows(3))
	 	TESTC_(RowsetA.DeleteRow(rghRow[ROW_ONE]),S_OK);

	//GetRowStatus (N, NULL, NULL) - E_INVALIDARG
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, FOUR_ROWS, NULL, NULL), E_INVALIDARG);

	//GetRowStatus (N, NULL, valid) - E_INVALIDARG
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, FOUR_ROWS, NULL, rgPendingStatus), E_INVALIDARG);
	TESTC(VerifyArray(FOUR_ROWS, rgPendingStatus, ULONG_MAX));

	//GetRowStatus (N, valid, NULL) - E_INVALIDARG
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, FOUR_ROWS, rghRow, NULL), E_INVALIDARG);

	//GetRowStatus (N, valid, valid) - S_OK
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL, FOUR_ROWS, rghRow, rgPendingStatus), S_OK);
	COMPC(rgPendingStatus[ROW_ONE],		(DBPENDINGSTATUS)(RowsetA.AllowPendingRows(3) ? DBPENDINGSTATUS_DELETED : DBPENDINGSTATUS_UNCHANGED));
	COMPC(rgPendingStatus[ROW_TWO],		DBPENDINGSTATUS_CHANGED);
	COMPC(rgPendingStatus[ROW_THREE],	DBPENDINGSTATUS_UNCHANGED);
	COMPC(rgPendingStatus[ROW_FOUR],	(DBPENDINGSTATUS)(RowsetA.AllowPendingRows(2) ? DBPENDINGSTATUS_CHANGED : DBPENDINGSTATUS_UNCHANGED));

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - N Changes [NULL, N, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_9()
{
	const int NROWS = SIX_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	ULONG cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);
	
	//Modify row(s)
 	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK);	//modify row 2
 	cModifiedRows = 1;
	
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK);	//modify row 3
	 	cModifiedRows++;
	}

	//Delete row(s)
 	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FIVE]),S_OK);	//insert row 5
	 	cModifiedRows++;
	}
 	if(RowsetA.AllowPendingRows(4))
	{
	 	TESTC_(RowsetA.InsertRow(&rghRow[ROW_SIX]),S_OK);	//insert row 6
	 	cModifiedRows++;
	}

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	COMPC(rgPendingStatus[0],DBPENDINGSTATUS_UNCHANGED);
	COMPC(rgPendingStatus[1],DBPENDINGSTATUS_CHANGED);
	COMPC(rgPendingStatus[3],DBPENDINGSTATUS_INVALIDROW);
	if(cModifiedRows >=2)
		COMPC(rgPendingStatus[2],DBPENDINGSTATUS_CHANGED);
	if(cModifiedRows >=3)
		COMPC(rgPendingStatus[4],DBPENDINGSTATUS_NEW);
	if(cModifiedRows >=4)
		COMPC(rgPendingStatus[5],DBPENDINGSTATUS_NEW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Empty Rowset [NULL, N, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_10()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];

	CRowsetUpdate EmptyRowsetA;
	TESTC_PROVIDER(EmptyRowsetA.CreateRowset(SELECT_EMPTYROWSET)==S_OK);

	//Can't fetch any rows, but is useful, since we can guarentee no other
	//row modidifying methods have been called...
	
	rghRow[ROW_ONE]   = DB_NULL_HROW;
	rghRow[ROW_TWO]   = DB_NULL_HROW;
	rghRow[ROW_THREE] = INVALID(HROW);
	rghRow[ROW_FOUR]  = DB_NULL_HROW;
	
	//GetRowStatus 
	TESTC_(EmptyRowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_E_ERRORSOCCURRED);

	COMPC(rgPendingStatus[0],DBPENDINGSTATUS_INVALIDROW);
	COMPC(rgPendingStatus[1],DBPENDINGSTATUS_INVALIDROW);
	COMPC(rgPendingStatus[2],DBPENDINGSTATUS_INVALIDROW);
	COMPC(rgPendingStatus[3],DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_11()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_12()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 hard deleted 2 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_13()
{
	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;
	
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_TWO]),S_OK)   //hard-delete row 2;
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FOUR]),S_OK)  //hard-delete row 4;
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_FIVE]),S_OK)  //hard-delete row 5;

	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK) //modify row 3;

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	
	TESTC(rgPendingStatus[ROW_ONE]   == DBPENDINGSTATUS_CHANGED);
	TESTC(rgPendingStatus[ROW_TWO]   == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_THREE] == (DBPENDINGSTATUS)(RowsetA.AllowPendingRows(2) ? DBPENDINGSTATUS_CHANGED : DBPENDINGSTATUS_UNCHANGED));
	TESTC(rgPendingStatus[ROW_FOUR]  == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_FIVE]  == DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TableInsert(THREE_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 3 invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_14()
{
	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,TWO_ROWS,rghRow),S_OK) ;
	
	//Rest are invalid
	rghRow[ROW_THREE] = DB_NULL_HROW;
	rghRow[ROW_FOUR]  = INVALID(HROW);
	rghRow[ROW_FIVE]  = DB_NULL_HROW;

	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)   //modify row 1;

	//GetRowStatus   
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	
	TESTC(rgPendingStatus[ROW_ONE]   == DBPENDINGSTATUS_CHANGED);
	TESTC(rgPendingStatus[ROW_TWO]   == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_THREE] == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_FOUR]  == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_FIVE]  == DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 hard deleted 1 invalid 1 modified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_15()
{
	const int NROWS = FIVE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,TWO_ROWS,rghRow),S_OK) ;
	
	//Rest are invalid
	rghRow[ROW_THREE] = DB_NULL_HROW;

	//Make changes(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_ONE]),S_OK)		//delete row 1;
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)			//modify row 2;

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	
	TESTC(rgPendingStatus[ROW_ONE]   == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_TWO]   == DBPENDINGSTATUS_CHANGED);
	TESTC(rgPendingStatus[ROW_THREE] == DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_16()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - All types
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_17()
{
	TBEGIN
	const int NROWS = SIX_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,FOUR_ROWS,rghRow),S_OK) ;
	

	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FIVE]),S_OK)    //1 inserted;
	rghRow[ROW_SIX] = DB_NULL_HROW;						 //1 invalid
	
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)//1 hard-deleted;
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK)      //1 deleted;
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)      //1 modified;
	
									//[ROW_FOUR]		 //1 unchanged

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	
	TESTC(rgPendingStatus[ROW_ONE]   == (DBPENDINGSTATUS)(RowsetA.AllowPendingRows(2) ? DBPENDINGSTATUS_CHANGED : DBPENDINGSTATUS_UNCHANGED));
	TESTC(rgPendingStatus[ROW_TWO]   == DBPENDINGSTATUS_DELETED);
	TESTC(rgPendingStatus[ROW_THREE] == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_FOUR]  == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_FIVE]  == DBPENDINGSTATUS_NEW);
	TESTC(rgPendingStatus[ROW_SIX]   == DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(FIVE_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - All Unchanged
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_18()
{
	const int NROWS = TWO_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,NROWS,rghRow),S_OK) ;

	//Make no changes

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(VerifyArray(NROWS,rgPendingStatus,DBPENDINGSTATUS_UNCHANGED));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - All Inserted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_19()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	cModifiedRows = RowsetA.GetMaxPendingRows();
	if(cModifiedRows==0 || cModifiedRows>NROWS)
		cModifiedRows = NROWS;

	//Insert row handle(s)
	TESTC_(RowsetA.InsertRow(cModifiedRows,rghRow),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,cModifiedRows,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(VerifyArray(cModifiedRows,rgPendingStatus,DBPENDINGSTATUS_NEW));

	//Update all
	TESTC_(RowsetA.UpdateAll(),S_OK) ;


CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - All Soft-Deleted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_20()
{
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL}; 
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	DBCOUNTITEM cModifiedRows = 0;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK) ;
	cModifiedRows = RowsetA.GetMaxPendingRows();
	if(cModifiedRows==0 || cModifiedRows>NROWS)
		cModifiedRows = NROWS;

	//Delete row(s)
	TESTC_(RowsetA.DeleteRow(cModifiedRows,rghRow),S_OK) ;

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,cModifiedRows,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(VerifyArray(cModifiedRows,rgPendingStatus,DBPENDINGSTATUS_DELETED));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_21()
{
	HROW hRow = DB_NULL_HROW; 
	DBPENDINGSTATUS rgPendingStatus[ONE_ROW];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Insert row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,ONE_ROW,&hRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]  == DBPENDINGSTATUS_NEW);

    //An insertion followed by a modification counts as an insertion
	//Update all
	TESTC_(RowsetA.UpdateAll(),S_OK) ;

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Modify Delete
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_22()
{
	HROW hRow = DB_NULL_HROW; 
	DBPENDINGSTATUS rgPendingStatus[ONE_ROW];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);

	//Modify row
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	//Delete row
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,ONE_ROW,&hRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	COMPC(rgPendingStatus[ROW_ONE],DBPENDINGSTATUS_DELETED)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Insert/Modify/Delete
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_23()
{
	HROW hRow = DB_NULL_HROW; 
	DBPENDINGSTATUS rgPendingStatus[ONE_ROW];
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Insert row
	TESTC_(RowsetA.InsertRow(&hRow),S_OK);
	TESTC_(RowsetA.ModifyRow(hRow),S_OK);
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,ONE_ROW,&hRow,rgPendingStatus),DB_E_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[0]==DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_24()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert/Delete/Modify - Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_25()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handles
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);

	//Insert row
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK);
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK);
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_THREE]),S_OK);

	//Update
	TESTC_(RowsetA.UpdateAll(),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_TWO]    == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_INVALIDROW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Make all type changes - Undo half - Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_26()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handles
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRow),S_OK);

	TESTC_(RowsetA.InsertRow(&rghRow[ROW_THREE]),S_OK);
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//Undo
	TESTC_(RowsetA.UndoRow(rghRow[ROW_TWO]),S_OK);

	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == (DBPENDINGSTATUS)(RowsetA.AllowPendingRows(2) ? DBPENDINGSTATUS_CHANGED : DBPENDINGSTATUS_UNCHANGED));
	TESTC(rgPendingStatus[ROW_TWO]    == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_NEW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Change N rows, GetPendingRows, GetRowStatus
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_27()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];
	ULONG cModifiedRows = 0;
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain row handles
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);

	//Make N changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK);
	cModifiedRows++;

	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
		cModifiedRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);
		cModifiedRows++;
	}

	//GetPendingRows = N
	TESTC_(RowsetA.GetPendingRows(cModifiedRows),S_OK);
	
	//GetRowStatus 
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == (DBPENDINGSTATUS)(RowsetA.AllowPendingRows(2) ? DBPENDINGSTATUS_CHANGED : DBPENDINGSTATUS_UNCHANGED));
	TESTC(rgPendingStatus[ROW_TWO]    == (DBPENDINGSTATUS)(RowsetA.AllowPendingRows(3) ? DBPENDINGSTATUS_DELETED : DBPENDINGSTATUS_UNCHANGED));
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_FOUR]   == DBPENDINGSTATUS_NEW);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call GetRowStatus 3 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowStatus::Variation_28()
{
	TBEGIN
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[NROWS];

	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	TESTC_PROVIDER(RowsetA.AllowPendingRows(NROWS));
	
	//Make sure we have QBU turned on 
	if(SupportedProperty(KAGPROP_QUERYBASEDUPDATES,DBPROPSET_PROVIDERROWSET))
		TESTC(RowsetA.GetProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET));

	//Obtain row handles
	TESTC_(RowsetA.GetRow(FIRST_ROW, THREE_ROWS, rghRow),S_OK);

	//Make N changes
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK);
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK);
	TESTC_(RowsetA.DeleteRow(rghRow[ROW_TWO]),S_OK);

	//GetRowStatus #1
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == DBPENDINGSTATUS_CHANGED);
	TESTC(rgPendingStatus[ROW_TWO]    == DBPENDINGSTATUS_DELETED);
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_FOUR]   == DBPENDINGSTATUS_NEW);

	//GetRowStatus #2
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),S_OK);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == DBPENDINGSTATUS_CHANGED);
	TESTC(rgPendingStatus[ROW_TWO]    == DBPENDINGSTATUS_DELETED);
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_FOUR]   == DBPENDINGSTATUS_NEW);

	//Update
	TESTC_(RowsetA.UpdateAll(),S_OK);
	
	//GetRowStatus #3
	TESTC_(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,NROWS,rghRow,rgPendingStatus),DB_S_ERRORSOCCURRED);
	TESTC(rgPendingStatus!=NULL);
	TESTC(rgPendingStatus[ROW_ONE]    == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_TWO]    == DBPENDINGSTATUS_INVALIDROW);
	TESTC(rgPendingStatus[ROW_THREE]  == DBPENDINGSTATUS_UNCHANGED);
	TESTC(rgPendingStatus[ROW_FOUR]   == DBPENDINGSTATUS_UNCHANGED);

CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowStatus::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//|	Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			07/25/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetUpdate::Init())
	// }}
	{
		return TEST_PASS;
	}
	return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid GetOriginalData calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
 	HACCESSOR hNullAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	HROW hRow = DB_NULL_HROW;
	void* pData = INVALID(void*);

	CRowsetUpdate RowsetA;
	
   	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetUpdate method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Create a NULL Accessor
	TESTC_(RowsetA.pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hNullAccessor,NULL),S_OK);
	 
	//Obtain the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	
	//create an error object
	TESTC(m_pExtError->CauseError());

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hNullAccessor,pData)),S_OK);
	
	//Do extended check following GetOriginalData
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));
	
	TESTC(pData==INVALID(void*));

	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(hNullAccessor     ,NULL),S_OK);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(hAccessor         ,NULL),DB_E_BADACCESSORHANDLE);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(INVALID(HACCESSOR),NULL),DB_E_BADACCESSORHANDLE);
	TESTC_(RowsetA.pIAccessor()->ReleaseAccessor(DB_NULL_HACCESSOR ,NULL),DB_E_BADACCESSORHANDLE);

CLEANUP:
	//Release the Accesssor
	RowsetA.ReleaseAccessor(hNullAccessor);
	RowsetA.ReleaseAccessor(hAccessor);
	
	//Release the row handle
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetOriginalData calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	//HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
   	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	//The work arround is to create a new rowset in this variation, which
	//will guarentee we don't have to call restart poisiton.
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Get the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//create an error object
	TESTC(m_pExtError->CauseError());

	//Call GetOriginalData with NULL pDdata
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,NULL)),E_INVALIDARG);
	
	//Do extended check following GetOriginalData
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetOriginalData calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
  	//HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
   	CRowsetUpdate RowsetA;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.

	//The work arround is to create a new rowset in this variation, which
	//will guarentee we don't have to call restart poisiton.
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Get the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Call GetOriginalData with NULL pDdata
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,RowsetA.m_hAccessor,NULL)),E_INVALIDARG);
	
	//Do extended check following GetOriginalData
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid GetPendingRows calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	TBEGIN
	const int NROWS = THREE_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL};

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetUpdate method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);
	
	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW,&rghRow[ROW_ONE]),S_OK) ;
	TESTC_(RowsetA.GetNextRows(&rghRow[ROW_TWO]),S_OK)      ;
	
	//Make 3 changes
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;
	
	//create an error object
	TESTC(m_pExtError->CauseError());

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL)),S_OK);

	//Do extended check following GetPendingRows
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetPendingRows calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;

   	//create an error object
	TESTC(m_pExtError->CauseError());

	//GetPendingRows DBPENDINGSTATUS_INVALIDROW
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_INVALIDROW,&cPendingRows,&rgPendingRows,&rgPendingStatus)),E_INVALIDARG);

	//Do extended check following GetPendingRows
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetPendingRows calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL};

	DBCOUNTITEM cPendingRows;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;

	CRowsetUpdate RowsetA;

	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//Obtain the row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS, rghRow),S_OK);

	//Modify row(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_THREE]),S_OK)	 //modify row 3;

	//GetPendingRows DBPENDINGSTATUS_INVALIDROW
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_INVALIDROW,&cPendingRows,&rgPendingRows,&rgPendingStatus)),E_INVALIDARG);

	//Do extended check following GetPendingRows
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	COMPC(cPendingRows,NO_ROWS)
	TESTC(rgPendingRows==NULL && rgPendingStatus==NULL);
	
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS, rghRow);	
	PROVIDER_FREE(rgPendingRows)
	PROVIDER_FREE(rgPendingStatus)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid GetRowStatus calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_7()
{
 	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	DBPENDINGSTATUS rgPendingStatus[1] = {ULONG_MAX};

	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetUpdate method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);

   	//create an error object
	TESTC(m_pExtError->CauseError());

	//GetRowStatus [invalid, 0, NULL, valid], should no nothing
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetRowStatus(INVALID(HCHAPTER),0,NULL,rgPendingStatus)),S_OK);

	//Do extended check following GetRowStatus
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	TESTC(rgPendingStatus[0] == ULONG_MAX);

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetRowStatus calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_8()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	
	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);

   	//create an error object
	TESTC(m_pExtError->CauseError());

	//GetRowStatus
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,ONE_ROW,rghRow,NULL)),E_INVALIDARG);

	//Do extended check following GetRowStatus
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetRowStatus calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_9()
{
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};
	
	CRowsetUpdate RowsetA;

	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//Fetch rows
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK);
	
	//GetRowStatus
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->GetRowStatus(NULL,ONE_ROW,rghRow,NULL)),E_INVALIDARG);

	//Do extended check following GetRowStatus
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid Undo calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_10()
{
	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetUpdate method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
	
	//create an error object
	TESTC(m_pExtError->CauseError());

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Undo(NULL,0,NULL,NULL,NULL,NULL)),S_OK);
	
	//Do extended check following Undo
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Invalid Undo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_11()
{
	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone;
	DBROWSTATUS* rgRowStatus;
		
	CRowsetUpdate RowsetA;
	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//create an error object
	TESTC(m_pExtError->CauseError());

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Undo(INVALID(HCHAPTER),ONE_ROW,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus)),E_INVALIDARG);
	
	//Do extended check following Undo
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	COMPC(cRowsUndone,NO_ROWS)
	TESTC(rgRowsUndone==NULL && rgRowStatus==NULL);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Invalid Undo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_12()
{
	DBCOUNTITEM cRowsUndone = 1;
	HROW* rgRowsUndone;
	DBROWSTATUS* rgRowStatus;
		
	CRowsetUpdate RowsetA;
	
	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
  
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Undo(INVALID(HCHAPTER),ONE_ROW,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus)),E_INVALIDARG);
	
	//Do extended check following Undo
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	COMPC(cRowsUndone,NO_ROWS)
	TESTC(rgRowsUndone==NULL && rgRowStatus==NULL);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Valid Update calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_13()
{
	CRowsetUpdate RowsetA;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetUpdate method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//create an error object
	TESTC(m_pExtError->CauseError());
	
	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Update(NULL,0,NULL,NULL,NULL,NULL)),S_OK);
	
	//Do extended check following Update
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Invalid Update calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_14()
{
	TBEGIN
  	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS,rghRow),S_OK);
	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;

	//create an error object
	TESTC(m_pExtError->CauseError());

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Update(INVALID(HCHAPTER),NROWS,NULL,&cUpdatedRows,&rgUpdatedRows,&rgRowStatus)),E_INVALIDARG);

	//Do extended check following Update
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	TESTC(cUpdatedRows==0 && rgUpdatedRows==NULL && rgRowStatus==NULL);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid Update calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_15()
{
	TBEGIN
  	DBCOUNTITEM cUpdatedRows = 1;
	HROW* rgUpdatedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	const int NROWS = FOUR_ROWS;
	HROW rghRow[NROWS] = {NULL,NULL,NULL,NULL}; 

	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
	
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain handle(s), starting at row 1
	TESTC_(RowsetA.GetRow(FIRST_ROW, NROWS,rghRow),S_OK);
	//Make changes(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)    //modify row 1;

	TESTC_(m_hr=(RowsetA.pIRowsetUpdate()->Update(INVALID(HCHAPTER),NROWS,NULL,&cUpdatedRows,&rgUpdatedRows,&rgRowStatus)),E_INVALIDARG);

	//Do extended check following Update
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));

	TESTC(cUpdatedRows==0 && rgUpdatedRows==NULL && rgRowStatus==NULL);
	
CLEANUP:
	RowsetA.ReleaseRows(NROWS,rghRow);
	PROVIDER_FREE(rgUpdatedRows);
	PROVIDER_FREE(rgRowStatus);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_FALSE GetPendingRows call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_16()
{
	DBCOUNTITEM cPendingRows = 1;

	CRowsetUpdate RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);
		
	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
	
	TESTC_(m_hr=RowsetA.pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,NULL,NULL),S_FALSE);
	//Do extended check following Update
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));
	COMPC(cPendingRows,NO_ROWS);

CLEANUP:
	TRETURN
}
//}}

// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DELETEDROW GetOriginalData call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_17()
{
	HROW hRow = DB_NULL_HROW;
	CRowsetUpdate RowsetA;
	
	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Obtain the first row	
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);

	//Hard-delete the row
	TESTC_(RowsetA.HardDeleteRow(hRow),S_OK);
	
	//Call GetOriginalData on the hard deleted row  
	TESTC_(m_hr=RowsetA.GetOriginalData(hRow, &RowsetA.m_pData),DB_E_DELETEDROW);

	//Do extended check following Update
	TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));
	TESTC(RowsetA.m_pData!=NULL) //provider shouldn't touch my alloced buffer;

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORTYPE GetOriginal call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_18()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;

	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the IRowsetUpdate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//Create the rowset
	CRowsetUpdate RowsetA;
	RowsetA.SetSettableProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateCommand()==S_OK);
	
	//Create an invalid Accessor for use with GetOriginalData
	//Must be done on the CommandObject, ParameterAccessors are not allowed
	//to be created on the RowsetObject
	TEST2C_(hr = GetAccessorAndBindings(RowsetA.pICommand(),DBACCESSOR_PARAMETERDATA,&hAccessor,
		NULL,NULL,NULL,DBPART_ALL,UPDATEABLE_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_BYREF,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT), S_OK, DB_E_BADACCESSORFLAGS);

	if(hr == S_OK)
	{
		//Now that we have the Accessor Created, now create the rowset
		//Must be a command query, since the accessor was created on the command
		TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_REVCOLLIST)==S_OK);
		
		//Obtain the first row
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Call GetOriginalData with an invalid accessor type
		TESTC_(m_hr=RowsetA.pIRowsetUpdate()->GetOriginalData(hRow,hAccessor,RowsetA.m_pData),DB_E_BADACCESSORTYPE);
		//Do extended check following Update
		TESTC(m_pExtError->ValidateExtended(m_hr, RowsetA.pIRowsetUpdate(), IID_IRowsetUpdate, LONGSTRING(__FILE__), __LINE__));
		TESTC(RowsetA.m_pData!=NULL);
	}
	else
	{
		//Make sure the provider doesn't support parameters (validated immediately)
		TESTC_(QI(RowsetA.pICommand(), IID_ICommandWithParameters), E_NOINTERFACE);
	}

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
//}}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetUpdate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombie)
//*-----------------------------------------------------------------------
//|	Test Case:		TCZombie - Zombie testing of IRowsetUpdate
//|	Created:			07/22/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Init()
{
	TBEGIN
	m_cPropSets = 0;
	m_rgPropSets = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		//Set Properties
		SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL,DBTYPE_I4);
				
		//register interface to be tested                     
		if(RegisterInterface(ROWSET_INTERFACE,IID_IRowsetUpdate))
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would have been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_1()
{
	IRowsetUpdate* pIRowsetUpdate = NULL;
	CRowsetUpdate RowsetA;
	
	HROW hRow = NULL;
	DBROWSTATUS rgRowStatus[ONE_ROW];
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC_PROVIDER(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowsetUpdate,m_cPropSets,m_rgPropSets));
	TESTC(pIRowsetUpdate!=NULL);
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction;

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Make a change, (delete it)	
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE));
	
	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Verify we still can use GetOriginalData after an ABORT
	TESTC_(pIRowsetUpdate->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),ExpectedHr);

	//Verify we still can use GetRowStatus after an ABORT
	TESTC_(pIRowsetUpdate->GetRowStatus(NULL,ONE_ROW,&hRow,rgRowStatus),ExpectedHr);

	//Verify we still can use GetPendingRows after an ABORT
	TESTC_(pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Update (all) after an ABORT
	TESTC_(pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Undo (all) after an ABORT
	TESTC_(pIRowsetUpdate->Undo(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);


CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	RowsetA.ReleaseRows(hRow);
	CleanUpTransaction(S_OK);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_2()
{
	IRowsetUpdate* pIRowsetUpdate = NULL;
	CRowsetUpdate RowsetA;
	
	HROW hRow = NULL;
	DBROWSTATUS rgRowStatus[ONE_ROW];
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC_PROVIDER(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowsetUpdate,m_cPropSets,m_rgPropSets));
	TESTC(pIRowsetUpdate!=NULL);
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction;

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Make a change, (delete it)	
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE));
	
	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Verify we still can use GetOriginalData after an ABORT
	TESTC_(pIRowsetUpdate->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),ExpectedHr);

	//Verify we still can use GetRowStatus after an ABORT
	TESTC_(pIRowsetUpdate->GetRowStatus(NULL,ONE_ROW,&hRow,rgRowStatus),ExpectedHr);

	//Verify we still can use GetPendingRows after an ABORT
	TESTC_(pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Update (all) after an ABORT
	TESTC_(pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Undo (all) after an ABORT
	TESTC_(pIRowsetUpdate->Undo(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);


CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	RowsetA.ReleaseRows(hRow);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_3()
{
	IRowsetUpdate* pIRowsetUpdate = NULL;
	CRowsetUpdate RowsetA;
	
	HROW hRow = NULL;
	DBROWSTATUS rgRowStatus[ONE_ROW];
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC_PROVIDER(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowsetUpdate,m_cPropSets,m_rgPropSets));
	TESTC(pIRowsetUpdate!=NULL);
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction;

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Make a change, (delete it)	
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Commit the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE));
	
	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Verify we still can use GetOriginalData after an COMMIT
	TESTC_(pIRowsetUpdate->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),ExpectedHr);

	//Verify we still can use GetRowStatus after an COMMIT
	TESTC_(pIRowsetUpdate->GetRowStatus(NULL,ONE_ROW,&hRow,rgRowStatus),ExpectedHr);

	//Verify we still can use GetPendingRows after an COMMIT
	TESTC_(pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Update (all) after an COMMIT
	TESTC_(pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Undo (all) after an COMMIT
	TESTC_(pIRowsetUpdate->Undo(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);


CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	RowsetA.ReleaseRows(hRow);
	CleanUpTransaction(S_OK);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_4()
{
	IRowsetUpdate* pIRowsetUpdate = NULL;
	CRowsetUpdate RowsetA;
	
	HROW hRow = NULL;
	DBROWSTATUS rgRowStatus[ONE_ROW];
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC_PROVIDER(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowsetUpdate,m_cPropSets,m_rgPropSets));
	TESTC(pIRowsetUpdate!=NULL);
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction;

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK);
	
	//Make a change, (delete it)	
	TESTC_(RowsetA.DeleteRow(hRow),S_OK);
	
	//Commit the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE));
	
	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Verify we still can use GetOriginalData after an COMMIT
	TESTC_(pIRowsetUpdate->GetOriginalData(hRow,RowsetA.m_hAccessor,RowsetA.m_pData),ExpectedHr);

	//Verify we still can use GetRowStatus after an COMMIT
	TESTC_(pIRowsetUpdate->GetRowStatus(NULL,ONE_ROW,&hRow,rgRowStatus),ExpectedHr);

	//Verify we still can use GetPendingRows after an COMMIT
	TESTC_(pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Update (all) after an COMMIT
	TESTC_(pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);

	//Verify we still can use Undo (all) after an COMMIT
	TESTC_(pIRowsetUpdate->Undo(NULL,0,NULL,NULL,NULL,NULL),ExpectedHr);


CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	RowsetA.ReleaseRows(hRow);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
	//FreeProperties
	FreeProperties(&m_cPropSets,&m_rgPropSets);
	return(CTransaction::Terminate());
}	// }}
// }}
// }}



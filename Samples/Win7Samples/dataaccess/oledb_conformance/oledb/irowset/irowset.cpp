//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IRowset.CPP | The test module for IRowset
//

//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#include "modstandard.hpp"	// Standard headers			
#include "IRowset.h"		// IRowset testmodule header
#include "ExtraLib.h"

			  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xeaf58a02, 0x0e5b, 0x11cf, { 0xac, 0x3d, 0x00, 0xaa, 0x00, 0x4a, 0x99, 0xe0 }};
DECLARE_MODULE_NAME("IRowset");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module IRowset interface");
DECLARE_MODULE_VERSION(834101102);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END


////////////////////////////////////////////////////////////////////////////
//  ModuleInit
//
////////////////////////////////////////////////////////////////////////////
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	//CommonModuleInit, Verify IRowset is supported, and Create a table
	return CommonModuleInit(pThisTestModule, IID_IRowset, SIX_ROWS);
}	
  

////////////////////////////////////////////////////////////////////////////
//  ModuleTerminate
//
////////////////////////////////////////////////////////////////////////////
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	//Release Session
	return CommonModuleTerminate(pThisTestModule);
}	


////////////////////////////////////////////////////////////////////////////
//  TCIRowset
//
////////////////////////////////////////////////////////////////////////////
class TCIRowset : public CRowset
{
public:
	//constructors
	TCIRowset(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIRowset();

	//methods
	virtual BOOL	Init();
	virtual BOOL	Terminate();

	//Overloaded CreateRowset method
	virtual HRESULT	CreateRowset
						(	
							EQUERY				eSQLStmt,				
							REFIID				riid = IID_IRowset,
							DBACCESSORFLAGS		dwAccessorFlags = DBACCESSOR_ROWDATA,		
							DBPART				dwPart = DBPART_ALL,					
							ECOLS_BOUND			eColsToBind = ALL_COLS_BOUND,
							ECOLUMNORDER		eBindingOrder = FORWARD,			
							ECOLS_BY_REF		eColsByRef = NO_COLS_BY_REF,				
							DBTYPE				wTypeModifier = DBTYPE_EMPTY,
							BLOBTYPE			dwBlobType = NO_BLOB_COLS
						);

	virtual BOOL	VerifyAddRefRows
						(	
							DBCOUNTITEM		cRows, 
							HROW*		rghRows, 
							ULONG		ulRefCount = 0,
							DBROWSTATUS dwRowStatus = DBROWSTATUS_S_OK
						);

	virtual BOOL	VerifyReleaseRows
						(	
							DBCOUNTITEM		cRows, 
							HROW*		rghRows, 
							ULONG		ulRefCount = 0,
							DBROWSTATUS dwRowStatus = DBROWSTATUS_S_OK
						);

	//@mfunc: verify the reference counts for an array of RefCounts
	virtual BOOL	VerifyRowStatus
						(	
							DBCOUNTITEM			cRows,							//[in] cRows
							DBROWSTATUS*	rgRowStatus,					//[in] rgRowStatus
							DBROWSTATUS		dwRowStatus = DBROWSTATUS_S_OK	//[in] Expected RowStatus 
						);

	//@mfunc: verify the position of the cursor in the row set
	virtual BOOL	VerifyNextFetchPosition
						(
								DBROWCOUNT	iRow			//the NextFetchPosition
						);	

	//@mfunc: verify RestartPosition
	virtual BOOL	VerifyRestartPosition
						(
								HCHAPTER hChapter = DB_NULL_HCHAPTER,
								BOOL fRowsReleased = TRUE
						);

	//@mfunc: verify GetNextRows and row values returned
	virtual BOOL    VerifyGetNextRows
						(
								DBROWCOUNT lOffset, 
								DBROWCOUNT cRows, 
								DBCOUNTITEM iRowStart, 
								ECOLUMNORDER eOrder = FORWARD, 
								DBCOUNTITEM cRowsExpected = MAXDBCOUNTITEM,
								DBMEMOWNERENUM eMemoryOwner = DBMEMOWNER_CLIENTOWNED
						);

	//@cmember: Confirm GetData retieve correct data into the consumers buffer
	virtual BOOL	VerifyAllRows();

	virtual BOOL	VerifyAccessorValidation
						(
								CRowset*		pCRowset, 
								DBACCESSORFLAGS dwAccessorFlags, 
								DWORD			dwBlobType, 
								HRESULT			hrDefferred, 
								DBBINDSTATUS	dwBindStatus, 
								BOOL			fAllowSuccess		= FALSE, 
								ECOLS_BOUND		eColsBound			= ALL_COLS_BOUND, 
								DBBINDSTATUS**	prgBindStatus		= NULL, 
								BOOL*			pfDefferred			= NULL, 
								ECOLS_BY_REF	eColsByRef			= NO_COLS_BY_REF,
								DBTYPE			dwModifier			= DBTYPE_EMPTY
						);

};


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::TCIRowset
//
////////////////////////////////////////////////////////////////////////////
TCIRowset::TCIRowset(WCHAR * wstrTestCaseName)	: CRowset(wstrTestCaseName) 
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::~TCIRowset
//
////////////////////////////////////////////////////////////////////////////
TCIRowset::~TCIRowset()
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowset::Init()
{
	return CRowset::Init();
}

////////////////////////////////////////////////////////////////////////////
//  TCIRowset::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowset::Terminate()
{
	return CRowset::Terminate();
}

	
////////////////////////////////////////////////////////////////////////////
//  TCIRowset::CreateRowset
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIRowset::CreateRowset
(	
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	//Deletgate
	//The only difference between this method and the CRowset one is that
	//The default for eColsToBind == ALL_COLS whereas the CRowset is UPDATEABLE
	//This Rowset test doesn't do any updating, so we need all cols bound...
	return CRowset::CreateRowset(eQuery, riid, NULL, dwAccessorFlags, dwPart,
		eColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType);
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::VerifyReleaseRows
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowset::VerifyReleaseRows
(	
	DBCOUNTITEM		cRows, 
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
//  TCIRowset::VerifyAddRefRows
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowset::VerifyAddRefRows
(	
	DBCOUNTITEM		cRows, 
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
//  TCIRowset::VerifyRowStatus
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowset::VerifyRowStatus
(	
	DBCOUNTITEM			cRows,			//[in] cRows
	DBROWSTATUS*	rgRowStatus,	//[in] rgRowStatus
	DBROWSTATUS		dwRowStatus 	//[in] Expected RowStatus 
)	
{
	//every element in the ref count array should be the same as cExpected
	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		if(rgRowStatus[i] != dwRowStatus)
			return FALSE;
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::VerifyNextFetchPosition
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowset::VerifyNextFetchPosition
(
	DBROWCOUNT	iRow		//the NextFetchPosition expected
)				
{
	TBEGIN
	HROW	hRow = DB_NULL_HROW;

	//Get the next row handle
	TESTC_(GetNextRows(&hRow),S_OK);
	
	//VerifyRowHandles
	TESTC(VerifyRowHandles(hRow, iRow));
	
	//release the row handle
	TESTC_(ReleaseRows(hRow),S_OK);
	hRow = NULL;

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::VerifyRestartPosition
//
////////////////////////////////////////////////////////////////////////////
BOOL	TCIRowset::VerifyRestartPosition(HCHAPTER hChapter, BOOL fRowsReleased)
{
	TBEGIN
	HRESULT hr = S_OK;

	//RestartPosition
	hr = RestartPosition(hChapter);
	
	//Verify Results
	if(fRowsReleased || SUCCEEDED(hr))
	{
		//Only valid return codes allowed...
		TEST2C_(hr, S_OK, DB_S_COMMANDREEXECUTED);
		
		//Verify at the first row
		TESTC(VerifyNextFetchPosition(FIRST_ROW));

		//Leave the state back at the begining
		TESTC_(RestartPosition(hChapter),S_OK);
	}
	else
	{
		//According to the 2.0 OLE DB Spec, some providers may not be able to 
		//RestartPosition when there are Rows currently held, (even with CANHOLDROWS)
		TESTC_(hr, DB_E_ROWSNOTRELEASED);
		
		//But there are some restrictions. (and you thought it was going to be easy)
		//DBPROP_QUICKRESTART must also be FALSE
		TESTC(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, VARIANT_FALSE));
		
		//Return false from this method to indicate to the caller
		//that the position is NOT at the head.  LTM will take care of any errors occuring above
		TWARNING("RestartPosition() failed, but allowed since QuickRestart=FALSE for this senario");
		QTESTC(FALSE);
	}

CLEANUP:
	TRETURN
}


//////////////////////////////////////////////////////////////////////////////
// TCIRowset::VerifyGetNextRows
//
//////////////////////////////////////////////////////////////////////////////
BOOL TCIRowset::VerifyGetNextRows(DBROWCOUNT lOffset, DBROWCOUNT cRows, DBCOUNTITEM iRowStart, ECOLUMNORDER eOrder, DBCOUNTITEM cRowsExpected, DBMEMOWNERENUM eMemoryOwner)
{
	TBEGIN

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;
	HROW* rghRowsInput = NULL;
	HRESULT hr = S_OK;

	//Alloc rghRows if consumer owned
	if(eMemoryOwner == DBMEMOWNER_CLIENTOWNED)
	{
		rghRows = PROVIDER_ALLOC_((DBCOUNTITEM)ABS(cRows), HROW);
		rghRowsInput = rghRows;
	}

	//GetNextRows
	//This is our helper function which does a majority of the validity 
	hr = GetNextRows(lOffset, cRows, &cRowsObtained, &rghRows);

	//If expected to return the same number of rows requested
	if((DBCOUNTITEM)ABS(cRows) == cRowsExpected || cRowsExpected == MAXDBCOUNTITEM)
	{
		//Verify results
		if(hr == DB_S_ROWLIMITEXCEEDED && (DBCOUNTITEM)ABS(cRows)>1)
		{
			TESTC((DBCOUNTITEM)cRows > m_ulMaxOpenRows); 
			TESTC(cRowsObtained < cRowsExpected);
			TESTC(VerifyRowHandles(cRowsObtained, rghRows, iRowStart, eOrder));
			TOUTPUT("Maximum Rows Exceeded with cRows = " << cRows);
		}
		else
		{	
			TESTC_(hr,S_OK);
			TESTC(cRowsObtained==(DBCOUNTITEM)ABS(cRows));
			TESTC(VerifyRowHandles(cRowsObtained, rghRows, iRowStart, eOrder));
			TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
			cRowsObtained = 0;
		}
	}
	else
	{
		//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
		//We no longer have DB_E_BADSTARTPOSITION for 2.x providers

		//Otherwise we were expecting not to be able to retreive the
		//full set of requested rows...
		TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_S_ROWLIMITEXCEEDED);

		//Verify Results...
		if(hr==DB_S_ENDOFROWSET)
		{
			TESTC(cRowsObtained == cRowsExpected);
			TESTC(VerifyRowHandles(cRowsObtained, rghRows, iRowStart, eOrder));
		}
		else if(hr == DB_S_ROWLIMITEXCEEDED)
		{
			TESTC((ULONG)cRows > m_ulMaxOpenRows); 
			TESTC(cRowsObtained < cRowsExpected);
			TESTC(VerifyRowHandles(cRowsObtained, rghRows, iRowStart, eOrder));
		}
		else
		{
			//No rows should be obtained.
			TESTC(cRowsObtained == 0);
			if(eMemoryOwner == DBMEMOWNER_PROVIDEROWNED)
			{
				TESTC(rghRows == NULL);
			}

			//E_OUTOFMEMORY
			if(hr == E_OUTOFMEMORY)
			{
				 //This should really only be reasonably be returned for large fetches or requests.
				TESTC((DBCOUNTITEM)ABS(cRows) >  1000 || (DBCOUNTITEM)ABS(lOffset) >  1000);
			}
		}
	}

	//Verify output array, depending upon consumer or provider allocated...
	if(eMemoryOwner == DBMEMOWNER_CLIENTOWNED)
	{
		//This is a users allocated static array,
		//This had better not be nulled out by the provider, if non-null on input
		TESTC(rghRows == rghRowsInput);
	}

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::VerifyAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowset::VerifyAllRows()
{
	TBEGIN
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;

	//get the number of rows in the table
	DBCOUNTITEM ulRowCount = pTable()->CountRowsOnTable();

	//loop through the rowset, retrieve one row at a time
	for(DBCOUNTITEM i=1; i<=ulRowCount; i++)
	{
		//GetNextRow 
		TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//VerifyRowHandles
		TESTC(VerifyRowHandles(cRowsObtained, rghRows, i));

		//release the row handle
		TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

 	//Verify the cursor is at the end of the rowset
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	PROVIDER_FREE(rghRows);

	//Call again at end of rowset...
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained==0 && rghRows == NULL);

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowset::VerifyAccessorValidation
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowset::VerifyAccessorValidation
(
	CRowset* pCRowset, 
	DBACCESSORFLAGS dwAccessorFlags, 
	DWORD dwBlobType, 
	HRESULT hrDefferred, 
	DBBINDSTATUS dwBindStatus, 
	BOOL fAllowSuccess, 
	ECOLS_BOUND eColsBound, 
	DBBINDSTATUS** prgBindStatus, 
	BOOL* pfDefferred, 
	ECOLS_BY_REF eColsByRef, 
	DBTYPE	dwModifier
)
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cRowSize = 0;
	HRESULT hr = S_OK;

	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBBINDSTATUS* rgBindStatus = NULL;
	void* pData = NULL;

	//Create Accessor
	hr = pCRowset->CreateAccessor(&hAccessor, dwAccessorFlags, DBPART_ALL, &cBindings, &rgBindings, &cRowSize, dwBlobType, &rgBindStatus, eColsBound, eColsByRef, dwModifier);
	TEST2C_(hr, S_OK, DB_E_ERRORSOCCURRED);

	//May have Deferred Accessor Validation
	if(hr == S_OK)
	{
		//Indicate Deferred Validation
		if(pfDefferred)
			*pfDefferred = TRUE;
		
		//Grab a row handle
		TESTC_(pCRowset->GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Get the row data...
		SAFE_ALLOC(pData, BYTE, cRowSize);
		TESTC_(GetRowData(hRow, &pData), DB_E_UNSUPPORTEDCONVERSION);
		
		//There are "special" cases where even though there should be an error,
		//A success code can be returned.  ie:  DBPROP_MULTIPLESTORAGEOBJECTS=FALSE
		//We should not be able to obtain more than 1 storage object, but the spec
		//has been relaxed to indicate FALSE - "may" not be able to obtain more than one.
		//So we need to allow success in special circumstances 
		//(by default - fAllowSuccess = FALSE)
		if(fAllowSuccess)
		{
			TEST3C_(hr, S_OK, DB_S_ERRORSOCCURRED, hrDefferred);
		}
		else
		{
			QTESTC_(hr, hrDefferred);
		}
	}
	else
	{
		//Indicate Non-Deferred Validation
		if(pfDefferred)
			*pfDefferred = FALSE;

		//Verify Accessor Status
		//Only verify the Accessor Status if the user doesn't want the Status returned.
		//So for specical cases, the variation knows better what the status of each should be,
		//not neccessarly all the same...
		if(prgBindStatus == NULL)
			for(DBORDINAL i=0; i<cBindings; i++)
			{
				TESTC(rgBindStatus[i] == dwBindStatus);
			}
	}

CLEANUP:
	if(prgBindStatus)
		*prgBindStatus = rgBindStatus;
	else
		PROVIDER_FREE(rgBindStatus);
	
	//Can only release out-of-line memory when there is actual data
	//If this fails, we can't release this memory...
	pCRowset->ReleaseAccessor(hAccessor, cBindings, rgBindings);
	pCRowset->ReleaseRows(hRow);
	SAFE_FREE(pData);
	TRETURN;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIRowset_RetrieveData)
//--------------------------------------------------------------------
// @class Retrieve data of all types from the rowset without coercion
//
class TCIRowset_RetrieveData : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_RetrieveData,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Default Accessor - no BLOBs
	int Variation_1();
	// @cmember Default Accessor  - with BLOBs
	int Variation_2();
	// @cmember BYREF accessor, no BLOBs
	int Variation_3();
	// @cmember BYREF with BLOBs
	int Variation_4();
	// @cmember OPTIMIZED, BLOBs only
	int Variation_5();
	// @cmember VECTOR, BYREF, with BLOBs
	int Variation_6();
	// @cmember VECTOR, with BLOBs
	int Variation_7();
	// @cmember OPTIMIZED, BYREF, with BLOBs
	int Variation_8();
	// @cmember ARRAY, BYREF, no BLOBs
	int Variation_9();
	// @cmember ARRAY, BYREF, with BLOBs
	int Variation_10();
	// @cmember OPTIMIZED Even Columns with BLOBs
	int Variation_11();
	// @cmember All Columns with BLOBs at End
	int Variation_12();
	// @cmember BLOBs at End only
	int Variation_13();
	// @cmember Very WIDE table
	int Variation_14();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_RetrieveData)
#define THE_CLASS TCIRowset_RetrieveData
BEG_TEST_CASE(TCIRowset_RetrieveData, TCIRowset, L"Retrieve data of all types from the rowset without coercion")
	TEST_VARIATION(1, 		L"Default Accessor - no BLOBs")
	TEST_VARIATION(2, 		L"Default Accessor  - with BLOBs")
	TEST_VARIATION(3, 		L"BYREF accessor, no BLOBs")
	TEST_VARIATION(4, 		L"BYREF with BLOBs")
	TEST_VARIATION(5, 		L"OPTIMIZED, BLOBs only")
	TEST_VARIATION(6, 		L"VECTOR, BYREF, with BLOBs")
	TEST_VARIATION(7, 		L"VECTOR, with BLOBs")
	TEST_VARIATION(8, 		L"OPTIMIZED, BYREF, with BLOBs")
	TEST_VARIATION(9, 		L"ARRAY, BYREF, no BLOBs")
	TEST_VARIATION(10, 		L"ARRAY, BYREF, with BLOBs")
	TEST_VARIATION(11, 		L"OPTIMIZED Even Columns with BLOBs")
	TEST_VARIATION(12, 		L"All Columns with BLOBs at End")
	TEST_VARIATION(13, 		L"BLOBs at End only")
	TEST_VARIATION(14, 		L"Very WIDE table")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Rowsets)
//--------------------------------------------------------------------
// @class Interesting rowsets
//
class TCIRowset_Rowsets : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Rowsets,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Empty Rowset: Fetch one row forward and skip forward
	int Variation_1();
	// @cmember Empty Rowset: fetch backward
	int Variation_2();
	// @cmember Empty Rowset, Skip one row backward
	int Variation_3();
	// @cmember single row, mutiple columns rowset, fetch two rows forward.  RestartPosition should succeed
	int Variation_4();
	// @cmember single row, multiple columns rowset, skip one row forward and fetch one row forward.
	int Variation_5();
	// @cmember single row, singel column rowset, fetch one row backward
	int Variation_6();
	// @cmember single row, single column rowset.  Skip two rows backward.
	int Variation_7();
	// @cmember rowset with maximun # of columns in the rowset  Skip one row forward and fetch one row forward
	int Variation_8();
	// @cmember rowset with maximun # of columns in the rowset  Skip one row backward and fetch one row backward
	int Variation_9();
	// @cmember Strict sequential rowset, RestartPosition optimization
	int Variation_10();
	// @cmember Release Rowset with active Row handles - verify released
	int Variation_11();
	// @cmember Release Rowset with active accessors - verify released
	int Variation_12();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Rowsets)
#define THE_CLASS TCIRowset_Rowsets
BEG_TEST_CASE(TCIRowset_Rowsets, TCIRowset, L"Interesting rowsets")
	TEST_VARIATION(1, 		L"Empty Rowset: Fetch one row forward and skip forward")
	TEST_VARIATION(2, 		L"Empty Rowset: fetch backward")
	TEST_VARIATION(3, 		L"Empty Rowset, Skip one row backward")
	TEST_VARIATION(4, 		L"single row, mutiple columns rowset, fetch two rows forward.  RestartPosition should succeed")
	TEST_VARIATION(5, 		L"single row, multiple columns rowset, skip one row forward and fetch one row forward.")
	TEST_VARIATION(6, 		L"single row, singel column rowset, fetch one row backward")
	TEST_VARIATION(7, 		L"single row, single column rowset.  Skip two rows backward.")
	TEST_VARIATION(8, 		L"rowset with maximun # of columns in the rowset  Skip one row forward and fetch one row forward")
	TEST_VARIATION(9, 		L"rowset with maximun # of columns in the rowset  Skip one row backward and fetch one row backward")
	TEST_VARIATION(10, 		L"Strict sequential rowset, RestartPosition optimization")
	TEST_VARIATION(11, 		L"Release Rowset with active Row handles - verify released")
	TEST_VARIATION(12, 		L"Release Rowset with active accessors - verify released")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Properties)
//--------------------------------------------------------------------
// @class Rowset properties
//
class TCIRowset_Properties : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Properties,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IRowsetLocate should not change fetch position
	int Variation_1();
	// @cmember IRowsetScroll should not change fetch position
	int Variation_2();
	// @cmember IRowsetExactScroll shoud not change fetch position
	int Variation_3();
	// @cmember CANHOLDROWS = FALSE
	int Variation_4();
	// @cmember DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM
	int Variation_5();
	// @cmember DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL
	int Variation_6();
	// @cmember DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL - access in a random matter
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Properties)
#define THE_CLASS TCIRowset_Properties
BEG_TEST_CASE(TCIRowset_Properties, TCIRowset, L"Rowset properties")
	TEST_VARIATION(1, 		L"IRowsetLocate should not change fetch position")
	TEST_VARIATION(2, 		L"IRowsetScroll should not change fetch position")
	TEST_VARIATION(3, 		L"IRowsetExactScroll shoud not change fetch position")
	TEST_VARIATION(4, 		L"CANHOLDROWS = FALSE")
	TEST_VARIATION(5, 		L"DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM")
	TEST_VARIATION(6, 		L"DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL")
	TEST_VARIATION(7, 		L"DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL - access in a random matter")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Parameters)
//--------------------------------------------------------------------
// @class Different combination of parameters of IRowset methods
//
class TCIRowset_Parameters : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Parameters,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRows==0; AddRef and ReleaseRows do nothing
	int Variation_1();
	// @cmember cRows=1 and rghRows contain two row handles
	int Variation_2();
	// @cmember One element of rghRows is NULL
	int Variation_3();
	// @cmember Any array of duplicated HRows
	int Variation_4();
	// @cmember AddRef and Release a released row handle
	int Variation_5();
	// @cmember Call RestartPosition three times.  Fetch position is not changed.
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Parameters)
#define THE_CLASS TCIRowset_Parameters
BEG_TEST_CASE(TCIRowset_Parameters, TCIRowset, L"Different combination of parameters of IRowset methods")
	TEST_VARIATION(1, 		L"cRows==0; AddRef and ReleaseRows do nothing")
	TEST_VARIATION(2, 		L"cRows=1 and rghRows contain two row handles")
	TEST_VARIATION(3, 		L"One element of rghRows is NULL")
	TEST_VARIATION(4, 		L"Any array of duplicated HRows")
	TEST_VARIATION(5, 		L"AddRef and Release a released row handle")
	TEST_VARIATION(6, 		L"Call RestartPosition three times.  Fetch position is not changed.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Sequence)
//--------------------------------------------------------------------
// @class Calling sequence between interfaces and methods
//
class TCIRowset_Sequence : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Sequence,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember AddRef, AddRef,ReleaseRows,ReleaseRows,ReleaseRows.  RefCount==0
	int Variation_1();
	// @cmember AddRef, ReleaseRows,AddRef,AddRef,AddRef,ReleaseRows.  RefCount==3
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Sequence)
#define THE_CLASS TCIRowset_Sequence
BEG_TEST_CASE(TCIRowset_Sequence, TCIRowset, L"Calling sequence between interfaces and methods")
	TEST_VARIATION(1, 		L"AddRef, AddRef,ReleaseRows,ReleaseRows,ReleaseRows.  RefCount==0")
	TEST_VARIATION(2, 		L"AddRef, ReleaseRows,AddRef,AddRef,AddRef,ReleaseRows.  RefCount==3")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Boundary)
//--------------------------------------------------------------------
// @class Boundary conditions
//
class TCIRowset_Boundary : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Boundary,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember AddRefRows--rghRows==NULL;E_INVALIDARG
	int Variation_1();
	// @cmember AddRefRows--pcRefCounted==NULL;S_OK
	int Variation_2();
	// @cmember AddRefRows--rgRefCounts==NULL;S_OK
	int Variation_3();
	// @cmember AddRefRows--pcRefCounted==NULL && rgRefCounts==NULL;S_OK
	int Variation_4();
	// @cmember AddRefRows - before any rows are fetched
	int Variation_5();
	// @cmember GetData--pData==NULL;E_INVALIDARG
	int Variation_6();
	// @cmember GetNextRows--pcRowsObtained==NULL;E_INVALIDARG
	int Variation_7();
	// @cmember GetNextRows--prghRows==NULL;E_INVALIDARG
	int Variation_8();
	// @cmember ReleaseRows--rghRows==NULL;E_INVALIDARG
	int Variation_9();
	// @cmember ReleaseRows--pcReleased=NULL;S_OK
	int Variation_10();
	// @cmember ReleaseRows--rgRefCounts==NULL;S_OK
	int Variation_11();
	// @cmember ReleaseRows--pcRelease==NULL;rgRefCounts=NULL;S_OK
	int Variation_12();
	// @cmember ReleaseRows - before any rows are fetched
	int Variation_13();
	// @cmember GetNextRows--*pcRowsObtained==0, *prgRows!=NULL
	int Variation_14();
	// @cmember RestartPosition - Without calling GetData
	int Variation_15();
	// @cmember IUnknown::AddRef / Release
	int Variation_16();
	// @cmember IUnknown::QueryInterface
	int Variation_17();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Boundary)
#define THE_CLASS TCIRowset_Boundary
BEG_TEST_CASE(TCIRowset_Boundary, TCIRowset, L"Boundary conditions")
	TEST_VARIATION(1, 		L"AddRefRows--rghRows==NULL;E_INVALIDARG")
	TEST_VARIATION(2, 		L"AddRefRows--pcRefCounted==NULL;S_OK")
	TEST_VARIATION(3, 		L"AddRefRows--rgRefCounts==NULL;S_OK")
	TEST_VARIATION(4, 		L"AddRefRows--pcRefCounted==NULL && rgRefCounts==NULL;S_OK")
	TEST_VARIATION(5, 		L"AddRefRows - before any rows are fetched")
	TEST_VARIATION(6, 		L"GetData--pData==NULL;E_INVALIDARG")
	TEST_VARIATION(7, 		L"GetNextRows--pcRowsObtained==NULL;E_INVALIDARG")
	TEST_VARIATION(8, 		L"GetNextRows--prghRows==NULL;E_INVALIDARG")
	TEST_VARIATION(9, 		L"ReleaseRows--rghRows==NULL;E_INVALIDARG")
	TEST_VARIATION(10, 		L"ReleaseRows--pcReleased=NULL;S_OK")
	TEST_VARIATION(11, 		L"ReleaseRows--rgRefCounts==NULL;S_OK")
	TEST_VARIATION(12, 		L"ReleaseRows--pcRelease==NULL;rgRefCounts=NULL;S_OK")
	TEST_VARIATION(13, 		L"ReleaseRows - before any rows are fetched")
	TEST_VARIATION(14, 		L"GetNextRows--*pcRowsObtained==0, *prgRows!=NULL")
	TEST_VARIATION(15, 		L"RestartPosition - Without calling GetData")
	TEST_VARIATION(16, 		L"IUnknown::AddRef / Release")
	TEST_VARIATION(17, 		L"IUnknown::QueryInterface")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Transactions)
//--------------------------------------------------------------------
// @class Use the interface within a transaction.  Zombie State.
//
class TCIRowset_Transactions : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Transactions,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with retaining
	int Variation_1();
	// @cmember Commit without retaining.
	int Variation_2();
	// @cmember Abort with retaining.
	int Variation_3();
	// @cmember Abort without retaining.
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Transactions)
#define THE_CLASS TCIRowset_Transactions
BEG_TEST_CASE(TCIRowset_Transactions, CTransaction, L"Use the interface within a transaction.  Zombie State.")
	TEST_VARIATION(1, 		L"Commit with retaining")
	TEST_VARIATION(2, 		L"Commit without retaining.")
	TEST_VARIATION(3, 		L"Abort with retaining.")
	TEST_VARIATION(4, 		L"Abort without retaining.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Chapters)
//--------------------------------------------------------------------
// @class Chapters specific test
//
class TCIRowset_Chapters : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Chapters,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetNextRows: Invalid hChapter
	int Variation_1();
	// @cmember GetNextRows:  DB_INVALID_CHAPTER
	int Variation_2();
	// @cmember ReleaseChapter: Active Row handles
	int Variation_3();
	// @cmember ReleaseChapter: NULL
	int Variation_4();
	// @cmember RestartPosition: Invalid hChapter
	int Variation_5();
	// @cmember RestartPosition: NULL
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Chapters)
#define THE_CLASS TCIRowset_Chapters
BEG_TEST_CASE(TCIRowset_Chapters, TCIRowset, L"Chapters specific test")
	TEST_VARIATION(1, 		L"GetNextRows: Invalid hChapter")
	TEST_VARIATION(2, 		L"GetNextRows:  DB_INVALID_CHAPTER")
	TEST_VARIATION(3, 		L"ReleaseChapter: Active Row handles")
	TEST_VARIATION(4, 		L"ReleaseChapter: NULL")
	TEST_VARIATION(5, 		L"RestartPosition: Invalid hChapter")
	TEST_VARIATION(6, 		L"RestartPosition: NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_PendingChange)
//--------------------------------------------------------------------
// @class Row handles with pending changes
//
class TCIRowset_PendingChange : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_PendingChange,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember methods called with rows having pending changes will return DB_E_BADROWHANDLE
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_PendingChange)
#define THE_CLASS TCIRowset_PendingChange
BEG_TEST_CASE(TCIRowset_PendingChange, TCIRowset, L"Row handles with pending changes")
	TEST_VARIATION(1, 		L"methods called with rows having pending changes will return DB_E_BADROWHANDLE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_CanHoldRows)
//--------------------------------------------------------------------
// @class Can Hold Rows in the rowset
//
class TCIRowset_CanHoldRows : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_CanHoldRows,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Reteive two discontiguous row handles
	int Variation_1();
	// @cmember release a middle row; release the first and the last row
	int Variation_2();
	// @cmember Release the last row, retrieve one more row, ERROR returneed
	int Variation_3();
	// @cmember Addref Rows and release the first and the last rows
	int Variation_4();
	// @cmember Release Rowset with active Row handles - verify released
	int Variation_5();
	// @cmember RestartPosition - with outstanding row handles
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_CanHoldRows)
#define THE_CLASS TCIRowset_CanHoldRows
BEG_TEST_CASE(TCIRowset_CanHoldRows, TCIRowset, L"Can Hold Rows in the rowset")
	TEST_VARIATION(1, 		L"Reteive two discontiguous row handles")
	TEST_VARIATION(2, 		L"release a middle row; release the first and the last row")
	TEST_VARIATION(3, 		L"Release the last row, retrieve one more row, ERROR returneed")
	TEST_VARIATION(4, 		L"Addref Rows and release the first and the last rows")
	TEST_VARIATION(5, 		L"Release Rowset with active Row handles - verify released")
	TEST_VARIATION(6, 		L"RestartPosition - with outstanding row handles")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_HoldRows_Discon)
//--------------------------------------------------------------------
// @class Hold rows and discontiguous
//
class TCIRowset_HoldRows_Discon : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_HoldRows_Discon,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Skip one row forward and retrieve one row forward.  Repeat
	int Variation_1();
	// @cmember Retrieve the two rows, call RestartPosition
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_HoldRows_Discon)
#define THE_CLASS TCIRowset_HoldRows_Discon
BEG_TEST_CASE(TCIRowset_HoldRows_Discon, TCIRowset, L"Hold rows and discontiguous")
	TEST_VARIATION(1, 		L"Skip one row forward and retrieve one row forward.  Repeat")
	TEST_VARIATION(2, 		L"Retrieve the two rows, call RestartPosition")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_Discontiguous)
//--------------------------------------------------------------------
// @class The rowset can has discontiguous rows
//
class TCIRowset_Discontiguous : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_Discontiguous,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve one row, release it; retrieve another row
	int Variation_1();
	// @cmember Retrieve one row, addRef it and release it;  retrieve another row
	int Variation_2();
	// @cmember Retrieve one row handle, addRef it and release it; Call RestartPositon
	int Variation_3();
	// @cmember Retrieve three rwo handles together.  Release the middle row,  Succeed
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_Discontiguous)
#define THE_CLASS TCIRowset_Discontiguous
BEG_TEST_CASE(TCIRowset_Discontiguous, TCIRowset, L"The rowset can has discontiguous rows")
	TEST_VARIATION(1, 		L"Retrieve one row, release it; retrieve another row")
	TEST_VARIATION(2, 		L"Retrieve one row, addRef it and release it;  retrieve another row")
	TEST_VARIATION(3, 		L"Retrieve one row handle, addRef it and release it; Call RestartPositon")
	TEST_VARIATION(4, 		L"Retrieve three rwo handles together.  Release the middle row,  Succeed")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_MaxOpenRows)
//--------------------------------------------------------------------
// @class test max open rows
//
class TCIRowset_MaxOpenRows : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_MaxOpenRows,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember MaxOpenRows is unlimted.  Retrieve rows in a sequential manner.
	int Variation_1();
	// @cmember MaxOpenRows is unlimted.  Retrieve rows in a group.
	int Variation_2();
	// @cmember Set the maxopen rows to be 5.
	int Variation_3();
	// @cmember MaxOpenRows is not unlimited.  Test the limit
	int Variation_4();
	// @cmember DBPROP_MAXROWS - Get Value
	int Variation_5();
	// @cmember DBPROP_MAXROWS - Set Value
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_MaxOpenRows)
#define THE_CLASS TCIRowset_MaxOpenRows
BEG_TEST_CASE(TCIRowset_MaxOpenRows, TCIRowset, L"test max open rows")
	TEST_VARIATION(1, 		L"MaxOpenRows is unlimted.  Retrieve rows in a sequential manner.")
	TEST_VARIATION(2, 		L"MaxOpenRows is unlimted.  Retrieve rows in a group.")
	TEST_VARIATION(3, 		L"Set the maxopen rows to be 5.")
	TEST_VARIATION(4, 		L"MaxOpenRows is not unlimited.  Test the limit")
	TEST_VARIATION(5, 		L"DBPROP_MAXROWS - Get Value")
	TEST_VARIATION(6, 		L"DBPROP_MAXROWS - Set Value")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_ForwardOnly)
//--------------------------------------------------------------------
// @class ScrollBackwards_ForwardOnly
//
class TCIRowset_ScrollBackwards_ForwardOnly : public TCIRowset { 
private:
	// @cmember ForwardOnly array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_ForwardOnly,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_ForwardOnly)
#define THE_CLASS TCIRowset_ScrollBackwards_ForwardOnly
BEG_TEST_CASE(TCIRowset_ScrollBackwards_ForwardOnly, TCIRowset, L"ScrollBackwards_ForwardOnly")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_Static)
//--------------------------------------------------------------------
// @class ScrollBackwards_Static
//
class TCIRowset_ScrollBackwards_Static : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_Static,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_Static)
#define THE_CLASS TCIRowset_ScrollBackwards_Static
BEG_TEST_CASE(TCIRowset_ScrollBackwards_Static, TCIRowset, L"ScrollBackwards_Static")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_Keyset)
//--------------------------------------------------------------------
// @class ScrollBackwards_Keyset
//
class TCIRowset_ScrollBackwards_Keyset : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_Keyset,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_Keyset)
#define THE_CLASS TCIRowset_ScrollBackwards_Keyset
BEG_TEST_CASE(TCIRowset_ScrollBackwards_Keyset, TCIRowset, L"ScrollBackwards_Keyset")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_Dynamic)
//--------------------------------------------------------------------
// @class ScrollBackwards_Dynamic
//
class TCIRowset_ScrollBackwards_Dynamic : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_Dynamic,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_Dynamic)
#define THE_CLASS TCIRowset_ScrollBackwards_Dynamic
BEG_TEST_CASE(TCIRowset_ScrollBackwards_Dynamic, TCIRowset, L"ScrollBackwards_Dynamic")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_Locate)
//--------------------------------------------------------------------
// @class ScrollBackwards_Locate
//
class TCIRowset_ScrollBackwards_Locate : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_Locate,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_Locate)
#define THE_CLASS TCIRowset_ScrollBackwards_Locate
BEG_TEST_CASE(TCIRowset_ScrollBackwards_Locate, TCIRowset, L"ScrollBackwards_Locate")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_ScrollBackwards_QueryBased)
//--------------------------------------------------------------------
// @class ScrollBackwards_QueryBased
//
class TCIRowset_ScrollBackwards_QueryBased : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_ScrollBackwards_QueryBased,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow==0, cRowToSkip forward > # of rows in the table
	int Variation_1();
	// @cmember cRow==0, cRowToSkip backward > # of rows in the table
	int Variation_2();
	// @cmember cRow==0, cRowToSkip =1
	int Variation_3();
	// @cmember cRow==0, cRowToSkip = -1
	int Variation_4();
	// @cmember cRows is negative.
	int Variation_5();
	// @cmember skip one row backward and fetch 5 row forward
	int Variation_6();
	// @cmember retrieve 2 rows.  Skip 3 rows backwards
	int Variation_7();
	// @cmember skip  backward more than the number of rows in the rowset
	int Variation_8();
	// @cmember skip forward more than the number of rows in the rowset
	int Variation_9();
	// @cmember skip forward and to the last row. skip backward to the first row
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_ScrollBackwards_QueryBased)
#define THE_CLASS TCIRowset_ScrollBackwards_QueryBased
BEG_TEST_CASE(TCIRowset_ScrollBackwards_QueryBased, TCIRowset, L"ScrollBackwards_QueryBased")
	TEST_VARIATION(1, 		L"cRow==0, cRowToSkip forward > # of rows in the table")
	TEST_VARIATION(2, 		L"cRow==0, cRowToSkip backward > # of rows in the table")
	TEST_VARIATION(3, 		L"cRow==0, cRowToSkip =1")
	TEST_VARIATION(4, 		L"cRow==0, cRowToSkip = -1")
	TEST_VARIATION(5, 		L"cRows is negative.")
	TEST_VARIATION(6, 		L"skip one row backward and fetch 5 row forward")
	TEST_VARIATION(7, 		L"retrieve 2 rows.  Skip 3 rows backwards")
	TEST_VARIATION(8, 		L"skip  backward more than the number of rows in the rowset")
	TEST_VARIATION(9, 		L"skip forward more than the number of rows in the rowset")
	TEST_VARIATION(10, 		L"skip forward and to the last row. skip backward to the first row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_ForwardOnly)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_ForwardOnly : public TCIRowset { 
private:
	// @cmember ForwardOnly array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_ForwardOnly,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_ForwardOnly)
#define THE_CLASS TCIRowset_FetchBackwards_ForwardOnly
BEG_TEST_CASE(TCIRowset_FetchBackwards_ForwardOnly, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_Static)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_Static : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_Static,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_Static)
#define THE_CLASS TCIRowset_FetchBackwards_Static
BEG_TEST_CASE(TCIRowset_FetchBackwards_Static, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_Keyset)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_Keyset : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_Keyset,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_Keyset)
#define THE_CLASS TCIRowset_FetchBackwards_Keyset
BEG_TEST_CASE(TCIRowset_FetchBackwards_Keyset, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_Dynamic)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_Dynamic : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_Dynamic,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_Dynamic)
#define THE_CLASS TCIRowset_FetchBackwards_Dynamic
BEG_TEST_CASE(TCIRowset_FetchBackwards_Dynamic, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_Locate)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_Locate : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_Locate,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_Locate)
#define THE_CLASS TCIRowset_FetchBackwards_Locate
BEG_TEST_CASE(TCIRowset_FetchBackwards_Locate, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_FetchBackwards_QueryBased)
//--------------------------------------------------------------------
// @class fetch backwards
//
class TCIRowset_FetchBackwards_QueryBased : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_FetchBackwards_QueryBased,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
	int Variation_1();
	// @cmember fetch forward and backward of all row handles. one at a time
	int Variation_2();
	// @cmember fetch forward and backward of all row handles
	int Variation_3();
	// @cmember fetch 3 rows backwards
	int Variation_4();
	// @cmember fetch two rows then 3 row backwards
	int Variation_5();
	// @cmember skip one row forward and fetch 2 rows backwards
	int Variation_6();
	// @cmember skip 2 rows forward and fetch 3 rows forward
	int Variation_7();
	// @cmember fetch three rows forward and repeat
	int Variation_8();
	// @cmember skip forward 2 rows and fetch 4 rows forward
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_FetchBackwards_QueryBased)
#define THE_CLASS TCIRowset_FetchBackwards_QueryBased
BEG_TEST_CASE(TCIRowset_FetchBackwards_QueryBased, TCIRowset, L"fetch backwards")
	TEST_VARIATION(1, 		L"cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"fetch forward and backward of all row handles. one at a time")
	TEST_VARIATION(3, 		L"fetch forward and backward of all row handles")
	TEST_VARIATION(4, 		L"fetch 3 rows backwards")
	TEST_VARIATION(5, 		L"fetch two rows then 3 row backwards")
	TEST_VARIATION(6, 		L"skip one row forward and fetch 2 rows backwards")
	TEST_VARIATION(7, 		L"skip 2 rows forward and fetch 3 rows forward")
	TEST_VARIATION(8, 		L"fetch three rows forward and repeat")
	TEST_VARIATION(9, 		L"skip forward 2 rows and fetch 4 rows forward")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_AllProperties)
//--------------------------------------------------------------------
// @class Can Hold rows + fetch backwards + scroll backwards + Discontiguous
//
class TCIRowset_AllProperties : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_AllProperties,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRow<0, skip backward 4 rows
	int Variation_1();
	// @cmember cRow<0, skip bakcward # of rows in the rowset
	int Variation_2();
	// @cmember skip two rows backwards and fetch one row bakwards
	int Variation_3();
	// @cmember retrieve the rows in the order of 1, 5, 3, 2,4
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_AllProperties)
#define THE_CLASS TCIRowset_AllProperties
BEG_TEST_CASE(TCIRowset_AllProperties, TCIRowset, L"Can Hold rows + fetch backwards + scroll backwards + Discontiguous")
	TEST_VARIATION(1, 		L"cRow<0, skip backward 4 rows")
	TEST_VARIATION(2, 		L"cRow<0, skip bakcward # of rows in the rowset")
	TEST_VARIATION(3, 		L"skip two rows backwards and fetch one row bakwards")
	TEST_VARIATION(4, 		L"retrieve the rows in the order of 1, 5, 3, 2,4")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIRowset_NoDiscontiguous)
//--------------------------------------------------------------------
// @class test a rowset with properties of CanHoldRows, CanScrollBackwards, and CanFetchBackwards
//
class TCIRowset_NoDiscontiguous : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIRowset_NoDiscontiguous,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember retrieve 5 row handles in the order of 3,4,2,5,1
	int Variation_1();
	// @cmember skip one row forward and fetch one row forward
	int Variation_2();
	// @cmember skip 2 rows backwards and fetch 3 rows backwards
	int Variation_3();
	// @cmember RestartPosition - with outstanding row handles
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIRowset_NoDiscontiguous)
#define THE_CLASS TCIRowset_NoDiscontiguous
BEG_TEST_CASE(TCIRowset_NoDiscontiguous, TCIRowset, L"test a rowset with properties of CanHoldRows, CanScrollBackwards, and CanFetchBackwards")
	TEST_VARIATION(1, 		L"retrieve 5 row handles in the order of 3,4,2,5,1")
	TEST_VARIATION(2, 		L"skip one row forward and fetch one row forward")
	TEST_VARIATION(3, 		L"skip 2 rows backwards and fetch 3 rows backwards")
	TEST_VARIATION(4, 		L"RestartPosition - with outstanding row handles")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(KeysetCursor)
//--------------------------------------------------------------------
// @class test keyset driven cursor model
//
class KeysetCursor : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(KeysetCursor,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember retrieve 5 row handles in the order of 3,4,2,5,1
	int Variation_1();
	// @cmember skip one row forward and fetch one row forward
	int Variation_2();
	// @cmember skip 2 rows backwards and fetch 3 rows backwards
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(KeysetCursor)
#define THE_CLASS KeysetCursor
BEG_TEST_CASE(KeysetCursor, TCIRowset, L"test keyset driven cursor model")
	TEST_VARIATION(1, 		L"retrieve 5 row handles in the order of 3,4,2,5,1")
	TEST_VARIATION(2, 		L"skip one row forward and fetch one row forward")
	TEST_VARIATION(3, 		L"skip 2 rows backwards and fetch 3 rows backwards")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DynamicCursor)
//--------------------------------------------------------------------
// @class test dynamic cursor
//
class DynamicCursor : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DynamicCursor,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember retrieve 5 row handles in the order of 3,4,2,5,1
	int Variation_1();
	// @cmember skip one row forward and fetch one row forward
	int Variation_2();
	// @cmember skip 2 rows and fetch 3 rows
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(DynamicCursor)
#define THE_CLASS DynamicCursor
BEG_TEST_CASE(DynamicCursor, TCIRowset, L"test dynamic cursor")
	TEST_VARIATION(1, 		L"retrieve 5 row handles in the order of 3,4,2,5,1")
	TEST_VARIATION(2, 		L"skip one row forward and fetch one row forward")
	TEST_VARIATION(3, 		L"skip 2 rows and fetch 3 rows")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Bookmarks)
//*-----------------------------------------------------------------------
// @class Test bookmarks on and off
//
class Bookmarks : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmarks,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Bookmarks == TRUE, ForwardOnly
	int Variation_1();
	// @cmember Bookmarks == TRUE, Static
	int Variation_2();
	// @cmember Bookmarks == TRUE, Keyset
	int Variation_3();
	// @cmember Bookmarks == TRUE, Dynamic
	int Variation_4();
	// @cmember Bookmarks == FALSE, ForwardOnly
	int Variation_5();
	// @cmember Bookmarks == FALSE, Static
	int Variation_6();
	// @cmember Bookmarks == FALSE, Keyset
	int Variation_7();
	// @cmember Bookmarks == FALSE, Dynamic
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Bookmarks)
#define THE_CLASS Bookmarks
BEG_TEST_CASE(Bookmarks, TCIRowset, L"Test bookmarks on and off")
	TEST_VARIATION(1, 		L"Bookmarks == TRUE, ForwardOnly")
	TEST_VARIATION(2, 		L"Bookmarks == TRUE, Static")
	TEST_VARIATION(3, 		L"Bookmarks == TRUE, Keyset")
	TEST_VARIATION(4, 		L"Bookmarks == TRUE, Dynamic")
	TEST_VARIATION(5, 		L"Bookmarks == FALSE, ForwardOnly")
	TEST_VARIATION(6, 		L"Bookmarks == FALSE, Static")
	TEST_VARIATION(7, 		L"Bookmarks == FALSE, Keyset")
	TEST_VARIATION(8, 		L"Bookmarks == FALSE, Dynamic")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Status)
//--------------------------------------------------------------------
// @class test the  DBROWSTATUS array
//
class Status : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Status,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_CANTFECTHBACKWARDS, DB_E_CANTSCROLLBACKWARDS
	int Variation_1();
	// @cmember An array of success scenario
	int Variation_2();
	// @cmember An array of invalid row handle
	int Variation_3();
	// @cmember An array of valid and unvalid row handle
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Status)
#define THE_CLASS Status
BEG_TEST_CASE(Status, TCIRowset, L"test the  DBROWSTATUS array")
	TEST_VARIATION(1, 		L"DB_E_CANTFECTHBACKWARDS, DB_E_CANTSCROLLBACKWARDS")
	TEST_VARIATION(2, 		L"An array of success scenario")
	TEST_VARIATION(3, 		L"An array of invalid row handle")
	TEST_VARIATION(4, 		L"An array of valid and unvalid row handle")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ExtendedErrors : public TCIRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	//@cmember Extended error object

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,TCIRowset);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ISupportErrorInfo for all rowset interfaces...
	int Variation_1();
	// @cmember Valid IRowset calls with previous error object existing.
	int Variation_2();
	// @cmember Valid IRowset calls with previous error object existing.
	int Variation_3();
	// @cmember E_INVALIDARG GetData call with previous error object existing
	int Variation_4();
	// @cmember E_INVALIDARG AddRefRows call with previous error object existing
	int Variation_5();
	// @cmember DB_E_ROWSNOTRELEASED RestartPosition call with previous error object existing
	int Variation_6();
	// @cmember DB_S_ENDOFROWSET GetNextRows call with previous error object existing
	int Variation_7();
	// @cmember E_INVALIDARG ReleaseRows call with previous error object existing
	int Variation_8();
	// @cmember E_INVALIDARG GetData call with no previous error object existing
	int Variation_9();
	// @cmember E_INVALIDARG AddRefRows call with no previous error object existing
	int Variation_10();
	// @cmember DB_E_ROWSNOTRELEASED RestartPosition call with no previous error object existing
	int Variation_11();
	// @cmember DB_S_ENDOFROWSET GetNextRows call with no previous error object existing
	int Variation_12();
	// @cmember E_INVALIDARG ReleaseRows call with no previous error object existing
	int Variation_13();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, TCIRowset, L"Extended Errors")
	TEST_VARIATION(1, 		L"ISupportErrorInfo for all rowset interfaces...")
	TEST_VARIATION(2, 		L"Valid IRowset calls with previous error object existing.")
	TEST_VARIATION(3, 		L"Valid IRowset calls with previous error object existing.")
	TEST_VARIATION(4, 		L"E_INVALIDARG GetData call with previous error object existing")
	TEST_VARIATION(5, 		L"E_INVALIDARG AddRefRows call with previous error object existing")
	TEST_VARIATION(6, 		L"DB_E_ROWSNOTRELEASED RestartPosition call with previous error object existing")
	TEST_VARIATION(7, 		L"DB_S_ENDOFROWSET GetNextRows call with previous error object existing")
	TEST_VARIATION(8, 		L"E_INVALIDARG ReleaseRows call with previous error object existing")
	TEST_VARIATION(9, 		L"E_INVALIDARG GetData call with no previous error object existing")
	TEST_VARIATION(10, 		L"E_INVALIDARG AddRefRows call with no previous error object existing")
	TEST_VARIATION(11, 		L"DB_E_ROWSNOTRELEASED RestartPosition call with no previous error object existing")
	TEST_VARIATION(12, 		L"DB_S_ENDOFROWSET GetNextRows call with no previous error object existing")
	TEST_VARIATION(13, 		L"E_INVALIDARG ReleaseRows call with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END
 
// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(32, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIRowset_RetrieveData)
	TEST_CASE(2, TCIRowset_Rowsets)
	TEST_CASE(3, TCIRowset_Properties)
	TEST_CASE(4, TCIRowset_Parameters)
	TEST_CASE(5, TCIRowset_Sequence)
	TEST_CASE(6, TCIRowset_Boundary)
	TEST_CASE(7, TCIRowset_Transactions)
	TEST_CASE(8, TCIRowset_Chapters)
	TEST_CASE(9, TCIRowset_PendingChange)
	TEST_CASE(10, TCIRowset_CanHoldRows)
	TEST_CASE(11, TCIRowset_HoldRows_Discon)
	TEST_CASE(12, TCIRowset_Discontiguous)
	TEST_CASE(13, TCIRowset_MaxOpenRows)
	TEST_CASE(14, TCIRowset_ScrollBackwards_ForwardOnly)
	TEST_CASE(15, TCIRowset_ScrollBackwards_Static)
	TEST_CASE(16, TCIRowset_ScrollBackwards_Keyset)
	TEST_CASE(17, TCIRowset_ScrollBackwards_Dynamic)
	TEST_CASE(18, TCIRowset_ScrollBackwards_Locate)
	TEST_CASE(19, TCIRowset_ScrollBackwards_QueryBased)
	TEST_CASE(20, TCIRowset_FetchBackwards_ForwardOnly)
	TEST_CASE(21, TCIRowset_FetchBackwards_Static)
	TEST_CASE(22, TCIRowset_FetchBackwards_Keyset)
	TEST_CASE(23, TCIRowset_FetchBackwards_Dynamic)
	TEST_CASE(24, TCIRowset_FetchBackwards_Locate)
	TEST_CASE(25, TCIRowset_FetchBackwards_QueryBased)
	TEST_CASE(26, TCIRowset_AllProperties)
	TEST_CASE(27, TCIRowset_NoDiscontiguous)
	TEST_CASE(28, KeysetCursor)
	TEST_CASE(29, DynamicCursor)
	TEST_CASE(30, Bookmarks)
	TEST_CASE(31, Status)
	TEST_CASE(32, ExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIRowset_RetrieveData)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_RetrieveData - Retrieve data of all types from the rowset without coercion
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_RetrieveData::Init()
{
	return TCIRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Default Accessor - no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_1()
{
	TBEGIN

	//open rowset with desired properties bind all columns
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		
	//verify the row set
	TESTC(VerifyAllRows())

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Default Accessor  - with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_2()
{
	TBEGIN
	
	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);

	//Requires IID_IRowsetLocate to position on Blobs
	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);
		
	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc BYREF accessor, no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_3()
{
	TBEGIN
	DBORDINAL i=0;

	//Requires IID_IRowsetLocate to position on Blobs
	//open rowset with desired properties
	//bind all columns, forward binding, all columns are ORed with _BYREF
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, VARIABLE_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_BYREF)==S_OK);

	//Make sure that all Variable Columns can be converted to BYREF
	for(i=0; i<m_cBindings; i++)
	{
		DBTYPE wFromType	= m_rgBinding[i].wType & ~DBTYPE_BYREF;
		DBTYPE wToType		= m_rgBinding[i].wType;
		TESTC_PROVIDER(CanConvert(m_pIRowset, wFromType, wToType)); 
	}

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc BYREF with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_4()
{
	TBEGIN
	DBORDINAL i=0;

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);

	//open rowset with desired properties
	//bind all columns, forward binding, all columns are ORed with _BYREF
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, VARIABLE_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_BYREF, BLOB_LONG)==S_OK);

	//Make sure that all Variable Columns can be converted to BYREF
	for(i=0; i<m_cBindings; i++)
	{
		DBTYPE wFromType	= m_rgBinding[i].wType & ~DBTYPE_BYREF;
		DBTYPE wToType		= m_rgBinding[i].wType;
		TESTC_PROVIDER(CanConvert(m_pIRowset, wFromType, wToType)); 
	}

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc OPTIMIZED, BLOBs only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_5()
{
	TBEGIN

	//open rowset with desired properties
	//bind all columns, forward binding, all columns are ORed with _BYREF
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		DBPART_ALL, VARIABLE_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF)==S_OK);

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc VECTOR, BYREF, with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_6()
{
	TBEGIN

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);

	//CreateRowset
	TESTC_(CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL), S_OK);

	//BYREF is not allowed to be used with other modifiers:
	//Should fail with DB_E_ERRORSOCCURRED and DBBINDSTATUS_BADBINDINFO
	//Need to also handle deferred validation...
	TESTC(VerifyAccessorValidation(this, DBACCESSOR_ROWDATA, NO_BLOB_COLS, 
		DB_E_UNSUPPORTEDCONVERSION, DBBINDSTATUS_BADBINDINFO, FALSE/*fAllowSuccess*/, 
		ALL_COLS_EXCEPTBOOKMARK, NULL/*prgBindStatus*/, NULL/*pfDefferred*/, NO_COLS_BY_REF,
		DBTYPE_BYREF | DBTYPE_VECTOR));

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc VECTOR, with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_7()
{
	TBEGIN

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);

	//open rowset with desired properties
	//bind all columns, forward binding, all columns are ORed with _BYREF
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc OPTIMIZED, BYREF, with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_8()
{
	TBEGIN
	DBORDINAL i=0;

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);

	//open rowset with desired properties
	//bind all columns, forward binding, all columns are ORed with _BYREF
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, VARIABLE_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_BYREF, BLOB_LONG)==S_OK);

	//Make sure that all Variable Columns can be converted to BYREF
	for(i=0; i<m_cBindings; i++)
	{
		DBTYPE wFromType	= m_rgBinding[i].wType & ~DBTYPE_BYREF;
		DBTYPE wToType		= m_rgBinding[i].wType;
		TESTC_PROVIDER(CanConvert(m_pIRowset, wFromType, wToType)); 
	}

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ARRAY, BYREF, no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_9()
{
	TBEGIN

	//CreateRowset
	TESTC_(CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL), S_OK);

	//BYREF is not allowed to be used with other modifiers:
	//Should fail with DB_E_ERRORSOCCURRED and DBBINDSTATUS_BADBINDINFO
	//Need to also handle deferred validation...
	TESTC(VerifyAccessorValidation(this, DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED, NO_BLOB_COLS, 
		DB_E_UNSUPPORTEDCONVERSION, DBBINDSTATUS_BADBINDINFO, FALSE/*fAllowSuccess*/, 
		ALL_COLS_BOUND, NULL/*prgBindStatus*/, NULL/*pfDefferred*/, NO_COLS_BY_REF,
		DBTYPE_BYREF | DBTYPE_ARRAY));

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ARRAY, BYREF, with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_10()
{
	TBEGIN

	//Requires DBPROP_IRowsetLocate
	SetSettableProperty(DBPROP_IRowsetLocate);

	//CreateRowset
	TESTC_(CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL), S_OK);

	//BYREF is not allowed to be used with other modifiers:
	//Should fail with DB_E_ERRORSOCCURRED and DBBINDSTATUS_BADBINDINFO
	//Need to also handle deferred validation...
	TESTC(VerifyAccessorValidation(this, DBACCESSOR_ROWDATA, BLOB_LONG, 
		DB_E_UNSUPPORTEDCONVERSION, DBBINDSTATUS_BADBINDINFO, FALSE/*fAllowSuccess*/, 
		ALL_COLS_EXCEPTBOOKMARK, NULL/*prgBindStatus*/, NULL/*pfDefferred*/, NO_COLS_BY_REF,
		DBTYPE_BYREF | DBTYPE_ARRAY));

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc OPTIMIZED Even Columns with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_11()
{
	TBEGIN

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		DBPART_ALL, EVEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc All Columns with BLOBs at End
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_12()
{
	TBEGIN

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_ALL_WITH_BLOB_AT_END, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc BLOBs at End only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_RetrieveData::Variation_13()
{
	TBEGIN
	ULONG i=0;

	//May require IRowsetLocate to position on Blobs
	SetSettableProperty(DBPROP_IRowsetLocate);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_ALL_WITH_BLOB_AT_END, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, VARIABLE_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_BYREF, BLOB_LONG)==S_OK);

	//Make sure that all Variable Columns can be converted to BYREF
	for(i=0; i<m_cBindings; i++)
	{
		DBTYPE wFromType	= m_rgBinding[i].wType & ~DBTYPE_BYREF;
		DBTYPE wToType		= m_rgBinding[i].wType;
		TESTC_PROVIDER(CanConvert(m_pIRowset, wFromType, wToType)); 
	}

	//verify the row set
	TESTC(VerifyAllRows());

CLEANUP:
	//drop the rowset
	DropRowset();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Very WIDE table (a special variation for covering BUG 57048)
//
// @rdesc TEST_PASS or TEST_FAIL
//
static void SetCommandProperties(/*in*/ICommandText* pCommand);

int TCIRowset_RetrieveData::Variation_14()
{
	TBEGIN

	static const WCHAR wszCREATE_TABLE_BY_SELECT[] = L"select 'text' as c0, '' as c1, '' as c2, '' as c3, '' as c4, '' as c5, '' as c6,"
		L"'' as c7, '' as c8, '' as c9, '' as c10, '' as c11, '' as c12, '' as c13, '' as c14, '' as c15, '' as c16, '' as c17, "
		L"'' as c18, '' as c19, '' as c20, '' as c21, '' as c22, '' as c23, '' as c24, '' as c25, '' as c26, '' as c27, '' as c28, "
		L"'' as c29, '' as c30, '' as c31, '' as c32, '' as c33, '' as c34, '' as c35, '' as c36, '' as c37, '' as c38, '' as c39, "
		L"'' as c40, '' as c41, '' as c42, '' as c43, '' as c44, '' as c45, '' as c46, '' as c47, '' as c48, '' as c49, '' as c50, "
		L"'' as c51, '' as c52, '' as c53, '' as c54, '' as c55, '' as c56, '' as c57, '' as c58, '' as c59, '' as c60, '' as c61, "
		L"'' as c62, '' as c63, '' as c64, '' as c65, '' as c66, '' as c67, '' as c68, '' as c69, '' as c70, '' as c71, '' as c72, "
		L"'' as c73, '' as c74, '' as c75, '' as c76, '' as c77, '' as c78, '' as c79, '' as c80, '' as c81, '' as c82, '' as c83, "
		L"'' as c84, '' as c85, '' as c86, '' as c87, '' as c88, '' as c89, '' as c90, '' as c91, '' as c92, '' as c93, '' as c94, "
		L"'' as c95, '' as c96, '' as c97, '' as c98, '' as c99, '' as c100, '' as c101, '' as c102, '' as c103, '' as c104, '' as c105, "
		L"'' as c106, '' as c107, '' as c108, '' as c109, '' as c110, '' as c111, '' as c112, '' as c113, '' as c114, '' as c115, "
		L"'' as c116, '' as c117, '' as c118, '' as c119, '' as c120, '' as c121, '' as c122, '' as c123, '' as c124, '' as c125, "
		L"'' as c126, '' as c127, '' as c128, '' as c129, '' as c130, '' as c131, '' as c132, '' as c133, '' as c134, '' as c135, "
		L"'' as c136, '' as c137, '' as c138, '' as c139, '' as c140, '' as c141, '' as c142, '' as c143, '' as c144, '' as c145, "
		L"'' as c146, '' as c147, '' as c148, '' as c149, '' as c150, '' as c151, '' as c152, '' as c153, '' as c154, '' as c155, "
		L"'' as c156, '' as c157, '' as c158, '' as c159, '' as c160, '' as c161, '' as c162, '' as c163, '' as c164, '' as c165, "
		L"'' as c166, '' as c167, '' as c168, '' as c169, '' as c170, '' as c171, '' as c172, '' as c173, '' as c174, '' as c175, "
		L"'' as c176, '' as c177, '' as c178, '' as c179, '' as c180, '' as c181, '' as c182, '' as c183, '' as c184, '' as c185, "
		L"'' as c186, '' as c187, '' as c188, '' as c189, '' as c190, '' as c191, '' as c192, '' as c193, '' as c194, '' as c195, "
		L"'' as c196, '' as c197, '' as c198, '' as c199, '' as c200, '' as c201, '' as c202, '' as c203, '' as c204, '' as c205, "
		L"'' as c206, '' as c207, '' as c208, '' as c209, '' as c210, '' as c211, '' as c212, '' as c213, '' as c214, '' as c215, "
		L"'' as c216, '' as c217, '' as c218, '' as c219, '' as c220, '' as c221, '' as c222, '' as c223, '' as c224, '' as c225, "
		L"'' as c226, '' as c227, '' as c228, '' as c229, '' as c230, '' as c231, '' as c232, '' as c233, '' as c234, '' as c235, "
		L"'' as c236, '' as c237, '' as c238, '' as c239, '' as c240, '' as c241, '' as c242, '' as c243, '' as c244, '' as c245, "
		L"'' as c246, '' as c247, '' as c248, '' as c249 into %s";

	WCHAR buf[4000];						// command text buffer
	HRESULT hr = 0;
	CTable WideTable(pISession());			// dummy class for creating TableName
	bool bTableCreated = false;				// flag - table was created successfully

	ICommandText*	pICommandText = NULL;	// ICommandText interface pointer
	IRowset*		pIRowset = NULL;		// IRowset interface pointer

	HROW hRow = 0, *phRow = &hRow;
	DBCOUNTITEM cRowsObtained = 0;
	DBROWCOUNT 	RowsAffected = 0;
	DBREFCOUNT     RefCounts[] = { -1 };
	DBROWSTATUS    RowStatus[] = { -1 };

	// run this variation only for Kagera against Jet
	TEST_PROVIDER(wcsstr(m_pwszInitString, L"Microsoft Access Driver") != NULL);

	// create a new table name
	TESTC_((hr = WideTable.MakeTableName(NULL)), S_OK);

	// get IID_ICommandText interface
	if (!VerifyInterface(pICommand(), IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText))
		goto CLEANUP;
	SetCommandProperties(pICommandText);

	// create a table
	swprintf(buf, wszCREATE_TABLE_BY_SELECT, WideTable.GetTableName());
	TESTC_(hr = pICommandText->SetCommandText(DBGUID_DBSQL, &buf[0]),S_OK);
	TESTC_(hr = pICommandText->Execute(NULL,	IID_NULL, NULL,	&RowsAffected, NULL),S_OK);
	bTableCreated = true;

	// select * from this table
	swprintf(buf, wszSELECT_ALLFROMTBL, WideTable.GetTableName());
	TESTC_(hr = pICommandText->SetCommandText(DBGUID_DBSQL, &buf[0]),S_OK);
	TESTC_(hr = pICommandText->Execute(NULL, IID_IRowset, NULL, &RowsAffected,	(IUnknown**)&pIRowset), S_OK);

	// fetch all rows (1 row)
	TESTC_(hr = pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow), S_OK) ;


CLEANUP: 
	// release Rowset
	if (*phRow && pIRowset)
		pIRowset->ReleaseRows(1, &hRow, NULL, &RefCounts[0], &RowStatus[0]);
	if (pIRowset)
		pIRowset->Release();

	// delete table
	if (bTableCreated && pICommandText)
	{
		swprintf(buf, wszDROP_TABLE, WideTable.GetTableName());
		hr = pICommandText->SetCommandText(DBGUID_DBSQL, &buf[0]);
		hr = pICommandText->Execute(NULL,	IID_NULL, NULL,	&RowsAffected, NULL);
	}

	// release command
	if (pICommandText)
		pICommandText->Release();

	TRETURN
}


// set command properties for reproducing bug 57048
static void SetCommandProperties(/*in*/ICommandText* pCommand)
{
	ASSERT(pCommand);
	ICommandProperties*	pCommandProperties = NULL;	// ICommandText interface pointer

	// get pCommandProperties interface
	HRESULT hr = pCommand->QueryInterface(IID_ICommandProperties, (void**)&pCommandProperties);
	if (hr!=S_OK || !pCommandProperties)
		return;

	// for reproducing bug 57048 we need set 
	//		DBPROP_CANHOLDROWS, DBPROP_CANSCROLLBACKWARDS, DBPROP_CANFETCHBACKWARDS, 
	//		DBPROP_IRowsetChange to  VARIANT_TRUE
	//			and
	//		DBPROP_UPDATABILITY to 7
	DBPROPID bool_props[] = 
	{ 
		DBPROP_CANHOLDROWS, DBPROP_CANSCROLLBACKWARDS, DBPROP_CANFETCHBACKWARDS, DBPROP_IRowsetChange 
	};
	DBPROP props[sizeof(bool_props)/sizeof(DBPROPID) + 1];

	memset(&props[0], 0, sizeof(props));
	
	props[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	props[0].dwPropertyID = DBPROP_UPDATABILITY; 
	props[0].vValue.vt = VT_I4;
	props[0].vValue.intVal = 7;

	for (int i=1; i<=sizeof(bool_props)/sizeof(DBPROPID); i++)
	{
		props[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		props[i].dwPropertyID = bool_props[i-1]; 
		props[i].vValue.vt = VT_BOOL;
		props[i].vValue.boolVal = VARIANT_TRUE;
	}

	// define DBPROPSET and set indicated properties
	DBPROPSET propSets[1];
	propSets[0].cProperties = sizeof(props)/sizeof(DBPROP);
	propSets[0].guidPropertySet = DBPROPSET_ROWSET;
	propSets[0].rgProperties = &props[0];
	hr = pCommandProperties->SetProperties(1, propSets);

	if (pCommandProperties)
		pCommandProperties->Release();
}




// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_RetrieveData::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Rowsets)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Rowsets - Interesting rowsets
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Rowsets::Init()
{
	return TCIRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Empty Rowset: Fetch one row forward and skip forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_1()
{
	TBEGIN

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_EMPTYROWSET, IID_IRowset)==S_OK);

	//fetch rows forward row forward
	//Empty Rowset should retrieve no rows...
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS));

	//skip one row forward and fetch one row forward
	//Empty Rowset should be off of the rowset...
	//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
	//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
	TESTC(VerifyGetNextRows(1, 1, NO_ROWS, FORWARD, NO_ROWS));

CLEANUP:
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty Rowset: fetch backward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_2()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;

	//Requires DBPROP_CANFETCHBACKWARDS
	SetProperty(DBPROP_CANFETCHBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_EMPTYROWSET, IID_IRowset)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET));

	//fetch one row backward
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	//drop the rowset
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty Rowset, Skip one row backward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_3()
{
	TBEGIN
	HROW hRow = ULONG_MAX;

	//Requires DBPROP_CANSCROLLBACKWARDS
	SetProperty(DBPROP_CANSCROLLBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_ALLFROMTBL, IID_IRowset)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET));

	//fetch one row backward, no op
	TESTC_(GetNextRows(-1, 0, &hRow), S_OK);

CLEANUP:
	//drop the rowset
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


		
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc single row, mutiple columns rowset, fetch two rows forward.  RestartPosition should succeed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_4()
{
	TBEGIN
	DBROWCOUNT i=0;

	//open rowset with desired properties
	TESTC_(CreateRowset(SELECT_VALIDATIONORDER),S_OK);

	//Ask for the largest set of cRows...
	//Should just return DB_S_ENDOFROWSET with m_ulTableRows in the Rowset
	TESTC(VerifyGetNextRows(0, MAXDBROWCOUNT, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));
	
	//Loop through large values...
	//Some providers like to hard code to work arround LONG_MAX, LONG_MIN
	//Work arround this!
	for(i=m_ulTableRows+1; i<MAXDBROWCOUNT && (i+i>i); i+=i)
	{
		//Restart Position
		TESTC_(RestartPosition(),S_OK);

		//fetch one more row than the total rows forward
		//NOTE:  This call is only expecting m_ulTableRows to be returned...
		TESTC(VerifyGetNextRows(0, i, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));
	}

CLEANUP:
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc single row, multiple columns rowset, skip one row forward and fetch one row forward.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_5()
{
	TBEGIN
	HROW  hRow = DB_NULL_HROW;
	DBCOUNTITEM cRowsetRows = 0;

	//Requires DBPROP_CANFETCHBACKWARDS
	SetProperty(DBPROP_CANFETCHBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_COUNT)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET));

	//NOTE: We can't use m_ulTableRows since thats the totoal rows in the table,
	//not neccessarily in our query.  So we will just quickly caluclate it, incase
	//a provider has a different meaning of SELECT_COUNT in the INI file...
	cRowsetRows = GetTotalRows();

	//Skip all rows
	//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
	//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
	TESTC_(GetNextRows(cRowsetRows, 1, &hRow), DB_S_ENDOFROWSET);

	//Can't use VerfiyRestartPosition with SELECT_COUNT since the data
	//is not what we are expecting, its the total row count...
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetNextRows(cRowsetRows, -1, &hRow), S_OK);

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc single row, singel column rowset, fetch one row backward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_6()
{
	TBEGIN
	HROW  hRow = DB_NULL_HROW;

	//Requires DBPROP_CANFETCHBACKWARDS
	SetProperty(DBPROP_CANFETCHBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_COUNT)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET));

	//fetch one backward
	TESTC_(GetNextRows(0, -1, &hRow), S_OK);
	TESTC(VerifyReleaseRows(1, &hRow));

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc single row, single column rowset.  Skip two rows backward.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_7()
{
	TBEGIN
	HROW  hRow = DB_NULL_HROW;

	//Requires DBPROP_CANSCROLLBACKWARDS
	SetProperty(DBPROP_CANSCROLLBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_COUNT)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET));

	//fetch one backward
	TESTC_(GetNextRows(-1, 1, &hRow), S_OK);
	TESTC(VerifyReleaseRows(1, &hRow));

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc rowset with maximun # of columns in the rowset  Skip one row forward and fetch one row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_8()
{
	TBEGIN

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//skip one row forward, fetch one forward
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc rowset with maximun # of columns in the rowset  Skip one row backward and fetch one row backward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_9()
{
	TBEGIN

	//Requires DBPROP_CANFETCHBACKWARDS
	SetProperty(DBPROP_CANFETCHBACKWARDS);
	SetProperty(DBPROP_CANSCROLLBACKWARDS);

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET));
	TESTC_PROVIDER(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET));

	//skip one row backward, fetch one backward
	TESTC(VerifyGetNextRows(-1, -1, m_ulTableRows-1));

CLEANUP:
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Strict sequential rowset, RestartPosition optimization
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_10()
{
	TBEGIN
	TCIRowset RowsetA;
	HRESULT hr = S_OK;

	//Open a new rowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//On the "freshly" created rowset, there should be no reason a provider cannot
	//call RestartPosition.  Should not recieve DB_S_COMMANDREEXECUTED or DB_E_CANNOTRESTART.
	//Even on providers/rowsets that are a live data stream calling RestartPosition before ANY 
	//rows are fetched should be a common optmization, since more services will do this blindly 
	//when handed a rowset to ensure its at the head, as they are not aware of the current position...

	//Call directly as our helper maskes the normally warnings...
	TEST3C_(hr = RowsetA.pIRowset()->RestartPosition(NULL),S_OK,DB_S_COMMANDREEXECUTED,DB_E_CANNOTRESTART);

	//Its a good idea to allow this optimization, although its not required by the spec...
	TESTW_(hr, S_OK);

CLEANUP:
	RowsetA.DropRowset();
	TRETURN
}


// }}


// {{ TCW_VAR_PROTOTYPE(11)
//--------------------------------------------------------------------
// @mfunc Release Rowset with active Row handles - verify released
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_11()
{
	HROW	hRow = DB_NULL_HROW;
	ULONG   i=0;

	TCIRowset RowsetA;
	
	//open rowset with desired properties
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//We want to verify that the row handles do not take any references on the Rowset
	//According to the spec, Releasing the Rowset even with unreleased rows should
	//Still release the rowset and all assoicated rows and accessors...

	//Retrive the first row
	TESTC_(RowsetA.GetNextRows(0, 1, &hRow),S_OK);
	//Verify this row
	TESTC(RowsetA.VerifyRowHandles(hRow, FIRST_ROW));

CLEANUP:
	//Don't release row handles
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc Release Rowset with active accessors - verify released
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Rowsets::Variation_12()
{
	HROW		hRow = DB_NULL_HROW;
	HACCESSOR	hAccessor = NULL;

	TCIRowset RowsetA;
	
	//open rowset with desired properties
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//We want to verify that the row handles do not take any references on the Rowset
	//According to the spec, Releasing the Rowset even with unreleased rows should
	//Still release the rowset and all assoicated rows and accessors...

	//Create an Accessor
	TESTC_(GetAccessorAndBindings(RowsetA(), DBACCESSOR_ROWDATA, &hAccessor, NULL, NULL, NULL),S_OK);

	//Retrive the first row
	TESTC_(RowsetA.GetNextRows(0, 1, &hRow),S_OK);
	//Verify this row
	TESTC(RowsetA.VerifyRowHandles(hRow, FIRST_ROW));

CLEANUP:
	//Don't release row handles
	//Don't release hAccessor
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Rowsets::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Properties)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Properties - Rowset properties
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Properties::Init()
{
	return TCIRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate should not change fetch position
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Properties::Variation_1()
{
	TBEGIN
	IRowsetLocate*	pIRowsetLocate = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	HROW  hRow = NULL;	
	HROW* rghRows = NULL;
	DBBKMARK cbBookmark = 0;
	BYTE* pBookmark = NULL;

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowsetLocate)==S_OK);

	//Obtain IRowsetLocate
	TESTC(VerifyInterface(pIRowset(), IID_IRowsetLocate, ROWSET_INTERFACE, (IUnknown**)&pIRowsetLocate));

	//Get the Bookmark for the first row
	TESTC_(GetNextRows(0, 1, &hRow), S_OK);
	TESTC_(GetBookmark(hRow, &cbBookmark, &pBookmark),S_OK); 
	TESTC(VerifyReleaseRows(1, &hRow));

	//NextFetchPosition Should be unchanged
	//Also the 2.0 spec indicates lRowOffset is ignored
	TESTC_(GetNextRows(10000, 0, &hRow), S_OK);

	//get the first row
	TESTC_(pIRowsetLocate->GetRowsAt(NULL, NULL, cbBookmark, pBookmark, 0,1, &cRowsObtained, &rghRows),S_OK);
	TESTC(cRowsObtained == 1 && rghRows != NULL);

	//make sure the row fetched is the first row
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	TESTC(VerifyReleaseRows(cRowsObtained, rghRows));

	//the cursor is at the third row right now
	TESTC(VerifyGetNextRows(1, 1, THIRD_ROW));

CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	PROVIDER_FREE(pBookmark);	
	SAFE_RELEASE(pIRowsetLocate);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IRowsetScroll should not change fetch position
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Properties::Variation_2()
{
	TBEGIN
	DBCOUNTITEM cRowsObtained = 0;
	HROW  hRow = NULL;	
	HROW* rghRows = NULL;
	IRowsetScroll*	pIRowsetScroll = NULL;

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowsetScroll)==S_OK);

	//Obtain IRowsetScroll
	TESTC(VerifyInterface(pIRowset(), IID_IRowsetScroll, ROWSET_INTERFACE, (IUnknown**)&pIRowsetScroll));

	//Retreive the first row handle
	TESTC_(pIRowsetScroll->GetRowsAtRatio(NULL, NULL, 0, 1, ONE_ROW, &cRowsObtained, &rghRows),S_OK);
	TESTC(cRowsObtained == 1 && rghRows != NULL);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);

	//NextFetchPosition Should be unchanged by IRowsetScroll
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowsetScroll);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IRowsetExactScroll shoud not change fetch position
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Properties::Variation_3()
{
	//TODO Change to use IRowsetExactScroll for V2
	//Currently just uses IRowsetScroll

	TBEGIN
	DBCOUNTITEM cRowsObtained = 0;
	HROW  hRow = NULL;	
	HROW* rghRows = NULL;
	IRowsetScroll*	pIRowsetScroll = NULL;

	//open rowset with desired properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowsetScroll)==S_OK);

	//Obtain IRowsetScroll
	TESTC(VerifyInterface(pIRowset(), IID_IRowsetScroll, ROWSET_INTERFACE, (IUnknown**)&pIRowsetScroll));

	//Retreive the first row handle
	TESTC_(pIRowsetScroll->GetRowsAtRatio(NULL, NULL, 0, 1, ONE_ROW, &cRowsObtained, &rghRows),S_OK);
	TESTC(cRowsObtained == 1 && rghRows != NULL);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);

	//NextFetchPosition Should be unchanged by IRowsetScroll
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowsetScroll);
	DropRowset();
	TRETURN
}


// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc CANHOLDROWS = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Properties::Variation_4()
{ 
	HROW hRowA = INVALID(HROW);
	HROW hRowB = INVALID(HROW);
	HRESULT hr = S_OK;
	DBCOUNTITEM cRowsObtained = 0;

	//DBPROP_CANHOLDROWS = FALSE
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET,(void*)VARIANT_FALSE, DBTYPE_BOOL);
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Verify DBPROP_CANHOLDROWS = FALSE
	//NOTE: This is only a warning since its odd to be able to successfully set CANHOLDROWS=TRUE
	//above and then GetProperties returns FALSE.  More than likely a bug in the provider, but their
	//is the "rare" condition allowed by the spec for this one property, that if FALSE has no meaning 
	//for the provider and they may or may not return ROWSNOTRELEASED, then GetProperties can return
	//TRUE...
	TESTW(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, VARIANT_FALSE));

	//Fetch Row 1
	TESTC_(GetNextRows(&hRowA), S_OK);

	//Setting CANHOLROWS=FALSE, According to the spec indicates that holding rows may or 
	//may not be allowed, since you didn't request the ability to hold rows.  Prevents the 
	//provider from having artifical limiting functionality.

	//Fetch Row 2
	TEST3C_(hr = GetNextRows(0, 1, &cRowsObtained, &hRowB), S_OK, DB_S_ROWLIMITEXCEEDED, DB_E_ROWSNOTRELEASED);
	if(hr == S_OK)
	{
		//Able to hold row when CANHOLDROWS = FALSE
		TESTC(VerifyRowHandles(hRowB, SECOND_ROW));
	}
	else if(hr == DB_S_ROWLIMITEXCEEDED)
	{
		//Unable to have more than 1 row open
		TESTC(cRowsObtained == 0 /* && hRowB == NULL*/);  //TODO Spec issue:
		TESTC(m_ulMaxOpenRows == 1);
	}
	else
	{
		//Unable to hold rows when CANHOLDROWS = FALSE
		TESTC(cRowsObtained == 0 /* && hRowB == NULL*/);  //TODO Spec issue:
	}


CLEANUP:
	ReleaseRows(hRowA);
	ReleaseRows(hRowB);
	DropRowset();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Properties::Variation_5()
{ 
	TBEGIN
	ULONG_PTR ulValue = 0;

	//DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM
	//NOTE: If this property is not supported, then the provider has no problem is
	//getting the columns already fetched (order doesn't matter).  Providers would support
	//this property if the "random" mode is expensive and can provide  a more
	//efficent (less functional) mode.
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);

	//Create a rowset, (Accessor contains BLOB columns)
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//If the property is not supported, then the provider has no problem returning data in 
	//a random order. (ie: don't check the return code from GetProperties)
	if(!GetProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, &ulValue) || ulValue==DBPROPVAL_AO_RANDOM)
	{
		//Obtain all the rows at once, and verify
		//This tests the senario of being able to "see" BLOBs are the beyond 
		//the current fetch position, meaning the provider may have to "cache" the entire BLOB
		//in order to obtain th value (ie: meaning of DBPROP_ACCESSORODER)
		TESTC(VerifyGetNextRows(0, m_ulTableRows, FIRST_ROW));
	}

CLEANUP:
	DropRowset();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Properties::Variation_6()
{ 
	TBEGIN

	//DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL
	//NOTE: DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS is covered in the IStorage test...
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIAL, DBTYPE_I4);

	//Create a rowset, (Accessor contains BLOB columns)
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//Obtain all the rows one at a time, obtaining all rows and columns sequentially
	//NOTE: Even if Sequential could not be set (readonly or not supported), reading the data
	//in a sequential matter is the "least common denominator" and must work in any of the 
	//property value settings
	TESTC(VerifyAllRows());

CLEANUP:
	DropRowset();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL - access in a random matter
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Properties::Variation_7()
{ 
	TBEGIN
	ULONG_PTR ulValue = 0;
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;
	void* pData = NULL;
	HRESULT hr = S_OK;

	//DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIAL
	//NOTE: DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS is covered in the IStorage test...
	SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIAL, DBTYPE_I4);

	//Create a rowset, (Accessor contains BLOB columns)
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowset, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG)==S_OK);

	//Maybe the property was not supported, or notsettable.
	if(!GetProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, &ulValue) || ulValue==DBPROPVAL_AO_RANDOM)
	{
		//Obtain all the rows at once, and verify
		//This tests the senario of being able to "see" BLOBs are the beyond 
		//the current fetch position, meaning the provider may have to "cache" the entire BLOB
		//in order to obtain th value (ie: meaning of DBPROP_ACCESSORODER)
		TESTC(VerifyGetNextRows(0, m_ulTableRows, FIRST_ROW));
	}
	else
	{
		//Obtain all the rows in the rowset (or as many as possible)
		TEST2C_(GetNextRows(0, m_ulTableRows, &cRowsObtained, &rghRows),S_OK,DB_S_ROWLIMITEXCEEDED);

		//The property value is Sequential, so try to obtain the rows in a random order anyway...
		//ie: Common user senario: not realizing their is (yet another) property for this 
		//and doing the operations anyway...
		for(ULONG iRow=0; iRow<cRowsObtained; iRow++)
		{
			//Obtain the Data for this row...
			//NOTE: The provider still might be alb eto obtain previous fetched rows
			//depending upon the columns/data/cursor, but will more than likely fail for the
			//BLOB columns with DBSTATUS_E_UNAVAILABLE as spec'd
			TEST3C_(hr = GetRowData(rghRows[iRow], &m_pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
			
			if(hr == S_OK)
			{
				//All data was successfully retrived (verify row data)
				//We can't just call VerifyRowHandles since that calls GetData again.
				//Were as we are testing "sequential" and getting data again is not a 
				//sequential fasion and a provider could fail that...
				if(!CompareTableData(FIRST_ROW+iRow, m_pData, m_cBindings, m_rgBinding))
				{
					//Data incorrect for this row!
					TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
					QTESTC(FALSE);
				}
			}
			else
			{
				//Verify the errors...
				for(ULONG iBinding=0; iBinding<m_cBindings; iBinding++)
				{
					DBBINDING* pBinding = &m_rgBinding[iBinding];
					DBSTATUS dbStatus = STATUS_BINDING(*pBinding, m_pData);
					
					switch(dbStatus)
					{
						case DBSTATUS_S_OK:
						case DBSTATUS_S_ISNULL:
							//Verify the data so this successfully obtained column
							if(!CompareTableData(FIRST_ROW+iRow, m_pData, 1/*cBindings*/, pBinding))
							{
								//Data incorrect for this row!
								TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
								QTESTC(FALSE);
							}
							break;
					
						default:
							TESTC(dbStatus == DBSTATUS_E_UNAVAILABLE);
					};
				}
			}

			ReleaseRowData(m_pData, m_hAccessor, FALSE/*fFreeBuffer*/);
		}
	}

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	SAFE_FREE(rghRows);
	DropRowset();
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Properties::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Parameters)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Parameters - Different combination of parameters of IRowset methods
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Parameters::Init()
{
	if(TCIRowset::Init())
	{
		//open rowset with desired properties
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRows==0; AddRef and ReleaseRows do nothing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_1()
{
	TBEGIN
	HROW	hRow = DB_NULL_HROW;
	ULONG	rgRefCounts[1] = {0};

	//Obtain the first row
	TESTC_(GetNextRows(0, 1, &hRow), S_OK);

	//ReleasRows cRows==0 (do nothing)
	TESTC_(m_pIRowset->ReleaseRows(0, &hRow, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(1, rgRefCounts, 0));
	
	//AddRef cRows==0 (do nothing)
	TESTC_(m_pIRowset->AddRefRows(0, &hRow, rgRefCounts, NULL), S_OK);
	TESTC(VerifyRefCounts(1, rgRefCounts, 0));
	TESTC(VerifyReleaseRows(1, &hRow));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRows=1 and rghRows contain two row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_2()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW	rghRows[TWO_ROWS];
	ULONG	rgRefCounts[TWO_ROWS];
		
	//This variation requires more than 1 active row
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= TWO_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//set up an arry of two row handles.
	TESTC_(GetNextRows(0, 2, &cRowsObtained, &rghRows[0]), S_OK);

	//addRefRows
	TESTC_(m_pIRowset->AddRefRows(1, rghRows, rgRefCounts, NULL), S_OK);
	TESTC(VerifyRefCounts(1, rgRefCounts, 2));

	//releaseRows
	TESTC_(m_pIRowset->ReleaseRows(1, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(1, rgRefCounts, 1));
	TESTC(VerifyReleaseRows(2, rghRows, 0));

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc One element of rghRows is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_3()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW	rghRows[3] = { LONG_MIN, NULL, LONG_MAX };
	ULONG	rgRefCounts[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
	ULONG	rgRowStatus[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
		
	//Create a "fresh" rowset...
	TCIRowset RowsetA;
	TESTC_(RowsetA.CreateRowset(SELECT_VALIDATIONORDER),S_OK);

	//RestartPosition, verify
	TESTC(RowsetA.VerifyRestartPosition());

	//retrieve one row
	TESTC_(RowsetA.GetNextRows(0, 1, &cRowsObtained, &rghRows[1]), S_OK);

	//addRef the array of row handles (with - valid, valid)
	TESTC_(RowsetA()->AddRefRows(3, rghRows, rgRefCounts, rgRowStatus), DB_S_ERRORSOCCURRED);
	//addRef the array of row handles (with - NULL, valid)
	TESTC_(RowsetA()->AddRefRows(3, rghRows, NULL, rgRowStatus), DB_S_ERRORSOCCURRED);
	//addRef the array of row handles (with - valid, NULL)
	TESTC_(RowsetA()->AddRefRows(3, rghRows, rgRefCounts, NULL), DB_S_ERRORSOCCURRED);

	//Verify RowStatus
	TESTC(rgRowStatus[0] == DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[1] == DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2] == DBROWSTATUS_E_INVALID);
	
	//Verify Reference Counts
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 0));
	TESTC(VerifyRefCounts(1, &rgRefCounts[1], 4));
	TESTC(VerifyRefCounts(1, &rgRefCounts[2], 0));

	//Release the array of row handles.  rgRowOptions should be ignored
	rgRefCounts[0] = ULONG_MAX;
	TESTC_(RowsetA()->ReleaseRows(3, rghRows, INVALID(DBROWOPTIONS*), rgRefCounts, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(rgRowStatus[0] == DBROWSTATUS_E_INVALID);
	TESTC(rgRowStatus[1] == DBROWSTATUS_S_OK);
	TESTC(rgRowStatus[2] == DBROWSTATUS_E_INVALID);

	//Verify Reference Counts
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 0));
	TESTC(VerifyRefCounts(1, &rgRefCounts[1], 3));
	TESTC(VerifyRefCounts(1, &rgRefCounts[2], 0));
											  
	//Release the array of row handles.  (With - NULL, NULL, NULL)
	TESTC_(RowsetA()->ReleaseRows(3, rghRows, NULL, NULL, NULL), DB_S_ERRORSOCCURRED);
	//Release the array of row handles.  (With - NULL, NULL, valid)
	TESTC_(RowsetA()->ReleaseRows(3, rghRows, NULL, NULL, rgRowStatus), DB_S_ERRORSOCCURRED);
	//Release the array of row handles.  (With - NULL, valid, NULL)
	TESTC_(RowsetA()->ReleaseRows(3, rghRows, NULL, rgRefCounts, NULL), DB_S_ERRORSOCCURRED);

	TCOMPARE(rgRowStatus[0], DBROWSTATUS_E_INVALID);
	TCOMPARE(rgRowStatus[1], DBROWSTATUS_S_OK);
	TCOMPARE(rgRowStatus[2], DBROWSTATUS_E_INVALID);
	TESTC(VerifyRefCounts(3, rgRefCounts, 0));

CLEANUP:
	ReleaseRows(3, rghRows);	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Any array of duplicated HRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_4()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[3] = { NULL, NULL, NULL };
	ULONG	rgRefCounts[3] = { 0, 0, 0 };
		
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve two rows
	TESTC_(GetNextRows(0, 1, &rghRows[0]), S_OK);

	//Duplicate row handle
	rghRows[1]=rghRows[0];

	//addRef 
	TESTC_(m_pIRowset->AddRefRows(2, rghRows, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 2));
	TESTC(VerifyRefCounts(1, &rgRefCounts[1], 3));

	//releaseRows
	TESTC_(m_pIRowset->ReleaseRows(2, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 2));
	TESTC(VerifyRefCounts(1, &rgRefCounts[1], 1));

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc AddRef and Release a released row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_5()
{
	HRESULT hr = S_OK;
	ULONG	cRowsObtained = 0;
	HROW	hRow = NULL;
	ULONG	ulRefCount = 1;
	DBROWSTATUS dwRowStatus = 0;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Obtain and release a rows handle
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC(VerifyReleaseRows(1, &hRow));

	//Since "Released" RowHandles are really undefined by the spec.
	//The provider may still have them arround, or may not.  The consumer
	//Really should assume there gone, but some providers may just keep them arround
	//All we can really check for here, is that either they are alive to released.

	//AddRef - Released row handle
	hr = m_pIRowset->AddRefRows(1, &hRow, &ulRefCount, &dwRowStatus);
	TESTC(hr == S_OK || hr == DB_E_ERRORSOCCURRED);
	
	if(hr == S_OK)
	{
		TESTC(VerifyRefCounts(1, &ulRefCount, 1));
		TESTC(dwRowStatus  == DBROWSTATUS_S_OK);

		//Release - Valid row handle
		TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL, &ulRefCount, &dwRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(dwRowStatus == DBROWSTATUS_S_OK);
	}
	if(hr == DB_E_ERRORSOCCURRED)
	{
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(dwRowStatus  == DBROWSTATUS_E_INVALID);

		//Release - Released row handle
		TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL, &ulRefCount, &dwRowStatus), DB_E_ERRORSOCCURRED);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(dwRowStatus  == DBROWSTATUS_E_INVALID);
	}

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Call RestartPosition three times.  Fetch position is not changed.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Parameters::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//move the cursor away from the first row
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

CLEANUP:
	TRETURN;
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Parameters::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Sequence)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Sequence - Calling sequence between interfaces and methods
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Sequence::Init()
{
	if(TCIRowset::Init())
	{
		//open rowset with desired properties
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc AddRef, AddRef,ReleaseRows,ReleaseRows,ReleaseRows.  RefCount==0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Sequence::Variation_1()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW	rghRows[3];
	ULONG	rgRefCounts[3];

	//This variation requires more than 1 active row
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= THREE_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get 3 rows
	TEST2C_(GetNextRows(0, 3, &cRowsObtained, rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);

	//addRef
	TESTC_(m_pIRowset->AddRefRows(cRowsObtained, rghRows, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(cRowsObtained, rgRefCounts, 2));

	//addRef again
	TESTC_(m_pIRowset->AddRefRows(cRowsObtained, rghRows, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(cRowsObtained, rgRefCounts, 3));

	//release three times
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(cRowsObtained, rgRefCounts, 2));

	//second relase 
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(cRowsObtained, rgRefCounts, 1));

	//third relase
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC(VerifyRefCounts(cRowsObtained, rgRefCounts, 0));
	
CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc AddRef, ReleaseRows,AddRef,AddRef,AddRef,ReleaseRows.  RefCount==3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Sequence::Variation_2()
{
	HROW	hRow;
	ULONG	ulRefCount;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get a row handle.  Skip a row
	TESTC_(GetNextRows(1, 1, &hRow),S_OK);

	//AddRef: RefCounts==2
	TESTC_(m_pIRowset->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 2));

	//AddRef: RefCounts==3
	TESTC_(m_pIRowset->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 3));

	//AddRef: RefCounts==4
	TESTC_(m_pIRowset->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 4));

	//ReleaseRows:RefCounts==3
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 3));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Sequence::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Boundary - Boundary conditions
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Boundary::Init()
{
	if(TCIRowset::Init())
	{
		//open rowset with desired properties
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows--rghRows==NULL;E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_1()
{
	HROW rghRows[1];
	ULONG rgRefCounts[1];
	DBROWSTATUS rgRowStatus[1];
	
	//No-op
	TESTC_(m_pIRowset->AddRefRows(0, NULL, NULL, NULL),S_OK);
	TESTC_(m_pIRowset->AddRefRows(0, rghRows, NULL, NULL),S_OK);
	TESTC_(m_pIRowset->AddRefRows(0, rghRows, rgRefCounts, NULL),S_OK);
	TESTC_(m_pIRowset->AddRefRows(0, rghRows, rgRefCounts, rgRowStatus),S_OK);
	
	//E_INVALIDARG
	TESTC_(m_pIRowset->AddRefRows(1, NULL, NULL, NULL),E_INVALIDARG);
	TESTC_(m_pIRowset->AddRefRows(1, NULL, rgRefCounts, rgRowStatus),E_INVALIDARG);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows--pcRefCounted==NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_2()
{
	HROW	hRow;
	ULONG	ulRefCount;

	//get one row handle
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC_(m_pIRowset->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 2));

CLEANUP:
	ReleaseRows(1, &hRow);
	ReleaseRows(1, &hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows--rgRefCounts==NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_3()
{
	HROW			rghRows[1];
	DBROWSTATUS		rgRowStatus[1];	

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get two row handle
	TESTC_(GetNextRows(0, 1, rghRows),S_OK);
	TESTC_(m_pIRowset->AddRefRows(1, rghRows, NULL, rgRowStatus),S_OK);
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);

CLEANUP:
	ReleaseRows(1, rghRows);
	ReleaseRows(1, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows--pcRefCounted==NULL && rgRefCounts==NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_4()
{
	HROW	rghRows[1];

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get row handles
	TESTC_(GetNextRows(0, 1, rghRows),S_OK);
	TESTC_(m_pIRowset->AddRefRows(1, rghRows, NULL, NULL),S_OK);

CLEANUP:
	ReleaseRows(1, rghRows);
	ReleaseRows(1, rghRows);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows - before any rows are fetched
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Boundary::Variation_5()
{ 
	ULONG	cRowsObtained = 0;
	HROW	rghRows[3] = { LONG_MIN, NULL, LONG_MAX };
	ULONG	rgRefCounts[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
	ULONG	rgRowStatus[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
		
	//Create a "fresh" rowset...
	CRowset RowsetA;
	TESTC_(RowsetA.CreateRowset(SELECT_VALIDATIONORDER),S_OK);

	//Before fetching any rows, call AddRefRows...
	//This is a good senario since most providers allocate a row buffer at fetch time which
	//will be NULL for this senario.  Make sure they handle it...
	
	//No-op
	TESTC_(RowsetA()->AddRefRows(0, NULL, NULL, NULL),S_OK);
	TESTC_(RowsetA()->AddRefRows(0, rghRows, NULL, NULL),S_OK);
	TESTC_(RowsetA()->AddRefRows(0, rghRows, rgRefCounts, NULL),S_OK);
	TESTC_(RowsetA()->AddRefRows(0, rghRows, rgRefCounts, rgRowStatus),S_OK);
	
	//E_INVALIDARG
	TESTC_(RowsetA()->AddRefRows(1, NULL, NULL, NULL),E_INVALIDARG);
	TESTC_(RowsetA()->AddRefRows(1, NULL, rgRefCounts, rgRowStatus),E_INVALIDARG);
	
	//(3, invalid, valid, valid) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->AddRefRows(THREE_ROWS, rghRows, rgRefCounts, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRowStatus, DBROWSTATUS_E_INVALID));
	TESTC(VerifyArray(THREE_ROWS, rgRefCounts, 0));
	
	//(3, invalid, valid, NULL) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->AddRefRows(THREE_ROWS, rghRows, rgRefCounts, NULL), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRefCounts, 0));

	//(3, invalid, NULL, valid) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->AddRefRows(THREE_ROWS, rghRows, NULL, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRowStatus, DBROWSTATUS_E_INVALID));

	//(3, invalid, NULL, NULL) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->AddRefRows(THREE_ROWS, rghRows, NULL, NULL), DB_E_ERRORSOCCURRED);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetData--pData==NULL;E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_6()
{
	HROW		hRow = NULL;
	HACCESSOR	hNullAccessor = NULL;
	HRESULT		hr = S_OK;

	//GetData (pData==NULL, valid accessor) - E_INVALIDARG
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC_(m_pIRowset->GetData(hRow, m_hAccessor, NULL),E_INVALIDARG);

	//Create a "null" accessor (used when inserting data)
	TEST2C_(hr = pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA, 0, NULL, 0, &hNullAccessor, NULL),S_OK, DB_E_NULLACCESSORNOTSUPPORTED);
		
	//Verify NullAccessors are correctly not supported if error occurrs...
	if(hr == DB_E_NULLACCESSORNOTSUPPORTED)
	{
		//Make sure that IRowsetChange is not supported, or at least 
		//IRowsetChange::InsertRow is not supported...
		ULONG_PTR dwValue = 0;
		if(GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET))
			TESTC(GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &dwValue) && !(dwValue & DBPROPVAL_UP_INSERT));
	}
	else
	{
		//GetData (pData==NULL, "null" Accessor) - S_OK
		//NullAccessors are supported, only at the rowset level, 
		TESTC_(m_pIRowset->GetData(hRow, hNullAccessor, NULL),S_OK);
	}


CLEANUP:
	ReleaseRows(hRow);
	ReleaseAccessor(hNullAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows--pcRowsObtained==NULL;E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_7()
{
	HROW*	rghRows = NULL;

	//GetNextRows
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, NULL, &rghRows),E_INVALIDARG);
	TESTC(rghRows == NULL);

CLEANUP:
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows--prghRows==NULL;E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_8()
{
	DBCOUNTITEM cRowsObtained = 1;

	//GetNextRows
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, NULL),E_INVALIDARG);
	TESTC(cRowsObtained == 0);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ReleaseRows--rghRows==NULL;E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_9()
{
	HROW  rghRows[2];
	ULONG rgRowOptions[2];
	ULONG rgRefCounts[2];
	DBROWSTATUS rgRowStatus[2];
	
	//No-op case
	TESTC_(m_pIRowset->ReleaseRows(0, NULL, NULL, NULL, NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(0, rghRows, NULL, NULL, NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(0, rghRows, rgRowOptions, rgRefCounts, rgRowStatus),S_OK);
	
	//E_INVALIDARG
	TESTC_(m_pIRowset->ReleaseRows(2, NULL, NULL, NULL, NULL),E_INVALIDARG);
	TESTC_(m_pIRowset->ReleaseRows(2, NULL, rgRowOptions, rgRefCounts, rgRowStatus),E_INVALIDARG);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ReleaseRows--pcReleased=NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_10()
{
	HROW	hRow;
	ULONG	ulRefCount;

	//get one row handle
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 0));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ReleaseRows--rgRefCounts==NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_11()
{
	HROW	rghRows[1];
	DBROWSTATUS	rgRowStatus[1];

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get one row handle
	TESTC_(GetNextRows(0, 1, rghRows),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, rghRows, NULL, NULL, rgRowStatus),S_OK);
	
	//Verify Status
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);

CLEANUP:
	ReleaseRows(1, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ReleaseRows--pcRelease==NULL;rgRefCounts=NULL;S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_12()
{
	HROW	rghRows[1];

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get one row handle
	TESTC_(GetNextRows(2, 1, rghRows),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL),S_OK);
	
CLEANUP:
	ReleaseRows(1, rghRows);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ReleaseRows - before any rows are fetched
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Boundary::Variation_13()
{ 
	ULONG	cRowsObtained = 0;
	HROW	rghRows[3] = { LONG_MIN, NULL, LONG_MAX };
	ULONG	rgRefCounts[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
	ULONG	rgRowStatus[3] = { ULONG_MAX, ULONG_MAX, ULONG_MAX };
		
	//Create a "fresh" rowset...
	CRowset RowsetA;
	TESTC_(RowsetA.CreateRowset(SELECT_VALIDATIONORDER),S_OK);

	//Before fetching any rows, call AddRefRows...
	//This is a good senario since most providers allocate a row buffer at fetch time which
	//will be NULL for this senario.  Make sure they handle it...
	
	//No-op
	TESTC_(RowsetA()->ReleaseRows(0, NULL, NULL, NULL, NULL),S_OK);
	TESTC_(RowsetA()->ReleaseRows(0, rghRows, NULL, NULL, NULL),S_OK);
	TESTC_(RowsetA()->ReleaseRows(0, rghRows, NULL, rgRefCounts, NULL),S_OK);
	TESTC_(RowsetA()->ReleaseRows(0, rghRows, NULL, rgRefCounts, rgRowStatus),S_OK);
	
	//E_INVALIDARG
	TESTC_(RowsetA()->ReleaseRows(1, NULL, NULL, NULL, NULL),E_INVALIDARG);
	TESTC_(RowsetA()->ReleaseRows(1, NULL, NULL, rgRefCounts, rgRowStatus),E_INVALIDARG);
	
	//(3, invalid, valid, valid) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->ReleaseRows(THREE_ROWS, rghRows, NULL, rgRefCounts, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRowStatus, DBROWSTATUS_E_INVALID));
	TESTC(VerifyArray(THREE_ROWS, rgRefCounts, 0));
	
	//(3, invalid, valid, NULL) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->ReleaseRows(THREE_ROWS, rghRows, NULL, rgRefCounts, NULL), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRefCounts, 0));

	//(3, invalid, NULL, valid) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->ReleaseRows(THREE_ROWS, rghRows, NULL, NULL, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyArray(THREE_ROWS, rgRowStatus, DBROWSTATUS_E_INVALID));

	//(3, invalid, NULL, NULL) - DB_E_ERRORSOCCURRED
	TESTC_(RowsetA()->ReleaseRows(THREE_ROWS, rghRows, NULL, NULL, NULL), DB_E_ERRORSOCCURRED);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows--*pcRowsObtained==0, *prgRows!=NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Boundary::Variation_14()
{
	HROW	rghRows[3];

	//NextFetchPosition should not be changed
	TESTC_(GetNextRows(0, 0, rghRows),S_OK);

CLEANUP:
	ReleaseRows(3, rghRows);
	TRETURN
}	


// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition - Without calling GetData
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Boundary::Variation_15()
{ 
	HROW	hRow = NULL;

	//NextFetchPosition should not be changed
	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(ReleaseRows(hRow), S_OK);

	//RestartPosition (without calling GetData first)
	//Most of the variations in this test always verify the row handle returned
	//From GetNextRows which is done by calling GetData.  We need to make sure that
	//RestartPosition works without any state setup from GetData...
	TESTC_(RestartPosition(),S_OK);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc IUnknown::AddRef / Release
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Boundary::Variation_16()
{ 
	TBEGIN
	ULONG ulOrgRefCount = GetRefCount(pIRowset());

	//AddRef/Release Combinations
	SetRefCount(pIRowset(), 100); // AddRef 100 times
	SetRefCount(pIRowset(), -10); // Release 10 times
	SetRefCount(pIRowset(),   1); // AddRef   1 time
	SetRefCount(pIRowset(), -90); // Release 90 times
	SetRefCount(pIRowset(),  -1); // Release  1 time

	//Make sure the RefCount is back where we started...
	TESTC(ulOrgRefCount == GetRefCount(pIRowset()));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc IUnknown::QueryInterface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_Boundary::Variation_17()
{ 
	TBEGIN
	
	//QI [MANDATORY] interfaces
	TCOMPARE_(DefaultObjectTesting(pIRowset(), ROWSET_INTERFACE));

	//IRowsetIdentity - is required for level-0, 
	//but may require actually setting the property
	if(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET))
	{
		//Should be available if the property is TRUE
		TCHECK(QI(pIRowset(),	IID_IRowsetIdentity),	S_OK);
	}
	else
	{
		//Otherwise create a rowset setting the property first
		CRowset RowsetB;
		TCHECK(RowsetB.CreateRowset(DBPROP_IRowsetIdentity),S_OK);

		//Should be available if the property is TRUE
		TCHECK(QI(RowsetB.pIRowset(),	IID_IRowsetIdentity),	S_OK);
	}

	//QI [OPTIONAL] interfaces
	//Providers that support an interface must also
	//support the property assoicated with that interface with a value of VARIANT_TRUE
	TCHECK(QI(pIRowset(),		IID_IChapteredRowset),			GetProperty(DBPROP_IChapteredRowset)			? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IColumnsRowset),			GetProperty(DBPROP_IColumnsRowset)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IConnectionPointContainer),	GetProperty(DBPROP_IConnectionPointContainer)	? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IDBAsynchStatus),			GetProperty(DBPROP_IDBAsynchStatus)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetChange),				GetProperty(DBPROP_IRowsetChange)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetFind),				GetProperty(DBPROP_IRowsetFind)					? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetIdentity),			GetProperty(DBPROP_IRowsetIdentity)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetIndex),				GetProperty(DBPROP_IRowsetIndex)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetLocate),				GetProperty(DBPROP_IRowsetLocate)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetRefresh),			GetProperty(DBPROP_IRowsetRefresh)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetScroll),				GetProperty(DBPROP_IRowsetScroll)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetUpdate),				GetProperty(DBPROP_IRowsetUpdate)				? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_IRowsetView),				GetProperty(DBPROP_IRowsetView)					? S_OK : E_NOINTERFACE);
	TCHECK(QI(pIRowset(),		IID_ISupportErrorInfo),			GetProperty(DBPROP_ISupportErrorInfo)			? S_OK : E_NOINTERFACE);

//CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Boundary::Terminate()
{
	return TCIRowset::Terminate();
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Transactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Transactions - Use the interface within a transaction.  Zombie State.
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Transactions::Init()
{
	if(CTransaction::Init())
	{
	   	//register interface to be tested                                         
		if(RegisterInterface(ROWSET_INTERFACE, IID_IRowset, 0, NULL)) 
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would have been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Transactions::Variation_1()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	
	DBROWSTATUS	ulRowStatus;
	ULONG		ulRefCount;
	HACCESSOR	hAccessor = NULL;

	DBLENGTH		cbRowSize = 0;
	BYTE*		pData = NULL;

	IRowset*	pIRowset = NULL;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowset, 0, NULL));

	//Create an Accessor
	TESTC_(GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA, &hAccessor, NULL, NULL, &cbRowSize),S_OK);
	pData = (BYTE*)PROVIDER_ALLOC(cbRowSize);
	
	//get a row handle
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);

	//commit the transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	
	if(!m_fCommitPreserve)
	{
		//zombie
		TESTC_(pIRowset->RestartPosition(NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),E_UNEXPECTED);
		TESTC(cRowsObtained == 0);
		
		TESTC_(pIRowset->AddRefRows(1,rghRows, NULL, NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),E_UNEXPECTED);

		//release should be successful, can always release rows in Zombie state
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}
	else
	{
		//fully functional
		//try to get another row handle
		TESTC_(pIRowset->ReleaseRows(1,rghRows, NULL, NULL, NULL),S_OK);
		TEST2C_(pIRowset->RestartPosition(NULL),S_OK, DB_S_COMMANDREEXECUTED);

		//get a row
		TESTC_(pIRowset->GetNextRows(NULL, 4, 1, &cRowsObtained, &rghRows),S_OK);
		TESTC_(pIRowset->AddRefRows(1, rghRows, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 2));

		//GetData
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}

CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rghRows);

	//release the hAccessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL),S_OK);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit without retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Transactions::Variation_2()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	
	DBROWSTATUS	ulRowStatus;
	ULONG		ulRefCount;
	HACCESSOR	hAccessor = NULL;

	DBLENGTH		cbRowSize = 0;
	BYTE*		pData = NULL;

	IRowset*	pIRowset = NULL;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowset, 0, NULL));

	//Create an Accessor
	TESTC_(GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA, &hAccessor, NULL, NULL, &cbRowSize),S_OK);
	pData = (BYTE*)PROVIDER_ALLOC(cbRowSize);
	
	//get a row handle
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);

	//commit the transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	
	if(!m_fCommitPreserve)
	{
		//zombie
		TESTC_(pIRowset->RestartPosition(NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),E_UNEXPECTED);
		TESTC(cRowsObtained == 0);
		
		TESTC_(pIRowset->AddRefRows(1,rghRows, NULL, NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),E_UNEXPECTED);

		//release should be successful
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}
	else
	{
		//fully functional
		//try to get another row handle
		TESTC_(pIRowset->ReleaseRows(1,rghRows, NULL, NULL, NULL),S_OK);
		TEST2C_(pIRowset->RestartPosition(NULL),S_OK, DB_S_COMMANDREEXECUTED);

		//get a row
		TESTC_(pIRowset->GetNextRows(NULL, 4, 1, &cRowsObtained, &rghRows),S_OK);
		TESTC_(pIRowset->AddRefRows(1, rghRows, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 2));

		//GetData
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}

CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rghRows);

	//release the hAccessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL),S_OK);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(XACT_E_NOTRANSACTION);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Transactions::Variation_3()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	
	DBROWSTATUS	ulRowStatus;
	ULONG		ulRefCount;
	HACCESSOR	hAccessor = NULL;

	DBLENGTH		cbRowSize = 0;
	BYTE*		pData = NULL;

	IRowset*	pIRowset = NULL;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowset, 0, NULL));

	//Create an Accessor
	TESTC_(GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA, &hAccessor, NULL, NULL, &cbRowSize),S_OK);
	pData = (BYTE*)PROVIDER_ALLOC(cbRowSize);
	
	//get a row handle
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);

	//Abort the transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	
	if(!m_fAbortPreserve)
	{
		//zombie
		TESTC_(pIRowset->RestartPosition(NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),E_UNEXPECTED);
		TESTC(cRowsObtained == 0);
		
		TESTC_(pIRowset->AddRefRows(1,rghRows, NULL, NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),E_UNEXPECTED);

		//release should be successful
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}
	else
	{
		//fully functional
		//try to get another row handle
		TESTC_(pIRowset->ReleaseRows(1,rghRows, NULL, NULL, NULL),S_OK);
		TEST2C_(pIRowset->RestartPosition(NULL),S_OK, DB_S_COMMANDREEXECUTED);

		//get a row
		TESTC_(pIRowset->GetNextRows(NULL, 4, 1, &cRowsObtained, &rghRows),S_OK);
		TESTC_(pIRowset->AddRefRows(1, rghRows, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 2));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);

		//GetData
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}

CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rghRows);

	//release the hAccessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL),S_OK);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort without retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Transactions::Variation_4()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	
	DBROWSTATUS	ulRowStatus;
	ULONG		ulRefCount;
	HACCESSOR	hAccessor = NULL;

	DBLENGTH		cbRowSize = 0;
	BYTE*		pData = NULL;

	IRowset*	pIRowset = NULL;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowset, 0, NULL));

	//Create an Accessor
	TESTC_(GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA, &hAccessor, NULL, NULL, &cbRowSize),S_OK);
	pData = (BYTE*)PROVIDER_ALLOC(cbRowSize);
	
	//get a row handle
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);

	//Abort the transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	
	if(!m_fAbortPreserve)
	{
		//zombie
		TESTC_(pIRowset->RestartPosition(NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),E_UNEXPECTED);
		TESTC(cRowsObtained == 0);
		
		TESTC_(pIRowset->AddRefRows(1,rghRows, NULL, NULL),E_UNEXPECTED);
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),E_UNEXPECTED);

		//release should be successful
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}
	else
	{
		//fully functional
		//try to get another row handle
		TESTC_(pIRowset->ReleaseRows(1,rghRows, NULL, NULL, NULL),S_OK);
		TEST2C_(pIRowset->RestartPosition(NULL),S_OK, DB_S_COMMANDREEXECUTED);

		//get a row
		TESTC_(pIRowset->GetNextRows(NULL, 4, 1, &cRowsObtained, &rghRows),S_OK);
		TESTC_(pIRowset->AddRefRows(1, rghRows, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 2));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);

		//GetData
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, rghRows, NULL, &ulRefCount, &ulRowStatus),S_OK);
		TESTC(VerifyRefCounts(1, &ulRefCount, 0));
		TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	}

CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rghRows);

	//release the hAccessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL),S_OK);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(XACT_E_NOTRANSACTION);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Transactions::Terminate()
{
	return CTransaction::Terminate();
}
// }}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Chapters)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Chapters - Chapters specific test
//|	Created:			11/03/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Chapters::Init()
{
	if(TCIRowset::Init())
	{
		//open rowset with desired properties
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows: Invalid hChapter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_1()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	HROW	hRow = NULL;
	HRESULT hr = E_FAIL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Pass an invalid hChapter
	//Providers that don't support chapters should just ingore hchapter
	//Providers that do support support chapters should have DB_E_BADCHAPTER
	hr = m_pIRowset->GetNextRows(INVALID(HCHAPTER), 0, 1, &cRowsObtained, &rghRows);
	TEST2C_(hr, S_OK, DB_E_BADCHAPTER);

	if(hr == S_OK)
	{
		//Should have retreived the first row
		TESTC(cRowsObtained == 1 && rghRows != NULL);
		TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
		TESTC(VerifyReleaseRows(cRowsObtained, rghRows));
	}
	else
	{
		//Failed, so should not have moved the NextFetchPosition
		TESTC(cRowsObtained == 0 && rghRows == NULL);
		TESTC_(GetNextRows(0, 1, &hRow),S_OK);
		TESTC(VerifyRowHandles(hRow, FIRST_ROW));
	}

CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows:  DB_INVALID_CHAPTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_2()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Providers ingore hChapter
	TESTC_(m_pIRowset->GetNextRows(DB_INVALID_HCHAPTER, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC(cRowsObtained == 1 && rghRows != NULL);
	TESTC(VerifyReleaseRows(cRowsObtained, rghRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc ReleaseChapter: Active Row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_3()
{
	TBEGIN
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Third row of the rowset
	TESTC_(GetNextRows(2, 1, &hRow),S_OK);
	TESTC(VerifyRowHandles(hRow, THIRD_ROW));

	//TODO V2
	//Release the chapter
//	TESTC_(pIRowset()->ReleaseChapter(INVALID(HCHAPTER)),E_INVALIDARG);

	//addRef the row handle
	TESTC_(pIRowset()->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 2));
	TESTC_(ReleaseRows(hRow),S_OK);
	TESTC_(ReleaseRows(hRow),S_OK);

	//Verify FetchPosition: is still intact
	TESTC(VerifyGetNextRows(0, 1, FOURTH_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ReleaseChapter: NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_4()
{
	TBEGIN

	//TODO V2
	//ReleaseChapter - NULL
//	TESTC_(pIRowset()->ReleaseChapter(NULL),E_INVALIDARG);

//CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition: Invalid hChapter
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_5()
{
	TBEGIN
	HROW	rghRows[2];
	HRESULT hr = E_FAIL;

	//Restart The Position
	TESTC(VerifyRestartPosition(NULL));
	
	//retieve one row forward
	TESTC_(GetNextRows(0, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	TESTC(VerifyReleaseRows(1, &rghRows[0], 0));

	//Pass an invalid hChapter
	//Providers that don't support chapters should just ingore hchapter
	//Providers that do support support chapters should have DB_E_BADCHAPTER
 	hr = m_pIRowset->RestartPosition(INVALID(HCHAPTER));
	TEST3C_(hr, S_OK, DB_S_COMMANDREEXECUTED, DB_E_BADCHAPTER);

	if(SUCCEEDED(hr))
	{		
		//Should have restarted and the next row is the First Row
		TESTC_(GetNextRows(0, 1, &rghRows[1]),S_OK);
		TESTC(VerifyRowHandles(rghRows[1], FIRST_ROW));
	}
	else
	{
		//Should NOT have restarted and the next row is the SecondRow
		TESTC_(GetNextRows(0, 1, &rghRows[1]),S_OK);
		TESTC(VerifyRowHandles(rghRows[1], SECOND_ROW));
	}

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition: NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Chapters::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition(NULL));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Chapters::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_PendingChange)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_PendingChange - Row handles with pending changes
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_PendingChange::Init()
{
	return TCIRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc methods called with rows having pending changes will return DB_E_BADROWHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_PendingChange::Variation_1()
{
	TBEGIN
	HROW			rghRows[2] = { NULL, NULL};
	ULONG			rgRefCount[2];
	DBROWSTATUS		rgRowStatus[2];

	CRowsetUpdate RowsetA;

	//IID_IRowsetUpdate implies IRowsetChange
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER, IID_IRowsetUpdate, NULL, 
		DBACCESSOR_ROWDATA, DBPART_ALL, UPDATEABLE_COLS_BOUND)==S_OK);
	
	//get the second row
	TESTC_(RowsetA.GetNextRows(1, 1, &rghRows[0]),S_OK);

	//get the data
	TESTC(RowsetA.VerifyRowHandles(rghRows[0], SECOND_ROW));

	//change data
	TESTC_(RowsetA.GetData(rghRows[0], RowsetA.m_hAccessor, RowsetA.m_pData),S_OK);
	TESTC_(RowsetA.SetData(rghRows[0], RowsetA.m_hAccessor, RowsetA.m_pData),S_OK);
	TESTC(RowsetA.VerifyRowHandles(rghRows[0], SECOND_ROW));

	//addref, release, and GetData on the row hanle that has pending changes
	TESTC_(RowsetA.pIRowset()->AddRefRows(2, rghRows, rgRefCount, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 2));
	TESTC(VerifyRefCounts(1, &rgRefCount[1], 0));
 	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK && rgRowStatus[1] == DBROWSTATUS_E_INVALID);

	TESTC_(RowsetA.pIRowset()->ReleaseRows(2, rghRows, NULL, rgRefCount, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 1));
	TESTC(VerifyRefCounts(1, &rgRefCount[1], 0));
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_PENDINGCHANGES && rgRowStatus[1] == DBROWSTATUS_E_INVALID);

	//GetData
	TESTC_(RowsetA.GetData(rghRows[0], RowsetA.m_hAccessor, RowsetA.m_pData),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(rghRows[0]);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_PendingChange::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_CanHoldRows)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_CanHoldRows - Can Hold Rows in the rowset
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_CanHoldRows::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		
		//Create rowset
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
		{
			if(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET))
				return TRUE;
		}
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Reteive two discontiguous row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_CanHoldRows::Variation_1()
{
	DBCOUNTITEM   cRowsObtained = 0;
	HROW	rghRows[2];

	//retieve one row forward
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows[0]),S_OK);
	TESTC(cRowsObtained == 1);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));

	if(m_ulMaxOpenRows == 1)
	{
		TESTC_(GetNextRows(1, 1, &cRowsObtained, &rghRows[1]), DB_S_ROWLIMITEXCEEDED);
		TESTC(cRowsObtained == 0);	
	}
	else
	{
		//skip one row and retrieve one row forward
		TESTC_(GetNextRows(1, 1, &rghRows[1]),S_OK);
		TESTC(VerifyRowHandles(rghRows[1], THIRD_ROW));
	}

	//retrieve the data on the first row and to make sure we can get data on a 
	//previously retrieved row handle
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc release a middle row; release the first and the last row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_CanHoldRows::Variation_2()
{
	HROW	rghRows[3];

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retieve one row forward
	TESTC_(GetNextRows(0, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	if(m_ulMaxOpenRows != 0)
		TESTC_(ReleaseRows(rghRows[0]),S_OK);
	
	//retrieve the second row
	TESTC_(GetNextRows(0, 1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], SECOND_ROW));
	if(m_ulMaxOpenRows != 0)
		TESTC_(ReleaseRows(rghRows[1]),S_OK);

	//retrieve the third row
	TESTC_(GetNextRows(0, 1, &rghRows[2]),S_OK);
	TESTC(VerifyRowHandles(rghRows[2], THIRD_ROW));
	if(m_ulMaxOpenRows != 0)
		TESTC_(ReleaseRows(rghRows[2]),S_OK);
	
	//try to relese the second row
	if(m_ulMaxOpenRows == 0)
	{	
		TESTC_(ReleaseRows(rghRows[1]),S_OK);
		TESTC_(ReleaseRows(rghRows[0]),S_OK);
		TESTC_(ReleaseRows(rghRows[2]),S_OK);
	}

CLEANUP:
	ReleaseRows(3, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Release the last row, retrieve one more row, ERROR returneed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_CanHoldRows::Variation_3()
{
	HROW	rghRows[FOUR_ROWS];
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= FOUR_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve three rows
	TESTC_(GetNextRows(0, 3, rghRows),S_OK);
	TESTC(VerifyRowHandles(3, rghRows, FIRST_ROW));

	//release the last row handle
	TESTC_(ReleaseRows(rghRows[2]),S_OK);

	//retrieve one more row handle
	TESTC_(GetNextRows(1, 1, &rghRows[3]),S_OK);
	TESTC(VerifyRowHandles(rghRows[3], FIFTH_ROW));

CLEANUP:
	ReleaseRows(FOUR_ROWS, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Addref Rows and release the first and the last rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_CanHoldRows::Variation_4()
{
	HROW	rghRows[FOUR_ROWS];
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= FOUR_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve three rows
	TESTC_(GetNextRows(0, 3, rghRows),S_OK);
	TESTC(VerifyRowHandles(3, rghRows, FIRST_ROW));

	//addRef to all of the three row handles
	TESTC_(m_pIRowset->AddRefRows(3, rghRows, NULL, NULL),S_OK);

	//release the first and third row handles
	TESTC_(ReleaseRows(rghRows[0]),S_OK);
	TESTC_(ReleaseRows(rghRows[2]),S_OK);

	//release the second row for the first time
	TESTC(VerifyReleaseRows(1, &rghRows[1], 1));

	//the three row handles are release together
	TESTC(VerifyReleaseRows(3, rghRows, 0));

CLEANUP:
	ReleaseRows(FOUR_ROWS, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc Release Rowset with active Row handles - verify released
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_CanHoldRows::Variation_5()
{
	HROW	hRow = DB_NULL_HROW;
	ULONG   i=0;
	
	TCIRowset RowsetA;
	
	//open rowset with desired properties
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC_PROVIDER(RowsetA.m_ulMaxOpenRows == 0 || RowsetA.m_ulMaxOpenRows >= RowsetA.m_ulTableRows);

	//RestartPosition, verify
	TESTC(RowsetA.VerifyRestartPosition());

	//We want to verify that the row handles do not take any references on the Rowset
	//According to the spec, Releasing the Rowset even with unreleased rows should
	//Still release the rowset and all assoicated rows and accessors...
	for(i=0; i<RowsetA.m_ulTableRows; i++)
	{
		//Retrive this row
		TESTC_(RowsetA.GetNextRows(0, 1, &hRow),S_OK);
		//Verify this row
		TESTC(RowsetA.VerifyRowHandles(hRow, FIRST_ROW+i));
	}


CLEANUP:
	//Don't release row handles
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition - with outstanding row handles
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_CanHoldRows::Variation_6()
{ 
	DBCOUNTITEM   cRowsObtained = 0;
	HROW*	rghRows = NULL;
	HROW	rghRows2[FIVE_ROWS];
	HRESULT hr = S_OK;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Retrieve rows
	TEST2C_(hr = GetNextRows(0, TWO_ROWS, &cRowsObtained, &rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);
	TESTC(VerifyRowHandles(cRowsObtained, rghRows, FIRST_ROW));

	//RestartPosition, (with outstanding row handles)
	if(!VerifyRestartPosition(NULL, FALSE/*fRowsReleased*/))
	{
		//Some providers require releasing all rows, before restarting...
		//We can still continue testing, but not the outstanding row handle part...
		ReleaseRows(cRowsObtained, rghRows);
		cRowsObtained = 0;
	}

	//The rest of the verification requires verification of outstanding row handles...
	if(hr == DB_S_ROWLIMITEXCEEDED)
		TESTC(m_ulMaxOpenRows == ONE_ROW);
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=FOUR_ROWS);
	
	//Obtain the second and third rows...
	//NOTE: If the above RestartPosition failed (ie: cRowsObtained==0) this call
	//will returned the fourth and fifth rows instead, since the cursor is not at the head...
	TESTC_(GetNextRows(1, TWO_ROWS, &rghRows2[SECOND_ROW]),S_OK);
	TESTC(VerifyRowHandles(TWO_ROWS, &rghRows2[SECOND_ROW], cRowsObtained ? SECOND_ROW : FOURTH_ROW));

	//Make sure the previously (outstanding) returned row handles are still valid...
	TESTC(VerifyRowHandles(cRowsObtained, rghRows, FIRST_ROW));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	ReleaseRows(TWO_ROWS, rghRows2);
	SAFE_FREE(rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_CanHoldRows::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_HoldRows_Discon)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_HoldRows_Discon - Hold rows and discontiguous
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_HoldRows_Discon::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		
		//Create rowset
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			if(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET))
				return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Skip one row forward and retrieve one row forward.  Repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_HoldRows_Discon::Variation_1()
{
	HROW	rghRows[2];
	ULONG   rgRefCounts[2];
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= TWO_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get one row handle
	TESTC_(GetNextRows(1, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], SECOND_ROW));

	//get another one
	TESTC_(GetNextRows(1, 1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], FOURTH_ROW));

	//AddRef the first row handle
	TESTC_(m_pIRowset->AddRefRows(1, &rghRows[0], &rgRefCounts[0], NULL), S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 2));

	//AddRef the second row handle
	TESTC_(m_pIRowset->AddRefRows(1, &rghRows[1], &rgRefCounts[1], NULL), S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCounts[1], 2));

	//RestartPosition, (FALSE - not all rows are released)
	VerifyRestartPosition(NULL, FALSE);

CLEANUP:
	ReleaseRows(2, rghRows);
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrieve the two rows, call RestartPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_HoldRows_Discon::Variation_2()
{
	HROW	rghRows[2];
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= TWO_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//get one row handle
	TESTC_(GetNextRows(2, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], THIRD_ROW));

	//get the last one
	TESTC_(GetNextRows(m_ulTableRows-(2+1+1), 1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], m_ulTableRows));

	//RestartPosition, (FALSE - not all rows are released)
	VerifyRestartPosition(NULL, FALSE);

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_HoldRows_Discon::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_Discontiguous)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_Discontiguous - The rowset can has discontiguous rows
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Discontiguous::Init()
{
	if(TCIRowset::Init())
	{
		//Create rowset
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve one row, release it; retrieve another row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Discontiguous::Variation_1()
{
	HRESULT hr = S_OK;
	HROW	rghRows[2] = {NULL, NULL};
	ULONG   rgRefCounts[2] = {0,0};

	//retrieve one row
	TESTC_(GetNextRows(0, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));

	//NextFetchPosition should not be changed
	TESTC_(GetNextRows(0, 0, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));

	//addRef the row
	TESTC_(m_pIRowset->AddRefRows(1, &rghRows[0], &rgRefCounts[0], NULL),S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 2));

	//release the row
	TESTC_(ReleaseRows(rghRows[0]),S_OK);

	//RestartPosition.  The Provider May or may not have the ability to HoldRows, 
	//even though we didn't specfically ask for CANHOLDROWS.
	hr = RestartPosition();
	TEST3C_(hr, S_OK, DB_S_COMMANDREEXECUTED, DB_E_ROWSNOTRELEASED);

	//If they have the ability to hold rows
	if(hr == S_OK || hr==DB_S_COMMANDREEXECUTED)
	{
		//retrieve another row
		TESTC_(GetNextRows(1, 1, &rghRows[1]),S_OK);
		//Since RestartPosition succeeded, we should be at the 2nd row
		TESTC(VerifyRowHandles(rghRows[1], 2));
	}
	//Otherwise
	else
	{
		//release the first row
		TESTC_(ReleaseRows(rghRows[0]),S_OK);

		//retrieve another row
		TESTC_(GetNextRows(1, 1, &rghRows[1]),S_OK);
		//Since RestartPosition didn't succeed, we should be at the 3rd row
		TESTC(VerifyRowHandles(rghRows[1], 3));
	}

	//release the second row
	TESTC_(ReleaseRows(rghRows[1]),S_OK);

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrieve one row, addRef it and release it;  retrieve another row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Discontiguous::Variation_2()
{
	HRESULT hr = S_OK;
	HROW		rghRows[2];
	ULONG		rgRefCounts[2];
	DBROWSTATUS	rgRowStatus[2];

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve one row
	TESTC_(GetNextRows(0, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));

	//GetNextRows.  The Provider May or may not have the ability to HoldRows, 
	//even though we didn't specfically ask for CANHOLDROWS.
	hr = GetNextRows(1, 1, &rghRows[1]);
	TEST2C_(hr, S_OK, DB_E_ROWSNOTRELEASED);

	//If they have the ability to hold rows
	if(hr == S_OK)
	{
		//release the row and set to invalid
		TESTC(VerifyRowHandles(rghRows[1], THIRD_ROW));
		TESTC_(ReleaseRows(rghRows[1]),S_OK);
		rghRows[1] = INVALID(HROW);
	}

	//release the rows
	TESTC_(pIRowset()->ReleaseRows(2, rghRows, NULL, rgRefCounts, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCounts[0], 0));
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);
	TESTC(rgRefCounts[1] == 0);
	TESTC(rgRowStatus[1] == DBROWSTATUS_E_INVALID);

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrieve one row handle, addRef it and release it; Call RestartPositon
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Discontiguous::Variation_3()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve one row
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW, FORWARD));

	//retrieve another row
	TESTC(VerifyGetNextRows(1, 1, THIRD_ROW, FORWARD));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Retrieve three rwo handles together.  Release the middle row,  Succeed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_Discontiguous::Variation_4()
{
	HRESULT hr = S_OK;
	HROW	rghRows[3];
	ULONG   rgRefCounts[3];
	ULONG   rgRowStatus[3];
	TESTC_PROVIDER(m_ulMaxOpenRows == 0 || m_ulMaxOpenRows >= THREE_ROWS);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//retrieve three row handles
	TESTC_(GetNextRows(0, 3, rghRows),S_OK);
	TESTC(VerifyRowHandles(3, rghRows, FIRST_ROW));

	//release the middle row handle
	TESTC_(pIRowset()->ReleaseRows(1, &rghRows[1], NULL, rgRefCounts, NULL),S_OK);

	//release the three row handles
	hr = pIRowset()->ReleaseRows(3, rghRows, NULL, NULL, rgRowStatus);

	//Some providers might be able to hold onto rows...
	if(rgRefCounts[0] != 0)
	{
		TESTC(hr == S_OK);
		TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);
		TESTC(rgRowStatus[1] == DBROWSTATUS_S_OK);		
		TESTC(rgRowStatus[2] == DBROWSTATUS_S_OK);
	}
		
	if(rgRefCounts[0] == 0)
	{
		TESTC(hr == DB_S_ERRORSOCCURRED);
		TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK);
		TESTC(rgRowStatus[1] == DBROWSTATUS_E_INVALID);
		TESTC(rgRowStatus[2] == DBROWSTATUS_S_OK);
	}
	
CLEANUP:
	ReleaseRows(3, rghRows);
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_Discontiguous::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_MaxOpenRows)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_MaxOpenRows - test max open rows
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_MaxOpenRows::Init()
{
	return TCIRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc MaxOpenRows is unlimted.  Retrieve rows in a sequential manner.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_MaxOpenRows::Variation_1()
{
	TBEGIN
	HROW*	rghRows = NULL;
	DBCOUNTITEM	i,cRows = 0;
	ULONG_PTR	ulMaxOpenRows = 0;

	HROW	hRow = NULL;
	DBCOUNTITEM   cRowsObtained = 0;

	//Set requireed Properties
	SetProperty(DBPROP_CANHOLDROWS);
	
	//Create rowset
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET));

	//GetProperty
	TESTC_PROVIDER(GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &ulMaxOpenRows));
	rghRows = PROVIDER_ALLOC_(m_ulTableRows, HROW);

	//Setup total expected open rows
	if(ulMaxOpenRows == 0 || ulMaxOpenRows > m_ulTableRows)
		ulMaxOpenRows = m_ulTableRows;

	//get all the rows in the rowset, one at a time
	for(i=0; i<ulMaxOpenRows; i++)
	{	
		TESTC_(GetNextRows(&rghRows[i]),S_OK);
		TESTC(VerifyRowHandles(rghRows[i], FIRST_ROW+i));
		cRows++;
	}

	//test pass is all the row handles are retrieved
	TESTC(cRows == ulMaxOpenRows);

	//Try to obtain one more
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &hRow), cRows < m_ulTableRows ? DB_S_ROWLIMITEXCEEDED : DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0);

	//release all the row handles
	TESTC_(ReleaseRows(cRows, rghRows),S_OK);

CLEANUP:
	ReleaseRows(m_ulTableRows, rghRows);
	DropRowset();
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}



// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc MaxOpenRows is unlimted.  Retrieve rows in a group.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_MaxOpenRows::Variation_2()
{ 
	TBEGIN
	DBCOUNTITEM	i,cRows = 0;
	ULONG_PTR	ulMaxOpenRows = 0;

	//Set requireed Properties
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//GetProperty
	//NOTE: If the property is not supported, then there is no limit (within reason)
	GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &ulMaxOpenRows);
	cRows = ulMaxOpenRows ? min(ulMaxOpenRows, m_ulTableRows) : m_ulTableRows;

	//NOTE: We want these calls on "freshly" created rowsets, so we create the 
	//rowset within the loop...
	DropRowset();

	//Spin through all types of request for row handles (in groups)
	for(i=0; i<cRows+2; i++)
	{
		//Create rowset
		TESTC_(CreateRowset(SELECT_VALIDATIONORDER),S_OK);

		//Obtain rows.
		//ie: {[0, 0]...[0, cTableRows-2]...[0, cTableRows]...[0, cTableRows+2]}
		TESTC(VerifyGetNextRows(0, i, FIRST_ROW, FORWARD, min(i,cRows), DBMEMOWNER_PROVIDEROWNED));
		DropRowset();
	}

CLEANUP:
	DropRowset();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set the maxopen rows to be 5.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_MaxOpenRows::Variation_3()
{
	TBEGIN
	DBCOUNTITEM		cRowsObtained = 0;
	HROW		rghRows[SIX_ROWS];
	DBCOUNTITEM		i,cRows = 0;
	HRESULT		hr = S_OK;

	//open a rowset with DBPROP_CANHOLDROWS and set DBPROP_MAXOPENROWS=5
	SetProperty(DBPROP_CANHOLDROWS);
	SetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, (void*)5, DBTYPE_I4);

	//Create rowset
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET));

	//retrieve max # of row, one at a time
	for(i=0; i<FIVE_ROWS; i++)
	{
		TESTC_(GetNextRows(&rghRows[i]),S_OK);
		TESTC(VerifyRowHandles(rghRows[i], FIRST_ROW+i));
		cRows++;
	}			 

	//AddRef the third row handle
	TESTC_(m_pIRowset->AddRefRows(1, &rghRows[2], NULL, NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, &rghRows[2], NULL, NULL, NULL),S_OK);

	//retrieve another row handle, 
	//2.0 Spec now indicates that "setting" MAXOPENROWS is just a "suggestion"
	//A provider can return more rows than specified by the consumer
	rghRows[5] = INVALID(HROW);
	hr = GetNextRows(0, 1, &cRowsObtained, &rghRows[5]);
	TEST2C_(hr, S_OK, DB_S_ROWLIMITEXCEEDED);
	if(hr==S_OK)
	{
		//S_OK
		TESTC(cRowsObtained != 0 && rghRows[5] != DB_NULL_HROW);
		TESTC(VerifyRowHandles(rghRows[5], FIRST_ROW+5));
	}
	else
	{
		//DB_S_ROWLIMITEXCEEDED
		TESTC(cRowsObtained == 0 /*&& rghRows[5] == DB_NULL_HROW*/); //TODO Spec issue: 
	}

CLEANUP:
	ReleaseRows(6, rghRows);
	DropRowset();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc MaxOpenRows is not unlimited.  Test the limit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_MaxOpenRows::Variation_4()
{
	TBEGIN
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*		rghRows = NULL;
	ULONG_PTR		ulMaxOpenRows = 0;

	//open a rowset with DBPROP_CANHOLDROWS
	SetProperty(DBPROP_CANHOLDROWS);

	//Create rowset
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &ulMaxOpenRows));
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET));

	//if MaxOpenRows is unlimited, pass the test variation
	TESTC_PROVIDER(ulMaxOpenRows != 0);

	//If there are less rows in the rowset than MaxOpenRows, we have no choice but
	//to skip this variation, since we cannot hit the maximum!
	TESTC_PROVIDER(m_ulTableRows > ulMaxOpenRows);

	//retrieve 1 less than the max number of rows
	TESTC_(GetNextRows(0, ulMaxOpenRows-1, &cRowsObtained, &rghRows),S_OK);
	PROVIDER_FREE(rghRows);

	//Try to retrieve three more row handles
	//MAXOPENROWS returned by the provider should be obeyed.
	//Since the consumer didn't set it, its exactly what the provider indicates...
	TESTC_(GetNextRows(0, 3, &cRowsObtained, &rghRows), DB_S_ROWLIMITEXCEEDED);
	TESTC(cRowsObtained == 1 && rghRows != NULL);
	TESTC(VerifyRowHandles(rghRows[0], ulMaxOpenRows));
	TESTC_(ReleaseRows(cRowsObtained, rghRows), S_OK);
	PROVIDER_FREE(rghRows);

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	DropRowset();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_MAXROWS - Get Value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_MaxOpenRows::Variation_5()
{
	TBEGIN
	ULONG_PTR		ulMaxRows = 0;
	DBCOUNTITEM		cExpectedRows = 0;

	//open a rowset with DBPROP_CANHOLDROWS
	SetProperty(DBPROP_CANHOLDROWS);

	//Create rowset
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET, &ulMaxRows));
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET));

	//Figure out how many rows were are expecting in the rowset
	cExpectedRows  = ulMaxRows > 0 ? ulMaxRows : m_ulTableRows;
	
	//Verify we have the correct number of rows in the rowset
	TESTC(cExpectedRows == GetTotalRows());

CLEANUP:
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_MAXROWS - Set Value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_MaxOpenRows::Variation_6()
{
	TBEGIN
	ULONG		ulMaxRows = 5;
	ULONG		cExpectedRows = 0;

	//open a rowset with DBPROP_CANHOLDROWS
	SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(SetSettableProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET, (void*)(LONG_PTR)ulMaxRows, DBTYPE_I4));

	//Create rowset
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Verify we have the correct number of rows in the rowset
	TESTC(ulMaxRows == GetTotalRows());

CLEANUP:
	DropRowset();
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_MaxOpenRows::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_ForwardOnly)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_ForwardOnly - ScrollBackwards_ForwardOnly
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_ForwardOnly::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANSCROLLBACKWARDS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;
	HRESULT hr = S_OK;
	DBROWCOUNT i = 0;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Loop through all large (negative) offset values...
	//Some providers like to hard code to work arround LONG_MAX, LONG_MIN
	//Work arround this!
	for(i=NEGATIVE(m_ulTableRows+10); i>LONG_MIN && (i+i<i); i+=i)
	{
		hr = GetNextRows(i, 1, &hRow);
		//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
		//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
		TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	}

	//Loop through all large offset values...
	//Some providers like to hard code to work arround LONG_MAX, LONG_MIN
	//Work arround this!
	for(i=m_ulTableRows + 10; i<MAXDBROWCOUNT && (i+i>i); i+=i)
	{
		hr = GetNextRows(i, 1, &hRow);
		//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
		//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
		TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	}

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	//Also the 2.0 spec indicates lRowOffset is ignored
	TESTC_(GetNextRows(-1000, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_5()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, &cRowsObtained, &rghRows), S_OK);
	TESTC(cRowsObtained == (m_ulTableRows-2) && rghRows != NULL);
	TESTC(VerifyRowHandles(cRowsObtained, rghRows, THIRD_ROW, FORWARD));

CLEANUP:
	CHECK(ReleaseRows(cRowsObtained, rghRows),S_OK);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  
	//Only one row (last row) should be retrieved, no reason for reaching any limit...
	TESTC(VerifyGetNextRows(-1, LONG_MAX, m_ulTableRows, FORWARD, 1, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_8()
{
	HROW hRow = NULL;
	HRESULT hr = S_OK;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip forward more than the number of rows in the rowset
	hr = GetNextRows(LONG_MAX, 1, &hRow);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	hr = GetNextRows(LONG_MAX-1, 1, &hRow);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);

	//skip backward more than the number of rows in the rowset
	hr = GetNextRows(LONG_MIN, 1, &hRow);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	hr = GetNextRows(LONG_MIN+1, 1, &hRow);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);

	//skip backward 5 more row
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
	//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_ForwardOnly::Variation_10()
{
	HRESULT hr = S_OK;
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch more rows than in the rowset
	TESTC(VerifyGetNextRows(1, LONG_MAX, m_ulTableRows, FORWARD, 1, DBMEMOWNER_PROVIDEROWNED));
		
	//Since the RowPosition is now unknown, I really can't verify it is correct.
	//But I can restart the position and skip to the last...
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_ForwardOnly::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_Static)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_Static - ScrollBackwards_Static
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Static::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANSCROLLBACKWARDS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==-(m_ulTableRows+2)
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+2), 1, &hRow), DB_S_ENDOFROWSET);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	TESTC_(GetNextRows(1, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  One row should be retrieved
	TESTC(VerifyGetNextRows(-1, m_ulTableRows, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_8()
{
	HROW hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip backward more than the number of rows in the rowset
	//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
	//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
	TESTC_(GetNextRows(m_ulTableRows+1, 1, &hRow), DB_S_ENDOFROWSET);

	//skip backward 5 more row
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//2.0 spec indicates DB_S_ENDOFROWSET for all outofbounds cases
	//We no longer have DB_E_BADSTARTPOSITION for 2.x providers
	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Static::Variation_10()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch two rows forward
	TESTC(VerifyGetNextRows(1, 2, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Static::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_Keyset)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_Keyset - ScrollBackwards_Keyset
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Keyset::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetProperty(DBPROP_OTHERUPDATEDELETE);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==-(m_ulTableRows+2)
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+2), 1, &hRow), DB_S_ENDOFROWSET);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	TESTC_(GetNextRows(1, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//GetNextRows
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  One row should be retrieved
	TESTC(VerifyGetNextRows(-1, m_ulTableRows, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_8()
{
	HROW hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip backward more than the number of rows in the rowset
	TESTC_(GetNextRows(m_ulTableRows+1, 1, &hRow), DB_S_ENDOFROWSET);

	//skip backward 5 more row
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Keyset::Variation_10()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch two rows forward
	TESTC(VerifyGetNextRows(1, 2, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Keyset::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_Dynamic)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_Dynamic - ScrollBackwards_Dynamic
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Dynamic::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetProperty(DBPROP_OTHERINSERT);

		//Create rowset
		//Note: We don't set CANHOLDROWS, since this maybe conflicting with
		//OtherInsert on some providers.  We are mainly interested with Scrolling
		//backwards on Dynamic cursor and less requirement on holding rows...
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//lOffset is off the rowset, 0 rows should be returned...
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+2), 1, &hRow), DB_S_ENDOFROWSET);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRows == 0, NextFetchPosition should be unchanged
	TESTC_(GetNextRows(1, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  One row should be retrieved
	TESTC(VerifyGetNextRows(-1, m_ulTableRows, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_8()
{
	HROW hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip backward more than the number of rows in the rowset
	TESTC_(GetNextRows(m_ulTableRows+1, 1, &hRow), DB_S_ENDOFROWSET);

	//skip backward 5 more rows
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Dynamic::Variation_10()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch two rows forward
	TESTC(VerifyGetNextRows(1, 2, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Dynamic::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_Locate)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_Locate - ScrollBackwards_Locate
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Locate::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
		SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==-(m_ulTableRows+2)
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+2), 1, &hRow), DB_S_ENDOFROWSET);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	TESTC_(GetNextRows(1, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  One row should be retrieved
	TESTC(VerifyGetNextRows(-1, m_ulTableRows, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_8()
{
	HROW hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip backward more than the number of rows in the rowset
	TESTC_(GetNextRows(m_ulTableRows+1, 1, &hRow), DB_S_ENDOFROWSET);

	//skip backward 5 more row
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_Locate::Variation_10()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch two rows forward
	TESTC(VerifyGetNextRows(1, 2, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_Locate::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_ScrollBackwards_QueryBased)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_ScrollBackwards_QueryBased - ScrollBackwards_QueryBased
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_QueryBased::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip forward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_1()
{
	HROW	hRow = DB_NULL_HROW;

	//cRow==0, cRowToSkip ==m_ulTableRows+2.  No-Op
	TESTC_(GetNextRows(m_ulTableRows, 0, &hRow),S_OK);

	//make sure the cursor is at the begining of the rowset
	//skip backward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(-1, 1, m_ulTableRows));

	//make sure the second to last row is fetched
	TESTC(VerifyGetNextRows(-2, 1, m_ulTableRows-1));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip backward > # of rows in the table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_2()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==-(m_ulTableRows+2)
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+2), 1, &hRow), DB_S_ENDOFROWSET);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip =1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_3()
{
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	TESTC_(GetNextRows(1, 0, &hRow),S_OK);

	//make sure the cursor is at the 1st row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRow==0, cRowToSkip = -1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_4()
{	
	HROW	hRow = DB_NULL_HROW;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//cRow==0, cRowToSkip ==1	No-Op
	TESTC_(GetNextRows(-1, 0, &hRow),S_OK);

	//make sure the cursor is at the first row of the rowset
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc cRows is negative.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-2), m_ulTableRows-2, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row backward and fetch 5 row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row backward and fetch m_ulTableRows row forward.  One row should be retrieved
	TESTC(VerifyGetNextRows(-1, m_ulTableRows, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc retrieve 2 rows.  Skip 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_7()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 3 row backwards
	TESTC(VerifyGetNextRows(-3, 1, m_ulTableRows-2, FORWARD));

	//skip one row and fetch one row forward
	TESTC(VerifyGetNextRows(1, 1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc skip  backward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_8()
{
	HROW hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip backward more than the number of rows in the rowset
	TESTC_(GetNextRows(m_ulTableRows+1, 1, &hRow), DB_S_ENDOFROWSET);

	//skip backward 5 more row
	TESTC_(GetNextRows(NEGATIVE(m_ulTableRows+1), 1, &hRow), DB_S_ENDOFROWSET);

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward more than the number of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_9()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW*	rghRows = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());
	TESTC_(GetNextRows(m_ulTableRows, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip to last row of the rowset
	TESTC(VerifyGetNextRows(m_ulTableRows-1, 1, m_ulTableRows));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc skip forward and to the last row. skip backward to the first row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_ScrollBackwards_QueryBased::Variation_10()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows-3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch (m_ulTableRows-3) rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows-3, SECOND_ROW, FORWARD));

	//skip one row forward and fetch two rows forward
	TESTC(VerifyGetNextRows(1, 2, m_ulTableRows, FORWARD, ONE_ROW, DBMEMOWNER_PROVIDEROWNED));

	//skip totalRowNum row backward and fetch one row forward
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_ScrollBackwards_QueryBased::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_ForwardOnly)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_ForwardOnly - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_ForwardOnly::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANFETCHBACKWARDS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_3()
{
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	HRESULT hr = S_OK;
	DBROWCOUNT i=0;

	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Do the LONG_MIN case
	//should return DB_S_ENDOFROWSET with m_ulTableRows returned in reserve order
	TESTC(VerifyGetNextRows(0, LONG_MIN, m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows backwards
	//Some providers like to hard code to work arround LONG_MAX, LONG_MIN
	//Work arround this!
	for(i= NEGATIVE(m_ulTableRows+10); i>LONG_MIN && (i+i<i); i+=i)
	{
		//RestartPosition, verify
		TESTC(VerifyRestartPosition());

		//Try to obtain all row fetched backwards...
		TESTC(VerifyGetNextRows(0, i, m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));
	}

	//Spec allows the cursor position to be unknown for this error.
	//So just restart it to be sure for the next call...
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//Try extreme fetch backwards
	hr = GetNextRows(LONG_MAX, LONG_MIN, &cRowsObtained, &rghRows);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_6()
{
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch 2 rows backwards
	TESTC(VerifyGetNextRows(1, -2, FIRST_ROW, REVERSE, ONE_ROW));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, 5, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_ForwardOnly::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_ForwardOnly::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_Static)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_Static - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Static::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_3()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows backwards
	TESTC(VerifyGetNextRows(0, NEGATIVE(m_ulTableRows+1), m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch backward again
	TESTC(VerifyGetNextRows(0, -1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_6()
{
	//Create a new rowset...
	//This is a good senario where RestartPosition has never been called and 
	//no previous rows have been fetched...
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Skip three row forward and fetch 4 rows backwards
	//Should be only able to obtain 3 rows, and then end of rowset
	TESTC(RowsetA.VerifyGetNextRows(3, -4, THIRD_ROW, REVERSE, THREE_ROWS));

	//fetch one row forward
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);
	
	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, 5, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Static::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Static::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_Keyset)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_Keyset - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Keyset::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetProperty(DBPROP_OTHERUPDATEDELETE);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_3()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows backwards
	TESTC(VerifyGetNextRows(0, NEGATIVE(m_ulTableRows+1), m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch backward again
	TESTC(VerifyGetNextRows(0, -1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_6()
{
	//Create a new rowset...
	//This is a good senario where RestartPosition has never been called and 
	//no previous rows have been fetched...
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	RowsetA.SetProperty(DBPROP_OTHERUPDATEDELETE);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Skip three row forward and fetch 4 rows backwards
	//Should be only able to obtain 3 rows, and then end of rowset
	TESTC(RowsetA.VerifyGetNextRows(3, -4, THIRD_ROW, REVERSE, THREE_ROWS));

	//fetch one row forward
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, 5, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Keyset::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Keyset::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_Dynamic)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_Dynamic - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Dynamic::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetProperty(DBPROP_OTHERINSERT);

		//Create rowset
		//Note: We don't set CANHOLDROWS, since this maybe conflicting with
		//OtherInsert on some providers.  We are mainly interested with Scrolling
		//backwards on Dynamic cursor and less requirement on holding rows...
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_3()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows backwards
	TESTC(VerifyGetNextRows(0, NEGATIVE(m_ulTableRows+1), m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch backward again
	TESTC(VerifyGetNextRows(0, -1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_6()
{
	//Create a new rowset...
	//This is a good senario where RestartPosition has never been called and 
	//no previous rows have been fetched...
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	RowsetA.SetProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Skip three row forward and fetch 4 rows backwards
	//Should be only able to obtain 3 rows, and then end of rowset
	TESTC(RowsetA.VerifyGetNextRows(3, -4, THIRD_ROW, REVERSE, THREE_ROWS));

	//fetch one row forward
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, 5, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Dynamic::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Dynamic::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_Locate)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_Locate - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Locate::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_IRowsetLocate);
		SetProperty(DBPROP_CANFETCHBACKWARDS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_3()
{
	ULONG	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows backwards
	TESTC(VerifyGetNextRows(0, NEGATIVE(m_ulTableRows+1), m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch backward again
	TESTC(VerifyGetNextRows(0, -1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_6()
{
	//Create a new rowset...
	//This is a good senario where RestartPosition has never been called and 
	//no previous rows have been fetched...
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Skip three row forward and fetch 4 rows backwards
	//Should be only able to obtain 3 rows, and then end of rowset
	TESTC(RowsetA.VerifyGetNextRows(3, -4, THIRD_ROW, REVERSE, THREE_ROWS));

	//fetch one row forward
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, FIFTH_ROW, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1, DBMEMOWNER_PROVIDEROWNED));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_Locate::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2, DBMEMOWNER_PROVIDEROWNED));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, THIRD_ROW, FORWARD, 0, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_Locate::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_FetchBackwards_QueryBased)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_FetchBackwards_QueryBased - fetch backwards
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_QueryBased::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANHOLDROWS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRowsToSkip is negative, return DB_E_CANSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_1()
{
	//Get Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

	//Get Second to the Last row of the Rowset
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-1));

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Get the Last row
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles. one at a time
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_2()
{
	ULONG	i = 0;
	HROW	hRow = NULL;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch all rows
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, 1, FIRST_ROW+i));

	//try to fetch forward two more row
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, 1, &hRow), DB_S_ENDOFROWSET);

	//fetch rows backwards
	for(i=0; i<m_ulTableRows; i++)
		TESTC(VerifyGetNextRows(0, -1, m_ulTableRows-i));

	//try to fetch backward two more row
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);
	TESTC_(GetNextRows(0, -1, &hRow), DB_S_ENDOFROWSET);

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc fetch forward and backward of all row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_3()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch 1 more than available rows backwards
	TESTC(VerifyGetNextRows(0, NEGATIVE(m_ulTableRows+1), m_ulTableRows, REVERSE, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch backward again
	TESTC(VerifyGetNextRows(0, -1, FIRST_ROW, REVERSE, 0, DBMEMOWNER_PROVIDEROWNED));

	//fetch 1 more than available rows forwards
	TESTC(VerifyGetNextRows(0, m_ulTableRows+1, FIRST_ROW, FORWARD, m_ulTableRows, DBMEMOWNER_PROVIDEROWNED));

	//try to fetch forward again
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW, FORWARD, 0, DBMEMOWNER_PROVIDEROWNED));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_4()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, m_ulTableRows, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch two rows then 3 row backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_5()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch the first two rows
	TESTC(VerifyGetNextRows(0, 2, FIRST_ROW, FORWARD));

	//fetch three rows backwards
	TESTC(VerifyGetNextRows(0, -3, SECOND_ROW, REVERSE, TWO_ROWS));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch 2 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_6()
{
	//Create a new rowset...
	//This is a good senario where RestartPosition has never been called and 
	//no previous rows have been fetched...
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	RowsetA.SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Skip three row forward and fetch 4 rows backwards
	//Should be only able to obtain 3 rows, and then end of rowset
	TESTC(RowsetA.VerifyGetNextRows(3, -4, THIRD_ROW, REVERSE, THREE_ROWS));

	//fetch one row forward
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows forward and fetch 3 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_7()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=5);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 3 rows forward
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//fetch 5 rows backwards
	TESTC(VerifyGetNextRows(0, -5, 5, REVERSE));

	//fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fetch three rows forward and repeat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_8()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//fetch three rows forward
	TESTC(VerifyGetNextRows(0, 3, FIRST_ROW));

	//skip one row forward, and fetch two rows forward
	TESTC(VerifyGetNextRows(1, m_ulTableRows, FIFTH_ROW, FORWARD, m_ulTableRows-3-1));

	//fetch one row backward
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc skip forward 2 rows and fetch 4 rows forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_FetchBackwards_QueryBased::Variation_9()
{
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=m_ulTableRows);

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//skip 2 rows forward and fetch 4 rows forward
	TESTC(VerifyGetNextRows(2, m_ulTableRows, THIRD_ROW, FORWARD, m_ulTableRows-2));

	//skip one row forward, and fetch one row forward
	TESTC(VerifyGetNextRows(0, 1, NO_ROWS, FORWARD, NO_ROWS));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_FetchBackwards_QueryBased::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_AllProperties)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_AllProperties - Can Hold rows + fetch backwards + scroll backwards + Discontiguous
//|	Created:			11/29/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_AllProperties::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetProperty(DBPROP_CANHOLDROWS);

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRow<0, skip backward 4 rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_AllProperties::Variation_1()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());

	//skip totalrows-1 backwards and fetch 2 rows backwards
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-1), -2, FIRST_ROW, REVERSE, ONE_ROW));

	//fetch one more row forward
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRow<0, skip bakcward # of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_AllProperties::Variation_2()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());

	//skip all rows backwards and fetch 1 rows backwards, no row should be returned
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows), -1, NO_ROWS, FORWARD, NO_ROWS));

	//Try all combinations of large values
	TESTC(VerifyGetNextRows(LONG_MIN, LONG_MAX, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MIN, LONG_MIN, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MAX, LONG_MAX, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MAX, LONG_MIN, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//They maybe adding these values together in some combination...
	TESTC(VerifyGetNextRows(LONG_MIN+1, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MIN,	1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MAX,	1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));
	TESTC(VerifyGetNextRows(LONG_MAX-1, 1, NO_ROWS, FORWARD, NO_ROWS, DBMEMOWNER_PROVIDEROWNED));

	//restart cursor position
	TESTC(VerifyRestartPosition());

	//NextFetchPosition should be unchanged
	TESTC(VerifyGetNextRows(m_ulTableRows, 0, NO_ROWS, FORWARD, NO_ROWS));
	TESTC(VerifyGetNextRows(1, 0, NO_ROWS, FORWARD, NO_ROWS));

	//skip all rows forward and fetch one row backward.
	TESTC(VerifyGetNextRows(m_ulTableRows, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc skip two rows backwards and fetch one row bakwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_AllProperties::Variation_3()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());

	//retrieve the third row
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-3), -1, THIRD_ROW));

	//skip two rows forward and fetch one row forward (last row)
	TESTC(VerifyGetNextRows((m_ulTableRows-3), 1, m_ulTableRows));

	//fetch one more row backward, the last row should be fetched again
	TESTC(VerifyGetNextRows(0, -1, m_ulTableRows));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc retrieve the rows in the order of 1, 5, 3, 2,4
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_AllProperties::Variation_4()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());

	//retrieve the first row
	TESTC(VerifyGetNextRows(0, 1, FIRST_ROW));

	//retrieve the fifth row
	TESTC(VerifyGetNextRows(3, 1, FIFTH_ROW));

	//retriev the third row
	TESTC(VerifyGetNextRows(-3, 1, THIRD_ROW));

	//retrieve the second row
	TESTC(VerifyGetNextRows(-1, -1, SECOND_ROW));

	//retrieve the fourth row
	TESTC(VerifyGetNextRows(3, -1, FOURTH_ROW));

CLEANUP:
	TRETURN
}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_AllProperties::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(TCIRowset_NoDiscontiguous)
//*-----------------------------------------------------------------------
//| Test Case:		TCIRowset_NoDiscontiguous - test a rowset with properties of CanHoldRows, CanScrollBackwards, and CanFetchBackwards
//|	Created:			12/14/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_NoDiscontiguous::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetProperty(DBPROP_CANHOLDROWS);			//Required Level-0
		SetProperty(DBPROP_IRowsetIdentity);		//Required Level-0

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc retrieve 5 row handles in the order of 3,4,2,5,1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_NoDiscontiguous::Variation_1()
{
	//retrieve the third row
	TESTC(VerifyGetNextRows(2, 1, THIRD_ROW));

	//retrieve the fourth row
	TESTC(VerifyGetNextRows(0, 1, FOURTH_ROW));
	
	//retrieve the 2nd row
	TESTC(VerifyGetNextRows(-2, -1, SECOND_ROW));
	
	//retrieve the fifth row
	TESTC(VerifyGetNextRows(4, -1, FIFTH_ROW));

	//retrieve the first row
	TESTC(VerifyGetNextRows(-3, -1, FIRST_ROW));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch one row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_NoDiscontiguous::Variation_2()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//restart cursor position
	TESTC(VerifyRestartPosition());

	//Fetch the second row
	TESTC_(GetNextRows(1, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], SECOND_ROW));
	
	//Fetch the first row
	TESTC_(GetNextRows(-1, -1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], FIRST_ROW));

	//restart the cursor position
	TESTC(VerifyRestartPosition());

	//fetch the first and second row handles again
	TESTC_(GetNextRows(0, 2, &rghRows[2]),S_OK);
	TESTC(VerifyRowHandles(2, &rghRows[2], FIRST_ROW));

	//Make sure the RowHandles are of the same row
	TESTC(IsSameRow(rghRows[0], rghRows[3]));
	TESTC(IsSameRow(rghRows[1], rghRows[2]));

	//Release the first and second row handles.  
	//Their reference counts should be 1, (only if there the same handle)
	TESTC(VerifyReleaseRows(1, &rghRows[0], rghRows[0] == rghRows[3] ? 1 : 0));
	TESTC(VerifyReleaseRows(1, &rghRows[1], rghRows[1] == rghRows[2] ? 1 : 0));

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows backwards and fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIRowset_NoDiscontiguous::Variation_3()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//skip 2 rows backwards and fetch three rows backwards. 
	//fetch the first three row handles
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-3), -3, THIRD_ROW, REVERSE));

CLEANUP:
	TRETURN
}




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition - with outstanding row handles
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIRowset_NoDiscontiguous::Variation_4()
{ 
	DBCOUNTITEM   cRowsObtained = 0;
	HROW*	rghRows = NULL;
	HROW	rghRows2[FIVE_ROWS];
	HRESULT hr = S_OK;

	//RestartPosition, verify
	TESTC(VerifyRestartPosition());

	//Retrieve rows
	TEST2C_(hr = GetNextRows(0, TWO_ROWS, &cRowsObtained, &rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);
	TESTC(VerifyRowHandles(cRowsObtained, rghRows, FIRST_ROW));

	//RestartPosition, (with outstanding row handles)
	if(!VerifyRestartPosition(NULL, FALSE/*fRowsReleased*/))
	{
		//Some providers require releasing all rows, before restarting...
		//We can still continue testing, but not the outstanding row handle part...
		ReleaseRows(cRowsObtained, rghRows);
		cRowsObtained = 0;
	}

	//The rest of the verification requires verification of outstanding row handles...
	if(hr == DB_S_ROWLIMITEXCEEDED)
		TESTC(m_ulMaxOpenRows == ONE_ROW);
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=FOUR_ROWS);
	
	//Obtain the second and third rows...
	//NOTE: If the above RestartPosition failed (ie: cRowsObtained==0) this call
	//will returned the fourth and fifth rows instead, since the cursor is not at the head...
	TESTC_(GetNextRows(1, TWO_ROWS, &rghRows2[SECOND_ROW]),S_OK);
	TESTC(VerifyRowHandles(TWO_ROWS, &rghRows2[SECOND_ROW], cRowsObtained ? SECOND_ROW : FOURTH_ROW));

	//Make sure the previously (outstanding) returned row handles are still valid...
	TESTC(VerifyRowHandles(cRowsObtained, rghRows, FIRST_ROW));

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	ReleaseRows(TWO_ROWS, rghRows2);
	SAFE_FREE(rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIRowset_NoDiscontiguous::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(KeysetCursor)
//*-----------------------------------------------------------------------
//| Test Case:		KeysetCursor - test keyset driven cursor model
//|	Created:			03/11/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		SetProperty(DBPROP_CANSCROLLBACKWARDS);
		SetProperty(DBPROP_CANFETCHBACKWARDS);
		SetProperty(DBPROP_OTHERUPDATEDELETE);
		SetProperty(DBPROP_CANHOLDROWS);			//Required Level-0
		SetProperty(DBPROP_IRowsetIdentity);		//Required Level-0

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc retrieve 5 row handles in the order of 3,4,2,5,1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_1()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());

	//retrieve the third row
	TESTC(VerifyGetNextRows(2, 1, THIRD_ROW));

	//retrieve the fourth row
	TESTC(VerifyGetNextRows(0, 1, FOURTH_ROW));
	
	//retrieve the 2nd row
	TESTC(VerifyGetNextRows(-2, -1, SECOND_ROW));
	
	//retrieve the fifth row
	TESTC(VerifyGetNextRows(4, -1, FIFTH_ROW));

	//retrieve the first row
	TESTC(VerifyGetNextRows(-3, -1, FIRST_ROW));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch one row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_2()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//restart cursor position
	TESTC(VerifyRestartPosition());

	//skip one row forward and fetch one row forward. Fetch the second row
	TESTC_(GetNextRows(1, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], SECOND_ROW));

	//skip one row backward and fetch oen row backward.  Fetch the first row
	TESTC_(GetNextRows(-1, -1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], FIRST_ROW));

	//restart the cursor position
	TESTC(VerifyRestartPosition());

	//fetch the first and second row handles again
	TESTC_(GetNextRows(0, 2, &rghRows[2]),S_OK);
	TESTC(VerifyRowHandles(rghRows[2], FIRST_ROW));
	TESTC(VerifyRowHandles(rghRows[3], SECOND_ROW));
	
	//Make sure the RowHandles are of the same row
	TESTC(IsSameRow(rghRows[0], rghRows[3]));
	TESTC(IsSameRow(rghRows[1], rghRows[2]));

	//release the first and second row handles.  Their reference counts should be 1
	//Their reference counts should be 1, (only if there the same handle)
	TESTC(VerifyReleaseRows(1, &rghRows[0], rghRows[0] == rghRows[3] ? 1 : 0));
	TESTC(VerifyReleaseRows(1, &rghRows[1], rghRows[1] == rghRows[2] ? 1 : 0));

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows backwards and fetch 3 rows backwards
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_3()
{
	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//skip 2 rows backwards and fetch three rows backwards. 
	//fetch the first three row handles
	TESTC(VerifyGetNextRows(NEGATIVE(m_ulTableRows-3), -3, THIRD_ROW, REVERSE));

CLEANUP:
	TRETURN
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor::Terminate()
{
	return TCIRowset::Terminate();
}


// {{ TCW_TC_PROTOTYPE(DynamicCursor)
//*-----------------------------------------------------------------------
//| Test Case:		DynamicCursor - test dynamic cursor
//|	Created:			03/11/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor::Init()
{
	if(TCIRowset::Init())
	{
		//Set required Properties
		//Note:  We have already testing scrolling and fetching backwards
		//with dynamic cursor, which some providers may not support those
		//combinations, so here, strictly test dynamic cursor without
		//these other requirements...
		SetProperty(DBPROP_OTHERINSERT);
		SetProperty(DBPROP_IRowsetIdentity);		//Required Level-0

		//Create rowset
		TEST_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc retrieve 5 row handles in the order of 3,4,2,5,1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_1()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];

	//NOTE:  We didn't ask for CANHOLDROWS, since this may be conflicting
	//with OtherInsert on some providers, and we want to do the most amount of
	//testing, so if CanHoldRows is on by default we will try to holdrows with
	//the dynamic cursor, otherwise were are forced to release them...
	
	//We also didn't ask for CanScrollBackwards or CanFetchBackwards since these
	//combinations are already tested in ScrollBackwards_Dynamic, and 
	//FetchBackwards_Dynamic, which may have been skipped due to conflicting 
	//properties.  This way we are testing Dynamic cursor, and if any of these
	//other properties are natively on, will test the additional functionality...

	//restart cursor position
	TESTC(VerifyRestartPosition());

	//retrieve the third row
	TESTC_(GetNextRows(2, 1, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(rghRows[0], THIRD_ROW));
	if(!m_bCanHoldRows)
		TESTC_(ReleaseRows(1, &rghRows[0]),S_OK);

	//retrieve the fourth row
	TESTC_(GetNextRows(0, 1, &rghRows[1]),S_OK);
	TESTC(VerifyRowHandles(rghRows[1], FOURTH_ROW));
	if(!m_bCanHoldRows)
		TESTC_(ReleaseRows(1, &rghRows[1]),S_OK);
	
	//retrieve the 2nd row
	if(m_bCanScrollBackwards && m_bCanFetchBackwards)
	{
		TESTC_(GetNextRows(-2, -1, &rghRows[2]),S_OK);
		TESTC(VerifyRowHandles(rghRows[2], SECOND_ROW));
		if(!m_bCanHoldRows)
			TESTC_(ReleaseRows(1, &rghRows[2]),S_OK);
	}
	
	//retrieve the fifth row
	if(m_bCanFetchBackwards)
	{
		TESTC_(GetNextRows(4, -1, &rghRows[3]),S_OK);
		TESTC(VerifyRowHandles(rghRows[3], FIFTH_ROW));
		if(!m_bCanHoldRows)
			TESTC_(ReleaseRows(1, &rghRows[3]),S_OK);
	}

	//retrieve the first row
	if(m_bCanScrollBackwards && m_bCanFetchBackwards)
	{	
		TESTC_(GetNextRows(-3, -1, &rghRows[4]),S_OK);
		TESTC(VerifyRowHandles(rghRows[4], FIRST_ROW));
	}

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc skip one row forward and fetch one row forward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_2()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];

	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//Fetch the second and third rows
	TESTC_(GetNextRows(1, 2, &rghRows[0]),S_OK);
	TESTC(VerifyRowHandles(2, &rghRows[0], SECOND_ROW));

	//We might need to release these rows, if CanHoldRows is not on...
	if(!m_bCanHoldRows)
		TESTC_(ReleaseRows(2, &rghRows[0]),S_OK);

	//restart the cursor position
	TESTC(VerifyRestartPosition());

	//fetch the first three rows... 
	TESTC_(GetNextRows(0, 3, &rghRows[2]),S_OK);
	TESTC(VerifyRowHandles(3, &rghRows[2], FIRST_ROW));

	if(m_bCanHoldRows)
	{
		//Make sure the RowHandles are the same row
		TESTC(IsSameRow(rghRows[0], rghRows[3]));
		TESTC(IsSameRow(rghRows[1], rghRows[4]));
	}

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc skip 2 rows and fetch 3 rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_3()
{
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;
	HRESULT hr = S_OK;

	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=3);

	//skip 2 rows fetch three rows. 
	TESTC(VerifyGetNextRows(2, 3, THIRD_ROW));

	//Fetch the first two rows again
	hr = GetNextRows(-3, -3, &cRowsObtained, &rghRows);
	TEST3C_(hr, DB_S_ENDOFROWSET, DB_E_CANTSCROLLBACKWARDS, DB_E_CANTFETCHBACKWARDS);

	if(hr==DB_S_ENDOFROWSET)
	{
		//Verify results
		TESTC(m_bCanFetchBackwards == TRUE);
		TESTC(m_bCanScrollBackwards == TRUE);
		
		//Verify Row handles and data
		TESTC(cRowsObtained == 2 && rghRows != NULL);
		TESTC(VerifyRowHandles(cRowsObtained, rghRows, SECOND_ROW, REVERSE));
	}
	else
	{
		//Verify results
		TESTC(cRowsObtained == 0 && rghRows == NULL);
		if(hr==DB_E_CANTSCROLLBACKWARDS)
			TESTC(m_bCanScrollBackwards == FALSE);
		if(hr==DB_E_CANTFETCHBACKWARDS)
			TESTC(m_bCanFetchBackwards == FALSE);
	}

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor::Terminate()
{
	return TCIRowset::Terminate();
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(Bookmarks)
//*-----------------------------------------------------------------------
//| Test Case:		Bookmarks - Test bookmarks on and off
//| Created:  	7/7/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks::Init()
{ 
	if(TCIRowset::Init())
	{
		//Bookmarks has to be supported for this testcase, either true or false
		TEST_PROVIDER(SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET));
		return TRUE;
	}

	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == TRUE, ForwardOnly
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_1()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//skip one row and fetch one row forward.
	TESTC(RowsetA.VerifyGetNextRows(1, 1, SECOND_ROW));

	//fetch the last row
	TESTC(RowsetA.VerifyGetNextRows(RowsetA.m_ulTableRows-2-1, 1, RowsetA.m_ulTableRows));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == TRUE, Static
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_2()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS);
	RowsetA.SetProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//skip one row and fetch one row forward.
	TESTC(RowsetA.VerifyGetNextRows(1, 1, SECOND_ROW));

	//fetch the first row again
	TESTC(RowsetA.VerifyGetNextRows(-2, 1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == TRUE, Keyset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_3()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS);
	RowsetA.SetProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//fetch the first row
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

	//fetch the first row (again)
	TESTC(RowsetA.VerifyGetNextRows(0, -1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//fetch the last row
	TESTC(RowsetA.VerifyGetNextRows(0, -1, RowsetA.m_ulTableRows));

	//fetch the last row (again)
	TESTC(RowsetA.VerifyGetNextRows(0, 1, RowsetA.m_ulTableRows));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == TRUE, Dynamic
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_4()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS);
	RowsetA.SetProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//fetch the first row
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

	//fetch the first row (again)
	if(RowsetA.m_bCanFetchBackwards)
		TESTC(RowsetA.VerifyGetNextRows(0, -1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//fetch the last row
	if(RowsetA.m_bCanFetchBackwards)
	{
		TESTC(RowsetA.VerifyGetNextRows(0, -1, RowsetA.m_ulTableRows));

		//fetch the last row (again)
		TESTC(RowsetA.VerifyGetNextRows(0, 1, RowsetA.m_ulTableRows));
	}

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == FALSE, ForwardOnly
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_5()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//skip one row and fetch one row forward.
	TESTC(RowsetA.VerifyGetNextRows(1, 1, SECOND_ROW));

	//fetch the last row
	TESTC(RowsetA.VerifyGetNextRows(RowsetA.m_ulTableRows-2-1, 1, RowsetA.m_ulTableRows));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == FALSE, Static
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_6()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	RowsetA.SetProperty(DBPROP_CANSCROLLBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//skip one row and fetch one row forward.
	TESTC(RowsetA.VerifyGetNextRows(1, 1, SECOND_ROW));

	//fetch the first row again
	TESTC(RowsetA.VerifyGetNextRows(-2, 1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == FALSE, Keyset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_7()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	RowsetA.SetProperty(DBPROP_OTHERUPDATEDELETE);
	RowsetA.SetProperty(DBPROP_CANFETCHBACKWARDS);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//fetch the first row
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

	//fetch the first row (again)
	TESTC(RowsetA.VerifyGetNextRows(0, -1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//fetch the last row
	TESTC(RowsetA.VerifyGetNextRows(0, -1, RowsetA.m_ulTableRows));

	//fetch the last row (again)
	TESTC(RowsetA.VerifyGetNextRows(0, 1, RowsetA.m_ulTableRows));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Bookmarks == FALSE, Dynamic
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Bookmarks::Variation_8()
{ 
	//Create rowset
	TCIRowset RowsetA;
	RowsetA.SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	RowsetA.SetProperty(DBPROP_OTHERINSERT);
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//fetch the first row
	TESTC(RowsetA.VerifyGetNextRows(0, 1, FIRST_ROW));

	//fetch the first row (again)
	if(RowsetA.m_bCanFetchBackwards)
		TESTC(RowsetA.VerifyGetNextRows(0, -1, FIRST_ROW));

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//fetch the last row
	if(RowsetA.m_bCanFetchBackwards)
	{
		TESTC(RowsetA.VerifyGetNextRows(0, -1, RowsetA.m_ulTableRows));

		//fetch the last row (again)
		TESTC(RowsetA.VerifyGetNextRows(0, 1, RowsetA.m_ulTableRows));
	}

	//restart cursor position
	TESTC(RowsetA.VerifyRestartPosition());

	//Fetch every row and verify the data...
	TESTC(RowsetA.VerifyAllRows());

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
BOOL Bookmarks::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowset::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(Status)
//*-----------------------------------------------------------------------
//| Test Case:		Status - test the  DBROWSTATUS array
//|	Created:			06/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Status::Init()
{
	if(TCIRowset::Init())
	{
		//Create rowset
		if(CreateRowset(SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTFECTHBACKWARDS, DB_E_CANTSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Status::Variation_1()
{
	HRESULT hr = S_OK;
	
	DBCOUNTITEM	cRowsObtained = 1;
	HROW*	rghRows = NULL;
	BOOL    fCanFetchBackwards = GetProperty(DBPROP_CANFETCHBACKWARDS);
	BOOL    fCanScrollBackwards = GetProperty(DBPROP_CANSCROLLBACKWARDS);

	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//can not scroll bakcwards.  As cRows==0, no-op.
	hr = GetNextRows(-1, 0, &cRowsObtained, &rghRows);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

	//It provider specific on cRows=0 wither it immeiately returns S_OK or 
	//weither it tries to validate other params
	TEST2C_(hr, S_OK, DB_E_CANTSCROLLBACKWARDS);
	if(fCanScrollBackwards)
		TESTC_(hr, S_OK);
	
	//DB_E_CANTFETCHBACKWARDS
	TESTC_(hr = GetNextRows(5, -2, &cRowsObtained, &rghRows), fCanFetchBackwards ? S_OK : DB_E_CANTFETCHBACKWARDS);
	TESTC_(ReleaseRows(cRowsObtained, rghRows), S_OK);
	PROVIDER_FREE(rghRows);
	
	//DB_E_CANTSCROLLBACKWARDS
	TESTC_(hr = GetNextRows(-1, 1, &cRowsObtained, &rghRows), fCanScrollBackwards ? S_OK : DB_E_CANTSCROLLBACKWARDS);
	TESTC_(ReleaseRows(cRowsObtained, rghRows), S_OK);
	PROVIDER_FREE(rghRows);

	//DB_S_ENDOFROWSET
	TESTC_(GetNextRows(m_ulTableRows + 10000, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && rghRows == NULL);

CLEANUP:
	PROVIDER_FREE(rghRows)
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc An array of success scenario
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Status::Variation_2()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];

	ULONG   rgRefCount[5];
	DBROWSTATUS rgRowStatus[5];

	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//get two row handles
	TESTC_(GetNextRows(0, 2, rghRows), S_OK);
	TESTC(VerifyRowHandles(2, rghRows, FIRST_ROW));

	//add ref
	TESTC_(m_pIRowset->AddRefRows(2, rghRows, rgRefCount, rgRowStatus),S_OK);
	TESTC(VerifyRefCounts(2, rgRefCount, 2));
	TESTC(VerifyRowStatus(2, rgRowStatus, DBROWSTATUS_S_OK));

	//release the row handle
	TESTC(VerifyReleaseRows(2, rghRows, 1, DBROWSTATUS_S_OK));
	TESTC(VerifyReleaseRows(2, rghRows, 0, DBROWSTATUS_S_OK));

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc An array of invalid row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Status::Variation_3()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[2];
	ULONG   rgRefCount[2] = { 1, 1 };
	DBROWSTATUS rgRowStatus[2] = { 0, 0};

	//restart cursor position
	TESTC(VerifyRestartPosition());

	//get one row handle
	TESTC_(GetNextRows(2, 1, rghRows), S_OK);
	TESTC(VerifyRowHandles(rghRows[0], THIRD_ROW));
	TESTC(VerifyReleaseRows(1, rghRows));

	//get an invalid row handle
	rghRows[0] = DB_NULL_HROW;
	rghRows[1] = ULONG_MAX;
	
	//add ref
	TESTC_(m_pIRowset->AddRefRows(2, rghRows, rgRefCount, rgRowStatus),	DB_E_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 0));
	TESTC(VerifyRefCounts(1, &rgRefCount[1], 0));
	TESTC(VerifyRowStatus(2, rgRowStatus, DBROWSTATUS_E_INVALID));

	//release the row handle
	TESTC_(pIRowset()->ReleaseRows(2, rghRows, NULL, rgRefCount, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 0));
	TESTC(VerifyRefCounts(1, &rgRefCount[1], 0));
	TESTC(VerifyRowStatus(2, rgRowStatus, DBROWSTATUS_E_INVALID));

	//add ref (NULL rgRefCount)
	TESTC_(m_pIRowset->AddRefRows(2, rghRows, NULL, rgRowStatus),	DB_E_ERRORSOCCURRED);
	TESTC(VerifyRowStatus(2, rgRowStatus, DBROWSTATUS_E_INVALID));
	
	//release the row handle (NULL rgRefCount)
	TESTC_(pIRowset()->ReleaseRows(2, rghRows, NULL, NULL, rgRowStatus), DB_E_ERRORSOCCURRED);
	TESTC(VerifyRowStatus(2, rgRowStatus, DBROWSTATUS_E_INVALID));

CLEANUP:
	ReleaseRows(2, rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc An array of valid and unvalid row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Status::Variation_4()
{
	ULONG	cRowsObtained = 0;
	HROW	rghRows[5];
	ULONG   rgRefCount[5];
	DBROWSTATUS rgRowStatus[5];

	//restart cursor position
	TESTC(VerifyRestartPosition());
	TESTC_PROVIDER(m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=2);

	//get two row handles
	TESTC_(GetNextRows(1, 2, rghRows),S_OK);
	TESTC(VerifyRowHandles(2, rghRows, SECOND_ROW));

	//release the second row handle
	TESTC(VerifyReleaseRows(1, &rghRows[1], 0 , DBROWSTATUS_S_OK));
	rghRows[1] = DB_NULL_HROW;

	//add ref
	TESTC_(m_pIRowset->AddRefRows(2, rghRows, rgRefCount, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 2));
	TESTC(VerifyRowStatus(1, &rgRowStatus[0], DBROWSTATUS_S_OK));
	TESTC(VerifyRefCounts(1, &rgRefCount[1], 0));
	TESTC(VerifyRowStatus(1, &rgRowStatus[1], DBROWSTATUS_E_INVALID));

	//release the row handle
	TESTC_(pIRowset()->ReleaseRows(2, rghRows, NULL, rgRefCount, rgRowStatus), DB_S_ERRORSOCCURRED);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 1));
	TESTC(VerifyRowStatus(1, &rgRowStatus[0], DBROWSTATUS_S_OK));
	TESTC(rgRefCount[1] == 0);
	TESTC(VerifyRowStatus(1, &rgRowStatus[1], DBROWSTATUS_E_INVALID));
	
	//release the row handle
	TESTC_(pIRowset()->ReleaseRows(1, &rghRows[0], NULL,rgRefCount,rgRowStatus), S_OK);
	TESTC(VerifyRefCounts(1, &rgRefCount[0], 0));
	TESTC(VerifyRowStatus(1, &rgRowStatus[0], DBROWSTATUS_S_OK));

CLEANUP:
	ReleaseRows(5, rghRows);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Status::Terminate()
{
	return TCIRowset::Terminate();
}	// }}
// }}
// }}
 

// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - Extended Errors
//|	Created:			07/20/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL ExtendedErrors::Init()
{
	return TCIRowset::Init();
}

 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ISupportErrorInfo for all rowset interfaces...
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ExtendedErrors::Variation_1()
{ 
	HRESULT hr = S_OK;
	ULONG i,cInterfaceMaps = 0;
	INTERFACEMAP* rgInterfaceMaps = NULL;

	COpenRowset OpenRowsetA;
	IRowset* pIRowset = NULL;
	IUnknown* pIUnknown = NULL;

	//Obtain all interfaces for this object...
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cInterfaceMaps, &rgInterfaceMaps));
	
	//Set all required properties and all other as optional.
	for(i=0; i<cInterfaceMaps; i++)
	{
		if(rgInterfaceMaps[i].dwPropertyID)
		{
			DBPROPOPTIONS dwPropOptions = DBPROPOPTIONS_OPTIONAL; 
			if(rgInterfaceMaps[i].fMandatory || rgInterfaceMaps[i].dwConfLevel==0)
				dwPropOptions = DBPROPOPTIONS_REQUIRED;

			//Set the property...
			TESTC(OpenRowsetA.SetProperty(rgInterfaceMaps[i].dwPropertyID, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, dwPropOptions));
		}
	}

	//Create rowset
	//NOTE: This function already does DefaultObjectTesting
	TEST2C_(OpenRowsetA.CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	
	//Run through the interfaces obtained (especially ISupportErrorInfo)...
	for(i=0; i<cInterfaceMaps; i++)
	{
		//Obtain this inteface.
		if(VerifyInterface(pIRowset, *(rgInterfaceMaps[i].pIID), ROWSET_INTERFACE, &pIUnknown))
			TESTC(DefaultInterfaceTesting(pIUnknown, ROWSET_INTERFACE, *(rgInterfaceMaps[i].pIID)));
		SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowset calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{  
	HRESULT hr = S_OK;
	HROW hRow = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Cause an error to occur
	m_pExtError->CauseError();

	//restart cursor position
	TESTC_(hr = RestartPosition(),S_OK);
	TESTC(XCHECK(pIRowset(), IID_IRowset, hr));	

	m_pExtError->CauseError();
	
	//NextFetchPosition should not be changed
	//It is provider specific wiether S_OK is returned or other 
	//parameter validation is done when cRows == 0
	hr = GetNextRows(NEGATIVE(m_ulTableRows+2), 0, &hRow);
	TEST2C_(hr, S_OK, DB_E_CANTSCROLLBACKWARDS);
	if(GetProperty(DBPROP_CANSCROLLBACKWARDS))
		TESTC_(hr, S_OK)

	//Check Extended Errors
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

	//make sure the cursor is at the beginning of the rowset
	//skip forward one row and fetch one row forward.
	TESTC(VerifyGetNextRows(1, 1, SECOND_ROW));

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowset calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	HRESULT hr = S_OK;

	DBBKMARK			cbBookmark;
	BYTE*			pBookmark = NULL;
	
	DBCOUNTITEM			cRowsObtained = 0;
	HROW*			rghRows = NULL;
	HROW			hRow = NULL;

	ULONG			ulRefCount = 0;
	IRowsetLocate*  pIRowsetLocate = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.

	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER, IID_IRowsetLocate)==S_OK);
	TESTC(VerifyInterface(pIRowset(), IID_IRowsetLocate, ROWSET_INTERFACE, (IUnknown**)&pIRowsetLocate));

	m_pExtError->CauseError();

	//get the book marks for the first row
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC_(hr = GetBookmark(hRow, &cbBookmark, &pBookmark),S_OK);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

	m_pExtError->CauseError();

	//addRef on the row handle and release it
	TESTC_(hr = pIRowset()->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	
	TESTC_(ReleaseRows(hRow),S_OK);
	TESTC_(ReleaseRows(hRow),S_OK);

	//NextFetchPosition should be unchanged
	//Also the 2.0 spec indicates lRowOffset is ignored
	TESTC_(GetNextRows(10000, 0, &hRow),S_OK);
	//get the first row
	TESTC_(pIRowsetLocate->GetRowsAt(NULL, NULL, cbBookmark, pBookmark, 0, 1, &cRowsObtained, &rghRows),S_OK);
	TESTC(cRowsObtained == 1 && rghRows != NULL);
	
	//get data for the first row
	TESTC(VerifyRowHandles(rghRows[0], FIRST_ROW));
	TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
	PROVIDER_FREE(rghRows);

	//the cursor is at the third row right now
	TESTC(VerifyGetNextRows(1, 1, THIRD_ROW));

CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	PROVIDER_FREE(pBookmark);	
	SAFE_RELEASE(pIRowsetLocate);
	DropRowset();
	TRETURN
}
//}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetData call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{  
	HRESULT hr = S_OK;
	HROW hRow = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//get one row handle
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC(VerifyRowHandles(hRow, FIRST_ROW));

	m_pExtError->CauseError();

	TESTC_(hr = pIRowset()->GetData(hRow, m_hAccessor, NULL),E_INVALIDARG);
	TESTC(XCHECK(pIRowset(), IID_IRowset, hr));	

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG AddRefRows call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{	
	HRESULT hr = S_OK;
	HROW hRow = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	m_pExtError->CauseError();
	
	TESTC_(hr = pIRowset()->AddRefRows(1, NULL, NULL, NULL),E_INVALIDARG);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ROWSNOTRELEASED RestartPosition call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
	HRESULT hr = S_OK;

	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_ALLFROMTBL)==S_OK);

	//retrieve one row
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC(VerifyRowHandles(hRow, FIRST_ROW));

	//NextFetchPosition should not be changed
	TESTC_(GetNextRows(0, 0, &hRow),S_OK);

	//addRef the row
	TESTC_(pIRowset()->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 2));
	TESTC_(ReleaseRows(hRow),S_OK);

	m_pExtError->CauseError();
	
	//call RestartPosition
	hr = RestartPosition();
	TEST3C_(hr, S_OK, DB_S_COMMANDREEXECUTED, DB_E_ROWSNOTRELEASED);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

	//release the first row
	TESTC_(ReleaseRows(hRow),S_OK);

	if(hr == S_OK)
	{
		TESTC_(GetNextRows(1, 1, &hRow),S_OK);
		TESTC(VerifyRowHandles(hRow, SECOND_ROW));
	}
	else if(hr == DB_E_ROWSNOTRELEASED)
	{
		TESTC_(GetNextRows(1, 1, &hRow),S_OK);
		TESTC(VerifyRowHandles(hRow, THIRD_ROW));
	}

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ENDOFROWSET GetNextRows call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_7()
{
	HRESULT hr = S_OK;

	DBCOUNTITEM cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_EMPTYROWSET)==S_OK);

	//count the row number in the rowset
	TESTC(GetTotalRows() == 0);
	m_pExtError->CauseError();
	
	//fetch one row forward
	TESTC_(hr = GetNextRows(0, 1, &cRowsObtained, &hRow), DB_S_ENDOFROWSET);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	
	TESTC(cRowsObtained == 0 && hRow == DB_NULL_HROW);

	m_pExtError->CauseError();
	
	//skip one row forward and fetch one row forward
	hr = GetNextRows(LONG_MAX, 1, &cRowsObtained, &hRow);
	TEST3C_(hr, DB_S_ENDOFROWSET, E_OUTOFMEMORY, DB_E_BADSTARTPOSITION);
	TESTC(cRowsObtained == 0 && hRow == DB_NULL_HROW);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG ReleaseRows call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_8()
{
	HRESULT hr = S_OK;

	ULONG cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	m_pExtError->CauseError();
	
	TESTC_(hr = pIRowset()->ReleaseRows(2, NULL, NULL, NULL, NULL),E_INVALIDARG);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetData call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_9()
{  
	HRESULT hr = S_OK;

	ULONG cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//get one row handle
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC(VerifyRowHandles(hRow, FIRST_ROW));
	
	//GetData
	TESTC_(hr = pIRowset()->GetData(hRow, m_hAccessor, NULL),E_INVALIDARG);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

	//ReleaseRows
	TESTC_(ReleaseRows(hRow),S_OK);

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG AddRefRows call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_10()
{	
	HRESULT hr = S_OK;

	ULONG cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	TESTC_(hr = pIRowset()->AddRefRows(1, NULL, NULL, NULL),E_INVALIDARG);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ROWSNOTRELEASED RestartPosition call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_11()
{
	HRESULT hr = S_OK;

	ULONG cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//retrieve one row
	TESTC_(GetNextRows(0, 1, &hRow),S_OK);
	TESTC(VerifyRowHandles(hRow, FIRST_ROW));

	//NextFetchPosition should not be changed
	TESTC_(GetNextRows(0, 0, &hRow),S_OK);
	//addRef the row
	TESTC_(pIRowset()->AddRefRows(1, &hRow, &ulRefCount, NULL),S_OK);
	TESTC(VerifyRefCounts(1, &ulRefCount, 2));
	TESTC_(ReleaseRows(hRow),S_OK);

	//call RestartPosition
	hr = RestartPosition();
	TEST3C_(hr, S_OK, DB_S_COMMANDREEXECUTED, DB_E_ROWSNOTRELEASED);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

	//release the first row
	TESTC_(ReleaseRows(hRow),S_OK);

	if(hr == S_OK)
	{
		TESTC_(GetNextRows(1, 1, &hRow),S_OK);
		TESTC(VerifyRowHandles(hRow, SECOND_ROW));
	}
	else if(hr == DB_E_ROWSNOTRELEASED)
	{
		TESTC_(GetNextRows(1, 1, &hRow),S_OK);
		TESTC(VerifyRowHandles(hRow, THIRD_ROW));
	}

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ENDOFROWSET GetNextRows call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_12()
{
	TBEGIN
	HRESULT hr = S_OK;

	DBCOUNTITEM cRowsObtained = 0;
	HROW  hRow = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	
	//count the row number in the rowset
	TESTC(GetTotalRows() == 0);

	//fetch one row forward
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &hRow), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 0 && hRow == NULL);

	//skip one row forward and fetch one row forward
	TESTC_(hr = GetNextRows(1, 1, &cRowsObtained, &hRow), DB_S_ENDOFROWSET);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	
	TESTC(cRowsObtained == 0 && hRow == NULL);

CLEANUP:
	ReleaseRows(hRow);
	DropRowset();
	TRETURN
}
//}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG ReleaseRows call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_13()
{
	HRESULT hr = S_OK;

	ULONG cRowsObtained = 0;
	HROW* rghRows = NULL;
	ULONG ulRefCount = 0;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get the rowset, create an accessor.  The rowset will only have 5 rows
	TESTC_PROVIDER(CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	TESTC_(hr = pIRowset()->ReleaseRows(2, NULL, NULL, NULL, NULL),E_INVALIDARG);
	TESTC(XCHECK(m_pIRowset, IID_IRowset, hr));	

CLEANUP:
	DropRowset();
	TRETURN
}
//}}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
 	return TCIRowset::Terminate();
}	// }}

// }}



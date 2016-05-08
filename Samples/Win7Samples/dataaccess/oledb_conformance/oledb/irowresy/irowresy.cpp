//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IROWRESY.CPP | Source file for IRowsetResynch Test Module.
//

#include "MODStandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "stddef.h"
#include "irowresy.h"
#include "txnbase.hpp"		//Base classes for transacted rowsets
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x30f88261, 0xb8b9, 0x11cf, { 0x97, 0x7f, 0x00, 0xaa, 0x00, 0xbd, 0xf9, 0x52 }};
DECLARE_MODULE_NAME("IRowsetResynch");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRowsetRefresh-IRowsetResynch Test Module");
DECLARE_MODULE_VERSION(840051520);
// }}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//This must be defined so that the base class constructors 
//in txnbase.hpp know what to use for this test module's name.
LPWSTR	gwszThisModuleName = L"IRowRefresh/IRowResynch";

DBLENGTH	g_ulRowSize		= 0;
BOOL		g_fDeletedRow	= FALSE;
BOOL		g_fBlobFail		= FALSE;
BOOL		g_fResynch		= FALSE;
BOOL		g_fRefresh		= FALSE;
BOOL		g_fVisualCache	= FALSE;
BOOL		g_fNOCHANGE		= FALSE;


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule	*pThisTestModule)
{	
	IOpenRowset		*pIOpenRowset		= NULL;
	IRowset			*pIRowset			= NULL;
	IRowsetResynch	*pIRowsetResynch	= NULL;
	IRowsetRefresh	*pIRowsetRefresh	= NULL;
	DBPROPSET		DBPropSetResynch;
	DBPROP			DBPropResynch;
	DBPROPSET		DBPropSetRefresh;
	DBPROP			DBPropRefresh;
	BOOL			fResults			= TEST_SKIPPED;;
	HRESULT			hr					= S_OK;

	//Structs for Resynch set properties
	DBPropSetResynch.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropSetResynch.rgProperties		= &DBPropResynch;
	DBPropSetResynch.cProperties		= 1;

	DBPropResynch.dwPropertyID			= DBPROP_IRowsetResynch;
	DBPropResynch.dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBPropResynch.colid					= DB_NULLID;
	DBPropResynch.vValue.vt				= VT_BOOL;
	V_BOOL(&(DBPropResynch.vValue))		= VARIANT_TRUE;	

	//Structs for Refresh set properties
	DBPropSetRefresh.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropSetRefresh.rgProperties		= &DBPropRefresh;
	DBPropSetRefresh.cProperties		= 1;

	DBPropRefresh.dwPropertyID			= DBPROP_IRowsetRefresh;
	DBPropRefresh.dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBPropRefresh.colid					= DB_NULLID;
	DBPropRefresh.vValue.vt				= VT_BOOL;
	V_BOOL(&(DBPropRefresh.vValue))		= VARIANT_TRUE;	

	if (ModuleCreateDBSession(pThisTestModule))
	{
		//Create a table we'll use for the whole test module,
		//store it in pVoid for now		
		pThisTestModule->m_pVoid = new CTable(
												(IUnknown *)pThisTestModule->m_pIUnknown2, 
												(LPWSTR)gwszModuleName
											);
		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			fResults = TEST_FAIL;
			goto DONE;
		}
		if (FAILED(((CTable *)pThisTestModule->m_pVoid)->CreateTable(NUM_ROWS*2)))
		{
			return FALSE;		
		}
		//Fail gracefully and quit module if we don't support openrowset
		if (FAILED(pThisTestModule->m_pIUnknown2->QueryInterface(
																	IID_IOpenRowset, 
																	(void **)&pIOpenRowset)))
		{
			fResults = TEST_SKIPPED;
			goto DONE;
		}
		
		//is Resynch supported?
		if	(	(pIOpenRowset->OpenRowset(	NULL, 
											&((CTable *)pThisTestModule->m_pVoid)->GetTableID(), 
											NULL, 
											IID_IRowset, 
											1, 
											&DBPropSetResynch, 
											(IUnknown **)&pIRowset)== S_OK)
																				&&
				(pIRowset->QueryInterface(IID_IRowsetResynch, (void **)&pIRowsetResynch)==S_OK)
			)
		{
			odtLog << L"IRowsetResynch Interface is supported.\n";
			odtLog << L"** WARNING - this interface has been depricated **.\n";
			g_fResynch	= TRUE;
		}
		else
		{
			odtLog << L"IRowsetResynch Interface is NOT supported.\n";
		}
		
		if(pIRowset)
		{
			pIRowset->Release();
			pIRowset = NULL;
		}

		//is Refresh supported?
		if	(	(pIOpenRowset->OpenRowset(	NULL, 
											&((CTable *)pThisTestModule->m_pVoid)->GetTableID(), 
											NULL, 
											IID_IRowset, 
											1, 
											&DBPropSetRefresh, 
											(IUnknown **)&pIRowset)== S_OK)
																				&&
				(pIRowset->QueryInterface(IID_IRowsetRefresh, (void **)&pIRowsetRefresh)==S_OK)
			)
		{
			g_fRefresh	= TRUE;
		}
		else
		{
			odtLog << L"IRowsetRefresh Interface is NOT supported.\n";
			fResults = TEST_SKIPPED;
			goto DONE;
		}

		if(pIRowset)
		{
			pIRowset->Release();
			pIRowset = NULL;
		}

		//test needs one of these to continue
		if (!g_fResynch && !g_fRefresh)
		{
			goto DONE;
		}

	if(pIRowsetResynch)
	{
		pIRowsetResynch->Release();
		pIRowsetResynch = NULL;
	}
	if(pIRowsetRefresh)
	{
		pIRowsetRefresh->Release();
		pIRowsetRefresh = NULL;
	}
		//check to see if DBROWSTATUS_S_NOCHANGE is returned by the provider when no change is made by resynch
		if (fnNOCHANGE(pIOpenRowset,pThisTestModule))
		{
			g_fNOCHANGE	= TRUE;
		}
			
		//check to see if the provider has a visual cache
		if (fnVisualCache(pIOpenRowset,pThisTestModule))
		{
			g_fVisualCache	= TRUE;
		}

		//Start with a table with rows.  The default will create a non updateable
		//column for the first column, which will be our index.  This will ensure
		//that when we change a row from another connection, we are not changing the key, 
		//so that in a keyset driven cursor, the changes are visible, and not hidden 
		//due to a changed an implicit delete and insert.
	
		//Init last actual insert number to last row we inserted
		g_ulLastActualInsert = NUM_ROWS*2;

		//Init last delete to 0		
		g_ulLastActualDelete = 0;

		//First row in the table is 1
		g_ulFirstRowInTable = 1;
		
		//If we made it this far, everything has succeeded
		fResults = TEST_PASS;
	}
DONE:
	if(pIOpenRowset)
	{
		pIOpenRowset->Release();
		pIOpenRowset = NULL;
	}
	if(pIRowset)
	{
		pIRowset->Release();
		pIRowset = NULL;
	}
	if(pIRowsetResynch)
	{
		pIRowsetResynch->Release();
		pIRowsetResynch = NULL;
	}
	if(pIRowsetRefresh)
	{
		pIRowsetRefresh->Release();
		pIRowsetRefresh = NULL;
	}

	return fResults;
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems3

//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	//We still own the table since all of our testcases
	//have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{
		if( ((CTable *)pThisTestModule->m_pVoid)->GetCommandSupOnCTable() )
			((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	//clean golbas in case test is run twice
	g_ulRowSize		= 0;
	g_fDeletedRow	= FALSE;
	g_fBlobFail		= FALSE;
	g_fResynch		= FALSE;
	g_fRefresh		= FALSE;
	g_fVisualCache	= FALSE;
	g_fNOCHANGE		= FALSE;

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
	TCBase() { SetTestCaseParam(TI_IRowsetResynch); }

	//Set the m_eTI
	virtual void SetTestCaseParam(ETESTINTERFACE eTestInterface = TI_IRowsetResynch)
	{
		m_eTI = eTestInterface;
	}

	HRESULT fnInterfaceSupported()
	{
		if(TI_IRowsetResynch==m_eTI)
		{
			if (!g_fResynch)
			{
				return TEST_SKIPPED;
			}
		}
		else
		{
			if (!g_fRefresh)
			{
				return TEST_SKIPPED;
			}
		}
		return TEST_PASS;
	}

	BOOL fnIsRowsetUpdatePending(IUnknown	*pIUnknown)
	{
		VARIANT_BOOL	bValue;

		//if IRowsetResynch is being used always return TRUE
		//this is because TRUE will cause the test to expect DBROWSTATUS_S_OK or DBROWSTATUS_S_NOCHANGE.
		//FALSE cause the test to expect only DBROWSTATUS_S_NOCHANGE.  IRowsetResynch only supports DBROWSTATUS_S_OK.
		//DBROWSTATUS_S_NOCHANGE  won't make a difference with IRowsetResynch but it needs to check for DBROWSTATUS_S_OK. 
		if (m_eTI==TI_IRowsetResynch)
		{
			return TRUE;
		}

		if (GetProperty(DBPROP_IRowsetUpdate,DBPROPSET_ROWSET,pIUnknown,&bValue))
		{
			if(bValue==VARIANT_TRUE)
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	//@cmember for keeping track of which interface to use
	ETESTINTERFACE	m_eTI;
};


//////////////////////////////////////////////////////////
//Helper function to determine if a property is supported
//on the given rowset
DBPROPSTATUS	IsRowsetPropSupported(IUnknown *pRowset, 
									  DBPROPID PropID)
{
	IRowsetInfo		*pIRowsetInfo		= NULL;
	DBPROPIDSET		PropIDSet;
	ULONG			cDBPropSet			= 0;
	DBPROPSET		*rgDBPropSet		= NULL;
	//This is our default error, even though its not
	//overly reflective of our routine failing
	DBPROPSTATUS	dwStatus			= DBPROPSTATUS_BADOPTION;	
	HRESULT			hr					= NOERROR;
	IMalloc			*pIMalloc			= NULL;

	if (FAILED(CoGetMalloc(1, &pIMalloc)))
	{
		return dwStatus;
	}
	ASSERT(pRowset);

	PropIDSet.cPropertyIDs		= 1;
	PropIDSet.guidPropertySet	= DBPROPSET_ROWSET;
	PropIDSet.rgPropertyIDs		= &PropID;

	if (pRowset->QueryInterface(IID_IRowsetInfo, (void **)&pIRowsetInfo) == S_OK)
	{
		if (pIRowsetInfo->GetProperties(1, &PropIDSet, &cDBPropSet, &rgDBPropSet) == S_OK)
		{
			if (rgDBPropSet)
				if (cDBPropSet == 1)
					if (rgDBPropSet[0].rgProperties)
						if (rgDBPropSet[0].cProperties == 1)
							//Get status we'll return (ie, supported, not supported)
							dwStatus = rgDBPropSet[0].rgProperties[0].dwStatus;
		}
	}
	
	if(rgDBPropSet)
	{
		if(rgDBPropSet->rgProperties)
		{	
			PROVIDER_FREE(rgDBPropSet->rgProperties);	
		}

		PROVIDER_FREE(rgDBPropSet);
	}
	if (pIRowsetInfo)
	{
		pIRowsetInfo->Release();
		pIRowsetInfo = NULL;
	}
	return dwStatus;
}

//////////////////////////////////////////////////////////
//Helper to see if DBROWSTATUS_S_NOCHANGE is returned by 
//the provider when no change is made by resynch
BOOL fnNOCHANGE(	IOpenRowset		*pIOpenRowset,
					CThisTestModule	*pThisTestModule)
{
	IRowsetRefresh	*pIRowsetRefresh= NULL;
	IRowset			*pIRowset		= NULL;
	const ULONG		cProps			= 1;
	DBPROPSET		DBPropSet;
	DBPROP			DBProp[cProps];
	DBORDINAL		cRowsetCols;
	const ULONG		cRows			= 1;
	HROW			rghRows[cRows];
	HROW			*phRows			= rghRows;
	DBCOUNTITEM		cRowsObtained	= 1; 
	DBCOUNTITEM		cRowsResynched	= 0;
	DBROWSTATUS		*rgReRowStatus	= NULL;
	HROW			*rghReRows		= NULL;
	BOOL			fNoChange		= FALSE;

	//if RefreshData is NOT supported then leave this global flag set as flag 
	//cause ResynchData does not support DBROWSTATUS_S_NOCHANGE
	if (g_fRefresh)
	{
		//Struct for set properties
		DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;
		DBPropSet.rgProperties		= DBProp;
		DBPropSet.cProperties		= cProps;
		
		DBProp[0].dwPropertyID		= DBPROP_IRowsetRefresh;
		DBProp[0].dwOptions			= 0;
		DBProp[0].colid				= DB_NULLID;
		DBProp[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[0].vValue))	= VARIANT_TRUE;			

		//get a rowset
		((CTable *)pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
															IID_IRowset, 
															1, 
															&DBPropSet, 
															(IUnknown**)&pIRowset, 
															NULL, 
															&cRowsetCols, 
															NULL, 
															0, 
															NULL,
															pIOpenRowset);	
			
		//get the first row
		TESTC_(pIRowset->RestartPosition(NULL), S_OK);
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows),S_OK);
		
		//get refresh(or resynch) from rowset
		if (!VerifyInterface(pIRowset, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;	
		}
			
		//refresh the data from the rowset
		TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &phRows[0], TRUE, &cRowsResynched, &rghReRows, &rgReRowStatus),S_OK);
			
		//Free the row for the GetNextRows call
		TESTC_(pIRowset->ReleaseRows(1, phRows, NULL, NULL, NULL), S_OK);

		if (rgReRowStatus[cRowsResynched-1]==DBROWSTATUS_S_NOCHANGE)
		{
			fNoChange	= TRUE;
		}
		//else
		//{
		//	fNoChange = FALSE;  //FASLE is default
		//}
	}
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetRefresh);
		
	PROVIDER_FREE(rgReRowStatus);
	PROVIDER_FREE(rghReRows);

	return fNoChange;
}


//////////////////////////////////////////////////////////
//Helper to see if the provider has a visual cache
BOOL fnVisualCache(IOpenRowset		*pIOpenRowset,
				   CThisTestModule	*pThisTestModule)
{
	HACCESSOR			hAccessor1		= DB_NULL_HACCESSOR;
	HACCESSOR			hAccessor2		= DB_NULL_HACCESSOR;
	IAccessor			*pIAccessor1	= NULL;
	IAccessor			*pIAccessor2	= NULL;
	DBBINDING			*rgBindings		= NULL;
	DBCOUNTITEM			cBindings		= 0;
	ITransactionLocal	*pITransactionLocal	= NULL;
	IRowsetChange		*pIRowsetChange	= NULL;
	IRowsetRefresh		*pIRowsetRefresh= NULL;
	IRowsetResynch		*pIRowsetResynch= NULL;
	IRowset				*pIRowset1		= NULL;
	IRowset				*pIRowset2		= NULL;
	const ULONG			cProps			= 4;
	DBPROPSET			DBPropSet;
	DBPROP				DBProp[cProps];
	HRESULT				hr				= S_OK;
	BYTE				*pData			= NULL;
	DB_LORDINAL			*rgTableColOrds;			
	DBORDINAL			cRowsetCols;
	const ULONG			cRows			= 1;
	HROW				rghRows1[cRows];
	HROW				*phRows1		= rghRows1;
	HROW				rghRows2[cRows];
	HROW				*phRows2		= rghRows2;
	BOOL				fFound			= FALSE;
	DBCOUNTITEM			cRowsObtained	= 1; 
	IMalloc				*pIMalloc		= NULL;	
	DBLENGTH			cRowSize		= 0;

	//If we can't get our memory allocator, we're in trouble anyway, so assert
	CoGetMalloc(MEMCTX_TASK, &pIMalloc);
	ASSERT(pIMalloc);

	//Struct for set properties
	DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropSet.rgProperties		= DBProp;
	DBPropSet.cProperties		= cProps;

	DBProp[0].dwPropertyID			= DBPROP_IRowsetChange;
	DBProp[0].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[0].colid					= DB_NULLID;
	DBProp[0].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[0].vValue))		= VARIANT_TRUE;	

	DBProp[1].dwPropertyID			= DBPROP_UPDATABILITY;
	DBProp[1].dwOptions				= 0;
	DBProp[1].colid					= DB_NULLID;
	DBProp[1].vValue.vt				= VT_I4;
	DBProp[1].vValue.lVal			= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	if(g_fRefresh)
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetRefresh;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;			
	}
	else
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetResynch;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;
	}

	DBProp[3].dwPropertyID			= DBPROP_OTHERUPDATEDELETE;
	DBProp[3].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[3].colid					= DB_NULLID;
	DBProp[3].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[3].vValue))		= VARIANT_TRUE;	

	//Get IRowset on the object
	if (!VerifyInterface(pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		goto CLEANUP;	
	}

	hr=pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, NULL);

	//get 2 rowsets
	((CTable *)pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
														IID_IRowset, 
														1, 
														&DBPropSet, 
														(IUnknown**)&pIRowset1, 
														NULL, 
														&cRowsetCols, 
														&rgTableColOrds, 
														0, 
														NULL,
														pIOpenRowset);	
		
	PROVIDER_FREE(rgTableColOrds);

	((CTable *)pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
														IID_IRowset, 
														1, 
														&DBPropSet, 
														(IUnknown**)&pIRowset2, 
														NULL, 
														&cRowsetCols, 
														&rgTableColOrds, 
														0, 
														NULL,
														pIOpenRowset);		

	//Get IRowset on the object
	if (!VerifyInterface(pIRowset1, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor1))
	{
		goto CLEANUP;	
	}

	//Get IRowset on the object
	if (!VerifyInterface(pIRowset2, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor2))
	{
		goto CLEANUP;	
	}

	TESTC_(GetAccessorAndBindings(	pIAccessor1, 
									DBACCESSOR_ROWDATA,
									&hAccessor1, 
									&rgBindings, 
									&cBindings, 
									&cRowSize,			
  									DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
									ALL_COLS_BOUND,
									FORWARD, 
									NO_COLS_BY_REF, 
									NULL, 
									NULL,
									NULL, 
									DBTYPE_EMPTY, 
									0, 
									NULL, 
									NULL, 
									NO_COLS_OWNED_BY_PROV, 
									DBPARAMIO_NOTPARAM,
									NO_BLOB_COLS),S_OK);


	//alloc here just so test has it to free for ReleaseInputBindingsMemory
	pData = (BYTE *)PROVIDER_ALLOC(cRowSize);
	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}
	pData	= NULL;

	TESTC_(GetAccessorAndBindings(	pIAccessor2, 
									DBACCESSOR_ROWDATA,
									&hAccessor2, 
									&rgBindings, 
									&cBindings, 
									NULL,			
  									DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
									ALL_COLS_BOUND,
									FORWARD, 
									NO_COLS_BY_REF, 
									NULL, 
									NULL,
									NULL, 
									DBTYPE_EMPTY, 
									0, 
									NULL, 
									NULL, 
									NO_COLS_OWNED_BY_PROV, 
									DBPARAMIO_NOTPARAM,
									NO_BLOB_COLS),S_OK);

	//Set data for all columns
	TESTC_(FillInputBindings(	((CTable *)pThisTestModule->m_pVoid), 
								DBACCESSOR_ROWDATA, 
								cBindings, 
								rgBindings, 
								&pData, 
								98, 
								((CTable *)pThisTestModule->m_pVoid)->CountColumnsOnTable(),
								rgTableColOrds), S_OK);

	//Get IRowsetChange on the object
	if (!VerifyInterface(pIRowset1, IID_IRowsetChange, ROWSET_INTERFACE, (IUnknown **)&pIRowsetChange))
	{
		goto CLEANUP;	
	}

	//get the first row through the 1st rowset
	pIRowset1->RestartPosition(NULL);

	TESTC_(pIRowset1->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1),S_OK);
	
	//get the first row through the 2nd rowset
	TESTC_(pIRowset2->RestartPosition(NULL), S_OK);
	TESTC_(pIRowset2->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2),S_OK);

	//change the data through the first rowset
	TESTC_(pIRowsetChange->SetData(phRows1[0], hAccessor1, pData), S_OK);
	TESTC_(pIRowset1->ReleaseRows(cRowsObtained, phRows1, NULL, NULL, NULL), S_OK);

	//get refresh(or resynch) from rowset2
	if(g_fRefresh)
	{
		if (!VerifyInterface(pIRowset2, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;	
		}
	}
	else
	{
		if (!VerifyInterface(pIRowset2, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown **)&pIRowsetResynch))
		{
			goto CLEANUP;	
		}
	}
		
	//get the visible data from the 2nd rowset
	if (g_fRefresh)
	{
		TESTC_(pIRowsetRefresh->GetLastVisibleData(phRows2[0], hAccessor2, pData),S_OK);
	}
	else
	{
		TESTC_(pIRowsetResynch->GetVisibleData(phRows2[0], hAccessor2, pData),S_OK);
	}
		
	//Check this row against ulRowNum, free pData when done
	if (CompareData(((CTable *)pThisTestModule->m_pVoid)->CountColumnsOnTable(), rgTableColOrds,
					98, pData, cBindings, rgBindings, 
					((CTable *)pThisTestModule->m_pVoid), pIMalloc, PRIMARY, 
					COMPARE_FREE, COMPARE_UNTIL_ERROR))
	{				
		//We found the row, woo-hoo.
		fFound = TRUE;
	}
	//Free the row for this GetNextRows call
	TESTC_(pIRowset2->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
CLEANUP:
	//clean up any changes this little kludge messed on the back end 
	hr=pITransactionLocal->Abort(NULL,FALSE,FALSE);

	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}
	PROVIDER_FREE(rgTableColOrds);

	//Release accessor1
	if ((hAccessor1 != DB_NULL_HACCESSOR) && pIAccessor1)
	{
		CHECK(pIAccessor1->ReleaseAccessor(hAccessor1, NULL), S_OK);
		hAccessor1 = DB_NULL_HACCESSOR;
	}
	//Release accessor2
	if ((hAccessor2 != DB_NULL_HACCESSOR) && pIAccessor2)
	{
		CHECK(pIAccessor2->ReleaseAccessor(hAccessor2, NULL), S_OK);
		hAccessor2 = DB_NULL_HACCESSOR;
	}


	SAFE_RELEASE(pITransactionLocal);
	SAFE_RELEASE(pIAccessor1);
	SAFE_RELEASE(pIAccessor2);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetRefresh);
	SAFE_RELEASE(pIRowsetResynch);

	SAFE_RELEASE(pIMalloc);
	//if found - no visualcache
	//if not found - there is a visible cache	
	return !fFound;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Declarations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//////////////////////////////////////////////////////////////
//Base class for all IRowResy test cases
//////////////////////////////////////////////////////////////
class CResynchRefresh : public CTxnImmed, public TCBase
{
// @access public
public:
	//@cmember Tells if long data can be supported on this rowset
	BLOBTYPE	m_fBindLongData;
	//@cmember Accessor handle for Read Only Rowset
	HACCESSOR	m_hROAccessor;
	//@cmember Accessor handle for Changeable Rowset
	HACCESSOR	m_hChgAccessor;
	//@cmember Accessor handle for Changeable Rowset
	HACCESSOR	m_hChgAccessor2;
	//@cmember Number of bindings in the Accessors
	DBCOUNTITEM	m_cBindings;
	//@cmember Array of bindings for the Accessors 
	DBBINDING	*m_rgBindings;
	//@cmember Row size for one row using the Accessors
	DBLENGTH	m_cbRowSize;
	//@cmember Memory to put address of cached data
	BYTE		*m_pRowsetData;
	//@cmember Memory to put address of visible data
	BYTE		*m_pVisibleData;
	//@cmember Memory to put address of newly resynch'd rowset data
	BYTE		*m_pResynchRowsetData;
	//@cmember Memory to put address of newly resynch'd visible data
	BYTE		*m_pResynchVisibleData;
	//@cmember Flag indicating if the row change is successful
	BOOL		m_fNoChange;
	//@cmember Flag indicating if pass by ref accessors are supported
	BOOL		m_fPassByRef;
	//@cmember Flag indicating if strong identity is supported
	BOOL		m_fStrongIdentity;
	//@cmember Memory for count of rows resynched
	DBCOUNTITEM	m_cRowsResynched;
	//@cmember Memory for array of hrows Resynched
	HROW		*m_rghRowsResynched;
	//@cmember Memory for array of row status
	DBROWSTATUS *m_rgRowStatus;
	//@cmember Flag indicating if CreateResynchObjects should
	//create accessors with BLOBS bound if it is supported.
	//Initialized to FALSE.
	BOOL		m_fRequestLongDataIfSupported;


	//@cmember IRowsetResynch interface on Read Only Rowset 
	IRowsetResynch	*m_pROIRowsetResynch;
	//@cmember IRowsetResynch interface on Changeable Rowset 
	IRowsetResynch	*m_pChgIRowsetResynch;

	//@cmember IRowsetRefresh interface on Read Only Rowset 
	IRowsetRefresh	*m_pROIRowsetRefresh;
	//@cmember IRowsetRefresh interface on Changeable Rowset 
	IRowsetRefresh	*m_pChgIRowsetRefresh;
	
	//@cmember IRowset interface used to free hRows
	IRowset		*m_pChgIRowset;
	//@cmember IRowset interface used to free hRows
	IRowset		*m_pROIRowset;


	//@cmember CTOR
	//CGetSession(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	CResynchRefresh(LPWSTR tcName) : CTxnImmed(tcName)
	{
		m_pROIRowsetResynch				= NULL;
		m_pChgIRowsetResynch			= NULL;
		m_pROIRowsetRefresh				= NULL;
		m_pChgIRowsetRefresh			= NULL;
		m_pChgIRowset					= NULL;
		m_pROIRowset					= NULL;
		m_hROAccessor					= DB_NULL_HACCESSOR;
		m_hChgAccessor					= DB_NULL_HACCESSOR;
		m_hChgAccessor2					= DB_NULL_HACCESSOR;
		m_cBindings						= 0;
		m_rgBindings					= NULL;
		m_cbRowSize						= 0;
		m_pRowsetData					= NULL;
		m_pVisibleData					= NULL;
		m_pResynchRowsetData			= NULL;
		m_pResynchVisibleData			= NULL;
		m_fNoChange						= FALSE;
		m_fPassByRef					= FALSE;			
		m_fBindLongData					= NO_BLOB_COLS;
		m_fRequestLongDataIfSupported	= FALSE;
	};

	//Allocs memory for data buffers
	BOOL AllocDataBuffers(DBLENGTH cbRowSize);

	//Frees memory for data buffers
	void FreeBuffers(DBLENGTH cbRowSize);

	//@cmember Frees any memory associated with the output params of ResynchRows
	void FreeOutParams();

	//@cmember Compares the outparams to make sure they contain the right values
	void CompareOutParams(DBCOUNTITEM cExpectedRows, HROW * rgExpectedhRows);

	//@cmember Checks that the output parameters are properly set when an error occurs
	void CheckOutParamsAreNulled();

	//@cmember Does generic IRowsetResynch testing -- to be called with different
	//property settings
	int TestResynchRefresh(ETESTROWSETTYPE eTestRowsetType);

	//@cmember Calls ChangeUnderlyingRowAndGetHrow and GetDataBuffers
	//retrieves visible data and cached data (via GetData) for that row
	BOOL GenerateResynchData(	CTxnRowset	*pFirstTxnRowset, 
								ISOLEVEL	fIsoLevel,
								HACCESSOR	hAccessor,
								DBBINDING	*rgBindings = NULL,
								DBCOUNTITEM	cBindings	= 0);

	//@cmember Determines if pass by ref accessors are supported 
	void DetermineProps();

	//@cmember checks property status for required properties after CreatRowsetObject()
	BOOL DidPropsFail(CTxnRowset * pRowset);	

	//@cmember Changes the underlying row via another transaction, and returns
	//the hRow corresponding to that row, before the cache has been resynch'd.
	//If phRow is NULL, the hRow is released and is not passed to the caller
	BOOL ChangeUnderlyingRowAndGetHrow(	CTxnRowset	*pFirstTxnRowset, 
										ISOLEVEL	fIsoLevel, 
										HROW		*phRow);

	//@cmember Does GetVisibleData and GetData on the cached row.  Then
	// does ResynchRows and GetData on the resynch'd row.
	BOOL GetDataBuffers(CTxnRowset	*pFirstTxnRowset, 
						ISOLEVEL	fIsoLevel, 
						HACCESSOR	hAccessor, 
						HROW		hRow,
						DBBINDING	*rgBindings	= NULL,
						DBCOUNTITEM	cBindings	= 0);

	//@cmember Verifies visible data buffer based on fSeeVisibleData & 
	//fSeeVisibleResynchedData flags and GetData cached buffer based on 
	//fSeeRowsetData flag.  If bindings are not passed, the data members 
	//m_cBindings and m_rgBindings are used for determining buffer format.  
	//Note that VERIFY_IGNORE is used when we are using BLOB columns, 
	//we can never get cached BLOB values.  So VERIFY_IGNORE is placed for
	//all fSeeRowseteData where any BLOBS are bound, and also whereever
	//VERIFY_OLD is normally expected and BLOBS are bound.
	BOOL VerifyData(EVERIFY		fSeeVisibleResynchedData, 
					EVERIFY		fSeeVisibleData,
					EVERIFY		fSeeRowsetData				= VERIFY_OLD,
					EVERIFY		fSeeRowsetResynchedData		= VERIFY_OLD,
					ULONG		ulOldRowNum					= 0, 
					ULONG		ulNewRowNum					= 0,
					DBCOUNTITEM	cVerifyBindings				= 0, 
					DBBINDING	*rgVerifyBindings			= NULL);

	//@cmember Sets all correct properties for testing rowsets with IRowsetResynch
	BOOL SetAllProperties(	CTxnRowset	*pTxnRowset, 
							ULONG		cAdditionalProps, 
							DBPROP		*rgAdditionalProps);

	//@cmember Changes the rowsets created in CResynchRefresh::Init to not support
	//some properties, as well as to support the properties passed in.
	//Also if fBindLongData is TRUE, this is set in the rowset objects 
	//so that they create accessors with long columns bound
	BOOL ChangeProperties(ULONG cAddProps, DBPROP * rgAddProps, ULONG cRemoveProps, DBPROPID * rgRemoveProps, 
		DBPROPOPTIONS * rgPropOptions, BLOBTYPE	fBindLongData = NO_BLOB_COLS);

	//@cmember Starts both Chg1 and RO1 rowsets in specificied transaction isolation level
	HRESULT StartTxns( ISOLEVEL fIsoLevel);
	
	//@cmember Ends both Chg1 and RO1 rowsets' transactions by doing a non retaining commit
	BOOL EndTxns(BOOL fAbort = FALSE);

	//@cmember creates all objects needed for CResynchRefresh::Init
	BOOL CreateResynchObjects(ETESTINTERFACE	eTI);

	//@cmember Releases all objects created by CreateResynchObjects
	void ReleaseResynchObjects();

	//@cmember ResynchRows\RefreshVisibleData wrapper
	HRESULT	ResynchRefresh(	HCHAPTER        hChapter,
							DBCOUNTITEM		cRows,
							const HROW      rghRows[],
							BOOL            fOverwrite,
							DBCOUNTITEM		*pcRowsRefreshed,
							HROW			**prghRowsRefreshed,
							DBROWSTATUS		**prgRowStatus,
							BOOL			fRO);

	//@cmember Get(Last)VisibleData wrapper
	HRESULT	GetLastVisibleData(	HROW		hRow,
								HACCESSOR	hAccessor,
								void		*pData,
								BOOL		fRO);

	//@cmember Init
	BOOL	Init(ETESTINTERFACE	eTI);
	//@cmember Terminate
	BOOL	Terminate();	
};


//--------------------------------------------------------------------
// @mfunc Init
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::Init(ETESTINTERFACE	eTI)
{
	const ULONG			cAdditionalProps = 2;
	DBPROP				rgAdditionalProps[cAdditionalProps];

	//set a which interface to use in the class
	m_eTI=eTI;

	if(TI_IRowsetResynch==eTI)
	{
		rgAdditionalProps[0].dwPropertyID		= DBPROP_IRowsetResynch;
		rgAdditionalProps[0].dwOptions			= 0;
		rgAdditionalProps[0].colid				= DB_NULLID;
		rgAdditionalProps[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgAdditionalProps[0].vValue))	= VARIANT_TRUE;
			
		rgAdditionalProps[1].dwPropertyID		= DBPROP_CANHOLDROWS;
		rgAdditionalProps[1].dwOptions			= 0;
		rgAdditionalProps[1].colid				= DB_NULLID;
		rgAdditionalProps[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgAdditionalProps[1].vValue))	= VARIANT_TRUE;
	}
	else
	{
		rgAdditionalProps[0].dwPropertyID		= DBPROP_IRowsetRefresh;
		rgAdditionalProps[0].dwOptions			= 0;
		rgAdditionalProps[0].colid				= DB_NULLID;
		rgAdditionalProps[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgAdditionalProps[0].vValue))	= VARIANT_TRUE;
			
		rgAdditionalProps[1].dwPropertyID		= DBPROP_CANHOLDROWS;
		rgAdditionalProps[1].dwOptions			= 0;
		rgAdditionalProps[1].colid				= DB_NULLID;
		rgAdditionalProps[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgAdditionalProps[1].vValue))	= VARIANT_TRUE;
	}

	if (CTxnImmed::Init())		
	{
		//Set properties for this rowset, including those needed for IRowsetResynch	/ IRowsetRefresh
		if (SetAllProperties(m_pChgRowset2, cAdditionalProps, rgAdditionalProps))				
		{
			//Set properties for this rowset, including those needed for IRowsetResynch	/ IRowsetRefresh
			if (SetAllProperties(m_pChgRowset1, cAdditionalProps, rgAdditionalProps))				
			{
				//Now do the same thing for the read only rowset
				if (SetAllProperties(m_pRORowset1, cAdditionalProps, rgAdditionalProps))				
				{
					//Make rowsets and accessors and get IRowsetResynch/IRowsetRefresh interfaces on the
					//first changeable rowset and the read only rowset				
					if (CreateResynchObjects(eTI))
					{					
						//Find out what type of property support we have
						DetermineProps();
						return TRUE;					
					}
					else
					{
						//This is the only valid reason for our object creation to fail.
						//NOTE this is also the only place we allow the funciton to fail
						//If we get past this function in init, we know the support is there and it 
						//should always succeed after that.
						odtLog << wszResynchNotSupported;
					}
				}
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//Allocs memory for data buffers
//If buffers already exists, it frees and reallocs the new size given
BOOL CResynchRefresh::AllocDataBuffers(DBLENGTH cbRowSize)
{
	//Free any existing buffers
	PROVIDER_FREE(m_pVisibleData);
	PROVIDER_FREE(m_pRowsetData);
	PROVIDER_FREE(m_pResynchRowsetData);
	PROVIDER_FREE(m_pResynchVisibleData);

	m_pVisibleData			= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	m_pRowsetData			= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	m_pResynchRowsetData	= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	m_pResynchVisibleData	= (BYTE *)PROVIDER_ALLOC(cbRowSize);

	memset(m_pVisibleData, 0, (size_t)cbRowSize);
	memset(m_pRowsetData, 0, (size_t)cbRowSize);
	memset(m_pResynchRowsetData, 0, (size_t)cbRowSize);
	memset(m_pResynchVisibleData, 0, (size_t)cbRowSize);

	return m_pVisibleData && m_pRowsetData && m_pResynchRowsetData && m_pResynchVisibleData;
}
//--------------------------------------------------------------------
//Frees memory for m_pVisibleData, m_pResynchRowsetData and m_pRowsetData buffers
void CResynchRefresh::FreeBuffers(DBLENGTH	cbRowSize)
{
	//Free any existing buffers
	PROVIDER_FREE(m_pVisibleData);
	PROVIDER_FREE(m_pRowsetData);
	PROVIDER_FREE(m_pResynchRowsetData);
	PROVIDER_FREE(m_pResynchVisibleData);

	if (m_pVisibleData)
	{
		memset(m_pVisibleData, 0, (size_t)cbRowSize);
	}
	if (m_pRowsetData)
	{
		memset(m_pRowsetData, 0, (size_t)cbRowSize);
	}
	if (m_pResynchRowsetData)
	{
		memset(m_pResynchRowsetData, 0, (size_t)cbRowSize);
	}
	if (m_pResynchVisibleData)
	{
		memset(m_pResynchVisibleData, 0, (size_t)cbRowSize);
	}
}
//--------------------------------------------------------------------
// @mfunc FreeOutParams
//
// @rdesc TRUE or FALSE
//
void CResynchRefresh::FreeOutParams()
{
	if (m_rghRowsResynched)
	{
		if (m_rghRowsResynched == (HROW*) JUNK_PTR)
			m_rghRowsResynched = NULL;
		else
			PROVIDER_FREE(m_rghRowsResynched);
	}
	if (m_rgRowStatus)
	{
		if (m_rgRowStatus == (DBROWSTATUS*) JUNK_PTR)
			m_rgRowStatus = NULL;
		else
			PROVIDER_FREE(m_rgRowStatus);
	}
}


//--------------------------------------------------------------------
// @mfunc CheckOutParamsAreNulled
//	
// @rdesc TRUE or FALSE
void CResynchRefresh::CheckOutParamsAreNulled()
{	
	//Params should be in this state when a fatal error such as E_FAIL 
	//is returned, or when no rows exist to be resynch'd.
	COMPARE(m_cRowsResynched, 0);
	COMPARE(m_rghRowsResynched, NULL);
	COMPARE(m_rgRowStatus, NULL);
}

//--------------------------------------------------------------------
// @mfunc CompareOutParams 
// Note that rgExpectedhRows is compared with
// m_rghRowsResych'd unless rgExpectedhRows is NULL, and cExpectedRows
// is compared with m_cRowsResynch'd.
//	
// @rdesc TRUE or FALSE

void CResynchRefresh::CompareOutParams(DBCOUNTITEM cExpectedRows, HROW * rgExpectedhRows)
{
	//Make sure the number of rows attempted to be resynched 
	//is correct
	COMPARE(cExpectedRows, m_cRowsResynched);

	//User expects this set of hRows to match the output param prghRowsResynched
	if (rgExpectedhRows)
	{
		while (cExpectedRows)
		{
			//Make our 1-based count a 0-based index into the hrow array
			cExpectedRows--;

			COMPARE(rgExpectedhRows[cExpectedRows], m_rghRowsResynched[cExpectedRows]);
		}
		
	}	
}


//--------------------------------------------------------------------
// @mfunc DetermineProps
//
// @rdesc TRUE or FALSE
//
void CResynchRefresh::DetermineProps()
{
	IDBProperties	*pIDBProp	= NULL;
	IRowsetInfo		*pIRowInfo	= NULL;
	DBPROPIDSET		PropIDSet;
	PROPID			PropID;
	ULONG			cPropSets	= 0;
	DBPROPSET		*rgPropSets = NULL;

	PropID = DBPROP_BYREFACCESSORS;
	PropIDSet.guidPropertySet	= DBPROPSET_DATASOURCEINFO;
	PropIDSet.cPropertyIDs		= 1;
	PropIDSet.rgPropertyIDs		= &PropID;

	
	//Find out if PASS BY REF Accessors are supported		
	if (VerifyInterface(m_pRORowset1->m_pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE,
		(IUnknown **)&pIDBProp))
	{	
		if (CHECK(pIDBProp->GetProperties(1, &PropIDSet, &cPropSets, 
			&rgPropSets), S_OK))
		{
			//We assume there is only one set with one property
			COMPARE(cPropSets, 1);	
			COMPARE(rgPropSets[0].cProperties, 1);	
			if (V_BOOL(&(rgPropSets[0].rgProperties[0].vValue)) == VARIANT_TRUE)				
				m_fPassByRef = TRUE;
			else
			{
				m_fPassByRef = FALSE;
			}
		}
		
		if(rgPropSets)
		{
			if(rgPropSets->rgProperties)
			{	
				PROVIDER_FREE(rgPropSets->rgProperties);	
			}

			PROVIDER_FREE(rgPropSets);
		}
		if (pIDBProp)
		{
			pIDBProp->Release();
			pIDBProp = NULL;
		}
	} 

	//Find out if STRONGIDENTITY is supported		
	if (VerifyInterface(m_pRORowset1->m_pIAccessor, IID_IRowsetInfo, ROWSET_INTERFACE,
		(IUnknown **)&pIRowInfo))
	{	
		PropID = DBPROP_STRONGIDENTITY;
		PropIDSet.guidPropertySet = DBPROPSET_ROWSET;
		PropIDSet.cPropertyIDs = 1;
		PropIDSet.rgPropertyIDs = &PropID;

		if (CHECK(pIRowInfo->GetProperties(1, &PropIDSet, &cPropSets, 
			&rgPropSets), S_OK))
		{
			//We assume there is only one set with one property
			COMPARE(cPropSets, 1);	
			COMPARE(rgPropSets[0].cProperties, 1);	
			if (V_BOOL(&(rgPropSets[0].rgProperties[0].vValue)) == VARIANT_TRUE)				
				m_fStrongIdentity = TRUE;
			else
			{
				m_fStrongIdentity = FALSE;
			}
		}

		if(rgPropSets)
		{
			if(rgPropSets->rgProperties)
			{	
				PROVIDER_FREE(rgPropSets->rgProperties);	
			}

			PROVIDER_FREE(rgPropSets);
		}
		if (pIRowInfo)
		{
			pIRowInfo->Release();
			pIRowInfo = NULL;
		}
	}
}


//--------------------------------------------------------------------
// @mfunc Checks if properties' status are correct for mandatory props
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::DidPropsFail(CTxnRowset * pRowset)
{
	ULONG	i, j;		
	//Look thru every property set
	for (i=0; i<pRowset->m_cPropSets; i++)
	{
		//And every array of properties in each set
		for (j=0; j< pRowset->m_rgPropSets[i].cProperties; j++)				
		{
			//Fail immediately if any required properties aren't set OK,
			if (pRowset->m_rgPropSets[i].rgProperties[j].dwOptions == DBPROPOPTIONS_REQUIRED &&
				pRowset->m_rgPropSets[i].rgProperties[j].dwStatus != DBPROPSTATUS_OK)
			{
				odtLog<<L"Not all requested properties are supported.\n";
				return TRUE;
			}
		}
	}
	
	//All required properties were set OK
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc TestResynchRefresh
//
// @rdesc TRUE or FALSE
//
int CResynchRefresh::TestResynchRefresh(ETESTROWSETTYPE eTestRowsetType)
{
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW		hRow			= DB_NULL_HROW;
	HROW		*phRow			= &hRow;
	BOOL		fResults		= FALSE;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//USE READONLY ROWSET
	if (eTestRowsetType == EREADONLY)
	{
		//Get an hRow	
		if (CHECK(m_hr = m_pROIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
		{	
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER, 1, phRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,TRUE), S_OK))
			{						
				CompareOutParams(1, phRow);						
				FreeOutParams();

				if (CHECK(m_pROIRowset->GetData(hRow, m_hROAccessor, m_pResynchRowsetData), S_OK))
				{
					//Call our methods on it for read only rowset
					if (CHECK(GetLastVisibleData(*phRow, m_hROAccessor, m_pResynchVisibleData,TRUE), S_OK))		
					{
						//this was changed because the test tried to keep track of what 
						//values were in the first row of the rowset.  provider can insert and delete
						//anywhere they want in their cache so it was impossible to know
						//for every provider.  the only way to know what value is there is to
						//read it in, which is the cache buffer
						fResults = COMPARE(CompareBuffer(m_pResynchRowsetData,m_pResynchVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					}
				}
			}
		}
		if (hRow != DB_NULL_HROW)
		{
			m_pROIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
		}
	}
	else
	//USE CHANGEABLE ROWSET
	{				
		//Get an hRow	
		if (CHECK(m_hr = m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
		{	
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER, 1, phRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
			{
				CompareOutParams(1, phRow);						
				FreeOutParams();

				if (CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData), S_OK))
				{
					//Call our methods on it for changeable rowset
					if (CHECK(GetLastVisibleData(*phRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), S_OK))		
					{
						//this was changed because the test tried to keep track of what 
						//values were in the first row of the rowset.  provider can insert and delete
						//anywhere they want in their cache so it was impossible to know
						//for every provider.  the only way to know what value is there is to
						//read it in, which is the cache buffer
						fResults = COMPARE(CompareBuffer(m_pResynchRowsetData,m_pResynchVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					}
				}
			}
		}
		if (hRow != DB_NULL_HROW)
		{
			m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
		}
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc CreateResynchObjects
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::CreateResynchObjects(ETESTINTERFACE	eTI)
{
	IRowsetIdentity *pIRowsetIdentity	= NULL;
	IRowsetLocate	*pIRowsetLocate		= NULL;		


	//Create all objects which deal with rowsets -- anything we
	//will need to recreate if we release the rowset	
	m_hr = m_pChgRowset1->MakeRowset();
	if (m_hr != S_OK)
	{
		//We want to fail the function if some of the properties aren't 
		//supported, but not increment the error count, as this just 
		//indicates optional interfaces weren't supported.
		if (m_hr != DB_S_ERRORSOCCURRED)
		{
			//If it's not DB_S_ERRORSOCCURED, it could be DB_E_ERRORSOCCURRED
			//for drivers which don't support any of the properties
			CHECK(m_hr, DB_E_ERRORSOCCURRED);

			goto FAILROWSET;
		}
	}
	else
	{
		//If properties failed on SetProperty before execute, our return 
		//code won't reflect it, so check array ourselves for any required failures
		if (DidPropsFail(m_pChgRowset1))
			goto FAILROWSET;				
	}
	

	m_hr = m_pRORowset1->MakeRowset();
	if (m_hr != S_OK)
	{
		//We want to fail the function if some of the properties aren't 
		//supported, but not increment the error count, as this just 
		//indicates optional interfaces weren't supported.
		if (m_hr != DB_S_ERRORSOCCURRED)
		{
			//If it's not DB_S_ERRORSOCCURED, it could be DB_E_ERRORSOCCURRED
			//for drivers which don't support any of the properties
			CHECK(m_hr, DB_E_ERRORSOCCURRED);

			goto FAILROWSET;
		}
	}
	else
	{
		//If properties failed on SetProperty before execute, our return 
		//code won't reflect it, so check array ourselves for any required failures		
		if (DidPropsFail(m_pRORowset1))
			goto FAILROWSET;
	}
	
	m_hr = m_pChgRowset2->MakeRowset();
	if (m_hr != S_OK)
	{
		//We want to fail the function if some of the properties aren't 
		//supported, but not increment the error count, as this just 
		//indicates optional interfaces weren't supported.
		if (m_hr != DB_S_ERRORSOCCURRED)
		{
			//If it's not DB_S_ERRORSOCCURED, it could be DB_E_ERRORSOCCURRED
			//for drivers which don't support any of the properties
			CHECK(m_hr, DB_E_ERRORSOCCURRED);

			goto FAILROWSET;
		}
	}
	else
	{
		//If properties failed on SetProperty before execute, our return 
		//code won't reflect it, so check array ourselves for any required failures		
		if (DidPropsFail(m_pChgRowset2))
			goto FAILROWSET;
	}
	
	if (eTI==TI_IRowsetResynch)
	{
		//IRowsetResynch is optional, fail here if it's not supported
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown **)&m_pChgIRowsetResynch))
		{			
			goto FAILROWSET;
		}
		if (!VerifyInterface(m_pRORowset1->m_pIAccessor, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown **)&m_pROIRowsetResynch))
		{
			goto FAILROWSET;
		}
	}
	else
	{
		//IRowsetResynch is optional, fail here if it's not supported
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&m_pChgIRowsetRefresh))
		{			
			goto FAILROWSET;
		}
		if (!VerifyInterface(m_pRORowset1->m_pIAccessor, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&m_pROIRowsetRefresh))
		{
			goto FAILROWSET;
		}
	}
	
	//Get IRowset ptr used for freeing hRows		
	if (VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, (IUnknown **)&m_pChgIRowset))
	{
		if (VerifyInterface(m_pRORowset1->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, (IUnknown **)&m_pROIRowset))
		{
			if (m_fRequestLongDataIfSupported &&
				VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetLocate,
				ROWSET_INTERFACE, (IUnknown **)&pIRowsetLocate))
			{	
				//We only want to bind long data if we know the 
				//table's long data columns will be updated by 
				//all rowsets manipulating them
				m_pChgRowset1->m_fBindLongData = BLOB_LONG;
				m_pChgRowset2->m_fBindLongData = BLOB_LONG;
						
				m_fBindLongData = BLOB_LONG;

				if (pIRowsetLocate)
				{
					pIRowsetLocate->Release();
					pIRowsetLocate = NULL;
				}
			}
			else
			{
				m_fBindLongData = NO_BLOB_COLS;
			}												
					
			//Create accessor for changeable rowset and keep the bindings
			if (CHECK(GetAccessorAndBindings(	m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
												&m_hChgAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  												DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
												UPDATEABLE_COLS_BOUND,FORWARD, NO_COLS_BY_REF, NULL, NULL,
												NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
												NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
												m_fBindLongData),S_OK))									
			{
				//Now create an accesor for the Read Only rowset using the same bindings
				if (CHECK(m_pRORowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
					m_cBindings, m_rgBindings, m_cbRowSize, &m_hROAccessor, NULL), S_OK))
					//Note, this frees them first if they exist and reallocs the correct size
					//to match the accessors we just created
				{
					if (AllocDataBuffers(m_cbRowSize))
					{
						g_ulRowSize=m_cbRowSize;
						return TRUE;	
					}
				}
			}
		}
	}
FAILROWSET:
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc ReleaseResynchObjects
//
// @rdesc TRUE or FALSE
//
void CResynchRefresh::ReleaseResynchObjects()
{
	//Create all objects which deal with rowsets -- anything we
	//will need to recreate if we release the rowset

 	if (m_pChgRowset1)
	{
		if (m_hChgAccessor != DB_NULL_HACCESSOR)
		{
			m_pChgRowset1->m_pIAccessor->ReleaseAccessor(m_hChgAccessor, NULL);
			m_hChgAccessor = DB_NULL_HACCESSOR;
		}
		//m_pChgRowset1->ReleaseRowsetObject();		
		//Release array generated in ExecuteCommand call
		if( m_pChgRowset1->m_rgTableColOrds )
		{
			PROVIDER_FREE(m_pChgRowset1->m_rgTableColOrds);
			m_pChgRowset1->m_rgTableColOrds = NULL;
		}	

		if(m_pChgRowset1->m_pIAccessor)
		{
			m_pChgRowset1->m_pIAccessor->Release();
			m_pChgRowset1->m_pIAccessor = NULL;
		}
	}
	if (m_pRORowset1)
	{
		if (m_hROAccessor != DB_NULL_HACCESSOR)
		{
			m_pRORowset1->m_pIAccessor->ReleaseAccessor(m_hROAccessor, NULL);
			m_hROAccessor = DB_NULL_HACCESSOR;
		}
		//m_pRORowset1->ReleaseRowsetObject();		
		if( m_pRORowset1->m_rgTableColOrds )
		{
			PROVIDER_FREE(m_pRORowset1->m_rgTableColOrds);
			m_pRORowset1->m_rgTableColOrds = NULL;
		}	

		if(m_pRORowset1->m_pIAccessor)
		{
			m_pRORowset1->m_pIAccessor->Release();
			m_pRORowset1->m_pIAccessor = NULL;
		}
	}
	
	if (m_pChgRowset2)
	{
		//m_pChgRowset2->ReleaseRowsetObject();
		if( m_pChgRowset2->m_rgTableColOrds )
		{
			PROVIDER_FREE(m_pChgRowset2->m_rgTableColOrds);
			m_pChgRowset2->m_rgTableColOrds = NULL;
		}	

		if(m_pChgRowset2->m_pIAccessor)
		{
			m_pChgRowset2->m_pIAccessor->Release();
			m_pChgRowset2->m_pIAccessor = NULL;
		}

	}

	if (m_pChgIRowsetResynch)
	{
		m_pChgIRowsetResynch->Release();
		m_pChgIRowsetResynch = NULL;
	}
	if (m_pROIRowsetResynch)
	{
		m_pROIRowsetResynch->Release();
		m_pROIRowsetResynch = NULL;
	}
				
	if (m_pChgIRowsetRefresh)
	{
		m_pChgIRowsetRefresh->Release();
		m_pChgIRowsetRefresh = NULL;
	}
	if (m_pROIRowsetRefresh)
	{
		m_pROIRowsetRefresh->Release();
		m_pROIRowsetRefresh = NULL;
	}

	if (m_pChgIRowset)
	{
		m_pChgIRowset->Release();
		m_pChgIRowset = NULL;
	}

	if (m_pROIRowset)
	{
		m_pROIRowset->Release();
		m_pROIRowset = NULL;
	}
	FreeBuffers(g_ulRowSize);
	FreeAccessorBindings(m_cBindings,m_rgBindings);
	m_rgBindings	= NULL;
	m_cBindings		= 0;
}


//--------------------------------------------------------------------
// @mfunc Sets all CTxnRowset properties, but moves OTHERINSERT
// back to the default if it is currently one of the set properties
// -- this will allow the provider to set OTHERINSERT if it
// is needed for the other requested properties to be met, but not require 
// it to be either on or off.
// We need to move OTHERINSERT to default since we set it explicitly on in the
// CTxnChgRowset::Init code, but for IRowsetResynch on some providers
// this property could conflict with CANHOLDROWS.  So we leave
// it up to the provider whether or not it is set, and then just set
// whatever other properties we need for the particular test we are doing.
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::SetAllProperties(CTxnRowset	*pTxnRowset, 
									   ULONG		cAdditionalProps, 
									   DBPROP		*rgAdditionalProps)
{	
	ULONG		cNewProps			= 0;	
	DBPROP		*rgNewDBProp		= NULL;	
	ULONG		i					= 0;
	ULONG		j					= 0;
	DBPROPSET	NewPropSet;
	ULONG		cOldProps			= 0;
	BOOL		fFoundOtherInsert	= FALSE;
	ULONG		ulNextPropIdx		= 0;

	if (pTxnRowset->m_rgPropSets)
		cOldProps = pTxnRowset->m_rgPropSets[0].cProperties;
									 
	//Find OTHERINSERT and make it the default value if it exists
	for (i=0; i<cOldProps; i++)
	{
		if (pTxnRowset->m_rgPropSets[0].rgProperties[i].dwPropertyID == DBPROP_OTHERINSERT)
		{
			pTxnRowset->m_rgPropSets[0].rgProperties[i].dwOptions	= 0;			
			pTxnRowset->m_rgPropSets[0].rgProperties[i].colid		= DB_NULLID;			
			pTxnRowset->m_rgPropSets[0].rgProperties[i].vValue.vt	= VT_EMPTY;			
			fFoundOtherInsert = TRUE;
			break;
		}
	}
	
	//We want to make sure we don't have more than one property set
	ASSERT(pTxnRowset->m_cPropSets < 2);
	
	//Allocate memory for old props, plus the new ones
	cNewProps	= cOldProps + cAdditionalProps;
	rgNewDBProp = (DBPROP *)PROVIDER_ALLOC((cNewProps+1) * sizeof(DBPROP));	
	if (!rgNewDBProp)
		return FALSE;
		
	//Copy our old properties over
	if (cOldProps)
	{
		memcpy(rgNewDBProp, pTxnRowset->m_rgPropSets[0].rgProperties, (cOldProps * sizeof(DBPROP)));	
	}

	ulNextPropIdx	+= cOldProps;

	//Now copy in the new properties we want to set
	for (i=ulNextPropIdx,j=cAdditionalProps; j>0; j--,i++)
	{
		//Copy to next slot in New array from next property in additional array		
		rgNewDBProp[i].dwPropertyID = rgAdditionalProps[j-1].dwPropertyID;
		rgNewDBProp[i].dwOptions = rgAdditionalProps[j-1].dwOptions;		
		rgNewDBProp[i].colid = rgAdditionalProps[j-1].colid;	
		rgNewDBProp[i].vValue.vt = rgAdditionalProps[j-1].vValue.vt;
		V_BOOL(&(rgNewDBProp[i].vValue)) = V_BOOL(&(rgAdditionalProps[j-1].vValue));
	}
	
	//Fill our single set struct of all new properties
	NewPropSet.rgProperties = rgNewDBProp;
	NewPropSet.cProperties = i;
	NewPropSet.guidPropertySet = DBPROPSET_ROWSET;
	
	//Now set properties including IRowsetResynch for our changeable rowset
	pTxnRowset->SetRowsetProperties(&NewPropSet, 1);		

	//Cleanup
	PROVIDER_FREE(rgNewDBProp);
	rgNewDBProp = NULL;

	return TRUE;
}											

//--------------------------------------------------------------------
// @mfunc Does GetVisibleData and GetData on the cached row.  Then
// does ResynchRows and GetData on the resynch'd row.
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::GetDataBuffers	(	
								CTxnRowset		*pFirstTxnRowset, 
								ISOLEVEL		fIsoLevel, 
								HACCESSOR		hAccessor, 
								HROW			hRow,
								DBBINDING		*rgBindings,
								DBCOUNTITEM		cBindings
								)
{
	BOOL				fResults				= FALSE;
	IRowset				*pIRowset				= NULL;
	IRowsetResynch		*pIRowsetResynch		= NULL;
	IRowsetRefresh		*pIRowsetRefresh		= NULL;
	ULONG				cRowsObtained			= 0;
	IRowsetInfo			*pIRowsetInfo			= NULL;
	DBPROPIDSET			PropIDSet;
	ULONG				cPropSets				= 0;
	DBPROPSET			*rgPropSets				= NULL;
	DBPROPSET			rgRowsetPropSet;
	DBPROPID			rgRowPropIDs[1];
	BOOL				fImmobileRows			= FALSE;
	WORD				i						= 0;
	HROW				hFoundRow				= DB_NULL_HROW;
	HRESULT				hr						= S_OK;
	DBBINDING			*rgBindingsT			= NULL;
	DBCOUNTITEM			cBindingsT				= 0;


	//if binding are NOT passed in used the class's default bindings
	if (cBindings)
	{
		cBindingsT	= cBindings;
		rgBindingsT	= rgBindings;
	}
	else
	{
		cBindingsT	= m_cBindings;
		rgBindingsT	= m_rgBindings;
	}

	//Get IRowset on object
	if (!VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, 
		(IUnknown **)&pIRowset))
		goto CLEANUP;	
				
	//Get IRowsetInfo on object
	if (!VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowsetInfo, ROWSET_INTERFACE, 
		(IUnknown **)&pIRowsetInfo))
		goto CLEANUP;

	//check if rows will be moved when they are changed
	//if this property is TRUE the rows should not be moved when IRowsetChange->SetData is called
	rgRowPropIDs[0] = DBPROP_IMMOBILEROWS;

	PropIDSet.cPropertyIDs		= 1;
	PropIDSet.rgPropertyIDs		= rgRowPropIDs;
	PropIDSet.guidPropertySet	= DBPROPSET_ROWSET;
	//Don't check return code as some or all of these may be unsupported
	pIRowsetInfo->GetProperties(1, &PropIDSet, &cPropSets, &rgPropSets);			
						
	//Find the rowset property set
	for (i=0; i<cPropSets; i++)
	{
		if (rgPropSets[i].guidPropertySet == DBPROPSET_ROWSET)
		{
			//Copy the set we want to check
			memcpy(&rgRowsetPropSet, &rgPropSets[i], sizeof(DBPROPSET));
		}
	}
	if (rgRowsetPropSet.rgProperties[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
	{
		if (V_BOOL(&(rgRowsetPropSet.rgProperties[0].vValue)) == VARIANT_TRUE)
		{
			fImmobileRows = TRUE;
		}
	}

	//Next GetData from rowset cache
	hr=pIRowset->GetData(hRow, hAccessor, m_pRowsetData);
	if (!hr==S_OK)
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
			DWORD	cCount;

			//loop through columns
			for	(cCount=0;cCount < cBindingsT;cCount++)
			{
//				switch (*((BYTE *)dwAddrGet+(rgBindingsT[cCount]).obStatus))
				switch	(STATUS_BINDING(rgBindingsT[cCount],m_pRowsetData))
				{
					case DBSTATUS_S_OK:
					case DBSTATUS_S_ISNULL:
						break;
					case DBSTATUS_S_TRUNCATED:
					case DBSTATUS_E_BADACCESSOR:
					case DBSTATUS_E_CANTCONVERTVALUE:
					case DBSTATUS_E_CANTCREATE:
					case DBSTATUS_E_DATAOVERFLOW:
					case DBSTATUS_E_SIGNMISMATCH:
					case DBSTATUS_E_PERMISSIONDENIED:
					case DBSTATUS_E_INTEGRITYVIOLATION:
					case DBSTATUS_E_SCHEMAVIOLATION:
					case DBSTATUS_E_BADSTATUS:
					case DBSTATUS_S_DEFAULT:
						//some error on some column :)
						goto CLEANUP;
					case DBSTATUS_E_UNAVAILABLE:
						m_rghRowsResynched	= NULL;
						m_rgRowStatus		= NULL;
						fResults			= TRUE;
						g_fBlobFail			= TRUE;
						goto CLEANUP;
					default:
						//error, all possible status are above
						goto CLEANUP;
				}
			}
		}
	}


	//GetVisibleData on our row if test is getting it from non refreshed visual cache
	//if the interface is IRowsetRefresh and the provider implements a visual cache try GetLastVisibleData
	//this visual cache should be the same as GetData
	//(can't have a delete row here since there is a visual cache)
	if (m_eTI!=TI_IRowsetResynch)
	{
		if (!VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowsetRefresh, ROWSET_INTERFACE, 
			(IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;			
		}
		hr	= pIRowsetRefresh->GetLastVisibleData(hRow, hAccessor, m_pVisibleData);
		COMPARE(S_OK,hr);
	}
	else
	{
		if (!VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowsetResynch, ROWSET_INTERFACE, 
			(IUnknown **)&pIRowsetResynch))
		{
			goto CLEANUP;			
		}
		hr	= pIRowsetResynch->GetVisibleData(hRow, hAccessor, m_pVisibleData);
	}

	if (!hr==S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the new row somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_DELETEDROW==hr)
		{
			g_fDeletedRow	= TRUE;
		}
		else
		{
			goto CLEANUP;
		}
	}
	
	//Now do a ResynchRows to see if new data is brought in
	if (m_eTI==TI_IRowsetResynch)
	{
		hr=pIRowsetResynch->ResynchRows(1, &hRow, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus);
	}
	else
	{
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			hr=pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &hRow, TRUE, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus);
		}
		else
		{
			hr=pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &hRow, FALSE, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus);
		}
	}

	if (!hr==S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_ERRORSOCCURRED==hr)
		{
			g_fDeletedRow	= TRUE;
		}
		else
		{
			goto CLEANUP;
		}
	}

	FreeOutParams();

	//Get Newly Resynch'd Data
	hr=pIRowset->GetData(hRow, hAccessor, m_pResynchRowsetData);
	if (!hr==S_OK)
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
			DWORD	cCount;

			//loop through columns
			for	(cCount=0;cCount < cBindingsT;cCount++)
			{
//				switch (*((BYTE *)dwAddrGet+(rgBindingsT[cCount]).obStatus))
				switch	(STATUS_BINDING(rgBindingsT[cCount],m_pResynchRowsetData))
				{
					case DBSTATUS_S_OK:
					case DBSTATUS_S_ISNULL:
						break;
					case DBSTATUS_S_TRUNCATED:
					case DBSTATUS_E_BADACCESSOR:
					case DBSTATUS_E_CANTCONVERTVALUE:
					case DBSTATUS_E_CANTCREATE:
					case DBSTATUS_E_DATAOVERFLOW:
					case DBSTATUS_E_SIGNMISMATCH:
					case DBSTATUS_E_PERMISSIONDENIED:
					case DBSTATUS_E_INTEGRITYVIOLATION:
					case DBSTATUS_E_SCHEMAVIOLATION:
					case DBSTATUS_E_BADSTATUS:
					case DBSTATUS_S_DEFAULT:
						//some error on some column :)
						goto CLEANUP;
					case DBSTATUS_E_UNAVAILABLE:
						m_rghRowsResynched	= NULL;
						m_rgRowStatus		= NULL;
						fResults			= TRUE;
						g_fBlobFail			= TRUE;
						goto CLEANUP;
					default:
						//error, all possible status are above
						goto CLEANUP;
				}
			}
		}
	}
	
	//Now GetVisibleData on our row
	//this now should come after the call to resynch the cache to make sure 
	//that the visible cache has the current data from the back end.
	if (m_eTI==TI_IRowsetResynch)
	{
		hr=pIRowsetResynch->GetVisibleData(hRow, hAccessor, m_pResynchVisibleData);
	}
	else
	{
		hr=pIRowsetRefresh->GetLastVisibleData(hRow, hAccessor, m_pResynchVisibleData);
	}

	if (!hr==S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the new row somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_DELETEDROW==hr)
		{
			g_fDeletedRow	= TRUE;
		}
		else
		{
			goto CLEANUP;
		}
	}

	fResults = TRUE;
CLEANUP:
	if(pIRowset)
	{
		pIRowset->Release();
		pIRowset = NULL;
	}
	if(pIRowsetResynch)
	{
		pIRowsetResynch->Release();
		pIRowsetResynch = NULL;
	}
	if(pIRowsetRefresh)
	{
		pIRowsetRefresh->Release();
		pIRowsetRefresh = NULL;
	}
	if(pIRowsetInfo)
	{
		pIRowsetInfo->Release();
		pIRowsetInfo = NULL;
	}
	PROVIDER_FREE(rgPropSets[0].rgProperties);
	PROVIDER_FREE(rgPropSets);
	return fResults;
}

														   
//--------------------------------------------------------------------
// @mfunc Makes the change on a second txn, then retrieves corresponding
// hRow for that row before the data has been resynch'd.
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::ChangeUnderlyingRowAndGetHrow(	CTxnRowset		*pFirstTxnRowset, 
														ISOLEVEL		fIsoLevel, 
														HROW			*phRow)
{
	HROW	hKeepRow;
	IRowset *pIRowset	= NULL;
	BOOL	fResults	= FALSE;

	//Get IRowset on  the object
	if (!VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, 
		(IUnknown **)&pIRowset))
		goto CLEANUP;	
	//Get row so that consumer has it in cache before we change it from another txn
	if (!COMPARE(pFirstTxnRowset->FindRow(GetNextRowToDelete(), &hKeepRow), TRUE))
	{
//		PrintRowset((IRowset*)pIRowset);
		odtLog << L"No "<<GetNextRowToDelete() << "\n";			
		goto CLEANUP;	
	}
	//Only keep this hRow if caller wants it
	if (phRow)
		*phRow = hKeepRow;
	else
		pIRowset->ReleaseRows(1, &hKeepRow, NULL, NULL, NULL);
	
	//Change row from another txn -- we always use m_pChgRowset2, regardless
	//of which rowset (RO or Chg) from which we call GetVisibleData
 	fResults = Change(m_pChgRowset2);

	//Change may not have succeeded due to txn locking, so we
	//can only expect success if the isolation is low enough	
	//We pass CHAOS when there is no txn
	if (fIsoLevel == ISOLATIONLEVEL_CHAOS ||
		fIsoLevel == ISOLATIONLEVEL_READUNCOMMITTED ||
		fIsoLevel == ISOLATIONLEVEL_READCOMMITTED)		
	{
		COMPARE(fResults, TRUE);
		
		//Sleep for a few seconds; this is to ensure that the back end has had
		//time to make this update visible to other transactions which
		//should see it.  This is only necessary for Access, which only does
		//this every few seconds.
		if (m_fOnAccess)
			Sleep(SLEEP_TIME);	//Takes milliseconds as param
	}
	else
	{		
		if (!fResults)
		{
			odtLog << L"Updating a row failed, possibly due to higher isolation levels locking the row on which the update failed.\n";			
			//g_DeleteIncrement();
			m_fNoChange	= TRUE;
			fResults	= TRUE;
		}
	}
CLEANUP:
	if(pIRowset)
	{
		pIRowset->Release();
		pIRowset = NULL;
	}
	return fResults;
}

//--------------------------------------------------------------------
// @mfunc Calls ChangeUnderlyingRowAndGetHrow and GetDataBuffers
// to generate the scenario we need to call VerifyData
//
// @rdesc TRUE or FALSE
//
BOOL	CResynchRefresh::GenerateResynchData(
										CTxnRowset	*pFirstTxnRowset, 
										ISOLEVEL	fIsoLevel, 
										HACCESSOR	hAccessor,
										DBBINDING	*rgBindings,
										DBCOUNTITEM	cBindings
									)
{
	HROW		hRow		= DB_NULL_HROW;
	HROW		hRowChanged	= DB_NULL_HROW;
	IRowset		*pIRowset	= NULL;
	BOOL		fResults	= FALSE;

	if (VerifyInterface(pFirstTxnRowset->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
		(IUnknown **)&pIRowset))
	{
		//Make a change via another txn and get the corresponding hRow on first txn
		if (ChangeUnderlyingRowAndGetHrow(pFirstTxnRowset, fIsoLevel, &hRow) != NOERROR)		
		{
			//Fill all data buffers using Resynch methods
			if (GetDataBuffers(pFirstTxnRowset, fIsoLevel, hAccessor, hRow, rgBindings, cBindings))
			{
				fResults = TRUE;
			}
		}
	}

	//Release hRow 
	if (pIRowset)
	{	
		if (hRow != DB_NULL_HROW)
			CHECK(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
		pIRowset->Release();
		pIRowset = NULL;
	}

	return fResults;
}
		
														   
//--------------------------------------------------------------------
// @mfunc Verifies the correct contents of the visible and cached data
// buffers, based on the fSeeNewVisibleData flag. 
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::VerifyData(	EVERIFY		fSeeVisibleResynchedData,	
									EVERIFY		fSeeVisibleData,
									EVERIFY		fSeeRowsetData,				//= VERIFY_OLD
									EVERIFY		fSeeRowsetResynchedData,	//= VERIFY_NEW
									ULONG		ulOldRowNum,				//= 0
									ULONG		ulNewRowNum,				//= 0
									DBCOUNTITEM	cVerifyBindings,			//= 0
									DBBINDING	*rgVerifyBindings)			//= NULL
{
	BOOL		fResults	= FALSE;	
	DBCOUNTITEM	cBindings	= cVerifyBindings;
	DBBINDING	*rgBindings	= rgVerifyBindings;

	if (g_fDeletedRow || g_fBlobFail)
	{
		//if this is true then the provider inserted/deleted a row instead of changing it
		//if this is done the first rowset has no knoweldge of the new row, it can't see it nor get it
		//so it can't compare against either
		//or
		//a BLOB column was in a row that was inserted and some providers have a hard time handling
		//BLOBs so they don't
		g_fDeletedRow	= FALSE;
		g_fBlobFail		= FALSE;
		return TRUE;
	}

	//If we haven't been passed any bindings, use the defaults
	if (!cBindings)
	{
		cBindings	= m_cBindings;
		rgBindings	= m_rgBindings;
	}
	
	//If ulOldRowNum is zero, we use the currently changed row num
	if (!ulOldRowNum)
	{
		ulOldRowNum = g_ulLastActualDelete;
	}
	//if the second Txn did not change anything
	if (m_fNoChange)
	{
		ulOldRowNum = GetNextRowToDelete();
		m_fNoChange = FALSE;
	}
	//Same for ulNewRowNum
	if (!ulNewRowNum)
	{
		ulNewRowNum = g_ulLastActualInsert;
	}

	//fSeeRowsetResynchedData - GetData after call to Resynch
	if (fSeeRowsetResynchedData == VERIFY_NEW)
	{
		if (m_eTI==TI_IRowsetRefreshFALSE)
		{
			//resynch'd data should always be the old values for RefreshFALSE, even 
			//if fSeeRowsetResynchedData says to expect new values
			//there is no visual cache this will still be the old vaules since the resynch calls
			//should have been a no-op
			if (!COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, m_pResynchRowsetData, 
				cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE))
			{
				goto CLEANUP;
			}
		}
		else
		{
			//resynch'd data should always be the new values for Resynch and RefreshTRUE
			if (!COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulNewRowNum, m_pResynchRowsetData, 
				cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE))
			{
				goto CLEANUP;
			}
		}
	}
	else
	{
		if (fSeeRowsetResynchedData == VERIFY_OLD)
		{
			//Unless we haven't changed anything in the second txn
			if (!(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, m_pResynchRowsetData, 
				cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE))
				goto CLEANUP;
		}
	}

	//fSeeRowsetData - GetData before any resynch calls
	if (fSeeRowsetData == VERIFY_NEW)
	{
		//If the hRow is a newly inserted row we'll see new values,
		fResults	= COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
			m_pChgRowset1->m_rgTableColOrds, ulNewRowNum, m_pRowsetData, 
			cBindings, rgBindings, m_pChgRowset1->m_pTable, 
			m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);		
	}
	else
	{	
		if (fSeeRowsetData == VERIFY_OLD)
		{
			//otherwise the cache should always be old values that haven't been updated yet
			 if(!CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, m_pRowsetData, 
				cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY))
				odtLog<<"Data mismatched due to wrong comparison in miscfunc\n";
			 fResults	= TRUE;

		}
	}

	//fSeeVisibleData - GetVisibleData before any resynch calls
	if (m_eTI==TI_IRowsetResynch || !g_fVisualCache)
	{
		//this will always go to the back end since it was 'resynch' and not 'refresh'
		//or because there is not a visual cache so the back end then becomes the only
		//place GetVisibleData can get its data
		if (fSeeVisibleData == VERIFY_NEW)
		{
			//Our visible data after resynch should be the new values if the second txn has changed the row
			//fOverwrtie shouldn't matter with this buffer.  this one gets overwritten reguardless.
			fResults	= COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulNewRowNum, 
				m_pVisibleData, cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);				
		}		
		else
		{
			if (fSeeVisibleData == VERIFY_OLD)
			{
				//but if the txn hasn't changed anything, we'll see ole values here
				if(!CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
					m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, 
					m_pVisibleData, cBindings, rgBindings, m_pChgRowset1->m_pTable, 
					m_pIMalloc, PRIMARY, COMPARE_ONLY))
					odtLog<<"Data mismatched due to wrong comparison in miscfunc\n";
				fResults = TRUE;
			}	
		}
	}
	else
	{
		if (fSeeVisibleData != VERIFY_IGNORE)
		{
			//this is 'refresh' from a visual cache pre calls to resynch (just like GetData)
			if (!CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
					m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, m_pVisibleData, 
					cBindings, rgBindings, m_pChgRowset1->m_pTable, 
					m_pIMalloc, PRIMARY, COMPARE_ONLY))
					odtLog<<"Data mismatched due to wrong comparison in miscfunc\n";
				fResults = TRUE;
		}
	}

	//fSeeVisibleResynchedData - GetVisibleData after resynch calls
	//this should be the same no matter if there is a visual cache
	if (fSeeVisibleResynchedData == VERIFY_NEW)
	{
		//Our visible data after resynch should be the new values if the second txn has changed the row
		//fOverwrtie shouldn't matter with this buffer.  this one gets overwritten reguardless.
		fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
			m_pChgRowset1->m_rgTableColOrds, ulNewRowNum, 
			m_pResynchVisibleData, cBindings, rgBindings, m_pChgRowset1->m_pTable, 
			m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);				
	}		
	else
	{
		if (fSeeVisibleResynchedData == VERIFY_OLD)
		{
			//but if the txn hasn't changed anything, we'll see ole values here
			if (!CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
				m_pChgRowset1->m_rgTableColOrds, ulOldRowNum, 
				m_pResynchVisibleData, cBindings, rgBindings, m_pChgRowset1->m_pTable, 
				m_pIMalloc, PRIMARY, COMPARE_ONLY))		
				odtLog<<"Data mismatched due to wrong comparison in miscfunc\n";
			fResults = TRUE;
		}	
	}
	
	//If we were supposed to ignore everything, return true
	if (fSeeVisibleResynchedData == VERIFY_IGNORE &&
		fSeeVisibleData			 == VERIFY_IGNORE &&
		fSeeRowsetData			 == VERIFY_IGNORE &&
		fSeeRowsetResynchedData	 == VERIFY_IGNORE)
	{
		fResults = TRUE;
	}
CLEANUP:
	return fResults;
}

//--------------------------------------------------------------------
// @mfunc Starts both Chg1 and RO1 rowsets in specificied transaction 
// isolation level
//
// @rdesc TRUE or FALSE
//
HRESULT CResynchRefresh::StartTxns( ISOLEVEL fIsoLevel)
{

	if (SUCCEEDED(m_hr	= m_pChgRowset1->m_pITxnLocal->StartTransaction(fIsoLevel, 0, NULL, NULL)))
	{
		m_hr = m_pRORowset1->m_pITxnLocal->StartTransaction(fIsoLevel, 0, NULL, NULL);
	}

	return ResultFromScode(m_hr);
}
	

//--------------------------------------------------------------------
// @mfunc Ends both Chg1 and RO1 rowsets' transactions by doing a non 
// retaining commit
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::EndTxns(BOOL fAbort)
{
	if (fAbort)
	{
		//End Transactions without starting new ones
		m_pChgRowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE);
		m_pRORowset1->m_pITxnLocal->Abort(NULL, FALSE, FALSE);
	}
	else
	{
		//End Transactions without starting new ones
		m_pChgRowset1->m_pITxnLocal->Commit(FALSE, 0, 0);
		m_pRORowset1->m_pITxnLocal->Commit(FALSE, 0, 0);
	}
	//Clean up rowsets in case they were zombied above
	//do this before checking the isoloeves below because
	//some providers might not allow starting a txn if a cursor is open
	ReleaseResynchObjects();	
	
	//Make sure our isolation level is set for autocommit so we can do 
	//things like create updateable rowsets on SQL Server -- something
	//prohibited if we are in Read Uncommitted. This is also here just
	//to verify that we can set this property.
	CHECK(m_pChgRowset1->SetAutoCommitIsoLevel(ISOLATIONLEVEL_READCOMMITTED), S_OK);
	CHECK(m_pRORowset1->SetAutoCommitIsoLevel(ISOLATIONLEVEL_READCOMMITTED), S_OK);
		
	//Generate new stuff for testcase again
	return COMPARE(CreateResynchObjects(m_eTI), TRUE);
}


//--------------------------------------------------------------------
// @mfunc Turns off CANHOLDROWS, and adds the properties specified for
// m_pChgRowset1, m_pChgRowset2 and m_pRORowset1.  Not that cRemoveProps must
// specify the number of elements in the rgRemoveProps and rgPropOptions 
// arrays.
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::ChangeProperties(	ULONG			cAddProps, 
										DBPROP			*rgAddProps,
										ULONG			cRemoveProps, 
										DBPROPID		*rgRemoveProps,
										DBPROPOPTIONS	*rgPropOptions,
										BLOBTYPE		fBindLongData)
{
	ULONG	i;
	ULONG	j;

	//Make sure we only have one property set, we'll assume that from now on
	ASSERT(m_pChgRowset1->m_cPropSets);
	ASSERT(m_pChgRowset1->m_rgPropSets);	

	/////////////////////////////////////////////////////////////
	//Now undo some of the stuff we don't want for this test case
	//but that CResynchRefresh::Init does
	/////////////////////////////////////////////////////////////
	
	//Release accessors
	ReleaseResynchObjects();
	
	//Look in all properties of our first (and only)
	//set for the ones we want to remove
	for (i=0; i<m_pChgRowset1->m_rgPropSets[0].cProperties; i++)
	{	
		//Look for each of the properties to be removed
		for (j=0; j<cRemoveProps; j++)
		{								
			if (m_pChgRowset1->m_rgPropSets[0].rgProperties[i].dwPropertyID == rgRemoveProps[j])
			{
				//Set Property to false to turn it off
				V_BOOL(&(m_pChgRowset1->m_rgPropSets[0].rgProperties[i].vValue)) = VARIANT_FALSE;
				//Set option as requested by caller for this property
				m_pChgRowset1->m_rgPropSets[0].rgProperties[i].dwOptions = rgPropOptions[j];
			}
		}
	}

	//Look in all properties of our first (and only)
	//set for the ones we want to remove
	for (i=0; i<m_pChgRowset2->m_rgPropSets[0].cProperties; i++)
	{	
		//Look for each of the properties to be removed
		for (j=0; j<cRemoveProps; j++)
		{								
			if (m_pChgRowset2->m_rgPropSets[0].rgProperties[i].dwPropertyID == rgRemoveProps[j])
			{
				//Set Property to false to turn it off
				V_BOOL(&(m_pChgRowset2->m_rgPropSets[0].rgProperties[i].vValue)) = VARIANT_FALSE;
				//Set option as requested by caller for this property
				m_pChgRowset2->m_rgPropSets[0].rgProperties[i].dwOptions = rgPropOptions[j];
			}
		}
	}

	//and for read only rowset, too		
	for (i=0; i<m_pRORowset1->m_rgPropSets[0].cProperties; i++)
	{	
		//Look for each of the properties to be removed
		for (j=0; j<cRemoveProps; j++)
		{								
			if (m_pRORowset1->m_rgPropSets[0].rgProperties[i].dwPropertyID == rgRemoveProps[j])
			{
				//Set Property to false to turn it off
				V_BOOL(&(m_pRORowset1->m_rgPropSets[0].rgProperties[i].vValue)) = VARIANT_FALSE;
				//Set option as requested by caller for this property
				m_pRORowset1->m_rgPropSets[0].rgProperties[i].dwOptions = rgPropOptions[j];
			}
		}
	}
	
	//Set all original properties, plus those passed to us
	if (SetAllProperties(m_pChgRowset1, cAddProps, rgAddProps))		
	{
		if (SetAllProperties(m_pChgRowset2, cAddProps, rgAddProps))
		{
			if (SetAllProperties(m_pRORowset1, cAddProps, rgAddProps))			
			{			
				//set the flags for the objects so the accessors 
				//are created with the right columsn bound
				m_pChgRowset1->m_fBindLongData = fBindLongData;
				m_pChgRowset2->m_fBindLongData = fBindLongData;
				

				//Now recreate the rowsets with the change in properties in effect													
				if (CreateResynchObjects(m_eTI))
					return TRUE;
			}
		}
	}
	return FALSE;
}


HRESULT	CResynchRefresh::ResynchRefresh(	HCHAPTER        hChapter,
											DBCOUNTITEM          cRows,
											const HROW      rghRows[],
											BOOL            fOverwrite,
											DBCOUNTITEM		*pcRowsRefreshed,
											HROW			**prghRowsRefreshed,
											DBROWSTATUS		**prgRowStatus,
											BOOL			fRO)
{
	if (TI_IRowsetResynch==m_eTI)
	{
		if (fRO)
		{
			return m_pROIRowsetResynch->ResynchRows(	cRows,
														rghRows,
														pcRowsRefreshed,
														prghRowsRefreshed,
														prgRowStatus);
		}
		else
		{
			return m_pChgIRowsetResynch->ResynchRows(	cRows,
														rghRows,
														pcRowsRefreshed,
														prghRowsRefreshed,
														prgRowStatus);
		}
	}
	else
	{
		if(fRO)
		{
			return m_pROIRowsetRefresh->RefreshVisibleData(		hChapter,
																cRows,
																rghRows,
																fOverwrite,
																pcRowsRefreshed,
																prghRowsRefreshed,
																prgRowStatus);
		}
		else
		{
			return m_pChgIRowsetRefresh->RefreshVisibleData(	hChapter,
																cRows,
																rghRows,
																fOverwrite,
																pcRowsRefreshed,
																prghRowsRefreshed,
																prgRowStatus);
		}
	}
	//should never get here
	return E_FAIL;;
}

	
HRESULT	CResynchRefresh::GetLastVisibleData(	HROW		hRow,
												HACCESSOR	hAccessor,
												void		*pData,
												BOOL		fRO)
{
	if (TI_IRowsetResynch==m_eTI)
	{
		if (fRO)
		{
			return m_pROIRowsetResynch->GetVisibleData(hRow,hAccessor,pData);
		}
		else
		{
			return m_pChgIRowsetResynch->GetVisibleData(hRow,hAccessor,pData);
		}
	}
	else
	{
		if (fRO)
		{
			return m_pROIRowsetRefresh->GetLastVisibleData(hRow,hAccessor,pData);
		}
		else
		{
			return m_pChgIRowsetRefresh->GetLastVisibleData(hRow,hAccessor,pData);
		}
	}
	//should never b=get here
	return E_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Terminate
//
// @rdesc TRUE or FALSE
//
BOOL CResynchRefresh::Terminate()
{
	if (m_pVisibleData)
	{
		PROVIDER_FREE(m_pVisibleData);
		m_pVisibleData = NULL;
	}
	
	if (m_pRowsetData)
	{
		PROVIDER_FREE(m_pRowsetData);
		m_pRowsetData = NULL;
	}
	
	if (m_pResynchRowsetData)
	{
		PROVIDER_FREE(m_pResynchRowsetData);
		m_pResynchRowsetData = NULL;
	}
	
	if (m_pResynchVisibleData)
	{
		PROVIDER_FREE(m_pResynchVisibleData);
		m_pResynchVisibleData = NULL;
	}
	
	ReleaseResynchObjects();
	
	//Release the bindings
	if (m_rgBindings)
	{
//		PROVIDER_FREE(m_rgBindings);
//		m_rgBindings = NULL;
	}

	return CTxnImmed::Terminate();
}


//////////////////////////////////////////////////////////////
//Base class for TCPropCanHoldRowsResynch and TCPropCanHoldRowsResynchBLOBS
//////////////////////////////////////////////////////////////
class CPropCanHoldRowsResynch : public CResynchRefresh
{
public:
	//@cmember CTOR
	CPropCanHoldRowsResynch(LPWSTR tcName) : CResynchRefresh(tcName){};

	
	//@cmember Does the Fetch Position testing for different rowsets
	int TestFetchPosition(	IRowset			*pIRowset, 
							IRowsetResynch	*pIRowsetResynch,
							IRowsetRefresh	*pIRowsetRefesh);

	//@cmember Tests Resynch/Refresh on first and last row for different rowsets
	int TestRows1AndN(	IRowset			*pIRowset, 
						IRowsetResynch	*pIRowsetResynch,
						IRowsetRefresh	*pIRowsetRefesh);

	//@cmember Tests Resynch/Refresh on All Rows for different rowsets
	int TestAllRows(	IRowset			*pIRowset, 
						IRowsetResynch	*pIRowsetResynch,
						IRowsetRefresh	*pIRowsetRefesh);
};

//--------------------------------------------------------------------
// @mfunc Does the Fetch Position testing for different rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CPropCanHoldRowsResynch::TestFetchPosition(	IRowset			*pIRowset, 
												IRowsetResynch	*pIRowsetResynch,
												IRowsetRefresh	*pIRowsetRefresh)
{
	const WORD		wRow			= 3;
	HROW			rghRow[wRow]	= {DB_NULL_HROW, DB_NULL_HROW, DB_NULL_HROW};
	HROW			hRow2			= DB_NULL_HROW;
	HROW			*phRow			= &rghRow[0];
	HROW			*phRow2			= &hRow2;	
	DBCOUNTITEM		cRowsObtained	= 0;
	BOOL			fResults		= FALSE;
	BYTE			*pFirstRowData	= NULL;
	BYTE			*pwRowthRowData	= NULL;
	DBBYTEOFFSET	cbSkip			= 0;

	//Determine if we have a bookmark bound, in which case we need to 
	//skip the bookmark column when we memcmp our buffers, since 
	//the bookmark may change
	if (IsRowsetPropSupported(pIRowset, DBPROP_BOOKMARKS) == DBPROPSTATUS_OK)
	{
		//Skip to next column bound -- this assumes status is the first
		//element of the data struct
		ASSERT(offsetof(DATA, sStatus) == 0);
		cbSkip = m_rgBindings[1].obStatus;
	}

	pFirstRowData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pFirstRowData)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}
	pwRowthRowData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pwRowthRowData)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}
	
	memset(m_pRowsetData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchVisibleData, 0, (size_t)m_cbRowSize);
	memset(pwRowthRowData, 0, (size_t)m_cbRowSize);
	memset(pwRowthRowData, 0, (size_t)m_cbRowSize);
	
	//Make sure we are at beginning of rowset after other variations
	TESTC_(pIRowset->RestartPosition(NULL), S_OK);

	//This will get us wRow rows
	TESTC_(pIRowset->GetNextRows(NULL, 0, wRow, &cRowsObtained, &phRow), S_OK);

	//Now GetData on the 1st and the wRowth row so we know what it is
	TESTC_(pIRowset->GetData(phRow[0], m_hChgAccessor, pFirstRowData), S_OK);
	TESTC_(pIRowset->GetData(phRow[wRow-1], m_hChgAccessor, pwRowthRowData), S_OK);

	//Now Release all the rows and start at the beginning again
	TESTC_(pIRowset->ReleaseRows(wRow, rghRow, NULL, NULL, NULL), S_OK);
	TESTC_(pIRowset->RestartPosition(NULL), S_OK);
		
	//This will get us wRow - 1 rows
	TESTC_(m_hr = pIRowset->GetNextRows(NULL, 0, (wRow-1), &cRowsObtained, &phRow), S_OK);

	//Resynch the first row
	if (m_eTI==TI_IRowsetResynch)
	{
		TESTC_(pIRowsetResynch->ResynchRows(1, &rghRow[0], &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus),S_OK);
	}
	else
	{
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &rghRow[0], TRUE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus),S_OK);
		}
		else
		{
			TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &rghRow[0], FALSE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus),S_OK);
		}
	}

	//Get visible data on first row
	if (m_eTI==TI_IRowsetResynch)
	{
		TESTC_(pIRowsetResynch->GetVisibleData(rghRow[0], m_hChgAccessor,m_pResynchVisibleData),S_OK);
	}
	else
	{
		TESTC_(pIRowsetRefresh->GetLastVisibleData(rghRow[0], m_hChgAccessor,m_pResynchVisibleData),S_OK);
	}

	CompareOutParams(1, &rghRow[0]);

	if (g_fNOCHANGE)
	{
		//no change was made here, if the provider can handle it, expect it
		COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
	}
	else
	{
		COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
	}

	//no change is made, these should be the same
	if(CompareBuffer(pFirstRowData,m_pResynchVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY)!=TRUE)
	{
		goto CLEANUP;
	}

	//Move to next row to verify that all of this did not 
	//screw up the fetch position, we should get the third row here
	TESTC_(m_hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow2), S_OK);
									
	//Now GetData
	TESTC_(pIRowset->GetData(hRow2, m_hChgAccessor, m_pRowsetData), S_OK);
	{	
		if(CompareBuffer(m_pRowsetData,pwRowthRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fResults	= TRUE;
		}
	}
CLEANUP:
	FreeOutParams();

	if (rghRow[0] != DB_NULL_HROW)
	{
		pIRowset->ReleaseRows((wRow-1), rghRow, NULL, NULL, NULL);		
	}
	if (hRow2 != DB_NULL_HROW)
	{
		pIRowset->ReleaseRows(1, &hRow2, NULL, NULL, NULL);		
	}
	PROVIDER_FREE(pFirstRowData);
	PROVIDER_FREE(pwRowthRowData);

	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}


//--------------------------------------------------------------------
// @mfunc Tests Resynch on first and last row for different rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CPropCanHoldRowsResynch::TestRows1AndN(	IRowset			*pIRowset, 
											IRowsetResynch	*pIRowsetResynch,
											IRowsetRefresh	*pIRowsetRefresh)
{
	const ULONG	ulMaxRows				= NUM_ROWS * 3;
	DBCOUNTITEM	cRowsObtained			= 0;
	DBCOUNTITEM	cFirstRowObtained		= 0;
	BOOL		fResults				= FALSE;
	HROW		hFirstRow				= DB_NULL_HROW;
	HROW		*phFirstRow				= &hFirstRow;
	HROW		rghRows[ulMaxRows];
	HROW		*phRows					= &rghRows[0];
	BYTE		*pResynchVisibleData2	= NULL;
	BYTE		*pRowsetData2			= NULL;
	BYTE		*pResynchRowsetData2	= NULL;
	BYTE		*p1stRowData			= NULL;
	BYTE		*pNthRowData			= NULL;
	DBBYTEOFFSET cbSkip					= 0;
	ULONG		*rgRefCounts			= NULL;
	ULONG		cFirstRowRefCount		= 1;	//Set it to non zero so we know it is changed by ReleaseRows
	ULONG		i						= 0;

	//Determine if we have a bookmark bound, in which case we need to 
	//skip the bookmark column when we memcmp our buffers, since 
	//the bookmark may change
	if (IsRowsetPropSupported(pIRowset, DBPROP_BOOKMARKS) == DBPROPSTATUS_OK)
	{
		//Skip to next column bound -- this assumes status is the first
		//element of the data struct
		ASSERT(offsetof(DATA, sStatus) == 0);
		cbSkip = m_rgBindings[1].obStatus;
	}

	p1stRowData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!p1stRowData)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}
	pNthRowData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pNthRowData)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}
	pResynchVisibleData2 = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pResynchVisibleData2)
		return TEST_FAIL;

	pRowsetData2 = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pRowsetData2)
		return TEST_FAIL;

	pResynchRowsetData2 = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pResynchRowsetData2)
		return TEST_FAIL;

	memset(m_pRowsetData, 0, (size_t)m_cbRowSize);
	memset(m_pVisibleData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchRowsetData, 0, (size_t)m_cbRowSize);
	
	memset(pRowsetData2, 0, (size_t)m_cbRowSize);
	memset(pResynchVisibleData2, 0, (size_t)m_cbRowSize);
	memset(pResynchRowsetData2, 0, (size_t)m_cbRowSize);
	
	memset(p1stRowData, 0, (size_t)m_cbRowSize);
	memset(pNthRowData, 0, (size_t)m_cbRowSize);

	//Make sure we are at beginning of rowset after other variations
	pIRowset->RestartPosition(NULL);

	//Get all the rows
	m_hr = pIRowset->GetNextRows(NULL, 0, ulMaxRows, &cRowsObtained, &phRows);

	if(!SUCCEEDED(m_hr))
	{
		goto CLEANUP;
	}
		
	//Now GetData on the first row so we know what it is
	TESTC_(pIRowset->GetData(phRows[0], m_hChgAccessor, p1stRowData), S_OK);
			
	//Now GetData on the last row so we know what it is
	TESTC_(pIRowset->GetData(phRows[cRowsObtained-1], m_hChgAccessor, pNthRowData), S_OK);
				
	//Now Release all the rows 
	TESTC_(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);						
		
	//Make sure we are at beginning of rowset again
	TESTC_(pIRowset->RestartPosition(NULL), S_OK);
						
	//Get first row, this one will induce the CANHOLDROWS and force a refetch
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cFirstRowObtained, &phFirstRow), S_OK);
							
	//Get all remaining possible rows (use NUM_ROWS * 3 so we're sure to get all of them)
	TESTC_(pIRowset->GetNextRows(NULL, 0, ulMaxRows, &cRowsObtained, &phRows), DB_S_ENDOFROWSET);
								
	/////////////////////////////////
	//Now get all data on first row and last hRow
	/////////////////////////////////
							
	//Get rowset data
	TESTC_(pIRowset->GetData(hFirstRow, m_hChgAccessor, m_pRowsetData), S_OK);
	TESTC_(pIRowset->GetData(rghRows[cRowsObtained-1], m_hChgAccessor, pRowsetData2), S_OK);
									
	//Try to get new visible data
	if (m_eTI==TI_IRowsetResynch)
	{
		TESTC_(pIRowsetResynch->GetVisibleData(hFirstRow, m_hChgAccessor,m_pResynchVisibleData), S_OK);
		TESTC_(pIRowsetResynch->GetVisibleData(rghRows[cRowsObtained-1], m_hChgAccessor,pResynchVisibleData2), S_OK);

		//Resynch the rows, using count of rows of 0
		TESTC_(pIRowsetResynch->ResynchRows(0, NULL,&m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
	}
	else
	{
		//Resynch the rows, using count of rows of 0
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,0, NULL,TRUE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
		}
		else
		{
			TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,0, NULL,FALSE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
		}
		TESTC_(pIRowsetRefresh->GetLastVisibleData(hFirstRow, m_hChgAccessor,m_pResynchVisibleData), S_OK);
		TESTC_(pIRowsetRefresh->GetLastVisibleData(rghRows[cRowsObtained-1], m_hChgAccessor,pResynchVisibleData2), S_OK);
	}
											
	//Check by hand instead of calling CompareOutParams() since our hRows
	//are not in a contiguous array as expected by this routine																																
	COMPARE(cRowsObtained+cFirstRowObtained, m_cRowsResynched);
	
	//Check first row
	COMPARE(m_rghRowsResynched[0], hFirstRow);
	
	//Check second thru last rows. Since we start with 1 due to the first row
	//being checked already at index 0 in m_rghRowsResynched, we can go through 
	//i=cRowsObtained
	for (i=1; i<=cRowsObtained; i++)
	{
		COMPARE(rghRows[i-1], m_rghRowsResynched[i]);
	}
	
	//Release these rows, we have ref counts on them already												
	TESTC_(pIRowset->ReleaseRows(m_cRowsResynched, m_rghRowsResynched, NULL, NULL, NULL), S_OK);
	
	while (m_cRowsResynched)
	{
		//Make our 1-based count a 0-based array index	
		m_cRowsResynched--;					

		if (g_fNOCHANGE)
		{
			//no change was made here, if the provider can handle it, expect it
			COMPARE(m_rgRowStatus[m_cRowsResynched], DBROWSTATUS_S_NOCHANGE);
		}
		else
		{
			COMPARE(m_rgRowStatus[m_cRowsResynched], DBROWSTATUS_S_OK);
		}
	}
	
	//Get resynch'd data	
	TESTC_(pIRowset->GetData(hFirstRow, m_hChgAccessor, m_pResynchRowsetData), S_OK);
	TESTC_(pIRowset->GetData(rghRows[cRowsObtained-1], m_hChgAccessor, pResynchRowsetData2), S_OK);
		
	//All data from our first row should still be old values	
	//since we didn't change anything underneath (fOvwewrite doesn't matter).  We use	
	//cbSkip to adjust if we have bookmark data we need to ignore.
	fResults =	COMPARE(CompareBuffer(m_pResynchVisibleData,p1stRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(m_pRowsetData,p1stRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(m_pResynchRowsetData,p1stRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

	//The same with data from our nth row
	fResults &=	COMPARE(CompareBuffer(pResynchVisibleData2,pNthRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(pRowsetData2,pNthRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(pResynchRowsetData2,pNthRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);


	rgRefCounts = (ULONG *)PROVIDER_ALLOC(cRowsObtained * sizeof(ULONG));

	if (!rgRefCounts)	
	{
		TESTC_(E_OUTOFMEMORY, S_OK);	//Record that wblobse ran out of memory	
	}
	else
	{
		//Verify the ref counts are correct for the second through last rows			
		TESTC_(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, rgRefCounts, NULL), S_OK);
		
		while (cRowsObtained)
		{
			cRowsObtained--;	//Make the 1 based count a zero based index
			
			COMPARE(rgRefCounts[cRowsObtained], 0);	
		}
	}
		
	//And make sure we release the first row and check the ref count
	TESTC_(pIRowset->ReleaseRows(1, phFirstRow, NULL, &cFirstRowRefCount, NULL), S_OK);										
		
	COMPARE(cFirstRowRefCount, 0);
CLEANUP:
	FreeOutParams();

	if (rgRefCounts)
		PROVIDER_FREE(rgRefCounts);
	
	if (pResynchVisibleData2)
		PROVIDER_FREE(pResynchVisibleData2);

	if (pRowsetData2)
		PROVIDER_FREE(pRowsetData2);

	if (pResynchRowsetData2)
		PROVIDER_FREE(pResynchRowsetData2);

	if (p1stRowData)
		PROVIDER_FREE(p1stRowData);

	if (pNthRowData)
		PROVIDER_FREE(pNthRowData);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Tests Resynch on All rows for different rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CPropCanHoldRowsResynch::TestAllRows(	IRowset			*pIRowset, 
											IRowsetResynch	*pIRowsetResynch,
											IRowsetRefresh	*pIRowsetRefresh)
{
	const ULONG	ulMaxRows			= NUM_ROWS * 3;
	HROW		rghRows[ulMaxRows];
	HROW		*phRow				= &rghRows[0];
	DBCOUNTITEM	cRowsObtained		= 0;
	BOOL		fResults			= FALSE;		
	ULONG		i					= 0;
	DBCOUNTITEM	ulArbitraryRowIdx	= 0;
	BYTE		*pArbRowData		= NULL;
	DBBYTEOFFSET cbSkip				= 0;

	//Determine if we have a bookmark bound, in which case we need to 
	//skip the bookmark column when we memcmp our buffers, since 
	//the bookmark may changem
	if (IsRowsetPropSupported(m_pChgIRowset, DBPROP_BOOKMARKS) == DBPROPSTATUS_OK)
	{
		//Skip to next column bound -- this assumes status is the first
		//element of the data struct
		ASSERT(offsetof(DATA, sStatus) == 0);
		cbSkip = m_rgBindings[1].obStatus;
	}

	pArbRowData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pArbRowData)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	memset(pArbRowData, 0, (size_t)m_cbRowSize);
	memset(m_pRowsetData, 0, (size_t)m_cbRowSize);
	memset(m_pVisibleData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchRowsetData, 0, (size_t)m_cbRowSize);

	//Make sure we are at beginning of rowset after other variations
	TESTC_(m_pChgIRowset->RestartPosition(NULL), S_OK);

	//Now get all the rows, ask for more than enough
	TESTC_(m_pChgIRowset->GetNextRows(NULL, 0, ulMaxRows, &cRowsObtained, &phRow), DB_S_ENDOFROWSET);
		
	/////////////////////////////////////////////////////////////
	//Fet all data on one arbitrary row
	/////////////////////////////////////////////////////////////
	ASSERT(cRowsObtained > 1);
	ulArbitraryRowIdx = cRowsObtained - 2;
			
	//Get data for verification use later
	TESTC_(m_pChgIRowset->GetData(phRow[ulArbitraryRowIdx], m_hChgAccessor, pArbRowData), S_OK);
			
	TESTC_(m_pChgIRowset->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL), S_OK);

	//Make sure we are at beginning of rowset 
	TESTC_(m_pChgIRowset->RestartPosition(NULL), S_OK);

	//Now get all the rows, ask for more than enough
	TESTC_(m_pChgIRowset->GetNextRows(NULL, 0, ulMaxRows, &cRowsObtained, &phRow), DB_S_ENDOFROWSET);

	//Get cached data
	TESTC_(m_pChgIRowset->GetData(phRow[ulArbitraryRowIdx], m_hChgAccessor, m_pRowsetData), S_OK);

	//Try to get new visible data
	if (m_eTI==TI_IRowsetResynch)
	{
		TESTC_(m_pChgIRowsetResynch->GetVisibleData(phRow[ulArbitraryRowIdx], m_hChgAccessor, m_pResynchVisibleData), S_OK);
						
		//Resynch the rows 
		TESTC_(m_pChgIRowsetResynch->ResynchRows(cRowsObtained, phRow, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
	}
	else
	{
		//Resynch the rows 
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			TESTC_(m_pChgIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,cRowsObtained, phRow, TRUE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
		}
		else
		{
			TESTC_(m_pChgIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,cRowsObtained, phRow, FALSE, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus), S_OK);
		}
		TESTC_(m_pChgIRowsetRefresh->GetLastVisibleData(phRow[ulArbitraryRowIdx], m_hChgAccessor, m_pResynchVisibleData), S_OK);			
	}

	CompareOutParams(cRowsObtained, phRow);
	//Row status should be OK for all rows. 
	for (i=0; i<cRowsObtained; i++)
	{
		if (g_fNOCHANGE)
		{
			//no change was made here, if the provider can handle it, expect it
			COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_NOCHANGE);
		}
		else
		{
			COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_OK);
		}
	}	
	//Now get data that was resynch'd
	TESTC_(m_pChgIRowset->GetData(phRow[ulArbitraryRowIdx], m_hChgAccessor, m_pResynchRowsetData), S_OK);

	fResults =	COMPARE(CompareBuffer(m_pRowsetData,pArbRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(m_pResynchRowsetData,pArbRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	fResults &=	COMPARE(CompareBuffer(m_pResynchVisibleData,pArbRowData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
CLEANUP:									
	FreeOutParams();

	m_pChgIRowset->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL);									
	
	if (pArbRowData)
		PROVIDER_FREE(pArbRowData);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCPropCanHoldRowsResynch)
//--------------------------------------------------------------------
// @class General Scenarios with CANHOLDROWS
//
class TCPropCanHoldRowsResynch : public CPropCanHoldRowsResynch {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	ISOLEVEL	m_fHighestSupportedIsoLevel;

	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropCanHoldRowsResynch,CPropCanHoldRowsResynch);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Isolation Level - Chaos
	int Variation_1();
	// @cmember Isolation Level - Read Uncommitted
	int Variation_2();
	// @cmember Isolation Level -  Read Committed
	int Variation_3();
	// @cmember Isolation Level -  Repeatable Read
	int Variation_4();
	// @cmember Isolation Level - Serializable
	int Variation_5();
	// @cmember Isolation Level - Unspecified
	int Variation_6();
	// @cmember Own Insert - Highest Isolation Level
	int Variation_7();
	// @cmember Variable Length Columns Only Bound
	int Variation_8();
	// @cmember Fixed Length Columns Only Bound
	int Variation_9();
	// @cmember All Columns Bound BYREF
	int Variation_10();
	// @cmember Own Update - Highest Isolation Level
	int Variation_11();
	// @cmember GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE
	int Variation_12();
	// @cmember GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE
	int Variation_13();
	// @cmember ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
	int Variation_14();
	// @cmember ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW
	int Variation_15();
	// @cmember ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW
	int Variation_16();
	// @cmember GetVisibleData with pData = NULL - E_INVALIDARG
	int Variation_17();
	// @cmember GetVisibleData with Null Accessor, pData = NULL
	int Variation_18();
	// @cmember GetVisibleData with Null Accessor, pData valid
	int Variation_19();
	// @cmember Fetch Position
	int Variation_20();
	// @cmember Rows 1 and n
	int Variation_21();
	// @cmember All Rows
	int Variation_22();
	// @cmember ResynchRows with rghRows = NULL - E_INVALIDARG
	int Variation_23();
	// @cmember ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID
	int Variation_24();
	// @cmember ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID
	int Variation_25();
	// @cmember ResynchRows with cRows = 0 and no active hRows
	int Variation_26();
	// @cmember ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows=0
	int Variation_27();
	// @cmember ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact
	int Variation_28();
	// @cmember Fetch Position with Read Only Rowset
	int Variation_29();
	// @cmember Rows 1 and n with Read Only Rowset
	int Variation_30();
	// @cmember All Rows with Read Only Rowset
	int Variation_31();
	// @cmember ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
	int Variation_32();
	// @cmember ResynchRows/RefreshVisibleData with held  cRows=0 and all params NULL
	int Variation_33();
	// @cmember ResynchRows with one invalid hRow - no params
	int Variation_34();
	// @cmember ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW -NULLS
	int Variation_35();
	// @cmember ResynchRows/RefreshVisibleData with hRow = hard deleted - no params - no op
	int Variation_36();
	// @cmember GetVisibleData with released HROW - DB_E_BADROWHANDLE
	int Variation_37();
	// @cmember SetData with all status IGNORE
	int Variation_38();
	// @cmember SetData with all status DEFAULT
	int Variation_39();
	// @cmember InsertRows with all status DEFAULT
	int Variation_40();
//
// }}
};
// {{ TCW_TESTCASE(TCPropCanHoldRowsResynch)
#define THE_CLASS TCPropCanHoldRowsResynch
BEG_TEST_CASE(TCPropCanHoldRowsResynch, CPropCanHoldRowsResynch, L"General Scenarios with CANHOLDROWS")
	TEST_VARIATION(1,		L"Isolation Level - Chaos")
	TEST_VARIATION(2,		L"Isolation Level - Read Uncommitted")
	TEST_VARIATION(3,		L"Isolation Level -  Read Committed")
	TEST_VARIATION(4,		L"Isolation Level -  Repeatable Read")
	TEST_VARIATION(5,		L"Isolation Level - Serializable")
	TEST_VARIATION(6,		L"Isolation Level - Unspecified")
	TEST_VARIATION(7,		L"Own Insert - Highest Isolation Level")
	TEST_VARIATION(8,		L"Variable Length Columns Only Bound")
	TEST_VARIATION(9,		L"Fixed Length Columns Only Bound")
	TEST_VARIATION(10,		L"All Columns Bound BYREF")
	TEST_VARIATION(11,		L"Own Update - Highest Isolation Level")
	TEST_VARIATION(12,		L"GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(13,		L"GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(14,		L"ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE")
	TEST_VARIATION(15,		L"ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW")
	TEST_VARIATION(16,		L"ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW")
	TEST_VARIATION(17,		L"GetVisibleData with pData = NULL - E_INVALIDARG")
	TEST_VARIATION(18,		L"GetVisibleData with Null Accessor, pData = NULL")
	TEST_VARIATION(19,		L"GetVisibleData with Null Accessor, pData valid")
	TEST_VARIATION(20,		L"Fetch Position")
	TEST_VARIATION(21,		L"Rows 1 and n")
	TEST_VARIATION(22,		L"All Rows")
	TEST_VARIATION(23,		L"ResynchRows with rghRows = NULL - E_INVALIDARG")
	TEST_VARIATION(24,		L"ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(25,		L"ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(26,		L"ResynchRows with cRows = 0 and no active hRows")
	TEST_VARIATION(27,		L"ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows=0")
	TEST_VARIATION(28,		L"ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact")
	TEST_VARIATION(29,		L"Fetch Position with Read Only Rowset")
	TEST_VARIATION(30,		L"Rows 1 and n with Read Only Rowset")
	TEST_VARIATION(31,		L"All Rows with Read Only Rowset")
	TEST_VARIATION(32,		L"ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE")
	TEST_VARIATION(33,		L"ResynchRows/RefreshVisibleData with held  cRows=0 and all params NULL")
	TEST_VARIATION(34,		L"ResynchRows with one invalid hRow - no params")
	TEST_VARIATION(35,		L"ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW -NULLS")
	TEST_VARIATION(36,		L"ResynchRows/RefreshVisibleData with hRow = hard deleted - no params - no op")
	TEST_VARIATION(37,		L"GetVisibleData with released HROW - DB_E_BADROWHANDLE")
	TEST_VARIATION(38,		L"SetData with all status IGNORE")
	TEST_VARIATION(39,		L"SetData with all status DEFAULT")
	TEST_VARIATION(40,		L"InsertRows with all status DEFAULT")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropNoHoldRowsResynch)
//--------------------------------------------------------------------
// @class Rowset without CANHOLDROWS
//
class TCPropNoHoldRowsResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropNoHoldRowsResynch,CResynchRefresh);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read Only Rowset without DBPROP_CANHOLDROWS
	int Variation_1();
	// @cmember Changeable Rowset without DBPROP_CANHOLDROWS
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCPropNoHoldRowsResynch)
#define THE_CLASS TCPropNoHoldRowsResynch
BEG_TEST_CASE(TCPropNoHoldRowsResynch, CResynchRefresh, L"Rowset without CANHOLDROWS")
	TEST_VARIATION(1,		L"Read Only Rowset without DBPROP_CANHOLDROWS")
	TEST_VARIATION(2,		L"Changeable Rowset without DBPROP_CANHOLDROWS")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropBookmarksResynch)
//--------------------------------------------------------------------
// @class Rowset with BOOKMARKS
//
class TCPropBookmarksResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropBookmarksResynch,CResynchRefresh);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read Only Rowset with DBPROP_BOOKMARKS
	int Variation_1();
	// @cmember Changeable Rowset with DBPROP_BOOKMARKS
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCPropBookmarksResynch)
#define THE_CLASS TCPropBookmarksResynch
BEG_TEST_CASE(TCPropBookmarksResynch, CResynchRefresh, L"Rowset with BOOKMARKS")
	TEST_VARIATION(1,		L"Read Only Rowset with DBPROP_BOOKMARKS")
	TEST_VARIATION(2,		L"Changeable Rowset with DBPROP_BOOKMARKS")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropDeferredResynch)
//--------------------------------------------------------------------
// @class Rowset with CANHOLDROWS and DEFERRED 
//
class TCPropDeferredResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropDeferredResynch,CResynchRefresh);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read Only Rowset with DBPROP_DEFERRED
	int Variation_1();
	// @cmember Changeable Rowset with DBPROP_DEFERRED
	int Variation_2();
	// }}
	//flag to chekc if the Init for this class passed
	BOOL m_fInitPass;
};
// {{ TCW_TESTCASE(TCPropDeferredResynch)
#define THE_CLASS TCPropDeferredResynch
BEG_TEST_CASE(TCPropDeferredResynch, CResynchRefresh, L"Rowset with CANHOLDROWS and DEFERRED ")
	TEST_VARIATION(1,		L"Read Only Rowset with DBPROP_DEFERRED")
	TEST_VARIATION(2,		L"Changeable Rowset with DBPROP_DEFERRED")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropCacheDeferredResynch)
//--------------------------------------------------------------------
// @class Rowset with CACHEDEFERRED
//
class TCPropCacheDeferredResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropCacheDeferredResynch,CResynchRefresh);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read Only Rowset with DBPROP_CACHEDEFERRED
	int Variation_1();
	// @cmember Changeable Rowset with DBPROP_CACHEDEFERRED
	int Variation_2();
	//flag to check if the Init for this class passed
	BOOL m_fInitPass;
	// }}
};
// {{ TCW_TESTCASE(TCPropCacheDeferredResynch)
#define THE_CLASS TCPropCacheDeferredResynch
BEG_TEST_CASE(TCPropCacheDeferredResynch, CResynchRefresh, L"Rowset with CACHEDEFERRED")
	TEST_VARIATION(1,		L"Read Only Rowset with DBPROP_CACHEDEFERRED")
	TEST_VARIATION(2,		L"Changeable Rowset with DBPROP_CACHEDEFERRED")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropUpdateResynch)
//--------------------------------------------------------------------
// @class Rowset with IRowsetUpdate
//
class TCPropUpdateResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropUpdateResynch,CResynchRefresh);
	// }}

	//@cmember IRowsetUpdate interface on Chg Rowset
	IRowsetUpdate	*m_pChgIRowsetUpdate1;
	IRowsetUpdate	*m_pChgIRowsetUpdate2;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Soft deleted row
	int Variation_1();
	// @cmember Soft inserted row
	int Variation_2();
	// @cmember Soft changed row
	int Variation_3();
	// @cmember Inserted row after Update
	int Variation_4();
	// @cmember Pending change
	int Variation_5();
	// @cmember GetOriginalData
	int Variation_6();
	// @cmember Pending Insert
	int Variation_7();
	// @cmember Pending Delete
	int Variation_8();
	// @cmember Insert, change and delete on a rowset
	int Variation_9();
	// }}
};
// {{ TCW_TESTCASE(TCPropUpdateResynch)
#define THE_CLASS TCPropUpdateResynch
BEG_TEST_CASE(TCPropUpdateResynch, CResynchRefresh, L"Rowset with IRowsetUpdate")
	TEST_VARIATION(1,		L"Soft deleted row")
	TEST_VARIATION(2,		L"Soft inserted row")
	TEST_VARIATION(3,		L"Soft changed row")
	TEST_VARIATION(4,		L"Inserted row after Update")
	TEST_VARIATION(5,		L"Pending change")
	TEST_VARIATION(6,		L"GetOriginalData")
	TEST_VARIATION(7,		L"Pending Insert")
	TEST_VARIATION(8,		L"Pending Delete")
	TEST_VARIATION(9,		L"Insert, change and delete on a rowset")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCZombieResynch)
//--------------------------------------------------------------------
// @class Rowset Preservation tests
//
class TCZombieResynch : public CTransaction, public TCBase
{
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:

	//@cmember Property set for rowset properties
	DBPROPSET	m_PropSet;	
	//@cmember Property struct for ResynchRows/RefreshVisibleData
	DBPROP		m_DBProp;
	//@cmember  Accessor
	HACCESSOR	m_hAccessor;
	//@cmember  Count of bindings
	DBCOUNTITEM	m_cBindings;
	//@cmember  Array of bindings
	DBBINDING	*m_rgBindings;
	//@cmember  Size of row
	DBLENGTH	m_cbRowSize;
	
	//@cmember hRow
	HROW		m_hRow;
	//@cmember Bogus data buffer
	BYTE		*m_pData;

	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombieResynch,CTransaction);
	// }}

	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	//@cmember Helper function for all rowset preservation testing
	//with respect to ResynchRows/RefreshVisibleData
	int TCZombieResynch::TestZombie(ETXN	eTxn, BOOL	fRetaining);
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with fRetaining = TRUE
	int Variation_1();
	// @cmember Commit with fRetaining = FALSE
	int Variation_2();
	// @cmember Abort with fRetaining = TRUE
	int Variation_3();
	// @cmember Abort with fRetaining = FALSE
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(TCZombieResynch)
#define THE_CLASS TCZombieResynch
BEG_TEST_CASE(TCZombieResynch, CTransaction, L"Rowset Preservation tests")
	TEST_VARIATION(1,		L"Commit with fRetaining = TRUE")
	TEST_VARIATION(2,		L"Commit with fRetaining = FALSE")
	TEST_VARIATION(3,		L"Abort with fRetaining = TRUE")
	TEST_VARIATION(4,		L"Abort with fRetaining = FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCExtendedErrorsResynch)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrorsResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	//@cmember Extended error object
	//CExtError * m_pExtError;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrorsResynch,CResynchRefresh);
	// }}
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid ResynchRows/RefreshVisibleData calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid GetVisibleData call with previous error object existing
	int Variation_2();
	// @cmember Invalid ResynchRows call with previous error object existing
	int Variation_3();
	// @cmember Invalid ResynchRows/RefreshVisibleData calls no with previous error object existing
	int Variation_4();
	// @cmember Invalid ResynchRows/RefreshVisibleData - E_INVALIDARG
	int Variation_5();
	// @cmember pcRowsRefreshed - NULL, ignore prghRowsRefreshed
	int Variation_6();
	// }}
};
// {{ TCW_TESTCASE(TCExtendedErrorsResynch)
#define THE_CLASS TCExtendedErrorsResynch
BEG_TEST_CASE(TCExtendedErrorsResynch, CResynchRefresh, L"Extended Errors")
	TEST_VARIATION(1,		L"Valid ResynchRows/RefreshVisibleData calls with previous error object existing.")
	TEST_VARIATION(2,		L"Invalid GetVisibleData call with previous error object existing")
	TEST_VARIATION(3,		L"Invalid ResynchRows call with previous error object existing")
	TEST_VARIATION(4,		L"Invalid ResynchRows/RefreshVisibleData calls no with previous error object existing")
	TEST_VARIATION(5,		L"Invalid ResynchRows - E_INVALIDARG")
	TEST_VARIATION(6,		L"pcRowsRefreshed - NULL, ignore prghRowsRefreshed")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(TCPropResynchOnlyResynch)
//--------------------------------------------------------------------
// @class Rowsets with only ResynchRows/RefreshVisibleData Requested
//
class TCPropResynchOnlyResynch : public CResynchRefresh {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropResynchOnlyResynch,CResynchRefresh);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read only rowset with Resynch Only
	int Variation_1();
	// @cmember Changeable rowset with Resynch Only
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCPropResynchOnlyResynch)
#define THE_CLASS TCPropResynchOnlyResynch
BEG_TEST_CASE(TCPropResynchOnlyResynch, CResynchRefresh, L"Rowsets with only ResynchRows/RefreshVisibleData Requested")
	TEST_VARIATION(1,		L"Read only rowset with Resynch Only")
	TEST_VARIATION(2,		L"Changeable rowset with Resynch Only")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCPropCanHoldRowsResynchBLOBS)
//--------------------------------------------------------------------
// @class CanHoldRows testcases using BLOB data
//
class TCPropCanHoldRowsResynchBLOBS : public CPropCanHoldRowsResynch {
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:

	ISOLEVEL	m_fHighestSupportedIsoLevel;

	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPropCanHoldRowsResynchBLOBS,CPropCanHoldRowsResynch);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Isolation Level - Chaos
	int Variation_1();
	// @cmember Isolation Level - Read Uncommitted
	int Variation_2();
	// @cmember Isolation Level -  Read Committed
	int Variation_3();
	// @cmember Isolation Level -  Repeatable Read
	int Variation_4();
	// @cmember Isolation Level - Serializable
	int Variation_5();
	// @cmember Isolation Level - Unspecified
	int Variation_6();
	// @cmember Own Insert - Highest Isolation Level
	int Variation_7();
	// @cmember Variable Length Columns Only Bound
	int Variation_8();
	// @cmember Fixed Length Columns Only Bound
	int Variation_9();
	// @cmember All Columns Bound BYREF
	int Variation_10();
	// @cmember Own Update - Highest Isolation Level
	int Variation_11();
	// @cmember GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE
	int Variation_12();
	// @cmember GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE
	int Variation_13();
	// @cmember ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
	int Variation_14();
	// @cmember ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW
	int Variation_15();
	// @cmember ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW
	int Variation_16();
	// @cmember GetVisibleData with pData = NULL - E_INVALIDARG
	int Variation_17();
	// @cmember GetVisibleData with Null Accessor, pData = NULL
	int Variation_18();
	// @cmember GetVisibleData with Null Accessor, pData valid
	int Variation_19();
	// @cmember Fetch Position
	int Variation_20();
	// @cmember Rows 1 and n
	int Variation_21();
	// @cmember All Rows
	int Variation_22();
	// @cmember ResynchRows with rghRows = NULL - E_INVALIDARG
	int Variation_23();
	// @cmember ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID
	int Variation_24();
	// @cmember ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID
	int Variation_25();
	// @cmember ResynchRows with cRows = 0 and no active hRows
	int Variation_26();
	// @cmember ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows=0
	int Variation_27();
	// @cmember ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact
	int Variation_28();
	// @cmember Fetch Position with Read Only Rowset
	int Variation_29();
	// @cmember Rows 1 and n with Read Only Rowset
	int Variation_30();
	// @cmember All Rows  with Read Only Rowset
	int Variation_31();
	// }}
};
// {{ TCW_TESTCASE(TCPropCanHoldRowsResynchBLOBS)
#define THE_CLASS TCPropCanHoldRowsResynchBLOBS
BEG_TEST_CASE(TCPropCanHoldRowsResynchBLOBS, CPropCanHoldRowsResynch, L"CanHoldRows testcases using BLOB data")
	TEST_VARIATION(1,		L"Isolation Level - Chaos")
	TEST_VARIATION(2,		L"Isolation Level - Read Uncommitted")
	TEST_VARIATION(3,		L"Isolation Level -  Read Committed")
	TEST_VARIATION(4,		L"Isolation Level -  Repeatable Read")
	TEST_VARIATION(5,		L"Isolation Level - Serializable")
	TEST_VARIATION(6,		L"Isolation Level - Unspecified")
	TEST_VARIATION(7,		L"Own Insert - Highest Isolation Level")
	TEST_VARIATION(8,		L"Variable Length Columns Only Bound")
	TEST_VARIATION(9,		L"Fixed Length Columns Only Bound")
	TEST_VARIATION(10,		L"All Columns Bound BYREF")
	TEST_VARIATION(11,		L"Own Update - Highest Isolation Level")
	TEST_VARIATION(12,		L"GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(13,		L"GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(14,		L"ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE")
	TEST_VARIATION(15,		L"ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW")
	TEST_VARIATION(16,		L"ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW")
	TEST_VARIATION(17,		L"GetVisibleData with pData = NULL - E_INVALIDARG")
	TEST_VARIATION(18,		L"GetVisibleData with Null Accessor, pData = NULL")
	TEST_VARIATION(19,		L"GetVisibleData with Null Accessor, pData valid")
	TEST_VARIATION(20,		L"Fetch Position")
	TEST_VARIATION(21,		L"Rows 1 and n")
	TEST_VARIATION(22,		L"All Rows")
	TEST_VARIATION(23,		L"ResynchRows with rghRows = NULL - E_INVALIDARG")
	TEST_VARIATION(24,		L"ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(25,		L"ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(26,		L"ResynchRows with cRows = 0 and no active hRows")
	TEST_VARIATION(27,		L"ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows=0")
	TEST_VARIATION(28,		L"ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact")
	TEST_VARIATION(29,		L"Fetch Position with Read Only Rowset")
	TEST_VARIATION(30,		L"Rows 1 and n with Read Only Rowset")
	TEST_VARIATION(31,		L"All Rows  with Read Only Rowset")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCNullRowResynch)
//--------------------------------------------------------------------
// @class Tests Resych with all NULLs
//
class TCNullRowResynch : public CRowsetObject, public TCBase
{
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNullRowResynch,CRowsetObject);
	// }}

	ITransactionLocal	*m_pITransactionLocal;
	DBBINDING			*m_rgBindings;
	DBCOUNTITEM			m_cBindings;
	DBLENGTH			m_cbRowSize;
	HACCESSOR			m_hAccessor;
	BYTE				*m_pData;
	IRowset				*m_pIRowset;
	HROW				m_hRow;
	HROW				*m_phRow;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Resynch on Null row
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(TCNullRowResynch)
#define THE_CLASS TCNullRowResynch
BEG_TEST_CASE(TCNullRowResynch, CRowsetObject, L"Tests Resych with all NULLs")
	TEST_VARIATION(1,		L"Resynch on Null row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMiscResynch)
//--------------------------------------------------------------------
// @class Misc Resych Tests
//
class TCMiscResynch : public CRowsetObject, public TCBase
{
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMiscResynch,CRowsetObject);
	// }}

	CTable			*m_pMiscTable;
	IRowset			*m_pIRowset;
	HROW			m_hRow;
	HROW			*m_phRow;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Resynch on a row with computed columns
	int Variation_1();
	// @cmember different accessors
	int Variation_2();
	// @cmember OTHERUPDATEDELETE - FALSE
	int Variation_3();
	// @cmember Resynch & Refresh on the same rowset
	int Variation_4();
	// @cmember GetVisible twice w/BLOBs
	int Variation_5();
	// }}
};
// {{ TCW_TESTCASE(TCMiscResynch)
#define THE_CLASS TCMiscResynch
BEG_TEST_CASE(TCMiscResynch, CRowsetObject, L"Misc Resych Tests")
	TEST_VARIATION(1,		L"Resynch on a row with computed columns")
	TEST_VARIATION(2,		L"Different accessors")
	TEST_VARIATION(3,		L"OTHERUPDATEDELETE - FALSE")
	TEST_VARIATION(4,		L"GetVisible twice w/BLOBs")
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

//12-22
COPY_TEST_CASE(TCPropCanHoldRowsRefreshTRUE, TCPropCanHoldRowsResynch)
COPY_TEST_CASE(TCPropNoHoldRowsRefreshTRUE, TCPropNoHoldRowsResynch)
COPY_TEST_CASE(TCPropBookmarksRefreshTRUE, TCPropBookmarksResynch)
COPY_TEST_CASE(TCPropDeferredRefreshTRUE, TCPropDeferredResynch)
COPY_TEST_CASE(TCPropCacheDeferredRefreshTRUE, TCPropCacheDeferredResynch)
COPY_TEST_CASE(TCPropUpdateRefreshTRUE, TCPropUpdateResynch)
COPY_TEST_CASE(TCZombieRefreshTRUE, TCZombieResynch)
COPY_TEST_CASE(TCExtendedErrorsRefreshTRUE, TCExtendedErrorsResynch)
COPY_TEST_CASE(TCPropRefreshOnlyRefreshTRUE, TCPropResynchOnlyResynch)
COPY_TEST_CASE(TCPropCanHoldRowsRefreshTRUEBLOBS, TCPropCanHoldRowsResynchBLOBS)
COPY_TEST_CASE(TCNullRowRefreshTRUE, TCNullRowResynch)
COPY_TEST_CASE(TCMiscRefreshTRUE, TCMiscResynch)

//23-33
COPY_TEST_CASE(TCPropCanHoldRowsRefreshFALSE, TCPropCanHoldRowsResynch)
COPY_TEST_CASE(TCPropNoHoldRowsRefreshFALSE, TCPropNoHoldRowsResynch)
COPY_TEST_CASE(TCPropBookmarksRefreshFALSE, TCPropBookmarksResynch)
COPY_TEST_CASE(TCPropDeferredRefreshFALSE, TCPropDeferredResynch)
COPY_TEST_CASE(TCPropCacheDeferredRefreshFALSE, TCPropCacheDeferredResynch)
COPY_TEST_CASE(TCPropUpdateRefreshFALSE, TCPropUpdateResynch)
COPY_TEST_CASE(TCZombieRefreshFALSE, TCZombieResynch)
COPY_TEST_CASE(TCExtendedErrorsRefreshFALSE, TCExtendedErrorsResynch)
COPY_TEST_CASE(TCPropRefreshOnlyRefreshFALSE, TCPropResynchOnlyResynch)
COPY_TEST_CASE(TCPropCanHoldRowsRefreshFALSEBLOBS, TCPropCanHoldRowsResynchBLOBS)
COPY_TEST_CASE(TCNullRowRefreshFALSE, TCNullRowResynch)
COPY_TEST_CASE(TCMiscRefreshFALSE, TCMiscResynch)

#if 0 
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(11, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCPropCanHoldRowsResynch)
	TEST_CASE(2, TCPropNoHoldRowsResynch)
	TEST_CASE(3, TCPropBookmarksResynch)
	TEST_CASE(4, TCPropDeferredResynch)
	TEST_CASE(5, TCPropCacheDeferredResynch)
	TEST_CASE(6, TCPropUpdateResynch)
	TEST_CASE(7, TCZombieResynch)
	TEST_CASE(8, TCExtendedErrorsResynch)
	TEST_CASE(9, TCPropResynchOnlyResynch)
	TEST_CASE(10, TCPropCanHoldRowsResynchBLOBS)
	TEST_CASE(11, TCNullRowResynch)
	TEST_CASE(12, TCMiscResynch)//	TEST_CASE(12, TCMiscResynch)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
		TEST_MODULE(36, ThisModule, gwszModuleDescrip)
			TEST_CASE(1, TCPropCanHoldRowsResynch)
			TEST_CASE(2, TCPropNoHoldRowsResynch)
			TEST_CASE(3, TCPropBookmarksResynch)
			TEST_CASE(4, TCPropDeferredResynch)
			TEST_CASE(5, TCPropCacheDeferredResynch)
			TEST_CASE(6, TCPropUpdateResynch)
			TEST_CASE(7, TCZombieResynch)
			TEST_CASE(8, TCExtendedErrorsResynch)
			TEST_CASE(9, TCPropResynchOnlyResynch)
			TEST_CASE(10, TCPropCanHoldRowsResynchBLOBS)
			TEST_CASE(11, TCNullRowResynch)
			TEST_CASE(12, TCMiscResynch)
			//switch which interface is used here
			TEST_CASE_WITH_PARAM(13, TCPropCanHoldRowsRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(14, TCPropNoHoldRowsRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(15, TCPropBookmarksRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(16, TCPropDeferredRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(17, TCPropCacheDeferredRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(18, TCPropUpdateRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(19, TCZombieRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(20, TCExtendedErrorsRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(21, TCPropRefreshOnlyRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(22, TCPropCanHoldRowsRefreshTRUEBLOBS, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(23, TCNullRowRefreshTRUE, TI_IRowsetRefreshTRUE)
			TEST_CASE_WITH_PARAM(24, TCMiscRefreshTRUE, TI_IRowsetRefreshTRUE)
			//
			TEST_CASE_WITH_PARAM(25, TCPropCanHoldRowsRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(26, TCPropNoHoldRowsRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(27, TCPropBookmarksRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(28, TCPropDeferredRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(29, TCPropUpdateRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(30, TCPropCacheDeferredRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(31, TCZombieRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(32, TCExtendedErrorsRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(33, TCPropRefreshOnlyRefreshFALSE, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(34, TCPropCanHoldRowsRefreshFALSEBLOBS, TI_IRowsetRefreshFALSE)
			TEST_CASE_WITH_PARAM(35, TCNullRowRefreshFALSE, TI_IRowsetRefreshFALSE)	
			TEST_CASE_WITH_PARAM(36, TCMiscRefreshFALSE, TI_IRowsetRefreshFALSE)	
		END_TEST_MODULE()
#endif


// {{ TCW_TC_PROTOTYPE(TCPropCanHoldRowsResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropCanHoldRowsResynch - General Scenarios with CANHOLDROWS
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropCanHoldRowsResynch::Init()
{
	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CPropCanHoldRowsResynch::Init(m_eTI))
	// }}
	{		
		//Find highest isolation level
		if (m_fSerializable)
		{
			m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_SERIALIZABLE;
		}
		else
		{
			if (m_fRepeatableRead)
			{	
				m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_REPEATABLEREAD;
			}
			else
			{
				if (m_fReadCommitted)
				{
					m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_READCOMMITTED;	
				}
				else
				{
					if (m_fReadUncommitted)
					{
						m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_READUNCOMMITTED;
					}
					else
					{
						m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_CHAOS;
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Chaos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_1()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos)
	{
	 	odtLog << wszNoProviderSupport << L"CHAOS" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
			
	//Start txn we're testing at Chaos Isolation Level
	if (CHECK(m_pRORowset1->m_pITxnLocal->StartTransaction(ISOLATIONLEVEL_CHAOS, 0, NULL, NULL), S_OK))	
	{
		if (CreateResynchObjects(m_eTI))
		{	
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, m_hROAccessor))
			{
				if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_OLD,VERIFY_NEW))
				{	
					//Everything went OK
					fResults = TRUE;
				}
			FreeOutParams();
			}
		}
	}
	COMPARE(EndTxns(), TRUE);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Read Uncommitted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_2()
{
	BOOL	fResults = FALSE;
	
	//If no provider support for this isolation level, just return pass
	if (!m_fReadUncommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READUNCOMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
		
	//Start txn we're testing at Read Uncommitted Isolation Level
	if (CHECK(m_pRORowset1->m_pITxnLocal->StartTransaction(ISOLATIONLEVEL_READUNCOMMITTED, 0, NULL, NULL), S_OK))	
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Now do the same thing on our read only rowset
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_READUNCOMMITTED,m_hROAccessor))
			{
				if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_OLD,VERIFY_NEW))
				{
					//Everything went OK
					fResults = TRUE;	
				}
				FreeOutParams();
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level -  Read Committed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_3()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fReadCommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READCOMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Read Committed Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_READCOMMITTED), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Test visible data on our changeable rowset	
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_READCOMMITTED, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_OLD,VERIFY_NEW))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_READCOMMITTED, m_hROAccessor))
					{
						if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_OLD,VERIFY_NEW))
						{	
							//Everything went OK
							fResults = TRUE;		
							FreeOutParams();
						}
					}
				}
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level -  Repeatable Read
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_4()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fRepeatableRead)
	{
	 	odtLog << wszNoProviderSupport << L"REPEATABLEREAD" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Read Repeated Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_REPEATABLEREAD), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Test visible data on our changeable rowset	
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_REPEATABLEREAD, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_OLD, VERIFY_OLD, VERIFY_OLD, VERIFY_OLD))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_REPEATABLEREAD, m_hROAccessor))
					{
						if (VerifyData(VERIFY_OLD, VERIFY_OLD, VERIFY_OLD, VERIFY_OLD))
						{	
							//Everything went OK
							fResults = TRUE;
						}
					}
				}
				FreeOutParams();
			}
		}
		EndTxns();
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Serializable
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_5()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"SERIALIZABLE" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Serializable Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_SERIALIZABLE), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Test visible data on our changeable rowset	
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_SERIALIZABLE, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_OLD, VERIFY_OLD, VERIFY_OLD, VERIFY_OLD))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_SERIALIZABLE, m_hROAccessor))
					{
						if (VerifyData(VERIFY_OLD, VERIFY_OLD, VERIFY_OLD, VERIFY_OLD))
						{	
							//Everything went OK
							fResults = TRUE;				
						}
					}
				}
				FreeOutParams();
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Unspecified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_6()
{
	BOOL	fResults = FALSE;	
	
	//Start txn we're testing at unspecified Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_UNSPECIFIED), XACT_E_ISOLATIONLEVEL))
	{						
		//Everything went OK
		fResults = TRUE;	
	}

	//in case a txn was started
	COMPARE(EndTxns(), TRUE);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Own Insert - Highest Isolation Level
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_7()
{	
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	HRESULT				ExpectedHr;
	BOOL				fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}
	
	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();	

	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own insert.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from inserting
	//due to the high isolation level
	if (!CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK))	
		goto CLEANUP;
	
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
		goto CLEANUP;	

	//Insert new row ourselves and keep hRow
	if (!COMPARE(m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow), TRUE))
	{
		goto CLEANUP;
	}
	//increment for comparasion
	g_ulLastActualInsert++;

	//If we don't support strong identity, our single row will fail
	if (m_fStrongIdentity)
	{
		ExpectedHr = S_OK;
	}
	else
	{
		ExpectedHr = DB_E_ERRORSOCCURRED;
	}

	//flag to see if RefreshVisibleRows has been called
	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;

	//Now do a ResynchRows to see if inserted row data is brought in
	if (!CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), ExpectedHr))
	{
		FreeOutParams();
		goto CLEANUP;
	}

	//don't check the rowstatus here.   depending on if the row has defualts there might be some refreshing going on
	
	//This only applies if we support resynch on this newly inserted row
	if (m_fStrongIdentity)
	{
		//Now Get Newly Resynch'd Data
		if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData), S_OK))
		{
			goto CLEANUP;
		}
		//Now GetVisibleData on the same row
		if (!CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), S_OK))
		{
			goto CLEANUP;
		}
		
		//refresh should bring in the defaults		

		//Now make sure we could see the new data at all times, since its our own insert
		if (VerifyData(VERIFY_NEW, VERIFY_IGNORE, VERIFY_IGNORE, VERIFY_NEW))										
		{
			//Everything went OK
			fResults = TRUE;
		}
	}
	//Make sure our status is right
	else
	{
		COMPARE(m_cRowsResynched,1);
		COMPARE(m_rghRowsResynched[0],hRow);
		CompareOutParams(1, &hRow);
		fResults = COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED);
	}

	fResults = TRUE;	
CLEANUP:
	if (5!=m_cRowsResynched)
	{
		FreeOutParams();
	}
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	//back out changes
	COMPARE(EndTxns(TRUE), TRUE);	
	g_ulLastActualInsert--;

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Variable Length Columns Only Bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_8()
{
	BOOL		fResults		= FALSE;	
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor		= DB_NULL_HACCESSOR;
	DBBINDING	*rgBindings		= NULL;
	DBCOUNTITEM	cBindings		= 0;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;	
	}
	//Get accessors with only variable length bound.  Bindings will be the same
	//for changeable and read only rowsets
	if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND,
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}
	if (!CHECK(GetAccessorAndBindings(m_pRORowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hROAccessor, NULL, NULL, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND,			
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}	
	//Test visible data on our changeable rowset	
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))
	{
		if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))			
		{
			//Now do the same thing on our read only rowset
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
			{
				if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
				{
					//Everything went OK
					fResults = TRUE;				
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	if (hChgAccessor != DB_NULL_HACCESSOR)	
		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
		
	if (hROAccessor != DB_NULL_HACCESSOR)
		m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);

	if (rgBindings)
		PROVIDER_FREE(rgBindings);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Fixed Length Columns Only Bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_9()
{
	BOOL		fResults		= FALSE;	
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor		= DB_NULL_HACCESSOR;
	DBBINDING	*rgBindings		= NULL;
	DBCOUNTITEM	cBindings		= 0;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;	
	}
	//Get accessors with only variable length bound.  Bindings will be the same
	//for changeable and read only rowsets

	//Get accessors with only fixed length bound, bindings 
	//are the same for changeable and RO accessors
	if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		FIXED_LEN_COLS_BOUND,			
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}
	//Test visible data on our changeable rowset	
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))
	{
		if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
		{
			ReleaseResynchObjects();
			if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
			{
				goto CLEANUP;		
			}
			if (!CHECK(GetAccessorAndBindings(m_pRORowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
				&hROAccessor, NULL, NULL, NULL,			
		 		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				FIXED_LEN_COLS_BOUND,			
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongData),S_OK))
			{
				goto CLEANUP;
			}			
			//Now do the same thing on our read only rowset
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
			{
				if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
				{
					//Everything went OK
					fResults = TRUE;
				}
			}
		}
	}	
CLEANUP:	
	FreeOutParams();
	if (rgBindings)
		PROVIDER_FREE(rgBindings);

	if (hChgAccessor != DB_NULL_HACCESSOR)
		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
	if (hROAccessor != DB_NULL_HACCESSOR)
		m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc All Columns Bound BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_10()
{
	BOOL		fResults		= FALSE;	
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor		= DB_NULL_HACCESSOR;
	ULONG		i				= 0;
	DBBINDING	*rgBindings		= NULL;
	DBCOUNTITEM	cBindings		= 0;	
	DBLENGTH	cRowSize		= 0;
	
	//Test visible data on our changeable rowset, repeat it several times
	//to flush out any potential memory leaks and stress test it
	for (i = 0; i< STRESS_RESYNCH_REPS; i++)
	{
		ReleaseResynchObjects();
		if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
		{
			goto CLEANUP;		
		}
		//Since some provider's only support variable length
		//cols by ref, limit the accessor to these
		if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, 
			DBACCESSOR_ROWDATA,
			&hChgAccessor, &rgBindings, &cBindings, &cRowSize,			
			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF,
			NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongData),S_OK))
		{
			goto CLEANUP;
		}
		//re-create buffers with the new accesor and bindings created just above
		FreeBuffers(m_cbRowSize);
		AllocDataBuffers(cRowSize);
			
		if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))	
		{
			if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
			{
				//re-re-create buffers so class functions work correctly.  yes, kind of hokey.
				//free with Compare to take a care of byref value
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pVisibleData,									cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pRowsetData,									cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pResynchRowsetData,							cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);								
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pResynchVisibleData,							cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);								

/*				FreeBuffers(cRowSize);
				AllocDataBuffers(m_cbRowSize);
				ReleaseResynchObjects();
				if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
				{
					FreeOutParams();
					goto CLEANUP;		
				}*/
				if (!CHECK(GetAccessorAndBindings(	m_pRORowset1->m_pIAccessor, 
													DBACCESSOR_ROWDATA,
													&hROAccessor, NULL, NULL, NULL,			
  													DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
													ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF,
													NULL, NULL,
													NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
													NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
													m_fBindLongData),S_OK))
				{
					FreeOutParams();
					goto CLEANUP;
				}
/*				//re-create buffers with the new accesor and bindings created just above
				FreeBuffers(m_cbRowSize);
				AllocDataBuffers(cRowSize);
*/
				//Now do the same thing on our read only rowset
				if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
				{
					if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
					{	
						//Everything went OK
						fResults = TRUE;
						//re-re-create buffers so class functoins work correctly.  yes, kind of hokey
						//free with Compare to take a care of byref value
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pVisibleData,									cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pRowsetData,									cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pResynchRowsetData,							cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);								
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pResynchVisibleData,							cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);								

						FreeBuffers(cRowSize);
						AllocDataBuffers(m_cbRowSize);
						//free mem each time through the loop
						FreeAccessorBindings(cBindings,rgBindings);
						rgBindings	= NULL;
						if (hChgAccessor != DB_NULL_HACCESSOR)
						{
							m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
						}
						if (hROAccessor != DB_NULL_HACCESSOR)
						{
							m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);
						}
						continue;
					}
				}
			}
		}
		break;
	}
	
	//If we broke before we finished loop, there was an error
	if (i != STRESS_RESYNCH_REPS)
	{
		fResults = FALSE;
	}	
CLEANUP:
	ReleaseResynchObjects();
	FreeOutParams();

	if (rgBindings)
	{
		FreeAccessorBindings(cBindings,rgBindings);
		rgBindings	= NULL;
		if (hChgAccessor != DB_NULL_HACCESSOR && m_pChgRowset1->m_pIAccessor)
		{
			m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
		}
		if (hROAccessor != DB_NULL_HACCESSOR && m_pRORowset1->m_pIAccessor)
		{
			m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);
		}
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Own Update - Highest Isolation Level
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_11()
{
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	HRESULT				hr;	
	BOOL				fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	if (!CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK))	
	{
		goto CLEANUP;
	}
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;
	}
	//Change a row ourselves and keep hRow
	if (!COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), &hRow), TRUE))
	{
		goto CLEANUP;
	}
	//increment this for the comparision
	g_ulLastActualInsert++;

	//Next GetData from cache
	if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pRowsetData), S_OK))
	{
		goto CLEANUP;
	}
	//Now do a ResynchRows to see if new data is brought in
	if (!CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
	{
		goto CLEANUP;
	}
	//Now Get Newly Resynch'd Data
	if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData), S_OK))
	{
		goto CLEANUP;
	}
	//Now GetVisibleData on the same row
	hr=GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE);
	if (hr!=S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_DELETEDROW==hr)
		{
			fResults	= TRUE;	
			goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}

	//no change is made, these should be the same
	if(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY)!=TRUE)
	{
		goto CLEANUP;
	}

	//Now make sure we could see the new data at all times, since our own update
	if (VerifyData(VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, VERIFY_IGNORE))										
	{
		//Everything went OK
		fResults = TRUE;
	}
CLEANUP:
	FreeOutParams();

	//abort the change, easier to manage the test this way
	COMPARE(EndTxns(TRUE), TRUE);	
	g_ulLastActualInsert--;

	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_12()
{
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	BOOL		fResults		= FALSE;
	HROW		hRow			= DB_NULL_HROW;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//We will skip this variation is provider doesn't support PASSBYREF
	if (!m_fPassByRef)
	{
		odtLog << L"This Provider doesn't support PASSBYREF accessors. Variation is not applicable.\n";
		return TEST_PASS;
	}

	//Get a PASSBYREF accessor
	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(
		DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, m_cBindings,
		m_rgBindings, m_cbRowSize, &hChgAccessor, NULL), S_OK))
	{
		//Get an hRow
		if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))				
		{	
			//Any pass by ref accessor should fail
			if (CHECK(GetLastVisibleData(hRow, hChgAccessor, m_pVisibleData,FALSE), DB_E_BADACCESSORHANDLE))
			{
				fResults = TRUE;
			}
			//Now clean up our row handle
			m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
		}

		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_13()
{
	HACCESSOR	hChgAccessor = DB_NULL_HACCESSOR;
	BOOL		fResults = FALSE;
	HROW		hRow = DB_NULL_HROW;
	DBBINDING	*rgBindings = NULL;
	DBCOUNTITEM	cBindings;
	DBLENGTH	cbRowSize;	
	
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Get bindings for a by ref accessor, use variable len only cols
	if (CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND, FORWARD, ALL_COLS_BY_REF,
		NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))					
	{
		//We only wanted bindings
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK);
		hChgAccessor = DB_NULL_HACCESSOR;
		
		//We know this is a variable length column by ref, so provider owned
		//should be fine here under normal GetData situations
		rgBindings[0].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
		
		//Now create the accessor which has provider owned memory 
		if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(
																DBACCESSOR_ROWDATA, cBindings,
																rgBindings, cbRowSize, 
																&hChgAccessor, NULL), S_OK))
		{
			//Get an hRow
			if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))				
			{	
				//Now try with the provider owned memory binding to GetVisibleData
				//This will either succeed or return an error if not supported				
				m_hr = GetLastVisibleData(hRow, hChgAccessor, m_pVisibleData,FALSE); 
				if (m_hr != S_OK)
				{
					CHECK(m_hr, DB_E_BADACCESSORHANDLE);
				}

				fResults = TRUE;

				//Now clean up our row handle
				m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
			}

			m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
		}
		
		PROVIDER_FREE(rgBindings);
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_14()
{	
	HROW	hRow			= DB_NULL_HROW;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Note, provider not required to check for this return code
	if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_BADROWHANDLE))
	{
		m_cRowsResynched	= 5;
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite,&m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))								
		{
			COMPARE(m_cRowsResynched,1);
			COMPARE(m_rghRowsResynched[0],DB_NULL_HROW);
			//Increment error count as needed while checking out param values
			CompareOutParams(1, &hRow);						
			COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_INVALID);

			FreeOutParams();
			return TEST_PASS;
		}
		FreeOutParams();
	}			
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = hard deleted - DB_E_DELETEDROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_15()
{
	BOOL	fResults	= FALSE;
	HROW	hRow		= DB_NULL_HROW;
	BOOL	fOverWrite	= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Delete the row with this txn
	if (COMPARE(Delete(m_pChgRowset1, &hRow), TRUE))
	{
		m_cRowsResynched	= 5;
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		//Check that row status is deleted
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))
		{		
			COMPARE(m_rghRowsResynched[0],hRow);
			COMPARE(m_cRowsResynched,1);
			CompareOutParams(1, &hRow);
					
			if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
			{
				//if there is no visual cache GetLastVisibleData has
				//to go to the back end.
				//if there is a visual cache GetLastVisibleData sees the delete anyway
				//because it was a hard delete
				if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), DB_E_DELETEDROW))
				{
					fResults = TRUE;
				}
			}
			FreeOutParams();
		}
	}
	//Release the row
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);	
	}
	//We want to release our rowset so the deleted hRow we introduced
	//won't be around for subsequent calls to GetNextRows (which should 
	//cause Resynch methods to fail with DELETED ROW)
	ReleaseResynchObjects();
	//Set up for next variation
	COMPARE(CreateResynchObjects(m_eTI), TRUE);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_16()
{
	HROW	hRow		= DB_NULL_HROW;
	BOOL	fResults	= FALSE;
	BOOL	fOverWrite	= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Get row so that provider has it in cache before we change it from another txn
	if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))			
	{
		//Delete the row using another txn
		if (COMPARE(Delete(m_pChgRowset2), TRUE))
		{		
			//Give Jet time to discover the change
			if (m_fOnAccess)
				Sleep(SLEEP_TIME);	//Takes milliseconds as param

			m_cRowsResynched	= 5;
			m_rghRowsResynched	= (HROW *)JUNK_PTR;
			m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))
			{
				COMPARE(m_rghRowsResynched[0],hRow);
				COMPARE(m_cRowsResynched,1);
				CompareOutParams(1, &hRow);

				if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
				{
					//if there is no visual cache GetLastVisibleData has
					//to go to the back end.
					if (m_eTI==TI_IRowsetResynch || !g_fVisualCache)
					{
						if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_DELETEDROW))
						{
							fResults = TRUE;
						}
					}
					else
					{
						//if this is Refresh and there is a visual cache
						//GetLastVisibleData will succeed. RefreshVisibleData failed,
						//no row was refreshed.  GetLastVisibleData gets the current
						//visual cache row value
						if (g_fVisualCache)
						{
							if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), S_OK))
							{
								fResults = TRUE;
							}
						}
					}
				}
				FreeOutParams();
			}
		}
	}
	//Release the row
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	//We want to release our rowset so the deleted hRow we introduced
	//won't be around for subsequent calls to GetNextRows (which should 
	//cause Resynch methods to fail with DELETED ROW)
	ReleaseResynchObjects();
	//Set up for next variation
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with pData = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_17()
{
	BOOL	fResults = FALSE;
	HROW	hRow = DB_NULL_HROW;
	
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Get valid hRow
	if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))
	{	
		//Try all valid params except pData = NULL
		if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, NULL,FALSE), E_INVALIDARG))
			fResults = TRUE;

		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
		
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with Null Accessor, pData = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_18()
{
	HROW		hRow			= DB_NULL_HROW;
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	ULONG		cRefCount		= 0;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0, NULL, 0, &hChgAccessor, NULL), S_OK))
	{
		if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, &hRow))
		{	
			/////////////////////////////////////////////////////////////////////
			//Use null accessor and NULL pData, so all these should get no data
			/////////////////////////////////////////////////////////////////////

			//Get data which should already be in the cache
			if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, NULL), S_OK))
			{
				//Resynch the cache	
				hr=ResynchRefresh(DB_NULL_HCHAPTER,1,&hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched,&m_rgRowStatus,FALSE);
				if (!hr==S_OK)
				{
					//account for the fact that some providers might delete/insert a row when asked to change it
					//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
					if (DB_S_ERRORSOCCURRED==hr || DB_E_ERRORSOCCURRED == hr) 
					{
						if (COMPARE(m_rgRowStatus[0],DBROWSTATUS_E_DELETED))
						{
							COMPARE(1,m_cRowsResynched);
							fResults	= TRUE;	
						}
					}
					goto CLEANUP;
				}
				else
				{
					CompareOutParams(1, &hRow);

					//Try to get new visible data
					if (CHECK(GetLastVisibleData(hRow, hChgAccessor,NULL,FALSE), S_OK))	
					{
						//Now try to get Newly Resynch'd Data
						if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, NULL), S_OK))
						{
							fResults = TRUE;
						}
					}
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, &cRefCount, NULL);		

	if (hChgAccessor != DB_NULL_HACCESSOR)
		if (CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK))
			hChgAccessor = DB_NULL_HACCESSOR;
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with Null Accessor, pData valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_19()
{
	HROW		hRow			= DB_NULL_HROW;
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	BYTE		*pCompareData	= NULL;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	pCompareData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pCompareData)
	{
		return TEST_FAIL;
	}
	memset(pCompareData,0,(size_t)m_cbRowSize);

	//Set buffers so we know if they've been touched
	memset(m_pRowsetData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchVisibleData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchRowsetData, 0, (size_t)m_cbRowSize);
	memset(pCompareData, 0, (size_t)m_cbRowSize);

	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0, NULL, 0, &hChgAccessor, NULL), S_OK))
	{
		if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, &hRow))
		{	
			/////////////////////////////////////////////////////
			//Use null accessor, so all these should get no data
			/////////////////////////////////////////////////////

			//Get data which should already be in the cache
			if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, m_pRowsetData), S_OK))
			{	
				//Resynch the cache	
				hr=ResynchRefresh(DB_NULL_HCHAPTER,1,&hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched,&m_rgRowStatus, FALSE);
				if (!hr==S_OK)
				{
					//account for the fact that some providers might delete/insert a row when asked to change it
					//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
					if (DB_S_ERRORSOCCURRED==hr || DB_E_ERRORSOCCURRED == hr) 
					{
						if (COMPARE(m_rgRowStatus[0],DBROWSTATUS_E_DELETED))
						{
							COMPARE(1,m_cRowsResynched);
							fResults	= TRUE;	
						}
					}
					goto CLEANUP;
				}
				else
				{
					if (g_fNOCHANGE)
					{
						//no change was made here, if the provider can handle it, expect it
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					}
					else
					{
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
					}

					FreeOutParams();

					//Try to get new visible data
					if (CHECK(GetLastVisibleData(hRow, hChgAccessor,m_pResynchVisibleData,FALSE), S_OK))	
						//Now try to get Newly Resynch'd Data
						if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, m_pResynchRowsetData), S_OK))
							//No of the buffers should have been touched, 0 == identical
							if (COMPARE(memcmp(m_pRowsetData, pCompareData, (size_t)m_cbRowSize), 0) &&
								COMPARE(memcmp(m_pResynchVisibleData, pCompareData, (size_t)m_cbRowSize), 0) &&
								COMPARE(memcmp(m_pResynchRowsetData, pCompareData, (size_t)m_cbRowSize), 0))
							{
								fResults = TRUE;
							}
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	PROVIDER_FREE(pCompareData);

	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);		

	if (hChgAccessor != DB_NULL_HACCESSOR)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK);
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Fetch Position
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_20()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	return TestFetchPosition(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Rows 1 and n
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_21()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}
	return TestRows1AndN(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc All Rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_22()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestAllRows(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with rghRows = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_23()
{		
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//All valid args except rghRows
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, NULL, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), E_INVALIDARG))
	{
		//Make sure all out parameters are zeroed/nulled out on error
		CheckOutParamsAreNulled();
		FreeOutParams();

		return TEST_PASS;
	}
	else
	{
		FreeOutParams();
		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_24()
{
	HROW		*phRows			= NULL;	
	BOOL		fResults		= FALSE;
	HROW		hSaveRow		= DB_NULL_HROW;
	DBCOUNTITEM	cRowsObtained	= 0;
	ULONG		i				= 0;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);

	//Get all the rows, ask for more than enough
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, NUM_ROWS * 3, &cRowsObtained, &phRows), DB_S_ENDOFROWSET))
	{
 		//Now make one of the elements invalid
		hSaveRow = phRows[cRowsObtained/2];
		phRows[cRowsObtained/2] = DB_NULL_HROW;
		
		//Resynch all the rows 
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,cRowsObtained, phRows, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_S_ERRORSOCCURRED))
		{
			CompareOutParams(cRowsObtained, phRows);
			
			//Verify status is correct for each element
			for (i=0; i<cRowsObtained; i++)
			{
				if (i == cRowsObtained /2)
					COMPARE(m_rgRowStatus[i], DBROWSTATUS_E_INVALID);
				else
				{
					if (g_fNOCHANGE)
					{
						//no change was made here, if the provider can handle it, expect it
						COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_NOCHANGE);
					}
					else
					{
						COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_OK);
					}
				}
			}
			fResults = TRUE;
		}

		//Move our munged row back so we can free whole array
		phRows[cRowsObtained/2] = hSaveRow;

		CHECK(m_pChgIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
		
		if (phRows)
		{
			PROVIDER_FREE(phRows);
		}
	}
	FreeOutParams();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_25()
{				
	HROW 	rgRows[NUM_ROWS*2];
	ULONG	i;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make all hRows invalid
	for (i=0; i< (NUM_ROWS*2); i++)
	{
		rgRows[i] = DB_NULL_HROW;
	}
	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
	//Try to Resynch all the invalid rows 
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,(NUM_ROWS*2), rgRows, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
	{
		for (i=0; i< (NUM_ROWS*2); i++)
		{
			COMPARE(m_rghRowsResynched[i],DB_NULL_HROW);
		}
		COMPARE(m_cRowsResynched,(NUM_ROWS*2));
		CompareOutParams((NUM_ROWS*2), rgRows);

		for (i=0; i<(NUM_ROWS*2); i++)
		{
			COMPARE(m_rgRowStatus[i], DBROWSTATUS_E_INVALID);
		}
		FreeOutParams();

		return TEST_PASS;
	}
	FreeOutParams();
	return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with cRows = 0 and no active hRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_26()
{
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	HROW * pJunkHRow = (HROW *)JUNK_PTR;

	//Make sure pJunkHrow is ignored with cRows is 0
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, pJunkHRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
	{
		//The three output params should be nulled/zeroed out
		CheckOutParamsAreNulled();		
		FreeOutParams();

		return TEST_PASS;
	}
		
	FreeOutParams();
	return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc IRowsetResynch with held hRows from different GetNextRows calls, cRows=0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_27()
{
	HROW		rghRow[2];	
	HROW		*phRows1		= &rghRow[0];	
	HROW		*phRows2		= &rghRow[1];	
	BOOL		fResults		= FALSE;	
	DBCOUNTITEM	cRowsObtained	= 0;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);
	//Get one row
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1), S_OK))
	{
		//Get another row
		if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2), S_OK))
		{				
			//Resynch all the rows from multiple GetNextRows calls
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
			{				
				//Even though we got the hRows in two calls,
				//we put consecutively in one array
				CompareOutParams(2, rghRow);

				if (g_fNOCHANGE)
					
				{				
					//no change was made here, if the provider can handle it, expect it
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_NOCHANGE);
				}
				else
				{
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
				}
				
				fResults = TRUE;								
			}
	
			//Release second row
			CHECK(m_pChgIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
		}

		//Release first row
		CHECK(m_pChgIRowset->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_28()
{
	HROW		rghRow[2];	
	HROW		*phRows1		= &rghRow[0];	
	HROW		*phRows2		= &rghRow[1];	
	BOOL		fResults		= FALSE;	
	DBCOUNTITEM	cRowsObtained	= 0;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);
	//Get one row
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1), S_OK))
	{
		//Get another row
		if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2), S_OK))
		{				
			//Resynch all the rows from multiple GetNextRows calls, using
			//exact hRows rather than cRows = 0
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,2, rghRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
			{			
				//Even though we got the hRows in two calls,
				//we put consecutively in one array
				CompareOutParams(2, rghRow);

				if (g_fNOCHANGE)
					
				{				
					//no change was made here, if the provider can handle it, expect it
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_NOCHANGE);
				}
				else
				{
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
				}

				FreeOutParams();
				
				fResults = TRUE;								
			}
			
			//Release second row
			CHECK(m_pChgIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
		}

		//Release first row
		CHECK(m_pChgIRowset->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Fetch Position with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_29()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestFetchPosition(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Rows 1 and n with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_30()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestRows1AndN(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc All Rows with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_31()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestAllRows(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc IRowsetResynch with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_32()
{	
	HROW	hRow			= DB_NULL_HROW;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Note, provider not required to check for this return code
	if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_BADROWHANDLE))
	{
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, NULL,&m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))								
		{
			if(COMPARE(m_rgRowStatus,(DBROWSTATUS *)JUNK_PTR),TRUE)
			{
				if(COMPARE(m_rghRowsResynched,(HROW *)JUNK_PTR),TRUE)
				{
					return TEST_PASS;
				}
			}
			FreeOutParams();
		}	
	}
	FreeOutParams();
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with held  cRows=0 and all params NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_33()
{
	HROW		rghRow[2];	
	HROW		*phRows1		= &rghRow[0];	
	HROW		*phRows2		= &rghRow[1];	
	BOOL		fResults		= FALSE;	
	DBCOUNTITEM	cRowsObtained	= 0;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);
	//Get one row
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1), S_OK))
	{
		//Get another row
		if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2), S_OK))
		{				
			//Resynch all the rows from multiple GetNextRows calls
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, NULL, NULL, NULL, FALSE), S_OK))
			{
				fResults = TRUE;								
			}	
			//Release second row
			CHECK(m_pChgIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
		}
		//Release first row
		CHECK(m_pChgIRowset->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
	}
				
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with one invalid hRow - no params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_34()
{
	HROW		*phRows			= NULL;	
	BOOL		fResults		= FALSE;
	HROW		hSaveRow		= DB_NULL_HROW;
	DBCOUNTITEM	cRowsObtained	= 0;
	ULONG		i				= 0;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);

	//Get all the rows, ask for more than enough
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, NUM_ROWS * 3, &cRowsObtained, &phRows), DB_S_ENDOFROWSET))
	{
 		//Now make one of the elements invalid
		hSaveRow = phRows[cRowsObtained/2];
		phRows[cRowsObtained/2] = DB_NULL_HROW;
		
		//Resynch all the rows 
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,cRowsObtained, phRows, fOverWrite, NULL, NULL, NULL, FALSE), DB_S_ERRORSOCCURRED))
		{
			fResults = TRUE;
		}
		
		//Move our munged row back so we can free whole array
		phRows[cRowsObtained/2] = hSaveRow;

		CHECK(m_pChgIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
		
		if (phRows)
		{
			PROVIDER_FREE(phRows);
		}
	}
		
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Hard inserted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_35()
{
	HROW		hRow			= DB_NULL_HROW;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	//Now try resynch methods on hard inserted row.  		
	if (m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow))
	{
		//g_ulLastActualInsert++;
		g_InsertIncrement();

		m_cRowsResynched	= 5;
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		//Now try resynch methods on inserted row
		hr = ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE);

		//have strong identity support?  can newly inserted rows be compared?
		if (m_fStrongIdentity)
		{
			COMPARE(S_OK,hr);
		}
		else
		{
			COMPARE(m_cRowsResynched,1);
			COMPARE(m_rghRowsResynched[0],hRow);
			CompareOutParams(1, &hRow);

			COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED);
		}

		hr = GetLastVisibleData(hRow, m_hChgAccessor,m_pResynchVisibleData,FALSE);
		
		//have strong identity support?  can newly inserted rows be compared?
		if (m_fStrongIdentity)
		{
			COMPARE(S_OK,hr);
		}
		else
		{
			COMPARE(DB_E_NEWLYINSERTED,hr);
		}
		
		//Next GetData 
		if (SUCCEEDED(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData)))
		{						
			fResults = TRUE;
		}
		//no change is made, these should be the same
		if(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY)!=TRUE)
		{
			goto CLEANUP;
		}
	}
CLEANUP:
	FreeOutParams();
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = hard deleted - no params - no op
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_36()
{
	BOOL		fResults		= FALSE;
	HROW		hRow			= DB_NULL_HROW;
	HROW		rghRow[1];	
	HROW		*phRows			= &rghRow[0];	
	DBCOUNTITEM	cRowsObtained	= 0;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	

	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows), S_OK))
	{
		//Release the row
		if (*phRows != DB_NULL_HROW)
		{
			m_pChgIRowset->ReleaseRows(1, phRows, NULL, NULL, NULL);	
		}
		//Delete the row
		if (COMPARE(Delete(m_pChgRowset1, &hRow), TRUE))
		{
			m_cRowsResynched	= 5;
			m_rghRowsResynched	= (HROW *)JUNK_PTR;
			m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
			//the row has been deleted, resynch deleted row
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))
			{
				COMPARE(m_cRowsResynched,1);
				//COMPARE(m_rghRowsResynched[0],hRow);
				if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
				{					
					if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_DELETEDROW))
					{
						fResults = TRUE;
					}					
				}
			}
		}
	}
	//free some mem				
	FreeOutParams();
	//Release the row
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);	
	}
	//We want to release our rowset so the deleted hRow we introduced
	//won't be around for subsequent calls to GetNextRows (which should 
	//cause Resynch methods to fail with DELETED ROW)
	ReleaseResynchObjects();
	//Set up for next variation
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with released HROW - DB_E_BADROWHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_37()
{
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK);

	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	//Change a row ourselves and keep hRow
	COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), &hRow), TRUE);

	m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);	
	
	//Now GetVisibleData on the released row
	CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE),DB_E_BADROWHANDLE);
		
	fResults = TRUE;

	COMPARE(EndTxns(TRUE), TRUE);	
	
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc SetData with all status IGNORE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_38()
{
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	HRESULT				hr;	
	BOOL				fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK);	
		
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	//Get row so that provider has it in cache before we change it from another txn
	COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE);

	//Now Get Data
	CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pRowsetData), S_OK);

	//Change a row with DBSTATUS_S_IGNORE as the status
	COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), NULL, TRUE), TRUE);

	//do a ResynchRows(0/NULL) to check what is brought in
	hr	= ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE);
	if (!hr==S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_DELETEDROW==hr)
		{
			fResults	= TRUE;	
			goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}

	if (m_cRowsResynched)
	{
		//Now Get Resynch'd Data
		CHECK(m_pChgIRowset->GetData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchRowsetData), S_OK);

		//Now GetVisibleData on the same row
		CHECK(GetLastVisibleData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchVisibleData,FALSE),S_OK);

		//no change is made, these should be the same
		COMPARE(CompareBuffer(m_pResynchVisibleData,m_pRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		COMPARE(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

		//Everything went OK
		fResults = TRUE;
	}
CLEANUP:
	FreeOutParams();

	COMPARE(EndTxns(TRUE), TRUE);	
	
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}



// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc SetData with all status DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_39()
{
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	HRESULT				hr;	
	BOOL				fOverWrite		= TRUE;
	ULONG				i				= 0;


	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK);
	
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	//Get row so that provider has it in cache before we change it from another txn
	COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE);

	//Now Get Data
	CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pRowsetData), S_OK);

	//Change a row with DBSTATUS_S_DEFAULT as the status
	COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), NULL, FALSE, TRUE), TRUE);

	//do a ResynchRows(0/NULL) to check what is brought in
	hr	= ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE);
	if (hr!=S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_ERRORSOCCURRED==hr)
		{
			COMPARE(1,m_cRowsResynched);
			COMPARE(m_rgRowStatus[0],DBROWSTATUS_E_DELETED);
			fResults	= TRUE;	
			goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}

	if (m_cRowsResynched)
	{
		if (fOverWrite)
		{
			//Now Get Resynch'd Data
			CHECK(m_pChgIRowset->GetData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchRowsetData), S_OK);
		}
		else
		{
			hr = m_pChgIRowset->GetData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchRowsetData);
			if (DB_E_ERRORSOCCURRED	==	hr || DB_S_ERRORSOCCURRED ==	hr)
			{
				for (i=0;i<m_cBindings;i++)
				{
//					if(	DBSTATUS_S_OK			!=	*(DBSTATUS *)((DWORD)m_pResynchRowsetData + m_rgBindings[i].obStatus)	&&
//						DBSTATUS_S_ISNULL		!=	*(DBSTATUS *)((DWORD)m_pResynchRowsetData + m_rgBindings[i].obStatus)	&&
//						DBSTATUS_E_UNAVAILABLE	!=	*(DBSTATUS *)((DWORD)m_pResynchRowsetData + m_rgBindings[i].obStatus))
					if	(	
							DBSTATUS_S_OK			!= STATUS_BINDING(m_rgBindings[i],m_pResynchRowsetData)	&&
							DBSTATUS_S_ISNULL		!= STATUS_BINDING(m_rgBindings[i],m_pResynchRowsetData)	&&
							DBSTATUS_E_UNAVAILABLE	!= STATUS_BINDING(m_rgBindings[i],m_pResynchRowsetData)	
						)
					{
						goto CLEANUP;
					}
				}
				fResults	= TRUE;
			}
			if (S_OK	!= hr)
			{
				goto CLEANUP;
			}
		}

		//Now GetVisibleData on the same row
		CHECK(GetLastVisibleData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchVisibleData,FALSE),S_OK);

		//no change is made, these should be the same
		COMPARE(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY,TRUE),TRUE);

		//Everything went OK
		fResults = TRUE;
	}
CLEANUP:
	FreeOutParams();

	COMPARE(EndTxns(TRUE), TRUE);	
	
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}



// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc InsertRows with all status DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynch::Variation_40()
{
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	BOOL				fOverWrite		= TRUE;
	ULONG				i				= 0;


	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK);	
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	//Insert new row with all DEFAULTS.  do not keep the hRow.
	//value shouldn't matter.  the function will fill the value part of the 
	//buffer with this seed but the function should use defaults instead.
	COMPARE(m_pChgRowset1->Insert(99, &hRow,TRUE), TRUE);

	//a GetData is useless since it is unknown whether a provider would go to the back end to resoluve the defauts

	if (m_fStrongIdentity)
	{			
		//Now do a ResynchRows(0/NULL) to check what is brought in
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE),S_OK);
	
		if (m_cRowsResynched)
		{
			//Get Resynch'd Data / should have the defualt since Resynch goes to the backend
			CHECK(m_pChgIRowset->GetData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchRowsetData), S_OK);

			for (i=0;i<m_cBindings;i++)
			{
//				if(DBSTATUS_S_OK!=*(DBSTATUS *)((DWORD)m_pResynchRowsetData + m_rgBindings[i].obStatus))
				if	(	
						DBSTATUS_S_OK	!= STATUS_BINDING(m_rgBindings[i],m_pResynchRowsetData)
					)
				{
					goto CLEANUP;
				}
			}

			//GetVisibleData on the same row
			CHECK(GetLastVisibleData(m_rghRowsResynched[0], m_hChgAccessor, m_pResynchVisibleData,FALSE),S_OK);

			//no change is made, these should be the same
			COMPARE(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

			//Everything went OK
			fResults = TRUE;
		}
	}
	else
	{
		//can't see the new row
		fResults = TRUE;
	}
CLEANUP:
	FreeOutParams();

	COMPARE(EndTxns(TRUE), TRUE);	
	
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
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
BOOL TCPropCanHoldRowsResynch::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CPropCanHoldRowsResynch::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropNoHoldRowsResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropNoHoldRowsResynch - Rowset without CANHOLDROWS
//|	Created:			06/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropNoHoldRowsResynch::Init()
{	
	DBPROPID		PropCanHoldRows = DBPROP_CANHOLDROWS;
	DBPROPOPTIONS	PropOption		= DBPROPOPTIONS_OPTIONAL;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Change the rowsets generated in CResynchRefresh::Init,
		//But only remove CANHOLDROWS, don't add any new props,
		//Make it setifcheap since if we can't set if off, the provider
		//apparently doesn't support removing it with the current
		//properties set, which is OK
		return ChangeProperties(0, NULL, 1, &PropCanHoldRows, &PropOption);
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read Only Rowset without DBPROP_CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropNoHoldRowsResynch::Variation_1()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestResynchRefresh(EREADONLY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Changeable Rowset without DBPROP_CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropNoHoldRowsResynch::Variation_2()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestResynchRefresh(ECHANGEABLE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropNoHoldRowsResynch::Terminate()
{	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropBookmarksResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropBookmarksResynch - Rowset with BOOKMARKS
//|	Created:			06/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropBookmarksResynch::Init()
{
	DBPROP			PropBkmks;
	DBPROPID		PropCanHoldRows = DBPROP_CANHOLDROWS;
	DBPROPOPTIONS	PropOption		= DBPROPOPTIONS_OPTIONAL;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	PropBkmks.dwPropertyID = DBPROP_BOOKMARKS;	
	PropBkmks.dwOptions = 0;
	PropBkmks.colid = DB_NULLID;
	PropBkmks.vValue.vt = VT_BOOL;
	V_BOOL(&(PropBkmks.vValue)) = VARIANT_TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Add bookmark property, remove Can Hold Rows
		return ChangeProperties(1, &PropBkmks, 1, &PropCanHoldRows, &PropOption);										

	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read Only Rowset with DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropBookmarksResynch::Variation_1()
{
	return TestResynchRefresh(EREADONLY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Changeable Rowset with DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropBookmarksResynch::Variation_2()
{
	return TestResynchRefresh(ECHANGEABLE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropBookmarksResynch::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropDeferredResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropDeferredResynch - Rowset with CANHOLDROWS and DEFERRED 
//|	Created:			06/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropDeferredResynch::Init()
{
	DBPROP	PropDeferred;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	PropDeferred.dwPropertyID		= DBPROP_DEFERRED;	
	PropDeferred.colid				= DB_NULLID;		//Set for all columns
	PropDeferred.dwOptions			= 0;
	PropDeferred.colid				= DB_NULLID;
	PropDeferred.vValue.vt			= VT_BOOL;
	V_BOOL(&(PropDeferred.vValue))	= VARIANT_TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Add deferred property, don't remove anything		
		m_fInitPass = ChangeProperties(1, &PropDeferred, 0, NULL, NULL);										
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read Only Rowset with DBPROP_DEFERRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropDeferredResynch::Variation_1()
{
	if (!m_fInitPass)
	{
		//if the init fails it is because the property couldn't not be set, this is not an error
		odtLog << L"DBPROP_DEFERRED not supported" << wszNewLine;
		return TEST_PASS;
	}

	//Test visible data on our read only rowset	-- we don't have
	//an isolation level, so pass CHAOS so it is ignored by this func
	if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor))
	{
		if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
		{
			FreeOutParams();
			return TEST_PASS;
		}
		FreeOutParams();
	}
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Changeable Rowset with DBPROP_DEFERRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropDeferredResynch::Variation_2()
{	
	if (!m_fInitPass)
	{
		//if the init fails it is because the property couldn't not be set, this is not an error
		odtLog << L"DBPROP_DEFERRED not supported" << wszNewLine;
		return TEST_PASS;
	}
	//Test visible data on our changeable rowset -- we don't have
	//an isolation level, so pass CHAOS so it is ignored by this func
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor))
	{
		if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
		{
			FreeOutParams();
			return TEST_PASS;
		}
		FreeOutParams();
	}
	return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropDeferredResynch::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropCacheDeferredResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropCacheDeferredResynch - Rowset with CACHEDEFERRED
//|	Created:			06/02/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropCacheDeferredResynch::Init()
{
	DBPROP	CacheDeferredProp;	

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	//CacheDeferred automatically implies deferred
	CacheDeferredProp.dwPropertyID = DBPROP_CACHEDEFERRED;	
	CacheDeferredProp.colid = DB_NULLID;		//Set for all columns
	CacheDeferredProp.dwOptions = 0;
	CacheDeferredProp.colid = DB_NULLID;
	CacheDeferredProp.vValue.vt = VT_BOOL;
	V_BOOL(&(CacheDeferredProp.vValue)) = VARIANT_TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Add deferred and cache deferred properties, don't remove anything
		m_fInitPass = ChangeProperties(1, &CacheDeferredProp, 0, NULL, NULL);										
		return TRUE;
	}
	return FALSE;

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read Only Rowset with DBPROP_CACHEDEFERRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCacheDeferredResynch::Variation_1()
{
	if (!m_fInitPass)
	{
		//if the init fails it is because the property couldn't not be set, this is not an error
		odtLog << L"DBPROP_CACHEDEFERRED not supported" << wszNewLine;
		return TEST_PASS;
	}
	//Cause some new visible data and retrieve it -- it should be cached
	if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, m_hROAccessor))
	{
		if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
		{
			//Resynch again should rewrite cached values, so try it once more
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, m_hROAccessor))
			{
				//Should see new stuff
				if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
				{
					FreeOutParams();
					return TEST_PASS;
				}
			}
		}
		FreeOutParams();
	}
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Changeable Rowset with DBPROP_CACHEDEFERRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCacheDeferredResynch::Variation_2()
{
	if (!m_fInitPass)
	{
		//if the init fails it is because the property couldn't not be set, this is not an error
		odtLog << L"DBPROP_CACHEDEFERRED not supported" << wszNewLine;
		return TEST_PASS;
	}
	//Cause some new visible data and retrieve it -- it should be cached
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor))
	{
		if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
		{
			//Resynch again should rewrite cached values, so try it once more
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor))
			{
				//Should see new stuff
				if (VerifyData(VERIFY_NEW,VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
				{
					FreeOutParams();
					return TEST_PASS;
				}
			}
		}
		FreeOutParams();
	}
	return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropCacheDeferredResynch::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropUpdateResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropUpdateResynch - Rowset with IRowsetUpdate
//|	Created:			06/02/96
//*-----------------------------------------------------------------------


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropUpdateResynch::Init()
{
	DBPROP	UpdateProp[2];	
	
	m_pChgIRowsetUpdate1 = NULL;
	m_pChgIRowsetUpdate2 = NULL;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	UpdateProp[0].dwPropertyID		= DBPROP_IRowsetChange;	
	UpdateProp[0].dwOptions			= 0;
	UpdateProp[0].colid				= DB_NULLID;
	UpdateProp[0].vValue.vt			= VT_BOOL;
	V_BOOL(&(UpdateProp[0].vValue)) = VARIANT_TRUE;

	UpdateProp[1].dwPropertyID		= DBPROP_IRowsetUpdate;	
	UpdateProp[1].dwOptions			= 0;
	UpdateProp[1].colid				= DB_NULLID;
	UpdateProp[1].vValue.vt			= VT_BOOL;
	V_BOOL(&(UpdateProp[1].vValue)) = VARIANT_TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Add IRowsetUpdate and don't remove anything
		if (ChangeProperties(2, UpdateProp, 0, NULL, NULL))
		{
			//If properties were set OK, we know IRowsetUpdate is supported
			if (VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetUpdate,ROWSET_INTERFACE, (IUnknown **)&m_pChgIRowsetUpdate1))
			{
				if (VerifyInterface(m_pChgRowset2->m_pIAccessor, IID_IRowsetUpdate,ROWSET_INTERFACE, (IUnknown **)&m_pChgIRowsetUpdate2))
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Soft deleted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_1()
{	
	HROW			hRow			= DB_NULL_HROW;
	HROW			hRowChanged		= DB_NULL_HROW;
	IRowsetChange	*pIRowsetChange = NULL;
	BOOL			fResults		= FALSE;
	DBROWSTATUS		PendingStatus;
	BOOL			fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If delete isn't supported this variation isn't applicable
	if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetChange,
		ROWSET_INTERFACE, (IUnknown **)&pIRowsetChange))
	{
		return TEST_SKIPPED;
	}
	if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, &hRow))
	{
		//Make permanent our update
		if (CHECK(m_pChgIRowsetUpdate2->Update(NULL, 0, NULL, NULL, NULL, NULL), S_OK))
		{		
			//Sleep for a few seconds; this is to ensure that the back end has had
			//time to make this update visible to other transactions which
			//should see it.  This is only necessary for Access, which only does
			//this every few seconds.
			if (m_fOnAccess)
			{
				Sleep(SLEEP_TIME);	//Takes milliseconds as param
			}
			//Soft delete the row we changed on second txn
			if (CHECK(pIRowsetChange->DeleteRows(NULL, 1, &hRow, NULL), S_OK))
			{	
				if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))
				{
					COMPARE(m_cRowsResynched,1);
					COMPARE(m_rghRowsResynched[0],hRow);
					if (!COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
					{
						fResults=TEST_FAIL;
						goto CLEANUP;
					}
					
					//Now try resynch methods on soft deleted row
					if (GetDataBuffers(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor, hRow))
					{
						if (g_fDeletedRow || g_fBlobFail)
						{
							//if this is true then hRow now points to a deleted row
							//buffers can not be filled from a deleted row
							//or
							//a BLOB column was in a row that was inserted and some providers have a hard time handling
							//BLOBs so they don't
							fResults=TRUE;
							goto CLEANUP;
						}
						//Status should now be Unchanged since we resynch'd 
						//and brought the deleted row back again
						if (CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingStatus), S_OK))
						{
							if (COMPARE(PendingStatus, DBPENDINGSTATUS_UNCHANGED))
							{
								//and everything should be the same as for a normal row
								if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_OLD, VERIFY_NEW))
								{
									fResults = TRUE;
								}
							}
						}
					}
				}
			}
		}
	}
CLEANUP:
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);
	
	FreeOutParams();
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	if (pIRowsetChange)
	{
		pIRowsetChange->Release();
		pIRowsetChange=NULL;
	}
	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Soft inserted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_2()
{
	HROW		hRow			= DB_NULL_HROW;
	BOOL		fResults		= FALSE;
	DBROWSTATUS PendingStatus;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	
	if (m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow))
	{
		m_cRowsResynched	= 5;
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		//Now do a ResynchRows, again this should be considered pending insert error
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
		{
			COMPARE(m_cRowsResynched,1);
			COMPARE(m_rghRowsResynched[0],hRow);
			CompareOutParams(1, &hRow);

			if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_PENDINGINSERT))
			{										
				if (CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow,&PendingStatus), S_OK))
				{
					if (COMPARE(PendingStatus, DBPENDINGSTATUS_NEW))
					{
						//Now try resynch methods on soft inserted row.  			
						//Soft insert has no row on server, should be considered an error
						if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor,m_pVisibleData,FALSE), DB_E_PENDINGINSERT))				
						{
							//Next GetData from cache, this may return DB_S_ERRORSOCCURRED if 
							//their are columns which the server has to update to complete the row
							if (SUCCEEDED(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pRowsetData)))
							{			
								fResults = TRUE;
							}
						}
					}
				}
				FreeOutParams();
			}
		}
	}
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Soft changed row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_3()
{	
	HROW			hRow			= DB_NULL_HROW;
	HROW			hRowChanged		= DB_NULL_HROW;
	BOOL			fResults		= FALSE;
	DBROWSTATUS		PendingStatus;
	BOOL			fOverWrite		= TRUE;

	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	
	//Change a row from underneath but keep the handle to
	//the old value still in the rowset cache
	if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1,ISOLATIONLEVEL_CHAOS, &hRow))//change 1 into 16 on the backend
	{
		//Make permanent our update
		if (CHECK(m_pChgIRowsetUpdate2->Update(NULL, 0, NULL, NULL, NULL, NULL), S_OK))
		{	
			//Sleep for a few seconds; this is to ensure that the back end has had
			//time to make this update visible to other transactions which
			//should see it.  This is only necessary for Access, which only does
			//this every few seconds.
			if (m_fOnAccess)
			{
				Sleep(SLEEP_TIME);	//Takes milliseconds as param
			}
			//Now change the row in the current rowset cache which has already been
			//changed on the server.  Use a new insert value that doesn't exist
			//in the row cache or on the server.  Note we are in delayed mode so
			//this change will never propagate to the server and GetNextRowToInsert count
			//won't increment.
			//We already have the hRow, so don't bother to keep another handle to it
			if (m_pChgRowset1->Change((GetNextRowToDelete()-1), GetNextRowToInsert(), NULL))//change 1 into 17
			{
				//Now try resynch methods on soft change row
				if (GetDataBuffers(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, m_hChgAccessor, hRow))
				{
					//if there is a visual cache lets see the change from rowset1.
					if (g_fVisualCache	&&	(m_eTI!=TI_IRowsetResynch))
					{
						//GetLastVsibleData does not overwrtie the visual cache
						fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
									m_pChgRowset1->m_rgTableColOrds, (GetNextRowToDelete()-1),
									m_pVisibleData, m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
									m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
						//the rowset buffer will have this rowset's change
						fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
									m_pChgRowset1->m_rgTableColOrds, GetNextRowToInsert(), 
									m_pRowsetData, m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
									m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
					}
					//if this is true then hRow now points to a deleted row
					//buffers can not be filled from a deleted row
					if (g_fDeletedRow)
					{
						fResults=TRUE;
						goto CLEANUP;
					}
					//if there is no visual cache lets see the change from rowset 2 from GetXVisibleData
					if (!g_fVisualCache)
					{
						//Visible data should be the new stuff as well
						fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
									m_pChgRowset1->m_rgTableColOrds, (GetNextRowToInsert()-1), 
									m_pVisibleData, m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
									m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
					}

					//Status should be unchanged since we resynch'd it (even
					//though data changed)
					if (CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingStatus), S_OK))
					{
						if (COMPARE(PendingStatus, DBPENDINGSTATUS_UNCHANGED))				
						{
							if (fOverWrite)
							{
								//Refresh TRUE should get new values.
								fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
									m_pChgRowset1->m_rgTableColOrds, (GetNextRowToInsert()-1), m_pResynchRowsetData, 
									m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
									m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
							}
							else
							{
								//Resynching should move everything to the new values.
								//Note we don't check the buffer m_pRowsetData which just
								//contains the values we set before calling resynch.  This
								//scenario is tested in Update. 
								fResults = COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
									m_pChgRowset1->m_rgTableColOrds, (GetNextRowToInsert()), m_pResynchRowsetData, 
									m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
									m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
							}
		
						}
					}
				}
			}
		}
	}
CLEANUP:
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	FreeOutParams();
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Inserted row after Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_4()
{
	HROW		hRow			= DB_NULL_HROW;
	BOOL		fResults		= TEST_FAIL;	
	DBROWSTATUS PendingStatus;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//Soft insert the row
	if (m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow))
	{	
		//Update the row to make it permanent
		if (CHECK(m_pChgIRowsetUpdate1->Update(NULL, 1, &hRow, NULL, NULL, NULL), S_OK))
		{
			//Sleep for a few seconds; this is to ensure that the back end has had
			//time to make this update visible to other transactions which
			//should see it.  This is only necessary for Access, which only does
			//this every few seconds.
			if (m_fOnAccess)
			{
				Sleep(SLEEP_TIME);	//Takes milliseconds as param
			}
			//Record that we just updated
			g_InsertIncrement();

			m_cRowsResynched	= 5;
			m_rghRowsResynched	= (HROW *)JUNK_PTR;
			m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
			//Now try resynch methods on inserted row -- this fails if we don't 
			//have strong identity support
			if (!m_fStrongIdentity)
			{
				//Now do a ResynchRows, this should be considered an error
				if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), DB_E_ERRORSOCCURRED))
				{
					COMPARE(m_cRowsResynched,1);
					COMPARE(m_rghRowsResynched[0],hRow);
					CompareOutParams(1, &hRow);

					if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED))
					{									
						if (CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingStatus), S_OK))
						{
							if (COMPARE(PendingStatus, DBPENDINGSTATUS_UNCHANGED))
							{
								if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_NEWLYINSERTED))				
								{
									fResults = TRUE;
								}
							}
						}
					}
					FreeOutParams();
				}
			}
			else
			{
				//Now do a ResynchRows
				if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite,&m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus,FALSE), S_OK))
				{
					COMPARE(m_cRowsResynched,1);
					COMPARE(m_rghRowsResynched[0],hRow);
					CompareOutParams(1, &hRow);

					if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK))
					{									
						if (CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingStatus), S_OK))
						{
							if (COMPARE(PendingStatus, DBPENDINGSTATUS_UNCHANGED))
							{
								if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), S_OK))				
								{
									//Visible data should be the new stuffeData,pCompareData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE) &&
									fResults =	COMPARE(CompareData(m_pChgRowset1->m_pTable->CountColumnsOnTable(), 
											m_pChgRowset1->m_rgTableColOrds, (GetNextRowToInsert()-1), 
											m_pVisibleData, m_cBindings, m_rgBindings, m_pChgRowset1->m_pTable, 
											m_pIMalloc, PRIMARY, COMPARE_ONLY), TRUE);
							
									fResults = TRUE;
								}
							}
						}
					}
					FreeOutParams();
				}
			}
		}
	}
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Pending change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_5()
{
	HROW			hRow				= DB_NULL_HROW;
	BOOL			fResults			= TEST_FAIL;	
	DBPENDINGSTATUS	PendingRowStatus	= (DBPENDINGSTATUS)JUNK_PTR;
	DBPENDINGSTATUS *rgPendingRow		= NULL;
	DBPENDINGSTATUS PendingStatusReq	= DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED;
	DBCOUNTITEM		uPRows				= 0;
	HROW			*rghPRow			= NULL;
	BOOL			fOverWrite			= TRUE;
	BYTE			*pPendingData		= NULL;
	BYTE			*pCacheData			= NULL;	


	pPendingData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pPendingData, 0, (size_t)m_cbRowSize);
	pCacheData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pCacheData, 0, (size_t)m_cbRowSize);
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//(Pending) Change a row ourselves and keep hRow
	COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), &hRow), TRUE);
		
	//get Pending data
	CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, pPendingData),S_OK);

	//Now do a ResynchRows 
	CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK);
	
	//if overwrite is true then this is Resync or fOverwrite in Refresh is TRUE
	//so there will be no pending status or pending rows
	if (fOverWrite)
	{
		CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingRowStatus), S_OK);

		//overwrite knocked off anything pending
		COMPARE(DBPENDINGSTATUS_UNCHANGED,PendingRowStatus);
		
		CHECK(m_pChgIRowsetUpdate1->GetPendingRows(NULL, PendingStatusReq, &uPRows, &rghPRow, &rgPendingRow), S_FALSE);
							
		COMPARE(0,uPRows);
		fResults = TEST_PASS;
	}
	//if fOverwrite is false there should be pending rows and pending row status
	else
	{
		CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingRowStatus), S_OK);
	
		//should still be pending
		COMPARE(DBPENDINGSTATUS_CHANGED,PendingRowStatus);
						
		//leaves the pending row
		CHECK(m_pChgIRowsetUpdate1->GetPendingRows(NULL, PendingStatusReq, &uPRows, &rghPRow, &rgPendingRow), S_OK);
							
		COMPARE(DBPENDINGSTATUS_CHANGED,rgPendingRow[0]);
								
		COMPARE(1,uPRows);

		//call update to xmit the changed row
		CHECK(m_pChgIRowsetUpdate1->Update(NULL, 1, &hRow, NULL, NULL, NULL), S_OK);
		
		g_DeleteAndInsertIncrement();	
		FreeOutParams();

		//call resynch/refresh again to make sure the changed row made it to the back end 
		//Now do a ResynchRows 
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK);
						
		//get resynched data
		CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, pCacheData),S_OK);

		//compare the data just read from the back end with the pending data, we should see the change
		COMPARE(CompareBuffer(pCacheData,pPendingData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
							
		fResults = TEST_PASS;
		FreeOutParams();
	}
		
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
		
	FreeOutParams();

	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	PROVIDER_FREE(pPendingData);
	PROVIDER_FREE(pCacheData);
	PROVIDER_FREE(rghPRow);
	PROVIDER_FREE(rgPendingRow);
	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_6()
{
	HROW			hRow				= DB_NULL_HROW;
	BOOL			fResults			= TEST_FAIL;	
	DBPENDINGSTATUS	PendingRowStatus	= (DBPENDINGSTATUS)JUNK_PTR;
	DBPENDINGSTATUS *rgPendingRow		= NULL;
	DBPENDINGSTATUS PendingStatusReq	= DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED;
	ULONG			uPRows				= 0;
	HROW			*rghPRow			= NULL;
	BOOL			fOverWrite			= TRUE;
	//data
	BYTE			*pOriginalData		= NULL;
	BYTE			*pResynchData		= NULL;
	BYTE			*pPendingData		= NULL;
	BYTE			*pVisibleData		= NULL;
	BYTE			*pCacheData1		= NULL;	
	HRESULT			hr;


	pOriginalData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pOriginalData, 0, (size_t)m_cbRowSize);
	pResynchData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pResynchData, 0, (size_t)m_cbRowSize);
	pPendingData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pPendingData, 0, (size_t)m_cbRowSize);
	pVisibleData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pVisibleData, 0, (size_t)m_cbRowSize);
	pCacheData1 = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	memset(pCacheData1, 0, (size_t)m_cbRowSize);

	g_fDeletedRow = FALSE;

	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//Change a row from underneath but keep the handle to
	//the old value still in the rowset cache
	if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1,ISOLATIONLEVEL_CHAOS, &hRow))
	{
		//get cache'd data off 1
		CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, pCacheData1),S_OK);

		//Make permanent our update from underneath
		CHECK(m_pChgIRowsetUpdate2->Update(NULL, 0, NULL, NULL, NULL, NULL), S_OK);

		//Now change the row in the current rowset cache which has already been
		//changed on the server.  Use a new insert value that doesn't exist
		//in the row cache or on the server.  Note we are in delayed mode so
		//this change will never propagate to the server and GetNextRowToInsert count
		//won't increment.
		//We already have the hRow, so don't bother to keep another handle to it
		if (m_pChgRowset1->Change((GetNextRowToDelete()-1), GetNextRowToInsert(), NULL))
		{
			//get pending data
			m_pChgIRowset->GetData(hRow, m_hChgAccessor, pPendingData);

			m_cRowsResynched	= 5;
			m_rghRowsResynched	= (HROW *)JUNK_PTR;
			m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
			//Now do a ResynchRows 
			hr = ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE);

			if (hr!=S_OK)
			{
				//account for the fact that some providers might delete/insert a row when asked to change it
				//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
				if (DB_E_ERRORSOCCURRED==hr)
				{
					COMPARE(m_cRowsResynched,1);
					COMPARE(m_rghRowsResynched[0],hRow);
					COMPARE(DBROWSTATUS_E_DELETED,m_rgRowStatus[0]);
					g_fDeletedRow	= TRUE;
				}
				else
				{
					FreeOutParams();
					goto CLEANUP;
				}
			}
			FreeOutParams();

			//get resynch'd data
			//for optimazation reasons, as long as this doesn't GPF practially anything can be returned here.
			m_pChgIRowset->GetData(hRow, m_hChgAccessor, pResynchData);

			//if this is resynch or there is no visual cache
			//GetXVisibleData will go to the back end
			if(TI_IRowsetResynch==m_eTI||!g_fVisualCache)
			{
				if (g_fDeletedRow)
				{
					//resynch goes to the back end.  it should see the deleted row if it is deleted
					CHECK(GetLastVisibleData(hRow,m_hChgAccessor,pVisibleData,FALSE),DB_E_DELETEDROW);
				}
				else
				{
					//resynch - go get the row 
					CHECK(GetLastVisibleData(hRow,m_hChgAccessor,pVisibleData,FALSE),S_OK);
				}
			}
			else
			{
				//if the row was deleted then refresh fails, no buffers are changed and GetLastVisibleData should work
				//if the row was not deleted this should then also work
				CHECK(GetLastVisibleData(hRow,m_hChgAccessor,pVisibleData,FALSE),S_OK);
			}

			//get original data (this is what the data first was, not what the back end is now)
			CHECK(m_pChgIRowsetUpdate1->GetOriginalData(hRow,m_hChgAccessor,pOriginalData),S_OK);

			//check the buffers
			if (!g_fDeletedRow)
			{
				if(fOverWrite)
				{
					//resynch should compare with the visible data since resynch overwrote the pending buffer
					COMPARE(CompareBuffer(pResynchData,pVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					//visible data and original should compare
					COMPARE(CompareBuffer(pVisibleData,pOriginalData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					//pending and resynch should not be the same if the resynch did  overwrite the pending buffer
					COMPARE(CompareBuffer(pPendingData,pResynchData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),FALSE);
					//first cache and original should NOT compare
					COMPARE(CompareBuffer(pCacheData1,pOriginalData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),FALSE);
				}
				else
				{
					//resynch should not compare with the visible data since resynch did not overwrite the pending buffer
					COMPARE(CompareBuffer(pResynchData,pVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),FALSE);
					//first cache and original should compare
					COMPARE(CompareBuffer(pCacheData1,pOriginalData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					//pending and resynch should be the same if the resynch did not overwrite the pending buffer
					COMPARE(CompareBuffer(pPendingData,pResynchData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
				}
			}
			//if refresh returned a deleted row then all calls after should be just like refresh was never called cause it failed.
			else
			{
				if(TI_IRowsetResynch==m_eTI || !g_fVisualCache)
				{
					//if the row was deleted and the interface was Resynch or there was no visual cache
					//then both Resych and GetVisibleData failed
					//so there are no buffers to check
				}
				else
				{
					//it doesn't matter what fOverwrite is here because refresh failed
							
					//visible data and original should compare if Refresh fails
					COMPARE(CompareBuffer(pVisibleData,pOriginalData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					//pending and resynch should be the same if Refresh fails
					COMPARE(CompareBuffer(pPendingData,pResynchData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
					//first cache and original should compare if Refresh fails
					COMPARE(CompareBuffer(pCacheData1,pOriginalData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
				}
			}

			fResults=TRUE;
		}
	}
CLEANUP:
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	PROVIDER_FREE(pOriginalData);
	PROVIDER_FREE(pResynchData);
	PROVIDER_FREE(pPendingData);
	PROVIDER_FREE(pVisibleData);
	PROVIDER_FREE(pCacheData1);
	PROVIDER_FREE(pOriginalData);

	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Pending insert
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_7()
{
	HROW			hRow				= DB_NULL_HROW;
	BOOL			fResults			= TEST_FAIL;	
	BOOL			fOverWrite			= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//(Pending) Insert a row ourselves and keep hRow
	if (m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow))
	{
		//Now do a ResynchRows 
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED);
			
		COMPARE(m_cRowsResynched,1);
		COMPARE(m_rghRowsResynched[0],hRow);
		COMPARE(DBROWSTATUS_E_PENDINGINSERT,m_rgRowStatus[0]);
		fResults	= TEST_PASS;
	}

	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	FreeOutParams();
	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Pending Delete
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_8()
{
	HROW			hRow				= DB_NULL_HROW;
	BOOL			fResults			= TEST_FAIL;	
	DBPENDINGSTATUS	PendingRowStatus	= (DBPENDINGSTATUS)JUNK_PTR;
	DBPENDINGSTATUS *rgPendingRow		= NULL;
	DBPENDINGSTATUS PendingStatusReq	= DBPENDINGSTATUS_NEW | DBPENDINGSTATUS_CHANGED | DBPENDINGSTATUS_DELETED;
	DBCOUNTITEM		uPRows				= 0;
	HROW			*rghPRow			= NULL;
	BOOL			fOverWrite			= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//(Pending) Delete a row ourselves and keep hRow
	if (m_pChgRowset1->Delete(GetNextRowToDelete(), &hRow))
	{
		//Now do a ResynchRows 
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK);

		//if overwrite is true then this is Resync or fOverwrite in Refresh is TRUE
		//so there will be no pending status or pending rows
		if (fOverWrite)
		{
			CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingRowStatus), S_OK);
					
			//overwrite knocked off anything pending
			COMPARE(DBPENDINGSTATUS_UNCHANGED,PendingRowStatus);
			CHECK(m_pChgIRowsetUpdate1->GetPendingRows(NULL, PendingStatusReq, &uPRows, &rghPRow, &rgPendingRow), S_FALSE);
			COMPARE(0,uPRows);
			fResults = TEST_PASS;
		}
		//if fOverwrite is false there should be pending rows and pending row status
		else
		{
			CHECK(m_pChgIRowsetUpdate1->GetRowStatus(NULL, 1, &hRow, &PendingRowStatus), S_OK);
					
			//should still be pending
			COMPARE(DBPENDINGSTATUS_DELETED,PendingRowStatus);
						
			//leaves the pending row
			CHECK(m_pChgIRowsetUpdate1->GetPendingRows(NULL, PendingStatusReq, &uPRows, &rghPRow, &rgPendingRow), S_OK);
			COMPARE(DBPENDINGSTATUS_DELETED,rgPendingRow[0]);
			COMPARE(1,uPRows);
			//call update to xmit the deleted row
			if (CHECK(m_pChgIRowsetUpdate1->Update(NULL, 1, &hRow, NULL, NULL, NULL), S_OK))
			{
				g_DeleteIncrement();
				FreeOutParams();
				//call resynch/refresh again to make sure the deleted row made it to the back end 
				//Now do a ResynchRows 
				CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED);
							
				COMPARE(m_cRowsResynched,1);
				COMPARE(m_rghRowsResynched[0],hRow);
				CompareOutParams(1, &hRow);

				if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
				{										
					fResults = TEST_PASS;
				}
			}
		}
	}
		
	FreeOutParams();
	//clean up any left over pending changes
	m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,NULL,NULL,NULL);
	m_pChgIRowsetUpdate2->Undo(NULL,0,NULL,NULL,NULL,NULL);

	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	PROVIDER_FREE(rghPRow);
	PROVIDER_FREE(rgPendingRow);
	if (fResults)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc insert, change and delete on a rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropUpdateResynch::Variation_9()
{
	IRowsetChange	*pIRowsetChange				= NULL;
	BOOL			fResults					= TEST_SKIPPED;
	BOOL			fOverWrite					= TRUE;
	DBCOUNTITEM		cRowsObtained				= 0;
	HROW			*pHRows						= NULL;
	DBCOUNTITEM		cRowsUpdated				= 0;
	DBCOUNTITEM		cRowsUndone					= 0;
	HRESULT			hr;

	BYTE			*pRowsetDataFirst			= NULL;	
	BYTE			*pRowsetDataLast			= NULL;	
	BYTE			*pRowsetDataNew				= NULL;	

	BYTE			*pVisibleDataFirst			= NULL;
	BYTE			*pVisibleDataLast			= NULL;
	BYTE			*pVisibleDataNew			= NULL;

	BYTE			*pResynchedRowsetDataFirst	= NULL;	
	BYTE			*pResynchedRowsetDataLast	= NULL;	
	BYTE			*pResynchedRowsetDataNew	= NULL;

	BYTE			*pResynchedVisibleDataFirst	= NULL;
	BYTE			*pResynchedVisibleDataLast	= NULL;
	BYTE			*pResynchedVisibleDataNew	= NULL;

	BYTE			*pChangeData				= NULL;	
	BYTE			*pInsertData				= NULL;	

	HROW			HRowOut;
	HROW			*pHRowOut					= &HRowOut;
	HROW			*pHRowUpdated				= NULL;


	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();

	//start a txn
	hr=m_pChgRowset1->m_pITxnLocal->StartTransaction(ISOLATIONLEVEL_CHAOS, 0, NULL, NULL);

	if (E_FAIL==hr)
	{
		goto CLEANUP;
	}
	if (S_OK!=hr)
	{
		fResults=TEST_FAIL;
		goto CLEANUP;
	}

	//alloc memory for the row buffers
	pRowsetDataFirst	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pRowsetDataLast		= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pRowsetDataNew		= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);

	pVisibleDataFirst	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pVisibleDataLast	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pVisibleDataNew		= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);

	pResynchedVisibleDataFirst	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchedVisibleDataLast	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchedVisibleDataNew	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);

	pResynchedRowsetDataFirst	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchedRowsetDataLast	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchedRowsetDataNew		= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);

	pChangeData	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pInsertData	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);

	if (!pRowsetDataFirst || !pRowsetDataLast || !pRowsetDataNew)
	{
		goto CLEANUP;
	}
	if (!pVisibleDataFirst || !pVisibleDataLast || !pVisibleDataNew)
	{
		goto CLEANUP;
	}
	if (!pResynchedVisibleDataFirst || !pResynchedVisibleDataLast || !pResynchedVisibleDataNew)
	{
		goto CLEANUP;
	}
	if (!pResynchedRowsetDataFirst || !pResynchedRowsetDataLast || !pResynchedRowsetDataNew)
	{
		goto CLEANUP;
	}
	if (!pChangeData || !pInsertData)
	{
		goto CLEANUP;
	}

	memset(pRowsetDataFirst,0,(size_t)m_cbRowSize);
	memset(pRowsetDataLast,0,(size_t)m_cbRowSize);
	memset(pRowsetDataNew,0,(size_t)m_cbRowSize);

	memset(pVisibleDataFirst,0,(size_t)m_cbRowSize);
	memset(pVisibleDataLast,0,(size_t)m_cbRowSize);
	memset(pVisibleDataNew,0,(size_t)m_cbRowSize);

	memset(pResynchedVisibleDataFirst,0,(size_t)m_cbRowSize);
	memset(pResynchedVisibleDataLast,0,(size_t)m_cbRowSize);
	memset(pResynchedVisibleDataNew,0,(size_t)m_cbRowSize);

	memset(pResynchedRowsetDataFirst,0,(size_t)m_cbRowSize);
	memset(pResynchedRowsetDataLast,0,(size_t)m_cbRowSize);
	memset(pResynchedRowsetDataNew,0,(size_t)m_cbRowSize);

	memset(pChangeData,0,(size_t)m_cbRowSize);
	memset(pInsertData,0,(size_t)m_cbRowSize);


	//Set data for all columns
	CHECK(FillInputBindings(	m_pChgRowset1->m_pTable, 
								DBACCESSOR_ROWDATA, 
								m_cBindings, 
								m_rgBindings, 
								&pChangeData, 
								g_ulLastActualInsert+1, 
								m_pChgRowset1->m_pTable->CountColumnsOnTable(),
								m_pChgRowset1->m_rgTableColOrds), S_OK);

	//Set data for all columns
	CHECK(FillInputBindings(	m_pChgRowset1->m_pTable, 
								DBACCESSOR_ROWDATA, 
								m_cBindings, 
								m_rgBindings, 
								&pInsertData, 
								g_ulLastActualInsert+2, 
								m_pChgRowset1->m_pTable->CountColumnsOnTable(),
								m_pChgRowset1->m_rgTableColOrds), S_OK);

	//If change isn't supported this variation isn't applicable
	if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IRowsetChange,ROWSET_INTERFACE, (IUnknown **)&pIRowsetChange))
	{
		return fResults;
	}

	fResults	= TEST_FAIL;

	//Get all the rows, ask for more than enough
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, NUM_ROWS * 3, &cRowsObtained, &pHRows), DB_S_ENDOFROWSET))
	{
		//get first and last row buffers
		CHECK(m_pChgIRowset->GetData(pHRows[0], m_hChgAccessor, pRowsetDataFirst), S_OK);
		CHECK(m_pChgIRowset->GetData(pHRows[cRowsObtained-1], m_hChgAccessor, pRowsetDataLast), S_OK);
	
		//change the first row
		CHECK(pIRowsetChange->SetData(pHRows[0], m_hChgAccessor, pChangeData), S_OK);

		//delete the last row
		CHECK(pIRowsetChange->DeleteRows(NULL, 1, &pHRows[cRowsObtained-1], NULL), S_OK);

		//insert a row
		CHECK(pIRowsetChange->InsertRow(NULL, m_hChgAccessor, pInsertData, &HRowOut), S_OK);

		//getLastVisibleData the row buffers
		CHECK(GetLastVisibleData(pHRows[0], m_hChgAccessor, pVisibleDataFirst,FALSE),S_OK);
		CHECK(GetLastVisibleData(pHRows[cRowsObtained-1], m_hChgAccessor, pVisibleDataLast,FALSE),S_OK);
		CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pVisibleDataNew,FALSE),DB_E_PENDINGINSERT);

		//Resynch all the rows except the newly pending inserted row
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,cRowsObtained, pHRows, fOverWrite, NULL, NULL, NULL, FALSE), S_OK);

		//getLastVisibleData the row buffers again
		CHECK(GetLastVisibleData(pHRows[0], m_hChgAccessor, pResynchedVisibleDataFirst,FALSE),S_OK);
		CHECK(GetLastVisibleData(pHRows[cRowsObtained-1], m_hChgAccessor, pResynchedVisibleDataLast,FALSE),S_OK);
		CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pResynchedVisibleDataNew,FALSE),DB_E_PENDINGINSERT);

		//get resyched data
		CHECK(m_pChgIRowset->GetData(pHRows[0], m_hChgAccessor, pResynchedRowsetDataFirst), S_OK);
		CHECK(m_pChgIRowset->GetData(pHRows[cRowsObtained-1], m_hChgAccessor, pResynchedRowsetDataLast), S_OK);
		hr	= m_pChgIRowset->GetData(pHRowOut[0], m_hChgAccessor, pResynchedRowsetDataNew);
		if (S_OK!=hr && DB_S_ERRORSOCCURRED!=hr)
		{
			goto CLEANUP;
		}

		if (fOverWrite)
		{
			//undo the insert since it is still pending
			CHECK(m_pChgIRowsetUpdate1->Undo(NULL,1,pHRowOut,NULL,NULL,NULL),S_OK);
			//call update and make sure no work gets done cause changes were resynched away
			CHECK(m_pChgIRowsetUpdate1->Update(NULL, 0, NULL, &cRowsUpdated, &pHRowUpdated, NULL), S_OK);
			m_pChgIRowset->ReleaseRows(cRowsUpdated, pHRowUpdated, NULL, NULL, NULL);
			PROVIDER_FREE(pHRowUpdated);
			if (cRowsUpdated)
			{
				goto CLEANUP;
			}
		}
		else
		{
			//lose these changes which are still here because pending changes should not be overwritten
			CHECK(m_pChgIRowsetUpdate1->Undo(NULL,0,NULL,&cRowsUndone,&pHRowUpdated,NULL),S_OK);
			m_pChgIRowset->ReleaseRows(cRowsUpdated, pHRowUpdated, NULL, NULL, NULL);
			PROVIDER_FREE(pHRowUpdated);
			if (cRowsUndone!=3)
			{
				goto CLEANUP;
			}
		}

		//compare buffers
		COMPARE(CompareBuffer(pRowsetDataFirst,pVisibleDataFirst,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		COMPARE(CompareBuffer(pRowsetDataLast,pVisibleDataLast,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

		COMPARE(CompareBuffer(pResynchedVisibleDataFirst,pVisibleDataFirst,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		COMPARE(CompareBuffer(pResynchedVisibleDataLast,pVisibleDataLast,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

		//if the pending row are not refreshed
		//they will have the changed values
		//otherwise they will have the old values
		if (!fOverWrite)
		{
			COMPARE(CompareBuffer(pResynchedRowsetDataFirst,pChangeData,m_cBindings,m_rgBindings,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE);
			//hard to compare a buffer from GetData the was a pending insert cause it can easily return rows as unavailable
			//COMPARE(CompareBuffer(pResynchedRowsetDataNew,pInsertData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		}
		else
		{
			COMPARE(CompareBuffer(pResynchedRowsetDataFirst,pVisibleDataFirst,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
			COMPARE(CompareBuffer(pResynchedRowsetDataLast,pVisibleDataLast,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		}
	
		//do the changes again. they should work cause they were killed in the resynch above unless they were pending
		//in which case they were undone
		m_pChgIRowset->ReleaseRows(1, pHRowOut, NULL, NULL, NULL);

		//change the first row
		CHECK(pIRowsetChange->SetData(pHRows[0], m_hChgAccessor, pChangeData), S_OK);

		//delete the last row
		CHECK(pIRowsetChange->DeleteRows(NULL, 1, &pHRows[cRowsObtained-1], NULL), S_OK);

		//insert a row
		CHECK(pIRowsetChange->InsertRow(NULL, m_hChgAccessor, pInsertData, &HRowOut), S_OK);

		//update - xmit these changes to the back end
		CHECK(m_pChgIRowsetUpdate1->Update(NULL, 0, NULL, &cRowsUpdated, &pHRowUpdated, NULL), S_OK);
		m_pChgIRowset->ReleaseRows(cRowsUpdated, pHRowUpdated, NULL, NULL, NULL);
		PROVIDER_FREE(pHRowUpdated);
		if (cRowsUpdated!=3)
		{
			goto CLEANUP;
		}

		//getLastVisibleData the row buffers
		CHECK(GetLastVisibleData(pHRows[0], m_hChgAccessor, pVisibleDataFirst,FALSE),S_OK);
		//always have to see own deletes no matter if there is a visual cache
		CHECK(GetLastVisibleData(pHRows[cRowsObtained-1], m_hChgAccessor, pVisibleDataLast,FALSE),DB_E_DELETEDROW);

		//can we see newly inserted rows
		if (m_fStrongIdentity)
		{
			CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pVisibleDataNew,FALSE),S_OK);
		}
		else
		{
			CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pVisibleDataNew,FALSE),DB_E_NEWLYINSERTED);
		}

		//Resynch all the rows except the newly pending inserted row
		//should error here on the deleted row
		CHECK(ResynchRefresh(DB_NULL_HCHAPTER,cRowsObtained, pHRows, fOverWrite, NULL, NULL, NULL, FALSE), DB_S_ERRORSOCCURRED);

		//getLastVisibleData the row buffers again
		CHECK(GetLastVisibleData(pHRows[0], m_hChgAccessor, pResynchedVisibleDataFirst,FALSE),S_OK);
		//always have to see own deletes no matter if there is a visual cache
		CHECK(GetLastVisibleData(pHRows[cRowsObtained-1], m_hChgAccessor, pVisibleDataLast,FALSE),DB_E_DELETEDROW);

		//can we see newly inserted rows
		if (m_fStrongIdentity)
		{
			CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pResynchedVisibleDataNew,FALSE),S_OK);
		}
		else
		{
			CHECK(GetLastVisibleData(pHRowOut[0], m_hChgAccessor, pResynchedVisibleDataNew,FALSE),DB_E_NEWLYINSERTED);
		}

		//get resyched data
		CHECK(m_pChgIRowset->GetData(pHRows[0], m_hChgAccessor, pResynchedRowsetDataFirst), S_OK);

		//compare buffers
		COMPARE(CompareBuffer(pVisibleDataFirst,pChangeData,m_cBindings,m_rgBindings,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE);
		COMPARE(CompareBuffer(pResynchedVisibleDataFirst,pVisibleDataFirst,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

		//no pending changes, fOverWrite won't matter here
		COMPARE(CompareBuffer(pResynchedRowsetDataFirst,pVisibleDataFirst,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);

		if (m_fStrongIdentity)
		{
			COMPARE(CompareBuffer(pVisibleDataNew, pInsertData,m_cBindings,m_rgBindings,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE);
			COMPARE(CompareBuffer(pResynchedVisibleDataNew,pInsertData,m_cBindings,m_rgBindings,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE);
		}

		fResults	= TEST_PASS;
	}
CLEANUP:
	//clean up any changes this variation made on the back end 
	if(m_pChgRowset1)
	{
		m_pChgRowset1->m_pITxnLocal->Abort(NULL,FALSE,FALSE);
	}
	if (m_pChgIRowset)
	{
		//release all the rows
		m_pChgIRowset->ReleaseRows(cRowsObtained, pHRows, NULL, NULL, NULL);
		PROVIDER_FREE(pHRows);
		m_pChgIRowset->ReleaseRows(1, pHRowOut, NULL, NULL, NULL);
	}
	
	//free the row buffers
	PROVIDER_FREE(pChangeData);
	PROVIDER_FREE(pInsertData);

	PROVIDER_FREE(pRowsetDataFirst);
	PROVIDER_FREE(pRowsetDataLast);
	PROVIDER_FREE(pRowsetDataNew);

	PROVIDER_FREE(pVisibleDataFirst);
	PROVIDER_FREE(pVisibleDataLast);
	PROVIDER_FREE(pVisibleDataNew);

	PROVIDER_FREE(pResynchedVisibleDataFirst);
	PROVIDER_FREE(pResynchedVisibleDataLast);
	PROVIDER_FREE(pResynchedVisibleDataNew);

	PROVIDER_FREE(pResynchedRowsetDataFirst);
	PROVIDER_FREE(pResynchedRowsetDataLast);
	PROVIDER_FREE(pResynchedRowsetDataNew);

	SAFE_RELEASE(pIRowsetChange);
	
	return fResults;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropUpdateResynch::Terminate()
{
	if (m_pChgIRowsetUpdate1)
	{
		m_pChgIRowsetUpdate1->Release();
		m_pChgIRowsetUpdate1 = NULL;
	}

	if (m_pChgIRowsetUpdate2)
	{
		m_pChgIRowsetUpdate2->Release();
		m_pChgIRowsetUpdate2 = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombieResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCZombieResynch - Rowset Preservation tests
//|	Created:			06/02/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc Tests zombie state (or lack thereof)
//
// @rdesc TRUE or FALSE
//
int TCZombieResynch::TestZombie(ETXN	eTxn, BOOL	fRetaining)
{
	IRowset			*pIRowset			= NULL;
	BOOL			fResults;	
	HROW			*phRow				= &m_hRow;
	ULONG			i					= 0;
	DBCOUNTITEM		cRowsObtained		= 0;	
	IRowsetResynch	*pIRowsetResynch	= NULL;
	IRowsetRefresh	*pIRowsetRefresh	= NULL;
	HROW			*rgTestHRow			= (HROW *)JUNK_PTR;
	DBROWSTATUS		*rgTestStatus		= (DBROWSTATUS *)JUNK_PTR;
	DBCOUNTITEM		cRowsResynched		= 0;

	if (m_eTI==TI_IRowsetResynch)
	{
		//Start a transaction.  Create a rowset with IRowsetResynch pointer.
		if(!StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, (IUnknown **)&pIRowsetResynch,1, &m_PropSet))
			goto CLEANUP;
	}
	else
	{
		//Start a transaction.  Create a rowset with IRowsetRefresh pointer.
		if(!StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, (IUnknown **)&pIRowsetRefresh,1, &m_PropSet))
			goto CLEANUP;
	}

	//Get an accessor
	if (!CHECK(GetAccessorAndBindings(m_pIRowset, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize), S_OK))	
		goto CLEANUP;

	//Get a big enough data buffer
	m_pData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!m_pData)
		goto CLEANUP;
	memset(m_pData,0,(size_t)m_cbRowSize);

	//Get an hRow 		
	if (!CHECK(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
		goto CLEANUP;
	
	//Test Commit
	if (eTxn == ECOMMIT)
	{
		if(!GetCommit(fRetaining))
			goto CLEANUP;

		if(!m_fCommitPreserve)
		{
			if (m_eTI==TI_IRowsetResynch)
			{
				//test zombie
				TESTC_(pIRowsetResynch->GetVisibleData(m_hRow, m_hAccessor, m_pData),E_UNEXPECTED);
				
				TESTC_(pIRowsetResynch->ResynchRows(1, &m_hRow, &cRowsResynched, &rgTestHRow, &rgTestStatus), E_UNEXPECTED);
			}
			else
			{
				if(m_eTI==TI_IRowsetRefreshTRUE)
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, TRUE, &cRowsResynched, &rgTestHRow, &rgTestStatus), E_UNEXPECTED);
				}
				else
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, FALSE, &cRowsResynched, &rgTestHRow, &rgTestStatus), E_UNEXPECTED);
				}
				//test zombie
				TESTC_(pIRowsetRefresh->GetLastVisibleData(m_hRow, m_hAccessor, m_pData),E_UNEXPECTED);
			}

			//Make sure all out parameters are zeroed/nulled out on error
			COMPARE(cRowsResynched, 0);
			COMPARE(rgTestHRow, NULL);
			COMPARE(rgTestStatus, NULL);
			fResults=TRUE;							
		}
		else
		{	
			if (m_eTI==TI_IRowsetResynch)
			{
				//test the rowset should be fully functional
				TESTC_(pIRowsetResynch->GetVisibleData(m_hRow, m_hAccessor, m_pData),S_OK);
		
				TESTC_(pIRowsetResynch->ResynchRows(1, &m_hRow, NULL, NULL, NULL), S_OK);				

				fResults=TRUE;			
			}
			else
			{		
				if (m_eTI==TI_IRowsetRefreshTRUE)
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, TRUE, NULL, NULL, NULL), S_OK);
				}
				else
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, FALSE, NULL, NULL, NULL), S_OK);
				}
				//test the rowset should be fully functional
				TESTC_(pIRowsetRefresh->GetLastVisibleData(m_hRow, m_hAccessor, m_pData),S_OK);
				fResults=TRUE;			
			}
		}				
	}	
	//Test Abort
	else
	{
		if(!GetAbort(fRetaining))
			goto CLEANUP;

		if(!m_fAbortPreserve)
		{
			if (m_eTI==TI_IRowsetResynch)
			{
				//test zombie
				TESTC_(pIRowsetResynch->GetVisibleData(m_hRow, m_hAccessor, m_pData),E_UNEXPECTED);
				
				TESTC_(pIRowsetResynch->ResynchRows(1, &m_hRow, NULL,&rgTestHRow, &rgTestStatus), E_UNEXPECTED);
			}
			else
			{				
				if (m_eTI==TI_IRowsetRefreshTRUE)
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, TRUE, NULL,&rgTestHRow, &rgTestStatus), E_UNEXPECTED);
				}
				else
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, FALSE, NULL,&rgTestHRow, &rgTestStatus), E_UNEXPECTED);
				}
				//test zombie
				TESTC_(pIRowsetRefresh->GetLastVisibleData(m_hRow, m_hAccessor, m_pData),E_UNEXPECTED);
			}
				
			//Add the bonus test of checking that rgTestHRow and 
			//rgTestStatus are untouched since pcRowsResynched is NULL
			//on the call above to ResynchRows
			COMPARE(rgTestHRow, (HROW *)JUNK_PTR);
			COMPARE(rgTestStatus, (DBROWSTATUS *)JUNK_PTR);

			fResults=TRUE;			
		}
		else
		{			
			if (m_eTI==TI_IRowsetResynch)
			{
				//test the rowset should be fully functional
				TESTC_(pIRowsetResynch->GetVisibleData(m_hRow, m_hAccessor, m_pData),S_OK);
				TESTC_(pIRowsetResynch->ResynchRows(1, &m_hRow, NULL,NULL, NULL), S_OK);
		
				fResults=TRUE;	
			}
			else
			{				
				if (m_eTI==TI_IRowsetRefreshTRUE)
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, TRUE, NULL,NULL, NULL), S_OK);
				}
				else
				{
					TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &m_hRow, FALSE, NULL,NULL, NULL), S_OK);
				}
				//test the rowset should be fully functional
				TESTC_(pIRowsetRefresh->GetLastVisibleData(m_hRow, m_hAccessor, m_pData),S_OK);

				fResults=TRUE;	
			}			
		}
	}
CLEANUP:
	//release the accessor
	if (m_hAccessor != DB_NULL_HACCESSOR)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	//and the memory
	if (m_rgBindings)
	{
		PROVIDER_FREE(m_rgBindings);
		m_rgBindings = NULL;
	}	
	if (m_pData)
	{
		PROVIDER_FREE(m_pData);
		m_pData = NULL;
	}
	
	//release the row handle 
	if(m_hRow != DB_NULL_HROW)
		CHECK(m_pIRowset->ReleaseRows(1, &m_hRow, NULL, NULL, NULL),S_OK);

	if(pIRowsetResynch)
	{
		pIRowsetResynch->Release();
		pIRowsetResynch=NULL;
	}
	if(pIRowsetRefresh)
	{
		pIRowsetRefresh->Release();
		pIRowsetRefresh=NULL;
	}
	//clean up.  Expected S_OK.
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);

	if(fResults)
		return TEST_PASS;
	else					  
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombieResynch::Init()
{
	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{                                                                           	           
		if (m_eTI==TI_IRowsetResynch)
		{
			//Build our property set for rowset properties		
			m_DBProp.dwPropertyID		= DBPROP_IRowsetResynch;
			m_DBProp.dwOptions			= 0;
			m_DBProp.colid				= DB_NULLID;
			m_DBProp.vValue.vt			= VT_BOOL;
			V_BOOL(&(m_DBProp.vValue))	= VARIANT_TRUE;

			m_PropSet.rgProperties		= &m_DBProp;
			m_PropSet.cProperties		= 1;
			m_PropSet.guidPropertySet	= DBPROPSET_ROWSET;

			//Init data members		
			m_hAccessor		= DB_NULL_HACCESSOR;
			m_hRow			= DB_NULL_HROW;
			m_cBindings		= 0;
			m_rgBindings	= NULL;
			m_cbRowSize		= 0;
			m_pData			= NULL;

			//register interface to be tested                                         
			if(RegisterInterface(ROWSET_INTERFACE, IID_IRowsetResynch, 1, &m_PropSet)) 	
				return TRUE;
		}
		else
		{
			//Build our property set for rowset properties		
			m_DBProp.dwPropertyID		= DBPROP_IRowsetRefresh;
			m_DBProp.dwOptions			= 0;
			m_DBProp.colid				= DB_NULLID;
			m_DBProp.vValue.vt			= VT_BOOL;
			V_BOOL(&(m_DBProp.vValue))	= VARIANT_TRUE;

			m_PropSet.rgProperties		= &m_DBProp;
			m_PropSet.cProperties		= 1;
			m_PropSet.guidPropertySet	= DBPROPSET_ROWSET;

			//Init data members		
			m_hAccessor		= DB_NULL_HACCESSOR;
			m_hRow			= DB_NULL_HROW;
			m_cBindings		= 0;
			m_rgBindings	= NULL;
			m_cbRowSize		= 0;
			m_pData			= NULL;

			//register interface to be tested                                         
			if(RegisterInterface(ROWSET_INTERFACE, IID_IRowsetRefresh, 1, &m_PropSet)) 	
				return TRUE;
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieResynch::Variation_1()
{		
	return TestZombie(ECOMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieResynch::Variation_2()
{
	return TestZombie(ECOMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieResynch::Variation_3()
{	
	return TestZombie(EABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieResynch::Variation_4()
{	
	return TestZombie(EABORT, FALSE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombieResynch::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrorsResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCExtendedErrorsResynch - Extended Errors
//|	Created:			07/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrorsResynch::Init()
{
	//Create an object for checking extended errors, which will use
	//m_pError to increment the error count as needed.
	//m_pExtError = new CExtError(m_pThisTestModule->m_ProviderClsid, m_pError);
	
	//if (!m_pExtError)
	//	return FALSE;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowsetResynch calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_1()
{
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW		hRow			= DB_NULL_HROW;
	HROW		*phRow			= &hRow;
	BOOL		fResults		= FALSE;
	DBPROPID	PropCanHoldRows = DBPROP_CANHOLDROWS;
	HRESULT		hr				= S_OK;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IRowsetResynch method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	//Get an hRow	
	if (CHECK(m_hr = m_pROIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
	{	
		//create an error object	
		m_pExtError->CauseError();

		if (CHECK(m_pROIRowset->GetData(hRow, m_hROAccessor, m_pRowsetData), S_OK))
		{
			//create an error object
			m_pExtError->CauseError();

			//call ResynchRows, try last three params as NULL for variety
			if (CHECK(hr=ResynchRefresh(DB_NULL_HCHAPTER,1, phRow, fOverWrite,NULL, NULL, NULL,TRUE), S_OK))		
			{
				if (m_eTI==TI_IRowsetResynch)
				{
					//Do extended check following ResynchRows
					fResults &= XCHECK(m_pROIRowsetResynch, IID_IRowsetResynch, hr);
				}
				else
				{
					//Do extended check following ResynchRows
					fResults &= XCHECK(m_pROIRowsetRefresh, IID_IRowsetRefresh, hr);
				}

				if (CHECK(m_pROIRowset->GetData(hRow, m_hROAccessor, m_pResynchRowsetData), S_OK))
				{
					//Call method GetVisibleData
					if (CHECK(GetLastVisibleData(*phRow,m_hROAccessor,m_pResynchVisibleData,TRUE), S_OK))		
					{
						if (m_eTI==TI_IRowsetResynch)
						{
							//Do extended check following GetVisibleData
							fResults = XCHECK(m_pROIRowsetResynch, IID_IRowsetResynch, hr);
						}
						else
						{
							//Do extended check following GetVisibleData
							fResults = XCHECK(m_pROIRowsetRefresh, IID_IRowsetRefresh, hr);
						}
						//There should be no new values found here, since we
						//didn't do a change on the other rowset, so expect
						//the first row in the rowset to always be returned
						//if (!VerifyData(VERIFY_OLD, VERIFY_OLD, VERIFY_OLD, GetNextRowToDelete()))
						if (!VerifyData(VERIFY_OLD, VERIFY_IGNORE, VERIFY_OLD, VERIFY_OLD, g_ulFirstRowInTable))
						{
							fResults = FALSE;
						}
					}
				}
			}
		}
	}	
	if (hRow != DB_NULL_HROW)
		m_pROIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetVisibleData call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_2()
{
	HROW		hRow			= DB_NULL_HROW;
	HROW		*phRow			= &hRow;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	DBCOUNTITEM	cRowsObtained	= 0;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the GetVisibleData method.
	//We then check extended errors to verify the right extended error behavior.
  
	//Get an hRow	
	if (CHECK(m_hr = m_pROIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
	{
		//create an error object
		m_pExtError->CauseError();

		//Call method with a NULL pData
		if (CHECK(hr=GetLastVisibleData(*phRow, NULL, m_pVisibleData,TRUE), DB_E_BADACCESSORHANDLE))	
		{
			if (m_eTI==TI_IRowsetResynch)
			{
				//Do extended check following GetVisibleData
				fResults = XCHECK(m_pROIRowsetResynch, IID_IRowsetResynch, hr);
			}
			else
			{
				//Do extended check following GetVisibleData
				fResults = XCHECK(m_pROIRowsetRefresh, IID_IRowsetRefresh, hr);
			}
		}
	}
	
	if (hRow != DB_NULL_HROW)
		m_pROIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid ResynchRows call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_3()
{	
	HRESULT	hr;
	BOOL	fTest		= FALSE;
	BOOL	fOverWrite	= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ResynchRows method.
	//We then check extended errors to verify the right extended error behavior.
  
  	//create an error object
	m_pExtError->CauseError();

	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;

	//All valid args except rghRows
	if (CHECK(hr=ResynchRefresh(DB_NULL_HCHAPTER,1, NULL, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus,FALSE), E_INVALIDARG))
	{
		//error so these have to be zero and NULL and NULL
		if (m_cRowsResynched || m_rghRowsResynched || m_rgRowStatus)
		{
			FreeOutParams();
			goto CLEANUP;
		}
		if(m_eTI==TI_IRowsetResynch)
		{
			//Do extended check following ResynchRows
			fTest = XCHECK(m_pChgIRowsetResynch, IID_IRowsetResynch, hr);
		}
		else
		{
			//Do extended check following ResynchRows
			fTest = XCHECK(m_pChgIRowsetRefresh, IID_IRowsetRefresh, hr);
		}
		FreeOutParams();
	}
CLEANUP:
	if(fTest)
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetResynch calls no with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_4()
{
	HROW	hRow		= DB_NULL_HROW;	
	HRESULT	hr;
	BOOL	fTest		= FALSE;
	BOOL	fOverWrite	= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetResynch method.
	//We then check extended errors to verify the right extended error behavior.
  
	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
		
	if (CHECK(hr=ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))								
	{
		COMPARE(m_cRowsResynched,1);
		COMPARE(m_rghRowsResynched[0],DB_NULL_HROW);
		CompareOutParams(1, &hRow);
			
		//Do extended check following ResynchRows
		if(m_eTI==TI_IRowsetResynch)
		{
			//Do extended check following ResynchRows
			fTest &= XCHECK(m_pChgIRowsetResynch, IID_IRowsetResynch, hr);
		}
		else
		{
			//Do extended check following ResynchRows
			fTest &= XCHECK(m_pChgIRowsetRefresh, IID_IRowsetRefresh, hr);
		}

		if (!COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_INVALID))
			fTest &= FALSE;	
	}
	//Note, provider not required to check for this return code
	if (CHECK(hr=GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData,FALSE), DB_E_BADROWHANDLE))					
	{
		//Do extended check following GetVisibleData
		if(m_eTI==TI_IRowsetResynch)
		{
			//Do extended check following ResynchRows
			fTest = XCHECK(m_pChgIRowsetResynch, IID_IRowsetResynch, hr);
		}
		else
		{
			//Do extended check following ResynchRows
			fTest = XCHECK(m_pChgIRowsetRefresh, IID_IRowsetRefresh, hr);
		}
	}
	FreeOutParams();

	if(fTest)
		return TRUE;
	else 
		return FALSE;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid ResynchRows - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_5()
{	
	HRESULT		hr;
	BOOL		fTest			= FALSE;
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW		hRow			= DB_NULL_HROW;
	HROW		*phRow			= &hRow;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//Get an hRow	
	if (CHECK(m_hr = m_pROIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow), S_OK))
	{	
		//pcRowsRefreshed was not a null pointer, and prghRowsRefreshed was a null pointer
		if (CHECK(hr=ResynchRefresh(DB_NULL_HCHAPTER,1, phRow, fOverWrite, &m_cRowsResynched,NULL, NULL, FALSE), E_INVALIDARG))
		{
			//error so these have to be zero and NULL and NULL
			if (m_cRowsResynched)
			{
				goto CLEANUP;
			}
			//Do extended check following ResynchRows
			if(m_eTI==TI_IRowsetResynch)
			{
				//Do extended check following ResynchRows
				fTest = XCHECK(m_pChgIRowsetResynch, IID_IRowsetResynch, hr);
			}
			else
			{
				//Do extended check following ResynchRows
				fTest = XCHECK(m_pChgIRowsetRefresh, IID_IRowsetRefresh, hr);
			}
		}
	}
CLEANUP:
	if (hRow != DB_NULL_HROW)
	{
		m_pROIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	if(fTest)
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}
// }}



// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pcRowsRefreshed - NULL, ignore prghRowsRefreshed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrorsResynch::Variation_6()
{	
	HRESULT	hr;
	ULONG	cRowsObtained	= 0;
	BOOL	fOverWrite		= TRUE;
	HROW	*pJunkHRow		= (HROW *)JUNK_PTR;	

	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//All valid args. prghRowsRefreshed is junk but is ignored when pcRowsRefreshed is NULL
	if (CHECK(hr=ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, NULL,&pJunkHRow, NULL, TRUE), S_OK))
	{
		return TRUE;
	}
	return FALSE;
}
// }}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrorsResynch::Terminate()
{
	//if (m_pExtError)
	//	delete m_pExtError;
	//m_pExtError = NULL;
	
	return(CResynchRefresh::Terminate());

}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPropResynchOnlyResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropResynchOnlyResynch - Rowsets with only IRowsetResynch Requested
//|	Created:			07/26/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropResynchOnlyResynch::Init()
{	
	const ULONG			cProps = 3;
	DBPROP				rgProps[cProps];
	DBPROPSET			DBPropSet;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	if (m_eTI==TI_IRowsetResynch)
	{
		rgProps[0].dwPropertyID			= DBPROP_IRowsetResynch;
		rgProps[0].dwOptions			= 0;
		rgProps[0].colid				= DB_NULLID;
		rgProps[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgProps[0].vValue))	= VARIANT_TRUE;
	}
	else
	{
		rgProps[0].dwPropertyID			= DBPROP_IRowsetRefresh;
		rgProps[0].dwOptions			= 0;
		rgProps[0].colid				= DB_NULLID;
		rgProps[0].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgProps[0].vValue))	= VARIANT_TRUE;
	}

	rgProps[1].dwPropertyID = DBPROP_IRowsetChange;
	rgProps[1].dwOptions = 0;
	rgProps[1].colid = DB_NULLID;
	rgProps[1].vValue.vt = VT_BOOL;
	V_BOOL(&(rgProps[1].vValue)) = VARIANT_TRUE;

	rgProps[2].dwPropertyID = DBPROP_UPDATABILITY;
	rgProps[2].dwOptions = 0;
	rgProps[2].colid = DB_NULLID;
	rgProps[2].vValue.vt = VT_I4;
	rgProps[2].vValue.lVal = DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	DBPropSet.rgProperties = rgProps;
	DBPropSet.cProperties = 1;	//Use only the first prop in the array for the first rowset
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;			

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CResynchRefresh::Init(m_eTI))
	// }}
	{
		//Release command objects so properties are wiped out
		ReleaseResynchObjects();
		m_pRORowset1->ReleaseCommandObject(); 
		m_pChgRowset1->ReleaseCommandObject();

		//Set only property IRowsetResynch for read only rowset
		m_pRORowset1->SetRowsetProperties(&DBPropSet, 1);
		//Add changeable props also for this rowset
		DBPropSet.cProperties = cProps;
		m_pChgRowset1->SetRowsetProperties(&DBPropSet, 1);

		return CreateResynchObjects(m_eTI);
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read only rowset with Resynch Only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropResynchOnlyResynch::Variation_1()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestResynchRefresh(EREADONLY);
}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Changeable rowset with Resynch Only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropResynchOnlyResynch::Variation_2()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestResynchRefresh(ECHANGEABLE);
}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropResynchOnlyResynch::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CResynchRefresh::Terminate());
}


// {{ TCW_TC_PROTOTYPE(TCPropCanHoldRowsResynchBLOBS)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPropCanHoldRowsResynchBLOBS - CanHoldRows testcases using BLOB data
//|	Created:			08/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropCanHoldRowsResynchBLOBS::Init()
{
	DBPROP	PropLocate;
	
	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	PropLocate.dwPropertyID			= DBPROP_IRowsetLocate;
	PropLocate.dwOptions			= DBPROPOPTIONS_REQUIRED;
	PropLocate.colid				= DB_NULLID;
	PropLocate.vValue.vt			= VT_BOOL;
	V_BOOL(&(PropLocate.vValue))	= VARIANT_TRUE;

	//Set this so that Init knows we want to try for blob columns
	m_fRequestLongDataIfSupported = TRUE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CPropCanHoldRowsResynch::Init(m_eTI))
	// }}
	{
		//Find highest isolation level
		if (m_fSerializable)
		{
			m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_SERIALIZABLE;
		}
		else
		{
			if (m_fRepeatableRead)
			{	
				m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_REPEATABLEREAD;
			}
			else
			{
				if (m_fReadCommitted)
				{
					m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_READCOMMITTED;	
				}
				else
				{
					if (m_fReadUncommitted)
					{
						m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_READUNCOMMITTED;
					}
					else
					{
						m_fHighestSupportedIsoLevel = ISOLATIONLEVEL_CHAOS;
					}
				}
			}
		}

		//Add IRowsetLocate as a requested property, this 
		//should cause our CreateResynchObjects routine
		//to always bind BLOBS, which is what we want to test
		return ChangeProperties(1, &PropLocate, 0, NULL, NULL, TRUE);				
	}
		
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Chaos
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_1()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos)
	{
	 	odtLog << wszNoProviderSupport << L"CHAOS" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
			
	//Start txn we're testing at Chaos Isolation Level
	if (CHECK(m_pRORowset1->m_pITxnLocal->StartTransaction(ISOLATIONLEVEL_CHAOS, 0, NULL, NULL), S_OK))	
	{
		if (CreateResynchObjects(m_eTI))
		{
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, m_hROAccessor))
			{
				//the rowset is ignore cause the blob might or might not forect the GetData to go to the back end
				if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_IGNORE,VERIFY_NEW))
				{	
					//Everything went OK
					fResults = TRUE;
				}
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Read Uncommitted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_2()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fReadUncommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READUNCOMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}	

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
		
	//Start txn we're testing at Read Uncommitted Isolation Level
	if (CHECK(m_pRORowset1->m_pITxnLocal->StartTransaction(ISOLATIONLEVEL_READUNCOMMITTED, 0, NULL, NULL), S_OK))	
	{
		if (CreateResynchObjects(m_eTI))
		{
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_READUNCOMMITTED, m_hROAccessor))
			{
				//the rowset is ignore cause the blob might or might not forect the GetData to go to the back end
				if (VerifyData(VERIFY_NEW,VERIFY_NEW,VERIFY_IGNORE,VERIFY_NEW))
				{	
					//Everything went OK
					fResults = TRUE;
				}
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level -  Read Committed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_3()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fReadCommitted)
	{
	 	odtLog << wszNoProviderSupport << L"READCOMMITTED" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Read Committed Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_READCOMMITTED), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_READCOMMITTED, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_READCOMMITTED, m_hROAccessor))
					{
						if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))
						{
							//Everything went OK
							fResults = TRUE;
						}
					}
				}
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level -  Repeatable Read
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_4()
{
	BOOL	fResults = FALSE;	
	
	//If no provider support for this isolation level, just return pass
	if (!m_fRepeatableRead)
	{
	 	odtLog << wszNoProviderSupport << L"REPEATABLEREAD" << wszNewLine;
	 	return TEST_PASS;

	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Read Repeated Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_REPEATABLEREAD), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Test visible data on our changeable rowset	
			//TODO:  Since Long data isn't necessarily transacted on the server
			//we should ignore the long data check the fixed data.  Need
			//to find out more details on how the server does this
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_REPEATABLEREAD, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_REPEATABLEREAD, m_hROAccessor))
					{
						if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))
						{	
							//Everything went OK
							fResults = TRUE;
						}
					}
				}				
			}
		}
		EndTxns();
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Serializable
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_5()
{
	BOOL	fResults = FALSE;	

	//If no provider support for this isolation level, just return pass
	if (!m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"SERIALIZABLE" << wszNewLine;
	 	return TEST_PASS;

	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Start txn we're testing at Serializable Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_SERIALIZABLE), S_OK))
	{
		if (CreateResynchObjects(m_eTI))
		{
			//Test visible data on our changeable rowset
			//TODO:  Since long data isn't transacted on the server,
			//should just ignore long data but still check fixed data.
			if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_SERIALIZABLE, m_hChgAccessor))		
			{
				if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))					
				{
					//Now do the same thing on our read only rowset
					if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_SERIALIZABLE, m_hROAccessor))
					{
						if (VerifyData(VERIFY_OLD,VERIFY_OLD,VERIFY_OLD,VERIFY_OLD))
						{	
							//Everything went OK
							fResults = TRUE;
						}
					}
				}				
			}
		}
		COMPARE(EndTxns(), TRUE);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Isolation Level - Unspecified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_6()
{
	BOOL	fResults = FALSE;	
	
	//Start txn we're testing at unspecified Isolation Level
	if (CHECK(StartTxns(ISOLATIONLEVEL_UNSPECIFIED), XACT_E_ISOLATIONLEVEL))
	{						
		//Everything went OK
		fResults = TRUE;	
	}

	//in case a txn was started
	COMPARE(EndTxns(), TRUE);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Own Insert - Highest Isolation Level
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_7()
{	
	BOOL				fResults		= FALSE;			
	HROW				hRow			= DB_NULL_HROW;	
	ULONG				cRowsObtained	= 0;
	HRESULT				ExpectedHr;
	BOOL				fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}
	
	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();	

	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own insert.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from inserting
	//due to the high isolation level
	if (!CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK))	
		goto CLEANUP;
	
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
		goto CLEANUP;	

	//Insert new row ourselves and keep hRow
	if (!COMPARE(m_pChgRowset1->Insert(GetNextRowToInsert(), &hRow), TRUE))
	{
		goto CLEANUP;
	}
	//increment for comparasion
	g_ulLastActualInsert++;

	//If we don't support strong identity, our single row will fail
	if (m_fStrongIdentity)
	{
		ExpectedHr = S_OK;
	}
	else
	{
		ExpectedHr = DB_E_ERRORSOCCURRED;
	}

	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;

	//Now do a ResynchRows to see if new data is brought in
	if (!CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), ExpectedHr))
	{	
		FreeOutParams();
		goto CLEANUP;
	}

	//This only applies if we support resynch on this newly inserted row
	//This only applies if we support resynch on this newly inserted row
	if (m_fStrongIdentity)
	{
		//Now Get Newly Resynch'd Data
		if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData), S_OK))
		{
			goto CLEANUP;
		}
		
		//Now GetVisibleData on the same row
		if (!CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), S_OK))
		{
			goto CLEANUP;
		}
		
		//refresh should bring in the defaults		

		//Now make sure we could see the new data at all times, since its our own insert
		if (VerifyData(VERIFY_NEW, VERIFY_IGNORE, VERIFY_IGNORE, VERIFY_NEW))
		{
			//Everything went OK
			fResults = TRUE;
		}
	}
	//Make sure our status is right
	else
	{
		COMPARE(m_cRowsResynched,1);
		COMPARE(m_rghRowsResynched[0],hRow);
		CompareOutParams(1, &hRow);
		fResults = COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED);
	}
	FreeOutParams();
	fResults	= TEST_PASS;
CLEANUP:
	if (5!=m_cRowsResynched)
	{
		FreeOutParams();
	}
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);

	//back out changes
	COMPARE(EndTxns(TRUE), TRUE);	
	g_ulLastActualInsert--;

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Variable Length Columns Only Bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_8()
{
	BOOL		fResults		= FALSE;	
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor		= DB_NULL_HACCESSOR;
	DBBINDING	*rgBindings		= NULL;
	DBCOUNTITEM	cBindings		= 0;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;	
	}
	//Get accessors with only variable length bound.  Bindings will be the same
	//for changeable and read only rowsets
	if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND,		
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}
	if (!CHECK(GetAccessorAndBindings(m_pRORowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hROAccessor, NULL, NULL, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND,			
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}
	//Test visible data on our changeable rowset	
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))
	{
		if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))
		{		
			//Now do the same thing on our read only rowset
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
			{
				if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
				{
					//Everything went OK
					fResults = TRUE;
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	if (hChgAccessor != DB_NULL_HACCESSOR)	
		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
		
	if (hROAccessor != DB_NULL_HACCESSOR)
		m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);

	if (rgBindings)
		PROVIDER_FREE(rgBindings);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Fixed Length Columns Only Bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_9()
{
	BOOL		fResults		= FALSE;	
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor		= DB_NULL_HACCESSOR;
	DBBINDING	*rgBindings		= NULL;
	DBCOUNTITEM	cBindings		= 0;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;
	}
	//Get accessors with only fixed length bound, bindings 
	//are the same for changeable and RO accessors
	if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, NULL,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		FIXED_LEN_COLS_BOUND,			
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))
	{
		goto CLEANUP;
	}
	//Test visible data on our changeable rowset	
	if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))
	{
		if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
		{
			ReleaseResynchObjects();
			if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
			{
				goto CLEANUP;		
			}
			if (!CHECK(GetAccessorAndBindings(m_pRORowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
				&hROAccessor, NULL, NULL, NULL,			
		 		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				FIXED_LEN_COLS_BOUND,			
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongData),S_OK))
			{
				FreeOutParams();
				goto CLEANUP;
			}			//Now do the same thing on our read only rowset
			if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
			{
				if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
				{
					//Everything went OK
					fResults = TRUE;
				}
			}
		}
	}
		
CLEANUP:
	FreeOutParams();
	if (rgBindings)
		PROVIDER_FREE(rgBindings);

	if (hChgAccessor != DB_NULL_HACCESSOR)
		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
	if (hROAccessor != DB_NULL_HACCESSOR)
		m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc All Columns Bound BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_10()
{
	BOOL		fResults = FALSE;	
	HACCESSOR	hChgAccessor = DB_NULL_HACCESSOR;
	HACCESSOR	hROAccessor = DB_NULL_HACCESSOR;
	ULONG		i;
	DBBINDING	*rgBindings = NULL;
	DBCOUNTITEM	cBindings	= 0;
	DBLENGTH	cRowSize	= 0;

	//Test visible data on our changeable rowset, repeat it several times
	//to flush out any potential memory leaks and stress test it
	for (i = 0; i< STRESS_RESYNCH_REPS; i++)
	{
		ReleaseResynchObjects();
		if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
		{
			goto CLEANUP;		
		}
		//Since some provider's only support variable length
		//cols by ref, limit the accessor to these
		if (!CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, 
			DBACCESSOR_ROWDATA,
			&hChgAccessor, &rgBindings, &cBindings, &cRowSize,			
			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF,
			NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongData),S_OK))
		{
			goto CLEANUP;
		}		
		//re-create buffers with the new accesor and bindings created just above
		FreeBuffers(m_cbRowSize);
		AllocDataBuffers(cRowSize);

		if (GenerateResynchData(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, hChgAccessor, rgBindings, cBindings))
		{
			if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
			{
				//re-re-create buffers so class functions work correctly.  yes, kind of hokey
				//free with Compare to take a care of byref value
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pVisibleData,									cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pRowsetData,									cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pResynchRowsetData,							cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);								
				CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
								m_pResynchVisibleData,							cBindings,							rgBindings,							
								m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
								FREE_ONLY
							);								
				AllocDataBuffers(m_cbRowSize);

				ReleaseResynchObjects();
				if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
				{
					FreeOutParams();
					goto CLEANUP;		
				}				
				if (!CHECK(GetAccessorAndBindings(m_pRORowset1->m_pIAccessor, 
					DBACCESSOR_ROWDATA,
					&hROAccessor, NULL, NULL, NULL,			
  					DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
					ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF,
					NULL, NULL,
					NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
					NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
					m_fBindLongData),S_OK))
				{
					FreeOutParams();
					goto CLEANUP;
				}
				//re-create buffers with the new accesor and bindings created just above
				FreeBuffers(m_cbRowSize);
				AllocDataBuffers(cRowSize);

				//Now do the same thing on our read only rowset
				if (GenerateResynchData(m_pRORowset1, ISOLATIONLEVEL_CHAOS, hROAccessor, rgBindings, cBindings))
				{
					if (VerifyData(VERIFY_NEW, VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, 0, 0, cBindings, rgBindings))					
					{	
						//Everything went OK
						fResults = TRUE;
						//re-re-create buffers so class functoins work correctly.  yes, kind of hokey
						//free with Compare to take a care of byref value
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pVisibleData,									cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pRowsetData,									cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pResynchRowsetData,							cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);								
						CompareData(	m_pChgRowset1->m_pTable->CountColumnsOnTable(),	m_pChgRowset1->m_rgTableColOrds,	1,
										m_pResynchVisibleData,							cBindings,							rgBindings,							
										m_pChgRowset1->m_pTable,						m_pIMalloc,							PRIMARY,
										FREE_ONLY
									);								
						AllocDataBuffers(m_cbRowSize);
						//free mem each time through the loop
						FreeAccessorBindings(cBindings,rgBindings);
						rgBindings	= NULL;
						if (hChgAccessor != DB_NULL_HACCESSOR)
						{
							m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
						}
						if (hROAccessor != DB_NULL_HACCESSOR)
						{
							m_pRORowset1->m_pIAccessor->ReleaseAccessor(hROAccessor, NULL);
						}
						continue;
					}			
				}
			}
		}
		break;
	}
	
	//If we broke before we finished loop, there was an error
	if (i != STRESS_RESYNCH_REPS)
	{
		fResults = FALSE;
	}
CLEANUP:
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Own Update - Highest Isolation Level
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_11()
{
	BOOL		fResults		= FALSE;			
	HROW		hRow			= DB_NULL_HROW;	
	ULONG		cRowsObtained	= 0;
	HRESULT		hr;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	//If no provider support for this isolation level, just return pass
	if (!m_fChaos && !m_fReadUncommitted && !m_fReadCommitted && 
		!m_fRepeatableRead && !m_fSerializable)
	{
	 	odtLog << wszNoProviderSupport << L"ANY" << wszNewLine;
	 	return TEST_PASS;
	}

	//Not all drivers allow starting a txn with rowsets
	//open, so release them before starting a txn
	ReleaseResynchObjects();
	
	//Set the highest possible isolation level, to make this test interesting
	//since regardless of isolation, we should always see our own change.
	//Note that we only start a txn for this rowset and not the RORowset,
	//since otherwise the RORowset could lock us from changing
	//due to the high isolation level
	if (!CHECK(m_pChgRowset1->m_pITxnLocal->StartTransaction(m_fHighestSupportedIsoLevel, 0, NULL, NULL), S_OK))	
	{
		goto CLEANUP;
	}
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		goto CLEANUP;
	}
	//Change a row ourselves and keep hRow
	if (!COMPARE(m_pChgRowset1->Change(GetNextRowToDelete(), GetNextRowToInsert(), &hRow), TRUE))
	{
		goto CLEANUP;
	}
	//increment for comparasion
	g_ulLastActualInsert++;

	//Next GetData from cache
	if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pRowsetData), S_OK))
	{
		goto CLEANUP;
	}
	//Now do a ResynchRows to see if new data is brought in
	if (!CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK))
	{
		goto CLEANUP;
	}
	//Now Get Newly Resynch'd Data
	if (!CHECK(m_pChgIRowset->GetData(hRow, m_hChgAccessor, m_pResynchRowsetData), S_OK))
	{
		goto CLEANUP;
	}
	//Now GetVisibleData on the same row
	hr=GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE);		
	if (hr!=S_OK)
	{
		//account for the fact that some providers might delete/insert a row when asked to change it
		//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
		if (DB_E_DELETEDROW==hr)
		{
			fResults	= TRUE;	
			goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}
	
	//no change is made, these should be the same
	if(CompareBuffer(m_pResynchVisibleData,m_pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY)!=TRUE)
	{
		goto CLEANUP;
	}

	//Now make sure we could see the new data at all times, since our own update
	if (VerifyData(VERIFY_NEW, VERIFY_IGNORE, VERIFY_NEW, VERIFY_IGNORE))					
	{	
		//Everything went OK
		fResults = TRUE;
	}
CLEANUP:
	FreeOutParams();
	
	//bakc out changes
	COMPARE(EndTxns(TRUE), TRUE);	
	g_ulLastActualInsert--;

	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with PASSBYREF  - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_12()
{
	HACCESSOR	hChgAccessor = DB_NULL_HACCESSOR;
	BOOL		fResults = FALSE;
	HROW		hRow = DB_NULL_HROW;

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//We will skip this variation is provider doesn't support PASSBYREF
	if (!m_fPassByRef)
	{
		odtLog << L"This Provider doesn't support PASSBYREF accessors. Variation is not applicable.\n";
		return TEST_PASS;
	}

	//Get a PASSBYREF accessor
	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(
		DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, m_cBindings,
		m_rgBindings, m_cbRowSize, &hChgAccessor, NULL), S_OK))
	{
		//Get an hRow
		if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))				
		{	
			//Any pass by ref accessor should fail
			if (CHECK(GetLastVisibleData(hRow, hChgAccessor, m_pVisibleData, FALSE), DB_E_BADACCESSORHANDLE))
				fResults = TRUE;
			//Now clean up our row handle
			m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
		}
		m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with PROVIDEROWNED  - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_13()
{
	HACCESSOR		hChgAccessor	= DB_NULL_HACCESSOR;
	BOOL			fResults		= FALSE;
	HROW			hRow			= DB_NULL_HROW;
	DBBINDING		*rgBindings		= NULL;
	DBCOUNTITEM		cBindings		= 0;
	DBLENGTH		cbRowSize		= 0;	
	DBCOLUMNINFO	*rgColInfo		= NULL;
	DBORDINAL		cCols			= 0;
	ULONG			i				= 0;
	ULONG			j				= 0;		
	OLECHAR			*pStringsBuffer = NULL;
	IColumnsInfo	*pIColInfo		= NULL;
	BOOL			fFoundNonLongCol= FALSE;
			
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Get bindings for a by ref accessor, use variable len only cols
	if (CHECK(GetAccessorAndBindings(m_pChgRowset1->m_pIAccessor, DBACCESSOR_ROWDATA,
		&hChgAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		VARIABLE_LEN_COLS_BOUND, FORWARD, ALL_COLS_BY_REF,			
		NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongData),S_OK))					
	{
		//We only wanted bindings
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK);
		hChgAccessor = DB_NULL_HACCESSOR;
		
		//Get the col info
		if (!VerifyInterface(m_pChgRowset1->m_pIAccessor, IID_IColumnsInfo, ROWSET_INTERFACE,(IUnknown **) &pIColInfo))
			goto CLEANUP;
	
		if (!CHECK(pIColInfo->GetColumnInfo(&cCols,	
			&rgColInfo,	&pStringsBuffer), S_OK))
			goto CLEANUP;
	
		//Make sure we use a non blob column; they are not allowed
		//for provider owned memory.
		for (i=0; i<cBindings; i++)
		{
			//Loop thru the colinfo...
			for (j=0; j<cCols; j++)
			{			
				 //...looking for the one which matches this binding
				if (rgBindings[i].iOrdinal == rgColInfo[j].iOrdinal)
				{
					//Find one that doesn't have LONG DATA
					if (!(rgColInfo[j].dwFlags & DBCOLUMNFLAGS_ISLONG))
					{
						rgBindings[i].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;

						//Mark that we found at least one non long col
						fFoundNonLongCol = TRUE;
					}
					//We found our colinfo for this binding, so stop looking
					break;					
				}
				//We only need one binding with provider owned memory
				if (fFoundNonLongCol)
					break;
			}		
			if (fFoundNonLongCol)
				break;
		}

		//We couldn't find a non long column to make provider owned
		if (!fFoundNonLongCol)
		{
			fResults = TRUE;	//We can't test anything in this var
			goto CLEANUP;
		}

		//Now create the accessor which has provider owned memory 
		if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings,
			rgBindings, cbRowSize, &hChgAccessor, NULL), S_OK))
		{
			//Get an hRow
			if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))				
			{	
				//Now try with the provider owned memory binding to GetVisibleData
				//This will either succeed or return an error if not supported				
				m_hr = GetLastVisibleData(hRow, hChgAccessor, m_pVisibleData, FALSE); 
				if (m_hr != S_OK)
					CHECK(m_hr, DB_E_BADACCESSORHANDLE);

				fResults = TRUE;

				//Now clean up our row handle
				m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
			}
			m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL);
		}
	}
CLEANUP:
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
		rgBindings = NULL;
	}

	if (rgColInfo)
	{
		PROVIDER_FREE(rgColInfo);
		rgColInfo = NULL;
	}

	if (pIColInfo)
	{
		pIColInfo->Release();
		pIColInfo = NULL;
	}
	
	if (pStringsBuffer)
	{
		PROVIDER_FREE(pStringsBuffer);
		pStringsBuffer = NULL;
	}
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = DB_NULL_HROW - DB_E_BADROWHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_14()
{	
	HROW	hRow		= DB_NULL_HROW;
	BOOL	fOverWrite	= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
		
	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;

	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
	{
		COMPARE(m_rghRowsResynched[0],DB_NULL_HROW);
		COMPARE(m_cRowsResynched,1);
		//Increment error count as needed while checking out param values
		CompareOutParams(1, &hRow);						
		COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_INVALID);
		FreeOutParams();

		//Note, provider not required to check for this return code
		if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pVisibleData, FALSE), DB_E_BADROWHANDLE))					
		{
			return TEST_PASS;
		}
	}
	FreeOutParams();
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc IRowsetResynch with hRow = hard deleted - DB_E_DELETEDROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_15()
{
	BOOL	fResults		= FALSE;
	HROW	hRow			= DB_NULL_HROW;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
		
	//Delete the row with this txn
	if (COMPARE(Delete(m_pChgRowset1, &hRow), TRUE))
	{
		m_cRowsResynched	= 5;
		m_rghRowsResynched	= (HROW *)JUNK_PTR;
		m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;

		//Check that row status is deleted
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
		{						
			COMPARE(m_rghRowsResynched[0],hRow);
			COMPARE(m_cRowsResynched,1);
			CompareOutParams(1, &hRow);
				
			if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
			{
				//if there is no visual cache GetLastVisibleData has
				//to go to the back end.
				//if there is a visual cache GetLastVisibleData sees the delete anyway
				//because it was a hard delete
				if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), DB_E_DELETEDROW))
				{
					fResults = TRUE;
				}
			}
			FreeOutParams();
		}
	}
	//Release the row
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);	
	}
	//We want to release our rowset so the deleted hRow we introduced
	//won't be around for subsequent calls to GetNextRows (which should 
	//cause Resynch methods to fail with DELETED ROW)
	ReleaseResynchObjects();
	//Set up for next variation
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with hRow = row deleted by another txn - DB_E_DELETEDROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_16()
{
	HROW	hRow			= DB_NULL_HROW;
	BOOL	fResults		= FALSE;
	BOOL	fOverWrite		= TRUE;
	HRESULT	hr;
	ULONG	i				= 0;


	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Get row so that provider has it in cache before we change it from another txn
	if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))			
	{
		//Delete the row using another txn
		if (COMPARE(Delete(m_pChgRowset2), TRUE))
		{
			if (m_fOnAccess)
				Sleep(SLEEP_TIME);	//Takes milliseconds as param

			m_cRowsResynched	= 5;
			m_rghRowsResynched	= (HROW *)JUNK_PTR;
			m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, &hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
			{	
				COMPARE(m_rghRowsResynched[0],hRow);
				COMPARE(m_cRowsResynched,1);
				CompareOutParams(1, &hRow);

				if (COMPARE(m_rgRowStatus[0], DBROWSTATUS_E_DELETED))
				{
					//if there is no visual cache GetLastVisibleData has
					//to go to the back end.
					if (m_eTI==TI_IRowsetResynch || !g_fVisualCache)
					{
						if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE), DB_E_DELETEDROW))
						{
							fResults = TRUE;
						}
					}
					else
					{
						//if this is Refresh and there is a visual cache
						//GetLastVisibleData will succeed. RefreshVisibleData failed,
						//no row was refreshed.  GetLastVisibleData gets the current
						//visual cache row value
						if (g_fVisualCache)
						{
							hr	= GetLastVisibleData(hRow, m_hChgAccessor, m_pResynchVisibleData,FALSE);

							if (DB_S_ERRORSOCCURRED==hr)
							{
								for (i=0;i<m_cBindings;i++)
								{
									//if there is a BLOB it will be UNAVAILABLE so allow for it as
									//a valid status for DB_S_ERRORSOCCURRED
									if	(
											*((BYTE *)m_pResynchVisibleData+(m_rgBindings[i]).obStatus)==DBSTATUS_E_UNAVAILABLE
										)
									{
										fResults	= TRUE;
									}
								}

							}

							if (S_OK==hr)
							{
								fResults = TRUE;
							}
						}
					}
				}
			}
			FreeOutParams();
		}	
	}
	//Release the row
	if (hRow != DB_NULL_HROW)
	{
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	//We want to release our rowset so the deleted hRow we introduced
	//won't be around for subsequent calls to GetNextRows (which should 
	//cause Resynch methods to fail with DELETED ROW)
	ReleaseResynchObjects();
	//Set up for next variation
	COMPARE(CreateResynchObjects(m_eTI), TRUE);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with pData = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_17()
{
	BOOL	fResults = FALSE;
	HROW	hRow = DB_NULL_HROW;
	
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Get valid hRow
	if (COMPARE(m_pChgRowset1->FindRow(GetNextRowToDelete(), &hRow), TRUE))
	{	
		//Try all valid params except pData = NULL
		if (CHECK(GetLastVisibleData(hRow, m_hChgAccessor, NULL, FALSE), E_INVALIDARG))
			fResults = TRUE;

		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
		

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with Null Accessor, pData = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_18()
{
	HROW		hRow			= DB_NULL_HROW;
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0, NULL, 0, &hChgAccessor, NULL), S_OK))
	{
		if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, &hRow))
		{
			/////////////////////////////////////////////////////////////////////
			//Use null accessor and NULL pData, so all these should get no data
			/////////////////////////////////////////////////////////////////////

			//Get data which should already be in the cache
			if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, NULL), S_OK))
			{	//Resynch the cache	
				hr=ResynchRefresh(DB_NULL_HCHAPTER,1,&hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched,&m_rgRowStatus, FALSE);
				if (!hr==S_OK)
				{
					//account for the fact that some providers might delete/insert a row when asked to change it
					//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
					if (DB_S_ERRORSOCCURRED==hr || DB_E_ERRORSOCCURRED==hr)
					{
						COMPARE(1,m_cRowsResynched);
						if (COMPARE(m_rgRowStatus[0],DBROWSTATUS_E_DELETED))
						{
							fResults	= TRUE;	
						}
					}
					goto CLEANUP;
				}
				else
				{
					CompareOutParams(1, &hRow);
					if (g_fNOCHANGE)
					{
						//no change was made here, if the provider can handle it, expect it
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					}
					else
					{
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
					}

					//Try to get new visible data
					if (CHECK(GetLastVisibleData(hRow, hChgAccessor,NULL, FALSE), S_OK))	
					{
						//Now try to get Newly Resynch'd Data
						if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, NULL), S_OK))
						{
							fResults = TRUE;
						}
					}
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);		

	if (hChgAccessor != DB_NULL_HACCESSOR)
		if (CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK))
			hChgAccessor = DB_NULL_HACCESSOR;
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData with Null Accessor, pData valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_19()
{
	HROW		hRow			= DB_NULL_HROW;
	HACCESSOR	hChgAccessor	= DB_NULL_HACCESSOR;
	BYTE		*pCompareData	= NULL;
	BOOL		fResults		= FALSE;
	HRESULT		hr;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	pCompareData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pCompareData)
	{
		return TEST_FAIL;
	}
	memset(pCompareData,0,(size_t)m_cbRowSize);

	//Set buffers so we know if they've been touched
	memset(m_pRowsetData, 0, (size_t)m_cbRowSize);
	memset(m_pVisibleData, 0, (size_t)m_cbRowSize);
	memset(m_pResynchRowsetData, 0, (size_t)m_cbRowSize);
	memset(pCompareData, 0, (size_t)m_cbRowSize);

	if (CHECK(m_pChgRowset1->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0, NULL, 0, &hChgAccessor, NULL), S_OK))
	{
		if (ChangeUnderlyingRowAndGetHrow(m_pChgRowset1, ISOLATIONLEVEL_CHAOS, &hRow))
		{	
			/////////////////////////////////////////////////////
			//Use null accessor, so all these should get no data
			/////////////////////////////////////////////////////

			//Get data which should already be in the cache
			if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, m_pRowsetData), S_OK))
			{
				//Resynch the cache	
				hr=ResynchRefresh(DB_NULL_HCHAPTER,1,&hRow, fOverWrite, &m_cRowsResynched,&m_rghRowsResynched,&m_rgRowStatus, FALSE);
				if (!hr==S_OK)
				{
					//account for the fact that some providers might delete/insert a row when asked to change it
					//this may put a hole in the rowset and put the newrow somewhere else in the rowset (most likley at the end of the rowset)
					if (DB_S_ERRORSOCCURRED==hr||DB_E_ERRORSOCCURRED==hr)
					{
						COMPARE(1,m_cRowsResynched);
						if (COMPARE(m_rgRowStatus[0],DBROWSTATUS_E_DELETED))
						{
							fResults	= TRUE;	
						}
					}
					goto CLEANUP;
				}
				else
				{				
					if (g_fNOCHANGE)
					{
						//no change was made here, if the provider can handle it, expect it
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					}
					else
					{
						COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
					}

					FreeOutParams();

					//Try to get new visible data
					if (CHECK(GetLastVisibleData(hRow, hChgAccessor,m_pResynchVisibleData, FALSE), S_OK))	
					{
						//Now try to get Newly Resynch'd Data
						if (CHECK(m_pChgIRowset->GetData(hRow, hChgAccessor, m_pResynchRowsetData), S_OK))
						{
							//No of the buffers should have been touched, 0 == identical
							if (COMPARE(CompareBuffer(m_pRowsetData,pCompareData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE) &&
								COMPARE(CompareBuffer(m_pResynchVisibleData,pCompareData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE) &&
								COMPARE(CompareBuffer(m_pResynchRowsetData,pCompareData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE))
							{
								fResults = TRUE;
							}
						}
					}
				}
			}
		}
	}
CLEANUP:
	FreeOutParams();
	PROVIDER_FREE(pCompareData);

	if (hRow != DB_NULL_HROW)
		m_pChgIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);		

	if (hChgAccessor != DB_NULL_HACCESSOR)
		CHECK(m_pChgRowset1->m_pIAccessor->ReleaseAccessor(hChgAccessor, NULL), S_OK);
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Fetch Position
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_20()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestFetchPosition(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);

}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Rows 1 and n
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_21()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestRows1AndN(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc All Rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_22()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestAllRows(m_pChgIRowset, m_pChgIRowsetResynch, m_pChgIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with rghRows = NULL - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_23()
{	
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//All valid args except rghRows
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,1, NULL, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), E_INVALIDARG))
	{
		//Make sure all out parameters are zeroed/nulled out on error
		CheckOutParamsAreNulled();
		FreeOutParams();
		return TEST_PASS;
	}
	else
	{
		FreeOutParams();
		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with one invalid hRow - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_24()
{
	HROW		*phRows			= NULL;	
	BOOL		fResults		= FALSE;
	HROW		hSaveRow		= DB_NULL_HROW;
	DBCOUNTITEM	cRowsObtained	= 0;
	ULONG		i				= 0;
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);

	//Get all the rows, ask for more than enough
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, NUM_ROWS * 3, &cRowsObtained, &phRows), DB_S_ENDOFROWSET))
	{
		//Now make one of the elements invalid
		hSaveRow = phRows[cRowsObtained/2];
		phRows[cRowsObtained/2] = DB_NULL_HROW;
		
		//Resynch all the rows 
		if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,cRowsObtained, phRows, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_S_ERRORSOCCURRED))
		{
			CompareOutParams(cRowsObtained, phRows);
			
			//Verify status is correct for each element
			for (i=0; i<cRowsObtained; i++)
			{
				if (i == cRowsObtained /2)
					COMPARE(m_rgRowStatus[i], DBROWSTATUS_E_INVALID);
				else
				{
					if (g_fNOCHANGE)
					{
						//no change was made here, if the provider can handle it, expect it
						COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_NOCHANGE);
					}
					else
					{
						COMPARE(m_rgRowStatus[i], DBROWSTATUS_S_OK);
					}
				}
			}
			
			fResults = TRUE;
		}
		

		//Move our munged row back so we can free whole array
		phRows[cRowsObtained/2] = hSaveRow;

		CHECK(m_pChgIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
		
		if (phRows)
		{
			PROVIDER_FREE(phRows);
		}
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with all invalid hRows - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_25()
{				
	HROW 	rgRows[(NUM_ROWS*2)];
	ULONG	i;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Make all hRows invalid
	for (i=0; i< (NUM_ROWS*2); i++)
		rgRows[i] = DB_NULL_HROW;
	
	m_cRowsResynched	= 5;
	m_rghRowsResynched	= (HROW *)JUNK_PTR;
	m_rgRowStatus		= (DBROWSTATUS *)JUNK_PTR;
	//Try to Resynch all the invalid rows 
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,(NUM_ROWS*2), rgRows, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), DB_E_ERRORSOCCURRED))
	{
		for (i=0; i< (NUM_ROWS*2); i++)
		{
			COMPARE(m_rghRowsResynched[i],DB_NULL_HROW);
		}
		COMPARE(m_cRowsResynched,(NUM_ROWS*2));
		CompareOutParams((NUM_ROWS*2), rgRows);

		for (i=0; i<(NUM_ROWS*2); i++)
		{
			COMPARE(m_rgRowStatus[i], DBROWSTATUS_E_INVALID);
		}
		FreeOutParams();

		return TEST_PASS;
	}
	FreeOutParams();
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows with cRows = 0 and no active hRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_26()
{
	HROW	*pJunkHRow		= (HROW *)JUNK_PTR;
	BOOL	fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Make sure pJunkHrow is ignored with cRows is 0
	if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, pJunkHRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK))
	{
		//The three output params should be nulled/zeroed out
		CheckOutParamsAreNulled();		
		FreeOutParams();
		return TEST_PASS;
	}
	FreeOutParams();
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows=0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_27()
{
	HROW		rghRow[2];	
	HROW		*phRows1		= &rghRow[0];	
	HROW		*phRows2		= &rghRow[1];	
	BOOL		fResults		= FALSE;	
	DBCOUNTITEM	cRowsObtained	= 0;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);
	//Get one row
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1), S_OK))
	{
		//Get another row
		if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2), S_OK))
		{				
			//Resynch all the rows from multiple GetNextRows calls
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,0, NULL, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK))
			{				
				//Even though we got the hRows in two calls,
				//we put consecutively in one array
				CompareOutParams(2, rghRow);

				if (g_fNOCHANGE)
				{
					//no change was made here, if the provider can handle it, expect it
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_NOCHANGE);
				}
				else
				{
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
				}
				fResults = TRUE;								
			}
			
			//Release second row
			CHECK(m_pChgIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
		}

		//Release first row
		CHECK(m_pChgIRowset->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc ResynchRows/RefreshVisibleData with held hRows from different GetNextRows calls, cRows exact
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_28()
{
	HROW		rghRow[2];	
	HROW		*phRows1		= &rghRow[0];	
	HROW		*phRows2		= &rghRow[1];	
	BOOL		fResults		= FALSE;	
	DBCOUNTITEM	cRowsObtained	= 0;	
	BOOL		fOverWrite		= TRUE;
	
	if (m_eTI==TI_IRowsetRefreshFALSE)
	{
		fOverWrite=FALSE;
	}

	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	//Make sure we are at beginning of rowset after other variations
	CHECK(m_pChgIRowset->RestartPosition(NULL), S_OK);
	//Get one row
	if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1), S_OK))
	{
		//Get another row
		if (CHECK(m_pChgIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2), S_OK))
		{				
			//Resynch all the rows from multiple GetNextRows calls, using
			//exact hRows rather than cRows = 0
			if (CHECK(ResynchRefresh(DB_NULL_HCHAPTER,2, rghRow, fOverWrite, &m_cRowsResynched, &m_rghRowsResynched, &m_rgRowStatus, FALSE), S_OK))
			{			
				//Even though we got the hRows in two calls,
				//we put consecutively in one array
				CompareOutParams(2, rghRow);
				if (g_fNOCHANGE)
				{
					//no change was made here, if the provider can handle it, expect it
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_NOCHANGE);
				}
				else
				{
					COMPARE(m_rgRowStatus[0], DBROWSTATUS_S_OK);
					COMPARE(m_rgRowStatus[1], DBROWSTATUS_S_OK);
				}
				fResults = TRUE;								
			}
			
			//Release second row
			CHECK(m_pChgIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);

		}

		//Release first row
		CHECK(m_pChgIRowset->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
	}
	FreeOutParams();
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Fetch Position with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_29()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestFetchPosition(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Rows 1 and n with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_30()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestRows1AndN(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc All Rows  with Read Only Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPropCanHoldRowsResynchBLOBS::Variation_31()
{
	ReleaseResynchObjects();
	if (!COMPARE(CreateResynchObjects(m_eTI), TRUE))
	{
		return FALSE;	
	}	
	return TestAllRows(m_pROIRowset, m_pROIRowsetResynch, m_pROIRowsetRefresh);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPropCanHoldRowsResynchBLOBS::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CPropCanHoldRowsResynch::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCNullRowResynch)
//*-----------------------------------------------------------------------
//|	Test Case:		TCNullRowResynch - Tests Resych with all NULLs
//|	Created:			10/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNullRowResynch::Init()
{
	const ULONG		cProps				= 3;
	DBPROP			rgNewDBProp[cProps];	
	DBPROPSET		NewPropSet;
	IRowsetChange	*pIRowsetChange		= NULL;
	BOOL			fResults			= TEST_FAIL;
	ULONG			j					= 0;	
	DBCOUNTITEM		cRowsObtained		= 0;
	HRESULT			hr;


	m_pITransactionLocal	= NULL;
	m_rgBindings			= NULL;
	m_cBindings				= 0;
	m_cbRowSize				= 0;
	m_hAccessor				= DB_NULL_HACCESSOR;
	m_pIRowset				= NULL;
	m_phRow					= &m_hRow;
	m_hRow					= DB_NULL_HROW;
	m_pData					= NULL;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetObject::Init())
	// }}
	{		
		//Get IRowset on the object
		if (!VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
								IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&m_pITransactionLocal))
		{
			goto CLEANUP;	
		}
		m_pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, NULL);

		//Set up our rowset properties
		rgNewDBProp[0].dwPropertyID = DBPROP_UPDATABILITY;
		rgNewDBProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[0].colid		= DB_NULLID;
		rgNewDBProp[0].vValue.vt	= VT_I4;
		rgNewDBProp[0].vValue.lVal	= DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE;

		if (m_eTI==TI_IRowsetResynch)
		{
			rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetResynch;
			rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
			rgNewDBProp[1].colid				= DB_NULLID;
			rgNewDBProp[1].vValue.vt			= VT_BOOL;
			V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
		}
		else
		{
			rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetRefresh;
			rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
			rgNewDBProp[1].colid				= DB_NULLID;
			rgNewDBProp[1].vValue.vt			= VT_BOOL;
			V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
		}

		rgNewDBProp[2].dwPropertyID			= DBPROP_IRowsetChange;
		rgNewDBProp[2].dwOptions			= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[2].colid				= DB_NULLID;
		rgNewDBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgNewDBProp[2].vValue))	= VARIANT_TRUE;
				
		NewPropSet.rgProperties		= rgNewDBProp;
		NewPropSet.cProperties		= cProps;
		NewPropSet.guidPropertySet	= DBPROPSET_ROWSET;
			
		SetRowsetProperties(&NewPropSet, 1);		

		//get a rowset
		CHECK(((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	SELECT_UPDATEABLE,
										IID_IRowset, 
										1, 
										&NewPropSet, 
										(IUnknown**)&m_pIRowset, 
										NULL, 
										NULL, 
										NULL, 
										0, 
										NULL,
										NULL),S_OK);	

		
		if (!VerifyInterface(m_pIRowset, IID_IAccessor, ROWSET_INTERFACE,(IUnknown **)&m_pIAccessor))
		{
			goto CLEANUP;
		}
		//create accessor on the rowset
		CHECK(GetAccessorAndBindings(	m_pIAccessor, 
										DBACCESSOR_ROWDATA,
										&m_hAccessor, 
										&m_rgBindings, 
										&m_cBindings,
										&m_cbRowSize,			
  										DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
										NULLABLE_COLS_BOUND	|UPDATEABLE_COLS_BOUND,					
										FORWARD, 
										NO_COLS_BY_REF, 
										NULL, 
										NULL,
										NULL, 
										DBTYPE_EMPTY, 
										0, 
										NULL, 
										NULL, 
										NO_COLS_OWNED_BY_PROV, 
										DBPARAMIO_NOTPARAM, 
										NO_BLOB_COLS),
										S_OK);

		/////////////////////////////////////////////////////////
		//Insert a null row into the table
		/////////////////////////////////////////////////////////
		m_pData = (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
		if (!m_pData)
		{
			goto CLEANUP;
		}
		memset(m_pData, 0, (size_t)m_cbRowSize);

		for (j=0; j<m_cBindings; j++)
		{
			//set data buffer's status field
//			*(DBSTATUS *)((DWORD)m_pData + m_rgBindings[j].obStatus) = DBSTATUS_S_ISNULL;
			STATUS_BINDING(m_rgBindings[j],m_pData)=DBSTATUS_S_ISNULL;
		}				
	
		//Get the row
		if (!VerifyInterface(m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,
			(IUnknown **)&pIRowsetChange))
		{
			goto CLEANUP;
		}
		//get a row
		m_pIRowset->RestartPosition(NULL);
		if (!CHECK(m_pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &m_phRow),S_OK)) 
		{
			goto CLEANUP;
		}
		//Make as many columns NULL as possible
		hr	= pIRowsetChange->SetData(m_hRow, m_hAccessor, m_pData);
		{
			if (DB_S_ERRORSOCCURRED == hr)
			{
				for (j=0; j<m_cBindings; j++)
				{
//					if (*(DBSTATUS *)((DWORD)m_pData + m_rgBindings[j].obStatus)!= DBSTATUS_S_ISNULL)
					if	(
							DBSTATUS_S_ISNULL	!= STATUS_BINDING(m_rgBindings[j],m_pData)
						)
					{
						goto CLEANUP;
					}
				}				
				fResults = TEST_SKIPPED;
				goto CLEANUP;
			}
			if (S_OK!=hr)
			{
				goto CLEANUP;
			}
			//We only succeed if we get this far
			fResults = TEST_PASS;
		}	
	}
CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	if (m_pIRowset)
	{
		if (m_hRow != DB_NULL_HROW)
		{
			m_pIRowset->ReleaseRows(cRowsObtained, m_phRow, NULL, NULL, NULL);
			m_hRow = DB_NULL_HROW;
		}
	}
	if (m_pIAccessor)
	{
		if (m_hAccessor != DB_NULL_HACCESSOR)
		{
			m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		}
	}

	if (m_rgBindings)
	{
		PROVIDER_FREE(m_rgBindings);
	}
	if (m_pData)
	{
		PROVIDER_FREE(m_pData);
	}

	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(m_pIRowset);

	return fResults;
}



// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc Resynch on Null row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNullRowResynch::Variation_1()
{
	BOOL				fResults			= FALSE;	
	BYTE				*pRowsetData		= NULL;
	BYTE				*pResynchRowsetData	= NULL;
	BYTE				*pResynchVisibleData= NULL;
	IRowsetResynch		*pIRowResynch		= NULL;	
	IRowsetRefresh		*pIRowRefresh		= NULL;	
	DBCOUNTITEM			cRowsResynched		= 0;
	DBROWSTATUS			*rgRowStatus		= NULL;
	HROW				*rghRows			= NULL;
	DBCOUNTITEM			cRowsObtained		= 0;

	if (m_eTI==TI_IRowsetResynch)
	{
		if (!VerifyInterface(m_pIAccessor, IID_IRowsetResynch, ROWSET_INTERFACE,(IUnknown **)&pIRowResynch))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (!VerifyInterface(m_pIAccessor, IID_IRowsetRefresh, ROWSET_INTERFACE,(IUnknown **)&pIRowRefresh))
		{
			goto CLEANUP;
		}
	}
	if (!VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset))
	{
		goto CLEANUP;
	}
	//create accessor on the rowset
	if (!CHECK(GetAccessorAndBindings(	m_pIAccessor, 
										DBACCESSOR_ROWDATA,
										&m_hAccessor, 
										&m_rgBindings, 
										&m_cBindings,
										&m_cbRowSize,			
  										DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
										ALL_COLS_BOUND,					
										FORWARD, 
										NO_COLS_BY_REF, 
										NULL, 
										NULL,
										NULL, 
										DBTYPE_EMPTY, 
										0, 
										NULL, 
										NULL, 
										NO_COLS_OWNED_BY_PROV, 
										DBPARAMIO_NOTPARAM, 
										BLOB_LONG),
										S_OK))
	{
		goto CLEANUP;
	}

	//alloc mem
	pRowsetData			= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchRowsetData	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	pResynchVisibleData	= (BYTE *)PROVIDER_ALLOC(m_cbRowSize);
	if (!pRowsetData || !pResynchRowsetData || !pResynchVisibleData)
	{
		goto CLEANUP;
	}				
	memset(pRowsetData, 0, (size_t)m_cbRowSize);
	memset(pResynchRowsetData, 0, (size_t)m_cbRowSize);
	memset(pResynchVisibleData, 0, (size_t)m_cbRowSize);

	/////////////////////////////////////////////////////////
	// Make sure we can do all our resynching on null
	// columns and it turns out the same as GetData
	/////////////////////////////////////////////////////////
	m_pIRowset->RestartPosition(NULL);
	if (!CHECK(m_pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &m_phRow),S_OK)) 
	{
		goto CLEANUP;	
	}
	if (!CHECK(m_pIRowset->GetData(m_hRow, m_hAccessor, pRowsetData), S_OK))
	{
		goto CLEANUP;
	}
	if (m_eTI==TI_IRowsetResynch)
	{
		if (!CHECK(pIRowResynch->GetVisibleData(m_hRow, m_hAccessor, pResynchVisibleData), S_OK))
		{
			goto CLEANUP;
		}

		if (!CHECK(pIRowResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			if (!CHECK(pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, TRUE, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
			{
				goto CLEANUP;
			}
		}
		else
		{
			if (!CHECK(pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, FALSE, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
			{
				goto CLEANUP;
			}
		}
		if (!CHECK(pIRowRefresh->GetLastVisibleData(m_hRow, m_hAccessor, pResynchVisibleData), S_OK))
		{
			goto CLEANUP;
		}
	}

	//We don't need the extra ref count ResynchRows Added
	CHECK(m_pIRowset->ReleaseRows(cRowsResynched, rghRows, NULL, NULL, NULL), S_OK);

	COMPARE(cRowsResynched, 1);
	if (g_fNOCHANGE)
	{
		//no change was made here, if the provider can handle it, expect it
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
	}
	else
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	}

	if (!CHECK(m_pIRowset->GetData(m_hRow, m_hAccessor, pResynchRowsetData), S_OK))
	{
		goto CLEANUP;
	}

	if (COMPARE(CompareBuffer(pRowsetData,pResynchVisibleData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE))
	{
		if (COMPARE(CompareBuffer(pResynchVisibleData,pResynchRowsetData,m_cBindings,m_rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE))
		{
			fResults = TRUE;
		}
	}
CLEANUP:
	if (m_pIRowset)
	{
		if (m_hRow != DB_NULL_HROW)
		{
			m_pIRowset->ReleaseRows(cRowsObtained, m_phRow, NULL, NULL, NULL);
			m_hRow = DB_NULL_HROW;
		}
	}

	if (m_pIAccessor)
	{
		if (m_hAccessor != DB_NULL_HACCESSOR)
		{
			m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		}
	}

	if (m_rgBindings)
	{
		PROVIDER_FREE(m_rgBindings);
	}
	if (pRowsetData)
	{
		PROVIDER_FREE(pRowsetData);
	}
	if (pResynchRowsetData)
	{
		PROVIDER_FREE(pResynchRowsetData);
	}
	if (pResynchVisibleData)
	{
		PROVIDER_FREE(pResynchVisibleData);
	}
	if (rgRowStatus)
	{
		PROVIDER_FREE(rgRowStatus);
	}

	SAFE_RELEASE(pIRowResynch);
	SAFE_RELEASE(pIRowRefresh);
	SAFE_RELEASE(m_pIRowset);

	if (rghRows)
		PROVIDER_FREE(rghRows);

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
BOOL TCNullRowResynch::Terminate()
{
	if (m_pITransactionLocal)
	{
		//clean up any changes this little kludge messed on the back end 
		m_pITransactionLocal->Abort(NULL,FALSE,FALSE);
	}
	SAFE_RELEASE(m_pITransactionLocal);
	SAFE_RELEASE(m_pIAccessor);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetObject::Terminate());
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCMisc)
//*-----------------------------------------------------------------------
//|	Test Case:		TCMisc 
//|	Created:			10/04/99
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMiscResynch::Init()
{
	BOOL			fResults			= TEST_FAIL;

	if (TEST_SKIPPED==fnInterfaceSupported())
	{
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetObject::Init())
	// }}
	{
		fResults	= TEST_PASS;
	}	

	return fResults;
}



// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc Resynch on a row with a computed column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMiscResynch::Variation_1()
{
	BOOL				fResults			= FALSE;	
	HACCESSOR			hAccessor			= DB_NULL_HACCESSOR;
	DBBINDING			*rgBindings			= NULL;
	DBCOUNTITEM			cBindings			= 0;
	DBLENGTH			cbRowSize			= 0;
	BYTE				*pRowsetData		= NULL;
	BYTE				*pResynchRowsetData	= NULL;
	BYTE				*pResynchVisibleData= NULL;
	IRowsetResynch		*pIRowResynch		= NULL;	
	IRowsetRefresh		*pIRowRefresh		= NULL;	
	IRowset				*pIRowset			= NULL;
	IAccessor			*pIAccessor			= NULL;
	DBCOUNTITEM			cRowsResynched		= 0;
	DBROWSTATUS			*rgRowStatus		= NULL;
	HROW				*rghRows			= NULL;
	DBCOUNTITEM			cRowsObtained		= 0;
	const ULONG			cProps				= 3;
	DBPROP				rgNewDBProp[cProps];	
	DBPROPSET			NewPropSet;
	HRESULT				hr;
	HROW				hRow				= DB_NULL_HROW;
	HROW				*phRow				= &hRow;


	//Set up our rowset properties
	rgNewDBProp[0].dwPropertyID = DBPROP_UPDATABILITY;
	rgNewDBProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;				
	rgNewDBProp[0].colid		= DB_NULLID;
	rgNewDBProp[0].vValue.vt	= VT_I4;
	rgNewDBProp[0].vValue.lVal	= DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE;

	if (m_eTI==TI_IRowsetResynch)
	{
		rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetResynch;
		rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[1].colid				= DB_NULLID;
		rgNewDBProp[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
	}
	else
	{
		rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetRefresh;
		rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[1].colid				= DB_NULLID;
		rgNewDBProp[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
	}

	rgNewDBProp[2].dwPropertyID			= DBPROP_IRowsetChange;
	rgNewDBProp[2].dwOptions			= DBPROPOPTIONS_REQUIRED;				
	rgNewDBProp[2].colid				= DB_NULLID;
	rgNewDBProp[2].vValue.vt			= VT_BOOL;
	V_BOOL(&(rgNewDBProp[2].vValue))	= VARIANT_TRUE;
				
	NewPropSet.rgProperties		= rgNewDBProp;
	NewPropSet.cProperties		= cProps;
	NewPropSet.guidPropertySet	= DBPROPSET_ROWSET;
			
	//get a rowset
	hr	= ((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	SELECT_COMPUTEDCOLLIST,
									IID_IRowset, 
									1, 
									&NewPropSet, 
									(IUnknown**)&pIRowset, 
									NULL, 
									NULL, 
									NULL, 
									0, 
									NULL,
									NULL);	

	if (m_eTI==TI_IRowsetResynch)
	{
		if (!VerifyInterface(pIRowset, IID_IRowsetResynch, ROWSET_INTERFACE,(IUnknown **)&pIRowResynch))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (!VerifyInterface(pIRowset, IID_IRowsetRefresh, ROWSET_INTERFACE,(IUnknown **)&pIRowRefresh))
		{
			goto CLEANUP;
		}
	}
	if (!VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE,(IUnknown **)&pIAccessor))
	{
		goto CLEANUP;
	}
	//create accessor on the rowset
	if (!CHECK(GetAccessorAndBindings(	pIAccessor, 
										DBACCESSOR_ROWDATA,
										&hAccessor, 
										&rgBindings, 
										&cBindings,
										&cbRowSize,			
  										DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
										ALL_COLS_BOUND,					
										FORWARD, 
										NO_COLS_BY_REF, 
										NULL, 
										NULL,
										NULL, 
										DBTYPE_EMPTY, 
										0, 
										NULL, 
										NULL, 
										NO_COLS_OWNED_BY_PROV, 
										DBPARAMIO_NOTPARAM, 
										NO_BLOB_COLS),
										S_OK))
	{
		goto CLEANUP;
	}

	//alloc mem
	pRowsetData			= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	pResynchRowsetData	= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	pResynchVisibleData	= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	if (!pRowsetData || !pResynchRowsetData || !pResynchVisibleData)
	{
		goto CLEANUP;
	}				
	memset(pRowsetData, 0, (size_t)cbRowSize);
	memset(pResynchRowsetData, 0, (size_t)cbRowSize);
	memset(pResynchVisibleData, 0, (size_t)cbRowSize);

	//get a row and make a change on it
	CHECK(pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow),S_OK);	
	
	CHECK(pIRowset->GetData(hRow, hAccessor, pRowsetData), S_OK);

	if (m_eTI==TI_IRowsetResynch)
	{
		if (!CHECK(pIRowResynch->GetVisibleData(hRow, hAccessor, pResynchVisibleData), S_OK))
		{
			goto CLEANUP;
		}

		if (!CHECK(pIRowResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			if (!CHECK(pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, TRUE, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
			{
				goto CLEANUP;
			}
		}
		else
		{
			if (!CHECK(pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, FALSE, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
			{
				goto CLEANUP;
			}
		}
		if (!CHECK(pIRowRefresh->GetLastVisibleData(hRow, hAccessor, pResynchVisibleData), S_OK))
		{
			goto CLEANUP;
		}
	}

	//We don't need the extra ref count ResynchRows Added
	CHECK(pIRowset->ReleaseRows(cRowsResynched, rghRows, NULL, NULL, NULL), S_OK);

	COMPARE(cRowsResynched, 1);
	
	if (g_fNOCHANGE)
	{
		//no change was made here, if the provider can handle it, expect it
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
	}
	else
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	}

	if (!CHECK(pIRowset->GetData(hRow, hAccessor, pResynchRowsetData), S_OK))
	{
		goto CLEANUP;
	}

	if (COMPARE(CompareBuffer(pRowsetData,pResynchVisibleData,cBindings,rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE))
	{
		if (COMPARE(CompareBuffer(pResynchVisibleData,pResynchRowsetData,cBindings,rgBindings,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE))
		{
			fResults = TRUE;
		}
	}
CLEANUP:
	if (pIRowset)
	{
		if (hRow != DB_NULL_HROW)
		{
			pIRowset->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL);
			hRow = DB_NULL_HROW;
		}
		if (pIRowset)
		{
			pIRowset->Release();
			pIRowset = NULL;
		}
	}

	if (pIAccessor)
	{
		if (hAccessor != DB_NULL_HACCESSOR)
		{
			pIAccessor->ReleaseAccessor(hAccessor, NULL);
		}
	}

	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}
	if (pRowsetData)
	{
		PROVIDER_FREE(pRowsetData);
	}
	if (pResynchRowsetData)
	{
		PROVIDER_FREE(pResynchRowsetData);
	}
	if (pResynchVisibleData)
	{
		PROVIDER_FREE(pResynchVisibleData);
	}
	if (rgRowStatus)
	{
		PROVIDER_FREE(rgRowStatus);
	}
	if (pIRowResynch)
	{
		pIRowResynch->Release();
		pIRowResynch = NULL;
	}
	if (rghRows)
	{
		PROVIDER_FREE(rghRows);
	}

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowRefresh);
	SAFE_RELEASE(pIRowResynch);
	SAFE_RELEASE(pIRowset);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc different accessors
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMiscResynch::Variation_2()
{
	BOOL				fResults			= FALSE;	
	HACCESSOR			hAccessor			= DB_NULL_HACCESSOR;
	DBBINDING			*rgBindings			= NULL;
	DBCOUNTITEM			cBindings			= 0;
	DBLENGTH			cbRowSize			= 0;
	BYTE				*pData				= NULL;
	BYTE				*pData2				= NULL;
	HACCESSOR			hAccessor2			= DB_NULL_HACCESSOR;
	DBBINDING			*rgBindings2		= NULL;
	DBCOUNTITEM			cBindings2			= 0;
	DBLENGTH			cbRowSize2			= 0;
	BYTE				*pRowsetData		= NULL;
	BYTE				*pResynchRowsetData	= NULL;
	BYTE				*pVisibleData		= NULL;
	IRowsetResynch		*pIRowResynch		= NULL;	
	IRowsetRefresh		*pIRowRefresh		= NULL;	
	IAccessor			*pIAccessor			= NULL;
	IRowset				*pIRowset			= NULL;
	DBCOUNTITEM			cRowsResynched		= 0;
	DBROWSTATUS			*rgRowStatus		= NULL;
	HROW				*rghRows			= NULL;
	DBCOUNTITEM			cRowsObtained		= 0;
	const ULONG			cProps				= 3;
	DBPROP				rgNewDBProp[cProps];	
	DBPROPSET			NewPropSet;
	IRowsetChange		*pIRowsetChange		= NULL;
	ITransactionLocal	*pITransactionLocal	= NULL;
	HRESULT				hr;
	HROW				hRow				= DB_NULL_HROW;
	HROW				*phRow				= &hRow;

	//Get IRowset on the object
	if (!VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
							IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		goto CLEANUP;	
	}
	g_ulLastActualInsert++;
	pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, NULL);

	//Set up our rowset properties
	rgNewDBProp[0].dwPropertyID = DBPROP_UPDATABILITY;
	rgNewDBProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;				
	rgNewDBProp[0].colid		= DB_NULLID;
	rgNewDBProp[0].vValue.vt	= VT_I4;
	rgNewDBProp[0].vValue.lVal	= DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE;

	if (m_eTI==TI_IRowsetResynch)
	{
		rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetResynch;
		rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[1].colid				= DB_NULLID;
		rgNewDBProp[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
	}
	else
	{
		rgNewDBProp[1].dwPropertyID			= DBPROP_IRowsetRefresh;
		rgNewDBProp[1].dwOptions			= DBPROPOPTIONS_REQUIRED;				
		rgNewDBProp[1].colid				= DB_NULLID;
		rgNewDBProp[1].vValue.vt			= VT_BOOL;
		V_BOOL(&(rgNewDBProp[1].vValue))	= VARIANT_TRUE;
	}

	rgNewDBProp[2].dwPropertyID			= DBPROP_IRowsetChange;
	rgNewDBProp[2].dwOptions			= DBPROPOPTIONS_REQUIRED;				
	rgNewDBProp[2].colid				= DB_NULLID;
	rgNewDBProp[2].vValue.vt			= VT_BOOL;
	V_BOOL(&(rgNewDBProp[2].vValue))	= VARIANT_TRUE;
				
	NewPropSet.rgProperties		= rgNewDBProp;
	NewPropSet.cProperties		= cProps;
	NewPropSet.guidPropertySet	= DBPROPSET_ROWSET;
					
	//get a rowset
	hr	= ((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
									IID_IRowset, 
									1, 
									&NewPropSet, 
									(IUnknown**)&pIRowset, 
									NULL, 
									NULL, 
									NULL, 
									0, 
									NULL,
									NULL);	

	if (m_eTI==TI_IRowsetResynch)
	{
		if (!VerifyInterface(pIRowset, IID_IRowsetResynch, ROWSET_INTERFACE,(IUnknown **)&pIRowResynch))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (!VerifyInterface(pIRowset, IID_IRowsetRefresh, ROWSET_INTERFACE,(IUnknown **)&pIRowRefresh))
		{
			goto CLEANUP;
		}
	}
		
	if (!VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE,(IUnknown **)&pIAccessor))
	{
		goto CLEANUP;
	}
	//create accessor on the rowset
	if (!CHECK(GetAccessorAndBindings(	pIAccessor, 
										DBACCESSOR_ROWDATA,
										&hAccessor, 
										&rgBindings, 
										&cBindings,
										&cbRowSize,			
  										DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
										ALL_COLS_BOUND,
										FORWARD, 
										NO_COLS_BY_REF, 
										NULL, 
										NULL,
										NULL, 
										DBTYPE_EMPTY, 
										0, 
										NULL, 
										NULL, 
										NO_COLS_OWNED_BY_PROV, 
										DBPARAMIO_NOTPARAM, 
										NO_BLOB_COLS),
										S_OK))
	{
		goto CLEANUP;
	}
	//create a 2nd accessor on the rowset
	if (!CHECK(GetAccessorAndBindings(	pIAccessor, 
										DBACCESSOR_ROWDATA,
										&hAccessor2, 
										&rgBindings2, 
										&cBindings2,
										&cbRowSize2,			
  										DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
										UPDATEABLE_COLS_BOUND,
										FORWARD, 
										NO_COLS_BY_REF, 
										NULL, 
										NULL,
										NULL, 
										DBTYPE_EMPTY, 
										0, 
										NULL, 
										NULL, 
										NO_COLS_OWNED_BY_PROV, 
										DBPARAMIO_NOTPARAM, 
										NO_BLOB_COLS),
										S_OK))
	{
		goto CLEANUP;
	}

	//alloc mem
	pRowsetData			= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	pResynchRowsetData	= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	pVisibleData		= (BYTE *)PROVIDER_ALLOC(cbRowSize);
	if (!pRowsetData || !pResynchRowsetData || !pVisibleData)
	{
		goto CLEANUP;
	}				
	memset(pRowsetData, 0, (size_t)cbRowSize);
	memset(pResynchRowsetData, 0, (size_t)cbRowSize);
	memset(pVisibleData, 0, (size_t)cbRowSize);

	//Set data for all columns
	TESTC_(FillInputBindings(	((CTable *)m_pThisTestModule->m_pVoid), 
								DBACCESSOR_ROWDATA, 
								cBindings, 
								rgBindings, 
								&pData, 
								g_ulLastActualInsert,
								((CTable *)m_pThisTestModule->m_pVoid)->CountColumnsOnTable(),
								m_rgTableColOrds), S_OK);

	//create another buffer based on the same seed with the 2nd accessor
	TESTC_(FillInputBindings(	((CTable *)m_pThisTestModule->m_pVoid), 
								DBACCESSOR_ROWDATA, 
								cBindings2, 
								rgBindings2, 
								&pData2, 
								g_ulLastActualInsert, 
								((CTable *)m_pThisTestModule->m_pVoid)->CountColumnsOnTable(),
								m_rgTableColOrds), S_OK);

	//get a row and make a change on it
	CHECK(pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow),S_OK);
	if (!VerifyInterface(pIRowset, IID_IRowsetChange, ROWSET_INTERFACE,(IUnknown **)&pIRowsetChange))
	{
		goto CLEANUP;
	}
	//change a row, updatable cols
	CHECK(pIRowsetChange->SetData(phRow[0], hAccessor, pData), S_OK);
	
	//now get data with 2nd accessor
	CHECK(pIRowset->GetData(hRow, hAccessor2, pRowsetData), S_OK);

	//GetVisible with the 2nd
	//check if the SetData did a delete/insert underneath first off
	if (m_eTI==TI_IRowsetResynch)
	{
		hr	= pIRowResynch->GetVisibleData(hRow, hAccessor2, pVisibleData);
		if (DB_E_DELETEDROW==hr)
		{
			if (!CHECK(pIRowResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRows, &rgRowStatus), DB_E_ERRORSOCCURRED))
			{
				goto CLEANUP;
			}
			COMPARE(1,cRowsResynched);
			COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
			fResults	= TRUE;
			goto CLEANUP;
		}
	}
	else
	{
		if (g_fVisualCache)
		{
			CHECK(pIRowRefresh->GetLastVisibleData(hRow, hAccessor2, pVisibleData),S_OK);
		}
		else
		{
			hr	= pIRowRefresh->GetLastVisibleData(hRow, hAccessor2, pVisibleData);
			if (DB_E_DELETEDROW==hr)
			{
				if (!CHECK(pIRowResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRows, &rgRowStatus), DB_E_ERRORSOCCURRED))
				{
					goto CLEANUP;
				}
				COMPARE(1,cRowsResynched);
				COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
				fResults	= TRUE;
				goto CLEANUP;
			}
		}
	}
	COMPARE(hr,S_OK);
	
	//resynch and check againg for deletedrow in case there was a visual cache
	if (m_eTI==TI_IRowsetResynch)
	{
		if (!CHECK(pIRowResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRows, &rgRowStatus), S_OK))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (m_eTI==TI_IRowsetRefreshTRUE)
		{
			hr	= pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, TRUE, &cRowsResynched, &rghRows, &rgRowStatus);
		}
		else
		{
			hr	= pIRowRefresh->RefreshVisibleData(DB_NULL_HCHAPTER, 0, NULL, FALSE, &cRowsResynched, &rghRows, &rgRowStatus);
		}
	}

	if (DB_E_ERRORSOCCURRED==hr)
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
		fResults	= TRUE;
		goto CLEANUP;
	}

	//don't need the extra ref count ResynchRows Added
	CHECK(pIRowset->ReleaseRows(cRowsResynched, rghRows, NULL, NULL, NULL), S_OK);
	//one active row
	COMPARE(cRowsResynched, 1);
	
	if (g_fNOCHANGE)
	{
		//no change was made here (the rowset changed it itself),
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_NOCHANGE);
	}
	else
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	}

	if (!CHECK(pIRowset->GetData(hRow, hAccessor2, pResynchRowsetData), S_OK))
	{
		goto CLEANUP;
	}

	//check that the buffer all have the 2nd accesor's version of the new buffer
	COMPARE(CompareBuffer(pRowsetData,pData2,cBindings2,rgBindings2,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE);
	COMPARE(CompareBuffer(pRowsetData,pVisibleData,cBindings2,rgBindings2,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
	COMPARE(CompareBuffer(pVisibleData,pResynchRowsetData,cBindings2,rgBindings2,m_pIMalloc,FALSE,FALSE,COMPARE_ONLY),TRUE);
		
	fResults = TRUE;
CLEANUP:
	if (pITransactionLocal)
	{
		//clean up any changes this little kludge messed on the back end 
		pITransactionLocal->Abort(NULL,FALSE,FALSE);
	}
	g_ulLastActualInsert--;

	if (pIRowset)
	{
		if (hRow != DB_NULL_HROW)
		{
			pIRowset->ReleaseRows(cRowsObtained, phRow, NULL, NULL, NULL);
			hRow = DB_NULL_HROW;
		}
	}

	if (pIAccessor)
	{
		if (hAccessor != DB_NULL_HACCESSOR)
		{
			pIAccessor->ReleaseAccessor(hAccessor, NULL);
		}
		if (hAccessor2 != DB_NULL_HACCESSOR)
		{
			pIAccessor->ReleaseAccessor(hAccessor2, NULL);
		}
	}

	if (pRowsetData)
	{
		PROVIDER_FREE(pRowsetData);
	}
	if (pResynchRowsetData)
	{
		PROVIDER_FREE(pResynchRowsetData);
	}
	if (pVisibleData)
	{
		PROVIDER_FREE(pVisibleData);
	}
	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}
	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings2, rgBindings2, pData2, TRUE);
	if (rgBindings2)
	{
		PROVIDER_FREE(rgBindings2);
	}
	if (rgRowStatus)
	{
		PROVIDER_FREE(rgRowStatus);
	}
	if (rghRows)
	{
		PROVIDER_FREE(rghRows);
	}

	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pITransactionLocal);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowRefresh);
	SAFE_RELEASE(pIRowResynch);
	SAFE_RELEASE(pIRowset);
	
	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc OTHERUPDATEDELETE - FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMiscResynch::Variation_3()
{
	HACCESSOR			hAccessor1		= DB_NULL_HACCESSOR;
	HACCESSOR			hAccessor2		= DB_NULL_HACCESSOR;
	IAccessor			*pIAccessor1	= NULL;
	IAccessor			*pIAccessor2	= NULL;
	DBBINDING			*rgBindings		= NULL;
	DBCOUNTITEM			cBindings		= 0;
	ITransactionLocal	*pITransactionLocal	= NULL;
	IRowsetChange		*pIRowsetChange	= NULL;
	IRowsetRefresh		*pIRowsetRefresh= NULL;
	IRowsetResynch		*pIRowsetResynch= NULL;
	IRowset				*pIRowset1		= NULL;
	IRowset				*pIRowset2		= NULL;
	const ULONG			cProps			= 4;
	DBPROPSET			DBPropSet;
	DBPROP				DBProp[cProps];
	HRESULT				hr				= S_OK;
	BYTE				*pData			= NULL;
	const ULONG			cRows			= 1;
	HROW				rghRows1[cRows];
	HROW				*phRows1		= rghRows1;
	HROW				rghRows2[cRows];
	HROW				*phRows2		= rghRows2;
	BOOL				fResult			= TEST_FAIL;
	DBCOUNTITEM			cRowsObtained	= 1; 
	IMalloc				*pIMalloc		= NULL;	
	DBLENGTH			cRowSize		= 0;
	DBCOUNTITEM			cRowsResynched	= 0;
	DBROWSTATUS			*rgReRowStatus	= NULL;
	HROW				*rghReRows		= NULL;


	//Struct for set properties
	DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropSet.rgProperties		= DBProp;
	DBPropSet.cProperties		= cProps;

	DBProp[0].dwPropertyID			= DBPROP_IRowsetChange;
	DBProp[0].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[0].colid					= DB_NULLID;
	DBProp[0].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[0].vValue))		= VARIANT_TRUE;	

	DBProp[1].dwPropertyID			= DBPROP_UPDATABILITY;
	DBProp[1].dwOptions				= 0;
	DBProp[1].colid					= DB_NULLID;
	DBProp[1].vValue.vt				= VT_I4;
	DBProp[1].vValue.lVal			= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	if(g_fRefresh)
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetRefresh;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;			
	}
	else
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetResynch;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;
	}

	DBProp[3].dwPropertyID			= DBPROP_OTHERUPDATEDELETE;
	DBProp[3].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[3].colid					= DB_NULLID;
	DBProp[3].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[3].vValue))		= VARIANT_FALSE;	

	//Get IRowset on the object
	if (!VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
							IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		goto CLEANUP;	
	}
	pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, NULL);

	//get 2 rowsets
	hr	= ((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
																IID_IRowset, 
																1, 
																&DBPropSet, 
																(IUnknown**)&pIRowset1, 
																NULL, 
																NULL, 
																NULL, 
																0, 
																NULL,
																NULL);	

	//if IRowsetRefresh conflicts with DBPROP_OTHERUPDATEDELETE-FALSE skip this
	if (hr != S_OK)
	{
		fResult	= TEST_SKIPPED;
		goto CLEANUP;
	}

	hr	= ((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
																IID_IRowset, 
																1, 
																&DBPropSet, 
																(IUnknown**)&pIRowset2, 
																NULL, 
																NULL, 
																NULL, 
																0, 
																NULL,
																NULL);	

	//if IRowsetRefresh conflicts with DBPROP_OTHERUPDATEDELETE-FALSE skip this
	if (hr != S_OK)
	{
		fResult	= TEST_SKIPPED;
		goto CLEANUP;
	}

	//Get accessor on the object
	if (!VerifyInterface(pIRowset1, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor1))
	{
		goto CLEANUP;	
	}

	//Get accessor on the object
	if (!VerifyInterface(pIRowset2, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor2))
	{
		goto CLEANUP;	
	}

	TESTC_(GetAccessorAndBindings(	pIAccessor1, 
									DBACCESSOR_ROWDATA,
									&hAccessor1, 
									&rgBindings, 
									&cBindings, 
									&cRowSize,			
  									DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
									ALL_COLS_BOUND,
									FORWARD, 
									NO_COLS_BY_REF, 
									NULL, 
									NULL,
									NULL, 
									DBTYPE_EMPTY, 
									0, 
									NULL, 
									NULL, 
									NO_COLS_OWNED_BY_PROV, 
									DBPARAMIO_NOTPARAM,
									NO_BLOB_COLS),S_OK);


	//alloc here just so test has it to free for ReleaseInputBindingsMemory
	pData = (BYTE *)PROVIDER_ALLOC(cRowSize);
	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}
	pData	= NULL;

	TESTC_(GetAccessorAndBindings(	pIAccessor2, 
									DBACCESSOR_ROWDATA,
									&hAccessor2, 
									&rgBindings, 
									&cBindings, 
									NULL,			
  									DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
									ALL_COLS_BOUND,
									FORWARD, 
									NO_COLS_BY_REF, 
									NULL, 
									NULL,
									NULL, 
									DBTYPE_EMPTY, 
									0, 
									NULL, 
									NULL, 
									NO_COLS_OWNED_BY_PROV, 
									DBPARAMIO_NOTPARAM,
									NO_BLOB_COLS),S_OK);

	//Set data for all columns
	TESTC_(FillInputBindings(	((CTable *)m_pThisTestModule->m_pVoid), 
								DBACCESSOR_ROWDATA, 
								cBindings, 
								rgBindings, 
								&pData, 
								g_ulLastActualInsert+1, 
								((CTable *)m_pThisTestModule->m_pVoid)->CountColumnsOnTable(),
								m_rgTableColOrds), S_OK);

	//Get IRowsetChange on the object
	if (!VerifyInterface(pIRowset1, IID_IRowsetChange, ROWSET_INTERFACE, (IUnknown **)&pIRowsetChange))
	{
		goto CLEANUP;	
	}


	//get the first row through the 1st rowset
	TESTC_(pIRowset1->RestartPosition(NULL), S_OK);
	TESTC_(pIRowset1->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows1),S_OK);
	
	//get the first row through the 2nd rowset
	TESTC_(pIRowset2->RestartPosition(NULL), S_OK);
	TESTC_(pIRowset2->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRows2),S_OK);

	//change the data through the first rowset
	TESTC_(pIRowsetChange->SetData(phRows1[0], hAccessor1, pData), S_OK);
	TESTC_(pIRowset1->ReleaseRows(cRowsObtained, phRows1, NULL, NULL, NULL), S_OK);

	//get refresh(or resynch) from rowset2
	if(g_fRefresh)
	{
		if (!VerifyInterface(pIRowset2, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;	
		}
	}
	else
	{
		if (!VerifyInterface(pIRowset2, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown **)&pIRowsetResynch))
		{
			goto CLEANUP;	
		}
	}
		
	//get the visible data from the 2nd rowset
	if (g_fRefresh)
	{
		//refresh the data from the rowset
		TESTC_(pIRowsetRefresh->RefreshVisibleData(DB_NULL_HCHAPTER,1, &phRows2[0], TRUE, &cRowsResynched, &rghReRows, &rgReRowStatus),S_OK);
		TESTC_(pIRowsetRefresh->GetLastVisibleData(phRows2[0], hAccessor2, pData),S_OK);
	}
	else
	{
		//refresh the data from the rowset
		TESTC_(pIRowsetResynch->ResynchRows(1, &phRows2[0], &cRowsResynched, &rghReRows, &rgReRowStatus),S_OK);
		TESTC_(pIRowsetResynch->GetVisibleData(phRows2[0], hAccessor2, pData),S_OK);
	}
		
	//Check this row against ulRowNum, free pData when done
	if (CompareData(((CTable *)m_pThisTestModule->m_pVoid)->CountColumnsOnTable(), m_rgTableColOrds,
					g_ulLastActualInsert+1, pData, cBindings, rgBindings, 
					((CTable *)m_pThisTestModule->m_pVoid), m_pIMalloc, PRIMARY, 
					COMPARE_FREE, COMPARE_UNTIL_ERROR))
	{				
		//We found the row, woo-hoo., resynch works w/OTHERUPDATEDELETE - FALSE
		fResult = TEST_PASS;
	}
	//Free the row for this GetNextRows call
	TESTC_(pIRowset2->ReleaseRows(1, phRows2, NULL, NULL, NULL), S_OK);
CLEANUP:
	//clean up any changes this little kludge messed on the back end 
	pITransactionLocal->Abort(NULL,FALSE,FALSE);

	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}

	//Release accessor1
	if ((hAccessor1 != DB_NULL_HACCESSOR) && pIAccessor1)
	{
		CHECK(pIAccessor1->ReleaseAccessor(hAccessor1, NULL), S_OK);
		hAccessor1 = DB_NULL_HACCESSOR;
	}
	//Release accessor2
	if ((hAccessor2 != DB_NULL_HACCESSOR) && pIAccessor2)
	{
		CHECK(pIAccessor2->ReleaseAccessor(hAccessor2, NULL), S_OK);
		hAccessor2 = DB_NULL_HACCESSOR;
	}

	PROVIDER_FREE(rgReRowStatus);
	PROVIDER_FREE(rghReRows);

	SAFE_RELEASE(pITransactionLocal);
	SAFE_RELEASE(pIAccessor1);
	SAFE_RELEASE(pIAccessor2);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetRefresh);
	SAFE_RELEASE(pIRowsetResynch);

	return fResult;
}
// }}



// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc GetVisible twice w/BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMiscResynch::Variation_4()
{
	HACCESSOR			hAccessor1		= DB_NULL_HACCESSOR;
	IAccessor			*pIAccessor1	= NULL;
	DBBINDING			*rgBindings		= NULL;
	DBCOUNTITEM			cBindings		= 0;
	ITransactionLocal	*pITransactionLocal	= NULL;
	IRowsetChange		*pIRowsetChange	= NULL;
	IRowsetRefresh		*pIRowsetRefresh= NULL;
	IRowsetResynch		*pIRowsetResynch= NULL;
	IRowset				*pIRowset1		= NULL;
	const ULONG			cProps			= 4;
	DBPROPSET			DBPropSet;
	DBPROP				DBProp[cProps];
	HRESULT				hr				= S_OK;
	BYTE				*pData			= NULL;
	const ULONG			cRows			= 2;
	HROW				rghRows1[cRows];
	HROW				*phRows1		= rghRows1;
	BOOL				fResult			= TEST_FAIL;
	DBCOUNTITEM			cRowsObtained	= 1; 
	DBLENGTH			cRowSize		= 0;
	DBCOUNTITEM			cRowsResynched	= 0;
	ULONG				i				= 0;


	//Struct for set properties
	DBPropSet.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropSet.rgProperties		= DBProp;
	DBPropSet.cProperties		= cProps;

	DBProp[0].dwPropertyID			= DBPROP_IRowsetChange;
	DBProp[0].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[0].colid					= DB_NULLID;
	DBProp[0].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[0].vValue))		= VARIANT_TRUE;	

	DBProp[1].dwPropertyID			= DBPROP_UPDATABILITY;
	DBProp[1].dwOptions				= 0;
	DBProp[1].colid					= DB_NULLID;
	DBProp[1].vValue.vt				= VT_I4;
	DBProp[1].vValue.lVal			= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	if(g_fRefresh)
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetRefresh;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;			
	}
	else
	{
		DBProp[2].dwPropertyID		= DBPROP_IRowsetResynch;
		DBProp[2].dwOptions			= 0;
		DBProp[2].colid				= DB_NULLID;
		DBProp[2].vValue.vt			= VT_BOOL;
		V_BOOL(&(DBProp[2].vValue))	= VARIANT_TRUE;
	}

	DBProp[3].dwPropertyID			= DBPROP_OTHERUPDATEDELETE;
	DBProp[3].dwOptions				= DBPROPOPTIONS_REQUIRED;
	DBProp[3].colid					= DB_NULLID;
	DBProp[3].vValue.vt				= VT_BOOL;
	V_BOOL(&(DBProp[3].vValue))		= VARIANT_FALSE;	

	//Get IRowset on the object
	if (!VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
							IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		goto CLEANUP;	
	}
	pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, NULL);

	//get a rowsets
	hr	= ((CTable *)m_pThisTestModule->m_pVoid)->CreateRowset(	USE_OPENROWSET,
																IID_IRowset, 
																1, 
																&DBPropSet, 
																(IUnknown**)&pIRowset1, 
																NULL, 
																NULL, 
																NULL, 
																0, 
																NULL,
																NULL);	

	//if IRowsetRefresh conflicts with DBPROP_OTHERUPDATEDELETE-FALSE skip this
	if (hr != S_OK )
	{
		fResult	= TEST_SKIPPED;
		goto CLEANUP;
	}


	//Get accessor on the object
	if (!VerifyInterface(pIRowset1, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor1))
	{
		goto CLEANUP;	
	}

	TESTC_(GetAccessorAndBindings(	pIAccessor1, 
									DBACCESSOR_ROWDATA,
									&hAccessor1, 
									&rgBindings, 
									&cBindings, 
									&cRowSize,			
  									DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
									ALL_COLS_BOUND,
									FORWARD, 
									NO_COLS_BY_REF, 
									NULL, 
									NULL,
									NULL, 
									DBTYPE_EMPTY, 
									0, 
									NULL, 
									NULL, 
									NO_COLS_OWNED_BY_PROV, 
									DBPARAMIO_NOTPARAM,
									BLOB_LONG),S_OK);

	//Set data for all columns
	TESTC_(FillInputBindings(	((CTable *)m_pThisTestModule->m_pVoid), 
								DBACCESSOR_ROWDATA, 
								cBindings, 
								rgBindings, 
								&pData, 
								g_ulLastActualInsert+1, 
								((CTable *)m_pThisTestModule->m_pVoid)->CountColumnsOnTable(),
								m_rgTableColOrds), S_OK);

	//Get IRowsetChange on the object
	if (!VerifyInterface(pIRowset1, IID_IRowsetChange, ROWSET_INTERFACE, (IUnknown **)&pIRowsetChange))
	{
		goto CLEANUP;	
	}


	//get the first row through the 1st rowset
	TESTC_(pIRowset1->RestartPosition(NULL), S_OK);
	TESTC_(pIRowset1->GetNextRows(NULL, 0, 2, &cRowsObtained, &phRows1),S_OK);

	if (g_fRefresh)
	{
		if (!VerifyInterface(pIRowset1, IID_IRowsetRefresh, ROWSET_INTERFACE, (IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;	
		}
		CHECK(pIRowsetRefresh->GetLastVisibleData(phRows1[0], hAccessor1, pData),S_OK);
		CHECK(pIRowsetRefresh->GetLastVisibleData(phRows1[1], hAccessor1, pData),S_OK);
		CHECK(pIRowset1->ReleaseRows(1, phRows1, NULL, NULL, NULL), S_OK);
		CHECK(pIRowsetRefresh->GetLastVisibleData(phRows1[1], hAccessor1, pData),S_OK);
	}
	else
	{
		if (!VerifyInterface(pIRowset1, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown **)&pIRowsetResynch))
		{
			goto CLEANUP;	
		}
		//BLOBS
		CHECK(pIRowsetResynch->GetVisibleData(phRows1[0], hAccessor1, pData),DB_S_ERRORSOCCURRED);
		for (i=0;i<cBindings;i++)
		{
//			if (DBSTATUS_E_UNAVAILABLE	!=(*(DBSTATUS *)((DWORD)pData + rgBindings[i].obStatus))	&&
//				DBSTATUS_S_OK			!=(*(DBSTATUS *)((DWORD)pData + rgBindings[i].obStatus)))
			if	(
					DBSTATUS_S_OK			!= STATUS_BINDING(rgBindings[i],pData)	&&
					DBSTATUS_E_UNAVAILABLE	!= STATUS_BINDING(rgBindings[i],pData)
				)
			{
				goto CLEANUP;
			}
		}

		//lets see this error again
		CHECK(pIRowsetResynch->GetVisibleData(phRows1[1], hAccessor1, pData),DB_S_ERRORSOCCURRED);
	}
	fResult	= TEST_PASS;
CLEANUP:
	//clean up any changes this little kludge messed on the back end 
	pITransactionLocal->Abort(NULL,FALSE,FALSE);

	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	if (rgBindings)
	{
		PROVIDER_FREE(rgBindings);
	}

	//Release accessor1
	if ((hAccessor1 != DB_NULL_HACCESSOR) && pIAccessor1)
	{
		CHECK(pIAccessor1->ReleaseAccessor(hAccessor1, NULL), S_OK);
		hAccessor1 = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(pITransactionLocal);
	SAFE_RELEASE(pIAccessor1);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetRefresh);
	SAFE_RELEASE(pIRowsetResynch);

	return fResult;
}
// }}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMiscResynch::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetObject::Terminate());
}	// }}
// }}
// }}
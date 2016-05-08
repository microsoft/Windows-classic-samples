//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IRSCROLL.CPP | IRSCROLL source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "IROWSCRL.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xd010f3e1, 0x2017, 0x11d1, { 0xa8, 0x83, 0x00, 0xc0, 0x4f, 0xd7, 0xa0, 0xf5 }};
DECLARE_MODULE_NAME("IRowsetScroll");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IRowsetScroll Interface");
DECLARE_MODULE_VERSION(795921705);
// }}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTable		*g_pCTable=NULL;						// Pointer to the global table
DBROWCOUNT	g_lRowLast = 0;						// Keep track of rows in g_pCTable
CTable		*g_p1RowTable=NULL;					// one row table;
CTable		*g_pEmptyTable=NULL;					// empty table
BOOL		g_fIsDBCreateCommandSupported = TRUE;
BOOL		g_fIsDBSchemaRowsetSupported = TRUE;
DBROWCOUNT	g_cMaxOpenRowsCount=0;				// Record the number of max open rows in the rowset
DBCOUNTITEM	g_cVerifyRows=0, g_cVerifyApproxPos=0, g_ulWeightedError=0, g_ulWeightedPosError=0;

enum ePrptIdx	{IDX_Bookmarks=0, IDX_CanFetchBackwards, IDX_ScrollBackwards,
	IDX_CanHoldRows, IDX_MaxOpenRows,IDX_RemoveDeleted, IDX_BookmarkSkipped,
	IDX_OtherUpdateDelete, IDX_OtherInsert,	IDX_IRowsetChange};

enum eRowsetType { USECOMMAND=0, USEOPENROWSET, USESCHEMAR };

#undef TESTC_PROVIDER
#define TESTC_PROVIDER(fPass) {if(!fPass) { odtLog << L"NotSupported by Provider, skipping Variation" << ENDL; fTestPass = TEST_SKIPPED; goto END; } }

#define PROPERTY_COUNT		(IDX_IRowsetChange+1)

#define EXACT TRUE
#define INEXACT FALSE

#define VerifyApproximatePosition(cPos, cRows, cExpectedPos) _VerifyApproximatePosition((cPos),(cRows),(cExpectedPos), LONGSTRING(__FILE__), __LINE__)
#define VerifyRowPosition(hRow, cRow, pCTable, fExact) _VerifyRowPosition((hRow), (cRow), (pCTable), (fExact),LONGSTRING(__FILE__), __LINE__)

// Record the properties default values
struct	DBPrptRecord
{
	BOOL	fSupported;
	BOOL	fDefault;
	BOOL	fSettable;
} g_rgDBPrpt[PROPERTY_COUNT];

//--------------------------------------------------------------------
// @func	Print out message on the screen that a particular property
//			is not supported.
//--------------------------------------------------------------------
BOOL	PrintNotSupported(ULONG	cProperties)
{
	BOOL	fPass=TRUE;

	switch(cProperties)
	{
		case IDX_Bookmarks:
				odtLog<<L"DBPROP_BOOKMARKS is not supported!\n";
			break;
		case IDX_CanFetchBackwards:
				odtLog<<L"DBPROP_CANFETCHBACKWARDS is not supported!\n";
			break;
		case IDX_ScrollBackwards:
				odtLog<<L"DBPROP_SCROLLBACKWARDS is not supported!\n";
			break;
		case IDX_CanHoldRows:
				odtLog<<L"DBPROP_CANHOLDROWS is not supported!\n";
			break;
		case IDX_MaxOpenRows:
				odtLog<<L"DBPROP_MAXOPENROWS is not supported!\n";
			break;
		case IDX_RemoveDeleted:
				odtLog<<L"DBPROP_REMOVEDELETED is not supported!\n";
			break;
		case IDX_BookmarkSkipped:
				odtLog<<L"DBPROP_BOOKMARKSKIPPED is not supported!\n";
			break;
		case IDX_OtherUpdateDelete:
				odtLog<<L"DBPROP_OTHERUPDATEDELETE is not supported!\n";
			break;
		case IDX_OtherInsert:
				odtLog<<"DBPROP_OTHERINSERT is not supported!\n";
			break;
		case IDX_IRowsetChange:
				odtLog<<L"IID_IRowsetChange is not supported!\n";
			break;
		default:
				odtLog<<L"Index to the property array is incorrect!\n";
				fPass=FALSE;
			break;
	}
	return fPass;
}

//-------------------------------------------------------------------------
// @func	Func.  Check whether provider allows to set DBPROP_MAXOPENROWS.
//
//-------------------------------------------------------------------------
void InitProp(IUnknown * pIDataSourceUnknown)
{
	ULONG				i=0;
	IDBProperties*		pIDBProperties	=NULL;
	DBPROPIDSET			rgPropertyIDSets;
	DBPROPID			rgDBPrpt[PROPERTY_COUNT];
	WCHAR*				pDescBuffer		=NULL;
	ULONG				cPropertyInfoSets=0;
	DBPROPINFOSET		*pPropertyInfoSets=NULL;

	rgPropertyIDSets.rgPropertyIDs=rgDBPrpt;
	rgPropertyIDSets.cPropertyIDs=PROPERTY_COUNT;
	rgPropertyIDSets.guidPropertySet=DBPROPSET_ROWSET;
	
	//Check if properites are supported
	//Init all the properties
	rgDBPrpt[IDX_Bookmarks]= DBPROP_BOOKMARKS;
	rgDBPrpt[IDX_CanFetchBackwards]= DBPROP_CANFETCHBACKWARDS;
	rgDBPrpt[IDX_ScrollBackwards]= DBPROP_CANSCROLLBACKWARDS;
	rgDBPrpt[IDX_CanHoldRows]= DBPROP_CANHOLDROWS;
	rgDBPrpt[IDX_MaxOpenRows]= DBPROP_MAXOPENROWS;
	rgDBPrpt[IDX_RemoveDeleted]= DBPROP_REMOVEDELETED;
	rgDBPrpt[IDX_BookmarkSkipped]= DBPROP_BOOKMARKSKIPPED;
	rgDBPrpt[IDX_OtherUpdateDelete]=DBPROP_OTHERUPDATEDELETE;
	rgDBPrpt[IDX_OtherInsert]=DBPROP_OTHERINSERT;
	rgDBPrpt[IDX_IRowsetChange]= DBPROP_IRowsetChange;

	//Verify and Create the Interface pointer for IRowsetScroll.
	if(!VerifyInterface(pIDataSourceUnknown, IID_IDBProperties,
			DATASOURCE_INTERFACE,(IUnknown **)&pIDBProperties))
		goto END;
	
	//GetPropertyID
	if(!SUCCEEDED(pIDBProperties->GetPropertyInfo(1,&rgPropertyIDSets,
		&cPropertyInfoSets,&pPropertyInfoSets,&pDescBuffer)))
		goto END;
	
	for(i=0; i<PROPERTY_COUNT; i++)
	{
		if(pPropertyInfoSets->rgPropertyInfos[i].dwFlags & DBPROPFLAGS_WRITE)
			g_rgDBPrpt[i].fSettable=TRUE;
		else
			g_rgDBPrpt[i].fSettable=FALSE;
	}

END:
	SAFE_RELEASE(pIDBProperties);
	COMPARE(FreeProperties(&cPropertyInfoSets,&pPropertyInfoSets,&pDescBuffer),TRUE);
}

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	HRESULT			hr;		
	ULONG			cProperties;
	ULONG			cCnt;
	DBPROPID		rgDBPrpt[PROPERTY_COUNT];
	DBPROPIDSET		rgPropertyIDSets;
	BOOL			fInit=FALSE;
	IUnknown		*pIRowset=NULL;

	DBPROPSET		*prgProperties=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	IDBCreateCommand	*pIDBCreateCommand=NULL;
	IDBSchemaRowset	*pIDBSchemaRowset=NULL;

	// Store a IDBCreateCommand pointer to pThisTestModule->m_pIUnknown
	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;

	// Reset global counters of Scroll accuracy
	g_cVerifyRows=g_ulWeightedError=g_cVerifyApproxPos=g_ulWeightedPosError=0;

	// Create the tables
	g_pCTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);
	g_p1RowTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);
	g_pEmptyTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);

	if(!g_pCTable || !SUCCEEDED(g_pCTable->CreateTable(TABLE_ROW_COUNT)) ) 
	{
		if(g_pCTable)
			delete g_pCTable;
		g_pCTable=NULL;

		odtLog<<wszCreateTableFailed;
		return FALSE;
	}

	g_lRowLast = g_pCTable->GetRowsOnCTable();

	if ( g_lRowLast < 8 )
	{
		odtLog<<L"Need at least an 8 row table for this test!\n";
		return FALSE;
	}

	if(!g_p1RowTable || !SUCCEEDED(g_p1RowTable->CreateTable(1,1,NULL,PRIMARY,TRUE)) )
	{
		if(g_p1RowTable)
			delete g_p1RowTable;
		g_p1RowTable = NULL;
	}

	if ( g_p1RowTable->GetRowsOnCTable() != 1 )
	{
		delete g_p1RowTable;
		g_p1RowTable = NULL;
	}

	if(!g_pEmptyTable || !SUCCEEDED(g_pEmptyTable->CreateTable(0,1,NULL,PRIMARY,TRUE)) )
	{
		if(g_pEmptyTable)
			delete g_pEmptyTable;
		g_pEmptyTable = NULL;
	}

	if ( g_pEmptyTable->GetRowsOnCTable() != 0 )
	{
		delete g_pEmptyTable;
		g_pEmptyTable = NULL;
	}

	// Make sure IRowsetScroll interface is supported by opening a rowset
	hr = g_pCTable->CreateRowset(USE_OPENROWSET,IID_IRowsetScroll,0,NULL,&pIRowset,				
							NULL, NULL);

	// If E_NOINTERFACE is returned, IRowsetScroll is not supported by the provider
	if(hr==E_NOINTERFACE)
	{
		odtLog<<wszIRowsetScrollNotSupported;
		return TEST_SKIPPED;
	}

	if(hr!=S_OK)
	{
		odtLog<<wszExcuteCommandFailed;
		return FALSE;
	}

	rgPropertyIDSets.rgPropertyIDs=rgDBPrpt;
	rgPropertyIDSets.cPropertyIDs=PROPERTY_COUNT;
	rgPropertyIDSets.guidPropertySet=DBPROPSET_ROWSET;
	
	// Check if properites are supported
	// Init all the properties
	rgDBPrpt[IDX_Bookmarks]= DBPROP_BOOKMARKS;
	rgDBPrpt[IDX_CanFetchBackwards]= DBPROP_CANFETCHBACKWARDS;
	rgDBPrpt[IDX_ScrollBackwards]= DBPROP_CANSCROLLBACKWARDS;
	rgDBPrpt[IDX_CanHoldRows]= DBPROP_CANHOLDROWS;
	rgDBPrpt[IDX_MaxOpenRows]= DBPROP_MAXOPENROWS;
	rgDBPrpt[IDX_RemoveDeleted]= DBPROP_REMOVEDELETED;
	rgDBPrpt[IDX_BookmarkSkipped]= DBPROP_BOOKMARKSKIPPED;
	rgDBPrpt[IDX_OtherUpdateDelete]=DBPROP_OTHERUPDATEDELETE;
	rgDBPrpt[IDX_OtherInsert]=DBPROP_OTHERINSERT;
	rgDBPrpt[IDX_IRowsetChange]= DBPROP_IRowsetChange;

	// Get the properties
	if(!VerifyInterface(pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
		return FALSE;

	if(!SUCCEEDED(hr=pIRowsetInfo->GetProperties(1,&rgPropertyIDSets, &cProperties, &prgProperties)))
		goto END;

	// cProperties has to be PROPERTY_COUNT
	if(prgProperties->cProperties!=PROPERTY_COUNT)
		goto END;

	// Mark the properties
	for(cCnt=0;cCnt<prgProperties->cProperties;cCnt++)
	{
		// Mark the properties not supported.
		if(prgProperties->rgProperties[cCnt].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
		{
			g_rgDBPrpt[cCnt].fSupported=FALSE;
			PrintNotSupported(cCnt);
		}
		else
		{	
			g_rgDBPrpt[cCnt].fSupported=TRUE;
			if(cCnt!=IDX_MaxOpenRows)
				g_rgDBPrpt[cCnt].fDefault=V_BOOL(&prgProperties->rgProperties[cCnt].vValue);
			else
				g_cMaxOpenRowsCount=prgProperties->rgProperties[IDX_MaxOpenRows].vValue.lVal;
		}
	}
	InitProp((IUnknown *)pThisTestModule->m_pIUnknown);
	fInit=TRUE;

	//IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
	{
		// Note the limitation and continue.
		g_fIsDBCreateCommandSupported = FALSE;
		odtLog << L"IDBCreateCommand is not supported by Provider." << ENDL;
	} 
	else
	{
		g_fIsDBCreateCommandSupported = TRUE;
	}

	//IDBSchemaRowset
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset))
	{
		// Note the limitation and continue.
		g_fIsDBSchemaRowsetSupported = FALSE;
		odtLog << L"IDBSchemaRowset is not supported by Provider." << ENDL;
	} 
	else
	{
		g_fIsDBSchemaRowsetSupported = TRUE;
	}

END:
	// Release interfaces
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pIDBSchemaRowset);
	
	// Free the memory
	if(prgProperties)
	{
		for(cCnt=0; cCnt<cProperties; cCnt++)
		{
			if(prgProperties[cCnt].rgProperties)
				PROVIDER_FREE(prgProperties[cCnt].rgProperties);
		}

		PROVIDER_FREE(prgProperties);
	}

	return fInit;
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
	// Drop the tables
	if(g_pCTable)
	{
		g_pCTable->DropTable();
		SAFE_DELETE(g_pCTable);
	}

	if(g_p1RowTable)
	{
		g_p1RowTable->DropTable();
		SAFE_DELETE(g_p1RowTable);
	}

	if(g_pEmptyTable)
	{
		g_pEmptyTable->DropTable();
		SAFE_DELETE(g_pEmptyTable);
	}

	odtLog << "The number row verifications: "<<g_cVerifyRows<<".\n";
	odtLog << "The weighted error count for row verification: "<<g_ulWeightedError<<".\n";

	odtLog << "The number Approximate Position verifications:  "<<g_cVerifyApproxPos<<".\n";
	odtLog << "The weighted error count for approximate row verifications: "<<g_ulWeightedPosError<<".\n";

	// Release IDBCreateCommand interface
	return (ModuleReleaseDBSession(pThisTestModule));
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCIRowsetScroll:	The base class for the rest of the test cases in
//						this test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//class TCIRowsetScroll : public COLEDB
class TCIRowsetScroll : public CRowsetObject
{
	private:

	protected:

		//@cmember: Interface pointer for IRowsetScroll
		IRowsetScroll		*m_pIRowsetScroll;

		//@cmember: Interface pointer for IRowset
		IRowset				*m_pIRowset;

		//@cmember: Interface pointer for IRowsetIdentity
		IRowsetIdentity		*m_pIRowsetIdentity;

		//@cmember:	Accessory handle
		HACCESSOR			m_hAccessor;

		//@cmember:	The size of a row
		ULONG_PTR			m_cRowSize;

		//@cmember:	The count of binding structure
		DBCOUNTITEM			m_cBinding;

		//@cmember: The array of binding strucuture
		DBBINDING			*m_rgBinding;

		//@cmember:	The pointer to the row buffer
		void				*m_pData;

		//@cmember
		DBCOUNTITEM			m_ulRowCount;

		//@cmember flag indicating how to openrowset.
		BOOL				m_eRowsetType;

		BOOL				m_fCanHoldRows;

		HROW				*m_pHRows;

		//@mfunc: Initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object, set properties, execute a sql statement,
		//		  and create a rowset object.  Create an accessor on the rowset 
		BOOL GetRowsetAndAccessor
		(	
			CTable				*pCTable,				
			EQUERY				eSQLStmt,				
			IID					riid,					
			ULONG				cProperties=0,			
			const DBPROPID		*rgProperties=NULL,			
			ULONG				cPropertiesUnset=0,
			const DBPROPID		*rgPropertiesUnset=NULL,	
			DBACCESSORFLAGS	dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			WCHAR				*pwszTableName=NULL,	
			EEXECUTE			eExecute=EXECUTE_IFNOERROR,
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY
		);

		//@mfun: Get the bookmark for the row 
		BOOL	GetBookmark
		(
			DBCOUNTITEM	ulRow,
			ULONG_PTR	*pcbBookmark,
			BYTE		**ppBookmark
		);

		//@mfunc: Release the memory referenced by the consumer's buffer
		void FreeMemory(CTable *pCTable=g_pCTable, 
						ECOMPARE_FREE eReadColumnsByRef=FREE_ONLY);

		//@mfunc: Release a rowset object and accessor created on it
		BOOL ReleaseRowsetAndAccessor();

		//@mfunc: Verify the position of the row handle in the row set
		BOOL	_VerifyRowPosition
							(	HROW		hRow,					// Row handle
								DBCOUNTITEM	cRow,					// Position expected
								CTable		*pCTable,				// Pointer to the CTable
								BOOL		fExact=FALSE,			// Does verification need to be exact?
								WCHAR		*wszFile=L"",
								UDWORD		udwLine = 0
							);
															

		//@mfunc: Verify the position of the cursor in the row set
		BOOL	VerifyCursorPosition
							(
							DBCOUNTITEM cRow,			// The cursor position expected
							CTable	*pCTable,			// Pointer to the CTable
							BOOL	fMoveBack=FALSE,	// Whether to move the cursor back to its original postion
														// If fMoveBack==FALSE; the cursor will be positioned one row 
														// after the original position.  If fMoveBack==TRUE,
														// DBPROP_CANSCROLLBACKWARDS needs to be set.
							EVALUE	eValue=PRIMARY,		// eValue for MakeData
							ECOMPARE_FREE	eReadColumnsByRef=FREE_ONLY	// If the accessor is ReadColumnsByRef
							);

		BOOL  _VerifyApproximatePosition
							(
								DBCOUNTITEM ulPosition, 
								DBCOUNTITEM cRows, 
								DBCOUNTITEM ulExpectedPosition,
								WCHAR *wszFile = L"",
								UDWORD udwLine = 0
							);

		BOOL PopulateTable();

		BOOL AlteringRowsIsOK();

		BOOL IsPropertyActive(DBPROPID DBPropID);

		BOOL SetRowsetType(eRowsetType RType);

		static DBCOUNTITEM	GetRatio
							(
								DBCOUNTITEM	cRows,
								DBCOUNTITEM	ulNumerator,
								DBCOUNTITEM	ulDenominator
							);

	public:
		// Constructor
		TCIRowsetScroll(WCHAR *wstrTestCaseName);

		// Destructor
		~TCIRowsetScroll();
};


//--------------------------------------------------------------------
// @mfunc Base class TCIRowsetScroll constructor, must take testcase name
//			as parameter.
//
TCIRowsetScroll::TCIRowsetScroll(WCHAR * wstrTestCaseName)	//Takes TestCase Class name as parameter
						: CRowsetObject (wstrTestCaseName)  
{
	// Initialize member data
	m_pIRowsetScroll = NULL;
	m_pIRowset = NULL;
	m_pIRowsetIdentity = NULL;
	m_hAccessor = NULL;
	m_cRowSize = 0;
	m_cBinding = 0;
	m_rgBinding = NULL;
	m_pData = NULL;
	m_pHRows = NULL;
	m_ulRowCount = 0;
	m_fCanHoldRows = FALSE;
	m_eRowsetType = USEOPENROWSET;
}


//--------------------------------------------------------------------
// @mfunc Base class TCIRowsetScroll destructor
//
TCIRowsetScroll::~TCIRowsetScroll()
{
}

//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetScroll::GetRowsetAndAccessor
(	
	CTable				*pCTable,				//The pointer to the table object	
	EQUERY				eSQLStmt,				//The SQL Statement to create
	IID					riid,						//The interface pointer to return
	ULONG				cProperties,			//The count of properties
	const DBPROPID		*rgProperties,			//The array of properties to be set
	ULONG				cPropertiesUnset,		//The count of properties to be unset
	const DBPROPID		*rgPropertiesUnset,	//The array of properties to be unset	
	DBACCESSORFLAGS		dwAccessorFlags,		//The accessor flags
	DBPART				dwPart,					//The type of binding
	ECOLS_BOUND			eColsToBind,			//The columns in accessor
	ECOLUMNORDER		eBindingOrder,			//The order to bind columns
	ECOLS_BY_REF		eColsByRef,				//Which columns to bind by reference
	WCHAR				*pwszTableName,		//The table name for the join statement
	EEXECUTE			eExecute,				//Execute only if all properties are set
	DBTYPE				dbTypeModifier			//The type modifier used for accessor
)
{
	BOOL			fSucceed=FALSE;
	ULONG			cCnt=0;
	DBPROPSET		DBPropSet;
	IRowsetInfo		*pIRowsetInfo=NULL;
	BOOL			fIsCanHoldRowsRequired = FALSE;

	//Init DBPropSet
	DBPropSet.rgProperties=NULL;
	DBPropSet.cProperties=cProperties+cPropertiesUnset+2;
	DBPropSet.guidPropertySet=DBPROPSET_ROWSET;

	//Allocate, add one for IRowsetScroll, IRowsetIdentity and one for CANHOLDROWS (cheap)
	DBPropSet.rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * ((DBPropSet.cProperties)+3));

	if(!DBPropSet.rgProperties && cProperties==0)
		goto END;

	//Go through the loop to set every DB Property required
	for(cCnt=0;cCnt<cProperties;cCnt++)
	{
		DBPropSet.rgProperties[cCnt].dwPropertyID=rgProperties[cCnt];
		DBPropSet.rgProperties[cCnt].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBPropSet.rgProperties[cCnt].colid=DB_NULLID;
		if (rgProperties[cCnt]==DBPROP_UPDATABILITY)
		{
		 	DBPropSet.rgProperties[cCnt].vValue.vt=VT_I4;
			DBPropSet.rgProperties[cCnt].vValue.lVal=
				DBPROPVAL_UP_CHANGE| DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;
		}
		else if (rgProperties[cCnt]==DBPROP_MAXOPENROWS)
		{
		 	DBPropSet.rgProperties[cCnt].vValue.vt=VT_I4;
			ASSERT(g_cMaxOpenRowsCount <= INT_MAX );
			DBPropSet.rgProperties[cCnt].vValue.lVal=(int)g_cMaxOpenRowsCount;
		}
		else
		{
			if ( rgProperties[cCnt] == DBPROP_CANHOLDROWS )
				fIsCanHoldRowsRequired = TRUE;
			DBPropSet.rgProperties[cCnt].vValue.vt=VT_BOOL;
			V_BOOL(&DBPropSet.rgProperties[cCnt].vValue)=VARIANT_TRUE;
		}
	}

	//Go through the loop to unset every DB Property required
	for(cCnt=cProperties;cCnt<cProperties+cPropertiesUnset;cCnt++)
	{
		DBPropSet.rgProperties[cCnt].dwPropertyID=rgPropertiesUnset[cCnt-cProperties];
		DBPropSet.rgProperties[cCnt].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBPropSet.rgProperties[cCnt].vValue.vt=VT_BOOL;
		DBPropSet.rgProperties[cCnt].colid=DB_NULLID;
		V_BOOL(&DBPropSet.rgProperties[cCnt].vValue)=VARIANT_FALSE;
	}

	//Set IRowsetScroll Property
	DBPropSet.rgProperties[cProperties+cPropertiesUnset].dwPropertyID=DBPROP_IRowsetScroll;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset].vValue.vt=VT_BOOL;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset].colid=DB_NULLID;
	V_BOOL(&DBPropSet.rgProperties[cProperties+cPropertiesUnset].vValue)=VARIANT_TRUE;

	//Set IRowsetIdentity Property
	DBPropSet.rgProperties[cProperties+cPropertiesUnset+1].dwPropertyID=DBPROP_IRowsetIdentity;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset+1].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset+1].vValue.vt=VT_BOOL;
	DBPropSet.rgProperties[cProperties+cPropertiesUnset+1].colid=DB_NULLID;
	V_BOOL(&DBPropSet.rgProperties[cProperties+cPropertiesUnset+1].vValue)=VARIANT_TRUE;

	//Set CanHoldRows (if not set already)
	if ( !fIsCanHoldRowsRequired )
	{
		DBPropSet.rgProperties[cProperties+cPropertiesUnset+2].dwPropertyID=DBPROP_CANHOLDROWS;
		DBPropSet.rgProperties[cProperties+cPropertiesUnset+2].dwOptions=DBPROPOPTIONS_REQUIRED;
		DBPropSet.rgProperties[cProperties+cPropertiesUnset+2].vValue.vt=VT_BOOL;
		DBPropSet.rgProperties[cProperties+cPropertiesUnset+2].colid=DB_NULLID;
		V_BOOL(&DBPropSet.rgProperties[cProperties+cPropertiesUnset+2].vValue)=VARIANT_TRUE;

		(DBPropSet.cProperties)++;
	}

	//Set the properties
	SetRowsetProperties(&DBPropSet, 1);
	
	//Get a rowset
	if ( m_eRowsetType == USECOMMAND )			
	{
		if(!g_fIsDBCreateCommandSupported)
		{
			odtLog << L"Command is not supported on MakeTable." << ENDL;
			goto END;
		}

		//Set CTable object
		SetTable(pCTable, DELETETABLE_NO);

		//Create the rowset object. Fail if properties are not supported.
		if (!SUCCEEDED(m_hr=CreateRowsetObject(eSQLStmt,IID_IAccessor, EXECUTE_IFNOERROR)) )
			goto END;
	}
	else if( m_eRowsetType == USEOPENROWSET )	
	{
		//Set CTable object
		SetTable(pCTable, DELETETABLE_NO);

		//Create the rowset object. Fail if properties are not supported.
		if ( !SUCCEEDED(m_hr=CreateRowsetObject(USE_OPENROWSET)) )
			goto END;
	}
	else if(m_eRowsetType == USESCHEMAR)			
	{
		//Create the rowset object. Fail if properties are not supported.
		if ( !SUCCEEDED(m_hr=CreateRowsetObject(SELECT_DBSCHEMA_PROVIDER_TYPES)) )
			goto END;
	}
	else
		ASSERT(!"Need more support");
	
	//Verify and Create the Interface pointer for IRowsetScroll.
	if(!VerifyInterface(m_pIAccessor, IID_IRowsetScroll,
			ROWSET_INTERFACE,(IUnknown **)&m_pIRowsetScroll))
	{
		odtLog << L"IRowsetScroll is not supported." << ENDL;
		goto END;
	}

	//Verify and Create the Interface pointer for IRowset.
	if(!VerifyInterface(m_pIAccessor, IID_IRowset,
			ROWSET_INTERFACE,(IUnknown **)&m_pIRowset))
	{
		odtLog << L"IRowset is not supported." << ENDL;
		goto END;
	}

	//Verify and Create the Interface pointer for IRowset.
	if(!VerifyInterface(m_pIAccessor, IID_IRowsetIdentity,
			ROWSET_INTERFACE,(IUnknown **)&m_pIRowsetIdentity))
	{
		odtLog << L"IRowsetIdentity is not supported." << ENDL;
		goto END;
	}

	if(FAILED(CountRowsOnRowset(m_pIRowset, &m_ulRowCount)))
		goto END;

	odtLog<<L"	Count of rows in the rowset = "<<m_ulRowCount<<L".\n";
	
	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		goto END;
	}
	else
	{
		DBCOUNTITEM cRowsObtained = 0;

		m_fCanHoldRows = TRUE;

		// pre-fetch all rows.
		if ( !CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, m_ulRowCount, &cRowsObtained, &m_pHRows), S_OK) )
			goto END;

		if (!COMPARE(cRowsObtained, m_ulRowCount))
			goto END;

		if ( !CHECK(m_pIRowset->RestartPosition(DB_NULL_HCHAPTER), S_OK) )
			goto END;
	}

	//If dwAccessorFlags=DBACCESSOR_PASSBYREF, no need to create an accessor
	if(dwAccessorFlags==DBACCESSOR_PASSBYREF)
	{
		fSucceed = TRUE;
		goto END;
	}

	//Create an accessor on the rowset
	if(!CHECK(GetAccessorAndBindings(m_pIRowset,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,NULL,NULL,dbTypeModifier),S_OK))
			goto END;

	//Allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(!m_pData)
		goto END;
	fSucceed = TRUE;

END:

	//Free memory
	if(DBPropSet.rgProperties)			   
		PROVIDER_FREE(DBPropSet.rgProperties);

	SAFE_RELEASE(pIRowsetInfo);

	return fSucceed;
}


//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		 after the GetRowsetAndAccessor that creates an accessor on the 
//		 rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetScroll::GetBookmark
(
	DBCOUNTITEM	ulRow,
	ULONG_PTR	*pcbBookmark,
	BYTE		**ppBookmark
)
{
	BOOL		fPass=FALSE;
	HROW *		pHRow=NULL;
	DBCOUNTITEM cCount;

	// ulRow has to start with 1
	ASSERT(m_pIRowset && pcbBookmark && ppBookmark && ulRow);

	// Restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	// Fetch the row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 (ulRow-1),	// Count of rows to skip before reading.
									 1,			// The number of rows to fetch.
									 &cCount,	// Actual number of fetched rows.
									 &pHRow),	// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	// Only one row handle is retrieved
	COMPARE(cCount, 1);

	// Get the data
	if(!CHECK(m_pIRowset->GetData(*pHRow, m_hAccessor, m_pData),S_OK))
		goto END;

	// Make sure the 0 column is for bookmark
	if(!COMPARE(m_rgBinding[0].iOrdinal, 0))
	{
		FreeMemory();
		goto END;
	}

	// Get the length of the bookmark
	*pcbBookmark= *((ULONG *)((BYTE *)m_pData+m_rgBinding[0].obLength));

	// Allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);

	if(!(*ppBookmark))
		goto END;

	// Copy the value of the bookmark into the consumer's buffer
	memcpy(*ppBookmark, (void *)((BYTE *)m_pData+m_rgBinding[0].obValue), *pcbBookmark);

	// Free the memory referenced by the consumer's buffer
	FreeMemory();

	fPass=TRUE;

END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	return fPass;
}


//--------------------------------------------------------------------
//@mfunc:	Free the memory referenced by the consumer's buffer
//			The function has to be called after IRowset::GetData()
//
//--------------------------------------------------------------------
void TCIRowsetScroll::FreeMemory(CTable *pCTable,ECOMPARE_FREE eReadColumnsByRef)
{
	// Make sure m_pData is not NULL
	if(!COMPARE(!m_pData, NULL))
		return;

	//make sure the columns are bound 
	if(!m_rgTableColOrds)
		return;
	// Call CompareData with the option to free the memory referenced by the consumer's 
	// buffer without comparing data
	//CompareData(m_cColumns,m_rgColumns,1,m_pData,m_cBinding,m_rgBinding,pCTable,
	CompareData(m_cRowsetCols, m_rgTableColOrds,1,m_pData,m_cBinding,m_rgBinding,pCTable,
				NULL,PRIMARY,FREE_ONLY);

	return;
}


//--------------------------------------------------------------------
//@mfunc: Release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
BOOL TCIRowsetScroll::ReleaseRowsetAndAccessor()
{
	BOOL		fPass=TRUE;

	m_ulRowCount=0;

	// Free the consumer's buffer
	PROVIDER_FREE(m_pData);
		
	// Free binding structure
	PROVIDER_FREE(m_rgBinding);
		

	// Free accessor handle
	if(m_hAccessor)
	{
		if(!m_pIAccessor)
			// Get an IAccessor pointer if it was not yet a valid pointer 
			if(VerifyInterface(m_pIRowset,IID_IAccessor,
						ROWSET_INTERFACE,(IUnknown **)&m_pIAccessor))
				fPass=FALSE;


		if(m_pIAccessor)
		{
			if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fPass=FALSE;

		}

		m_hAccessor=NULL;
	}
	
	if ( m_fCanHoldRows && m_pIRowset && m_pHRows )
	{
		if (!CHECK(m_pIRowset->ReleaseRows(m_ulRowCount, m_pHRows, NULL, NULL, NULL), S_OK))
			fPass=FALSE;


		PROVIDER_FREE(m_pHRows);
	}

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetScroll);
	SAFE_RELEASE(m_pIRowsetIdentity);

	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	return fPass;
}


//--------------------------------------------------------------------
//
//@mfunc: Verify the position of the row handle in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor
//
//--------------------------------------------------------------------

BOOL	TCIRowsetScroll::_VerifyRowPosition
(
	HROW		hRow,		// Row handle
	DBCOUNTITEM cRow,		// Position expected
	CTable *	pCTable,	// Pointer to the CTable
	BOOL		fExact,		// Enforce exactness
	WCHAR *		wszFile,
	UDWORD		udwLine			
)
{
	// Input validation
	if(!pCTable)
		return FALSE;

	// m_pIRowset has to be valid
	if(!m_pIRowset || !m_pData || !m_pIRowsetIdentity)
		return FALSE;

	g_cVerifyRows++;

	if ( m_fCanHoldRows )
	{
		if ( FAILED(m_hr = m_pIRowsetIdentity->IsSameRow(hRow, m_pHRows[cRow-1])) )
			return FALSE;	

		if ( m_hr == S_OK )
			return TRUE;

		if ( m_hr == S_FALSE )
		{
			// sequentially search until a match.
			for ( ULONG i = 1; i<=m_ulRowCount; i++ )
			{
				if ( FAILED(m_hr = m_pIRowsetIdentity->IsSameRow(hRow, m_pHRows[i-1])) )
				{
					if( m_hr == DB_E_DELETEDROW )
						continue;
					else
						return FALSE;
				}

				if ( m_hr == S_OK )
				{
					odtLog<<" Inexact Position occurred. Recieved: "<<i<<". Ideally expecting row: "<<cRow<<".\n";
					odtLog<<" Called from "<<wszFile<<", Line: "<<udwLine<<".\n";
					g_ulWeightedError += (i-cRow)*(i-cRow);

					if ( fExact )
						return FALSE; 
					else
						return TRUE;
				}
			}
		}

		// no match, should never happen
		odtLog << "Compare row didn't exist in rowset!\n";
		return FALSE;
	}


	// Get Data for the row
	if(!CHECK(m_pIRowset->GetData(hRow,m_hAccessor,m_pData),S_OK))
		return FALSE;

	// Compare the data with the row expected in the rowset
	//if(!CompareData(m_cColumns,m_rgColumns,cRow,m_pData,m_cBinding,m_rgBinding,pCTable,
	if(!CompareData(m_cRowsetCols, m_rgTableColOrds,cRow,m_pData,m_cBinding,m_rgBinding,pCTable,
					NULL,PRIMARY,FREE_ONLY,COMPARE_UNTIL_ERROR))
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//
//@mfunc: Verify the position of the cursor in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor.  
//
//--------------------------------------------------------------------

BOOL	TCIRowsetScroll::VerifyCursorPosition
(
	DBCOUNTITEM		cRow,				// The cursor position expected
	CTable			*pCTable,			// Pointer to the CTable
	BOOL			fMoveBack,			// Whether to move the cursor back to its original postion
										// If fMoveBack==FALSE; the cursor will be positioned one row 
										// after the original position.  If fMoveBack==TRUE,
										// DBPROP_CANSCROLLBACKWARDS needs to be set.
	EVALUE			eValue,				// eValue for MakeData
	ECOMPARE_FREE	eReadColumnsByRef	// If the accessor is ReadColumnsByRef
)
{
	HROW		hRow[1];
	HROW *		pHRow=hRow;
	DBCOUNTITEM cRows;
	BOOL		fTestPass=TRUE;

	// Input validation
	if(COMPARE(pCTable, NULL))
		return FALSE;

	// m_pIRowset has to be valid
	if(COMPARE(m_pIRowset, NULL) ||
	   COMPARE(m_pData,NULL))
		return FALSE;

	// Get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		return FALSE;
	
	if(!VerifyRowPosition(hRow[0],cRow,pCTable,EXACT))
		fTestPass=FALSE;
	
	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);

	// Reposition the cursor to its original position
	if(fMoveBack)
	{
		if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,0,&cRows,&pHRow),S_OK))
			fTestPass=FALSE;
	}

	return (fTestPass);
}

BOOL TCIRowsetScroll::_VerifyApproximatePosition
(
	DBCOUNTITEM ulPosition, 
	DBCOUNTITEM cRows, 
	DBCOUNTITEM ulExpectedPosition, 
	WCHAR *		wszFile,
	UDWORD		udwLine 
)
{
	BOOL fWarning = FALSE, fPass = TRUE;
	g_cVerifyApproxPos++;
	
	if (!COMPARE((cRows >= ulPosition), TRUE))
	{
		fPass = FALSE;
		fWarning = TRUE;
		goto END;
	}

	if ( ulPosition != ulExpectedPosition )
	{
		odtLog << wszInexactPositionWarning; 
		odtLog << "Expected *pulPosition: "<<ulExpectedPosition<<" Received: "<<ulPosition<<".\n";
		fWarning = TRUE;
		g_ulWeightedPosError+=(ulPosition-ulExpectedPosition)*(ulPosition-ulExpectedPosition);
	}

	if ( cRows != m_ulRowCount )
	{
		odtLog << wszInexactRowCountWarning; 
		odtLog << "Expected *pcRows: "<<m_ulRowCount<<" Received: "<<cRows<<".\n";
		fWarning = TRUE;		
		g_ulWeightedPosError+=(cRows-m_ulRowCount)*(cRows-m_ulRowCount);
	}

END:

	if ( fWarning )
		odtLog << L"Called from "<<wszFile<<". Line: "<<udwLine<<L".\n";

	return fPass;
}

//--------------------------------------------------------------------
//
//@mfunc: Replenish the table
//
//--------------------------------------------------------------------
BOOL	TCIRowsetScroll::PopulateTable()
{ 	
	//delete all rows in the table.
	if(!CHECK(g_pCTable->DeleteRows(ALLROWS),S_OK))
		return FALSE;

	// Regenerate the rowset
	if(!CHECK(g_pCTable->Insert(PRIMARY, 1, g_lRowLast),S_OK))
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------
//
//@mfunc: verify that updating / deleting rows is OK against this provider.
//
//	The result of this particular function is independant of whether the provider
// allows updates/deletes.  The focus is rather whether deleting a row will
// cause the table to fall into an "unknown" state.
//
//--------------------------------------------------------------------
BOOL TCIRowsetScroll::AlteringRowsIsOK()
{
	// If a specific table was set on the backend, assume we cannot alter it.
	if ( GetModInfo()->GetTableName() && !GetModInfo()->GetFileName() )
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------
//@mfunc:	Check if a property is supported on a rowset
//--------------------------------------------------------------------

BOOL TCIRowsetScroll::IsPropertyActive(DBPROPID DBPropID)
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//QI for IRowsetInfo interface
	if(!CHECK(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK))
		goto CLEANUP;

	//ask for DBPROP_BOOKMARKSKIPPED
	if(!CHECK(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,
		&pDBPropSet),S_OK))
		goto CLEANUP;

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
	{
		if(pDBPropSet->rgProperties)
			PROVIDER_FREE(pDBPropSet->rgProperties);

		PROVIDER_FREE(pDBPropSet);
	}

	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

BOOL TCIRowsetScroll::SetRowsetType(eRowsetType RType)
{
	m_eRowsetType = RType;

	switch ( RType )
	{
	case USECOMMAND:
		return g_fIsDBCreateCommandSupported;
	case USESCHEMAR:
		return g_fIsDBSchemaRowsetSupported;
	default:
		return TRUE;
	}
}

DBCOUNTITEM	TCIRowsetScroll::GetRatio
(
	DBCOUNTITEM	cRows,
	DBCOUNTITEM	ulNumerator,
	DBCOUNTITEM	ulDenominator
)
{
	return (DBCOUNTITEM)(((double) ulNumerator * cRows) / ulDenominator);
}


//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
// a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetScroll::Init()
{
	return (CRowsetObject::Init());
}

//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetScroll::Terminate()
{	
	return (CRowsetObject::Terminate());
}


//--------------------------------------------------------------------
// @class test DBPROP_BOOKMARKS
//
class Bookmarks : public TCIRowsetScroll {
private:

public:
	~Bookmarks (void) {};								
    Bookmarks ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember *pBookmark=DBBMK_FIRST, S_OK
	int Variation_1();
	// @cmember *pBookmark=DBBMK_LAST. S_OK is returned
	int Variation_2();
	// @cmember *pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK
	int Variation_3();
	// @cmember Get one row handle at a time
	int Variation_4();
};

// {{ TCW_TEST_CASE_MAP(Bookmarks_cmd)
//--------------------------------------------------------------------
// @class test DBPROP_BOOKMARKS
//
class Bookmarks_cmd : public Bookmarks {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmarks_cmd,Bookmarks);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Bookmarks_cmd)
#define THE_CLASS Bookmarks_cmd
BEG_TEST_CASE(Bookmarks_cmd, Bookmarks, L"test DBPROP_BOOKMARKS")
	TEST_VARIATION(1,		L"*pBookmark=DBBMK_FIRST, S_OK")
	TEST_VARIATION(2,		L"*pBookmark=DBBMK_LAST. S_OK is returned")
	TEST_VARIATION(3,		L"*pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK")
	TEST_VARIATION(4,		L"Get one row handle at a time")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Bookmarks_SchemaR)
//--------------------------------------------------------------------
// @class Using Schema Rowsets
//
class Bookmarks_SchemaR : public Bookmarks {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmarks_SchemaR,Bookmarks);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};

// {{ TCW_TESTCASE(Bookmarks_SchemaR)
#define THE_CLASS Bookmarks_SchemaR
BEG_TEST_CASE(Bookmarks_SchemaR, Bookmarks, L"Using Schema Rowsets")
	TEST_VARIATION(1,		L"*pBookmark=DBBMK_FIRST, S_OK")
	TEST_VARIATION(2,		L"*pBookmark=DBBMK_LAST. S_OK is returned")
	TEST_VARIATION(3,		L"*pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK")
	TEST_VARIATION(4,		L"Get one row handle at a time")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Bookmarks_OpenRowset)
//--------------------------------------------------------------------
// @class In Context of IOpenRowset
//
class Bookmarks_OpenRowset : public Bookmarks {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmarks_OpenRowset,Bookmarks);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Bookmarks_OpenRowset)
#define THE_CLASS Bookmarks_OpenRowset
BEG_TEST_CASE(Bookmarks_OpenRowset, Bookmarks, L"In Context of IOpenRowset")
	TEST_VARIATION(1,		L"*pBookmark=DBBMK_FIRST, S_OK")
	TEST_VARIATION(2,		L"*pBookmark=DBBMK_LAST. S_OK is returned")
	TEST_VARIATION(3,		L"*pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK")
	TEST_VARIATION(4,		L"Get one row handle at a time")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

//--------------------------------------------------------------------
// @class Test DBPROP_CANHOLDROWS property
//
class CanHoldRows : public TCIRowsetScroll {
private:

public:

	~CanHoldRows (void) {};								
    CanHoldRows ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember Get first two rows, get next three rows, pass bookmark of second row. S_OK returned.
	int Variation_1();
	// @cmember Get first two rows, *pBookmark=second row bookmark. S_OK returned.
	int Variation_2();
	// @cmember Get first 5 rows, get last 5 rows, ulNumerator=1, ulDenominator=2, S_OK and 10th row returned.
	int Variation_3();
};


// {{ TCW_TEST_CASE_MAP(CanHoldRows_cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class CanHoldRows_cmd : public CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanHoldRows_cmd,CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanHoldRows_cmd)
#define THE_CLASS CanHoldRows_cmd
BEG_TEST_CASE(CanHoldRows_cmd, CanHoldRows, L"In context of commands")
	TEST_VARIATION(1,		L"Get first two rows, get next three rows, pass bookmark of second row. S_OK returned.")
	TEST_VARIATION(2,		L"Get first two rows, *pBookmark=second row bookmark. S_OK returned.")
	TEST_VARIATION(3,		L"Get first 5 rows, get last 5 rows, ulNumerator=1, ulDenominator=2, S_OK and middle row returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanHoldRows_SchemaR)
//--------------------------------------------------------------------
// @class In context of Schema rowsets
//
class CanHoldRows_SchemaR : public CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanHoldRows_SchemaR,CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanHoldRows_SchemaR)
#define THE_CLASS CanHoldRows_SchemaR
BEG_TEST_CASE(CanHoldRows_SchemaR, CanHoldRows, L"In context of Schema rowsets")
	TEST_VARIATION(1,		L"Get first two rows, get next three rows, pass bookmark of second row. S_OK returned.")
	TEST_VARIATION(2,		L"Get first two rows, *pBookmark=second row bookmark. S_OK returned.")
	TEST_VARIATION(3,		L"Get first 5 rows, get last 5 rows, ulNumerator=1, ulDenominator=2, S_OK and middle row returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanHoldRows_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class CanHoldRows_OpenRowset : public CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanHoldRows_OpenRowset,CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanHoldRows_OpenRowset)
#define THE_CLASS CanHoldRows_OpenRowset
BEG_TEST_CASE(CanHoldRows_OpenRowset, CanHoldRows, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"Get first two rows, get next three rows, pass bookmark of second row. S_OK returned.")
	TEST_VARIATION(2,		L"Get first two rows, *pBookmark=second row bookmark. S_OK returned.")
	TEST_VARIATION(3,		L"Get first 5 rows, get last 5 rows, ulNumerator=1, ulDenominator=2, S_OK and middle row returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test DBPROP_CANFETCHBACKWARDS
//
class CanFetchBackwards : public TCIRowsetScroll {
private:
	
public:

	~CanFetchBackwards (void) {};								
    CanFetchBackwards ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember ulNumerator=1, ulDenominator=20, cRows=-2
	int Variation_1();
	// @cmember ulNumerator=1, ulDenominator=4, cRows=-6
	int Variation_2();
	// @cmember ulNumerator=3, ulDenominator=5, cRows=-2
	int Variation_3();
	// @cmember ulNumerator=lastrow-1, ulDenominator=lastrow, cRows=2. DB_S_ENDOFROWSET.
	int Variation_4();	
};

// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class CanFetchBackwards_Cmd : public CanFetchBackwards {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_Cmd,CanFetchBackwards);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_Cmd)
#define THE_CLASS CanFetchBackwards_Cmd
BEG_TEST_CASE(CanFetchBackwards_Cmd, CanFetchBackwards, L"In context of commands")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-2")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=4, cRows=-6")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=5, cRows=-2")
	TEST_VARIATION(4,		L"ulNumerator=lastrow-1, ulDenominator=lastrow, cRows=2. DB_S_ENDOFROWSET.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class CanFetchBackwards_SchemaR : public CanFetchBackwards {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_SchemaR,CanFetchBackwards);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_SchemaR)
#define THE_CLASS CanFetchBackwards_SchemaR
BEG_TEST_CASE(CanFetchBackwards_SchemaR, CanFetchBackwards, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-2")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=4, cRows=-6")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=5, cRows=-2")
	TEST_VARIATION(4,		L"ulNumerator=lastrow-1, ulDenominator=lastrow, cRows=2. DB_S_ENDOFROWSET.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class CanFetchBackwards_OpenRowset : public CanFetchBackwards {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_OpenRowset,CanFetchBackwards);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_OpenRowset)
#define THE_CLASS CanFetchBackwards_OpenRowset
BEG_TEST_CASE(CanFetchBackwards_OpenRowset, CanFetchBackwards, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-2")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=4, cRows=-6")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=5, cRows=-2")
	TEST_VARIATION(4,		L"ulNumerator=lastrow-1, ulDenominator=lastrow, cRows=2. DB_S_ENDOFROWSET.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test DBPROP_CANFETCHBACKWARDS + DBPROP_CANHOLDROWS
//
class CanFetchBackwards_CanHoldRows : public TCIRowsetScroll {
private:
	
public:
	~CanFetchBackwards_CanHoldRows (void) {};								
    CanFetchBackwards_CanHoldRows ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember ulNumerator=1, ulDenominator=20, cRows=-1
	int Variation_1();
	// @cmember ulNumerator=3, ulDenominator=20, cRows=-3
	int Variation_2();
};


// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_CanHoldRows_Cmd)
//--------------------------------------------------------------------
// @class In context of Commands
//
class CanFetchBackwards_CanHoldRows_Cmd : public CanFetchBackwards_CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_CanHoldRows_Cmd,CanFetchBackwards_CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_CanHoldRows_Cmd)
#define THE_CLASS CanFetchBackwards_CanHoldRows_Cmd
BEG_TEST_CASE(CanFetchBackwards_CanHoldRows_Cmd, CanFetchBackwards_CanHoldRows, L"In context of Commands")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=3, ulDenominator=RowCount, cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_CanHoldRows_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class CanFetchBackwards_CanHoldRows_SchemaR : public CanFetchBackwards_CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_CanHoldRows_SchemaR,CanFetchBackwards_CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_CanHoldRows_SchemaR)
#define THE_CLASS CanFetchBackwards_CanHoldRows_SchemaR
BEG_TEST_CASE(CanFetchBackwards_CanHoldRows_SchemaR, CanFetchBackwards_CanHoldRows, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=3, ulDenominator=RowCount, cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(CanFetchBackwards_CanHoldRows_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class CanFetchBackwards_CanHoldRows_OpenRowset : public CanFetchBackwards_CanHoldRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanFetchBackwards_CanHoldRows_OpenRowset,CanFetchBackwards_CanHoldRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(CanFetchBackwards_CanHoldRows_OpenRowset)
#define THE_CLASS CanFetchBackwards_CanHoldRows_OpenRowset
BEG_TEST_CASE(CanFetchBackwards_CanHoldRows_OpenRowset, CanFetchBackwards_CanHoldRows, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=RowCount, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=3, ulDenominator=RowCount, cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test DBPROP_MAXOPENROWS
//
class MaxOpenRows : public TCIRowsetScroll {
private:
	
public:
	~MaxOpenRows (void) {};								
    MaxOpenRows ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember MAXOPENROWS=3, ratio=9/10, and cRows=4. DB_S_ENDOFROWSET is returned.
	int Variation_1();
	// @cmember MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.
	int Variation_2();
	// @cmember MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.
	int Variation_3();

};


// {{ TCW_TEST_CASE_MAP(MaxOpenRows_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class MaxOpenRows_Cmd : public MaxOpenRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxOpenRows_Cmd,MaxOpenRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(MaxOpenRows_Cmd)
#define THE_CLASS MaxOpenRows_Cmd
BEG_TEST_CASE(MaxOpenRows_Cmd, MaxOpenRows, L"In context of commands")
	TEST_VARIATION(1,		L"MAXOPENROWS=3, ratio=9/10, and cRows=4. DB_S_ENDOFROWSET is returned.")
	TEST_VARIATION(2,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
	TEST_VARIATION(3,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(MaxOpenRows_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class MaxOpenRows_SchemaR : public MaxOpenRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxOpenRows_SchemaR,MaxOpenRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(MaxOpenRows_SchemaR)
#define THE_CLASS MaxOpenRows_SchemaR
BEG_TEST_CASE(MaxOpenRows_SchemaR, MaxOpenRows, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"MAXOPENROWS=3, ratio=9/10, and cRows=4. DB_S_ENDOFROWSET is returned.")
	TEST_VARIATION(2,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
	TEST_VARIATION(3,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(MaxOpenRows_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class MaxOpenRows_OpenRowset : public MaxOpenRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxOpenRows_OpenRowset,MaxOpenRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(MaxOpenRows_OpenRowset)
#define THE_CLASS MaxOpenRows_OpenRowset
BEG_TEST_CASE(MaxOpenRows_OpenRowset, MaxOpenRows, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"MAXOPENROWS=3, ratio=9/10, and cRows=4. DB_S_ENDOFROWSET is returned.")
	TEST_VARIATION(2,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
	TEST_VARIATION(3,		L"MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Delete rows from the rowset
//
class DeleteRows : public TCIRowsetScroll {
private:
	
public:
	~DeleteRows (void) {};								
    DeleteRows ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember *pBookmark=second row bookmark, delete second row.  DB_E_DELETEDROW is returned.
	int Variation_1();
	// @cmember *pBookmark=DBBMK_LAST, delete last row.  S_OK is returned.
	int Variation_2();
	// @cmember Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.
	int Variation_3();
	// @cmember Delete 10th row. ulNumerator=1, ulDenominator=2, and cRows=1. S_OK and middle handle returned.
	int Variation_4();
	// @cmember Delete last row. ulNumerator=RowCount, ulDenominator=RowCount, S_OK and 20th row handle returned.
	int Variation_5();
};


// {{ TCW_TEST_CASE_MAP(DeleteRows_cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class DeleteRows_cmd : public DeleteRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows_cmd,DeleteRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(DeleteRows_cmd)
#define THE_CLASS DeleteRows_cmd
BEG_TEST_CASE(DeleteRows_cmd, DeleteRows, L"In context of commands")
	TEST_VARIATION(1,		L"*pBookmark=second row bookmark, delete second row.  DB_E_DELETEDROW is returned.")
	TEST_VARIATION(2,		L"*pBookmark=DBBMK_LAST, delete last row.  S_OK is returned.")
	TEST_VARIATION(3,		L"Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.")
	TEST_VARIATION(4,		L"Delete 10th row. ulNumerator=1, ulDenominator=2, and cRows=1. S_OK and middle handle returned.")
	TEST_VARIATION(5,		L"Delete last row. ulNumerator=RowCount, ulDenominator=RowCount, S_OK and 20th row handle returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(DeleteRows_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class DeleteRows_OpenRowset : public DeleteRows {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows_OpenRowset,DeleteRows);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(DeleteRows_OpenRowset)
#define THE_CLASS DeleteRows_OpenRowset
BEG_TEST_CASE(DeleteRows_OpenRowset, DeleteRows, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"*pBookmark=second row bookmark, delete second row.  DB_E_DELETEDROW is returned.")
	TEST_VARIATION(2,		L"*pBookmark=DBBMK_LAST, delete last row.  S_OK is returned.")
	TEST_VARIATION(3,		L"Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.")
	TEST_VARIATION(4,		L"Delete 10th row. ulNumerator=1, ulDenominator=2, and cRows=1. S_OK and middle handle returned.")
	TEST_VARIATION(5,		L"Delete last row. ulNumerator=RowCount, ulDenominator=RowCount, S_OK and 20th row handle returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class test DBPROP_REMOVEDELETED
//
class RemoveDeleted : public TCIRowsetScroll {
private:
	
public:
	~RemoveDeleted (void) {};								
    RemoveDeleted ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember *pBookmark points to a deleted row.  DB_E_BADBOOKMARD is returned.
	int Variation_1();
	// @cmember Delete the 3rd row.  *pBookmark points to the second row. S_OK is returned. 
	int Variation_2();
	// @cmember Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.
	int Variation_3();
	// @cmember Delete 18th row. ulNumerator=9, ulDenominator=10, and cRows=3. DB_S_ENDOFROWSET is returned.
	int Variation_4();
	// @cmember Delete 19th row. ulNumerator=19, ulDenominator=RowCount, and cRows=1. S_OK and 20th row handle returned.
	int Variation_5();
	// @cmember Delete Half the row. Use ratio of 3/4
	int Variation_6();
};

// {{ TCW_TEST_CASE_MAP(RemoveDeleted_Cmd)
//--------------------------------------------------------------------
// @class In context of Commands
//
class RemoveDeleted_Cmd : public RemoveDeleted {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RemoveDeleted_Cmd,RemoveDeleted);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(RemoveDeleted_Cmd)
#define THE_CLASS RemoveDeleted_Cmd
BEG_TEST_CASE(RemoveDeleted_Cmd, RemoveDeleted, L"In context of Commands")
	TEST_VARIATION(1,		L"*pBookmark points to a deleted row.  DB_E_BADBOOKMARD is returned.")
	TEST_VARIATION(2,		L"Delete the 3rd row.  *pBookmark points to the second row. S_OK is returned. ")
	TEST_VARIATION(3,		L"Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.")
	TEST_VARIATION(4,		L"Delete 18th row. ulNumerator=9, ulDenominator=10, and cRows=3. DB_S_ENDOFROWSET is returned.")
	TEST_VARIATION(5,		L"Delete 19th row. ulNumerator=19, ulDenominator=RowCount, and cRows=1. S_OK and 20th row handle returned.")
	TEST_VARIATION(6,		L"Delete half the row. Use ratio of 3/4. Verify ratio applies to undeleted rows.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(RemoveDeleted_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class RemoveDeleted_OpenRowset : public RemoveDeleted {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RemoveDeleted_OpenRowset,RemoveDeleted);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(RemoveDeleted_OpenRowset)
#define THE_CLASS RemoveDeleted_OpenRowset
BEG_TEST_CASE(RemoveDeleted_OpenRowset, RemoveDeleted, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"*pBookmark points to a deleted row.  DB_E_BADBOOKMARD is returned.")
	TEST_VARIATION(2,		L"Delete the 3rd row.  *pBookmark points to the second row. S_OK is returned. ")
	TEST_VARIATION(3,		L"Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.")
	TEST_VARIATION(4,		L"Delete 18th row. ulNumerator=9, ulDenominator=10, and cRows=3. DB_S_ENDOFROWSET is returned.")
	TEST_VARIATION(5,		L"Delete 19th row. ulNumerator=19, ulDenominator=RowCount, and cRows=1. S_OK and 20th row handle returned.")
	TEST_VARIATION(6,		L"Delete half the row. Use ratio of 3/4. Verify ratio applies to undeleted rows.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test single row rowset
//
class Rowset_SingleRow : public TCIRowsetScroll {
private:
	
public:
	~Rowset_SingleRow (void) {};								
    Rowset_SingleRow ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember *pBookmark=first row bookmark.  S_OK is returned.
	int Variation_1();
	// @cmember ulNumerator=1, ulDenominator=1, cRows=-1. S_OK returned.
	int Variation_2();
	// @cmember ulNumerator=0, ulDenominator=1, cRows=1. S_OK returned.
	int Variation_3();
	// @cmember *pBookmark=DBBMK_LAST.  S_OK is returned.
	int Variation_4();
	// @cmember *pBookmark=DBBMK_FIRST  S_OK is returned.
	int Variation_5();
	// @cmember ulNumerator=1, ulDenominator=3, cRows=1. S_OK returned.
	int Variation_6();
};


// {{ TCW_TEST_CASE_MAP(Rowset_SingleRow_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Rowset_SingleRow_Cmd : public Rowset_SingleRow {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_SingleRow_Cmd,Rowset_SingleRow);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Rowset_SingleRow_Cmd)
#define THE_CLASS Rowset_SingleRow_Cmd
BEG_TEST_CASE(Rowset_SingleRow_Cmd, Rowset_SingleRow, L"In context of commands")
	TEST_VARIATION(1,		L"*pBookmark=first row bookmark.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=1, cRows=-1. S_OK returned.")
	TEST_VARIATION(3,		L"ulNumerator=0, ulDenominator=1, cRows=1. S_OK returned.")
	TEST_VARIATION(4,		L"*pBookmark=DBBMK_LAST.  S_OK is returned.")
	TEST_VARIATION(5,		L"*pBookmark=DBBMK_FIRST  S_OK is returned.")
	TEST_VARIATION(6,		L"ulNumerator=1, ulDenominator=3, cRows=1. S_OK returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Rowset_SingleRow_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Rowset_SingleRow_OpenRowset : public Rowset_SingleRow {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_SingleRow_OpenRowset,Rowset_SingleRow);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Rowset_SingleRow_OpenRowset)
#define THE_CLASS Rowset_SingleRow_OpenRowset
BEG_TEST_CASE(Rowset_SingleRow_OpenRowset, Rowset_SingleRow, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"*pBookmark=first row bookmark.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=1, cRows=-1. S_OK returned.")
	TEST_VARIATION(3,		L"ulNumerator=0, ulDenominator=1, cRows=1. S_OK returned.")
	TEST_VARIATION(4,		L"*pBookmark=DBBMK_LAST.  S_OK is returned.")
	TEST_VARIATION(5,		L"*pBookmark=DBBMK_FIRST  S_OK is returned.")
	TEST_VARIATION(6,		L"ulNumerator=1, ulDenominator=3, cRows=1. S_OK returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test empty rowset
//
class Rowset_Empty : public TCIRowsetScroll {
private:
	
public:
	~Rowset_Empty (void) {};								
    Rowset_Empty ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember *pBookmark=DBBMK_FIRST.  S_OK is returned.
	int Variation_1();
	// @cmember ulNumerator=1, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET and no row handle returned.
	int Variation_2();
	// @cmember ulNumerator=1, ulDenominator=1, cRows=0. S_OK and no row handle returned.
	int Variation_3();
	// @cmember ulNumerator=0, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET
	int Variation_4();
	// @cmember *pBookmark=DBBMK_LAST.  S_OK is returned.
	int Variation_5();
};


// {{ TCW_TEST_CASE_MAP(Rowset_Empty_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Rowset_Empty_Cmd : public Rowset_Empty {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_Empty_Cmd,Rowset_Empty);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Rowset_Empty_Cmd)
#define THE_CLASS Rowset_Empty_Cmd
BEG_TEST_CASE(Rowset_Empty_Cmd, Rowset_Empty, L"In context of commands")
	TEST_VARIATION(1,		L"*pBookmark=DBBMK_FIRST.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET and no row handle returned.")
	TEST_VARIATION(3,		L"ulNumerator=1, ulDenominator=1, cRows=0. S_OK and no row handle returned.")
	TEST_VARIATION(4,		L"ulNumerator=0, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET")
	TEST_VARIATION(5,		L"*pBookmark=DBBMK_LAST.  S_OK is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Rowset_Empty_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Rowset_Empty_OpenRowset : public Rowset_Empty {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_Empty_OpenRowset,Rowset_Empty);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Rowset_Empty_OpenRowset)
#define THE_CLASS Rowset_Empty_OpenRowset
BEG_TEST_CASE(Rowset_Empty_OpenRowset, Rowset_Empty, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"*pBookmark=DBBMK_FIRST.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=1, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET and no row handle returned.")
	TEST_VARIATION(3,		L"ulNumerator=1, ulDenominator=1, cRows=0. S_OK and no row handle returned.")
	TEST_VARIATION(4,		L"ulNumerator=0, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET")
	TEST_VARIATION(5,		L"*pBookmark=DBBMK_LAST.  S_OK is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Boundary conditions for GetApproximatePosition
//
class Boundary_GetApproximatePosition : public TCIRowsetScroll {
private:
	
	ULONG_PTR	m_cbBookmark;
	BYTE		*m_pBookmark;
	DBBOOKMARK	m_DBBookmark;
	DBCOUNTITEM	m_pulPosition;
	DBCOUNTITEM	m_pcRows;

public:
	~Boundary_GetApproximatePosition(void) {};								
    Boundary_GetApproximatePosition ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember pBookmark==Null pointer. E_INVALIDARG is returned.
	int Variation_1();
	// @cmember pBookmark==NULL. DB_E_BADBOOKMARK is returned.
	int Variation_2();
	// @cmember pBookmark points to a invalid bookmark. DB_E_BADBOOKMARK is returned.
	int Variation_3();
	// @cmember pcRows==Null pointer. S_OK and no count of rows returned.
	int Variation_4();
	// @cmember pulPosition is NULL, S_OK
	int Variation_5();
	// @cmember cbBookmark = 0, pBookmark = NULL
	int Variation_6();
};


// {{ TCW_TEST_CASE_MAP(Boundary_GetApproximatePosition_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Boundary_GetApproximatePosition_Cmd : public Boundary_GetApproximatePosition {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetApproximatePosition_Cmd,Boundary_GetApproximatePosition);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetApproximatePosition_Cmd)
#define THE_CLASS Boundary_GetApproximatePosition_Cmd
BEG_TEST_CASE(Boundary_GetApproximatePosition_Cmd, Boundary_GetApproximatePosition, L"In context of commands")
	TEST_VARIATION(1,		L"pBookmark==Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"pBookmark==NULL. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(3,		L"pBookmark points to a invalid bookmark. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(4,		L"pcRows==Null pointer. S_OK and no count of rows returned.")
	TEST_VARIATION(5,		L"pulPosition is NULL, S_OK")
	TEST_VARIATION(6,		L"cbBookmark = 0, pBookmark = NULL")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Boundary_GetApproximatePosition_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Boundary_GetApproximatePosition_OpenRowset : public Boundary_GetApproximatePosition {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetApproximatePosition_OpenRowset,Boundary_GetApproximatePosition);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetApproximatePosition_OpenRowset)
#define THE_CLASS Boundary_GetApproximatePosition_OpenRowset
BEG_TEST_CASE(Boundary_GetApproximatePosition_OpenRowset, Boundary_GetApproximatePosition, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"pBookmark==Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"pBookmark==NULL. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(3,		L"pBookmark points to a invalid bookmark. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(4,		L"pcRows==Null pointer. S_OK and no count of rows returned.")
	TEST_VARIATION(5,		L"pulPosition is NULL, S_OK")
	TEST_VARIATION(6,		L"cbBookmark = 0, pBookmark = NULL")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Boundary_GetApproximatePosition_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class Boundary_GetApproximatePosition_SchemaR : public Boundary_GetApproximatePosition {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetApproximatePosition_SchemaR,Boundary_GetApproximatePosition);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetApproximatePosition_SchemaR)
#define THE_CLASS Boundary_GetApproximatePosition_SchemaR
BEG_TEST_CASE(Boundary_GetApproximatePosition_SchemaR, Boundary_GetApproximatePosition, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"pBookmark==Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"pBookmark==NULL. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(3,		L"pBookmark points to a invalid bookmark. DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(4,		L"pcRows==Null pointer. S_OK and no count of rows returned.")
	TEST_VARIATION(5,		L"pulPosition is NULL, S_OK")
	TEST_VARIATION(6,		L"cbBookmark = 0, pBookmark = NULL")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Boundary conditions for GetRowsAtRatio
//
class Boundary_GetRowsAtRatio : public TCIRowsetScroll {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	DBCOUNTITEM	m_cRowsObtained;
	HROW		m_hRows;
	HROW		*m_prghRows;
	
public:
	~Boundary_GetRowsAtRatio(void) {};								
    Boundary_GetRowsAtRatio ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	

	// @cmember pcRowsObtained=Null pointer. E_INVALIDARG is returned.
	int Variation_1();
	// @cmember prghRows=NULL pointer. E_INVALIDARG is returned.
	int Variation_2();
};


// {{ TCW_TEST_CASE_MAP(Boundary_GetRowsAtRatio_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Boundary_GetRowsAtRatio_Cmd : public Boundary_GetRowsAtRatio {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetRowsAtRatio_Cmd,Boundary_GetRowsAtRatio);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetRowsAtRatio_Cmd)
#define THE_CLASS Boundary_GetRowsAtRatio_Cmd
BEG_TEST_CASE(Boundary_GetRowsAtRatio_Cmd, Boundary_GetRowsAtRatio, L"In context of commands")
	TEST_VARIATION(1,		L"pcRowsObtained=Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"prghRows=NULL pointer. E_INVALIDARG is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Boundary_GetRowsAtRatio_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Boundary_GetRowsAtRatio_OpenRowset : public Boundary_GetRowsAtRatio {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetRowsAtRatio_OpenRowset,Boundary_GetRowsAtRatio);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetRowsAtRatio_OpenRowset)
#define THE_CLASS Boundary_GetRowsAtRatio_OpenRowset
BEG_TEST_CASE(Boundary_GetRowsAtRatio_OpenRowset, Boundary_GetRowsAtRatio, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"pcRowsObtained=Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"prghRows=NULL pointer. E_INVALIDARG is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Boundary_GetRowsAtRatio_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class Boundary_GetRowsAtRatio_SchemaR : public Boundary_GetRowsAtRatio {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetRowsAtRatio_SchemaR,Boundary_GetRowsAtRatio);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Boundary_GetRowsAtRatio_SchemaR)
#define THE_CLASS Boundary_GetRowsAtRatio_SchemaR
BEG_TEST_CASE(Boundary_GetRowsAtRatio_SchemaR, Boundary_GetRowsAtRatio, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"pcRowsObtained=Null pointer. E_INVALIDARG is returned.")
	TEST_VARIATION(2,		L"prghRows=NULL pointer. E_INVALIDARG is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Valid and invalid parameters passed into methods
//
class Parameters : public TCIRowsetScroll {
private:
	
public:
	~Parameters(void) {};								
    Parameters ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember GetApproximatePosition: cbBookmark=0, pBookmark=null pointer, S_OK is returned.
	int Variation_1();
	// @cmember GetApproximatePosition: cbBookmark=0, pulPosition=null pointer, S_OK is returned.
	int Variation_2();
	// @cmember GetRowsAtRatio: cRows=0, *prghRows not NULL on input. *prghRows not null on output. S_OK is returned.
	int Variation_3();
	// @cmember GetRowsAtRatio: cRows=0, *prghRows=NULL on input, *prghRows null on output. S_OK is returned.
	int Variation_4();
	// @cmember GetRowsAtRatio: ulNumerator=0, ulDenominator=4, cRows=5.  S_OK and first 5 rows returned.
	int Variation_5();
	// @cmember GetRowsAtRatio: ulNumerator=5, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.
	int Variation_6();
	// @cmember GetRowsAtRatio: ulNumerator=0, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.
	int Variation_7();
	// @cmember GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.  DB_E_BADRATIO is returned.
	int Variation_8();
	// @cmember GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.
	int Variation_9();
	// @cmember GetRowsAtRatio: ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.
	int Variation_10();
	// @cmember GetRowsAtRatio: ulNumerator=1, ulDenominator=1, cRows=1.  DB_S_ENDOFROWSET
	int Variation_11();
	// @cmember GetRowsAtRatio: ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1, cRows=1.  middle row.
	int Variation_12();
};

// {{ TCW_TEST_CASE_MAP(Parameters_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Parameters_Cmd : public Parameters {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Parameters_Cmd,Parameters);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Parameters_Cmd)
#define THE_CLASS Parameters_Cmd
BEG_TEST_CASE(Parameters_Cmd, Parameters, L"In context of commands")
	TEST_VARIATION(1,		L"GetApproximatePosition: cbBookmark=0, pBookmark=null pointer, S_OK is returned.")
	TEST_VARIATION(2,		L"GetApproximatePosition: cbBookmark=0, pulPosition=null pointer, S_OK is returned.")
	TEST_VARIATION(3,		L"GetRowsAtRatio: cRows=0, *prghRows not NULL on input. *prghRows not null on output. S_OK is returned.")
	TEST_VARIATION(4,		L"GetRowsAtRatio: cRows=0, *prghRows=NULL on input, *prghRows null on output. S_OK is returned.")
	TEST_VARIATION(5,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=4, cRows=5.  S_OK and first 5 rows returned.")
	TEST_VARIATION(6,		L"GetRowsAtRatio: ulNumerator=5, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(7,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(8,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(9,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(10,	L"GetRowsAtRatio: ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(11,	L"GetRowsAtRatio: ulNumerator=1, ulDenominator=1, cRows=1.  DB_S_ENDOFROWSET.")
	TEST_VARIATION(12,	L"GetRowsAtRatio: ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1, cRows=1.  middle row.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Parameters_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Parameters_OpenRowset : public Parameters {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Parameters_OpenRowset,Parameters);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Parameters_OpenRowset)
#define THE_CLASS Parameters_OpenRowset
BEG_TEST_CASE(Parameters_OpenRowset, Parameters, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"GetApproximatePosition: cbBookmark=0, pBookmark=null pointer, S_OK is returned.")
	TEST_VARIATION(2,		L"GetApproximatePosition: cbBookmark=0, pulPosition=null pointer, S_OK is returned.")
	TEST_VARIATION(3,		L"GetRowsAtRatio: cRows=0, *prghRows not NULL on input. *prghRows not null on output. S_OK is returned.")
	TEST_VARIATION(4,		L"GetRowsAtRatio: cRows=0, *prghRows=NULL on input, *prghRows null on output. S_OK is returned.")
	TEST_VARIATION(5,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=4, cRows=5.  S_OK and first 5 rows returned.")
	TEST_VARIATION(6,		L"GetRowsAtRatio: ulNumerator=5, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(7,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(8,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(9,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(10,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(11,	L"GetRowsAtRatio: ulNumerator=1, ulDenominator=1, cRows=1.  DB_S_ENDOFROWSET.")
	TEST_VARIATION(12,	L"GetRowsAtRatio: ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1, cRows=1.  middle row.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Parameters_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class Parameters_SchemaR : public Parameters {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Parameters_SchemaR,Parameters);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Parameters_SchemaR)
#define THE_CLASS Parameters_SchemaR
BEG_TEST_CASE(Parameters_SchemaR, Parameters, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"GetApproximatePosition: cbBookmark=0, pBookmark=null pointer, S_OK is returned.")
	TEST_VARIATION(2,		L"GetApproximatePosition: cbBookmark=0, pulPosition=null pointer, S_OK is returned.")
	TEST_VARIATION(3,		L"GetRowsAtRatio: cRows=0, *prghRows not NULL on input. *prghRows not null on output. S_OK is returned.")
	TEST_VARIATION(4,		L"GetRowsAtRatio: cRows=0, *prghRows=NULL on input, *prghRows null on output. S_OK is returned.")
	TEST_VARIATION(5,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=4, cRows=5.  S_OK and first 5 rows returned.")
	TEST_VARIATION(6,		L"GetRowsAtRatio: ulNumerator=5, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(7,		L"GetRowsAtRatio: ulNumerator=0, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(8,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.  DB_E_BADRATIO is returned.")
	TEST_VARIATION(9,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(10,		L"GetRowsAtRatio: ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.")
	TEST_VARIATION(11,	L"GetRowsAtRatio: ulNumerator=1, ulDenominator=1, cRows=1.  DB_S_ENDOFROWSET.")
	TEST_VARIATION(12,	L"GetRowsAtRatio: ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1, cRows=1.  middle row.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Specific sequences of API calls where order is interesting.
//
class Sequence : public TCIRowsetScroll {
private:
	
public:
	~Sequence(void) {};								
    Sequence ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember Open rowset, fetch all of the rows, *pBookmark=DBBMK_FIRST.  S_OK is returned.
	int Variation_1();
	// @cmember ulNumerator=4, ulDenominator=5, and cRows=-3
	int Variation_2();
};


// {{ TCW_TEST_CASE_MAP(Sequence_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Sequence_Cmd : public Sequence {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Sequence_Cmd,Sequence);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Sequence_Cmd)
#define THE_CLASS Sequence_Cmd
BEG_TEST_CASE(Sequence_Cmd, Sequence, L"In context of commands")
	TEST_VARIATION(1,		L"Open rowset, fetch all of the rows, *pBookmark=DBBMK_FIRST.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=4, ulDenominator=5, and cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Sequence_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Sequence_OpenRowset : public Sequence {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Sequence_OpenRowset,Sequence);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Sequence_OpenRowset)
#define THE_CLASS Sequence_OpenRowset
BEG_TEST_CASE(Sequence_OpenRowset, Sequence, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"Open rowset, fetch all of the rows, *pBookmark=DBBMK_FIRST.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=4, ulDenominator=5, and cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Sequence_SchemaR)
//--------------------------------------------------------------------
// @class In context of Schema Rowset
//
class Sequence_SchemaR : public Sequence {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Sequence_SchemaR,Sequence);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Sequence_SchemaR)
#define THE_CLASS Sequence_SchemaR
BEG_TEST_CASE(Sequence_SchemaR, Sequence, L"In context of Schema Rowset")
	TEST_VARIATION(1,		L"Open rowset, fetch all of the rows, *pBookmark=DBBMK_FIRST.  S_OK is returned.")
	TEST_VARIATION(2,		L"ulNumerator=4, ulDenominator=5, and cRows=-3")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Make sure multiple methods can hold the same rows at the same time.
//
class Consistency : public TCIRowsetScroll {
private:
	
public:
	~Consistency(void) {};								
    Consistency ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.
	int Variation_1();
	// @cmember Verify IRowsetScroll inherits from IRowsetLocate
	int Variation_2();
};


// {{ TCW_TEST_CASE_MAP(Consistency_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Consistency_Cmd : public Consistency {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Consistency_Cmd,Consistency);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Consistency_Cmd)
#define THE_CLASS Consistency_Cmd
BEG_TEST_CASE(Consistency_Cmd, Consistency, L"In context of commands")
	TEST_VARIATION(1,		L"Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.")
	TEST_VARIATION(2,		L"Verify IRowsetScroll inherits from IRowsetLocate")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Consistency_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Consistency_OpenRowset : public Consistency {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Consistency_OpenRowset,Consistency);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Consistency_OpenRowset)
#define THE_CLASS Consistency_OpenRowset
BEG_TEST_CASE(Consistency_OpenRowset, Consistency, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.")
	TEST_VARIATION(2,		L"Verify IRowsetScroll inherits from IRowsetLocate")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Consistency_SchemaR)
//--------------------------------------------------------------------
// @class In context of a schema rowset
//
class Consistency_SchemaR : public Consistency {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Consistency_SchemaR,Consistency);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Consistency_SchemaR)
#define THE_CLASS Consistency_SchemaR
BEG_TEST_CASE(Consistency_SchemaR, Consistency, L"In context of a schema rowset")
	TEST_VARIATION(1,		L"Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.")
	TEST_VARIATION(2,		L"Verify IRowsetScroll inherits from IRowsetLocate")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test related interface IRowset.
//
class Related_IRowset : public TCIRowsetScroll {
private:
	
public:
	~Related_IRowset(void) {};								
    Related_IRowset ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember GetRowsAtRatio does not change the cursor location of the rowset. DB_E_ROWSNOTRELEASED is returned.
	int Variation_1();
	// @cmember Retrieve rows with GetNextRows and get same rows with GetRowsAtRatio. DB_E_ROWSNOTRELEASED is returned.
	int Variation_2();
};


// {{ TCW_TEST_CASE_MAP(Related_IRowset_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class Related_IRowset_Cmd : public Related_IRowset {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowset_Cmd,Related_IRowset);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Related_IRowset_Cmd)
#define THE_CLASS Related_IRowset_Cmd
BEG_TEST_CASE(Related_IRowset_Cmd, Related_IRowset, L"In context of commands")
	TEST_VARIATION(1,		L"GetRowsAtRatio does not change the cursor location of the rowset. DB_E_ROWSNOTRELEASED is returned.")
	TEST_VARIATION(2,		L"Retrieve rows with GetNextRows and get same rows with GetRowsAtRatio. DB_E_ROWSNOTRELEASED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Related_IRowset_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class Related_IRowset_OpenRowset : public Related_IRowset {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowset_OpenRowset,Related_IRowset);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Related_IRowset_OpenRowset)
#define THE_CLASS Related_IRowset_OpenRowset
BEG_TEST_CASE(Related_IRowset_OpenRowset, Related_IRowset, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"GetRowsAtRatio does not change the cursor location of the rowset. DB_E_ROWSNOTRELEASED is returned.")
	TEST_VARIATION(2,		L"Retrieve rows with GetNextRows and get same rows with GetRowsAtRatio. DB_E_ROWSNOTRELEASED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Related_IRowset_SchemaR)
//--------------------------------------------------------------------
// @class In context of a schema Rowset
//
class Related_IRowset_SchemaR : public Related_IRowset {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowset_SchemaR,Related_IRowset);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(Related_IRowset_SchemaR)
#define THE_CLASS Related_IRowset_SchemaR
BEG_TEST_CASE(Related_IRowset_SchemaR, Related_IRowset, L"In context of a schema Rowset")
	TEST_VARIATION(1,		L"GetRowsAtRatio does not change the cursor location of the rowset. DB_E_ROWSNOTRELEASED is returned.")
	TEST_VARIATION(2,		L"Retrieve rows with GetNextRows and get same rows with GetRowsAtRatio. DB_E_ROWSNOTRELEASED is returned.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test GetRowsAtRatio via a Keyset Driven cursor.
//
class KeysetCursor : public TCIRowsetScroll {
private:
	
public:
	~KeysetCursor(void) {};								
    KeysetCursor ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember ulNumerator=1, ulDenominator=20, cRows=-1
	int Variation_1();
	// @cmember ulNumerator=2, ulDenominator=20, cRows=-3
	int Variation_2();
	// @cmember ulNumerator=3, ulDenominator=20, cRows=(-3
	int Variation_3();
	// @cmember Get one row handle at a time.
	int Variation_4();
};


// {{ TCW_TEST_CASE_MAP(KeysetCursor_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class KeysetCursor_Cmd : public KeysetCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(KeysetCursor_Cmd,KeysetCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(KeysetCursor_Cmd)
#define THE_CLASS KeysetCursor_Cmd
BEG_TEST_CASE(KeysetCursor_Cmd, KeysetCursor, L"In context of commands")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(KeysetCursor_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class KeysetCursor_OpenRowset : public KeysetCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(KeysetCursor_OpenRowset,KeysetCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(KeysetCursor_OpenRowset)
#define THE_CLASS KeysetCursor_OpenRowset
BEG_TEST_CASE(KeysetCursor_OpenRowset, KeysetCursor, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(KeysetCursor_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class KeysetCursor_SchemaR : public KeysetCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(KeysetCursor_SchemaR,KeysetCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(KeysetCursor_SchemaR)
#define THE_CLASS KeysetCursor_SchemaR
BEG_TEST_CASE(KeysetCursor_SchemaR, KeysetCursor, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


//--------------------------------------------------------------------
// @class Test GetRowsAtRatio via a Dynamic cursor.
//
class DynamicCursor : public TCIRowsetScroll {
private:
	
public:
	~DynamicCursor(void) {};								
    DynamicCursor ( wchar_t* pwszTestCaseName) : TCIRowsetScroll(pwszTestCaseName) { };			
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// @cmember ulNumerator=1, ulDenominator=20, cRows=(-1
	int Variation_1();
	// @cmember ulNumerator=2, ulDenominator=20, cRows=(-3
	int Variation_2();
	// @cmember ulNumerator=3, ulDenominator=20, cRows=(-3
	int Variation_3();
	// @cmember Get one row handle at a time.
	int Variation_4();
};


// {{ TCW_TEST_CASE_MAP(DynamicCursor_Cmd)
//--------------------------------------------------------------------
// @class In context of commands
//
class DynamicCursor_Cmd : public DynamicCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DynamicCursor_Cmd,DynamicCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(DynamicCursor_Cmd)
#define THE_CLASS DynamicCursor_Cmd
BEG_TEST_CASE(DynamicCursor_Cmd, DynamicCursor, L"In context of commands")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=(-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(DynamicCursor_OpenRowset)
//--------------------------------------------------------------------
// @class In context of IOpenRowset
//
class DynamicCursor_OpenRowset : public DynamicCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DynamicCursor_OpenRowset,DynamicCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(DynamicCursor_OpenRowset)
#define THE_CLASS DynamicCursor_OpenRowset
BEG_TEST_CASE(DynamicCursor_OpenRowset, DynamicCursor, L"In context of IOpenRowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=(-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(DynamicCursor_SchemaR)
//--------------------------------------------------------------------
// @class In context of a Schema Rowset
//
class DynamicCursor_SchemaR : public DynamicCursor {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DynamicCursor_SchemaR,DynamicCursor);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
};
// {{ TCW_TESTCASE(DynamicCursor_SchemaR)
#define THE_CLASS DynamicCursor_SchemaR
BEG_TEST_CASE(DynamicCursor_SchemaR, DynamicCursor, L"In context of a Schema Rowset")
	TEST_VARIATION(1,		L"ulNumerator=1, ulDenominator=20, cRows=(-1")
	TEST_VARIATION(2,		L"ulNumerator=2, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(3,		L"ulNumerator=3, ulDenominator=20, cRows=(-3")
	TEST_VARIATION(4,		L"Get one row handle at a time.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Zombie)
//--------------------------------------------------------------------
// @class Test zombie states
//
class Zombie : public CTransaction {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPSET		m_DBPropSet;
	ULONG				m_cProperty;
	DBPROP			m_rgProperty[2];
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Zombie,CTransaction);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with fRetaining=TRUE.
	int Variation_1();
	// @cmember Commit with fRetaining=FALSE.
	int Variation_2();
	// @cmember Abort with fRetaining=TRUE.
	int Variation_3();
	// @cmember Abort with fRetaining=FALSE
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(Zombie)
#define THE_CLASS Zombie
BEG_TEST_CASE(Zombie, CTransaction, L"Test zombie states")
	TEST_VARIATION(1,		L"Commit with fRetaining=TRUE.")
	TEST_VARIATION(2,		L"Commit with fRetaining=FALSE.")
	TEST_VARIATION(3,		L"Abort with fRetaining=TRUE.")
	TEST_VARIATION(4,		L"Abort with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(48, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Bookmarks_cmd)
	TEST_CASE(2, Bookmarks_SchemaR)
	TEST_CASE(3, Bookmarks_OpenRowset)
	TEST_CASE(4, CanHoldRows_cmd)
	TEST_CASE(5, CanHoldRows_SchemaR)
	TEST_CASE(6, CanHoldRows_OpenRowset)
	TEST_CASE(7, CanFetchBackwards_Cmd)
	TEST_CASE(8, CanFetchBackwards_SchemaR)
	TEST_CASE(9, CanFetchBackwards_OpenRowset)
	TEST_CASE(10, CanFetchBackwards_CanHoldRows_Cmd)
	TEST_CASE(11, CanFetchBackwards_CanHoldRows_SchemaR)
	TEST_CASE(12, CanFetchBackwards_CanHoldRows_OpenRowset)
	TEST_CASE(13, MaxOpenRows_Cmd)
	TEST_CASE(14, MaxOpenRows_SchemaR)
	TEST_CASE(15, MaxOpenRows_OpenRowset)
	TEST_CASE(16, DeleteRows_cmd)
	TEST_CASE(17, DeleteRows_OpenRowset)
	TEST_CASE(18, RemoveDeleted_Cmd)
	TEST_CASE(19, RemoveDeleted_OpenRowset)
	TEST_CASE(20, Rowset_SingleRow_Cmd)
	TEST_CASE(21, Rowset_SingleRow_OpenRowset)
	TEST_CASE(22, Rowset_Empty_Cmd)
	TEST_CASE(23, Rowset_Empty_OpenRowset)
	TEST_CASE(24, Boundary_GetApproximatePosition_Cmd)
	TEST_CASE(25, Boundary_GetApproximatePosition_OpenRowset)
	TEST_CASE(26, Boundary_GetApproximatePosition_SchemaR)
	TEST_CASE(27, Boundary_GetRowsAtRatio_Cmd)
	TEST_CASE(28, Boundary_GetRowsAtRatio_OpenRowset)
	TEST_CASE(29, Boundary_GetRowsAtRatio_SchemaR)
	TEST_CASE(30, Parameters_Cmd)
	TEST_CASE(31, Parameters_OpenRowset)
	TEST_CASE(32, Parameters_SchemaR)
	TEST_CASE(33, Sequence_Cmd)
	TEST_CASE(34, Sequence_OpenRowset)
	TEST_CASE(35, Sequence_SchemaR)
	TEST_CASE(36, Consistency_Cmd)
	TEST_CASE(37, Consistency_OpenRowset)
	TEST_CASE(38, Consistency_SchemaR)
	TEST_CASE(39, Related_IRowset_Cmd)
	TEST_CASE(40, Related_IRowset_OpenRowset)
	TEST_CASE(41, Related_IRowset_SchemaR)
	TEST_CASE(42, KeysetCursor_Cmd)
	TEST_CASE(43, KeysetCursor_OpenRowset)
	TEST_CASE(44, KeysetCursor_SchemaR)
	TEST_CASE(45, DynamicCursor_Cmd)
	TEST_CASE(46, DynamicCursor_OpenRowset)
	TEST_CASE(47, DynamicCursor_SchemaR)
	TEST_CASE(48, Zombie)
END_TEST_MODULE()
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Bookmarks - test DBPROP_BOOKMARKS
//|	Created:			08/28/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	// IRowsetScroll is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetScroll))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST. S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmarks::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		return TEST_FAIL;

	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 1), TRUE))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST. S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmarks::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		return TEST_FAIL;
	
	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, m_ulRowCount), TRUE))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc  *pBookmark=DBBMK_INVALID. DB_E_BADBOOKMARK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmarks::Variation_3()
{
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	pulPosition=3;
	DBCOUNTITEM	pcRows=3;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),DB_E_BADBOOKMARK))
	{
		return TEST_FAIL;
	}

	// pulPosition and pcRows should be untouched.
	if(!COMPARE(3, pulPosition))  
		return TEST_FAIL;
	
	if(!COMPARE(3, pcRows))
		return TEST_FAIL;

	return TEST_PASS;
}

//*-----------------------------------------------------------------------
// @mfunc Get one row handle at a time (cRows==1).
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmarks::Variation_4()
{
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	DBCOUNTITEM	cRowCount;
	ULONG_PTR	cbBookmark;
	BYTE *		pBookmark=NULL;
	BOOL		fTestPass=FALSE;

	// Get one row handle at a time, from the first row
	for(cRowCount=1; cRowCount<=m_ulRowCount; cRowCount++)
	{
		// Get the bookmark for the first row
		if(!GetBookmark(cRowCount, &cbBookmark, &pBookmark))
			goto END;
		
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows),S_OK))
			goto END;

		if(!VerifyApproximatePosition(pulPosition, pcRows, cRowCount))
			goto END;

		// Release the bookmark
		PROVIDER_FREE(pBookmark);
		pBookmark=NULL;
	}

	fTestPass=TRUE;

END:
	// Release the bookmark
	if(pBookmark)
		PROVIDER_FREE(pBookmark);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks::Terminate()
{	
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Bookmarks_cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Bookmarks_cmd - test DBPROP_BOOKMARKS
//|	Created:			08/28/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;
	
	return (Bookmarks::Init());
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_cmd::Terminate()
{

	return(Bookmarks::Terminate());
}	
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Bookmarks_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Bookmarks_SchemaR - Using Schema Rowsets
//|	Created:			08/28/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;
	
	return (Bookmarks::Init());
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_SchemaR::Terminate()
{
	return(Bookmarks::Terminate());
}	
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Bookmarks_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Bookmarks_OpenRowset - In Context of IOpenRowset
//|	Created:			08/28/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	return (Bookmarks::Init());
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmarks_OpenRowset::Terminate()
{	
	return(Bookmarks::Terminate());
}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		CanHoldRows - Test DBPROP_CANHOLDROWS property
//|	Created:			08/29/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Init()
{
	DBPROPID	guidProperty=DBPROP_CANHOLDROWS;

	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTGROUPBY, IID_IRowsetScroll,
		1,&guidProperty))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc Get first two rows, get next three rows, pass bookmark of second row. S_OK returned.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_1()
{
	ULONG_PTR	cbBookmark=0;
	BYTE		*pBookmark=NULL;
	HROW		*pHRow=NULL;
	HROW		*pHRow2=NULL;
	DBCOUNTITEM	cCount=0;
	DBCOUNTITEM	cCount2=0;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	BOOL		fTestPass=FALSE;

	// Fetch the first two rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 0,			// Count of rows to skip before reading.
									 2,			// The number of rows to fetch.
									 &cCount,	// Actual number of fetched rows.
									 &pHRow),	// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(2, cCount))
		goto END;

	// Get a bookmark for the second row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto END;

	// Fetch the next three rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 0,			// Count of rows to skip before reading.
									 3,			// The number of rows to fetch.
									 &cCount2,	// Actual number of fetched rows.
									 &pHRow2),	// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(3, cCount2))
		goto END;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;

	if(!VerifyApproximatePosition(pulPosition, pcRows, 2))
		goto END;

	fTestPass = TEST_PASS;

END:
	// Release the bookmarks
	PROVIDER_FREE(pBookmark);

	// Release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cCount, pHRow, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRow2)
	{
		CHECK(m_pIRowset->ReleaseRows(cCount2, pHRow2, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow2);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc Get first two rows, *pBookmark=second row bookmark. S_OK returned.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_2()
{
	ULONG_PTR	cbBookmark=0;
	BYTE		*pBookmark=NULL;
	HROW		*pHRow=NULL;
	HROW		*pHRow2=NULL;
	DBCOUNTITEM	cCount=0;
	DBCOUNTITEM	cCount2=0;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	BOOL		fTestPass=FALSE;

	//Start from the begining
	if(!CHECK(m_pIRowset->RestartPosition(NULL), S_OK))
		goto END;

	// Get a bookmark for the second row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto END;

	// Fetch the first two rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 0,			// Count of rows to skip before reading.
									 2,			// The number of rows to fetch.
									 &cCount,	// Actual number of fetched rows.
									 &pHRow),	// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(2, cCount))
		goto END;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;
	
	if(!VerifyApproximatePosition(pulPosition, pcRows, 2))
		goto END;

	// Fetch the next row to verify cursor position
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 0,			// Count of rows to skip before reading.
									 1,			// The number of rows to fetch.
									 &cCount2,	// Actual number of fetched rows.
									 &pHRow2),	// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	
	if(!COMPARE(VerifyRowPosition(*pHRow2, 3, g_pCTable, EXACT),TRUE))
		goto END;

	fTestPass=TRUE;

END:
	// Release the bookmarks
	PROVIDER_FREE(pBookmark);

	// Release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRow2)
	{
		CHECK(m_pIRowset->ReleaseRows(1, pHRow2, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow2);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc Get first 5 rows, get last 5 rows, ulNumerator=1, ulDenominator=2, S_OK and 10th row returned.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_3()
{
	DBCOUNTITEM	cCount=0;
	HROW		*prghRows=NULL;
	DBCOUNTITEM		cCount2=0;
	HROW		*prghRows2=NULL;
	HROW		*prghRows3=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;

	//Start from the begining
	if(!CHECK(m_pIRowset->RestartPosition(NULL), S_OK))
		goto END;

	// Fetch first few rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,	// Chapter handle.
									 0,					// Count of rows to skip before reading.
									 2,					// The number of rows to fetch.
									 &cCount,			// Actual number of fetched rows.
									 &prghRows),		// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(2, cCount))
		goto END;

	// Fetch last five rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,		// Chapter handle.
									 m_ulRowCount-4,		// Count of rows to skip before reading.
									 2,						// The number of rows to fetch.
									 &cCount2,				// Actual number of fetched rows.
									 &prghRows2),			// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(2, cCount))
		goto END;

	// ulNumerator=1, ulDenominator=2, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,2,
		1,&cRowsObtained,&prghRows3),S_OK))
		goto END;

	// One row should have been returned
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	if(!VerifyRowPosition(*prghRows3, GetRatio(m_ulRowCount,1,2)+1, g_pCTable, INEXACT))
		goto END;

	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(2,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(prghRows2)
	{
		CHECK(m_pIRowset->ReleaseRows(2,prghRows2,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows2);
	}

	if(prghRows3)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows3,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows3);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(CanHoldRows_cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		CanHoldRows_cmd - In context of commands
//|	Created:			08/29/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanHoldRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_cmd::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanHoldRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanHoldRows_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		CanHoldRows_SchemaR - In context of Schema rowsets
//|	Created:			08/29/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_SchemaR::Init()
{
	BOOL	fPass;
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	fPass = CanHoldRows::Init();
	if(TEST_PASS == fPass)	
	{
		if	( m_ulRowCount < 5 )
			return TEST_SKIPPED;
		else
			return TEST_PASS;
	}
	else
		return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanHoldRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanHoldRows_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		CanHoldRows_OpenRowset - In context of IOpenRowset
//|	Created:			08/29/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return CanHoldRows::Init();
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanHoldRows::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards - Test DBPROP_CANFETCHBACKWARDS
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards::Init()
{
	DBPROPID	guidProperty=DBPROP_CANFETCHBACKWARDS;
	
	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and an accessor.  
	// DBPROP_CANFETCHBACKWARDS is requested.
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetScroll,
		1,&guidProperty))
		return TEST_SKIPPED;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc  ulNumerator=0, ulDenominator=last row, cRows=(-2). DB_S_ENDOFROWSET.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards::Variation_1()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=0, ulDenominator=last row, cRows=(-2).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,m_ulRowCount,
		-2,&cRowsObtained,&prghRows),DB_S_ENDOFROWSET))
		goto END;
	
	if(!COMPARE(cRowsObtained, 0))
		goto END;

	if( !COMPARE(prghRows,NULL) )
		goto END;
		
	fTestPass=TRUE;

END:
	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
	PROVIDER_FREE(prghRows);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc  ulNumerator=1, ulDenominator=4, cRows=(-6). DB_S_ENDOFROWSET and five row handles returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*rghRows;
	BOOL		fTestPass=FALSE;
	DBCOUNTITEM	cRowCount;
	HRESULT	hr;

	rghRows = (HROW *)PROVIDER_ALLOC(sizeof(HROW)*(m_ulRowCount/4+2));

	 // ulNumerator=1, ulDenominator=4, cRows=(-6).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,4,
		-(LONG(m_ulRowCount/4)+2),&cRowsObtained,&rghRows);

	if (!(COMPARE(SUCCEEDED(hr), TRUE)) )
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		if (!COMPARE(cRowsObtained<(m_ulRowCount/4)+2, TRUE))
			goto END;
	}
	else 
	{
		odtLog << L"Warning, ideally. DB_S_ENDOFROWSET\n";
		if(!COMPARE(cRowsObtained, (m_ulRowCount/4)+2))
			fTestPass = FALSE;
		else
			fTestPass = TRUE;
		goto END;
	}

	// Make sure the row handles are returned in the traversal order
	for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
	{
		if(!VerifyRowPosition(rghRows[cRowCount],cRowsObtained-cRowCount,g_pCTable,INEXACT))
			goto END;
	}

	
	fTestPass=TRUE;

END:
	// Release the row handles
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);

	PROVIDER_FREE(rghRows);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//*-----------------------------------------------------------------------
// @mfunc ulNumerator=3, ulDenominator=5, cRows=(-2). S_OK returned and 12th and 11th row handles returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards::Variation_3()
{
	DBCOUNTITEM	cRowsObtained=0,iCount;
	HROW		rghRows[5];
	HROW		*prghRows=rghRows;
	BOOL		fTestPass=FALSE;
	HRESULT		hr = E_FAIL;

	// ulNumerator=3, ulDenominator=5, cRows=(-2).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,5,
		-2,&cRowsObtained,&prghRows);

	if (!(COMPARE(SUCCEEDED(hr), TRUE)) )
		goto END;

	if ( hr == S_OK )
	{
		if(!COMPARE(cRowsObtained, 2))
			goto END;
	}
	else
	{
		if(!COMPARE(cRowsObtained<2, TRUE))
			goto END;
	}
	
	for ( iCount = 0; iCount < cRowsObtained; iCount++ )
	{
		VerifyRowPosition(prghRows[iCount],GetRatio(m_ulRowCount,3,5) + 1 - iCount,g_pCTable,INEXACT);	
	}

	fTestPass=TRUE;

END:
	// Release the row handles
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc ulNumerator=lastrow-1,ulDenominator=last row, cRows=2, expect DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards::Variation_4()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr = S_OK;

	 // ulNumerator=lastrow-1, ulDenominator=last row, cRows=(2).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,m_ulRowCount-1,m_ulRowCount,
		2,&cRowsObtained,&prghRows);

	if (!(COMPARE(SUCCEEDED(hr), TRUE)) )
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		if (!COMPARE(cRowsObtained, 1))
			goto END;
	}
	else 
	{
		odtLog << L"Warning, ideally. DB_S_ENDOFROWSET and 1 row should have been returned.\n";
		fTestPass = TRUE;
		goto END;
	}

	if( !COMPARE(VerifyRowPosition(*prghRows, m_ulRowCount, g_pCTable, INEXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
	PROVIDER_FREE(prghRows);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_Cmd - In context of commands
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanFetchBackwards::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_Cmd::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_SchemaR - In context of a Schema Rowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanFetchBackwards::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_OpenRowset - In context of IOpenRowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanFetchBackwards::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_CanHoldRows - Test DBPROP_CANFETCHBACKWARDS + DBPROP_CANHOLDROWS
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows::Init()
{
	DBPROPID	guidProperty[2];
	ULONG		cPrptSet=0;
	
	if(!TCIRowsetScroll::Init())
		return FALSE;

	guidProperty[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	guidProperty[cPrptSet++]=DBPROP_CANHOLDROWS;
	
	// Create a rowset and an accessor.  
	// DBPROP_CANFETCHBACKWARDS and DBPROP_CANHOLDROWS are requested.
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
		cPrptSet,guidProperty))
		return TEST_SKIPPED;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=last row, cRows=(-1). S_OK and one row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards_CanHoldRows::Variation_1()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=1, ulDenominator=last row, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,m_ulRowCount,
		-1,&cRowsObtained,&prghRows),S_OK))
		goto END;
	
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	if( !COMPARE(VerifyRowPosition(*prghRows, 2, g_pCTable, INEXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
	PROVIDER_FREE(prghRows);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=3, ulDenominator=last row, cRows=(-3). Do not release rows, try another combination. S_OK returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanFetchBackwards_CanHoldRows::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	DBCOUNTITEM	cRowsObtained2=0;
	HROW		rghRows[5];
	HROW		*prghRows=rghRows;
	HROW		*prghRows2=NULL;
	BOOL		fTestPass=FALSE;
	DBCOUNTITEM	cRowCount;
	HRESULT		hr = E_FAIL;

	// ulNumerator=2, ulDenominator=last row, cRows=(-3).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		-3,&cRowsObtained,&prghRows);
	
	if ( !COMPARE(hr==S_OK || hr==DB_S_ENDOFROWSET, TRUE) )
		goto END;

	// The 3rd, 2nd, and 1st row handles should be retrieved
	if ( hr == S_OK )
	{
		if( !COMPARE(cRowsObtained, 3))
			goto END;
	}
	else
	{
		if ( !COMPARE(cRowsObtained<3, TRUE) )
			goto END;
	}

	// Make sure the row handles are returned in the traversal order
	for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
	{
		if( !COMPARE(VerifyRowPosition(prghRows[cRowCount],cRowsObtained-cRowCount+1,g_pCTable, INEXACT), TRUE) )
			goto END;
	}

	// ulNumerator=1, ulDenominator=4, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,4,
		-1,&cRowsObtained2,&prghRows2),S_OK))
		goto END;

	if(!COMPARE(cRowsObtained2, 1))
		goto END;

	if( !COMPARE(VerifyRowPosition(*prghRows2, m_ulRowCount/4+1, g_pCTable, INEXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	if(prghRows2)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained2,prghRows2,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows2);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_CanHoldRows_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_CanHoldRows_Cmd - In context of Commands
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanFetchBackwards_CanHoldRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards_CanHoldRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_CanHoldRows_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_CanHoldRows_SchemaR - In context of a Schema Rowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_SchemaR::Init()
{
	BOOL	fPass;
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	fPass = CanFetchBackwards_CanHoldRows::Init();
	if(TEST_PASS == fPass)
	{
		if	( m_ulRowCount < 5 )
			return TEST_SKIPPED;
		else
			return TEST_PASS;
	}
	else
		return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards_CanHoldRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanFetchBackwards_CanHoldRows_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		CanFetchBackwards_CanHoldRows_OpenRowset - In context of IOpenRowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	return (CanFetchBackwards_CanHoldRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanFetchBackwards_CanHoldRows_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CanFetchBackwards_CanHoldRows::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		MaxOpenRows - Test DBPROP_MAXOPENROWS
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	//Create a rowset and an accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST,
			IID_IRowsetScroll))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc MAXOPENROWS=3, ulNumerator=9, ulDenominator=10, and cRows=4. DB_S_ENDOFROWSET is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_1()
{
	HROW		*prghRows=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	ULONG		cMaxOpenRowsCount=0;

	//remember g_cMaxOpenRowsCount
	ASSERT(g_cMaxOpenRowsCount <= UINT_MAX );
	cMaxOpenRowsCount=(ULONG)g_cMaxOpenRowsCount;

	if(g_rgDBPrpt[IDX_MaxOpenRows].fSettable)
	{
		//Re-open a rowset with DBPROP_MAXOPENROWSET set to 3
		ReleaseRowsetAndAccessor();
		g_cMaxOpenRowsCount = 3;	

		if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST,
			IID_IRowsetScroll,1,&guidProperty))
			goto END;
	}

	//If DBPROP_MAXOPENROWS=0 then there is no limit on the number of rows that can be active.
	if(g_cMaxOpenRowsCount == 0)
	{
  		//ulNumerator=9, ulDenominator=10
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,9,10,
				m_ulRowCount+1,&cRowsObtained,&prghRows), DB_S_ENDOFROWSET))
			goto END;

		if( !COMPARE(cRowsObtained < m_ulRowCount, TRUE) )
			goto END;

		if( !COMPARE(VerifyRowPosition(*prghRows, GetRatio(m_ulRowCount,9,10)+1, g_pCTable, INEXACT), TRUE) )
			goto END;
	}
	else if(g_cMaxOpenRowsCount < LONG(m_ulRowCount))
	{
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,10,
				m_ulRowCount+1,&cRowsObtained,&prghRows), DB_S_ROWLIMITEXCEEDED))
			goto END;
		//only g_cMaxOpenRowsCount rows could be fetched
		if(!COMPARE(cRowsObtained, (ULONG)g_cMaxOpenRowsCount))
			goto END;
		if( !COMPARE(VerifyRowPosition(*prghRows, 1, g_pCTable, EXACT), TRUE) )
			goto END;
	}
	else
	{
		//ulNumerator=9, ulDenominator=10
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,9,10,
				m_ulRowCount+1,&cRowsObtained,&prghRows), DB_S_ENDOFROWSET))
			goto END;

		if( !COMPARE(cRowsObtained < m_ulRowCount, TRUE) )
			goto END;

		if( !COMPARE(VerifyRowPosition(*prghRows, (9*m_ulRowCount)/10+1, g_pCTable, INEXACT), TRUE) )
			goto END;
	}

	fTestPass=TRUE;

END:
	//Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	//Restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;
		
	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc MAXOPENROWS=2, ulNumerator=1, ulDenominator=3, and cRows=3. DB_S_ROWLIMITEXCEEDED is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_2()
{
	HROW		*prghRows=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	DBCOUNTITEM	cMaxOpenRowsCount=0;

	//remember g_cMaxOpenRowsCount
	cMaxOpenRowsCount=g_cMaxOpenRowsCount;

	//Re-open a rowset with DBPROP_MAXOPENROWS set
	if(g_rgDBPrpt[IDX_MaxOpenRows].fSettable)
	{
		ReleaseRowsetAndAccessor();
		g_cMaxOpenRowsCount = 2;	

		//Open rowset and accessor
		if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
			1,&guidProperty))
			goto END;
	}

	if(g_cMaxOpenRowsCount==0)
	{
		//ulNumerator=1, ulDenominator=3, cRows=cRowset/3.
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,3,
			m_ulRowCount/3,&cRowsObtained,&prghRows), S_OK))
			goto END;

		if(!COMPARE(cRowsObtained, (ULONG)(m_ulRowCount/3)))
			goto END;	
	}
	//ulNumerator=1, ulDenominator=3, cRows=3.
	else if( g_cMaxOpenRowsCount < 3 )
	{
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,3,
			3,&cRowsObtained,&prghRows), DB_S_ROWLIMITEXCEEDED))
			goto END;

		//Only g_cMaxOpenRowsCount rows can be fetched.
		if(!COMPARE(cRowsObtained, (ULONG)g_cMaxOpenRowsCount))
			goto END;
	}
	else
	{
		//ulNumerator=1, ulDenominator=3, cRows=3.
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,3,
				3,&cRowsObtained,&prghRows), DB_S_ENDOFROWSET))

		if(!COMPARE(cRowsObtained, 3))
			goto END;
	}

	if( !COMPARE(VerifyRowPosition(*prghRows, GetRatio(m_ulRowCount,1,3)+1, g_pCTable, INEXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	//Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	//Restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc Use default MaxOpenRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_3()
{
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	HROW		*prghRows=NULL;
	DBCOUNTITEM	cRows;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;

	//Re-open a rowset with DBPROP_MAXOPENROWS set to default value.
	if(g_rgDBPrpt[IDX_MaxOpenRows].fSettable)
	{
		ReleaseRowsetAndAccessor();

		//Open rowset and accessor
		if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
			1,&guidProperty))
			goto END;
	}

	//If MaxOpenRows is default to 0, retrieve # of rows in the rowset,
	// else retrieve the MaxOpenRows in the rowset
	if( !g_cMaxOpenRowsCount || g_cMaxOpenRowsCount > LONG(m_ulRowCount) )
		cRows=m_ulRowCount;
	else
		cRows=g_cMaxOpenRowsCount;

	//ulNumerator=1, ulDenominator=cRows, cRows=cRows.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,cRows,
		cRows,&cRowsObtained,&prghRows), S_OK))
		goto END;

	//All the row handles should be retrieved
	if(!COMPARE(cRowsObtained, cRows))
		goto END;

	if( !COMPARE(VerifyRowPosition(prghRows[cRowsObtained-1],cRows,g_pCTable, EXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	//Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows::Terminate()
{
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(MaxOpenRows_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		MaxOpenRows_Cmd - In context of commands
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	//Make sure DBPROP_MAXOPENROWS is supported
	if(!g_rgDBPrpt[IDX_MaxOpenRows].fSupported)
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (MaxOpenRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(MaxOpenRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MaxOpenRows_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		MaxOpenRows_SchemaR - In context of a Schema Rowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	//Make sure DBPROP_MAXOPENROWS is supported
	if(!g_rgDBPrpt[IDX_MaxOpenRows].fSupported)
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (MaxOpenRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(MaxOpenRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MaxOpenRows_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		MaxOpenRows_OpenRowset - In context of IOpenRowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	//Make sure DBPROP_MAXOPENROWS is supported
	if(!g_rgDBPrpt[IDX_MaxOpenRows].fSupported)
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (MaxOpenRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(MaxOpenRows::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		DeleteRows - Delete rows from the rowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=second row bookmark, delete second row.  DB_E_DELETEDROW is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_1()
{
	DBPROPID		guidProperty[3];
	ULONG			cProperties;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;
	DBCOUNTITEM		pulPosition=0;
	DBCOUNTITEM		pcRows=0;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;
	guidProperty[2]=DBPROP_OTHERUPDATEDELETE;

/* NOTE:  
DBPROP_OTHERINSERT = FALSE, DBPROP_OTHERUPDATEDELETE = FALSE, means static cursor.
Since IRowsetChange is being asked for that implies that the cursor will no longer
be static.  Therefore a call to GetProperties will need to be made to see if the
above mentioned properties were set.  
*/
	// Set DBPROP_OTHERUPDATEDELETE if supported
	if(g_rgDBPrpt[IDX_OtherUpdateDelete].fSupported)
		cProperties=3;
	else
		cProperties=2;

	// Open rowset, and accessor.  Request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		cProperties, guidProperty));

	// Get the bookmark for the 2nd row	
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto END;

	// Get the row handle for the 2nd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRowsObtained,&pHRow),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto END;

	// Delete the 2nd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to a deleted row.  S_OK (DB_E_BADBOOKMARK is RemoveDeleted property is set.
	if ( IsPropertyActive(DBPROP_BOOKMARKSKIPPED) )
	{
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows), S_OK))
			goto END;
	
		if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 3), TRUE) )
			goto END;
	}
	else if ( IsPropertyActive(DBPROP_REMOVEDELETED) )
	{
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows), DB_E_BADBOOKMARK))
			goto END;
	}
	else
	{
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows), S_OK))
			goto END;
		
		if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 2), TRUE) )
			goto END;
	}

	fTestPass = TEST_PASS;

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();


	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST, delete last row.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_2()
{
	DBPROPID		guidProperty[2];
	ULONG			cProperties=2;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;
	DBCOUNTITEM		pulPosition=0;
	DBCOUNTITEM		pcRows=0;
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;

	// Open rowset, and accessor.  Request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		cProperties, guidProperty));

	// Get the row handle for the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,m_ulRowCount-1,1,&cRowsObtained,&pHRow),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto END;

	// Delete the last row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows), S_OK))
		goto END;

	if ( IsPropertyActive(DBPROP_REMOVEDELETED) )
	{
		m_ulRowCount--;
		if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, m_ulRowCount-1), TRUE) )
			goto END;
	}
	else
	{
		if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, m_ulRowCount), TRUE) )
			goto END;
	}

	fTestPass = TEST_PASS;

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_3()
{
	DBPROPID		guidProperty[2];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		2,guidProperty));

	// Get the row handle for the 4th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the 4th row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!COMPARE(cRowsObtained, 1))
		goto END;

	fTestPass = TEST_PASS;

	if ( S_OK == m_pIRowsetIdentity->IsSameRow(prghRows[0], m_pHRows[3]) )
	{
		//Get Data for the row
		if(!CHECK(m_pIRowset->GetData(prghRows[0],m_hAccessor,m_pData),DB_E_DELETEDROW))
			fTestPass = TEST_FAIL;
	}


END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}


	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Delete 10th row. ulNumerator=1, ulDenominator=2, and cRows=1. S_OK and 10th row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_4()
{
	DBPROPID		guidProperty[2];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		2,guidProperty));

	// Get a row handle 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(m_ulRowCount/2),1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	// ulNumerator=1, ulDenominator=2, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,2,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// One row handle should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	fTestPass = TEST_PASS;

	if ( S_OK == m_pIRowsetIdentity->IsSameRow(prghRows[0], m_pHRows[m_ulRowCount/2]) )
	{
		//Get Data for the row
		if ( !CHECK(m_pIRowset->GetData(*prghRows,m_hAccessor,m_pData),DB_E_DELETEDROW) )
			fTestPass=TEST_FAIL;
	}
	
END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Delete 20th row. ulNumerator=20, ulDenominator=20, and cRows=1. S_OK and 20th row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_5()
{
	DBPROPID		guidProperty[2];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;
	HRESULT			hr=S_OK;
	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		2,guidProperty));

	// Get the row handle for the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(m_ulRowCount-1),1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the last row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	// ulNumerator=last, ulDenominator=last, cRows=-1. (this fetch is exact).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,m_ulRowCount,m_ulRowCount,
		-1,&cRowsObtained,&prghRows), hr))
		goto END;

	if(hr==S_OK)
	{
		// One row handle should be retrieved
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		if (IsPropertyActive(DBPROP_REMOVEDELETED))
		{
			if( !COMPARE(VerifyRowPosition(prghRows[0], m_ulRowCount-1, g_pCTable, EXACT), TRUE))
				goto END;			
		}
		else
		{
			//Get Data for the row 
			if(!CHECK(m_pIRowset->GetData(*prghRows,m_hAccessor,m_pData),DB_E_DELETEDROW))
				goto END;
		}
	}
	else
	{
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}
	fTestPass=TRUE;

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Terminate()
{
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(DeleteRows_cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		DeleteRows_cmd - In context of commands
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_cmd::Init()
{	

	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	if (!AlteringRowsIsOK())
	{
		odtLog<<L"Skipped, Pre-exisiting table was set; changing would put table in unknown state.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (DeleteRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DeleteRows::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DeleteRows_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		DeleteRows_OpenRowset - In context of IOpenRowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_OpenRowset::Init()
{	

	if (!SetRowsetType(USEOPENROWSET) )
		return TEST_SKIPPED;

	if (!AlteringRowsIsOK())
	{
		odtLog<<L"Skipped, Pre-exisiting table was set; changing would put table in unknown state.\n";
		return TEST_SKIPPED;
	}


	// {{ TCW_INIT_BASECLASS_CHECK
	return (DeleteRows::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DeleteRows::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		RemoveDeleted - test DBPROP_REMOVEDELETED
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc  *pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_1()
{
	DBPROPID		rgguidProperty[4];
	ULONG			cProperties;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	DBCOUNTITEM		pulPosition=0;
	DBCOUNTITEM		pcRows=0;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;
	rgguidProperty[3]=DBPROP_OTHERUPDATEDELETE;

/* NOTE:  
DBPROP_OTHERINSERT = FALSE, DBPROP_OTHERUPDATEDELETE = FALSE, means static cursor.
Since IRowsetChange is being asked for that implies that the cursor will no longer
be static.  Therefore a call to GetProperties will need to be made to see if the
above mentioned properties were set.  
*/
	// Set DBPROP_OTHERUPDATELETE if supported
	if(g_rgDBPrpt[IDX_OtherUpdateDelete].fSupported)
		cProperties=4;
	else
		cProperties=3;

	// Open rowset, and accessor.  Request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		cProperties,rgguidProperty));

	// Get the bookmark for the 3rd row
	if(!GetBookmark(3, &cbBookmark, &pBookmark))
		goto END;

	// Get the row handle for the 3rd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained,&pHRow),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to a deleted row.
	if ( IsPropertyActive(DBPROP_BOOKMARKSKIPPED) )
	{
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows), S_OK))
			goto END;
	
		if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 3), TRUE) )
			goto END;
	}
	else
	{
		if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
			&pulPosition,&pcRows), DB_E_BADBOOKMARK))
			goto END;
	}

	fTestPass=TRUE;

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	// Release the bookmark
	PROVIDER_FREE(pBookmark);

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Delete the 3rd row.  *pBookmark points to the second row.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_2()
{
	DBPROPID		rgguidProperty[3];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	DBCOUNTITEM		pulPosition=0;
	DBCOUNTITEM		pcRows=0;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, Request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		3,rgguidProperty));

	// Get the bookmark for the 2nd row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto END;

	// Get the row handle for the 3rd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained,&pHRow),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto END;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// Should get second row.
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		&pulPosition,&pcRows), S_OK))
		goto END;

	if( !COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 2), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	// Release the bookmark
	PROVIDER_FREE(pBookmark);

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Ratio points to deleted row. DB_E_DELETEDROW and no row handles returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_3()
{
	DBPROPID		rgguidProperty[3];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		3,rgguidProperty));

	// Get the row handle for the 1st row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	COMPARE(cRowsObtained, 1);

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the 1st row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;
	m_ulRowCount--;

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	// ulNumerator=0, ulDenominator=20, cRows=1. The second row should be retrieved
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,m_ulRowCount,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// No row handles should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	if( !COMPARE(VerifyRowPosition(*prghRows, 2, g_pCTable, EXACT), TRUE))
		goto END;

	fTestPass = TRUE;
	

END:
	// Release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Delete 18th row. ulNumerator=9, ulDenominator=10, and cRows=3. DB_S_ENDOFROWSET is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_4()
{
	DBPROPID		rgguidProperty[3];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;
	HRESULT			hr;
	IRowsetChange	*pIRowsetChange=NULL;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		3,rgguidProperty));

	// Get the row handle for the 9/10 row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,GetRatio(m_ulRowCount, 9, 10)-1,1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	if(!COMPARE(cRowsObtained, 1))
		goto END;

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;

	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	// ulNumerator=9, ulDenominator=10, cRows=3.
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,9,10,
		((m_ulRowCount)/10)+1,&cRowsObtained,&prghRows);

	if (!COMPARE(SUCCEEDED(hr), TRUE))
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		if (!COMPARE(cRowsObtained<(m_ulRowCount/10)+1, TRUE))
			goto END;
	}
	else
	{
		odtLog << L"Warning, ideally. DB_S_ENDOFROWSET\n";
		if(!COMPARE(cRowsObtained, (m_ulRowCount/10)+1))
			goto END;
	}

	// The first row handle should be 19th row
	if( !COMPARE(VerifyRowPosition(*prghRows, GetRatio(m_ulRowCount, 9, 10)+1, g_pCTable, INEXACT), TRUE))
		goto END;
	
	fTestPass=TRUE;

END:
	// Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Delete 19th row. ulNumerator=19, ulDenominator=20, and cRows=1. S_OK and 20th row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_5()
{
	DBPROPID		rgguidProperty[3];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;
	HRESULT			hr;
	IRowsetChange	*pIRowsetChange=NULL;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;


	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		3,rgguidProperty));

	// Get the row handle for the next to last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(m_ulRowCount-2),1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	if(!COMPARE(cRowsObtained, 1))
		goto END;

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,prghRows,NULL),S_OK))
		goto END;

	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	// Use (m_ulRowCount-2)/(m_ulRowCount-1) as the ratio
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,m_ulRowCount-2,m_ulRowCount-1,
		2,&cRowsObtained,&prghRows);		

	if (!COMPARE(SUCCEEDED(hr), TRUE))
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		if (!COMPARE(cRowsObtained, 1))
			goto END;
	}
	else
	{
		odtLog << L"Warning, ideally. DB_S_ENDOFROWSET\n";
		if(!COMPARE(cRowsObtained, 2))
			goto END;
	}

	if( !COMPARE(VerifyRowPosition(prghRows[0], m_ulRowCount, g_pCTable, INEXACT), TRUE) )
		goto END;
	
	fTestPass=TRUE;

END:
	// Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}

//*-----------------------------------------------------------------------
// @mfunc Delete half the row. Request 3/4. Verify 3/4 row is returned
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_6()
{
	DBPROPID		rgguidProperty[3];
	HROW			*prghRows=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;
	HRESULT			hr;
	IRowsetChange	*pIRowsetChange=NULL;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	// Open rowset and accessor, request IRowsetChange and IRowsetScroll
	TESTC_PROVIDER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetScroll,
		3,rgguidProperty));

	// Get the row handle for the last half of the rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(m_ulRowCount/2)-1,m_ulRowCount,&cRowsObtained,&prghRows),DB_S_ENDOFROWSET))
		goto END;

	if(!COMPARE(cRowsObtained >=m_ulRowCount/2, TRUE))
		goto END;

	// QI for IRowsetChange pointer
	if(FAILED(m_pIRowsetScroll->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto END;

	// Delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,cRowsObtained,prghRows,NULL),S_OK))
		goto END;

	SAFE_RELEASE(pIRowsetChange);

	// Release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(prghRows);
	prghRows=NULL;

	// Use 3/4 ratio
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,4,
		2,&cRowsObtained,&prghRows);		

	if (!COMPARE(SUCCEEDED(hr), TRUE))
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		odtLog << L"Warning, ideally, S_OK should be returned\n";
		if (!COMPARE(cRowsObtained <= 1, TRUE))
			goto END;
	}
	else
	{
		if(!COMPARE(cRowsObtained, 2))
			goto END;

		if( !COMPARE(VerifyRowPosition(prghRows[0], GetRatio(m_ulRowCount/2, 3, 4)+1 , g_pCTable, INEXACT), TRUE) )
			goto END;

		if( !COMPARE(VerifyRowPosition(prghRows[1], GetRatio(m_ulRowCount/2, 3, 4)+2, g_pCTable, INEXACT), TRUE) )
			goto END;
	}

	fTestPass=TRUE;

END:
	// Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Terminate()
{
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(RemoveDeleted_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		RemoveDeleted_Cmd - In context of Commands
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	if ( !AlteringRowsIsOK() )
	{
		odtLog<<L"Skipped, Pre-exisiting table was set; changing would put table in unknown state.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (RemoveDeleted::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(RemoveDeleted::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(RemoveDeleted_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		RemoveDeleted_OpenRowset - In context of IOpenRowset
//|	Created:			09/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	if ( !AlteringRowsIsOK() )
	{
		odtLog<<L"Skipped, Pre-exisiting table was set; changing would put table in unknown state.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (RemoveDeleted::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(RemoveDeleted::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_SingleRow - Test single row rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow::Init()
{
	DBPROPID	guidProperty=DBPROP_CANHOLDROWS;

	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetScroll, 1, &guidProperty))
		return FALSE;

	if ( m_ulRowCount != 1)
	{
		odtLog<<L"Couldn't obtain a 1 row table.\n";
		return FALSE;
	}

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=first row bookmark.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_1()
{
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	BOOL		fTestPass=FALSE;

	// Get the bookmark for the first row
	if(!GetBookmark(1,&cbBookmark,&pBookmark))
		goto END;
		
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;
	
	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 1), TRUE))
		goto END;

	fTestPass = TEST_PASS;

END:
	// Release the bookmark
	PROVIDER_FREE(pBookmark);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=1, cRows=-1. S_OK returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	DBCOUNTITEM	cRowsObtained2=0;
	HROW		*prghRows=NULL;
	HROW		*prghRows2=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	 // ulNumerator=1, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,1,
		-1,&cRowsObtained,&prghRows),hr))
		goto END;
	
	if(hr==S_OK)
	{
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		if(!VerifyRowPosition(prghRows[0], 1, g_p1RowTable, EXACT))
			goto END;

		// Reset the cursor
		if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
			goto END;
		
		// Fetch the row and verify reference count
		if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained2,&prghRows2),S_OK))
			goto END;

		// Release the row handle and check ref count
		if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
			goto END;
	}
	else
	{
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}

	fTestPass=TRUE;


END:
	// Release the row handle
	PROVIDER_FREE(prghRows);
	
	if(prghRows2)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained2, prghRows2, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows2);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=0, ulDenominator=1, cRows=1. S_OK returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_3()
{
	DBCOUNTITEM	cRowsObtained=0;
	DBCOUNTITEM	cRowsObtained2=0;
	HROW		*prghRows=NULL;
	HROW		*prghRows2=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	 // ulNumerator=1, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,1,
		1,&cRowsObtained,&prghRows),hr))
		goto END;
	
	if(hr==S_OK)
	{
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		if(!VerifyRowPosition(prghRows[0], 1, g_p1RowTable, EXACT))
			goto END;

		// Reset the cursor
		if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
			goto END;
		
		// Fetch the row and verify reference count
		if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained2,&prghRows2),S_OK))
			goto END;

		// Release the row handle and check ref count
		if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
			goto END;
	}
	else
	{
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}

	fTestPass=TRUE;


END:
	// Release the row handle
	PROVIDER_FREE(prghRows);
	
	if(prghRows2)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained2, prghRows2, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows2);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_4()
{
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	DBBOOKMARK	dbBookmark = DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&dbBookmark;
	BOOL		fTestPass=FALSE;
		
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;
	
	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 1), TRUE))
		goto END;

	fTestPass = TEST_PASS;

END:
	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_5()
{
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	DBBOOKMARK	dbBookmark = DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&dbBookmark;
	BOOL		fTestPass=FALSE;
		
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;
	
	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 1), TRUE))
		goto END;

	fTestPass = TEST_PASS;

END:
	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=3, cRows=1. S_OK returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_6()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	 // ulNumerator=1, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,3,
		1,&cRowsObtained,&prghRows),hr))
		goto END;
	
	if(hr==S_OK)
	{
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		if(!VerifyRowPosition(prghRows[0], 1, g_p1RowTable, EXACT))
			goto END;
	}
	else
	{
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}

	fTestPass=TRUE;

END:
	// Release the row handle
	PROVIDER_FREE(prghRows);

	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Rowset_SingleRow_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_SingleRow_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	if (!g_p1RowTable)
	{
		odtLog<<L"Couldn't obtain a 1 row table.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Rowset_SingleRow::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Rowset_SingleRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Rowset_SingleRow_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_SingleRow_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	if (!g_p1RowTable)
	{
		odtLog<<L"Couldn't obtain a 1 row table.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Rowset_SingleRow::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Rowset_SingleRow::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_Empty - Test empty rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

///--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty::Init()
{
	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pEmptyTable, SELECT_ALLFROMTBL, IID_IRowsetScroll))
		return FALSE;

	if ( m_ulRowCount != 0)
	{
		odtLog<<L"Couldn't obtain an empty row table.\n";
		return FALSE;
	}

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_Empty::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		return TEST_FAIL;

	if((pcRows != 0) || (pulPosition != 0))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET and no row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_Empty::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=1, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,1,
		1,&cRowsObtained,&prghRows),DB_S_ENDOFROWSET))
		goto END;

	if(!COMPARE(prghRows,NULL))
		goto END;
	
	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=1, cRows=0. S_OK and no row handle returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_Empty::Variation_3()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=1, ulDenominator=1, cRows=0.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,1,
		0,&cRowsObtained,&prghRows),S_OK))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc ulNumerator=0, ulDenominator=1, cRows=1. DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_Empty::Variation_4()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=1, ulDenominator=1, cRows=0.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,1,
		1,&cRowsObtained,&prghRows),DB_S_ENDOFROWSET))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_Empty::Variation_5()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		return TEST_FAIL;

	if((pcRows != 0) || (pulPosition != 0))
		return TEST_FAIL;

	return TEST_PASS;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty::Terminate()
{	
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}	


// {{ TCW_TC_PROTOTYPE(Rowset_Empty_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_Empty_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	if (!g_pEmptyTable)
	{
		odtLog<<L"Couldn't obtain an empty table.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Rowset_Empty::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Rowset_Empty::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Rowset_Empty_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Rowset_Empty_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	if (!g_pEmptyTable)
	{
		odtLog<<L"Couldn't obtain an empty table.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Rowset_Empty::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_Empty_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Rowset_Empty::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetApproximatePosition - Boundary conditions for GetApproximatePosition
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition::Init()
{
	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
		0,NULL))
		return FALSE;

	// Get a valid bookmark value, points to the first row
	m_cbBookmark=1;
	m_DBBookmark=DBBMK_FIRST;
	m_pBookmark=(BYTE *)&m_DBBookmark;
	m_pulPosition=0;
	m_pcRows=0;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc pBookmark==Null pointer. E_INVALIDARG is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_1()
{
	if(CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,m_cbBookmark,NULL,
		&m_pulPosition,&m_pcRows),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc pBookmark==NULL. DB_E_BADBOOKMARK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_2()
{
	if(CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,NULL,
		&m_pulPosition,&m_pcRows),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc pBookmark points to a invalid bookmark. DB_E_BADBOOKMARK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_3()
{
	DBBOOKMARK dbBookmark = DBBMK_INVALID;
	BYTE *pBookmark = (BYTE *)&dbBookmark;

	if(CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&m_pulPosition,&m_pcRows),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc pcRows==Null pointer. S_OK and no count of rows returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_4()
{
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,m_cbBookmark,m_pBookmark,
		&m_pulPosition,NULL),S_OK))
		return TEST_FAIL;

	if(!COMPARE(VerifyApproximatePosition(m_pulPosition, m_ulRowCount, 1), TRUE))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc pulPosition is NULL and pcRows is NULL, S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_5()
{
	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,m_cbBookmark,m_pBookmark,
		NULL,NULL),S_OK))
		return TEST_FAIL;
	
	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc cbBookmark = 0, pBookmark = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetApproximatePosition::Variation_6()
{
	DBCOUNTITEM	ulPosition = MAXDBCOUNTITEM;
	DBCOUNTITEM	cRows = 0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,0,NULL,
		&ulPosition,&cRows),S_OK))
		return TEST_FAIL;
	
	COMPARE(ulPosition, MAXDBCOUNTITEM);

	if(!COMPARE(VerifyApproximatePosition(0, cRows, 0), TRUE))
		return TEST_FAIL;
	
	return TEST_PASS;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}	


// {{ TCW_TC_PROTOTYPE(Boundary_GetApproximatePosition_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetApproximatePosition_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetApproximatePosition::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetApproximatePosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetApproximatePosition_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetApproximatePosition_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetApproximatePosition::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetApproximatePosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetApproximatePosition_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetApproximatePosition_SchemaR - In context of a Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetApproximatePosition::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetApproximatePosition_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetApproximatePosition::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetRowsAtRatio - Boundary conditions for GetRowsAtRatio
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

///--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio::Init()
{
	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
		0,NULL))
		return FALSE;

	m_prghRows=&m_hRows;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc pcRowsObtained=Null pointer. E_INVALIDARG is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAtRatio::Variation_1()
{
	if(CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,1,
		1,NULL,&m_prghRows),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc prghRows=NULL pointer. E_INVALIDARG is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAtRatio::Variation_2()
{
	if(CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,1,
		1,&m_cRowsObtained,NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}

// {{ TCW_TC_PROTOTYPE(Boundary_GetRowsAtRatio_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetRowsAtRatio_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetRowsAtRatio::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetRowsAtRatio::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetRowsAtRatio_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetRowsAtRatio_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetRowsAtRatio::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetRowsAtRatio::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetRowsAtRatio_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary_GetRowsAtRatio_SchemaR - In context of a Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Boundary_GetRowsAtRatio::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAtRatio_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Boundary_GetRowsAtRatio::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Parameters - Valid and invalid parameters passed into methods
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters::Init()
{
	IColumnsInfo	*pIColumnsInfo=NULL;
	DBCOUNTITEM		cColumns;
	DBCOLUMNINFO	*prgInfo=NULL;
	WCHAR			*pStringsBuffer=NULL;
	BOOL			fInitPass=FALSE;

	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetScroll))
		return FALSE;

	// Make sure IColumnsInfo returns correct infomation on the 0th column,
	// which is the bookmark column
	if(!CHECK(m_pIRowsetScroll->QueryInterface(IID_IColumnsInfo, 
		(void **)&pIColumnsInfo),S_OK))
		return FALSE;

	if(!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &prgInfo,&pStringsBuffer),S_OK))
		goto END;

	// The first element in the array should be the bookmark column
	if(!COMPARE(prgInfo->iOrdinal, 0))
		goto END;

	if(!COMPARE(((prgInfo->dwFlags) & DBCOLUMNFLAGS_ISBOOKMARK),(DBCOLUMNFLAGS_ISBOOKMARK)))
		goto END;

	fInitPass=TRUE;

END:
	// Release the interface pointer
	SAFE_RELEASE(pIColumnsInfo);

	// Free mem
	PROVIDER_FREE(prgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return fInitPass;	   	
}


//*-----------------------------------------------------------------------
// @mfunc GetApproximatePosition: cbBookmark=0, pBookmark=null pointer, S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_1()
{
	ULONG_PTR	cbBookmark=0;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		&pulPosition,&pcRows),S_OK))
		return TEST_FAIL;

	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 0), TRUE))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc GetApproximatePosition: cbBookmark=0, pulPosition=null pointer, S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_2()
{
	ULONG_PTR	cbBookmark=0;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	*pulPosition=NULL;
	DBCOUNTITEM	pcRows=0;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,cbBookmark,pBookmark,
		pulPosition,&pcRows),S_OK))
		return TEST_FAIL;

	if(!COMPARE(VerifyApproximatePosition(0, pcRows, 0), TRUE))
		return TEST_FAIL;

	return TEST_PASS;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: cRows=0, *prghRows not NULL on input. *prghRows not null on output. S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_3()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		rghRows[5];
	HROW		*prghRows=rghRows;
	BOOL		fTestPass=TRUE;

	// ulNumerator=1, ulDenominator=2, cRows=0.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,2,
		0,&cRowsObtained,&prghRows),S_OK))
		goto END;

	// No rows should have been returned
	if(!COMPARE(cRowsObtained, 0))
		goto END;

	// prghRows should not be NULL
	if(!COMPARE(prghRows, rghRows))
		fTestPass=FALSE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		//PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: cRows=0, *prghRows=NULL on input, *prghRows null on output. S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_4()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=1, ulDenominator=3, cRows=0.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,3,
		0,&cRowsObtained,&prghRows),S_OK))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=0, ulDenominator=4, cRows=5.  S_OK and first 5 rows returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_5()
{
	HROW *		prghRows=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass=FALSE;

	// ulNumerator=0, ulDenominator=4, cRows=5.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,4,
		5,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// Five row handles should be retrieved.
	if(!COMPARE(cRowsObtained, 5))
		goto END;


	// Verify the row handle are retrieved correctly
	for(cRowCount=0; cRowCount<5; cRowCount++)
	{
		if(!VerifyRowPosition(prghRows[cRowCount],cRowCount+1,g_pCTable,EXACT))
			goto END;
	}

	fTestPass=TRUE;

END:
	// Release the row handles
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=5, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_6()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=5, ulDenominator=0, cRows=5.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,5,0,
		5,&cRowsObtained,&prghRows),DB_E_BADRATIO))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=0, ulDenominator=0, cRows=5.  DB_E_BADRATIO is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_7()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=0, ulDenominator=0, cRows=5.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,0,
		5,&cRowsObtained,&prghRows),DB_E_BADRATIO))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.  DB_E_BADRATIO is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_8()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	 // ulNumerator=ULONG_MAX, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,MAXDBCOUNTITEM,1,
		1,&cRowsObtained,&prghRows),DB_E_BADRATIO))
		goto END;
	
	if(!COMPARE(prghRows, NULL))
		goto END;

	if(COMPARE(cRowsObtained, 0))
		fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_9()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	// ulNumerator=ULONG_MAX, ulDenominator=ULONG_MAX, cRows=-1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,MAXDBCOUNTITEM,MAXDBCOUNTITEM,
		-1,&cRowsObtained,&prghRows),hr))
		goto END;

	if(hr==S_OK)
	{
		// The last row handle should be retrieved
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		// Verify the last row
		if(!VerifyRowPosition(prghRows[0],m_ulRowCount,g_pCTable,EXACT))
			goto END;
	}
	else
	{
		// No row should be retrieved
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}		
	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1.  S_OK and last row returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_10()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;

	// ulNumerator=ULONG_MAX-1, ulDenominator=ULONG_MAX, cRows=1
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,MAXDBCOUNTITEM-1,MAXDBCOUNTITEM,
		1,&cRowsObtained,&prghRows),hr))
		goto END;

	if(hr==S_OK)
	{
		// The last row handle should be retrieved
		if(!COMPARE(cRowsObtained, 1))
			goto END;

		// Verify the last row
		if( !COMPARE(VerifyRowPosition(prghRows[0],m_ulRowCount,g_pCTable,INEXACT), TRUE) )
			goto END;
	}
	else
	{
		// No row should be retrieved
		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}		
	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=1, ulDenominator=1 cRows=1.  DB_S_ENDOFROWSET.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_11()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=1, ulDenominator=1, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,1,
		1,&cRowsObtained,&prghRows),DB_S_ENDOFROWSET))
		goto END;

	// The last row handle should be retrieved
	if(!COMPARE(cRowsObtained, 0))
		goto END;

	if(!COMPARE(prghRows, NULL))
		goto END;

	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio: ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1 cRows=1.  middle row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_12()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=LONG_MAX/2, ulDenominator=LONG_MAX+1 cRows=1.  middle row.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,MAXDBROWCOUNT/2,DBCOUNTITEM(MAXDBROWCOUNT)+1,
		1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	// expect one row
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	// Verify the middle row
	if( !COMPARE(VerifyRowPosition(prghRows[0],m_ulRowCount/2,g_pCTable,INEXACT), TRUE) )
		goto END;

	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Parameters_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Parameters_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Parameters::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Parameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Parameters_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Parameters_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return(Parameters::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Parameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Parameters_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Parameters_SchemaR - In context of a Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_SchemaR::Init()
{
	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Parameters::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Parameters::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Sequence - Specific sequences of API calls where order is interesting.
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc Open rowset, fetch all of the rows, *pBookmark=DBBMK_FIRST.  S_OK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cCount=0;
	DBCOUNTITEM	pulPosition=0;
	DBCOUNTITEM	pcRows=0;
	BOOL		fTestPass=TEST_FAIL;

	// Fetch all the rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,				// Chapter handle.
									 0,					// Count of rows to skip before reading.
									 m_ulRowCount,		// The number of rows to fetch.
									 &cCount,			// Actual number of fetched rows.
									 &pHRow),			// Array of handles of retrieved rows.
									 S_OK))
		goto END;

	if(!COMPARE(ULONG(m_ulRowCount), cCount))
		goto END;

	if(!CHECK(m_pIRowsetScroll->GetApproximatePosition(NULL,1,pBookmark,
		&pulPosition,&pcRows),S_OK))
		goto END;

	if(!COMPARE(VerifyApproximatePosition(pulPosition, pcRows, 1), TRUE))
		return TEST_FAIL;

	fTestPass = TEST_PASS;

END:
	// Release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cCount, pHRow, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=4, ulDenominator=5, and cRows=(-3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr=S_OK;

	if(g_rgDBPrpt[IDX_CanFetchBackwards].fDefault==VARIANT_FALSE)
		hr=DB_E_CANTFETCHBACKWARDS;
	
	// ulNumerator=4, ulDenominator=5, cRows=(-3).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,4,5,
		-3,&cRowsObtained,&prghRows),hr))
		goto END;
	
	if(hr==S_OK)
	{
		if(!COMPARE(cRowsObtained,3))
			goto END;

		if(!VerifyRowPosition(prghRows[0],GetRatio(m_ulRowCount,4,5)+1, g_pCTable,INEXACT))
			goto END;	

		if(!VerifyRowPosition(prghRows[1],GetRatio(m_ulRowCount,4,5), g_pCTable,INEXACT))
			goto END;	

		if(!VerifyRowPosition(prghRows[2],GetRatio(m_ulRowCount,4,5)-1, g_pCTable,INEXACT))
			goto END;	
	}
	else
	{
		if(!COMPARE(prghRows, NULL))
			goto END;

		if(!COMPARE(cRowsObtained, 0))
			goto END;
	}

	fTestPass=TRUE;

END:
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Sequence_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Sequence_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Sequence::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Sequence::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Sequence_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Sequence_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Sequence::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Sequence::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Sequence_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Sequence_SchemaR - In context of Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_SchemaR::Init()
{
	BOOL	fPass;

	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	fPass = Sequence::Init();
	if(TEST_PASS == fPass)
	{
		if	( m_ulRowCount < 5 )
			return TEST_SKIPPED;
		else
			return TEST_PASS;
	}
	else
		return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Sequence::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Consistency - Make sure multiple methods can hold the same rows at the same time.
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency::Init()
{
	DBPROPID	guidPrpt[1];
	
	guidPrpt[0]=DBPROP_CANHOLDROWS;
	
	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and accessor, requesting DBPROP_CANHOLDROWS
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll, NUMELEM(guidPrpt),
		guidPrpt))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Consistency::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark1=(BYTE *)&DBBookmark;
	BYTE			*rgpBookmarks2[3];
	ULONG_PTR		rgcbBookmarks2[3];

	DBCOUNTITEM		cRowsObtained1=0;
	DBCOUNTITEM		cRowsObtained2=0;
	DBCOUNTITEM		cRowsObtained3=0;
	
	HROW			rghRows1[3]={NULL,NULL,NULL};
	HROW			*prghRows1=rghRows1;
	HROW			rghRows2[3]={NULL,NULL,NULL};
	HROW			*prghRows3=NULL;
	
	IRowsetLocate	*pIRowsetLocate=NULL;
	BOOL			fTestPass=FALSE;
	DBREFCOUNT		cRefCount;

	// QI for IRowsetLocate pointer
	if(!CHECK(m_pIRowsetScroll->QueryInterface(IID_IRowsetLocate,
		(void **)&pIRowsetLocate),S_OK))
		goto END;

	// Retrieve the 3rd, 4th, and 5th row handles by calling GetRowsAt
	if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark1,2,3,
		&cRowsObtained1,&prghRows1),S_OK))
		goto END;

	COMPARE(cRowsObtained1, 3);

	// Get the bookmark for the first row
	if(!GetBookmark(1,rgcbBookmarks2,rgpBookmarks2))
		goto END;

	// Get the bookmark for the 2nd row
	if(!GetBookmark(2,&rgcbBookmarks2[1],&rgpBookmarks2[1]))
		goto END;

	// Get the bookmark for the 3rd row
	if(!GetBookmark(3,&rgcbBookmarks2[2],&rgpBookmarks2[2]))
		goto END;

	// Retrieve the 1st, 2nd, and 3rd row handles by call GetRowsByBookmark.
	// The 3rd row handle is being retrieved for the second time.
	if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks2,
		(const BYTE **)rgpBookmarks2,rghRows2,NULL),S_OK))
		goto END;

	// Now retrieve the 3rd row handle again by calling GetRowsAtRatio.
	// ulNumerator=3, ulDenominator=last row, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		1,&cRowsObtained3,&prghRows3), S_OK))
		goto END;
	
	COMPARE(cRowsObtained3, 1);

	// Make sure the 3rd row handle is retrieved three times
	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows3,NULL,&cRefCount,NULL),S_OK))
		goto END;

	if(COMPARE(cRefCount >= 1, TRUE))
		fTestPass=TRUE;

END:
	
	// Free the Bookmark memory allocated
	PROVIDER_FREE(rgpBookmarks2[2]);
	PROVIDER_FREE(rgpBookmarks2[1]);
	PROVIDER_FREE(rgpBookmarks2[0]);

	// Release IRowsetLocate pointer 
	if(pIRowsetLocate)
		pIRowsetLocate->Release();

	// Release the row handle
	if(cRowsObtained1)
		CHECK(m_pIRowset->ReleaseRows(3,prghRows1,NULL,NULL,NULL),S_OK);

	if(cRowsObtained2)
		CHECK(m_pIRowset->ReleaseRows(3,rghRows2,NULL,NULL,NULL),S_OK);

	if(prghRows3)
	{
		if(fTestPass)
		{
			PROVIDER_FREE(prghRows3);
		}
		else
		{
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained3, prghRows3, NULL, NULL, NULL),S_OK);
			PROVIDER_FREE(prghRows3);
		}
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

//*-----------------------------------------------------------------------
// @mfunc Retrieve three row handles from GetRowsAt,GetRowsByBookmark, and GetRowsAtRatio.
//			Check IRowsetScroll inherits from IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Consistency::Variation_2()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark1=(BYTE *)&DBBookmark;
	BYTE			*rgpBookmarks2[3];
	ULONG_PTR		rgcbBookmarks2[3];

	DBCOUNTITEM		cRowsObtained1=0;
	DBCOUNTITEM		cRowsObtained2=0;
	DBCOUNTITEM		cRowsObtained3=0;
	
	HROW			rghRows1[3]={NULL,NULL,NULL};
	HROW			*prghRows1=rghRows1;
	HROW			rghRows2[3]={NULL,NULL,NULL};
	HROW			*prghRows3=NULL;
	
	BOOL			fTestPass=FALSE;
	DBREFCOUNT		cRefCount;

	// Retrieve the 3rd, 4th, and 5th row handles by calling GetRowsAt
	if(!CHECK(m_pIRowsetScroll->GetRowsAt(NULL,NULL,1,pBookmark1,2,3,
		&cRowsObtained1,&prghRows1),S_OK))
		goto END;

	COMPARE(cRowsObtained1, 3);

	// Get the bookmark for the first row
	if(!GetBookmark(1,rgcbBookmarks2,rgpBookmarks2))
		goto END;

	// Get the bookmark for the 2nd row
	if(!GetBookmark(2,&rgcbBookmarks2[1],&rgpBookmarks2[1]))
		goto END;

	// Get the bookmark for the 3rd row
	if(!GetBookmark(3,&rgcbBookmarks2[2],&rgpBookmarks2[2]))
		goto END;

	// Retrieve the 1st, 2nd, and 3rd row handles by call GetRowsByBookmark.
	// The 3rd row handle is being retrieved for the second time.
	if(!CHECK(m_pIRowsetScroll->GetRowsByBookmark(NULL,3,rgcbBookmarks2,
		(const BYTE **)rgpBookmarks2,rghRows2,NULL),S_OK))
		goto END;

	// Now retrieve the 3rd row handle again by calling GetRowsAtRatio.
	// ulNumerator=3, ulDenominator=last row, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		1,&cRowsObtained3,&prghRows3), S_OK))
		goto END;
	
	COMPARE(cRowsObtained3, 1);

	// Make sure the 3rd row handle is retrieved three times
	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows3,NULL,&cRefCount,NULL),S_OK))
		goto END;

	if(COMPARE(cRefCount >= 1, TRUE))
		fTestPass=TRUE;

END:
	
	// Free the Bookmark memory allocated
	PROVIDER_FREE(rgpBookmarks2[2]);
	PROVIDER_FREE(rgpBookmarks2[1]);
	PROVIDER_FREE(rgpBookmarks2[0]);

	// Release the row handle
	if(cRowsObtained1)
		CHECK(m_pIRowset->ReleaseRows(3,prghRows1,NULL,NULL,NULL),S_OK);

	if(cRowsObtained2)
		CHECK(m_pIRowset->ReleaseRows(3,rghRows2,NULL,NULL,NULL),S_OK);

	if(prghRows3)
	{
		if(fTestPass)
		{
			PROVIDER_FREE(prghRows3);
		}
		else
		{
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained3, prghRows3, NULL, NULL, NULL),S_OK);
			PROVIDER_FREE(prghRows3);
		}
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Consistency_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Consistency_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Consistency::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Consistency::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Consistency_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Consistency_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Consistency::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Consistency::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Consistency_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Consistency_SchemaR - In context of a schema rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_SchemaR::Init()
{
	BOOL	fPass;

	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	fPass = Consistency::Init();
	if(TEST_PASS == fPass)
	{
		if	( m_ulRowCount < 5 )
			return TEST_SKIPPED;
		else
			return TEST_PASS;
	}
	else
		return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Consistency::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		Related_IRowset - Test related interface IRowset.
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset::Init()
{
	if(!TCIRowsetScroll::Init())
		return FALSE;

	// Create a rowset and an accessor.
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetScroll))
		return FALSE;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc GetRowsAtRatio does not change the cursor location of the rowset. DB_E_ROWSNOTRELEASED is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_1()
{
	HROW			rghRows[1];
	HROW			*prghRows=rghRows;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;

	if (IsPropertyActive(DBPROP_CANHOLDROWS))
		return TEST_SKIPPED;  //skip

	// Move the cursor position to before the third row.
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,0,&cRowsObtained,&prghRows),S_OK))
		goto END;

	// No row should have been retrieved.
	if(!COMPARE(cRowsObtained, 0))
		goto END;

	// ulNumerator=0, ulDenominator=last row, cRows=1.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,m_ulRowCount,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!COMPARE(VerifyRowPosition(prghRows[0], 1, g_pCTable,EXACT), TRUE))
		goto END;

	// IRowset->GetNextRows returns DB_E_ROWSNOTRELEASED
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&prghRows),DB_E_ROWSNOTRELEASED))
		goto END;

	// No row should be retrieved
	if(!COMPARE(cRowsObtained,0))
		goto END;

	// Release the row handle retrieved by IRowsetScroll::GetRowsAtRatio
	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;

	// IRowset->GetNextRows to retrieve the third row, return S_OK
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	if(!COMPARE(cRowsObtained, 1))
		goto END;

	if(!COMPARE(VerifyRowPosition(prghRows[0], 3, g_pCTable, EXACT), TRUE) )
		goto END;
		
	fTestPass=TRUE;

END:
	// Free the row handle
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL),S_OK);

	return fTestPass;
}


//*-----------------------------------------------------------------------
// @mfunc Retrieve rows with GetNextRows and get same rows with GetRowsAtRatio. DB_E_ROWSNOTRELEASED is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		rghRows[1];
	HROW		*prghRows=rghRows;
	BOOL		fTestPass=FALSE;

	if (IsPropertyActive(DBPROP_CANHOLDROWS))
		return TEST_SKIPPED;  //skip

	// Get the first row handle.
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&prghRows),S_OK))
		goto END;

	// IRowsetScroll::GetRowsAtRatio returns DB_E_ROWSNOTRELEASED.
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,m_ulRowCount,
		1,&cRowsObtained,&prghRows), DB_E_ROWSNOTRELEASED))
		goto END;

	// No row should have been retrieved.
	if(!COMPARE(cRowsObtained,0))
		goto END;

	// Release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;

	 // IRowsetScroll::GetRowsAtRatio returns S_OK
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,0,m_ulRowCount,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// Verify the first row handle is retrieved
	if(!VerifyRowPosition(prghRows[0], 1, g_pCTable, EXACT))
		goto END;
		
	fTestPass=TRUE;

END:
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Related_IRowset_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		Related_IRowset_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Related_IRowset::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Related_IRowset::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_IRowset_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		Related_IRowset_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return (Related_IRowset::Init());
	// }}
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Related_IRowset::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_IRowset_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		Related_IRowset_SchemaR - In context of a schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_SchemaR::Init()
{
	BOOL	fPass;

	if (!SetRowsetType(USESCHEMAR))
		return TEST_SKIPPED;

	fPass = Related_IRowset::Init();
	
	if(TEST_PASS == fPass)
	{
		if	( m_ulRowCount < 5 )
			return TEST_SKIPPED;
		else
			return TEST_PASS;
	}

	return fPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset_SchemaR::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Related_IRowset::Terminate());
}	// }}
// }}
// }}


//*-----------------------------------------------------------------------
//|	Test Case:		KeysetCursor - Test GetRowsAtRatio via a Keyset Driven cursor.
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor::Init()
{
	DBPROPID	guidPropertySet[4];
	ULONG		cPrptSet=0;

	if(!TCIRowsetScroll::Init())
		return FALSE;

	guidPropertySet[cPrptSet]=DBPROP_CANFETCHBACKWARDS;
	cPrptSet++;
	
	guidPropertySet[cPrptSet]=DBPROP_CANSCROLLBACKWARDS;
	cPrptSet++;
	
	guidPropertySet[cPrptSet]=DBPROP_CANHOLDROWS;
	cPrptSet++;
	
	guidPropertySet[cPrptSet]=DBPROP_OTHERUPDATEDELETE;
	cPrptSet++;
	
	// Create a rowset and an accessor.  
	// DBPROP_CANFETCHBACKWARDS, DBPROP_CANSCROLLBACKWARDS and DBPROP_CANHOLDROWS
	// are requested 
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
		cPrptSet, guidPropertySet))
		return TEST_SKIPPED;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=20, cRows=(-1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_1()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=1, ulDenominator=20, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,m_ulRowCount,
		-1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// Only one row should have been retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	if(!VerifyRowPosition(*prghRows, 2, g_pCTable,INEXACT))
		goto END;
			
	fTestPass=TRUE;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=2, ulDenominator=20, cRows=(-3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_2()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT	hr;

	// ulNumerator=1, ulDenominator=20, cRows=(-3).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,m_ulRowCount,-3,&cRowsObtained,&prghRows);
	
	if (!COMPARE(SUCCEEDED(hr),TRUE))
		goto END;

	if ( hr == DB_S_ENDOFROWSET )
	{
		if(!COMPARE(cRowsObtained<3, TRUE))
			goto END;
	}
	else
	{
		odtLog << L"Warning, ideally. DB_S_ENDOFROWSET\n";
		if(!COMPARE(cRowsObtained, 3))
			goto END;
	}

	fTestPass=TEST_PASS;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=3, ulDenominator=20, cRows=(-3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_3()
{
	DBCOUNTITEM	cFirstRowsObtained;
	DBCOUNTITEM	cSecondRowsObtained;
	HROW		*pFirsthRow=NULL;
	HROW		*pSecondhRow=NULL;
	BOOL		fTestPass=FALSE;
	HRESULT		hr = E_FAIL;

	// ulNumerator=3, ulDenominator=last row, cRows=(-3).
	hr = m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		-3,&cFirstRowsObtained,&pFirsthRow);

	if ( !COMPARE(hr==S_OK || hr==DB_S_ENDOFROWSET, TRUE) )
		goto END;

	// The 3rd, 2nd, and 1st row handles should be retrieved
	if ( hr == S_OK )
	{
		if( !COMPARE(cFirstRowsObtained, 3))
			goto END;
	}
	else
	{
		if ( !COMPARE(cFirstRowsObtained<3, TRUE) )
			goto END;
	}

	if(cFirstRowsObtained > 0 && !VerifyRowPosition(*pFirsthRow, 4, g_pCTable, INEXACT))
		goto END;

	// ulNumerator=4, ulDenominator=last row, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,4,m_ulRowCount,
		-1,&cSecondRowsObtained,&pSecondhRow), S_OK))
		goto END;

	// Only one row should have been retrieved
	if(!COMPARE(cSecondRowsObtained, 1))
		goto END;

	if(!VerifyRowPosition(*pSecondhRow, 5, g_pCTable, INEXACT))
		goto END;

	fTestPass=TRUE;

END:
	// Release the row handles
	if(pFirsthRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cFirstRowsObtained,pFirsthRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pFirsthRow);
	}

	// Release the row handles
	if(pSecondhRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cSecondRowsObtained,pSecondhRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pSecondhRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc Get one row handle at a time.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int KeysetCursor::Variation_4()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		rghRows[1];
	HROW		*prghRows=rghRows;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass = TEST_FAIL;

	// Get one row handle at a time from the first row
	for(cRowCount=0; cRowCount<ULONG(m_ulRowCount); cRowCount++)
	{
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,cRowCount,m_ulRowCount,
			1,&cRowsObtained,&prghRows), S_OK))
			return TEST_FAIL;

		if(!VerifyRowPosition(rghRows[0],cRowCount+1,g_pCTable, (cRowCount==0 ? EXACT : INEXACT)))
			goto END;
				
		if(!CHECK(m_pIRowset->ReleaseRows(1,rghRows,NULL,NULL,NULL),S_OK))
			goto END;	
	}
	
	//cRowCount=m_ulRowCount;
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,cRowCount,m_ulRowCount,
		1,&cRowsObtained,&prghRows), DB_S_ENDOFROWSET))
			goto END;

	fTestPass = TEST_PASS;

END:
	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}	


// {{ TCW_TC_PROTOTYPE(KeysetCursor_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		KeysetCursor_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	return KeysetCursor::Init();	
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(KeysetCursor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(KeysetCursor_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		KeysetCursor_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return KeysetCursor::Init();
	// }}	
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(KeysetCursor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(KeysetCursor_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		KeysetCursor_SchemaR - In context of a Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_SchemaR::Init()
{
	// wrong to ask for Dynamic cursor on Schema Rowset
	return TEST_SKIPPED;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL KeysetCursor_SchemaR::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(KeysetCursor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DynamicCursor)
//*-----------------------------------------------------------------------
//|	Test Case:		DynamicCursor - Test GetRowsAtRatio via a Dynamic cursor.
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor::Init()
{
	DBPROPID	guidPropertySet[4];
	ULONG		cPrptSet=0;

	if(!TCIRowsetScroll::Init())
		return FALSE;

	guidPropertySet[cPrptSet]=DBPROP_CANFETCHBACKWARDS;
	cPrptSet++;

	guidPropertySet[cPrptSet]=DBPROP_CANSCROLLBACKWARDS;
	cPrptSet++;

	guidPropertySet[cPrptSet]=DBPROP_CANHOLDROWS;
	cPrptSet++;
		
	guidPropertySet[cPrptSet]=DBPROP_OTHERINSERT;
	cPrptSet++;
	
	// Create a rowset and an accessor.  
	// DBPROP_CANFETCHBACKWARDS, DBPROP_CANSCROLLBACKWARDS and DBPROP_CANHOLDROWS
	// are requested.
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetScroll,
		cPrptSet, guidPropertySet))
		return TEST_SKIPPED;

	return TRUE;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=1, ulDenominator=20, cRows=(-1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_1()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=1, ulDenominator=last row, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,m_ulRowCount,
		-1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	// Only one row should have been retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto END;

	// The first row should have been retrieved
	if(!VerifyRowPosition(prghRows[0], 2, g_pCTable, INEXACT))
		goto END;
			
	fTestPass=TRUE;

END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=2, ulDenominator=20, cRows=(-3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_2()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		*prghRows=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=2, ulDenominator=m_ulRowCount, cRows=(-3).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,1,m_ulRowCount,
		-3,&cRowsObtained,&prghRows), DB_S_ENDOFROWSET))
		goto END;

	// Two rows should have been retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto END;

	// Verify the ows
	if(!VerifyRowPosition(prghRows[0], 2, g_pCTable, INEXACT))
		goto END;

	if(!VerifyRowPosition(prghRows[1], 1, g_pCTable, INEXACT))
		goto END;
	
	fTestPass=TRUE;
END:
	// Release the row handle
	if(prghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,prghRows,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(prghRows);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc ulNumerator=3, ulDenominator=20, cRows=(-3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_3()
{
	DBCOUNTITEM	cFirstRowsObtained;
	DBCOUNTITEM	cSecondRowsObtained;
	HROW		*pFirsthRow=NULL;
	HROW		*pSecondhRow=NULL;
	BOOL		fTestPass=FALSE;

	// ulNumerator=3, ulDenominator=last row, cRows=(-3).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,3,m_ulRowCount,
		-3,&cFirstRowsObtained,&pFirsthRow), S_OK))
		goto END;

	// Three rows should have been retrieved
	if(!COMPARE(cFirstRowsObtained, 3))
		goto END;

	if(!VerifyRowPosition(*pFirsthRow, 4, g_pCTable, INEXACT))
		goto END;

	// ulNumerator=4, ulDenominator=last row, cRows=(-1).
	if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,4,m_ulRowCount,
		-1,&cSecondRowsObtained,&pSecondhRow), S_OK))
		goto END;

	// Only one row should have been retrieved
	if(!COMPARE(cSecondRowsObtained, 1))
	{
		fTestPass = FALSE;
		goto END;
	}

	// The 4th row handle should have been retrieved
	if(!VerifyRowPosition(*pSecondhRow, 5, g_pCTable, INEXACT))
		goto END;
	
	fTestPass=TRUE;

END:
	// Release the row handles
	if(pFirsthRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cFirstRowsObtained,pFirsthRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pFirsthRow);
	}

	// Release the row handles
	if(pSecondhRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cSecondRowsObtained,pSecondhRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pSecondhRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc Get one row handle at a time.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DynamicCursor::Variation_4()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		rghRows[1];
	HROW		*prghRows=rghRows;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass = TEST_FAIL;

	// Get one row handle at a time from the first row
	for(cRowCount=0; cRowCount < m_ulRowCount; cRowCount++)
	{
		if(!CHECK(m_pIRowsetScroll->GetRowsAtRatio(NULL,NULL,cRowCount,m_ulRowCount,
			1,&cRowsObtained,&prghRows), S_OK))
			return TEST_FAIL;

		// Verify the row position
		if(!VerifyRowPosition(rghRows[0],cRowCount+1,g_pCTable,(cRowCount == 0 ? EXACT : INEXACT)))
			goto END;

		if(!CHECK(m_pIRowset->ReleaseRows(1,rghRows,NULL,NULL,NULL),S_OK))
			goto END;
	}

	fTestPass = TEST_PASS;

END:	
	return fTestPass;
}


//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetScroll::Terminate());
}


// {{ TCW_TC_PROTOTYPE(DynamicCursor_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		DynamicCursor_Cmd - In context of commands
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_Cmd::Init()
{
	if (!SetRowsetType(USECOMMAND))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return DynamicCursor::Init();
}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_Cmd::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DynamicCursor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DynamicCursor_OpenRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		DynamicCursor_OpenRowset - In context of IOpenRowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_OpenRowset::Init()
{
	if (!SetRowsetType(USEOPENROWSET))
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	return DynamicCursor::Init();
	// }}
}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_OpenRowset::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DynamicCursor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DynamicCursor_SchemaR)
//*-----------------------------------------------------------------------
//|	Test Case:		DynamicCursor_SchemaR - In context of a Schema Rowset
//|	Created:			09/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_SchemaR::Init()
{
	// wrong to ask for Dynamic cursor on Schema Rowset
	return TEST_SKIPPED;
}



// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DynamicCursor_SchemaR::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(DynamicCursor::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Zombie::Init()
{
	if(!CTransaction::Init())
	   	return TEST_SKIPPED;                                                         
                        
	m_cProperty=1;

	m_DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	m_DBPropSet.cProperties=m_cProperty;
	m_DBPropSet.rgProperties=m_rgProperty;

   // Get key cursor & IRowsetChange                                          
	m_rgProperty[0].dwPropertyID=DBPROP_IRowsetScroll;                             
	m_rgProperty[0].dwOptions=DBPROPOPTIONS_REQUIRED;                                                
	m_rgProperty[0].vValue.vt=VT_BOOL;                                          
	V_BOOL(&m_rgProperty[0].vValue)=VARIANT_TRUE;                                                                                                                  
                                                                                 
   // Register interface to be tested                                         
	if(!RegisterInterface(ROWSET_INTERFACE, IID_IRowsetScroll, 1, &m_DBPropSet)) 
   		return TEST_SKIPPED;
	
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=TRUE. Query based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_1()
{
	DBCOUNTITEM				cRowsObtained;
	HROW					rghRows[1];
	HROW					*prghRows=rghRows;
	BOOL					fTestPass=FALSE;
	IRowsetScroll		*pIRowsetScroll=NULL;

	// Start a transaction. Create a rowset with IRowsetScroll pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetScroll,
		1, &m_DBPropSet))
		goto END;

	// ulNumerator=50, ulDenominator=100, cRows=1.
	if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;

	prghRows=NULL;
		
	// Commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto END;

	if(!m_fCommitPreserve)
	{
		// Test zombie
		if(CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), E_UNEXPECTED))
		{
			if((COMPARE(cRowsObtained, 0)) && (COMPARE(prghRows, NULL)))
				fTestPass=TRUE;
			goto END;
		}
	}
	else  // fully functional
	{
		// ulNumerator=50, ulDenominator=100, cRows=1.
		if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), S_OK))
			goto END;

		// One row should have been retrieved
		if(COMPARE(cRowsObtained, 1))
			fTestPass=TRUE;
	}

END:
	// Release the row handle
	if(prghRows)
		CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetScroll);

	// Clean up. Expected S_OK.
	CleanUpTransaction(S_OK);

	if(fTestPass)
		return TEST_PASS;
	else					  
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=FALSE. Cursor based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_2()
{
	DBCOUNTITEM				cRowsObtained;
	HROW					rghRows[1];
	HROW					*prghRows=rghRows;
	BOOL					fTestPass=FALSE;
	IRowsetScroll			*pIRowsetScroll=NULL;

	// Start a transaction. Create a rowset with IRowsetScroll pointer.
	// Cursor based update
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetScroll,
		1, &m_DBPropSet))
		goto END;

	// ulNumerator=77, ulDenominator=435, cRows=1.
	if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,77,435,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;

	prghRows=NULL;

	// Commit the transaction with fRetaining==FALSE
	if(!GetCommit(FALSE))
		goto END;

	if(!m_fCommitPreserve)
	{
		// Test zombie
		if(CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), E_UNEXPECTED))
		{
			if((COMPARE(cRowsObtained, 0)) && (COMPARE(prghRows, NULL)))
				fTestPass=TRUE;
			goto END;
		}
	}
	else // fully functional
	{
		// ulNumerator=50, ulDenominator=100, cRows=1.
		if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), S_OK))
			goto END;

		// One row should have been retrieved
		if(COMPARE(cRowsObtained, 1))
			fTestPass=TRUE;
	}

END:
	// Release the row handle
	if(prghRows)
		CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetScroll);

	// Clean up. Expected XACT_E_NOTRANSACTION.
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=TRUE. Cursor  based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_3()
{
	DBCOUNTITEM			cRowsObtained;
	HROW				rghRows[1];
	HROW				*prghRows=rghRows;
	BOOL				fTestPass=FALSE;
	IRowsetScroll		*pIRowsetScroll=NULL;

	// Start a transaction. Create a rowset with IRowsetScroll pointer.
	// Cursor based update
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetScroll,
		1, &m_DBPropSet))
		goto END;

	// ulNumerator=111, ulDenominator=999, cRows=1.
	if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,111,999,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;

	prghRows=NULL;

	// Abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto END;

	if(!m_fAbortPreserve)
	{
		// Test zombie
		if(CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), E_UNEXPECTED))
		{
			if((COMPARE(cRowsObtained, 0)) && (COMPARE(prghRows, NULL)))
				fTestPass=TRUE;
			goto END;
		}
	}
	else // fully functional
	{
		// ulNumerator=50, ulDenominator=100, cRows=1.
		if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), S_OK))
			goto END;

		// One row should have been retrieved
		if(COMPARE(cRowsObtained, 1))
			fTestPass=TRUE;
	}

END:
	// Release the row handle
	if(prghRows)
		CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetScroll);

	// Clean up
	CleanUpTransaction(S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=FALSE. Query based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_4()
{
	DBCOUNTITEM			cRowsObtained;
	HROW				rghRows[1];
	HROW				*prghRows=rghRows;
	BOOL				fTestPass=FALSE;
	IRowsetScroll		*pIRowsetScroll=NULL;

	// Start a transaction. Create a rowset with IRowsetScroll pointer.
	// Query based update.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetScroll,
		1, &m_DBPropSet))
		goto END;

	// ulNumerator=555, ulDenominator=1100, cRows=1.
	if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,555,1100,
		1,&cRowsObtained,&prghRows), S_OK))
		goto END;

	if(!CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK))
		goto END;
	prghRows=NULL;

	// Abort the transaction with fRetaining==FALSE
	if(!GetAbort(FALSE))
		goto END;

	if(!m_fAbortPreserve)
	{
		// Test zombie
		if(CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), E_UNEXPECTED))
		{
			if((COMPARE(cRowsObtained, 0)) && (COMPARE(prghRows, NULL)))
				fTestPass=TRUE;
			goto END;
		}
	}
	else // fully functional
	{
		// ulNumerator=50, ulDenominator=100, cRows=1.
		if(!CHECK(pIRowsetScroll->GetRowsAtRatio(NULL,NULL,50,100,
			1,&cRowsObtained,&prghRows), S_OK))
			goto END;
	
		// One row should have been retrieved
		if(COMPARE(cRowsObtained, 1))
			fTestPass=TRUE;
	}

END:
	// Release the row handle
	if(prghRows)
		CHECK(m_pIRowset->ReleaseRows(1,prghRows,NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetScroll);

	//clean up
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	return fTestPass;
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
	return(CTransaction::Terminate());
}
// }}

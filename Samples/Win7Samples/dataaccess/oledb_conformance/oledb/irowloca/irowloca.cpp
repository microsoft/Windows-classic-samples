//--------------------------------------------------------------------
// Microsoft OLE DB Test				 
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IRowLoca.cpp | Source file for the test module IRowsetLocate.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "irowloca.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xfff25b4c, 0x7217, 0x11cf, { 0x89, 0x83, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("IRowsetLocate");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("The test module for IRowsetLocate");
DECLARE_MODULE_VERSION(836347189);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTable *		g_pCTable=NULL;				// pointer to the global table
CTable *		g_p1RowTable=NULL;
IDBProperties *	g_pIDBProperties=NULL;			// pointer to IDBCreateSession interface
DBROWCOUNT		g_lRowLast = 0;					// Keeps track of rows on table
DBTYPE			g_dwBookmarkType=DBTYPE_EMPTY;	// the db data type of bookmarks


enum ePrptIdx	{
	IDX_Bookmarks=0, 
	IDX_OrderedBookmarks, 
	IDX_LiteralBookmarks,
	IDX_FetchBackwards, 
	IDX_ScrollBackwards, 
	IDX_CanHoldRows,
	IDX_RemoveDeleted, 
	IDX_BookmarkSkipped,
	IDX_OtherUpdateDelete,	
	IDX_OtherInsert, 
	IDX_BookmarkType,
	IDX_IRowsetDeleteBookmarks, 
	IDX_IRowsetChange};

/////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////
#define TESTC_DRIVER(exp)		{ if((exp)==FALSE) { odtLog << L"NotSupported by Provider, skipping Variation\n"; fTestPass = TEST_SKIPPED; goto CLEANUP;			} }

#define PROPERTY_COUNT	(IDX_IRowsetChange+1)

//record the properties default values
struct	DBPrptRecord
{
	BOOL	fSupported;
	BOOL	fDefault;
	BOOL	fSettable;
}g_rgDBPrpt[PROPERTY_COUNT];

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
	rgDBPrpt[IDX_OrderedBookmarks]= DBPROP_ORDEREDBOOKMARKS;
	rgDBPrpt[IDX_LiteralBookmarks]= DBPROP_LITERALBOOKMARKS;
	rgDBPrpt[IDX_FetchBackwards]= DBPROP_CANFETCHBACKWARDS;
	rgDBPrpt[IDX_ScrollBackwards]= DBPROP_CANSCROLLBACKWARDS;
	rgDBPrpt[IDX_CanHoldRows]= DBPROP_CANHOLDROWS;
	rgDBPrpt[IDX_RemoveDeleted]= DBPROP_REMOVEDELETED;
	rgDBPrpt[IDX_BookmarkSkipped]= DBPROP_BOOKMARKSKIPPED;
	rgDBPrpt[IDX_OtherUpdateDelete]=DBPROP_OTHERUPDATEDELETE;
	rgDBPrpt[IDX_OtherInsert]=DBPROP_OTHERINSERT;
	rgDBPrpt[IDX_IRowsetDeleteBookmarks]=DBPROP_IRowsetChange;
	rgDBPrpt[IDX_BookmarkType]=DBPROP_BOOKMARKTYPE;
	rgDBPrpt[IDX_IRowsetChange]= DBPROP_IRowsetChange;

	//Verify and Create the Interface pointer for 
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
	FreeProperties(&cPropertyInfoSets,&pPropertyInfoSets,&pDescBuffer);
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
	HRESULT				hr;
	ULONG				ulIndex;
	DBORDINAL			cColumns;
	DB_LORDINAL			*pColumns=NULL;		
	ULONG				cProperties;
	DBPROPSET			*prgProperties=NULL;
	DBPROPIDSET			DBPropIDSet;
	BOOL				fInit=FALSE;
	IUnknown			*pIRowset=NULL;
	WCHAR				*pStringsBuffer=NULL;
	DBCOLUMNINFO		*rgInfo=NULL;
	IColumnsInfo		*pIColumnsInfo=NULL;
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cPropertyCount=PROPERTY_COUNT;

	//Initialize
	DBPropIDSet.rgPropertyIDs=NULL;
	DBPropIDSet.cPropertyIDs=PROPERTY_COUNT;
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;

	//Create a Data Source Object and Initialize
	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;

	//IDBCreateSession
	if(!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&g_pIDBProperties))
		return FALSE;

	// When used with an .ini file, this test performs destructive variations that alter the data
	// of non-updateable columns.  Hence the test notifies PrivLib that read only column data
	// should not be validated
	if (GetModInfo())
		GetModInfo()->SetCompReadOnlyCols(FALSE);

	//create the table
	g_pCTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);
	g_p1RowTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);

	if(!g_pCTable || 
		!SUCCEEDED(g_pCTable->CreateTable(TABLE_ROW_COUNT,1,NULL,PRIMARY,TRUE)) )
	{
		if(g_pCTable)
		{
			delete g_pCTable;
			g_pCTable = NULL;
		}

		odtLog<<wszCreateTableFailed;
		return FALSE;
	}

	g_lRowLast = g_pCTable->GetRowsOnCTable();

	if ( g_lRowLast < 8 )
	{
		odtLog<<L"Need at least an 8 row table for this test!\n";
		return FALSE;
	}

	if(!g_p1RowTable || 
		!SUCCEEDED(g_p1RowTable->CreateTable(1,1,NULL,PRIMARY,TRUE)) )
	{
		if(g_p1RowTable)
		{
			delete g_p1RowTable;
			g_p1RowTable = NULL;
		}
	}

	if ( g_p1RowTable->GetRowsOnCTable() != 1 )
		odtLog<<L"Warning: cannot get an one row table and the row number is "<<g_p1RowTable->GetRowsOnCTable()<<L".\n";

	//make sure IRowsetLocate interface is supported by Opening a rowset
	//and Requesting the IRowsetLocate interface.
	hr = g_pCTable->CreateRowset(USE_OPENROWSET, IID_IRowsetLocate, 
									0,	NULL, &pIRowset, NULL, &cColumns, &pColumns);
	
	//free the memory
	PROVIDER_FREE(pColumns);

	//if E_NOINTERFACE is returned, IRowsetLocate is not supported by the provider
	if(hr==ResultFromScode(E_NOINTERFACE))
	{
		odtLog<<wszIRowsetLocateNotSupported;
		return TEST_SKIPPED;
	}

	if(hr!=ResultFromScode(S_OK))
	{
		odtLog<<wszOpenRowsetFailed;
		return FALSE;
	}

	//get IColumnsInfo pointer
	pIRowset->QueryInterface(IID_IColumnsInfo, (LPVOID *)&pIColumnsInfo);

	if(!pIColumnsInfo)
		goto CLEANUP;

	hr=pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer);

	if(hr!=ResultFromScode(S_OK))
		goto CLEANUP;

	//copy the DBTYPE of bookmark columns
	g_dwBookmarkType=rgInfo->wType;

	//the column for bookmark should be 0
	if(0!=rgInfo->iOrdinal)
		goto CLEANUP;
	
	//check if properites are supported
	cPropertyCount=IDX_IRowsetChange+1;
	DBPropIDSet.rgPropertyIDs=(DBPROPID *)PROVIDER_ALLOC(cPropertyCount *
		sizeof(DBPROPID));

	//if properites are supported
	//init all the properties
	DBPropIDSet.rgPropertyIDs[IDX_Bookmarks]= DBPROP_BOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_OrderedBookmarks]= DBPROP_ORDEREDBOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_LiteralBookmarks]= DBPROP_LITERALBOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_FetchBackwards]= DBPROP_CANFETCHBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_ScrollBackwards]= DBPROP_CANSCROLLBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_CanHoldRows]= DBPROP_CANHOLDROWS;
	DBPropIDSet.rgPropertyIDs[IDX_RemoveDeleted]= DBPROP_REMOVEDELETED;
	DBPropIDSet.rgPropertyIDs[IDX_BookmarkSkipped]= DBPROP_BOOKMARKSKIPPED;
	DBPropIDSet.rgPropertyIDs[IDX_OtherUpdateDelete]=DBPROP_OTHERUPDATEDELETE;
	DBPropIDSet.rgPropertyIDs[IDX_OtherInsert]=DBPROP_OTHERINSERT;
	//For version # 2 only
	//rgDBPrpt[IDX_IRowsetDeleteBookmarks]=DBPROP_IRowsetDeleteBookmarks;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetDeleteBookmarks]=DBPROP_IRowsetChange;
	DBPropIDSet.rgPropertyIDs[IDX_BookmarkType]=DBPROP_BOOKMARKTYPE;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetChange]=DBPROP_IRowsetChange;

	if(!VerifyInterface(pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
		return FALSE;

	//mark everything as supported
	for(ulIndex=0; ulIndex<PROPERTY_COUNT; ulIndex++)
	{
			g_rgDBPrpt[ulIndex].fSupported=TRUE;
			g_rgDBPrpt[ulIndex].fDefault=FALSE;
	}

	if(!SUCCEEDED(hr=pIRowsetInfo->GetProperties(1,&DBPropIDSet, &cProperties, &prgProperties)))
		goto CLEANUP;

	//mark the properties
	for(ulIndex=0; ulIndex<PROPERTY_COUNT; ulIndex++)
	{
		//mark the not supported properties
		if(prgProperties[0].rgProperties[ulIndex].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
		{
			g_rgDBPrpt[ulIndex].fSupported=FALSE;
			g_rgDBPrpt[ulIndex].fDefault=FALSE;
		}
		else
		{	
			if(prgProperties[0].rgProperties[ulIndex].dwStatus!=DBPROPSTATUS_OK)
				odtLog<<L"Error: default value failed for properties indexed at "<<ulIndex<<L".\n";

			//mark as supported properties
			g_rgDBPrpt[ulIndex].fSupported=TRUE;

			if(ulIndex==IDX_BookmarkType)
			{
			   if(prgProperties[0].rgProperties[ulIndex].vValue.lVal
				   !=DBPROPVAL_BMK_NUMERIC)
			   {
				   odtLog<<L"ERROR: The bookmark is not based on numeric!\n";

				   if(prgProperties[0].rgProperties[ulIndex].vValue.lVal
					  !=DBPROPVAL_BMK_KEY)
					   odtLog<<L"ERROR: The bookmark type return false information!\n";
			   }

			}
			else
			{
				g_rgDBPrpt[ulIndex].fDefault=
				V_BOOL(&prgProperties[0].rgProperties[ulIndex].vValue);
			}
		}
	} 

	InitProp((IUnknown *)pThisTestModule->m_pIUnknown);
	fInit=TRUE;

CLEANUP:					   
	
	//release rowset objects
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);

	//free the memory
	PROVIDER_FREE(DBPropIDSet.rgPropertyIDs);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	FreeProperties(&cProperties,&prgProperties);
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
	//drop tables
	if(g_pCTable)
	{
		g_pCTable->DropTable();
		delete g_pCTable;
		g_pCTable = NULL;
	}

	if(g_p1RowTable)
	{
		g_p1RowTable->DropTable();
		delete g_p1RowTable;
		g_p1RowTable = NULL;
	}


	SAFE_RELEASE(g_pIDBProperties);

	//Release IDBCreateCommand interface
	return (ModuleReleaseDBSession(pThisTestModule));
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCIRowsetLocate:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCIRowsetLocate : public CTestCases
{
	private:

	protected:

		//@cmember: interface pointer for IRowsetLocate
		IRowsetLocate		*m_pIRowsetLocate;

		//@cmember: interface pointer for IRowset
		IRowset				*m_pIRowset;

		//@cmember: interface pointer for IAccessor
		IAccessor			*m_pIAccessor;

		//@cmember: the array of rowset object pointers
		IUnknown			*m_pIUnknown;


		//@cmember: Count of Columns for a rowset
		DBORDINAL			m_cColumns;

		//@cmember: the array of Column ordinals in the backend table
		DB_LORDINAL			*m_rgColumns;

		//@cmember:	accessory handle
		HACCESSOR			m_hAccessor;

		//@cmember:	the size of a row
		DBCOUNTITEM			m_cRowSize;

		//@cmember:	the count of binding structure
		DBCOUNTITEM			m_cBinding;

		//@cmember: the array of binding strucuture
		DBBINDING			*m_rgBinding;

		//@cmember:	the pointer to the row buffer
		void				*m_pData;

		//@cmember: HRESULT
		HRESULT    			m_hr;

		//@cmember: Expected HRESULT
		HRESULT    			m_ExpHR;

		//@cmember: the flag set when DBPROP_CANHOLDROWS is on
		BOOL				m_fHoldRows;
		
		//@cmember: the flag set when DBPROP_ORDEREDBOOKMARKS is on
		BOOL				m_fOrderedBookmark;

		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object and set properties, execute a sql statement,
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
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			WCHAR				*pwszTableName=NULL,	
			EEXECUTE			eExecute=EXECUTE_IFNOERROR,
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			BOOL				fBindLongColumns=TRUE,
			ULONG				cOptProperties=0,
			const DBPROPID		*rgOptProperties=NULL
		);

				//@mfun: create an accessor on the rowset.  
		BOOL	GetAccessorOnRowset
		(
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			BOOL				fBindLongColumns=TRUE
		);

		//@mfun: Get the bookmark for the row 
		BOOL	GetBookmark
		(
			ULONG_PTR	ulRow,
			ULONG_PTR *	pcbBookmark,
			BYTE **		ppBookmark
		);

		//@mfunc: compare two bookmars literally
		BOOL CompareLiteralBookmark
		(		ULONG_PTR		cbBookmark1,
				const	BYTE	*pBookmark1,
				ULONG_PTR		cbBookamrk2,
				const	BYTE	*pBookmark2,
				DBCOMPARE		*pDBCompare
		);

		BOOL	BookmarkSkipped();

		BOOL	RemoveDeleted();

		BOOL	GetProp(DBPROPID	DBPropID); 

		//@mfunc: release the memory referenced by the consumer's buffer
		void FreeMemory(CTable *pCTable=g_pCTable);

		//@mfunc: release a rowset object and accessor created on it
		BOOL ReleaseRowsetAndAccessor();

		//@mfunc: release a accessor created on it
		BOOL ReleaseAccessorOnRowset();

		//mfunc: populate the table after delete some rows
		BOOL	PopulateTable();

		//@mfunc: verify the position of the row handle in the row set
		BOOL	VerifyRowPosition
				(
					HROW		hRow,				//row handle
					ULONG_PTR	cRow,				//position expected
					CTable *	pCTable,			//pointer to the CTable
					EVALUE		eValue = PRIMARY	//eValue for MakeData
				);

		//@mfunc: verify the position of the cursor in the row set
		BOOL	VerifyCursorPosition
				(
					ULONG_PTR	 cRow,			//the cursor potision expected
					CTable *	pCTable,		//pointer to the CTable
					BOOL		fMoveBack=FALSE,//whether move the cursor back to its original postion
												//if fMoveBack==FALSE; the cursor will be positioned one row 
												//after the original position.  If fMoveBack==TRUE,
												//DBPROP_CANSCROLLBACKWARDS needs to be set.
					EVALUE		eValue=PRIMARY	//eValue for MakeData
				);

		//@mfunc: verify whether delete/update operations are legal against target Provider.
		BOOL AlteringRowsIsOK();

	public:
		//constructor
		TCIRowsetLocate(WCHAR *wstrTestCaseName);

		//destructor
		virtual ~TCIRowsetLocate();
};


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetLocate constructor, must take testcase name
//			as parameter.
//
TCIRowsetLocate::TCIRowsetLocate(WCHAR * wstrTestCaseName)	//Takes TestCase Class name as parameter
						: CTestCases (wstrTestCaseName) 
{
	//initialize member data
	m_pIRowsetLocate=NULL;
	m_pIRowset = NULL;
	m_pIAccessor=NULL;
	m_pIUnknown = NULL;
	m_cColumns=0;
	m_rgColumns=NULL;
	m_hAccessor=NULL;
	m_cRowSize=0;
	m_cBinding=0;
	m_rgBinding=NULL;
	m_pData=NULL;
	m_hr=E_FAIL;
	m_ExpHR=E_FAIL;
	m_fHoldRows=FALSE;
	m_fOrderedBookmark=FALSE;
}


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetLocate destructor
//
TCIRowsetLocate::~TCIRowsetLocate()
{

}

//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetLocate::Init()
{
	return (CTestCases::Init());
}


//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetLocate::Terminate()
{
	return (CTestCases::Terminate());
}

	
//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetLocate::GetRowsetAndAccessor
(	
	CTable				*pCTable,				//the pointer to the table object	
	EQUERY				eSQLStmt,				//the SQL Statement to create
	IID					riid,					//the interface pointer to return
	ULONG				cProperties,			//the count of properties
	const DBPROPID		*rgProperties,			//the array of properties to be set
	ULONG				cPropertiesUnset,		//the count of properties to be unset
	const DBPROPID		*rgPropertiesUnset,		//the array of properties to be unset	
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	WCHAR				*pwszTableName,			//the table name for the join statement
	EEXECUTE			eExecute,				//execute only if all properties are set
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	BOOL				fBindLongColumns,		//whether to long columns
	ULONG				cOptProperties,			//the count of Optional properties
	const DBPROPID		*rgOptProperties		//the array of optional props to be set
)
{
	HRESULT hr; 

	ULONG				cPropSets=1;
	DBPROPSET			rgPropSets[2];
	
	ULONG				cProp = 0;
	DBPROP				DBProp;

	BOOL				bReturn = FALSE;
	BLOBTYPE			blobType;

	if(fBindLongColumns)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;

	//init rgPropSets[0]
	rgPropSets[0].rgProperties   = NULL;
	rgPropSets[0].cProperties    = 0;
	rgPropSets[0].guidPropertySet= DBPROPSET_ROWSET;

	//Set up the DB Properties struct
	if(cProperties || cPropertiesUnset || cOptProperties)
	{
		//allocate 
		//Might need an extra for DBPROP_UPDATABILITY (+1)
		rgPropSets[0].rgProperties=(DBPROP *)PROVIDER_ALLOC
			(sizeof(DBPROP) * (cProperties + cPropertiesUnset + cOptProperties + 1));

		if(!rgPropSets[0].rgProperties)
			goto CLEANUP;

		ULONG i;
		//go through the loop to set every DB Property required
		for(i=0; i<cProperties; i++)
		{
			//Set KAGPROP_QUERYBASEDUPDATES if need be
			switch(rgProperties[i])
			{
				case KAGPROP_QUERYBASEDUPDATES:
					memset(&DBProp, 0, sizeof(DBPROP));
					DBProp.dwPropertyID=KAGPROP_QUERYBASEDUPDATES;
					DBProp.dwOptions=DBPROPOPTIONS_REQUIRED;
					DBProp.vValue.vt=VT_BOOL;
					V_BOOL(&DBProp.vValue)=VARIANT_TRUE;

					rgPropSets[1].rgProperties=&DBProp;
					rgPropSets[1].cProperties=1;
					rgPropSets[1].guidPropertySet=DBPROPSET_PROVIDERROWSET;

					cPropSets++;
					break;
					
				case DBPROP_UPDATABILITY:
					memset(&rgPropSets[0].rgProperties[cProp], 0, sizeof(DBPROP));
					rgPropSets[0].rgProperties[cProp].dwPropertyID=DBPROP_UPDATABILITY;
					rgPropSets[0].rgProperties[cProp].dwOptions=DBPROPOPTIONS_REQUIRED;
					rgPropSets[0].rgProperties[cProp].vValue.vt=VT_I4;
					rgPropSets[0].rgProperties[cProp].vValue.lVal=
					DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT;
					cProp++;
					break;
			
				default:
					memset(&rgPropSets[0].rgProperties[cProp], 0, sizeof(DBPROP));
					rgPropSets[0].rgProperties[cProp].dwPropertyID   = rgProperties[i];
					rgPropSets[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_REQUIRED;
					rgPropSets[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
					V_BOOL(&rgPropSets[0].rgProperties[cProp].vValue)= VARIANT_TRUE;
					cProp++;
					break;
			}
		}

		//go through the loop to unset every DB Property required
		for(i=0; i<cPropertiesUnset; i++)
		{
			//Only UnSet this property, if it is supported!
			//Otherwise even though we don't want this property it may
			//fail since it might not be supported on a certain driver
			//For Example:  On Access, DBPROP_OTHERINSERT is NOTSUPPORTED
			//So if we try to set to to FALSE to Guarnentee not a ForwardOnly
			//cursor the Rowset will fail, since its not supported...
			if(SupportedProperty(rgPropertiesUnset[i], DBPROPSET_ROWSET, g_pIDBProperties))
			{
				memset(&rgPropSets[0].rgProperties[cProp], 0, sizeof(DBPROP));
				rgPropSets[0].rgProperties[cProp].dwPropertyID	= rgPropertiesUnset[i];
				rgPropSets[0].rgProperties[cProp].dwOptions		= DBPROPOPTIONS_REQUIRED;
				rgPropSets[0].rgProperties[cProp].vValue.vt		= VT_BOOL;
				V_BOOL(&rgPropSets[0].rgProperties[cProp].vValue)	= VARIANT_FALSE;
				cProp++;
			}
		}

		for(i=0; i<cOptProperties; i++)
		{
			memset(&rgPropSets[0].rgProperties[cProp], 0, sizeof(DBPROP));
			rgPropSets[0].rgProperties[cProp].dwPropertyID   = rgOptProperties[i];
			rgPropSets[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_OPTIONAL;
			rgPropSets[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
			V_BOOL(&rgPropSets[0].rgProperties[cProp].vValue)= VARIANT_TRUE;
			cProp++;
			break;
		}

		rgPropSets[0].cProperties = cProp;
	}


	//Set properties and execute the SQL statement
	//May fail due to combinations of properties
	if (!pCTable->GetCommandSupOnCTable())
	{
		// We depend on commands to give back certain types of rowset.
		// If commandds are not supported, bail our for certain query types
		if(eSQLStmt != SELECT_EMPTYROWSET)
			eSQLStmt = USE_OPENROWSET;
		else
			goto CLEANUP;
	}

	hr = pCTable->CreateRowset( eSQLStmt, IID_IRowsetLocate, cPropSets,	rgPropSets, &m_pIUnknown,				
							NULL, &m_cColumns, &m_rgColumns);

	if(hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED || hr==DB_E_NOTSUPPORTED)
	{
		goto CLEANUP;
	}
	
	if(!CHECK(hr,S_OK))
		goto CLEANUP;

	//If a pointer to a rowset is returned, retrieved the pointer
	if(riid==IID_IRowsetLocate)
	{
		m_pIRowsetLocate=(IRowsetLocate *)m_pIUnknown;
		COMPARE((m_pIRowsetLocate->AddRef() >= 1), TRUE);
	}

	//queryinterface for IRowset.  IRowsetLocate implies IRowset
	if(!SUCCEEDED(m_pIUnknown->QueryInterface(IID_IRowset,(LPVOID *)&m_pIRowset)))
		goto CLEANUP;

	//queryinterface for IAccessor
	if(!SUCCEEDED(m_pIUnknown->QueryInterface(IID_IAccessor,(LPVOID *)&m_pIAccessor)))
		goto CLEANUP;

	//if dwAccessorFlags=DBACCESSOR_PASSBYREF, no need to create an accessor
	if(dwAccessorFlags==DBACCESSOR_PASSBYREF)
	{
		bReturn = TRUE;
		goto CLEANUP;
	}

	//create an accessor on the rowset
	if(!CHECK(GetAccessorAndBindings(m_pIUnknown,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,NULL,NULL,dbTypeModifier,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,blobType),S_OK))
			goto CLEANUP;

	//allocate memory for the row
	m_pData = PROVIDER_ALLOC(m_cRowSize);
	if(m_pData)
		bReturn = TRUE;

CLEANUP:
	//free the memory
	PROVIDER_FREE(rgPropSets[0].rgProperties);

	return bReturn;
}

BOOL	TCIRowsetLocate::GetAccessorOnRowset
(
			DBACCESSORFLAGS		dwAccessorFlags,		
			DBPART				dwPart,					
			ECOLS_BOUND			eColsToBind,			
			ECOLUMNORDER		eBindingOrder,			
			ECOLS_BY_REF		eColsByRef,				
			DBTYPE				dbTypeModifier,
			BOOL				fBindLongColumns
)
{
	BLOBTYPE	blobType;

	if(fBindLongColumns)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;

		//create an accessor on the rowset
	if(!CHECK(GetAccessorAndBindings(m_pIUnknown,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,NULL,NULL,dbTypeModifier,blobType),S_OK))
			return FALSE;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(!m_pData)
		return FALSE;

	return TRUE;


}


//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetLocate::GetBookmark
(
	ULONG_PTR	ulRow,
	ULONG_PTR *	pcbBookmark,
	BYTE **		ppBookmark
)
{
	BOOL		fPass = FALSE;
	HROW *		pHRow = NULL;
	DBCOUNTITEM	cCount;
	DBREFCOUNT	cRefCount;

	//ulRow has to start with 1
	if(!pcbBookmark || !ppBookmark || !ulRow)
		return FALSE;

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	//fetch the row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(ulRow-1),1,&cCount,&pHRow),S_OK))
		goto CLEANUP;

	//only one row handle is retrieved
	COMPARE(cCount, 1);

	//get the data
	if(!CHECK(m_pIRowset->GetData(*pHRow, m_hAccessor, m_pData),S_OK))
		goto CLEANUP;

	//make sure the 0 column is for bookmark
	if(!COMPARE(m_rgBinding[0].iOrdinal, 0))
	{
		FreeMemory();
		goto CLEANUP;
	}

	//get the length of the bookmark
	*pcbBookmark= LENGTH_BINDING(m_rgBinding[0], m_pData);
		
	//allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);

	if(!(*ppBookmark))
		goto CLEANUP;

	//copy the value of the bookmark into the consumer's buffer
	memcpy(*ppBookmark, (BYTE *)m_pData+m_rgBinding[0].obValue, *pcbBookmark);

	//free the memory referenced by the consumer's buffer
	FreeMemory();

	fPass=TRUE;

CLEANUP:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,&cRefCount,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	return fPass;
}

//--------------------------------------------------------------------
//@mfunc: compare two bookmars literally.
//
//
//--------------------------------------------------------------------

BOOL TCIRowsetLocate::CompareLiteralBookmark
(
	ULONG_PTR		cbBookmark1,
	const	BYTE	*pBookmark1,
	ULONG_PTR		cbBookmark2,
	const	BYTE	*pBookmark2,
	DBCOMPARE		*pDBCompare
)
{
	ULONG_PTR	cBytes;

	//the bookmarks as to be one byte long
	if(!pBookmark1 || !pBookmark2 || !cbBookmark1 || !pDBCompare)
		return FALSE;

	//the length of two bookmarks have to be the same
	if(cbBookmark1 != cbBookmark2)
		return FALSE;

	//the  data type of bookmark should not be ORed with any type modifiers
	switch(g_dwBookmarkType)
	{
		case DBTYPE_I1:
				if(*(char *)pBookmark1 == *(char *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(char *)pBookmark1 > *(char *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_I2:
				if(*(SHORT *)pBookmark1 == *(SHORT *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(SHORT *)pBookmark1 > *(SHORT *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_I4:
				if(*(LONG *)pBookmark1 == *(LONG *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(LONG *)pBookmark1 > *(LONG *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_UI1: 
				if(*(unsigned char *)pBookmark1 == *(unsigned char *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(unsigned char *)pBookmark1 > *(unsigned char *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_UI2:
				if(*(USHORT *)pBookmark1 == *(USHORT *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(USHORT *)pBookmark1 > *(USHORT *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_UI4:
				if(*(ULONG *)pBookmark1 == *(ULONG *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(ULONG *)pBookmark1 > *(ULONG *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_BYTES:
				//compare byte by byte as the unsigned bytes, most significant
				//byte at the 0th position
				for(cBytes=cbBookmark1; cBytes > 0; cBytes--)
				{
					if(*(unsigned char*)(pBookmark1+(cBytes-1)) > *(unsigned char *)(pBookmark2+(cBytes-1)))
					{
						*pDBCompare=DBCOMPARE_GT;
						break;
					}
					else
					{
						if(*(unsigned char*)(pBookmark1+(cBytes-1)) < *(unsigned char *)(pBookmark2+(cBytes-1)))
						{
							*pDBCompare=DBCOMPARE_LT;
							break;
						}
					}
				}

				if(cBytes==0)
					*pDBCompare=DBCOMPARE_EQ;

				return TRUE;
		
		case DBTYPE_I8:
				if(*(LONGLONG *)pBookmark1 == *(LONGLONG *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(LONGLONG *)pBookmark1 > *(LONGLONG *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		case DBTYPE_UI8:
				if(*(ULONGLONG *)pBookmark1 == *(ULONGLONG *)pBookmark2)
					*pDBCompare=DBCOMPARE_EQ;
				else 
				{
					if(*(ULONGLONG *)pBookmark1 > *(ULONGLONG *)pBookmark2)
						*pDBCompare=DBCOMPARE_GT;
					else
						*pDBCompare=DBCOMPARE_LT;
				}
				return TRUE;
		//bookmark should not be float or double
		case DBTYPE_R4:
		case DBTYPE_R8:
		case DBTYPE_EMPTY:
		case DBTYPE_NULL:
		case DBTYPE_DATE:
		case DBTYPE_NUMERIC:
		case DBTYPE_BOOL:
		case DBTYPE_CY:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BSTR:
		case DBTYPE_VARIANT:
		case DBTYPE_IDISPATCH:
		case DBTYPE_IUNKNOWN:
		case DBTYPE_GUID:
		case DBTYPE_ERROR:
		case DBTYPE_DBDATE:
		case DBTYPE_DBTIME:
		case DBTYPE_DBTIMESTAMP:
		case DBTYPE_UDT:
		default:
				return FALSE;
	}
}

//--------------------------------------------------------------
//
//	 Get information about properties
//
//-----------------------------------------------------------------
BOOL TCIRowsetLocate::GetProp(DBPROPID	DBPropID)
{

	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty=0;
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

	if(FAILED(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet)))
		goto CLEANUP;


	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:

	for(ULONG i=0;i<cProperty;i++)
		PRVTRACE(L"[%d] status == %d \n",pDBPropSet[0].rgProperties[i].dwPropertyID,pDBPropSet[0].rgProperties[i].dwStatus);

	FreeProperties(&cProperty,&pDBPropSet); 
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}


//--------------------------------------------------------------------
//@mfunc:	If BookmarkSkipped is supported on the rowset
//--------------------------------------------------------------------
BOOL TCIRowsetLocate::BookmarkSkipped()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_BOOKMARKSKIPPED;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	if(!g_rgDBPrpt[IDX_BookmarkSkipped].fSupported)
		return FALSE;

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

//--------------------------------------------------------------------
// Populate the table after some rows are deleted
//
BOOL TCIRowsetLocate::PopulateTable()
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
//@mfunc:	If RemoveDeleted is supported on the rowset
//--------------------------------------------------------------------

BOOL TCIRowsetLocate::RemoveDeleted()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_REMOVEDELETED;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	if(!g_rgDBPrpt[IDX_RemoveDeleted].fSupported)
		return FALSE;

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

//--------------------------------------------------------------------
//@mfunc:	free the memory referenced by the consumer's buffer
//			The function has to be called after IRowset::GetData()
//
//--------------------------------------------------------------------
void TCIRowsetLocate::FreeMemory(CTable *pCTable)
{
	//make sure m_pData is not NULL
	if(!COMPARE(!m_pData, NULL))
		return;

	//make sure the columns are bound 
	if(!m_rgColumns)
		return;

	//call compareData with the option to free the memory referenced by the consumer's 
	//buffer without comparing data
	CompareData(m_cColumns,m_rgColumns,1,m_pData,m_cBinding,m_rgBinding,pCTable,
				NULL,PRIMARY,FREE_ONLY);

	return;
}


//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
BOOL TCIRowsetLocate::ReleaseRowsetAndAccessor()
{
	BOOL		fPass=TRUE;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgColumns);

	//free accessor handle
	if(m_hAccessor)
	{
		if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fPass=FALSE;

		m_hAccessor=NULL;
	}

	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetLocate);
	SAFE_RELEASE(m_pIUnknown);

	return fPass;
}


BOOL TCIRowsetLocate::ReleaseAccessorOnRowset()
{
	BOOL		fPass=TRUE;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);

	//free accessor handle
	if(m_hAccessor)
	{
		if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fPass=FAIL;

		m_hAccessor=NULL;
	}

	return fPass;
}

//--------------------------------------------------------------------
//
//@mfunc: verify the position of the row handle in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor
//
//--------------------------------------------------------------------

BOOL	TCIRowsetLocate::VerifyRowPosition
(
	HROW		hRow,		//row handle
	ULONG_PTR	cRow,		//potision expected
	CTable *	pCTable,	//pointer to the CTable
	EVALUE		eValue		//if the accessor is ReadColumnsByRef
)
{
	//input validation
	if(!pCTable)
		return FALSE;

	//m_pIRowset has to be valid
	if(!m_pIRowset || !m_pData)
		return FALSE;

	//Get Data for the row
	if(!CHECK(m_pIRowset->GetData(hRow,m_hAccessor,m_pData),S_OK))
		return FALSE;

	//compare the data with the row expected in the rowset
	if(!CompareData(m_cColumns,m_rgColumns,cRow,m_pData,m_cBinding,m_rgBinding,pCTable,
					NULL,eValue))
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------
//
//@mfunc: verify the position of the cursor in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor.  
//
//--------------------------------------------------------------------

BOOL	TCIRowsetLocate::VerifyCursorPosition
(
	ULONG_PTR	cRow,				//the cursor potision expected
	CTable *	pCTable,			//pointer to the CTable
	BOOL		fMoveBack,			//whether move the cursor back to its original postion
									//if fMoveBack==FALSE; the cursor will be positioned one row 
									//after the original position.  If fMoveBack==TRUE,
									//DBPROP_CANSCROLLBACKWARDS needs to be set.
	EVALUE		eValue				//eValue for MakeData
)
{
	HROW		hRow[1];
	HROW *		pHRow=hRow;
	DBCOUNTITEM	cRows;
	BOOL		fTestPass=TRUE;

	//input validation
	if(COMPARE(pCTable, NULL))
		return FALSE;

	//m_pIRowset has to be valid
	if(COMPARE(m_pIRowset, NULL) ||
	   COMPARE(m_pData,NULL))
		return FALSE;

	//Get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		return FALSE;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(hRow[0],cRow,pCTable,eValue),TRUE))
		fTestPass=FALSE;
	
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);

	//reposition the cursor to its original position
	if(fMoveBack)
	{
		if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,0,&cRows,&pHRow),S_OK))
			fTestPass=FALSE;
	}

	return (fTestPass);
		
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
BOOL TCIRowsetLocate::AlteringRowsIsOK()
{
	// If a specific table was set on the backend, assume we cannot alter it unless
	// we have complete information about it.
	if ( GetModInfo()->GetTableName() && !GetModInfo()->GetFileName())
		return FALSE;
	else
		return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(BOOKMARKS)
//--------------------------------------------------------------------
// @class test DBPROP_BOOKMARKS
//
class BOOKMARKS : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BOOKMARKS,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Compare return DBCOMPARE_NE for different rows and Hash return different hash value.
	int Variation_1();
	// @cmember Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.
	int Variation_2();
	// @cmember *pBookmark=DBBMK_FIRST and lRowsOffset=# of rows in the rowset. DB_S_ENDOFROWSET is returned and no row handles
	int Variation_3();
	// @cmember *pBookmark=DBBMK_LAST and IRowsetOffset=1. DB_S_ENDOFROWSET is returned and no row handles is retrieved.
	int Variation_4();
	// @cmember *pBookmark=DBBMK_FIRST and IRowsOffset=# of rows in the rowset-1.  cRows=2.  DB_S_ENDOFROWS is returned and the last row handl
	int Variation_5();
	// @cmember *pBookmark=DBBMK_LAST and IRowsetOffset==0.  cRows=1.  S_OK is returned and the last row handle is retrieved.
	int Variation_6();
	// @cmember *pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1
	int Variation_7();
	// @cmember Pass an array of bookmarks, one bookmark for each row handle.  S_OK and all the row handles in the rowset should be r
	int Variation_8();
	// @cmember Pass an array of two bookmarks that point to the same row.  S_OK and the row handle is retrieved twice.
	int Variation_9();
	// @cmember Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the first row, DB_E_ROWSNOTRELEASED should be returned.  R
	int Variation_10();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(BOOKMARKS)
#define THE_CLASS BOOKMARKS
BEG_TEST_CASE(BOOKMARKS, TCIRowsetLocate, L"test DBPROP_BOOKMARKS")
	TEST_VARIATION(1, 		L"Compare return DBCOMPARE_NE for different rows and Hash return different hash value.")
	TEST_VARIATION(2, 		L"Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.")
	TEST_VARIATION(3, 		L"*pBookmark=DBBMK_FIRST and lRowsOffset=# of rows in the rowset. DB_S_ENDOFROWSET is returned and no row handles")
	TEST_VARIATION(4, 		L"*pBookmark=DBBMK_LAST and IRowsetOffset=1. DB_S_ENDOFROWSET is returned and no row handles is retrieved.")
	TEST_VARIATION(5, 		L"*pBookmark=DBBMK_FIRST and IRowsOffset=# of rows in the rowset-1.  cRows=2.  DB_S_ENDOFROWS is returned and the last row handl")
	TEST_VARIATION(6, 		L"*pBookmark=DBBMK_LAST and IRowsetOffset==0.  cRows=1.  S_OK is returned and the last row handle is retrieved.")
	TEST_VARIATION(7, 		L"*pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1")
	TEST_VARIATION(8, 		L"Pass an array of bookmarks, one bookmark for each row handle.  S_OK and all the row handles in the rowset should be r")
	TEST_VARIATION(9, 		L"Pass an array of two bookmarks that point to the same row.  S_OK and the row handle is retrieved twice.")
	TEST_VARIATION(10, 		L"Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the first row, DB_E_ROWSNOTRELEASED should be returned.  R")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(OrderedBookmarks)
//--------------------------------------------------------------------
// @class test DBPROP_ORDEREDBOOKMARKS
//
class OrderedBookmarks : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(OrderedBookmarks,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.
	int Variation_1();
	// @cmember The first bookmark is after the second. Compare return DBCOMPARE_GT for the row and Hash return a bigger hash value for the fir
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(OrderedBookmarks)
#define THE_CLASS OrderedBookmarks
BEG_TEST_CASE(OrderedBookmarks, TCIRowsetLocate, L"test DBPROP_ORDEREDBOOKMARKS")
	TEST_VARIATION(1, 		L"Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.")
	TEST_VARIATION(2, 		L"The first bookmark is after the second. Compare return DBCOMPARE_GT for the row and Hash return a bigger hash value for the fir")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(OrderedBookmarks_Fetch_Scroll)
//--------------------------------------------------------------------
// @class DBPROP_ORDEREDBOOKMARKS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANSCROLLBACKWARDS
//
class OrderedBookmarks_Fetch_Scroll : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(OrderedBookmarks_Fetch_Scroll,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The first bookmark is before the second.  Compare return for the row and Hash return a smaller hash value for the first bookmar
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(OrderedBookmarks_Fetch_Scroll)
#define THE_CLASS OrderedBookmarks_Fetch_Scroll
BEG_TEST_CASE(OrderedBookmarks_Fetch_Scroll, TCIRowsetLocate, L"DBPROP_ORDEREDBOOKMARKS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANSCROLLBACKWARDS")
	TEST_VARIATION(1, 		L"The first bookmark is before the second.  Compare return for the row and Hash return a smaller hash value for the first bookmar")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(LiteralBookmarks)
//--------------------------------------------------------------------
// @class test DBPROP_LITERALBOOKMARKS
//
class LiteralBookmarks : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(LiteralBookmarks,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Compare the bookmarks for the different row directly, based on the data type of the bookmark.
	int Variation_1();
	// @cmember Compare the bookmarks for the same row directly, based on the data type of the bookmark.
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(LiteralBookmarks)
#define THE_CLASS LiteralBookmarks
BEG_TEST_CASE(LiteralBookmarks, TCIRowsetLocate, L"test DBPROP_LITERALBOOKMARKS")
	TEST_VARIATION(1, 		L"Compare the bookmarks for the different row directly, based on the data type of the bookmark.")
	TEST_VARIATION(2, 		L"Compare the bookmarks for the same row directly, based on the data type of the bookmark.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Literal_Ordered_CanHoldRows)
//--------------------------------------------------------------------
// @class Test DBPROP_LITERALBOOKMARKS + DBPROP_ORDEREDBOOKMARKS + DBPROP_CANHOLDROWS
//
class Literal_Ordered_CanHoldRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Literal_Ordered_CanHoldRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The first bookmark is after the second.  Compare the bookmarks directly, based on the data type of the bookmark.
	int Variation_1();
	// @cmember The first bookmark is before the second.  Compare the bookmarks directly, based on the data type of the bookmark.
	int Variation_2();
	// @cmember The first bookmark is the same as the second.  Compare the bookmarks directly, based on the data type of the bookmark.
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Literal_Ordered_CanHoldRows)
#define THE_CLASS Literal_Ordered_CanHoldRows
BEG_TEST_CASE(Literal_Ordered_CanHoldRows, TCIRowsetLocate, L"Test DBPROP_LITERALBOOKMARKS + DBPROP_ORDEREDBOOKMARKS + DBPROP_CANHOLDROWS")
	TEST_VARIATION(1, 		L"The first bookmark is after the second.  Compare the bookmarks directly, based on the data type of the bookmark.")
	TEST_VARIATION(2, 		L"The first bookmark is before the second.  Compare the bookmarks directly, based on the data type of the bookmark.")
	TEST_VARIATION(3, 		L"The first bookmark is the same as the second.  Compare the bookmarks directly, based on the data type of the bookmark.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ScrollBackwards)
//--------------------------------------------------------------------
// @class Test DBPROP_CANSCROLLBACKWARDS
//
class ScrollBackwards : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ScrollBackwards,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember *pBookmark=DBBMK_FIRST and lRowsetOffset=-1.  DB_S_ENDOFROWSET is returned and no row handles is retrieved.
	int Variation_1();
	// @cmember *pBookmark=DBBMK_LAST and lRowsetOffset=-(# of rows in the rowset
	int Variation_2();
	// @cmember *pBookmark is the bookmark on the second row of the rowset.  lRowsetOffset=-1, cRows=# of rows in the rowset.  S_OK is returned
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ScrollBackwards)
#define THE_CLASS ScrollBackwards
BEG_TEST_CASE(ScrollBackwards, TCIRowsetLocate, L"Test DBPROP_CANSCROLLBACKWARDS")
	TEST_VARIATION(1, 		L"*pBookmark=DBBMK_FIRST and lRowsetOffset=-1.  DB_S_ENDOFROWSET is returned and no row handles is retrieved.")
	TEST_VARIATION(2, 		L"*pBookmark=DBBMK_LAST and lRowsetOffset=-(# of rows in the rowset")
	TEST_VARIATION(3, 		L"*pBookmark is the bookmark on the second row of the rowset.  lRowsetOffset=-1, cRows=# of rows in the rowset.  S_OK is returned")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(FetchBackwards)
//--------------------------------------------------------------------
// @class test DBPROP_CANFETCHBACKWARDS
//
class FetchBackwards : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(FetchBackwards,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember *pBookmark=DBBMK_FIRST and lRowsetOffset==1.  cRows=-3.  DB_S_ENDOFROWSET is returned and only two row handle is retrieved.
	int Variation_1();
	// @cmember *pBookmark=DBBMK_LAST and lRowsetOffset==0. cRows=-6.  DB_S_ENDOFROWSET is returned and only 5 row handles are retrieved.  Ver
	int Variation_2();
	// @cmember *pBookmark is the bookmark on the 4th row.  lRowsetOffset=1 and cRows=-2.  S_OK is returned and the 5th and the 4th row handles
	int Variation_3();
	// @cmember *pBookmark=DBBMK_LAST.  Get one row handle at a time (lRowsetOffset==0 and cRows==-1
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(FetchBackwards)
#define THE_CLASS FetchBackwards
BEG_TEST_CASE(FetchBackwards, TCIRowsetLocate, L"test DBPROP_CANFETCHBACKWARDS")
	TEST_VARIATION(1, 		L"*pBookmark=DBBMK_FIRST and lRowsetOffset==1.  cRows=-3.  DB_S_ENDOFROWSET is returned and only two row handle is retrieved.")
	TEST_VARIATION(2, 		L"*pBookmark=DBBMK_LAST and lRowsetOffset==0. cRows=-6.  DB_S_ENDOFROWSET is returned and only 5 row handles are retrieved.  Ver")
	TEST_VARIATION(3, 		L"*pBookmark is the bookmark on the 4th row.  lRowsetOffset=1 and cRows=-2.  S_OK is returned and the 5th and the 4th row handles")
	TEST_VARIATION(4, 		L"*pBookmark=DBBMK_LAST.  Get one row handle at a time (lRowsetOffset==0 and cRows==-1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_Fetch)
//--------------------------------------------------------------------
// @class test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS
//
class Scroll_Fetch : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_Fetch,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember *pBookmark=DBBMK_LAST and lRowOffset=-2.  cRows=-3.  The third, 4th, and 5th row handles are retrieved.  *pBoolmark=DBBMK_FIRST
	int Variation_1();
	// @cmember cRows == LONG_MIN
	int Variation_2();
	// @cmember lRowsOffset == LONG_MIN
	int Variation_3();
	// @cmember cRows = LONG_MAX/2 + 1
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Scroll_Fetch)
#define THE_CLASS Scroll_Fetch
BEG_TEST_CASE(Scroll_Fetch, TCIRowsetLocate, L"test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS")
	TEST_VARIATION(1, 		L"*pBookmark=DBBMK_LAST and lRowOffset=-2.  cRows=-3.  The third, 4th, and 5th row handles are retrieved.  *pBoolmark=DBBMK_FIRST")
	TEST_VARIATION(2, 		L"cRows == LONG_MIN")
	TEST_VARIATION(3, 		L"lRowsOffset == LONG_MIN")
	TEST_VARIATION(4, 		L"cRows = LONG_MAX/2 + 1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_Fetch_CanHoldRows)
//--------------------------------------------------------------------
// @class Test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANHOLDROWS
//
class Scroll_Fetch_CanHoldRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_Fetch_CanHoldRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.
	int Variation_1();
	// @cmember *pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles
	int Variation_2();
	// @cmember *pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.  *pBookmar
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Scroll_Fetch_CanHoldRows)
#define THE_CLASS Scroll_Fetch_CanHoldRows
BEG_TEST_CASE(Scroll_Fetch_CanHoldRows, TCIRowsetLocate, L"Test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANHOLDROWS")
	TEST_VARIATION(1, 		L"pBookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.")
	TEST_VARIATION(2, 		L"*pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles")
	TEST_VARIATION(3, 		L"*pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.  *pBookmar")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CanHoldRows)
//--------------------------------------------------------------------
// @class test DBPROP_CANHOLDROWS
//
class CanHoldRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanHoldRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the third row, S_OK.
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(CanHoldRows)
#define THE_CLASS CanHoldRows
BEG_TEST_CASE(CanHoldRows, TCIRowsetLocate, L"test DBPROP_CANHOLDROWS")
	TEST_VARIATION(1, 		L"Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the third row, S_OK.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MaxOpenRows)
//--------------------------------------------------------------------
// @class test DBPROP_MAXOPENROWS
//
class MaxOpenRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxOpenRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Set MAXOPENROWS to 3.  *pBookmark points to the second row.  lRowsOffset==1 and cRows=4.  DB_S_ENDOFROWSET
	int Variation_1();
	// @cmember SetMAXOPENROWS to 2.  *pBookmark points to the third row.  lRowsOffset==0 and cRows=3.  DB_S_ROWLIMITEXCEEDED
	int Variation_2();
	// @cmember Set MAXOPENROWS to 3.  Pass an array of 3 bookmarks to different rows.  S_OK.
	int Variation_3();
	// @cmember Set MAXOPENROWS to 2.  Pass an array of 4 bookmarks. DB_S_ROWLIMITEXCEEDED is returned  and only two row handles are retrieved.
	int Variation_4();
	// @cmember Use default MaxOpenRows
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(MaxOpenRows)
#define THE_CLASS MaxOpenRows
BEG_TEST_CASE(MaxOpenRows, TCIRowsetLocate, L"test DBPROP_MAXOPENROWS")
	TEST_VARIATION(1, 		L"Set MAXOPENROWS to 3.  *pBookmark points to the second row.  lRowsOffset==1 and cRows=4.  DB_S_ENDOFROWSET")
	TEST_VARIATION(2, 		L"SetMAXOPENROWS to 2.  *pBookmark points to the third row.  lRowsOffset==0 and cRows=3.  DB_S_ROWLIMITEXCEEDED")
	TEST_VARIATION(3, 		L"Set MAXOPENROWS to 3.  Pass an array of 3 bookmarks to different rows.  S_OK.")
	TEST_VARIATION(4, 		L"Set MAXOPENROWS to 2.  Pass an array of 4 bookmarks. DB_S_ROWLIMITEXCEEDED is returned  and only two row handles are retrieved.")
	TEST_VARIATION(5, 		L"Use default MaxOpenRows")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Related_IRowset)
//--------------------------------------------------------------------
// @class test related interface IRowset
//
class Related_IRowset : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowset,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetRowsAt does not change the cursor location of the rowset.
	int Variation_1();
	// @cmember GetRowsByBookmark does not change the cursor location of the rowset
	int Variation_2();
	// @cmember Retrieve the same row handle twice by GetRowsAt and GetRowsByBookmark
	int Variation_3();
	// @cmember Make sure IRowsetLocate::GetRowsAt return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is not rele
	int Variation_4();
	// @cmember Make sure IRowsetLocate::GetRowsByBookmark return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is
	int Variation_5();
	// @cmember Fetch the last row.  Fetch the second to last row.  Fetch the 1st row.  Fetch the last row.
	int Variation_6();
	// @cmember position at the lst row.  GetRowsAt.  Fetch backwards one row.
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Related_IRowset)
#define THE_CLASS Related_IRowset
BEG_TEST_CASE(Related_IRowset, TCIRowsetLocate, L"test related interface IRowset")
	TEST_VARIATION(1, 		L"GetRowsAt does not change the cursor location of the rowset.")
	TEST_VARIATION(2, 		L"GetRowsByBookmark does not change the cursor location of the rowset")
	TEST_VARIATION(3, 		L"Retrieve the same row handle twice by GetRowsAt and GetRowsByBookmark")
	TEST_VARIATION(4, 		L"Make sure IRowsetLocate::GetRowsAt return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is not rele")
	TEST_VARIATION(5, 		L"Make sure IRowsetLocate::GetRowsByBookmark return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is")
	TEST_VARIATION(6, 		L"Fetch the last row.  Fetch the second to last row.  Fetch the 1st row.  Fetch the last row.")
	TEST_VARIATION(7, 		L"position at the lst row.  GetRowsAt.  Fetch backwards one row.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Rowset_SingleRow)
//--------------------------------------------------------------------
// @class test single row rowset
//
class Rowset_SingleRow : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_SingleRow,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Compare return DBCOMPARE_EQ and Hash return the same value
	int Variation_1();
	// @cmember GetRowsAt: *pBookmark=DBBMK_FIRST. lRowsOFfset==1 and cRows=1.  DB_S_ENDOFROWSET
	int Variation_2();
	// @cmember GetRowsAt: *pBookmark=DBBMK_LAST, lRowsOffset=0 and cRows=1.  S_OK
	int Variation_3();
	// @cmember GetRowsByBookmark: Pass array of two bookmarks: the first and the last row.
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Rowset_SingleRow)
#define THE_CLASS Rowset_SingleRow
BEG_TEST_CASE(Rowset_SingleRow, TCIRowsetLocate, L"test single row rowset")
	TEST_VARIATION(1, 		L"Compare return DBCOMPARE_EQ and Hash return the same value")
	TEST_VARIATION(2, 		L"GetRowsAt: *pBookmark=DBBMK_FIRST. lRowsOFfset==1 and cRows=1.  DB_S_ENDOFROWSET")
	TEST_VARIATION(3, 		L"GetRowsAt: *pBookmark=DBBMK_LAST, lRowsOffset=0 and cRows=1.  S_OK")
	TEST_VARIATION(4, 		L"GetRowsByBookmark: Pass array of two bookmarks: the first and the last row.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Consistency)
//--------------------------------------------------------------------
// @class make sure GetRowsAt and GetRowsByBookmark return the same value
//
class Consistency : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Consistency,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve three row handles from GetRowsAt and GetRowsByBookmark, one of which is overlapping
	int Variation_1();
	// @cmember Retrive the first row, GetRowsAt uses DBBMK_FIRST
	int Variation_2();
	// @cmember Retrieve the last row.  GetRowsAt uses DBBMK_LAST.
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Consistency)
#define THE_CLASS Consistency
BEG_TEST_CASE(Consistency, TCIRowsetLocate, L"make sure GetRowsAt and GetRowsByBookmark return the same value")
	TEST_VARIATION(1, 		L"Retrieve three row handles from GetRowsAt and GetRowsByBookmark, one of which is overlapping")
	TEST_VARIATION(2, 		L"Retrive the first row, GetRowsAt uses DBBMK_FIRST")
	TEST_VARIATION(3, 		L"Retrieve the last row.  GetRowsAt uses DBBMK_LAST.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Parameters)
//--------------------------------------------------------------------
// @class valid and invalid parameters passed into methods
//
class Parameters : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Parameters,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetRowsAt: *pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK
	int Variation_1();
	// @cmember GetRowsAt: cRow==0 and *prghRows is not NULL on input
	int Variation_2();
	// @cmember GetRowsAt: *pcRowsObtained is 0 and *prghRows is NULL.
	int Variation_3();
	// @cmember GetRowsByBookmark: cRows==0 and *prghRows and *prgErrors are not NULL.
	int Variation_4();
	// @cmember rgpBookmarks points to DBBMK_INVALID and fRetrunErrors=FALSE.  *prghRows *prgErrors are NULL on input.  DB_S_ERRORSOCCURRED
	int Variation_5();
	// @cmember Pass an array of 3 bookmarks.  The first bookmark is valid, the second bookmark is a NULL pointer and the third element of rgcb
	int Variation_6();
	// @cmember Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST, valid, DBBMK_INVALID.  DB_S_ERRORSOCCURRED
	int Variation_7();
	// @cmember Pass an array of 3 bookmarks: valid, NULL pointer, DBBMK_FIRST.  fReturnErrors=FALSE and prgErrors=NULL, pcErrors is not NULL.
	int Variation_8();
	// @cmember Pass an array of 3 bookmarks: first cbBokmark is less length, second *pBookmark is bogus, and third is valid
	int Variation_9();
	// @cmember Hash:Pass an array of two bookmarks, one is valid and the other is not.  	pcErrors==NULL and prgErrors==NULL
	int Variation_10();
	// @cmember Hash:Pass an invalid bookmark.  pcErrors==NULL and prgErrors is not NULL.
	int Variation_11();
	// @cmember Pass an array of 4 bookmarks.  The first element of rgcbBookmarks is 0, the second bookmark is valid,  the third and the forth
	int Variation_12();
	// @cmember Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST,valid, DBBMK_INVALID.
	int Variation_13();
	// @cmember Valid bookmark, *pcErrors=0
	int Variation_14();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Parameters)
#define THE_CLASS Parameters
BEG_TEST_CASE(Parameters, TCIRowsetLocate, L"valid and invalid parameters passed into methods")
	TEST_VARIATION(1, 		L"GetRowsAt: *pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK")
	TEST_VARIATION(2, 		L"GetRowsAt: cRow==0 and *prghRows is not NULL on input")
	TEST_VARIATION(3, 		L"GetRowsAt: *pcRowsObtained is 0 and *prghRows is NULL.")
	TEST_VARIATION(4, 		L"GetRowsByBookmark: cRows==0 and *prghRows and *prgErrors are not NULL.")
	TEST_VARIATION(5, 		L"rgpBookmarks points to DBBMK_INVALID and fRetrunErrors=FALSE.  *prghRows *prgErrors are NULL on input.  DB_S_ERRORSOCCURRED")
	TEST_VARIATION(6, 		L"Pass an array of 3 bookmarks.  The first bookmark is valid, the second bookmark is a NULL pointer and the third element of rgcb")
	TEST_VARIATION(7, 		L"Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST, valid, DBBMK_INVALID.  DB_S_ERRORSOCCURRED")
	TEST_VARIATION(8, 		L"Pass an array of 3 bookmarks: valid, NULL pointer, DBBMK_FIRST.  fReturnErrors=FALSE and prgErrors=NULL, pcErrors is not NULL.")
	TEST_VARIATION(9, 		L"Pass an array of 3 bookmarks: first cbBokmark is less length, second *pBookmark is bogus, and third is valid")
	TEST_VARIATION(10, 		L"Hash:Pass an array of two bookmarks, one is valid and the other is not.  	pcErrors==NULL and prgErrors==NULL")
	TEST_VARIATION(11, 		L"Hash:Pass an invalid bookmark.  pcErrors==NULL and prgErrors is not NULL.")
	TEST_VARIATION(12, 		L"Pass an array of 4 bookmarks.  The first element of rgcbBookmarks is 0, the second bookmark is valid,  the third and the forth")
	TEST_VARIATION(13, 		L"Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST,valid, DBBMK_INVALID.")
	TEST_VARIATION(14, 		L"Valid bookmark, *pcErrors=0")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Boundary_Compare)
//--------------------------------------------------------------------
// @class boundary conditions for Compare
//
class Boundary_Compare : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	ULONG_PTR	m_cbBookmark;
	BYTE		*m_pBookmark;
	DBCOMPARE	m_DBCompare;
	DBBOOKMARK	m_DBBookmark;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_Compare,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cbBookmark1==0
	int Variation_1();
	// @cmember cbBookmark2==0
	int Variation_2();
	// @cmember pBookmark1==NULL
	int Variation_3();
	// @cmember pBookmark2==NULL
	int Variation_4();
	// @cmember pdwComparistion==NULL
	int Variation_5();
	// @cmember pBookmark1==DBBMK_INVALID
	int Variation_6();
	// @cmember pBookmark2==DBBMK_INVALID
	int Variation_7();
	// @cmember pBookmark1==DBBMK_FIRST
	int Variation_8();
	// @cmember pBookmark2==DBBMK_FIRST
	int Variation_9();
	// @cmember pBookmark1==DBBMK_LAST
	int Variation_10();
	// @cmember pBookmark2==DBBMK_LAST
	int Variation_11();
	// @cmember cbBookmark1==length-1
	int Variation_12();
	// @cmember cbBookmark1==length+1
	int Variation_13();
	// @cmember cbBookmark2==length-1
	int Variation_14();
	// @cmember cbBookmark2==length+1
	int Variation_15();
	// @cmember pBookmark1 points to bogus bookmark
	int Variation_16();
	// @cmember pBookmark2 points to bogus bookmark
	int Variation_17();
	// @cmember cbBmk1 != 1, cbBmk2 == 1
	int Variation_18();
	// @cmember cbBmk1 == 1, cbBmk2 != 1
	int Variation_19();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Boundary_Compare)
#define THE_CLASS Boundary_Compare
BEG_TEST_CASE(Boundary_Compare, TCIRowsetLocate, L"boundary conditions for Compare")
	TEST_VARIATION(1, 		L"cbBookmark1==0")
	TEST_VARIATION(2, 		L"cbBookmark2==0")
	TEST_VARIATION(3, 		L"pBookmark1==NULL")
	TEST_VARIATION(4, 		L"pBookmark2==NULL")
	TEST_VARIATION(5, 		L"pdwComparistion==NULL")
	TEST_VARIATION(6, 		L"pBookmark1==DBBMK_INVALID")
	TEST_VARIATION(7, 		L"pBookmark2==DBBMK_INVALID")
	TEST_VARIATION(8, 		L"pBookmark1==DBBMK_FIRST")
	TEST_VARIATION(9, 		L"pBookmark2==DBBMK_FIRST")
	TEST_VARIATION(10, 		L"pBookmark1==DBBMK_LAST")
	TEST_VARIATION(11, 		L"pBookmark2==DBBMK_LAST")
	TEST_VARIATION(12, 		L"cbBookmark1==length-1")
	TEST_VARIATION(13, 		L"cbBookmark1==length+1")
	TEST_VARIATION(14, 		L"cbBookmark2==length-1")
	TEST_VARIATION(15, 		L"cbBookmark2==length+1")
	TEST_VARIATION(16, 		L"pBookmark1 points to bogus bookmark")
	TEST_VARIATION(17, 		L"pBookmark2 points to bogus bookmark")
	TEST_VARIATION(18, 		L"cbBmk1 != 1, cbBmk2 == 1")
	TEST_VARIATION(19, 		L"cbBmk1 == 1, cbBmk2 != 1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Boundary_GetRowsAt)
//--------------------------------------------------------------------
// @class boundary conditions for GetRowsAt
//
class Boundary_GetRowsAt : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	ULONG_PTR	m_cbBookmark;
	BYTE		*m_pBookmark;
	DBBOOKMARK	m_DBBookmark;
	HROW		*m_pHRow;
	DBCOUNTITEM	m_cRowsObtained;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetRowsAt,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cbBookmark==0, E_INVALIDARG
	int Variation_1();
	// @cmember pBookmark==NULL; E_INVALIDARG
	int Variation_2();
	// @cmember pcRowsObtained==NULL; E_INVALIDARG
	int Variation_3();
	// @cmember prgRows==NULL; E_INVALIDARG
	int Variation_4();
	// @cmember lRowsOffset==-1. DB_S_ENDOFROWSET
	int Variation_5();
	// @cmember cRows=-1, DB_E_CANTFETCHBACKWARDS
	int Variation_6();
	// @cmember cbBookmark=length-1, DB_E_BADBOOKMARK
	int Variation_7();
	// @cmember cbBookmark=length+1; DB_E_BADBOOKMARK
	int Variation_8();
	// @cmember pBookmark points to a bogus bookmark
	int Variation_9();
	// @cmember cRows == LONG_MAX
	int Variation_12();
	// @cmember lRowsOffset == LONG_MAX
	int Variation_13();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Boundary_GetRowsAt)
#define THE_CLASS Boundary_GetRowsAt
BEG_TEST_CASE(Boundary_GetRowsAt, TCIRowsetLocate, L"boundary conditions for GetRowsAt")
	TEST_VARIATION(1, 		L"cbBookmark==0, E_INVALIDARG")
	TEST_VARIATION(2, 		L"pBookmark==NULL; E_INVALIDARG")
	TEST_VARIATION(3, 		L"pcRowsObtained==NULL; E_INVALIDARG")
	TEST_VARIATION(4, 		L"prgRows==NULL; E_INVALIDARG")
	TEST_VARIATION(5, 		L"lRowsOffset==-1. DB_S_ENDOFROWSET")
	TEST_VARIATION(6, 		L"cRows=-1, DB_E_CANTFETCHBACKWARDS")
	TEST_VARIATION(7, 		L"cbBookmark=length-1, DB_E_BADBOOKMARK")
	TEST_VARIATION(8, 		L"cbBookmark=length+1; DB_E_BADBOOKMARK")
	TEST_VARIATION(9, 		L"pBookmark points to a bogus bookmark")
	TEST_VARIATION(12, 		L"cRows == LONG_MAX")
	TEST_VARIATION(13, 		L"lRowsOffset == LONG_MAX")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Boundary_GetRowsByBookmarks)
//--------------------------------------------------------------------
// @class boundary conditions for GetRowsByBookmark
//
class Boundary_GetRowsByBookmarks : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	ULONG_PTR		m_cbBookmark;
	BYTE			*m_rgpBookmarks[2];
	DBBOOKMARK		m_DBBookmark;
	DBCOUNTITEM		m_cRowsObtained;
	HROW			m_rghRows[1];
	HROW			*m_pHRow;


public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_GetRowsByBookmarks,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember rgcbBookmarks==NULL; E_INVALIDARG
	int Variation_1();
	// @cmember prgBookmarks==NULL; E_INVALIDARG
	int Variation_2();
	// @cmember pcRowsObtained=NULL; E_INVALIDARG
	int Variation_3();
	// @cmember prghRows=NULL; E_INVALIDARG
	int Variation_4();
	// @cmember fReturnErrors==TRUE and pcErrors==NULL; E_INVALIDARG
	int Variation_5();
	// @cmember fReturnedErrors=TRUE and prgErrors=NULL; E_INVALIDARG
	int Variation_6();
	// @cmember fReturnErrors=FALSE and pcErrors=NULL; S_OK
	int Variation_7();
	// @cmember fReturnErrors=FALSE and prgErrors=NULl; S_OK
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Boundary_GetRowsByBookmarks)
#define THE_CLASS Boundary_GetRowsByBookmarks
BEG_TEST_CASE(Boundary_GetRowsByBookmarks, TCIRowsetLocate, L"boundary conditions for GetRowsByBookmark")
	TEST_VARIATION(1, 		L"rgcbBookmarks==NULL; E_INVALIDARG")
	TEST_VARIATION(2, 		L"prgBookmarks==NULL; E_INVALIDARG")
	TEST_VARIATION(3, 		L"pcRowsObtained=NULL; E_INVALIDARG")
	TEST_VARIATION(4, 		L"prghRows=NULL; E_INVALIDARG")
	TEST_VARIATION(5, 		L"fReturnErrors==TRUE and pcErrors==NULL; E_INVALIDARG")
	TEST_VARIATION(6, 		L"fReturnedErrors=TRUE and prgErrors=NULL; E_INVALIDARG")
	TEST_VARIATION(7, 		L"fReturnErrors=FALSE and pcErrors=NULL; S_OK")
	TEST_VARIATION(8, 		L"fReturnErrors=FALSE and prgErrors=NULl; S_OK")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Boundary_Hash)
//--------------------------------------------------------------------
// @class boundary conditions for Hash
//
class Boundary_Hash : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	ULONG_PTR		m_rgcbBookmarks[1];
	BYTE			*m_rgpBookmarks[1];
	DBBOOKMARK		m_DBBookmark;
	DBHASHVALUE		m_rgHashedValues[1];

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary_Hash,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cBookmarks==0, S_OK
	int Variation_1();
	// @cmember rgcbBookmarks==NULL; E_INVALIDARG
	int Variation_2();
	// @cmember rgpBookmark==NULL; E_INVALIDARG
	int Variation_3();
	// @cmember rgHashedvalues==NULL; E_INVALIDARG
	int Variation_4();
	// @cmember pcErros is valid and prgErrors=NULL; E_INVALIDARG
	int Variation_5();
	// @cmember pcErros=NULL, S_OK
	int Variation_6();
	// @cmember pcErros==NULL and prgErrors=NULL; S_OK
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Boundary_Hash)
#define THE_CLASS Boundary_Hash
BEG_TEST_CASE(Boundary_Hash, TCIRowsetLocate, L"boundary conditions for Hash")
	TEST_VARIATION(1, 		L"cBookmarks==0, S_OK")
	TEST_VARIATION(2, 		L"rgcbBookmarks==NULL; E_INVALIDARG")
	TEST_VARIATION(3, 		L"rgpBookmark==NULL; E_INVALIDARG")
	TEST_VARIATION(4, 		L"rgHashedvalues==NULL; E_INVALIDARG")
	TEST_VARIATION(5, 		L"pcErros is valid and prgErrors=NULL; E_INVALIDARG")
	TEST_VARIATION(6, 		L"pcErros=NULL, S_OK")
	TEST_VARIATION(7, 		L"pcErros==NULL and prgErrors=NULL; S_OK")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Zombie)
//--------------------------------------------------------------------
// @class zombie states
//
class Zombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPSET	m_DBPropSet;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Zombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with retaining.
	int Variation_1();
	// @cmember Commit without retaining.
	int Variation_2();
	// @cmember Abort with retaining.
	int Variation_3();
	// @cmember Abort without retaining.
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Zombie)
#define THE_CLASS Zombie
BEG_TEST_CASE(Zombie, CTransaction, L"zombie states")
	TEST_VARIATION(1, 		L"Commit with retaining.")
	TEST_VARIATION(2, 		L"Commit without retaining.")
	TEST_VARIATION(3, 		L"Abort with retaining.")
	TEST_VARIATION(4, 		L"Abort without retaining.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Rowset_BigRowset)
//--------------------------------------------------------------------
// @class test basic functionality on a big rowset
//
class Rowset_BigRowset : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset_BigRowset,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(Rowset_BigRowset)
#define THE_CLASS Rowset_BigRowset
BEG_TEST_CASE(Rowset_BigRowset, TCIRowsetLocate, L"test basic functionality on a big rowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(keysetCursor)
//--------------------------------------------------------------------
// @class test GetRowsAt via a keyset driven cursor
//
class keysetCursor : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(keysetCursor,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Bookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.
	int Variation_1();
	// @cmember *pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles
	int Variation_2();
	// @cmember *pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.
	int Variation_3();
	// @cmember *pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(keysetCursor)
#define THE_CLASS keysetCursor
BEG_TEST_CASE(keysetCursor, TCIRowsetLocate, L"test GetRowsAt via a keyset driven cursor")
	TEST_VARIATION(1, 		L"Bookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.")
	TEST_VARIATION(2, 		L"*pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles")
	TEST_VARIATION(3, 		L"*pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.")
	TEST_VARIATION(4, 		L"*pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ExtendedErrors : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	//@cmember Extended error object
	CExtError * m_pExtError;
	
	ULONG_PTR	m_cbBookmark;
	BYTE		*m_pBookmark;
	DBBOOKMARK	m_DBBookmark;
	DBCOUNTITEM	m_cRowsObtained;
	HROW		m_hRow;
	HROW		*m_pHRow;
	BYTE		*m_rgpBookmarks[2];
	HROW		m_rghRows[1];
	ULONG_PTR	m_rgcbBookmarks[1];
	DBHASHVALUE	m_rgHashedValues[1];

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IRowsetLocate calls with previous error object existing.
	int Variation_1();
	// @cmember Valid IRowsetLocate calls with previous error object existing.
	int Variation_2();
	// @cmember valid IRowsetLocate calls with previous error object existing
	int Variation_3();
	// @cmember DB_E_CANTSCROLLBACKWARDS GetRowsAt call with previous error object existing
	int Variation_4();
	// @cmember E_INVALIDARG Compare call with previous error object existing
	int Variation_5();
	// @cmember E_INVALIDARG Hash call with previous error object existing
	int Variation_6();
	// @cmember E_INVALIDARG GetRowsByBookMark call with previous error object existing
	int Variation_7();
	// @cmember DB_E_CANTFETCHBACKWARDS GetRowsAt call with no previous error object existing
	int Variation_8();
	// @cmember DB_E_BADBOOKMARK Compare call with no previous error object existing
	int Variation_9();
	// @cmember E_INVALIDARG Hash call with no previous error object existing
	int Variation_10();
	// @cmember E_INVALIDARG GetRowsByBookMark call with no previous error object existing
	int Variation_11();
	// @cmember DB_E_BADSTARTPOSTION
	int Variation_12();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, TCIRowsetLocate, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IRowsetLocate calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Valid IRowsetLocate calls with previous error object existing.")
	TEST_VARIATION(3, 		L"valid IRowsetLocate calls with previous error object existing")
	TEST_VARIATION(4, 		L"DB_E_CANTSCROLLBACKWARDS GetRowsAt call with previous error object existing")
	TEST_VARIATION(5, 		L"E_INVALIDARG Compare call with previous error object existing")
	TEST_VARIATION(6, 		L"E_INVALIDARG Hash call with previous error object existing")
	TEST_VARIATION(7, 		L"E_INVALIDARG GetRowsByBookMark call with previous error object existing")
	TEST_VARIATION(8, 		L"DB_E_CANTFETCHBACKWARDS GetRowsAt call with no previous error object existing")
	TEST_VARIATION(9, 		L"DB_E_BADBOOKMARK Compare call with no previous error object existing")
	TEST_VARIATION(10, 		L"E_INVALIDARG Hash call with no previous error object existing")
	TEST_VARIATION(11, 		L"E_INVALIDARG GetRowsByBookMark call with no previous error object existing")
	TEST_VARIATION(12, 		L"DB_E_BADSTARTPOSTION")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DeleteRows)
//--------------------------------------------------------------------
// @class delete rows from the rowset
//
class DeleteRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember *pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned and no row handles will be retrieved.
	int Variation_1();
	// @cmember Delete the 4th row.  *pBookmark points to the second row. lRowOffset=3 and cRows=1.  S_OK
	int Variation_2();
	// @cmember Delete the 5th row. *pBookmark points to the 4th row. lRowOffset=1 and cRows=1.  S_OK
	int Variation_3();
	// @cmember Delete the second row.  Pass an array of 3 bookmarks: bookmark to the second row, to the first row, and to the second row.  DB_
	int Variation_4();
	// @cmember Get the 3rd row handle.  GetRowsAt.  Delete the 3rd row.  GetNextRows
	int Variation_5();
	// @cmember Position on 5th row.  Delete 3rd row.  Position on 3rd row.  delete 5th row.  Position on 5th row.  Fetch from the 5th row
	int Variation_6();
	// @cmember Delete the 1st row.  GetRowsAt based on standard bookmarks.
	int Variation_7();
	// @cmember Delete the last row.  Fetch backwards one row handle.  Fetch backwards again.  GetRowsAt based on stand
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DeleteRows)
#define THE_CLASS DeleteRows
BEG_TEST_CASE(DeleteRows, TCIRowsetLocate, L"delete rows from the rowset")
	TEST_VARIATION(1, 		L"*pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned and no row handles will be retrieved.")
	TEST_VARIATION(2, 		L"Delete the 4th row.  *pBookmark points to the second row. lRowOffset=3 and cRows=1.  S_OK")
	TEST_VARIATION(3, 		L"Delete the 5th row. *pBookmark points to the 4th row. lRowOffset=1 and cRows=1.  S_OK")
	TEST_VARIATION(4, 		L"Delete the second row.  Pass an array of 3 bookmarks: bookmark to the second row, to the first row, and to the second row.  DB_")
	TEST_VARIATION(5, 		L"Get the 3rd row handle.  GetRowsAt.  Delete the 3rd row.  GetNextRows")
	TEST_VARIATION(6, 		L"Position on 5th row.  Delete 3rd row.  Position on 3rd row.  delete 5th row.  Position on 5th row.  Fetch from the 5th row")
	TEST_VARIATION(7, 		L"Delete the 1st row.  GetRowsAt based on standard bookmarks.")
	TEST_VARIATION(8, 		L"Delete the last row.  Fetch backwards one row handle.  Fetch backwards again.  GetRowsAt based on stand")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RemoveDeleted)
//--------------------------------------------------------------------
// @class test DBPROP_REMOVEDELETED
//
class RemoveDeleted : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RemoveDeleted,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember *pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned.
	int Variation_1();
	// @cmember Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET
	int Variation_2();
	// @cmember Delete the 4th row. *pBookmark points to the 3th row. lRowOffset=1 and cRows=1.  S_OK is returned and the 5th row handle is ret
	int Variation_3();
	// @cmember Delete the first row.  Pass an array of bookmarks: bookmark to the first row and the second row.
	int Variation_4();
	// @cmember Delete the third row.  Pass an array of two bookmarks: bookmark to the second and 4th row
	int Variation_5();
	// @cmember Delete 2nd row.  pBookmark points to 4th row.  cRows=-2, lOffset=-1.  Verify 3rd and 1st row
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(RemoveDeleted)
#define THE_CLASS RemoveDeleted
BEG_TEST_CASE(RemoveDeleted, TCIRowsetLocate, L"test DBPROP_REMOVEDELETED")
	TEST_VARIATION(1, 		L"*pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned.")
	TEST_VARIATION(2, 		L"Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET")
	TEST_VARIATION(3, 		L"Delete the 4th row. *pBookmark points to the 3th row. lRowOffset=1 and cRows=1.  S_OK is returned and the 5th row handle is ret")
	TEST_VARIATION(4, 		L"Delete the first row.  Pass an array of bookmarks: bookmark to the first row and the second row.")
	TEST_VARIATION(5, 		L"Delete the third row.  Pass an array of two bookmarks: bookmark to the second and 4th row")
	TEST_VARIATION(6, 		L"Delete 2nd row.  pBookmark points to 4th row.  cRows=-2, lOffset=-1.  Verify 3rd and 1st row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BookmarkSkipped)
//--------------------------------------------------------------------
// @class Test DBPROP_BOOKMARKSKIPPPED
//
class BookmarkSkipped : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BookmarkSkipped,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=5.  DB_S_BOOKMARKSKIPPED
	int Variation_1();
	// @cmember Delete the last row.  *pBookmark=DBBML_LAST and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED
	int Variation_2();
	// @cmember Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED
	int Variation_3();
	// @cmember Delete the second row.  *pBookmark points to the second row and lRowOffset=1 and cRows=1.  DB_S_BOOKMARKSKIPPED
	int Variation_4();
	// @cmember Delete the 5th row.  Pass an array of two bookmarks: the bookmark to the 4th and 5th row. DB_S_ERRORSOCCURED
	int Variation_5();
	// @cmember Delete third row. *pBookmark=3rd row, lRowsOffset=-1, cRows=1, DB_S_BOOKMARKSKIPPED
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(BookmarkSkipped)
#define THE_CLASS BookmarkSkipped
BEG_TEST_CASE(BookmarkSkipped, TCIRowsetLocate, L"Test DBPROP_BOOKMARKSKIPPPED")
	TEST_VARIATION(1, 		L"Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=5.  DB_S_BOOKMARKSKIPPED")
	TEST_VARIATION(2, 		L"Delete the last row.  *pBookmark=DBBML_LAST and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED")
	TEST_VARIATION(3, 		L"Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED")
	TEST_VARIATION(4, 		L"Delete the second row.  *pBookmark points to the second row and lRowOffset=1 and cRows=1.  DB_S_BOOKMARKSKIPPED")
	TEST_VARIATION(5, 		L"Delete the 5th row.  Pass an array of two bookmarks: the bookmark to the 4th and 5th row. DB_S_ERRORSOCCURED")
	TEST_VARIATION(6, 		L"Delete third row. *pBookmark=3rd row, lRowsOffset=-1, cRows=1, DB_S_BOOKMARKSKIPPED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RemoveDeleted_BookmarkSkipped)
//--------------------------------------------------------------------
// @class Test DBPROP_REMOVEDELETED + DBPROP_BOOKMARKSKIPPED
//
class RemoveDeleted_BookmarkSkipped : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RemoveDeleted_BookmarkSkipped,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=0 and cRows=4.
	int Variation_1();
	// @cmember Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=1 and cRows=1.
	int Variation_2();
	// @cmember Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET is returned and the 4th a
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(RemoveDeleted_BookmarkSkipped)
#define THE_CLASS RemoveDeleted_BookmarkSkipped
BEG_TEST_CASE(RemoveDeleted_BookmarkSkipped, TCIRowsetLocate, L"Test DBPROP_REMOVEDELETED + DBPROP_BOOKMARKSKIPPED")
	TEST_VARIATION(1, 		L"Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=0 and cRows=4.")
	TEST_VARIATION(2, 		L"Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=1 and cRows=1.")
	TEST_VARIATION(3, 		L"Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET is returned and the 4th a")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ChangeRows)
//--------------------------------------------------------------------
// @class change rows in the rowset
//
class ChangeRows : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	IRowsetChange	*m_pIRowsetChange;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ChangeRows,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Change the first row and the 3rd row.  *pBookmark=DBBMK_FISRT and lRowOffset=1 and cRow=1.  The 3rd row is retrieved.
	int Variation_1();
	// @cmember Change the second row.  *pBookmark points to the second row.  lRowOffset=0 and cRow=2.  The 3rd and 4th row handles are retriev
	int Variation_2();
	// @cmember Change the 1st and the last row.  Pass an array of 3 bookmarks: bookmark to the first row, to the last row, and bookmark to the
	int Variation_3();
	// @cmember Change the second row.  Pass an array of 2 bookmarks: invalid and bookmark to the second row.  DB_S_ERRORSOCCURRED
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ChangeRows)
#define THE_CLASS ChangeRows
BEG_TEST_CASE(ChangeRows, TCIRowsetLocate, L"change rows in the rowset")
	TEST_VARIATION(1, 		L"Change the first row and the 3rd row.  *pBookmark=DBBMK_FISRT and lRowOffset=1 and cRow=1.  The 3rd row is retrieved.")
	TEST_VARIATION(2, 		L"Change the second row.  *pBookmark points to the second row.  lRowOffset=0 and cRow=2.  The 3rd and 4th row handles are retriev")
	TEST_VARIATION(3, 		L"Change the 1st and the last row.  Pass an array of 3 bookmarks: bookmark to the first row, to the last row, and bookmark to the")
	TEST_VARIATION(4, 		L"Change the second row.  Pass an array of 2 bookmarks: invalid and bookmark to the second row.  DB_S_ERRORSOCCURRED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Empty_Rowset)
//*-----------------------------------------------------------------------
// @class Test empty rowset cases
//
class Empty_Rowset : public TCIRowsetLocate { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Empty_Rowset,TCIRowsetLocate);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember GetRowsAt forward with DBMK_FIRST
	int Variation_1();
	// @cmember GetRowsAt backward with DBMK_LAST
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Empty_Rowset)
#define THE_CLASS Empty_Rowset
BEG_TEST_CASE(Empty_Rowset, TCIRowsetLocate, L"Test empty rowset cases")
	TEST_VARIATION(1, 		L"GetRowsAt forward with DBMK_FIRST")
	TEST_VARIATION(2, 		L"GetRowsAt backward with DBMK_LAST")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END
 

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(29, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, BOOKMARKS)
	TEST_CASE(2, OrderedBookmarks)
	TEST_CASE(3, OrderedBookmarks_Fetch_Scroll)
	TEST_CASE(4, LiteralBookmarks)
	TEST_CASE(5, Literal_Ordered_CanHoldRows)
	TEST_CASE(6, ScrollBackwards)
	TEST_CASE(7, FetchBackwards)
	TEST_CASE(8, Scroll_Fetch)
	TEST_CASE(9, Scroll_Fetch_CanHoldRows)
	TEST_CASE(10, CanHoldRows)
	TEST_CASE(11, MaxOpenRows)
	TEST_CASE(12, Related_IRowset)
	TEST_CASE(13, Rowset_SingleRow)
	TEST_CASE(14, Consistency)
	TEST_CASE(15, Parameters)
	TEST_CASE(16, Boundary_Compare)
	TEST_CASE(17, Boundary_GetRowsAt)
	TEST_CASE(18, Boundary_GetRowsByBookmarks)
	TEST_CASE(19, Boundary_Hash)
	TEST_CASE(20, Zombie)
	TEST_CASE(21, Rowset_BigRowset)
	TEST_CASE(22, keysetCursor)
	TEST_CASE(23, ExtendedErrors)
	TEST_CASE(24, DeleteRows)
	TEST_CASE(25, RemoveDeleted)
	TEST_CASE(26, BookmarkSkipped)
	TEST_CASE(27, RemoveDeleted_BookmarkSkipped)
	TEST_CASE(28, ChangeRows)
	TEST_CASE(29, Empty_Rowset)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(BOOKMARKS)
//*-----------------------------------------------------------------------
//| Test Case:		BOOKMARKS - test DBPROP_BOOKMARKS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BOOKMARKS::Init()
{
	BOOL fTestPass = FALSE;
	IRowsetInfo *pIRowsetInfo=NULL;
	DBPROPIDSET DBPropIDSet;
	DBPROPID DBPropID=DBPROP_CANHOLDROWS;
	ULONG cProperty = 0;
	DBPROPSET *pDBPropSet = NULL;

	DBPropIDSet.rgPropertyIDs=&DBPropID;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate));

	if(!VerifyInterface(m_pIRowsetLocate,IID_IRowsetInfo,
				ROWSET_INTERFACE,(IUnknown **)&pIRowsetInfo))
		goto CLEANUP;

	if(!CHECK(pIRowsetInfo->GetProperties(1,&DBPropIDSet, &cProperty, &pDBPropSet), S_OK))
		goto CLEANUP;

	//check whether DBPROP_CANHOLDROWS is on
	if(V_BOOL(&(pDBPropSet->rgProperties[0].vValue))==VARIANT_TRUE)
		m_fHoldRows=TRUE;
	else
		m_fHoldRows=FALSE;
	
	fTestPass = TRUE;

CLEANUP:

	FreeProperties(&cProperty, &pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Compare return DBCOMPARE_NE for different rows and Hash return different hash value.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_1()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE *			rgpBookmarks[2] = {NULL, NULL};
	DBCOMPARE		dwComparison = DBCOMPARE_EQ;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass = FALSE;

	//get the bookmark for the 5th row
	if(!GetBookmark(5,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 3rd row 
	if(!GetBookmark(3,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//Compare return DBCOMPARE_NE for the different row
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	// don't supported ordered bookmarks
	if(GetProp(DBPROP_ORDEREDBOOKMARKS))
	{
		if(!COMPARE(dwComparison, DBCOMPARE_GT))
			goto CLEANUP;
	}
	else
	{	// temp table
		if(!COMPARE(dwComparison, DBCOMPARE_NE))
			goto CLEANUP;
	}

	//Hash returns different values
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(rgHashedValues[0]!=rgHashedValues[1])
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_2()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE *			rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass=FALSE;

	//get the bookmark for the middle row
	if(!GetBookmark(ULONG(g_lRowLast/2),&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the middle row again
	if(!GetBookmark(ULONG(g_lRowLast/2),&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//Compare return DBCOMPARE_EQ for the same row
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Hash returns same values
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(rgHashedValues[0], rgHashedValues[1]))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST and lRowsOffset=# of rows in the rowset. DB_S_ENDOFROWSET is returned and no row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_3()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW *		pHRow=hRow;
	HRESULT		hr;

	if(!CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,g_lRowLast,
		1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
	{
		if(hr==ResultFromScode(S_OK))
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);

		return TEST_FAIL;
	}

	//no row should be retrieved
	if(!COMPARE(cRowsObtained, 0))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST and IRowsetOffset=1. DB_S_ENDOFROWSET is returned and no row handles is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW *		pHRow=hRow;
	HRESULT		hr;

	if(!CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,
		1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
	{
		if(hr==ResultFromScode(S_OK))
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);

		return TEST_FAIL;
	}

	//no row should be retrieved
	if(!COMPARE(cRowsObtained, 0))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST and IRowsOffset=# of rows in the rowset-1.  cRows=2.  DB_S_ENDOFROWS is returned and the last row handl
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_5()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		hRow[2];
	HROW *		pHRow=hRow;
	HRESULT		hr;
	BOOL		fTestPass=FALSE;

	// *pBookmark points to the first row and IRowsOffset=# of rows in the rowset-1
	if(!CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,g_lRowLast-1,
		2,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
	{
		if(hr==ResultFromScode(S_OK))
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
		else
			CHECK(hr,S_OK);

		return TEST_FAIL;
	}

	//the last row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(hRow[0],g_lRowLast,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,hRow,NULL,NULL,NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST and IRowsetOffset==0.  cRows=1.  S_OK is returned and the last row handle is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_6()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW *		pHRow=hRow;

	// *pBookmark=DBBMK_LAST and IRowsetOffset==0.  cRows=1. 	
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,
		1,&cRowsObtained,&pHRow),S_OK))
		return TEST_FAIL;

	//the last row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		return TEST_FAIL;

	if(!CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK))
		return TEST_FAIL;

	//repeat 	
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,
		1,&cRowsObtained,&pHRow),S_OK))
		return TEST_FAIL;

	//the last row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		return TEST_FAIL;

	COMPARE(VerifyRowPosition(hRow[0],g_lRowLast,g_pCTable),TRUE);

	if(!CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_7()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;		
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW *		pHRow=hRow;
	DBCOUNTITEM	cRowCount;

	//get one row handle at a time, from the first row
	for(cRowCount=0; cRowCount<ULONG(g_lRowLast); cRowCount++)
	{
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,cRowCount,
			1,&cRowsObtained,&pHRow),S_OK))
			return TEST_FAIL;

		//verify the row position
		if(!COMPARE(VerifyRowPosition(hRow[0],cRowCount+1,g_pCTable),TRUE))
		{
			CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
			return TEST_FAIL;
		}
		else
			CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
	}

	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of bookmarks, one bookmark for each row handle.  S_OK and all the row handles in the rowset should be r
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_8()
{
	ULONG_PTR *	rgcbBookmarks;
	BYTE **		rgpBookmarks;
	HROW *		rgHRow;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass=FALSE;

	rgcbBookmarks = (ULONG_PTR *)PROVIDER_ALLOC(sizeof(ULONG_PTR)*g_lRowLast);
	rgpBookmarks = (BYTE **)PROVIDER_ALLOC(sizeof(BYTE *)*g_lRowLast);
	rgHRow = (HROW *)PROVIDER_ALLOC(sizeof(HROW)*g_lRowLast);

	//get bookmarks for each row in the row set
	for(cRowCount=0;cRowCount<ULONG(g_lRowLast);cRowCount++)
		//init the pointer
		rgpBookmarks[cRowCount]=NULL;

	for(cRowCount=0;cRowCount<ULONG(g_lRowLast);cRowCount++)
	{
		if(!GetBookmark(cRowCount+1,&rgcbBookmarks[cRowCount],&rgpBookmarks[cRowCount]))
			goto CLEANUP;
	}

	//get the row handles
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,g_lRowLast,rgcbBookmarks,
	(const BYTE **)rgpBookmarks,rgHRow,NULL),S_OK))
		goto CLEANUP;

	//make sure the row handle are retrieved correct
	for(cRowCount=0; cRowCount<ULONG(g_lRowLast); cRowCount++)
	{
		if(!COMPARE(VerifyRowPosition(rgHRow[cRowCount], cRowCount+1,g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:

	//release all the bookmark
	for(cRowCount=0;cRowCount<ULONG(g_lRowLast);cRowCount++)
		PROVIDER_FREE(rgpBookmarks[cRowCount]);

	//release all the rows
	CHECK(m_pIRowset->ReleaseRows(g_lRowLast,rgHRow,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(rgpBookmarks);
	PROVIDER_FREE(rgcbBookmarks);
	PROVIDER_FREE(rgHRow);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of two bookmarks that point to the same row.  S_OK and the row handle is retrieved twice.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_9()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	HROW			rgHRow[2];
	DBREFCOUNT		cRefCount;
	DBROWSTATUS		rgDBRowStatus[2];
	BOOL			fTestPass=FALSE;

	//get the bookmark for the 10th row
	if(!GetBookmark(ULONG_PTR(g_lRowLast/3),&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 10th row again
	if(!GetBookmark(ULONG_PTR(g_lRowLast/3),&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//get the row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHRow,rgDBRowStatus),S_OK))
		goto CLEANUP;

	if(!(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK) &&
		COMPARE(rgDBRowStatus[1], DBROWSTATUS_S_OK)))
		goto CLEANUP;

	//Release the row handle,  The reference count should be 1
	if(!CHECK(m_pIRowset->ReleaseRows(1,rgHRow,NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	//If the HRows are the same ref count should be 1 after release
	if(!COMPARE(cRefCount,ULONG_PTR(((rgHRow[0] == rgHRow[1]) ? 1 : 0))))
		goto CLEANUP;

	//release again
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(rgHRow[1]),NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the first row, DB_E_ROWSNOTRELEASED should be returned.  R
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BOOKMARKS::Variation_10()
{
	ULONG_PTR		cbFirstBookmark;
	ULONG_PTR		cbSecondBookmark;
	BYTE			*pFirstBookmark=NULL;
	BYTE			*pSecondBookmark=NULL;
	HROW			FirstHRow=NULL;
	HROW			SecondHRow=NULL;
	DBROWSTATUS		DBRowStatus;
	BOOL			fTestPass=FALSE;

	//get the bookmark for the first row
	if(!GetBookmark(1, &cbFirstBookmark, &pFirstBookmark))
		return TEST_FAIL;

	//get the bookmark for the second row
	if(!GetBookmark(2, &cbSecondBookmark, &pSecondBookmark))
		goto CLEANUP;

	//retrieve the first row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbFirstBookmark,
		(const BYTE **)&pFirstBookmark,&FirstHRow,
		&DBRowStatus),S_OK))
		goto CLEANUP;

	COMPARE(DBRowStatus, DBROWSTATUS_S_OK);

	if(!m_fHoldRows)
	{
		//try to retrieve the second row handle
		if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbSecondBookmark,
		(const BYTE **)&pSecondBookmark,&SecondHRow,
		NULL),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
		
		//SecondHRow should be DB_NULL_HROW
		COMPARE(SecondHRow, DB_NULL_HROW);
	}
	else
	{
		//try to retrieve the second row handle
		if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbSecondBookmark,
		(const BYTE **)&pSecondBookmark,&SecondHRow,
		NULL),S_OK))
			goto CLEANUP;
	}

	//release the first row handle
	CHECK(m_pIRowset->ReleaseRows(1,&FirstHRow,NULL,NULL,NULL),S_OK);
	FirstHRow=NULL;

	//try to retrieve the second row handle again
	if(CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbSecondBookmark,
	(const BYTE **)&pSecondBookmark,&SecondHRow,
	NULL),S_OK))
		fTestPass=TRUE;


CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pFirstBookmark);
	PROVIDER_FREE(pSecondBookmark);

	//release the row handle
	if(FirstHRow)
		CHECK(m_pIRowset->ReleaseRows(1,&FirstHRow,NULL,NULL,NULL),S_OK);

	if(SecondHRow)
		CHECK(m_pIRowset->ReleaseRows(1,&SecondHRow,NULL,NULL,NULL),S_OK);

	if(fTestPass)
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
BOOL BOOKMARKS::Terminate()
{
	ReleaseRowsetAndAccessor();
		
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(OrderedBookmarks)
//*-----------------------------------------------------------------------
//| Test Case:		OrderedBookmarks - test DBPROP_ORDEREDBOOKMARKS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL OrderedBookmarks::Init()
{
	BOOL fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset by a join statement;create an accessor.  
	//DBPROP_ORDEREDBOOKMARKS is requested and 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetLocate));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Compare return DBCOMPARE_EQ for the same row and Hash return the same hash value.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OrderedBookmarks::Variation_1()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass=FALSE;

	//get the bookmark for the 40th row
	if(!GetBookmark(g_lRowLast-2,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 40th row again
	if(!GetBookmark(g_lRowLast-2,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//Compare return DBCOMPARE_EQ for the same row
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Hash returns same values
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(rgHashedValues[0], rgHashedValues[1]))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc The first bookmark is after the second. Compare return DBCOMPARE_GT for the row and Hash return a bigger hash value for the fir
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OrderedBookmarks::Variation_2()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass=FALSE;

	//get the bookmark for the 5th row
	if(!GetBookmark(5,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 2nd row again
	if(!GetBookmark(2,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//Compare return DBCOMPARE_GT for the same row
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(GetProp(DBPROP_ORDEREDBOOKMARKS))
	{

		if(!COMPARE(dwComparison, DBCOMPARE_GT))
			goto CLEANUP;

		//Hash returns a bigger value for the first row handle
		if(CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
			fTestPass=TRUE;

		//hash values are not necessary in this order
		//if(rgHashedValues[0] > rgHashedValues[1])
		//	fTestPass=TRUE;
	}
	else
	{
		if(COMPARE(dwComparison, DBCOMPARE_NE))
			fTestPass=TRUE;

	}

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL OrderedBookmarks::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(OrderedBookmarks_Fetch_Scroll)
//*-----------------------------------------------------------------------
//| Test Case:		OrderedBookmarks_Fetch_Scroll - DBPROP_ORDEREDBOOKMARKS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANSCROLLBACKWARDS
//|					DBPROP_ORDEREDBOOKMARKS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANSCROLLBACKWARDS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL OrderedBookmarks_Fetch_Scroll::Init()
{
	BOOL		fTestPass = FALSE;
	DBPROPID	guidPropertySet[3];
	ULONG		cPrptSet=0;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;	
	guidPropertySet[cPrptSet++]=DBPROP_ORDEREDBOOKMARKS;
	
	//create a rowset by a join statement; create an accessor.  
	//DBPROP_ORDEREDBOOKMARKS,DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The first bookmark is before the second.  Compare return for the row and Hash return a smaller hash value for the first bookmar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OrderedBookmarks_Fetch_Scroll::Variation_1()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];	
	DBCOUNTITEM		cRowsObtained;
	HROW			*pHRow=NULL;
	BOOL			fTestPass=FALSE;

	//get the bookmark for the 3th row
	if(!GetBookmark(3,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 4th row 
	if(!GetBookmark(4,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//Compare return DBCOMPARE_LT
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_LT))
		goto CLEANUP;

	//Hash returns a smaller value for the first row handle
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	//make sure we can retrieve the 3rd row handle using a Ordered Bookmark
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,rgcbBookmarks[0],
		rgpBookmarks[0],0,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	if(COMPARE(VerifyRowPosition(*pHRow,3,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	//free the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL OrderedBookmarks_Fetch_Scroll::Terminate()
{

	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(LiteralBookmarks)
//*-----------------------------------------------------------------------
//| Test Case:		LiteralBookmarks - test DBPROP_LITERALBOOKMARKS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL LiteralBookmarks::Init()
{
	DBPROPID		guidPropertySet=DBPROP_LITERALBOOKMARKS;
	BOOL			fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset; create an accessor.  
	//DBPROP_LITERALBOOKMARKS is requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1, &guidPropertySet));

	if(GetProp(DBPROP_ORDEREDBOOKMARKS))
		m_fOrderedBookmark=TRUE;

	fTestPass = TRUE;

CLEANUP:

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Compare the bookmarks for the different row directly, based on the data type of the bookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralBookmarks::Variation_1()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	DBHASHVALUE		rgHashedValues[2];	
	DBCOUNTITEM		cRowsObtained;
	HROW			*pHRow=NULL;
	BOOL			fTestPass=FALSE;

	//get the bookmark a row near the end
	if(!GetBookmark(g_lRowLast-4,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark very close to the end
	if(!GetBookmark(g_lRowLast-2,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//compare the two bookmarks literally
	if(!CompareLiteralBookmark(rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1], &dwComparison))
		goto CLEANUP;

	if(dwComparison==DBCOMPARE_EQ)
		goto CLEANUP;

	//reset dwComparison
	dwComparison=DBCOMPARE_EQ;

	//Compare return DBCOMPARE_NE for the different row
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if ( m_fOrderedBookmark )
	{
		if(!COMPARE(dwComparison, DBCOMPARE_LT))
			goto CLEANUP;
	}
	else
	{
		if(!COMPARE(dwComparison, DBCOMPARE_NE))
			goto CLEANUP;
	}

	//Hash returns  different value
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(rgHashedValues[0] == rgHashedValues[1])
		goto CLEANUP;

	//make sure we can retrieve the row handle using a Ordered Bookmark
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,rgcbBookmarks[0],
		rgpBookmarks[0],0,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	if(COMPARE(VerifyRowPosition(*pHRow,g_lRowLast-4,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	//free the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Compare the bookmarks for the same row directly, based on the data type of the bookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int LiteralBookmarks::Variation_2()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];	
	BOOL			fTestPass=FALSE;

	//get the bookmark for the 4th row
	if(!GetBookmark(4,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 4th row again
	if(!GetBookmark(4,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//compare the two bookmarks literally
	if(!CompareLiteralBookmark(rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1], &dwComparison))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//reset dwComparison
	dwComparison=DBCOMPARE_NE;

	//Compare return DBCOMPARE_EQ
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Hash returns same values
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(rgHashedValues[0], rgHashedValues[1]))
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL LiteralBookmarks::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Literal_Ordered_CanHoldRows)
//*-----------------------------------------------------------------------
//| Test Case:		Literal_Ordered_CanHoldRows - Test DBPROP_LITERALBOOKMARKS + DBPROP_ORDEREDBOOKMARKS + DBPROP_CANHOLDROWS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Literal_Ordered_CanHoldRows::Init()
{
	DBPROPID	guidPropertySet[3];
	ULONG		cPrptSet=0;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	guidPropertySet[cPrptSet++]=DBPROP_LITERALBOOKMARKS;	
	guidPropertySet[cPrptSet++]=DBPROP_CANHOLDROWS;
	guidPropertySet[cPrptSet++]=DBPROP_ORDEREDBOOKMARKS;
	
	//create a rowset;  create an accessor.  
	//DBPROP_ORDEREDBOOKMARKS,DBPROP_LITERALBOOKMARKS and DBPROP_CANHOLDROWS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ABCANDCOLLIST, IID_IRowsetLocate,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The first bookmark is after the second.  Compare the bookmarks directly, based on the data type of the bookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Literal_Ordered_CanHoldRows::Variation_1()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	DBHASHVALUE		rgHashedValues[2];
	
	BOOL			fTestPass=FALSE;

	//get the bookmark for the middle row
	if(!GetBookmark(g_lRowLast/2,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for an earlier row again
	if(!GetBookmark(g_lRowLast/3,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//compare the two bookmarks literally
	if(!CompareLiteralBookmark(rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1], &dwComparison))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_GT))
		goto CLEANUP;

	//reset dwComparison
	dwComparison=DBCOMPARE_EQ;

	//Compare return DBCOMPARE_GT
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_GT))
		goto CLEANUP;

	//Hash returns bigger value for the first bookmark
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(rgHashedValues[0] > rgHashedValues[1])
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc The first bookmark is before the second.  Compare the bookmarks directly, based on the data type of the bookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Literal_Ordered_CanHoldRows::Variation_2()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	DBHASHVALUE		rgHashedValues[2];
	
	BOOL			fTestPass=FALSE;

	//get the bookmark for a row
	if(!GetBookmark(g_lRowLast/4,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for a later row 
	if(!GetBookmark(g_lRowLast/2,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//compare the two bookmarks literally
	if(!CompareLiteralBookmark(rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1], &dwComparison))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_LT))
		goto CLEANUP;

	//reset dwComparison
	dwComparison=DBCOMPARE_EQ;

	//Compare return DBCOMPARE_LT
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_LT))
		goto CLEANUP;

	//Hash returns smaller value for the first bookmark
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc The first bookmark is the same as the second.  Compare the bookmarks directly, based on the data type of the bookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Literal_Ordered_CanHoldRows::Variation_3()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_NE;
	DBHASHVALUE		rgHashedValues[2];	
	BOOL			fTestPass=FALSE;

	//get a bookmark 
	if(!GetBookmark(g_lRowLast-5,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get same bookmark again
	if(!GetBookmark(g_lRowLast-5,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//compare the two bookmarks literally
	if(!CompareLiteralBookmark(rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1], &dwComparison))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//reset dwComparison
	dwComparison=DBCOMPARE_NE;

	//Compare return DBCOMPARE_EQ
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Hash returns same value
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(rgHashedValues[0], rgHashedValues[1]))
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Literal_Ordered_CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ScrollBackwards)
//*-----------------------------------------------------------------------
//| Test Case:		ScrollBackwards - Test DBPROP_CANSCROLLBACKWARDS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ScrollBackwards::Init()
{
	DBPROPID	guidPropertySet;
	ULONG		cPrptSet=0;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and an accessor.  
	//DBPROP_CANSCROLLBACKWARDS is requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,&guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST and lRowsetOffset=-1.  DB_S_ENDOFROWSET is returned and no row handles is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=5;
	HROW		*pHRow=NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL, NULL, 1, pBookmark, -1,
		1,&cRowsObtained, &pHRow),DB_S_ENDOFROWSET))
		return TEST_FAIL;

	if(!COMPARE(cRowsObtained, 0))
		return TEST_FAIL;

	if(!COMPARE(pHRow,NULL))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST and lRowsetOffset=-(# of rows in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBROWOFFSET	lRowOffset=g_lRowLast;
	DBCOUNTITEM	cRowsObtained;
	HROW		*pHRow=NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL, NULL, 1, pBookmark, -lRowOffset,
		3,&cRowsObtained, &pHRow),DB_S_ENDOFROWSET))
		return TEST_FAIL;

	if(!COMPARE(cRowsObtained, 0))
		return TEST_FAIL;

	if(!COMPARE(pHRow,NULL))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark is the bookmark on the second row of the rowset.  lRowsetOffset=-1, cRows=# of rows in the rowset.  S_OK is returned
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_3()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass=FALSE;

	//pBookmark is the bookmark on the second row of the rowset.
	if(!GetBookmark(2,&cbBookmark,&pBookmark))
		goto CLEANUP;

	 // lRowsetOffset=-1, cRows=# of rows in the rowset
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
		-1,g_lRowLast,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;
	
	if(!COMPARE(cRowsObtained, ULONG(g_lRowLast)))
		goto CLEANUP;

	//make sure the row handles are returned in the traversal order
	for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
	{
		if(!COMPARE(VerifyRowPosition(pHRow[cRowCount],cRowCount+1,g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ScrollBackwards::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(FetchBackwards)
//*-----------------------------------------------------------------------
//| Test Case:		FetchBackwards - test DBPROP_CANFETCHBACKWARDS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL FetchBackwards::Init()
{
	DBPROPID	guidPropertySet=DBPROP_CANFETCHBACKWARDS;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;
	
	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS is requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1,&guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST and lRowsetOffset==1.  cRows=-3.  DB_S_ENDOFROWSET is returned and only two row handle is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_1()
{

	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;


	 // *pBookmark=DBBMK_FIRST and lRowsetOffset==1.  cRows=-3.
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		1,-3,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;
	
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//make sure the row handles are returned in the order of 2nd row and 1st row
	if(!COMPARE(VerifyRowPosition(*pHRow, 2,g_pCTable),TRUE))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(pHRow[1],1,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST and lRowsetOffset==0. cRows=-6.  DB_S_ENDOFROWSET is returned and only 5 row handles are retrieved.  Ver
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_2()
{

	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow = NULL;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass=FALSE;

	pHRow = (HROW *)PROVIDER_ALLOC(sizeof(HROW)*g_lRowLast);

	 // *pBookmark=DBBMK_LAST and lRowsetOffset==0. 
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		0,-(g_lRowLast+1),&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;
	
	if(!COMPARE(cRowsObtained, ULONG(g_lRowLast)))
		goto CLEANUP;

	//make sure the row handles are returned in the traversal order
	for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
	{
		if(!COMPARE(VerifyRowPosition(pHRow[cRowCount],cRowsObtained-cRowCount,g_pCTable),TRUE))
			goto CLEANUP;
	}
		
	fTestPass=TRUE;

CLEANUP:

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow,NULL,NULL, NULL),S_OK);

	PROVIDER_FREE(pHRow);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark is the bookmark on the 4th row.  lRowsetOffset=1 and cRows=-2.  S_OK is returned and the 5th and the 4th row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_3()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get the bookmark for the 4th row
	if(!GetBookmark(4,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//lRowsetOffset=1 and cRows=-2
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,-2,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//the 5th and 4th row handles should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow,5,g_pCTable),TRUE))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(pHRow[1],4,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST.  Get one row handle at a time (lRowsetOffset==0 and cRows==-1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	DBROWCOUNT	cRowCount=0;
	BOOL		fTestPass=TRUE;

	while(fTestPass && (cRowCount<g_lRowLast))
	{
		//get the row handle
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,-cRowCount,
			1,&cRowsObtained,&pHRow),S_OK))
			fTestPass=FALSE;

		if(fTestPass)
		{
			if(!COMPARE(VerifyRowPosition(*pHRow, g_lRowLast-cRowCount,g_pCTable),TRUE))
				fTestPass=FALSE;
		}

		//release the row handle
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;

		cRowCount++;
	}

	if(fTestPass)
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
BOOL FetchBackwards::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_Fetch)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_Fetch - test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch::Init()
{
	DBPROPID	guidPropertySet[2];
	ULONG		cPrptSet=0;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST and lRowOffset=-2.  cRows=-3.  The third, 4th, and 5th row handles are retrieved.  *pBoolmark=DBBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained;
	HROW			*pHRow=NULL;
	BOOL			fTestPass=FALSE;

	// *pBookmark=DBBMK_LAST and lRowOffset=2-g_lRowLast.  cRows=-3
	//The third, 2nd, and 1st row handles are retrieved
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,3-g_lRowLast,-3,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 3))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(pHRow[0],3,g_pCTable),TRUE))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(pHRow[1],2,g_pCTable),TRUE))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(pHRow[2],1,g_pCTable),TRUE))
		goto CLEANUP;


	// *pBookmark=DBBMK_FIRST, lRowOffset=1 and cRows=5.  Return DB_E_ROWSNOTRELEASED
	DBBookmark=DBBMK_FIRST;
	
	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,5,
			&cRowsObtained,&pHRow),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	}

	//release the row handles
	CHECK(m_pIRowset->ReleaseRows(3, pHRow,NULL, NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark=DBBMK_FIRST, lRowOffset=1 and cRows=g_lRowLast.  Return DB_S_ENDOFROWSET
	DBBookmark=DBBMK_FIRST;
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,g_lRowLast,
		&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//make sure all but one row is retrieved and the 1st row retrieved is the 2nd row
	if(!COMPARE(cRowsObtained, ULONG(g_lRowLast-1)))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(pHRow[0],2,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow,NULL,NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	
	return fTestPass;
}


// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRows == LONG_MIN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scroll_Fetch::Variation_2()
{ 
	ULONG_PTR	cbBookmark;
	BYTE*		pBookmark = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		pHRow = NULL;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass = FALSE;

	//Get the last bookmark
	TESTC(GetBookmark(g_lRowLast,&cbBookmark,&pBookmark));

	 // cRows == LONG_MIN, lRowsOffset = 0
	TEST2C_(m_hr = m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
					0,LONG_MIN,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET,E_OUTOFMEMORY);

	//make sure the row handles are returned in reverse order
	if (m_hr == DB_S_ENDOFROWSET)
	{
		TESTC(cRowsObtained == ULONG(g_lRowLast));
		for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
			TESTC(VerifyRowPosition(pHRow[cRowCount],g_lRowLast-cRowCount,g_pCTable));
	}

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc lRowsOffset == LONG_MIN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scroll_Fetch::Variation_3()
{ 
	ULONG_PTR	cbBookmark;
	BYTE*		pBookmark = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		pHRow = NULL;
	BOOL		fTestPass = FALSE;

	//Get the last bookmark
	TESTC(GetBookmark(g_lRowLast,&cbBookmark,&pBookmark));

	 // cRows == 1, lRowsOffset == LONG_MIN
	TEST2C_(m_hr = m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
					LONG_MIN,1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET,E_OUTOFMEMORY);

	TESTC(cRowsObtained == 0);
	TESTC(pHRow == NULL);
	
	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//release the rowhandle
	if (pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRows = LONG_MAX/2 + 1
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scroll_Fetch::Variation_4()
{ 
	ULONG_PTR	cbBookmark;
	BYTE *		pBookmark = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		pHRow = NULL;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass = FALSE;

	//Get the first bookmark
	TESTC(GetBookmark(1,&cbBookmark,&pBookmark));

	 // cRows == LONG_MAX/2 +1 == 1073741824, lRowsOffset = 0
	TEST2C_(m_hr = m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
					0,MAXDBROWCOUNT/2+1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET,E_OUTOFMEMORY);

	//make sure the row handles are returned in reverse order
	if (m_hr == DB_S_ENDOFROWSET)
	{
		TESTC(cRowsObtained == ULONG_PTR(g_lRowLast));
		for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
			TESTC(VerifyRowPosition(pHRow[cRowCount], cRowCount+1, g_pCTable));
	}

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_Fetch_CanHoldRows)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_Fetch_CanHoldRows - Test DBPROP_CANSCROLLBACKWARDS + DBPROP_CANFETCHBACKWARDS + DBPROP_CANHOLDROWS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_CanHoldRows::Init()
{
	DBPROPID	guidPropertySet[3];
	ULONG		cPrptSet=0;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANHOLDROWS;
	
	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS  and DBPROP_CANHOLDROWS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_CanHoldRows::Variation_1()
{
	ULONG_PTR	cbBookmark=0;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the second row
	if(!GetBookmark(2,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark, -1, -1,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the first row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_CanHoldRows::Variation_2()
{
	ULONG_PTR	cbBookmark=0;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the third row
	if(!GetBookmark(3,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-3
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark, -1, -3,
		&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//two rows should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//the second row should be retrieved first
	if(!COMPARE(VerifyRowPosition(*pHRow, 2, g_pCTable),TRUE))
		goto CLEANUP;

	//the first row should be retrieved 
	if(COMPARE(VerifyRowPosition(pHRow[1], 1, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.  *pBookmar
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_CanHoldRows::Variation_3()
{
	ULONG_PTR	cbFirstBookmark=0, cbSecondBookmark=0;
	BYTE		*pFirstBookmark=NULL, *pSecondBookmark=NULL;
	DBCOUNTITEM	cFirstRowsObtained=0, cSecondRowsObtained=0;
	HROW		*pFirstHRow=NULL;
	HROW		*pSecondHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the 4th row
	if(!GetBookmark(4,&cbFirstBookmark,&pFirstBookmark))
		goto CLEANUP;

	//get a bookmark for the 5th row
	if(!GetBookmark(5,&cbSecondBookmark,&pSecondBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-3
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbFirstBookmark,pFirstBookmark, -1, -3,
		&cFirstRowsObtained,&pFirstHRow),S_OK))
		goto CLEANUP;

	//three rows should be retrieved
	if(!COMPARE(cFirstRowsObtained, 3))
		goto CLEANUP;

	//the third row is the first row handle to retrieve
	if(!COMPARE(VerifyRowPosition(*pFirstHRow, 3, g_pCTable),TRUE))
		goto CLEANUP;

	// *pBookmark points to the 5th row.  lRowstOffset=-1 and cRows=-1.  
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbSecondBookmark,pSecondBookmark, 
		-1, -1, &cSecondRowsObtained,&pSecondHRow),S_OK))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(cSecondRowsObtained, 1))
		goto CLEANUP;

	//the 4th row handle should be retrieved
	if(COMPARE(VerifyRowPosition(*pSecondHRow, 4, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pFirstBookmark);
	PROVIDER_FREE(pSecondBookmark);

	//release the row handle
	if(pFirstHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cFirstRowsObtained,pFirstHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pFirstHRow);
	}

	//release the row handle
	if(pSecondHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cSecondRowsObtained,pSecondHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pSecondHRow);
	}

	return fTestPass;
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanHoldRows)
//*-----------------------------------------------------------------------
//| Test Case:		CanHoldRows - test DBPROP_CANHOLDROWS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Init()
{
	DBPROPID	guidProperty=DBPROP_CANHOLDROWS;
	BOOL		fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and  accessor
	//DBPROP_CANHOLDROWS 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTGROUPBY, IID_IRowsetLocate,
		1,&guidProperty));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Pass a bookmark to the second row.  Retrieve it.  Pass a bookmark to the third row, S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_1()
{
	ULONG_PTR	rgcbBookmarks[2];
	BYTE *		rgpBookmarks[2]={NULL,NULL};
	HROW		HRowSecond=NULL;
	HROW		HRowThird=NULL;
	HROW		rgHRowArray[2]={NULL,NULL};
	DBREFCOUNT	rgRefCounts[2];
	BOOL		fTestPass=FALSE;

	//get a bookmark for the seond row
	if(!GetBookmark(2, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;

	//get a bookmark for the third row
	if(!GetBookmark(3, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//get the second row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&rgcbBookmarks[0],
		(const BYTE **)&rgpBookmarks[0],&HRowSecond,NULL),S_OK))
		goto CLEANUP;
	
	//verify the row position
	if(!COMPARE(VerifyRowPosition(HRowSecond, 2, g_pCTable),TRUE))
		goto CLEANUP;

	//get the third row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&rgcbBookmarks[1],
		(const BYTE **)&rgpBookmarks[1],&HRowThird,NULL),S_OK))
		goto CLEANUP;

	//verify the row position
	if(!COMPARE(VerifyRowPosition(HRowThird, 3, g_pCTable),TRUE))
		goto CLEANUP;

	//retrieve the two row handles again
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,rgHRowArray,NULL),S_OK))
		goto CLEANUP;
	
	//the reference count of the row handles should be 2 before AddRef
	if(!CHECK(m_pIRowset->AddRefRows(2,rgHRowArray,rgRefCounts,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(rgRefCounts[0] >= 1, TRUE) && COMPARE(rgRefCounts[1] >= 1, TRUE))
		fTestPass=TRUE;

	//release the row handles as we have AddRefed them
	CHECK(m_pIRowset->ReleaseRows(2, rgHRowArray,NULL,NULL, NULL),S_OK);

CLEANUP:
	
	//release the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	//release the row handles
	if(HRowSecond)
		CHECK(m_pIRowset->ReleaseRows(1, &HRowSecond,NULL,NULL, NULL),S_OK);

	if(HRowThird)
		CHECK(m_pIRowset->ReleaseRows(1, &HRowThird, NULL,NULL, NULL),S_OK);

	if(rgHRowArray[0])
		CHECK(m_pIRowset->ReleaseRows(2, rgHRowArray, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MaxOpenRows)
//*-----------------------------------------------------------------------
//| Test Case:		MaxOpenRows - test DBPROP_MAXOPENROWS
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set MAXOPENROWS to 3.  *pBookmark points to the second row.  lRowsOffset==1 and cRows=4.  DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_1()
{
	//TODO This test should not be commented out!
	return TEST_PASS;

/*	
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	ULONG	cOpenRowCount=0;
	ULONG	cMaxOpenRowsCount;
	ULONG	cbBookmark;
	BYTE	*pBookmark=NULL;
	HROW	*pHRow=NULL;
	ULONG	cRowsObtained=0;
	BOOL	fTestPass=FALSE;

	//record the g_cMaxOpenRows
	cMaxOpenRowsCount=g_cMaxOpenRowsCount;

	//set max open rows to 3 
	g_cMaxOpenRowsCount=3;

	//open rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable5Rows, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1,&guidProperty))
		goto CLEANUP;

	//get bookmark to the second row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto CLEANUP;

	// *pBookmark points to the second row.  lRowsOffset==1, cRows==4 
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,4,
		&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//only three row handles should be retrieved
	if(!COMPARE(cRowsObtained, 3))
		goto CLEANUP;

	//the first row handle should be 3rd row
	if(COMPARE(VerifyRowPosition(*pHRow, 3, g_pCTable5Rows),TRUE))
		fTestPass=TRUE;


CLEANUP:
	
	 //release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}


	//restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;

	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;		 
*/
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetMAXOPENROWS to 2.  *pBookmark points to the third row.  lRowsOffset==0 and cRows=3.  DB_S_ROWLIMITEXCEEDED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_2()
{
	//TODO This test should not be commented out!

	return TEST_PASS;
/*
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	DBCOUNTITEM	cOpenRowCount=0;
	DBCOUNTITEM	cMaxOpenRowsCount;
	ULONG_PTR	cbBookmark;
	BYTE *		pBookmark=NULL;
	HROW *		pHRow=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;

	//record the g_cMaxOpenRows
	cMaxOpenRowsCount=g_cMaxOpenRowsCount;

	//set max open rows to 2 
	g_cMaxOpenRowsCount=2;

	//open rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable5Rows, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1,&guidProperty))
		goto CLEANUP;

	//get bookmark to the third row
	if(!GetBookmark(3, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//lRowsOffset==0 and cRows=3.
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,3,
		&cRowsObtained,&pHRow),DB_S_ROWLIMITEXCEEDED))
		goto CLEANUP;

	//only two row handles should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//the first row handle should be 3rd row
	if(!COMPARE(VerifyRowPosition(*pHRow, 3, g_pCTable5Rows),TRUE))
		goto CLEANUP;

	//the second row handle should be 4th row
	if(COMPARE(VerifyRowPosition(pHRow[1], 4, g_pCTable5Rows),TRUE))
		fTestPass=TRUE;;

CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}


	//restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;

	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;		
*/
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set MAXOPENROWS to 3.  Pass an array of 3 bookmarks to different rows.  S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_3()
{	
	//TODO This test should not be commented out!

	return TEST_PASS;
/*
	DBPROPID	guidProperty=DBPROP_MAXOPENROWS;
	ULONG_PTR	rgcbBookmarks[3];
	BYTE *		rgpBookmarks[3]={NULL,NULL,NULL};
	DBCOUNTITEM	cMaxOpenRowsCount=0;
	DBCOUNTITEM	cRowsObtained;
	HROW *		pHRow=NULL;
	DBCOUNTITEM	ulCount;
	BOOL		fTestPass=FALSE;


	//record the g_cMaxOpenRows
	cMaxOpenRowsCount=g_cMaxOpenRowsCount;

	//set max open rows to 3 
	g_cMaxOpenRowsCount=3;

	//open rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1,&guidProperty))
		goto CLEANUP;

	//an array of 3 bookmarks to different rows
	for(ulCount=0;ulCount<3;ulCount++)
	{
		if(!GetBookmark((ulCount+1)*10, &rgcbBookmarks[ulCount],&rgpBookmarks[ulCount]))
			goto CLEANUP;
	}

	//retrieve 3 row handles
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,&cRowsObtained,&pHRow,NULL),S_OK))
		goto CLEANUP;

	//three rows should be retrieved
	if(!COMPARE(cRowsObtained, 3))
		goto CLEANUP;

	//verify all the row handles
	for(ulCount=0;ulCount<3;ulCount++)
	{
		if(!COMPARE(VerifyRowPosition(pHRow[ulCount],(ulCount+1)*10,g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	for(ulCount=0; ulCount<3; ulCount++)
		PROVIDER_FREE(rgpBookmarks[ulCount]);

	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;

	//release the rowset
	ReleaseRowsetAndAccessor();


	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
*/
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set MAXOPENROWS to 2.  Pass an array of 4 bookmarks. DB_S_ROWLIMITEXCEEDED is returned  and only two row handles are retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_4()
{
	//TODO This test should not be commented out!

	return TEST_PASS;
/*
	DBPROPID		guidProperty=DBPROP_MAXOPENROWS;
	ULONG_PTR		rgcbBookmarks[4];
	BYTE			*rgpBookmarks[4]={NULL,NULL,NULL,NULL};
	DBCOUNTITEM		cMaxOpenRowsCount=0;
	DBCOUNTITEM		cRowsObtained;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		ulCount;
	
	DBINDEXEDERROR	*prgErrors=NULL;
	BOOL			fTestPass=FALSE;


	//record the g_cMaxOpenRows
	cMaxOpenRowsCount=g_cMaxOpenRowsCount;

	//set max open rows to 2 
	g_cMaxOpenRowsCount=2;

	//open rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		1,&guidProperty))
		goto CLEANUP;

	//an array of 4 bookmarks to different rows
	for(ulCount=0;ulCount<4;ulCount++)
	{
		if(!GetBookmark((ulCount+1)*9, &rgcbBookmarks[ulCount],&rgpBookmarks[ulCount]))
			goto CLEANUP;
	}

	//retrieve 4 row handles
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,4,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,&cRowsObtained,&pHRow,TRUE,&cErrors,&prgErrors),
		DB_S_ROWLIMITEXCEEDED))
		goto CLEANUP;

	//2 rows should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//verify no Errors should be returned
	if(!COMPARE(cErrors, 0))
		goto CLEANUP;

	if(!COMPARE(prgErrors, NULL))
		goto CLEANUP;

	//verify all the row handles
	for(ulCount=0;ulCount<2;ulCount++)
	{
		if(!COMPARE(VerifyRowPosition(pHRow[ulCount],(ulCount+1)*9,g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	//release the bookmarks
	for(ulCount=0; ulCount<4; ulCount++)
		PROVIDER_FREE(rgpBookmarks[ulCount]);

	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}
	
	//release the error information if there is any
	if(prgErrors)
		PROVIDER_FREE(prgErrors);

	//restore the max open rows
	g_cMaxOpenRowsCount=cMaxOpenRowsCount;

	//release the rowset
	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;  
*/
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Use default MaxOpenRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxOpenRows::Variation_5()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	BOOL		fTestPass=FALSE;


	//open rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		0,NULL))
		goto CLEANUP;

	// *pBookmark points to the first row
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,g_lRowLast,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//all the row handles should be retrieved
	if(!COMPARE(cRowsObtained, ULONG(g_lRowLast)))
		goto CLEANUP;

	//verify the last row
	if(COMPARE(VerifyRowPosition(pHRow[cRowsObtained-1],g_lRowLast,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxOpenRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_IRowset)
//*-----------------------------------------------------------------------
//| Test Case:		Related_IRowset - test related interface IRowset
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset::Init()
{
	BOOL fTestPass = FALSE;
	DBPROPID	rgDBPropID[2];

	rgDBPropID[0]=DBPROP_CANSCROLLBACKWARDS;
	rgDBPropID[1]=DBPROP_CANFETCHBACKWARDS;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//This is for provider like Monarch.
	//Monarch does not allow to set DBPROP_CANSCROLLBACKWARDS and DBPROP_CANFETCHBACKWARDS
	//but, when we have IRowsetLocate, these properties are automatically on.
	//We first create the rowset without setting the properties, then check whether they are on.
	//If they are not on, release and reopen the rowset with properties set.

	//create a rowset and  create an accessor.  Set IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		0, NULL));
	
	//If the properties are already on, we are set.  Otherwise, release the rowset and reopen one
	if(!GetProperty(DBPROP_CANSCROLLBACKWARDS,DBPROPSET_ROWSET,m_pIAccessor)||!GetProperty(DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET,m_pIAccessor))
	{
		ReleaseRowsetAndAccessor();
		//create a rowset and  create an accessor.  Set IRowsetLocate, and two properties
		TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
			2, rgDBPropID));
	}

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt does not change the cursor location of the rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_1()
{
	HROW *	 	pHRow=NULL;
	DBCOUNTITEM	cRows;
	BOOL		fTestPass=FALSE;


	//get the second to the last row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,1,&cRows, &pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, g_lRowLast-1, g_pCTable), TRUE))
		goto CLEANUP;

	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//get the last row handle 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,-1,&cRows, &pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, g_lRowLast, g_pCTable), TRUE))
		goto CLEANUP;

	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//get the third to the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,-1,&cRows, &pHRow),S_OK))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(*pHRow, g_lRowLast-2, g_pCTable), TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//free the memory
	PROVIDER_FREE(pHRow);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetRowsByBookmark does not change the cursor location of the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_2()
{
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	HROW			hRow[1];
	HROW			*pHRow=hRow;
	HROW			hRowHandle=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;

	//get a bookmark for the first row
	if(!GetBookmark(1,&cbBookmark, &pBookmark))
		goto CLEANUP;

	//restart position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//move the cursor position to before the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-1,0,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//no row should have be retrieved
	if(!COMPARE(cRowsObtained, 0))
		goto CLEANUP;


	//retrieve the first row
	if(!COMPARE(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,&hRowHandle,NULL),S_OK))
		goto CLEANUP;

	//verify the first row is retrieved
	if(!COMPARE(VerifyRowPosition(hRowHandle, 1, g_pCTable),TRUE))
		goto CLEANUP;

	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowset->GetNextRows, return DB_E_ROWSNOTRELEASED
		if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	}

	//release the row handle retrieved by IRowsetLocate::GetRowsByBookmark
	if(!CHECK(m_pIRowset->ReleaseRows(1,&hRowHandle,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hRowHandle=NULL;

	//IRowset->GetNextRows to retrieve the last row, return S_OK
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-1,1,&cRowsObtained, &pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//verify the last row is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, g_lRowLast, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//free the bookmark
	PROVIDER_FREE(pBookmark);

	//free the row handle
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);

	if(hRowHandle)
		CHECK(m_pIRowset->ReleaseRows(1,&hRowHandle,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrieve the same row handle twice by GetRowsAt and GetRowsByBookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_3()
{
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmarkGetRowsAt=(BYTE *)&DBBookmark;
	HROW			hRow[1];
	HROW			*pHRow=hRow;
	HROW			hRowHandle=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;


	//get a bookmark for the 2rd row
	if(!GetBookmark(2,&cbBookmark, &pBookmark))
		goto CLEANUP;

	//retrieve the 2rd row
	if(!COMPARE(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,&hRowHandle,NULL),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(hRowHandle, 2, g_pCTable), TRUE))
		goto CLEANUP;

	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowsetLocate->GetRowsAt to retriev the first row, return DB_E_ROWSNOTRELEASED
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkGetRowsAt,
			0,1,&cRowsObtained, &pHRow),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	}

	//release the row handle retrieved by IRowsetLocate::GetRowsByBookmark
	if(!CHECK(m_pIRowset->ReleaseRows(1,&hRowHandle,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	hRowHandle=NULL;

	//IRowsetLocate->GetRowsAt to retrieve the first row, return S_OK
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkGetRowsAt,
		0,1,&cRowsObtained, &pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//verify the first row is retrieved
	if(!COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable),TRUE))
		goto CLEANUP;

	
	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowsetLocate->GetRowsByBookmark return DB_E_ROWSNOTRELEASED
		if(!COMPARE(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmark,&hRowHandle,NULL),
			DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	}

	//release the row handle retrieved by IRowsetLocate::GetRowsAt
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	cRowsObtained=0;

	//retrieve the 2rd row by IRowsetLocate::GetRowsByBookmark
	if(!COMPARE(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,&hRowHandle,NULL),S_OK))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(hRowHandle, 2, g_pCTable),TRUE))
		fTestPass=TRUE;


CLEANUP:

	//free the bookmark
	PROVIDER_FREE(pBookmark);

	//free the row handle
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);

	if(hRowHandle)
		CHECK(m_pIRowset->ReleaseRows(1, &hRowHandle, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Make sure IRowsetLocate::GetRowsAt return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is not rele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		hRow[1];
	HROW		*pHRow=hRow;
	BOOL		fTestPass=FALSE;

	 //restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//get the first row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable), TRUE))
		goto CLEANUP;


	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowsetLocate::GetRowsAt return DB_E_ROWSNOTRELEASED
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,1,&cRowsObtained,
			&pHRow),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;

		//no row should be retrieved
		if(!COMPARE(cRowsObtained,0))
			goto CLEANUP;
	}

	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	 //IRowsetLocate::GetRowsAt return S_OK
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,1,&cRowsObtained,
		&pHRow),S_OK))
		goto CLEANUP;

	//verify the second row handle is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 2, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,hRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Make sure IRowsetLocate::GetRowsByBookmark return DB_E_ROWSNOTRELEASED if the row handles retrieved by IRowset::GetNextRows is
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_5()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		hRow[1];
	HROW		*pHRow=hRow;
	HROW		hRowHandle=NULL;
	BOOL		fTestPass=FALSE;

	//get the bookmark for the 6th row
	if(!GetBookmark(6,&cbBookmark,&pBookmark))
		goto CLEANUP;

	 //restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//get the 4th row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, 4, g_pCTable), TRUE))
		goto CLEANUP;

	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowsetLocate::GetRowsByBookmark return DB_E_ROWSNOTRELEASED
		if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,
			&hRowHandle,NULL),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	}
	
	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	cRowsObtained=0;

	 //IRowsetLocate::GetRowsByBookmark return S_OK
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,
		&hRowHandle,NULL),S_OK))
		goto CLEANUP;

	//verify the 6th row handle is retrieved
	if(COMPARE(VerifyRowPosition(hRowHandle, 6, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handle
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,hRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	if(hRowHandle)
		CHECK(m_pIRowset->ReleaseRows(1,&hRowHandle,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Fetch the last row.  Fetch the second to last row.  Fetch the 1st row.  Fetch the last row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_6()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained=0;
	HROW		hRow[5];
	HROW		*pHRow=hRow;
	HROW		*pHRowHandle=NULL;
	BOOL		fTestPass=FALSE;

   	//get the bookmark for the 3rd row
	if(!GetBookmark(3,&cbBookmark,&pBookmark))
		goto CLEANUP;

	 //restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//fetch the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast,-1,&cRowsObtained,
		&pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, g_lRowLast, g_pCTable), TRUE))
		goto CLEANUP;

	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//call GetNextRows
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
		-1,1,&cRowsObtained,&pHRowHandle),S_OK))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRowHandle, 2, g_pCTable), TRUE))
		goto CLEANUP;

	//release the row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRowHandle,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//release memory
	PROVIDER_FREE(pHRowHandle);
	pHRowHandle=NULL;

	//fetch the second to the last row
    if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,-3,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 3);

	//check the row position.  The last row fetched should be the (g_lRowLast-5) 55th row
	if(!COMPARE(VerifyRowPosition(hRow[2],g_lRowLast-5 , g_pCTable),TRUE))
		goto CLEANUP;
	
	if(!COMPARE(VerifyRowPosition(hRow[1],g_lRowLast-4, g_pCTable), TRUE))
		goto CLEANUP;
	
	if(!COMPARE(VerifyRowPosition(hRow[0], g_lRowLast-3, g_pCTable), TRUE))
		goto CLEANUP;
	
	
	//release row handles
	if(!CHECK(m_pIRowset->ReleaseRows(3,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//fetch one more row
   if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,2,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;	 

  	COMPARE(cRowsObtained, 2);

	//check the row position.  
	if(COMPARE(VerifyRowPosition(hRow[0],g_lRowLast-6 , g_pCTable),TRUE))
	{
		if(COMPARE(VerifyRowPosition(hRow[1],g_lRowLast-5 , g_pCTable),TRUE))
			fTestPass = TRUE;
	}
		

CLEANUP:

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handles
	if(pHRowHandle)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowHandle,NULL,NULL,NULL),S_OK);

		//release memory
		PROVIDER_FREE(pHRowHandle); 
	}

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc position at the lst row.  GetRowsAt.  Fetch backwards one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowset::Variation_7()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	HROW			hRow[1];
	HROW			*pHRow=hRow;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;

	//restartposition
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//move the cursor position to before the third row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,0,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//no row should have be retrieved
	if(!COMPARE(cRowsObtained, 0))
		goto CLEANUP;

	// *pBookmark points to the first row, lRowOffset=0, cRow=1
	if(!COMPARE(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//verify the first row is retrieved
	if(!COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable),TRUE))
		goto CLEANUP;

	if ( !GetProp(DBPROP_CANHOLDROWS) )
	{
		//IRowset->GetNextRows, return DB_E_ROWSNOTRELEASED
		if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),DB_E_ROWSNOTRELEASED))
			goto CLEANUP;
	
		//no row should be retrieved
		if(!COMPARE(cRowsObtained,0))
			goto CLEANUP;
	}

	//release the row handle retrieved by IRowsetLocate::GetRowsAt
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	//IRowset->GetNextRows to retrieve the third row, return S_OK
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),S_OK))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//verify the first row is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//free the row handle
	if(cRowsObtained)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowset::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Rowset_SingleRow)
//*-----------------------------------------------------------------------
//| Test Case:		Rowset_SingleRow - test single row rowset
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow::Init()
{
	BOOL fTestPass = FALSE;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	if ( !g_p1RowTable )
		return TEST_SKIPPED;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetLocate));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Compare return DBCOMPARE_EQ and Hash return the same value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_1()
{
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	ULONG_PTR		rgcbBookmarks[2];
	DBCOMPARE		dwComparison;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass=FALSE;

	//get the bookmark for the first row
	if(!GetBookmark(1,&rgcbBookmarks[0],&rgpBookmarks[0]))
		goto CLEANUP;

	//get the bookmark for the first row again
	if(!GetBookmark(1,&rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//Compare should return DBCOMPARE_EQ
	if(!CHECK(m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1], rgpBookmarks[1],&dwComparison),S_OK))
		goto CLEANUP;

	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Hash should return the same value
	if(!CHECK(m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		goto CLEANUP;

	if(!COMPARE(rgHashedValues[0], rgHashedValues[1]))
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt: *pBookmark=DBBMK_FIRST. lRowsOFfset==1 and cRows=1.  DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_2()
{
	DBCOUNTITEM	cRowsObtained=0;
	HROW		*pHRow=NULL;
	BYTE		*pBookmark=NULL;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BOOL		fTestPass=FALSE;
	DBCOUNTITEM	totalRows = 0;

	pBookmark=(BYTE *)&DBBookmark;

	//For testing other providers.
	//e.g. for index server, currently we always have 60 rows
	if(!CHECK(CountRowsOnRowset(m_pIRowset, &totalRows), S_OK))
		goto CLEANUP;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,totalRows,1,&cRowsObtained,
		&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//no row should be retrieved
	if(!COMPARE(cRowsObtained, 0))
		goto CLEANUP;

	if(!COMPARE(pHRow, NULL))
		goto CLEANUP;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,(totalRows+1),&cRowsObtained,
		&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//one row should be retrieved
	if(!COMPARE(cRowsObtained, totalRows))
		goto CLEANUP;

	if(pHRow)
		fTestPass=TRUE;

CLEANUP:
	//release the row handle
	if(cRowsObtained && pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt: *pBookmark=DBBMK_LAST, lRowsOffset=0 and cRows=1.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_3()
{
	DBCOUNTITEM	cRowsObtained;
	HROW		*pHRow=NULL;
	BYTE		*pBookmark;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BOOL		fTestPass=FALSE;

	pBookmark=(BYTE *)&DBBookmark;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRowsObtained,
		&pHRow),S_OK))
		goto CLEANUP;

	//one row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//pHRow should points to memory allocated by the provider
	if(!pHRow)
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:
	//release the row handle
	if(cRowsObtained && pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetRowsByBookmark: Pass array of two bookmarks: the first and the last row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset_SingleRow::Variation_4()
{
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	ULONG_PTR		rgcbBookmarks[2];
	HROW			rghRow[2];
	HROW			*pHRow=rghRow;
	DBREFCOUNT		cRefCount;
	BOOL			fTestPass=FALSE;

	//get the bookmark for the first row
	if(!GetBookmark(1,&rgcbBookmarks[0],&rgpBookmarks[0]))
		goto CLEANUP;

	//get the bookmark for the first row again
	if(!GetBookmark(1,&rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//get the row handles for the first row
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle twice
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	if(!CHECK(m_pIRowset->ReleaseRows(1,&(rghRow[1]),NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_SingleRow::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Consistency)
//*-----------------------------------------------------------------------
//| Test Case:		Consistency - make sure GetRowsAt and GetRowsByBookmark return the same value
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency::Init()
{
	BOOL fTestPass = FALSE;
	DBPROPID	guidPrpt=DBPROP_CANHOLDROWS;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor, requesting DBPROP_CANHOLDROWS
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable,SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,1,&guidPrpt));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve three row handles from GetRowsAt and GetRowsByBookmark, one of which is overlapping
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Consistency::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark1=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained1=0;
	HROW			hRow1[3];
	HROW			*pHRow1=hRow1;

	BYTE			*rgpBookmarks2[3]={NULL,NULL,NULL};
	ULONG_PTR		rgcbBookmarks2[3];
	DBCOUNTITEM		cRowsObtained2=0;
	HROW			hRow2[3];
	HROW			*pHRow2=hRow2;
	BOOL			fTestPass=FALSE;
	DBREFCOUNT		cRefCount;
	DBROWSTATUS		rgRowStatus[3];


	//retrieve the 3rd, 4th, and 5th row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark1,2,3,
		&cRowsObtained1,&pHRow1),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained1, 3);

	//get the bookmark for the first row
	if(!GetBookmark(1,rgcbBookmarks2,rgpBookmarks2))
		goto CLEANUP;

	//get the bookmark for the 2nd row
	if(!GetBookmark(2,&rgcbBookmarks2[1],&rgpBookmarks2[1]))
		goto CLEANUP;

	//get the bookmark for the 3rd row
	if(!GetBookmark(3,&rgcbBookmarks2[2],&rgpBookmarks2[2]))
		goto CLEANUP;

	//retriev the first row handle again
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks2,
		(const BYTE **)rgpBookmarks2,pHRow2,rgRowStatus),S_OK))
		goto CLEANUP;

	cRowsObtained2=3;

	for(cRefCount=0; cRefCount<3; cRefCount++)
		COMPARE(rgRowStatus[cRefCount],DBROWSTATUS_S_OK);

	//make sure the 3rd row handle is retrieved twice
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(hRow2[2]),NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	//If the HRows are the same ref count should be 1 after release
	if(COMPARE(cRefCount,ULONG(((pHRow1[0] == hRow2[2]) ? 1 : 0))))
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks2[0]);
	PROVIDER_FREE(rgpBookmarks2[1]);
	PROVIDER_FREE(rgpBookmarks2[2]);

	//release the row handle
	if(cRowsObtained1)
		CHECK(m_pIRowset->ReleaseRows(3,pHRow1,NULL,NULL,NULL),S_OK);

	if(cRowsObtained2)
	{	
		if(fTestPass)
			//do not attempt to release the second row handle again
			CHECK(m_pIRowset->ReleaseRows(2,hRow2,NULL,NULL,NULL),S_OK);
		else
			CHECK(m_pIRowset->ReleaseRows(3,pHRow2,NULL,NULL,NULL),S_OK);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrive the first row, GetRowsAt uses DBBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Consistency::Variation_2()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark1=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained1=0;
	HROW			hRow1[1];
	HROW			*pHRow1=hRow1;

	BYTE			*rgpBookmarks2[1]={NULL};
	ULONG_PTR		rgcbBookmarks2[1];
	DBCOUNTITEM		cRowsObtained2=0;
	HROW			hRow2[1];
	HROW			*pHRow2=hRow2;
	BOOL			fTestPass=FALSE;
	DBREFCOUNT		cRefCount;

	//retrieve the first row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark1,0,1,
		&cRowsObtained1,&pHRow1),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained1, 1);

	//get the bookmark for the first row
	if(!GetBookmark(1,rgcbBookmarks2,rgpBookmarks2))
		goto CLEANUP;

	//retrieve the first row handle again
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,rgcbBookmarks2,
		(const BYTE **)rgpBookmarks2,pHRow2,
		NULL),S_OK))
		goto CLEANUP;

	cRowsObtained2=1;

	//make sure the first row handle is retrieved
	if(!COMPARE(VerifyRowPosition(*pHRow1, 1,g_pCTable),TRUE))
		goto CLEANUP;

	//make sure the first row handle is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow2, 1,g_pCTable),TRUE))
		fTestPass=TRUE;


CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks2[0]);

	//release the row handle
	if(cRowsObtained1)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow1,NULL,&cRefCount,NULL),S_OK);
		if(cRowsObtained2)
		{
			//If the HRows are the same ref count should be 1 after release
			if(!COMPARE(cRefCount,ULONG(((pHRow1[0] == pHRow2[0]) ? 1 : 0))))
				fTestPass=FALSE;
		}
	}

	if(cRowsObtained2)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow2,NULL,&cRefCount,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrieve the last row.  GetRowsAt uses DBBMK_LAST.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Consistency::Variation_3()
{
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark1=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained1=0;
	HROW			hRow1[1];
	HROW			*pHRow1=hRow1;
	BYTE			*rgpBookmarks2[1]={NULL};
	ULONG_PTR		rgcbBookmarks2[1];
	DBCOUNTITEM		cRowsObtained2=0;
	HROW			hRow2[1];
	HROW			*pHRow2=hRow2;
	BOOL			fTestPass=FALSE;
	DBREFCOUNT		cRefCount;

	//retrieve the last row handle
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark1,0,1,
		&cRowsObtained1,&pHRow1),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained1, 1);

	//get the bookmark for the last row
	if(!GetBookmark(g_lRowLast,rgcbBookmarks2,rgpBookmarks2))
		goto CLEANUP;

	//retrieve the last row handle again
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,rgcbBookmarks2,
		(const BYTE **)rgpBookmarks2,pHRow2,NULL),S_OK))
		goto CLEANUP;

	cRowsObtained2=1;

	//make sure the last row handle is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow1,g_lRowLast,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks2[0]);

	//release the row handle
	if(cRowsObtained1)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow1,NULL,&cRefCount,NULL),S_OK);
		if(cRowsObtained2)
		{
			//If the HRows are the same ref count should be 1 after release
			if(!COMPARE(cRefCount,ULONG(((pHRow1[0] == pHRow2[0]) ? 1 : 0))))
				fTestPass=FALSE;
		}
	}


	if(cRowsObtained2)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow2,NULL,&cRefCount,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Consistency::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Parameters)
//*-----------------------------------------------------------------------
//| Test Case:		Parameters - valid and invalid parameters passed into methods
//|	Created:		02/07/96
//|	Updated:		04/25/98
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
	BOOL			fTestPass=FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor	 
	//
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate));

	//make sure IColumnsInfo returns correct infomation on the 0th column,
	//which is the bookmark column
	if(!CHECK(m_pIRowsetLocate->QueryInterface(IID_IColumnsInfo, 
		(void **)&pIColumnsInfo),S_OK))
		goto CLEANUP;

	if(!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &prgInfo,&pStringsBuffer),S_OK))
		goto CLEANUP;

	//the first element in the array should be the bookmark column
	if(!COMPARE(prgInfo->iOrdinal, 0))
		goto CLEANUP;

	if(!COMPARE((prgInfo->dwFlags)&DBCOLUMNFLAGS_ISBOOKMARK,DBCOLUMNFLAGS_ISBOOKMARK))
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:
	//release the interface pointer
	SAFE_RELEASE(pIColumnsInfo);

	//release te memory
	PROVIDER_FREE(prgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return fTestPass;	   	
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt: *pBookmark=DBBMK_INVALID, DB_E_BADBOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_INVALID;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained;
	HROW			*prghRows=NULL;

	if(CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		0,1,&cRowsObtained,&prghRows),DB_E_BADBOOKMARK))
	{
		COMPARE(prghRows, NULL);
		return TEST_PASS;
	}
	else
	{
		if(cRowsObtained)
			CHECK(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL,NULL,NULL),S_OK);

		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt: cRow==0 and *prghRows is not NULL on input
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_2()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained;
	HROW			hRow[1];
	HROW			*prghRows=hRow;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,0,
		&cRowsObtained, &prghRows),S_OK))
		return TEST_FAIL;

	if(!COMPARE(cRowsObtained,0))
		return TEST_FAIL;

	//prghRows should not be set to NULL
	if(prghRows==NULL)
		return TEST_FAIL;

	return TEST_PASS;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt: *pcRowsObtained is 0 and *prghRows is NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_3()
{
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowsObtained;
	HROW			*prghRows=NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,0,
		&cRowsObtained, &prghRows),S_OK))
		return TEST_FAIL;

	if(!COMPARE(cRowsObtained,0))
		return TEST_FAIL;

	//prghRows should still be  NULL
	if(prghRows)
		return TEST_FAIL;

	return TEST_PASS;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetRowsByBookmark: cRows==0 and *prghRows and *prgErrors are not NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_4()
{
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	HROW			hRow[1];
	HROW			*phRow=hRow;
	BOOL			fTestPass=FALSE;


	//get the bookmark for the third row
	if(!GetBookmark(3,&cbBookmark,&pBookmark))
		return TEST_FAIL; 

	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,0,&cbBookmark,
		(const BYTE **)&pBookmark,phRow,NULL),S_OK))
		goto CLEANUP;


	//try again by passing phRow and pErrors as NULLs
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,NULL,NULL),E_INVALIDARG))
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc rgpBookmarks points to DBBMK_INVALID and fRetrunErrors=FALSE.  *prghRows *prgErrors are NULL on input.  DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_5()
{
	ULONG_PTR		cbBookmark=1;
	DBBOOKMARK		DBBookmark=DBBMK_INVALID;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	HROW			hRow=NULL;
	DBROWSTATUS		DBRowStatus;
	BOOL			fTestPass=FALSE;


	//pass the invalid bookmark
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,&hRow,&DBRowStatus),
		DB_E_ERRORSOCCURRED))
		goto CLEANUP;

	//make sure no rows is retrieved
	if(!COMPARE(hRow,DB_NULL_HROW))
		goto CLEANUP;

	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID))
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 3 bookmarks.  The first bookmark is valid, the second bookmark is a NULL pointer and the third element of rgcb
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_6()
{
	ULONG_PTR		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	HROW			hRow[3];
	DBCOUNTITEM		cCount;
	HROW			*phRow=hRow;
	DBROWSTATUS		rgDBRowStatus[3];
	BOOL			fTestPass=FALSE;

	//Init
	for(cCount=0; cCount<3; cCount++)
		hRow[cCount]=(HROW)1;

	//set up the bookmarks.  
	//1st is valid, 2nd is NULL pointer and third of rgcbBookmarks is 0
	//get a valid bookmark
	if(!GetBookmark(1,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//fill up the rest
	if(!GetBookmark(3, &rgcbBookmarks[1], &rgpBookmarks[2]))
		goto CLEANUP;

	rgpBookmarks[1]=NULL;
	rgcbBookmarks[2]=0;

	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,phRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(hRow[1], DB_NULL_HROW) ||
		!COMPARE(hRow[2],DB_NULL_HROW)
	  )
		goto CLEANUP;

	//only row is valid
	if(hRow[0]==DB_NULL_HROW)
		goto CLEANUP;

	if(	!(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK) 
			&& COMPARE(rgDBRowStatus[1], DBROWSTATUS_E_INVALID)
			&& COMPARE(rgDBRowStatus[2], DBROWSTATUS_E_INVALID) 
		  )
		)
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(hRow[0],1,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1, phRow, NULL,NULL,NULL),S_OK);

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[2]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST, valid, DBBMK_INVALID.  DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_7()
{
	ULONG_PTR		rgcbBookmarks[4]={1,1,1,1};
	DBBOOKMARK		rgDBBookmarks[3]={DBBMK_FIRST, DBBMK_LAST, DBBMK_INVALID};
	BYTE			*rgpBookmarks[4]={NULL,NULL,NULL,NULL};
	HROW			hRow[4];
	HROW			*phRow=hRow;
	DBROWSTATUS		rgDBRowStatus[4];
	BOOL			fTestPass=FALSE;

	//set up the bookmarks. 
	//1st=DBBMK_FIRST, 2nd=DBBMK_VALIST, 3rd=valid, 4th=DBBMK_INVALID
	//get a valid bookmark
	if(!GetBookmark(2,&rgcbBookmarks[2],&rgpBookmarks[2]))
		return TEST_FAIL;
	
	rgpBookmarks[0]=(BYTE *)&rgDBBookmarks[0];
	rgpBookmarks[1]=(BYTE *)&rgDBBookmarks[1];
	rgpBookmarks[3]=(BYTE *)&rgDBBookmarks[2];


	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,4,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,phRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(hRow[0], DB_NULL_HROW) ||
		!COMPARE(hRow[1],DB_NULL_HROW)  ||
		!COMPARE(hRow[3],DB_NULL_HROW)
	  )
		goto CLEANUP;

	if(hRow[2]==DB_NULL_HROW)
		goto CLEANUP;


	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID) 
	&& COMPARE(rgDBRowStatus[1], DBROWSTATUS_E_INVALID) 
	&& COMPARE(rgDBRowStatus[2], DBROWSTATUS_S_OK) 
	&& COMPARE(rgDBRowStatus[3], DBROWSTATUS_E_INVALID))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1, &(hRow[2]), NULL,NULL,NULL),S_OK);

	//release the memory used by the bookmark
	PROVIDER_FREE(rgpBookmarks[2]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 3 bookmarks: valid, NULL pointer, DBBMK_FIRST.  fReturnErrors=FALSE and prgErrors=NULL, pcErrors is not NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_8()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	ULONG_PTR		rgcbBookmarks[3] = {0, 0, 0};
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	HROW			hRow[3];
	HROW			*phRow=hRow;
	DBROWSTATUS		rgDBRowStatus[3];
	BOOL			fTestPass=FALSE;

	//set up the bookmarks.  
	//1st is valid, 2nd is NULL pointer, 3rd is DBBMK_FIRST
	//get a valid bookmark
	if(!GetBookmark(1,&rgcbBookmarks[0],&rgpBookmarks[0]))
		goto CLEANUP;

	//fill up the rest
	if(!GetBookmark(2, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//free the memory
	PROVIDER_FREE(rgpBookmarks[1]);
	rgpBookmarks[1]=NULL;

	rgcbBookmarks[2]=2;
	rgpBookmarks[2]=(BYTE *)&DBBookmark;

	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,phRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(hRow[1],DB_NULL_HROW)  ||
		!COMPARE(hRow[2],DB_NULL_HROW)
	  )
		goto CLEANUP;

	if(hRow[0]==DB_NULL_HROW)
		goto CLEANUP;

	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK)
		&&COMPARE(rgDBRowStatus[1],DBROWSTATUS_E_INVALID)
		&&COMPARE(rgDBRowStatus[2],DBROWSTATUS_E_INVALID))
		fTestPass=TRUE;

CLEANUP:
	
	//release the memory used by the bookmark
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	//release the row handle
	if (phRow)
		CHECK(m_pIRowset->ReleaseRows(1, phRow, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 3 bookmarks: first cbBokmark is less length, second *pBookmark is bogus, and third is valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_9()
{
	BYTE			byteBookmark=4;
	ULONG_PTR		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL, NULL, NULL};
	HROW			hRow[3];
	HROW			*phRow=hRow;
	DBROWSTATUS		rgDBRowStatus[3];
	BOOL			fTestPass=FALSE;

	//set up the bookmarks.  
	//1st cbBookmark is less length, 2nd is bogus, 3rd is valid
	//get a valid bookmark
	if(!GetBookmark(1,&rgcbBookmarks[2],&rgpBookmarks[2]))
		return TEST_FAIL;

	//1st bookmark is less length
	if(!GetBookmark(2, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;

	rgcbBookmarks[0]--;

	//2nd is bogus
	rgcbBookmarks[1]=rgcbBookmarks[0];
	rgpBookmarks[1]=&byteBookmark;


	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks,phRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(hRow[0],DB_NULL_HROW)  ||
		!COMPARE(hRow[1],DB_NULL_HROW)
	  )
		goto CLEANUP;

	if(hRow[2]==DB_NULL_HROW)
		goto CLEANUP;


	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID) 
	&& COMPARE(rgDBRowStatus[1], DBROWSTATUS_E_INVALID) 
	&& COMPARE(rgDBRowStatus[2], DBROWSTATUS_S_OK) )
		fTestPass=TRUE;

CLEANUP:

	//release the memory used by the bookmark
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[2]);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1, &(hRow[2]), NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Hash:Pass an array of two bookmarks, one is valid and the other is not.  	pcErrors==NULL and prgErrors==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_10()
{
	ULONG_PTR	rgcbBookmarks[2];
	BYTE		*rgpBookmarks[2]={NULL,NULL};
	DBHASHVALUE	rgHashedValues[2];
	BOOL		fTestPass=FALSE;

	//get a valid bookmark
	if(!GetBookmark(1,&rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;

	//get a invalid bookmark
	if(!GetBookmark(2,&rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;
	rgcbBookmarks[1]--;

	//pass rgBookmarkStatus as NULL
	if(CHECK(m_pIRowsetLocate->Hash(NULL, 2, rgcbBookmarks, (const BYTE **)rgpBookmarks, 
		rgHashedValues,NULL),DB_S_ERRORSOCCURRED))
			fTestPass=TRUE;
	
CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Hash:Pass an invalid bookmark.  pcErrors==NULL and prgErrors is not NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_11()
{
	ULONG_PTR		cbBookmark=1;
	DBBOOKMARK		DBBookmark=DBBMK_INVALID;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBHASHVALUE		dwHashedValue;
	DBROWSTATUS		DBRowStatus;

	//pass pcErrors==NULL and prgErrors==NULL.
	if(!CHECK(m_pIRowsetLocate->Hash(NULL, 1, &cbBookmark, (const BYTE **)&pBookmark, 
		&dwHashedValue,&DBRowStatus),DB_E_ERRORSOCCURRED))
			return TEST_FAIL;

	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 4 bookmarks.  The first element of rgcbBookmarks is 0, the second bookmark is valid,  the third and the forth
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_12()
{
	ULONG_PTR		rgcbBookmarks[4];
	BYTE			*rgpBookmarks[4]={NULL, NULL, NULL, NULL};
	DBHASHVALUE		rgHashedValues[4];
	DBROWSTATUS		rgDBRowStatus[4];
	BOOL			fTestPass=FALSE;
	
	//get a valid bookmark
	if(!GetBookmark(5, &rgcbBookmarks[1], &rgpBookmarks[1]))
		return TEST_FAIL;

	//1st rgcbBookmarks is 0
	if(!GetBookmark(2, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;
	rgcbBookmarks[0]=0;

	//3rd and 4th is NULL pointer
	rgpBookmarks[2]=NULL;
	rgpBookmarks[3]=NULL;
	rgcbBookmarks[2]=rgcbBookmarks[0];
	rgcbBookmarks[3]=rgcbBookmarks[0];

	if(!CHECK(m_pIRowsetLocate->Hash(NULL,4,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,rgDBRowStatus),DB_S_ERRORSOCCURRED))
		goto CLEANUP;


	if(		COMPARE(rgDBRowStatus[0],DBROWSTATUS_E_INVALID) 
		&&	COMPARE(rgDBRowStatus[1],DBROWSTATUS_S_OK) 
		&&	COMPARE(rgDBRowStatus[2],DBROWSTATUS_E_INVALID)
		&&	COMPARE(rgDBRowStatus[3],DBROWSTATUS_E_INVALID))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Pass an array of 4 bookmarks: DBBMK_FIRST, DBBMK_LAST,valid, DBBMK_INVALID.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_13()
{
	DBBOOKMARK		rgDBBookmarks[3]={DBBMK_FIRST, DBBMK_LAST, DBBMK_INVALID};
	ULONG_PTR		rgcbBookmarks[5]={1,1,1,1,1};
	BYTE			*rgpBookmarks[5]={NULL,NULL,NULL,NULL,NULL};
	DBROWSTATUS		rgDBRowStatus[5];
	DBHASHVALUE		rgHashedValues[5];
	BOOL			fTestPass=FALSE;
	
	//the 3rd bookmark is valid
	if(!GetBookmark(6, &rgcbBookmarks[2], &rgpBookmarks[2]))
		return TEST_FAIL;

	//the 5th bookmark is valid
	if(!GetBookmark(6, &rgcbBookmarks[4], &rgpBookmarks[4]))
		goto CLEANUP;

	//the 1st, 2nd and 4th is DBBMK_FIST, DBBMK_LAST, and DBBMK_INVALID
	rgpBookmarks[0]=(BYTE *)&rgDBBookmarks[0];
	rgpBookmarks[1]=(BYTE *)&rgDBBookmarks[1];
	rgpBookmarks[3]=(BYTE *)&rgDBBookmarks[2];

	if(!CHECK(m_pIRowsetLocate->Hash(NULL,5,rgcbBookmarks,(const BYTE **)rgpBookmarks,
	rgHashedValues,rgDBRowStatus),DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//the two hash values has to be the same
	if(!COMPARE(rgHashedValues[2], rgHashedValues[4]))
		goto CLEANUP;

	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID)
	&& COMPARE(rgDBRowStatus[1], DBROWSTATUS_E_INVALID)
	&& COMPARE(rgDBRowStatus[2], DBROWSTATUS_S_OK)
	&& COMPARE(rgDBRowStatus[3], DBROWSTATUS_E_INVALID) 
	&& COMPARE(rgDBRowStatus[4], DBROWSTATUS_S_OK) )
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[2]);
	PROVIDER_FREE(rgpBookmarks[4]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid bookmark, *pcErrors=0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Parameters::Variation_14()
{
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark;
	DBHASHVALUE		dwHashedValue;
	DBROWSTATUS		DBRowStatus;
	BOOL			fTestPass=FALSE;

	//get a valid bookmark
	if(!GetBookmark(1,&cbBookmark, &pBookmark))
		return TEST_FAIL;

	//pass pcErrors==NULL and prgErrors is not NULL.
	if(!CHECK(m_pIRowsetLocate->Hash(NULL, 1, &cbBookmark, (const BYTE **)&pBookmark, 
		&dwHashedValue,&DBRowStatus),S_OK))
			goto CLEANUP;

	if(COMPARE(DBRowStatus, DBROWSTATUS_S_OK))
		fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Parameters::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_Compare)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary_Compare - boundary conditions for Compare
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_Compare::Init()
{
	BOOL fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetLocate));
	
	if(GetProp(DBPROP_ORDEREDBOOKMARKS))	
		m_fOrderedBookmark=TRUE;
	
	//get a valid bookmark value
	m_cbBookmark=0;
	m_pBookmark=NULL;
						
	if(!GetBookmark(1,&m_cbBookmark, &m_pBookmark))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark1==0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_1()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,0,m_pBookmark,m_cbBookmark,
		m_pBookmark,&m_DBCompare),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark2==0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_2()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,0,
		m_pBookmark,&m_DBCompare),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark1==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_3()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,NULL,m_cbBookmark,
		m_pBookmark,&m_DBCompare),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookmark2==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_4()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,m_cbBookmark,
		NULL,&m_DBCompare),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pdwComparistion==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_5()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,m_cbBookmark,
		m_pBookmark,NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pBookmark1==DBBMK_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_6()
{
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;

	if(CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,m_cbBookmark,
		m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pBookmark2==DBBMK_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_7()
{
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;

	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,STD_BOOKMARKLENGTH,
		(BYTE *)&DBBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc pBookmark1==DBBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_8()
{
	m_DBBookmark=DBBMK_FIRST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,
			STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;

	if(!COMPARE(m_DBCompare, DBCOMPARE_EQ))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc pBookmark2==DBBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_9()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	
	m_DBBookmark=DBBMK_FIRST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,
				STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;
	
	//the result is independent on whether the DBPROP_ORDEREDBOOKMARKS is on.
	if(!COMPARE(m_DBCompare, DBCOMPARE_NE))
		return TEST_FAIL;
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pBookmark1==DBBMK_LAST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_10()
{
	m_DBBookmark=DBBMK_LAST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,
				STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;

	if(!COMPARE(m_DBCompare, DBCOMPARE_EQ))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc pBookmark2==DBBMK_LAST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_11()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	
	m_DBBookmark=DBBMK_LAST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&m_DBBookmark,
				STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;

	//the result is independent on whether the DBPROP_ORDEREDBOOKMARKS is on.
	if(!COMPARE(m_DBCompare, DBCOMPARE_NE))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark1==length-1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_12()
{

	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark-1,m_pBookmark,m_cbBookmark,
		m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark1==length+1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_13()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark+1,m_pBookmark,m_cbBookmark,
		m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark2==length-1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_14()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,m_cbBookmark-1,
		m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark2==length+1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_15()
{
	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,m_cbBookmark+1,
		m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc pBookmark1 points to bogus bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_16()
{	
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;

	if(CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,
		m_cbBookmark,m_pBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc pBookmark2 points to bogus bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Compare::Variation_17()
{
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;

	if(CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,STD_BOOKMARKLENGTH,
		(BYTE *)&DBBookmark,&m_DBCompare),DB_E_BADBOOKMARK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc cbBmk1 != 1, cbBmk2 == 1
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Boundary_Compare::Variation_18()
{ 
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,m_cbBookmark,m_pBookmark,
			STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;

	if(!COMPARE(m_DBCompare, DBCOMPARE_NE))
		return TEST_FAIL;

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc cbBmk1 == 1, cbBmk2 != 1
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Boundary_Compare::Variation_19()
{ 
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;

	if(!CHECK(m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,
			m_cbBookmark,m_pBookmark,&m_DBCompare),S_OK))
		return TEST_FAIL;

	if(!COMPARE(m_DBCompare, DBCOMPARE_NE))
		return TEST_FAIL;

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_Compare::Terminate()
{
	//free the memory used by the bookmark
	PROVIDER_FREE(m_pBookmark);

	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetRowsAt)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary_GetRowsAt - boundary conditions for GetRowsAt
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAt::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	// Create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, 
						IID_IRowsetLocate))
		return FALSE;

	// Get a Bookmark that points to the first row
	m_cbBookmark=1;
	m_DBBookmark=DBBMK_FIRST;
	m_pBookmark=(BYTE *)&m_DBBookmark;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark==0, E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_1()
{
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,0,m_pBookmark,
								0,1,&m_cRowsObtained,&m_pHRow),E_INVALIDARG))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookmark==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_2()
{	
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,NULL,
								0,1,&m_cRowsObtained,&m_pHRow),E_INVALIDARG))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pcRowsObtained==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_3()
{
	// Initialize the output values
	m_pHRow	= NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,
								m_pBookmark,0,1,NULL,&m_pHRow),E_INVALIDARG))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc prgRows==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_4()
{
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,
						m_pBookmark,0,1,&m_cRowsObtained,NULL),E_INVALIDARG))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained,0);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc lRowsOffset==-1. DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_5()
{
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;
	
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,
				m_pBookmark,-1,1,&m_cRowsObtained,&m_pHRow),DB_S_ENDOFROWSET))
		return TEST_FAIL;

	// Since IRowsetLocate is on, DBPROP_CANSCROLLBACKWARDS is TRUE
	COMPARE(GetProperty(DBPROP_CANSCROLLBACKWARDS,
						DBPROPSET_ROWSET,m_pIRowsetLocate), TRUE);

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc cRows=-1, DB_E_CANTFETCHBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_6()
{
	// Initialize the output values
	m_ExpHR			= DB_E_CANTFETCHBACKWARDS;
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;

	if(GetProperty(DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET,m_pIRowsetLocate))
		m_ExpHR = S_OK;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,
						m_pBookmark,0,-1,&m_cRowsObtained,&m_pHRow),m_ExpHR))
		return TEST_FAIL;

	// Check to see if set on output
   	if(m_ExpHR != S_OK)
	{
		COMPARE(m_cRowsObtained, 0);
		COMPARE(m_pHRow, NULL);
	}
	else
		COMPARE(m_cRowsObtained,1);

	CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained, m_pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(m_pHRow);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark=length-1, DB_E_BADBOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_7()
{
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark-1,
					m_pBookmark,0,1,&m_cRowsObtained,&m_pHRow),E_INVALIDARG))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc cbBookmark=length+1; DB_E_BADBOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_8()
{
	// Initialize the output values
	m_cRowsObtained = INVALID(ULONG);
	m_pHRow			= NULL;
	
	//cbBookmark=length +1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark+1,
				m_pBookmark,0,1,&m_cRowsObtained,&m_pHRow),DB_E_BADBOOKMARK))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc pBookmark points to a bogus bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsAt::Variation_9()
{
	// Initialize the output values
	ULONG cbBookmark = 56;
	m_cRowsObtained  = INVALID(ULONG);
	m_pHRow			 = NULL;

	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,(BYTE *)&cbBookmark,
							0,1,&m_cRowsObtained,&m_pHRow),DB_E_BADBOOKMARK))
		return TEST_FAIL;

	// Check to see if set on output
	COMPARE(m_cRowsObtained, 0);
	COMPARE(m_pHRow, NULL);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc cRows == LONG_MAX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Boundary_GetRowsAt::Variation_12()
{ 
	ULONG_PTR	cbBookmark;
	BYTE*		pBookmark = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		pHRow = NULL;
	DBCOUNTITEM	cRowCount;
	BOOL		fTestPass = FALSE;

	//Get the first bookmark
	TESTC(GetBookmark(1,&cbBookmark,&pBookmark));

	 // cRows == LONG_MAX, lRowsOffset = 0
	TEST2C_(m_hr = m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
					0,MAXDBROWCOUNT,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET,E_OUTOFMEMORY);

	//Verify Rows
	if (m_hr == DB_S_ENDOFROWSET)
	{
		TESTC(cRowsObtained == ULONG(g_lRowLast));
		for(cRowCount=0; cRowCount<cRowsObtained; cRowCount++)
			TESTC(VerifyRowPosition(pHRow[cRowCount],cRowCount+1,g_pCTable));
	}

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//releaset the rowhandle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc lRowsOffset == LONG_MAX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Boundary_GetRowsAt::Variation_13()
{ 
	ULONG_PTR	cbBookmark;
	BYTE*		pBookmark = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		pHRow = NULL;
	BOOL		fTestPass = FALSE;

	//Get the first bookmark
	TESTC(GetBookmark(1,&cbBookmark,&pBookmark));

	 // cRows == 1, lRowsOffset = LONG_MAX
	TEST2C_(m_hr = m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
					MAXDBROWCOUNT,1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET,E_OUTOFMEMORY);

	TESTC(cRowsObtained == 0);
	TESTC(pHRow == NULL);

	fTestPass=TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(pBookmark);

	//releaset the rowhandle
	if (pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL, NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsAt::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_GetRowsByBookmarks)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary_GetRowsByBookmarks - boundary conditions for GetRowsByBookmark
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsByBookmarks::Init()
{
	BOOL fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_REVCOLLIST, IID_IRowsetLocate));

	m_rgpBookmarks[0]=NULL;
	m_rgpBookmarks[1]=NULL;
	m_cbBookmark=1;

	m_pHRow=m_rghRows;
	m_DBBookmark=DBBMK_LAST;
					
	//get a valid bookmark value
	if(!GetBookmark(5,&m_cbBookmark, &m_rgpBookmarks[0]))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc rgcbBookmarks==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_1()
{
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_pHRow, NULL),E_INVALIDARG))
		return TEST_FAIL;

	return TEST_PASS;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc prgBookmarks==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_2()
{
	if(CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&m_cbBookmark,NULL,
		m_pHRow, NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pcRowsObtained=NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_3()
{
	if(CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&m_cbBookmark,(const BYTE **)m_rgpBookmarks,
		NULL, NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc prghRows=NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_4()
{
	DBROWSTATUS	DBRowStatus;

	if(CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&m_cbBookmark,(const BYTE **)m_rgpBookmarks,
		NULL,&DBRowStatus),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fReturnErrors==TRUE and pcErrors==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_5()
{
	if(CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,0,&m_cbBookmark,(const BYTE **)m_rgpBookmarks,
		m_pHRow,NULL),S_OK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc fReturnedErrors=TRUE and prgErrors=NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_6()
{
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&m_cbBookmark,(const BYTE **)m_rgpBookmarks,
		m_pHRow,NULL),S_OK))
		return TEST_FAIL;
														  
	CHECK(m_pIRowset->ReleaseRows(1,m_pHRow, NULL,NULL,NULL),S_OK);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc fReturnErrors=FALSE and pcErrors=NULL; S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_7()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	ULONG_PTR		cbBookmark=1;

	//try to retrieve an row by an invalid bookmark
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,
		&cbBookmark,(const BYTE **)&pBookmark,
		m_pHRow,NULL),DB_E_ERRORSOCCURRED))
		return TEST_FAIL;
	
	COMPARE(*m_pHRow, DB_NULL_HROW);

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc fReturnErrors=FALSE and prgErrors=NULl; S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_GetRowsByBookmarks::Variation_8()
{
	DBROWSTATUS		DBRowStatus;

	//only one row handle should be retrieved
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&m_cbBookmark,
		(const BYTE **)m_rgpBookmarks,m_pHRow,
		&DBRowStatus),S_OK))
		return TEST_FAIL;

	CHECK(m_pIRowset->ReleaseRows(1,m_pHRow, NULL,NULL,NULL),S_OK);
		
	//check the row status
	if(COMPARE(DBRowStatus, DBROWSTATUS_S_OK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_GetRowsByBookmarks::Terminate()
{
	//free memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary_Hash)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary_Hash - boundary conditions for Hash
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_Hash::Init()
{
	BOOL fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetLocate));

	m_rgpBookmarks[0]=NULL;
	m_DBBookmark=DBBMK_LAST;

	//get a valid bookmark value
	if(!GetBookmark(3,m_rgcbBookmarks, m_rgpBookmarks))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:
	
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cBookmarks==0, S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_1()
{
	//cBookmark==0, do nothing
	if(CHECK(m_pIRowsetLocate->Hash(NULL,0,m_rgcbBookmarks,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),S_OK))
		return TEST_PASS;

	return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc rgcbBookmarks==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_2()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc rgpBookmark==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_3()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,m_rgcbBookmarks,NULL,
		m_rgHashedValues,NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc rgHashedvalues==NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_4()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,m_rgcbBookmarks,(const BYTE **)m_rgpBookmarks,
		NULL,NULL),E_INVALIDARG))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pcErros is valid and prgErrors=NULL; E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_5()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,m_rgcbBookmarks,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),S_OK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pcErros=NULL, S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_6()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,m_rgcbBookmarks,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),S_OK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pcErros==NULL and prgErrors=NULL; S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary_Hash::Variation_7()
{
	if(CHECK(m_pIRowsetLocate->Hash(NULL,1,m_rgcbBookmarks,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),S_OK))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary_Hash::Terminate()
{
	//free the memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		Zombie - zombie states
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Zombie::Init()
{
	m_DBPropSet.rgProperties=NULL;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!CTransaction::Init())
	// }}
		return TEST_SKIPPED;	

	m_DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	m_DBPropSet.cProperties=2;
	m_DBPropSet.rgProperties=(DBPROP *)PROVIDER_ALLOC(2 * sizeof(DBPROP));

	if(!m_DBPropSet.rgProperties)
		return FALSE;

	//DBPROP_IRowsetLocate                                                                                
	memset(&m_DBPropSet.rgProperties[0], 0, sizeof(DBPROP));
	m_DBPropSet.rgProperties[0].dwPropertyID=DBPROP_IRowsetLocate;                    
   	m_DBPropSet.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_DBPropSet.rgProperties[0].vValue.vt=VT_BOOL;                                        
   	V_BOOL(&m_DBPropSet.rgProperties[0].vValue)=VARIANT_TRUE; 
	
	//DBPROP_CANHOLDROWS                                                                               
	memset(&m_DBPropSet.rgProperties[1], 0, sizeof(DBPROP));
	m_DBPropSet.rgProperties[1].dwPropertyID=DBPROP_CANHOLDROWS;                    
   	m_DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_DBPropSet.rgProperties[1].vValue.vt=VT_BOOL;                                        
   	V_BOOL(&m_DBPropSet.rgProperties[1].vValue)=VARIANT_TRUE; 
   	
	//register interface to be tested                                         
   	if(!RegisterInterface(ROWSET_INTERFACE, IID_IRowsetLocate, 1, &m_DBPropSet)) 
   		return FALSE;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_1()
{
	IRowsetLocate	*pIRowsetLocate=NULL;
	HROW			*pHRow=NULL;
	HROW			HRowByBookmark=NULL;
	DBCOUNTITEM		cRows=1;
	DBROWSTATUS		DBRowStatus;
	DBBOOKMARK		DBBookmarkOne=DBBMK_FIRST;
	BYTE			*pBookmarkOne=(BYTE *)&DBBookmarkOne;
	DBBOOKMARK		DBBookmarkTwo=DBBMK_FIRST;
	BYTE			*pBookmarkTwo=(BYTE *)&DBBookmarkTwo;
	ULONG_PTR		cbBookmark=1;
	DBCOMPARE		dwComparison;
	DBHASHVALUE		HashedValues = NULL;
	BOOL			fTestPass=FALSE;

	//start a transaction
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetLocate,
		1, &m_DBPropSet))
		goto CLEANUP;

	if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
		STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
		goto CLEANUP;

	//the two bookmark should be the same
	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto CLEANUP;

	
	if(!m_fCommitPreserve)
	{
		//zombie
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),E_UNEXPECTED))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),E_UNEXPECTED))
			goto CLEANUP;

		COMPARE(cRows, 0);

		//no row should be retrieved
		COMPARE(pHRow, NULL);

		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;
	}
	else
	{
		//fully functional
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
			goto CLEANUP;

		//the two bookmark should be the same
		if(!COMPARE(dwComparison, DBCOMPARE_EQ))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		//1 row should be retrieved
		COMPARE(cRows, 1);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

	   	//no rows should be retrieved
		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HRowByBookmark,NULL);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HashedValues, NULL);
	}

	fTestPass=TRUE;

CLEANUP:
	//release memory
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(HRowByBookmark)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowByBookmark,NULL,NULL,NULL),S_OK);
	}


	//release the interface pointer
	SAFE_RELEASE(pIRowsetLocate);

	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit without retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_2()
{
	IRowsetLocate	*pIRowsetLocate=NULL;
	HROW			*pHRow=NULL;
	HROW			HRowByBookmark=NULL;
	DBCOUNTITEM		cRows;
	DBROWSTATUS		DBRowStatus;
	DBBOOKMARK		DBBookmarkOne=DBBMK_FIRST;
	BYTE			*pBookmarkOne=(BYTE *)&DBBookmarkOne;
	DBBOOKMARK		DBBookmarkTwo=DBBMK_LAST;
	BYTE			*pBookmarkTwo=(BYTE *)&DBBookmarkTwo;
	DBCOUNTITEM		cbBookmark=1;
	DBCOMPARE		dwComparison;
	DBHASHVALUE		HashedValues = NULL;
	BOOL			fTestPass=FALSE;

	//start a transaction
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetLocate,
		1, &m_DBPropSet))
		goto CLEANUP;

	if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
		STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
		goto CLEANUP;

	//the two bookmark should be the same
	if(!COMPARE(dwComparison, DBCOMPARE_NE))
		goto CLEANUP;

	//commit the transaction with fRetaining==FALSE
	if(!GetCommit(FALSE))
		goto CLEANUP;

	if(!m_fCommitPreserve)
	{
		//zombie
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),E_UNEXPECTED))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),E_UNEXPECTED))
			goto CLEANUP;

		//no row should be retrieved
		COMPARE(pHRow, NULL);

		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;

	   	//no row should be retrieved
		COMPARE(HRowByBookmark, DB_NULL_HROW);

		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;
	}
	else
	{
		//fully functional
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
			goto CLEANUP;

		//the two bookmark should be the same
		if(!COMPARE(dwComparison, DBCOMPARE_NE))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		//1 row should be retrieved
		COMPARE(cRows, 1);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

	   	//no rows should be retrieved
		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HRowByBookmark,NULL);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HashedValues, NULL);
	}

	fTestPass=TRUE;

CLEANUP:
	//release memory
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(HRowByBookmark)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowByBookmark,NULL,NULL,NULL),S_OK);
	}


	//release the interface pointer
	SAFE_RELEASE(pIRowsetLocate);

	//clean up.  Expected XACT_E_NOTRANSACTION.
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_3()
{
	IRowsetLocate	*pIRowsetLocate=NULL;
	HROW			*pHRow=NULL;
	HROW			HRowByBookmark=NULL;
	DBCOUNTITEM		cRows;
	DBROWSTATUS		DBRowStatus;
	DBBOOKMARK		DBBookmarkOne=DBBMK_LAST;
	BYTE			*pBookmarkOne=(BYTE *)&DBBookmarkOne;
	DBBOOKMARK		DBBookmarkTwo=DBBMK_LAST;
	BYTE			*pBookmarkTwo=(BYTE *)&DBBookmarkTwo;
	ULONG_PTR		cbBookmark=1;
	DBCOMPARE		dwComparison;
	DBHASHVALUE		HashedValues = NULL;
	BOOL			fTestPass=FALSE;

	//start a transaction
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetLocate,
		1, &m_DBPropSet))
		goto CLEANUP;

	if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
		STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
		goto CLEANUP;

	//the two bookmark should be the same
	if(!COMPARE(dwComparison, DBCOMPARE_EQ))
		goto CLEANUP;

	//Abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto CLEANUP;

	
	if(!m_fAbortPreserve)
	{
		//zombie
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),E_UNEXPECTED))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),E_UNEXPECTED))
			goto CLEANUP;

		//no row should be retrieved
		COMPARE(pHRow, NULL);

		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;

	   	//no row should be retrieved
		COMPARE(HRowByBookmark, DB_NULL_HROW);

		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;
	}
	else
	{
		//fully functional
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
			goto CLEANUP;

		//the two bookmark should be the same
		if(!COMPARE(dwComparison, DBCOMPARE_EQ))
			goto CLEANUP;

		//DB_E_BADSTARTPOSION - 5 rows after DBBMK_LAST
		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),DB_S_ENDOFROWSET))
			goto CLEANUP;

		//0 rows should be retrieved
		COMPARE(cRows, 0);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

	   	//no rows should be retrieved
		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HRowByBookmark,NULL);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HashedValues, NULL);
	}

	fTestPass=TRUE;

CLEANUP:
	//release memory
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(HRowByBookmark)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowByBookmark,NULL,NULL,NULL),S_OK);
	}

	//release the interface pointer
	SAFE_RELEASE(pIRowsetLocate);

	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort without retaining.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_4()
{
	IRowsetLocate	*pIRowsetLocate=NULL;
	HROW			*pHRow=NULL;
	HROW			HRowByBookmark=NULL;
	DBCOUNTITEM		cRows;
	DBROWSTATUS		DBRowStatus;
	DBBOOKMARK		DBBookmarkOne=DBBMK_LAST;
	BYTE			*pBookmarkOne=(BYTE *)&DBBookmarkOne;
	DBBOOKMARK		DBBookmarkTwo=DBBMK_FIRST;
	BYTE			*pBookmarkTwo=(BYTE *)&DBBookmarkTwo;
	ULONG_PTR		cbBookmark=1;
	DBCOMPARE		dwComparison;
	DBHASHVALUE		HashedValues = NULL;
	BOOL			fTestPass=FALSE;

	//start a transaction
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetLocate,
		1, &m_DBPropSet))
		goto CLEANUP;

	if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
		STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),S_OK))
		goto CLEANUP;

	//the two bookmark should be the same
	if(!COMPARE(dwComparison, DBCOMPARE_NE))
		goto CLEANUP;

	//Abort the transaction with fRetaining==FALSE
	if(!GetAbort(FALSE))
		goto CLEANUP;

	
	if(!m_fAbortPreserve)
	{
		//zombie
		if(!CHECK(pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,pBookmarkOne,
			STD_BOOKMARKLENGTH,pBookmarkTwo,&dwComparison),E_UNEXPECTED))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkOne,
			5,1,&cRows,&pHRow),E_UNEXPECTED))
			goto CLEANUP;

		//no row should be retrieved
		COMPARE(pHRow, NULL);

		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;

	   	//no row should be retrieved
		COMPARE(HRowByBookmark, DB_NULL_HROW);

		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus),E_UNEXPECTED))
			goto CLEANUP;
	}
	else
	{
		//fully functional
		if(!CHECK(pIRowsetLocate->Compare(NULL,1,pBookmarkOne,
			1,pBookmarkTwo,&dwComparison),S_OK))
			goto CLEANUP;

		//the two bookmark should be the same
		if(!COMPARE(dwComparison, DBCOMPARE_NE))
			goto CLEANUP;

		if(!CHECK(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmarkTwo,
			5,1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		//1 row should be retrieved
		COMPARE(cRows, 1);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HRowByBookmark,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

	   	//no rows should be retrieved
		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HRowByBookmark,NULL);

		//DB_E_ERRORSOCCURRED - Cannot be standard bookmarks DBBMK_FIRST
		if(!CHECK(pIRowsetLocate->Hash(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmarkOne,&HashedValues,&DBRowStatus), DB_E_ERRORSOCCURRED))
			goto CLEANUP;

		COMPARE(DBRowStatus, DBROWSTATUS_E_INVALID);
		COMPARE(HashedValues, NULL);
	}

	fTestPass=TRUE;

CLEANUP:
	//release memory
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(HRowByBookmark)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowByBookmark,NULL,NULL,NULL),S_OK);
	}


	//release the interface pointer
	SAFE_RELEASE(pIRowsetLocate);

	//clean up.  Expected XACT_E_NOTRANSACTION.
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
	if(m_DBPropSet.rgProperties)
		PROVIDER_FREE(m_DBPropSet.rgProperties);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Rowset_BigRowset)
//*-----------------------------------------------------------------------
//| Test Case:		Rowset_BigRowset - test basic functionality on a big rowset
//|	Created:		02/12/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_BigRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetLocate::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset_BigRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(keysetCursor)
//*-----------------------------------------------------------------------
//| Test Case:		keysetCursor - test GetRowsAt via a keyset driven cursor
//|	Created:		03/11/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL keysetCursor::Init()
{
	BOOL		fTestPass = FALSE;
	DBPROPID	guidPropertySet[4];
	ULONG		cPrptSet=0;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	guidPropertySet[cPrptSet++]=DBPROP_CANHOLDROWS;	   
	guidPropertySet[cPrptSet++]=DBPROP_OTHERUPDATEDELETE;

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS  and DBPROP_CANHOLDROWS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Bookmark points to the second row.  lRowOffset=-1 cRows=-1.  S_OK is returned and the first row handle is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int keysetCursor::Variation_1()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the second row
	if(!GetBookmark(2,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark, -1, -1,
		&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the first row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 1, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to the third row.  lRowOffset=-1 and cRows=-3.  DB_S_ENDOFROWSET is returned and the 2nd  and 1st row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int keysetCursor::Variation_2()
{
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;
	DBCOUNTITEM	cRowsObtained;
	HROW		*pHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the third row
	if(!GetBookmark(3,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-3
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark, -1, -3,
		&cRowsObtained,&pHRow),DB_S_ENDOFROWSET))
		goto CLEANUP;

	//two rows should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//the second row should be retrieved first
	if(!COMPARE(VerifyRowPosition(*pHRow, 2, g_pCTable),TRUE))
		goto CLEANUP;

	//the first row should be retrieved 
	if(COMPARE(VerifyRowPosition(pHRow[1], 1, g_pCTable),TRUE))
		fTestPass=TRUE;


CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pBookmark);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to the 4th row.  lRowstOffset=-1 and cRows=-3.  S_OK is returned.  Do not release the row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int keysetCursor::Variation_3()
{
	ULONG_PTR	cbFirstBookmark;
	BYTE		*pFirstBookmark=NULL;
	ULONG_PTR	cbSecondBookmark;
	BYTE		*pSecondBookmark=NULL;
	DBCOUNTITEM	cFirstRowsObtained;
	DBCOUNTITEM	cSecondRowsObtained;
	HROW		*pFirstHRow=NULL;
	HROW		*pSecondHRow=NULL;
	BOOL		fTestPass=FALSE;

	//get a bookmark for the 4th row
	if(!GetBookmark(4,&cbFirstBookmark,&pFirstBookmark))
		goto CLEANUP;

	//get a bookmark for the 5th row
	if(!GetBookmark(5,&cbSecondBookmark,&pSecondBookmark))
		goto CLEANUP;

	//lRowOffset=-1 cRows=-3
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbFirstBookmark,pFirstBookmark, -1, -3,
		&cFirstRowsObtained,&pFirstHRow),S_OK))
		goto CLEANUP;

	//three rows should be retrieved
	if(!COMPARE(cFirstRowsObtained, 3))
		goto CLEANUP;

	//the third row is the first row handle to retrieve
	if(!COMPARE(VerifyRowPosition(*pFirstHRow, 3, g_pCTable),TRUE))
		goto CLEANUP;

	// *pBookmark points to the 5th row.  lRowstOffset=-1 and cRows=-1.  
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbSecondBookmark,pSecondBookmark, 
		-1, -1, &cSecondRowsObtained,&pSecondHRow),S_OK))
		goto CLEANUP;

	//only one row should be retrieved
	if(!COMPARE(cSecondRowsObtained, 1))
		goto CLEANUP;

	//the 4th row handle should be retrieved
	if(COMPARE(VerifyRowPosition(*pSecondHRow, 4, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	
	//release the bookmark
	PROVIDER_FREE(pFirstBookmark);
	PROVIDER_FREE(pSecondBookmark);

	//release the row handle
	if(pFirstHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cFirstRowsObtained,pFirstHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pFirstHRow);
	}

	//release the row handle
	if(pSecondHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cSecondRowsObtained,pSecondHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pSecondHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST.  Get one row handle at a time (lRowsetOffset==0 and cRows==1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int keysetCursor::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;		
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW		*pHRow=hRow;
	DBCOUNTITEM	cRowCount;

	//get one row handle at a time, from the first row
	for(cRowCount=0; cRowCount<ULONG(g_lRowLast); cRowCount++)
	{
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,cRowCount,
			1,&cRowsObtained,&pHRow),S_OK))
			return TEST_FAIL;

		//verify the row position
		if(!COMPARE(VerifyRowPosition(hRow[0],cRowCount+1,g_pCTable),TRUE))
		{
			CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
			return TEST_FAIL;
		}
		else
			CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
	}

	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL keysetCursor::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - Extended Errors
//|	Created:		07/23/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL ExtendedErrors::Init()
{	
	//Create an object for checking extended errors, which will use
	//m_pError to increment the error count as needed.
	m_pExtError = new CExtError(m_pThisTestModule->m_ProviderClsid, m_pError);
	
	if (!m_pExtError)
		return FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	return TRUE;
}

 
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowsetLocate calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{  
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	DBHASHVALUE		rgHashedValues[2];
	BOOL			fTestPass=FALSE;
	HRESULT			hr;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;
	
	if(GetProp(DBPROP_ORDEREDBOOKMARKS))
		m_fOrderedBookmark=TRUE;
	
	//get the bookmark for the 30th row
	if(!GetBookmark(g_lRowLast/2,&rgcbBookmarks[0],&rgpBookmarks[0]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	//get the bookmark for the 20th row 
	if(!GetBookmark(g_lRowLast/3,&rgcbBookmarks[1],&rgpBookmarks[1]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}
	
	m_pExtError->CauseError();
	
	//Compare return DBCOMPARE_NE for the different row
	if(CHECK(hr=m_pIRowsetLocate->Compare(NULL,rgcbBookmarks[0],rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),S_OK))
		//Do extended check following Compare
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}
	if(m_fOrderedBookmark)
	{
		if(!COMPARE(dwComparison, DBCOMPARE_GT))
		{
			fTestPass = FALSE;
			goto CLEANUP;
		}
	}
	else
	{
		if(!COMPARE(dwComparison, DBCOMPARE_NE))
		{
			fTestPass = FALSE;
			goto CLEANUP;
		}
	}

	m_pExtError->CauseError();
	
	//Hash returns different values
	if(CHECK(hr=m_pIRowsetLocate->Hash(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHashedValues,NULL),S_OK))
		//Do extended check following Hash
		fTestPass &= XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else 
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	if(rgHashedValues[0]!=rgHashedValues[1])
		fTestPass &=TRUE;

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	
	ReleaseRowsetAndAccessor();
		
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowsetLocate calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained;
	HROW		hRow[1];
	HROW		*pHRow=hRow;
	HRESULT		hr;
	BOOL		fTestPass = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	m_pExtError->CauseError();
	
	// *pBookmark=DBBMK_LAST and IRowsetOffset==0.  cRows=1. 	
	if(CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,
		1,&cRowsObtained,&pHRow),S_OK))
		//Do extended check following GetRowsAt
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}
	//the last row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	if(!CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	//repeat 	
	if(!CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,
		1,&cRowsObtained,&pHRow),S_OK))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	//the last row should be retrieved
	if(!COMPARE(cRowsObtained, 1))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}


	if(!CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

CLEANUP:
	ReleaseRowsetAndAccessor();		

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc valid IRowsetLocate calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{  
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	HROW			rgHRow[2];
	DBREFCOUNT		cRefCount;
	DBROWSTATUS		rgDBRowStatus[2];
	BOOL			fTestPass=FALSE;
	HRESULT			hr;

	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	//get the bookmark for the 5th row
	if(!GetBookmark(5,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 5th row again
	if(!GetBookmark(5,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	//create an error object
	m_pExtError->CauseError();
	
	//get the row handle
	if(CHECK(hr=m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHRow,rgDBRowStatus),S_OK))
		//Do extended check following GetRowsByBookmark
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	if(!(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK) &&
		COMPARE(rgDBRowStatus[1], DBROWSTATUS_S_OK)))
		goto CLEANUP;

	//Release the row handle,  The reference count should be 1
	if(!CHECK(m_pIRowset->ReleaseRows(1,rgHRow,NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	//If the HRows are the same ref count should be 1 after release
	if(!COMPARE(cRefCount,ULONG(((rgHRow[0] == rgHRow[1]) ? 1 : 0))))
		goto CLEANUP;

	//release again
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(rgHRow[1]),NULL,&cRefCount,NULL),S_OK))
		goto CLEANUP;

	fTestPass = TRUE;

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	ReleaseRowsetAndAccessor();		

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTSCROLLBACKWARDS GetRowsAt call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{	
	HRESULT		hr;
	BOOL		fTestPass=FALSE;
	DBPROPID	rgPropertiesUnset[2]; 
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	rgPropertiesUnset[0]= DBPROP_CANSCROLLBACKWARDS;
	rgPropertiesUnset[1]= DBPROP_IRowsetLocate;
	
	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowset,
							0,NULL,2,rgPropertiesUnset,DBACCESSOR_PASSBYREF));

	//get a valid bookmark value, points to the first row
	m_pHRow=&m_hRow;
	m_cbBookmark=1;
	m_DBBookmark=DBBMK_LAST;
	m_pBookmark=(BYTE *)&m_DBBookmark;
	m_cRowsObtained=1;

	// Queryinterface for IRowsetLocate
	if(!VerifyInterface(m_pIRowset, IID_IRowsetLocate,
						ROWSET_INTERFACE,(IUnknown **)&m_pIRowsetLocate))
		goto CLEANUP;

	//create an error object
	m_pExtError->CauseError();

	if(CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,m_pBookmark,-1,1,
		&m_cRowsObtained,&m_pHRow),DB_E_CANTSCROLLBACKWARDS))
	{
		//Do extended check following GetRowsAt
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	
		COMPARE(m_cRowsObtained,0);
	}
   	
CLEANUP:

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG Compare call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	BOOL			fTestPass=FALSE;
	HRESULT			hr;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	//get the bookmark for the 30th row
	if(!GetBookmark(ULONG(g_lRowLast/2),&rgcbBookmarks[0],&rgpBookmarks[0]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	//get the bookmark for the 20th row 
	if(!GetBookmark(ULONG(g_lRowLast/3),&rgcbBookmarks[1],&rgpBookmarks[1]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}
	
	m_pExtError->CauseError();
	
	if(CHECK(hr=m_pIRowsetLocate->Compare(NULL,0,rgpBookmarks[0],
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),E_INVALIDARG))
		//Do extended check following Compare
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

CLEANUP:

	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	
	ReleaseRowsetAndAccessor();
		
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG Hash call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
	HRESULT hr;
	BOOL fTestPass=FALSE;
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	m_rgpBookmarks[0]=NULL;
	m_DBBookmark=DBBMK_LAST;

	//get a valid bookmark value
	if(!GetBookmark(3,m_rgcbBookmarks, m_rgpBookmarks))
		goto CLEANUP;

	m_pExtError->CauseError();
	
	if(CHECK(hr=m_pIRowsetLocate->Hash(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),E_INVALIDARG))
		//Do extended check following Hash
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	

CLEANUP:
	
	//free the memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetRowsByBookMark call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_7()
{	
	HRESULT hr;
	BOOL fTestPass = FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate))
		return TEST_FAIL;

	m_rgpBookmarks[0]=NULL;
	m_rgpBookmarks[1]=NULL;
	m_cbBookmark=1;

	m_pHRow=m_rghRows;
	m_DBBookmark=DBBMK_LAST;


	//get a valid bookmark value
	if(!GetBookmark(5,&m_cbBookmark, &m_rgpBookmarks[0]))
		goto CLEANUP;
   	
	m_pExtError->CauseError();
	
	
	//Do extended check following GetRowsByBookmark
	if(CHECK(hr=m_pIRowsetLocate->GetRowsByBookmark(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_pHRow, NULL),E_INVALIDARG))
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	

CLEANUP:
	
	//free memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	
	ReleaseRowsetAndAccessor();
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTFETCHBACKWARDS GetRowsAt call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_8()
{	
	HRESULT hr;
	BOOL fTestPass=FALSE;
	DBPROPID	rgPropertiesUnset[1]; 
		
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	rgPropertiesUnset[0]= DBPROP_CANFETCHBACKWARDS;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		0,NULL,1,rgPropertiesUnset,DBACCESSOR_PASSBYREF));

	//get a valid bookmark value, points to the first row
	m_pHRow=&m_hRow;
	m_cbBookmark=1;
	m_DBBookmark=DBBMK_FIRST;
	m_pBookmark=(BYTE *)&m_DBBookmark;
	m_cRowsObtained=1;

	//Do extended check following GetRowsAt
	if(CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,m_pBookmark,0,-1,
		&m_cRowsObtained,&m_pHRow),DB_E_CANTFETCHBACKWARDS))
	{
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	
		COMPARE(m_cRowsObtained,0);
	}
  
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBOOKMARK Compare call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_9()
{
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	DBCOMPARE		dwComparison=DBCOMPARE_EQ;
	BOOL			fTestPass=FALSE;
	HRESULT			hr;
	DBBOOKMARK		DBBookmark=DBBMK_INVALID;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.

	//create a rowset and accessor.  DBPROP_BOOKMARKS should be supported if
	//IRowsetLocate is requested on the rowset
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	//get the bookmark for the 30th row
	if(!GetBookmark(ULONG(g_lRowLast/2),&rgcbBookmarks[0],&rgpBookmarks[0]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

	//get the bookmark for the 20th row 
	if(!GetBookmark(ULONG(g_lRowLast/3),&rgcbBookmarks[1],&rgpBookmarks[1]))
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}
	
	if(CHECK(hr=m_pIRowsetLocate->Compare(NULL,STD_BOOKMARKLENGTH,(BYTE *)&DBBookmark,
		rgcbBookmarks[1],rgpBookmarks[1],&dwComparison),DB_E_BADBOOKMARK))
		//Do extended check following Compare
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);
	else
	{
		fTestPass = FALSE;
		goto CLEANUP;
	}

CLEANUP:
	
	//free memory pointed by the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	
	ReleaseRowsetAndAccessor();
		
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG Hash call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_10()
{
	HRESULT hr;
	BOOL fTestPass=FALSE;
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetLocate))
		return TEST_FAIL;

	m_rgpBookmarks[0]=NULL;
	m_DBBookmark=DBBMK_LAST;

	//get a valid bookmark value
	if(!GetBookmark(3,m_rgcbBookmarks, m_rgpBookmarks))
		goto CLEANUP;

	if(CHECK(hr=m_pIRowsetLocate->Hash(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_rgHashedValues,NULL),E_INVALIDARG))
		//Do extended check following Hash
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	

CLEANUP:
	
	//free the memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG GetRowsByBookMark call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_11()
{	
	HRESULT hr;
	BOOL fTestPass = FALSE;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate))
		return TEST_FAIL;

	m_rgpBookmarks[0]=NULL;
	m_rgpBookmarks[1]=NULL;
	m_cbBookmark=1;

	m_pHRow=m_rghRows;
	m_DBBookmark=DBBMK_LAST;


	//get a valid bookmark value
	if(!GetBookmark(5,&m_cbBookmark, &m_rgpBookmarks[0]))
		goto CLEANUP;
   	
	if(CHECK(hr=m_pIRowsetLocate->GetRowsByBookmark(NULL,1,NULL,(const BYTE **)m_rgpBookmarks,
		m_pHRow, NULL),E_INVALIDARG))
		//Do extended check following GetRowsByBookmark
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	

CLEANUP:
	
	//free memory used by the bookmark
	PROVIDER_FREE(m_rgpBookmarks[0]);
	
	ReleaseRowsetAndAccessor();
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc DB_E_BADSTARTPOSTION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_12()
{
	HRESULT hr;
	BOOL fTestPass=FALSE;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetLocate method.
	//We then check extended errors to verify the right extended error behavior.
	
	//create a rowset and accessor
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		0,NULL,0,NULL,DBACCESSOR_PASSBYREF)) 
		return TEST_FAIL;

	//get a valid bookmark value, points to the first row
	m_pHRow=&m_hRow;
	m_cbBookmark=1;
	m_DBBookmark=DBBMK_FIRST;
	m_pBookmark=(BYTE *)&m_DBBookmark;
	m_cRowsObtained=1;
	
	m_pExtError->CauseError();

	if(CHECK(hr=m_pIRowsetLocate->GetRowsAt(NULL,NULL,m_cbBookmark,m_pBookmark,-5,1,
		&m_cRowsObtained,&m_pHRow),DB_S_ENDOFROWSET))
		//Do extended check following GetRowsAt
		fTestPass = XCHECK(m_pIRowsetLocate, IID_IRowsetLocate, hr);	

	COMPARE(m_cRowsObtained,0);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
	//free error object
	if (m_pExtError)
		delete m_pExtError;
	m_pExtError = NULL;
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DeleteRows)
//*-----------------------------------------------------------------------
//| Test Case:		DeleteRows - delete rows from the rowset
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	if (!AlteringRowsIsOK())
		return TEST_SKIPPED;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned and no row handles will be retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_1()
{
	DBPROPID		guidProperty[4];
	HROW			*pHRow=NULL;
	HROW			*pHRowDeleted=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_OTHERUPDATEDELETE;
	guidProperty[2]=DBPROP_UPDATABILITY;
	guidProperty[3]=DBPROP_CANHOLDROWS;

	//set DBPROP_OTHERUPDATELETE is supported

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		4,guidProperty));

	//get the bookmark for the 5th row	
	if(!GetBookmark(5, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the row handle for the 5th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,4,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto CLEANUP;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	// *pBookmark points to a deleted row.  DB_E_BADBOOKMARK
	if(BookmarkSkipped())
	{
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,1,
			&cRowsObtained,&pHRowDeleted), DB_S_BOOKMARKSKIPPED))
			goto CLEANUP;

		if(COMPARE(cRowsObtained, 1) && COMPARE(VerifyRowPosition(*pHRowDeleted,6,g_pCTable),TRUE))
			fTestPass=TRUE;
	}
	else
	{
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,1,
			&cRowsObtained,&pHRowDeleted), DB_E_BADBOOKMARK))
			goto CLEANUP;

		//no row should be retrieved
		if(COMPARE(cRowsObtained,0) && COMPARE(pHRowDeleted, NULL))
			fTestPass=TRUE;
	}

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowDeleted)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDeleted,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDeleted);

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete the 4th row.  *pBookmark points to the second row. lRowOffset=3 and cRows=1.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_2()
{
	DBPROPID		guidProperty[2];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		2,guidProperty));
		
	//get the bookmark for the 2nd row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the row handle for the 4th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 4th row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the second row. lRowOffset=3 and cRows=1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,3,1,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	//the 5th row should be retrieved
	if(RemoveDeleted())
	{
	   	if(COMPARE(VerifyRowPosition(*pHRow, 6, g_pCTable),TRUE))
			fTestPass=TRUE;
	}
	else
	{

		if(COMPARE(VerifyRowPosition(*pHRow, 5, g_pCTable),TRUE))
			fTestPass=TRUE;
	}

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete the 5th row. *pBookmark points to the 4th row. lRowOffset=1 and cRows=1.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_3()
{
	/*  TO DO
    DBPROPID				guidProperty[2] = {DBPROP_IRowsetChange, DBPROP_OTHERUPDATEDELETE};
	ULONG					cProperties = NUMELEM(guidProperty);
	HROW					*pHRow=NULL;
	DBCOUNTITEM				cRowsObtained=0;
	IRowsetChange			*pIRowsetChange=NULL;
	ULONG_PTR				cbBookmark=0, cbDeleteBookmark=0;
	BYTE					*pBookmark=NULL, *pDeleteBookmark=NULL;
	BOOL					fTestPass=FALSE;

	//open rowset and accessor, Request IRowsetDeleteBookmarks and IRowsetLocate
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cProperties,guidProperty))
		return TEST_SKIPPED;

	//get the 5th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//get the bookmark for the 4th row
	if(!GetBookmark(4, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the bookmark for the 5th row
	if(!GetBookmark(5, &cbDeleteBookmark, &pDeleteBookmark))
		goto CLEANUP;

	//QI for IRowsetDelete pointer
	if(!CHECK(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto CLEANUP;

	//delete the 5th row
	if(!CHECK(pIRowsetDelete->DeleteRowsByBookmark(NULL,1,
		&cbDeleteBookmark,(const BYTE **)&pDeleteBookmark,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	// *pBookmark points to the 4th row. lRowOffset=1 and cRows=1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,1,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the 5th row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 5, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pDeleteBookmark);

	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else   
		return TEST_FAIL;
	*/
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Delete the second row.  Pass an array of 3 bookmarks: bookmark to the second row, to the first row, and to the second row.  DB_
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_4()
{	
	DBPROPID		guidProperty[3];
	DBCOUNTITEM		cRows;
	HROW			*pHRow=NULL;
	HROW			rgHRow[3]={NULL, NULL, NULL};
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	ULONG_PTR		ulCount;
	DBROWSTATUS		rgDBRowStatus[3];
	BOOL			fTestPass=FALSE;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_OTHERUPDATEDELETE;
	guidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(guidProperty),guidProperty));

	//get the bookmark for the 2nd row
	if(!GetBookmark(2, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;
	
	if(!GetBookmark(1, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	if(!GetBookmark(2, &rgcbBookmarks[2], &rgpBookmarks[2]))
		goto CLEANUP;
	
	//Restart position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//get the row handle for the 2nd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRows, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_hr=m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 2nd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//Pass an array of 3 bookmarks: bookmark to the second row, 
	//to the first row, and to the second row
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//only one row is retrieved
	if(!COMPARE(rgHRow[0],DB_NULL_HROW) && !COMPARE(rgHRow[2],DB_NULL_HROW))
		goto CLEANUP;

	//Verify the first row is retrieved
	COMPARE(VerifyRowPosition(rgHRow[1], 1, g_pCTable), TRUE);
		

	if(	COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID) && 
		COMPARE(rgDBRowStatus[1], DBROWSTATUS_S_OK) &&
		COMPARE(rgDBRowStatus[2], DBROWSTATUS_E_INVALID)  
	   )
	   fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	for(ulCount=0; ulCount<3; ulCount++)
		PROVIDER_FREE(rgpBookmarks[ulCount]);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Get the 3rd row handle.  GetRowsAt.  Delete the 3rd row.  GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_5()
{
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRows;
	DBPROPID	rgDBPropID[4]={DBPROP_IRowsetChange,
								DBPROP_UPDATABILITY,
								DBPROP_CANSCROLLBACKWARDS,
								DBPROP_CANFETCHBACKWARDS,
								};
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL		fTestPass=FALSE;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgDBPropID),rgDBPropID));

	//get the 3rd row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//getNextRows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//the 4th row should be retrieved
	if(!COMPARE(VerifyRowPosition(*pHRow, 4,g_pCTable), TRUE))
		goto CLEANUP;


	//release
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	PROVIDER_FREE(pHRow);
	pHRow=NULL;


	//restartposition
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	if(RemoveDeleted())
	{
		//position after the deleted row
		if(!CHECK(m_pIRowset->GetNextRows(NULL,2,-1,&cRows,&pHRow),S_OK))
			goto CLEANUP;
	}
	else
	{
		//position after the deleted row
		if(!CHECK(m_pIRowset->GetNextRows(NULL,3,-1,&cRows,&pHRow),S_OK))
			goto CLEANUP;
	}

	//release
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//get the 4th row 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,-1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//the 4th row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 4,g_pCTable), TRUE))
		fTestPass=TRUE;


CLEANUP:
	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the interface pointer
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();
	
	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Position on 5th row.  Delete 3rd row.  Position on 3rd row.  delete 5th row.  Position on 5th row.  Fetch from the 5th row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_6()
{
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRows;
	HROW		*pGetHRow=NULL;
	DBPROPID	rgDBPropID[5]={DBPROP_IRowsetChange,
								DBPROP_UPDATABILITY,
								DBPROP_CANHOLDROWS,
								DBPROP_CANFETCHBACKWARDS,
								DBPROP_CANSCROLLBACKWARDS,
								};
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;
	

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgDBPropID),rgDBPropID));

	//get the 3rd row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//get the last row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-4,1,&cRows,&pGetHRow),S_OK))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release
	if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	if(!RemoveDeleted())
	{
		//delete the last row
		if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pGetHRow,NULL),S_OK))
			goto CLEANUP;
	}


	//restartposition
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	if(RemoveDeleted())
	{
		//get the 4th row handle
		if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK))
			goto CLEANUP;
			  
	   	if(!COMPARE(VerifyRowPosition(*pHRow,4,g_pCTable),TRUE))
			goto CLEANUP;

		//release
		if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			goto CLEANUP;
		PROVIDER_FREE(pHRow);
		pHRow=NULL;

		//delete the last row
		if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pGetHRow,NULL),S_OK))
			goto CLEANUP;

		//try to get the last row
		if(CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-4,-1,&cRows,&pHRow),DB_S_ENDOFROWSET))
			fTestPass=TRUE;

	}
	else
	{
		//get the 3rd row handle
		if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		//release
		if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			goto CLEANUP;
		PROVIDER_FREE(pHRow);
		pHRow=NULL;

		//try to get the last row
		if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-3,-1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		//release
		if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			goto CLEANUP;
		PROVIDER_FREE(pHRow);
		pHRow=NULL;

		//try to get the 1st row
		if(!CHECK(m_pIRowset->GetNextRows(NULL,-(g_lRowLast-2),-1,&cRows,&pHRow),S_OK))
			goto CLEANUP;

		if(COMPARE(VerifyRowPosition(*pHRow,1,g_pCTable),TRUE))
			fTestPass=TRUE;

	}
CLEANUP:
	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the row handle
	if(pGetHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pGetHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pGetHRow);
	}

	//release the interface pointer
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Delete the 1st row.  GetRowsAt based on standard bookmarks.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_7()
{
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRows;
	HROW		*pGetHRow=NULL;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBPROPID	rgDBPropID[5]={DBPROP_IRowsetChange,
								DBPROP_UPDATABILITY,
								DBPROP_CANHOLDROWS,
								DBPROP_CANFETCHBACKWARDS,
								DBPROP_CANSCROLLBACKWARDS,
								};
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=FALSE;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgDBPropID),rgDBPropID));

	//get the 1st row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 1st row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//call get RowsAt
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		0,2,&cRows,&pGetHRow),S_OK))
		goto CLEANUP;

	if(RemoveDeleted())
	{
		//the second row handle is the seond row
		if(COMPARE(VerifyRowPosition(pGetHRow[1],3,g_pCTable),TRUE))
			fTestPass=TRUE;
	}
	else
	{
		//the second row handle is the third row
		if(COMPARE(VerifyRowPosition(pGetHRow[1],2,g_pCTable),TRUE))
			fTestPass=TRUE;
	}

CLEANUP:
	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the row handle
	if(pGetHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(2,pGetHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pGetHRow);
	}

	//release the interface pointer
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();
	
	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Delete the last row.  Fetch backwards one row handle.  Fetch backwards again.  GetRowsAt based on stand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_8()
{
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cRows;
	HROW		*pGetHRow=NULL;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	DBPROPID	rgDBPropID[5]={DBPROP_IRowsetChange,
								DBPROP_UPDATABILITY,
								DBPROP_CANHOLDROWS,
								DBPROP_CANFETCHBACKWARDS,
								DBPROP_CANSCROLLBACKWARDS,
								};
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL		fTestPass=FALSE; 

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgDBPropID),rgDBPropID));

	//get the last row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,-1,&cRows,&pHRow),S_OK))
		goto CLEANUP;
	
	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the last row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//fetch backwards 2 rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,-1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	//should fetch row g_lRowLast-2
	if(!COMPARE(VerifyRowPosition(*pHRow,g_lRowLast-2,g_pCTable),TRUE))
		goto CLEANUP;

	//call GetRowsAt
	if(RemoveDeleted())
	{
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		-1,-1,&cRows,&pGetHRow),S_OK))
		goto CLEANUP;
	}
	else
	{
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,
		-2,-1,&cRows,&pGetHRow),S_OK))
		goto CLEANUP;
	}

	//should fetch the g_lRowLast-2 row
	if(!COMPARE(VerifyRowPosition(*pGetHRow,g_lRowLast-2,g_pCTable),TRUE))
		goto CLEANUP;

	//delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pGetHRow,NULL),S_OK))
		goto CLEANUP;

	//release row handles
	CHECK(m_pIRowset->ReleaseRows(1,pGetHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pGetHRow);
	pGetHRow=NULL;


	//get next rows
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,-1,&cRows,&pGetHRow),S_OK))
		goto CLEANUP;  

	//should fetch the g_lRowLast-3 row
	if(COMPARE(VerifyRowPosition(*pGetHRow,g_lRowLast-3,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the row handle
	if(pGetHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pGetHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pGetHRow);
	}

	//release the interface pointer
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();
	
	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(RemoveDeleted)
//*-----------------------------------------------------------------------
//| Test Case:		RemoveDeleted - test DBPROP_REMOVEDELETED
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Init()
{	
	// {{ TCW_INIT_BASECLASS_CHECK
	if (!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	if (!AlteringRowsIsOK())
		return TEST_SKIPPED;

	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark points to a deleted row.  DB_E_BADBOOKMARK is returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_1()
{
	DBPROPID		rgguidProperty[3];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the 4th row
	if(!GetBookmark(4, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the row handle for the 4th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	if (!GetProp(DBPROP_BOOKMARKSKIPPED))
	{
		// *pBookmark points to a deleted row.  DB_E_BADBOOKMARK
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,1,
			&cRowsObtained,&pHRow), DB_E_BADBOOKMARK))
			goto CLEANUP;

		//no row should be retrieved
		if(COMPARE(cRowsObtained,0) && COMPARE(pHRow, NULL))
			fTestPass=TRUE;
	}
	else
	{
		// *pBookmark points to a deleted row.  DB_E_BADBOOKMARK
		if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,1,
			&cRowsObtained,&pHRow), DB_S_BOOKMARKSKIPPED))
			goto CLEANUP;

		//no row should be retrieved
		if(COMPARE(cRowsObtained,1) && COMPARE(VerifyRowPosition(*pHRow,5,g_pCTable),TRUE))
			fTestPass=TRUE;
	}

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	
	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET
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
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_REMOVEDELETED;
  	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark middle row
	if(!GetBookmark(ULONG(g_lRowLast/2), &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the row handle for the middle row + 1
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(g_lRowLast/2),1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the middle row. lRowOffset=1 and cRows=2
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,2,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	//2 rows should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, (g_lRowLast/2)+2, g_pCTable),TRUE))
		goto CLEANUP;
	
	if(COMPARE(VerifyRowPosition(pHRow[1],(g_lRowLast/2)+3,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete the 4th row. *pBookmark points to the 3th row. lRowOffset=1 and cRows=1.  S_OK is returned and the 5th row handle is ret
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_3()
{
	DBPROPID				rgguidProperty[4];
	HROW					*pHRow=NULL;
	HROW					*pHRowDeleted=NULL;
	DBCOUNTITEM				cRowsObtained=0;
	IRowsetChange			*pIRowsetChange=NULL;
	ULONG_PTR				cbBookmark;
	BYTE					*pBookmark=NULL;
	BOOL					fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_CANHOLDROWS;
	rgguidProperty[3]=DBPROP_UPDATABILITY;
	
	//open rowset and accessor, Request IRowsetChangeBookmarks and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the g_lRowLast-2
	if(!GetBookmark(g_lRowLast-2, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get g_lRowLast-1 row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-2,1,&cRowsObtained,&pHRowDeleted),S_OK))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the g_lRowLast-1 row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRowDeleted,NULL),S_OK))
		goto CLEANUP;

	// *pBookmark points to the g_lRowLast-2 row. lRowOffset=1 and cRows=2
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,2,
		&cRowsObtained,&pHRow), DB_S_ENDOFROWSET))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the last row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, g_lRowLast, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowDeleted)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDeleted,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowDeleted);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Delete the first row.  Pass an array of bookmarks: bookmark to the first row and the second row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_4()
{
	DBPROPID				rgguidProperty[2] = { DBPROP_IRowsetChange, DBPROP_UPDATABILITY};
	HROW					*pHRow=NULL;
	HROW					rgHRow[2] = { DB_NULL_HROW, DB_NULL_HROW };
	IRowsetChange			*pIRowsetChange=NULL;
	DBCOUNTITEM				cRowsObtained=0;
	ULONG_PTR				rgcbBookmarks[2];
	BYTE					*rgpBookmarks[2]={NULL,NULL};
	DBROWSTATUS				rgRowStatus[2];
	BOOL					fTestPass=FALSE;
	DBCOUNTITEM				ulRowEarly=0, ulRowAfter=0;

	ulRowEarly = (g_lRowLast-5);
	ulRowAfter = (g_lRowLast-4);

	//open rowset and accessor, Request IRowsetChangeBookmarks and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));	

	//get the bookmark for the 14th row
	if(!GetBookmark(ulRowEarly, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;

	//get the bookmark for the 15th row
	if(!GetBookmark(ulRowAfter, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//get 15th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,ulRowAfter-1,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 15th row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	if(!CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK))
		goto CLEANUP;
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//Pass an array of bookmarks: bookmark to the 14th row and the 15th row.
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHRow,rgRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	if(!COMPARE(rgHRow[1],DB_NULL_HROW))
		goto CLEANUP;

	if(!COMPARE(rgRowStatus[0],DBROWSTATUS_S_OK) ||
		!COMPARE(rgRowStatus[1], DBROWSTATUS_E_INVALID))
		goto CLEANUP;
	
	//the 14th row should be retrieved
	if(COMPARE(VerifyRowPosition(rgHRow[0], ulRowEarly, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChangeBookmark pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}


	if(rgHRow[1])
		CHECK(m_pIRowset->ReleaseRows(1,&(rgHRow[0]),NULL,NULL,NULL),S_OK);

	//release the bookmark
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Delete the third row.  Pass an array of two bookmarks: bookmark to the second and 4th row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_5()
{
	DBPROPID		rgguidProperty[3];
	HROW			*pHRow=NULL;
	HROW			rgHRow[2]={NULL, NULL};
	DBCOUNTITEM		cRows=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL,NULL};
	ULONG_PTR		ulCount;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the 2nd and 4th row
	if(!GetBookmark(2, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;
	
	if(!GetBookmark(4, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP;

	//get the row handle for the 3rd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRows, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 3rd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//Pass an array of 2 bookmarks: bookmark to the second row, 
	//and the 4th row
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHRow,NULL),
		S_OK))
		goto CLEANUP;

	//Verify the 2nd row is retrieved
	if(!COMPARE(VerifyRowPosition(rgHRow[0], 2, g_pCTable),TRUE))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(rgHRow[1], 4, g_pCTable),TRUE))
	   fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	
	if(rgHRow[0])
		CHECK(m_pIRowset->ReleaseRows(2, rgHRow,NULL,NULL,NULL),S_OK);

	//release the bookmark
	for(ulCount=0; ulCount<2; ulCount++)
		PROVIDER_FREE(rgpBookmarks[ulCount]);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Delete 2nd row.  pBookmark points to 4th row.  cRows=-2, lOffset=-1.  Verify 3rd and 1st row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RemoveDeleted::Variation_6()
{ 
	DBPROPID		rgguidProperty[3];
	HROW*			pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange*	pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE*			pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_REMOVEDELETED;
  	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the 4th row
	TESTC(GetBookmark(4, &cbBookmark, &pBookmark));

	//get the row handle for the 2nd row
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRowsObtained,&pHRow),S_OK);
	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	TESTC(SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,(void **)&pIRowsetChange)));

	//delete the 2nd row
	TESTC_(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	// *pBookmark points to the middle row. lRowOffset=1 and cRows=2
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,-1,-2,
		&cRowsObtained,&pHRow), S_OK);

	//2 rows should be retrieved
	TESTC(cRowsObtained == 2);

	TESTC(VerifyRowPosition(pHRow[0], 3, g_pCTable));
	TESTC(VerifyRowPosition(pHRow[1], 1, g_pCTable));
	
	fTestPass = TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(BookmarkSkipped)
//*-----------------------------------------------------------------------
//| Test Case:		BookmarkSkipped - Test DBPROP_BOOKMARKSKIPPPED
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BookmarkSkipped::Init()
{
	BOOL		fTestPass = FALSE;
	DBPROPID	DBPropID=DBPROP_BOOKMARKSKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	if (!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	if (!AlteringRowsIsOK())
		return TEST_SKIPPED;

	//open rowset, and accessor.DBPROP_BOOKMARKSKIPPED should be not settable
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,1,&DBPropID))
		odtLog<<L"DBPORP_BOOKMARKSKIPPED is not settable!\n";

	ReleaseRowsetAndAccessor();

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_COLLISTFROMTBL, IID_IRowsetLocate,
		0,NULL));

	//return if the bookmarkskipped is not settable and variant-false
	if ( !GetProp(DBPROP_BOOKMARKSKIPPED) )
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=5.  DB_S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_1()
{
	DBPROPID		rgguidProperty[4];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRowCount;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_OTHERUPDATEDELETE;
	rgguidProperty[3]=DBPROP_UPDATABILITY;
	
	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	if(!TCIRowsetLocate::BookmarkSkipped())
		goto CLEANUP;

	//get the row handle for the 1st row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 1st row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the deleted 1st row.  
	//lRowOffset=0 and cRows=5.  S_OK
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,5,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	//5 row should be retrieved
	if(!COMPARE(cRowsObtained,5))
		goto CLEANUP;

	//verify the rows retrieved	is in the ordr of 2,3,4,5, 6
	for(cRowCount=0;cRowCount<cRowsObtained;cRowCount++)
	{
		if(!COMPARE(VerifyRowPosition(pHRow[cRowCount],cRowCount+2,g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete the last row.  *pBookmark=DBBML_LAST and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_2()
{
	DBPROPID		rgguidProperty[3];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the row handle for the last row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,g_lRowLast-1,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the last row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the deleted last row.  
	//lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	//no row should be retrieved
	if(COMPARE(cRowsObtained,1) && COMPARE(VerifyRowPosition(*pHRow,g_lRowLast-1,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete the first row.  *pBookmark=DBBMK_FISRT and lRowOffset=0 and cRows=1.  DB_S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_3()
{
	DBPROPID		rgguidProperty[3];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0, cbBookmark=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the row handle for the 1st row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//get the bookmark for the 1st
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 1st row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the deleted 1st row.  
	//lRowOffset=0 andcRows=1.  DB_S_BOOKMARKSKIPPED
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,1,
		&cRowsObtained,&pHRow), DB_S_BOOKMARKSKIPPED))
		goto CLEANUP;

	//1 row should be retrieved
	if(!COMPARE(cRowsObtained,1))
		goto CLEANUP;

	//verify the rows retrieved	is the 2nd row handle
	if(COMPARE(VerifyRowPosition(*pHRow,2,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	SAFE_FREE(pBookmark);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Delete the second row.  *pBookmark points to the second row and lRowOffset=1 and cRows=1.  DB_S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_4()
{
	DBPROPID				rgguidProperty[3];
	HROW					*pHRow=NULL;
	DBCOUNTITEM				cRowsObtained=0;
	IRowsetChange			*pIRowsetChange=NULL;
	ULONG_PTR				cbBookmark;
	BYTE					*pBookmark=NULL;
	BOOL					fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_CANHOLDROWS;

	//open rowset and accessor, Request IRowsetChangeBookmarks and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the 2nd 
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the 2nd row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL, 1,1,&cRowsObtained, &pHRow),S_OK))
		goto CLEANUP;

	//QI for IRowsetChangepointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 2nd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	// *pBookmark points to the 2nd row. lRowOffset=1 and cRows=1
	//DB_S_BOOKMARKSIPPED
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,1,
		&cRowsObtained,&pHRow), DB_S_BOOKMARKSKIPPED))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the 3rd row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 3, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChangeBookmark pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Delete the 5th row.  Pass an array of two bookmarks: the bookmark to the 4th and 5th row. DB_S_ERRORSOCCURED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_5()
{
	DBPROPID		rgguidProperty[4];
	HROW			*pHRow=NULL;
	HROW			rgHRow[2];
	DBCOUNTITEM		cRows;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL,NULL};
	ULONG_PTR		ulCount;
	DBROWSTATUS		rgDBRowStatus[2];
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_OTHERUPDATEDELETE;
	rgguidProperty[3]=DBPROP_UPDATABILITY;

	//init
	rgHRow[1]=NULL;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));		

	//get the bookmark for the 5nd and 4th row
	if(!GetBookmark(5, &rgcbBookmarks[0], &rgpBookmarks[0]))
		goto CLEANUP;
	
	if(!GetBookmark(4, &rgcbBookmarks[1], &rgpBookmarks[1]))
		goto CLEANUP; 

	//get the row handle for the 5th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,4,1,&cRows,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRows, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 5th row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//Pass an array of 2 bookmarks: bookmark to the 5th row, 
	//and the 4th row
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHRow,rgDBRowStatus),
		DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//one rows is retrieved
	if(!COMPARE(rgHRow[0],DB_NULL_HROW))
		goto CLEANUP;

	//Verify the 4th row is retrieved
	if(!COMPARE(VerifyRowPosition(rgHRow[1], 4, g_pCTable),TRUE))
		goto CLEANUP;


	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID) &&
		COMPARE(rgDBRowStatus[1],DBROWSTATUS_S_OK))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(rgHRow[1])
		CHECK(m_pIRowset->ReleaseRows(1,&(rgHRow[1]),NULL,NULL,NULL),S_OK);

	//release the bookmark
	for(ulCount=0; ulCount<2; ulCount++)
		PROVIDER_FREE(rgpBookmarks[ulCount]);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Delete third row. *pBookmark=3rd row, lRowsOffset=-1, cRows=1, DB_S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int BookmarkSkipped::Variation_6()
{ 
	DBPROPID				rgguidProperty[3];
	HROW*					pHRow=NULL;
	DBCOUNTITEM				cRowsObtained=0;
	IRowsetChange*			pIRowsetChange=NULL;
	ULONG_PTR				cbBookmark;
	BYTE*					pBookmark=NULL;
	BOOL					fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[2]=DBPROP_CANHOLDROWS;

	//open rowset and accessor, Request IRowsetChangeBookmarks and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the 3rd row
	TESTC(GetBookmark(3, &cbBookmark, &pBookmark));

	//get the 2nd row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 2,1,&cRowsObtained, &pHRow),S_OK);

	//QI for IRowsetChangepointer
	TESTC(SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)));

	//delete the 3rd row
	TESTC_(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);

	// *pBookmark points to the 2nd row. lRowOffset=-1 and cRows=-1
	//DB_S_BOOKMARKSIPPED
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,-1,-1,
		&cRowsObtained,&pHRow), DB_S_BOOKMARKSKIPPED);

	TESTC(cRowsObtained == 1);

	//the 2nd row should be retrieved
	//Note that the skipped bookmark counts as one of the skipped rows
	//with respect to lRowsOffset
	TESTC(VerifyRowPosition(pHRow[0], 2, g_pCTable));
	fTestPass = TEST_PASS;

CLEANUP:
	//release IRowsetChangeBookmark pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL BookmarkSkipped::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(RemoveDeleted_BookmarkSkipped)
//*-----------------------------------------------------------------------
//| Test Case:		RemoveDeleted_BookmarkSkipped - Test DBPROP_REMOVEDELETED + DBPROP_BOOKMARKSKIPPED
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_BookmarkSkipped::Init()
{
	BOOL fTestPass = FALSE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if (!TCIRowsetLocate::Init())
	// }}
		return FALSE;
		
	if (!AlteringRowsIsOK())
		return TEST_SKIPPED;

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		0,NULL));

	//Return if the bookmarkskipped is not settable and variant-false
	if (!GetProp(DBPROP_BOOKMARKSKIPPED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=0 and cRows=4.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted_BookmarkSkipped::Variation_1()
{
	DBPROPID		rgguidProperty[5];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0, cbBookmark=0;
	IRowsetChange	*pIRowsetChange=NULL;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[3]=DBPROP_OTHERUPDATEDELETE;
	rgguidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the row handle for the 1st and 2nd row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 2);

	//get the bookmark for the 1st
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 1st and 2nd row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,2,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the 1st row. lRowOffset=0 and cRows=
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,3,
		&cRowsObtained,&pHRow), DB_S_BOOKMARKSKIPPED))
		goto CLEANUP;

	//the 3rd, 4th and 5th row should be retrieved 
	if(!COMPARE(cRowsObtained, 3))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(pHRow[0], 3, g_pCTable),TRUE))
		goto CLEANUP;
	
	if(!COMPARE(VerifyRowPosition(pHRow[1], 4, g_pCTable),TRUE))
		goto CLEANUP;

	if(COMPARE(VerifyRowPosition(pHRow[2], 5, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete the first and the second row.  *pBookmark=DBBMK_FIRST and lRowOffset=1 and cRows=1.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted_BookmarkSkipped::Variation_2()
{
	DBPROPID				rgguidProperty[5];
	HROW					*pHRow=NULL;
	HROW					*pHRowDeleted=NULL;
	DBCOUNTITEM				cRowsObtained=0, cbBookmark=0;
	IRowsetChange			*pIRowsetChange=NULL;
	BYTE					*pBookmark=NULL;
	BOOL					fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]=DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[3]=DBPROP_OTHERUPDATEDELETE;
	rgguidProperty[4]=DBPROP_CANHOLDROWS;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//get the bookmark for the 1st
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the 1st and 2nd row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRowsObtained,&pHRowDeleted),S_OK))
		goto CLEANUP;

	//delete the 1st and 2nd row handles
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,2,pHRowDeleted,NULL),S_OK))
		goto CLEANUP;

	// *pBookmark=DBBMK_FIRST and lRowOffset=2 and cRows=1.  
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,
		2,1,&cRowsObtained, &pHRow),DB_S_BOOKMARKSKIPPED))
		goto CLEANUP;

	if(!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//the 4th row should be retrieved
	if(COMPARE(VerifyRowPosition(*pHRow, 4, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowDeleted)
	{
		CHECK(m_pIRowset->ReleaseRows(2,pHRowDeleted,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowDeleted);
	}

	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete the 3th row.  *pBookmark points to the second row. lRowOffset=1 and cRows=3.  DB_S_ENDOFROWSET is returned and the 4th a
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted_BookmarkSkipped::Variation_3()
{
	DBPROPID		rgguidProperty[4];
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	IRowsetChange	*pIRowsetChange=NULL;
	ULONG_PTR		cbBookmark;
	BYTE			*pBookmark=NULL;
	BOOL			fTestPass=FALSE;

	rgguidProperty[0]=DBPROP_IRowsetChange;
	rgguidProperty[1]= DBPROP_REMOVEDELETED;
	rgguidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	rgguidProperty[3]=DBPROP_UPDATABILITY;

	//open rowset and accessor, Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(rgguidProperty),rgguidProperty));

	//get the bookmark for the 29th row
	if(!GetBookmark(ULONG(g_lRowLast/2)-1, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//get the row handle for the 30th row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,ULONG(g_lRowLast/2)-1,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowsetLocate->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange)))
		goto CLEANUP;

	//delete the 30th row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	// *pBookmark points to the 29th row. lRowOffset=1 and cRows=2
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,1,2,
		&cRowsObtained,&pHRow), S_OK))
		goto CLEANUP;

	//the 31stth and 32nd row should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(*pHRow, ULONG(g_lRowLast/2)+1, g_pCTable),TRUE))
		goto CLEANUP;
	
	if(COMPARE(VerifyRowPosition(pHRow[1],ULONG(g_lRowLast/2)+2,g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release IRowsetChange pointer 
	SAFE_RELEASE(pIRowsetChange);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the bookmark
	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();

	if( TEST_SKIPPED != fTestPass )
		PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted_BookmarkSkipped::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ChangeRows)
//*-----------------------------------------------------------------------
//| Test Case:		ChangeRows - change rows in the rowset
//|	Created:		02/07/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ChangeRows::Init()
{
	BOOL fTestPass = FALSE;
	DBPROPID	guidPropertySet[3];

	m_pIRowsetChange=NULL;

	guidPropertySet[0]=DBPROP_IRowsetChange;
	guidPropertySet[1]=DBPROP_UPDATABILITY;
	guidPropertySet[2]=DBPROP_OTHERUPDATEDELETE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if (!TCIRowsetLocate::Init())
	// }}
		return FALSE;

	if (!AlteringRowsIsOK())
		return TEST_SKIPPED;

	//create a rowset and  create an accessor.  Set IRowsetChange
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		NUMELEM(guidPropertySet), guidPropertySet,0,NULL,DBACCESSOR_PASSBYREF));

	//QI for IRowsetChange pointer
	if(!SUCCEEDED(m_pIRowset->QueryInterface(IID_IRowsetChange,
		(void **)&m_pIRowsetChange)))
		return TEST_SKIPPED;

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Change the first row and the 3rd row.  *pBookmark=DBBMK_FISRT and lRowOffset=1 and cRow=1.  The 3rd row is retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ChangeRows::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	DBCOUNTITEM		cRowCount=0;
	BOOL			fTestPass=FALSE;

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

	//change the first and third row
	for(cRowCount=1; cRowCount<=3; cRowCount=cRowCount+2)
	{
		if(cRowCount==3)
		{
			//get the third row handle
			if(!CHECK(m_pIRowset->GetNextRows(NULL,cRowCount-2,1,&cRowsObtained,&pHRow),S_OK))
				goto CLEANUP;
		}
		else
		{
			 	//get the first row handle
			if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&pHRow),S_OK))
				goto CLEANUP;
		}

		//get data for the row
		if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
			goto CLEANUP;

		//set data for the row
		if(!CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,m_pData),S_OK))
			goto CLEANUP;

		//release the memory from GetData
		FreeMemory();

		//make sure set data is set the correct data
		COMPARE(VerifyRowPosition(*pHRow, cRowCount, g_pCTable),TRUE);

		//release the row handle
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);   
		pHRow=NULL;
	}			

	//pBookmark=DBBMK_FIRST and lRowOffset=1, cRow=1
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,1,1,&cRowsObtained,
		&pHRow),S_OK))
		goto CLEANUP;

	//verify the third row is retrieved
	if(COMPARE(VerifyRowPosition(*pHRow,2, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseAccessorOnRowset();
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Change the second row.  *pBookmark points to the second row.  lRowOffset=0 and cRow=2.  The 3rd and 4th row handles are retriev
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ChangeRows::Variation_2()
{
	ULONG_PTR		cbBookmark=0;
	BYTE			*pBookmark=NULL;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,ALL_COLS_BOUND))
		goto CLEANUP;

	//get the bookmark for the second row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
		goto CLEANUP;

	//restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	ReleaseAccessorOnRowset();

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;


	//change the second row
	//get the row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//get data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
		goto CLEANUP;

	//set data for the row
	if(!CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,m_pData),S_OK))
		goto CLEANUP;

	//release the memory from GetData
	FreeMemory();

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);   
	pHRow=NULL;

	// *pBookmark points to the second row.  lRowOffset=0 and cRow=2.	
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,cbBookmark,pBookmark,0,2,
		&cRowsObtained, &pHRow),S_OK))
		goto CLEANUP;

	//the 2nd and third row should be retrieved
	if(!COMPARE(cRowsObtained, 2))
		goto CLEANUP;

	//verify the third row is retrieved	in the second slot
	if(COMPARE(VerifyRowPosition(pHRow[1], 3, g_pCTable),TRUE))
		fTestPass=TRUE;

CLEANUP:
	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	
	//release the bookmark
	PROVIDER_FREE(pBookmark);

	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Change the 1st and the last row.  Pass an array of 3 bookmarks: bookmark to the first row, to the last row, and bookmark to the
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ChangeRows::Variation_3()
{
	ULONG_PTR		rgcbBookmarks[3];
	DBCOUNTITEM		rgcRowCount[3]={1, g_lRowLast, 3};
	DBCOUNTITEM		cRowCount=0;
	HROW			rgHRow[3]={NULL,NULL,NULL};
	BYTE			*rgpBookmarks[3]={NULL, NULL, NULL};
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRowsObtained=0;
	BOOL			fTestPass=FALSE;

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,ALL_COLS_BOUND))
		goto CLEANUP;

	//get the bookmark for the 1st, last and third row
	for(cRowCount=0; cRowCount<3; cRowCount++)
	{
		if(!GetBookmark(rgcRowCount[cRowCount],&rgcbBookmarks[cRowCount],
			&rgpBookmarks[cRowCount]))
			goto CLEANUP;
	}

	ReleaseAccessorOnRowset();

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;


	//restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	//change the first and last row
	for(cRowCount=0; cRowCount<2; cRowCount=cRowCount+1)
	{
		//restartPosition
		if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
			goto CLEANUP;

		//get the row handle
		if(!CHECK(m_pIRowset->GetNextRows(NULL,rgcRowCount[cRowCount]-1,1,
			&cRowsObtained,&pHRow),S_OK))
			goto CLEANUP;

		//get data for the row
		if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
			goto CLEANUP;

		//set data for the row
		if(!CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,m_pData),S_OK))
			goto CLEANUP;

		//release the memory from GetData
		FreeMemory();

		//release the row handle
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);   
		pHRow=NULL;
	}

	// Pass an array of 3 bookmarks: bookmark to the first row, to the last row, 
	// and bookmark to the third row
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,3,rgcbBookmarks,(const BYTE **)rgpBookmarks,
		rgHRow,NULL),S_OK))
		goto CLEANUP;

	//verify the 1st, last and the third row is retrieved
	for(cRowCount=0; cRowCount<3; cRowCount++)
	{
		if(!COMPARE(VerifyRowPosition(rgHRow[cRowCount], rgcRowCount[cRowCount], g_pCTable),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(rgHRow[0])
		CHECK(m_pIRowset->ReleaseRows(3, rgHRow,NULL,NULL,NULL),S_OK);
	
	//free the bookmarks
	for(cRowCount=0; cRowCount<3; cRowCount++)
		PROVIDER_FREE(rgpBookmarks[cRowCount]);

	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Change the second row.  Pass an array of 2 bookmarks: invalid and bookmark to the second row.  DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ChangeRows::Variation_4()
{
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	HROW			*pHRow=NULL;
	HROW			rgHRow[2];
	DBCOUNTITEM		cRowsObtained=0;
	DBROWSTATUS		rgDBRowStatus[2];
	BOOL			fTestPass=FALSE;

	rgHRow[0]=NULL;

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,ALL_COLS_BOUND))
		goto CLEANUP;

	//get the bookmark for the 2nd row
	if(!GetBookmark(2,&rgcbBookmarks[0],&rgpBookmarks[0]))
		goto CLEANUP;

	//get an invalid bookmark
	rgcbBookmarks[1]=1;
	rgpBookmarks[1]=(BYTE *)&DBBookmark;

	//restart the position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		goto CLEANUP;

	ReleaseAccessorOnRowset();

	if(!GetAccessorOnRowset(DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

	//change the 2nd row
	//get the row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRowsObtained,&pHRow),S_OK))
		goto CLEANUP;

	//get data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
		goto CLEANUP;

	//set data for the row
	if(!CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,m_pData),S_OK))
		goto CLEANUP;

	//release the memory from GetData
	FreeMemory();

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);   
	pHRow=NULL;

	//Pass an array of 2 bookmarks: invalid and bookmark to the second row.	
	if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,2,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHRow,rgDBRowStatus),DB_S_ERRORSOCCURRED))
		goto CLEANUP;

	//verify the 2nd row is retrieved
	if(!COMPARE(rgHRow[1],DB_NULL_HROW))
		goto CLEANUP;

	if(!COMPARE(VerifyRowPosition(rgHRow[0], 2, g_pCTable),TRUE))
		goto CLEANUP;

	if(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK) && 
		COMPARE(rgDBRowStatus[1],DBROWSTATUS_E_INVALID))
		fTestPass=TRUE;

CLEANUP:
	//release the row handles
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}  

	if(rgHRow[0])
		CHECK(m_pIRowset->ReleaseRows(1,rgHRow,NULL, NULL, NULL),S_OK);
	
	//free the bookmarks
	PROVIDER_FREE(rgpBookmarks[0]);

	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ChangeRows::Terminate()
{
	//release the IRowsetChange pointer
	SAFE_RELEASE(m_pIRowsetChange);
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(Empty_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		Empty_Rowset - Test empty rowset cases
//| Created:  	1/18/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Empty_Rowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetLocate::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt forward with DBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Empty_Rowset::Variation_1()
{ 
	BOOL		fTestPass = TEST_FAIL;
	DBBOOKMARK	bmkFirst = DBBMK_FIRST;
	DBBOOKMARK	*pBmk = &bmkFirst;
	ULONG_PTR	cbBmk = STD_BOOKMARKLENGTH;
	DBCOUNTITEM	cRowsObtained = MAXDBCOUNTITEM;
	HROW		hRow = DB_NULL_HROW;
	HROW		*phRow = NULL;
	DBROWSTATUS RowStatus = 0;

	//create a rowset and accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_EMPTYROWSET, IID_IRowsetLocate));
	
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBmk,(const BYTE **)&pBmk,
		&hRow,&RowStatus),DB_E_ERRORSOCCURRED);
	TESTC(hRow == DB_NULL_HROW);
	TESTC(RowStatus == DBROWSTATUS_E_INVALID);

	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,STD_BOOKMARKLENGTH,
				(BYTE *)&bmkFirst,0,1,&cRowsObtained,&phRow),DB_S_ENDOFROWSET);

	TESTC(cRowsObtained == 0);
	TESTC(phRow == NULL);
	fTestPass = TEST_PASS;

CLEANUP:

	PROVIDER_FREE(phRow);

	ReleaseRowsetAndAccessor();
	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt backward with DBMK_LAST
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Empty_Rowset::Variation_2()
{ 
	BOOL		fTestPass = TEST_FAIL;
	DBBOOKMARK	bmkLast = DBBMK_LAST;
	DBCOUNTITEM	cRowsObtained = MAXDBCOUNTITEM;
	HROW*		pHRow = NULL;
	DBPROPID	guidProperty = DBPROP_CANFETCHBACKWARDS;
	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_EMPTYROWSET, IID_IRowsetLocate,
		1,&guidProperty));
	
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,STD_BOOKMARKLENGTH,
				(BYTE *)&bmkLast,0,-1,&cRowsObtained,&pHRow),DB_S_ENDOFROWSET);

	TESTC(cRowsObtained == 0);
	TESTC(pHRow == NULL);
	fTestPass = TEST_PASS;

CLEANUP:
	if (pHRow)
		PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();
	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Empty_Rowset::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetLocate::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

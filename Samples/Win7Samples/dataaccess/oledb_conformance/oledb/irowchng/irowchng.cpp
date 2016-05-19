//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module SETDATA.CPP | Source file for test module IRowsetChange
//
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "modstandard.hpp"
#include "irowchng.h"
#include "extralib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xa41ff7d5, 0x8669, 0x11cf, { 0x89, 0x94, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("IRowsetChange");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("The test module for interface IRowsetChange:SetData");
DECLARE_MODULE_VERSION(839035527);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

#define TESTC_SetData(exp){TESTB = TEST_PASS; if((TEST_SKIPPED==exp)) {TESTB = TEST_SKIPPED; goto CLEANUP;}}

#ifdef _WIN64
//this would be more descriptive as DBROWCOUNT but since
//privlib want a ULONG for it's GetProperty, eventhough
//properties are unsigned, ULONG to VARIANT will be handled instead
#define V_DBCOUNTITEM(X)		V_UNION(X, ullVal)
#else
#define V_DBCOUNTITEM(X)		V_UNION(X, ulVal)
#endif	// _WIN64


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const ULONG		g_cPropertyIDs		= 3;
DBPROPID		g_rgPropertyIDs[g_cPropertyIDs]	= {	DBPROP_OTHERINSERT, DBPROP_OTHERUPDATEDELETE, DBPROP_OWNUPDATEDELETE};
BOOL			g_fMAXPENDINGROWS	= FALSE;
DBCOUNTITEM		g_lMaxPendRows		= -1;
WORD			g_ACCESSORDER_VALUE	= DBPROPVAL_AO_RANDOM;
DBCOUNTITEM		g_ulRowCount;
BOOL			g_fUseMaxRows;

//--------------------------------------------------------------------
// @Check if an integer is in the array
//
//--------------------------------------------------------------------
BOOL	InArray(ULONG ulNumber, ULONG	cCount, ULONG	*rgNumber)
{
	ULONG	cIndex;

	for(cIndex=0; cIndex<cCount; cIndex++)
	{
		if(ulNumber==rgNumber[cIndex])
		{
			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------
// @is property call IRowsetIdentity->IsSameRow
//--------------------------------------------------------------------
BOOL	fnIsSameRow	(	IUnknown	*pIUnknown,
						HROW		*phThisRow,
						HROW		*phThatRow
					)
{
	IRowsetIdentity	*pIRowsetIdentity = NULL;

	//Compare rows, the two row handles should have the same value
	//get property info on the rowset 
	if(!VerifyInterface(pIUnknown, IID_IRowsetIdentity, ROWSET_INTERFACE, (IUnknown**)&pIRowsetIdentity))
		goto CLEANUP;

	if (!pIRowsetIdentity)
		goto CLEANUP;

	if (S_OK==pIRowsetIdentity->IsSameRow(*phThisRow,*phThatRow))
	{
		SAFE_RELEASE(pIRowsetIdentity);
		return TRUE;	
	}	
CLEANUP:
	SAFE_RELEASE(pIRowsetIdentity);
	odtLog<<L"Rows do not compare."<<ENDL;
	return FALSE;		
}


DBSTATUS GetStatus(void *pData, DBBINDING *pBinding)
{
	return STATUS_BINDING(*pBinding,pData);
}


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule *pThisTestModule)
{

	//CommonModuleInit, Verify IRowset is supported, and Create a table
	BOOL fPass = CommonModuleInit(pThisTestModule, IID_IRowsetChange, 20);
	GetModInfo()->SetCompReadOnlyCols(FALSE);
	if (TEST_PASS == fPass)
	{
		// Reset the global
		g_ulRowCount        = g_pTable->GetRowsOnCTable();
		g_fMAXPENDINGROWS	= FALSE;
		g_lMaxPendRows		= -1;
		g_ACCESSORDER_VALUE	= DBPROPVAL_AO_RANDOM;
		g_fUseMaxRows		= FALSE;
	}	

	return fPass;
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
	ULONG	cCnt	= 0;

	if (g_pTable)
	{
		//if an ini file is being used then delete and repopulate
		if(GetModInfo()->GetFileName())
		{
			//delete all rows in the table.
			if(g_pTable->DeleteRows(ALLROWS) == S_OK)
			{
				// RePopulate table in case an .ini file is being used.
				for(cCnt=1; cCnt<=g_ulRowCount; cCnt++)
				{
					if(g_pTable->Insert(cCnt, PRIMARY) != S_OK)
					{
						return FALSE;
					}
				}
			}
		}
	}

	return CommonModuleTerminate(pThisTestModule);
}	
	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCIRowsetChange:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCIRowsetChange : public CRowsetObject
{
	private:

	protected:
		//@cmember: interface pointer for IRowsetChange
		IRowsetChange		*m_pIRowsetChange;

		//@cmember: interface pointer for IRowsetUpdate
		IRowsetUpdate		*m_pIRowsetUpdate;

		//@cmember: interface pointer for IRowsetLocate
		IRowsetLocate		*m_pIRowsetLocate;

		//@cmember: interface pointer for IRowset
		IRowset				*m_pIRowset;

		//@cmember: interface pointer for IRowsetIdentity
		IRowsetIdentity		*m_pIRowsetIdentity;

		//@cmember:	accessory handle
		HACCESSOR			m_hAccessor;

		//@cmember:	the count of rows
		DBCOUNTITEM			m_cRowsObtained;

		//@cmember:	the size of a row
		DBLENGTH			m_cRowSize;

		//@cmember:	the count of binding structure
		DBCOUNTITEM			m_cBinding;

		//@cmember: the array of binding strucuture
		DBBINDING			*m_rgBinding;

		//@cmember: the column information
		DBCOLUMNINFO		*m_rgInfo;

		//@cmember: the string buffer to hold the name
		WCHAR				*m_pStringsBuffer;

		//@cmember:	the pointer to the row buffer
		void				*m_pData;

		//@cmember 
		DBCOUNTITEM			m_ulTableRows;
		
		//@cmember 
		BOOL				m_bIndexExists;

		//@cmember: the location of accessor handle
		EACCESSORLOCATION	m_eAccessorLocation;

		//@cmember: The Providers Updatability Flags for DBPROP_UPDATABILITY
		ULONG_PTR			m_ulpProvUpdFlags;

		//@cmember: The Updatability Flags for DBPROP_UPDATABILITY
		ULONG_PTR			m_ulpUpdFlags;

		//@cmember: The Updatability Flags for DBPROP_UPDATABILITY
		BOOL				m_fUseCmdTmeOut;

		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object and set properties, execute a sql statement,
		//		  and create a rowset object.  Create an accessor on the rowset 
		BOOL GetRowsetAndAccessor
		(	
			EQUERY				eSQLStmt,
			ULONG				cProperties			=0,			
			const DBPROPID		*rgProperties		=NULL,			
			ULONG				cPropertiesUnset	=0,
			const DBPROPID		*rgPropertiesUnset	=NULL,
			EACCESSORLOCATION	eAccessorLocation	=NO_ACCESSOR,
			BOOL				fBindLongColumn		=FALSE,
			DBACCESSORFLAGS		dwAccessorFlags		=DBACCESSOR_ROWDATA,		
			DBPART				dwPart				=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind			=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder		=FORWARD,			
			ECOLS_BY_REF		eColsByRef			=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier		=DBTYPE_EMPTY,
			DBCOUNTITEM			cColsToBind			=0,
			ULONG_PTR			*rgColsToBind		=NULL,
			ECOLS_MEM_PROV_OWNED 
								eColsMemProvOwned	=NO_COLS_OWNED_BY_PROV,	//@paramopt [IN] Which columns' memory is to be owned by the provider
			DBPARAMIO			eParamIO			= DBPARAMIO_NOTPARAM,	//@paramopt [IN] Parameter type to specify for eParmIO
			HRESULT				*pHResult			= NULL,
			HRESULT				HRPossible			= NULL
		);


		//@mfun: create an accessor on the rowset.  
		BOOL	GetAccessorOnRowset
		(
			EACCESSORLOCATION	eAccessorLocation=ON_ROWSET_ACCESSOR,
			BOOL				fBindLongColumn=FALSE,
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			DBORDINAL			cColsToBind=0,
			DBORDINAL			*rgColsToBind=NULL,
			ECOLS_MEM_PROV_OWNED 
								eColsMemProvOwned = NO_COLS_OWNED_BY_PROV,	//@paramopt [IN] Which columns' memory is to be owned by the provider
			DBPARAMIO			eParamIO = DBPARAMIO_NOTPARAM		//@paramopt [IN] Parameter type to specify for eParmIO
		);

		//@mfun: Get the bookmark for the row 
		BOOL	GetBookmark
		(
			DBROWCOUNT	ulRow,
			DBBKMARK	*pcbBookmark,
			BYTE		**ppBookmark
		);

		BOOL	GetBookmarkByRow
		(
			HROW	hRow,
			DBBKMARK*pcbBookmark,
			BYTE	**ppBookmark
		);


		//@mfunc: Get cursor type of the rowset
		ECURSOR	GetCursorType();

		//@mfunc: Return TRUE is we are on QueryBased Update mode
		BOOL	MultipleChanges();

		//@mfun: Return TRUE if we are on buffered update mode
		BOOL	BufferedUpdate();

		BOOL	GetProp(DBPROPID DBPropID);

		//@mfunc: Get the Nullable and Updatable column
		BOOL	GetNullableAndUpdatable(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);

 		//@mfunc: Get the Not-Nullable and Updatable column
		BOOL	GetNotNullableAndUpdatable(	DBORDINAL	*pcbCol,
											DBORDINAL	**prgColNum);

		//@mfunc: Get the Numeric and Updatable column
		BOOL	GetNumericAndUpdatable(	DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);

		//@mfunc: Get the Numeric and Updatable column
		BOOL	GetFloatAndUpdatable(	DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);


		//@mfunc: Get the Fixed Length and Updatable column
		BOOL	GetFixedLengthAndUpdatable(	DBORDINAL	*pcbCol,
											DBORDINAL	**prgColNum);

		//@mfunc: Get the non key non BLOB
		BOOL	GetNonKeyNonBLOB(	DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum);

		//@mfunc: Get the BLOB
		BOOL	GetBLOB(	DBORDINAL	*pcbCol,
							DBORDINAL	**prgColNum);

		//@mfunc: Get the BLOB
		BOOL	GetImage(	DBORDINAL	*pcbCol,
							DBORDINAL	**prgColNum);

		//@mfunc: Get the Variable Length and Updatable column
		BOOL	GetVariableLengthAndUpdatable(	DBORDINAL	*pcbCol,
												DBORDINAL	**prgColNum,
												BOOL		fLong=TRUE,
												BOOL		fBytes=TRUE);

		//@mfunc: Get the Variable Length and Updatable column
		BOOL	GetStr(	DBORDINAL	*pcbCol,
						DBORDINAL	**prgColNum);


		//@mfunc: Get not updatable column
		ULONG	GetNotUpdatable(	DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum);

		BOOL	GetUpdatableCols(	DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum);

 
		ULONG	GetAllButFirst(	DBORDINAL	*pcbCol,
								DBORDINAL	**prgColNum);

		//@mfunc: verify the position of the row handle in the row set
		BOOL	VerifyRowPosition(	HROW	hRow,			//row handle
									ULONG	cRow,		//potision expected
									EVALUE	eValue=PRIMARY);	//eValue for MakeData


		HRESULT SetData(HROW hRow, HACCESSOR hAccessor, void* pData);
		HRESULT GetData(HROW hRow, HACCESSOR hAccessor, void* pData);
	
		void fnIgnoreAutoInc(void	**pData);


		//@mfunc: release the memory referenced by the consumer's buffer
		void FreeMemory();

		//@mfunc: release the accessor on the rowset
		void ReleaseAccessorOnRowset();

		//@mfunc: release a rowset object and accessor created on it
		void ReleaseRowsetAndAccessor(ULONG RowRefCnt=0, ULONG CmdRefCnt=0);

	public:
		//constructor
		TCIRowsetChange(WCHAR *wstrTestCaseName):CRowsetObject(wstrTestCaseName)
		{	
			m_pIRowsetChange	=NULL;
			m_pIRowsetUpdate	=NULL;
			m_pIRowsetLocate	=NULL;
			m_pIRowsetIdentity	=NULL;
			m_pIRowset			=NULL;
			m_hAccessor			=NULL;
			m_cRowsObtained		=0;
			m_cRowSize			=0;
			m_cBinding			=0;
			m_rgBinding			=NULL;
			m_rgInfo			=NULL;
			m_pStringsBuffer	=NULL;
			m_pData				=NULL;
			m_eAccessorLocation	=NO_ACCESSOR;
			m_bIndexExists		=FALSE;
			m_ulTableRows		=0;
			m_ulpProvUpdFlags	=0;
			m_ulpUpdFlags		=DBPROPVAL_UP_CHANGE;
			m_fUseCmdTmeOut		=FALSE;
		};
		//destructor
		virtual ~TCIRowsetChange(){};
};


//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetChange::Init()
{
	//Init baseclass
	CRowsetObject::Init();
	
	//Use the global DSO created in Module init
	SetDataSourceObject(g_pIDBInitialize); 

	//Use the global DB Rowset created in Module init
	SetDBSession(g_pIOpenRowset); 

	//Use the global CTable created in Module init, by default
	SetTable(g_pTable, DELETETABLE_NO);		

	//Use the global C1RowTable for the second table, if ever needed
	SetTable2(g_p1RowTable, DELETETABLE_NO);

	m_ulTableRows	= g_pTable->CountRowsOnTable();
	g_ulRowCount	= m_ulTableRows;

	g_pTable->DoesIndexExist(&m_bIndexExists);
	
	// Get the value for DBPROP_UPDATABILITY if ReadOnly
	if( !SettableProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,g_pIDBCreateSession) )
	{
		// Create a Command object
		CreateRowsetObject(USE_OPENROWSET,IID_IRowset,EXECUTE_IFNOERROR);
		GetProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,m_pIAccessor,&m_ulpProvUpdFlags);//this should be a long, not a ulong
		ReleaseRowsetObject();
	}

	return TRUE;
}


//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetChange::Terminate()
{
	ULONG	cCnt	= 0;
 
	if (g_pTable)
	{
		//if an ini file is being used then delete and repopulate
		if(GetModInfo()->GetFileName())
		{
			//delete all rows in the table.
			if(g_pTable->DeleteRows(ALLROWS) == S_OK)
			{
				// RePopulate table in case an .ini file is being used.
				for(cCnt=1; cCnt<=g_ulRowCount; cCnt++)
				{
					if(g_pTable->Insert(cCnt, PRIMARY) != S_OK)
					{
						return FALSE;
					}
				}
			}
		}
	}

	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();
	return(CRowsetObject::Terminate());
}

	
HRESULT TCIRowsetChange::SetData(HROW hRow, HACCESSOR hAccessor, void* pData)
{
	HRESULT			hr				= S_OK;
	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM		cBindings		= 0;
	DBBINDING		*rgBindings		= NULL;
	CRowObject		RowObject;

	//Does the provider support Row Objects?
	hr = RowObject.CreateRowObject(m_pIRowset, hRow);

	//Verify Results...
	if(SUCCEEDED(hr) && hAccessor && hRow && pData)
	{
		//Obtain the accessor bindings
		ASSERT(m_pIAccessor);
		QTESTC_(hr = m_pIAccessor->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

		//Obtain the data
		hr = RowObject.SetColumns(cBindings, rgBindings, pData);
	}
	else
	{
		ASSERT(m_pIRowsetChange);
		hr = m_pIRowsetChange->SetData(hRow, hAccessor, pData);
	}

	if (pData)
	{
		//Display any binding errors and status'
		TESTC(VerifyBindings(hr, m_pIAccessor, hAccessor, pData));
	}
CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}


HRESULT TCIRowsetChange::GetData(HROW hRow, HACCESSOR hAccessor, void* pData)
{
	DBACCESSORFLAGS		dwAccessorFlags;
	DBCOUNTITEM			cBindings	= 0;
	DBBINDING			*rgBindings = NULL;
	CRowObject			RowObject;
	HRESULT				hr			= S_OK;

	//Does the provider support Row Objects?
	hr = RowObject.CreateRowObject(m_pIRowset, hRow);

	//Verify Results...
	if(SUCCEEDED(hr) && hAccessor && hRow && pData)
	{
		//Obtain the accessor bindings
		ASSERT(m_pIAccessor);
		QTESTC_(hr = m_pIAccessor->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)
  
		//Get the Data for row object
		hr = RowObject.GetColumns(cBindings, rgBindings, pData);
	}
	else
	{
		ASSERT(m_pIRowset);
		hr = m_pIRowset->GetData(hRow, hAccessor, pData);
	}

	//Display any binding errors and status'
	TESTC(VerifyBindings(hr, m_pIAccessor, hAccessor, pData));

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}

		
void TCIRowsetChange::fnIgnoreAutoInc(void	**pData)
{
	DBORDINAL	cColsInTable= 0;
	ULONG		i			= 0;	
	ULONG		j			= 0;	
	DBORDINAL	cColOrd		= 0;
	CCol		CurCol;			//Current Column

	//get the number of columns on the table
	cColsInTable = g_pTable->CountColumnsOnTable();

	//Copy the provider types from the tables column info.
	for(i=0;i<cColsInTable; i++)
	{
		// get column info for nCol'th  column from the global table 
		g_pTable->GetColInfo(i+1, CurCol);

		//if there is an auto inc col
		if (CurCol.GetAutoInc())
		{
			cColOrd = CurCol.GetColNum();
				
			//loop through the bindings looking for the same ordinal
			for (j=0;j<m_cBinding;j++)
			{
				if (m_rgBinding[j].iOrdinal==cColOrd)
				{
					//set the binding status to ignore
//					*(DBSTATUS *)(((DWORD)*pData)+m_rgBinding[j].obStatus)=DBSTATUS_S_IGNORE;
					STATUS_BINDING(m_rgBinding[j],pData)=DBSTATUS_S_IGNORE;
					break;
				}
			}
		}
	}	
}

		
//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetRowsetAndAccessor
(	
	EQUERY				eSQLStmt,				//the SQL Statement to create
	ULONG				cProperties,			//the count of properties
	const DBPROPID		*rgProperties,			//the array of properties to be set
	ULONG				cPropertiesUnset,		//the count of properties to be unset
	const DBPROPID		*rgPropertiesUnset,		//the array of properties to be unset	
	EACCESSORLOCATION	eAccessorLocation,		//where the accessor should be created
	BOOL				fBindLongColumn,		//whether to bind LONG columns
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	DBORDINAL			cColsToBind,			//the count of columns to bind
	DBORDINAL			*rgColsToBind,			//the array of column ordinals to bind
	ECOLS_MEM_PROV_OWNED eColsMemProvOwned,		//@paramopt [IN] Which columns' memory is to be owned by the provider
	DBPARAMIO			eParamIO,				//@paramopt [IN] Parameter type to specify for eParmIO
	HRESULT				*pHResult,				//the hresult to return
	HRESULT				HRPossible				//in case an valid error HR might be possible in this method
)
{
	IColumnsInfo		*pIColumnsInfo	= NULL;
	IRowset				*pIRowset		= NULL;
	DBCOUNTITEM			cRowsObtained	= 0;
	HROW				*pHRow			= NULL;

	ULONG				cDBPropSet		= 1;
	DBPROPSET			rgDBPropSet[2];
	
	ULONG				cProp			= 0;
	
	HRESULT				hr				= S_OK;
	BOOL				fPass			= FALSE;
	BLOBTYPE			blobType;
	BOOL				fUseServiceComp	= GetModInfo()->UseServiceComponents();
	ULONG				ulIndex			= 0;
	ULONG				ulUpdValue		= 0;
	DBCOUNTITEM			ulMaxPendRows	= 0;
	ULONG				cExtraProps		= 1;
	ULONG				i				= 0;

	m_pIAccessor	= NULL;

	if(fBindLongColumn)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;


	//Set up the DB Properties struct
	if(cProperties || cPropertiesUnset)
	{
		//if MaxPendingRows is needed, make sure space is alloicated for it.
		if (-1!=g_lMaxPendRows)
		{
			cExtraProps++;
		}
		//if m_fUseCmdTmeOut is needed, make sure space is alloicated for it too.
		if (m_fUseCmdTmeOut)
		{
			cExtraProps++;
		}
		if(fUseServiceComp)
		{
			cExtraProps++;
		}
		if(g_fUseMaxRows)
		{
			cExtraProps++;
		}

		//init DBPropSet[0]
		rgDBPropSet[0].rgProperties   = NULL;
		rgDBPropSet[0].cProperties    = 1;
		rgDBPropSet[0].guidPropertySet= DBPROPSET_ROWSET;

		//allocate 
		//Might need an extra for DBPROP_UPDATABILITY (+1)
		rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC
			(sizeof(DBPROP) * (cProperties + cPropertiesUnset + cExtraProps));

		memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(cProperties + cPropertiesUnset + cExtraProps));
		if(!rgDBPropSet[0].rgProperties)
			goto CLEANUP;

		rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetFind;
		rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
		rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
		rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
		V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_TRUE;
		
		//make sure the test checks properties against sc/ce
		if (fUseServiceComp)
		{
			//Get Rowset interface so info from RowsetInfo  can be obtainied
			// call IOpenRowset to return a Rowset		
			hr = m_pTable->CreateRowset	(
											USE_OPENROWSET,
											IID_IRowset,
											1,
											rgDBPropSet,
											(IUnknown**)&pIRowset,
											NULL,
											NULL,
											NULL
										);		
		}
		else
		{
			//Get Rowset interface so info from RowsetInfo  can be obtainied
			// call IOpenRowset to return a Rowset		
			hr = m_pTable->CreateRowset	(
											USE_OPENROWSET,
											IID_IRowset,
											0,
											NULL,
											(IUnknown**)&pIRowset,
											NULL,
											NULL,
											NULL
										);		
		}

		if (pIRowset)
		{
			//go through the loop to set every DB Property required
			for(i=0; i<cProperties; i++)
			{
				//if the property is supported AND
				//if the property is writeable OR the default is VARIANT_TRUE
				if( SupportedProperty(rgProperties[i],DBPROPSET_ROWSET,g_pIDBCreateSession) &&
					(SettableProperty(rgProperties[i],DBPROPSET_ROWSET,g_pIDBCreateSession) ||
					 GetProperty(rgProperties[i],DBPROPSET_ROWSET,pIRowset)) )
				{
					rgDBPropSet[0].rgProperties[cProp].dwPropertyID   = rgProperties[i];
					rgDBPropSet[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_REQUIRED;
					rgDBPropSet[0].rgProperties[cProp].colid			=DB_NULLID;

					//check to see if the DBPROP_ACCESSORDER need to be set
					if (rgProperties[i]==DBPROP_ACCESSORDER)
					{
						rgDBPropSet[0].rgProperties[cProp].vValue.vt      = VT_I4;
						V_I4(&rgDBPropSet[0].rgProperties[cProp].vValue)= g_ACCESSORDER_VALUE;
					}
					else
					{
						rgDBPropSet[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
						V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)= VARIANT_TRUE;
					}
					cProp++;
				}
				else
				{
					odtLog<<L"A property neccessary to execute this variation was not settable."<<ENDL;
					fPass=FALSE;
					goto CLEANUP;
				}
			}
		}
		else
		{
				odtLog<<L"this provider is useless."<<ENDL;
				goto CLEANUP;
		}

		//if variation wants MAXPENDINGROWS set to something, it won't be -1 here
		if (-1!=g_lMaxPendRows)
		{
			//if MAXPENDINGROWS is supported
			if( SupportedProperty(DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET,g_pIDBCreateSession))
			{
				// Figure out the DBPROP_MAXPENDINGRROWS value if ReadOnly
				if( !(SettableProperty(DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET,g_pIDBCreateSession)))
				{
					//if READ Only then get its value
					if(!GetProperty(DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET,pIRowset,&ulMaxPendRows) )
					{
						//skip the calling variation
						odtLog<<L"DBPROP_MAXPENDINGROWS was not settable."<<ENDL;
						fPass=FALSE;
						goto CLEANUP;
					}
					g_lMaxPendRows=ulMaxPendRows;
				}
			}
			else
			{
				//skip the calling variation
				odtLog<<L"DBPROP_MAXPENDINGROWS was not settable."<<ENDL;
				fPass=FALSE;
				goto CLEANUP;
			}
			//set DBPROP_MAXPENDINGROWS
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID	= DBPROP_MAXPENDINGROWS;
			rgDBPropSet[0].rgProperties[cProp].dwOptions	= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt	= VT_I4;
			rgDBPropSet[0].rgProperties[cProp].colid		= DB_NULLID;
			V_DBCOUNTITEM(&rgDBPropSet[0].rgProperties[cProp].vValue)=DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;
			cProp++;
		}

		//set DBPROP_UPDATABILITY
		rgDBPropSet[0].rgProperties[cProp].dwPropertyID		= DBPROP_UPDATABILITY;
		rgDBPropSet[0].rgProperties[cProp].dwOptions		= DBPROPOPTIONS_REQUIRED;
		rgDBPropSet[0].rgProperties[cProp].vValue.vt		= VT_I4;
		rgDBPropSet[0].rgProperties[cProp].colid			= DB_NULLID;
		V_DBCOUNTITEM(&rgDBPropSet[0].rgProperties[cProp].vValue)=m_ulpUpdFlags;
		cProp++;

		//if service components are request let try hard to give'em
		if (fUseServiceComp)
		{
			//set DBPROP_IRowsetFind
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID		= DBPROP_IRowsetFind;
			rgDBPropSet[0].rgProperties[cProp].dwOptions		= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt		= VT_BOOL;
			rgDBPropSet[0].rgProperties[cProp].colid			= DB_NULLID;
			V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)	= VARIANT_TRUE;
			cProp++;
		}

		if (m_fUseCmdTmeOut)
		{
			//set DBPROP_COMMANDTIMEOUT to 1 second
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID	= DBPROP_COMMANDTIMEOUT;
			rgDBPropSet[0].rgProperties[cProp].dwOptions	= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt	= VT_I4;
			rgDBPropSet[0].rgProperties[cProp].colid		= DB_NULLID;
			rgDBPropSet[0].rgProperties[cProp].vValue.lVal	= 1;
			cProp++;
		}

		if (g_fUseMaxRows)
		{
			//set DBPROP_MAXROWS
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID	= DBPROP_MAXROWS;
			rgDBPropSet[0].rgProperties[cProp].dwOptions	= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt	= VT_I4;
			rgDBPropSet[0].rgProperties[cProp].colid		= DB_NULLID;
			rgDBPropSet[0].rgProperties[cProp].vValue.lVal	= 1;
			cProp++;
		}

		//go through the loop to unset every DB Property required
		for(i=0; i<cPropertiesUnset; i++)
		{
			//if the property is NOT writeable (read-only) AND the default is VARIANT_TRUE)
			//skip the variation
			if	( 
					((!	SettableProperty(rgPropertiesUnset[i],DBPROPSET_ROWSET,g_pIDBCreateSession)) &&
						GetProperty(rgPropertiesUnset[i],DBPROPSET_ROWSET,pIRowset))
				)
			{
				odtLog<<L"A property neccessary to execute this variation was not settable."<<ENDL;
				fPass=FALSE;
				goto CLEANUP;
			}
			else
			{
				if( SupportedProperty(rgPropertiesUnset[i],DBPROPSET_ROWSET,g_pIDBCreateSession))
				{
					rgDBPropSet[0].rgProperties[cProp].dwPropertyID		= rgPropertiesUnset[i];
					rgDBPropSet[0].rgProperties[cProp].dwOptions		= DBPROPOPTIONS_REQUIRED;
					rgDBPropSet[0].rgProperties[cProp].vValue.vt		= VT_BOOL;
					rgDBPropSet[0].rgProperties[cProp].colid			= DB_NULLID;
					V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)	= VARIANT_FALSE;
					cProp++;
				}
				else
				{
					odtLog<<L"A property neccessary to execute this variation was not settable."<<ENDL;
					fPass=FALSE;
					goto CLEANUP;
				}
			}
		}
		rgDBPropSet[0].cProperties = cProp;
	}
	  
	//release the rowset on session so no rowset is open on the session
	//if there is an open rowset on the session it might cause problem 
	//if firehose mode is being used
	SAFE_RELEASE(pIRowset);

	//create an accessor on the command
	if(eAccessorLocation==ON_COMMAND_ACCESSOR)
	{
		if(!m_pICommand)
		{
			//create the rowset object
			TESTC_(CreateRowsetObject(eSQLStmt,IID_IRowset,EXECUTE_IFNOERROR),S_OK);

			//release the rowset object, but keep the command around
			ReleaseRowsetObject(0);
		}
	 	
		//asked for a command interface.  if we don't have it do not continue
		if (!m_pICommand)
		{
			goto CLEANUP;
		}

		//get the accessor handle
		hr = GetAccessorAndBindings(
										m_pICommand,
										dwAccessorFlags,
										&m_hAccessor,
										&m_rgBinding,
										&m_cBinding,
										&m_cRowSize,
										dwPart,
										eColsToBind,
										eBindingOrder,
										eColsByRef,
										NULL,
										NULL,
										NULL,
										dbTypeModifier,
										cColsToBind,
										(LONG_PTR *)rgColsToBind,
										NULL,
										eColsMemProvOwned,
										eParamIO,
										blobType);
		
		if (DBACCESSOR_PARAMETERDATA&dwAccessorFlags)
		{
			if (DB_E_BADACCESSORFLAGS==hr)
			{
				goto CLEANUP;
			}
		}
		if (DBACCESSOR_PASSBYREF&dwAccessorFlags)
		{
			if (DB_E_BYREFACCESSORNOTSUPPORTED==hr)
			{
				goto CLEANUP;
			}
		}
	}

	if(!SUCCEEDED(SetRowsetProperties(rgDBPropSet, cDBPropSet)))
		goto CLEANUP;

	//create the rowset object
	//May fail due to combinations of properties
	if(m_pIDBCreateCommand==NULL && (eSQLStmt== SELECT_ORDERBYNUMERIC 
									|| eSQLStmt == SELECT_REVCOLLIST
									|| eSQLStmt == SELECT_COLLISTFROMTBL))
		eSQLStmt = SELECT_ALLFROMTBL;

	hr = CreateRowsetObject(eSQLStmt,IID_IUnknown,EXECUTE_IFNOERROR);
	if(hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED)
	{
		for (ulIndex=0;ulIndex<rgDBPropSet[0].cProperties;ulIndex++)
		{
			//if multiple props were set they might be conflicting on some providers
			if (DB_E_ERRORSOCCURRED		==	hr												&&
				DBPROPSTATUS_CONFLICTING==	rgDBPropSet[0].rgProperties[ulIndex].dwStatus	&&
				1						<	rgDBPropSet[0].cProperties)
			{
				odtLog<<L"Conflict."<<ENDL;
			}
		}
		goto CLEANUP;
	}

	if (HRPossible)
	{
		if (HRPossible==hr)
		{
			goto CLEANUP;
		}
	}
	
	TESTC_(hr,S_OK);
		
	//get the IRowsetChange pointer
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetChange, (LPVOID *)&m_pIRowsetChange),S_OK);
		
	//get the IRowsetUpdate pointer if present
	if(!SUCCEEDED(hr=m_pIAccessor->QueryInterface(IID_IRowsetUpdate,(LPVOID *)&m_pIRowsetUpdate)))
		TESTC_(hr, E_NOINTERFACE);

	//get the IRowsetLocate pointer if present
	if(!SUCCEEDED(hr=m_pIAccessor->QueryInterface(IID_IRowsetLocate,(LPVOID *)&m_pIRowsetLocate)))
		TESTC_(hr, E_NOINTERFACE);
	
	//get the IRowsetIdentity pointer if present
	if(!SUCCEEDED(hr=m_pIAccessor->QueryInterface(IID_IRowsetIdentity,(LPVOID *)&m_pIRowsetIdentity)))
		TESTC_(hr, E_NOINTERFACE);
	
	//get the IRowset  pointer  
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowset,(LPVOID *)&m_pIRowset),S_OK);
			
	// get the columns infomation
	TESTC_(m_pIAccessor->QueryInterface(IID_IColumnsInfo, (LPVOID *)&pIColumnsInfo),S_OK);
	TESTC_(pIColumnsInfo->GetColumnInfo(&m_cRowsetCols,&m_rgInfo, &m_pStringsBuffer),S_OK);

	//remember where the accessor handle is created
	m_eAccessorLocation=eAccessorLocation;

	switch(eAccessorLocation)
	{
		case ON_COMMAND_ACCESSOR:
				//can not create an accessor on the command object if the 
				//the rowset is a simple rowset
				if(eSQLStmt==USE_OPENROWSET)
					goto CLEANUP;
			break;
		case ON_ROWSET_FETCH_ACCESSOR:
				//can not create an accessor after the first fetch if no IRowset
				//is present on the rowset
				if(!m_pIRowset)
					goto CLEANUP;
				TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained, &pHRow), S_OK);
					
		case ON_ROWSET_ACCESSOR:  
				//can not create an accessor on a rowset if no IRowset is present
				if(!m_pIRowset)
					goto CLEANUP;

				//create an accessor on the rowset
				hr	= GetAccessorAndBindings(m_pIAccessor,dwAccessorFlags,&m_hAccessor,
				&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
				eColsByRef,NULL,NULL,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *)rgColsToBind,
				NULL,eColsMemProvOwned,eParamIO,blobType);

                //if the test created a NULL accessor
                if (!m_cBinding)
                {
		            fPass=TEST_SKIPPED;
		            goto CLEANUP;
                }

				if (DBACCESSOR_PARAMETERDATA&dwAccessorFlags)
				{
					if (DB_E_BADACCESSORFLAGS==hr)
					{
						goto CLEANUP;
					}
				}
				if (DBACCESSOR_PASSBYREF&dwAccessorFlags)
				{
					if (DB_E_BYREFACCESSORNOTSUPPORTED==hr)
					{
						goto CLEANUP;
					}
				}
			break;
		case NO_ACCESSOR:
			break;
		default:
			return FALSE;
	}

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);
	if(!m_pData)
	{
		goto CLEANUP;
	}
	
	fPass=TRUE;
CLEANUP:
	g_fUseMaxRows = FALSE;
	if(rgDBPropSet[0].rgProperties)			   
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);

	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRowset);

	if(m_pIRowset) 
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		//restart position.  The rowset returns to its original state
		hr = m_pIRowset->RestartPosition(NULL);
		CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);
	}

	if( pHResult )
		*pHResult = hr;

	return fPass;
}

//--------------------------------------------------------------------
//@mfunc: Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetAccessorOnRowset
(	
	EACCESSORLOCATION		eAccessorLocation,	//where the accessor should be created
	BOOL					fBindLongColumn,	//whether to bind the LONG accessor
	DBACCESSORFLAGS			dwAccessorFlags,	//the accessor flags
	DBPART					dwPart,				//the type of binding
	ECOLS_BOUND				eColsToBind,		//the columns in accessor
	ECOLUMNORDER			eBindingOrder,		//the order to bind columns
	ECOLS_BY_REF			eColsByRef,			//which columns to bind by reference
	DBTYPE					dbTypeModifier,		//the type modifier used for accessor
	DBORDINAL				cColsToBind,		//the count of columns to bind
	DBORDINAL				*rgColsToBind,		//the array of column ordinals to bind
	ECOLS_MEM_PROV_OWNED 	eColsMemProvOwned,	//@paramopt [IN] Which columns' memory is to be owned by the provider
	DBPARAMIO				eParamIO			//@paramopt [IN] Parameter type to specify for eParmIO
)
{
	IUnknown			*pIUnknown		=NULL;
	DBCOUNTITEM			cRowsObtained	=0;
	HROW				*pHRow			=NULL;
	DBORDINAL			cCnt			=0;
	BOOL				fPass			=FALSE;
	BLOBTYPE			blobType;
	HRESULT				hr				= S_OK;

	if(fBindLongColumn)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;


	//remember where the accessor handle is created
	m_eAccessorLocation=eAccessorLocation;

	//eAccessorLocation can not be NO_ACCESSOR
	if(!COMPARE((eAccessorLocation!=NO_ACCESSOR),TRUE))
	{	
		fPass=FALSE;
		goto CLEANUP;
	}
	
	switch(eAccessorLocation)
	{
		case ON_ROWSET_FETCH_ACCESSOR:
				//can not create an accessor after the first fetch if no IRowset
				//is present on the rowset
				if(!m_pIRowset)
					goto CLEANUP;
				TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained, &pHRow), S_OK);
				pIUnknown=m_pIAccessor;
			break;
		case ON_ROWSET_ACCESSOR:  
				//can not create an accessor on a rowset if no IRowset is present
				if(!m_pIRowset)
					goto CLEANUP;
				pIUnknown=m_pIAccessor;
			break;
		case ON_COMMAND_ACCESSOR:
		default:
			return FALSE;
	}

	// Free the bindings if set
	if( m_cBinding && m_rgBinding )
		FreeAccessorBindings(m_cBinding, m_rgBinding);
	SAFE_FREE(m_pData);

	//create an accessor on the rowset
	hr = GetAccessorAndBindings(pIUnknown,dwAccessorFlags,&m_hAccessor,
							&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
							eColsByRef,NULL,&cCnt,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *) rgColsToBind,
							NULL, eColsMemProvOwned,eParamIO,blobType);

	//DB_E_ERRORSOCCURRED is temperarily acceptable for providers that don't support provider
	//owned memory. a new prop to determine if a provider supports this is on its way
	if (DB_E_ERRORSOCCURRED==hr)
	{
		fPass=TEST_SKIPPED;
		goto CLEANUP;
	}
	if (S_OK!=hr)
	{
		fPass=FALSE;
		goto CLEANUP;
	}

	//make sure cCnt should be the same as m_cRowsetCols
	if(!COMPARE(cCnt, m_cRowsetCols))
		goto CLEANUP;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(m_pData)
		fPass=TRUE;
CLEANUP:
	
	if(eAccessorLocation == ON_ROWSET_FETCH_ACCESSOR)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		// restart position.  The rowset returns to its original state
		hr = m_pIRowset->RestartPosition(NULL);
		TEST2C_(hr, S_OK, DB_S_COMMANDREEXECUTED);
	}	

	return fPass;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetChange::GetBookmark
(
	DBROWCOUNT	ulRow,
	DBBKMARK	*pcbBookmark,
	BYTE		**ppBookmark
)
{
	BOOL		fPass	=FALSE;
	HROW		hRow[1];
	HROW		*pHRow	=hRow;
	DBCOUNTITEM	cCount	=0;

	//the rowset has to expose IRowset in order to have bookmark
	if(!m_pIRowset)
		return FALSE;

	//ulRow has to start with 1
	if(!pcbBookmark || !ppBookmark || !ulRow)
		return FALSE;

	//restart the cursor position
	HRESULT hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		return FALSE;

	//fetch the row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(ulRow-1),1,&cCount,&pHRow),S_OK))
		return FALSE;

	//only one row handle is retrieved
	COMPARE(cCount, 1);

	//get the bookmark by the row handle
	fPass=GetBookmarkByRow(*pHRow, pcbBookmark, ppBookmark);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	return fPass;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetChange::GetBookmarkByRow
(
		HROW		hRow,
		DBBKMARK	*pcbBookmark,
		BYTE		**ppBookmark
)
{
	BOOL		fPass		= FALSE;
	void		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;
	DBCOUNTITEM	cBinding	= 0;
	DBLENGTH	cbRowSize	= 0;
	DBBINDING	*pBinding	= NULL;
	DB_LORDINAL	ulColToBind	= 0;	

	//the rowset has to expose IRowset in order to have bookmark
	if(!m_pIRowset)
		return FALSE;

	//check the input
	if(!pcbBookmark || !ppBookmark)
		return FALSE;

	//create an accessor to binding the bookmark
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
	DBACCESSOR_ROWDATA, &hAccessor, &pBinding,
	&cBinding,&cbRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
	USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, 
	DBTYPE_EMPTY, 1, (DB_LORDINAL *)&ulColToBind),S_OK);
		

	//allocate memory 
	if(!(pData=PROVIDER_ALLOC(cbRowSize)))
		goto CLEANUP;

	//get the data
	TESTC_(GetData(hRow, hAccessor, pData),S_OK);
		
	//get the length of the bookmark
//	*pcbBookmark= *((ULONG *)(dwAddr+pBinding[0].obLength));
	*pcbBookmark=LENGTH_BINDING(pBinding[0], pData);

	//allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);

	if(!(*ppBookmark))
		goto CLEANUP;

	//copy the value of the bookmark into the consumer's buffer
//	memcpy(*ppBookmark, (void *)(dwAddr+pBinding[0].obValue), *pcbBookmark);
	memcpy(*ppBookmark, &VALUE_BINDING(pBinding[0], pData), (size_t)*pcbBookmark);

	fPass=TRUE;

CLEANUP:
	//release the memory
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBinding);

	//free the accessor
	CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	return fPass;
}
//--------------------------------------------------------------------
//
//@mfunc: Get cursor type of the rowset.  Has to be called after a rowset 
//			generated.
//
//
//--------------------------------------------------------------------
ECURSOR	TCIRowsetChange::GetCursorType()
{
	IRowsetInfo			*pIRowsetInfo		= NULL;
	ULONG				cProperty			= 0;
	DBPROPID			rgPropertyIDs[3];
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet			= NULL;
	ECURSOR				eCursor				= FORWARD_ONLY_CURSOR;

	//initialization
	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;

	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=3;
	DBPropIDSet.rgPropertyIDs=rgPropertyIDs;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
		

	//ask for the 3 properties
	pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	//only one property set should be returned
	if(!COMPARE(cProperty, 1))
		goto CLEANUP;

	if(V_BOOL(&pDBPropSet[0].rgProperties[0].vValue)==VARIANT_TRUE)
		eCursor=DYNAMIC_CURSOR;
	else
	{
		if(V_BOOL(&pDBPropSet[0].rgProperties[1].vValue)==VARIANT_TRUE)
			eCursor=KEYSET_DRIVEN_CURSOR;
		else
		{
			if(V_BOOL(&pDBPropSet[0].rgProperties[2].vValue)==VARIANT_TRUE)	
				eCursor=STATIC_CURSOR;
		}
	}

CLEANUP:
	
	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return eCursor;
}

BOOL	TCIRowsetChange::GetProp(DBPROPID	DBPropID)
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
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for IID_IRowsetUpdate
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:

	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfunc: Return TRUE is we are Mulltiple changes mode.  Has to
//			to called after a rowset is created.
//
////--------------------------------------------------------------------

BOOL	TCIRowsetChange::MultipleChanges()
{
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ULONG				cProperty		= 0;
	DBPROPID			DBPropID		= DBPROP_REPORTMULTIPLECHANGES;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet		= NULL;
	BOOL				fSupported		= FALSE;

	//initialize
	DBPropIDSet.guidPropertySet	= DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs	= 1;
	DBPropIDSet.rgPropertyIDs	= &DBPropID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
			
	//ask for DBPROP_REPORTMULTIPLECHANGES
	if(!SUCCEEDED(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet)))
	{
		goto CLEANUP;
	}
	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
	{
		fSupported=TRUE;
	}
CLEANUP:
	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}
//--------------------------------------------------------------------
//
//@mfun: Return TRUE if we are on buffered update mode
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::BufferedUpdate()
{
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ULONG				cProperty		= 0;
	DBPROPID			DBPropID		= DBPROP_IRowsetUpdate;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet		= NULL;
	BOOL				fSupported		= FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;


	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
		

	//ask for IID_IRowsetUpdate
	if(!SUCCEEDED(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet)))
		goto CLEANUP;

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Nullable and Updatable column	ordinals arrays. 
//			Exclude the first columns 
//
//		  The function allocation memory for the ordinals array.  Return
//		  TURE is one or more nullable and updatable column exists'
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetNullableAndUpdatable(
									DBORDINAL	*pcbCol,		//[out] the count of the rgColNum
									DBORDINAL	**prgColNum		//[out] the col ordinals array
												)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
	   	//exclude the first column
		if(m_rgInfo[cColsCount].iOrdinal==1)
			continue;

		//if the col is long and not BLOB and either writeable or maybe writeable
		//not BLOB because BOLBS are not bound in the accssor in places this function is used
		if	(
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISNULLABLE) &&
				!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISLONG) &&
				(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
					(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
			  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Not-Nullable and Updatable column	ordinals arrays. 
//
//		  The function allocation memory for the ordinals array.  Return
//		  TURE is one or more nullable and updatable column exists'
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetNotNullableAndUpdatable(
								DBORDINAL	*pcbCol,			//[out] the count of the rgColNum
								DBORDINAL	**prgColNum			//[out] the col ordinals array
													)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol		= 0;
	*prgColNum	=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if((!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISNULLABLE)) &&
		   ((m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
			(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//---------------------------------------------------------------------------------
//
//	Get the updatable columns.
//
//
//---------------------------------------------------------------------------------
BOOL	TCIRowsetChange::GetUpdatableCols(	DBORDINAL	*pcbCol,
											DBORDINAL	**prgColNum)
{
	ULONG	cColsCount=0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		if(
		   (m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
		    (m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Numeric and Updatable column ordinals arrays.	The first
//		  column is excluded.
//
//		  The function allocation memory for the ordinals array.  Return
//		  TURE is one or more numeric and updatable column exists
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetNumericAndUpdatable(
							DBORDINAL	*pcbCol,			//[out] the count of the rgColNum   
							DBORDINAL	**prgColNum			//[out] the col ordinals array      
)
{
	ULONG	cColsCount = 0;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;


	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		//exclude the first column
		if(m_rgInfo[cColsCount].iOrdinal==1)
			continue;

		if((IsNumericType(m_rgInfo[cColsCount].wType)) &&
		   ((m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
			(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Numeric and Updatable column ordinals arrays.	The first
//		  column is excluded.
//
//		  The function allocation memory for the ordinals array.  Return
//		  TURE is one or more numeric and updatable column exists
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetFloatAndUpdatable(
							DBORDINAL	*pcbCol,			//[out] the count of the rgColNum   
							DBORDINAL	**prgColNum			//[out] the col ordinals array      
)
{
	ULONG	cColsCount = 0;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;


	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		//exclude the first column
		if(m_rgInfo[cColsCount].iOrdinal==1)
			continue;

		if(
			(IsNumericType(m_rgInfo[cColsCount].wType))							&&
			(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))	&&
			(m_rgInfo[cColsCount].dwFlags & (~DBCOLUMNFLAGS_ISFIXEDLENGTH))
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Fixed Length and Updatable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetFixedLengthAndUpdatable(	DBORDINAL	*pcbCol,
														DBORDINAL	**prgColNum)
{

	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);


	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if(	(IsFixedLength(m_rgInfo[cColsCount].wType)) &&
			(m_rgInfo[cColsCount].wType == DBTYPE_I4 || m_rgInfo[cColsCount].wType == DBTYPE_I2) &&
			(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the NonKeyNonBLOB
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetNonKeyNonBLOB(	DBORDINAL	*pcbCol,
											DBORDINAL	**prgColNum)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
	{
		return FALSE;
	}

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		if(
			(	!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISLONG))	&&
			(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)	||
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	if(*pcbCol)
	{
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Variable Length and Updatable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetVariableLengthAndUpdatable	(
												DBORDINAL	*pcbCol,
												DBORDINAL	**prgColNum,
												BOOL		fLong,
												BOOL		fBytes
														)
{
	
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if((!IsFixedLength(m_rgInfo[cColsCount].wType)) &&
		   ((m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
			(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )
		  )
		{
			//get rid of the long column
			if(!fLong && (m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISLONG))
			   continue;

			if(!fBytes && (m_rgInfo[cColsCount].wType == DBTYPE_BYTES))
				continue;


			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;		
		}
	}
	
	if(*pcbCol)
		return TRUE;

	 return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the BLOB columns
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetBLOB(	DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
	{
		return FALSE;
	}
	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		if(
			(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISLONG))	&&
			(	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)	||
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	if(*pcbCol)
	{
		return TRUE;
	}
	return FALSE;
}


//--------------------------------------------------------------------
//
//@mfunc: Get an image column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetImage(	DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
	{
		return FALSE;
	}
	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		if(m_rgInfo[cColsCount].wType	==	DBTYPE_BYTES)
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	if(*pcbCol)
	{
		return TRUE;
	}
	return FALSE;
}


//--------------------------------------------------------------------
//
//@mfunc: Get str columns 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetChange::GetStr(
									DBORDINAL	*pcbCol,
									DBORDINAL	**prgColNum)
{
	
	ULONG	cColsCount	 = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);


	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if(( m_rgInfo[cColsCount].wType==DBTYPE_STR ||
			 m_rgInfo[cColsCount].wType==DBTYPE_WSTR
			) 
			&&
		   ((m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
			(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )
		  )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
	{
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get not updatable column
//
//--------------------------------------------------------------------
ULONG	TCIRowsetChange::GetAllButFirst(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum)
{
	ULONG	cColsCount = 0;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);


	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if(m_rgInfo[cColsCount].iOrdinal!=1 && m_rgInfo[cColsCount].iOrdinal!=0)
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	 return FALSE;
	
}

//--------------------------------------------------------------------
//
//@mfunc: Get not updatable column
//
//--------------------------------------------------------------------
ULONG	TCIRowsetChange::GetNotUpdatable(	DBORDINAL	*pcbCol,
											DBORDINAL	**prgColNum)

{
	ULONG	cColsCount;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		//skip column 0
		if(m_rgInfo[cColsCount].iOrdinal==0)
			continue;

		if(!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE) &&
		   !(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	 return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: verify the position of the row handle in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor
//
//--------------------------------------------------------------------

BOOL	TCIRowsetChange::VerifyRowPosition(
											HROW	hRow,	//row handle
											ULONG	cRow,	//potision expected
											EVALUE	eValue)	//eValue for MakeData
{

	//m_pIRowset has to be valid
	if(!m_pIRowset || !m_pData)
		return FALSE;

	//Get Data for the row
	if(!CHECK(GetData(hRow,m_hAccessor,m_pData),S_OK))
		return FALSE;

	//compare the data with the row expected in the rowset
	if(!CompareData(m_cRowsetCols,m_rgTableColOrds,cRow,m_pData,m_cBinding,
		m_rgBinding,m_pTable,m_pIMalloc,eValue))
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//@mfunc:	free the memory referenced by the consumer's buffer
//			The function has to be called after IRowset::GetData()
//
//--------------------------------------------------------------------
void TCIRowsetChange::FreeMemory()
{
	//make sure m_pData is not NULL
	if(!COMPARE(!m_pData, NULL))
		return;

	//call compareData with the option to free the memory referenced by the consumer's 
	//buffer without comparing data
	CompareData(m_cRowsetCols,m_rgTableColOrds,1,m_pData,m_cBinding,m_rgBinding,
		m_pTable, m_pIMalloc,PRIMARY,FREE_ONLY);

	return;
}

//--------------------------------------------------------------------
//@mfunc: release an accessor created on the rowset object
//
//--------------------------------------------------------------------
void TCIRowsetChange::ReleaseAccessorOnRowset()
{


	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize=0;
	m_cBinding=0;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);

	//free accessor handle
	if(m_hAccessor)
	{
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
		{
			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
			m_hAccessor=NULL;
		}
		else
			odtLog<<L"Error: Can not release accessor on the command object!"<<ENDL;
	}
	
	//free binding structure
	PROVIDER_FREE(m_rgBinding);
}


//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
void TCIRowsetChange::ReleaseRowsetAndAccessor(ULONG RowRefCnt, ULONG CmdRefCnt)
{
	IAccessor *pIAccessor = NULL;

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize=0;
	m_cBinding=0;

	// Set the DBPROP_UPDATABILITY back to DBPROPVAL_UP_CHANGE
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);

	//free accessor handle, if a rowset Accessor
	if(m_hAccessor && m_pIAccessor && m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
		m_hAccessor=NULL;
	}
	
	//free accessor handle, if a command Accessor
	if(m_hAccessor && m_pICommand && m_eAccessorLocation==ON_COMMAND_ACCESSOR)
	{
		//QI for the accessor handle on the command object
		if(CHECK(m_pICommand->QueryInterface(IID_IAccessor, (void**)&pIAccessor),S_OK))
			CHECK(pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);

	}

	//Release accessors
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(m_pIAccessor);
	m_hAccessor=NULL;
	
	//release IRowset pointer
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetChange);
	SAFE_RELEASE(m_pIRowsetIdentity);
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowsetLocate);

	//free binding structure
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgInfo);
	PROVIDER_FREE(m_pStringsBuffer);

	ReleaseRowsetObject(RowRefCnt);
	ReleaseCommandObject(CmdRefCnt);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Boundary)
//--------------------------------------------------------------------
// @class Testing boundary conditions
//
class Boundary : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPID	m_rgPropertyIDs[3];
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE
	int Variation_1();
	// @cmember immediate update mode.  Null Accessor, NULL pData with DBPROPVAL_UP_CHANGE
	int Variation_2();
	// @cmember immediate update mode.  NULL pData.  E_INVALIDARG
	int Variation_3();
	// @cmember buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE
	int Variation_4();
	// @cmember immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE
	int Variation_5();
	// @cmember buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE
	int Variation_6();
	// @cmember immediate update mode.  Valid accessor with bad ordinals
	int Variation_7();
	// @cmember immediate update mode.  OPTIMIZED accessor after a Fetch (DB_E_BADACCESSORFLAGS)
	int Variation_8();
	// @cmember immediate update mode.  Valid accessor with different Parameter Accessor Flags
	int Variation_9();
	// @cmember immediate update mode.  Valid accessor with different Row Accessor Flags
	int Variation_10();
	// @cmember inherited parameter accessors with SetData
	int Variation_11();
	// @cmember immediate update mode.  more Valid accessor with bad ordinals
	int Variation_12();
	// @cmember immediate update mode.  get bookmarks on inheritied accessor
	int Variation_13();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Boundary)
#define THE_CLASS Boundary
BEG_TEST_CASE(Boundary, TCIRowsetChange, L"Testing boundary conditions")
	TEST_VARIATION(1, 		L"immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE")
	TEST_VARIATION(2, 		L"immediate update mode.  Null Accessor, NULL pData with DBPROPVAL_UP_CHANGE")
	TEST_VARIATION(3, 		L"immediate update mode.  NULL pData.  E_INVALIDARG")
	TEST_VARIATION(4, 		L"buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE")
	TEST_VARIATION(5, 		L"immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE")
	TEST_VARIATION(6, 		L"buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE")
	TEST_VARIATION(7, 		L"immediate update mode.  Valid accessor with bad ordinals")
	TEST_VARIATION(8, 		L"immediate update mode.  OPTIMIZED accessor after a Fetch (DB_E_BADACCESSORFLAGS)")
	TEST_VARIATION(9, 		L"immediate update mode.  Valid accessor with different Parameter Accessor Flags")
	TEST_VARIATION(10, 		L"immediate update mode.  Valid accessor with different Row Accessor Flags")
	TEST_VARIATION(11, 		L"inherited parameter accessors with SetData")
	TEST_VARIATION(12, 		L"immediate update mode.  more Valid accessor with bad ordinals")
	TEST_VARIATION(13, 		L"immediate update mode.  get bookmarks on inheritied accessor")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Rowsets)
//--------------------------------------------------------------------
// @class Testing read-only rowset and empty
//
class Rowsets : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowsets,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Read-only rowset with
	int Variation_1();
	// @cmember Read only rowset with left outer join.DB_E_PROPERTIESNOTAVAILABLE.
	int Variation_2();
	// @cmember Read only rowset with right outer join.DB_E_PROPERTIESNOTAVAILABLE.
	int Variation_3();
	// @cmember Empty rowset.  S_OK
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Rowsets)
#define THE_CLASS Rowsets
BEG_TEST_CASE(Rowsets, TCIRowsetChange, L"Testing read-only rowset and empty")
	TEST_VARIATION(1, 		L"Read-only rowset with")
	TEST_VARIATION(2, 		L"Read only rowset with left outer join.DB_E_PROPERTIESNOTAVAILABLE.")
	TEST_VARIATION(3, 		L"Read only rowset with right outer join.DB_E_PROPERTIESNOTAVAILABLE.")
	TEST_VARIATION(4, 		L"Empty rowset.  S_OK")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MayWriteColumn)
//--------------------------------------------------------------------
// @class test DBPROP_MAYWRITECOLUMN
//
class MayWriteColumn : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MayWriteColumn,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Make sure MSDASQL does not support DBPROP_MAYWRITECOLUMN
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(MayWriteColumn)
#define THE_CLASS MayWriteColumn
BEG_TEST_CASE(MayWriteColumn, TCIRowsetChange, L"test DBPROP_MAYWRITECOLUMN")
	TEST_VARIATION(1, 		L"Make sure MSDASQL does not support DBPROP_MAYWRITECOLUMN")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MaxPendingChangeRows)
//--------------------------------------------------------------------
// @class test DBPROP_MAXPENDINGCHANGEROWS
//
class MaxPendingChangeRows : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxPendingChangeRows,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Verify DBPROP_MAXPENDINGROWS
	int Variation_1();
	// @cmember Verify DBPROP_MAXOPENROWS
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(MaxPendingChangeRows)
#define THE_CLASS MaxPendingChangeRows
BEG_TEST_CASE(MaxPendingChangeRows, TCIRowsetChange, L"test DBPROP_MAXPENDINGCHANGEROWS")
	TEST_VARIATION(1, 		L"Verify DBPROP_MAXPENDINGROWS")
	TEST_VARIATION(2, 		L"Verify DBPROP_MAXOPENROWS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CacheDeferred)
//--------------------------------------------------------------------
// @class test DBPROP_CACHEDEFERRED
//
class CacheDeferred : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CacheDeferred,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Kagera does not support DBPROP_CACHDEFERRED
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CacheDeferred)
#define THE_CLASS CacheDeferred
BEG_TEST_CASE(CacheDeferred, TCIRowsetChange, L"test DBPROP_CACHEDEFERRED")
	TEST_VARIATION(1, 		L"to be coded")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(NoColumn_Row_Restrict)
//--------------------------------------------------------------------
// @class test DBPROP_NOCOLUMNRESTRICT and DBPROP_NOROWRESTRICT
//
class NoColumn_Row_Restrict : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(NoColumn_Row_Restrict,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Select Count(*)
	int Variation_1();
	// @cmember Select * from Table.  Restrict on table.  No restrict on column
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(NoColumn_Row_Restrict)
#define THE_CLASS NoColumn_Row_Restrict
BEG_TEST_CASE(NoColumn_Row_Restrict, TCIRowsetChange, L"test DBPROP_NOCOLUMNRESTRICT and DBPROP_NOROWRESTRICT")
	TEST_VARIATION(1, 		L"Select Count(*)")
	TEST_VARIATION(2, 		L"Select * from Table.  Restrict on table.  No restrict on column")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Computed_Columns)
//--------------------------------------------------------------------
// @class test rowset with computed columns
//
class Computed_Columns : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Computed_Columns,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Accessor binds the computed column.  DB_SEC_E_PERMISSIONDENIED
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Computed_Columns)
#define THE_CLASS Computed_Columns
BEG_TEST_CASE(Computed_Columns, TCIRowsetChange, L"test rowset with computed columns")
	TEST_VARIATION(1, 		L"Accessor binds the computed column.  DB_SEC_E_PERMISSIONDENIED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Forward_Query)
//--------------------------------------------------------------------
// @class forward only cursor.  Query based update
//
class Forward_Query : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Forward_Query,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.
	int Variation_1();
	// @cmember Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change
	int Variation_2();
	// @cmember Boundary cases with qbu cursor
	int Variation_3();
	// @cmember DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu
	int Variation_4();
	// @cmember SetData not supported, qbu
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Forward_Query)
#define THE_CLASS Forward_Query
BEG_TEST_CASE(Forward_Query, TCIRowsetChange, L"forward only cursor.  Query based update")
	TEST_VARIATION(1, 		L"Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.")
	TEST_VARIATION(2, 		L"Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change")
	TEST_VARIATION(3, 		L"Boundary cases with qbu cursor")
	TEST_VARIATION(4, 		L"DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu")
	TEST_VARIATION(5, 		L"SetData not supported, qbu")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Forward_Cursor)
//--------------------------------------------------------------------
// @class Forward only cursor.  Cursor based update.
//
class Forward_Cursor : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Forward_Cursor,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.
	int Variation_1();
	// @cmember Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change
	int Variation_2();
	// @cmember Change a row with DBTYPE_BYREF.  cbMaxLen > string length, no truncation should occur, SetData should return S_OK.
	int Variation_3();
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_4();
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_5();
	// @cmember Insert a row and check to see it (OWNINSERT is off)
	int Variation_6();
	// @cmember set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, SetData
	int Variation_7();
	// @cmember set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, Insert
	int Variation_8();
	// @cmember Fail the SetData and check that the status of the remaining unset cols is DBSTATUS_E_UNAVAILABLE
	int Variation_9();
	// @cmember The accessor only has status binding for DBSTATUS_S_OK
	int Variation_10();
	// @cmember The accessor only has status binding for DBSTATUS_S_ISNULL
	int Variation_11();
	// @cmember The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
	int Variation_12();
	// @cmember SetData the variable length columns without the length being bound. should be ok
	int Variation_13();
	// @cmember Buffered update mode. Multiple commands that modify same set of columns in hrow
	int Variation_14();
	// @cmember SetData in a read only rowset
	int Variation_15();
	// @cmember The accessor only has length binding.  E_FAIL.
	int Variation_16();
	// @cmember The length binding > cbMaxLen for a variable length column. (DB_S_ERRORSOCCURRED)
	int Variation_17();
	// @cmember check for truncation in variable length columns
	int Variation_18();
	// @cmember Delete-Insert-undoDelete-Update
	int Variation_19();
	// @cmember check status NULL if no row handles used
	int Variation_20();
	// @cmember The length binding > cbMaxLen for a variable length column with no status bound. (DB_S_ERRORSOCCURRED)
	int Variation_21();
	// @cmember check for truncation in variable length columns with no status bound.
	int Variation_22();
	// @cmember QI check
	int Variation_23();
	// @cmember check for truncation in fixed length columns
	int Variation_24();
	// @cmember bind BLOBs as IUnKnown
	int Variation_25();
	// @cmember DBPROP_IRowsetChange FALSE with UPDATABILITY - conflicting
	int Variation_26();	
	// @cmember DBPROP_IRowsetChange and DBPROP_IRowsetUpdate
	int Variation_27();	
	// @cmember change multiple cols with some failures
	int Variation_28();	
	// @cmember set no data in a BLOB column
	int Variation_29();	
	// @cmember InsertRows and DBPROP_MAX
	int Variation_30();	
	// @cmember Insert no BLOB, change BLOB
	int Variation_31();	

// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Forward_Cursor)
#define THE_CLASS Forward_Cursor
BEG_TEST_CASE(Forward_Cursor, TCIRowsetChange, L"Forward only cursor.  Cursor based update.")
	TEST_VARIATION(1, 		L"Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.")
	TEST_VARIATION(2, 		L"Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change")
	TEST_VARIATION(3, 		L"Change a row with DBTYPE_BYREF.  cbMaxLen > string length, no truncation should occur, SetData should return S_OK.")
	TEST_VARIATION(4, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(5, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(6, 		L"Insert a row and check to see it (OWNINSERT is off)")
	TEST_VARIATION(7, 		L"Set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, SetData")
	TEST_VARIATION(8, 		L"Set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, Insert")
	TEST_VARIATION(9, 		L"Fail the SetData and check that the status of the remaining unset cols is DBSTATUS_E_UNAVAILABLE")
	TEST_VARIATION(10, 		L"The accessor only has status binding for DBSTATUS_S_OK")
	TEST_VARIATION(11, 		L"The accessor only has status binding for DBSTATUS_S_ISNULL")
	TEST_VARIATION(12, 		L"The status flag specified in a binding structure is neither DBSTATUS_S_OK nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE")
	TEST_VARIATION(13, 		L"SetData the variable length columns without the length being bound. should be ok")
	TEST_VARIATION(14, 		L"Buffered update mode. Multiple commands that modify same set of columns in hrow")
	TEST_VARIATION(15, 		L"SetData in a read only rowset")
	TEST_VARIATION(16, 		L"The accessor only has length binding.  E_FAIL.")
	TEST_VARIATION(17, 		L"The length binding > cbMaxLen for a variable length column. (DB_S_ERRORSOCCURRED)")
	TEST_VARIATION(18, 		L"check for truncation in variable length columns.")
	TEST_VARIATION(19, 		L"Delete-Insert-undoDelete-Update")
	TEST_VARIATION(20, 		L"check status NULL if no row handles used")
	TEST_VARIATION(21, 		L"The length binding > cbMaxLen for a variable length column with no status bound. (DB_S_ERRORSOCCURRED)")
	TEST_VARIATION(22, 		L"check for truncation in variable length columns with no status bound.")
	TEST_VARIATION(23, 		L"QI check.")
	TEST_VARIATION(24, 		L"check for truncation in fixed length columns.")
	TEST_VARIATION(25, 		L"bind BLOBs as IUnKnown")
	TEST_VARIATION(26, 		L"DBPROP_IRowsetChange FALSE with UPDATABILITY - conflicting")
	TEST_VARIATION(27, 		L"DBPROP_IRowsetChange and DBPROP_IRowsetUpdate")
	TEST_VARIATION(28, 		L"change multiple cols with some failures")
	TEST_VARIATION(29, 		L"set no data in a BLOB column")
	TEST_VARIATION(30, 		L"InsertRows and DBPROP_MAX")
	TEST_VARIATION(31, 		L"Insert no BLOB, change BLOB")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(QueryBaseUpdates)
//--------------------------------------------------------------------
// @class Test QueryBaseUpdates that update more than one rows.
//
class QueryBaseUpdates : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(QueryBaseUpdates,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Immediate update mode. Update one non-key column with values that affect more than one rows.  Verify the change is visible
	int Variation_1();
	// @cmember Buffered update mode.  Update one non-key column with values that affect more than one rows.  Verify the change is visible the
	int Variation_2();
	// @cmember Boundary checks from a static cursor
	int Variation_3();
	// @cmember NEWLYINSERTED from a static cursor
	int Variation_4();
	// @cmember key non-key cols, set 2 rows the same and delete
	int Variation_5();
	// @cmember Delete rows from a static cursor/immediate mode, delete not supported
	int Variation_6();
	// @cmember delete with a row handle from an unallocated rowset
	int Variation_7();
	// @cmember delete with a row handle from an allocated rowset
	int Variation_8();
	// @cmember Boundary cases with update pending
	int Variation_9();
	// @cmember BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu
	int Variation_10();
	// @cmember SetData not supported, qbu
	int Variation_11();
	// @cmember QBU with just floats
	int Variation_12();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(QueryBaseUpdates)
#define THE_CLASS QueryBaseUpdates
BEG_TEST_CASE(QueryBaseUpdates, TCIRowsetChange, L"Test QueryBaseUpdates that update more than one rows.")
	TEST_VARIATION(1, 		L"Immediate update mode. Update one non-key column with values that affect more than one rows.  Verify the change is visible")
	TEST_VARIATION(2, 		L"Buffered update mode.  Update one non-key column with values that affect more than one rows.  Verify the change is visible")
	TEST_VARIATION(3, 		L"Boundary checks from a update pending")
	TEST_VARIATION(4, 		L"NEWLYINSERTED from update pending")
	TEST_VARIATION(5, 		L"key non-key cols, set 2 rows the same and delete")
	TEST_VARIATION(6, 		L"Delete rows from a update pending mode, delete not supported")
	TEST_VARIATION(7, 		L"delete with a row handle from an unallocated rowset")
	TEST_VARIATION(8, 		L"delete with a row handle from an allocated rowset")
	TEST_VARIATION(9, 		L"Boundary cases with update pending")
	TEST_VARIATION(10, 		L"BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu")
	TEST_VARIATION(11, 		L"SetData not supported, qbu")
	TEST_VARIATION(12, 		L"QBU with just floats")
	END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Static_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Static cursor in immediate update mode.
//
class Static_Cursor_Immediate : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Cursor_Immediate,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Update a row to be the first row in the row set with some columns set to NULL, and call GetData to see.
	int Variation_1();
	// @cmember Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again.
	int Variation_2();
	// @cmember Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again
	int Variation_3();
	// @cmember Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Static_Cursor_Immediate)
#define THE_CLASS Static_Cursor_Immediate
BEG_TEST_CASE(Static_Cursor_Immediate, TCIRowsetChange, L"Static cursor in immediate update mode.")
	TEST_VARIATION(1, 		L"Update a row to be the first row in the row set with some columns set to NULL, and call GetData to see.")
	TEST_VARIATION(2, 		L"Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again.")
	TEST_VARIATION(3, 		L"Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again")
	TEST_VARIATION(4, 		L"Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Static_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Static_Cursor_Buffered
//
class Static_Cursor_Buffered : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Cursor_Buffered,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_1();
	// @cmember Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
	int Variation_2();
	// @cmember Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
	int Variation_3();
	// @cmember Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Static_Cursor_Buffered)
#define THE_CLASS Static_Cursor_Buffered
BEG_TEST_CASE(Static_Cursor_Buffered, TCIRowsetChange, L"Static_Cursor_Buffered")
	TEST_VARIATION(1, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(2, 		L"Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again")
	TEST_VARIATION(3, 		L"Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.")
	TEST_VARIATION(4, 		L"Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Keyset_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Keyset_Cursor_Buffered
//
class Keyset_Cursor_Buffered : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Cursor_Buffered,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_1();
	// @cmember Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
	int Variation_2();
	// @cmember Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Keyset_Cursor_Buffered)
#define THE_CLASS Keyset_Cursor_Buffered
BEG_TEST_CASE(Keyset_Cursor_Buffered, TCIRowsetChange, L"Keyset_Cursor_Buffered")
	TEST_VARIATION(1, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(2, 		L"Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again")
	TEST_VARIATION(3, 		L"Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Keyset_Remove_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Keyset_Remove_Curosr_Buffered
//
class Keyset_Remove_Cursor_Immediate : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Remove_Cursor_Immediate,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_1();
	// @cmember DBPROP_CHANGEINSERTEDROWS is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED
	int Variation_2();
	// @cmember DBPROP_SERVERDATAONINSERT is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED
	int Variation_3();
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_4();
	// @cmember fetch newly deleted row
	int Variation_5();
	// @cmember Insert a row and check to see it (OWNINSERT is off)
	int Variation_6();
	// @cmember  Boundary cases cursor.
	int Variation_7();
	// @cmember BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
	int Variation_8();
	// @cmember SetData not supported.
	int Variation_9();
	// @cmember Fetch after a deleted row 
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Keyset_Remove_Cursor_Immediate)
#define THE_CLASS Keyset_Remove_Cursor_Immediate
BEG_TEST_CASE(Keyset_Remove_Cursor_Immediate, TCIRowsetChange, L"Keyset_Remove_Curosr_Buffered")
	TEST_VARIATION(1, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(2, 		L"DBPROP_CHANGEINSERTEDROWS is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED")
	TEST_VARIATION(3, 		L"DBPROP_SERVERDATAONINSERT is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED")
	TEST_VARIATION(4, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(5, 		L"Fetch newly deleted row")
	TEST_VARIATION(6, 		L"Insert a row and check to see it (OWNINSERT is off)")
	TEST_VARIATION(7, 		L"Boundary cases cursor.")
	TEST_VARIATION(8, 		L"BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE")
	TEST_VARIATION(9, 		L"SetData not supported.")
	TEST_VARIATION(10, 		L"Fetch after a deleted row.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Keyset_Remove_Buffered)
//--------------------------------------------------------------------
// @class Keyset_Remove_Buffered
//
class Keyset_Remove_Buffered : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Remove_Buffered,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_1();
	// @cmember Retrieve row, update row update pending don't release handle, GetData RETURNPENDINGINSERTS FALSE
	int Variation_2();
	// @cmember Error should NULL out hRow
	int Variation_3();
	// @cmember DB_E_ROWSNOTRELEASED
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Keyset_Remove_Buffered)
#define THE_CLASS Keyset_Remove_Buffered
BEG_TEST_CASE(Keyset_Remove_Buffered, TCIRowsetChange, L"Keyset_Remove_Buffered")
	TEST_VARIATION(1, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(2, 		L"Retrieve row, update row update pending don't release handle, GetData RETURNPENDINGINSERTS FALSE")
	TEST_VARIATION(3, 		L"Error should NULL out hRow")
	TEST_VARIATION(4, 		L"DB_E_ROWSNOTRELEASED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Dynamic_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Dynamic_Cursor_Buffered
//
class Dynamic_Cursor_Buffered : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic_Cursor_Buffered,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
	int Variation_1();
	// @cmember Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
	int Variation_2();
	// @cmember Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Dynamic_Cursor_Buffered)
#define THE_CLASS Dynamic_Cursor_Buffered
BEG_TEST_CASE(Dynamic_Cursor_Buffered, TCIRowsetChange, L"Dynamic_Cursor_Buffered")
	TEST_VARIATION(1, 		L"Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see")
	TEST_VARIATION(2, 		L"Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again")
	TEST_VARIATION(3, 		L"Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Forward_Cursor)
//--------------------------------------------------------------------
// @class visibility of row handles
//
class Visible_Forward_Cursor : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Forward_Cursor,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember In immediate update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.
	int Variation_1();
	// @cmember In buffered update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Forward_Cursor)
#define THE_CLASS Visible_Forward_Cursor
BEG_TEST_CASE(Visible_Forward_Cursor, TCIRowsetChange, L"visibility of row handles")
	TEST_VARIATION(1, 		L"In immediate update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.")
	TEST_VARIATION(2, 		L"In buffered update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Static_Cursor)
//--------------------------------------------------------------------
// @class visibility of row handles
//
class Visible_Static_Cursor : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Static_Cursor,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember In immediate update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset.  RestartPosition
	int Variation_1();
	// @cmember In buffered update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset. RestartPosition
	int Variation_2();
	// @cmember In immediate update mode.  Create two rowsets on the same table.  One rowset 	changes a key value.  The other rowset should no
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Static_Cursor)
#define THE_CLASS Visible_Static_Cursor
BEG_TEST_CASE(Visible_Static_Cursor, TCIRowsetChange, L"visibility of row handles")
	TEST_VARIATION(1, 		L"In immediate update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset.  RestartPosition")
	TEST_VARIATION(2, 		L"In buffered update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset. RestartPosition")
	TEST_VARIATION(3, 		L"In immediate update mode.  Create two rowsets on the same table.  One rowset 	changes a key value.  The other rowset should no")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Keyset_Command_Cursor)
//--------------------------------------------------------------------
// @class Visible_Keyset_Command_Cursor
//
class Visible_Keyset_Command_Cursor : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Keyset_Command_Cursor,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row
	int Variation_1();
	// @cmember In immediate update mode.  Create two rowsets on the same table.  The second rowset change a non key column.
	int Variation_2();
	// @cmember In buffered update mode.  Create two rowsets on the same table.  The second rowset change a row
	int Variation_3();
	// @cmember In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non key column.
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Keyset_Command_Cursor)
#define THE_CLASS Visible_Keyset_Command_Cursor
BEG_TEST_CASE(Visible_Keyset_Command_Cursor, TCIRowsetChange, L"Visible_Keyset_Command_Cursor")
	TEST_VARIATION(1, 		L"In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row")
	TEST_VARIATION(2, 		L"In immediate update mode.  Create two rowsets on the same table.  The second rowset change a non key column.")
	TEST_VARIATION(3, 		L"In buffered update mode.  Create two rowsets on the same table.  The second rowset change a row")
	TEST_VARIATION(4, 		L"In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non key column.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Dynamic_Command_Cursor)
//--------------------------------------------------------------------
// @class Visible_Dynamic_Command_Cursor
//
class Visible_Dynamic_Command_Cursor : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Dynamic_Command_Cursor,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row
	int Variation_1();
	// @cmember In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non-key column.
	int Variation_2();
	// @cmember Immediate update mode. A second command object deletes one row by SQL text.
	int Variation_3();
	// @cmember Immediate update mode. A second commnd object changes one row by SQL text.
	int Variation_4();
	// @cmember Buffered update mode. A second commnd object changes one row by SQL text.
	int Variation_5();
	// @cmember DBPROP_CHANGEINSERTEDROWS-TRUE.A second commnd object change a row by SQL text.It tries to change same row again-succeeds.
	int Variation_6();
	// @cmember DBPROP_CHANGEINSERTEDROWS-FALSE.A second commnd object change a row by SQL text.It tries to change same row again-fails.
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Dynamic_Command_Cursor)
#define THE_CLASS Visible_Dynamic_Command_Cursor
BEG_TEST_CASE(Visible_Dynamic_Command_Cursor, TCIRowsetChange, L"Visible_Dynamic_Command_Cursor")
	TEST_VARIATION(1, 		L"In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row")
	TEST_VARIATION(2, 		L"In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non-key column.")
	TEST_VARIATION(3, 		L"Immediate update mode. A second command object deletes one row by SQL text.")
	TEST_VARIATION(4, 		L"Immediate update mode. A second commnd object changes one row by SQL text.")
	TEST_VARIATION(5, 		L"Buffered update mode. A second commnd object changes one row by SQL text.")
	TEST_VARIATION(6, 		L"DBPROP_CHANGEINSERTEDROWS-TRUE.A second commnd object change a row by SQL text.It tries to change same row again-succeeds.")
	TEST_VARIATION(7, 		L"DBPROP_CHANGEINSERTEDROWS-FALSE.A second commnd object change a row by SQL text.It tries to change same row again-fails.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Bookmark_Forward)
//--------------------------------------------------------------------
// @class Bookmark_Forward
//
class Bookmark_Forward : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmark_Forward,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
	int Variation_1();
	// @cmember buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
	int Variation_2();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true
	int Variation_3();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-false,BMS-true
	int Variation_4();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false
	int Variation_5();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-false,BMS-false
	int Variation_6();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-true
	int Variation_7();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-false
	int Variation_8();
	// @cmember delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
	int Variation_9();
	// @cmember delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Bookmark_Forward)
#define THE_CLASS Bookmark_Forward
BEG_TEST_CASE(Bookmark_Forward, TCIRowsetChange, L"Bookmark_Forward")
	TEST_VARIATION(1, 		L"immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt")
	TEST_VARIATION(2, 		L"buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.")
	TEST_VARIATION(3, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(4, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-false,BMS-true")
	TEST_VARIATION(5, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(6, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-false,BMS-false")
	TEST_VARIATION(7, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(8, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(9, 		L"delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true")
	TEST_VARIATION(10, 		L"delete last row of 1 row table and try to get bookmark. RD=true,BMS=true")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Bookmark_Static)
//--------------------------------------------------------------------
// @class Bookmark_Static
//
class Bookmark_Static : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmark_Static,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
	int Variation_1();
	// @cmember buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Bookmark_Static)
#define THE_CLASS Bookmark_Static
BEG_TEST_CASE(Bookmark_Static, TCIRowsetChange, L"Bookmark_Static")
	TEST_VARIATION(1, 		L"immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt")
	TEST_VARIATION(2, 		L"buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Bookmark_Keyset)
//--------------------------------------------------------------------
// @class Bookmark_Keyset
//
class Bookmark_Keyset : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmark_Keyset,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
	int Variation_1();
	// @cmember buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
	int Variation_2();
	// @cmember Immediate update mode.  No row revisited.
	int Variation_3();
	// @cmember Buffered update mode.  No row revisited.
	int Variation_4();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true
	int Variation_5();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false
	int Variation_6();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-true
	int Variation_7();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-false
	int Variation_8();
	// @cmember delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
	int Variation_9();
	// @cmember delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Bookmark_Keyset)
#define THE_CLASS Bookmark_Keyset
BEG_TEST_CASE(Bookmark_Keyset, TCIRowsetChange, L"Bookmark_Keyset")
	TEST_VARIATION(1, 		L"immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt")
	TEST_VARIATION(2, 		L"buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.")
	TEST_VARIATION(3, 		L"Immediate update mode.  No row revisited.")
	TEST_VARIATION(4, 		L"Buffered update mode.  No row revisited.")
	TEST_VARIATION(5, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(6, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(7, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(8, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(9, 		L"delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true")	
	TEST_VARIATION(10, 		L"delete last row of 1 row table and try to get bookmark. RD=true,BMS=true")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Bookmark_Dynamic)
//--------------------------------------------------------------------
// @class Bookmark_Dynamic
//
class Bookmark_Dynamic : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Bookmark_Dynamic,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
	int Variation_1();
	// @cmember buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
	int Variation_2();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true
	int Variation_3();
	// @cmember delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false
	int Variation_4();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-true
	int Variation_5();
	// @cmember delete last row then make sure bookmark sees the new last row. RD-true,BMS-false
	int Variation_6();
	// @cmember delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
	int Variation_7();
	// @cmember delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
	int Variation_8();
// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Bookmark_Dynamic)
#define THE_CLASS Bookmark_Dynamic
BEG_TEST_CASE(Bookmark_Dynamic, TCIRowsetChange, L"Bookmark_Dynamic")
	TEST_VARIATION(1, 		L"immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt")
	TEST_VARIATION(2, 		L"buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.")
	TEST_VARIATION(3, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(4, 		L"delete last row then make sure BMK_LAST sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(5, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-true")
	TEST_VARIATION(6, 		L"delete last row then make sure bookmark sees the new last row. RD-true,BMS-false")
	TEST_VARIATION(7, 		L"delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true")
	TEST_VARIATION(8, 		L"delete last row of 1 row table and try to get bookmark. RD=true,BMS=true")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(OrderedBookmark_Keyset)
//--------------------------------------------------------------------
// @class OrderedBookmark_Keyset
//
class OrderedBookmark_Keyset : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(OrderedBookmark_Keyset,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember immediate update mode.  Create two rowsets on the same table.  One rowset 	changes one row.
	int Variation_1();
	// @cmember buffered update mode.  Create two rowsets on the same table.  The second row set changed a row.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(OrderedBookmark_Keyset)
#define THE_CLASS OrderedBookmark_Keyset
BEG_TEST_CASE(OrderedBookmark_Keyset, TCIRowsetChange, L"OrderedBookmark_Keyset")
	TEST_VARIATION(1, 		L"immediate update mode.  Create two rowsets on the same table.  One rowset 	changes one row.")
	TEST_VARIATION(2, 		L"buffered update mode.  Create two rowsets on the same table.  The second row set changed a row.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Invalid_Keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Invalid_Keyset_Cursor_Immediate
//
class Invalid_Keyset_Cursor_Immediate : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Invalid_Keyset_Cursor_Immediate,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The accessor is DBACCESSOR_READ | DBACCCESOR_PASSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
	int Variation_1();
	// @cmember The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
	int Variation_2();
	// @cmember The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
	int Variation_3();
	// @cmember The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE
	int Variation_4();
	// @cmember Set a duplicate column on which a unique index is created. 	DB_E_INTEGRITYVIOLATION.
	int Variation_5();
	// @cmember The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta
	int Variation_6();
	// @cmember The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b
	int Variation_7();
	// @cmember The accessor only has length binding.  E_FAIL.(See testing issue #2
	int Variation_8();
	// @cmember The accessor only has status and length binding.  Some the columns are not set 	to NULL.  E_FAIL.
	int Variation_9();
	// @cmember Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED
	int Variation_10();
	// @cmember Setting a bookmark column DBSTATUS_E_?
	int Variation_11();
	// @cmember asking for no REMOVEDELETED on dynamic cursor
	int Variation_12();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Invalid_Keyset_Cursor_Immediate)
#define THE_CLASS Invalid_Keyset_Cursor_Immediate
BEG_TEST_CASE(Invalid_Keyset_Cursor_Immediate, TCIRowsetChange, L"Invalid_Keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"The accessor is DBACCESSOR_READ | DBACCCESOR_PASSBYREF. DB_E_READONLYACCESSOR or E_FAIL.")
	TEST_VARIATION(2, 		L"The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.")
	TEST_VARIATION(3, 		L"The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE")
	TEST_VARIATION(4, 		L"The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE")
	TEST_VARIATION(5, 		L"Set a duplicate column on which a unique index is created. 	DB_E_INTEGRITYVIOLATION.")
	TEST_VARIATION(6, 		L"The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta")
	TEST_VARIATION(7, 		L"The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b")
	TEST_VARIATION(8, 		L"The accessor only has length binding.  E_FAIL.(See testing issue #2")
	TEST_VARIATION(9, 		L"The accessor only has status and length binding.  Some the columns are not set 	to NULL.  E_FAIL.")
	TEST_VARIATION(10, 		L"Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED")
	TEST_VARIATION(11, 		L"Setting a bookmark column DBSTATUS_E_?")
	TEST_VARIATION(12, 		L"asking for no REMOVEDELETED on dynamic cursor")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Invalid_Keyset_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Invalid_Keyset_Cursor_Buffered
//
class Invalid_Keyset_Cursor_Buffered : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Invalid_Keyset_Cursor_Buffered,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The Accessor is read-only accessor. DB_E_READONLYACCESSOR or E_FAIL.
	int Variation_1();
	// @cmember The accessor is DBACCESSOR_READ | DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
	int Variation_2();
	// @cmember Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED
	int Variation_3();
	// @cmember The status flag specified in a binding structure is MAX(DWORD
	int Variation_4();
	// @cmember Change the primary key of two rows to be the same. 	DB_E_INTEGRITYVIOLATION.
	int Variation_5();
	// @cmember The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta
	int Variation_6();
	// @cmember The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b
	int Variation_7();
	// @cmember The accessor only has status binding for DBSTATUS_S_OK.  E_FAIL.
	int Variation_8();
	// @cmember The accessor is not a row accessor.  DB_E_BADACCESSORTYPE.
	int Variation_9();
	// @cmember REMOVEDELTED FALSE, LITERALIDENITTY TRUE in buffered mode.
	int Variation_10();
	// @cmember REMOVEDELTED FALSE, LITERALIDENITTY FALSE in buffered mode.
	int Variation_11();
	// @cmember REMOVEDELTED TRUE, LITERALIDENITTY TRUE in buffered mode.
	int Variation_12();
	// @cmember REMOVEDELTED TRUE, LITERALIDENITTY FALSE in buffered mode.
	int Variation_13();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Invalid_Keyset_Cursor_Buffered)
#define THE_CLASS Invalid_Keyset_Cursor_Buffered
BEG_TEST_CASE(Invalid_Keyset_Cursor_Buffered, TCIRowsetChange, L"Invalid_Keyset_Cursor_Buffered")
	TEST_VARIATION(1, 		L"The Accessor is read-only accessor. DB_E_READONLYACCESSOR or E_FAIL.")
	TEST_VARIATION(2, 		L"The accessor is DBACCESSOR_READ | DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.")
	TEST_VARIATION(3, 		L"Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED")
	TEST_VARIATION(4, 		L"The status flag specified in a binding structure is MAX(DWORD")
	TEST_VARIATION(5, 		L"Change the primary key of two rows to be the same. 	DB_E_INTEGRITYVIOLATION.")
	TEST_VARIATION(6, 		L"The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta")
	TEST_VARIATION(7, 		L"The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b")
	TEST_VARIATION(8, 		L"The accessor only has status binding for DBSTATUS_S_OK.  E_FAIL.")
	TEST_VARIATION(9, 		L"The accessor is not a row accessor.  DB_E_BADACCESSORTYPE.")
	TEST_VARIATION(10, 		L"REMOVEDELTED FALSE, LITERALIDENITTY TRUE in buffered mode.")
	TEST_VARIATION(11, 		L"REMOVEDELTED FALSE, LITERALIDENITTY FALSE in buffered mode.")
	TEST_VARIATION(12, 		L"REMOVEDELTED TRUE, LITERALIDENITTY TRUE in buffered mode.")
	TEST_VARIATION(13, 		L"REMOVEDELTED TRUE, LITERALIDENITTY FALSE in buffered mode.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Valid_Keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Valid_Keyset_Cursor_Immediate
//
class Valid_Keyset_Cursor_Immediate : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPID	m_rgPropertyIDs[3];
	ULONG	m_cGuid;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Valid_Keyset_Cursor_Immediate,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Change fixed length columns, in FORWARD order of the rowset.
	int Variation_1();
	// @cmember Change the fixed length data type columns with bogus length information.
	int Variation_2();
	// @cmember Change the variable length columns with only value binding.
	int Variation_3();
	// @cmember Change the whole row with status and value binding.  Set some columns to 	NULL.
	int Variation_4();
	// @cmember Change a row with DBTYPE_BYREF.
	int Variation_5();
	// @cmember Change a row with DBTYPE_ARRAY.
	int Variation_6();
	// @cmember Change a row with DBTYPE_VECTOR.
	int Variation_7();
	// @cmember Duplicate column bindings in the same accessor.
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Valid_Keyset_Cursor_Immediate)
#define THE_CLASS Valid_Keyset_Cursor_Immediate
BEG_TEST_CASE(Valid_Keyset_Cursor_Immediate, TCIRowsetChange, L"Valid_Keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Change fixed length columns, in FORWARD order of the rowset.")
	TEST_VARIATION(2, 		L"Change the fixed length data type columns with bogus length information.")
	TEST_VARIATION(3, 		L"Change the variable length columns with only value binding.")
	TEST_VARIATION(4, 		L"Change the whole row with status and value binding.  Set some columns to 	NULL.")
	TEST_VARIATION(5, 		L"Change a row with DBTYPE_BYREF.")
	TEST_VARIATION(6, 		L"Change a row with DBTYPE_ARRAY.")
	TEST_VARIATION(7, 		L"Change a row with DBTYPE_VECTOR.")
	TEST_VARIATION(8, 		L"Duplicate column bindings in the same accessor.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(Related_Delete_NewRow)
//--------------------------------------------------------------------
// @class Related interfaces with IRowsetDelete and IRowsetNewRow
//
class Related_Delete_NewRow : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_Delete_NewRow,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Delete one row.  Add a new row and change it.  Immediate update mode.
	int Variation_1();
	// @cmember Delete one row.  Add a new row and change it.  Query and buffered update mode.
	int Variation_2();
	// @cmember Delete rows from a static cursor/immediate update mode.
	int Variation_3();
	// @cmember Boundary checks from a static cursor.
	int Variation_4();
	// @cmember NEWLYINSERTED from a static cursor.
	int Variation_5();
	// @cmember key non-key cols, set 2 rows the same and delete
	int Variation_6();
	// @cmember Delete rows from a static cursor/immediate mode, delete not supported
	int Variation_7();
	// @cmember delete with a row handle from an unallocated row handle
	int Variation_8();
	// @cmember delete with a row handle from an allocated row handle
	int Variation_9();
	// @cmember insert a row update then Hard Delete the row
	int Variation_10();
	// @cmember delete the same row handle
	int Variation_11();
//
//
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Related_Delete_NewRow)
#define THE_CLASS Related_Delete_NewRow
BEG_TEST_CASE(Related_Delete_NewRow, TCIRowsetChange, L"Related interfaces with IRowsetDelete and IRowsetNewRow")
	TEST_VARIATION(1, 		L"Delete one row.  Add a new row and change it.  Immediate update mode.")
	TEST_VARIATION(2, 		L"Delete one row.  Add a new row and change it.  Query and buffered update mode.")
	TEST_VARIATION(3, 		L"Delete rows from a static cursor/immediate update mode.")
	TEST_VARIATION(4, 		L"Boundary checks from a static cursor.")
	TEST_VARIATION(5, 		L"NEWLYINSERTED from a static cursor.")
	TEST_VARIATION(6, 		L"key non-key cols, set 2 rows the same and delete.")
	TEST_VARIATION(7, 		L"Delete rows from a static cursor/immediate mode, delete not supported")
	TEST_VARIATION(8, 		L"delete with a row handle from an unallocated row handle")
	TEST_VARIATION(9, 		L"delete with a row handle from an allocated row handle")
	TEST_VARIATION(10, 		L"insert a row update then Hard Delete the row,")
	TEST_VARIATION(11, 		L"delete the same row handle")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Sequence)
//--------------------------------------------------------------------
// @class sequence testing
//
class Sequence : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Sequence,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Call SetData three times on the same row in an immediately update mode.  One attempt is not successful.
	int Variation_1();
	// @cmember Call SetData on two different row handles in an immediately update mode. One attempt is not successful
	int Variation_2();
	// @cmember Call SetData twice on the same row in a buffered update mode.  The second attempt is not successful.
	int Variation_3();
	// @cmember In buffered update mode, retrieve 50 row handles, call IRowsetChange on each row handle,  then call IRowstUpdate::Update
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Sequence)
#define THE_CLASS Sequence
BEG_TEST_CASE(Sequence, TCIRowsetChange, L"sequence testing")
	TEST_VARIATION(1, 		L"Call SetData three times on the same row in an immediately update mode.  One attempt is not successful.")
	TEST_VARIATION(2, 		L"Call SetData on two different row handles in an immediately update mode. One attempt is not successful")
	TEST_VARIATION(3, 		L"Call SetData twice on the same row in a buffered update mode.  The second attempt is not successful.")
	TEST_VARIATION(4, 		L"In buffered update mode, retrieve 50 row handles, call IRowsetChange on each row handle,  then call IRowstUpdate::Update")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Transaction)
//--------------------------------------------------------------------
// @class Testing IRowsetChange within a transaction
//
class Transaction : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPSET	m_DBPropSet;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Transaction,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with fRetaining=TRUE.  Cursor based.
	int Variation_1();
	// @cmember Commit with fRetaining=FALSE. Query based.
	int Variation_2();
	// @cmember Abort with fRetaining=TRUE.  Query based.
	int Variation_3();
	// @cmember Abort with fRetaining=FALSE.  Cursor based.
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Transaction)
#define THE_CLASS Transaction
BEG_TEST_CASE(Transaction, CTransaction, L"Testing IRowsetChange within a transaction")
	TEST_VARIATION(1, 		L"Commit with fRetaining=TRUE.  Cursor based.")
	TEST_VARIATION(2, 		L"Commit with fRetaining=FALSE. Query based.")
	TEST_VARIATION(3, 		L"Abort with fRetaining=TRUE.  Query based.")
	TEST_VARIATION(4, 		L"Abort with fRetaining=FALSE.  Cursor based.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ExtendedErrors : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid DeleteRows calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid DeleteRows calls with previous error object existing
	int Variation_2();
	// @cmember Invalid DeleteRows calls with no previous error object existing
	int Variation_3();
	// @cmember Valid SetData calls with previous error object existing.
	int Variation_4();
	// @cmember Invalid SetData calls with previous error object existing
	int Variation_5();
	// @cmember Invalid SetData calls with no previous error object existing
	int Variation_6();
	// @cmember Valid InsertRow calls with previous error object existing.
	int Variation_7();
	// @cmember Invalid InsertRow calls with previous error object existing
	int Variation_8();
	// @cmember Invalid InsertRow calls with no previous error object existing
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, TCIRowsetChange, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid DeleteRows calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid DeleteRows calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid DeleteRows calls with no previous error object existing")
	TEST_VARIATION(4, 		L"Valid SetData calls with previous error object existing.")
	TEST_VARIATION(5, 		L"Invalid SetData calls with previous error object existing")
	TEST_VARIATION(6, 		L"Invalid SetData calls with no previous error object existing")
	TEST_VARIATION(7, 		L"Valid InsertRow calls with previous error object existing.")
	TEST_VARIATION(8, 		L"Invalid InsertRow calls with previous error object existing")
	TEST_VARIATION(9, 		L"Invalid InsertRow calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ProviderOwnedMem)
//--------------------------------------------------------------------
// @class Use ProviderOwned Memory for SetData
//
class ProviderOwnedMem : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ProviderOwnedMem,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ProdiverOwnedMem for SetData.  Keyset_Query_Buffered
	int Variation_1();
	// @cmember ProdiverOwnedMem for SetData.  Keyset_Cursor_Imm
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ProviderOwnedMem)
#define THE_CLASS ProviderOwnedMem
BEG_TEST_CASE(ProviderOwnedMem, TCIRowsetChange, L"Use ProviderOwned Memory for SetData")
	TEST_VARIATION(1, 		L"ProdiverOwnedMem for SetData.  Keyset_Query_Buffered")
	TEST_VARIATION(2, 		L"ProdiverOwnedMem for SetData.  Keyset_Cursor_Imm")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(SetDefault)
//*-----------------------------------------------------------------------
// @class Default value in data setting operations
//
class SetDefault : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	CTable		*m_pCustomTable;	// a custom table (ask for def cols, etc)
	BOOL		*m_rgbDefault;		// array indicating the defaltable types
	ULONG		m_nRows;			// initial number of rows in the table
	BOOL		m_fCustomTables;	// if customized tables can be built
	ULONG		m_cSeed;			// seed used to build default values


	// @cmember Builds a list of info about the default property of the columns
	BOOL		GetDefaultColumns();

	// @cmember mask the m_fHasDefault field of columns in m_pTable
	void		MaskDefColumns(BOOL fMask);

	// @cmember Creates a rowset, accessors and fill input bindings
	BOOL		PrepareForSetData(
		DBORDINAL		cSelectedColumn,	// [in]	the ordinal of the selected column (1 based)
		DBSTATUS		Status,				// [in] the status value for the selected column
		DBCOUNTITEM		lRowsOffset,		// [in] row number for data creation
		BYTE			**ppData			// [out] data buffer
	);

	// @cmember SetData, check it, get data, check it 
	virtual BOOL	SetAndCheckDefault(
		BYTE	*pData,				// [in] buffer for IRowsetChange::SetData
		HRESULT	hrSetDataExpected,	// [in] expected hr for IRowsetChange::SetData
		BOOL	fValidate = TRUE,	// [in] validation flag
		HRESULT	*hrSetData = NULL	// [out] actual result of IRowsetChange::SetData
	);

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetDefault,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Set default values on all default columns
	int Variation_1();
	// @cmember Set default value on a default column; set many columns, one is asked default
	int Variation_2();
	// @cmember Set default value on a not nullable default column
	int Variation_3();
	// @cmember Set default value on a nullable default column (def == NULL)
	int Variation_4();
	// @cmember Set default value on a nullable default column (def != NULL)
	int Variation_5();
	// @cmember Set default value on a not default column
	int Variation_6();
	// @cmember Set default value on a unique default value
	int Variation_7();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(SetDefault)
#define THE_CLASS SetDefault
BEG_TEST_CASE(SetDefault, TCIRowsetChange, L"Default value in data setting operations")
	TEST_VARIATION(1, 		L"Set default values on all default columns")
	TEST_VARIATION(2, 		L"Set default value on a default column; set many columns, one is asked default")
	TEST_VARIATION(3, 		L"Set default value on a not nullable default column")
	TEST_VARIATION(4, 		L"Set default value on a nullable default column (def == NULL)")
	TEST_VARIATION(5, 		L"Set default value on a nullable default column (def != NULL)")
	TEST_VARIATION(6, 		L"Set default value on a not default column")
	TEST_VARIATION(7, 		L"Set default value on a unique default value")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(SetIgnore)
//*-----------------------------------------------------------------------
// @class Ignore data in SetData
//
class SetIgnore : public SetDefault { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	// @cmember SetData, check it, get data, check it 
	BOOL	SetAndCheckDefault(
		BYTE	*pData,				// [in] buffer for IRowsetChange::SetData
		HRESULT	hrSetDataExpected,	// [in] expected hr for IRowsetChange::SetData
		BOOL	fValidate	= TRUE,	// [in] validation flag
		HRESULT	*hrSetData	= NULL,	// [out] actual result of IRowsetChange::SetData
		BOOL	fCheck		= TRUE	// [in] whether to do a check a GetData
	);

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetIgnore,SetDefault);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Set ignore values on all updateable columns
	int Variation_1();
	// @cmember Set ignore value on a column; set many columns, one is asked ignored
	int Variation_2();
	// @cmember Set ignore value on a not nullable default column
	int Variation_3();
	// @cmember Set ignore value on a nullable default column (def == NULL)
	int Variation_4();
	// @cmember Set ignore value on a nullable default column (def != NULL)
	int Variation_5();
	// @cmember Set ignore value on a not default column
	int Variation_6();
	// @cmember Bind a single column and set status to DBSTATUS_S_IGNORE
	int Variation_7();
	// @cmember Bind all updateable cols, set status to DBSTATUS_S_IGNORE for half of them
	int Variation_8();
	// @cmember Bind a single read only column and set status to DBSTATUS_S_IGNORE
	int Variation_9();
	// @cmember SetData(IGNORE)/SetData()
	int Variation_10();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(SetIgnore)
#define THE_CLASS SetIgnore
BEG_TEST_CASE(SetIgnore, SetDefault, L"Ignore data in SetData")
	TEST_VARIATION(1, 		L"Set ignore values on all updateable columns")
	TEST_VARIATION(2, 		L"Set ignore value on a column; set many columns, one is asked ignored")
	TEST_VARIATION(3, 		L"Set ignore value on a not nullable default column")
	TEST_VARIATION(4, 		L"Set ignore value on a nullable default column (def == NULL)")
	TEST_VARIATION(5, 		L"Set ignore value on a nullable default column (def != NULL)")
	TEST_VARIATION(6, 		L"Set ignore value on a not default column")
	TEST_VARIATION(7, 		L"Bind a single column and set status to DBSTATUS_S_IGNORE")
	TEST_VARIATION(8, 		L"Bind all updateable cols, set status to DBSTATUS_S_IGNORE for half of them")
	TEST_VARIATION(9, 		L"Bind a single read only column and set status to DBSTATUS_S_IGNORE")
	TEST_VARIATION(10, 		L"SetData(IGNORE)/SetData()/GPF")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Invalid_Keyset_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Invalid_Keyset_Cursor_Buffered
//
class DeleteRows : public TCIRowsetChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows,TCIRowsetChange);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember delete array of handles that point to one row
	int Variation_1();
	// @cmember DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_S_MULTIPLECHANGES
	int Variation_2();
	// @cmember DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_INVALID
	int Variation_3();
	// @cmember  use rows with immediate deletes in other methods
	int Variation_4();
	// @cmember DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED
	int Variation_5();
	// @cmember DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_NEWLYINSERTED
	int Variation_6();
	// @cmember DeleteRows from a zombie
	int Variation_7();
	// @cmember DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_PENDINGINSERT
	int Variation_8();
	// @cmember use pending deletes by using them in other methods (RESYNC)
	int Variation_9();
	// @cmember use pending deletes by using them in other methods 
	int Variation_10();
	// @cmember delete pending insert, Change
	int Variation_11();
	// @cmember delete pending insert, Update
	int Variation_12();
	// @cmember delete pending insert, Resync
	int Variation_13();
	// @cmember check for pending delete, LITERALIDENTITY - FALSE
	int Variation_14();
	// @cmember check for pending delete, LITERALIDENTITY - TRUE
	int Variation_15();
	// @cmember Deleted handle, no status
	int Variation_16();
	// @cmember Invalid handle, no status
	int Variation_17();
	// @cmember DB_S_OK when deleting an inserted/trasmitted row from a pending rowset
	int Variation_18();
	// @cmember delete/update/reposition cursor
	int Variation_19();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(DeleteRows)
#define THE_CLASS DeleteRows
BEG_TEST_CASE(DeleteRows, TCIRowsetChange, L"DeleteRows")
	TEST_VARIATION(1, 		L"delete array of handles that point to one row")
	TEST_VARIATION(2, 		L"DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_S_MULTIPLECHANGES")
	TEST_VARIATION(3, 		L"DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_INVALID")
	TEST_VARIATION(4, 		L"use rows with immediate deletes in other methods")
	TEST_VARIATION(5, 		L"DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED")
	TEST_VARIATION(6, 		L"DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_NEWLYINSERTED")
	TEST_VARIATION(7, 		L"DeleteRows from a zombie.")
	TEST_VARIATION(8, 		L"DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_PENDINGINSERT")
	TEST_VARIATION(9, 		L"use pending deletes by using them in other methods (RESYNC)")
	TEST_VARIATION(10, 		L"use pending deletes by using them in other methods.")
	TEST_VARIATION(11, 		L"delete pending insert, Change.")
	TEST_VARIATION(12, 		L"delete pending insert, Update")
	TEST_VARIATION(13, 		L"delete pending insert, Resync.")
	TEST_VARIATION(14, 		L"check for pending delete, LITERALIDENTITY - FALSE")
	TEST_VARIATION(15, 		L"check for pending delete, LITERALIDENTITY - TRUE")
	TEST_VARIATION(16, 		L"Deleted handle, no status")
	TEST_VARIATION(17, 		L"Invalid handle, no status")
	TEST_VARIATION(18, 		L"DB_S_OK when deleting an inserted/trasmitted row from a pending rowset")
	TEST_VARIATION(19, 		L"delete/update/reposition cursor")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(36, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Boundary)
	TEST_CASE(2, Rowsets)
	TEST_CASE(3, MayWriteColumn)
	TEST_CASE(4, MaxPendingChangeRows)
	TEST_CASE(5, CacheDeferred)
	TEST_CASE(6, NoColumn_Row_Restrict)
	TEST_CASE(7, Computed_Columns)
	TEST_CASE(8, Forward_Query)
	TEST_CASE(9, Forward_Cursor)
	TEST_CASE(10, QueryBaseUpdates)
	TEST_CASE(11, Static_Cursor_Immediate)
	TEST_CASE(12, Static_Cursor_Buffered)
	TEST_CASE(13, Keyset_Cursor_Buffered)
	TEST_CASE(14, Keyset_Remove_Cursor_Immediate)
	TEST_CASE(15, Keyset_Remove_Buffered)
	TEST_CASE(16, Dynamic_Cursor_Buffered)
	TEST_CASE(17, Visible_Forward_Cursor)
	TEST_CASE(18, Visible_Static_Cursor)
	TEST_CASE(19, Visible_Keyset_Command_Cursor)
	TEST_CASE(20, Visible_Dynamic_Command_Cursor)
	TEST_CASE(21, Bookmark_Forward)
	TEST_CASE(22, Bookmark_Static)
	TEST_CASE(23, Bookmark_Keyset)
	TEST_CASE(24, Bookmark_Dynamic)
	TEST_CASE(25, OrderedBookmark_Keyset)
	TEST_CASE(26, Invalid_Keyset_Cursor_Immediate)
	TEST_CASE(27, Invalid_Keyset_Cursor_Buffered)
	TEST_CASE(28, Valid_Keyset_Cursor_Immediate)
	TEST_CASE(29, Related_Delete_NewRow)
	TEST_CASE(30, Sequence)
	TEST_CASE(31, Transaction)
	TEST_CASE(32, ExtendedErrors)
	TEST_CASE(33, ProviderOwnedMem)
	TEST_CASE(34, SetDefault)
	TEST_CASE(35, SetIgnore)
	TEST_CASE(36, DeleteRows)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary - Testing boundary conditions
//|	Created:			04/05/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Init()
{	
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_1()
{
	HROW		hRow[1];
	HROW		*pHRow	  = hRow;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	BOOL		fPass	  = TEST_SKIPPED;

	// Initialize
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE));
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//get data for the first row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained,&pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	TESTC_(GetData(hRow[0],m_hAccessor,m_pData),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
	
	//use NULL hRow
	TESTC_(SetData(DB_NULL_HROW,hAccessor,m_pData),DB_E_BADROWHANDLE);
	TESTC_(SetData(NULL,hAccessor,m_pData),DB_E_BADROWHANDLE);
		 
	//use NULL hAccessor
	TESTC_(SetData(hRow[0],DB_NULL_HACCESSOR,m_pData),DB_E_BADACCESSORHANDLE);
	TESTC_(SetData(hRow[0],NULL,m_pData),DB_E_BADACCESSORHANDLE);

	//use valid hAccessor
	TESTC_(SetData(hRow[0],hAccessor,m_pData),S_OK);
	
	//make sure the data is not changed
	TESTC(CompareData(m_cRowsetCols,m_rgTableColOrds,1,m_pData,m_cBinding,m_rgBinding,m_pTable,NULL));

	fPass = TRUE;

CLEANUP:
	
	//release objects
	if( m_pIRowset ) 
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( m_pIAccessor && hAccessor ) 
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Null Accessor, NULL pData with DBPROPVAL_UP_CHANGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_2()
{
	HROW		hRow[1];
	HROW		*pHRow	  = hRow;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	BOOL		fPass	  = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR));
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	//use NULL hRow
	TESTC_(SetData(DB_NULL_HROW,hAccessor,m_pData),DB_E_BADROWHANDLE);
	TESTC_(SetData(NULL,hAccessor,m_pData),DB_E_BADROWHANDLE);
 
	//use NULL hAccessor
	TESTC_(SetData(hRow[0],DB_NULL_HACCESSOR,m_pData),DB_E_BADACCESSORHANDLE);
	TESTC_(SetData(hRow[0],NULL,m_pData),DB_E_BADACCESSORHANDLE);

	//use valid hAccessor
	TESTC_(SetData(hRow[0],hAccessor,NULL),S_OK);
		
	fPass=TRUE;	

CLEANUP:

	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( m_pIAccessor && hAccessor )
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  NULL pData.  E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_3()
{
	HROW		hRow[1];
	HROW		*pHRow = hRow;
	BOOL		fPass  = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,m_rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	//use NULL hRow
	TESTC_(SetData(DB_NULL_HROW,m_hAccessor,m_pData),DB_E_BADROWHANDLE);
	TESTC_(SetData(hRow[0],DB_NULL_HACCESSOR,m_pData),DB_E_BADACCESSORHANDLE);
	TESTC_(SetData(hRow[0],m_hAccessor,NULL),E_INVALIDARG);
		
	fPass=TRUE;	

CLEANUP:
	
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_4()
{
	HROW		hRow[1];
	HROW		*pHRow	  = hRow;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	BOOL		fPass	  = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE));
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	TESTC_(SetData(hRow[0],hAccessor,NULL),S_OK);
		
	fPass=TRUE;	

CLEANUP:

	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( m_pIAccessor && hAccessor ) 
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Null Accessor, valid pData with DBPROPVAL_UP_CHANGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_5()
{
	HACCESSOR	hAccessor  = DB_NULL_HACCESSOR;
	BOOL		fPass	   = TEST_SKIPPED;
	ULONG_PTR	ulUpdValue = 0;

	// Initialize
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE));
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//Get the value for DBPROP_UPDATABILITY
	GetProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,m_pIRowsetChange,&ulUpdValue);

	//create an NULL accessor
	if( ulUpdValue & DBPROPVAL_UP_INSERT )
		CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0, 
													&hAccessor,NULL),S_OK);
	else
		CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,
							&hAccessor,NULL),DB_E_NULLACCESSORNOTSUPPORTED);

	fPass = TRUE;
	
CLEANUP:
	
	// Release the Accessor
	if( m_pIAccessor && hAccessor ) 
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc buffered update mode.   NULL accessor and NULL pData with DBPROPVAL_UP_CHANGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_6()
{
	HACCESSOR	hAccessor  = DB_NULL_HACCESSOR;
	BOOL		fPass	   = TEST_SKIPPED;
	ULONG_PTR	ulUpdValue = 0;

	// Initialize
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE));		
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//Get the value for DBPROP_UPDATABILITY
	GetProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,m_pIRowsetChange,&ulUpdValue);

	//create an NULL accessor
	if( ulUpdValue & DBPROPVAL_UP_INSERT )
		CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0, 
													&hAccessor,NULL),S_OK);
	else
		CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,
							&hAccessor,NULL),DB_E_NULLACCESSORNOTSUPPORTED);

	fPass = TRUE;
	
CLEANUP:

	// Release the Accessor
	if( m_pIAccessor && hAccessor )
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Valid accessor with bad ordinals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_7()
{
	HROW		hRow[1];
	HROW		*pHRow	    = hRow;
	IAccessor	*pIAccessor = NULL;
	HACCESSOR	hAccessor1  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor2  = DB_NULL_HACCESSOR;
	HRESULT		hr			= E_FAIL;
	BOOL		fPass	    = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	//create an accessor with invalid ordinals
	TESTC(VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

    // Verify we've got at least one binding 
    QTESTC(m_cBinding != 0);

	//bind ordinals out of bounds
	m_rgBinding[0].iOrdinal	= 0;
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK);
	m_rgBinding[0].iOrdinal	= m_rgBinding[m_cBinding-1].iOrdinal;
	m_rgBinding[0].iOrdinal++;
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor2,NULL),S_OK);

	//Release the first Rowset and Command
	ReleaseRowsetAndAccessor(0,1);
	TESTC(VerifyInterface(pIAccessor,IID_ICommand,COMMAND_INTERFACE,(IUnknown**)&m_pICommand));
	
	// Fixup the Updatablity flag agian
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	if(!GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,&hr))
	{
		TESTC_(hr, DB_E_BADORDINAL);
		fPass=TRUE;
		goto CLEANUP;
	}

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	TESTC_(GetData(hRow[0],m_hAccessor,m_pData),S_OK);

	//use valid hRow with bad ordinals
	TEST2C_(SetData(hRow[0],hAccessor1,m_pData),DB_E_BADORDINAL,DB_E_ERRORSOCCURRED);
	TEST2C_(SetData(hRow[0],hAccessor2,m_pData),DB_E_BADORDINAL,DB_E_ERRORSOCCURRED);
		
	fPass=TRUE;	
CLEANUP:
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	if( pIAccessor && hAccessor2 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor2,NULL),S_OK);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  OPTIMIZED accessor after a Fetch (DB_E_BADACCESSORFLAGS)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_8()
{
	HROW		*pHRow	   = NULL;
	HACCESSOR	hAccessor1 = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor2 = DB_NULL_HACCESSOR;
	BOOL		fPass	   = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE));
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;

	//create an Optimized accessor before the GetNextRows
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_OPTIMIZED,
								m_cBinding,m_rgBinding,0,&hAccessor1,NULL), S_OK);
	
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	//release the row just fetched and the first accessor
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if( m_pIAccessor && hAccessor1 )
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
		hAccessor1 = DB_NULL_HACCESSOR;
	}
	//create an Optimized accessor after the GetNextRows
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_OPTIMIZED,
				m_cBinding,m_rgBinding,0,&hAccessor2,NULL),DB_E_BADACCESSORFLAGS);
	
	//should return a NULL handle
	TESTC(hAccessor2 == NULL);

	fPass=TRUE;	

CLEANUP:
	//release objects 
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	if( m_pIAccessor && hAccessor1 )
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
		hAccessor1 = DB_NULL_HACCESSOR;
	}

	if( m_pIAccessor && hAccessor2 )
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor2,NULL),S_OK);
		hAccessor2 = DB_NULL_HACCESSOR;
	}

	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Valid accessor with different Parameter Accessor Flags
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_9()
{
	HROW		hRow[1];
	HROW		*pHRow	    = hRow;
	IAccessor	*pIAccessor = NULL;
	HACCESSOR	hAccessor1  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor2  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor3  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor4  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor5  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor6  = DB_NULL_HACCESSOR;
	HRESULT		hr			= E_FAIL;
	BOOL		fPass	    = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported

	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	//Note: we should use DBACCESSOR_ROWDATA instead of DBACCESSOR_PARAMETERDATA so that ordinals in m_rgBinding structures 
	//		match the ones used for GetData(hRow[0],m_hAccessor,m_pData) later in the variation
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT));
	
	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	//create an accessor
	TESTC(VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

	//create accessor with different Parameter accessor flags
	TEST2C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
							m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK,DB_E_BADACCESSORFLAGS);

	TEST2C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA|DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor2,NULL),S_OK,DB_E_BADACCESSORFLAGS);

	TEST2C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_PARAMETERDATA,
							m_cBinding,m_rgBinding,0,&hAccessor3,NULL),S_OK,DB_E_BADACCESSORFLAGS);

	TEST2C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA|DBACCESSOR_OPTIMIZED,
							m_cBinding,m_rgBinding,0,&hAccessor4,NULL),S_OK,DB_E_BADACCESSORFLAGS);

	TEST3C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA|DBACCESSOR_PASSBYREF,
							m_cBinding,m_rgBinding,0,&hAccessor5,NULL),
										S_OK,DB_E_BYREFACCESSORNOTSUPPORTED,DB_E_BADACCESSORFLAGS);

	TEST3C_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA|DBACCESSOR_PASSBYREF|DBACCESSOR_OPTIMIZED,
							m_cBinding,m_rgBinding,0,&hAccessor6,NULL),
										S_OK,DB_E_BYREFACCESSORNOTSUPPORTED,DB_E_BADACCESSORFLAGS);

	//Release the first Rowset and Command
	ReleaseRowsetAndAccessor(0,1);
	TESTC(VerifyInterface(pIAccessor,IID_ICommand,COMMAND_INTERFACE,(IUnknown**)&m_pICommand));
	
	// Fixup the Updatablity flag again
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	if(!GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT,NULL,DB_E_BYREFACCESSORNOTSUPPORTED))
	{
		fPass=TRUE;
		goto CLEANUP;
	}

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	TESTC_(GetData(hRow[0],m_hAccessor,m_pData),S_OK);

	fnIgnoreAutoInc(&m_pData);

	//use valid hRow
	if( hAccessor1 )
		TEST2C_(SetData(hRow[0],hAccessor1,m_pData),DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);
	if( hAccessor2 )
		TEST3C_(SetData(hRow[0],hAccessor2,m_pData),S_OK,DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);
	if( hAccessor3 )
		TEST3C_(SetData(hRow[0],hAccessor3,m_pData),DB_E_BADACCESSORTYPE,DB_E_ERRORSOCCURRED,DB_E_BADACCESSORHANDLE);
	if( hAccessor4 )
		TEST2C_(SetData(hRow[0],hAccessor4,m_pData),DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);
	if( hAccessor5 )
		TEST2C_(SetData(hRow[0],hAccessor5,m_pData),DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);
	if( hAccessor6 )
		TEST2C_(SetData(hRow[0],hAccessor6,m_pData),DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);
		
	fPass=TRUE;	
CLEANUP:
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	if( pIAccessor && hAccessor2 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor2,NULL),S_OK);
	if( pIAccessor && hAccessor3 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor3,NULL),S_OK);
	if( pIAccessor && hAccessor4 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor4,NULL),S_OK);
	if( pIAccessor && hAccessor5 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor5,NULL),S_OK);
	if( pIAccessor && hAccessor6 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor6,NULL),S_OK);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Valid accessor with different Row Accessor Flags
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_10()
{
	HROW		hRow[1];
	HROW		*pHRow	    = hRow;
	IAccessor	*pIAccessor = NULL;
	HACCESSOR	hAccessor1  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor2  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor3  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor4  = DB_NULL_HACCESSOR;
	HRESULT		hr			= E_FAIL;
	BOOL		fPass	    = TEST_SKIPPED;
		
	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;;//DBPROPVAL_UP_CHANGE;
		
	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,TRUE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
										UPDATEABLE_COLS_BOUND));

	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	//create an accessor off the command
	TESTC(VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

	//create accessor with different Row accessor flags
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK);

	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_OPTIMIZED,
							m_cBinding,m_rgBinding,0,&hAccessor2,NULL),S_OK);

	TEST2C_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_PASSBYREF,
						m_cBinding,m_rgBinding,0,&hAccessor3,NULL),S_OK,DB_E_BYREFACCESSORNOTSUPPORTED);

	TEST2C_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA|DBACCESSOR_PASSBYREF|DBACCESSOR_OPTIMIZED,
						m_cBinding,m_rgBinding,0,&hAccessor4,NULL),S_OK,DB_E_BYREFACCESSORNOTSUPPORTED);

	//Release the first Rowset and Command
	ReleaseRowsetAndAccessor(0,1);
	TESTC(VerifyInterface(pIAccessor,IID_ICommand,COMMAND_INTERFACE,(IUnknown**)&m_pICommand));
	
	// Fixup the Updatablity flag again
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	//DB_E_BYREFACCESSORNOTSUPPORTED could be returned here becasue m_pICommand is inheritted from the
	//pIAccessor which could have been created with DBACCESSOR_PASSBYREF
	if(!GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,NULL,DB_E_BYREFACCESSORNOTSUPPORTED))
	{
		fPass=TRUE;
		goto CLEANUP;
	}

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	TESTC_(GetData(hRow[0],hAccessor1,m_pData),S_OK);
	
	fnIgnoreAutoInc(&m_pData);

	//use valid hRow
	if( hAccessor1 )
		TESTC_(SetData(hRow[0],hAccessor1,m_pData),S_OK);
	if( hAccessor2 )
		TESTC_(SetData(hRow[0],hAccessor2,m_pData),S_OK);
	if( hAccessor3 )
		TEST2C_(SetData(hRow[0],hAccessor3,m_pData),S_OK,DB_E_BADACCESSORTYPE);
	if( hAccessor4 )
		TEST2C_(SetData(hRow[0],hAccessor4,m_pData),S_OK,DB_E_BADACCESSORTYPE);
		
	fPass=TRUE;	
CLEANUP:
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	if( pIAccessor && hAccessor2 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor2,NULL),S_OK);
	if( pIAccessor && hAccessor3 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor3,NULL),S_OK);
	if( pIAccessor && hAccessor4 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor4,NULL),S_OK);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc inherited parameter accessors with SetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_11()
{
	HROW			hRow[1];
	HROW			*pHRow				= hRow;
	IAccessor		*pIAccessor			= NULL;
	HACCESSOR		hAccessor1			= DB_NULL_HACCESSOR;
	void			*pData1				= NULL;
	void			*pData2				= NULL;
	HRESULT			hr					= E_FAIL;
	BOOL			fPass				= TEST_SKIPPED;
	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM		cBindings			= 0;
	DBBINDING		*rgBindings			= NULL;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT));
	
	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData1,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData2,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	fnIgnoreAutoInc(&pData1);
	fnIgnoreAutoInc(&pData2);

	fPass = TEST_FAIL;
	
	//create an accessor object
	TESTC(VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

	//create accessor with different Parameter AND Rowset accessor flags
//	TESTC_(hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA,
//							m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK);//,DB_E_BADACCESSORFLAGS);
	hr=pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor1,NULL);

	if (hr == DB_E_BADACCESSORFLAGS)
	{
		// Provider may not support PARAMETER accessors - verify it
		hr = pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBinding, m_rgBinding, 0, &hAccessor1, NULL);
		TESTC_(hr, DB_E_BADACCESSORFLAGS);

		// Now create only Rowset accessor
		hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBinding, m_rgBinding, 0, &hAccessor1, NULL);
		TESTC_(hr, S_OK);
	}
	else
		TESTC_(hr, S_OK);


	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	//set data with row/param accessor
	if( hAccessor1 )
		TEST2C_(SetData(hRow[0],hAccessor1,pData1),DB_E_BADACCESSORTYPE,DB_E_BADACCESSORHANDLE);

	//Release the first Rowset and Command
	ReleaseRowsetAndAccessor(0,1);

	//get a new command object of the accessor
	TESTC(VerifyInterface(pIAccessor,IID_ICommand,COMMAND_INTERFACE,(IUnknown**)&m_pICommand));
	
	// Fixup the Updatablity flag again
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset without an accessor on the rowset,(use inherited accessor)
	if(!GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,m_rgPropertyIDs,0,NULL,NO_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,0,NULL,DB_E_BYREFACCESSORNOTSUPPORTED))
	{
		fPass=TEST_PASS;
		goto CLEANUP;
	}

	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);

	//set data with inherited row/param accessor from command
	if( hAccessor1 )
		TESTC_(SetData(hRow[0],hAccessor1,pData2),S_OK);
		
	QTESTC_(hr = pIAccessor->GetBindings(hAccessor1, &dwAccessorFlags, &cBindings, &rgBindings),S_OK);
	COMPARE(dwAccessorFlags, (DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA));

	//create an accessor object from the rowset object (should be the accessor that the rowset inherited)
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	SAFE_RELEASE(pIAccessor);
	FreeAccessorBindings(cBindings, rgBindings);
	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));
	
	QTESTC_(hr = pIAccessor->GetBindings(hAccessor1, &dwAccessorFlags, &cBindings, &rgBindings),S_OK);
	COMPARE(dwAccessorFlags, (DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA));
	
	fPass=TEST_PASS;	
CLEANUP:
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	FreeAccessorBindings(cBindings, rgBindings);

	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  more Valid accessor with bad ordinals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_12()
{
	HROW		hRow[1];
	HROW		*pHRow	    = hRow;
	IAccessor	*pIAccessor = NULL;
	HACCESSOR	hAccessor1  = DB_NULL_HACCESSOR;
	HACCESSOR	hAccessor2  = DB_NULL_HACCESSOR;
	HRESULT		hr			= E_FAIL;
	BOOL		fPass	    = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_rgPropertyIDs[1]	= DBPROP_BOOKMARKS;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,m_rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	//create an accessor with invalid ordinals
	TESTC(VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

	m_rgBinding[0].iOrdinal	= 9999;
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK);

	m_rgBinding[0].iOrdinal	= m_rgBinding[m_cBinding-1].iOrdinal++;
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor2,NULL),S_OK);

	//Release the first Rowset and Command
	ReleaseRowsetAndAccessor(0,1);
	TESTC(VerifyInterface(pIAccessor,IID_ICommand,COMMAND_INTERFACE,(IUnknown**)&m_pICommand));
	
	// Fixup the Updatablity flag agian
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	if(!GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
		FORWARD,NO_COLS_BY_REF,0,0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,&hr))
	{
		TEST2C_(hr, DB_E_BADORDINAL,DB_E_ERRORSOCCURRED);
		fPass=TRUE;
		goto CLEANUP;
	}

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	TESTC_(GetData(hRow[0],m_hAccessor,m_pData),S_OK);

	//use valid hRow with bad ordinals
	TEST2C_(SetData(hRow[0],hAccessor1,m_pData),DB_E_BADORDINAL,DB_E_ERRORSOCCURRED);
	TEST2C_(SetData(hRow[0],hAccessor2,m_pData),DB_E_BADORDINAL,DB_E_ERRORSOCCURRED);
		
	fPass=TRUE;	

CLEANUP:
	
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);
	if( pIAccessor && hAccessor2 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor2,NULL),S_OK);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  get bookmarks on inheritied accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_13()
{
	HROW		hRow[1];
	HROW		*pHRow	    = hRow;
	IAccessor	*pIAccessor = NULL;
	HACCESSOR	hAccessor1  = DB_NULL_HACCESSOR;
	HRESULT		hr			= E_FAIL;
	BOOL		fPass	    = TEST_SKIPPED;

	// Initialize 
	m_cRowsObtained		= 0;
	m_rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	m_rgPropertyIDs[1]	= DBPROP_BOOKMARKS;
	m_ulpUpdFlags		= DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,m_rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	QTESTC(m_pICommand != NULL);
	QTESTC(m_pIRowsetChange != NULL);
	fPass = FALSE;
	
	//create an accessor with invalid ordinals
	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,COMMAND_INTERFACE,(IUnknown**)&pIAccessor));

	// Verify we've got at least one binding 
    QTESTC(m_cBinding != 0);

	m_rgBinding[0].iOrdinal	= 0;
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
							m_cBinding,m_rgBinding,0,&hAccessor1,NULL),S_OK);

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&m_cRowsObtained, &pHRow),S_OK);
	TESTC(m_cRowsObtained == 1);
	//use valid hRow with bad ordinals
	TESTC_(GetData(hRow[0],hAccessor1,m_pData),S_OK);
		
	fPass = TEST_PASS;
CLEANUP:	
	//release objects 
	if( m_pIRowset )
		CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,hRow,NULL,NULL,NULL),S_OK);
	
	if( pIAccessor && hAccessor1 )
		CHECK(pIAccessor->ReleaseAccessor(hAccessor1,NULL),S_OK);

	SAFE_RELEASE(pIAccessor);
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Rowsets)
//*-----------------------------------------------------------------------
//| Test Case:		Rowsets - Testing read-only rowset and empty
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowsets::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Read-only rowset with
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowsets::Variation_1()
{

	IUnknown *pIRowset	= NULL;
	BOOL	 fTestPass	= TEST_SKIPPED;

	
	if (!m_pIDBCreateCommand || 
		!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
		goto CLEANUP;
		
	fTestPass=FALSE;

	//open an empty rowset, asking for IRowsetChange interface pointer
	m_hr = g_pTable->ExecuteCommand(SELECT_COUNT, IID_IRowsetChange,NULL,NULL,NULL,NULL,
										EXECUTE_IFNOERROR,0,NULL,NULL,&pIRowset,NULL);
	TESTC(m_hr == S_OK ||m_hr ==DB_E_ERRORSOCCURRED);
	
	fTestPass=TRUE;
CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Read only rowset with left outer join.DB_E_PROPERTIESNOTAVAILABLE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowsets::Variation_2()
{
	IUnknown	*pIRowset			= NULL;
	BOOL		fTestPass			= TEST_SKIPPED;
	size_t		fEscapeClauses		= FALSE;
	size_t		fODBCExt			= FALSE;
	size_t		fANSI				= FALSE;
	ULONG_PTR	ulSQLSupportMask	= 0;
	DBPROPSET	rgDBPropSet[1];
	LPWSTR		pwsz2ndTName		= NULL;


	rgDBPropSet[0].cProperties    = 1;
	rgDBPropSet[0].guidPropertySet= DBPROPSET_ROWSET;	

	rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * (1));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(1));
	if(!rgDBPropSet[0].rgProperties)
		goto CLEANUP;
	
	rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_TRUE;

	if (!m_pIDBCreateCommand || 
		!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET, g_pIDBCreateSession))
		goto CLEANUP;

	// skip variation if there is no SQL Support
	QTESTC(GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulSQLSupportMask));

	fTestPass=FALSE;

	fEscapeClauses	= DBPROPVAL_SQL_ESCAPECLAUSES & ulSQLSupportMask;
	fODBCExt		= DBPROPVAL_SQL_ODBC_EXTENDED & ulSQLSupportMask;
	fANSI			= DBPROPVAL_SQL_ANSI92_INTERMEDIATE & ulSQLSupportMask;


	pwsz2ndTName	= g_p1RowTable->GetTableName();

	if (fEscapeClauses)
	{
		//open a left outer join, asking for IRowsetChange interface pointer
		m_hr=g_pTable->ExecuteCommand(SELECT_LEFTOUTERJOIN_ESC, IID_IRowsetChange,pwsz2ndTName,
										NULL,NULL,NULL,EXECUTE_IFNOERROR,1,rgDBPropSet,NULL,&pIRowset,NULL);
	}
	else
	{
		//open a left outer join, asking for IRowsetChange interface pointer
		m_hr=g_pTable->ExecuteCommand(SELECT_LEFTOUTERJOIN, IID_IRowsetChange,pwsz2ndTName,
										NULL,NULL,NULL,EXECUTE_IFNOERROR,1,rgDBPropSet,NULL,&pIRowset,NULL);
	}
 
	m_pIRowset=(IRowset*)pIRowset;

	//S_OK is ok here because a provider may not support the whole extended level 
	//	but might suport some SQL syntax that is part of the extended level
	//DB_E_ERRORSOCCURRED because change to a temp table might not be possilbe
	//DB_E_ERRORSINCOMMAND because join might not be supported
	if ( fODBCExt || fANSI)
	{
		TESTC( m_hr == S_OK || m_hr == DB_E_ERRORSOCCURRED )
	}
	else
	{
		TESTC( m_hr == S_OK || m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_E_ERRORSINCOMMAND)
	}

	fTestPass = TRUE;
CLEANUP:
	if(rgDBPropSet[0].rgProperties)			   
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);

	SAFE_RELEASE(pIRowset);
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Read only rowset with right outer join.DB_E_PROPERTIESNOTAVAILABLE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowsets::Variation_3()
{
	IUnknown	*pIRowset			= NULL;
	BOOL		fTestPass			= TEST_SKIPPED;
	size_t		fEscapeClauses		= FALSE;
	ULONG_PTR	ulSQLSupportMask	= 0;
	size_t		fODBCExt			= FALSE;
	size_t		fANSI				= FALSE;

	DBPROPSET			rgDBPropSet[1];

	rgDBPropSet[0].cProperties    = 1;
	rgDBPropSet[0].guidPropertySet= DBPROPSET_ROWSET;	

	rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * (1));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(1));
	if(!rgDBPropSet[0].rgProperties)
		goto CLEANUP;
	
	rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_TRUE;

	if (!m_pIDBCreateCommand || 
		!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
		goto CLEANUP;

	// skip variation if there is no SQL Support
	QTESTC(GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulSQLSupportMask));

	fTestPass=FALSE;

	fEscapeClauses	= DBPROPVAL_SQL_ESCAPECLAUSES & ulSQLSupportMask;
	fODBCExt		= DBPROPVAL_SQL_ODBC_EXTENDED & ulSQLSupportMask;
	fANSI			= DBPROPVAL_SQL_ANSI92_INTERMEDIATE & ulSQLSupportMask;

	if (fEscapeClauses)
	{
		//open a rightouter join, asking for IRowsetChange interface pointer
		m_hr=g_pTable->ExecuteCommand(SELECT_RIGHTOUTERJOIN_ESC, IID_IRowsetChange,g_p1RowTable->GetTableName(),
										NULL,NULL,NULL,EXECUTE_IFNOERROR,1,rgDBPropSet,NULL,&pIRowset,NULL);
	}
	else
	{
		//open a rightouter, asking for IRowsetChange interface pointer
		m_hr=g_pTable->ExecuteCommand(SELECT_RIGHTOUTERJOIN, IID_IRowsetChange,g_p1RowTable->GetTableName(),
										NULL,NULL,NULL,EXECUTE_IFNOERROR,1,rgDBPropSet,NULL,&pIRowset,NULL);
	}

	//S_OK is ok here because a provider may not support the whole extended level 
	//	but might suport some SQL syntax that is part of the extended level
	//DB_E_ERRORSOCCURRED because change to a temp table might not be possilbe
	//DB_E_ERRORSINCOMMAND because join might not be supported
	if ( fODBCExt || fANSI)
	{
		TESTC( m_hr == S_OK || m_hr == DB_E_ERRORSOCCURRED )
	}
	else
	{
		TESTC( m_hr == S_OK || m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_E_ERRORSINCOMMAND)
	}
	fTestPass = TRUE;
CLEANUP:
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty rowset.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowsets::Variation_4()
{
	IUnknown		*pIRowset	= NULL;
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr = E_FAIL;	

	if (!m_pIDBCreateCommand || 
		!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//open an empty rowset, asking for IRowsetChange interface pointer
	hr = g_pTable->ExecuteCommand(SELECT_EMPTYROWSET, IID_IRowsetChange,
		NULL,NULL,NULL,NULL,EXECUTE_IFNOERROR,0,NULL,NULL,&pIRowset,NULL);

	if (FAILED(hr))
		TESTC_(hr, DB_E_NOTSUPPORTED)
	else
		TESTC_(hr, S_OK)

	fTestPass=TRUE;

CLEANUP:
	if(pIRowset)
		COMPARE(pIRowset->Release(), 0);
	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowsets::Terminate()
{
	return(TCIRowsetChange::Terminate());
}


// {{ TCW_TC_PROTOTYPE(MayWriteColumn)
//*-----------------------------------------------------------------------
//| Test Case:		MayWriteColumn - test DBPROP_MAYWRITECOLUMN
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MayWriteColumn::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Make sure MSDASQL does not support DBPROP_MAYWRITECOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MayWriteColumn::Variation_1()
{
	
	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MayWriteColumn::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MaxPendingChangeRows)
//*-----------------------------------------------------------------------
//| Test Case:		MaxPendingChangeRows - test DBPROP_MAXPENDINGCHANGEROWS
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxPendingChangeRows::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify DBPROP_MAXPENDINGROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxPendingChangeRows::Variation_1()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperties=0;
	DBPROPSET			*pDBPropSet=NULL;
	DBPROPIDSET			DBPropIDSet;
	DBPROPID			DBPropID=DBPROP_IRowsetChange;
	BOOL				fTestPass=TEST_SKIPPED;
	ULONG				ulPendingChanges=0;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,&DBPropID));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	// Set to FALSE
	fTestPass = FALSE;

	//get the IRowsetInfo pointer
	TESTC_(m_pIRowsetChange->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
		
	//ask for information on DBPROP_MAXPENDINGROWS
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//ask for information on DBPROP_MAXPENDINGROWS
	DBPropID=DBPROP_MAXPENDINGROWS;

	m_hr=pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet);
	if(m_hr == S_OK && pDBPropSet->rgProperties->dwStatus == DBPROPSTATUS_OK)
	{
		odtLog <<L"DBPROP_MAXPENDINGROWS is set to " <<V_I4(&pDBPropSet->rgProperties->vValue) <<ENDL;
		fTestPass = TRUE;
	}
	else
	{
		odtLog <<L"DBPROP_MAXPENDINGROWS is not supported" <<ENDL;
		fTestPass=TEST_SKIPPED;
	}
	
CLEANUP:

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	SAFE_RELEASE(pIRowsetInfo);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Verify DBPROP_MAXOPENROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxPendingChangeRows::Variation_2()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperties=0;
	DBPROPSET			*pDBPropSet=NULL;
	DBPROPIDSET			DBPropIDSet;
	DBPROPID			DBPropID=DBPROP_IRowsetChange;
	BOOL				fTestPass=TEST_SKIPPED;
	ULONG				ulMaxOpenRows=0;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,&DBPropID));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	// Set to FALSE
	fTestPass = FALSE;

	//get the IRowsetInfo pointer
	TESTC_(m_pIRowsetChange->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
		
	//ask for information on DBPROP_MAXPENDINGCHNAGEROWS
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//ask for information on DBPROP_MAXOPENROWS
	DBPropID=DBPROP_MAXOPENROWS;

	m_hr=pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet);
	if(m_hr == S_OK && pDBPropSet->rgProperties->dwStatus == DBPROPSTATUS_OK)
	{
		odtLog <<L"DBPROP_MAXOPENROWS is set to " <<V_I4(&pDBPropSet->rgProperties->vValue) <<ENDL;
		fTestPass = TRUE;
	}
	else
	{
		odtLog <<L"DBPROP_MAXOPENROWS is not supported" <<ENDL;
		fTestPass=TEST_SKIPPED;
	}

CLEANUP:

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	SAFE_RELEASE(pIRowsetInfo);
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
BOOL MaxPendingChangeRows::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CacheDeferred)
//*-----------------------------------------------------------------------
//| Test Case:		CacheDeferred - test DBPROP_CACHEDEFERRED
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CacheDeferred::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc to be coded DBPROP_CACHDEFERRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CacheDeferred::Variation_1()
{
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CacheDeferred::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(NoColumn_Row_Restrict)
//*-----------------------------------------------------------------------
//| Test Case:		NoColumn_Row_Restrict - test DBPROP_NOCOLUMNRESTRICT and DBPROP_NOROWRESTRICT
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL NoColumn_Row_Restrict::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Select Count(*
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NoColumn_Row_Restrict::Variation_1()
{
	BOOL				fTestPass=TEST_SKIPPED;
	ULONG				cProperties=0;
	IRowsetInfo			*pIRowsetInfo=NULL;
	DBPROPID			DBPropIDColumn=DBPROP_COLUMNRESTRICT;		
	DBPROPID			DBPropIDRow=DBPROP_ROWRESTRICT;		
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;

	//open the rowset
	TESTC_PROVIDER(CreateRowsetObject(SELECT_EMPTYROWSET,IID_IRowset,EXECUTE_IFNOERROR)==S_OK);
	
	if(!SupportedProperty(DBPROP_COLUMNRESTRICT,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
		odtLog<<L"DBPROP_COLUMNRESTRICT not supported, test Skipped."<<ENDL;
		goto CLEANUP;
	}
	if(!SupportedProperty(DBPROP_ROWRESTRICT,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
		odtLog<<L"DBPROP_ROWRESTRICT not supported, test Skipped."<<ENDL;
		goto CLEANUP;
	}

	fTestPass = FALSE;
	//QI for IRowsetInfo
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
		
	//init
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=NULL;

	DBPropIDSet.rgPropertyIDs=&DBPropIDColumn;

	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet),S_OK);

	//no column restrict on this rowset
	if( (V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_TRUE) &&
		(V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_FALSE) )
		goto CLEANUP;

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	DBPropIDSet.rgPropertyIDs=&DBPropIDRow;

	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet),S_OK);
			
	//no row restrict on this rowset
	if( (V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_TRUE) &&
		(V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_FALSE) )
		goto CLEANUP;

	fTestPass=TRUE;
CLEANUP:

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	SAFE_RELEASE(pIRowsetInfo);
	ReleaseRowsetObject();
	ReleaseCommandObject();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Select * from Table.  Restrict on table.  No restrict on column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NoColumn_Row_Restrict::Variation_2()
{
	BOOL				fTestPass=TEST_SKIPPED;
	ULONG				cProperties=0;
	IRowsetInfo			*pIRowsetInfo=NULL;
	DBPROPID			DBPropIDColumn=DBPROP_COLUMNRESTRICT;		
	DBPROPID			DBPropIDRow=DBPROP_ROWRESTRICT;		
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	
	//open the rowset with IOpenRowset
	TESTC_PROVIDER(CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR)==S_OK);

	if(!SupportedProperty(DBPROP_COLUMNRESTRICT,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
		odtLog<<L"DBPROP_COLUMNRESTRICT not supported, test Skipped."<<ENDL;
		goto CLEANUP;
	}
	if(!SupportedProperty(DBPROP_ROWRESTRICT,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
		odtLog<<L"DBPROP_ROWRESTRICT not supported, test Skipped."<<ENDL;
		goto CLEANUP;
	}
	
	fTestPass = FALSE;
	//QI for IRowsetInfo
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//init
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=NULL;

	DBPropIDSet.rgPropertyIDs=&DBPropIDColumn;

	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet),S_OK);
			
	//there is column restrict on this rowset
	if(	V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_TRUE	&&
		V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_FALSE )
		goto CLEANUP;

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	DBPropIDSet.rgPropertyIDs=&DBPropIDRow;

	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperties,&pDBPropSet),S_OK);
			
	//no row restrict on this rowset
	if( V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_TRUE &&
		V_BOOL(&pDBPropSet->rgProperties->vValue)!=VARIANT_FALSE )
		goto CLEANUP;

	fTestPass=TRUE;

CLEANUP:

	//free the memory
	FreeProperties(&cProperties,&pDBPropSet);
	
	SAFE_RELEASE(pIRowsetInfo);
	ReleaseRowsetObject();
	ReleaseCommandObject();
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL NoColumn_Row_Restrict::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Computed_Columns)
//*-----------------------------------------------------------------------
//| Test Case:		Computed_Columns - test rowset with computed columns
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Computed_Columns::Init()
{
	DBPROPID	DBPropID=DBPROP_IRowsetChange;
	BOOL		fPass = TEST_SKIPPED;
	
	if(!TCIRowsetChange::Init())
		return FALSE;

	if(!m_pIDBCreateCommand)
	{
		odtLog << "Command Not Supported Skipping Variations"<<ENDL;
		return TEST_SKIPPED;
	}

	//create an accessor with IRowsetChange.  Immediately update mode.
	//create an accessor with value binding only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COMPUTEDCOLLIST,1,&DBPropID,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH));

	fPass = TRUE;

CLEANUP:
	return fPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Accessor binds the computed column.  DB_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Computed_Columns::Variation_1()
{
	return TEST_PASS;

	//TODO no way to test since different driver behave differently


}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Computed_Columns::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Forward_Query)
//*-----------------------------------------------------------------------
//| Test Case:		Forward_Query - forward only cursor.  Query based update
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Query::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query::Variation_1()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows		= 0;
	DBORDINAL	cCol		= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	ULONG		cCount		= 0;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	//ReleaseRowsetAndAccessor() so that an accessor handle can be created on the command
	ReleaseRowsetAndAccessor();

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2, g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))

	
	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//set a nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cCol-1])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}

		}
	}

	//has to find such a column
	if(cCount==m_cBinding)
		goto CLEANUP;

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);		

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	//make sure GetData should be able to see the change
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query::Variation_2()
{
	DBPROPID	rgPropertyIDs[2];
	DBCOUNTITEM	cRows		=0;
	DBORDINAL	cCol		=0;
	DBORDINAL	*rgColNumber=NULL;
	HROW		*pHRow		=NULL;
	ULONG		cCount		=0;
	BYTE		*pData		=NULL;
	BOOL		fTestPass	=TEST_SKIPPED;
	DBROWSTATUS	*DBRowStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	//create an accessor with IRowsetChange.  Buffered update mode.
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,2,
										g_rgPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
		
	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;
	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,&pData,g_ulNextRow++,
								m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set a nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cCol-1])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}
		}
	}

	//has to find such a column
	if(cCount==m_cBinding)
	{
		goto CLEANUP;
	}

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//call update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,&DBRowStatus),S_OK);
		
	//make sure GetData should be able to see the change
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;	
CLEANUP:
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary cases with qbu cursor.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query::Variation_3()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows		= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;
	DBORDINAL	cCol		= 0;
	DBORDINAL	ulCol		= 0;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}

	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,(&rgColNumber[ulCol]) ))

	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,NULL),E_INVALIDARG);		
	TESTC_(SetData(*pHRow,NULL,pData),DB_E_BADACCESSORHANDLE);		
	TESTC_(SetData(NULL,m_hAccessor,pData),DB_E_BADROWHANDLE);		
	TESTC_(SetData(*pHRow,hAccessor,NULL),S_OK);		
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query::Variation_4()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;		
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	void		*pGetData		= NULL;
	DBPROPID	rgPropertyIDs[1];
	DBSTATUS	dbsSecondColStatus;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	DBORDINAL	ulCol			= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	 
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	fTestPass=FALSE;

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol]) ))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the second status binding to anything other than _OK or NULL
//	*(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus)=DBSTATUS_S_TRUNCATED;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_TRUNCATED;
	
	// keep track of colstatus
//	dbsSecondColStatus = *(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus);
	dbsSecondColStatus = STATUS_BINDING(m_rgBinding[0],pData);

	//set data should fail
	m_hr=SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE) )
		goto CLEANUP;
	
	TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_BADSTATUS);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRow);

	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		if(m_cBinding > 1)
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),dbsSecondColStatus);
	}
	else
	{
		if(m_cBinding > 1)
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_UNAVAILABLE);
			
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
			goto CLEANUP;

		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//columns should not be changed
		if(!COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetData not supported, qbu
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query::Variation_5()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows		= 0;
	ULONG		*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_NOTSUPPORTED);		
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
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
BOOL Forward_Query::Terminate()
{
	return(TCIRowsetChange::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Forward_Cursor)
//*-----------------------------------------------------------------------
//| Test Case:		Forward_Cursor - Forward only cursor.  Cursor based update.
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Cursor::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_1()
{
	DBCOUNTITEM	cRows		=0;
	DBPROPID	DBPropIDSet	=DBPROP_IRowsetChange;
	HROW		*pHRow		=NULL;
	BYTE		*pData		=NULL;
	BOOL		fTestPass	=TEST_SKIPPED;
	

	ULONG	rgNumber=1;
	
	TESTC_PROVIDER((SELECT_ALLFROMTBL,1,&DBPropIDSet,2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,
					DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY, 
					FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&rgNumber));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//make sure GetData should be able to see the change
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;	
CLEANUP:
	PROVIDER_FREE(pData);
	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode.  Retrieve a row handle, update a row with some columns set to NULL, and call GetData to see the change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_2()
{
	DBCOUNTITEM	cRows		=0;		
	DBORDINAL	cCol		=0;
	DBORDINAL	*rgColNumber=NULL;
	HROW		*pHRow		=NULL;
	ULONG		cCount		=0;
	BYTE		*pData		=NULL;
	DBPROPID	DBPropIDSet	=DBPROP_IRowsetUpdate;
	BOOL		fTestPass	=TEST_SKIPPED;

	//create an accessor with IRowsetChange.  Buffered update mode.
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,&DBPropIDSet,2,
										g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	
	//set a first nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cCol-1])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}
		}
	}

	//has to find such a column
	if(cCount==m_cBinding)
		goto CLEANUP;

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//call update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//make sure GetData should be able to see the change
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;
	
CLEANUP:
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);
	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Change a row with DBTYPE_BYREF.  cbMaxLen > string length, no truncation should occur, SetData should return S_OK.
//		no truncation should occur, SetData should return S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_3()
{
	HRESULT		hr			 = S_OK;
	BOOL		fTestPass	 = TEST_SKIPPED;
	DBCOUNTITEM	cRows		 = 0;
	HROW		*pHRow		 = NULL;
	DBORDINAL	cCnt		 = 0;
	DBORDINAL	ulCol		 = 0;
	DBORDINAL	*rgColNumber = NULL;
	void		*pData		 = NULL;
	DBORDINAL	rgColToBind[1];
	DBPROPID	rgPropertyIDs[1];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//the first col here is the index
	rgColToBind[0]=1;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;
	
	fTestPass = FALSE;

	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if( ((m_rgInfo[rgColNumber[cCnt]].wType & DBTYPE_ARRAY) == 0) &&
			((m_rgInfo[rgColNumber[cCnt]].wType & DBTYPE_VECTOR) == 0) )
		  break;
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	//this is the variable length updateable column
	rgColToBind[0]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(	ON_ROWSET_ACCESSOR,
										TRUE,
										DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY,
										FORWARD,
										VARIABLE_LEN_COLS_BY_REF,
										DBTYPE_EMPTY,
										1,
										rgColToBind))

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	m_rgBinding[0].cbMaxLen=0;	

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data should fail
	hr=SetData(*pHRow,m_hAccessor,pData);
		
	if(CHECK(hr==S_OK, TRUE))	
		fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	//release the row handle
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_4()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows				=0;
	HROW		*pHRow				=NULL;
	HROW		*pHRowSecond		=NULL;
	ULONG		cRefCount			=0;
	DBPROPID	rgPropertyIDs[1];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	HRESULT hr = S_OK;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);

	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
		

	//release the row & data
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pData);

	//make data and insert it in a new row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//should be delete and here because DBPROP_REMOVEDELETED is variant_false
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),DB_E_DELETEDROW);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
    //Since GetData is expected to fail we do not need to release this memory
    //ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	PROVIDER_FREE(m_pData);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_5()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows				=0;
	HROW		*pHRow				=NULL;
	HROW		*pHRowSecond		=NULL;
	ULONG		cRefCount			=0;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass			=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[2]=DBPROP_IConnectionPointContainer;

	HRESULT hr = S_OK;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);

	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
		

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pData);

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//should be S_OK here because DBPROP_REMOVEDELETED is variant_true
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	PROVIDER_FREE(m_pData);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Insert a row and check to see it (OWNINSERT is off)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_6()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows				=0;
	HROW		*pHRow				=NULL;
	DBPROPID	rgPropertyIDs[2];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;
	HRESULT		hr					= S_OK;
	HRESULT		RPhr				= S_OK;
	BOOL		fFound				= FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_LITERALIDENTITY;

	rgUnPropertyIDs[0]=DBPROP_OWNINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//get a new data buffer to set the data a row. 
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
	m_ulTableRows++;

	//restart
	RPhr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(RPhr==S_OK || RPhr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//loop through rowset searching for the inserted row
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fFound	= TRUE;
		}
	}
	//OWNINSERT is FALSE, if the test sees the row, FAIL
	//only if the command was not re-ececuted because if it was re-exectued the rowset is new
	//and the props do not apply so see the row
	if (RPhr==DB_S_COMMANDREEXECUTED)
	{
		if(fFound)
		{
			fTestPass	= TRUE;
		}
	}
	else
	{
		if(!fFound)
		{
			fTestPass	= TRUE;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(m_pData);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, SetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_7()
{
	DBCOUNTITEM	cRows					= 0;
	const ULONG	cRowsToSet				= 5;
	HROW		*pHRow					= NULL;
	BYTE		*pData					= NULL;
	BOOL		fTestPass				= TEST_SKIPPED;
	DBPROPID	rgPropertyIDs[4];
	void		*rgpData[cRowsToSet]	= {NULL,NULL,NULL,NULL,NULL};
	ULONG		cCount					= 0;
	BOOL		fFound					= FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_ACCESSORDER;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	//set the value for DBPROP_ACCESSORDER 
	g_ACCESSORDER_VALUE = DBPROPVAL_AO_RANDOM;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{	
		goto CLEANUP;
	}

	//make data for SetData
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	fTestPass = FALSE;
	
	//get the row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,cRowsToSet,&cRows,&pHRow),S_OK);

	//set data on these handles
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		//set data
		TESTC_(SetData(pHRow[cCount],m_hAccessor,rgpData[cCount]),S_OK);
	}
	
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(cRowsToSet, pHRow, NULL,NULL,NULL),S_OK);

	//back to the top of the rowset
	m_hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(m_hr==S_OK || m_hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//retrieve the first row handle that was just Set
	while (S_OK==(m_hr=m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || m_hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should see the data
		if(CompareBuffer(m_pData,rgpData[0],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)		
		{
			fFound=TRUE;
			break;
		}
	}

	if (!fFound)
	{
		goto CLEANUP;
	}

	//check data on these handles
	for(cCount=1; cCount<cRowsToSet; cCount++)
	{
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

		//get the data for the row
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change/data
		if(!COMPARE(CompareBuffer(m_pData,rgpData[cCount],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;	
CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc set ACCESSOR_ORDER to RANDOM with forward only cursor of BLOBS are cached, Insert
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_8()
{
	DBCOUNTITEM	cRows					= 0;
	const ULONG	cRowsToSet				= 5;
	HROW		*pHRow					= NULL;
	BYTE		*pData					= NULL;
	BOOL		fTestPass				= TEST_SKIPPED;
	DBPROPID	rgPropertyIDs[4];
	void		*rgpData[cRowsToSet]	= {NULL,NULL,NULL,NULL,NULL};
	ULONG		cCount					= 0;
	BOOL		fFound					= FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_ACCESSORDER;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	//set the value for DBPROP_ACCESSORDER 
	g_ACCESSORDER_VALUE = DBPROPVAL_AO_RANDOM;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	if (!m_pIRowsetChange)
	{	
		goto CLEANUP;
	}

	//make data for Insert
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	fTestPass = FALSE;
	
	//get some row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,cRowsToSet,&cRows,&pHRow),S_OK);

	//set data on these handles
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		//insert row
		TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,rgpData[cCount],NULL),S_OK);
	}
	
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(cRowsToSet, pHRow, NULL,NULL,NULL),S_OK);

	//back to the top of the rowset
	m_hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(m_hr==S_OK || m_hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//retrieve the first row handle that was just inserted
	while (S_OK==(m_hr=m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || m_hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should see the data
		if(CompareBuffer(m_pData,rgpData[0],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)		
		{
			fFound=TRUE;
			break;
		}
	}

	if (!fFound)
	{
		goto CLEANUP;
	}

	//check data on these handles
	for(cCount=1; cCount<cRowsToSet; cCount++)
	{
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

		//get the data for the row
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change/data
		if(!COMPARE(CompareBuffer(m_pData,rgpData[cCount],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}
	fTestPass=TRUE;	
CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<cRowsToSet; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Fail the SetData and check that the status of the remaining unset cols is DBSTATUS_E_UNAVAILABLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_9()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	ULONG			cCount			= 0;
	HROW			*pHRow			= NULL;
	BYTE			*pData		= NULL;
	DBPROPID		rgPropertyIDs[2];
	HRESULT			hr;
	DBSTATUS		sCheckStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//acceesor with only length binding
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	
	//get a new data buffer to set the data for a row. 
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
//	*(ULONG *)((DWORD)pData+m_rgBinding[0].obStatus)=DBSTATUS_E_BADSTATUS;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_E_BADSTATUS;

	//set data should fail
	hr=SetData(*pHRow,m_hAccessor,pData);
	
	if (DB_E_ERRORSOCCURRED==hr)
	{
		sCheckStatus = DBSTATUS_E_UNAVAILABLE;
	}
	else
	{
		if(DB_S_ERRORSOCCURRED==hr)
		{
			sCheckStatus = DBSTATUS_S_OK;
		}
		else
		{
			goto CLEANUP;
		}
	}

	//the first col should fail
	//the other col should be OK or UNAVAILABLE depending on the provider
	for(cCount=1;cCount<m_cBinding;cCount++)
	{
		//GetColInfo will return info on the bookmark col but the bookmark probablly won't be bound
		//by the accessor, if so the arrays will be off by one.
		if (m_rgInfo[cCount].iOrdinal==m_rgBinding[cCount].iOrdinal)
		{
			if (m_rgInfo[cCount].dwFlags&DBCOLUMNFLAGS_WRITEUNKNOWN)
			{
				continue;
			}
		}
		else
		{
			if (m_rgInfo[cCount+1].dwFlags&DBCOLUMNFLAGS_WRITEUNKNOWN)
			{
				continue;
			}
		}

		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),sCheckStatus))
		{
			goto CLEANUP;
		}
	}
	fTestPass=TRUE;	
CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
//
// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_10()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount;
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset so the test can pick not nullable updatable cols
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_STATUS,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	//get an array of numeric and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);
	//has to find such a column
	if(!cCol)
	{
		goto CLEANUP;
	}

	fTestPass=FALSE;
	
	//Create a new accessor that binds only the nullable updatable cols
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}
	//set data should fail
	TEST2C_(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED, E_FAIL);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),DBSTATUS_E_UNAVAILABLE))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_ISNULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_11()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount;
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset so the test can pick not nullable updatable cols
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_STATUS,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	//get an array of numeric and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);
	//has to find such a column
	if(!cCol)
	{
		goto CLEANUP;
	}

	fTestPass=FALSE;
	
	//Create a new accessor that binds only the not nullable updatable cols
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
	}

	//set data should pass
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),DBSTATUS_S_ISNULL))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_12()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;		
	DBORDINAL	rgColNumber[2];
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	void		*pGetData		= NULL;
	DBPROPID	rgPropertyIDs[3];
	DBSTATUS	dbsSecondColStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	HRESULT hr = S_OK;

	rgColNumber[0]=1;
	rgColNumber[1]=3;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	 
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	
	
	//create an accessor on the command object on updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,
										2,rgColNumber))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the second status binding to anything other than _OK or NULL
//	*(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus)=DBSTATUS_S_TRUNCATED;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_TRUNCATED;
	
	// keep track of 2nd colstatus
//	dbsSecondColStatus = *(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[1].obStatus);
	dbsSecondColStatus = STATUS_BINDING(m_rgBinding[1],pData);

	//set data should fail
	m_hr=SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE) )
		goto CLEANUP;
	
	TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_BADSTATUS);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRow);

	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		if(m_cBinding > 1)
			TESTC_(GetStatus(pData, &(m_rgBinding[1])),dbsSecondColStatus);
	}
	else
	{
		if(m_cBinding > 1)
			TESTC_(GetStatus(pData, &(m_rgBinding[1])),DBSTATUS_E_UNAVAILABLE);
			
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
			goto CLEANUP;

		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//columns should not be changed
		if(!COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
			goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// }}
// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc SetData the variable length columns without the length being bound. should be ok
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_13()
{
	HROW		hRow			= NULL;
	HROW		*pHRow			= NULL;
	DBCOUNTITEM	cRows			= 0;
	void		*pData			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	DBORDINAL	*rgColsToBind	= NULL;
	ULONG		cColsNumber		= 0;
	ULONG		cCount			= 0;
	DBPROPID	rgDBPROPID[2];
	HRESULT		hr;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;

	//get a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR));

	rgColsToBind=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cBinding);

	//get variable length string cols and not nullable cols
	//also grab cols that are not nullable so they won't cause the insert to fail
	for (cCount=0;cCount<m_cRowsetCols;cCount++)
	{
		if	(	(m_rgInfo[cCount].wType == DBTYPE_WSTR || m_rgInfo[cCount].wType == DBTYPE_STR)	||
				!(m_rgInfo[cCount].dwFlags & DBCOLUMNFLAGS_ISNULLABLE) &&
				(	(m_rgInfo[cCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
					(m_rgInfo[cCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
		   )	)

		{
			if (0==m_rgInfo[cCount].iOrdinal)
			{
				continue;
			}
			rgColsToBind[cColsNumber]=m_rgInfo[cCount].iOrdinal;
			cColsNumber++;
		}
	}
	if(!cColsNumber)
	{
		goto CLEANUP;
	}
	
	ReleaseRowsetAndAccessor();
	//reset this global.  hack for now
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	fTestPass	= TEST_FAIL;
	
	//value and status binding only
	TESTC(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind));
	//make data for setdata
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//SetData should be successful, variable length cols can use the NULL terminated string
	//they do not need the length 
	hr=SetData(*pHRow,m_hAccessor,pData);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(S_OK!=hr)
	{
		//debug this a bit
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
//			COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus),DBSTATUS_S_OK);
			COMPARE(STATUS_BINDING(m_rgBinding[cCount],pData),DBSTATUS_S_OK);
		}
		goto CLEANUP;
	}

	//check the row is inserted
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
	{
		goto CLEANUP;
	}
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the row handle
		hr = GetData(*pHRow,m_hAccessor,m_pData);
		//Taking cares of SQL Providers for keyset cursor
		TESTC(hr == S_OK || hr == DB_E_DELETEDROW);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	PROVIDER_FREE(rgColsToBind);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
//}



// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode. Multiple commands that modify same set of columns in hrow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_14()
{
	const ULONG	cRowsToUse			= 5;
	DBCOUNTITEM	cRows				= 0;
	ULONG		cCol				= 0;
	HROW		*pHRow				= NULL;
	ULONG		cCount				= 0;
	BYTE		*pData0				= NULL;
	BYTE		*pData1				= NULL;
	BYTE		*pData4				= NULL;
	BOOL		fTestPass			= TEST_SKIPPED;
	void		*rgpData[cRowsToUse]= {NULL,NULL,NULL,NULL,NULL};
	HRESULT		hr;
	DBPROPID	DBPropIDSet	=DBPROP_IRowsetUpdate;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	//create an accessor with IRowsrretChange.  Buffered update mode.
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,&DBPropIDSet,2,
										g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;
	
	//alloc data to get orig. rows
	for(cCount=0; cCount<cRowsToUse; cCount++)
	{
		if(!(rgpData[cCount]=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		{
			goto CLEANUP;
		}
	}

	//get the row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,1,cRowsToUse,&cRows,&pHRow),S_OK);
		
	//get the data for the rows
	TESTC_(GetData(pHRow[0],m_hAccessor,rgpData[0]),S_OK);	
	TESTC_(GetData(pHRow[1],m_hAccessor,rgpData[1]),S_OK);	
	TESTC_(GetData(pHRow[2],m_hAccessor,rgpData[2]),S_OK);	
	TESTC_(GetData(pHRow[3],m_hAccessor,rgpData[3]),S_OK);	
	TESTC_(GetData(pHRow[4],m_hAccessor,rgpData[4]),S_OK);	

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData0,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData1,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData4,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data on a few rows
	TESTC_(SetData(pHRow[0],m_hAccessor,pData0),S_OK);
	TESTC_(SetData(pHRow[4],m_hAccessor,pData4),S_OK);
	TESTC_(SetData(pHRow[1],m_hAccessor,pData1),S_OK);

	//call update
	TESTC_(m_pIRowsetUpdate->Update(NULL,cRows,pHRow,NULL,NULL,NULL),S_OK);

	//release all held row handles
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;
		
	//get the row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,1,cRowsToUse,&cRows,&pHRow),S_OK);
		
	//get the data for the row and 	check GetData should have the correct changes
	TESTC_(GetData(pHRow[0],m_hAccessor,m_pData),S_OK);	
	if(!COMPARE(CompareBuffer(m_pData,pData0,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//get the data for the row and 	check GetData should have the correct changes
	TESTC_(GetData(pHRow[1],m_hAccessor,m_pData),S_OK);	
	if(!COMPARE(CompareBuffer(m_pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//get the data for the row and 	check GetData should have the correct changes
	TESTC_(GetData(pHRow[2],m_hAccessor,m_pData),S_OK);	
	if(!COMPARE(CompareBuffer(m_pData,rgpData[2],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	TESTC_(GetData(pHRow[3],m_hAccessor,m_pData),S_OK);	
	if(!COMPARE(CompareBuffer(m_pData,rgpData[3],m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	TESTC_(GetData(pHRow[4],m_hAccessor,m_pData),S_OK);	
	if(!COMPARE(CompareBuffer(m_pData,pData4,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData0);
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData4);
	//Release the row handle and memory
	for(cCount=0; cCount<cRowsToUse; cCount++)
	{
		if(rgpData[cCount])
		{
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
		}
	}
	
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc SetData in a read only rowset (system table)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_15()
{ 
	DBCOUNTITEM		cRows				= 0;
	ULONG			cCol				= 0;
	HROW			*pHRow				= NULL;
	BOOL			fTestPass			= TEST_SKIPPED;
	void			*pData				= NULL;
	void			*pData1				= NULL;
	HRESULT			hr;
	IDBSchemaRowset	*pIDBSchemaRowset	= NULL;		
	IRowset			*pTypesRowset		= NULL;
	WCHAR			*pwszSySTableName		= NULL;
	IRowset			*pIRowset			= NULL;
	IRowsetChange	*pIRowsetChange		= NULL;

	DBPROPSET		rgDBPROPSET[1]		= {NULL};
	DBPROP			rgDBPROP[3]			= {NULL};
	DBID			TableID				= DB_NULLID;

	IColumnsInfo	*pIColumnsInfo		= NULL;
	DBORDINAL		cColumns			= 0;
	DBCOLUMNINFO	*prgInfo			= NULL;
	WCHAR			*pStringsBuffer		= NULL;
	ULONG			cTypeCol			= 0;
	ULONG			cNameCol			= 0;
	ULONG			i					= 0;


	// Initialize updatability and set some rowset properties 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	rgDBPROP[0].dwPropertyID	= DBPROP_IRowsetChange;
	rgDBPROP[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPROP[0].vValue.vt		= VT_BOOL;
	rgDBPROP[0].colid			= DB_NULLID;
   	V_BOOL(&rgDBPROP[0].vValue)	= VARIANT_TRUE;                                   

	rgDBPROP[1].dwPropertyID	= DBPROP_CANHOLDROWS;
	rgDBPROP[1].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPROP[1].vValue.vt		= VT_BOOL;
	rgDBPROP[1].colid			= DB_NULLID;
   	V_BOOL(&rgDBPROP[1].vValue)	= VARIANT_TRUE;                                   

	rgDBPROP[2].dwPropertyID	= DBPROP_UPDATABILITY;
	rgDBPROP[2].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPROP[2].colid			= DB_NULLID;
   	rgDBPROP[2].vValue.vt		= VT_I4;
	V_I4(&rgDBPROP[2].vValue)=DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	rgDBPROPSET[0].rgProperties		= &rgDBPROP[0];
	rgDBPROPSET[0].cProperties		= 3;
	rgDBPROPSET[0].guidPropertySet	= DBPROPSET_ROWSET;

	//get the schema interface
	if(!VerifyInterface(m_pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE,(IUnknown**)&pIDBSchemaRowset))
	{
		goto CLEANUP;
	}

	//get a rowset of the schema table of system tables
	if	(!CHECK(hr = pIDBSchemaRowset->GetRowset(	
						NULL, 								// aggregation
						DBSCHEMA_TABLES,					// REFGUID
						0,									// count of restrictions (1:types)
						NULL,								// list of restrictions
						IID_IRowset,						// REFFID
						0,									// count of properties
						NULL,								// range of properties
						(IUnknown**)&pTypesRowset			// returned result set
		),S_OK))
	{
		goto CLEANUP;
	}

	// get bindings and column info for the schema table of system tables
	if(!CHECK(hr=GetAccessorAndBindings(
		pTypesRowset,								// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&m_hAccessor,								// @parmopt [OUT] Accessor created
		&m_rgBinding,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cBinding,								// @parmopt [OUT] Count of bindings
		&m_cRowSize,								// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		NULL,										// @parmopt [OUT] Array of DBCOLUMNINFO
		NULL,										// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
	{
		goto CLEANUP;
	}

	//alloc memory buffer
	if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//get the colinfo interface
	if(!VerifyInterface(pTypesRowset, IID_IColumnsInfo, ROWSET_INTERFACE,(IUnknown**)&pIColumnsInfo))
	{
		goto CLEANUP;
	}

	//get the position of the table-name and the table-type
	if (!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &prgInfo,&pStringsBuffer), S_OK))
	{
		goto CLEANUP;
	}	
	for (i=0;i<cColumns;i++)
	{
		if (prgInfo[i].pwszName)
		{
			if(wcsstr(prgInfo[i].pwszName, L"TABLE_TYPE"))
			{
				cTypeCol	= i;
			}
			if(wcsstr(prgInfo[i].pwszName, L"TABLE_NAME"))
			{
				cNameCol	= i;
			}
		}
	}

	hr=pTypesRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//skip the first one just cause
	hr	= pTypesRowset->GetNextRows(NULL,0,1,&cRows, &pHRow);
	if (hr!=S_OK && hr != DB_S_ENDOFROWSET)
	{
		goto CLEANUP;
	}
	if( cRows ==0)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}


	TESTC_(pTypesRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//retrieve the row handles
	while (S_OK==(hr = pTypesRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			fTestPass	= TEST_SKIPPED;
			goto CLEANUP;
		}
		//Get the data for the row handle
		TESTC_(pTypesRowset->GetData(pHRow[0],m_hAccessor,pData),S_OK);
		TESTC_(pTypesRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//figure out which are system tables (looking for a read only table)
		if(	wcsstr(((WCHAR*)((BYTE*)pData + m_rgBinding[cTypeCol].obValue)), L"SYSTEM TABLE")	||
			wcsstr(((WCHAR*)((BYTE*)pData + m_rgBinding[cTypeCol].obValue)), L"SYSTEM VIEW"))
		{
			//get the system table name
//			pwszSySTableName = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (*(ULONG *)((DWORD)pData + m_rgBinding[cNameCol].obLength)));
			pwszSySTableName = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) + (LENGTH_BINDING(m_rgBinding[cNameCol],pData)));

//			wcscpy(pwszSySTableName, ((WCHAR*)((BYTE*)pData + m_rgBinding[cNameCol].obValue)));
			wcscpy(pwszSySTableName,  (WCHAR*)&VALUE_BINDING(m_rgBinding[cNameCol], pData));

			//Try to open a rowset on this table and break when we do this successfully


			//set table name to that of the system table
			TableID.uGuid.guid		= GUID_NULL;
			TableID.eKind			= DBKIND_NAME;	
			TableID.uName.pwszName	= wcsDuplicate(pwszSySTableName);
			
			//get the system table rowset
			hr	= g_pIOpenRowset->OpenRowset(		NULL, 
													&TableID, 
													NULL,
													IID_IRowset, 
													1, 
													rgDBPROPSET, 
													(IUnknown **)&pIRowset
											);

			if (DB_SEC_E_PERMISSIONDENIED==hr)
			{
				fTestPass = TEST_PASS;
				goto CLEANUP;
			}
			if (S_OK==hr)
				break;
			else //continue loop
				SAFE_RELEASE(pIRowset);
		}
	}

	//free memory
	if (pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}

	pData=NULL;
	ReleaseRowsetAndAccessor();
	if(pHRow && pTypesRowset)
	{
		CHECK(pTypesRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	//if we could not find a read-only table then skip the test
	if(!pIRowset)
	{
		fTestPass = TEST_SKIPPED;

		goto CLEANUP;
	}

	//get the IRowsetChange pointer
	TESTC_(pIRowset->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange),S_OK);

	if (!pIRowsetChange)
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;
	
	// get bindings and column info for the system table
	if(!CHECK(hr=GetAccessorAndBindings(
		pIRowset,									// @parmopt [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parmopt [IN]  Properties of the Accessor
		&m_hAccessor,								// @parmopt [OUT] Accessor created
		&m_rgBinding,								// @parmopt [OUT] Array of DBBINDINGS
		&m_cBinding,								// @parmopt [OUT] Count of bindings
		&m_cRowSize,								// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND,						// @parmopt [IN]  Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		NULL,										// @parmopt [OUT] Array of DBCOLUMNINFO
		NULL,										// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_pStringsBuffer,
		DBTYPE_EMPTY,								// @parmopt [IN]  Modifier to be OR'd with each binding type.
		0,											// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		NULL,										// @parmopt [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
		NULL,										// @parmopt [IN]  Corresponds to what ordinals are specified for each binding, if 
		NO_COLS_OWNED_BY_PROV, 						// @parmopt [IN]  Which columns' memory is to be owned by the provider
		DBPARAMIO_NOTPARAM,							// @parmopt [IN]  Parameter kind specified for all bindings 
		BLOB_LONG,									// @parmopt [IN]  how to bind BLOB Columns
		NULL),S_OK))								// @parmopt [OUT] returned status array from CreateAccessor
	{
		goto CLEANUP;
	}
	
	//alloc memory buffer
	if(!(pData1=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	hr=pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get a row handle
	hr = pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow);

	//if the table has no rows lets end right here
	if(DB_S_ENDOFROWSET	== hr || DB_E_BADSTARTPOSITION	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	TESTC_(hr,S_OK);
	//get the data for the rows
	TESTC_(pIRowset->GetData(pHRow[0],m_hAccessor,pData1),S_OK);	

	//set data 
	m_hr	= pIRowsetChange->SetData(*pHRow,m_hAccessor,pData1);

	//Do extended check following SetData
	XCHECK(pIRowsetChange, IID_IRowsetChange, E_FAIL);

	if (E_FAIL == m_hr)
	{
		fTestPass=TRUE;
	}
CLEANUP:
	//free the memory used by pData
	if (pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if (pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}	
	if(pHRow && pIRowset)
	{
		CHECK(pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(TableID.uName.pwszName);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(prgInfo);
	PROVIDER_FREE(m_pStringsBuffer);
	PROVIDER_FREE(pwszSySTableName);

	//release rowsets
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pTypesRowset);
	SAFE_RELEASE(pIDBSchemaRowset);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has length binding.  E_FAIL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_16()
{
	BOOL		fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount=0;
	HROW		*pHRow=NULL;
	void		*pData=NULL;
	DBPROPID	rgPropertyIDs[1];

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);		
	
	//acceesor with only length binding
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
									DBPART_LENGTH,UPDATEABLE_COLS_BOUND))

	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
		LENGTH_BINDING(m_rgBinding[cCount],pData)=2;
	}
	//set data should fail
	TEST2C_(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED, E_FAIL);	
	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc The length binding > cbMaxLen for a variable length column. (DB_S_ERRORSOCCURRED)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_17()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	ULONG			cCnt			= 0;
	DBORDINAL		ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBORDINAL		rgColToBind[2];
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[1];

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_ARRAY) == 0) &&
			((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_VECTOR) == 0)
		  )
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	

	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		2,rgColToBind))

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the data length to cbMaxLen + 1
//	*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)=m_rgBinding[1].cbMaxLen+1;
	LENGTH_BINDING(m_rgBinding[1],pData)=m_rgBinding[1].cbMaxLen+2; // WCHAR length assert

	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);
	
	if (DB_E_ERRORSOCCURRED==hr)
	{
//		if(!COMPARE(DBSTATUS_E_CANTCONVERTVALUE, *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)))
		if(!COMPARE(STATUS_BINDING(m_rgBinding[1],pData),DBSTATUS_E_CANTCONVERTVALUE))
		{
			goto CLEANUP;
		}
		//check the staus for the 1st column
//		if(!COMPARE(DBSTATUS_E_UNAVAILABLE, *(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
		if(!COMPARE(STATUS_BINDING(m_rgBinding[0],pData),DBSTATUS_E_UNAVAILABLE))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
//			if(!COMPARE(DBSTATUS_E_CANTCONVERTVALUE, *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)))
			if(!COMPARE(STATUS_BINDING(m_rgBinding[1],pData),DBSTATUS_E_CANTCONVERTVALUE))
			{
				goto CLEANUP;
			}

			//check the staus for the 1st column
//			if(!COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
			if(!COMPARE(STATUS_BINDING(m_rgBinding[0],pData),DBSTATUS_S_OK))
			{
				goto CLEANUP;
			}
		}
		else
		{
			TEST2C_(hr, DB_E_CANTCONVERTVALUE, E_FAIL);            
		}
	}

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc check for truncation in variable length columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_18()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	ULONG			cCnt			= 0;
	DBORDINAL		ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBORDINAL		rgColToBind[2];
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[1];
	HACCESSOR		hAccessor		= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber,FALSE,FALSE))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	//and not long
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	(	((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_ARRAY) == 0) &&
				((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_VECTOR) == 0))		&&
				((m_rgInfo[(rgColNumber[cCnt]-1)].dwFlags & DBCOLUMNFLAGS_ISLONG) == 0)
		  )
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
		
	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		2,rgColToBind))

	//reset the cbMaxLen to cbMaxLen + 2
	m_rgBinding[1].cbMaxLen+=2;

	//create an accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding, m_rgBinding, 0, &hAccessor,NULL),S_OK);		

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the data length to cbMaxLen + 2
//	*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)=m_rgBinding[1].cbMaxLen;
	LENGTH_BINDING(m_rgBinding[1],pData)=m_rgBinding[1].cbMaxLen+2;

    
	//set data should fail
	hr = SetData(*pHRow,hAccessor,pData);
	
	if (DB_E_ERRORSOCCURRED==hr)
	{
//		if	(	DBSTATUS_E_CANTCONVERTVALUE	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)	&&
//				DBSTATUS_E_SCHEMAVIOLATION	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)
		if	(	DBSTATUS_E_CANTCONVERTVALUE	!= STATUS_BINDING(m_rgBinding[1],pData)	&&
				DBSTATUS_E_SCHEMAVIOLATION	!= STATUS_BINDING(m_rgBinding[1],pData)
			)
		{
			goto CLEANUP;
		}
		//check the staus for the 1st column
//		if(!COMPARE(DBSTATUS_E_UNAVAILABLE,*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
		if(!COMPARE(DBSTATUS_E_UNAVAILABLE,STATUS_BINDING(m_rgBinding[0],pData)))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
//		if	(	DBSTATUS_E_CANTCONVERTVALUE	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)	&&
//				DBSTATUS_E_SCHEMAVIOLATION	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)
		if	(	DBSTATUS_E_CANTCONVERTVALUE	!= STATUS_BINDING(m_rgBinding[1],pData)	&&
				DBSTATUS_E_SCHEMAVIOLATION	!= STATUS_BINDING(m_rgBinding[1],pData)
			)
		{
			goto CLEANUP;
		}

			//check the staus for the 1st column
//			if(!COMPARE(DBSTATUS_S_OK,*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
			if(!COMPARE(DBSTATUS_S_OK,STATUS_BINDING(m_rgBinding[0],pData)))
			{
				goto CLEANUP;
			}
		}
		else
		{
			TEST2C_(hr,DB_E_CANTCONVERTVALUE, E_FAIL);
		}
	}

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
				
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Delete-Insert-undoDelete-Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_19()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	DBORDINAL		*rgColNumber	= NULL;
	DBPROPID		rgPropertyIDs[3];
	DBROWSTATUS		*prgRowStatus	= NULL;
	HROW			*rgUpdatedRows	= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	HRESULT hr = S_OK;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,0,
										NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);
			
	//insert a row with the same data
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,m_pData,NULL),S_OK);

	//undo the delete, leaving 2 of the same row
	TESTC_(m_pIRowsetUpdate->Undo(NULL,1,pHRow, NULL, NULL, NULL),S_OK);

	//transmitt all the changes (just the insert should be transmitted)
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cRows,&rgUpdatedRows,&prgRowStatus),DB_E_ERRORSOCCURRED);
	COMPARE(prgRowStatus[0],DBROWSTATUS_E_INTEGRITYVIOLATION);
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(rgUpdatedRows);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc check status NULL if no row handles used
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_20()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	DBORDINAL		*rgColNumber	= NULL;
	DBPROPID		rgPropertyIDs[3];
	DBROWSTATUS		*prgRowStatus	= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;


	HRESULT hr = S_OK;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,0,
										NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//insert a row with the same data
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,m_pData,NULL),S_OK);

	//transmitt all the changes (just the insert should be transmitted)
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,&prgRowStatus),DB_E_ERRORSOCCURRED);

	//this has a 1 to 1 correspondance with rghRows or *prgRows
	//is both are null there is no correspondance so this should be NULL
	if (prgRowStatus)
	{
		goto CLEANUP;
	}
		
	fTestPass=TRUE;
CLEANUP:
	if (prgRowStatus)
	{
		PROVIDER_FREE(prgRowStatus);
	}
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc The length binding > cbMaxLen for a variable length column with no status bound. (DB_S_ERRORSOCCURRED)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_21()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	ULONG			cCnt			= 0;
	DBORDINAL		ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBORDINAL		rgColToBind[2];
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[1];

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_ARRAY) == 0) &&
			((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_VECTOR) == 0)
		  )
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
		
	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		2,rgColToBind))

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the data length to cbMaxLen + 1
//	*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)=m_rgBinding[1].cbMaxLen+1;
	LENGTH_BINDING(m_rgBinding[1],pData)=m_rgBinding[1].cbMaxLen+2; //WCHAR length assert

	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);
	
	// Check for valid returncodes
    TEST4C_(hr,DB_E_ERRORSOCCURRED,DB_S_ERRORSOCCURRED,DB_E_CANTCONVERTVALUE, E_FAIL);

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc check for truncation in variable length columns with no status bound.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_22()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	ULONG			cCnt			= 0;
	DBORDINAL		ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBORDINAL		rgColToBind[2];
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[1];
	HACCESSOR		hAccessor		= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	(	((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_ARRAY) == 0) &&
				((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_VECTOR) == 0))		&&
				((m_rgInfo[(rgColNumber[cCnt]-1)].dwFlags & DBCOLUMNFLAGS_ISLONG) == 0)
		  )
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available."<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
		
	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,2,rgColToBind))

	//reset the cbMaxLen to cbMaxLen + 2
	m_rgBinding[1].cbMaxLen+=2;

	//create an accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding, m_rgBinding, 0, &hAccessor,NULL),S_OK);		

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the data length to cbMaxLen
//	*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)=m_rgBinding[1].cbMaxLen;
	LENGTH_BINDING(m_rgBinding[1],pData)=m_rgBinding[1].cbMaxLen;

	//set data should fail
	hr = SetData(*pHRow,hAccessor,pData);
	
	// Check for valid returncodes
    TEST4C_(hr,DB_E_ERRORSOCCURRED,DB_S_ERRORSOCCURRED,DB_E_CANTCONVERTVALUE, E_FAIL);  

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
				
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc QI check
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_23()
{
	BOOL			fTestPass		= TEST_FAIL;
	ULONG			cDBPropSet		= 1;
	DBPROPSET		rgDBPropSet[1];
	HRESULT			hr;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	//init DBPropSet with FALSE DBPROP_IRowsetChange
	rgDBPropSet[0].rgProperties		= NULL;
	rgDBPropSet[0].cProperties		= cDBPropSet;
	rgDBPropSet[0].guidPropertySet	= DBPROPSET_ROWSET;

	rgDBPropSet[0].rgProperties		= (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP)*(1));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(1));
	if(!rgDBPropSet[0].rgProperties)
	{
		goto CLEANUP;
	}

	rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_FALSE;

	//create a rowset without an accessor
	if(!SUCCEEDED(SetRowsetProperties(rgDBPropSet, cDBPropSet)))
	{
		goto CLEANUP;
	}

	hr=CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR);

	//check for DB_E_ERRORSOCCURED in case DBPROP_IRowsetChange is not settable to FALSE
	if (S_OK==hr)
	{
		//get the IRowsetChange pointer
		TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetChange, (LPVOID *)&m_pIRowsetChange),E_NOINTERFACE);
	}
	else
	{
		if (DB_E_ERRORSOCCURRED!=hr)
		{
			goto CLEANUP;
		}
	}
	fTestPass	= TEST_PASS;
CLEANUP:
	if(rgDBPropSet[0].rgProperties)			   
	{//
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	}
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc check for truncation in fixed length columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_24()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	ULONG			cCnt			= 0;
	ULONG			ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
//	DBORDINAL		rgColToBind[2];
	void			*pData			= NULL;
//	DBPROPID		rgPropertyIDs[1];
	HACCESSOR		hAccessor		= NULL;

/*
	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetFixedLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no fixed length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}

	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	//and not long
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_ARRAY) == 0)	&&
			((m_rgInfo[(rgColNumber[cCnt]-1)].wType & DBTYPE_VECTOR) == 0)	&&
			(m_rgInfo[rgColNumber[cCnt]].iOrdinal)		//not the bookmark value
			)
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no fixed length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
		
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		2,rgColToBind))

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	m_rgBinding[1].wType = DBTYPE_I8;

	//create an accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding, m_rgBinding, 0, &hAccessor,NULL),S_OK);		

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//set data should fail
	hr = SetData(*pHRow,hAccessor,pData);
	
	if (DB_E_ERRORSOCCURRED==hr)
	{
//		if	(	DBSTATUS_E_DATAOVERFLOW		!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)	&&
//				DBSTATUS_E_SCHEMAVIOLATION	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)
		if	(	DBSTATUS_E_DATAOVERFLOW		!= STATUS_BINDING(m_rgBinding[1],pData)	&&
				DBSTATUS_E_SCHEMAVIOLATION	!= STATUS_BINDING(m_rgBinding[1],pData)
			)
			)
		{
			goto CLEANUP;
		}
		//check the staus for the 1st column
//		if(!COMPARE(DBSTATUS_E_UNAVAILABLE,*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
		if(!COMPARE(DBSTATUS_E_UNAVAILABLE,STATUS_BINDING(m_rgBinding[0],pData)))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
//			if	(	DBSTATUS_E_DATAOVERFLOW		!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)	&&
//					DBSTATUS_E_SCHEMAVIOLATION	!= *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)
			if	(	DBSTATUS_E_DATAOVERFLOW		!= STATUS_BINDING(m_rgBinding[1],pData)	&&
					DBSTATUS_E_SCHEMAVIOLATION	!= STATUS_BINDING(m_rgBinding[1],pData)
				)
			{
				goto CLEANUP;
			}

			//check the staus for the 1st column
//			if(!COMPARE(DBSTATUS_S_OK,*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)))
			if(!COMPARE(DBSTATUS_S_OK,STATUS_BINDING(m_rgBinding[0],pData)))
			{
				goto CLEANUP;
			}
		}
		else
		{
			TESTC_(hr,DB_E_DATAOVERFLOW);
		}
	}

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
				
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
*/	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc bind BLOBs as IUnKnown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_25()
{
	HRESULT		hr					= S_OK;
	BOOL		fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM	cRows				= 0;
	HROW		*pHRow				= NULL;
	ULONG		cCnt				= 0;
	ULONG		ulCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	void		*pData				= NULL;
	void		*pData1				= NULL;
	DBORDINAL	rgColToBind[1];
	DBPROPID	rgPropertyIDs[1];
	HACCESSOR	hAccessor			= NULL;
	DBLENGTH	cbRowSize			= 0;
	ULONG		i					= 0;


	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//the first col here is the index
	rgColToBind[0]=1;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all columns. Include BLOB_IID_ISEQSTREAM
	//and BLOB_BIND_STR.
	TESTC_(GetAccessorAndBindings(	m_pIRowset, 
									DBACCESSOR_ROWDATA,
									&hAccessor, 
									&m_rgBinding,
									&m_cBinding, 
									&cbRowSize,			
  									DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
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
									BLOB_IID_ISEQSTREAM|BLOB_BIND_STR, 
									NULL),S_OK);

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//allocate memory for the row
	if(!(pData1=PROVIDER_ALLOC(cbRowSize)))
	{
		goto CLEANUP;
	}

	//Get the data for the row handle
	TESTC_(GetData(*pHRow,hAccessor,pData1),S_OK);

	m_rgBinding[0].cbMaxLen=0;	

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data should fail
	hr=SetData(*pHRow,hAccessor,pData);
		
	if (E_UNEXPECTED == hr)
	{
		if(!GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS, DBPROPSET_ROWSET, (IUnknown*)m_pIRowset))		
		{
			goto CLEANUP;
		}
	}
	else
	{
		CHECK(hr==S_OK, TRUE);
	}
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	IUnknown* pIUnknown = NULL;

	if(pData)
	{
		//Get our storage object first
		COMPARE(GetStorageObject(m_cBinding, m_rgBinding, pData, IID_ISequentialStream, &pIUnknown),TRUE);
		SAFE_RELEASE(pIUnknown);

		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pData1)
	{
		//Get our storage object first
		COMPARE(GetStorageObject(m_cBinding, m_rgBinding, pData1, IID_ISequentialStream, &pIUnknown),TRUE);
		SAFE_RELEASE(pIUnknown);

		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}


	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	for (i=1;i<m_cBinding;i++)
	{
		PROVIDER_FREE(m_rgBinding[i].pObject);
	}

	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_IRowsetChange FALSE with UPDATABILITY - conflicting
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_26()
{
	BOOL			fTestPass		= TEST_FAIL;
	ULONG			cDBPropSet		= 1;
	DBPROPSET		rgDBPropSet[1];

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	//init DBPropSet with FALSE DBPROP_IRowsetChange
	rgDBPropSet[0].rgProperties		= NULL;
	rgDBPropSet[0].cProperties		= 2;
	rgDBPropSet[0].guidPropertySet	= DBPROPSET_ROWSET;

	rgDBPropSet[0].rgProperties		= (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP)*(2));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(2));
	if(!rgDBPropSet[0].rgProperties)
	{
		goto CLEANUP;
	}

	rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_FALSE;

	rgDBPropSet[0].rgProperties[1].dwPropertyID		= DBPROP_UPDATABILITY;                    
   	rgDBPropSet[0].rgProperties[1].dwOptions		= DBPROPOPTIONS_REQUIRED;                                              
   	rgDBPropSet[0].rgProperties[1].vValue.vt		= VT_I4;
   	rgDBPropSet[0].rgProperties[1].colid			= DB_NULLID;
	V_I4(&rgDBPropSet[0].rgProperties[1].vValue)	= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset without an accessor
	if(!SUCCEEDED(SetRowsetProperties(rgDBPropSet, cDBPropSet)))
	{
		goto CLEANUP;
	}

	TEST2C_(CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR),DB_E_ERRORSOCCURRED,DB_S_ERRORSOCCURRED);
				
	if(DBPROPSTATUS_CONFLICTING	== m_rgPropSets[0].rgProperties[1].dwStatus)
	{
		fTestPass	= TEST_PASS;
	}

	fTestPass	= TEST_PASS;
CLEANUP:
	if(rgDBPropSet[0].rgProperties)			   
	{
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	}
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_IRowsetChange and DBPROP_IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_27()
{
	BOOL			fTestPass		= TEST_FAIL;
	ULONG			cDBPropSet		= 1;
	DBPROPSET		rgDBPropSet[1];
	HRESULT			hr;

	//make sure change is supported so DB+E+ERRORSOCCURRED can not  be returned
	if (!SupportedInterface(IID_IRowsetChange,ROWSET_INTERFACE))
	{
		odtLog << L"IRowsetChange is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	//init DBPropSet with FALSE DBPROP_IRowsetChange
	rgDBPropSet[0].rgProperties		= NULL;
	rgDBPropSet[0].cProperties		= 3;
	rgDBPropSet[0].guidPropertySet	= DBPROPSET_ROWSET;

	rgDBPropSet[0].rgProperties		= (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP)*(3));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(3));
	if(!rgDBPropSet[0].rgProperties)
	{
		goto CLEANUP;
	}

	rgDBPropSet[0].rgProperties[0].dwPropertyID		= DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)	= VARIANT_TRUE;

	rgDBPropSet[0].rgProperties[1].dwPropertyID		= DBPROP_UPDATABILITY;                    
   	rgDBPropSet[0].rgProperties[1].dwOptions		= DBPROPOPTIONS_REQUIRED;                                              
   	rgDBPropSet[0].rgProperties[1].vValue.vt		= VT_I4;
   	rgDBPropSet[0].rgProperties[1].colid			= DB_NULLID;
	V_DBCOUNTITEM(&rgDBPropSet[0].rgProperties[1].vValue)	= m_ulpUpdFlags;

	rgDBPropSet[0].rgProperties[2].dwPropertyID		= DBPROP_IRowsetUpdate;
	rgDBPropSet[0].rgProperties[2].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[2].vValue.vt		= VT_BOOL;
	rgDBPropSet[0].rgProperties[2].colid			= DB_NULLID;
	V_BOOL(&rgDBPropSet[0].rgProperties[2].vValue)	= VARIANT_TRUE;

	//create a rowset without an accessor
	if(!SUCCEEDED(SetRowsetProperties(rgDBPropSet, cDBPropSet)))
	{
		goto CLEANUP;
	}
	
	if(g_pTable->GetCommandSupOnCTable())
	{
		//allow DB_S_ERRORSOCCURRED here in case DBPROP_IRowsetUpdate is not supported but even if 
		//DBPROP_IRowsetUpdate is not supported DB_E_ERRORSOCCURRED should not be returned on call to
		//SetProperties.
		TEST2C_(hr=CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR),S_OK,DB_S_ERRORSOCCURRED);
		TEST3C_(m_rgPropSets[0].rgProperties[2].dwStatus,DBPROPSTATUS_NOTSUPPORTED,DBPROPSTATUS_NOTSETTABLE,DBPROPSTATUS_OK);
	}
	else
	{
		//allow DB_E_ERRORSOCCURRED here in case DBPROP_IRowsetUpdate is not supported but even if 
		//DBPROP_IRowsetUpdate is not supported DB_S_ERRORSOCCURRED should not be returned on call to
		//OpenRowset
		TEST2C_(hr=CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR),S_OK,DB_E_ERRORSOCCURRED);
		TEST3C_(m_rgPropSets[0].rgProperties[2].dwStatus,DBPROPSTATUS_NOTSUPPORTED,DBPROPSTATUS_NOTSETTABLE,DBPROPSTATUS_OK);
	}
		
	fTestPass	= TEST_PASS;
CLEANUP:
	if(rgDBPropSet[0].rgProperties)			   
	{
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	}
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}



// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc change multiple cols with some failures
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_28()
{
	BOOL		fTestPass				= TEST_SKIPPED;
	DBCOUNTITEM	cRows					= 0;
	HROW		*pHRow					= NULL;
	DBCOUNTITEM	cNotUpdateCol			= 0;
	DBORDINAL	*rgNotUpdateColNumber	= NULL;
	DBCOUNTITEM	cUpdateCol				= 0;
	DBORDINAL	*rgUpdateColNumber		= NULL;
	DBCOUNTITEM	cCol					= 0;
	DBORDINAL	rgColNumber[2]			= {NULL,NULL};
	ULONG		cCount					= 0;
	ULONG		cBinding				= 0;
	void		*pData					= NULL;
	void		*pGetData				= NULL;
	DBPROPID	rgPropertyIDs[1];
	ULONG		i						= 0;
	ULONG		j						= 0;
	HRESULT		hr						= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;


	//create a rowset on all columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,
		TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, ALL_COLS_BOUND));

	//Get Non Updatable column.  If all columns are updatable, exit.
	GetNotUpdatable(&cNotUpdateCol,&rgNotUpdateColNumber);
	if (!cNotUpdateCol)
	{
		goto CLEANUP;
	}

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	
/*	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//Get Non Updatable column.  If all columns are updatable, exit.
	GetNotUpdatable(&cNotUpdateCol,&rgNotUpdateColNumber);
	if (!cNotUpdateCol)
	{
		goto CLEANUP;
	}

	//get updateable columns
	if(!GetUpdatableCols(&cUpdateCol, &rgUpdateColNumber))
	{
		goto CLEANUP;
	}

	//use one from each array
	cCol			= 2;
	rgColNumber[0]	= rgNotUpdateColNumber[0];
	rgColNumber[1]	= rgUpdateColNumber[0];

	//create an accessor on the rowset object on the all the columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
										DBTYPE_EMPTY, cCol, rgColNumber))
*/
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);

	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(	m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds, PRIMARY),S_OK);
		
	for (i=0;i<m_cBinding;i++)
	{
		//force SetData to use the read only (FillBindings will ignore read only cols)
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[i].obStatus) = DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[i],pData)=DBSTATUS_S_OK;
	}

	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);
	
	if (hr==DB_E_ERRORSOCCURRED)
	{
		//this is acceptable but it is not what is being tested
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC_(DB_S_ERRORSOCCURRED,hr);

	//allocate memory for the row
	if(!(pGetData=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			
	PROVIDER_FREE(pHRow);
	
	for (i=0;i<cNotUpdateCol;i++)
	{
		for (j=0;j<m_cBinding;j++)
		{
			//if this is a read only column that was bound
			if (rgNotUpdateColNumber[i]==m_rgBinding[j].iOrdinal)
			{
				//don't check timestamp columns
				if (m_rgInfo[j].dwFlags & DBCOLUMNFLAGS_ISROWVER)
				{
					continue;
				}
				//check the failed status
//				if (!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[j].obStatus), DBSTATUS_E_PERMISSIONDENIED))
				if(!COMPARE(DBSTATUS_E_PERMISSIONDENIED,STATUS_BINDING(m_rgBinding[j],pData)))
				{
					goto CLEANUP;
				}
				
				//make sure the GetData for the read only column doesn't have the value that failed in SetData
//				if (	*(BYTE *)((DWORD)pGetData+m_rgBinding[j].obValue) == 
//						*(BYTE *)((DWORD)pData+m_rgBinding[j].obValue)
				if (	&VALUE_BINDING(m_rgBinding[j], pGetData) == 
						&VALUE_BINDING(m_rgBinding[j], pData)
					)
				{
					goto CLEANUP;
				}
				//GetData for the read only column should be the same as from the first GetData call
//				if (!COMPARE(	*(BYTE *)((DWORD)pGetData+m_rgBinding[j].obValue), 
//								*(BYTE *)((DWORD)m_pData+m_rgBinding[j].obValue)
				if (!COMPARE(	&VALUE_BINDING(m_rgBinding[j], pGetData), 
								&VALUE_BINDING(m_rgBinding[j], m_pData)
							))
				{
					goto CLEANUP;
				}
			}
		}
	}
	fTestPass=TEST_PASS;
CLEANUP:
	CoTaskMemFree(pData); pData = NULL; //PROVIDER_FREE(pData);
	PROVIDER_FREE(pGetData);	
	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(rgNotUpdateColNumber);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

	
// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc set no data in a BLOB column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_29()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM	cRows				= 0;
	HROW		*pHRow				= NULL;
	void		*pData				= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cColNum				= 0;
	DBORDINAL	*rgColNumberNum		= NULL;
	DBORDINAL	cColBLOB			= 0;
	DBORDINAL	*rgColNumberBLOB	= NULL;
	DBORDINAL	rgColNumber[2]		= {0,0};


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset so the test can pick not nullable updatable cols
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}

	//get an array of numeric and updatable columns
	GetImage(&cColBLOB,&rgColNumberBLOB);
	//has to find such a column
	if(!cColBLOB)
	{
		goto CLEANUP;
	}

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cColNum,&rgColNumberNum);
	//has to find such a column
	if(!cColNum)
	{
		goto CLEANUP;
	}
	
	rgColNumber[0]=rgColNumberNum[0];
	rgColNumber[1]=rgColNumberBLOB[0];
	
	fTestPass=FALSE;
	
	//Create a new accessor that binds a fixed length and a BLOB
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,2,rgColNumber))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data a row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);


	//make sure and put the empty string in the BLOB
	if ( IsNumericType(m_rgBinding[0].wType))
	{
		//fill the BLOB buffer with empty string
//		*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)= 0;
		LENGTH_BINDING(m_rgBinding[1],pData)=0;
	}
	else
	{
		//fill the BLOB buffer with empty string
//		*(ULONG *)((DWORD)pData+m_rgBinding[0].obLength)= 0;
		LENGTH_BINDING(m_rgBinding[0],pData)=0;
	}

	//set data, should pass
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//Get the data for a row handle
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	//set the data again, should pass
	TESTC_(SetData(*pHRow,m_hAccessor,m_pData),S_OK);

	fTestPass=TRUE;
CLEANUP:
	if(m_pIRowsetChange&&pHRow)
	{
		//delete the row so it doesn't cause problems later for providers that fail this variation
		CHECK(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);
	}
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumberBLOB);
	PROVIDER_FREE(rgColNumberNum);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc InsertRows and DBPROP_MAX
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_30()
{
	BYTE		*pData				= NULL;
	DBCOUNTITEM	cRows				= 0;
	HROW		*pHRow				= NULL;
	ULONG		cRefCount			= 0;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass			= TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	//sets the prop DBPROP_MAXROWS to 1 for the rowset
	g_fUseMaxRows	= TRUE;

	HRESULT hr = S_OK;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass	= TEST_FAIL;

	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//try to get 2 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),DB_S_ROWLIMITEXCEEDED);

	if (1==cRows)
	{
		fTestPass	= TEST_PASS;
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
		
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc insert no BLOB, change BLOB
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor::Variation_31()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM	cRows				= 0;
//	HROW		*pHRow				= NULL;
	void		*pData				= NULL;
	DBPROPID	rgPropertyIDs[3];
	DBORDINAL	cColNum				= 0;
	DBORDINAL	*rgColNumberNum		= NULL;
	DBORDINAL	cColBLOB			= 0;
	DBORDINAL	*rgColNumberBLOB	= NULL;
	DBORDINAL	rgColNumber[2]		= {0,0};
	HROW		HRowNew				= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CHANGEINSERTEDROWS;
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset so the test can pick not nullable updatable cols
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}

	//amke data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowNew),S_OK);
	PROVIDER_FREE(pData);
		
	//get an array of BLOBs
	GetImage(&cColBLOB,&rgColNumberBLOB);
	//has to find such a BLOB column
	if(!cColBLOB)
	{
		goto CLEANUP;
	}

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cColNum,&rgColNumberNum);
	//has to find such a column
	if(!cColNum)
	{
		goto CLEANUP;
	}
	
	rgColNumber[0]=rgColNumberNum[0];
	rgColNumber[1]=rgColNumberBLOB[0];
	
	fTestPass=FALSE;
	
	//Create a new accessor that now binds a fixed length and a BLOB
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,2,rgColNumber))

	//get a new data buffer to set the data a row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//set data, should pass
	TESTC_(SetData(HRowNew,m_hAccessor,pData),S_OK);
		
	//Get the data for a row handle
	TESTC_(GetData(HRowNew,m_hAccessor,m_pData),S_OK);
		
	//make sure GetData should be able to see the change
	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)		
	{
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumberBLOB);
	PROVIDER_FREE(rgColNumberNum);
	if(HRowNew && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,&HRowNew,NULL,NULL,NULL),S_OK);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Cursor::Terminate()
{
	return(TCIRowsetChange::Terminate());
}

// {{ TCW_TC_PROTOTYPE(QueryBaseUpdates)
//*-----------------------------------------------------------------------
//| Test Case:		QueryBaseUpdates - Test QueryBaseUpdates that update more than one rows.
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL QueryBaseUpdates::Init()
{
	BOOL fPass = FALSE;

	if(!TCIRowsetChange::Init())
		return FALSE;

	fPass = TRUE;

	return fPass;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode. Update one non-key column with values that affect more than one rows.  Verify the change is visible
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_1()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows		= 0;
	DBORDINAL	cCol		= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	HRESULT		hr			= S_OK;
	DBCOUNTITEM	ulCol		= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;

	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//get the a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data a row.  Set
	//the data to be the same as the another row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,(g_ulNextRow-1),m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data. Change the  row to the same as the other handle's data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//release the memory
	PROVIDER_FREE(pData);
	
	//get a new data buffer with new data.  
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//Set the data to be the same row handle, should have another row that has matching data in this column
	//some provider MIGHT do query based upadtes which would affect both rows
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
				
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;
	
	//retrieve the row handles
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		
		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change, twice
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)		
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	PROVIDER_FREE(rgColNumber);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(m_pData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode.  Update one non-key column with values that affect more than one rows.  Verify the change is visible the
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_2()
{
	DBPROPID	rgPropertyIDs[2];
	DBCOUNTITEM	cRows		= 0;
	DBORDINAL	cCol		= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	HRESULT		hr			= S_OK;
	DBCOUNTITEM	ulCol		= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	//create an accessor with IRowsetChange. Buffered update mode.
	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass = FALSE;
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
		goto CLEANUP;

	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
		ulCol=cCol-1;
	else
		ulCol=3;

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
									FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//get a new data buffer to set the data a row.  Set
	//the data to be the same as the another row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data. Change the  row to the same as the other handle's data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//release the memory
	PROVIDER_FREE(pData);
	
	//update 
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
	//Set the data to be the same row handle, should have another row that has matching data in this column
	//some provider MIGHT do query based upadtes which would affect both rows
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,&pData,g_ulNextRow++,
								m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data. Change the  row to the same as the other handle's data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//update 
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;
	
	//retrieve a row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		
		//Get the data for a row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(m_pData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary checks from update pending
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_3()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	HROW			*pHRowNew		= NULL;
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[2]={NULL,NULL};
	ULONG			uRefCount1		= 0;
	ULONG			uRefCount2		= 0;
	DBROWSTATUS		DBRowStatus[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	TESTC_PROVIDER(	GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,2,g_rgPropertyIDs,
					ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
					DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
			
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRowDelete),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,DBRowStatus),E_INVALIDARG);
			
	//no op
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,pHRowDelete,DBRowStatus),S_OK);
			
	//no op
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,NULL,DBRowStatus),S_OK);
			
	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,2,pHRowDelete,DBRowStatus),S_OK);
	//delete the rows again
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRowDelete,DBRowStatus),DB_S_ERRORSOCCURRED);
	COMPARE(DBRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPARE(DBRowStatus[1], DBROWSTATUS_E_DELETED);
	COMPARE(DBRowStatus[2], DBROWSTATUS_S_OK);

	fTestPass	= TEST_PASS;	
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowDelete);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4) 
//*-----------------------------------------------------------------------
// @mfunc NEWLYINSERTED from update pending
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_4()
{
	BOOL			fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM		cRows				= 0;
	HROW			HRowNew				= NULL;
	void			*pData				= NULL;
	DBPROPID		rgPropertyIDs[2]	= {NULL,NULL};
	DBPROPID		rgUnPropertyIDs[3]	= {NULL,NULL,NULL};
	DBROWSTATUS		DBRowStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgUnPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgUnPropertyIDs[2]=DBPROP_CHANGEINSERTEDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	TESTC_PROVIDER(	GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,3,rgUnPropertyIDs,
					ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
					DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowNew),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowNew,&DBRowStatus),DB_E_ERRORSOCCURRED);
			
	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_NEWLYINSERTED))
	{
		goto CLEANUP;
	}
	fTestPass=TRUE;
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}

	if(&HRowNew && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowNew, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5) 
//*-----------------------------------------------------------------------
// @mfunc key non-key cols, set 2 rows the same and delete
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_5()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	void			*pData			= NULL;
	void			*pData1			= NULL;
	DBPROPID		rgPropertyIDs[2]= {NULL,NULL};
	DBROWSTATUS		DBRowStatus;
	DBORDINAL		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBORDINAL		ulCol			= 0;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	//create an accessor with IRowsetChange. Buffered update mode.
	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_UPDATEABLE,2,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;

	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
		ulCol=cCol-1;
	else
		ulCol=3;

	//get 2 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(	ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//get a new data buffer to set the data a row.  Set
	//the data to be the same as the another row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data. Change the  row to the same as the other handle's data
	TESTC_(SetData(pHRow[0],m_hAccessor,pData),S_OK);
	TESTC_(SetData(pHRow[1],m_hAccessor,pData),S_OK);

	//delete the row, first one only
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&pHRow[0],&DBRowStatus),S_OK);

	if(!COMPARE(DBRowStatus, DBROWSTATUS_S_OK))
	{
		goto CLEANUP;
	}

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		if (DB_E_ROWSNOTRELEASED==hr)
		{
			//if CANHOLDROWS is TRUE and RestartPosition returns DB_E_ROWSNOTRELEASED
			//then check DBPROP_QUICKRESTART.  
			//if DBPROP_QUICKRESTART is FALSE (command re-executed) this could be valid 
			//but if DBPROP_QUICKRESTART is TRUE DB_E_ROWSNOTRELEASED should not be 
			//returned from RestartPosition if CANHOLDROWS is requested
			if(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, (IUnknown*)m_pIRowset))		
			{
				goto CLEANUP;
			}
			else
			{
				//if here then DBPROP_QUICKRESTART is TRUE 
				fTestPass	 = TEST_SKIPPED;
				goto CLEANUP;
			}
		}
	}

	//get 2 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);

	 //allocate memory for GetData
	if(!(pData1=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//get the first 2 rows

	//row 1
	hr=m_pIRowset->GetData(pHRow[0],m_hAccessor,pData1);
	
	//DBPROP_REMOVEDELETEDROWS maybe false
	if(DB_E_DELETEDROW==hr)
	{
		TESTC_(m_pIRowset->GetData(pHRow[1],m_hAccessor,pData1),S_OK);
		//second row here (pData1) should  match the changed data
		if(!CompareBuffer(pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
	}
	//DBPROP_REMOVEDELETEDROWS maybe true
	if(S_OK==hr)
	{
		//the deleted row was removed so the first row fetch should now be the seond changed row
		if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->GetData(pHRow[1],m_hAccessor,pData1),S_OK);
		//second row here (pData1) should  now not match the changed data
		if(CompareBuffer(pData1,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Delete rows from update pending, delete not supported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_6()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	DBPROPID		rgPropertyIDs[1]= {NULL};
	DBROWSTATUS		DBRowStatus;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRowDelete),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRowDelete,&DBRowStatus),DB_E_NOTSUPPORTED);
			
	fTestPass=TRUE;
CLEANUP:
	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowDelete);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc delete with a row handle from an unallocated rowset
//	
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_7()
{
	HROW			HRow			= 27;
	DBPROPID		rgPropertyIDs[1];
	BOOL			fTestPass		= TEST_SKIPPED;
	DBROWSTATUS		rgDBRowStatus[1];
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange || !m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRow),S_OK);

	//delete the row using different row handle
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRow,rgDBRowStatus),DB_E_ERRORSOCCURRED);

	TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID));

	fTestPass	= TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc delete with a row handle from an allocated rowset
//	
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_8()
{
	HROW			HRow			= 27;
	DBPROPID		rgPropertyIDs[1];
	BOOL			fTestPass		= TEST_SKIPPED;
	DBROWSTATUS		rgDBRowStatus[1];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange || !m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//delete the row using different row handle
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRow,rgDBRowStatus),DB_E_ERRORSOCCURRED);

	TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID));

	fTestPass	= TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary cases with update pending
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_9()
{
	DBPROPID	rgPropertyIDs[2];
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,NULL),E_INVALIDARG);		
	TESTC_(SetData(*pHRow,NULL,pData),DB_E_BADACCESSORHANDLE);		
	TESTC_(SetData(NULL,m_hAccessor,pData),DB_E_BADROWHANDLE);		
	TESTC_(SetData(*pHRow,hAccessor,NULL),S_OK);		
		
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE, qbu
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_10()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	void		*pGetData		= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBSTATUS	dbsSecondColStatus;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	DBORDINAL	ulCol			= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	 
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	fTestPass=FALSE;

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the second status binding to anything other than _OK or NULL
//	*(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus)=DBSTATUS_S_TRUNCATED;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_TRUNCATED;
	
	// keep track of colstatus
//	dbsSecondColStatus = *(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus);
	dbsSecondColStatus = STATUS_BINDING(m_rgBinding[0],pData);

	//set data should fail
	m_hr=SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE) )
		goto CLEANUP;
	
	TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_BADSTATUS);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRow);

	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),dbsSecondColStatus);
		}
	}
	else
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_UNAVAILABLE);
		}
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
		{
			goto CLEANUP;
		}
		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//columns should not be changed
		if(!COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;
CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc SetData not supported, qbu
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_11()
{
	DBPROPID	rgPropertyIDs[2];
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_NOTSUPPORTED);		
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc QBU with just floats
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryBaseUpdates::Variation_12()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	DBPROPID		rgPropertyIDs[2]= {NULL,NULL};
	DBORDINAL		*rgColsToBind	= NULL;
	DBORDINAL		cColsNumber		= 0;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
	{
		m_ulpUpdFlags = m_ulpProvUpdFlags;
	}

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=KAGPROP_INCLUDENONEXACT;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}

	//Get a variable length column.  .
	if(!GetFloatAndUpdatable(&cColsNumber, &rgColsToBind))
	{
		odtLog << "no float updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=KAGPROP_INCLUDENONEXACT;

	//get an accessor with just the floats
	GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		cColsNumber, rgColsToBind);
		
	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK);

	//get a new data buffer to set the data.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&m_pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(rgColsToBind);

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

BOOL QueryBaseUpdates::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Static_Cursor_Immediate - Static cursor in immediate update mode.
//|					not visible - OTHERUPDATEDELETE is VARIANT_FALSE
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Immediate::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
	{
		return FALSE;
	}
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,NO_ACCESSOR));

	//for static cursor
	COMPARE(BufferedUpdate(),FALSE);
		
	fTestPass=TRUE;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Update a row to be the first row in the row set with some columns set to NULL, and call GetData to see.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Immediate::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	HROW		*pHRow			= NULL;
	ULONG		cCount			= 0;
	BYTE		*pDataNoFetch	= NULL;
	BYTE		*pData			= NULL;
	BYTE		*pBackEndData	= NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass		= TEST_SKIPPED;
	HRESULT		hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
			
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	//get an array of nullable and updatable columns
	
	(&cCol,&rgColNumber);
	if(!cCol)
		goto CLEANUP;

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	//get the row handle
	TESTC_(m_hr=m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//get a new data buffer to set the data.  The data is for first row.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set the first nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[0])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}
		}
	}

	//has to find such a column
	if(cCount==m_cBinding)
		goto CLEANUP;

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//allocate another buffer
	pBackEndData=(BYTE *)PROVIDER_ALLOC(m_cRowSize);

	if(!pBackEndData)
		goto CLEANUP;
	
	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
	
	//refetch the row handles
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if(cRows == 0)
			break;

		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change, DBPROP_OWNUPDATEDELETE is FALSE
		//but query is re executed
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
		if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			odtLog << "Error - this row should not be visible"<<ENDL;
			break;
		}
	}
CLEANUP:
	//free the memory used by pBackEndData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pBackEndData,TRUE);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Immediate::Variation_2()
{ 	
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	HROW		*pHRowDuplicate	= NULL;
	DBPROPID	rgPropertyIDs[6];
	BOOL		fTestPass		= TEST_SKIPPED;
	HRESULT		hrRP			= S_OK;
	HRESULT		hr				= S_OK;
	BYTE		*pBackEndData	= NULL;
	BYTE		*pDataNoFetch	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[5]=DBPROP_IRowsetLocate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,6,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//retrieve the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//move the cursor away
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowDuplicate),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	//retrieve the row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)m_ulTableRows +1,-1,&cRows,&pHRowDuplicate),S_OK);

	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRow), (DWORD)(*pHRowDuplicate));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRow,pHRowDuplicate))
		{
			goto CLEANUP;
		}
	}

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
			
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pDataNoFetch),S_OK);

	//the change is only gaurenteed to be visible if LITERALIDENTITY is TRUE
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		//make sure GetData should be able to see the change, row has not been refetched
		if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	//restart position
	hrRP=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//if the command was re-executed the change will be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//make sure GetData should be able to see the change - command was re-executed
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//make sure GetData shouldn't be able to see the change (it should see old data)
			//OWNUPADTEDELETE is off
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowDuplicate && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBackEndData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Immediate::Variation_3()
{
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	HROW		*pHRowDuplicate	= NULL;
	DBPROPID	rgPropertyIDs[5];
	BOOL		fTestPass		= TEST_SKIPPED;
	BYTE		*pDataNoFetch	= NULL;
	BYTE		*pBackEndData	= NULL;
	HRESULT		hrRP			= S_OK;
	HRESULT		hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;	 
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
		
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//retrieve the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//move the cursor away 
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowDuplicate),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);	
	PROVIDER_FREE(pHRowDuplicate);
	
	//retrieve the  row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)(m_ulTableRows)+1,-1,&cRows,&pHRowDuplicate),S_OK);
		
    //the two row handles should have the same value
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRow), (DWORD)(*pHRowDuplicate));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRow,pHRowDuplicate))
		{
			goto CLEANUP;
		}
	}

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	//make changes to the row.  Make its value as the 3th row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pDataNoFetch),S_OK);

	//the change is only gaurenteed to be visible if LITERALIDENTITY is TRUE
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		//make sure GetData should be able to see the change, row has not been refetched
		if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);

	//restart position
	hrRP=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowDuplicate)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
			break;

		//Get the data for the ith row handle
		TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowDuplicate);

		//if the command was re-executed the change will be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//make sure GetData should be able to see the change - command was re-executed
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//make sure GetData shouldn't be able to see the change (it should see old data)
			//OWNUPADTEDELETE is off
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowDuplicate && m_pIRowset)
		m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL);
	PROVIDER_FREE(pHRowDuplicate);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change
//		One rowset 	changes a key value.  The other rowset should no
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Immediate::Variation_4()
{
	HROW			*pHRow			= NULL;
	HROW			*pHRowSecond	= NULL;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	void			*pData			= NULL;
	void			*pDataSecond	= NULL;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	ICommand		*pICommand		= NULL;
	IUnknown        *pIRowset		= NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass		= TEST_SKIPPED;
	BYTE			*pDataNoFetch	= NULL;
	HRESULT			hrRP			= S_OK;
	HRESULT			hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_LITERALIDENTITY;//DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	
	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	//release the row set
	ReleaseRowsetAndAccessor();

	if ((!m_pIDBCreateCommand) || (!m_pIRowsetChange))
		goto CLEANUP;

	fTestPass = FALSE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	//get the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//get a copy of the data on  row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//create a new data buffer to set the data for ther numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object//QI for IRowsetInfo pointer
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
	
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);

	//get data on the first rowset.  
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, row has not been refetched
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//alloc new memory
	if(!(pDataSecond=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//restart position on a completly different rowset
	hrRP=((IRowset *)(pIRowset))->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	while (S_OK==(hr = ((IRowset *)(pIRowset))->GetNextRows(NULL,0,1,&cRows, &pHRowSecond)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
			break;

		//Get the data for the ith row handle
		TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		TESTC_(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);

		//if the command was re-executed the change will be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//the second rowset should see the change if rexeccuted
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//the second rowset should NOT see the change (it should see old data)
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pDataSecond);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

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
BOOL Static_Cursor_Immediate::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Static_Cursor_Buffered - Static_Cursor_Buffered
//|					not visible - OTHERUPDATEDELETE is VARIANT_FALSE
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Buffered::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass			=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,2,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,NO_ACCESSOR));
	
	//for static cursor
	COMPARE(BufferedUpdate(),TRUE);

	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	HROW		*pHRow			= NULL;
	ULONG		cCount			= 0;
	BYTE		*pData			= NULL;
	BYTE		*pBackEndData	= NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass		= TEST_SKIPPED;
	HRESULT		hr				= S_OK;
	BYTE		*pDataNoFetch	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}

    fTestPass = FALSE;
	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//get a new data buffer to set the data.  The data is for first row.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set a first nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cCol-1])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}
		}
	}

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//has to find such a column
	if(cCount==m_cBinding)
		goto CLEANUP;

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
	
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//allocate another buffer
	pBackEndData=(BYTE *)PROVIDER_ALLOC(m_cRowSize);

	if(!pBackEndData)
		goto CLEANUP;
	
	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))

	//retrieve the row handles
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData shouldbe able to see the change, OTHERUPDATEDELETE is FALSE
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
		if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			odtLog << "Error - this row should not be visible"<<ENDL;
			break;
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_2()
{
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	HROW		*pHRowDuplicate	= NULL;
	DBPROPID	rgPropertyIDs[5];
	BOOL		fTestPass		= TEST_SKIPPED;
	HRESULT		hr				= S_OK;
	BYTE		*pBackEndData	= NULL;
	BYTE		*pDataNoFetch	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;
	//retrieve the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//move the cursor away to the row
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowDuplicate),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	//retrieve the row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)(m_ulTableRows)+1,-1,&cRows,&pHRowDuplicate),S_OK);
		
    //the two row handles should have the same value
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRow), (DWORD)(*pHRowDuplicate));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRow,pHRowDuplicate))
		{
			goto CLEANUP;
		}
	}

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, no fetch yet
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//free the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
		
	//release the rowset
	ReleaseRowsetAndAccessor();
	
	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))
	
	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//retrieve the row handles
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should  be able to see the change - query was re-exed
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
		if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			odtLog << "Error - this row should not be visible"<<ENDL;
			break;
		}
	}	
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_3()
{
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	HROW		*pHRowDuplicate	= NULL;
	BYTE		*pBackEndData	= NULL;
	DBPROPID	rgPropertyIDs[5];
	BOOL		fTestPass		= TEST_SKIPPED;
	BYTE		*pDataNoFetch	= NULL;
	HRESULT		hrRP			= S_OK;
	HRESULT		hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;
	//retrieve the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//move the cursor away to the 1 row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowDuplicate),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	//retrieve the row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)m_ulTableRows+1,-1,&cRows,&pHRowDuplicate),S_OK);
	
    //the two row handles should have the same value
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRow), (DWORD)(*pHRowDuplicate));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRow,pHRowDuplicate))
			goto CLEANUP;
	}

	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//make changes to the  row.  Make its value as the 3th row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, row has not been refetched
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//restart position
	hrRP=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowDuplicate)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
		{	
			break;
		}
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRowDuplicate,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowDuplicate);

		//if the command was re-executed the change will be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//make sure GetData should be able to see the change - command was re-executed
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//make sure GetData shouldn't be able to see the change
			//OWNUPADTEDELETE is off
			if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowDuplicate && m_pIRowset)
		m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL);
	PROVIDER_FREE(pHRowDuplicate);
	
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset changes a value.  The other rowset should not see change
//		One rowset 	changes a key value.  The other rowset should no
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_4()
{
	HROW			*pHRow			= NULL;
	HROW			*pHRowSecond	= NULL;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	void			*pData			= NULL;
	void			*pDataSecond	= NULL;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	ICommand		*pICommand		= NULL;
	IUnknown        *pIRowset		= NULL;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass		= TEST_SKIPPED;
	BYTE			*pDataNoFetch	= NULL;
	HRESULT			hrRP			= S_OK;
	HRESULT			hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetUpdate;	

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,NO_ACCESSOR));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;
	
	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
	{
		goto CLEANUP;
	}

	//release the row set
	ReleaseRowsetAndAccessor();

	if(!m_pIDBCreateCommand)
		return TEST_PASS;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,g_cPropertyIDs,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	//get the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//get a copy of the data on  row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object//QI for IRowsetInfo pointer
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//execute the same command object to get a DIFFERENT ROWSET POINTER
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
	

	//set data on the FIRST rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);

	//update the FIRST rowset
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//get data on the FIRST rowset
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, row has not been refetched
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//Release the 1st row handle (test is now holding no row handles)
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//alloc new memory	
	if(!(pDataSecond=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//restart position of SECOND (OTHER) rowset
	hrRP=((IRowset *)(pIRowset))->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	//Fetch through SECOND rowset from a different rowset pointer
	while (S_OK==(hr = ((IRowset *)(pIRowset))->GetNextRows(NULL,0,1,&cRows, &pHRowSecond)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
		{	
			break;
		}
		//Get the data for the ith row handle (2nd rowset from 2nd rowset pointer)
		TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		TESTC_(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);

		//if the command was re-executed the change will not be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//the second rowset should  see the change if reexecuted, it should see the old row
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//the second rowset should NOT see the change, it should see the old row
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	pIRowset->Release();;
	//SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

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
BOOL Static_Cursor_Buffered::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Keyset_Cursor_Buffered - Keyset_Cursor_Buffered
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Buffered::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_1()
{	
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	HROW		*pHRowDuplicate	= NULL;
	ULONG		cRefCount;
	DBPROPID	rgPropertyIDs[6];
	BOOL		fTestPass		= TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[5]=DBPROP_IRowsetIdentity;

	HRESULT hr = S_OK;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//Identity is level 0 and required
	if (!m_pIRowsetIdentity)
	{
		goto CLEANUP;
	}
	//retrieve the ith row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowSecond),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowSecond);
	
	//retrieve the ith row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)m_ulTableRows+1,-1,&cRows,&pHRowDuplicate),S_OK);
	
    //the two row handles should have the same value
	TESTC_(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowDuplicate),S_OK);
		
	//make changes to the 5th row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
			
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//free the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL, &cRefCount, NULL),S_OK);
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		hr = GetData(*pHRow,m_hAccessor,m_pData);
		//Taking cares of SQL Providers for keyset cursor
		TESTC(hr==S_OK || hr == DB_E_DELETEDROW);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	

	PROVIDER_FREE(pHRowDuplicate);
	
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_2()
{
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	cCol			=0;
	DBORDINAL	*rgColNumber	=NULL;
	HROW		*pHRow			=NULL;
	DBCOUNTITEM	cCount;
	ULONG		cColNull		=0;
	BYTE		*pData			=NULL;
	BYTE		*pBackEndData	=NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass		=TEST_SKIPPED;
	HRESULT		hr				=S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;

	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK); 
		
	//get a new data buffer to set the data.  The data is for first row.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	
	//set a nullable and updatble columns to NULL
	while(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cCol-1])
			{
//				*(DBSTATUS *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				cColNull++;
				cCount=m_cBinding;
			}
		}
		cCol--;
	}

	//has to find such a column 
	if(!cColNull)
	{
		fTestPass = TRUE;
		goto CLEANUP;
	}

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
	
	
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//allocate another buffer
	pBackEndData=(BYTE *)PROVIDER_ALLOC(m_cRowSize);

	if(!pBackEndData)
		goto CLEANUP;
	
	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by m_pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pBackEndData,TRUE);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_3()
{
	BYTE		*pData			=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			=NULL;
	HROW		*pHRowSecond	=NULL;
	HROW		*pHRowDuplicate	=NULL;
	DBPROPID	rgPropertyIDs[6];
	ULONG		cRefCount		=0;
	BOOL		fTestPass		=TEST_SKIPPED;
	HRESULT		hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
 	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[5]=DBPROP_IRowsetIdentity;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}

	fTestPass=FALSE;

	//Identity is level 0 and required
	if (!m_pIRowsetIdentity)
	{
		goto CLEANUP;
	}
	//retrieve the ith row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-1,1,&cRows,&pHRow),S_OK);
		
	//move the cursor away to the 1 row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)(m_ulTableRows)+1,-1,&cRows,&pHRowSecond),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);	
	PROVIDER_FREE(pHRowSecond);

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		if (DB_E_ROWSNOTRELEASED==hr)
		{
			//if CANHOLDROWS is TRUE and RestartPosition returns DB_E_ROWSNOTRELEASED
			//then check DBPROP_QUICKRESTART.  
			//if DBPROP_QUICKRESTART is FALSE (command re-executed) this could be valid 
			//but if DBPROP_QUICKRESTART is TRUE DB_E_ROWSNOTRELEASED should not be 
			//returned from RestartPosition if CANHOLDROWS is requested
			if(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, (IUnknown*)m_pIRowset))		
			{
				goto CLEANUP;
			}
			else
			{
				//if here then DBPROP_QUICKRESTART is TRUE 
				fTestPass	 = TEST_SKIPPED;
				goto CLEANUP;
			}
		}
	}

	//retrieve the ith row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,-1,&cRows,&pHRowDuplicate),S_OK);

    //the two row handles should have the same value
	TESTC_(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowDuplicate),S_OK);

	//make changes to the 20th row.  Make its value as the 3th row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	if(pHRowDuplicate && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	PROVIDER_FREE(pData);
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
BOOL Keyset_Cursor_Buffered::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Remove_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Keyset_Remove_Cursor_Immediate - Keyset_Remove_Curosr_Buffered
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Remove_Cursor_Immediate::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass	=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_1()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow				=NULL;
	HROW		*pHRowSecond		=NULL;
	ULONG		cRefCount			=0;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass			=TEST_SKIPPED;
	IRowsetInfo	*pIRowsetInfo	= NULL;
	ULONG		cProperty		= 0;
	DBPROPIDSET	DBPropIDSet;
	DBPROPID	DBPropID		= DBPROP_REMOVEDELETED;
	DBPROPSET	*pDBPropSet		= NULL;
	BOOL		fRemoveDeleteRows;


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	HRESULT hr = S_OK;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;
	if(!VerifyInterface(m_pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
	{
		goto CLEANUP;
	}
	//get default value for REMOVEDELTED
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);
	fRemoveDeleteRows = ((*((*pDBPropSet).rgProperties)).vValue).boolVal;

	//retrieve the ith row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);

	//make changes to the ith row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
		

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pData);

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//do the first row first out of the loop
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK);
	//Get the data for the 1st row handle
	if (fRemoveDeleteRows)
	{
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	}
	else
	{
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),DB_E_DELETEDROW);
	}
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	PROVIDER_FREE(m_pData);

	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_CHANGEINSERTEDROWS is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_2()
{
	BYTE		*pDataA		= NULL;
	BYTE		*pDataB		= NULL;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass	= TEST_SKIPPED;
	HROW		HRowX		= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_CHANGEINSERTEDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;
	
	fTestPass=FALSE;

	//make data buffers
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pDataA,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pDataB,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pDataA,&HRowX),S_OK);

	//set data, 
	//DBPROP_CHANGEINSERTEDROWS is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED
	TESTC_(SetData(HRowX,m_hAccessor,pDataB),S_OK);
	
	fTestPass=TRUE;	 

	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pDataA,TRUE);
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pDataB,TRUE);
CLEANUP:
	if(HRowX)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowX,NULL,NULL,NULL),S_OK);
		HRowX	= NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_SERVERDATAONINSERT is VARIANT_TRUE,	should not see DB_E_NEWLYINSERTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_3()
{
	BYTE		*pDataX		= NULL;
	BYTE		*pDataY		= NULL;
	BYTE		*pDataXX	= NULL;
	DBPROPID	rgPropertyIDs[4];
	BOOL		fTestPass	= TEST_SKIPPED;
	HROW		HRowX		= NULL;
	HROW		HRowY		= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[3]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_SERVERDATAONINSERT;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;

	//create buffers
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pDataX,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pDataY,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pDataXX,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pDataX,&HRowX),S_OK);

	//insert a second row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pDataY,&HRowY),S_OK);

	//set data on the first inserted row
	//should now see this row cause SERVERDATAONINSERT is TRUE
	TESTC_(SetData(HRowX,m_hAccessor,pDataXX),S_OK);

	fTestPass = TEST_PASS;
CLEANUP:		
	if(HRowY)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowY,NULL,NULL,NULL),S_OK);
		HRowY	= NULL;
	}
	if(HRowX)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowX,NULL,NULL,NULL),S_OK);
		HRowX	= NULL;
	}
	PROVIDER_FREE(pDataX);
	PROVIDER_FREE(pDataY);
	PROVIDER_FREE(pDataXX);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_4()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows				= 0;
	HROW		*pHRow				=NULL;
	HROW		*pHRowSecond		=NULL;
	ULONG		cRefCount			=0;
	DBPROPID	rgPropertyIDs[2];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	HRESULT hr = S_OK;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);

	//make changes to the row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
		

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pData);

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//should be delete and here because DBPROP_REMOVEDELETED is variant_false
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),DB_E_DELETEDROW);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
    //Since GetData is expected to fail we do not need to release this memory
    //ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc fetch newly deleted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_5()
{
	DBPROPID	rgPropertyIDs[3];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass	= TEST_SKIPPED;
	HROW		*pHRow		=NULL;
	HROW		*pHRowSecond=NULL;
	DBCOUNTITEM	cRows			= 0;
	HRESULT		hr			=S_OK;
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	
	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//release the rows
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//get the data for the row, return DB_E_DELETEDROW
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),DB_E_DELETEDROW);

	fTestPass = TEST_PASS;
CLEANUP:		
	//release the row handle
	if (m_pIRowset)
	{
		//release the row handle
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);	
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Insert a row and check to see it (OWNINSERT is off)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_6()
{
	BYTE		*pData				=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow				=NULL;
	DBPROPID	rgPropertyIDs[4];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;
	HRESULT		hr					= S_OK;
	HRESULT		RPhr				= S_OK;
	BOOL		fFound				= FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_LITERALIDENTITY;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]=DBPROP_OWNINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//get a new data buffer to set the data a row.  
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);

	//restart
	RPhr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(RPhr==S_OK || RPhr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//loop through rowset searching for the inserted row
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fFound	= TRUE;
		}
	}
	//OWNINSERT is FALSE, if the test sees the row, FAIL
	//only if the command was not re-ececuted because if it was re-exectued the rowset is new
	//and the props do not apply so see the row
	if (RPhr==DB_S_COMMANDREEXECUTED)
	{
		if(fFound)
		{
			fTestPass	= TRUE;
		}
	}
	else
	{
		if(!fFound)
		{
			fTestPass	= TRUE;
		}
	}
CLEANUP:
	//free the memory used by pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(m_pData);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary cases cursor.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_7()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;
	DBORDINAL	cCol		= 0;
	DBORDINAL	ulCol		= 0;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}

	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,NULL),E_INVALIDARG);		
	TESTC_(SetData(*pHRow,NULL,pData),DB_E_BADACCESSORHANDLE);		
	TESTC_(SetData(NULL,m_hAccessor,pData),DB_E_BADROWHANDLE);		
	TESTC_(SetData(*pHRow,hAccessor,NULL),S_OK);		
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc BSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_8()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	void		*pGetData		= NULL;
	DBPROPID	rgPropertyIDs[1];
	DBSTATUS	dbsSecondColStatus;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	DBORDINAL	ulCol			= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	 
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}
	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
	{
		ulCol=cCol-1;
	}
	else
	{
		ulCol=3;
	}
	fTestPass=FALSE;

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the second status binding to anything other than _OK or NULL
//	*(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus)=DBSTATUS_S_TRUNCATED;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_TRUNCATED;
	
	// keep track of colstatus
//	dbsSecondColStatus = *(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus);
	dbsSecondColStatus = STATUS_BINDING(m_rgBinding[0],pData);

	//set data should fail
	m_hr=SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE) )
		goto CLEANUP;
	
	TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_BADSTATUS);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRow);

	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),dbsSecondColStatus);
		}
	}
	else
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_UNAVAILABLE);
		}
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
			goto CLEANUP;

		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//columns should not be changed
		if(!COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;

CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc SetData not supported, 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_9()
{
	DBPROPID	rgPropertyIDs[1];
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	*rgColNumber= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	BYTE		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));
		
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;

	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								(BYTE **)&pData,
								g_ulNextRow++,
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_NOTSUPPORTED);		
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Fetch after a deleted row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Cursor_Immediate::Variation_10()
{
	DBPROPID	rgPropertyIDs[5];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass		= TEST_SKIPPED;
	HROW		*pHRow2			= NULL;
	HROW		*pHRow3			= NULL;
	BYTE		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	HRESULT		hr;


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[3]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetLocate;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,1,rgUnPropertyIDs,
										ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;
	
	fTestPass=FALSE;

	//get the 3rd row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow3),S_OK);
	//get the data for the 3rd row
	TESTC_(GetData(*pHRow3,m_hAccessor,m_pData),S_OK);
	//get the row just before the one deleted
	TESTC_(m_pIRowset->GetNextRows(NULL,-2,1,&cRows,&pHRow2),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow2, NULL),S_OK);
		
	//try to retrieve the row handle again.  For keyset cursor, the change should
	//be in place or at the end of the rowset
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow3),S_OK);
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow3),S_OK);
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow3),S_OK);

	//allocation memory
	pData=(BYTE *)PROVIDER_ALLOC(m_cRowSize);

	//get the data for the 3rd row
	TESTC_(GetData(*pHRow3,m_hAccessor,pData),S_OK);
	
	//make sure GetData see the third row
	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		fTestPass=TRUE;
	}
CLEANUP:
	if(pHRow2 && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow2,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow2);
	
	if(pHRow3 && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow3,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow3);

	PROVIDER_FREE(pData);
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
BOOL Keyset_Remove_Cursor_Immediate::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(Keyset_Remove_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Keyset_Remove_Buffered - Keyset_Remove_Buffered
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Remove_Buffered::Init()
{
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass	=	TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_REMOVEDELETED;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	fTestPass=FALSE;
	COMPARE(BufferedUpdate(),TRUE);
	fTestPass=TRUE;
	
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Buffered::Variation_1()
{
	BYTE	*pData=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW	*pHRow=NULL;
	HROW	*pHRowSecond=NULL;
	HROW	*pHRowDuplicate=NULL;
	ULONG	cRefCount;
	DBPROPID	rgPropertyIDs[7];
	BOOL	fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[3]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[5]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;

	HRESULT hr = S_OK;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,7,rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//identity is level 0  and required
	if (!m_pIRowsetIdentity)
		goto CLEANUP;

	//retrieve the ith row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//move the cursor away to the ith row
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-2,1,&cRows,&pHRowSecond),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowSecond);
	
	//retrieve the ith row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,-(LONG)m_ulTableRows+1,-1,&cRows,&pHRowDuplicate),S_OK);
		
    //the two row handles should have the same value
	TESTC_(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowDuplicate),S_OK);
		

	//make changes to the ith row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRowDuplicate,m_hAccessor,m_pData),S_OK);
		
	//make sure GetData should be able to see the change on cursor based update
	if(!COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
	{
		goto CLEANUP;
	}

	//free the memory used by m_pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);
	//free the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL, &cRefCount, NULL),S_OK);		
	PROVIDER_FREE(pHRowDuplicate);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL, &cRefCount, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//try to retrieve the row handle again.  For keyset cursor, the change should
	//be in place or at the end of the rowset
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}

	//free the memory used by m_pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);
	 
	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}	
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	if(pHRowDuplicate && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDuplicate,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDuplicate);
	
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrieve row, update row update pending don't release handle, GetData RETURNPENDINGINSERTS FALSE
//			don't release the row handle, call GetData to See the row
//			eventhough RETURNPENDINGINSERTS is FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Buffered::Variation_2()
{
	BYTE		*pData				= NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW		hRow				= NULL;
	HROW		*pHRow				= NULL;
	DBPROPID	rgPropertyIDs[4];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;;

	HRESULT hr = S_OK;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,1, rgUnPropertyIDs,
										ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//identity is level 0  and required
	if (!m_pIRowsetIdentity)
	{
		goto CLEANUP;
	}
	//make a buffer
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//Now Insert the row
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,pData, &hRow),S_OK)		
	
	//release the row
	if(hRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
		hRow				= NULL;
	}

	//row still not released until pending change is trasmitted to the data source
	TESTC_(m_pIRowset->RestartPosition(NULL),DB_E_ROWSNOTRELEASED);

	//transmit the changes, now everythign should work ok
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	//restart
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//find the row
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	if(hRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	}
		
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Error should NULL out hRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Buffered::Variation_3()
{
	BYTE		*pData				= NULL;
	HROW		hRow				= NULL;
	HROW		hRow2				= NULL;
	DBPROPID	rgPropertyIDs[4];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;;

	HRESULT hr = S_OK;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,1, rgUnPropertyIDs,
										ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//identity is level 0  and required
	if (!m_pIRowsetIdentity)
	{
		goto CLEANUP;
	}

	//make a buffer
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);


	//Now Insert the row with ERROR
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,NULL, &hRow),E_INVALIDARG)		
	//this has to be NULLed out on error from InsertRow
	if(hRow)
	{
		goto CLEANUP;
	}

	//Now Insert the row with ERROR
	TESTC_(m_pIRowsetChange->InsertRow(NULL, 0,pData, &hRow),DB_E_BADACCESSORHANDLE)		
	//this has to be NULLed out on error from InsertRow
	if(hRow)
	{
		goto CLEANUP;
	}

	//Now Insert the row
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,pData, &hRow),S_OK)		
	//Now Insert the row in error
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,pData, &hRow2),DB_E_ROWSNOTRELEASED)		
	//this has to be NULLed out on error from InsertRow
	if(hRow2)
	{
		goto CLEANUP;
	}
	
	fTestPass	= TEST_PASS;
CLEANUP:
	if(hRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	}
	if(hRow2 && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hRow2,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pData);
		
	ReleaseRowsetAndAccessor();

	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ROWSNOTRELEASED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Remove_Buffered::Variation_4()
{  
	DBPROPID		rgPropertyIDs[3];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	HRESULT			hr					= S_OK;
	HROW			*pHRow				= NULL;
	DBCOUNTITEM	cRows			= 0;


	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_REMOVEDELETED;

	rgUnPropertyIDs[0]	= DBPROP_LITERALIDENTITY;
	rgUnPropertyIDs[1]	= DBPROP_CANHOLDROWS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate || (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//row still not released until pending change is trasmitted to the data source
	TESTC_(m_pIRowset->RestartPosition(NULL),DB_E_ROWSNOTRELEASED);

	//transmitt the changes, now everything should work ok
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	//restart position. 
	hr = m_pIRowset->RestartPosition(NULL);
	CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);
	
	fTestPass = TRUE;
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Remove_Buffered::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Dynamic_Cursor_Buffered - Dynamic_Cursor_Buffered
//|	Created:			04/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Buffered::Init()
{
	DBPROPID	rgPropertyIDs[4];
	BOOL	fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;
	rgPropertyIDs[2]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	fTestPass=FALSE;

	COMPARE(BufferedUpdate(),TRUE);
	COMPARE(GetProp(DBPROP_CANSCROLLBACKWARDS),TRUE);
	COMPARE(GetProp(DBPROP_CANFETCHBACKWARDS),TRUE);

	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row handle, update a row to be the first row in the row set with some columns set to NULL, and call GetData to see
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Cursor_Buffered::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	HROW		*pHRow			= NULL;
	ULONG		cCount			= 0;
	BYTE		*pData			= NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass		= TRUE;
	HRESULT		hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
										0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;

	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);
		
	//get the row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get a new data buffer to set the data.  The data is for first row.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//set a first nullable and updatble columns to NULL
	if(cCol)
	{
		for(cCount=0;cCount<m_cBinding;cCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[0])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				break;
			}
		}
	}

	//has to find such a column
	if(cCount==m_cBinding)
		goto CLEANUP;

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//get the data for the row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);


	//make sure GetData should be able to see the change
	if(!COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//free the memory used by m_pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;

			//free the memory used by m_pData
			ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

			break;
		}

		//free the memory used by m_pData
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle.  Move the cursor to the end of the rowset.  Retrieve the row handle again
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Cursor_Buffered::Variation_2()
{
	BYTE	*pData=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW	*pHRow=NULL;
	DBPROPID	rgPropertyIDs[4];
	BOOL	fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;
	rgPropertyIDs[2]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;

	HRESULT hr = S_OK;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;

	//retrieve the 5th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,4,1,&cRows,&pHRow),S_OK);
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL, NULL),S_OK);
	
	//retrieve another row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,-1,-1,&cRows,&pHRow),S_OK);

	if(!COMPARE(cRows, 1))
		goto CLEANUP;

	//make changes to the 5th row.  Make its value as the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);


	//make sure GetData should be able to see the change
	if(!COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
	
	//free the memory used by m_pData
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)m_pData);

	//release the rowset
	ReleaseRowsetAndAccessor();

	//reexecute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,
									ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrive a row handle. Move the cursor to the beginning of the rowset. Retrieve the row handle again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Cursor_Buffered::Variation_3()
{
	BYTE	*pData=NULL;
	BYTE	*pGetData=NULL;
	DBCOUNTITEM	cRows			= 0;
	HROW	*pHRow=NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL	fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;

	HRESULT hr = S_OK;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//allocation memory
	pGetData=(BYTE *)PROVIDER_ALLOC(m_cRowSize);

	if(!pGetData)
		goto CLEANUP;

	//retrieve the 3rd row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//Keep a copy of the original data
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//make changes to the 3rdth row.  Make its value as the 1st row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRow,m_hAccessor,pGetData),S_OK);
		

	//make sure GetData should be able to see the changed data
	//all I need to do is check for the 1st long column
	if(!COMPARE(CompareBuffer(pGetData,pData,1,&(m_rgBinding[3]),m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;


	//undo the change
	TESTC_(m_pIRowsetUpdate->Undo(NULL,1,pHRow, NULL, NULL, NULL),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRow,m_hAccessor,pGetData),S_OK);
		

	//make sure GetData should be able to see the original data
	if(!COMPARE(CompareBuffer(m_pData,pGetData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//free the memory used by pGetData
//	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pGetData);

	//free the row handle
	 TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		

	 //restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pGetData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pGetData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pGetData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Buffered::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}


// {{ TCW_TC_PROTOTYPE(Visible_Forward_Cursor)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Forward_Cursor - visibility of row handles
//|	Created:			04/11/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Forward_Cursor::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Forward_Cursor::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	BYTE		*pData			= NULL;
	DBPROPID	rgDBPropID[1];
	BOOL		fTestPass		= TEST_SKIPPED;
	HRESULT		hr				= S_OK;
	IRowsetInfo	*pIRowsetInfo	= NULL;
	ULONG		cProperty		= 0;
	DBPROPIDSET	DBPropIDSet;
	DBPROPID	DBPropID		= DBPROP_REMOVEDELETED;
	DBPROPSET	*pDBPropSet		= NULL;
	BOOL		fRemoveDeleteRows;

	rgDBPropID[0]	= DBPROP_IRowsetChange;

	//create a rowset and create a read/write accessor.  Forward only binding
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,
		2,g_rgPropertyIDs,NO_ACCESSOR));
		
	//Create a new accessor that binds only to the changed column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;
	
	fTestPass = FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;
	if(!VerifyInterface(m_pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
	{
		goto CLEANUP;
	}
	//get default value for REMOVEDELTED
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);
	fRemoveDeleteRows = ((*((*pDBPropSet).rgProperties)).vValue).boolVal;

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get the data
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
	
	//Release the row
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);	
	//restart position
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}


	//do the first row first out of the loop
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK);
	//Get the data for the 1st row handle
	if (fRemoveDeleteRows)
	{
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}
	else
	{
		hr=GetData(*pHRow,m_hAccessor,m_pData);
		if (S_OK==hr)
		{
			//make sure GetData should be able to see the change
			if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				goto CLEANUP;
			}
		}
		else
		{
			//this could be DB_E_DELETEDROW if DBPROP_REMOVEDELETED is FALSE and
			//backend does delete/insert for its updates
			if (DB_E_DELETEDROW!=hr)
			{
				goto CLEANUP;
			}
		}
	}
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(m_pData);

	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode, call SetData to change a non-key column.  Make sure the update is visible after RestartPosition.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Forward_Cursor::Variation_2()
{				
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	HROW			*pHRow			= NULL;
	BYTE			*pData			= NULL;
	DBCOUNTITEM		cUpdatedRows	= 0;
	HROW			*rgUpdatedRows	= NULL;
	ULONG			cRefCount		= 0;
	DBPROPID		rgDBPropIDSet[1];
	BOOL			fTestPass		= TEST_SKIPPED;
	HRESULT			hr				= S_OK;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	ULONG			cProperty		= 0;
	DBPROPIDSET		DBPropIDSet;
	DBPROPID		DBPropID		= DBPROP_REMOVEDELETED;
	DBPROPSET		*pDBPropSet		= NULL;
	BOOL			fRemoveDeleteRows;

	rgDBPropIDSet[0]     = DBPROP_IRowsetUpdate;

	//create a rowset and create a read/write accessor.  Forward only binding
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,1,rgDBPropIDSet,
		2,g_rgPropertyIDs,NO_ACCESSOR));
								  
	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get an array of numeric and updatable columns
	GetUpdatableCols(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	//Create a new accessor that binds only to the changed column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;
	if(!VerifyInterface(m_pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
	{
		goto CLEANUP;
	}
	//get default value for REMOVEDELTED
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);
	fRemoveDeleteRows = ((*((*pDBPropSet).rgProperties)).vValue).boolVal;

	//get the third row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get the data
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//Release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cUpdatedRows,&rgUpdatedRows,NULL),S_OK);
		
	//update should be successful and no row handle should be returned
	//as the row handle is released with pending change status
	COMPARE(cUpdatedRows, 1);

	//release the rgUpdatedRows
	TESTC_(m_pIRowset->ReleaseRows(1,rgUpdatedRows,NULL, &cRefCount, NULL),S_OK);		
	//restart position
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//do the first row first out of the loop
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK);
	//Get the data for the 1st row handle
	if (fRemoveDeleteRows)
	{
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}
	else
	{
		hr=GetData(*pHRow,m_hAccessor,m_pData);
		if (S_OK==hr)
		{
			//make sure GetData should be able to see the change
			if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				goto CLEANUP;
			}
		}
		else
		{
			//this could be DB_E_DELETEDROW if DBPROP_REMOVEDELETED is FALSE and
			//backend does delete/insert for its updates
			if (DB_E_DELETEDROW!=hr)
			{
				goto CLEANUP;
			}
		}
	}
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}	
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(rgUpdatedRows);
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

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
BOOL Visible_Forward_Cursor::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Static_Cursor)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Static_Cursor - visibility of row handles
//|					OWNUPDATEDELETE is VARIANT_TRUE, changes are visible to  self
//|	Created:			04/11/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Static_Cursor::Init()
{
	return TCIRowsetChange::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc	In immediate update mode, call SetData to change a non key column.  
//			Move the cursor to the end of the rowset.  RestartPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
// @mfunc In immediate update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset.  RestartPosition
int Visible_Static_Cursor::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	cCol;
	DBORDINAL	*rgColNumber	=NULL;
	HROW		*pHRow			=NULL;
	BYTE		*pData			=NULL;
	BYTE		*pBackEndData	=NULL;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass		=TEST_SKIPPED;
	BYTE		*pDataNoFetch	= NULL;
	HRESULT		hrRP			= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;

	HRESULT hr		= S_OK;

	//create a rowset and create a read/write accessor.  Forward only binding
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,
		2,g_rgPropertyIDs,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;
	
	//Create a new accessor that binds only to the changed column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//get the third row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get the data
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//make sure the change is visiable throught the other row handle
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, row has not been refetched
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//restart position
	hrRP=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change
		//OWNUPADTEDELETE is on
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
		if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			odtLog << "Error - this row should not be visible"<<ENDL;
			break;
		}
	}	
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode, call SetData to change a non key column.  Move the cursor to the end of the rowset. RestartPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Static_Cursor::Variation_2()
{
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol;
	DBORDINAL		*rgColNumber	=NULL;
	HROW			*pHRow			=NULL;
	BYTE			*pData			=NULL;
	BYTE			*pBackEndData	=NULL;
	DBPROPID		rgPropertyIDs[4];
	BOOL			fTestPass		=TEST_SKIPPED;
	HRESULT			hrRP			=S_OK;

   	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_IRowsetChange;
	rgPropertyIDs[3]=DBPROP_OWNUPDATEDELETE;

	HRESULT hr		= S_OK;

	//create a rowset and create a read/write accessor.  Forward only binding
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,4,rgPropertyIDs,
		2,g_rgPropertyIDs,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;

	//Create a new accessor that binds only to the changed column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
										USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//get the third row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get the data
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);


	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
		
	//Release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);
		
	//restart position
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	//allocate memory
	if(!(pBackEndData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(GetData(*pHRow,m_hAccessor,pBackEndData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		//OWNUPADTEDELETE is on
		if(CompareBuffer(pBackEndData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
		if(CompareBuffer(pBackEndData,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			odtLog << "Error - this row should not be visible"<<ENDL;
			break;
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBackEndData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode.  Create two rowsets on the same table.  One rowset 	changes a key value.  The other rowset should no
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Static_Cursor::Variation_3()
{
	HROW			*pHRow			= NULL;
	HROW			*pHRowSecond	= NULL;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	void			*pData			= NULL;
	void			*pDataSecond	= NULL;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	ICommand		*pICommand		= NULL;
	IUnknown        *pIRowset		= NULL;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass		= TEST_SKIPPED;
	BYTE			*pDataNoFetch	= NULL;
	HRESULT			hrRP			= S_OK;
	HRESULT			hr				= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;
	
	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//release the row set
	ReleaseRowsetAndAccessor();

	if(!m_pIDBCreateCommand)
		return TEST_PASS;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	//get the  row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//get a copy of the data on  row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	if(!(pDataNoFetch=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object//QI for IRowsetInfo pointer
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
	
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);

	//get data on the first rowset.  
	TESTC_(GetData(*pHRow,m_hAccessor,pDataNoFetch),S_OK);
		
	//make sure GetData should be able to see the change, row has not been refetched
	if(!COMPARE(CompareBuffer(pDataNoFetch,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;

	//restart position
	hrRP=((IRowset *)(pIRowset))->RestartPosition(NULL);
	if(!CHECK(hrRP==S_OK || hrRP==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	if(!(pDataSecond=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	while (S_OK==(hr = ((IRowset *)(pIRowset))->GetNextRows(NULL,0,1,&cRows, &pHRowSecond)) || hr == DB_S_ENDOFROWSET)
	{
		if( cRows ==0)
		{	
			break;
		}
		//Get the data for the ith row handle
		TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		TESTC_(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);

		//if the command was re-executed the change will be visible
		if (DB_S_COMMANDREEXECUTED==hrRP)
		{
			//the second rowset should see the change if rexeccuted
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
		else
		{
			//the second rowset should NOT see the change (it should see old data)
			if(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fTestPass=TRUE;
				break;
			}
			if(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				odtLog << "Error - this row should not be visible"<<ENDL;
				break;
			}
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataNoFetch);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

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
BOOL Visible_Static_Cursor::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Keyset_Command_Cursor)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Keyset_Command_Cursor - Visible_Keyset_Command_Cursor
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Keyset_Command_Cursor::Init()
{
	DBPROPID	rgPropertyIDs[2];									  
	BOOL		fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	if(!m_pIDBCreateCommand)
	{
		odtLog << "Command Not Supported Skipping Variations"<<ENDL;
		return TEST_SKIPPED;
	}

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	fTestPass = FALSE;
	
	COMPARE(BufferedUpdate(), FALSE);

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();   
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Keyset_Command_Cursor::Variation_1()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	HROW			*pHRowLast=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	ICommand		*pICommand=NULL;
	IUnknown		*pIRowset=NULL;
	IRowsetChange	*pIRowsetChange=NULL;
	DBROWSTATUS		DBRowStatus;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr = S_OK;

	rgPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
									0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get the 10th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
	
	//get a copy of the data on 10th row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//create a new data buffer to set the data for all updatable columns
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	if(!m_pIDBCreateCommand)
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}
		
	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//retrieve the 10th row handle on the second rowset
	TESTC_(((IRowset*)pIRowset)->GetNextRows(NULL,9,1,&cRows, &pHRowSecond),S_OK);
		
	//get data on the second rowset.  Share the same accessor on the command
	if(SUCCEEDED(m_hr=(((IRowset*)pIRowset)->GetData(*pHRowSecond,m_hAccessor,pDataSecond))))
	{
		//the second rowset should see the change 
		if(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}
	else
	{
		//the GetData realized the row is deleted
		if(!COMPARE(m_hr, DB_E_DELETEDROW))
			goto CLEANUP;
	}

	TESTC_(((IRowset*)pIRowset)->ReleaseRows(1, pHRowSecond,NULL,NULL,NULL),S_OK);
			
	PROVIDER_FREE(pHRowSecond);
	
	fTestPass=TRUE;

CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,&DBRowStatus),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,&DBRowStatus),S_OK);
	PROVIDER_FREE(pHRowSecond);

	
	if(pHRowLast && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowLast,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowLast);
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode.  Create two rowsets on the same table.  The second rowset change a non key column.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Keyset_Command_Cursor::Variation_2()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol;
	DBORDINAL		*rgColNumber=NULL;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	ICommand		*pICommand=NULL;
	IUnknown        *pIRowset=NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	ReleaseRowsetAndAccessor();

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,2,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	 //get the 10th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//get a copy of the data on 10th row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
	//create a new data buffer to set the data for the numeric and updatablecolumn
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);
	
	if(!m_pIDBCreateCommand)
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}
	
	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);

	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//retrieve the 10th row handle on the second rowset
	TESTC_(((IRowset*)pIRowset)->GetNextRows(NULL,9,1,&cRows,&pHRowSecond),S_OK);
		
	//get data on the second rowset.  Share the same accessor on the command
	TESTC_(((IRowset*)pIRowset)->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		
	//the second rowset should see the change
	if(COMPARE(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode.  Create two rowsets on the same table.  The second rowset change a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Keyset_Command_Cursor::Variation_3()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	HROW			*pHRowLast=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	ICommand		*pICommand=NULL;
	IUnknown		*pIRowset=NULL;
	IRowsetChange	*pIRowsetChange=NULL;
	DBROWSTATUS		DBRowStatus;
	DBROWSTATUS		*pDBRowStatus=NULL;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;

	//get the 3th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);


	//get a copy of the data on 3rd row
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	

	//create a new data buffer to set the data for all updatable columns
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);
	
	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);


	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,&pDBRowStatus),S_OK);
		
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//for keyset driven cursor with cursor based update, the update of a new key
	//value should be visible as a delete only.

	//retrieve the 3rd row handle on the second rowset
	TESTC_(((IRowset*)pIRowset)->GetNextRows(NULL,2,1,&cRows, &pHRowSecond),S_OK);
		
	//get data on the second rowset.  Share the same accessor on the command
	if(SUCCEEDED(m_hr=((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond)))
	{
		//the second rowset should see the change 
		if(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}
	else
	{
		//the GetData realized the row is deleted( for sql providers)
		if(!COMPARE(m_hr, DB_E_DELETEDROW))
			goto CLEANUP;
	}

	//release the row handle
	TESTC_(((IRowset*)pIRowset)->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	fTestPass=TRUE;

CLEANUP:
	//free the memory
	PROVIDER_FREE(pDBRowStatus);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL, &DBRowStatus, NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL, &DBRowStatus, NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	if(pHRowLast && m_pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowLast,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowLast);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non key column.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Keyset_Command_Cursor::Variation_4()
{
	return TEST_PASS;

}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Keyset_Command_Cursor::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Dynamic_Command_Cursor)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Dynamic_Command_Cursor - Visible_Dynamic_Command_Cursor
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Dynamic_Command_Cursor::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL	fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	if(!m_pIDBCreateCommand)
		goto CLEANUP;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	fTestPass = FALSE;
	COMPARE(BufferedUpdate(),FALSE);
	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode.  Create two rowsets on the same table.  The second rowset change a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_1()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	ICommand		*pICommand=NULL;
	IUnknown        *pIRowset=NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the third row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
	
	//create a new data buffer to set the data for all updatable columns
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);
	
	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);

	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//set data on the 2rd rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);

	TESTC_(((IRowset*)m_pIRowset)->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//retrieve the row handle
	//restartposition
	m_hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(m_hr==S_OK || m_hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	while(((IRowset*)pIRowset)->GetNextRows(NULL,0,1,&cRows, &pHRowSecond) == S_OK)
	{
		if(cRows == 0)
			break;

		//Get the data for the xth row handle
		TESTC_(((IRowset*)pIRowset)->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		TESTC_(((IRowset*)pIRowset)->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);

		//make sure GetData should be able to see the change
		//the second rowset should see the change.  
		if(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}	
	
CLEANUP:
	//release memory
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataSecond, TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode.  Create two rowsets on the same table.  The second rowset change a non-key column.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_2()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cCol;
	DBORDINAL		*rgColNumber=NULL;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	IRowsetInfo		*pIRowsetInfo=NULL;
	ICommand		*pICommand=NULL;
	IUnknown        *pIRowset=NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
//		0,NULL,NO_ACCESSOR));
		0,NULL,NO_ACCESSOR,TRUE));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	ReleaseRowsetAndAccessor();

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
										0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	//get the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
		
	//get a copy of the data 
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//create a new data buffer to set the data for the numeric and updatable column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//for dynamic cursor, the update for non-key value is always visible.retrieve the 4th row handle on the second rowset
	TESTC_(((IRowset*)pIRowset)->GetNextRows(NULL,3,1,&cRows,&pHRowSecond),S_OK);
		
	//get data on the second rowset.  Share the same accessor on the command
	TESTC_(((IRowset*)pIRowset)->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		
	//the second rowset should see the change
	if(COMPARE(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	if(rgColNumber)
		PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowSecond && pIRowset)
	{
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);
	}


	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode. A second command object deletes one row by SQL text.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_3()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	IRowset			*pIRowset=NULL;
	IRowsetChange	*pIRowsetChange=NULL;

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
 	//get the last row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-1,1,&cRows,&pHRow),S_OK);
		
	//execute the command again
	TESTC_(m_pICommand->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset),S_OK);
		
	//QI for the IRowsetChange
	TESTC_(pIRowset->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange),S_OK);
		
	//get the same row
	TESTC_(pIRowset->GetNextRows(NULL,m_ulTableRows-1,1,&cRows,&pHRowSecond),S_OK);
		
	//delete the row
	TESTC_(pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowSecond, NULL),S_OK);
		
	//set data on the deleted row
	if(CHECK(SetData(*pHRow, m_hAccessor, pData),DB_E_DELETEDROW))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle on the second row set
	if(pHRowSecond && pIRowset)
	{
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);
	}

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode. A second commnd object changes one row by SQL text.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_4()
{
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	DBPROPID			rgPropertyIDs[2];
	IRowset			*pIRowset=NULL;
	IRowsetChange	*pIRowsetChange=NULL;
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pDataSecond,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
 	//get the 1st row row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get data
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//execute the command again
	TESTC_(m_pICommand->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset),S_OK);
		
	//QI for the IRowsetChange
	TESTC_(pIRowset->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange),S_OK);
		
	//get the same row
	TESTC_(pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowSecond),S_OK);
		
	//Update the row
	TESTC_(pIRowsetChange->SetData(*pHRowSecond, m_hAccessor,pDataSecond),S_OK);
		
	//set data on the 1st row handle.  The row appear to be deleted.
	if(CHECK(SetData(*pHRow, m_hAccessor, pData),DB_E_DELETEDROW))
		fTestPass=TRUE;

CLEANUP:
	//release the row handle on the second row set
	if(pHRowSecond && pIRowset)
	{
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);
	}

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);

	if(pDataSecond)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataSecond, TRUE);

	//release the 1st rowset
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}


	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode. A second commnd object changes one row by SQL text.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_5()
{
	HROW			*pHRow					= NULL;
	HROW			*pUpdatedHRow			= NULL;
	HROW			*pHRowSecond			= NULL;
	DBROWSTATUS		*pDBRowStatus			= NULL;
	DBCOUNTITEM		cRows					= 0;
	IRowset			*pIRowset				= NULL;
	IRowsetChange	*pIRowsetChange			= NULL;
	IRowsetUpdate	*pIRowsetUpdate			= NULL;
	void			*pData					= NULL;
	void			*pDataSecond			= NULL;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass				= TEST_SKIPPED;
	HRESULT			hr						= S_OK;
	DWORD			cCount					= 0;

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_REMOVEDELETED;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,
										ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a new data buffer to set the data for all updatable columns to 4
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pDataSecond,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
 	//get the 2nd row row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//execute the command again
	TESTC_(m_pICommand->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset),S_OK);
		
	//QI for the IRowsetChange
	TESTC_(pIRowset->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange),S_OK);
	//QI for the IRowsetUpdate
	TESTC_(pIRowset->QueryInterface(IID_IRowsetUpdate, (LPVOID *)&pIRowsetUpdate),S_OK);
	//get the same row
	TESTC_(pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRowSecond),S_OK);
	//Update the row
	TESTC_(pIRowsetChange->SetData(*pHRowSecond, m_hAccessor,pDataSecond),S_OK);
	//get data
	TESTC_(pIRowset->GetData(*pHRowSecond, m_hAccessor, m_pData),S_OK);	
	//push update to back end  //some provider's can choose to delete/insert instead of update
	TESTC_(pIRowsetUpdate->Update(NULL,1,pHRowSecond,NULL,NULL,NULL),S_OK);
							   
	//Update the row in the cache
	TESTC_(SetData(*pHRow, m_hAccessor,pDataSecond),S_OK);
	//push update tp back end, row is moved on back endso this should fail
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),DB_E_ERRORSOCCURRED);
	
	PROVIDER_FREE(pDBRowStatus);
	PROVIDER_FREE(pUpdatedHRow);

	//try again
	//set data on the 1st row handle, just cache so it should be ok
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//udpate should fail
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),DB_E_ERRORSOCCURRED);
		
	//the test will not expect  for the status because this is a dynamic curosr
	//so the row is moved to the end of the row set
	if(COMPARE(*pDBRowStatus, DBROWSTATUS_E_DELETED))
		fTestPass=TRUE;

	//get data
	hr=pIRowset->GetData(*pHRowSecond, m_hAccessor, m_pData);
	if (S_OK!=hr)
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
			//loop through columns
			for	(
				cCount=0;
				cCount < m_cBinding;
				cCount++
				)
			{
//				switch (*((BYTE *)dwAddrGet+(m_rgBinding[cCount]).obStatus))
				switch (STATUS_BINDING(m_rgBinding[cCount],pData))
				{
					case DBSTATUS_S_OK:
					case DBSTATUS_E_UNAVAILABLE:
						break;
					case DBSTATUS_S_ISNULL:
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
					default:
						goto CLEANUP;
				}
			}
		}
		else
		{
			goto CLEANUP;
		}
	}

CLEANUP:
	//release the row handle on the second row set
	if(pHRowSecond && pIRowset)
		CHECK(((IRowset *)(pIRowset))->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
	
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetUpdate);

	if(pDataSecond)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataSecond, TRUE);

	//release the row handle on the 1st row set
	PROVIDER_FREE(pDBRowStatus);

	//there is pending change
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pUpdatedHRow);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_CHANGEINSERTEDROWS-TRUE.A second commnd object change a row by SQL text.It tries to change same row again-succeeds.
//			It then tries to change the same row again.  if DBPROP_CHANGEINSERTEDROWS is 
//			VARIANT_TRUE the second change will work.	
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_6()
{
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	DBPROPID		rgPropertyIDs[4];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_CHANGEINSERTEDROWS;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_OWNINSERT;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pDataSecond,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
 	//get the 1st row row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	
	//get data
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//release the row
//	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
//	PROVIDER_FREE(pHRow);
	
	//Now Insert the row
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,pData, pHRow),S_OK)		
	
	//set data on the 1st row handle. 
//	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
	
	//set data on the 1st row handle. 
	if(CHECK(SetData(*pHRow, m_hAccessor, pDataSecond),S_OK))
	{
		fTestPass=TRUE;
	}

CLEANUP:
	if(pDataSecond)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataSecond, TRUE);

	//release the 1st rowset
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_CHANGEINSERTEDROWS-FALSE.A second commnd object change a row by SQL text.It tries to change same row again-fails.
//			It then tries to change the same row again.  if DBPROP_CHANGEINSERTEDROWS is 
//			VARIANT_FALSE the second change won't work, the row will not
//			be found when it is changed for the second time.	
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Cursor::Variation_7()
{
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cRows			= 0;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	DBPROPID		rgPropertyIDs[3];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OWNINSERT;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]=DBPROP_CHANGEINSERTEDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_COMMAND_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create a new data buffer to set the data for all updatable columns to 1
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pDataSecond,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
 	//get the 1st row row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get data
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//Now Insert the row
	TESTC_(m_pIRowsetChange->InsertRow(NULL, m_hAccessor,pData, pHRow),S_OK)
		
	//set data on the 1st row handle. 
	if(CHECK(SetData(*pHRow, m_hAccessor, pDataSecond),DB_E_NEWLYINSERTED))
	{
		fTestPass=TRUE;
	}

CLEANUP:
	if(pDataSecond)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataSecond, TRUE);

	//release the 1st rowset
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Dynamic_Command_Cursor::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Bookmark_Forward)
//*-----------------------------------------------------------------------
//| Test Case:		Bookmark_Forward - Bookmark_Forward
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmark_Forward::Init()
{

	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass=TEST_SKIPPED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	COMPARE(BufferedUpdate(),FALSE);
	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst=NULL;
	HROW			*pHRowSecond=NULL;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[4];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_IRowsetIdentity;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get the first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);
	
	//move the cursor away
	TESTC_(m_pIRowset->GetNextRows(NULL,10,0,&cRows,&pHRowSecond),S_OK);
	
	//no row should be retrieved
	if(!COMPARE(cRows, 0) || !COMPARE(pHRowSecond, NULL))
		goto CLEANUP;

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond),S_OK);
		
	//the two row handles have to be the same
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRowFirst), (DWORD)(*pHRowSecond));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRowFirst,pHRowSecond))
			goto CLEANUP;
	}

	//set new data to the first row.make data for the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,(BYTE **)&pData,
							g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRowSecond, m_hAccessor,pData),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	//not going to use pHRowSecond again
	PROVIDER_FREE(pHRowSecond);
	PROVIDER_FREE(pHRowFirst);

	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
		
	//retrieve the  row handles
	//while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowSecond)) || hr == DB_S_ENDOFROWSET )
	//{
	//	if( cRows ==0)
	//		break;
	//	//Get the data for the 10th row handle
	//	TESTC_(GetData(*pHRowSecond,m_hAccessor,m_pData),S_OK);
	//	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	//	PROVIDER_FREE(pHRowSecond);
	//	//make sure GetData should be able to see the change
	//	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	//	{
	//		fTestPass=TRUE;
	//		break;
	//	}
	//}
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRowFirst && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_2()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	HROW		HRow				= NULL;
	HACCESSOR	hAccessorBookmark	= NULL;
	DBBKMARK	cbBookmark			= 0;
	DBORDINAL	cCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	DBPROPID	rgPropertyIDs[3];
	void		*pData				= NULL;
	BYTE		*pBookmark			= NULL;
	HRESULT		hr					= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	
	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		2,g_rgPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	//get the bookmark for the 20th row in the rowset
	if(!GetBookmark(1,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//release the accessor on the rowset
	ReleaseAccessorOnRowset();

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber))

	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//get the ith row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)(&pBookmark),&HRow,NULL),S_OK);
		
	//set data
	TESTC_(SetData(HRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&HRow,NULL,NULL,NULL),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
	
	//restart position
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
		
	//get data
	TESTC_(GetData(HRow, m_hAccessor, m_pData),S_OK);
		
	//the data has to be same as what was set
	//if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;
	
CLEANUP:				   
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pData);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row, RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_3()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;

	rgUnPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,6,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the new last row
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=false, BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_4()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond=NULL;
	DBPROPID		rgPropertyIDs[5];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_BOOKMARKSKIPPED;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;
	rgUnPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,
		2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
 
	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the deleted row but just marked deleted
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK(REMOVEDELETED is FALSE).  anything else is an error.
	//here is some code to help debug if the hr is not DB_S_ENDOFROWSET
	if (S_OK!=hr)
	{
		//if GetRowsAt returned a row handle try to do some debugging
		if (pHRowSecond)
		{
			//double check that the deleted row and the bookmarked row are the same
			hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
			if (S_OK==hr)
			{
				//these are the same, fail the variation, GerRowsAt should have returned
				goto CLEANUP;
			}
			else
			{
				//this should never happen, this is also an  error
				goto CLEANUP;
			}
		}
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_5()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond=NULL;
	DBPROPID		rgPropertyIDs[5];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,
		2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the deleted row (REMOVEDELETED=TRUE)
	//new last row of the rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=false, BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_6()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond=NULL;
	DBPROPID		rgPropertyIDs[4];
	DBPROPID		rgUnPropertyIDs[3];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;
	rgUnPropertyIDs[1]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,
		3,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
 
	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.  
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the deleted row but REMOVEDELETED is FALSE so its just marked deleted
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK eventhough BOOKMARKSSKIPPED is false cause REMOVEDELETED is FALSE.  
	//anything else is an error.
	//here is some code to help debug if the hr is not DB_S_ENDOFROWSET
	if (S_OK!=hr)
	{
		//if GetRowsAt returned a row handle try to do some debugging
		if (pHRowSecond)
		{
			//double check that the deleted row and the bookmarked row are the same
			hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
			if (S_OK==hr)
			{
				//these are the same, fail the variation, GerRowsAt should have returned
				goto CLEANUP;
			}
			else
			{
				//this should never happen, this is also an  error
				goto CLEANUP;
			}
		}
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_7()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBORDINAL		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;

	rgUnPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,
		1,rgUnPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond);
	
	//this should always be DB_S_ENDOFROWSET.  REMOVEDELETED is TRUE so the row with the bookmark is gonzo.
	//try to get the next row
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	if (pHRowSecond)
	{
		//double check that the deleted row and the row brought back from GetRowsAt are different
		hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
		if (S_OK==hr)
		{
			//these are the same, fail the variation, GerRowsAt should have returned
			goto CLEANUP;
		}
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_8()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[4];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBORDINAL		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_REMOVEDELETED;

	rgUnPropertyIDs[1]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,
		2,rgUnPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	//this should always be DB_E_BADBOOKMARK.  BOOKMARKSSKIPPED is FALSE
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond),DB_E_BADBOOKMARK);
	
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_9()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[5];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[4]=DBPROP_BOOKMARKSKIPPED;

	rgUnPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	
			
	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//no rows, has to be DB_E_ENDOFROWSET
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Forward::Variation_10()
{
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[5];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	BYTE			*pBookmark	= NULL;	
	ULONG			cCol		= 0;
	DBCOUNTITEM		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[4]=DBPROP_BOOKMARKSKIPPED;

	rgUnPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	
			
	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,5,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

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
BOOL Bookmark_Forward::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(Bookmark_Static)
//*-----------------------------------------------------------------------
//| Test Case:		Bookmark_Static - Bookmark_Static
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmark_Static::Init()
{
	DBPROPID	rgPropertyIDs[3];
	BOOL	fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,NO_ACCESSOR));

	COMPARE(BufferedUpdate(),FALSE);

	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Static::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst=NULL;
	HROW			*pHRowSecond=NULL;
	void			*pData=NULL;
	void			*pDataSecond=NULL;
	DBPROPID		rgPropertyIDs[5];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_CANSCROLLBACKWARDS;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);
	
	//move the cursor away
	TESTC_(m_pIRowset->GetNextRows(NULL,10,0,&cRows,&pHRowSecond),S_OK);
	
	//no row should be retrieved
	if(!COMPARE(cRows, 0) || !COMPARE(pHRowSecond, NULL))
		goto CLEANUP;

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond),S_OK);
		
	//the two row handles have to be the same
	//the two row handles have to be the same
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRowFirst), (DWORD)(*pHRowSecond));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRowFirst,pHRowSecond))
			goto CLEANUP;
	}

	//get a copy of the old data
	TESTC_(GetData(*pHRowFirst, m_hAccessor, m_pData),S_OK);
		
	//set new data to the first row.make data for the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRowSecond, m_hAccessor,pData),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	//not going to use pHRowSecond again
	PROVIDER_FREE(pHRowSecond);
	
	//for static cursor, no change should be seen get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowFirst),S_OK);
		
	//allocate memory for GetData
	pDataSecond=PROVIDER_ALLOC(m_cRowSize);
	if(!pDataSecond)
		PROVIDER_FREE(pDataSecond);

	//get data
	TESTC_(GetData(*pHRowFirst,m_hAccessor,pDataSecond),S_OK);
		
	//the change is not seen
	if(COMPARE(CompareBuffer(pDataSecond,m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(1,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pDataSecond);

	if(pHRowFirst && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Static::Variation_2()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	HROW		HRow				= NULL;
	HROW		HRowSecond			= NULL;
	HACCESSOR	hAccessorBookmark	= NULL;
	DBBKMARK	cbBookmark			= 0;
	DBORDINAL	cCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	DBPROPID	rgPropertyIDs[4];
	void		*pData				= NULL;
	BYTE		*pBookmark			= NULL;
	HRESULT		hr					= S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANSCROLLBACKWARDS;
	
	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,
		g_cPropertyIDs,g_rgPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the bookmark for the 20th row in the rowset
	if(!GetBookmark(10,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//release the accessor on the rowset
	ReleaseAccessorOnRowset();

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber))

	//create a new data buffer to set the data for the numeric and updatable column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)(&pBookmark),&HRow,NULL),S_OK);
		
	//set data
	TESTC_(SetData(HRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&HRow,NULL,NULL,NULL),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
		
	HRow=NULL;

	//restart position
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		goto CLEANUP;

	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRowSecond,NULL),S_OK);
		
	//get data
	TESTC_(GetData(HRowSecond, m_hAccessor, m_pData),S_OK);
		
	//the data has to be same as what was before 
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),FALSE))
		fTestPass=TRUE;

CLEANUP:				   
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
	
	if(HRowSecond)
		CHECK(m_pIRowset->ReleaseRows(1,&HRowSecond,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmark_Static::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Bookmark_Keyset)
//*-----------------------------------------------------------------------
//| Test Case:		Bookmark_Keyset - Bookmark_Keyset
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE6
//
BOOL Bookmark_Keyset::Init()
{
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass=TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	//TESTC_PROVIDER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported &&
	//	g_rgDBPrpt[IDX_OtherUpdateDelete].fSupported);

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	fTestPass=FALSE;

	COMPARE(BufferedUpdate(),FALSE);

	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst=NULL;
	HROW			*pHRowSecond=NULL;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[5];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);
		

	//move the cursor away
	TESTC_(m_pIRowset->GetNextRows(NULL,10,0,&cRows,&pHRowSecond),S_OK);
		

	//no row should be retrieved
	if(!COMPARE(cRows, 0) || !COMPARE(pHRowSecond, NULL))
		goto CLEANUP;

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,
		&pHRowSecond),S_OK);
		

	//the two row handles have to be the same
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRowFirst), (DWORD)(*pHRowSecond));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRowFirst,pHRowSecond))
			goto CLEANUP;
	}

	//set new data to the first row.make data for the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRowSecond, m_hAccessor,pData),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	//not going to use pHRowSecond again
	PROVIDER_FREE(pHRowSecond);


	//for keyset driven cursor on cursor based update, we should be able
	//to see the change.  The change should be either at the last row or
	//in place

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,
		&pHRowFirst),S_OK);


	//get data
	m_hr=GetData(*pHRowFirst,m_hAccessor,m_pData);

	if(m_hr!=ResultFromScode(DB_E_DELETEDROW))
	{

		//the data should be the same as what was set
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}
	
	//release
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowFirst);
	pHRowFirst=NULL;

	//get the last row
	DBBookmark=DBBMK_LAST;

	//get the last row
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,
			&cRows, &pHRowFirst),S_OK);
				
	//get data
	TESTC_(GetData(*pHRowFirst,m_hAccessor,m_pData),S_OK);
		
	//the data should be the same as what was set
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
	{
		fTestPass = TRUE;
		goto CLEANUP;
	}

	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRowFirst && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_2()
{
	BOOL		fTestPass=TEST_SKIPPED;
	HROW		HRow=NULL;
	HACCESSOR	hAccessorBookmark=NULL;
	DBBKMARK	cbBookmark=0;
	DBORDINAL	cCol;
	DBORDINAL	*rgColNumber=NULL;
	DBPROPID	rgPropertyIDs[3];
	void		*pData=NULL;
	BYTE		*pBookmark=NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	//create an accessor on the bookmark column
	cCol=0;

	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol))

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the bookmark for the 20th row in the rowset
	if(!GetBookmark(10,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//release the accessor on the rowset
	ReleaseAccessorOnRowset();

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)(&pBookmark),&HRow,NULL),S_OK);
		
	//set data
	TESTC_(SetData(HRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&HRow,NULL,NULL,NULL),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
		
	//for keyset driven cursor on a querybased update, the change to the non-key value should be visible.

	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
		
	//get data
	TESTC_(GetData(HRow, m_hAccessor, m_pData),S_OK);
		
	//the data has to be same as what was set
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:				   
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBookmark);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Immediate update mode.  No row revisited.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_3()
{
	DBBOOKMARK		DBBookmark=DBBMK_LAST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow=NULL;
	HROW			*pHRowSecond=NULL;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[5];
	IRowsetIdentity	*pIRowsetIdentity=NULL;
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//identity is level 0 so it is required
	if (!m_pIRowsetIdentity)
		goto CLEANUP;

	//QI For IRowsetIdentity pointer
	TESTC_(m_pIRowsetChange->QueryInterface(IID_IRowsetIdentity,
		(LPVOID *)&pIRowsetIdentity),S_OK);
		

	//get the first row handle again by DBBMK_LAST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
		

	//set new data to the first row.
	//make data for the last row
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//set data
	TESTC_(SetData(*pHRow, m_hAccessor,pData),S_OK);
		

	//get the row handle again by DBBMK_LAST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,
		&pHRowSecond),S_OK);
		

	//get data
	TESTC_(GetData(*pHRowSecond,m_hAccessor,m_pData),S_OK);
		

	//the data should be the same as what was set
	if(!COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		goto CLEANUP;
	

	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowSecond && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowSecond);
	}

	SAFE_RELEASE(pIRowsetIdentity);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Buffered update mode.  No row revisited.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_4()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	HROW		HRow				= NULL;
	HACCESSOR	hAccessorBookmark	= NULL;
	DBBKMARK	cbBookmark			= 0;
	DBORDINAL	cCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	DBPROPID	rgPropertyIDs[3];
	void		*pData				= NULL;
	BYTE		*pBookmark			= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));
	
	//create an accessor on the bookmark column
	cCol=0;

	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol))

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;

	//get the bookmark for the 15th row in the rowset
	if(!GetBookmark(13,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//get the 15th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)(&pBookmark),&HRow,NULL),S_OK);
		

	//release the accessor on the rowset
	ReleaseAccessorOnRowset();

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
	{
		fTestPass = TRUE;
		goto CLEANUP;
	}

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//set data
	TESTC_(SetData(HRow,m_hAccessor,pData),S_OK);
		

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
		

	//for keyset driven cursor on a querybased update, the change 
	//to the non-key value should be visible.

	//get the 15th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
		

	//get data
	TESTC_(GetData(HRow, m_hAccessor, m_pData),S_OK);
		

	//the data has to be same as what was set
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:				   
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	if(HRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
	}

	if(pBookmark)
		PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_5()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[7];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,7,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.  
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the new last row
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_6()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond=NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[1]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,6,rgPropertyIDs,
		2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the deleted row (REMOVEDELETED=TRUE) so get the new last row
	//new last row of the rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}
	
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_7()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[7];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBORDINAL		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,7,rgPropertyIDs,
		1,rgUnPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond);
	
	//this should always be DB_S_ENDOFROWSET.  REMOVEDELETED is TRUE so the row with the bookmark is gonzo.
	//try to get the next row
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	if (pHRowSecond)
	{
		//double check that the deleted row and the row brought back from GetRowsAt are different
		hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
		if (S_OK==hr)
		{
			//these are the same, fail the variation, GerRowsAt should have returned
			goto CLEANUP;
		}
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_8()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBORDINAL		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[1]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,
		2,rgUnPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	//this should always be DB_E_BADBOOKMARK.  BOOKMARKSSKIPPED is FALSE
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond),DB_E_BADBOOKMARK);
	
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_9()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[1]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	

	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,6,rgPropertyIDs,
		2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//no rows, has to be DB_E_ENDOFROWSET
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Keyset::Variation_10()
{
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBPROPID		rgUnPropertyIDs[2];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	BYTE			*pBookmark	= NULL;	
	DBORDINAL		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;
	rgUnPropertyIDs[1]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	

	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,6,rgPropertyIDs,
		2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	//this should always be DB_E_BADBOOKMARK.  BOOKMARKSSKIPPED is FALSE
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond),DB_E_BADBOOKMARK);
	
	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

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
BOOL Bookmark_Keyset::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Bookmark_Dynamic)
//*-----------------------------------------------------------------------
//| Test Case:		Bookmark_Dynamic - Bookmark_Dynamic
//|	Created:			04/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Bookmark_Dynamic::Init()
{
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass=TEST_SKIPPED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERINSERT;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	fTestPass=FALSE;

	COMPARE(BufferedUpdate(),FALSE);
	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode, call SetData on the key value on the first row handle retrieved by IRowsetLocate::GetRowsAt
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_1()
{
	DBBOOKMARK		DBBookmark=DBBMK_FIRST;
	BYTE			*pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst=NULL;
	HROW			*pHRowSecond=NULL;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[5];
	BOOL			fTestPass=TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_OTHERINSERT;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;

	//get the first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);
		
	//move the cursor away
	TESTC_(m_pIRowset->GetNextRows(NULL,10,0,&cRows,&pHRowSecond),S_OK);
		
	//no row should be retrieved
	if(!COMPARE(cRows, 0) || !COMPARE(pHRowSecond, NULL))
	{
		goto CLEANUP;
	}

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond),S_OK);

	//the two row handles have to be the same
	//Compare rows, the two row handles should have the same value
	if (GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET, g_pIDBInitialize))
	{
		COMPARE((DWORD)(*pHRowFirst), (DWORD)(*pHRowSecond));
	}
	else
	{
		if (!fnIsSameRow(m_pIRowset,pHRowFirst,pHRowSecond))
		{
			goto CLEANUP;
		}
	}

	//set new data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(SetData(*pHRowSecond, m_hAccessor,pData),S_OK);
		
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	//not going to use pHRowSecond again
	PROVIDER_FREE(pHRowSecond);

	//retriev 
	DBBookmark=DBBMK_FIRST;

	//get the first row handle again by DBBMK_FIRST
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowFirst),S_OK);
	
	//get data
	TESTC_(GetData(*pHRowFirst,m_hAccessor,m_pData),S_OK);
		
	//the data should be the same as what was set
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRowFirst && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc buffered udpate mode, call SetData one the whole row on a middle row handle retrieved by IRowsetLocate::GetRowsByBookmark.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_2()
{
	BOOL		fTestPass=TEST_SKIPPED;
	HROW		HRow=NULL;
	HACCESSOR	hAccessorBookmark=NULL;
	DBBKMARK	cbBookmark=0;
	DBCOUNTITEM	cCol;
	DBORDINAL	*rgColNumber=NULL;
	DBPROPID	rgPropertyIDs[3];
	void		*pData=NULL;
	BYTE		*pBookmark=NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OTHERINSERT;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create an accessor on the bookmark column
	cCol=0;

	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol))

	//get the bookmark for the 20th row in the rowset
	if(!GetBookmark(2,&cbBookmark,&pBookmark))
		goto CLEANUP;

	//release the accessor on the rowset
	ReleaseAccessorOnRowset();

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber))

	//create a new data buffer to set the data for the numeric and updatable
	//column
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,1,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
		(const BYTE **)(&pBookmark),&HRow,NULL),S_OK);

	//set data
	TESTC_(SetData(HRow,m_hAccessor,pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&HRow,NULL,NULL,NULL),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);
		
	//get the 20th row handle by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
		
	//get data
	TESTC_(GetData(HRow, m_hAccessor, m_pData),S_OK);
		
	//the data has to be same as what was set
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:				   
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBookmark);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_3()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[8];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[7]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,8,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.  
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the new last row
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure BMK_LAST sees the new last row. RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_4()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond=NULL;
	DBPROPID		rgPropertyIDs[7];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass=TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[6]=DBPROP_OTHERINSERT;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,7,rgPropertyIDs,
		1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.  The query will be re-executed for forward only cursor
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//row should be the deleted row (REMOVEDELETED=TRUE) so get the new last row
	//new last row of the rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//double check that the deleted row and the row brought back from GetRowsAt are different
	hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
	if (S_OK==hr)
	{
		//these are the same, fail the variation, GerRowsAt should have returned
		goto CLEANUP;
	}
	
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_5()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[8];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBCOUNTITEM		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[7]=DBPROP_OTHERINSERT;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,8,rgPropertyIDs,
		0,NULL, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond);
	
	//this should always be DB_S_ENDOFROWSET.  REMOVEDELETED is TRUE so the row with the bookmark is gonzo.
	//try to get the next row
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	if (pHRowSecond)
	{
		//double check that the deleted row and the row brought back from GetRowsAt are different
		hr=m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowSecond);
		if (S_OK==hr)
		{
			//these are the same, fail the variation, GerRowsAt should have returned
			goto CLEANUP;
		}
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc delete last row then make sure bookmark sees the new last row, RD=true,BMS=false
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_6()
{
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[7];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;
	DBCOUNTITEM		cCol		= 0;
	DBBKMARK		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[6]=DBPROP_OTHERINSERT;

	rgUnPropertyIDs[0]=DBPROP_BOOKMARKSKIPPED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//create an accessor on the bookmark column
	cCol=0;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,7,rgPropertyIDs,
		1,rgUnPropertyIDs, ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cCol));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the last row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	//this should always be DB_E_BADBOOKMARK.  BOOKMARKSSKIPPED is FALSE
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond),DB_E_BADBOOKMARK);
	
	fTestPass=TRUE;
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get BMK_LAST. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_7()
{
	DBBOOKMARK		DBBookmark	= DBBMK_LAST;
	BYTE			*pBookmark	= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[8];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	ULONG			cCountRows	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[7]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	

	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,8,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by DBBMK_LAST
	//no rows, has to be DB_E_ENDOFROWSET
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,1,pBookmark,0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc delete last row of 1 row table and try to get bookmark. RD=true,BMS=true
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Bookmark_Dynamic::Variation_8()
{
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;
	HROW			*pHRowSecond= NULL;
	DBPROPID		rgPropertyIDs[8];
	BOOL			fTestPass	= TEST_SKIPPED;
	HRESULT			hr;
	DBROWCOUNT		cCountRows	= 0;
	BYTE			*pBookmark	= NULL;	
	DBCOUNTITEM		cCol		= 0;
	DBCOUNTITEM		cbBookmark	= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetLocate;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_BOOKMARKSKIPPED;
	rgPropertyIDs[6]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[7]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	//set the table to one row table
	SetTable(g_p1RowTable,DELETETABLE_NO);	

	//if an ini file is being used then delete and set to one row table
	if(GetModInfo()->GetFileName())
	{
		//delete all rows in the table.
		if(g_p1RowTable->DeleteRows(ALLROWS) == S_OK)
		{
			// RePopulate table 
			if(g_p1RowTable->Insert(1, PRIMARY) != S_OK)
			{
				return FALSE;
			}
		}
	}

	//open a rowset and create an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,8,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the last row handle (should be handle to the only row)
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)))
	{
		cCountRows++;
		if( cRows ==0)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//there should only be one rwo in this table
	if (cCountRows!=1)
	{
		goto CLEANUP;
	}

	if(DB_S_ENDOFROWSET != hr)
	{
		goto CLEANUP;
	}

	//get the bookmark for the last row in the rowset
	if(!GetBookmark(cCountRows,&cbBookmark,&pBookmark))
	{
		goto CLEANUP;
	}

	//so restart position. 
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//and fetch last row
	TESTC_(m_pIRowset->GetNextRows(NULL,(cCountRows-1),1,&cRows, &pHRow),S_OK);

	//delete the only row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position.
	hr=m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get the LAST row handle again by its bookmark
	//row should end of rowset
	hr=m_pIRowsetLocate->GetRowsAt(NULL,DB_NULL_HCHAPTER,cbBookmark,(pBookmark),0,1,&cRows,&pHRowSecond);
	
	//this should always be S_OK.  REMOVEDELETED is TRUE so a new last row should be fetched.  
	//anything else is an error.
	//here is some code to help debug if the hr is not S_OK
	if (DB_S_ENDOFROWSET!=hr)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;
CLEANUP:
	//make this a one row table again
	g_p1RowTable->Insert(1, PRIMARY);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	PROVIDER_FREE(pBookmark);

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
BOOL Bookmark_Dynamic::Terminate()
{
	return(TCIRowsetChange::Terminate());
}//}}
//}}
//}}

// {{ TCW_TC_PROTOTYPE(OrderedBookmark_Keyset)
//*-----------------------------------------------------------------------
//| Test Case:		OrderedBookmark_Keyset - OrderedBookmark_Keyset
//|	Created:			04/16/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL OrderedBookmark_Keyset::Init()
{
	DBPROPID	rgPropertyIDs[4];
	BOOL		fTestPass		= TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
	rgPropertyIDs[3]=DBPROP_ORDEREDBOOKMARKS;

	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	fTestPass	= FALSE;

	COMPARE(BufferedUpdate(),FALSE);
	
	fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc immediate update mode.  Create two rowsets on the same table.  One rowset 	changes one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OrderedBookmark_Keyset::Variation_1()
{
	HROW			*pHRow			= NULL;
	HROW			*pHRowSecond	= NULL;
	DBBOOKMARK		DBBookmark		= DBBMK_FIRST;
	BYTE			*pBookmark		= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cRows			= 0;
	void			*pData			= NULL;
	void			*pDataSecond	= NULL;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	ICommand		*pICommand		= NULL;
	IUnknown        *pIRowset		= NULL;
	IRowsetLocate	*pIRowsetLocate	= NULL;
	DBPROPID		rgPropertyIDs[4];
	BOOL			fTestPass		= TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
	rgPropertyIDs[3]=DBPROP_ORDEREDBOOKMARKS;

	if(!m_pIDBCreateCommand)
		return TEST_SKIPPED;

	//create a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS, 
		UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the 1st row handle by 1st bookmark
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
		
	//create a new data buffer to set the data for all updatable columns
	//the new data is for the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	//QI for IRowsetInfo pointer
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		
	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIRowset),S_OK);
		
	//QI for IRowsetLocate pointer on the second rowset
	if(!COMPARE(((IRowset *)pIRowset)->QueryInterface(IID_IRowset,(LPVOID *)&pIRowsetLocate),S_OK))
	{
		goto CLEANUP;
	}

	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//allocate memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	
	//retrieve the 1st row handle on the second rowset by 1st bookmark
	if(SUCCEEDED(m_hr=pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond)))
	{
		//get data on the second rowset.  Share the same accessor on the command
		TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
	}
	else
	{
		//the first row is detected as deleted
		COMPARE(m_hr, DB_E_DELETEDROW);

		//get the last row by it bookmark
		DBBookmark=DBBMK_LAST;
		TESTC_(pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond),S_OK);
			goto CLEANUP;

		//get data on the second rowset.  Share the same accessor on the command
		TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);		
	}

	//the second rowset should be able to see the change
	if(COMPARE(CompareBuffer(pDataSecond,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowSecond && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}


	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowsetLocate);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc buffered update mode.  Create two rowsets on the same table.  The second row set changed a row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OrderedBookmark_Keyset::Variation_2()
{
	HROW			*pHRow				= NULL;
	HROW			*pHRowSecond		= NULL;
	DBCOUNTITEM		cRows				= 0;
	DBBOOKMARK		DBBookmark			= DBBMK_FIRST;
	BYTE			*pBookmark			= (BYTE *)&DBBookmark;
	DBCOUNTITEM		cCol				= 0;
	DBORDINAL		*rgColNumber		= NULL;
	void			*pData				= NULL;
	void			*pDataSecond		= NULL;
	IRowsetInfo		*pIRowsetInfo		= NULL;
	IRowsetLocate	*pIRowsetLocate		= NULL;
	ICommand		*pICommand			= NULL;
	IUnknown        *pIRowset			= NULL;
	DBPROPID		rgPropertyIDs[4];
	BOOL			fTestPass			= TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
	rgPropertyIDs[3]=DBPROP_ORDEREDBOOKMARKS;


	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	//get an array of numeric and updatable columns
	GetNumericAndUpdatable(&cCol,&rgColNumber);

	//has to find such a column
	if(!cCol)
		goto CLEANUP;

	ReleaseRowsetAndAccessor();

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber));

	if ((!m_pIRowsetChange) || (!m_pIRowsetLocate) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get the 20th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,10,1,&cRows,&pHRow),S_OK);
		
	//get the 1st row handle by 1st bookmark
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
		
	//create a new data buffer to set the data for the numeric and updatable//column
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,1,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//create another rowset object on the same command object
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);
	
	if(!m_pIDBCreateCommand)
		return TEST_PASS;

	//get the ICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand,(IUnknown **)&pICommand),S_OK);
		

	//exeute the same command object
	TESTC_(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,
		&pIRowset),S_OK);
	
	//QI for IRowsetLocate pointer on the second rowset
	if(!COMPARE(((IRowset *)pIRowset)->QueryInterface(IID_IRowset,
		(LPVOID *)&pIRowsetLocate),S_OK))
		goto CLEANUP;

	//set data on the first rowset
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
	
	//allocat memory of pDataSecond
	if(!(pDataSecond=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	//get the 1st row handle by 1st bookmark on the second rowset by its bookmark
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRowSecond),S_OK);
		
	//get data on the second rowset.  Share the same accessor on the command
	TESTC_(((IRowset *)(pIRowset))->GetData(*pHRowSecond,m_hAccessor,pDataSecond),S_OK);
		
	//the second rowset should see the change
	if(COMPARE(CompareBuffer(pDataSecond, pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	if(rgColNumber)
		PROVIDER_FREE(rgColNumber);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(pHRowSecond && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataSecond);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowsetLocate);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);

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
BOOL OrderedBookmark_Keyset::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Invalid_Keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Invalid_Keyset_Cursor_Immediate - Invalid_Keyset_Cursor_Immediate
//|	Created:			04/16/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Cursor_Immediate::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass = TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	if(!TCIRowsetChange::Init())
		return FALSE;

		//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	fTestPass	= TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The accessor is DBACCESSOR_READ | DBACCCESOR_PASSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_1()
{
	BOOL		fTestPass	= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow		= NULL;
	void		*pData		= NULL;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fGetAccessor;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
	
	HRESULT hr = S_OK;

	if (!GetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO,	m_pIDBInitialize, VARIANT_TRUE))
	{
		goto CLEANUP;
	}

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;
	
	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	//create an accessor on the command object on updatable columns
	fGetAccessor = GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,
		DBACCESSOR_PASSBYREF | DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND);
	if (fGetAccessor==TEST_SKIPPED)
	{
		goto CLEANUP;
	}
	TESTC_PROVIDER(fGetAccessor);

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		

	//insert should fail
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),DB_E_BADACCESSORTYPE);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//set data should fail
	if(CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,pData),DB_E_BADACCESSORTYPE))
	{
		fTestPass=TRUE;
	}
CLEANUP:

	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_2()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL. DB_E_BADSTATUSVALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_3()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	DBORDINAL	rgColNumber[2];
	HROW		*pHRow			= NULL;
	void		*pData			= NULL;
	void		*pGetData		= NULL;
	DBPROPID	rgPropertyIDs[3];
	DBSTATUS	dbsSecondColStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	HRESULT hr = S_OK;

	rgColNumber[0]=1;
	rgColNumber[1]=3;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	 
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	
	
	//create an accessor on the command object on updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
									USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,
									2,rgColNumber))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the second status binding to anything other than _OK or NULL
//	*(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[0].obStatus)=DBSTATUS_S_TRUNCATED;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_TRUNCATED;
	
	// keep track of 2nd colstatus
//	dbsSecondColStatus = *(DBSTATUS *)(((DWORD)pData)+	m_rgBinding[1].obStatus);
	dbsSecondColStatus = STATUS_BINDING(m_rgBinding[0],pData);
	
	//set data should fail
	m_hr=SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE) )
		goto CLEANUP;
	
	TESTC_(GetStatus(pData, &(m_rgBinding[0])),DBSTATUS_E_BADSTATUS);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRow);

	if ( m_hr == DB_S_ERRORSOCCURRED )
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[1])),dbsSecondColStatus);
		}
	}
	else
	{
		if(m_cBinding > 1)
		{
			TESTC_(GetStatus(pData, &(m_rgBinding[1])),DBSTATUS_E_UNAVAILABLE);
		}
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
		{
			goto CLEANUP;
		}
		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//columns should not be changed
		if(!COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;

CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_4()
{
	BOOL		fTestPass	= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow		= NULL;
	IAccessor	*pIAccessor	= NULL;
	HACCESSOR	hAccessor	= NULL;
	DBORDINAL	iOrdinal;
	void		*pData		= NULL;
	void		*pGetData	= NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create an accessor on the command object on updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
									UPDATEABLE_COLS_BOUND))

	//save the old column number
	iOrdinal=m_rgBinding[m_cBinding-1].iOrdinal;
	m_rgBinding[m_cBinding-1].iOrdinal=m_cRowsetCols+10;

	//QI for IAccessor pointer on the rowset object
	TESTC_(m_pIRowset->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);
		
	if(FAILED(m_hr=pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,0,&hAccessor,NULL)))
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//copy back the column number
	m_rgBinding[m_cBinding-1].iOrdinal=iOrdinal;

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data should fail
	TESTC_(m_hr=SetData(*pHRow,hAccessor,pData),DB_E_BADORDINAL);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
		goto CLEANUP;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
	//data should not be changed
	if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	if(hAccessor)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				
	SAFE_RELEASE(pIAccessor);
	m_hAccessor=NULL;

	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	
	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
		
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set a duplicate column on which a unique index is created. 	DB_E_INTEGRITYVIOLATION.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_5()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow=NULL;
	DBORDINAL		rgColNumber[1];
	void			*pData=NULL;
	void			*pGetData=NULL;
	DBPROPID		rgPropertyIDs[3];
	HROW			*pUpdatedHRow=NULL;
	DBROWSTATUS		*pDBRowStatus=NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;
		
	HRESULT hr = S_OK;
	
	if(!m_pIDBCreateCommand || !m_bIndexExists)
		return TEST_PASS;

	rgColNumber[0]=g_pTable->GetIndexColumn();

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create an accessor on the command object on the 1st columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
										1,rgColNumber))
										
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get the 2nd row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get the previous (1st) row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data (first row) should be ok
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;
	
	//get a (2nd) row handle - again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//set this time (the 2nd time) should fail
	hr=SetData(*pHRow,m_hAccessor,pData);	

	if (DB_E_ERRORSOCCURRED==hr)
	{
		if(!COMPARE(GetStatus(pData,&(m_rgBinding[0])),DBSTATUS_E_INTEGRITYVIOLATION))
			goto CLEANUP;
	}
	else if (DB_E_INTEGRITYVIOLATION!=hr)
	{
		odtLog << "INTEGRITYVIOLATION ERROR"<<ENDL;
		goto CLEANUP;
	}
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
		goto CLEANUP;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
	// data should not be changed
	if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_6()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW*		pHRow			= NULL;
	DBCOUNTITEM	ulCol			= 0;
	DBORDINAL*	rgColNumber		= 0;
	void*		pData			= NULL;
	void*		pGetData		= NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
   	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	//Get Not nullable and Updatable column.  .
	if(!GetNotNullableAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog<<L"There are no Updateble, not null columns, test Skipped."<<ENDL;	
		goto CLEANUP;
	}

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;

	//create an accessor on the command object on the nullable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		1,rgColNumber))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set the not nullable column to NULL
//	*(DBSTATUS *)(((DWORD)pData)+m_rgBinding[0].obStatus)=DBSTATUS_S_ISNULL;
	*(DBSTATUS*)&STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_ISNULL;

	//set data should fail
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
	{
		goto CLEANUP;
	}
	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
	//no data should be changed
	if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(rgColNumber);

	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_7()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cCnt;
	DBCOUNTITEM		ulCol;
	DBORDINAL		*rgColNumber = NULL;
	DBORDINAL		rgColToBind[2];
	void			*pData=NULL;
	void			*pGetData=NULL;
	DBPROPID		rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
 	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	rgColToBind[0]=1;
	rgColToBind[1]=1;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}	

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(	((m_rgInfo[rgColNumber[cCnt]-1].wType & DBTYPE_ARRAY) == 0) &&
			((m_rgInfo[rgColNumber[cCnt]-1].wType & DBTYPE_VECTOR) == 0)
		  )
		{
		  break;
		}
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
		
	//if this is a bookmarked rowset account for the first ordinal being zero
	if (m_rgInfo[0].iOrdinal==1)
	{
		rgColNumber[cCnt]--;
	}
	rgColToBind[1]=m_rgInfo[rgColNumber[cCnt]].iOrdinal;

	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		2,rgColToBind))

	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the length to cbMaxLen + 1
//	*(ULONG *)((DWORD)pData+m_rgBinding[1].obLength)=m_rgBinding[1].cbMaxLen+1;
	LENGTH_BINDING(m_rgBinding[1],pData)=m_rgBinding[1].cbMaxLen+1;

	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);
	
	if (DB_E_ERRORSOCCURRED==hr)
	{
//		if(!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus),DBSTATUS_E_CANTCONVERTVALUE))
		if(!COMPARE(STATUS_BINDING(m_rgBinding[1],pData),DBSTATUS_E_CANTCONVERTVALUE))
		{
			goto CLEANUP;
		}
		//check the status for the 1st column
//		if(!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus),DBSTATUS_E_UNAVAILABLE))
		if(!COMPARE(STATUS_BINDING(m_rgBinding[0],pData),DBSTATUS_E_UNAVAILABLE))
		{
			goto CLEANUP;
		}
	}
	else
	{
		if (DB_S_ERRORSOCCURRED==hr)
		{
//			if(!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus),DBSTATUS_E_CANTCONVERTVALUE))
			if(!COMPARE(*(DBSTATUS*)&STATUS_BINDING(m_rgBinding[1],pData),DBSTATUS_E_CANTCONVERTVALUE))
			{
				goto CLEANUP;
			}

			//check the staus for the 1st column
//			if(!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus),DBSTATUS_S_OK))
			if(!COMPARE(STATUS_BINDING(m_rgBinding[0],pData),DBSTATUS_S_OK))
			{
				goto CLEANUP;
			}
		}
		else
		{
			goto CLEANUP;
		}
	}

	//release the row handle
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
	{
		goto CLEANUP;
	}

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//Make sure no row is inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
		
	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has length binding.  E_FAIL.(See testing issue #2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_8()
{
	BOOL		fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount=0;
	HROW		*pHRow=NULL;
	void		*pData=NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;	 

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);		
	
	//acceesor with only length binding
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
									DBPART_LENGTH,UPDATEABLE_COLS_BOUND))

	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
	//set data should fail
	if(CHECK(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED))
	{
		//can't check status, it wasn't bound
		fTestPass=TRUE;
	}

CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status and length binding.  Some the columns are not set 	to NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_9()
{
	BOOL		fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount=0;
	HROW		*pHRow=NULL;
	void		*pData=NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//acceesor with only length binding
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
										DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND))

	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK;
		LENGTH_BINDING(m_rgBinding[cCount],pData)=2;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}

	//set data should fail
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),DBSTATUS_E_UNAVAILABLE))
		{
			goto CLEANUP;
		}
	}

	//release the row handle
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	
	ReleaseAccessorOnRowset();

	//get accessor on the rowset for length binding only
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,
					 DBACCESSOR_ROWDATA,DBPART_LENGTH,UPDATEABLE_COLS_BOUND))

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
		LENGTH_BINDING(m_rgBinding[cCount],pData)=2;
	}

	//set data should fail
	if(CHECK(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED))
	{
		//can't chceck status, onlylength is bound here
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_10()
{
	BOOL		fTestPass				= TEST_SKIPPED;
	DBCOUNTITEM	cRows					= 0;
	HROW		*pHRow					= NULL;
	DBCOUNTITEM	cNotUpdateCol			= 0;
	DBCOUNTITEM	*rgNotUpdateColNumber	= NULL;
	DBCOUNTITEM	cCol					= 0;
	DBORDINAL	*rgColNumber			= NULL;
	ULONG		cCount					= 0;
	ULONG		cBinding				= 0;
	void		*pData					= NULL;
	void		*pGetData				= NULL;
	DBPROPID	rgPropertyIDs[3];
	DBCOUNTITEM	i						= 0;
	DBCOUNTITEM	j						= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//Get Non Updatable column.  If all columns are updatable, exit.
	GetNotUpdatable(&cNotUpdateCol,&rgNotUpdateColNumber);

	if(cNotUpdateCol==0)
	{
		fTestPass=TEST_SKIPPED;
		goto CLEANUP;
	}

	//get an array of columns that exclude the 1st column
	if(!GetAllButFirst(&cCol, &rgColNumber))
	{
		goto CLEANUP;
	}
	//create an accessor on the rowset object on the all the columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
										DBTYPE_EMPTY, cCol, rgColNumber))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);

	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
				(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//fill input bindings will set the staus of read-only cols to DBSTATUS_S_IGNORE
	//change it back to S_OK so we try to bind to the read-only columns
	for (i=0;i<cCol;i++)
	{
		for (j=0;j<cNotUpdateCol;j++)
		{
			if (m_rgBinding[i].iOrdinal==rgNotUpdateColNumber[j])
			{
//				*(ULONG *)((DWORD)pData+m_rgBinding[i].obStatus)=DBSTATUS_S_OK;
				STATUS_BINDING(m_rgBinding[i],pData)=DBSTATUS_S_OK;
			}
		}
	}
		
	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);

	if ( !COMPARE(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED, TRUE ) )
	{
		goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			
	PROVIDER_FREE(pHRow);
	
	if ( hr == DB_E_ERRORSOCCURRED )
	{
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
		{
			goto CLEANUP;
		}
		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//no data should be changed
		if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			fTestPass=TRUE;
		}
	}
	else
	{
		// check status of each not updateable column
		for (i=0;i<cCol;i++)
		{
			for (j=0;j<cNotUpdateCol;j++)
			{
				if (m_rgBinding[i].iOrdinal==rgNotUpdateColNumber[j])
				{
//					if (!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[i].obStatus), DBSTATUS_E_PERMISSIONDENIED))
					if (!COMPARE(STATUS_BINDING(m_rgBinding[i],pData), DBSTATUS_E_PERMISSIONDENIED))
					{
						goto CLEANUP;
					}
				}
			}
		}
	}
	fTestPass=TEST_PASS;
CLEANUP:
	CoTaskMemFree(pData); pData = NULL; //PROVIDER_FREE(pData);
	PROVIDER_FREE(pGetData);	
	
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(rgNotUpdateColNumber);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Setting a bookmark column DBSTATUS_E_?
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_11()
{
	BOOL		fTestPass				= TEST_SKIPPED;
	DBCOUNTITEM	cRows					= 0;
	ULONG		cCount					= 0;
	HROW		*pHRow					= NULL;
	DBCOUNTITEM	cNotUpdateCol			= 0;
	DBCOUNTITEM	*rgNotUpdateColNumber	= NULL;
	DBCOUNTITEM	cCol					= 0;
	DBORDINAL	rgColNumber[1];
	void		*pData					= NULL;
	void		*pGetData				= NULL;
	DBPROPID	rgPropertyIDs[4];
	DBPROPID	rgUnPropertyIDs[1];
	DWORD		dwStatus				= DBSTATUS_S_OK;	 

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetLocate;
	rgPropertyIDs[2]=DBPROP_BOOKMARKS;
	rgPropertyIDs[3]=DBPROP_OTHERUPDATEDELETE;

	rgUnPropertyIDs[0]=DBPROP_COLUMNRESTRICT;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,1,rgUnPropertyIDs,NO_ACCESSOR));

	//Get Non Updatable column.  If all columns are updatable, exit.
	GetNotUpdatable(&cNotUpdateCol,&rgNotUpdateColNumber);
	if(cNotUpdateCol==0)
		goto CLEANUP;

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//set array for the bookmark col
	rgColNumber[0]=0;
	cCol=1;

	//create an accessor on the rowset object on the all the columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	 FALSE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		DBTYPE_EMPTY, cCol, rgColNumber))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);

	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//make new data to set
	if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	memset(pData,0,(size_t)m_cRowSize);

//	*(ULONG *)((DWORD)pData+m_rgBinding[0].obStatus)	= DBSTATUS_S_OK;
//	*(ULONG *)((DWORD)pData+m_rgBinding[0].obValue)		= 2525;
//	*(ULONG *)((DWORD)pData+m_rgBinding[0].obLength)	= 4;
	*(DBBKMARK*)&VALUE_BINDING(m_rgBinding[0], pData)=2525;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_OK;
	LENGTH_BINDING(m_rgBinding[0],pData)=4;
		
	//set data should fail
	if(FAILED(m_hr=SetData(*pHRow,m_hAccessor,pData)))
	{	 
//		dwStatus=*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus);
		dwStatus=STATUS_BINDING(m_rgBinding[0],pData);

		if (COMPARE(m_hr == DB_E_ERRORSOCCURRED || m_hr == DB_S_ERRORSOCCURRED, TRUE))
		{
			//not sure what the error status should be but i'll use INTEGRITYVIOLATION for the time being
			if(!COMPARE(dwStatus,DBSTATUS_E_PERMISSIONDENIED))
			{
				goto CLEANUP;
			}
		}
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);		
	
	PROVIDER_FREE(pHRow);
	
	fTestPass=TRUE;

CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(rgNotUpdateColNumber);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc asking for no REMOVEDELETED on dynamic cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_12()
{
	BOOL		fTestPass				=TEST_PASS;
	DBPROPID	rgPropertyIDs[2];
	DBPROPID	rgUnPropertyIDs[1];
	DWORD		dwStatus				=DBSTATUS_S_OK;	 

	rgPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,1,rgUnPropertyIDs,NO_ACCESSOR));

	//DBPROP_OTHERINSERT gets a dynamic cursor
	//DBPROP_REMOVEDELETED must be TRUE if there is a dynamic cursor
	//so if we have a rowset with the props requested there is an error
	fTestPass	= TEST_FAIL;
CLEANUP:
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Cursor_Immediate::Terminate()
{

	ReleaseRowsetAndAccessor();
	return(TCIRowsetChange::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Invalid_Keyset_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Invalid_Keyset_Cursor_Buffered - Invalid_Keyset_Cursor_Buffered
//|	Created:			04/16/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Cursor_Buffered::Init()
{
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass = TEST_SKIPPED;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	if(!TCIRowsetChange::Init())
		return FALSE;
	
	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	fTestPass = TRUE;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The Accessor is read-only accessor. DB_E_READONLYACCESSOR or E_FAIL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_1()
{
	BOOL	fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW	*pHRow=NULL;
	void	*pData=NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_BYREFACCESSORS;
	
	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	fTestPass=FALSE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor on the command object on updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,
		DBACCESSOR_PASSBYREF | DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//insert should fail
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),DB_E_BADACCESSORTYPE);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRow),S_OK);
		
	//set data should fail
	if(CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,pData),DB_E_BADACCESSORTYPE))
		fTestPass=TRUE;
CLEANUP:

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc The accessor is DBACCESSOR_READ | DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR or E_FAIL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_2()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set an auto increment column.  DB_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_3()
{
	BOOL		fTestPass				= TEST_SKIPPED;
	DBCOUNTITEM	cRows					= 0;
	ULONG		cCount					= 0;
	HROW		*pHRow					= NULL;
	HROW		*pUpdatedHRow			= NULL;
	DBROWSTATUS	*pDBRowStatus			= NULL;
	DBCOUNTITEM	cNotUpdateCol			= 0 ;
	DBCOUNTITEM	*rgNotUpdateColNumber	= NULL;
	DBCOUNTITEM	cCol					= 0;
	DBORDINAL	*rgColNumber			= NULL;
	void		*pData					= NULL;
	void		*pGetData				= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBCOUNTITEM	i						= 0;
	DBCOUNTITEM	j						= 0;


	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR));

	//Get Non Updatable column.  If all columns are updatable, exit.
	GetNotUpdatable(&cNotUpdateCol,&rgNotUpdateColNumber);
	if(cNotUpdateCol==0)
	{	
		goto CLEANUP;
	}
	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//get an array of columns that exclude the 1st column
	if(!GetAllButFirst(&cCol, &rgColNumber))
	{
		goto CLEANUP;
	}
	//create an accessor on the rowset object on the all the columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	 FALSE,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		DBTYPE_EMPTY, cCol, rgColNumber))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);

	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
				(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//fill input bindings will set the staus of read-only cols to DBSTATUS_S_IGNORE
	//change it back to S_OK so we try to bind to the read-only columns
	for (i=0;i<cCol;i++)
	{
		for (j=0;j<cNotUpdateCol;j++)
		{
			if (m_rgBinding[i].iOrdinal==rgNotUpdateColNumber[j])
			{
//				*(ULONG *)((DWORD)pData+m_rgBinding[i].obStatus)=DBSTATUS_S_OK;
				STATUS_BINDING(m_rgBinding[i],pData)=DBSTATUS_S_OK;
			}
		}
	}

	//set data should fail
	if(FAILED(m_hr=SetData(*pHRow,m_hAccessor,pData)))
	{	 
		if ( !COMPARE(m_hr == DB_S_ERRORSOCCURRED || m_hr == DB_E_ERRORSOCCURRED, TRUE ) )
		{
			goto CLEANUP;
		}
	}
	else
	{
		//update should fail
		hr=m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus);
		if ( !COMPARE(hr == DB_E_ERRORSOCCURRED || hr == DB_S_ERRORSOCCURRED, TRUE) )
		{
			if(!COMPARE(*pDBRowStatus,DBROWSTATUS_E_INTEGRITYVIOLATION))
				goto CLEANUP;
		}
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	
	if ( m_hr == DB_E_ERRORSOCCURRED )
	{
		//allocate memory for getting data again
		pGetData=PROVIDER_ALLOC(m_cRowSize);
		if(!pGetData)
		{
			goto CLEANUP;
		}
		//restart position
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
		{
			goto CLEANUP;
		}
		//get a row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
		
		//get data on the row handle
		TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
		//no data should be changed
		if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		{
			fTestPass=TRUE;
		}
	}
	else
	{
		// check status of each not updateable column
		for (i=0;i<cCol;i++)
		{
			for (j=0;j<cNotUpdateCol;j++)
			{
				if (m_rgBinding[i].iOrdinal==rgNotUpdateColNumber[j])
				{
//					if (!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[i].obStatus), DBSTATUS_E_PERMISSIONDENIED))
					if (!COMPARE(STATUS_BINDING(m_rgBinding[i],pData), DBSTATUS_E_PERMISSIONDENIED))
					{
						goto CLEANUP;
					}
				}
			}
		}
	}
	fTestPass=TEST_PASS;
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	PROVIDER_FREE(pGetData);	
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(rgNotUpdateColNumber);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc The status flag specified in a binding structure is MAX(DWORD)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_4()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM	cRows				= 0;
	HROW*		pHRow				= NULL;
	void*		pData				= NULL;
	DBPROPID	rgPropertyIDs[2];
	ULONG		cCount				= 0;
	HRESULT		hr					= S_OK;
	DBCOUNTITEM	ulIndex				= 0;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;


	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,NO_ACCESSOR));
	
	if (!m_pIRowsetChange||!m_pIRowsetUpdate)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		

	//create an accessor on the rowset object on updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND))

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the first status binding to anything other than _OK or NULL
//	*(ULONG *)(((DWORD)pData)+	m_rgBinding[3].obStatus)=MAX_ULONG;
	STATUS_BINDING(m_rgBinding[3],pData)=MAX_ULONG;

	//set data should fail
	hr = SetData(*pHRow,m_hAccessor,pData);

	for (cCount=0;cCount<m_cBinding;cCount++)
	{
		if (DB_E_ERRORSOCCURRED==hr)
		{
//			if( *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)!=DBSTATUS_E_CANTCONVERTVALUE &&
//				*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)!=DBSTATUS_E_UNAVAILABLE )
			if	(	DBSTATUS_E_CANTCONVERTVALUE	!= STATUS_BINDING(m_rgBinding[1],pData)	&&
					DBSTATUS_E_UNAVAILABLE		!= STATUS_BINDING(m_rgBinding[0],pData))
			{
				goto CLEANUP;
			}
		}
		else
		{
			if (DB_S_ERRORSOCCURRED==hr)
			{
//				if( *(DBSTATUS *)((DWORD)pData+m_rgBinding[1].obStatus)!=DBSTATUS_E_CANTCONVERTVALUE &&
//					*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)!=DBSTATUS_S_OK )
				if	(	DBSTATUS_E_CANTCONVERTVALUE	!= STATUS_BINDING(m_rgBinding[1],pData)	&&
						DBSTATUS_S_OK				!= STATUS_BINDING(m_rgBinding[0],pData))
				{
					goto CLEANUP;
				}
			}
			else
			{
				goto CLEANUP;
			}
		}
	}
		
	if(!COMPARE(GetStatus(pData,&(m_rgBinding[3])),DBSTATUS_E_BADSTATUS))
	{
		goto CLEANUP;
	}
	//if some columns were set expect a status of OK for all but the bad status column
	if (DB_S_ERRORSOCCURRED==hr)
	{
		//the status for the rest
		for(ulIndex=m_cBinding; ulIndex>0; ulIndex--)
		{
			if(ulIndex==4)
			{
				continue;
			}
			if(!COMPARE(GetStatus(pData,&(m_rgBinding[ulIndex-1])),DBSTATUS_S_OK))
			{
				goto CLEANUP;
			}
		}
	}
	//if no columns were set expect a status of UNAVAILABLE for all but the bad status column
	if (DB_E_ERRORSOCCURRED==hr)
	{
		//the status for the rest
		for(ulIndex=m_cBinding; ulIndex>0; ulIndex--)
		{
			if(ulIndex==4)
			{
				continue;
			}
			if(!COMPARE(GetStatus(pData,&(m_rgBinding[ulIndex-1])),DBSTATUS_E_UNAVAILABLE))
			{
				goto CLEANUP;
			}
		}
	}

	fTestPass=TRUE;
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Change the primary key of two rows to be the same. 	DB_E_INTEGRITYVIOLATION.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_5()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		rgColNumber[2];
	HROW			*pHRow=NULL;
	HROW			*pUpdatedHRow=NULL;
	DBROWSTATUS		*pDBRowStatus=NULL;
	void			*pData=NULL;
	void			*pGetData=NULL;
	DBPROPID		rgPropertyIDs[2];
	DBPROPID		rgPropertyIDsOff[1];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	rgPropertyIDsOff[0]=DBPROP_REMOVEDELETED;

	HRESULT hr = S_OK;

	if(!m_pIDBCreateCommand || !m_bIndexExists)
	{
		return TEST_PASS;
	}
	rgColNumber[0]=g_pTable->GetIndexColumn();

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,1,rgPropertyIDsOff,NO_ACCESSOR))
	
	//create an accessor on the rowset object on 1st column twice
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
										1,rgColNumber))

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get handle to 2nd row
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
  
	//get the previous (1st) row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//set data (first row) should be ok
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),S_OK);

	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;
	
	//get a (2nd) row handle - again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//set this time (the 2nd time) should pass
	TESTC_(SetData(*pHRow,m_hAccessor,pData),S_OK);	
	//update should fail when data is actually pushed to the back end
	hr=m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus);

	if (DB_E_ERRORSOCCURRED==hr)
	{
		if(!COMPARE(*pDBRowStatus,DBROWSTATUS_E_INTEGRITYVIOLATION))
		{
			goto CLEANUP;
		}
	}
	else if (DB_E_INTEGRITYVIOLATION!=hr)
	{
		odtLog << "INTEGRITYVIOLATION ERROR"<<ENDL;
		goto CLEANUP;
	}

	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	//restart position
	TESTC_(m_pIRowset->RestartPosition(NULL),DB_E_ROWSNOTRELEASED);
	//update failed so there are still pending changes.  undo them so test can RestartPosition
	TESTC_(m_pIRowsetUpdate->Undo(NULL,1,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),S_OK);
	//release the row handle so test can RestartPosition
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
		goto CLEANUP;

	//get a row handle
	m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow);
	GetData(*pHRow, m_hAccessor, pGetData);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
	
	//the 1st column should not be changed
	if(COMPARE(CompareBuffer(pGetData, m_pData,1,&(m_rgBinding[0]),m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;	
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHEMAVIOLATION.  The row goes back to its original sta
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_6()
{ 
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	HROW			*pUpdatedRow	= NULL;
	DBROWSTATUS		*pDBRowStatus	= NULL;
	DBCOUNTITEM		ulCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	ULONG			cCount			= 0;
	void			*pData			= NULL;
	void			*pGetData		= NULL;
	DBPROPID		rgPropertyIDs[2];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR))

	//Get Not nullable and Updatable column.  .
	if(!GetNotNullableAndUpdatable(&ulCol, &rgColNumber))
		goto CLEANUP;

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass=FALSE;

	//accessor on not nullable and updatable column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
										1,rgColNumber))

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
	
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
	
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, SECONDARY),S_OK);
		
	//set the not nullable column to NULL
//	*(DBSTATUS *)(((DWORD)pData)+m_rgBinding[0].obStatus)=DBSTATUS_S_ISNULL;
	STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_ISNULL;

	//set data should fail
	if(FAILED(m_hr=SetData(*pHRow,m_hAccessor,pData)))
	{	 
		COMPARE(m_hr, DB_E_ERRORSOCCURRED);

		if(!COMPARE(GetStatus(pData,&(m_rgBinding[0])),DBSTATUS_E_INTEGRITYVIOLATION))
			goto CLEANUP;  
	}
	else
	{
		//update should fail
		TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,&cRows,&pUpdatedRow,&pDBRowStatus),DB_E_ERRORSOCCURRED);
			
		if(!COMPARE(*pDBRowStatus,DBROWSTATUS_E_INTEGRITYVIOLATION))
		{
			goto CLEANUP;
		}	
		COMPARE(*pDBRowStatus,DBROWSTATUS_S_PENDINGCHANGES);
	}
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//undo all the updates
	TESTC_(m_pIRowsetUpdate->Undo(NULL,0,NULL,NULL,NULL,NULL),S_OK);
		
	//allocate memory for getting data again
	pGetData=PROVIDER_ALLOC(m_cRowSize);
	if(!pGetData)
		goto CLEANUP;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//get data on the row handle
	TESTC_(GetData(*pHRow, m_hAccessor, pGetData),S_OK);
		
	//no data should be changed
	if(COMPARE(CompareBuffer(pGetData, m_pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),TRUE))
		fTestPass=TRUE;
CLEANUP:	
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	PROVIDER_FREE(pGetData);
	PROVIDER_FREE(pUpdatedRow);
	PROVIDER_FREE(pDBRowStatus);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc The length binding > cbMaxLen for a variable length column.  	DB_S_ERRORSOCCURRED.  Truncation should occur. Check the status b
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_7()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow=NULL;
	DBCOUNTITEM		cCnt;
	DBCOUNTITEM		ulCol;
	DBORDINAL		*rgColNumber = NULL;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[2];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	HRESULT hr = S_OK;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR))

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//Get a variable length column.  .
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber,FALSE))
	if(!GetVariableLengthAndUpdatable(&ulCol, &rgColNumber))
	{
		odtLog << "no variable length updateable columns available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}

	//find a data type that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR
	for(cCnt=0; cCnt<ulCol; cCnt++)
	{
		if(((m_rgInfo[rgColNumber[cCnt]-1].wType & DBTYPE_ARRAY) == 0) &&
			((m_rgInfo[rgColNumber[cCnt]-1].wType & DBTYPE_VECTOR) == 0)
		  )
		  break;
	}

	if(cCnt==ulCol)
	{
		odtLog << "no variable length updateable columns that is not ORed with DBTYPE_ARRAY or DBTYPE_VECTOR available"<<ENDL;
		fTestPass = TRUE;
		goto CLEANUP;
	}
		
	//accessor on 1st variable length and updatable column 
	TESTC_PROVIDER(	GetAccessorOnRowset(	ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
											DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
											USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
											1,((&rgColNumber[cCnt]))
										))
					
	//restartPosition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,4,1,&cRows,&pHRow),S_OK);
		
	//make new data to set
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		
	//change the length to cbMaxLen + 1
//	*(ULONG *)((DWORD)pData+m_rgBinding[0].obLength)=m_rgBinding[0].cbMaxLen+1;
	LENGTH_BINDING(m_rgBinding[0],pData)=m_rgBinding[0].cbMaxLen+1;

	//set data should fail
	if((m_hr=SetData(*pHRow,m_hAccessor,pData))==DB_E_ERRORSOCCURRED)
	{	 
//		if(!COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus), DBSTATUS_E_CANTCONVERTVALUE))
		if(!COMPARE(STATUS_BINDING(m_rgBinding[0],pData), DBSTATUS_E_CANTCONVERTVALUE))
		{
			goto CLEANUP;
		}
	}
	else
	{
		//has to be successful code
		COMPARE(m_hr, ResultFromScode(S_OK));

		//update should fail
		TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),DB_E_ERRORSOCCURRED);
	}

	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_8()
{
	BOOL		fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cCount;
	HROW		*pHRow=NULL;
	void		*pData=NULL;
	DBPROPID	rgPropertyIDs[2];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

	//create a rowset without an accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,NO_ACCESSOR))

	if (!m_pIRowsetChange||!m_pIRowsetUpdate)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		
	//accessor with only status binding
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
									DBPART_STATUS,UPDATEABLE_COLS_BOUND))

	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}
	//set data should fail
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_ERRORSOCCURRED);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),DBSTATUS_E_UNAVAILABLE))
		 goto CLEANUP;
	}


	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc The accessor is not a row accessor.  DB_E_BADACCESSORTYPE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_9()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	DBPROPID	rgPropertyIDs[2];
	void		*pData			= NULL;
	 
	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,FALSE,
										DBACCESSOR_ROWDATA,DBPART_LENGTH|DBPART_STATUS|DBPART_VALUE,
										UPDATEABLE_COLS_BOUND,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
										0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT))

	//create data to set to the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//release the accessor
	ReleaseRowsetAndAccessor();

	if (!m_pIRowsetChange||!m_pIRowsetUpdate)
	{
		goto CLEANUP;
	}
		
	//now make a param accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_COMMAND_ACCESSOR,FALSE,
										DBACCESSOR_PARAMETERDATA,DBPART_LENGTH|DBPART_STATUS|DBPART_VALUE,
										UPDATEABLE_COLS_BOUND,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
										0,NULL,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT))
	fTestPass=FALSE;
										
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//set data should fail
	if(CHECK(m_pIRowsetChange->SetData(*pHRow,m_hAccessor,pData),DB_E_BADACCESSORTYPE))
		fTestPass=TRUE;
  
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELTED FALSE LITERALINDETITY TRUE in buffered mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_10()
{
	DBPROPID	rgPropertyIDs[6];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass				= TEST_SKIPPED;
	HROW		*pHRow					= NULL;
	HROW		*pHRowb					= NULL;
	DBCOUNTITEM	cRows					= 0;
	HRESULT		hr						= S_OK;
	HROW		*pUpdatedHRow			= NULL;
	DBROWSTATUS	*pDBRowStatus			= NULL;
	void		*pData					= NULL;
	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[5]=DBPROP_LITERALIDENTITY;
	
	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange||!m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	TESTC_(GetData(*pHRow,m_hAccessor,pData),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowb),S_OK);
	//get the data for the row, return DB_E_DELETEDROW
	hr=GetData(*pHRowb,m_hAccessor,m_pData);

	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}
	//if the hole was not returned and the row was returned then make sure it returns
	//the correct buffer
	if (S_OK==hr)
	{
		if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
	}

	//these should  be the same rows, should see the hole or the deleted row
	//LITERALIDENTITY TRUE, REMOVEDELETED FALSE 
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowb),S_OK);
	
	//call update, transmit the delete
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);

	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowb,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowb);
	pHRowb=NULL;
	
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the next 5 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,5,&cRows,&pHRow),S_OK);

	//call update for the first 5 rows
	if	(
			!CHECK(m_pIRowsetUpdate->Update(NULL,5,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),DB_S_ERRORSOCCURRED)
		)
	{
		goto CLEANUP;
	}
	
	//only the first row should be marked as deleted since this was the only row transmitted to the data source
	//prior to this update, the fourth row just deleted should not cause an error
	if	(! 
			(COMPARE(pDBRowStatus[0],DBROWSTATUS_E_DELETED)) &&
			(COMPARE(pDBRowStatus[3],DBROWSTATUS_S_OK))
		)
	{
		goto CLEANUP;
	}
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	hr=GetData(*pHRow,m_hAccessor,m_pData);

	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
	//get the data for the row, return DB_E_DELETEDROW
	hr=GetData(*pHRow,m_hAccessor,m_pData);

	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass = TEST_PASS;
CLEANUP:		
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	//release the row handle
	if (m_pIRowset && pHRow)
	{
		//release the row handle
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK)
		PROVIDER_FREE(pHRow);
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELTED FALSE LITERALINDETITY FALSE in buffered mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_11()
{
	DBPROPID	rgPropertyIDs[5];
	DBPROPID	rgUnPropertyIDs[2];
	BOOL		fTestPass				= TEST_SKIPPED;
	HROW		*pHRow					= NULL;
	HROW		*pHRowb					= NULL;
	DBCOUNTITEM	cRows			= 0;
	HRESULT		hr						= S_OK;
	HROW		*pUpdatedHRow			= NULL;
	DBROWSTATUS	*pDBRowStatus			= NULL;
	void		*pData1					= NULL;
	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_IRowsetIdentity;
	
	rgUnPropertyIDs[0]=DBPROP_REMOVEDELETED;
	rgUnPropertyIDs[1]=DBPROP_LITERALIDENTITY;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,2,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange||!m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	if(!(pData1=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	TESTC_(GetData(*pHRow,m_hAccessor,pData1),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowb),S_OK);
	//get the data for the row, return S_OK, should not see the pending change 
	TESTC_(GetData(*pHRowb,m_hAccessor,m_pData),S_OK);

	//the deleted row should be visible here, check its buffer
	if(!CompareBuffer(m_pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		goto CLEANUP;
	}

	//these should be the same rows, 
	//LITERALIDENTITY TRUE, REMOVEDELETED FALSE 
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowb),S_OK);
	
	//call update, transmit the delete
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowb,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowb);
	pHRowb=NULL;
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;


	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	
	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the next 5 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,5,&cRows,&pHRow),S_OK);

	//call update for the first 5 rows
	if	(
			!CHECK(m_pIRowsetUpdate->Update(NULL,5,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),DB_S_ERRORSOCCURRED)
		)
	{
		goto CLEANUP;
	}
	
	//only the first row should be marked as deleted since this was the only row transmitted to the data source
	//prior to this update, the fourth row just deleted should not cause an error
	if	(! 
			(COMPARE(pDBRowStatus[0],DBROWSTATUS_E_DELETED)) &&
			(COMPARE(pDBRowStatus[3],DBROWSTATUS_S_OK))
		)
	{
		goto CLEANUP;
	}
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	hr=GetData(*pHRow,m_hAccessor,m_pData);

	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
	hr=GetData(*pHRow,m_hAccessor,m_pData);

	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass = TEST_PASS;
CLEANUP:		
	if(pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);

	//release the row handle
	if (m_pIRowset && pHRow)
	{
		//release the row handle
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK)
		PROVIDER_FREE(pHRow);
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELTED TRUE, LITERALIDENITTY TRUE in buffered mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_12()
{
	DBPROPID	rgPropertyIDs[7];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass	= TEST_SKIPPED;
	HROW		*pHRow		= NULL;
	HROW		*pHRowb		= NULL;
	DBCOUNTITEM	cRows			= 0;
	DBCOUNTITEM	cUpdatedRows	= 0;
	DBCOUNTITEM	cRowsExpected	= 0;
	HRESULT		hr			= S_OK;
	HROW		*pUpdatedHRow			= NULL;
	DBROWSTATUS	*pDBRowStatus			= NULL;
	void		*pData1					= NULL;
	void		*pData2					= NULL;


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OWNUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[4]=DBPROP_LITERALIDENTITY;
	rgPropertyIDs[5]=DBPROP_IRowsetIdentity;
	rgPropertyIDs[6]=DBPROP_IRowsetUpdate;
	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,7,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if(!(pData1=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	if(!(pData2=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	
	if (!m_pIRowsetChange||!m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowb),S_OK);
	TESTC_(GetData(*pHRowb,m_hAccessor,pData1),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowb, NULL),S_OK);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//get the data for the row, return S_OK 
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//the deleted row should not be visible here, check its buffer
	if(CompareBuffer(m_pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		goto CLEANUP;
	}

	//these should not be the same rows, 
	//LITERALIDENTITY TRUE, REMOVEDELETED TRUE 
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowb),S_FALSE);

	//call update, transmit the delete
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);

	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;
	PROVIDER_FREE(pHRowb);
	pHRowb=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	TESTC_(GetData(*pHRow,m_hAccessor,pData2),S_OK);

	//the deleted row should not be visible here, no hole so the buffer should
	//be from a new row
	if(CompareBuffer(pData1,pData2,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		goto CLEANUP;
	}
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the next 5 row handles, expect 5 rows to be fetched
	cRowsExpected = 5;
	TESTC_(m_pIRowset->GetNextRows(NULL,0,5,&cRows,&pHRow),S_OK);

	//call update for the first 5 rows
	if	(
			!CHECK(m_pIRowsetUpdate->Update(NULL,5,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),S_OK)
		)
	{
		//this is an error, lets do some more debugging
		//no row should be marked as deleted since this was REMOVEDELETED is TRUE
		for(DBCOUNTITEM i=0; i<cRows; i++)
		{
			//no holes from  delete
			COMPARE(pDBRowStatus[0],DBROWSTATUS_S_OK);
		}
		m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL);
		PROVIDER_FREE(pHRow);
		goto CLEANUP;
	}
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//get the data for the row, S_OK because REMOVEDELETED is TRUE
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//get the data for the row, return S_OK because REMOVEDELETED is TRUE
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass = TEST_PASS;
CLEANUP:		
	if(pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}
	PROVIDER_FREE(pData2);
	//release the row handle
	if (m_pIRowset)
	{
		//release the row handle
		m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL);
		PROVIDER_FREE(pHRow);
		m_pIRowset->ReleaseRows(1,pHRowb, NULL,NULL,NULL);
		PROVIDER_FREE(pHRowb);
	}
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELTED TRUE, LITERALIDENITTY FALSE in buffered mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Buffered::Variation_13()
{
	DBPROPID	rgPropertyIDs[6];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass	= TEST_SKIPPED;
	HROW		*pHRow		= NULL;
	HROW		*pHRowb		= NULL;
	DBCOUNTITEM	cRows			= 0;
	HRESULT		hr			= S_OK;
	HROW		*pUpdatedHRow			= NULL;
	DBROWSTATUS	*pDBRowStatus			= NULL;
	void		*pData1					= NULL;
	void		*pData2					= NULL;


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OWNUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_REMOVEDELETED;
	rgPropertyIDs[5]=DBPROP_IRowsetIdentity;

	rgUnPropertyIDs[0]=DBPROP_LITERALIDENTITY;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if(!(pData1=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	if(!(pData2=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	
	if (!m_pIRowsetChange||!m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowb),S_OK);
	TESTC_(GetData(*pHRowb,m_hAccessor,pData1),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowb, NULL),S_OK);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//get the data for the row, return S_OK 
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//the deleted row should  be visible here, check its buffer
	if(!CompareBuffer(m_pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		goto CLEANUP;
	}

	//these should be the same rows, 
	//LITERALIDENTITY FALSE, REMOVEDELETED TRUE 
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRowb),S_OK);
	
	//call update, transmit the delete
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);

	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;
	PROVIDER_FREE(pHRowb);
	pHRowb=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	TESTC_(GetData(*pHRow,m_hAccessor,pData2),S_OK);

	//the deleted row should not be visible here, no hole so the buffer should
	//be from a new row
	if(CompareBuffer(pData1,pData2,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
	{
		goto CLEANUP;
	}
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow, NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the next 5 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,5,&cRows,&pHRow),S_OK);

	//call update for the first 5 rows
	if	(
			!CHECK(m_pIRowsetUpdate->Update(NULL,5,pHRow,&cRows,&pUpdatedHRow,&pDBRowStatus),S_OK)
		)
	{
		//this is an error, lets do some more debugging
		//no row should be marked as deleted since this was REMOVEDELETED is TRUE
		COMPARE(pDBRowStatus[0],DBROWSTATUS_S_OK);//no hole from first delete, should be new row
		COMPARE(pDBRowStatus[1],DBROWSTATUS_S_OK);
		COMPARE(pDBRowStatus[2],DBROWSTATUS_S_OK);
		COMPARE(pDBRowStatus[3],DBROWSTATUS_S_OK);
		COMPARE(pDBRowStatus[4],DBROWSTATUS_S_OK);
		goto CLEANUP;
	}
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 1th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//get the data for the row, S_OK because REMOVEDELETED is TRUE
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	//retrieve the 4th row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
	//get the data for the row, return S_OK because REMOVEDELETED is TRUE
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass = TEST_PASS;
CLEANUP:		
	if(pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}
	PROVIDER_FREE(pData2);
	//release the row handle
	if (m_pIRowset)
	{
		//release the row handle
		m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK;
		PROVIDER_FREE(pHRow);
		m_pIRowset->ReleaseRows(1,pHRowb, NULL,NULL,NULL),S_OK;
		PROVIDER_FREE(pHRowb);
	}
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pDBRowStatus);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Cursor_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetChange::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Valid_Keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Valid_Keyset_Cursor_Immediate - Valid_Keyset_Cursor_Immediate
//|	Created:			04/17/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Valid_Keyset_Cursor_Immediate::Init()
{
	BOOL fTestPass = TEST_SKIPPED;
	
	m_rgPropertyIDs[0]=DBPROP_IRowsetChange;
	m_rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	m_rgPropertyIDs[2]=DBPROP_IRowsetLocate;

	m_cGuid=3;

	if(!TCIRowsetChange::Init())
		return FALSE;

	fTestPass = TRUE;


	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Change fixed length columns, in FORWARD order of the rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_1()
{
	DBCOUNTITEM		cCol = 0;
	DBORDINAL*		rgColNumber=NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW*			pHRow=NULL;
	void*			pData=NULL;
	BOOL			fTestPass=TEST_SKIPPED;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass = FALSE;
   	
	//get the fixed length updatable columns
	if(!GetUpdatableCols(&cCol, &rgColNumber))
		goto CLEANUP;

	//create a write only accessor on the fixed length updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, 
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//create data to set to the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();

	//reexcute the query
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,0,NULL,
		ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))
	
	
	//retrieve the 12 row handle
	while(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow) == S_OK)
	{
		if(cRows == 0)
			break;

		//Get the data for the 10th row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}	
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(m_pData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Change the fixed length data type columns with bogus length information.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_2()
{
	DBCOUNTITEM		cCnt				= 0;
	DBCOUNTITEM		cCol				= 0;
	DBORDINAL		*rgColNumber		= NULL;
	DBCOUNTITEM		cRows				= 0;
	HROW			rghRows[1]			= {NULL};
	HROW*			pHRow				= &rghRows[0];
	void			*pData				= NULL;
	BOOL			fTestPass			= TEST_SKIPPED;
	HRESULT			hr;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,0,NULL,NO_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

   	fTestPass = FALSE;
	
	//get the fixed length updatable columns
	if(!GetFixedLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	//create a write only accessor on the fixed length updatable columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,  
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//create data to set to the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//make the length information bogus
	for(cCnt=0;cCnt<m_cBinding;cCnt++)
	{
//		*(ULONG *)(((DWORD)pData)+m_rgBinding[cCnt].obLength)=0;
		LENGTH_BINDING(m_rgBinding[cCnt],pData)=0;
	}

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);

	if(*pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		hr = GetData(*pHRow,m_hAccessor,m_pData);
		//Taking cares of SQL Providers for keyset cursor
		TESTC(hr == S_OK || hr == DB_E_DELETEDROW);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		*pHRow = NULL;

		//make sure CompareBuffer will not compare length information
		for(cCnt=0;cCnt<m_cBinding;cCnt++)	
		{
			m_rgBinding[cCnt].dwPart=m_rgBinding[cCnt].dwPart&(~DBPART_LENGTH);
		}
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(*pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(m_pData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Change the variable length columns with only value binding.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_3()
{
	DBCOUNTITEM		cCol;
	DBORDINAL		*rgColNumber=NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		=NULL;
	void			*pData		=NULL;
	void			*pSecondData=NULL;
	BOOL			fTestPass	=TEST_SKIPPED;

	HRESULT hr = S_OK;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,
		0,NULL,NO_ACCESSOR));	  
	
   	//get the variable length updatable columns.  No bytes
	if(!GetVariableLengthAndUpdatable(&cCol, &rgColNumber,TRUE,FALSE))
		goto CLEANUP;

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//create a write only accessor on the variable length updatable columns
	//do not include the last column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, 
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,/*cCol-1*/1,rgColNumber))

	//create data to set to the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//set data
	TESTC_(m_pIRowsetChange->SetData(*pHRow, m_hAccessor, pData),S_OK);

	//free the memory
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	//release the accessor
	ReleaseAccessorOnRowset();

    //get the variable length updatable columns
	if(!GetStr(&cCol, &rgColNumber))
		goto CLEANUP;

	//create a write only accessor on the variable length updatable columns
	//do not include the last column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, 
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))

	//create data to set to the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//allocate memory
	if(!(pSecondData=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//set data
	TESTC_(m_pIRowsetChange->SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	//retrieve the 12 row handle
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(pSecondData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Change the whole row with status and value binding.  Set some columns to 	NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_4()
{
	DBCOUNTITEM		cCol			= 0;
	DBCOUNTITEM		cIndex			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	void			*pData			= NULL;
	void			*pSecondData	= NULL;
	BOOL			fTestPass		= TEST_SKIPPED;

	HRESULT hr = S_OK;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,
		0,NULL,NO_ACCESSOR))

   	//get the variable length updatable columns
	if(!GetVariableLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create a write only accessor on the variable length updatable columns
	//do not include the last column
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE, 
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol-1,rgColNumber))

	//create data to set to the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//search for the DBTYPE_STR data
	for(cCol=0; cCol<m_cBinding; cCol++)
	{
		if(m_rgBinding[cCol].wType==DBTYPE_STR)
		{
			cIndex=cCol;
			break;
		}
	}

	//exit if no such column is found
	if(cCol==m_cBinding)
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}

	//add a NULL terminator to the data buffer
//	*((BYTE *)((DWORD)pData+m_rgBinding[cIndex].obValue+4))='\0';
	*((size_t *)&VALUE_BINDING(m_rgBinding[0], pData))='\0';

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//allocate memory
	if(!(pSecondData=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;
	
	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		

	//get data
	TESTC_(GetData(*pHRow, m_hAccessor,pSecondData),S_OK);

	// Make sure data should be changed	except for DBTYPE_BYTES
	if(!memcmp	( 
//					(void *)((DWORD)pData+m_rgBinding[cIndex].obValue),
//			 		(void *)((DWORD)pSecondData+m_rgBinding[cIndex].obValue),
//					*(ULONG *)((DWORD)pData+m_rgBinding[cIndex].obLength)
					&VALUE_BINDING(m_rgBinding[cIndex], pData),
					&VALUE_BINDING(m_rgBinding[cIndex], pSecondData),
					LENGTH_BINDING(m_rgBinding[cIndex],pData)
				)
	   )

	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pSecondData);
	PROVIDER_FREE(pData);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Change a row with DBTYPE_BYREF.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_5()
{
	DBCOUNTITEM		cRows			= 0;
	ULONG			cCount			= 0;
	HROW			*pHRow			= NULL;
	void			*pData			= NULL;
	BOOL			fTestPass		= TEST_SKIPPED;
	DBBINDING		*rgBinding		= NULL;
	DBCOUNTITEM		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		cRow			= g_ulNextRow++;

	HRESULT hr = S_OK;

	//create a rowset and accessor that binds to DBTYPE_BYREF
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR))

	//GetUpdatable columns
	if(!GetUpdatableCols(&cCol, &rgColNumber))
		goto CLEANUP;

	for(cCount=0; cCount<cCol; cCount++)
	{
		TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, 
			DBACCESSOR_ROWDATA,
			DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
			USE_COLS_TO_BIND_ARRAY,
			FORWARD,NO_COLS_BY_REF,DBTYPE_BYREF,
			1,&(rgColNumber[cCount])))

		if (!m_pIRowsetChange)
			goto CLEANUP;

		fTestPass=FALSE;
		
		//get a row handle
		if(cCount==0)
		{
			TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);				
		}
		else
		{
			TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK);
		}

		//create data to set to the last row
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,cRow,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
			
		//set data
		TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);		

		if(pHRow && m_pIRowset)
			CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//restartpostion
		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
			goto CLEANUP;

		if(pData)
		{
			ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData);
			PROVIDER_FREE(pData);
			PROVIDER_FREE(m_pData);
			PROVIDER_FREE(m_rgBinding);
			m_cBinding	= 0;
		}
	}
	
	ReleaseRowsetAndAccessor();

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, TRUE, 
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,
		1,rgColNumber))

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		// the Data should be the same as what was set
		if(CompareData(m_cRowsetCols,m_rgTableColOrds,cRow,m_pData,m_cBinding,m_rgBinding,m_pTable,m_pIMalloc))

		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(rgBinding);
	PROVIDER_FREE(m_pData);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Change a row with DBTYPE_ARRAY.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_6()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Change a row with DBTYPE_VECTOR.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_7()
{
	return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Duplicate column bindings in the same accessor.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_8()
{
	DBCOUNTITEM		cCol;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	DBBYTEOFFSET	obValue;
	void			*pData			= NULL;
	HACCESSOR		hAccessor		= NULL;
	BOOL			fTestPass		= TEST_SKIPPED;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,0,NULL,NO_ACCESSOR))

   	//get the 1st numeric and updatable columns
	if(!GetNumericAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//can not go on testing without more than 1 column in the table
	if(cCol<2)
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}

	//create a write only accessor on the 1st numeric updatable column twice
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,  
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,2,rgColNumber))

	//copy the binding strcture
	m_rgBinding[1].iOrdinal=m_rgBinding[0].iOrdinal;
	m_rgBinding[1].wType=m_rgBinding[0].wType;
	m_rgBinding[1].cbMaxLen=m_rgBinding[0].cbMaxLen;

	//create an accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
		m_cBinding, m_rgBinding, 0, &hAccessor,NULL),S_OK);		

	obValue=m_rgBinding[1].obValue;

	//create data to set to the last row
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//make the second copy of the data into the first half 
	//of pData buffer
	TESTC_(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,1,m_rgBinding,
		(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//set data
	TESTC_(SetData(*pHRow, hAccessor, pData),S_OK);
		
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	//release the accessor
	if(hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
		hAccessor=NULL;
	}

	ReleaseRowsetAndAccessor();

	//reexcute the query
	//create an accessor on the 1st numeric updatable column only once
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cGuid,m_rgPropertyIDs,0,NULL,
		ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,rgColNumber))

	fTestPass=FALSE;
	//get the last row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//get data
	TESTC_(GetData(*pHRow, m_hAccessor,m_pData),S_OK);
		
	// the Data should be the same as what was set for the second time
//	if(COMPARE(CompareDBTypeData(	(void *)((DWORD)m_pData+m_rgBinding[0].obValue),
	if(COMPARE(CompareDBTypeData(	(size_t *)&VALUE_BINDING(m_rgBinding[0], m_pData),
									(void *)((size_t)pData+obValue),
									m_rgBinding[0].wType,
									0,
									0,
									0,
									m_pIMalloc,
									FALSE,
									DBTYPE_EMPTY,
									0),TRUE))
	{
		PROVIDER_FREE(pData);
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(rgColNumber);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//release the accessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Valid_Keyset_Cursor_Immediate::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}



// {{ TCW_TC_PROTOTYPE(Related_Delete_NewRow)
//*-----------------------------------------------------------------------
//| Test Case:		Related_Delete_NewRow - Related interfaces with IRowsetDelete and IRowsetNewRow
//|	Created:			04/22/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_Delete_NewRow::Init()
{
	BOOL fTestPass = TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;


	fTestPass = TRUE;

	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Delete one row.  Add a new row and change it.  Immediate update mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_1()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	HROW			*pHRowNew		= NULL;
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[5];
	ULONG			uRefCount		= 0;
	DBROWSTATUS		DBRowStatus;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[3]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[4]=DBPROP_IRowsetLocate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass	= TEST_FAIL;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,10,1,&cRows, &pHRowDelete),S_OK);
		
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowDelete,NULL),S_OK);
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowDelete,&DBRowStatus),DB_E_ERRORSOCCURRED);

	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_DELETED))
	{
		goto CLEANUP;
	}

	//addRef on a deleted row.  No ref count is added
	TESTC_(m_pIRowset->AddRefRows(1,pHRowDelete,&uRefCount,&DBRowStatus),DB_E_ERRORSOCCURRED);
		
	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_DELETED))
		goto CLEANUP;

	//get data on a deleted row is undefined, but no crash
	m_hr=GetData(*pHRowDelete, m_hAccessor, m_pData);

	//try to change the deleted row
	TESTC_(SetData(*pHRowDelete,m_hAccessor, pData),DB_E_DELETEDROW);
		
	//try to retrieve a deleted row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, -1,&cRows, &pHRowNew),S_OK);

	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);

	//release the deleted row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	
	PROVIDER_FREE(pHRowDelete);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRowNew, NULL,NULL,NULL),S_OK);
	 
	ReleaseRowsetAndAccessor();

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//reexecute the query, 
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND))
	
	
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowNew)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(GetData(*pHRowNew,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRowNew,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowNew);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:	
	//free the memory used by pData
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowDelete);
	}

	if(pHRowNew && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowNew, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowNew);
	}
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete one row.  Add a new row and change it.  Query and buffered update mode.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_2()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	HROW			*pHRowNew		= NULL;
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[5];
	ULONG			uRefCount1		= 0;
	ULONG			uRefCount2		= 0;
	DBROWSTATUS		DBRowStatus;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_CANFETCHBACKWARDS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,
				0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
				DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,10,1,&cRows, &pHRowDelete),S_OK);

	//addRef on a soft deleted row.  Should be OK
	TESTC_(m_pIRowset->AddRefRows(1,pHRowDelete,&uRefCount1,&DBRowStatus),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowDelete,NULL),S_OK);
			
	//addRef on a soft deleted row.  Should be OK
	TESTC_(m_pIRowset->AddRefRows(1,pHRowDelete,&uRefCount2,&DBRowStatus),S_OK);
		
	if(uRefCount2 <= uRefCount1)
		goto CLEANUP;

	if(!COMPARE(DBRowStatus, DBROWSTATUS_S_OK))
		goto CLEANUP;

	//release the row twice  S_OK
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDelete,NULL,&uRefCount1,&DBRowStatus),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDelete,NULL,&uRefCount2,&DBRowStatus),S_OK);
	
	if(1!=uRefCount2)
	{
		odtLog << "warning, reference count mismatch"<<ENDL;
	}
	
	if(!COMPARE(DBRowStatus, DBROWSTATUS_S_PENDINGCHANGES))
		goto CLEANUP;

	//try to change the deleted row
	TESTC_(SetData(*pHRowDelete,m_hAccessor, pData),DB_E_DELETEDROW);
		
	//try to read the deleted row
	GetData(*pHRowDelete,m_hAccessor, m_pData);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL, 1, pHRowDelete,NULL,NULL,NULL),S_OK);
		
	//addRef on a hard deleted row. 
	TESTC_(m_pIRowset->AddRefRows(1,pHRowDelete,&uRefCount1,&DBRowStatus),DB_E_ERRORSOCCURRED);
	
	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_DELETED))
		goto CLEANUP;
	
	//try to retrieve a deleted row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, -1,&cRows, &pHRowNew),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);
		
	//release the deleted row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
		 
	PROVIDER_FREE(pHRowDelete);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRowNew, NULL,NULL,NULL),S_OK);
		 
	ReleaseRowsetAndAccessor();

	//reexecute the query, creaete a read-only accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,
		0,NULL,ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,
		UPDATEABLE_COLS_BOUND))
	
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowNew)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(GetData(*pHRowNew,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRowNew,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowNew);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			break;
		}
	}
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	if(pHRowDelete && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDelete);

	if(pHRowNew && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowNew, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowNew);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete rows from a static cursor/immediate update mode
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_3()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	DBPROPID		rgPropertyIDs[2]= {NULL,NULL};
	DBROWSTATUS		DBRowStatus[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;
	m_fUseCmdTmeOut = TRUE;
	//open a rowset with an accessor on all updatable columns with a static cursor
	TESTC_PROVIDER(	GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,2,g_rgPropertyIDs,
					ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
					DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRowDelete),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRowDelete,DBRowStatus),S_OK);
			
	COMPARE(DBRowStatus[0], DBROWSTATUS_S_OK);
	COMPARE(DBRowStatus[1], DBROWSTATUS_S_OK);
	COMPARE(DBRowStatus[2], DBROWSTATUS_S_OK);

	fTestPass=TRUE;
CLEANUP:
	m_fUseCmdTmeOut = FALSE;
	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowDelete);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary checks from a static cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_4()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	HROW			*pHRowNew		= NULL;
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[2]={NULL,NULL};
	ULONG			uRefCount1		= 0;
	ULONG			uRefCount2		= 0;
	DBROWSTATUS		DBRowStatus[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	TESTC_PROVIDER(	GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,2,g_rgPropertyIDs,
					ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
					DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
			
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRowDelete),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,DBRowStatus),E_INVALIDARG);
			
	//no op
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,pHRowDelete,DBRowStatus),S_OK);
			
	//no op
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,NULL,DBRowStatus),S_OK);
			
	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,2,pHRowDelete,DBRowStatus),S_OK);
	//delete the rows again
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRowDelete,DBRowStatus),DB_S_ERRORSOCCURRED);
	COMPARE(DBRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPARE(DBRowStatus[1], DBROWSTATUS_E_DELETED);
	COMPARE(DBRowStatus[2], DBROWSTATUS_S_OK);

	fTestPass	= TEST_PASS;	
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowDelete);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5) 
//*-----------------------------------------------------------------------
// @mfunc NEWLYINSERTED from a static cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_5()
{
	BOOL			fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			HRowNew				= NULL;
	void			*pData				= NULL;
	DBPROPID		rgPropertyIDs[2]	= {NULL,NULL};
	DBPROPID		rgUnPropertyIDs[3]	= {NULL,NULL,NULL};
	DBROWSTATUS		DBRowStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]=DBPROP_OTHERINSERT;
	rgUnPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgUnPropertyIDs[2]=DBPROP_CHANGEINSERTEDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	TESTC_PROVIDER(	GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,3,rgUnPropertyIDs,
					ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
					DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowNew),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowNew,&DBRowStatus),DB_E_ERRORSOCCURRED);
			
	if(!COMPARE(DBRowStatus, DBROWSTATUS_E_NEWLYINSERTED))
	{
		goto CLEANUP;
	}
	fTestPass=TRUE;
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}

	if(&HRowNew && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRowNew, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6) 
//*-----------------------------------------------------------------------
// @mfunc key non-key cols, set 2 rows the same and delete
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_6()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow			= NULL;
	void			*pData			= NULL;
	void			*pData1			= NULL;
	DBPROPID		rgPropertyIDs[2]= {NULL,NULL};
	DBROWSTATUS		DBRowStatus;
	DBCOUNTITEM		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		ulCol			= 0;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns with a static cursor
	//create an accessor with IRowsetChange. Buffered update mode.
	//No accessor to be created.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,2,g_rgPropertyIDs,NO_ACCESSOR));

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	//get an array of fixed length and updatable columns
	if(!COMPARE(GetNonKeyNonBLOB(&cCol,&rgColNumber),TRUE))
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;

	//pick the third column.  if there is not 3 columns use the last one
	if(cCol<3)
		ulCol=cCol-1;
	else
		ulCol=3;

	//get 2 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);

	//create an accessor to the first numeric and updatable column.
	TESTC_PROVIDER(GetAccessorOnRowset(	ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
										FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,1,&(rgColNumber[ulCol])))

	//get a new data buffer to set the data a row.  Set
	//the data to be the same as the another row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
	
	//set data. Change the  row to the same as the other handle's data
	TESTC_(SetData(pHRow[0],m_hAccessor,pData),S_OK);
	TESTC_(SetData(pHRow[1],m_hAccessor,pData),S_OK);

	//delete the row, first one only
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&pHRow[0],&DBRowStatus),S_OK);
			
	if(!COMPARE(DBRowStatus, DBROWSTATUS_S_OK))
	{
		goto CLEANUP;
	}

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		if (DB_E_ROWSNOTRELEASED==hr)
		{
			//if CANHOLDROWS is TRUE and RestartPosition returns DB_E_ROWSNOTRELEASED
			//then check DBPROP_QUICKRESTART.  
			//if DBPROP_QUICKRESTART is FALSE (command re-executed) this could be valid 
			//but if DBPROP_QUICKRESTART is TRUE DB_E_ROWSNOTRELEASED should not be 
			//returned from RestartPosition if CANHOLDROWS is requested
			if(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, (IUnknown*)m_pIRowset))		
			{
				goto CLEANUP;
			}
			else
			{
				//if here then DBPROP_QUICKRESTART is TRUE 
				fTestPass	 = TEST_SKIPPED;
				goto CLEANUP;
			}
		}
	}

	//get 2 row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);

	 //allocate memory for GetData
	if(!(pData1=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//get the first 2 rows

	//row 1
	hr=m_pIRowset->GetData(pHRow[0],m_hAccessor,pData1);
	
	//DBPROP_REMOVEDELETEDROWS maybe false
	if(DB_E_DELETEDROW==hr)
	{
		TESTC_(m_pIRowset->GetData(pHRow[1],m_hAccessor,pData1),S_OK);
		//second row here (pData1) should  match the changed data
		if(!CompareBuffer(pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
	}
	//DBPROP_REMOVEDELETEDROWS maybe true
	if(S_OK==hr)
	{
		//the deleted row was removed so the first row fetch should now be the seond changed row
		if(!CompareBuffer(pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowset->GetData(pHRow[1],m_hAccessor,pData1),S_OK);
		//second row here (pData1) should  now not match the changed data
		if(CompareBuffer(pData,pData1,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto CLEANUP;
		}
	}

	fTestPass=TRUE;
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(pData1)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData1,TRUE);
	}

	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Delete rows from a static cursor/immediate mode, delete not supported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_7()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete	= NULL;
	DBPROPID		rgPropertyIDs[2]= {NULL,NULL};
	DBROWSTATUS		DBRowStatus;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;


	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_INSERT;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,
		2,g_rgPropertyIDs,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRowDelete),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRowDelete,&DBRowStatus),DB_E_NOTSUPPORTED);
			
	fTestPass=TRUE;
CLEANUP:
	if(pHRowDelete && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRowDelete);
	PROVIDER_FREE(m_pData);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc delete with a row handle from an unallocated rowset
//	
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_8()
{
	HROW			HRow			= 27;
	DBPROPID		rgPropertyIDs[1];
	BOOL			fTestPass		= TEST_SKIPPED;
	DBROWSTATUS		rgDBRowStatus[1];
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow		= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange || !m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,3,&cRows, &pHRow),S_OK);

	//delete the row using different row handle
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRow,rgDBRowStatus),DB_E_ERRORSOCCURRED);

	TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID));

	fTestPass	= TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc delete with a row handle from an allocated rowset
//	
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_9()
{
	HROW			HRow			= 27;
	DBPROPID		rgPropertyIDs[1];
	BOOL			fTestPass		= TEST_SKIPPED;
	DBROWSTATUS		rgDBRowStatus[1];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,2,g_rgPropertyIDs,
										ON_COMMAND_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange || !m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;

	//delete the row using different row handle
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRow,rgDBRowStatus),DB_E_ERRORSOCCURRED);

	TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_INVALID));

	fTestPass	= TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc insert a row update then Hard Delete the row,
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_10()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	HROW			HRow;
	void			*pData			= NULL;
	DBPROPID		rgPropertyIDs[6];
	DBROWSTATUS		rgDBRowStatus[1];
	DBCOUNTITEM		cRows			= 0;
	HROW			*pUpdatedHRow	= NULL;
	DBROWSTATUS		*pRowStatus		= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[3]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[4]=DBPROP_CANFETCHBACKWARDS;
	rgPropertyIDs[5]=DBPROP_CHANGEINSERTEDROWS;
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgPropertyIDs,
				0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
				DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRow),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL, 1, &HRow,&cRows,&pUpdatedHRow,&pRowStatus),S_OK);
	
	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRow,rgDBRowStatus),S_OK);

	fTestPass	= TEST_PASS;
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);
	}
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pRowStatus);
	if (m_pData)
	{
		PROVIDER_FREE(m_pData);
	}
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc delete the same row handle
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_Delete_NewRow::Variation_11()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	HROW			HRow[2];
	HROW			*pHRow			= NULL;
	DBPROPID		rgPropertyIDs[3];
	DBROWSTATUS		rgDBRowStatus[2];
	DBCOUNTITEM		cRows			= 0;
	HRESULT			hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
				0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
				DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;
			
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 1, 1, &cRows, &pHRow),S_OK);

	//make an array of the same row handle
	HRow[0]	= *pHRow;
	HRow[1]	= *pHRow;

	//delete the row
	hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,2,HRow,rgDBRowStatus);

	//S_OK could be returned here
	if (S_OK==hr)
	{
		TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK));
		TESTC(COMPARE(rgDBRowStatus[1], DBROWSTATUS_S_OK));
	}
	//or DB_S_ERRORSOCCURRED
	if (DB_S_ERRORSOCCURRED==hr)
	{
		if (rgDBRowStatus[0]==DBROWSTATUS_S_OK)
		{
			TESTC(COMPARE(rgDBRowStatus[1], DBROWSTATUS_E_DELETED));
		}
		else
		{
			TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_E_DELETED));
			TESTC(COMPARE(rgDBRowStatus[1], DBROWSTATUS_S_OK));
		}
	}
	fTestPass	= TEST_PASS;
CLEANUP:
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
	}
	if (m_pData)
	{
		PROVIDER_FREE(m_pData);
	}
	PROVIDER_FREE(pHRow);
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
BOOL Related_Delete_NewRow::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Sequence)
//*-----------------------------------------------------------------------
//| Test Case:		Sequence - sequence testing
//|	Created:			04/22/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence::Init()
{
	BOOL fTestPass = TEST_SKIPPED;

	if(!TCIRowsetChange::Init())
		return FALSE;


	fTestPass = TRUE;

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Call SetData three times on the same row in an immediately update mode.  One attempt is not successful.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_1()
{
	BOOL		fTestPass=TEST_SKIPPED;   
	void		*pData=NULL;
	void		*pBadData=NULL;	
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow=NULL;
	DBPROPID	rgPropertyIDs[2];
	HRESULT		hr;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

   	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,
		UPDATEABLE_COLS_BOUND))

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 2, 1, &cRows, &pHRow),S_OK);

	TESTC_(GetData(*pHRow, m_hAccessor, m_pData),S_OK);
		

	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//fillup input data again
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pBadData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//set data
	hr=SetData(*pHRow, m_hAccessor, pBadData);
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	fTestPass = TRUE;

CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE); 

	if(pBadData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pBadData,TRUE); 

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Call SetData on two different row handles in an immediately update mode. One attempt is not successful
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_2()
{
	BOOL		fTestPass=TEST_SKIPPED;
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow=NULL;
	HROW		*pHRowSecond=NULL;
	void		*pData=NULL;
	void		*pDataSecond=NULL;
	DBPROPID	rgPropertyIDs[3];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;


   	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, TRUE,
		 DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,
		UPDATEABLE_COLS_BOUND))

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass=FALSE;
	
	//fillup input data	to the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//fillup input data again for the second row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pDataSecond,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 3, 1, &cRows, &pHRow),S_OK);
		
	//get another row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRows, &pHRowSecond),S_OK);
		
	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
		
	//set data to the second row
	TESTC_(SetData(*pHRowSecond, m_hAccessor, pDataSecond),S_OK);
		
	//relase the 2nd row handle
 	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass=TRUE;
CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE); 

  	if(pDataSecond)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pDataSecond,TRUE); 

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Call SetData twice on the same row in a buffered update mode.  The second attempt is not successful.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_3()
{
	BOOL		fTestPass=TEST_SKIPPED;   
	void		*pData=NULL;
	void		*pBadData=NULL;
	ULONG		ulStatus;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cBinding		= 0;
	HROW		*pHRow=NULL;
	HROW		*pUpdatedHRow=NULL;
	ULONG		cRefCount=0;
	DBROWSTATUS	*pRowStatus=NULL;
	DBPROPID	rgPropertyIDs[3];
	HRESULT		hr = S_OK;

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	

   	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE, DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,
		UPDATEABLE_COLS_BOUND))

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//fillup input data again
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pBadData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//change the status binding of the first column to an invalid value
//	ulStatus=*(DBSTATUS *)((DWORD)pBadData+m_rgBinding[0].obStatus);
//	*(DBSTATUS *)((DWORD)pBadData+m_rgBinding[0].obStatus)=DBSTATUS_E_CANTCONVERTVALUE;
	ulStatus=STATUS_BINDING(m_rgBinding[0],pBadData);
	STATUS_BINDING(m_rgBinding[0],pBadData)=DBSTATUS_E_CANTCONVERTVALUE;

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 3, 1, &cRows, &pHRow),S_OK);
	
	//set data
	TESTC_(SetData(*pHRow, m_hAccessor, pData),S_OK);
	
	//udpate data
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//set data
	hr = SetData(*pHRow, m_hAccessor, pBadData);

	TESTC(COMPARE(hr == DB_E_ERRORSOCCURRED || hr == DB_S_ERRORSOCCURRED, TRUE));

	if ( hr == DB_S_ERRORSOCCURRED )
	{
		// check status 
//		fTestPass = COMPARE(*(DBSTATUS *)((DWORD)pBadData+m_rgBinding[0].obStatus),DBSTATUS_E_BADSTATUS);
		fTestPass = COMPARE(STATUS_BINDING(m_rgBinding[0],pBadData),DBSTATUS_E_BADSTATUS);
		for (cBinding = 1; cBinding<m_cBinding; cBinding++)
		{
			if	(
//					DBSTATUS_S_OK		!=	*(DBSTATUS *)((DWORD)pBadData+m_rgBinding[cBinding].obStatus)	&&
//					DBSTATUS_S_ISNULL	!=	*(DBSTATUS *)((DWORD)pBadData+m_rgBinding[cBinding].obStatus)	
					DBSTATUS_S_OK		!=	STATUS_BINDING(m_rgBinding[cBinding],pBadData)	&&
					DBSTATUS_S_ISNULL	!=	STATUS_BINDING(m_rgBinding[cBinding],pBadData)	
				)
			{
				fTestPass = FALSE;
			}
		}
	}
		
	//udpate data
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow, &cRows,&pUpdatedHRow,&pRowStatus),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow, NULL, &cRefCount, NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	pHRow=NULL;

	fTestPass=TRUE;	  
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pUpdatedHRow);
	PROVIDER_FREE(pRowStatus);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE); 

	if(pBadData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pBadData,TRUE); 

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc In buffered update mode, retrieve 50 row handles, call IRowsetChange on each row handle,  then call IRowstUpdate::Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_4()
{
	BOOL		fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM	cCnt				= g_ulNextRow++;
	DBCOUNTITEM	cCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	void		*pData				= NULL;
	DBCOUNTITEM	cRows				= 0;
	HROW		*pHRow=NULL;
	DBCOUNTITEM	cUpdatedRows		= 0;
	DBCOUNTITEM	cMaxPendingRows		= 0;
	HROW		*pUpdatedHRow		= NULL;
	DBROWSTATUS	*rgDBRowStatus		= NULL;
	DBPROPID	rgPropertyIDs[2];

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;

   	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,
		0,NULL,NO_ACCESSOR))

	TESTC(GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, m_pIRowset, &cMaxPendingRows));
	
	//get variable length updated columns
	if(!GetVariableLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate)|| (!m_cBinding))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create an write only accessor on the rowset for variable length columns
	TESTC_PROVIDER(GetAccessorOnRowset(ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY,
		cCol, rgColNumber))

	//get data for change row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pData,cCnt++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get all row handles
	TESTC_(m_pIRowset->GetNextRows(NULL, 0,m_ulTableRows,&cRows,&pHRow),S_OK);
		
	if(!COMPARE(cRows, m_ulTableRows))
		goto CLEANUP;

	if ( cMaxPendingRows > m_ulTableRows )
	{
		//update all the row handles
		for(cCnt=0; cCnt<m_ulTableRows; cCnt++)
		{
			//set data
			TESTC_(SetData(pHRow[cCnt], m_hAccessor, pData),S_OK);
		}

	
		//call update to update the 31 rows at the same time
		TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cUpdatedRows,&pUpdatedHRow,&rgDBRowStatus),S_OK);
	
		if(COMPARE(cUpdatedRows, m_ulTableRows))
			fTestPass=TRUE;
	}
	else
	{
		for ( cCnt=0; cCnt<m_ulTableRows; cCnt++ )
		{
			//set data for one row
			TESTC_(SetData(pHRow[cCnt], m_hAccessor, pData),S_OK);

			TESTC_(m_pIRowsetUpdate->Update(NULL,1,&(pHRow[cCnt]),&cUpdatedRows,&pUpdatedHRow,&rgDBRowStatus),S_OK);
	
			TESTC(COMPARE(cUpdatedRows, 1));
			TESTC(COMPARE(pHRow[cCnt], pUpdatedHRow[0]));			
			TESTC(COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK));
				
			PROVIDER_FREE(pUpdatedHRow);
			PROVIDER_FREE(rgDBRowStatus);
		}

		fTestPass = TRUE;
	}

CLEANUP:
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(rgDBRowStatus);

	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE); 

	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(pUpdatedHRow);

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
BOOL Sequence::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Transaction)
//*-----------------------------------------------------------------------
//| Test Case:		Transaction - Testing IRowsetChange within a transaction
//|	Created:			04/23/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Transaction::Init()
{
	ULONG ulUpdValue = 0;
	m_DBPropSet.rgProperties=NULL;

	if(!CTransaction::Init())
		return TEST_SKIPPED;

	m_DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	m_DBPropSet.cProperties=3;
	m_DBPropSet.rgProperties=(DBPROP *)PROVIDER_ALLOC(6*sizeof(DBPROP));

	if(!m_DBPropSet.rgProperties)
		return FALSE;

	//IID_IRowsetChange
   	m_DBPropSet.rgProperties[0].dwPropertyID=DBPROP_IRowsetChange;                             
   	m_DBPropSet.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
   	m_DBPropSet.rgProperties[0].vValue.vt=VT_BOOL;
   	m_DBPropSet.rgProperties[0].colid			= DB_NULLID;
   	V_BOOL(&m_DBPropSet.rgProperties[0].vValue)=VARIANT_TRUE;                                   
                                                                                 
	//DBPROP_UPDATEABILITY
	m_DBPropSet.rgProperties[1].dwPropertyID=DBPROP_UPDATABILITY;                    
   	m_DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_DBPropSet.rgProperties[1].vValue.vt=VT_I4;
   	m_DBPropSet.rgProperties[1].colid			= DB_NULLID;
	V_I4(&m_DBPropSet.rgProperties[1].vValue)=DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//DBPROP_CANHOLDROWS
	m_DBPropSet.rgProperties[2].dwPropertyID=DBPROP_CANHOLDROWS;                    
   	m_DBPropSet.rgProperties[2].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_DBPropSet.rgProperties[2].vValue.vt=VT_BOOL;
   	m_DBPropSet.rgProperties[2].colid			= DB_NULLID;
   	V_BOOL(&m_DBPropSet.rgProperties[2].vValue)=VARIANT_TRUE; 

	if(SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
		//register interface to be tested
		if(!RegisterInterface(ROWSET_INTERFACE, IID_IRowsetChange, 1, &m_DBPropSet))
		{
			//all of these props are not supported
			return TEST_SKIPPED;
		}
	}
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=TRUE.  Cursor based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_1()
{
	BOOL				fTestPass=TEST_SKIPPED;
	HACCESSOR			hAccessor=NULL;
	IAccessor			*pIAccessor=NULL;
	void				*pSetData=NULL;
	void				*pSetSecondData=NULL;
	DBLENGTH			cRowSize=0; 
	DBCOUNTITEM			cBinding=0;
	DBBINDING			*rgBinding=NULL;
	IRowsetChange		*pIRowsetChange=NULL;
	DBCOUNTITEM			cRows			= 0;
	HROW				*pHRow=NULL;


	if(!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
			odtLog<<L"IRowsetChange not supported, test Skipped."<<ENDL;
			goto CLEANUP;
	}

	TESTC_PROVIDER(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowsetChange,1,&m_DBPropSet));

	fTestPass=FALSE;
	
	//create an accessor on the rowset object so that it can be shared
	TESTC_(GetAccessorAndBindings(
									m_pIRowset,
									DBACCESSOR_ROWDATA,
									&hAccessor, 
									&rgBinding,
									&cBinding,
									&cRowSize,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_NONINDEX_COLS_BOUND
									),S_OK);
		

	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows, &pHRow),S_OK);
	
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
						(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//Get a second buffer to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
						(BYTE **)&pSetSecondData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data							  
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor,pSetData),S_OK);

	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
	{
		goto CLEANUP;
	}
	if(!m_fCommitPreserve)
	{
		if(CHECK(pIRowsetChange->SetData(*pHRow, hAccessor, pSetSecondData),E_UNEXPECTED))
		{
			fTestPass=TRUE;
			goto CLEANUP;
		}
	}

	//test the rowset should be fully functional
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor, pSetSecondData),S_OK);
		

	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto CLEANUP;

	//releaset the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	fTestPass = TRUE;
		
	hAccessor=NULL;
	PROVIDER_FREE(rgBinding);
CLEANUP:
	//release the accessor on the second rowset
	if(hAccessor)
	{
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,
			(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

   	if(pSetSecondData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetSecondData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	SAFE_RELEASE(pIRowsetChange);

	//clean up
	CleanUpTransaction(S_OK);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=FALSE. Query based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_2()
{
	BOOL				fTestPass=TEST_SKIPPED;
	HACCESSOR			hAccessor=NULL;
	IAccessor			*pIAccessor=NULL;
	void				*pSetData=NULL;
	DBLENGTH			cRowSize=0; 
	DBCOUNTITEM			cBinding=0;
	DBBINDING			*rgBinding=NULL;
	IRowsetChange		*pIRowsetChange=NULL;
	DBCOUNTITEM			cRows			= 0;
	HROW				*pHRow=NULL;

	if(!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
			odtLog<<L"IRowsetChange not supported, test Skipped."<<ENDL;
			goto CLEANUP;
	}

	//start a transaction.  Create a rowset with IRowsetChange pointer.
	TESTC_PROVIDER(StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIRowsetChange,1,&m_DBPropSet));

	fTestPass=FALSE;
	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows, &pHRow),S_OK);
	
	//create an accessor on the rowset object so that it can be shared
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,
									&hAccessor, &rgBinding,&cBinding,&cRowSize,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
									UPDATEABLE_NONINDEX_COLS_BOUND),S_OK);
		
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor,pSetData),S_OK);
		
	//commit the transaction with fRetaining==FALSE
	if(!GetCommit(FALSE))
		goto CLEANUP;

	if(!m_fCommitPreserve)
	{
		//test zombie
		if(CHECK(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData),E_UNEXPECTED))
			fTestPass=TRUE;

		goto CLEANUP;
	}
				 
	//test the rowset should be fully functional
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData), S_OK);
		
	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	fTestPass = TRUE;
		
	hAccessor=NULL;
	PROVIDER_FREE(rgBinding);
CLEANUP:
	//release the accessor on the second rowset
	if(hAccessor)
	{
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,
			(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	SAFE_RELEASE(pIRowsetChange);

	//clean up
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=TRUE.  Query based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_3()
{
	BOOL				fTestPass			= TEST_SKIPPED;
	BOOL				fFound				= FALSE;
	HACCESSOR			hAccessor			= NULL;
	IAccessor			*pIAccessor			= NULL;
	void				*pSetData			= NULL;
	DBLENGTH			cRowSize			= 0; 
	void				*pGetOriginalData	= NULL;
	void				*pData				= NULL;
	DBCOUNTITEM			cBinding			= 0;
	DBBINDING			*rgBinding			= NULL;
	IRowsetChange		*pIRowsetChange		= NULL;
	DBCOUNTITEM			cRows				= 0;
	HROW				*pHRow				= NULL;
	HRESULT				hr					= S_OK;
	IMalloc				*pIMalloc;	

	CoGetMalloc(MEMCTX_TASK, &pIMalloc);
	if(!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
			odtLog<<L"IRowsetChange not supported, test Skipped."<<ENDL;
			goto CLEANUP;
	}
 
	//start a transaction.  Create a rowset with IRowsetChange pointer.
	TESTC_PROVIDER(StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,1,&m_DBPropSet));

	fTestPass=FALSE;

	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows, &pHRow),S_OK);
		
	//create an accessor on the command object so that it can be shared
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding,&cBinding,&cRowSize,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_NONINDEX_COLS_BOUND),S_OK);
		
	 //allocate memory for GetData
	if(!(pGetOriginalData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	TESTC_(m_pIRowset->GetData(*pHRow, hAccessor,pGetOriginalData),S_OK);
		
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
//	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor,pSetData),S_OK);
	pIRowsetChange->SetData(*pHRow, hAccessor,pSetData);
					
	//Abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto CLEANUP;

	if(!m_fAbortPreserve)
	{
		//test zombie
		if(CHECK(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData),E_UNEXPECTED))
			fTestPass=TRUE;

		goto CLEANUP;
	}
				 
	//release the row handle on the 1st rowset
	if(pHRow && m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	//get the data from the row
	 //allocate memory for GetData
	if(!(pData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the ith row handle
		TESTC_(m_pIRowset->GetData(*pHRow,hAccessor,pData),S_OK);
		//make sure GetData should NOT be able to see the change
		//ABORT was called!
		if(CompareBuffer(pData,pSetData,cBinding,rgBinding,pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fFound=TRUE;
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if (fFound)
	{
		goto CLEANUP;
	}

	//free the memory used by m_pData
	ReleaseInputBindingsMemory(cBinding,rgBinding,(BYTE *)pData);

	//test the rowset should be fully functional
	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))
	{
		goto CLEANUP;
	}
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows, &pHRow),S_OK);
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData),S_OK);
		
	//Abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto CLEANUP;
		
	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	fTestPass = TRUE;
CLEANUP:
	//release the accessor on the second rowset
	if(hAccessor)
	{
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,
			(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	if(pGetOriginalData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pGetOriginalData, TRUE);

	SAFE_RELEASE(pIMalloc);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	SAFE_RELEASE(pIRowsetChange);

	//clean up
	CleanUpTransaction(S_OK);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=FALSE.  Cursor based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_4()
{
	BOOL				fTestPass		= TEST_SKIPPED;
	HACCESSOR			hAccessor		= NULL;
	IAccessor			*pIAccessor		= NULL;
	void				*pSetData		= NULL;
	DBLENGTH			cRowSize		= 0; 
	void				*pGetData		= NULL;
	DBCOUNTITEM			cBinding		= 0;
	DBBINDING			*rgBinding		= NULL;
	IRowsetChange		*pIRowsetChange	= NULL;
	DBCOUNTITEM			cRows			= 0;
	HROW				*pHRow			= NULL;
	HROW				*pSecondHRow	= NULL;
	IUnknown			*pRowset		= NULL;

	if(!SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,g_pIDBCreateSession))
	{
			odtLog<<L"IRowsetChange not supported, test Skipped."<<ENDL;
			goto CLEANUP;
	}

	//start a transaction.  Create a rowset with IRowsetChange pointer.
	TESTC_PROVIDER(StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,1,&m_DBPropSet));

	fTestPass=FALSE;
	
	//get a row
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK);
		
	//create an accessor on the command object so that it can be shared
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor,&rgBinding,&cBinding,&cRowSize,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_NONINDEX_COLS_BOUND),S_OK);
		
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//set data
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor,pSetData),S_OK);
		
	//Abort the transaction with fRetaining==FALSE
	if(!GetAbort(FALSE))
		goto CLEANUP;

	if(!m_fAbortPreserve)
	{
		//test zombie
		if(CHECK(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData),E_UNEXPECTED))
			fTestPass=TRUE;

		goto CLEANUP;
	}
				 
	//test the rowset should be fully functional
	TESTC_(pIRowsetChange->SetData(*pHRow, hAccessor, pSetData),S_OK);
		
	//releaset the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	fTestPass = TRUE;
		
	hAccessor=NULL;
	PROVIDER_FREE(rgBinding);


CLEANUP:
	//release GetData buffers
	PROVIDER_FREE(pGetData);

	//release the accessor on the second rowset
	if(hAccessor)
	{
		//QI for IAccessor.  The accessor could be on the 1st rowset 
		if(pRowset)
		{
			if(CHECK(pRowset->QueryInterface(IID_IAccessor,
			(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
			
		}
		else
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,
			(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}

	}

	//release the row handles generated for the second time
	if(pSecondHRow)
		CHECK(((IRowset *)pRowset)->ReleaseRows(1, pSecondHRow, NULL,NULL, NULL),S_OK);
	PROVIDER_FREE(pSecondHRow);
	SAFE_RELEASE(pRowset);

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);


	//release the row handle on the 1st rowset
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	SAFE_RELEASE(pIRowsetChange);

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
BOOL Transaction::Terminate()
{
	if(m_DBPropSet.rgProperties)
		PROVIDER_FREE(m_DBPropSet.rgProperties);

	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - Extended Errors
//|	Created:			07/26/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL ExtendedErrors::Init()
{
	if(!TCIRowsetChange::Init())
		return FALSE;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid DeleteRows calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{  
	BOOL			fTestPass=TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowDelete=NULL;
	DBPROPID		rgPropertyIDs[3];
	HRESULT			hr;
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.
	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[2]=DBPROP_OTHERUPDATEDELETE;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));
	
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows, &pHRowDelete),S_OK);
			
	//create an error object
	m_pExtError->CauseError();

	//delete the row
	if(CHECK(hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRowDelete,NULL),S_OK))
		//Do extended check following DeleteRows
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);	
	else
		goto CLEANUP;
	
	//release the deleted row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
		 
	PROVIDER_FREE(pHRowDelete);
		
CLEANUP:
	if(pHRowDelete && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowDelete, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowDelete);
	
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid DeleteRows calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{	
	BOOL				fTestPass=TEST_SKIPPED;
	DBPROPID			rgPropertyIDs[3];
	HRESULT				hr;
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetIdentity method.
	//We then check extended errors to verify the right extended error behavior.
	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//create an error object
	m_pExtError->CauseError();

	//delete the row
	if(CHECK(hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,NULL),E_INVALIDARG))
		//Do extended check following DeleteRows
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid DeleteRows calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBPROPID		rgPropertyIDs[3];
	HRESULT			hr;
	
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.
	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,
		0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//delete the row
	if(CHECK(hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,NULL),E_INVALIDARG))
		//Do extended check following DeleteRows
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid SetData calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{
	HROW		hRow[1];
	HROW		*pHRow			= hRow;
	DBCOUNTITEM	cRowsObtained	= 0;
	HACCESSOR	hAccessor;
	DBPROPID	rgPropertyIDs[1];
	HRESULT		hr;
	BOOL		fPass			= TEST_SKIPPED;
   	
	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.

	rgPropertyIDs[0]=DBPROP_IRowsetUpdate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR));
	
	if (!m_pIRowsetChange)
		goto CLEANUP;

	fPass = FALSE;
	
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
		0,NULL,0,&hAccessor,NULL),S_OK);
		

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),S_OK);
		
	COMPARE(cRowsObtained, 1);

	//create an error object
	//m_pExtError->CauseError();

	if(CHECK(hr=SetData(hRow[0],hAccessor,NULL),S_OK))
		//Do extended check following SetData
		fPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);	

	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
		
	CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid SetData calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{
	HROW		hRow[1];
	HROW		*pHRow			= hRow;
	DBCOUNTITEM	cRowsObtained	= 0;
	HACCESSOR	hAccessor;
	BOOL		fPass			= TEST_SKIPPED;
	DBPROPID	rgPropertyIDs[1];
	HRESULT		hr;

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR));

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fPass = FALSE;
	
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
		0,NULL,0,&hAccessor,NULL),S_OK);
		

	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),S_OK);
		
	COMPARE(cRowsObtained, 1);

	//create an error object
	m_pExtError->CauseError();

	//use NULL hRow
	if(CHECK(hr=m_pIRowsetChange->SetData(DB_NULL_HROW,hAccessor,m_pData),
		DB_E_BADROWHANDLE))
		//Do extended check following SetData
		fPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;

	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
		
	CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid SetData calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
	HROW		hRow[1];
	HROW		*pHRow			= hRow;
	DBCOUNTITEM	cRowsObtained	= 0;
	HACCESSOR	hAccessor;
	HRESULT		hr;
	BOOL		fPass			= TEST_SKIPPED;
	DBPROPID	rgPropertyIDs[1];

	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//create a rowset and an accessor on the rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR))

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fPass = FALSE;
	
	//create an NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
		
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &pHRow),S_OK);
		
	COMPARE(cRowsObtained, 1);

	//use NULL hAccessor
	if(CHECK(hr=SetData(hRow[0],DB_NULL_HACCESSOR,m_pData),DB_E_BADACCESSORHANDLE))
		//Do extended check following SetData
		fPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else 
		goto CLEANUP;

	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
		
	CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid InsertRow calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_7()
{
	BOOL			fTestPass=TEST_SKIPPED;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[3];
	HRESULT			hr;
  
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,
									0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//create an error object
	m_pExtError->CauseError();

	//insert a new row
	if(CHECK(hr=m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,NULL),S_OK))
		//Do extended check following InsertRow
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;

CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid InsertRow calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_8()
{
	BOOL			fTestPass=TEST_SKIPPED;
	DBPROPID		rgPropertyIDs[3];
	HRESULT			hr;

  	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.

	
	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;
	
	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,
									0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//create an error object
	m_pExtError->CauseError();

	//insert a new row
	if(CHECK(hr=m_pIRowsetChange->InsertRow(NULL,m_hAccessor,NULL,NULL),E_INVALIDARG))
		//Do extended check following InsertRow
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Invalid InsertRow calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_9()
{
	BOOL			fTestPass=TRUE;
	void			*pData=NULL;
	DBPROPID		rgPropertyIDs[4];
	HRESULT			hr;
  
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetChange method.
	//We then check extended errors to verify the right extended error behavior.


	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_OTHERUPDATEDELETE;
	rgPropertyIDs[2]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	//open a rowset with an accessor on all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgPropertyIDs,
									0,NULL,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE | DBPART_LENGTH |DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//fillup input data
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	if(CHECK(hr=m_pIRowsetChange->InsertRow(NULL,DB_NULL_HACCESSOR,pData,NULL),DB_E_BADACCESSORHANDLE))
		//Do extended check following InsertRow
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, hr);
	else
		goto CLEANUP;


CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE *)pData,TRUE);

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
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(ProviderOwnedMem)
//*-----------------------------------------------------------------------
//| Test Case:		ProviderOwnedMem - Use ProviderOwned Memory for SetData
//|	Created:			07/31/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ProviderOwnedMem::Init()
{
	BOOL fTestPass = TEST_SKIPPED;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIRowsetChange::Init())
	// }}
		return FALSE;
	fTestPass = TRUE;


	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ProdiverOwnedMem for SetData.  Keyset_Query_Buffered
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ProviderOwnedMem::Variation_1()
{		 
	DBCOUNTITEM		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst		= NULL;
	HROW			*pHRowSecond	= NULL;
	DBPROPID		rgDBProp[4];
	void			*pData			= NULL;
	BOOL			fTestPass		= TEST_SKIPPED;

	HRESULT hr = S_OK;

	//Init
	rgDBProp[0]=DBPROP_IRowsetChange;
	rgDBProp[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBProp[2]=DBPROP_CANHOLDROWS;	
	rgDBProp[3]=DBPROP_IRowsetLocate;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,4,rgDBProp,0,NULL,NO_ACCESSOR));

	
   	//get the variable length updatable columns
	if(!GetVariableLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	if (!m_pIRowsetChange)
		goto CLEANUP;

	fTestPass = FALSE;
	
	//create a write only accessor on the variable length updatable columns
	//do not include the first column.  Use Provider owned memory.
	fTestPass = GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE, DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
									USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,
									DBTYPE_BYREF,cCol,rgColNumber,ALL_COLS_OWNED_BY_PROV);
	//DB_E_ERRORSOCCURRED is temperarily acceptable for providers that don't support provider
	//owned memory. a new prop to determine if a provider supports this is on its way
	if (TEST_SKIPPED == fTestPass)
		goto CLEANUP;

	if (TRUE!=fTestPass)
		goto CLEANUP;

	//get a first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);
		
	//get a second row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowSecond),S_OK);
		
	//get data for the original	values of variable length data types
	TESTC_(GetData(*pHRowFirst, m_hAccessor,m_pData),S_OK);
		
	//set data to the second row handle
	TESTC_(SetData(*pHRowSecond, m_hAccessor, m_pData),S_OK);

	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowFirst);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowSecond);
	
	fTestPass=TRUE;
	
CLEANUP:
	
	PROVIDER_FREE(rgColNumber);

	if(pHRowFirst && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);

	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ProdiverOwnedMem for SetData.  Keyset_Cursor_Imm
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ProviderOwnedMem::Variation_2()
{
	DBCOUNTITEM		cCol			= 0;
	DBORDINAL		*rgColNumber	= NULL;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRowFirst		= NULL;
	HROW			*pHRowSecond	= NULL;
	DBPROPID		rgDBProp[3];
	void			*pData			= NULL;
	BOOL			fTestPass		= TEST_SKIPPED;

	HRESULT hr = S_OK;

	//Init
	rgDBProp[0]=DBPROP_IRowsetUpdate;
	rgDBProp[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBProp[2]=DBPROP_CANHOLDROWS;

	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgDBProp,0,NULL,NO_ACCESSOR));
	
   	//get the variable length updatable columns
	if(!GetVariableLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	if ((!m_pIRowsetChange) || (!m_pIRowsetUpdate))
		goto CLEANUP;

	fTestPass = FALSE;
	
	//create a write only accessor on the variable length updatable columns
	//do not include the first column.  Use Provider owned memory.
	fTestPass= GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE, DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
									USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,
									DBTYPE_BYREF,cCol,rgColNumber,ALL_COLS_OWNED_BY_PROV);
	//DB_E_ERRORSOCCURRED is temperarily acceptable for providers that don't support provider
	//owned memory. a new prop to determine if a provider supports this is on its way
	if (TEST_SKIPPED == fTestPass)
		goto CLEANUP;

	if (TRUE!=fTestPass)
		goto CLEANUP;

	//get a first row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowFirst),S_OK);

	//get a second row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowSecond),S_OK);
		
	//get data for the original	values of variable length data types
	TESTC_(GetData(*pHRowFirst, m_hAccessor,m_pData),S_OK);
			
	//set data to the second row handle
	TESTC_(SetData(*pHRowSecond, m_hAccessor, m_pData),S_OK);
							  
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRowSecond,NULL,NULL,NULL),S_OK);
		
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRowFirst);
	
	fTestPass = TRUE;
	
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColNumber);

	if(pHRowFirst && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowFirst,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowFirst);

	if(pHRowSecond && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRowSecond,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRowSecond);
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
BOOL ProviderOwnedMem::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetChange::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(SetDefault)
//*-----------------------------------------------------------------------
//| Test Case:		SetDefault - Default value in data setting operations
//| Created:  	6/8/98
//*-----------------------------------------------------------------------



//*-----------------------------------------------------------------------
// @mfunc Builds a list of info about the default property of the columns
//
//*-----------------------------------------------------------------------
BOOL SetDefault::GetDefaultColumns()
{
	CRowset		Rowset;
	BOOL		fRes				= FALSE;
	ULONG		i					= 0;
	const ULONG	cRes				= 4;
	VARIANT		rgRes[cRes];
	CCol		col;
	HROW		hRow;
	DBBINDING	*pColNameBinding	= NULL;
	DBBINDING	*pColGUIDBinding	= NULL;
	DBBINDING	*pColPROPIDBinding	= NULL;
	DBBINDING	*pOrdinalPosBinding	= NULL;
	DBBINDING	*pHasDefaultBinding	= NULL;
	DBBINDING	*pDefaultBinding	= NULL;
	DBBINDING	*pIsNullableBinding	= NULL;
	DBID		ColID;
	DBCOUNTITEM	nCol				= 0;
	ULONG		cCol				= 0;
	DBORDINAL	cNumColonTable		= m_pTable->CountColumnsOnTable();

	// init the retrictions for COLUMNS Schema Rowset
	for (i=0; i<cRes; i++)
	{
		VariantInit(&rgRes[i]);
	}
	// set the table name restriction
	rgRes[2].vt = DBTYPE_BSTR;
	V_BSTR(&rgRes[2]) = SysAllocString(m_pTable->GetTableName());

	// alloc mem for the output parameters and initialize
	SAFE_ALLOC(m_rgbDefault, BOOL, cNumColonTable);
	memset(m_rgbDefault, 0, (size_t)(sizeof(BOOL)*cNumColonTable));

	// set restrictions and create rowset, accessor and bindings (do not bind bookmark)
	Rowset.SetRestrictions(cRes, rgRes);
	TESTC_PROVIDER(Rowset.CreateRowset(SELECT_DBSCHEMA_COLUMNS, IID_IRowset, m_pTable, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK) == S_OK);

	// map various bindings
	pColNameBinding		= &Rowset.m_rgBinding[3];
	pColGUIDBinding		= &Rowset.m_rgBinding[4];
	pColPROPIDBinding	= &Rowset.m_rgBinding[5];
	pOrdinalPosBinding	= &Rowset.m_rgBinding[6];
	pHasDefaultBinding	= &Rowset.m_rgBinding[7];
	pDefaultBinding		= &Rowset.m_rgBinding[8];
	pIsNullableBinding	= &Rowset.m_rgBinding[10];

	// iterate the columns
	while (Rowset.GetNextRows(&hRow) == S_OK)
	{
		// get data for this column
		TESTC_PROVIDER(Rowset.GetRowData(hRow, &Rowset.m_pData) == S_OK);
		//check binding status for info required
		TESTC(	DBSTATUS_S_OK == STATUS_BINDING(*pColNameBinding, Rowset.m_pData) 
			||	DBSTATUS_S_OK == STATUS_BINDING(*pColGUIDBinding, Rowset.m_pData) 
			||	DBSTATUS_S_OK == STATUS_BINDING(*pColPROPIDBinding, Rowset.m_pData)
			||	DBSTATUS_S_OK == STATUS_BINDING(*pOrdinalPosBinding, Rowset.m_pData));
		TESTC(DBSTATUS_S_OK == STATUS_BINDING(*pHasDefaultBinding, Rowset.m_pData));

		// identify the column (using the ordinal position, name, guid or propid)
		if (DBSTATUS_S_OK == STATUS_BINDING(*pOrdinalPosBinding, Rowset.m_pData))
		{
			// using the ordinal position
			nCol = *(unsigned int*)((BYTE*)Rowset.m_pData + pOrdinalPosBinding->obValue);
		}
		else
		{
			if (DBSTATUS_S_OK == STATUS_BINDING(*pColNameBinding, Rowset.m_pData))
			{
				// identify the column through its name
				ColID.eKind = DBKIND_NAME;
				ColID.uName.pwszName = (WCHAR*)((BYTE*)Rowset.m_pData + pColNameBinding->obValue);
			}
			else 
				if (DBSTATUS_S_OK == STATUS_BINDING(*pColGUIDBinding, Rowset.m_pData))
				{
					// get the column based on guid
					ColID.eKind = DBKIND_GUID;
					ColID.uGuid.guid = *(GUID*)((BYTE*)Rowset.m_pData+pColGUIDBinding->obValue);
				}
				else
				{
					// get the propid of the column
					ColID.eKind = DBKIND_PROPID;
					ColID.uName.ulPropid = *(PROPID*)((BYTE*)Rowset.m_pData+pColPROPIDBinding->obValue);
				}
			// retrieve column info from the table and get column number (ordinal position)
			m_pTable->GetColInfo(&ColID, col);
			nCol = col.GetColNum();
		}

		col = m_pTable->GetColInfoForUpdate(nCol);

		// go and check that the column has a default value
		if (VARIANT_TRUE == *(VARIANT_BOOL*)((BYTE*)Rowset.m_pData+pHasDefaultBinding->obValue))
		{
			m_rgbDefault[nCol-1] = TRUE;

			if (DBSTATUS_S_OK == STATUS_BINDING(*pDefaultBinding, Rowset.m_pData))
			{
				VARIANT	vDefault;
				// get column default
				col.SetHasDefault(TRUE);
				vDefault.vt = VT_BSTR;
				V_BSTR(&vDefault) = SysAllocString((WCHAR*)((BYTE*)Rowset.m_pData+pDefaultBinding->obValue));
				col.SetDefaultValue(vDefault);
				VariantClear(&vDefault);
			}
			else
				if (DBSTATUS_S_ISNULL == STATUS_BINDING(*pDefaultBinding, Rowset.m_pData))
				{
					col.SetNullable(TRUE);
				}
		}

		// check if the column is nullable
		if (	col.GetNullable()
			||	(	DBSTATUS_S_OK == STATUS_BINDING(*pIsNullableBinding, Rowset.m_pData)
				&&	VARIANT_TRUE == *(VARIANT_BOOL*)((BYTE*)Rowset.m_pData+pIsNullableBinding->obValue)))
		{
			col.SetNullable(TRUE);
			m_rgbDefault[nCol-1] = TRUE;
		}
	}

	fRes = TRUE;
CLEANUP:
	// clean restriction array
	for (i=0; i<cRes; i++)
	{
		VariantClear(&rgRes[i]);
	}
	return fRes;
} // SetDefault::GetDefaultColumns




//*-----------------------------------------------------------------------
// @mfunc mask the m_fHasDefault field of columns in m_pTable
//
void SetDefault::MaskDefColumns(BOOL fMask)
{
	ULONG		iCol	= 0;
	DBORDINAL	cCol	= g_pTable->CountColumnsOnTable();
	CCol		col;

	for (iCol=0; iCol < cCol; iCol++)
	{
		col = g_pTable->GetColInfoForUpdate(iCol+1);
		if (m_rgbDefault[iCol] && fMask)
			g_pTable->SetHasDefaultValue(col, fMask);
		else
			g_pTable->SetHasDefaultValue(col, FALSE);
	}
	return;
} //SetDefault::MaskDefColumns



//*-----------------------------------------------------------------------
// @mfunc SetDefault::PrepareForSetData
//			Creates a rowset and accessors, fills the input bindings and
//			set the proper status of the given column
//
// @rdesc TEST_FAIL on failure or TEST_PASS on success. TEST_SKIPPED when prop not supported
//
BOOL SetDefault::PrepareForSetData(
	DBORDINAL		cSelectedColumn,	// [in]	the ordinal of the selected column (1 based)
	DBSTATUS		Status,				// [in] the status value for the selected column
	DBCOUNTITEM		lRowsOffset,		// [in] row number for data creation
	BYTE			**ppData			// [out] data buffer
)
{
	BOOL		fTestPass	= TEST_SKIPPED;
	ULONG		cBinding	= 0;

	// properties asked for the rowset related to bindings
	DBPROPID	rgPropertyIDs[] = {DBPROP_IRowsetChange}; 

	TESTC(NULL != ppData);
	*ppData = NULL;

	//create an accessor on the command object on a updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,NUMELEM(rgPropertyIDs),rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,0,NULL));
	TESTC(0 < m_cBinding);

	fTestPass = TEST_FAIL;

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								ppData,
								lRowsOffset,					// row number for data creation
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	// set status in bindings
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		if (m_rgBinding[cBinding].iOrdinal == cSelectedColumn)
		{
			*(ULONG *)(*ppData+m_rgBinding[cBinding].obStatus) = Status;
			break;
		}
	}

	fTestPass = TEST_PASS;
CLEANUP:
	return fTestPass;
} // SetDefault::PrepareForSetData



//*-----------------------------------------------------------------------
// @mfunc SetData, check it, get data, check it 
//
// @rdesc TRUE or FALSE
//
BOOL SetDefault::SetAndCheckDefault(
	BYTE	*pData,					// [in] buffer for IRowsetChange::SetData
	HRESULT	hrSetDataExpected,		// [in] expected hr for IRowsetChange::SetData
	BOOL	fValidate /*= TRUE*/,	// [in] validation flag
	HRESULT	*hrSetData /*= NULL*/	// [out] actual result of IRowsetChange::SetData
)
{
	BOOL			fRes				= TRUE;
	HRESULT			hr;
	HRESULT			hrGetData;
	// buffer for row data
	BYTE			*pDataCC			= NULL;
	BYTE			*pDataOne			= NULL;	// for comparison
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cRows				= 0;
	ULONG			lRowsOffset			= 0;
	ULONG			ulStatus;
	ULONG			ulStatusCC;
	ULONG			ulStatusGet;
	void			*pDefault			= NULL;
	USHORT			cb					= 0;
	ULONG			size;
	ULONG			cBinding			= 0;
	CCol			col;
	DBORDINAL		cOrdinalPos			= 0;
	DBORDINAL		nCols				= m_pTable->CountColumnsOnTable();
	WCHAR			*pwszMakeData		= NULL;


	// make sure pData is not null
	if (NULL == pData)
	{
		odtLog << "ERROR: pData is NULL"<<ENDL;
		fRes = FALSE;
		goto CLEANUP;
	}

	// carbon copy of original pData in pDataCC
	SAFE_ALLOC(pDataCC, BYTE, m_cRowSize);
	memcpy(pDataCC, pData, (size_t)m_cRowSize);

	//get the row handle
	if (!CHECK(m_pIRowset->GetNextRows(NULL,lRowsOffset,1,&cRows,&pHRow),S_OK))
	{
		fRes = FALSE;
	}

	// set data
	hr = SetData(*pHRow,m_hAccessor,pData);

	// check hr
	if (NULL != hrSetData)
	{
		*hrSetData = hr;
	}
	if (S_OK!=hr)
	{
		if (fValidate)
		{
			//skip this check if test violates an integrity violation
			for (cBinding=0;cBinding<m_cBinding;cBinding++)
			{
				if	(	DB_E_INTEGRITYVIOLATION	==	hr			||
						(	DB_E_ERRORSOCCURRED				==	hr	&&
							DBSTATUS_E_INTEGRITYVIOLATION	==	*(ULONG *)(pData+m_rgBinding[cBinding].obStatus)
						)
					)
				{
					goto CLEANUP;
				}
			}
			//some provider can't set a default value
			//if they return can't and DB_E_ERRORSOCCURRED make sure they
			//set the status to DBSTATUS_E_BADSTATUS where it was asked to be DBSTATUS_S_DEFAULT
			if (DB_E_ERRORSOCCURRED!=hr)
			{
				odtLog << "Error in IRowsetChange::SetData return value"<<ENDL;
				fRes = FALSE;
				COMPARE(1,0);
				goto CLEANUP;
			}
			else
			{
				for (cBinding=0;cBinding<m_cBinding;cBinding++)
				{
					if (DBSTATUS_E_BADSTATUS!=	*(ULONG *)(pData+m_rgBinding[cBinding].obStatus)	&&
						DBSTATUS_S_DEFAULT	==	*(ULONG *)(pDataCC+m_rgBinding[cBinding].obStatus))	
					{
						odtLog << "Error in IRowsetChange::SetData return value"<<ENDL;
						COMPARE(1,0);
						fRes = FALSE;
						goto CLEANUP;
					}
					if (DBSTATUS_E_BADSTATUS==	*(ULONG *)(pData+m_rgBinding[cBinding].obStatus)	&&
						DBSTATUS_S_DEFAULT	==	*(ULONG *)(pDataCC+m_rgBinding[cBinding].obStatus))	
					{
						odtLog << "Warning in IRowsetChange:Provider does not support DBSTATUS_S_DEFAULT"<<ENDL;
						goto CLEANUP;
					}
	//				goto CLEANUP;
				}
			}
		}
		goto CLEANUP;
	}

	//get the data from the row
	SAFE_ALLOC(pDataOne, BYTE, m_cRowSize);
	memset(pDataOne, 0, (size_t)m_cRowSize);

	// get the same row
	hrGetData = GetData(*pHRow,m_hAccessor,pDataOne);

	// general check for data
	if (!COMPARE(CompareBuffer(pDataOne, pDataCC, m_cBinding, m_rgBinding, m_pIMalloc, TRUE, FALSE, COMPARE_ONLY, TRUE), TRUE))
	{
		fRes = FALSE;
	}
		
	m_pTable->SetIndexColumn(1);

	// for the column with the ordinal cCount
	// the provider can return either a status of OK, NULL or UNAVAILABLE
	// for the column marked as DEFAULT going in
	// the provider detected a default value for the column => DBSTATUS_S_OK or DBSTATUS_S_ISNULL
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{	
		// get ordinal of the column
		cOrdinalPos = m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);

		// make sure it is in range
		if (!COMPARE(cOrdinalPos <= nCols, TRUE))
			fRes = FALSE;

		// get status
		ulStatus	= *(ULONG*)(pData+m_rgBinding[cBinding].obStatus);
		ulStatusCC	= *(ULONG*)(pDataCC+m_rgBinding[cBinding].obStatus);
		ulStatusGet	= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obStatus);

		// get data size
		size		= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obLength);
		
		// if column status was DBSTATUS_S_DEFAULT... 
		if (DBSTATUS_S_DEFAULT == ulStatusCC)
		{
			// check success in setting data
			if (DBSTATUS_S_DEFAULT	 != ulStatus && 
				DBSTATUS_E_BADSTATUS != ulStatus)
			{
				odtLog << "ERROR: Bad return status for column " << col.GetColName() << " in SetData"<<ENDL;
				fRes = FALSE;
			}

			if (col.GetHasDefault())
			{
				// status for getting data 		
				switch (ulStatusGet)
				{
					case DBSTATUS_S_OK:
						m_pTable->MakeData(pwszMakeData,m_cSeed,cOrdinalPos,PRIMARY);
						pDefault = WSTR2DBTYPE(pwszMakeData, m_rgBinding[cBinding].wType, &cb);
						if (!CompareDBTypeData(	pDefault, 
												(void*)(pDataOne+m_rgBinding[cBinding].obValue),
												m_rgBinding[cBinding].wType, 
												(USHORT)size, 
												(BYTE)col.GetPrecision(),
												(BYTE)col.GetScale(), 
												m_pIMalloc, 
												FALSE,
												DBTYPE_EMPTY,
												size))
						//if (!COMPARE(memcmp(pDefault, (void*)(pDataOne+m_rgBinding[cBinding].obValue), size), 0))
						{
							odtLog << L"ERROR comparing the returned value for column " << cOrdinalPos << L" (default)"<<ENDL;
							fRes = FALSE;
						}
						break;
					case DBSTATUS_E_UNAVAILABLE:
						break;
					default:
						odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() <<ENDL;
						fRes = FALSE;
						break;
				}
			}
			else
			{
				// the default value is null
				// status for getting data 		
				switch (ulStatusGet)
				{
					case DBSTATUS_S_OK:
						// the column was not recognized as default
						// still it could be a default column
						// do not return error in this case
						break;
					case DBSTATUS_S_ISNULL:
						if (!col.GetNullable() || col.GetHasDefault())
						{
							fRes = FALSE;
						}
						break;
					case DBSTATUS_E_UNAVAILABLE:
						break;
					default:
						odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() <<ENDL;
						fRes = FALSE;
						break;
				}
			}
			SAFE_FREE(pDefault);
		}
	}
CLEANUP:
	SAFE_FREE(pDefault);
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	SAFE_FREE(pHRow);
	SAFE_FREE(pDataCC);
	SAFE_FREE(pDataOne);
	return fRes;
} //SetDefault::SetAndCheckDefault



//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetDefault::Init()
{ 
	BOOL				fRes				= TEST_SKIPPED;
	ULONG				nCol				= 0;
	DBCOLUMNDESC		*pColumnDesc		= NULL;
	ULONG				cColumnDesc			= 1;
	HRESULT				hr;
	ULONG				nDefault			= 0;
	ITableDefinition	*pITableDefinition	= NULL;
	DBCOUNTITEM			cSeed				= g_ulNextRow++;
	DBORDINAL			cColsOnTable		= 0;
	DBPROPID			rgPropertyIDs[1];

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	
	//set the number of rows each table will have in each variation
	m_nRows			= 12;
	m_pCustomTable	= new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	m_rgbDefault	= NULL;


	// {{ TCW_INIT_BASECLASS_CHECK
	// }}
	if(TCIRowsetChange::Init())
	{ 
		//first check if IRowsetChange is supported
		//create a rowset and an accessor on the rowset
		m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

		TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,TRUE));
		if (!m_pIRowsetChange)
		{
			goto CLEANUP;
		}

		fRes	= TEST_FAIL;
		// check if there is any way to create a customized table
		if(		!VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition)
			&&	(	!g_pTable->GetCommandSupOnCTable()	||	
					!g_pTable->GetSQLSupport()
				))
		{
			// there is no way to build customized table, use the existing one
			m_fCustomTables = FALSE;
			GetDefaultColumns();
		}
		else
		{
			// use customized table
			m_fCustomTables = TRUE;

			// set custom CTable
			SetTable(m_pCustomTable, DELETETABLE_NO);		

			//get the number of columns on the table
			cColsOnTable = g_pTable->CountColumnsOnTable();

			// alloc space for bool table
			SAFE_ALLOC(m_rgbDefault, BOOL, cColsOnTable);
			// initialize the array to FALSE
			memset(m_rgbDefault, 0, (size_t)(sizeof(BOOL)*cColsOnTable));

			for (nCol = 0; nCol < cColsOnTable; nCol++)
			{
				SAFE_ALLOC(pColumnDesc, DBCOLUMNDESC, 1);

				// get column info for nCol'th  column from the global table 
				CCol& rCol = g_pTable->GetColInfoForUpdate(nCol+1);

				//sets the default in the CCol list for a column
				if (g_pTable->SetDefaultValue(rCol, cSeed))
				{
					// build the column description (set pColumnDesc from the global table created in ExtraLib)
					g_pTable->BuildColumnDesc(pColumnDesc, rCol);

					// set column desc as current column desc for the table
					m_pCustomTable->SetColumnDesc(pColumnDesc, 1);
					m_pCustomTable->SetBuildColumnDesc(FALSE);	// use the pcolumnDesc

					//if the table can be created with this default for this type then it
					//is a valid default
					if (S_OK == (hr = m_pCustomTable->CreateTable(0, 0)))
					{
						m_rgbDefault[rCol.GetColNum()-1] = TRUE;
					}
					else
					{
						g_pTable->SetHasDefaultValue(rCol, FALSE);
					}
					m_pCustomTable->DropTable();

					// release column desc
					m_pCustomTable->SetColumnDesc(NULL, 0);
					ReleaseColumnDesc(pColumnDesc, 1);
					pColumnDesc = NULL;
				}
				else
					SAFE_FREE(pColumnDesc);
			}
		}
		fRes = TEST_PASS;
	} 
CLEANUP:
	ReleaseRowsetAndAccessor();
	SAFE_RELEASE(pITableDefinition);
	m_pCustomTable->SetColumnDesc(NULL, 0);
	return fRes;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set default values on all default columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_1()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos		= 0;
	ULONG			cBinding		= 0;
	CCol			col;
	// buffer for row data
	BYTE			*pData			= NULL;
	ULONG			nDefault		= 0;
	
	MaskDefColumns(TRUE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	// create rowset and accessor and fill input bindings
	// zero in the first param means ignore the first 2 params
	TESTC_SetData(PrepareForSetData(0, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));
		
	fTestRes	= TEST_FAIL;

	// bind status to DBSTATUS_S_DEFAULT for all the updateable and default columns
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		// get column number and retrieve column
		cOrdinalPos	= m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);
		// if column has a default value set status in bindings
		// nullable columns are considered as well, according to SQL92
		if (col.GetHasDefault() || col.GetNullable())
		{
			*(ULONG *)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_DEFAULT;
			nDefault++;
		}
	}

	if (0 == nDefault)
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "Skip variation: No default columns were found"<<ENDL;
		goto CLEANUP;
	}

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a default column; set many columns, one is asked default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_2()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			ulColAttr;
	
	MaskDefColumns(TRUE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	if (m_fCustomTables)
	{
		// build the custom table
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	// get a default column
	ulColAttr = COL_COND_DEFAULT;
	if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
	{
		// there is no explicit default, get a nullable column
		ulColAttr = COL_COND_NULL;
		if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			odtLog << "No default column was detected"<<ENDL;
			goto CLEANUP;
		}
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));
		
	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not nullable default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_3()
{ 
	BOOL			fTestRes			= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc		= NULL;
	DBORDINAL		cColumnDesc			= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_NOTNULL};
	ULONG			cColAttr			= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet		= FALSE;
	WORD			i					= 0;

	MaskDefColumns(TRUE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find a default nullable column"<<ENDL;
		goto CLEANUP;
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	if (m_fCustomTables)
	{
		fOldNullable = col.GetNullable();
		col.SetNullable(FALSE);

		// restore previous nullable status of the column 
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		col.SetNullable(fOldNullable);

		//loop through already set column property values
		for (i=0;i<rgColumnDesc[col.GetColNum()-1].rgPropertySets->cProperties;i++)
		{
			//if DBPROPSET_COLUMN is set, change the value to FALSE instead of adding another DBPROPSET_COLUMN 
			//to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_FALSE;
				fPropNULLSet = TRUE;
			}
		}
		//make sure DBPROP_COL_NULLABLE is set (to VARIANT_FALSE)
		if (!fPropNULLSet)
		{
			SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
				&rgColumnDesc[col.GetColNum()-1].cPropertySets,
				&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
				VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		// retrieve column
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(!col.GetNullable());
	}

	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def == NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_4()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	// get an implicit default (nullable) column (def == null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find an updateable column"<<ENDL;
		goto CLEANUP;
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	if (m_fCustomTables)
	{
		g_pTable->BuildColumnDescs(&rgColumnDesc);

		//loop through already set column property values
		for (i=0;i<rgColumnDesc[col.GetColNum()-1].rgPropertySets->cProperties;i++)
		{
			//if DBPROPSET_COLUMN is set, change the value to TRUE instead of adding another DBPROPSET_COLUMN 
			//to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_TRUE;
				fPropNULLSet = TRUE;
			}
		}
		//make sure DBPROP_COL_NULLABLE is set (to VARIANT_TRUE)
		if (!fPropNULLSet)
		{
			SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
				&rgColumnDesc[col.GetColNum()-1].cPropertySets,
				&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
				VT_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		TESTC(col.GetNullable());
		TESTC(!col.GetHasDefault());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def != NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_5()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;
	
	// get a default, nullable column (def != NULL)
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UPDATEABLE, COL_COND_NULL};
	ULONG			cColAttr			= 3;

	// reset all the columns to their non default value
	MaskDefColumns(TRUE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;
	
	// get a default nullable column (default != null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not identify column with specified attributes"<<ENDL;
		goto CLEANUP;
	}

	// prezerve info about that column
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	if (m_fCustomTables)
	{
		g_pTable->BuildColumnDescs(&rgColumnDesc);

		//loop through already set column property values
		for (i=0;i<rgColumnDesc[col.GetColNum()-1].rgPropertySets->cProperties;i++)
		{
			//if DBPROPSET_COLUMN is set, change the value to TRUE instead of adding another DBPROPSET_COLUMN 
			//to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_TRUE;
				fPropNULLSet = TRUE;
			}
		}
		//make sure DBPROP_COL_NULLABLE is set (to VARIANT_TRUE)
		if (!fPropNULLSet)
		{
			SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
				&rgColumnDesc[col.GetColNum()-1].cPropertySets,
				&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
				VT_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(col.GetNullable());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));

	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_6()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	MaskDefColumns(FALSE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	if (m_fCustomTables)
	{
		//get a updateable column (not def)
		//this is because a custom table won't have non default or non null cols so this would
		//be skipped if 
		if (!g_pTable->GetColWithAttr(COL_COND_UPDATEABLE, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
		// make the column not default and not nullable
		col.SetHasDefault(FALSE);

		fOldNullable = col.GetNullable();
		col.SetNullable(FALSE);

		// restore previous nullable status of the column 
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		col.SetNullable(fOldNullable);
		
		//loop through already set column property values
		for (i=0;i<rgColumnDesc[col.GetColNum()-1].rgPropertySets->cProperties;i++)
		{
			//if DBPROPSET_COLUMN is set, change the value to FALSE instead of adding another DBPROPSET_COLUMN 
			//to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_FALSE;
				fPropNULLSet = TRUE;
			}
			//if DBPROP_COL_DEFAULT is set then free all properties and reset the NULL (which is all this variation needs)
			//this variation does not want a default sodo not set one with through the column properites
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_DEFAULT)
			{
				//no default property should be set here and all the test needs is NULL so free'em up and
				//set NULL to FALSE
				FreeProperties(&rgColumnDesc[col.GetColNum()-1].cPropertySets,&rgColumnDesc[col.GetColNum()-1].rgPropertySets);
				fPropNULLSet = FALSE;
				break;
			}		
		}
		//make sure DBPROP_COL_NULLABLE is set (to VARIANT_FALSE)
		if (!fPropNULLSet)
		{
			SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
				&rgColumnDesc[col.GetColNum()-1].cPropertySets,
				&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
				VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}
	else
	{
		ULONG	rgColAttr[]	= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NOTNULL};

		// get an updateable column (not def)
		if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, DB_E_ERRORSOCCURRED, TRUE, &hr));
	TESTC(DB_E_ERRORSOCCURRED == hr || DB_E_INTEGRITYVIOLATION == hr);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	}
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a unique default value
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetDefault::Variation_7()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UNIQUE};
	ULONG			cColAttr			= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	if (!SettableProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN))
	{
		fTestRes = TEST_SKIPPED;
		goto CLEANUP;
	}	
	
	// reset all the columns to their non default value
	MaskDefColumns(TRUE);
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	// get a default column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		// try again, this time looking for a nullable column
		rgColAttr[0] = COL_COND_NULL;
		if (m_fCustomTables || !g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	
	if (m_fCustomTables)
	{
		// make the column default and non nullable
		fOldNullable = col.GetNullable();
		col.SetNullable(FALSE);
		col.SetUnique(TRUE);

		// restore previous nullable status of the column 
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		col.SetNullable(fOldNullable);

		//loop through already set column property values
		for (i=0;i<rgColumnDesc[col.GetColNum()-1].rgPropertySets->cProperties;i++)
		{
			//if DBPROPSET_COLUMN is set, change the value to FALSE instead of adding another DBPROPSET_COLUMN 
			//to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_FALSE;
				fPropNULLSet = TRUE;
			}
		}
		//make sure DBPROP_COL_NULLABLE is set (to VARIANT_FALSE)
		if (!fPropNULLSet)
		{
			SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
				&rgColumnDesc[col.GetColNum()-1].cPropertySets,
				&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
				VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		// retrieve the column and make sure is has a default value
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_DEFAULT, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	SetAndCheckDefault(pData, S_OK, TRUE, &hr);
	TESTC(DB_E_INTEGRITYVIOLATION == hr || DB_E_ERRORSOCCURRED == hr);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	}
	fTestRes = TEST_PASS;   
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL SetDefault::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	SAFE_FREE(m_rgbDefault);
	delete m_pCustomTable;
	return(TCIRowsetChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(SetIgnore)
//*-----------------------------------------------------------------------
//| Test Case:		SetIgnore - Ignore data in SetData
//| Created:  	6/18/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetIgnore::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	return SetDefault::Init();
	// }}
} 


//*-----------------------------------------------------------------------
// @mfunc SetData, check it, get data, check it 
//
// @rdesc TRUE or FALSE
//
BOOL SetIgnore::SetAndCheckDefault(
	BYTE	*pData,					// [in] buffer for IRowsetChange::SetData
	HRESULT	hrSetDataExpected,		// [in] expected hr for IRowsetChange::SetData
	BOOL	fValidate, /*= TRUE*/	// [in] validation flag
	HRESULT	*hrSetData, /*= NULL*/	// [out] actual result of IRowsetChange::SetData
	BOOL	fCheck		/*= TRUE*/	// [in] whether to do a check a GetData
)
{
	BOOL			fRes				= TRUE;
	HRESULT			hr;
	HRESULT			hrGetData;
	// buffer for row data
	BYTE			*pDataCC			= NULL;
	BYTE			*pDataOne			= NULL;	// for comparison
	BYTE			*pDataZero			= NULL; // initial values
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cRows				= 0;
	ULONG			lRowsOffset			= 0;

	ULONG			ulStatus;
	ULONG			ulStatusCC;
	ULONG			ulStatusGet;
	ULONG			ulStatusGetZero;

	ULONG			size				= 0;
	ULONG			cBinding			= 0;
	CCol			col;
	DBORDINAL		cOrdinalPos			= 0;
	DBORDINAL		nCols				= m_pTable->CountColumnsOnTable();

	// make sure pData is not null
	if (NULL == pData)
	{
		odtLog << "ERROR: pData is NULL"<<ENDL;
		fRes = FALSE;
		goto CLEANUP;
	}

	// carbon copy of original pData in pDataCC
	SAFE_ALLOC(pDataCC, BYTE, m_cRowSize);
	memcpy(pDataCC, pData, (size_t)m_cRowSize);
	SAFE_ALLOC(pDataZero, BYTE, m_cRowSize);
	memset(pDataZero, 0, (size_t)m_cRowSize);

	//get the row handle
	if (!CHECK(m_pIRowset->GetNextRows(NULL,lRowsOffset,1,&cRows,&pHRow),S_OK))
		fRes = FALSE;

	// get data in pDataZero
	TESTC_(hr = GetData(*pHRow,m_hAccessor,pDataZero), S_OK);

	// set data from pData
	hr = SetData(*pHRow,m_hAccessor,pData);

	// check hr
	if (NULL != hrSetData)
	{
		*hrSetData = hr;
	}
	if (fValidate && !CHECK(hr, hrSetDataExpected))
	{
		odtLog << "Error in IRowsetChange::SetData return value"<<ENDL;
		fRes = FALSE;
	}

	if (!fCheck)
	{
		goto CLEANUP;
	}

	//get the data from the row
	SAFE_ALLOC(pDataOne, BYTE, m_cRowSize);
	memset(pDataOne, 0, (size_t)m_cRowSize);

	// get the same row data in pDataOne
	hrGetData = GetData(*pHRow,m_hAccessor,pDataOne);

	// general check for data (data got - pDataOne, against data set - pData
	if (!COMPARE(CompareBuffer(pDataOne, pData, m_cBinding, m_rgBinding, m_pIMalloc, TRUE,FALSE,COMPARE_ONLY), TRUE))
	{
		fRes = FALSE;
	}
	// for the column with the ordinal cCount
	// the provider can return either a status of OK, NULL or UNAVAILABLE
	// for the column marked as DEFAULT going in
	// the provider detected a default value for the column => DBSTATUS_S_OK or DBSTATUS_S_ISNULL
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{	
		// get ordinal of the column
		cOrdinalPos = m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);

		// make sure it is in range
		if (!COMPARE(cOrdinalPos <= nCols, TRUE))
			fRes = FALSE;

		// get statuses
		ulStatus		= *(ULONG*)(pData+m_rgBinding[cBinding].obStatus);
		ulStatusCC		= *(ULONG*)(pDataCC+m_rgBinding[cBinding].obStatus);
		ulStatusGet		= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obStatus);
		ulStatusGetZero	= *(ULONG*)(pDataZero+m_rgBinding[cBinding].obStatus);

		// get data size
		size		= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obLength);
		
		// if column status was DBSTATUS_S_DEFAULT... 
		if (DBSTATUS_S_IGNORE == ulStatusCC)
		{
			// check success in setting data
			if (S_OK == hr && !COMPARE(DBSTATUS_S_IGNORE, ulStatus))
			{
				odtLog << "ERROR: Bad return status for column " << col.GetColName() << " in SetData"<<ENDL;
				fRes = FALSE;
			}

			// status for getting data 		
			switch (ulStatusGet)
			{
				case DBSTATUS_S_OK:
					// both get statuses should be DBSTATUS_S_OK and values should be the same
					if (!COMPARE(DBSTATUS_S_OK, ulStatusGetZero))
					{
						odtLog << "ERROR: mismatch in getting statuses"<<ENDL;
						fRes = FALSE;
					}
					else if (!CompareDBTypeData(	(void*)(pDataZero+m_rgBinding[cBinding].obValue), 
													(void*)(pDataOne+m_rgBinding[cBinding].obValue),
													m_rgBinding[cBinding].wType, 
													(USHORT)size, 
													(BYTE)col.GetPrecision(),
													(BYTE)col.GetScale(), 
													m_pIMalloc, 
													FALSE,
													DBTYPE_EMPTY,
													*(ULONG*)(pDataZero+m_rgBinding[cBinding].obLength)))
						{
							odtLog << L"ERROR comparing the returned value for column " << cOrdinalPos << L" (ignore)"<<ENDL;
							fRes = FALSE;
						}
					break;
				case DBSTATUS_S_ISNULL:
					if (!col.GetNullable() || !COMPARE(ulStatusGetZero, DBSTATUS_S_ISNULL))
					{
						odtLog << "ERROR: mismatch for get statuses: DBSTATUS_S_ISNULL"<<ENDL;
						fRes = FALSE;
					}
					break;
				case DBSTATUS_E_UNAVAILABLE:
					break;
				default:
					odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() <<ENDL;
					fRes = FALSE;
					break;
			}
		}
	}
CLEANUP:
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	SAFE_FREE(pHRow);
	SAFE_FREE(pDataCC);
	SAFE_FREE(pDataOne);
	SAFE_FREE(pDataZero);
	return fRes;
} //SetIgnore::SetAndCheckDefault



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set ignore values on all updateable columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_1()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos		= 0;
	DBCOUNTITEM		cBinding		= 0;
	CCol			col;
	// buffer for row data
	BYTE			*pData			= NULL;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(0, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));
		
	fTestRes	= TEST_FAIL;

	// bind status to DBSTATUS_S_IGNORE for all the updateable and default columns
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		// get column number and retrieve column
		cOrdinalPos	= m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);
		*(ULONG *)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;
	}

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set ignore value on a column; set many columns, one is asked ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_2()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			cSelectedColumn		= 0;
	DBCOUNTITEM		cBinding;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// build the custom table
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));	

	fTestRes	= TEST_FAIL;

	cBinding = m_cBinding / 2;

	// ask for column default
	*(ULONG*)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set ignore value on a not nullable default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_3()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData			= NULL;
	DBORDINAL		cSelectedColumn	= 0;
	ULONG			rgColAttr[]		= {COL_COND_DEFAULT, COL_COND_NOTNULL};
	ULONG			cColAttr		= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;

	MaskDefColumns(TRUE);

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find a default nullable column"<<ENDL;
		goto CLEANUP;
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	if (m_fCustomTables) 
	{
		fOldNullable = col.GetNullable();
		col.SetNullable(FALSE);

		// restore previous nullable status of the column 
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		col.SetNullable(fOldNullable);

		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[col.GetColNum()-1].cPropertySets,
			&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
			VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		// retrieve column
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(!col.GetNullable());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set ignore value on a nullable default column (def == NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_4()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NULL};
	ULONG			cColAttr			= 3;

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);

	// set an implicit default (nullable) column (def == null)

	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find an updateable column"<<ENDL;
		goto CLEANUP;
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	if (m_fCustomTables)
	{
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		
		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[col.GetColNum()-1].cPropertySets,
			&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
			VT_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		TESTC(col.GetNullable());
		TESTC(!col.GetHasDefault());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set ignore value on a nullable default column (def != NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_5()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	
	// get a default, nullable column (def != NULL)
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UPDATEABLE, COL_COND_NULL};
	ULONG			cColAttr			= 3;

	// reset all the columns to their non default value
	MaskDefColumns(TRUE);

	// get a default nullable column (default != null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not identify column with specified attributes"<<ENDL;
		goto CLEANUP;
	}

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);

	// prezerve info about that column
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);


	if (m_fCustomTables)
	{
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		
		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[col.GetColNum()-1].cPropertySets,
			&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
			VT_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(col.GetNullable());
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Set ignore value on a not default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_6()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	DBORDINAL		cSelectedColumn		= 0;
	BOOL			fOldNullable;

	MaskDefColumns(FALSE);

	if (m_fCustomTables)
	{
		// set a updateable column (not def)
		if (!g_pTable->GetColWithAttr(COL_COND_UPDATEABLE, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
		// make the column not default and not nullable
		col.SetHasDefault(FALSE);
		fOldNullable = col.GetNullable();
		col.SetNullable(FALSE);
	
		// restore previous nullable status of the column 
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		col.SetNullable(fOldNullable);

		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[col.GetColNum()-1].cPropertySets,
			&rgColumnDesc[col.GetColNum()-1].rgPropertySets,
			VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}
	else
	{
		ULONG	rgColAttr[]	= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NOTNULL};

		// get an updateable column (not def)
		if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(cSelectedColumn, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));

	fTestRes	= TEST_FAIL;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Bind a single column and set status to DBSTATUS_S_IGNORE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_7()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	// properties asked for the rowset related to bindings
	DBPROPID		rgPropertyIDs[2];
	// buffer for row data
	BYTE			*pData				= NULL;
	DBCOUNTITEM		cSelectedColumn		= 0;

	rgPropertyIDs[0]=DBPROP_CANHOLDROWS;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;

	if (m_fCustomTables)
	{
		// set a updateable column (not def)
		// create a table with as many default columns as possible
		m_pCustomTable->SetBuildColumnDesc(TRUE);	// create ColList again
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}
	if (!m_pTable->GetColWithAttr(COL_COND_UPDATEABLE, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		goto CLEANUP;
	}

	//create an accessor on the command object on a updatable column only
	fTestRes = TEST_SKIPPED;
	//create an accessor on the command object on the numeric and updatable column only
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,NUMELEM(rgPropertyIDs),rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cSelectedColumn));
	fTestRes = TEST_FAIL;
	TESTC(1 == m_cBinding);

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								&pData,
								g_ulNextRow++,					// row number for data creation
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	// set status in bindings
	*(ULONG *)(pData+m_rgBinding[0].obStatus) = DBSTATUS_S_IGNORE;

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->DropTable();
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Bind all updateable cols, set status to DBSTATUS_S_IGNORE for half of them
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_8()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			cBinding;
	CCol			col;
	
	if (m_fCustomTables)
	{
		// create a table with as many default columns as possible
		m_pCustomTable->SetBuildColumnDesc(TRUE);	// create ColList again
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(0, DBSTATUS_S_IGNORE, g_ulNextRow++, &pData));
		
	fTestRes	= TEST_FAIL;

	// set status in bindings
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		col = m_pTable->GetColInfoForUpdate(m_rgBinding[cBinding].iOrdinal);		
		if (	cBinding % 2
			&&	(col.GetHasDefault() || col.GetNullable()))
		{
			*(ULONG *)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;
			break;
		}
	}

	TESTC(SetAndCheckDefault(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->DropTable();
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Bind a single read only column and set status to DBSTATUS_S_IGNORE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_9()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;				// properties asked for the rowset related to bindings
	DBPROPID		rgPropertyIDs[] = {DBPROP_IRowsetChange};
	BYTE			*pData				= NULL;					// buffer for row data
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hrOut				= S_OK;

	if (m_fCustomTables)
	{
		// set a updateable column (not def)
		// create a table with as many default columns as possible
		m_pCustomTable->SetBuildColumnDesc(TRUE);	// create ColList again
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}
  	if (!m_pTable->GetColWithAttr(COL_COND_NOTUPDATEABLE, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		goto CLEANUP;
	}

	//create an accessor on the command object on a updatable column only
	fTestRes = TEST_SKIPPED;
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,NUMELEM(rgPropertyIDs),rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,1,&cSelectedColumn));
	fTestRes = TEST_FAIL;
	TESTC(1 == m_cBinding);

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								&pData,
								g_ulNextRow++,					// row number for data creation
								m_cRowsetCols,
								m_rgTableColOrds, 
								PRIMARY),S_OK);
		
	// set status in bindings
	*(DBSTATUS *)(pData+m_rgBinding[0].obStatus) = DBSTATUS_S_IGNORE;

	//since the read only column has a status of IGNORE it should not cause an error
	//expect here either S_OK if all other columns are nullable or
	//DB_E_INTEGRITYVIOLATION in case another column in the row that is not bound is not nullable
	TESTC(SetAndCheckDefault(pData, S_OK,TRUE, &hrOut, TRUE));

	if (DB_E_INTEGRITYVIOLATION==hrOut || S_OK==hrOut)
	{
		fTestRes = TEST_PASS;
	}
	if (DB_E_ERRORSOCCURRED==hrOut)
	{
		if (*(ULONG *)(pData+m_rgBinding[0].obStatus)==DBSTATUS_E_INTEGRITYVIOLATION)
		{
			fTestRes = TEST_PASS;
		}
	}
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->DropTable();
	// release buffers
	SAFE_FREE(pData);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc SetData()/SetData(IGNORE)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int SetIgnore::Variation_10()
{ 
	BOOL			fTestRes		= TEST_SKIPPED;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos		= 0;
	ULONG			cBinding		= 0;
	CCol			col;
	// buffer for row data
	BYTE			*pDataIn		= NULL;
	BYTE			*pDataCopy		= NULL;
	BYTE			*pDataReIn		= NULL;
	ULONG			nDefault		= 0;
	HROW			*pHRow			= NULL;
	DBCOUNTITEM		cRows			= 0;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(m_nRows, 1), S_OK);
	}

	//create an accessor on the command object on a updatable column only
	// create rowset and accessor and fill input bindings
	TESTC_SetData(PrepareForSetData(0, DBSTATUS_S_OK, g_ulNextRow++, &pDataIn));
		
	fTestRes	= TEST_FAIL;

	//get the row handle
	if (!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
	{
		goto CLEANUP;
	}

	// get data in pDataIn
	TESTC_(GetData(*pHRow,m_hAccessor,pDataIn), S_OK);

	//make a copy of the buffer and allocate a third
	SAFE_ALLOC(pDataCopy, BYTE, m_cRowSize);
	memcpy(pDataCopy, pDataIn,(size_t)m_cRowSize);
	SAFE_ALLOC(pDataReIn, BYTE, m_cRowSize);
	memset(pDataReIn, 0, (size_t)m_cRowSize);

	// SetData with what was just read in by GetData
	TESTC_(SetData(*pHRow,m_hAccessor,pDataIn),S_OK);

	// bind status to DBSTATUS_S_IGNORE for all the updateable and default columns
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		// get column number and retrieve column
		cOrdinalPos	= m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);
		*(ULONG *)(pDataIn+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;
	}

	//SetData again with IGNORE
	TESTC_(SetData(*pHRow,m_hAccessor,pDataIn),S_OK);

	//GetData in pDataReIn
	TESTC_(GetData(*pHRow,m_hAccessor,pDataReIn), S_OK);

	//compare the buffers
	if (!COMPARE(CompareBuffer(pDataCopy, pDataReIn, m_cBinding, m_rgBinding, m_pIMalloc, TRUE, FALSE, COMPARE_ONLY, TRUE), TRUE))
	{
		goto CLEANUP;
	}

	fTestRes = TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	// release buffers
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	SAFE_FREE(pHRow);

	SAFE_FREE(pDataIn);
	SAFE_FREE(pDataCopy);
	SAFE_FREE(pDataReIn);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL SetIgnore::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(SetDefault::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(DeleteRows)
//*-----------------------------------------------------------------------
//| Test Case:		DeleteRows - Testing DeleteRows
//|	Created:			06/24/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Init()
{
	DBPROPID	rgPropertyIDs[1];
	BOOL		fTestPass	= TEST_SKIPPED;
 
	if(!TCIRowsetChange::Init())
	{
		return FALSE;
	}
	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COLLISTFROMTBL,1,rgPropertyIDs,0,NULL,NO_ACCESSOR));

	fTestPass=TRUE;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc delete array of handles that point to one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_1()
{  
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow				= NULL;
	HROW		rgHRow[4];
	DBPROPID	rgPropertyIDs[1];
	BOOL		fTestPass			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[4];
	HRESULT		hr					= S_OK;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(4));

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	g_fMAXPENDINGROWS	= TRUE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);
	
	//build an array of the same HROW plus a different HROW
	rgHRow[0] = pHRow[0];
	rgHRow[1] = pHRow[0];
	rgHRow[2] = pHRow[1];
	rgHRow[3] = pHRow[0];

	//delete the rows
	hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,4,rgHRow,rgRowStatus);
	
	//all other row status are undefined
	if (S_OK==hr)
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
		COMPARE(rgRowStatus[1], DBROWSTATUS_S_OK);
		COMPARE(rgRowStatus[2], DBROWSTATUS_S_OK);
		COMPARE(rgRowStatus[3], DBROWSTATUS_S_OK);
	}
	else
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
		COMPARE(rgRowStatus[1], DBROWSTATUS_E_DELETED);
		COMPARE(rgRowStatus[2], DBROWSTATUS_S_OK);
		COMPARE(rgRowStatus[3], DBROWSTATUS_E_DELETED);
	}

	fTestPass=TRUE;	 

CLEANUP:
	
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	g_fMAXPENDINGROWS	 = FALSE;
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_S_MULTIPLECHANGES.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_2()
{
//add code here :)
	return TEST_PASS;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_INVALID.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_3()
{  
	const ULONG		culNumProps			= 2;
	BOOL			fTestPass			= TEST_SKIPPED;
	DBCOUNTITEM		cRows			= 0;
	HROW			*pHRow				= NULL;
	HRESULT			hr					= S_OK;
	IRowset			*pIRowset1			= NULL;
	IRowset			*pIRowset2			= NULL;
	IRowsetChange	*pIRowsetChange1	= NULL;
	IRowsetChange	*pIRowsetChange2	= NULL;
	DBPROPSET		rgDBPropSet[1];
	ULONG			cDBPropSet			= 1;
	DBROWSTATUS		rgRowStatus[1];

	//init DBPropSet[0]
	rgDBPropSet[0].rgProperties   = NULL;
	rgDBPropSet[0].cProperties    = culNumProps;
	rgDBPropSet[0].guidPropertySet= DBPROPSET_ROWSET;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	//allocate 
	rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * (culNumProps));
	memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(culNumProps));
	if(!rgDBPropSet[0].rgProperties)
	{
		goto CLEANUP;
	}
	
	//set props
	rgDBPropSet[0].rgProperties[0].dwPropertyID=DBPROP_IRowsetChange;
	rgDBPropSet[0].rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[0].vValue.vt=VT_BOOL;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	rgDBPropSet[0].rgProperties[0].vValue.lVal=VARIANT_TRUE;

	rgDBPropSet[0].rgProperties[1].dwPropertyID=DBPROP_UPDATABILITY;
	rgDBPropSet[0].rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBPropSet[0].rgProperties[1].vValue.vt=VT_I4;
	rgDBPropSet[0].rgProperties[0].colid			= DB_NULLID;
	rgDBPropSet[0].rgProperties[1].vValue.lVal=DBPROPVAL_UP_DELETE | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT;

	//Get Rowset1
	//call IOpenRowset to return a Rowset
	hr=m_pTable->CreateRowset	(
									USE_OPENROWSET,
									IID_IRowset,
									1,
									rgDBPropSet,
									(IUnknown**)&pIRowset1,
									NULL,
									NULL,
									NULL);	

	//Get Rowset2
	//call IOpenRowset to return a Rowset
	hr=m_pTable->CreateRowset	(
									USE_OPENROWSET,
									IID_IRowset,
									1,
									rgDBPropSet,
									(IUnknown**)&pIRowset2,
									NULL,
									NULL,
									NULL);	

	//get the IRowsetChange pointer 1
	TESTC_(pIRowset1->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange1),S_OK);		
	//get the IRowsetChange pointer 2
	TESTC_(pIRowset2->QueryInterface(IID_IRowsetChange, (LPVOID *)&pIRowsetChange2),S_OK);
		
	fTestPass=FALSE;
			  		
	//retrieve row handle
	TESTC_(pIRowset1->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK);
	
	//delete a row from this rowset
	TESTC_(pIRowsetChange1->DeleteRows(DB_NULL_HCHAPTER,1,&pHRow[0],rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	//delete a row from the other rowset
	//as long as this doesn't GPF it is ok
	hr=pIRowsetChange2->DeleteRows(DB_NULL_HCHAPTER,1,&pHRow[1],rgRowStatus);
	
	if(DB_E_ERRORSOCCURRED==hr)
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_E_INVALID);
	}
	if(S_OK==hr)
	{
		COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	}
	fTestPass=TRUE;	 
CLEANUP:

	PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	
	CHECK(pIRowset1->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//release IRowset pointer
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowsetChange1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRowsetChange2);

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc use rows with immediate deletes in other methods
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_4()
{  
	BYTE			*pData				= NULL;
	DBCOUNTITEM		cRows			= 0;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	DBROWSTATUS		*prgRowStatus		= NULL;
	HROW			*pHRow				= NULL;
	HRESULT			hr					= S_OK;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetIdentity;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//try to use HRowX
	//Set/Get
	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
	
	//get the data
	TESTC_(GetData(*pHRow, m_hAccessor, pData),DB_E_DELETEDROW);

	//compare the handles
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,*pHRow),DB_E_DELETEDROW);
	

	fTestPass=TRUE;	 
CLEANUP:
	
	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_5()
{  
	DBCOUNTITEM	cRows				= 0;
	DBCOUNTITEM	i					= 0;
	HROW		*pHRow				= NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[4];
	HRESULT		hr					= S_OK;
	ULONG		cPropSets			= 0;
	DBPROPSET	*rgPropSets			= NULL;


	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(4));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	// Initialize UPDATABILITY flag for class
	g_lMaxPendRows = 2;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_IRowsetUpdate;	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate || (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve 1 more row handle than MAXPENDINGROWS
	TESTC_(m_pIRowset->GetNextRows(NULL,0,(g_lMaxPendRows+1),&cRows,&pHRow),S_OK);
	
	//delete the rows
	hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,(g_lMaxPendRows+1),pHRow,rgRowStatus);
	
	if (S_OK==hr)
	{
		//if DeleteRows returns S_OK is has to be because g_lMaxPendRows is zero or no limit
		for (i=0;i<(g_lMaxPendRows+1);i++)
		{
			COMPARE(rgRowStatus[i], DBROWSTATUS_S_OK);
		}
		COMPARE(g_lMaxPendRows,0);
		fTestPass=TRUE;	 
		goto CLEANUP;
	}
	if (DB_S_ERRORSOCCURRED==hr)
	{
		for (i=0;i<g_lMaxPendRows;i++)
		{
			COMPARE(rgRowStatus[i], DBROWSTATUS_S_OK);
		}
			
		COMPARE(rgRowStatus[g_lMaxPendRows], DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED);
		fTestPass=TRUE;	 
	}
CLEANUP:
	
	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	g_lMaxPendRows=-1;
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED status check all rows, DBROWSTATUS_E_NEWLYINSERTED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_6()
{  
	BYTE		*pData				= NULL;
	DBPROPID	rgPropertyIDs[1];
	DBPROPID	rgUnPropertyIDs[1];
	BOOL		fTestPass			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[1];
	HROW		HRowX				= NULL;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgUnPropertyIDs[0]	= DBPROP_CHANGEINSERTEDROWS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),DB_E_ERRORSOCCURRED);
	COMPARE(rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED);

	fTestPass=TRUE;	 
CLEANUP:
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DeleteRows from a zombie.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_7()
{  
	ITransactionLocal	*pITransactionLocal	= NULL;
	DBPROPID			rgPropertyIDs[1];
	BOOL				fTestPass			= TEST_SKIPPED;
	DBROWSTATUS			rgRowStatus[1];
	DBPROPID			rgUnPropertyIDs[1];
	HROW				*pHRow				= NULL;
	DBCOUNTITEM			cRows			= 0;
	ULONG				ulTransactionLevel	= 0;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;

	rgUnPropertyIDs[0]	= DBPROP_COMMITPRESERVE;

	//Get ITransactionLocal on test session
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		odtLog << L"ITransactionLocal not supported."<<ENDL;
		return TEST_SKIPPED;
	}
	
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, &ulTransactionLevel),S_OK);	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));


	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=TEST_FAIL;

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//commit the session
	if(XACT_E_NOTSUPPORTED==pITransactionLocal->Commit(FALSE, XACTTC_ASYNC_PHASEONE, 0))
	{
		if(XACT_E_NOTSUPPORTED==pITransactionLocal->Commit(FALSE, XACTTC_SYNC_PHASEONE, 0))
		{
			if(XACT_E_NOTSUPPORTED==pITransactionLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0))
			{
				if(XACT_E_NOTSUPPORTED==pITransactionLocal->Commit(FALSE, XACTTC_SYNC, 0))
				{
					if(XACT_E_NOTSUPPORTED==pITransactionLocal->Commit(FALSE, 0, 0))
					{
						//commit retaining not supported
						odtLog << L"Non-Retaining Commit not supported, returning TEST_PASS."<<ENDL;
						fTestPass = TEST_SKIPPED;
						goto CLEANUP;
					}
				}
			}
		}
	}	

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),E_UNEXPECTED);
	
	fTestPass=TEST_PASS;	 
CLEANUP:
	
	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	SAFE_RELEASE(pITransactionLocal);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED status check all rows, not DBROWSTATUS_E_PENDINGINSERT.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_8()
{  
	BYTE		*pData				= NULL;
	DBPROPID	rgPropertyIDs[2];
	BOOL		fTestPass			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[1];
	HROW		HRowX				= NULL;
	DBPROPID	rgUnPropertyIDs[1];

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	//DBROWSTATUS_E_PENDINGINSERT is not a valid return from DeleteRows
	//even with DBPROP_RETURNPENDINGINSERTS set to FALSE
	rgUnPropertyIDs[0]	= DBPROP_RETURNPENDINGINSERTS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	fTestPass=TRUE;	 
CLEANUP:
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc use pending deletes by using them in other methods 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_9()
{  
	BYTE			*pDataV				= NULL;	
	BYTE			*pData				= NULL;	
	DBCOUNTITEM		cRows			= 0;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	DBROWSTATUS		*prgRowStatus		= NULL;
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cRowsResynched		= 0;
	HROW			*rghRowsResynched	= NULL;
	IRowsetResynch	*pIRowsetResynch	= NULL;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_IRowsetResynch;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	if(!VerifyInterface(m_pIRowsetChange, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown**)&pIRowsetResynch))
	{
		goto CLEANUP;
	}
	TESTC_(pIRowsetResynch->ResynchRows(0, pHRow, &cRowsResynched, &rghRowsResynched, &prgRowStatus),S_OK);
	COMPARE(prgRowStatus[0], DBROWSTATUS_S_OK);

	if(!(pDataV=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	TESTC_(pIRowsetResynch->GetVisibleData(*pHRow, m_hAccessor,pDataV), S_OK);
	
	fTestPass=TRUE;	 
CLEANUP:
	
	if(m_pIRowset) {
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(rghRowsResynched);

	SAFE_RELEASE(pIRowsetResynch);

	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataV);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc use pending deletes by using them in other methods 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_10()
{  
	BYTE			*pData				= NULL;
	DBCOUNTITEM		cRows			= 0;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	DBROWSTATUS		*prgRowStatus		= NULL;
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cPendingRows		= 0;
	HROW			*rgPendingRows		= NULL;
	DBPENDINGSTATUS	rgPendingStatus[1];
	DBPENDINGSTATUS	*prgPendingStatus	= NULL;
	HRESULT			hr					= S_OK;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_CHANGE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_IRowsetIdentity;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,cRows,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	TESTC_(SetData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);
	//get the data
	hr=GetData(*pHRow, m_hAccessor, pData);
	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	//the following methods should support a row handle to a pending insert
	TESTC_(m_pIRowsetUpdate->GetRowStatus(NULL,1,pHRow,rgPendingStatus),S_OK);
	COMPARE(rgPendingStatus[0], DBPENDINGSTATUS_DELETED);

	TESTC_(m_pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&prgPendingStatus),S_OK);
	TESTC_(m_pIRowsetIdentity->IsSameRow(*pHRow,rgPendingRows[0]),S_OK);
	COMPARE(prgPendingStatus[0], DBPENDINGSTATUS_DELETED);

	TESTC_(m_pIRowsetUpdate->Undo(NULL,1,pHRow,NULL,NULL,&prgRowStatus),S_OK);
	COMPARE(prgRowStatus[0], DBROWSTATUS_S_OK);
	
	TESTC_(m_pIRowsetUpdate->GetOriginalData(*pHRow,m_hAccessor,pData),S_OK);

	fTestPass=TRUE;	 
CLEANUP:

	if(m_pIRowset) {
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
		CHECK(m_pIRowset->ReleaseRows(cPendingRows,rgPendingRows,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);
	PROVIDER_FREE(rgPendingRows);

	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(prgPendingStatus);
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc delete pending insert, Change
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_11()
{  
	BYTE			*pData				= NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	HROW			HRowX				= NULL;
	HRESULT			hr					= S_OK;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate || (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//try to use HRowX
	//set data
	TESTC_(SetData(HRowX,m_hAccessor,pData),DB_E_DELETEDROW);
	
	//get the data
	hr=GetData(HRowX, m_hAccessor, pData);
	//if GetData returned S_OK, it is legal here because reporting the row is actually a pending delete
	//may be too expense for some providers
	if (hr!=DB_E_DELETEDROW && hr!=S_OK)
	{
		goto CLEANUP;
	}

	fTestPass=TRUE;	 
CLEANUP:
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc delete pending insert, Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_12()
{  
	BYTE			*pData				= NULL;
	DBPROPID		rgPropertyIDs[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	DBROWSTATUS		*prgRowStatus		= NULL;
	HROW			HRowX				= NULL;
	DBCOUNTITEM		cPendingRows		= 0;
	HROW			*rgPendingRows		= NULL;
	DBPENDINGSTATUS	rgPendingStatus[1];
	DBPENDINGSTATUS	*prgPendingStatus	= NULL;


	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(
		(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//try to use HRowX
	//it should fail in everything except ReleaseRows

	TESTC_(m_pIRowsetUpdate->GetOriginalData(HRowX,m_hAccessor,pData),DB_E_DELETEDROW);

	TESTC_(m_pIRowsetUpdate->GetRowStatus(NULL,1,&HRowX,rgPendingStatus),DB_E_ERRORSOCCURRED);
	COMPARE(rgPendingStatus[0], DBPENDINGSTATUS_INVALIDROW);

	TESTC_(m_pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&prgPendingStatus),S_FALSE);

	TESTC_(m_pIRowsetUpdate->Undo(NULL,1,&HRowX,NULL,NULL,&prgRowStatus),DB_E_ERRORSOCCURRED);
	COMPARE(prgRowStatus[0], DBROWSTATUS_E_DELETED);

	fTestPass=TRUE;	 
CLEANUP:

	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cPendingRows,rgPendingRows,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(prgPendingStatus);
	PROVIDER_FREE(pData);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc delete pending insert, Resync
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_13()
{  
	BYTE			*pData				= NULL;
	BYTE			*pDataV				= NULL;
	DBPROPID		rgPropertyIDs[3];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	DBROWSTATUS		*prgRowStatus		= NULL;
	HROW			HRowX				= NULL;
	DBCOUNTITEM		cRowsResynched		= 0;
	HROW			*rghRowsResynched	= NULL;
	IRowsetResynch	*pIRowsetResynch	= NULL;


	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_IRowsetResynch;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//try to use HRowX
	//it should fail in everything except ReleaseRows
	//set data
	//Resyncs
	if(!VerifyInterface(m_pIRowsetChange, IID_IRowsetResynch, ROWSET_INTERFACE, (IUnknown**)&pIRowsetResynch))
	{
		goto CLEANUP;
	}

	TESTC_(pIRowsetResynch->ResynchRows(1, &HRowX, &cRowsResynched, &rghRowsResynched, &prgRowStatus),DB_E_ERRORSOCCURRED);
	COMPARE(prgRowStatus[0], DBROWSTATUS_E_DELETED);

	if(!(pDataV=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}
	TESTC_(pIRowsetResynch->GetVisibleData(HRowX, m_hAccessor,pDataV), DB_E_DELETEDROW);
					
	fTestPass=TRUE;	 
CLEANUP:
	
	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRowsResynched,rghRowsResynched,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(rghRowsResynched);
	
	SAFE_RELEASE(pIRowsetResynch);

	PROVIDER_FREE(prgRowStatus);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pDataV);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc check for pending delete, LITERALIDENTITY - FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_14()
{  
	DBPROPID		rgPropertyIDs[4];
	DBPROPID		rgUnPropertyIDs[1];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	HRESULT			hr					= S_OK;
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cRows			= 0;


	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_REMOVEDELETED;
	rgPropertyIDs[3]	= DBPROP_CANHOLDROWS;

	rgUnPropertyIDs[0]	= DBPROP_LITERALIDENTITY;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgPropertyIDs,1,rgUnPropertyIDs,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//restart position. 
	hr = m_pIRowset->RestartPosition(NULL);
	CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);
	
	//retrieve row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//should be deleted row and here, eventhough DBPROP_REMOVEDELETED is variant_true
	//literidentity if variant_false so pending deleted rows are returned
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),DB_E_DELETEDROW);
CLEANUP:

	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc check for pending delete, LITERALIDENTITY - TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_15()
{  
	DBPROPID		rgPropertyIDs[5];
	BOOL			fTestPass			= TEST_SKIPPED;
	DBROWSTATUS		rgRowStatus[1];
	HRESULT			hr					= S_OK;
	HROW			*pHRow				= NULL;
	DBCOUNTITEM		cRows			= 0;
	BYTE			*pData				= NULL;


	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_IRowsetUpdate;
	rgPropertyIDs[2]	= DBPROP_REMOVEDELETED;
	rgPropertyIDs[3]	= DBPROP_LITERALIDENTITY;
	rgPropertyIDs[4]	= DBPROP_CANHOLDROWS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_pIRowsetUpdate|| (!m_cBinding))
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;
	}

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	TESTC_(GetData(*pHRow,m_hAccessor,pData),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//restart position. 
	hr = m_pIRowset->RestartPosition(NULL);
	CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);
	
	//retrieve row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//DBPROP_REMOVEDELETED is VARIANT_TRUE
	TESTC_(GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	//these should be different rows
	if(COMPARE(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY),FALSE))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	if(m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	PROVIDER_FREE(pData);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Deleted handle, no status
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_16()
{  
	DBCOUNTITEM		cRows				= 0;
	DBPROPID		rgPropertyIDs[2]	= {NULL,NULL};
	BOOL			fTestPass			= TEST_SKIPPED;
	HROW			*pHRow				= NULL;
	BYTE			*pData				= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_CANHOLDROWS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);
	//delete the row again but do not use a status
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),DB_E_ERRORSOCCURRED);
	
	fTestPass=TRUE;
CLEANUP:
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invlaid handle, no status
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_17()
{  
	DBCOUNTITEM		cRows				= 0;
	DBPROPID		rgPropertyIDs[2]	= {NULL,NULL};
	BOOL			fTestPass			= TEST_SKIPPED;
	HROW			rgHRow[1];
	HROW			*pHRow				= NULL;
	BYTE			*pData				= NULL;

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE;

	// Check to see if supported
	if( m_ulpProvUpdFlags && (m_ulpProvUpdFlags & m_ulpUpdFlags) )
		m_ulpUpdFlags = m_ulpProvUpdFlags;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_CANHOLDROWS;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	//retrieve row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	rgHRow[0] = *pHRow+11;

	//delete the row with an invlaid handle but do not use a status
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&rgHRow[0],NULL),DB_E_ERRORSOCCURRED);
	
	fTestPass=TRUE;
CLEANUP:
	if(m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
	}
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_S_OK when deleting an inserted/trasmitted row from a pending rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_18()
{  
	BYTE		*pData				= NULL;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[1];
	HROW		HRowX				= NULL;
	DBROWSTATUS	*rgDBRowStatus		= NULL;

	//muck up the DBROWSTATUS
	memset(rgRowStatus,1,sizeof(DBROWSTATUS)*(1));

	// Initialize UPDATABILITY flag for class
	m_ulpUpdFlags = DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE;

	rgPropertyIDs[0]	= DBPROP_IRowsetChange;
	rgPropertyIDs[1]	= DBPROP_CHANGEINSERTEDROWS;
	rgPropertyIDs[2]	= DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	
	fTestPass=FALSE;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK);
		  		
	//insert a new row		
	TESTC_(m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,&HRowX),S_OK);

	//call update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&HRowX,NULL,NULL,&rgDBRowStatus),S_OK);	
	COMPARE(rgDBRowStatus[0], DBROWSTATUS_S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&HRowX,rgRowStatus),S_OK);
	COMPARE(rgRowStatus[0], DBSTATUS_S_OK);

	fTestPass=TRUE;	 
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgDBRowStatus);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc delete/update/reposition cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_19()
{
	DBCOUNTITEM	cRows				= 0;
	BYTE		*pData				= NULL;
	HROW		*pHRow				= NULL;
	DBPROPID	rgPropertyIDs[3];
	BOOL		fTestPass			= TEST_SKIPPED;
	ULONG		cProperty			= 0;
	HRESULT		hr;

	//allocation memory
	pHRow = (HROW*)PROVIDER_ALLOC(sizeof(HROW));

	rgPropertyIDs[0]=DBPROP_IRowset;
	rgPropertyIDs[1]=DBPROP_IRowsetChange;	
	rgPropertyIDs[2]=DBPROP_IRowsetUpdate;

	// Initialize 
	m_ulpUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange || !m_cBinding)
	{
		goto CLEANUP;
	}
	fTestPass=FALSE;

	//retrieve the ith row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//release my main row
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//retrieve the ith row handle again
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,pHRow,NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,pHRow,NULL,NULL,NULL),S_OK);
		
	//release my main row
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	//retrieve the ith row handle
	TESTC_(hr=m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);	

	fTestPass=TRUE;
CLEANUP:			
	if(pHRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
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
BOOL DeleteRows::Terminate()
{
	return(TCIRowsetChange::Terminate());
}	// }}
// }}















































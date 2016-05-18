//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module irownew.CPP | The test module for IRowsetNewRow.
//

#include "modstandard.hpp"
#include "irownew.h"
#include "extralib.h"


#undef TESTC_PROVIDER
#define TESTC_PROVIDER(hr) { TESTB = TEST_PASS; if(hr==S_FALSE) { TOUTPUT(L"NotSupported by Provider, skipping Variation"); fTestPass = TEST_SKIPPED; goto CLEANUP; } }



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x2d8c4a62, 0x336d, 0x11d1, { 0xae, 0xde, 0x0, 0xc0, 0x4f, 0xd7, 0x6, 0x80 } };
DECLARE_MODULE_NAME("IRowsetNewRow");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("The test module for IRowsetNewRow");
DECLARE_MODULE_VERSION(839384934);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END
// }}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//DBPROPID	g_rgDBPropID[2]={DBPROP_OTHERINSERT, DBPROP_OTHERUPDATEDELETE};
//DBCOUNTITEM	g_ulRowCount;
DBPROPID g_rgDBPropID[3]={	DBPROP_OTHERINSERT, DBPROP_OTHERUPDATEDELETE,DBPROP_OWNUPDATEDELETE};
DBCOUNTITEM g_ulRowCount;


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// Return the status value indicated by the binding structure 
//--------------------------------------------------------------------
DBSTATUS GetStatus(void *pData, DBBINDING *pBinding)
{
	BYTE	*pbAddr;

	pbAddr=(BYTE *)pData;
	return(*(DBSTATUS *)(pbAddr+(pBinding->obStatus)));
}

//--------------------------------------------------------------------
// @func Module level initialization routiner
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule *pThisTestModule)
{
	BOOL fPass = CommonModuleInit(pThisTestModule, IID_IRowsetChange, 15);

	if (TEST_PASS == fPass)
	{
		g_ulRowCount = g_pTable->GetRowsOnCTable();
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
BOOL ModuleTerminate(CThisTestModule *pThisTestModule)
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
//	TCIRowsetNewRow:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCIRowsetNewRow : public CRowsetObject
{
	private:

	protected:

		//@cmember: interface pointer for IRowsetChange
		IRowsetChange		*m_pIRowsetChange;

		//@cmember: interface pointer for IRowsetUpdate
		IRowsetUpdate		*m_pIRowsetUpdate;

		//@cmember: interface pointer for IRowsetLocate
		IRowsetLocate		*m_pIRowsetLocate;

		IRowsetIdentity		*m_pIRowsetIdentity;

		//@cmember: interface pointer for IRowset
		IRowset				*m_pIRowset;

		//@cmember:	accessory handle
		HACCESSOR			m_hAccessor;

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

		//@cmember: the location of accessor handle
		EACCESSORLOCATION	m_eAccessorLocation;

		//@cmember 
		DBCOUNTITEM			m_ulTableRows;

		//@cmember 
		BOOL				m_bIndexExists;

		//@cmember: The Updatability Flags for DBPROP_UPDATABILITY
		ULONG				m_ulUpdFlags;
		
		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object and set properties, execute a sql statement,
		//		  and create a rowset object.  Create an accessor on the rowset 
		HRESULT GetRowsetAndAccessor
		(	
			EQUERY				eSQLStmt,
			ULONG				cProperties				= 0,			
			const DBPROPID		*rgProperties			= NULL,			
			ULONG				cPropertiesUnset		= 0,
			const DBPROPID		*rgPropertiesUnset		= NULL,
			EACCESSORLOCATION	eAccessorLocation		= NO_ACCESSOR,
			BOOL				fBindLongColumn			= FALSE,
			DBACCESSORFLAGS		dwAccessorFlags			= DBACCESSOR_ROWDATA,		
			DBPART				dwPart					= DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind				= ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder			= FORWARD,			
			ECOLS_BY_REF		eColsByRef				= NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier			= DBTYPE_EMPTY,
			DBORDINAL			cColsToBind			 	= 0,
			DBORDINAL			*rgColsToBind			= NULL,
			ECOLS_MEM_PROV_OWNED 	eColsMemProvOwned	= NO_COLS_OWNED_BY_PROV,
			DBPARAMIO			eParamIO				= DBPARAMIO_NOTPARAM
		);


		//@mfun: create an accessor on the rowset.  
		BOOL	GetAccessorOnRowset
		(
			EACCESSORLOCATION	eAccessorLocation	= ON_ROWSET_ACCESSOR,
			BOOL				fBindLongColumn		= FALSE,
			DBACCESSORFLAGS		dwAccessorFlags		= DBACCESSOR_ROWDATA,		
			DBPART				dwPart				= DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind			= UPDATEABLE_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder		= FORWARD,			
			ECOLS_BY_REF		eColsByRef			= NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier		= DBTYPE_EMPTY,
			DBORDINAL			cColsToBind			= 0,
			DBORDINAL			*rgColsToBind		= NULL,
			DBPARAMIO			eParamIO			= DBPARAMIO_NOTPARAM//@paramopt [IN] Parameter type to specify for eParmIO
		);

		//@mfun: Get the bookmark for the row 
		BOOL	GetBookmark
		(
			ULONG		ulRow,
			DBBKMARK	*pcbBookmark,
			BYTE		**ppBookmark
		);

		HRESULT	GetBookmarkByRow
		(
			HROW		hRow,
			DBBKMARK	*pcbBookmark,
			BYTE		**ppBookmark
		);

		//@mfunc: Get cursor type of the rowset
		ECURSOR	GetCursorType();

		//@mfunc: Return TRUE if the deleted rows are removed
		BOOL	RemoveDeleted();

		//@mfunc: Return TRUE if the ROWRESTICT is TRUE.
		BOOL	RowStrict();

		//@mfunc: Return TRUE if the property is VARIANT_TRUE.
		BOOL	GetProp(DBPROPID DBPropID);

		//@mfunc: Return TRUE if the property is set with dwPropVal.
		BOOL	CheckProp(DWORD dwPropVal);

		//@mfun: Return TRUE if we are on buffered update mode
		BOOL	BufferedUpdate();

		//@mfun: Return TRUE if strong identity
		BOOL	StrongIdentity();

		BOOL	GetUpdatableCols(DBORDINAL	*pcbCol,
								DBORDINAL	**prgColNum);


		//@mfunc: Get the Nullable and Updatable column
		BOOL	GetNullableAndUpdatable(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);


 		//@mfunc: Get the Not-Nullable and Updatable column
		BOOL	GetNotNullableAndUpdatable(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);


		//@mfunc: Get the Fixed Length and Updatable column
		BOOL	GetFixedLengthAndUpdatable(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);

		//@mfunc: Get the Variable Length and Updatable column
		BOOL	GetVariableLengthAndUpdatable(DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum);

		//@mfunc: Get not updatable column
		DBORDINAL	GetNotUpdatable();

		//@mfunc: Get first updatable column
		DBORDINAL	GetFirstUpdatable();

		//@mfunc: Get long and updatable columns
		DBORDINAL	GetLongAndUpdatable();

		//@mfunc: release the memory referenced by the consumer's buffer
		void FreeMemory();

		//@mfunc: release the accessor on the rowset
		void ReleaseAccessorOnRowset();

		//@mfunc: release a rowset object and accessor created on it
		void ReleaseRowsetAndAccessor();

		//@mfunc: Check if the row data in pRowData matches a row obtainable from pIRowset
		BOOL CheckRowVisible(IRowset *pIRowset, void *pRowData, BOOL fRowVisible);

	public:
		//constructor
		TCIRowsetNewRow(WCHAR *wstrTestCaseName);

		//destructor
		~TCIRowsetNewRow();
};


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetNewRow constructor, must take testcase name
//			as parameter.
//
TCIRowsetNewRow::TCIRowsetNewRow(WCHAR * wstrTestCaseName)	//Takes TestCase Class name as parameter
						: CRowsetObject(wstrTestCaseName) 
{

		m_pIRowsetChange=NULL;
		m_pIRowsetUpdate=NULL;
		m_pIRowsetLocate=NULL;
		m_pIRowsetIdentity=NULL;
		m_pIRowset=NULL;
		m_hAccessor=NULL;
		m_cRowSize=0;
		m_cBinding=0;
		m_rgBinding=NULL;
		m_rgInfo=NULL;
		m_pStringsBuffer=NULL;
		m_pData=NULL;
		m_eAccessorLocation=NO_ACCESSOR;
		m_ulTableRows=0;
		m_bIndexExists = FALSE;
		m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;
}


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetNewRow destructor
//
TCIRowsetNewRow::~TCIRowsetNewRow()
{

}

//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetNewRow::Init()
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

	m_ulTableRows = g_pTable->CountRowsOnTable();

	g_pTable->DoesIndexExist(&m_bIndexExists);

	return TRUE;

}


//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetNewRow::Terminate()
{
	//Release the existing Session
	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();
	return(CRowsetObject::Terminate());
}

	
//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
HRESULT	TCIRowsetNewRow::GetRowsetAndAccessor
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
	ECOLS_MEM_PROV_OWNED 	eColsMemProvOwned,	//@paramopt [IN] Which columns' memory is to be owned by the provider
	DBPARAMIO			eParamIO				//@paramopt [IN] Parameter type to specify for eParmIO
)
{
	IColumnsInfo		*pIColumnsInfo			= NULL;
	ICommandProperties	*pICommandProperties	= NULL;
	DBCOUNTITEM			cRowsObtained			= 0;
	HROW				*pHRow					= NULL;
	ULONG				cDBPropSet				= 1;
	DBPROPSET			rgDBPropSet[2]			= {NULL,NULL};
	ULONG				cProp					= 0;
	ULONG				cNumErrors				= 0;
	ULONG				cNumNotSupported		= 0;
	HRESULT				hr						= S_FALSE;
	HRESULT				hrReturn				= S_FALSE;
	BLOBTYPE			blobType;
	BOOL				fTestPass				= FALSE;
	ULONG				cPropSets				= 0;
	DBPROPSET			*rgPropSets				= NULL;	

	if(fBindLongColumn)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;

	//init DBPropSet[0]
	rgDBPropSet[0].rgProperties   = NULL;
	rgDBPropSet[0].cProperties    = 0;
	rgDBPropSet[0].guidPropertySet= DBPROPSET_ROWSET;

	//Set up the DB Properties struct
	if(cProperties || cPropertiesUnset)
	{
		//Might need an extra for DBPROP_UPDATABILITY (+1)
		rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * (cProperties + cPropertiesUnset + 1));
		if(!rgDBPropSet[0].rgProperties)
			goto CLEANUP;
		
		//Memset to zeros
		memset(rgDBPropSet[0].rgProperties, 0, (sizeof(DBPROP) * (cProperties + cPropertiesUnset + 1))); 

		ULONG i;
		//go through the loop to set every DB Property required
		for(i=0; i<cProperties; i++)
		{
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID   = rgProperties[i];
			rgDBPropSet[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].colid		= DB_NULLID;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
			V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)= VARIANT_TRUE;
			cProp++;
		}

		if ( DBPROPFLAGS_WRITE & GetPropInfoFlags(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pIDBInitialize))
		{
			//set DBPROP_UPDATABILITY
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID	= DBPROP_UPDATABILITY;
			rgDBPropSet[0].rgProperties[cProp].dwOptions	= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].colid		= DB_NULLID;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt	= VT_I4;
			rgDBPropSet[0].rgProperties[cProp].vValue.lVal	= m_ulUpdFlags;
			cProp++;
		}

		//go through the loop to unset every DB Property required
		for(i=0; i<cPropertiesUnset; i++)
		{
			rgDBPropSet[0].rgProperties[cProp].dwPropertyID		= rgPropertiesUnset[i];
			rgDBPropSet[0].rgProperties[cProp].dwOptions		= DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cProp].colid			= DB_NULLID;
			rgDBPropSet[0].rgProperties[cProp].vValue.vt		= VT_BOOL;
			V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)	= VARIANT_FALSE;
			cProp++;
		}

		rgDBPropSet[0].cProperties = cProp;
	}
	  
	
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
		
		//get the accessor handle
		hr=GetAccessorAndBindings(	m_pICommand,dwAccessorFlags,&m_hAccessor,&m_rgBinding,&m_cBinding,&m_cRowSize,
									dwPart,eColsToBind,eBindingOrder,eColsByRef,NULL,NULL,NULL,dbTypeModifier,
									cColsToBind,(LONG_PTR *)rgColsToBind,NULL,eColsMemProvOwned,eParamIO,blobType);

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

	if(!m_pIDBCreateCommand &&
	   (eSQLStmt==SELECT_ALLFROMTBL || eSQLStmt==SELECT_ORDERBYNUMERIC || eSQLStmt==USE_SUPPORTED_SELECT_ALLFROMTBL))
	{
		eSQLStmt = USE_OPENROWSET;
	}
		
	//create the rowset object May fail due to combinations of properties
	hr = CreateRowsetObject(eSQLStmt,IID_IRowset,EXECUTE_IFNOERROR);
	if(hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED)
	{
		if ( eSQLStmt != USE_OPENROWSET && hr == DB_E_ERRORSOCCURRED)
		{
			//GetProperties with DBPROPSET_PROPERTIESINERROR.					
			if ( hr == DB_E_ERRORSOCCURRED && m_pICommand )
			{
				//Setup input DBPROPSET_PROPERTIESINERROR
				const ULONG cPropertyIDSets = 1;
				DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
				rgPropertyIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;
				rgPropertyIDSets[0].cPropertyIDs = 0;
				rgPropertyIDSets[0].rgPropertyIDs = NULL;

				if ( FAILED(m_pICommand->QueryInterface(IID_ICommandProperties, (void**)&pICommandProperties)) )
					goto CLEANUP;
				if ( FAILED(pICommandProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets)) )
					goto CLEANUP;
			}
		}

		//if PIE returned no errors use the error status returned on the SetPropperties call
		//PIE does not necessarily have to return error if it is being called from a layer on top of
		//the provider, ie. Service Components or Remoting
		if (!rgPropSets)
		{
			DumpPropertyErrors(m_cPropSets, m_rgPropSets, &cNumErrors, &cNumNotSupported);
		}
		else
		{
			if (rgPropSets->rgProperties)
			{
				DumpPropertyErrors(cPropSets, rgPropSets, &cNumErrors, &cNumNotSupported);
			}
			else
			{
				DumpPropertyErrors(m_cPropSets, m_rgPropSets, &cNumErrors, &cNumNotSupported);
			}
		}

		if( cNumErrors || cNumNotSupported )
		{
			if( cNumErrors )
			{
				odtLog << L"Bad property validation \n";
				hrReturn = E_FAIL;				
			}
			else if( cNumNotSupported )
			{
				hrReturn = S_FALSE; // so rest of code sees this is a Not Supported error
				odtLog << L"One or more required properties are not supported, not settable, or conflict \n";				
			}

			odtLog << L"Rowset not created; test not run.\n\n";
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

	//if eAccessorLocation is NO_ACCESSOR, no need to create an accessor
	if(eAccessorLocation==NO_ACCESSOR)
	{	
		hrReturn=S_OK;
		goto CLEANUP;
	}
	
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
				TESTC_(GetAccessorAndBindings(m_pIAccessor,dwAccessorFlags,&m_hAccessor,
				&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
				eColsByRef,NULL,NULL,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *)rgColsToBind,
				NULL,eColsMemProvOwned,eParamIO,blobType),S_OK);

			break;
		default:
			return FALSE;
	}


	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);
	if(!m_pData)
		goto CLEANUP;

	hrReturn=S_OK;

CLEANUP:
	PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandProperties);
	FreeProperties(&cPropSets, &rgPropSets);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		//restart position.  The rowset returns to its original state
		hr = m_pIRowset->RestartPosition(NULL);
		CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);	
	}	

	return hrReturn;
}

//--------------------------------------------------------------------
//@mfunc: Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::GetAccessorOnRowset
(	
	EACCESSORLOCATION	eAccessorLocation,		//where the accessor should be created
	BOOL				fBindLongColumn,		//whether to bind the column column
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	DBORDINAL			cColsToBind,			//the count of columns to bind
	DBORDINAL			*rgColsToBind,
	DBPARAMIO			eParamIO				//the array of column ordinals to bind
)
{
	IUnknown			*pIUnknown	= NULL;
	DBCOUNTITEM			cRowsObtained;
	HROW				*pHRow		= NULL;
	DBORDINAL			cCnt		= 0;
	BOOL				fPass		= FALSE;
	BLOBTYPE			blobType;

	if(fBindLongColumn)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;


	//remember where the accessor handle is created
	m_eAccessorLocation=eAccessorLocation;

	//eAccessorLocation can not be NO_ACCESSOR
	if(!COMPARE(eAccessorLocation!=NO_ACCESSOR,TRUE))
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

	//create an accessor on the rowset
	TESTC_(GetAccessorAndBindings(pIUnknown,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,&cCnt,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *) rgColsToBind,
		NULL,NO_COLS_OWNED_BY_PROV,eParamIO,blobType),S_OK);

	//make sure cCnt should be the same as m_cRowsetCols
	if(!COMPARE(cCnt, m_cRowsetCols))
		goto CLEANUP;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(m_pData)
		fPass=TRUE;

CLEANUP:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		//restart position.  The rowset returns to its original state
		HRESULT hr = m_pIRowset->RestartPosition(NULL);
		CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);	
	}	

	return fPass;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetNewRow::GetBookmark
(
	ULONG		ulRow,
	DBBKMARK	*pcbBookmark,
	BYTE		**ppBookmark
)
{
	BOOL		fPass	= FALSE;
	HROW		hRow[1];
	HROW		*pHRow	= hRow;
	DBCOUNTITEM	cCount;

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
	fPass=CHECK(GetBookmarkByRow(*pHRow, pcbBookmark, ppBookmark),S_OK);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);

	COMPARE(cCount, 0);

	return fPass;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
HRESULT TCIRowsetNewRow::GetBookmarkByRow
(
		HROW		hRow,
		DBBKMARK	*pcbBookmark,
		BYTE		**ppBookmark
)
{
	HRESULT		hr = E_FAIL;
	void		*pData=NULL;
	HACCESSOR	hAccessor=NULL;
	DBCOUNTITEM	cBinding;
	DBLENGTH	cbRowSize;
	DBBINDING	*pBinding=NULL;
	DBORDINAL	ulColToBind=0;	

	//the rowset has to expose IRowset in order to have bookmark
	if(!m_pIRowset)
		return FALSE;

odtLog << "1 potato";
	//check the input
	if(!pcbBookmark || !ppBookmark)
		return FALSE;
odtLog << "2 potato";

	//create an accessor to binding the bookmark
	QTESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA, &hAccessor, &pBinding,
					&cBinding,&cbRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
					USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, 
					DBTYPE_EMPTY, 1, (LONG_PTR *)&ulColToBind),S_OK);

odtLog << "3 potato";

	//allocate memory 
	if(!(pData=PROVIDER_ALLOC(cbRowSize)))
		goto CLEANUP;
odtLog << "4 potato";

	//get the data
	QTESTC_(m_pIRowset->GetData(hRow, hAccessor, pData),S_OK);
odtLog << "5 potato";

	//get the length of the bookmark
//	*pcbBookmark= *((ULONG *)(dwAddr+pBinding[0].obLength));
	*pcbBookmark=LENGTH_BINDING(pBinding[0], pData);
odtLog << "6 potato";

	//allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);
odtLog << "7 potato";

	if(!(*ppBookmark))
		goto CLEANUP;
odtLog << "8 potato";

	//copy the value of the bookmark into the consumer's buffer
//	memcpy(*ppBookmark, (void *)(dwAddr+pBinding[0].obValue), *pcbBookmark);
	memcpy(*ppBookmark, &VALUE_BINDING(pBinding[0], pData), (size_t)*pcbBookmark);
odtLog << "9 potato";

	hr = S_OK;

CLEANUP:
	//release the memory
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBinding);

	//free the accessor
	if(hAccessor != DB_NULL_HACCESSOR)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	return hr;
}


//--------------------------------------------------------------------
//
//@mfunc: Get cursor type of the rowset.  Has to be called after a rowset 
//			generated.
//
//
//--------------------------------------------------------------------
ECURSOR	TCIRowsetNewRow::GetCursorType()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			rgDBPROPID[3];
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	ECURSOR				eCursor=FORWARD_ONLY_CURSOR;

	//initialization
	rgDBPROPID[0]=DBPROP_OTHERINSERT;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_OWNUPDATEDELETE;

	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=3;
	DBPropIDSet.rgPropertyIDs=rgDBPROPID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for the 3 properties
	pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	//only one property set should be returned
	if(!COMPARE(cProperty, 1))
		goto CLEANUP;

	// this is the odbc cursor model - valid only when running against kagera
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
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return eCursor;
}
//--------------------------------------------------------------------
//
//@mfunc: Return TRUE if the deleted rows are removed  Has to be called after
//			a rowset is generated.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::RemoveDeleted()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_REMOVEDELETED;
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
	if(!SUCCEEDED(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet)))
		goto CLEANUP;

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfunc: Return TRUE is strong identity
//
////--------------------------------------------------------------------

BOOL	TCIRowsetNewRow::StrongIdentity()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_STRONGIDENTITY;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;						
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for DBPROP_STRONGIDENTITY
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}


//--------------------------------------------------------------------
//
//@mfun: Return TRUE if we are on buffered update mode
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::BufferedUpdate()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_IRowsetUpdate;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	if (!m_pIRowsetUpdate)
		return FALSE;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for IID_IRowsetUpdate
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfun: Return TRUE if we are on buffered update mode
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::RowStrict()
{
	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPID			DBPropID=DBPROP_ROWRESTRICT;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSupported=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;


	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for ROWRESTRICT
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//---------------------------------------------------------------------------------
//
//	Get the updatable columns.
//
//
//---------------------------------------------------------------------------------
BOOL	TCIRowsetNewRow::GetUpdatableCols
(
DBORDINAL	*pcbCol,
DBORDINAL	**prgColNum
)
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
		if((m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE)||
		    (m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
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



//@mfunc: Return TRUE if the property is set with dwPropVal.
BOOL	TCIRowsetNewRow::CheckProp(DWORD dwPropVal)
{
 	IRowsetInfo			*pIRowsetInfo=NULL;
	ULONG				cProperty;
	DBPROPIDSET			DBPropIDSet;
	DBPROPID			DBPropID=DBPROP_UPDATABILITY;
	DBPROPSET			*pDBPropSet=NULL;
	BOOL				fSet=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;


	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for the property
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if((pDBPropSet->rgProperties->vValue).lVal & dwPropVal)
		fSet=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSet;

}

BOOL	TCIRowsetNewRow::GetProp(DBPROPID	DBPropID)
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
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for IID_IRowsetUpdate
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:
	if(pDBPropSet)
		PROVIDER_FREE(pDBPropSet->rgProperties);
	PROVIDER_FREE(pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfunc: Get the Nullable and Updatable column	ordinals arrays. 
//
//		  The function allocation memory for the ordinals array.  Return
//		  TURE is one or more nullable and updatable column exists'
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::GetNullableAndUpdatable(
DBORDINAL	*pcbCol,			//[out] the count of the rgColNum
DBORDINAL	**prgColNum			//[out] the col ordinals array
)
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
BOOL	TCIRowsetNewRow::GetNotNullableAndUpdatable(
DBORDINAL	*pcbCol,			//[out] the count of the rgColNum
DBORDINAL	**prgColNum			//[out] the col ordinals array
)
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

//--------------------------------------------------------------------
//
//@mfunc: Get the Fixed Length and Updatable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::GetFixedLengthAndUpdatable
(
DBORDINAL	*pcbCol,
DBORDINAL	**prgColNum
)
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
		//have to consider either fixed length or not nullable
		if((IsFixedLength(m_rgInfo[cColsCount].wType) ||
			!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISNULLABLE)) &&
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
//@mfunc: Get the Variable Length and Updatable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetNewRow::GetVariableLengthAndUpdatable
(
DBORDINAL	*pcbCol,
DBORDINAL	**prgColNum
)
{
	
	ULONG	cColsCount;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cRowsetCols);


	//add the 1st column, for the convenience of insert a new row
	*prgColNum[*pcbCol]=1;
	(*pcbCol)++;

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if((!IsFixedLength(m_rgInfo[cColsCount].wType) ||
			!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISNULLABLE)
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
		return TRUE;

	 return FALSE;
}

//--------------------------------------------------------------------
//
//@mfunc: Get not updatable column
//
//--------------------------------------------------------------------
DBORDINAL	TCIRowsetNewRow::GetNotUpdatable()
{
	
	ULONG	cColsCount;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		//skip column 0
		if(m_rgInfo[cColsCount].iOrdinal==0)
			continue;

		if(!(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE) &&
		   !(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN) 
		  )
			return m_rgInfo[cColsCount].iOrdinal;
	}
	
	return 0;
}

//--------------------------------------------------------------------
//
//@mfunc: Get first updatable column
//
//--------------------------------------------------------------------
DBORDINAL	TCIRowsetNewRow::GetFirstUpdatable()
{
	DBORDINAL cbCols = 0;
	DBORDINAL *rgCols = NULL;
	DBORDINAL ulUpdatableCol = 0;

	GetUpdatableCols(&cbCols, &rgCols);

	if (cbCols)
		ulUpdatableCol = rgCols[0];

	PROVIDER_FREE(rgCols);

	return ulUpdatableCol;
}

//--------------------------------------------------------------------
//
//@mfunc: Get not updatable column
//
//--------------------------------------------------------------------
DBORDINAL	TCIRowsetNewRow::GetLongAndUpdatable()
{
	
	ULONG	cColsCount;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{

		if((	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE) ||
				(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
			)
			&& 	(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_ISLONG)
		  )	   
			return m_rgInfo[cColsCount].iOrdinal;
	}
	
	return 0;
}

//--------------------------------------------------------------------
//@mfunc:	free the memory referenced by the consumer's buffer
//			The function has to be called after IRowset::GetData()
//
//--------------------------------------------------------------------
void TCIRowsetNewRow::FreeMemory()
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
void TCIRowsetNewRow::ReleaseAccessorOnRowset()
{
	//free the consumer buffer
	PROVIDER_FREE(m_pData);

	//free accessor handle
	if(m_hAccessor)
	{
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_pIAccessor && m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
		{
			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
			m_hAccessor=NULL;
		}
	}
	

	//free binding structure
	FreeAccessorBindings(m_cBinding, m_rgBinding);

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize  = 0;
	m_cBinding  = 0;
	m_rgBinding = NULL;
}


//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
void TCIRowsetNewRow::ReleaseRowsetAndAccessor()
{
	//free the consumer buffer
	PROVIDER_FREE(m_pData);

	//free accessor handle
	if(m_hAccessor)
	{
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_pIAccessor && m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
		{
			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
			m_hAccessor=NULL;
		}
	}
	
	//release IRowset pointer
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetChange);
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowsetLocate);
	SAFE_RELEASE(m_pIRowsetIdentity)

	//free accessor handle
	if(m_hAccessor)
	{
		//release the m_pIAccessor so that the rowset object is gone
		SAFE_RELEASE(m_pIAccessor);
		
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_pICommand && m_eAccessorLocation==ON_COMMAND_ACCESSOR)
		{
			IAccessor *pIAccessor = NULL;

			//QI for the accessor handle on the command object
			if(CHECK(m_pICommand->QueryInterface(IID_IAccessor, 
				(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
				SAFE_RELEASE(pIAccessor);
			}
		}

		m_hAccessor=NULL;
	}


	//free binding structure
	PROVIDER_FREE(m_rgInfo);
	PROVIDER_FREE(m_pStringsBuffer);
	FreeAccessorBindings(m_cBinding, m_rgBinding);

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize  = 0;
	m_cBinding  = 0;
	m_rgBinding = NULL;

	ReleaseRowsetObject(0);
	ReleaseCommandObject(0);
}


//--------------------------------------------------------------------
//@mfunc: Check if the row data in pRowData matches a row obtainable from pIRowset
//
//--------------------------------------------------------------------
BOOL TCIRowsetNewRow::CheckRowVisible(IRowset *pIRowset, void *pRowData, BOOL fRowIsVisible)
{
	HRESULT			hr			= E_FAIL;
	DBCOUNTITEM		cRows		= 0;
	HROW			*pHRow		= NULL;
	BOOL			fPass		= TEST_FAIL;
	BOOL			fVisible	= FALSE;

	while (S_OK==(hr = pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		
		//Get the data for the row handle
		TESTC_(pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pRowData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fVisible=TRUE;
			FreeMemory();
			break;
		}

		FreeMemory();
	}

CLEANUP:
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	return (fVisible == fRowIsVisible);
}


// - - - - - - - - d- - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Rowset)
//--------------------------------------------------------------------
// @class Test read-only rowsets
//
class Rowset : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Rowset,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  select count(*
	int Variation_1();
	// @cmember Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  The query is left outer join
	int Variation_2();
	// @cmember Empty rowset.  Should be able to insert one row.
	int Variation_3();
	// @cmember Insert the variable length columns without the length being bound.
	int Variation_4();
	// @cmember The accessor only has status binding for DBSTATUS_S_OK.
	int Variation_5();
	// @cmember The accessor only has status binding for DBSTATUS_S_ISNULL.
	int Variation_6();
	// @cmember The accessor only has status binding for DBSTATUS_S_ISNULL with not null cols
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Rowset)
#define THE_CLASS Rowset
BEG_TEST_CASE(Rowset, TCIRowsetNewRow, L"Test read-only rowsets")
	TEST_VARIATION(1, 		L"Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  select count(*")
	TEST_VARIATION(2, 		L"Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  The query is left outer join")
	TEST_VARIATION(3, 		L"Empty rowset.  Should be able to insert one row.")
	TEST_VARIATION(4, 		L"Insert the variable length columns without the length being bound.")
	TEST_VARIATION(5, 		L"The accessor only has status binding for DBSTATUS_S_OK.")
	TEST_VARIATION(6, 		L"The accessor only has status binding for DBSTATUS_S_ISNULL.")
	TEST_VARIATION(7, 		L"The accessor only has status binding for DBSTATUS_S_ISNULL with not null cols.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MayWriteColumn)
//--------------------------------------------------------------------
// @class Test DBPROP_MAYWRITECOLUMN.
//
class MayWriteColumn : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MayWriteColumn,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember MaywriteColumn set to FALSE to all columns
	int Variation_1();
	// @cmember MayWriteColumn set to TRUE to all columns.  Should fail.
	int Variation_2();
	// @cmember MayWriteColumn set to FALSE to all columns then insert.  Should fail.
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(MayWriteColumn)
#define THE_CLASS MayWriteColumn
BEG_TEST_CASE(MayWriteColumn, TCIRowsetNewRow, L"Test DBPROP_MAYWRITECOLUMN.")
	TEST_VARIATION(1, 		L"MaywriteColumn set to FALSE to all columns")
	TEST_VARIATION(2, 		L"MayWriteColumn set to TRUE to all columns.  Should fail.")
	TEST_VARIATION(3, 		L"MayWriteColumn set to FALSE to all columns then insert.  Should fail.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ComputedColumns)
//--------------------------------------------------------------------
// @class test computed columns by executing select col1, col1-col1 from table
//
class ComputedColumns : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ComputedColumns,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Accessor binds the computed columns.  DBSTATUS_E_PERMISSIONDENIED
	int Variation_1();
	// @cmember In immediate update mode, call InsertRow to insert a new row with a new value for the second column
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ComputedColumns)
#define THE_CLASS ComputedColumns
BEG_TEST_CASE(ComputedColumns, TCIRowsetNewRow, L"test computed columns by executing select col1, col1-col1 from table")
	TEST_VARIATION(1, 		L"Accessor binds the computed columns.  DBSTATUS_E_PERMISSIONDENIED")
	TEST_VARIATION(2, 		L"In immediate update mode, call InsertRow to insert a new row with a new value for the second column")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Forward_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Forward_Cursor_Immediate
//
class Forward_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Forward_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Forward_Cursor_Immediate)
#define THE_CLASS Forward_Cursor_Immediate
BEG_TEST_CASE(Forward_Cursor_Immediate, TCIRowsetNewRow, L"Forward_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Forward_Query_Buffered)
//--------------------------------------------------------------------
// @class Forward_Query_Buffered
//
class Forward_Query_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Forward_Query_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the end of the rowset. Verify ref. count.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Forward_Query_Buffered)
#define THE_CLASS Forward_Query_Buffered
BEG_TEST_CASE(Forward_Query_Buffered, TCIRowsetNewRow, L"Forward_Query_Buffered")
	TEST_VARIATION(1, 		L"Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the end of the rowset. Verify ref. count.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Static_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Static_Cursor_Buffered
//
class Static_Cursor_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Cursor_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the middle of rowset. Verify ref. count.
	int Variation_1();
	// @cmember Soft Insert.  Check visibility of data while row handle kept using GetData
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Static_Cursor_Buffered)
#define THE_CLASS Static_Cursor_Buffered
BEG_TEST_CASE(Static_Cursor_Buffered, TCIRowsetNewRow, L"Static_Cursor_Buffered")
	TEST_VARIATION(1, 		L"Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the middle of rowset. Verify ref. count.")
	TEST_VARIATION(2, 		L"Soft Insert.  Check visibility of data while row handle kept using GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Static_Query_Immediate)
//--------------------------------------------------------------------
// @class Static_Query_Immediate
//
class Static_Query_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Query_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Static_Query_Immediate)
#define THE_CLASS Static_Query_Immediate
BEG_TEST_CASE(Static_Query_Immediate, TCIRowsetNewRow, L"Static_Query_Immediate")
	TEST_VARIATION(1, 		L"Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Keyset_Cursor_Immediate
//
class Keyset_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the end of rowset.  Insert a new row to the 1st row with NULL.  Verify the ref count of the row handle.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Keyset_Cursor_Immediate)
#define THE_CLASS Keyset_Cursor_Immediate
BEG_TEST_CASE(Keyset_Cursor_Immediate, TCIRowsetNewRow, L"Keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Move the cursor to the end of rowset.  Insert a new row to the 1st row with NULL.  Verify the ref count of the row handle.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Keyset_Cursor_Buffer)
//--------------------------------------------------------------------
// @class Keyset_Cursor_Buffer
//
class Keyset_Cursor_Buffer : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Cursor_Buffer,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the begining of the rowset.  Insert a new row  to the end of rowset. Verify ref. count.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Keyset_Cursor_Buffer)
#define THE_CLASS Keyset_Cursor_Buffer
BEG_TEST_CASE(Keyset_Cursor_Buffer, TCIRowsetNewRow, L"Keyset_Cursor_Buffer")
	TEST_VARIATION(1, 		L"Move the cursor to the begining of the rowset.  Insert a new row  to the end of rowset. Verify ref. count.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Dynamic_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Dynamic_Cursor_Immediate
//
class Dynamic_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the end of rowset.  Insert a new row wiht NULL to  1st row.  Verify the reference count of the row handle.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Dynamic_Cursor_Immediate)
#define THE_CLASS Dynamic_Cursor_Immediate
BEG_TEST_CASE(Dynamic_Cursor_Immediate, TCIRowsetNewRow, L"Dynamic_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Move the cursor to the end of rowset.  Insert a new row wiht NULL to  1st row.  Verify the reference count of the row handle.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Dynamic_Query_Buffered)
//--------------------------------------------------------------------
// @class Dynamic_Query_Buffered
//
class Dynamic_Query_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic_Query_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Move the cursor to the begining of the rowset.  Insert a new row to the middle of the rowset. Verify ref. count.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Dynamic_Query_Buffered)
#define THE_CLASS Dynamic_Query_Buffered
BEG_TEST_CASE(Dynamic_Query_Buffered, TCIRowsetNewRow, L"Dynamic_Query_Buffered")
	TEST_VARIATION(1, 		L"Move the cursor to the begining of the rowset.  Insert a new row to the middle of the rowset. Verify ref. count.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Static_Command_Immediate)
//--------------------------------------------------------------------
// @class Visible_Static_Command_Immediate
//
class Visible_Static_Command_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Static_Command_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Static_Command_Immediate)
#define THE_CLASS Visible_Static_Command_Immediate
BEG_TEST_CASE(Visible_Static_Command_Immediate, TCIRowsetNewRow, L"Visible_Static_Command_Immediate")
	TEST_VARIATION(1, 		L"Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Keyset_Command_Buffered)
//--------------------------------------------------------------------
// @class Visible_Keyset_Command_Buffered
//
class Visible_Keyset_Command_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Keyset_Command_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Keyset_Command_Buffered)
#define THE_CLASS Visible_Keyset_Command_Buffered
BEG_TEST_CASE(Visible_Keyset_Command_Buffered, TCIRowsetNewRow, L"Visible_Keyset_Command_Buffered")
	TEST_VARIATION(1, 		L"Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Dynamic_Command_Buffered)
//--------------------------------------------------------------------
// @class Visible_Dynamic_Command_Buffered
//
class Visible_Dynamic_Command_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Dynamic_Command_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Dynamic_Command_Buffered)
#define THE_CLASS Visible_Dynamic_Command_Buffered
BEG_TEST_CASE(Visible_Dynamic_Command_Buffered, TCIRowsetNewRow, L"Visible_Dynamic_Command_Buffered")
	TEST_VARIATION(1, 		L"Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Visible_Dynamic_Query_Immediate)
//--------------------------------------------------------------------
// @class Visible_Dynamic_Query_Immediate
//
class Visible_Dynamic_Query_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Visible_Dynamic_Query_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Visible_Dynamic_Query_Immediate)
#define THE_CLASS Visible_Dynamic_Query_Immediate
BEG_TEST_CASE(Visible_Dynamic_Query_Immediate, TCIRowsetNewRow, L"Visible_Dynamic_Query_Immediate")
	TEST_VARIATION(1, 		L"Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BMK_Forward_Query_Immediate)
//--------------------------------------------------------------------
// @class BMK_Forward_Query_Immediate
//
class BMK_Forward_Query_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BMK_Forward_Query_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.
	int Variation_1();
	// @cmember InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BMK_Forward_Query_Immediate)
#define THE_CLASS BMK_Forward_Query_Immediate
BEG_TEST_CASE(BMK_Forward_Query_Immediate, TCIRowsetNewRow, L"BMK_Forward_Query_Immediate")
	TEST_VARIATION(1, 		L"call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.")
	TEST_VARIATION(2, 		L"InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BMK_Static_Query_Buffered)
//--------------------------------------------------------------------
// @class BMK_Static_Query_Buffered
//
class BMK_Static_Query_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BMK_Static_Query_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BMK_Static_Query_Buffered)
#define THE_CLASS BMK_Static_Query_Buffered
BEG_TEST_CASE(BMK_Static_Query_Buffered, TCIRowsetNewRow, L"BMK_Static_Query_Buffered")
	TEST_VARIATION(1, 		L"call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BMK_keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class BMK_keyset_Cursor_Immediate
//
class BMK_keyset_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BMK_keyset_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Try to set data to the bookmark column.  DB_E_ACCESSVIOLATION
	int Variation_1();
	// @cmember call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.
	int Variation_2();
	// @cmember InsertRow Data to insert a middle row.  Verify  by IRowsetLocate::GetRowsByBookmark
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BMK_keyset_Cursor_Immediate)
#define THE_CLASS BMK_keyset_Cursor_Immediate
BEG_TEST_CASE(BMK_keyset_Cursor_Immediate, TCIRowsetNewRow, L"BMK_keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Try to set data to the bookmark column.  DB_E_ACCESSVIOLATION")
	TEST_VARIATION(2, 		L"call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.")
	TEST_VARIATION(3, 		L"InsertRow Data to insert a middle row.  Verify  by IRowsetLocate::GetRowsByBookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BMK_Dynamic_Cursor_Buffered)
//--------------------------------------------------------------------
// @class BMK_Dynamic_Cursor_Buffered
//
class BMK_Dynamic_Cursor_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BMK_Dynamic_Cursor_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.
	int Variation_1();
	// @cmember InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BMK_Dynamic_Cursor_Buffered)
#define THE_CLASS BMK_Dynamic_Cursor_Buffered
BEG_TEST_CASE(BMK_Dynamic_Cursor_Buffered, TCIRowsetNewRow, L"BMK_Dynamic_Cursor_Buffered")
	TEST_VARIATION(1, 		L"call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.")
	TEST_VARIATION(2, 		L"InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(boundary_keyset_immediate)
//--------------------------------------------------------------------
// @class boundary_keyset_immediate
//
class boundary_keyset_immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(boundary_keyset_immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember hAccessor is NULL accessor and phRow is valid
	int Variation_1();
	// @cmember hAccessor is NULL accessor and phRow is NULL.
	int Variation_2();
	// @cmember hAccessor is valid accessor and phRow is valid.  pData is NULL.  E_INVALIDARG
	int Variation_3();
	// @cmember test DB_E_ROWSNOTRELEASED.
	int Variation_4();
	// @cmember hChapter ignored
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(boundary_keyset_immediate)
#define THE_CLASS boundary_keyset_immediate
BEG_TEST_CASE(boundary_keyset_immediate, TCIRowsetNewRow, L"boundary_keyset_immediate")
	TEST_VARIATION(1, 		L"hAccessor is NULL accessor and phRow is valid")
	TEST_VARIATION(2, 		L"hAccessor is NULL accessor and phRow is NULL.")
	TEST_VARIATION(3, 		L"hAccessor is valid accessor and phRow is valid.  pData is NULL.  E_INVALIDARG")
	TEST_VARIATION(4, 		L"test DB_E_ROWSNOTRELEASED.")
	TEST_VARIATION(5, 		L"hChapter ignored")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(boundary_keyset_buffered)
//--------------------------------------------------------------------
// @class boundary_keyset_buffered
//
class boundary_keyset_buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(boundary_keyset_buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember hAccessor is NULL accessor and phRow is valid.
	int Variation_1();
	// @cmember hAccessor is NULL accessor and phRow is NULL.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(boundary_keyset_buffered)
#define THE_CLASS boundary_keyset_buffered
BEG_TEST_CASE(boundary_keyset_buffered, TCIRowsetNewRow, L"boundary_keyset_buffered")
	TEST_VARIATION(1, 		L"hAccessor is NULL accessor and phRow is valid.")
	TEST_VARIATION(2, 		L"hAccessor is NULL accessor and phRow is NULL.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Invalid_Keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Invalid_Keyset_Cursor_Immediate
//
class Invalid_Keyset_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	DBORDINAL	m_cColNumber;
	DBORDINAL	m_cColUpdatable;
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Invalid_Keyset_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The accessor is DBACCESSOR_READ | DBACCCESOR_ROWDATA. DB_E_READONLYACCESSOR.
	int Variation_1();
	// @cmember Try to set an auto increment column.
	int Variation_2();
	// @cmember The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE.
	int Variation_3();
	// @cmember The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHENMAVIOLATION.
	int Variation_4();
	// @cmember The accessor only has status binding for DBSTATUS_S_OK.
	int Variation_5();
	// @cmember Set a duplicate column on which a unique index is created. DB_E_INTEGRITYVIOLATION.  No data is changed.
	int Variation_6();
	// @cmember The accessor is a parameter accessor.  DB_E_BADACCESSORTYPE.
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Invalid_Keyset_Cursor_Immediate)
#define THE_CLASS Invalid_Keyset_Cursor_Immediate
BEG_TEST_CASE(Invalid_Keyset_Cursor_Immediate, TCIRowsetNewRow, L"Invalid_Keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"The accessor is DBACCESSOR_READ | DBACCCESOR_ROWDATA. DB_E_READONLYACCESSOR.")
	TEST_VARIATION(2, 		L"Try to set an auto increment column.")
	TEST_VARIATION(3, 		L"The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE.")
	TEST_VARIATION(4, 		L"The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHENMAVIOLATION.")
	TEST_VARIATION(5, 		L"The accessor only has status binding for DBSTATUS_S_OK.")
	TEST_VARIATION(6, 		L"Set a duplicate column on which a unique index is created. DB_E_INTEGRITYVIOLATION.  No data is changed.")
	TEST_VARIATION(7, 		L"The accessor is a parameter accessor.  DB_E_BADACCESSORTYPE.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Invalid_Keyset_Query_Immediate)
//--------------------------------------------------------------------
// @class Invalid_Keyset_Query_Immediate
//
class Invalid_Keyset_Query_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Invalid_Keyset_Query_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR
	int Variation_1();
	// @cmember The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL.  	DB_E_BADSTATUSVALU
	int Variation_2();
	// @cmember The accessor only has length binding.  E_FAIL
	int Variation_3();
	// @cmember The iOrdinal is out of range.
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Invalid_Keyset_Query_Immediate)
#define THE_CLASS Invalid_Keyset_Query_Immediate
BEG_TEST_CASE(Invalid_Keyset_Query_Immediate, TCIRowsetNewRow, L"Invalid_Keyset_Query_Immediate")
	TEST_VARIATION(1, 		L"The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR")
	TEST_VARIATION(2, 		L"The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL.  	DB_E_BADSTATUSVALU")
	TEST_VARIATION(3, 		L"The accessor only has length binding.  E_FAIL")
	TEST_VARIATION(4, 		L"The iOrdinal is out of range.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Valid_Keyset_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Valid_Keyset_Cursor_Immediate
//
class Valid_Keyset_Cursor_Immediate : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPID	m_rgDBPROPID[3];
	ULONG	m_cDBPROPID;
	BOOL m_fCanConvertFromArray;
	BOOL m_fCanConvertFromVector;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Valid_Keyset_Cursor_Immediate,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Insert the variable length columns, in forward order of the rowset.  Value & length binding only.
	int Variation_1();
	// @cmember Insert the fixed length columns with only value binding.
	int Variation_2();
	// @cmember DBTYPE_BYREF binding for all columns.
	int Variation_3();
	// @cmember DBTYPE_ARRAY binding for all columns.
	int Variation_4();
	// @cmember DBTYPE_VECTOR binding for all columns.
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Valid_Keyset_Cursor_Immediate)
#define THE_CLASS Valid_Keyset_Cursor_Immediate
BEG_TEST_CASE(Valid_Keyset_Cursor_Immediate, TCIRowsetNewRow, L"Valid_Keyset_Cursor_Immediate")
	TEST_VARIATION(1, 		L"Insert the variable length columns, in forward order of the rowset.  Value & length binding only.")
	TEST_VARIATION(2, 		L"Insert the fixed length columns with only value binding.")
	TEST_VARIATION(3, 		L"DBTYPE_BYREF binding for all columns.")
	TEST_VARIATION(4, 		L"DBTYPE_ARRAY binding for all columns.")
	TEST_VARIATION(5, 		L"DBTYPE_VECTOR binding for all columns.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Valid_Keyset_Query_Buffered)
//--------------------------------------------------------------------
// @class Valid_Keyset_Query_Buffered
//
class Valid_Keyset_Query_Buffered : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();


	DBPROPID		m_rgDBPROPID[5];
	ULONG		m_cDBPROPID;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Valid_Keyset_Query_Buffered,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Insert the fixed length data type columns with bogus length information.
	int Variation_1();
	// @cmember Insert  the whole row with status only.  Set everything to NULL
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Valid_Keyset_Query_Buffered)
#define THE_CLASS Valid_Keyset_Query_Buffered
BEG_TEST_CASE(Valid_Keyset_Query_Buffered, TCIRowsetNewRow, L"Valid_Keyset_Query_Buffered")
	TEST_VARIATION(1, 		L"Insert the fixed length data type columns with bogus length information.")
	TEST_VARIATION(2, 		L"Insert  the whole row with status only.  Set everything to NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Sequence)
//--------------------------------------------------------------------
// @class sequence testing
//
class Sequence : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Sequence,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Set DBPROP_CanHoldRows.  Set 3 rows in a row
	int Variation_1();
	// @cmember Unset DBPROP_CanHoldrows.  Insert 2 rows.  DB_E_ROWSNOTRELEASED.
	int Variation_2();
	// @cmember Buffered mode.  Unset DBPROP_CanHoldRows.  Insert 2 rows.
	int Variation_3();
	// @cmember Insert 50 rows.  Update.
	int Variation_4();
	// @cmember Buffered mode,DBPROP_CanHoldRows FALSE,Insert row/release it,insert/get another
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Sequence)
#define THE_CLASS Sequence
BEG_TEST_CASE(Sequence, TCIRowsetNewRow, L"sequence testing")
	TEST_VARIATION(1, 		L"Set DBPROP_CanHoldRows.  Set 3 rows in a row")
	TEST_VARIATION(2, 		L"Unset DBPROP_CanHoldrows.  Insert 2 rows.  DB_E_ROWSNOTRELEASED.")
	TEST_VARIATION(3, 		L"Buffered mode.  Unset DBPROP_CanHoldRows.  Insert 2 rows.")
	TEST_VARIATION(4, 		L"Insert 50 rows.  Update.")
	TEST_VARIATION(5, 		L"Buffered mode,DBPROP_CanHoldRows FALSE,Insert row/release it,insert/get another")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Related_IRowsetDelete)
//--------------------------------------------------------------------
// @class test Related_IRowsetDelete wiht IRowsetNewRow
//
class Related_IRowsetDelete : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowsetDelete,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember In immediately update mode, insert two rows, delete one, the insert another row
	int Variation_1();
	// @cmember In immediate update mode, insert one row, change it and delete it.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Related_IRowsetDelete)
#define THE_CLASS Related_IRowsetDelete
BEG_TEST_CASE(Related_IRowsetDelete, TCIRowsetNewRow, L"test Related_IRowsetDelete wiht IRowsetNewRow")
	TEST_VARIATION(1, 		L"In immediately update mode, insert two rows, delete one, the insert another row")
	TEST_VARIATION(2, 		L"In immediate update mode, insert one row, change it and delete it.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Related_IRowsetChange)
//--------------------------------------------------------------------
// @class test IRowsetChange and IRowsetNewRow
//
class Related_IRowsetChange : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_IRowsetChange,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Ichange the primary key of one row, insert a row with	the original primary key.  S_OK.  Insert another row with the new primary
	int Variation_1();
	// @cmember In immediate update mode, inser two rows, change one of the row.  Verify.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Related_IRowsetChange)
#define THE_CLASS Related_IRowsetChange
BEG_TEST_CASE(Related_IRowsetChange, TCIRowsetNewRow, L"test IRowsetChange and IRowsetNewRow")
	TEST_VARIATION(1, 		L"Ichange the primary key of one row, insert a row with	the original primary key.  S_OK.  Insert another row with the new primary")
	TEST_VARIATION(2, 		L"In immediate update mode, inser two rows, change one of the row.  Verify.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Transaction)
//--------------------------------------------------------------------
// @class testing zombie state
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
	// @cmember Commit with fRetaining=TRUE.  Query based
	int Variation_1();
	// @cmember Commit with fRetaining=FALSE. Cursor based
	int Variation_2();
	// @cmember Abort with fRetaining=TRUE.  Cursor  based.
	int Variation_3();
	// @cmember Abort with fRetaining=FALSE.  Query based
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Transaction)
#define THE_CLASS Transaction
BEG_TEST_CASE(Transaction, CTransaction, L"testing zombie state")
	TEST_VARIATION(1, 		L"Commit with fRetaining=TRUE.  Query based")
	TEST_VARIATION(2, 		L"Commit with fRetaining=FALSE. Cursor based")
	TEST_VARIATION(3, 		L"Abort with fRetaining=TRUE.  Cursor  based.")
	TEST_VARIATION(4, 		L"Abort with fRetaining=FALSE.  Query based")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(EmptyTable)
//--------------------------------------------------------------------
// @class insert a new row into an empty table
//
class EmptyTable : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(EmptyTable,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Insert a new row into an empty table
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(EmptyTable)
#define THE_CLASS EmptyTable
BEG_TEST_CASE(EmptyTable, TCIRowsetNewRow, L"insert a new row into an empty table")
	TEST_VARIATION(1, 		L"Insert a new row into an empty table")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(AppendOnly)
//--------------------------------------------------------------------
// @class test DBPROP_APPENDONLY
//
class AppendOnly : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(AppendOnly,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember check props with APPENDONLY
	int Variation_1();
	// @cmember AppendOnly conflict with DBPROP_CANSCROLLBACKWARDS, and OTHERINSERT.
	int Variation_2();
	// @cmember the rowset is empty.
	int Variation_3();
	// @cmember REFRESH/RESYNC, should not bring back old rows.
	int Variation_4();
	// @cmember RestartPosition/Get fetches only newlyinserted rows.
	int Variation_5();
	// @cmember Change newly inserted row..
	int Variation_6();
	// @cmember update pending,RestartPosition/Get fetches only newlyinserted rows.
	int Variation_7();
	// @cmember update pending,REFRESH/RESYNC, should not bring back old rows.
	int Variation_8();
	// @cmember update pending,Commit.
	int Variation_9();
	// @cmember update pending,Abort.
	int Variation_10();
	// @cmember Commit.
	int Variation_11();
	// @cmember Abort.
	int Variation_12();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(AppendOnly)
#define THE_CLASS AppendOnly
BEG_TEST_CASE(AppendOnly, TCIRowsetNewRow, L"test DBPROP_APPENDONLY")
	TEST_VARIATION(1, 		L"check props with APPENDONLY")
	TEST_VARIATION(2, 		L"AppendOnly conflict with DBPROP_CANSCROLLBACKWARDS, and OTHERINSERT.")
	TEST_VARIATION(3, 		L"the rowset is empty.")
	TEST_VARIATION(4, 		L"REFRESH/RESYNC, should not bring back old rows.")
	TEST_VARIATION(5, 		L"RestartPosition/Get fetches only newlyinserted rows.")
	TEST_VARIATION(6, 		L"Change newly inserted row..")
	TEST_VARIATION(7, 		L"update pending,RestartPosition/Get fetches only newlyinserted rows.")
	TEST_VARIATION(8, 		L"update pending,REFRESH/RESYNC, should not bring back old rows.")
	TEST_VARIATION(9, 		L"update pending,Commit.")
	TEST_VARIATION(10, 		L"update pending,Abort.")
	TEST_VARIATION(11, 		L"Commit.")
	TEST_VARIATION(12, 		L"Abort.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(BMK_Static_Buffered_OwnInsert)
//*-----------------------------------------------------------------------
// @class Test static cursors capable of seeing their own inserts
//
class BMK_Static_Buffered_OwnInsert : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BMK_Static_Buffered_OwnInsert,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Test visibility of newly inserted row
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(BMK_Static_Buffered_OwnInsert)
#define THE_CLASS BMK_Static_Buffered_OwnInsert
BEG_TEST_CASE(BMK_Static_Buffered_OwnInsert, TCIRowsetNewRow, L"Test static cursors capable of seeing their own inserts")
	TEST_VARIATION(1, 		L"Test visibility of newly inserted row")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Buffered_ReturnPendingInsert)
//*-----------------------------------------------------------------------
// @class Check ReturnPendingInsert property
//
class Buffered_ReturnPendingInsert : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Buffered_ReturnPendingInsert,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Soft Insert, check visibility
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Buffered_ReturnPendingInsert)
#define THE_CLASS Buffered_ReturnPendingInsert
BEG_TEST_CASE(Buffered_ReturnPendingInsert, TCIRowsetNewRow, L"Check ReturnPendingInsert property")
	TEST_VARIATION(1, 		L"Soft Insert, check visibility")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Immediate_ServerDataOnInsert)
//*-----------------------------------------------------------------------
// @class Test retrieval of bookmarks on newly inserted rows
//
class Immediate_ServerDataOnInsert : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Immediate_ServerDataOnInsert,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Test visibility of bookmark
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Immediate_ServerDataOnInsert)
#define THE_CLASS Immediate_ServerDataOnInsert
BEG_TEST_CASE(Immediate_ServerDataOnInsert, TCIRowsetNewRow, L"Test retrieval of bookmarks on newly inserted rows")
	TEST_VARIATION(1, 		L"Test visibility of bookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Buffered_ServerDataOnInsert)
//*-----------------------------------------------------------------------
// @class Test retrieval of bookmarks on newly inserted rows
//
class Buffered_ServerDataOnInsert : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Buffered_ServerDataOnInsert,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Test visibility of bookmark
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Buffered_ServerDataOnInsert)
#define THE_CLASS Buffered_ServerDataOnInsert
BEG_TEST_CASE(Buffered_ServerDataOnInsert, TCIRowsetNewRow, L"Test retrieval of bookmarks on newly inserted rows")
	TEST_VARIATION(1, 		L"Test visibility of bookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Immediate_ChangeInsertedRow)
//*-----------------------------------------------------------------------
// @class Test DBPROP_CHANGEINSERTEDROW
//
class Immediate_ChangeInsertedRow : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Immediate_ChangeInsertedRow,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Explicitly request ChangeInsertedRows
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Immediate_ChangeInsertedRow)
#define THE_CLASS Immediate_ChangeInsertedRow
BEG_TEST_CASE(Immediate_ChangeInsertedRow, TCIRowsetNewRow, L"Test DBPROP_CHANGEINSERTEDROW")
	TEST_VARIATION(1, 		L"Explicitly request ChangeInsertedRows")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Buffered_ChangeInsertedRow)
//*-----------------------------------------------------------------------
// @class Test DBPROP_CHANGEINSERTEDROW in buffered mode
//
class Buffered_ChangeInsertedRow : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Buffered_ChangeInsertedRow,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Explicitly request ChangeInsertedRows in buffered mode
	int Variation_1();
	// @cmember Delete a newly inserted row
	int Variation_2();
	// @cmember SetData on newly inserted followed by Delete
	int Variation_3();
	// @cmember Multiple SetData on newly inserted
	int Variation_4();
	// @cmember Multilpe newly Inserted and interleaved SetData and Delete ops
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Buffered_ChangeInsertedRow)
#define THE_CLASS Buffered_ChangeInsertedRow
BEG_TEST_CASE(Buffered_ChangeInsertedRow, TCIRowsetNewRow, L"Test DBPROP_CHANGEINSERTEDROW in buffered mode")
	TEST_VARIATION(1, 		L"Explicitly request ChangeInsertedRows in buffered mode")
	TEST_VARIATION(2, 		L"Delete a newly inserted row")
	TEST_VARIATION(3, 		L"SetData on newly inserted followed by Delete")
	TEST_VARIATION(4, 		L"Multiple SetData on newly inserted")
	TEST_VARIATION(5, 		L"Multilpe newly Inserted and interleaved SetData and Delete ops")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(InsertDefault)
//*-----------------------------------------------------------------------
// @class InsertRow using DBSTATUS_S_DEFAULT
//
class InsertDefault : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	CTable		*m_pCustomTable;	// a custom table (ask for def cols, etc)
	BOOL		*m_rgbDefault;		// array indicating the defaltable types
	ULONG		m_nRows;			// initial number of rows in the table
	BOOL		m_fCustomTables;	// if customized tables can be built
	DBCOUNTITEM	m_cSeed;			// seed used to build default values

	// @cmember Builds a list of info about the default property of the columns
	BOOL		GetDefaultColumns();

	// @cmember mask the m_fHasDefault field of columns in m_pTable
	void		MaskDefColumns(BOOL fMask);

	// @cmember Creates a rowset, accessors and fill input bindings
	HRESULT		PrepareForInsert(
		DBORDINAL		cSelectedColumn,	// [in]	the ordinal of the selected column (1 based)
		DBSTATUS		Status,				// [in] the status value for the selected column
		LONG			lRowsOffset,		// [in] row number for data creation
		BYTE			**ppData,			// [out] data buffer
		ULONG			cPropsToUse=1
	);

	// @cmember SetData, check it, get data, check it 
	virtual BOOL	InsertAndCheckDefault(
		BYTE	*pData,				// [in] buffer for IRowsetChange::SetData
		HRESULT	hrSetDataExpected,	// [in] expected hr for IRowsetChange::SetData
		BOOL	fValidate = TRUE,	// [in] validation flag
		HRESULT	*hrSetData = NULL	// [out] actual result of IRowsetChange::SetData
	);

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(InsertDefault,TCIRowsetNewRow);
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
	// @cmember Set default values on all default columns
	int Variation_8();
	// @cmember Set default value on a default column; set columns, one is asked default (SDI)
	int Variation_9();
	// @cmember Set default value on a not nullable default column (SDI)
	int Variation_10();
	// @cmember Set default value on a nullable default column (def == NULL)(SDI)
	int Variation_11();
	// @cmember Set default value on a nullable default column (def != NULL)(SDI)
	int Variation_12();
	// @cmember Set default value on a not default column (SDI)
	int Variation_13();
	// @cmember Set default value on a unique default value (SDI)
	int Variation_14();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(InsertDefault)
#define THE_CLASS InsertDefault
BEG_TEST_CASE(InsertDefault, TCIRowsetNewRow, L"InsertRow using DBSTATUS_S_DEFAULT")
	TEST_VARIATION(1, 		L"Set default values on all default columns")
	TEST_VARIATION(2, 		L"Set default value on a default column; set many columns, one is asked default")
	TEST_VARIATION(3, 		L"Set default value on a not nullable default column")
	TEST_VARIATION(4, 		L"Set default value on a nullable default column (def == NULL)")
	TEST_VARIATION(5, 		L"Set default value on a nullable default column (def != NULL)")
	TEST_VARIATION(6, 		L"Set default value on a not default column")
	TEST_VARIATION(7, 		L"Set default value on a unique default value")
	TEST_VARIATION(8, 		L"Set default values on all default columns")
	TEST_VARIATION(9, 		L"Set default value on a default column; set columns, one is asked default(SDI)")
	TEST_VARIATION(10, 		L"Set default value on a not nullable default column(SDI)")
	TEST_VARIATION(11, 		L"Set default value on a nullable default column (def == NULL)(SDI)")
	TEST_VARIATION(12, 		L"Set default value on a nullable default column (def != NULL)(SDI)")
	TEST_VARIATION(13, 		L"Set default value on a not default column(SDI)")
	TEST_VARIATION(14, 		L"Set default value on a unique default value(SDI)")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(InsertIgnore)
//*-----------------------------------------------------------------------
// @class InsertRow using DBSTATUS_S_IGNORE
//
class InsertIgnore : public InsertDefault { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	// @cmember InsertRowset, check it, get data, check it 
	BOOL	InsertAndCheckIgnore(
		BYTE	*pData,				// [in] buffer for IRowsetChange::SetData
		HRESULT	hrSetDataExpected,	// [in] expected hr for IRowsetChange::SetData
		BOOL	fValidate = TRUE,	// [in] validation flag
		HRESULT	*hrSetData = NULL	// [out] actual result of IRowsetChange::SetData
	);

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(InsertIgnore,InsertDefault);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Insert ignore values on all updateable columns
	int Variation_1();
	// @cmember Insert ignore value on a column; set many columns, one is asked ignored
	int Variation_2();
	// @cmember Insert ignore value on a not nullable default column
	int Variation_3();
	// @cmember Insert ignore value on a nullable default column (def == NULL)
	int Variation_4();
	// @cmember Insert ignore value on a nullable default column (def != NULL)
	int Variation_5();
	// @cmember Insert ignore value on a not default column
	int Variation_6();
	// @cmember Set status to DBSTATUS_S_IGNORE on a non updateable column
	int Variation_7();
	// @cmember Insert all updateable cols, set status to DBSTATUS_S_IGNORE for half of them
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(InsertIgnore)
#define THE_CLASS InsertIgnore
BEG_TEST_CASE(InsertIgnore, InsertDefault, L"InsertRow using DBSTATUS_S_IGNORE")
	TEST_VARIATION(1, 		L"Insert ignore values on all updateable columns")
	TEST_VARIATION(2, 		L"Insert ignore value on a column; set many columns, one is asked ignored")
	TEST_VARIATION(3, 		L"Insert ignore value on a not nullable default column")
	TEST_VARIATION(4, 		L"Insert ignore value on a nullable default column (def == NULL)")
	TEST_VARIATION(5, 		L"Insert ignore value on a nullable default column (def != NULL)")
	TEST_VARIATION(6, 		L"Insert ignore value on a not default column")
	TEST_VARIATION(7, 		L"Set status to DBSTATUS_S_IGNORE on a non updateable column")
	TEST_VARIATION(8, 		L"Insert all updateable cols, set status to DBSTATUS_S_IGNORE for half of them")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ErrorCases)
//*-----------------------------------------------------------------------
// @class ErrorCases 
//
class ErrorCases : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ErrorCases,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DB_E_CANTCONVERTVALUE
	int Variation_1();
	// @cmember DB_E_DATAOVERFLOW
	int Variation_2();
	// @cmember DB_E_MAXPENDCHANGESEXCEEDED
	int Variation_3();
	// @cmember DB_E_ROWLIMITEDEXCEEDED
	int Variation_4();
	// @cmember DB_E_NOTSUPPORTED
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Buffered_ChangeInsertedRow)
#define THE_CLASS ErrorCases
BEG_TEST_CASE(ErrorCases, TCIRowsetNewRow, L"Test error cases")
	TEST_VARIATION(1, 		L"DB_E_CANTCONVERTVALUE")
	TEST_VARIATION(2, 		L"DB_E_DATAOVERFLOW")
	TEST_VARIATION(3, 		L"DB_E_MAXPENDCHANGESEXCEEDED")
	TEST_VARIATION(4, 		L"DB_E_ROWLIMITEDEXCEEDED")
	TEST_VARIATION(5, 		L"DB_E_NOTSUPPORTED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ErrorCases)
//*-----------------------------------------------------------------------
// @class ErrorCases 
//
class ServerDataOnInsertII : public TCIRowsetNewRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ServerDataOnInsertII,TCIRowsetNewRow);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Fetch computed column
	int Variation_1();
	// @cmember Fetch default value
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Buffered_ChangeInsertedRow)
#define THE_CLASS ServerDataOnInsertII
BEG_TEST_CASE(ServerDataOnInsertII, TCIRowsetNewRow, L"ServerDataOnInsertII")
	TEST_VARIATION(1, 		L"Fetch computed column")
	TEST_VARIATION(2, 		L"Fetch default value")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END
// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(41, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Rowset)
	TEST_CASE(2, MayWriteColumn)
	TEST_CASE(3, ComputedColumns)
	TEST_CASE(4, Forward_Cursor_Immediate)
	TEST_CASE(5, Forward_Query_Buffered)
	TEST_CASE(6, Static_Cursor_Buffered)
	TEST_CASE(7, Static_Query_Immediate)
	TEST_CASE(8, Keyset_Cursor_Immediate)
	TEST_CASE(9, Keyset_Cursor_Buffer)
	TEST_CASE(10, Dynamic_Cursor_Immediate)
	TEST_CASE(11, Dynamic_Query_Buffered)
	TEST_CASE(12, Visible_Static_Command_Immediate)
	TEST_CASE(13, Visible_Keyset_Command_Buffered)
	TEST_CASE(14, Visible_Dynamic_Command_Buffered)
	TEST_CASE(15, Visible_Dynamic_Query_Immediate)
	TEST_CASE(16, BMK_Forward_Query_Immediate)
	TEST_CASE(17, BMK_Static_Query_Buffered)
	TEST_CASE(18, BMK_keyset_Cursor_Immediate)
	TEST_CASE(19, BMK_Dynamic_Cursor_Buffered)
	TEST_CASE(20, boundary_keyset_immediate)
	TEST_CASE(21, boundary_keyset_buffered)
	TEST_CASE(22, Invalid_Keyset_Cursor_Immediate)
	TEST_CASE(23, Invalid_Keyset_Query_Immediate)
	TEST_CASE(24, Valid_Keyset_Cursor_Immediate)
	TEST_CASE(25, Valid_Keyset_Query_Buffered)
	TEST_CASE(26, Sequence)
	TEST_CASE(27, Related_IRowsetDelete)
	TEST_CASE(28, Related_IRowsetChange)
	TEST_CASE(29, Transaction)
	TEST_CASE(30, EmptyTable)
	TEST_CASE(31, AppendOnly)
	TEST_CASE(32, BMK_Static_Buffered_OwnInsert)
	TEST_CASE(33, Buffered_ReturnPendingInsert)
	TEST_CASE(34, Immediate_ServerDataOnInsert)
	TEST_CASE(35, Buffered_ServerDataOnInsert)
	TEST_CASE(36, Immediate_ChangeInsertedRow)
	TEST_CASE(37, Buffered_ChangeInsertedRow)
	TEST_CASE(38, InsertDefault)
	TEST_CASE(39, InsertIgnore)
	TEST_CASE(40, ErrorCases)
	TEST_CASE(41, ServerDataOnInsertII)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		Rowset - Test read-only rowsets
//|	Created:			05/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset::Init()
{
	if(TCIRowsetNewRow::Init())
		return TRUE; 

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  select count(*
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_1()
{
	BOOL				fTestPass=FALSE;
	
	//open a rowset, asking for IRowsetChange interface pointer
	TESTC_(g_pTable->CreateRowset(SELECT_ALLFROMTBL, IID_IRowsetChange,0,NULL,
								(IUnknown**)&m_pIRowset,0, NULL),S_OK);
		
	fTestPass = TRUE;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Request IRowsetNewRow on a read-only rowset by ICommand::Execute.  DB_E_PROPERTYNOTAVAILABLE.  The query is left outer join
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_2()
{
	BOOL				fTestPass=FALSE;
	
	if(m_pIDBCreateCommand)
	{
		//open a rowset, asking for IRowsetChange interface pointer
		m_hr = g_pTable->CreateRowset(SELECT_COUNT, IID_IRowsetChange,0,NULL, (IUnknown**)&m_pIRowset,				
									0, NULL);
		TESTC(m_hr == S_OK || m_hr == DB_E_ERRORSOCCURRED);
	}
		
	fTestPass = TRUE;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty rowset.  Should be able to insert one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_3()
{
	DBPROPID	rgDBPROPID[3];
	DBCOUNTITEM	cRows		= 0;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	void		*pData		= NULL;
	BOOL		fTestPass	= TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;

	//create a forward-only rowset with query based updates
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,
										FALSE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass = FALSE;
	//fill up buffer to make for the 5th row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//InsertRow should succeed
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
		
	//get data should succeed
	TESTC_(m_pIRowset->GetData(hRow, m_hAccessor, m_pData),S_OK);
		
	//compare data should be successful
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE))
		goto CLEANUP;

	//get new rows should be successful
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//check the reference count of the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	
	fTestPass=TRUE;

CLEANUP:
	PROVIDER_FREE(pData);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Insert the variable length columns without the length being bound. should be ok
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_4()
{
	HROW		hRow			= NULL;
	HROW		*pHRow			= NULL;
	DBCOUNTITEM	cRows			= 0;
	void		*pData			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	DBORDINAL	*rgColsToBind	= NULL;
	DBORDINAL	cColsNumber		= 0;
	ULONG		cCount			= 0;
	DBPROPID	rgDBPROPID[2];
	HRESULT		hr;


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
	fTestPass	= TEST_FAIL;
	
	//value and status binding only
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
							0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
							FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind))
		goto CLEANUP;

	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert should be successful, variable length cols can use the NULL terminated string
	//they do not need the length 
	hr	= m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);

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
	//hRow should not be NULL
	if(!hRow)
	{
		goto CLEANUP;
	}

	//check the row is inserted
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
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
		{
			break;
		}
		//Get the data for the row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			//delete this row.  it probably inserts null values into key columns which will be tried in later variations
			//multiple attemptes to insert NULL values  into key columns will result in integrity violations
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			TESTC_(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}

		TESTC_(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColsToBind);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
//}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_5()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	ULONG		cCount;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset so the test can pick not nullable updatable cols
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_STATUS,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
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
	FreeAccessorBindings(m_cBinding, m_rgBinding);
	PROVIDER_FREE(m_pData);
	
	//Create a new accessor that binds only the not nullable updatable cols
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))
	{
		goto CLEANUP;
	}
		
	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}

	//InsertRow should fail
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_ERRORSOCCURRED);
		
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

	if(m_pIRowset && hRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	}

	//release the accessor
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_ISNULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_6()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	ULONG		cCount			= 0;
	ULONG		cCountII		= 0;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	void		*pDataCpy		= NULL;
	DBBINDING*	rgBindings		= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	HACCESSOR	hAccessor;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset to get the rgBindings built
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//get an array of nullable and updatable columns
	GetNullableAndUpdatable(&cCol,&rgColNumber);
	//has to find such a column
	if(!cCol)
	{
		goto CLEANUP;
	}

	//
	//make data for the row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	fTestPass=FALSE;
	
	// Duplicate the pData
	pDataCpy = PROVIDER_ALLOC(m_cRowSize);
	TESTC(pDataCpy != NULL);
	memcpy(pDataCpy, pData, (size_t)m_cRowSize);
	
	rgBindings = (DBBINDING*) PROVIDER_ALLOC(m_cBinding * sizeof(DBBINDING));
	TESTC(rgBindings != NULL);
	memcpy(rgBindings, m_rgBinding, (size_t)(m_cBinding * sizeof(DBBINDING)));

	//set all the nullable and updatable bindings in rbBinding to only have the dwPart set to DBPART_STATUS
	for (cCount=0;cCount<cCol;cCount++)
	{
		for (cCountII=0;cCountII<m_cBinding;cCountII++)
		{
			if (m_rgBinding[cCountII].iOrdinal==rgColNumber[cCount])
			{
				m_rgBinding[cCountII].dwPart=DBPART_STATUS;
//				*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCountII].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCountII],pData)=DBSTATUS_S_ISNULL;
				continue;
			}
		}
	}

	//create a new accessor with the new bindings
	TESTC_(m_pIAccessor->CreateAccessor(	DBACCESSOR_ROWDATA,
											m_cBinding,
											m_rgBinding,
											m_cRowSize,
											&hAccessor,
											NULL),S_OK);

	//InsertRow should pass
	//it shouldn't matter that the buffer has the length and value fields filled, the accessor says for nullable
	//cols their status is NULL so the provider shouldn't even look at the length or value for these fields
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pData,&hRow),S_OK);

	//set all the nullable and updatable bindings in rbBinding to only have the dwPart set to DBPART_STATUS
	for (cCount=0;cCount<cCol;cCount++)
	{
		for (cCountII=0;cCountII<m_cBinding;cCountII++)
		{
			if (m_rgBinding[cCountII].iOrdinal==rgColNumber[cCount])
			{
				if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCountII])),DBSTATUS_S_ISNULL))
				{
					goto CLEANUP;
				}
				continue;
			}
		}
	}

	fTestPass=TRUE;
CLEANUP:
	
	CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL), S_OK);
	
	PROVIDER_FREE(rgColNumber);
	
	if( m_pIRowset && hRow )
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	
	//release the accessor
	PROVIDER_FREE(pData);
	ReleaseInputBindingsMemory(m_cBinding, rgBindings, (BYTE *)pDataCpy, TRUE);
	PROVIDER_FREE(rgBindings);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_ISNULL with not null cols
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Rowset::Variation_7()
{
	BOOL		fTestPass		= TEST_SKIPPED;
	ULONG		cCount			= 0;
	ULONG		cCountII		= 0;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	void		*pDataCpy		= NULL;
	DBBINDING*	rgBindings		= NULL;
	DBPROPID	rgPropertyIDs[2];
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	HACCESSOR	hAccessor		= NULL;

	rgPropertyIDs[0]=DBPROP_IRowsetChange;
	rgPropertyIDs[1]=DBPROP_CANHOLDROWS;

	// Initialize 
	m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	//create a rowset to get the rgBindings built
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgPropertyIDs,0,NULL,ON_ROWSET_ACCESSOR,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND)); 	
	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//get an array of nullable and updatable columns
	GetNotNullableAndUpdatable(&cCol,&rgColNumber);
	//has to find such a column
	if(!cCol)
	{
		goto CLEANUP;
	}

	//reexecute the command with just cols we want
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY, cCol,rgColNumber))
	{
		goto CLEANUP;
	}
	//make data for the row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	fTestPass=FALSE;
		
	//set all the nullable and updatable bindings in rbBinding to only have the dwPart set to DBPART_STATUS
//	for (cCount=0;cCount<cCol;cCount++)
//	{
		for (cCountII=0;cCountII<m_cBinding;cCountII++)
		{
//			if (m_rgBinding[cCountII].iOrdinal==rgColNumber[cCount])
//			{
				m_rgBinding[cCountII].dwPart=DBPART_STATUS;
//				*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCountII].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCountII],pData)=DBSTATUS_S_ISNULL;
				continue;
//			}
		}
//	}

	//create a new accessor with the new bindings
//	TESTC_(m_pIAccessor->CreateAccessor(	DBACCESSOR_ROWDATA,
//											m_cBinding,
//											m_rgBinding,
//											m_cRowSize,
//											&hAccessor,
//											NULL),S_OK);

	//InsertRow should fail with some sort of integrity violation
	TEST2C_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_INTEGRITYVIOLATION,DB_E_ERRORSOCCURRED);

	fTestPass=TRUE;
CLEANUP:
	if (hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL), S_OK);
	}
	PROVIDER_FREE(rgColNumber);
	
	if( m_pIRowset && hRow )
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	
	//release the accessor
	PROVIDER_FREE(pData);
	ReleaseInputBindingsMemory(m_cBinding, rgBindings, (BYTE *)pDataCpy, TRUE);
	PROVIDER_FREE(rgBindings);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Rowset::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(MayWriteColumn)
//*-----------------------------------------------------------------------
//| Test Case:		MayWriteColumn - Test DBPROP_MAYWRITECOLUMN.
//|	Created:			05/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MayWriteColumn::Init()
{
	if(!TCIRowsetNewRow::Init())
		return FALSE;

	if (!SettableProperty(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET))
		return TEST_SKIPPED;

	if (!(GetPropInfoFlags(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK))
		return TEST_SKIPPED;

	return TRUE;

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc MaywriteColumn set to FALSE to all columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MayWriteColumn::Variation_1()
{    
	DBPROPID	rgDBPROPID[2];
	BOOL		fTestPass=TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_MAYWRITECOLUMN;
	
	//open a rowset, specifying all the columns not to be settable
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,1,rgDBPROPID,
		1,&(rgDBPROPID[1]),NO_ACCESSOR));

	//get accessor, binds to all updatable columns
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, DBACCESSOR_ROWDATA,	
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

	fTestPass = FALSE;
	//the # of columns should be 0
	if(COMPARE(m_cBinding, 0))
		fTestPass=TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc MayWriteColumn set to TRUE to all columns.  Should fail.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MayWriteColumn::Variation_2()
{
	DBPROPID	rgDBPROPID[2];
	BOOL		fTestPass=TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_MAYWRITECOLUMN;
	
	fTestPass = FALSE;
	
	//open a rowset, specifying all the columns to be settable. Should fail
	if (COMPARE(GetRowsetAndAccessor(SELECT_ALLFROMTBL,2,rgDBPROPID,	0, NULL,NO_ACCESSOR),E_FAIL))
	{		
		fTestPass=TRUE;
	}
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc MayWriteColumn set to FALSE to all columns then insert.  Should fail.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MayWriteColumn::Variation_3()
{
	HROW		hNewRow		= NULL;
	void		*pData		= NULL;
	DBPROPID	rgDBPROPID[1];
	DBPROPID	rgNotDBPROPID[1];
	BOOL		fTestPass	= TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_MAYWRITECOLUMN;
	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPROPID,1,rgNotDBPROPID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
			
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = FALSE;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	//insert the new row, MAYWRITECOLUMN is VARIANT_FALSE
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),DB_E_ERRORSOCCURRED);		

	fTestPass=TRUE;
CLEANUP:
	PROVIDER_FREE(pData);

	if(hNewRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
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
BOOL MayWriteColumn::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}


// {{ TCW_TC_PROTOTYPE(ComputedColumns)
//*-----------------------------------------------------------------------
//| Test Case:		ComputedColumns - test computed columns by executing select col1, col1-col1 from table
//|	Created:			05/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ComputedColumns::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANFETCHBACKWARDS;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor with IRowsetNewRow.  Immediately update mode.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COMPUTEDCOLLIST,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,	DBPART_VALUE|DBPART_STATUS));

	fTestPass = TRUE;
CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Accessor binds the computed columns.  DBSTATUS_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ComputedColumns::Variation_1()
{
	ULONG			i				= 0;
	DBORDINAL		cCol			= 0;
	DBCOUNTITEM		cRows			= 0;
	DBORDINAL		cColumns		= 0;
	HROW			*pHRow			= NULL;
	HROW			hNewRow			= NULL;
	HACCESSOR		hAccessor		= NULL;
	BOOL			fTestPass		= TRUE;
	HRESULT			hr				= E_FAIL;
	IColumnsInfo	*pIColumnsInfo	= NULL;
	DBCOLUMNINFO	*rgColInfo		= NULL;
	WCHAR			*pStringsBuffer = NULL;
	BOOL			fWriteUnknown;

	if(!m_pIDBCreateCommand)
		goto CLEANUP;
	
	fTestPass=FALSE;
	//get the first row handler
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
   	//get ordinal of the last column(the computed column)
	cCol = m_rgBinding[m_cBinding-1].iOrdinal;

	COMPC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown **)&pIColumnsInfo), TRUE);
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColInfo, &pStringsBuffer),S_OK);

	// Check that the computed column is not reported as writable
	COMPC(rgColInfo[i].dwFlags & DBCOLUMNFLAGS_WRITE, FALSE);

	// Some providers may not be able to detect that computed columns are read-only;
	fWriteUnknown = (rgColInfo[i].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN);

	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,1,&(m_rgBinding[m_cBinding-1]),0,&hAccessor,NULL),S_OK);
	
	//get data
	TESTC_(m_pIRowset->GetData(*pHRow,hAccessor,m_pData),S_OK);
	
	hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,m_pData,&hNewRow);

	if (!fWriteUnknown)
	{
		// The provider reported that the computed column was read-only
		// In this case, the provider must return DB_E_ERRORSOCCURRED
		// and set the appropriate status in the SetData buffer
		TESTC_(hr,DB_E_ERRORSOCCURRED);

		//PERMISSIONDENIED should only be returned if ROWRESTRICT is on
		if(GetProperty(DBPROP_COLUMNRESTRICT, DBPROPSET_ROWSET, m_pIRowset, VARIANT_TRUE))
		{
			TESTC(GetStatus(m_pData, &(m_rgBinding[m_cBinding-1])) == DBSTATUS_E_PERMISSIONDENIED);
		}
		else
		{
			TESTC(GetStatus(m_pData, &(m_rgBinding[m_cBinding-1])) == DBSTATUS_E_UNAVAILABLE);
		}
		fTestPass = TRUE;
	}
	else
	{
		// The provider was unable to determine whether the column was read-only
		// Expect any failed HRESULT
		if(COMPARE(FAILED(hr),TRUE))
		{
			fTestPass = TRUE;
		}
	}
		
	//no row handle should be retrieved
	COMPARE(hNewRow, NULL);
CLEANUP:
	//release the accessor
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the row handle
	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(pStringsBuffer);
	SAFE_RELEASE(pIColumnsInfo);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode, call InsertRow to insert a new row with a new value for the second column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ComputedColumns::Variation_2()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBPROPID		rgDBPROPID[5];
	void			*pData			= NULL;
	void			*pVisibleData	= NULL;
	HROW			hNewRow			= NULL;
	IRowsetResynch	*pIRowsetResynch= NULL;
	ULONG			cCount			= 0;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetResynch;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;
	rgDBPROPID[3]=DBPROP_IRowsetUpdate;
	rgDBPROPID[4]=DBPROP_OWNINSERT;

	//this variation wants its own rowset, make sure the one created in the init is gone
	ReleaseRowsetAndAccessor();

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COMPUTEDCOLLIST,5,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetChange||!m_pIRowsetUpdate)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass	= TEST_FAIL;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&m_pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,m_pData,&hNewRow),S_OK);

	//update to inserted row to back end
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);

	if(!(pVisibleData=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;	 
	}
	//Try to get visible data on the row
	//should bring back computed column
	if(!VerifyInterface(m_pIRowset, IID_IRowsetResynch,ROWSET_INTERFACE,(IUnknown **)&pIRowsetResynch))
			goto CLEANUP;

	//get value of DBPROP_STRONGIDENTITY for this rowset
	//if strong identity is VARIANT_TURE then 
	if(GetProp(DBPROP_STRONGIDENTITY))
	{
		TESTC_(pIRowsetResynch->GetVisibleData(hNewRow, m_hAccessor,pVisibleData), S_OK);

		//all columns including the computed columns should be ok
		for(cCount=0; cCount<m_cBinding; cCount++)
		{
// 			COMPARE(*(DBSTATUS *)((DWORD)pVisibleData+m_rgBinding[cCount].obStatus),DBSTATUS_S_OK);
			COMPARE(STATUS_BINDING(m_rgBinding[cCount],pVisibleData),DBSTATUS_S_OK);
		}
	}
	else
	{
		//allocate memory for GetData
		if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
		{
			goto CLEANUP;
		}
		//get data on the new row handle
		TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, pData),S_OK);

		//all columns including the computed columns should be ok
		for(cCount=0; cCount<m_cBinding; cCount++)
		{
//			COMPARE(*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus),DBSTATUS_S_OK);
			COMPARE(STATUS_BINDING(m_rgBinding[cCount],pData),DBSTATUS_S_OK);
		}
	}
	fTestPass = TRUE;
CLEANUP:
	SAFE_RELEASE(pIRowsetResynch);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(pVisibleData);
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
BOOL ComputedColumns::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}


// {{ TCW_TC_PROTOTYPE(Forward_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Forward_Cursor_Immediate - Forward_Cursor_Immediate
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Cursor_Immediate::Init()
{
	BOOL		fTestPass=FALSE;

	if(!TCIRowsetNewRow::Init())
		return FALSE; 
	
	DBPROPID	DBPropID=DBPROP_IRowsetChange;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,&DBPropID,2,g_rgDBPropID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.
// Verify the reference count of the row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Cursor_Immediate::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	DBPROPID	DBPropID	= DBPROP_IRowsetChange;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;

	//move the cursor to the end of rowset
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows-1,0,&cRows,&pHRow),S_OK);
		
	//no row should be retrieved
	if(!COMPARE(pHRow, NULL))
		goto CLEANUP;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);
		
	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
		
	hNewRow=NULL;

	//free the memory used by m_pData
	FreeMemory();

	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,&DBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData);
	
CLEANUP:
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Forward_Query_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Forward_Query_Buffered - Forward_Query_Buffered
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Query_Buffered::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=TEST_FAIL;
	HRESULT		hr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	hr = GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,2,g_rgDBPropID,ON_ROWSET_ACCESSOR, FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    if ( hr == S_OK )
	{
		//we should be on a buffered update mode
		COMPC(BufferedUpdate(), TRUE);

		fTestPass =  TRUE;
	}
	else if ( hr == S_FALSE )
	{
		fTestPass = TEST_SKIPPED;
	}
	
CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the end of the rowset. Verify ref. count.
// Insert a new row with NULL values to the end of the rowset. 
// Verify ref. count.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Query_Buffered::Variation_1()
{
	HROW		hNewRow				= NULL;
	HROW		*pHRow				= NULL;
	DBCOUNTITEM	cRows				= 0;
	ULONG		cCount				= 0;
	DBORDINAL	cColCount			= 0;
	DBORDINAL	cCol				= 0;
	DBORDINAL	*rgColNumber		= NULL;
	DBPROPID	DBPropID			= DBPROP_IRowsetChange;
	BOOL		fTestPass			= FALSE;

	BOOL		fFirstColNullable	= FALSE;
	void		*pInsertedRowData	= NULL;
	void		*pFirstRowData		= NULL;
	HRESULT		hr;

	//get the nullable and updatable columns
	if(!GetNullableAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	//move the cursor to after the 1st row of rowset
	TESTC_(m_pIRowset->GetNextRows(NULL,1,0,&cRows, &pHRow),S_OK);
	
	//no row should be retrieved
	if(!COMPARE(pHRow, NULL))
		goto CLEANUP;

	//make data for the first row
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pFirstRowData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//make data for the last row
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pInsertedRowData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		for(cColCount=0; cColCount<cCol; cColCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cColCount])
			{
				//Is the first column nullable
				if(rgColNumber[cColCount]==1)
					fFirstColNullable = TRUE;
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				*(DBSTATUS*)&STATUS_BINDING(m_rgBinding[cCount],pInsertedRowData)=DBSTATUS_S_ISNULL;
				//exit the inner loop
				cColCount=cCol;
			}
		}
	}

	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pInsertedRowData, &hNewRow),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);
		
	//the data should be the same
	if(!CompareBuffer(m_pData, pInsertedRowData, m_cBinding, m_rgBinding, m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	
	hNewRow=NULL;

	//free the memory used by m_pData
	FreeMemory();
	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,&DBPropID,
							0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pInsertedRowData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			fTestPass=TRUE;
			TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
CLEANUP:
	//release the memory of col number array
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pInsertedRowData);
	PROVIDER_FREE(pFirstRowData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Query_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Static_Cursor_Buffered - Static_Cursor_Buffered
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Buffered::Init()
{  
	DBPROPID	rgDBPROPID[2];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	
	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the begining of the rowset.  Insert a new row with NULL values to the middle of rowset. Verify ref. count.
// Insert a new row with NULL values to the middle of rowset. 
// Verify ref. count.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	BOOL		fTestPass	= FALSE;
	DBPROPID	rgDBPROPID[2];
	HRESULT		hr			= S_OK;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;

	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND),S_OK);
	
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //make data for the middle row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);

	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//free the memory used by m_pData
	FreeMemory();

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
		 
	hNewRow=NULL;

	hr = m_pIRowset->RestartPosition(NULL);
	TESTC_(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE);
		
	//make sure we can still get the 1st row handle 
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,0,NULL,
							ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
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
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Soft Insert.  Check visibility of data while row handle kept using GetData
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Static_Cursor_Buffered::Variation_2()
{ 
	HROW		hNewRow=DB_NULL_HROW;
	ULONG		cRows=0;
	void		*pData=NULL;
	BOOL		fTestPass=FALSE;	
	DBPROPID	rgDBPROPID[2];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;

	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //make data for the middle row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
		
	//get data on the new row handle; the data should be visible before being transmitted
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);

	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	//free the memory used by m_pData
	FreeMemory();
	hNewRow=DB_NULL_HROW;
		 
	fTestPass = TEST_PASS;
	
CLEANUP:
	PROVIDER_FREE(pData);

	//release the row handle
	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Buffered::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Query_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Static_Query_Immediate - Static_Query_Immediate
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Query_Immediate::Init()
{
	DBPROPID	rgDBPROPID[5];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANSCROLLBACKWARDS;
	rgDBPROPID[2]=DBPROP_CANFETCHBACKWARDS;
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;
	rgDBPROPID[4]=DBPROP_IRowsetLocate;


	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgDBPROPID,2,g_rgDBPropID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;
CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the end of rowset.  Insert a new row to the 1st row.  Verify the reference count of the row handle.
// Insert a new row to the 1st row.  
// Verify the reference count of the row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Query_Immediate::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	ULONG		cCount		= 0;
	DBORDINAL	cColCount	= 0;
	DBORDINAL	cCol		= 0;
	DBORDINAL	*rgColNumber= NULL;
	BOOL		fTestPass	= TRUE;
	DBPROPID	rgDBPROPID[5];
	HRESULT		hr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANSCROLLBACKWARDS;
	rgDBPROPID[2]=DBPROP_CANFETCHBACKWARDS;
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;
	rgDBPROPID[4]=DBPROP_IRowsetLocate;

	//get the nullable and updatable columns
	if(!GetNullableAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	fTestPass=FALSE;
	//move the cursor to after the 1st row of rowset
	TESTC_(m_pIRowset->GetNextRows(NULL,1,0,&cRows,&pHRow),S_OK);
		
	//no row should be retrieved
	if(!COMPARE(pHRow, NULL))
		goto CLEANUP;

	//make data for the 13rd row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		for(cColCount=0; cColCount<cCol; cColCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cColCount])
			{
//				*(ULONG *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				//exit the inner loop
				cColCount=cCol;
			}
		}
	}

	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);
		
	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//free the memory used by m_pData
	FreeMemory();

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	hNewRow=NULL;

	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA, 
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{			
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			fTestPass=TRUE;
			TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}

	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData);

	
CLEANUP:
	//release the memory of col number array
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Keyset_Cursor_Immediate - Keyset_Cursor_Immediate
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Immediate::Init()
{
	DBPROPID	rgDBPROPID[6];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;
	rgDBPROPID[3]=DBPROP_CANFETCHBACKWARDS;
	rgDBPROPID[4]=DBPROP_IRowsetIdentity;
	rgDBPROPID[5]=DBPROP_IRowsetLocate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,6,rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the end of rowset.  Insert a new row to the 1st row with NULL.  Verify the ref count of the row handle.
//Insert a new row to the 1st row with NULL.  
//Verify the ref count of the row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Immediate::Variation_1()
{
	HROW		hNewRow			= NULL;
	HROW		*pHRow			= NULL;
	DBCOUNTITEM	cRows			= 0;
	void		*pData			= NULL;
	void		*pDataCpy		= NULL;
	ULONG		cCount			= 0;
	DBORDINAL	cColCount		= 0;
	DBORDINAL	cCol			= 0;
	DBORDINAL	*rgColNumber	= NULL;
	DBPROPID	DBPropID[2]		= {DBPROP_IRowsetChange,DBPROP_IRowsetLocate};
	BOOL		fTestPass		= TRUE;

	HRESULT hr = S_OK;

	//get the nullable and updatable columns
	if(!GetNullableAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	fTestPass=FALSE;
	//move the cursor to after the 60th row of rowset.  No Op.
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows,0,&cRows,&pHRow),S_OK);
		
	//no row should be retrieved
	if(!COMPARE(pHRow, NULL))
		goto CLEANUP;
	
	//make data for the 71st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	// Duplicate the pData
	pDataCpy = PROVIDER_ALLOC(m_cRowSize);
	TESTC(pDataCpy != NULL);
	memcpy(pDataCpy, pData, (size_t)m_cRowSize);
	
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		for(cColCount=0; cColCount<cCol; cColCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cColCount] &&
				m_rgBinding[cCount].iOrdinal!=1	)
			{
//				*(DBSTATUS *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				cColCount=cCol;
			}
		}
	}

	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
	
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
		goto CLEANUP;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}

		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}

	if ( hr == DB_S_ENDOFROWSET )
		goto CLEANUP;

	//free the memory used by m_pData
	if(StrongIdentity())
		CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, hNewRow),S_OK);
		
	//release the row handles
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	hNewRow=NULL;

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,DBPropID,
							0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //Get the last row handle. 
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);

		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			fTestPass=TRUE;
			TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}

	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataCpy);


CLEANUP:
	//release the memory of col number array
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pDataCpy);
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Cursor_Buffer)
//*-----------------------------------------------------------------------
//| Test Case:		Keyset_Cursor_Buffer - Keyset_Cursor_Buffer
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Buffer::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,0,NULL,
										ON_ROWSET_ACCESSOR,TRUE, DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//we should be on a buffered update mode
//	COMPC(BufferedUpdate(), TRUE);

	fTestPass = TRUE;
CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the begining of the rowset.  Insert a new row  to the end of rowset. Verify ref. count.
//  Insert a new row  to the end of rowset. .
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffer::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	ULONG		cCount		= 0;
	DBORDINAL	cColCount	= 0;
	DBORDINAL	cCol		= 0;
	DBORDINAL	*rgColNumber= NULL;
	BOOL		fTestPass	= TRUE;
 	DBPROPID	rgDBPROPID[4];
	HRESULT		hr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;
	rgDBPROPID[3]=DBPROP_IRowsetUpdate;	

	//get the nullable and updatable columns
	if(!GetNullableAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	fTestPass=FALSE;

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		for(cColCount=0; cColCount<cCol; cColCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColNumber[cColCount])
			{
//				*(DBSTATUS *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				cColCount=cCol;
			}
		}
	}


	//insert the new row
	m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow);
	
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	hNewRow=NULL;

	//make sure we can still get the 1st row handle 
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,	&pHRow),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	

	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,
							0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA, 
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))

		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		FreeMemory();
	}
CLEANUP:
	//release the memory of col number array
	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Buffer::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Dynamic_Cursor_Immediate - Dynamic_Cursor_Immediate
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Immediate::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERINSERT;
	rgDBPROPID[2]=DBPROP_IRowsetIdentity;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the end of rowset.  Insert a new row wiht NULL to  1st row.  Verify the reference count of the row handle.
// Insert a new row wiht NULL to  1st row.  
// Verify the reference count of the row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Cursor_Immediate::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	DBPROPID	DBPropID	= DBPROP_IRowsetChange;
	BOOL		fTestPass	= FALSE;

	HRESULT hr = S_OK;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);
		
	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//free the memory used by m_pData
	FreeMemory();

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
		 
	hNewRow=NULL;

	//restartpositon
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
		goto CLEANUP;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{				
			fTestPass=TRUE;
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}
	
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData);	
	
	//release the row handle
	//TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	//PROVIDER_FREE(pHRow);
	
	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,&DBPropID,
							0,NULL,ON_ROWSET_ACCESSOR,FALSE, DBACCESSOR_ROWDATA, 
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))
		goto CLEANUP;
	
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}

	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData);
	
CLEANUP:
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic_Query_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Dynamic_Query_Buffered - Dynamic_Query_Buffered
//|	Created:			05/07/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Query_Buffered::Init()
{
	DBPROPID	rgDBPROPID[4];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_OTHERINSERT;


	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	
	//we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Move the cursor to the begining of the rowset.  Insert a new row to the middle of the rowset. Verify ref. count.
// Insert a new row to the middle of the rowset. Verify ref. count.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Query_Buffered::Variation_1()
{
	HROW		hNewRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	DBPROPID	DBPropID	= DBPROP_IRowsetChange;
	BOOL		fTestPass	= FALSE;

	HRESULT hr = S_OK;

	//make data for the 15th row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert the new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hNewRow),S_OK);
		
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hNewRow,NULL,NULL,NULL),S_OK);
		
	//get data on the new row handle
	TESTC_(m_pIRowset->GetData(hNewRow, m_hAccessor, m_pData),S_OK);

	//the data should be the same
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,	m_pIMalloc,TRUE,FALSE,COMPARE_ONLY))
		goto CLEANUP;

	//free the memory used by m_pData
	FreeMemory();

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	hNewRow=NULL;

	hr = m_pIRowset->RestartPosition(NULL);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
	ReleaseRowsetAndAccessor();

	//re-execute the command
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,&DBPropID,
							0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
CLEANUP:
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{	
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hNewRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Query_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Static_Command_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Static_Command_Immediate - Visible_Static_Command_Immediate
//|	Created:			05/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Static_Command_Immediate::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns.  Write only //accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
							2,g_rgDBPropID,ON_ROWSET_FETCH_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Static_Command_Immediate::Variation_1()
{
	void			*pData		= NULL;
	DBPROP			rgDBProp[2];
	DBPROPSET		DBPropSet;
	IUnknown		*pIUnknown	= NULL;
	IRowset			*pIRowset	= NULL;
	IAccessor		*pIAccessor	= NULL;
	HACCESSOR		hAccessor	= DB_NULL_HACCESSOR;
	HROW			*pHRow		= NULL;
	DBCOUNTITEM		cRows		= 0;
	BOOL			fTestPass	= FALSE;


	if(!m_pIDBCreateCommand)
		return TEST_PASS;
	
	//initialize
	DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropSet.cProperties=2;
	DBPropSet.rgProperties=rgDBProp;
	DBPropSet.rgProperties[0].dwPropertyID=DBPROP_IRowsetChange;
	DBPropSet.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[0].colid		= DB_NULLID;
	DBPropSet.rgProperties[0].vValue.vt=VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[0].vValue)=VARIANT_TRUE;

	DBPropSet.rgProperties[1].dwPropertyID=DBPROP_CANHOLDROWS;
	DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[1].colid		= DB_NULLID;
	DBPropSet.rgProperties[1].vValue.vt=VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[1].vValue)=VARIANT_TRUE;


	//open another rowset
	
	TESTC_(m_pTable->ExecuteCommand(SELECT_ORDERBYNUMERIC,IID_IRowsetChange, NULL, NULL, 0, NULL, 
									EXECUTE_IFNOERROR, 1, &DBPropSet,NULL, &pIUnknown, NULL),S_OK);
		
	//QI for IRowset
	TESTC_(pIUnknown->QueryInterface(IID_IRowset, (LPVOID *)&pIRowset),S_OK);

	//QI for IAccessor
	TESTC_(pIUnknown->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);
				
	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		   
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor,pData,NULL),S_OK);

	//create an accessor on the second rowset, on the first column//only
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,1,m_rgBinding,0,&hAccessor,NULL),S_OK);
	
	// look for the new row from the other rowset interface
	while (S_OK==(m_hr = pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || m_hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(pIRowset->GetData(*pHRow,hAccessor,m_pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=FALSE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
	//if we reach the end of the rowset, the test will pass
	if(COMPARE(m_hr, DB_S_ENDOFROWSET))
		fTestPass=TRUE;

CLEANUP:
	//release the accessor handle
	if(hAccessor != DB_NULL_HACCESSOR)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	//release the row handle
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}		

	//release the interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIUnknown);
	
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData, TRUE);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Static_Command_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Keyset_Command_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Keyset_Command_Buffered - Visible_Keyset_Command_Buffered
//|	Created:			05/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Keyset_Command_Buffered::Init()
{
	DBPROPID	rgDBPROPID[4];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetUpdate;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;
	rgDBPROPID[3]=DBPROP_IRowsetLocate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns.  Write only 
	//accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,
		0,NULL,ON_ROWSET_FETCH_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should not see.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Keyset_Command_Buffered::Variation_1()
{
	void				*pData			= NULL;
	DBPROP				rgDBProp[4];
	DBPROPSET			DBPropSet;
	IUnknown			*pIUnknown		= NULL;
	IRowset				*pIRowset		= NULL;
	IAccessor			*pIAccessor		= NULL;
	HACCESSOR			hAccessor		= NULL;
	HROW				*pHRow			= NULL;
	HROW				hRow			= NULL;
	HROW				*pPendingHRow	= NULL;
	DBCOUNTITEM			cRows			= 0;
	BOOL				fTestPass		= FALSE;

	HRESULT hr = S_OK;

	if(!m_pIDBCreateCommand)
		return TEST_PASS;
	if(!m_pIRowsetUpdate)
		return TEST_PASS;

	//initialize
	DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropSet.cProperties=4;
	DBPropSet.rgProperties=rgDBProp;

	DBPropSet.rgProperties[0].dwPropertyID		= DBPROP_IRowsetUpdate;
	DBPropSet.rgProperties[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[0].colid				= DB_NULLID;
	DBPropSet.rgProperties[0].vValue.vt			= VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[0].vValue)	= VARIANT_TRUE;

	DBPropSet.rgProperties[1].dwPropertyID		= DBPROP_OTHERUPDATEDELETE;
	DBPropSet.rgProperties[1].dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[1].colid				= DB_NULLID;
	DBPropSet.rgProperties[1].vValue.vt			= VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[1].vValue)	= VARIANT_TRUE;

	DBPropSet.rgProperties[3].dwPropertyID		= DBPROP_OTHERINSERT;
	DBPropSet.rgProperties[3].dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[3].colid				= DB_NULLID;
	DBPropSet.rgProperties[3].vValue.vt			= VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[3].vValue)	= VARIANT_FALSE;

   	DBPropSet.rgProperties[2].dwPropertyID		= DBPROP_IRowsetLocate;
	DBPropSet.rgProperties[2].dwOptions			= DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[2].colid				= DB_NULLID;
	DBPropSet.rgProperties[2].vValue.vt			= VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[2].vValue)	= VARIANT_TRUE;

	//open another rowset															  
	hr= m_pTable->ExecuteCommand(SELECT_ORDERBYNUMERIC,IID_IRowsetChange, NULL, NULL, 0, NULL,
									EXECUTE_IFNOERROR, 1, &DBPropSet,NULL, &pIUnknown, NULL);

	//if thr correct prop are not set no use go on.
	if (hr==DB_S_ERRORSOCCURRED || hr==DB_S_ERRORSOCCURRED)
	{
		fTestPass	= TEST_PASS;
		goto CLEANUP;
	}
	if (hr!=S_OK)
	{
		goto CLEANUP;
	}

	//QI for IRowset
	TESTC_(pIUnknown->QueryInterface(IID_IRowset, (LPVOID *)&pIRowset),S_OK);

	//QI for IAccessor
	TESTC_(pIUnknown->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);
												   		
	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		   
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor,pData,&hRow),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);
		

	hr = m_pIRowset->RestartPosition(NULL);

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pPendingHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pPendingHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pPendingHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pPendingHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
	//create an accessor on the second rowset, on the first colum
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,1,m_rgBinding,0,&hAccessor,NULL),S_OK);
		
	while (S_OK==(m_hr = pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || m_hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for row handle
		TESTC_(pIRowset->GetData(*pHRow,hAccessor,m_pData),S_OK);
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=FALSE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
	//if we reach the end of the rowset, the test will pass
	if(COMPARE(m_hr, DB_S_ENDOFROWSET))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	//release the accessor handle
	if(hAccessor != DB_NULL_HACCESSOR)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	//release the row handle
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}	
	
	//release the row handle
	if(pPendingHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pPendingHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pPendingHRow);
	}		


	//release the interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIUnknown);
	 
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Keyset_Command_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetNewRow::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Visible_Dynamic_Command_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Dynamic_Command_Buffered - Visible_Dynamic_Command_Buffered
//|	Created:			05/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Dynamic_Command_Buffered::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERINSERT;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns.  Write only accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
										0,NULL,ON_ROWSET_FETCH_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Command_Buffered::Variation_1()
{
	void				*pData		= NULL;
	DBPROP				rgDBProp[2];
	DBPROPSET			DBPropSet;
	IUnknown			*pIUnknown	= NULL;
	IRowset				*pIRowset	= NULL;
	IAccessor			*pIAccessor	= NULL;
	HACCESSOR			hAccessor	= NULL;
	HROW				*pHRow		= NULL;
	DBCOUNTITEM			cRows		= 0;
	BOOL				fTestPass	= FALSE;
	HRESULT				hr;

	//initialize
	DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropSet.cProperties=2;
	DBPropSet.rgProperties=rgDBProp;
	DBPropSet.rgProperties[0].dwPropertyID=DBPROP_IRowsetChange;
	DBPropSet.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[0].colid		= DB_NULLID;
	DBPropSet.rgProperties[0].vValue.vt=VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[0].vValue)=VARIANT_TRUE;

	DBPropSet.rgProperties[1].dwPropertyID=DBPROP_OTHERINSERT;
	DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;
	DBPropSet.rgProperties[1].colid		= DB_NULLID;
	DBPropSet.rgProperties[1].vValue.vt=VT_BOOL;
	V_BOOL(&DBPropSet.rgProperties[1].vValue)=VARIANT_TRUE;

	if(!m_pIDBCreateCommand)
		return TEST_PASS;

	//open another rowset
	TESTC_(m_pTable->ExecuteCommand(SELECT_ORDERBYNUMERIC,IID_IRowsetChange, NULL, NULL, 0, NULL,
									EXECUTE_IFNOERROR, 1, &DBPropSet,NULL, &pIUnknown, NULL),S_OK);
		

	//QI for IRowset
	TESTC_(pIUnknown->QueryInterface(IID_IRowset,(LPVOID *)&pIRowset),S_OK);

	//QI for IAccessor
	TESTC_(pIUnknown->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);

		
	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
   
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor,pData,NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cRows, &pHRow,NULL),S_OK);
		
	//one row should be updated
	if(!COMPARE(cRows, 1))
		goto CLEANUP;

	//free the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, pHRow, NULL, NULL, NULL),S_OK);
		
	PROVIDER_FREE(pHRow);
	
	//create an accessor on the second rowset.
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,0,&hAccessor,NULL),S_OK);
		
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,1,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}


CLEANUP:
	//release the accessor handle
	if(hAccessor)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	//release the row handle
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}		

	//release the interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIUnknown);
	
	PROVIDER_FREE(pData);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Dynamic_Command_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();
		
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Visible_Dynamic_Query_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Visible_Dynamic_Query_Immediate - Visible_Dynamic_Query_Immediate
//|	Created:			05/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Visible_Dynamic_Query_Immediate::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERINSERT;


	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns.  Write only 
	//accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
								0,NULL,ON_ROWSET_FETCH_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Create two rowsets on the same table.  One rowset insert one row to the end of the rowset.  The other rowset should see insert.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Visible_Dynamic_Query_Immediate::Variation_1()
{
	void				*pData			= NULL;
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ICommand			*pICommand		= NULL;
	IUnknown			*pIUnknown		= NULL;
	IRowset				*pIRowset		= NULL;
	IAccessor			*pIAccessor		= NULL;
	HACCESSOR			hAccessor		= NULL;
	HROW				*pHRow			= NULL;
	DBCOUNTITEM			cRows			= 0;
	BOOL				fTestPass		= FALSE;
	HRESULT				hr;

	if(!m_pIDBCreateCommand)
		return TEST_PASS;

	//get IRowsetInfo pointer for the rowset
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowsetInfo, (LPVOID *)&pIRowsetInfo),S_OK);
		
	//get the pICommand pointer
	TESTC_(pIRowsetInfo->GetSpecification(IID_ICommand, (IUnknown **)&pICommand),S_OK);

	//open another rowset
	TESTC_(pICommand->Execute(NULL,IID_IRowsetChange,NULL, NULL, &pIUnknown),S_OK);
		
	//QI for IRowset
	TESTC_(pIUnknown->QueryInterface(IID_IRowset, (LPVOID *)&pIRowset),S_OK);
		
	//QI for IAccessor
	TESTC_(pIUnknown->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);

	//make data for insert.  Make the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
   	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor,pData,NULL),S_OK);

	//create an accessor on the second rowset
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,0,&hAccessor,NULL),S_OK);
		
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,1,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}


CLEANUP:
	//release the accessor handle
	if(hAccessor != DB_NULL_HACCESSOR)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	//release the row handle
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}		

	//release the interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommand);

	if(pData)
		PROVIDER_FREE(pData);

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
BOOL Visible_Dynamic_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();
		
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(BMK_Forward_Query_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		BMK_Forward_Query_Immediate - BMK_Forward_Query_Immediate
//|	Created:			05/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_Forward_Query_Immediate::Init()
{		   
	BOOL		fTestPass=TEST_PASS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;
		
	DBPROPID	rgDBPROPID[3];
	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;

	
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
							3,g_rgDBPropID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND));
CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_Forward_Query_Immediate::Variation_1()
{
	void		*pData		= NULL;
	DBCOUNTITEM	cRows		= 0;
	DBBOOKMARK	DBBookmark	= DBBMK_FIRST;
	BYTE		*pBookmark	= (BYTE *)&DBBookmark;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TRUE;
	HRESULT		hr;

	//make data for insert. Insert the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	PROVIDER_FREE(pData);
	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
	{
		goto CLEANUP;	 
	}
	//Get a new row by IRowsetLocate::GetRowsAt
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
							  
	//GetData
	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData),S_OK);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);	
	PROVIDER_FREE(pHRow);
	
	m_hr = m_pIRowset->RestartPosition(NULL);
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}

	fTestPass=TRUE;
	
CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL, NULL, NULL), S_OK);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_Forward_Query_Immediate::Variation_2()
{
	void		*pData			= NULL;
	DBCOUNTITEM	cRows			= 0;
	DBBOOKMARK	DBBookmark		= DBBMK_LAST;
	BYTE		*pBookmark		= (BYTE *)&DBBookmark;
	HROW		hRow			= NULL;
	HROW		*pHRow			= NULL;
	BOOL		fTestPass		= TRUE;
	HRESULT		hr;

	//make data for insert. Insert the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	/*PROVIDER_FREE(pData);
	
	//Get the last row by IRowsetLocate::GetRowsByBookmark
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
					  
	//GetData
	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,&pData),S_OK);

	TESTC_(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);	
	
	PROVIDER_FREE(pHRow);
	*/
	
	m_hr = m_pIRowset->RestartPosition(NULL);
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}

CLEANUP:
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);
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
BOOL BMK_Forward_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(BMK_Static_Query_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		BMK_Static_Query_Buffered - BMK_Static_Query_Buffered
//|	Created:			05/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_Static_Query_Buffered::Init()
{
	DBPROPID	rgDBPROPID[5];
	DBPROPID	DBPropUnset=DBPROP_OTHERUPDATEDELETE;
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;
	rgDBPROPID[3]=DBPROP_CANSCROLLBACKWARDS;
	

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,
							1,&DBPropUnset,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.
// @Verify DBPROP_CANHOLDROWS.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_Static_Query_Buffered::Variation_1()
{
	void		*pData		= NULL;
	DBCOUNTITEM	cRows		= 0;
	DBBOOKMARK	DBBookmark	= DBBMK_LAST;
	BYTE		*pBookmark	= (BYTE *)&DBBookmark;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	DBROWSTATUS	DBRowStatus;
	BOOL		fTestPass	= FALSE;

	//make data for insert. Insert the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		

	//insert a new row.  Do not have to call Update yet.
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
		
	//Get a new row by IRowsetLocate::GetRowsAt for the last row  //we can not hold rows
	if(GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),DB_E_ROWSNOTRELEASED);
	}
	
	//release the newly inserted row
	TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,&DBRowStatus),S_OK);
	
	COMPARE(DBRowStatus, DBROWSTATUS_S_PENDINGCHANGES);
	hRow=NULL;

	//call GetRowsAt again for the last row
	if(GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),DB_E_ROWSNOTRELEASED);
		
		if(!COMPARE(pHRow, NULL))
			goto CLEANUP;
	}

	//call Update to update the data to the backend
	TESTC_(m_pIRowsetUpdate->Update(NULL, 0, NULL,NULL,NULL,NULL),S_OK);

	//call GetRowsAt again for the last row
	TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,0,1,&cRows,&pHRow),S_OK);
		
	fTestPass=TRUE;

CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);
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
BOOL BMK_Static_Query_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(BMK_keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		BMK_keyset_Cursor_Immediate - BMK_keyset_Cursor_Immediate
//|	Created:			05/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_keyset_Cursor_Immediate::Init()
{
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_OTHERUPDATEDELETE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
							0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Try to set data to the bookmark column.  DB_E_ACCESSVIOLATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_keyset_Cursor_Immediate::Variation_1()
{
	HACCESSOR	hAccessor	= NULL;	
	DBORDINAL	ulColToBind=0;
	void		*pData		= NULL;
	DBCOUNTITEM	cBinding;
	DBBINDING	*pBinding	= NULL;
	DBLENGTH	cbRowSize;
	BOOL		fTestPass	= FALSE;
	DBPROPID	rgDBPROPID[3];
	HROW		hRow		= 1l;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_OTHERUPDATEDELETE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//create an accessor on the 0th column
	TESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA, &hAccessor, &pBinding,
								&cBinding,&cbRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
								USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, 
								DBTYPE_EMPTY, 1, (LONG_PTR *)&ulColToBind),S_OK);
	
	if(!(pData=PROVIDER_ALLOC(cbRowSize)))
		goto CLEANUP;	 

	//assign the bookmark value to NULL
//	*(DBSTATUS *)(((DWORD)(pData))+pBinding->obStatus)=DBSTATUS_S_ISNULL;
	STATUS_BINDING(*pBinding,pData)=DBSTATUS_S_ISNULL;
	
	//set data should fail
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pData,&hRow),DB_E_ERRORSOCCURRED);
		
	COMPARE(hRow,NULL);

	//verify the status
	if	(	GetStatus(pData, pBinding)==DBSTATUS_E_INTEGRITYVIOLATION ||
			GetStatus(pData, pBinding)==DBSTATUS_E_PERMISSIONDENIED
		)
	{
		fTestPass=TRUE;
	}
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBinding);

	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc call InsertRow insert the last row.  Verify IRowsetLocate::GetRowsAt with DBBMK_LAST.
// Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_keyset_Cursor_Immediate::Variation_2()
{
	void		*pData		= NULL;
	DBCOUNTITEM	cRows		= 0;
	DBBOOKMARK	DBBookmark	= DBBMK_FIRST;
	BYTE		*pBookmark	= (BYTE *)&DBBookmark;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= FALSE;
	DBPROPID	rgDBPROPID[3];
	HRESULT		hr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_OTHERUPDATEDELETE;


	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA, 
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert. Insert the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a new row.  
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
		
	//GetRowsAt returns DB_E_ROWSNOTRELEASED
	if(GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,m_ulTableRows,1,&cRows,&pHRow),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,m_ulTableRows,1,&cRows,&pHRow),DB_E_ROWSNOTRELEASED);
	}
		
	//release the newly inserted row
	TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);

	hRow=NULL;

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
				ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
			break;
		}
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)m_pData);
	}

		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData);
	
CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc InsertRow Data to insert a middle row.  Verify  by IRowsetLocate::GetRowsByBookmark
// Verify  by IRowsetLocate::GetRowsByBookmark.Verify DBPROP_CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_keyset_Cursor_Immediate::Variation_3()
{
	void		*pData			=NULL;
	DBBKMARK	cbBookmark		= 0;
	DBBOOKMARK	DBBookmark		= DBBMK_LAST;
	BYTE		*pLastBookmark	= (BYTE *)&DBBookmark;
	BYTE		*pBookmark		= NULL;
	HROW		hRowNew			= NULL;
	HROW		*pNewHRow		= &hRowNew;
	HROW		HRow			= NULL;
	BOOL		fTestPass		= FALSE;
	DBPROPID	rgDBPROPID[5];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;
	rgDBPROPID[4]=DBPROP_IRowsetIdentity;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,5,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

odtLog << "1 baloney";

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
odtLog << "2 baloney";
	//make data for insert. Insert the the seond row row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
odtLog << "3 baloney";
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,pNewHRow),S_OK);
	
odtLog << "4 baloney";
	//Get a bookmark 
	if(S_OK!=GetBookmarkByRow(hRowNew, &cbBookmark, &pBookmark))
	{
odtLog << "5 baloney";
		// The bookmark column is technically a computed column and some providers may not
		// allow retrieval of computed columns on a newly inserted row.
		if(!GetProperty(DBPROP_SERVERDATAONINSERT, DBPROPSET_ROWSET, m_pIRowset, VARIANT_TRUE))
		{
odtLog << "6 baloney";
			fTestPass = TRUE;
		}
		goto CLEANUP;
	}
	
odtLog << "7 baloney";
	//Get the row by IRowsetLocate::GetRowsByBookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
						  
	//GetData
odtLog << "8 baloney";
	TESTC_(m_pIRowset->GetData(HRow,m_hAccessor,m_pData),S_OK);
	
odtLog << "9 baloney";
	//should be able to see the changes
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		goto CLEANUP;

odtLog << "10 baloney";
	//IsSameRow should return S_OK
	if(StrongIdentity())
		TESTC_(m_pIRowsetIdentity->IsSameRow(HRow, hRowNew),S_OK);
odtLog << "11 baloney";
	
	fTestPass=TRUE;

CLEANUP:
	//release the bookmark
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pData);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL,NULL,NULL), S_OK);
	
	if(hRowNew)
		CHECK(m_pIRowset->ReleaseRows(1,&hRowNew, NULL,NULL,NULL), S_OK);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_keyset_Cursor_Immediate::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}


// {{ TCW_TC_PROTOTYPE(BMK_Dynamic_Cursor_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		BMK_Dynamic_Cursor_Buffered - BMK_Dynamic_Cursor_Buffered
//|	Created:			05/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_Dynamic_Cursor_Buffered::Init()
{
	DBPROPID	rgDBPROPID[5];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;	
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;
	rgDBPROPID[4]=DBPROP_OTHERINSERT;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc call InsertRow insert the first row.  Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.
// Verify IRowsetLocate::GetRowsAt with DBBMK_FIRST.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_Dynamic_Cursor_Buffered::Variation_1()
{
	void		*pData		= NULL;
	DBCOUNTITEM	cRows		= 0;
	DBBOOKMARK	DBBookmark	= DBBMK_FIRST;
	BYTE		*pBookmark	= (BYTE *)&DBBookmark;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;

	//make data for insert. Insert the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a new row.  
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	hRow=NULL;

	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}
	
CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL), S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc InsertRow Data to insert a last row.  Verify  by IRowsetLocate::GetRowsByBookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BMK_Dynamic_Cursor_Buffered::Variation_2()
{
	void		*pData		= NULL;
	DBBKMARK	cbBookmark	= 0;
	BYTE		*pBookmark	= NULL;
	HROW		HRow		= NULL;
	HROW		hRowNew		= NULL;
	BOOL		fTestPass	= FALSE;


	//make data for insert. Insert the the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRowNew),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRowNew,NULL,NULL,NULL),S_OK);

	// Get a bookmark 
	if(S_OK != GetBookmarkByRow(hRowNew, &cbBookmark, &pBookmark))
	{
		// The bookmark column is technically a computed column and some providers may not
		// allow retrieval of computed columns on a newly inserted row.
		if(!GetProperty(DBPROP_SERVERDATAONINSERT, DBPROPSET_ROWSET, m_pIRowset, VARIANT_TRUE))
			fTestPass = TRUE;

		goto CLEANUP;
	}

	//Get the row by IRowsetLocate::GetRowsByBookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
					  
	//GetData
	TESTC_(m_pIRowset->GetData(HRow,m_hAccessor,m_pData),S_OK);
	
	//should be able to see the changes
	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		fTestPass=TRUE;
	
CLEANUP:
	//release the bookmark
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pData);

	if(HRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL,NULL,NULL), S_OK);
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
BOOL BMK_Dynamic_Cursor_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(boundary_keyset_immediate)
//*-----------------------------------------------------------------------
//| Test Case:		boundary_keyset_immediate - boundary_keyset_immediate
//|	Created:			05/13/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL boundary_keyset_immediate::Init()
{
	DBPROPID	rgDBPROPID[1];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	// Test case name is misleading, the cursor is not necessarily keyset
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ALLFROMTBL,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,	 ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));
	
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc hAccessor is NULL accessor and phRow is valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_immediate::Variation_1()
{

	HACCESSOR	hAccessor	= NULL;
	DBCOUNTITEM	cRows		= 0;
	HROW		HRow		= NULL;
	HROW		*pHRow		= &HRow;
	BOOL		fTestPass	= FALSE;

	HRESULT hr = S_OK;

	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,DB_NULL_HACCESSOR,NULL,NULL),DB_E_BADACCESSORHANDLE);

	//create a NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
			
	//hAccessor is NULL accessor and phRow is valid
	m_hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,NULL,NULL);
	
	if(E_FAIL == m_hr || DB_E_INTEGRITYVIOLATION == m_hr)
	{

		//hAccessor is NULL accessor and phRow is valid
		m_hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,NULL,&HRow);
			
		if(!(E_FAIL == m_hr || DB_E_INTEGRITYVIOLATION == m_hr))
			goto CLEANUP;
		
		//no row handle should be returned.
		COMPARE(HRow, NULL);

		//try to get the last row handle
		TESTC_(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK)
		
		fTestPass=TRUE;

	}
	else
	{
		//m_hr has to be S_OK.
		TESTC_(m_hr, S_OK);
		
		//hAccessor is NULL accessor and phRow is valid
		m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,NULL,&HRow);

		if(HRow)
			CHECK(m_pIRowset->ReleaseRows(1,&HRow,NULL,NULL,NULL),S_OK);

		HRow=NULL;

		hr = m_pIRowset->RestartPosition(NULL);
		if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
			goto CLEANUP;

		//try to get the last row handle
		if(CHECK(m_pIRowset->GetNextRows(NULL,m_ulTableRows,1,&cRows,&pHRow),S_OK))
			fTestPass=TRUE;
	}

CLEANUP:					 
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc hAccessor is NULL accessor and phRow is NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_immediate::Variation_2()
{
	HACCESSOR		hAccessor=NULL;
	BOOL			fTestPass=FALSE;

	//create a NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);

	//null accessor, pdata=NULL, and pHRow=NULL
	m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,NULL,NULL);

	//call again
	m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,NULL,NULL);

	fTestPass=TRUE;


CLEANUP:					 
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc hAccessor is valid accessor and phRow is valid.  pData is NULL.  E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_immediate::Variation_3()
{
	HROW			hRow		= 1;
	BOOL			fTestPass	= FALSE;

	
	//hAccessor is valid and pHRow is valid
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,NULL,&hRow),E_INVALIDARG);
		
	//the row handle should not be touched
	if(COMPARE(hRow, NULL))
		fTestPass=TRUE;


CLEANUP:					 
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc test DB_E_ROWSNOTRELEASED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_immediate::Variation_4()
{

	BOOL		fTestPass	= FALSE;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;

	HRESULT hr = S_OK;

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
		goto CLEANUP;

	//get the data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//get a row handle first
	TESTC_(m_pIRowset->GetNextRows(NULL,2,1,&cRows,&pHRow),S_OK);
			
	//insert should return DB_E_ROWSNOTRELEASED
	if(GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_ROWSNOTRELEASED);

		//no row should be returned
		if(!COMPARE(hRow, NULL))
			goto CLEANUP;
	}

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	//insert
	if(!GetProp(DBPROP_CANHOLDROWS))
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow), S_OK);
			
	//get next rows should fail
	if(!GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),DB_E_ROWSNOTRELEASED);
		//no rows should be returned
		if(!COMPARE(pHRow, NULL))
			goto CLEANUP;
	}
	else
		TESTC_(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK);
		

	
	//release the row
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
	hRow=NULL;

	//GetNextRows
	if(CHECK(m_pIRowset->GetNextRows(NULL, 0, 1, &cRows, &pHRow),S_OK))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	if(pHRow)
	{		
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc hChapter ignored
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_immediate::Variation_5()
{
	BOOL	fTestPass	= FALSE;
	ULONG	cRows		= 0;
	void	*pData		= NULL;
	HROW	hRow		= NULL;
	HROW	*pHRow		= NULL;

	HRESULT hr = S_OK;

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
	{
		goto CLEANUP;
	}
	//get the data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	TESTC_(m_pIRowsetChange->InsertRow(1,m_hAccessor,pData,&hRow),S_OK);

	fTestPass	= TRUE;
CLEANUP:
	//release the memory
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	}
	//release the row handle
	if(hRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
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
BOOL boundary_keyset_immediate::Terminate()
{

	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(boundary_keyset_buffered)
//*-----------------------------------------------------------------------
//| Test Case:		boundary_keyset_buffered - boundary_keyset_buffered
//|	Created:			05/13/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL boundary_keyset_buffered::Init()
{
	DBPROPID	rgDBPROPID[2];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	
	if(!TCIRowsetNewRow::Init())
		return FALSE;

	// Test case name is misleading, the cursor is not necessarily keyset
	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
									0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	//we should be on a buffered update mode
	COMPC(BufferedUpdate(), TRUE);

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc hAccessor is NULL accessor and phRow is valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_buffered::Variation_1()
{
	HACCESSOR		hAccessor=NULL;
	HROW			hRow=NULL;
	void			*pData=&hRow;	//pData points to invalid data
 	BOOL			fTestPass=FALSE;
		  

   	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,DB_NULL_HACCESSOR,NULL,NULL),DB_E_BADACCESSORHANDLE);
		
	//create a NULL accessor
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,0,NULL,0,&hAccessor,NULL),S_OK);
			
	//hAccessor is NULL accessor and phRow is valid
	m_hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pData,&hRow);

	//update
	m_hr=m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL);

	m_hr=m_pIRowsetUpdate->Undo(NULL,1,&hRow,NULL,NULL,NULL);

	fTestPass=TRUE;
CLEANUP:					 
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc hAccessor is NULL accessor and phRow is NULL.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int boundary_keyset_buffered::Variation_2()
{

	BOOL	fTestPass=FALSE;
	void	*pData=NULL;

	//get the data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert should return S_OK
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL), S_OK);
		
	//update
	if(CHECK(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL, NULL, NULL),S_OK))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);


	return fTestPass;
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL boundary_keyset_buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Invalid_Keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Invalid_Keyset_Cursor_Immediate - Invalid_Keyset_Cursor_Immediate
//|	Created:			05/13/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Cursor_Immediate::Init()
{
	DBPROPID	rgDBPROPID[2];
	BOOL		fTestPass=FALSE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	// Test case name is misleading, the cursor is not necessarily keyset
	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,NO_ACCESSOR));

	m_cColNumber	= GetNotUpdatable();
	m_cColUpdatable	= GetFirstUpdatable();

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The accessor is DBACCESSOR_READ | DBACCCESOR_ROWDATA. DB_E_READONLYACCESSOR.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_1()
{  	
	DBORDINAL	ulColsToBind	= 1;
	HROW		*pHRow			= NULL;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	BOOL		fTestPass		= FALSE;

	ulColsToBind=GetLongAndUpdatable();

	if(ulColsToBind==0)
	{
		fTestPass=TRUE;
		goto CLEANUP;
	}

	//create an accessor that has length binding length only binding is disabled by the spec except for long data
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,       
							1,&ulColsToBind))
			goto CLEANUP;
	
	fTestPass=TRUE;
CLEANUP:
	//release the memory
	PROVIDER_FREE(pData);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hRow)
	   	CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the accessor handle
	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Try to set an auto increment column.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_2()
{
	
	DBORDINAL	rgColsToBind[2]	= {1, 1};
	HROW		*pHRow			= NULL;
	IRowsetInfo	*pIRowsetInfo	= NULL;
	ICommand	*pICommand		= NULL;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	IUnknown	*pIUnknown		= NULL;
	BOOL		fTestPass		= TRUE;
	HRESULT		hr				= E_FAIL;
   
	//test pass if all columns are updatable
	if(m_cColNumber==0 || m_cColUpdatable==0)
	{
		return TEST_SKIPPED;
	}
	//Set up columns to update
	rgColsToBind[0]=m_cColUpdatable;
	rgColsToBind[1]=m_cColNumber;

	//reexecute the command so that we can have an accessor on the command to share
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY, 2,rgColsToBind))
	{
		goto CLEANUP;
	}
	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//get a copy of the original data
	memcpy(m_pData, pData, (size_t)m_cRowSize);
		
	if(m_bIndexExists)
	{
		//insert should fail
		hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);
	
		if (hr == DB_E_INTEGRITYVIOLATION)
		{
			//no row should be inserted
			if(!COMPARE(hRow, NULL))
			{
				goto CLEANUP;
			}
		}
		else if (hr == DB_S_ERRORSOCCURRED)
		{
			//the status of the set columns should be success
			if(!COMPARE(GetStatus(pData, &(m_rgBinding[0])), DBSTATUS_S_OK))
			{
				goto CLEANUP;
			}
			//Check status for the non-updatable column
			if(!COMPARE(GetStatus(pData, &(m_rgBinding[1])),DBSTATUS_E_INTEGRITYVIOLATION))
			{
				goto CLEANUP;
			}
				
			//a row should be inserted
			if(!COMPARE(hRow!=NULL, TRUE))
			{
				goto CLEANUP;
			}
		}
	}
	fTestPass=TRUE;

CLEANUP:
	
	if(pHRow)
	{
		CHECK(((IRowset *)(pIUnknown))->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
		
	
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommand);

	//release the row handle
	if(hRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	}

	//release the accessor handle
	FreeMemory();
	ReleaseAccessorOnRowset();
	PROVIDER_FREE(pData);	

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc The column number specified in the last binding structure = # of columns of the 	rowset+1.	DB_E_COLUMNUNAVAILABLE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_3()
{	
	HROW			hRow		= NULL;
	HACCESSOR		hAccessor	= NULL;
	DBBINDSTATUS *	prgDBBindStatus = NULL;	
	void			*pData		= NULL;
	BOOL			fTestPass	= TRUE;

	//create an accessor which binds to all the updatale columns
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))
			goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
        goto CLEANUP;
    }

	// Allocate binding status array
	SAFE_ALLOC(prgDBBindStatus, DBBINDSTATUS, m_cBinding);

	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	fTestPass = FALSE;
	//change to column number of the last binding structure
	m_rgBinding[m_cBinding-1].iOrdinal=m_cBinding+100;

	//create an accessor
	if(FAILED(m_hr=m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding, m_rgBinding, 0,&hAccessor,prgDBBindStatus)))
	{
		COMPARE(m_hr, DB_E_ERRORSOCCURRED);
		COMPARE(prgDBBindStatus[m_cBinding-1],DBBINDSTATUS_BADORDINAL);
		fTestPass=TRUE;
		hRow=NULL;
		goto CLEANUP;
	}

	//insert should fail
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pData,&hRow),DB_E_BADORDINAL);
		
	//no row should be inserted
	if(COMPARE(hRow, NULL))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
	//release the accessor handle
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL), S_OK);

	//release the accessor handle
	ReleaseAccessorOnRowset();

	SAFE_FREE(prgDBBindStatus);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc The accessor sets the status field of non nullable columns NULL.  	DB_E_SCHENMAVIOLATION.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_4()
{
	DBORDINAL	*rgColsToBind	= NULL;
	DBORDINAL	cColNumber		= 0;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	BOOL		fTestPass		= TRUE;

	//get updatble and not nullable columns
	if(!GetNotNullableAndUpdatable(&cColNumber,&rgColsToBind))
		goto CLEANUP;

	//create an accessor which binds to all updatble 
	//but not nullable columns
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,       
							cColNumber,rgColsToBind))
			goto CLEANUP;

	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//make the last column to NULL
//	*(DBSTATUS *)(((DWORD)pData)+m_rgBinding[m_cBinding-1].obStatus)=DBSTATUS_S_ISNULL;
	STATUS_BINDING(m_rgBinding[m_cBinding-1],pData)=DBSTATUS_S_ISNULL;
	
	//insert should fail
	m_hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);

	// three possibilites
	if (m_hr == DB_E_INTEGRITYVIOLATION)
	{
		// Provider detects the integrityviolation but is unable to detect the binding that caused it
		// and rejects the insert
		fTestPass = COMPARE(hRow, NULL);
	}
	else if (m_hr == DB_E_ERRORSOCCURRED)
	{
		// Provider detects the integrityviolation and detects the binding that caused it
		// and rejects the insert
		if(COMPARE(GetStatus(pData, &(m_rgBinding[m_cBinding-1])), DBSTATUS_E_INTEGRITYVIOLATION) ||
			COMPARE(GetStatus(pData, &(m_rgBinding[m_cBinding-1])), DBSTATUS_E_PERMISSIONDENIED))
			fTestPass=TRUE;

		COMPARE(hRow, NULL);
	}
	else if (m_hr == DB_S_ERRORSOCCURRED)
	{
		// Provider detects the integrityviolation and detects the binding that caused it	
		// but inserts the row as if the bad bindings had not been made
		if(COMPARE(GetStatus(pData, &(m_rgBinding[m_cBinding-1])), DBSTATUS_E_INTEGRITYVIOLATION) ||
			COMPARE(GetStatus(pData, &(m_rgBinding[m_cBinding-1])), DBSTATUS_E_PERMISSIONDENIED))
			fTestPass=TRUE;

		COMPARE(hRow!=NULL, TRUE);
	}
	else
		TESTC_(m_hr, DB_E_INTEGRITYVIOLATION); // raise an error, if none of the 3 above cases apply.

	
CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	PROVIDER_FREE(rgColsToBind);
	
	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	

	//release the accessor handle
	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has status binding for DBSTATUS_S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_5()
{
	
	ULONG		cCount;
	HROW		hRow		= NULL;
	void		*pData		= NULL;
	BOOL		fTestPass	= TRUE;

	//create an accessor that only has status binding
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_STATUS,UPDATEABLE_COLS_BOUND))
	{
		hRow=NULL;
		goto CLEANUP;
	}

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//get some data to insert
	pData=PROVIDER_ALLOC(m_cRowSize);

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}

	fTestPass = FALSE;
	//insert should fail
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_ERRORSOCCURRED);
		
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])),DBSTATUS_E_UNAVAILABLE))
			goto CLEANUP;
	}

	//no row should be inserted
	if(COMPARE(hRow, NULL))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the accessor handle
	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Set a duplicate column on which a unique index is created. DB_E_INTEGRITYVIOLATION.  No data is changed.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_6()
{
	DBORDINAL	ulColsToBind	= 1;
	HROW		hRow			= NULL;
	void		*pData			= NULL;
	BOOL		fTestPass		= TRUE;
	HRESULT		hr				= S_OK;

	//create an accessor which binds to 1st column
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,       
							1,&ulColsToBind))
	{
		goto CLEANUP;
	}

	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//copy the old data
	if(!memcpy(m_pData, pData, (size_t)m_cRowSize))
		goto CLEANUP;

	if(m_bIndexExists)
	{
		//insert could pass or return DB_E_INTEGRITYVIOLATION if a not null column in not bound
		hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);

		if (S_OK!=hr&&DB_E_INTEGRITYVIOLATION!=hr)
		{
			goto CLEANUP;
		}

		//2nd insert should fail with INTEGRITYVIOLATION becuase of inserting duplicate column 
		//or because a not null column in not bound
		hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);

		//set the status of the buffers to the same so the compare below is equal
		//since this is a error that is not ERRORSOCCURED it is undefined what is in the stauts part
		//of the buffer
//		*(DBSTATUS *)((DWORD)m_pData+m_rgBinding[0].obStatus)=DBSTATUS_S_OK; 
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)=DBSTATUS_S_OK; 
		STATUS_BINDING(m_rgBinding[0],m_pData)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_OK;
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_INTEGRITYVIOLATION);

		//set the status of the buffers to the same so the compare below is equal
		//since this is a error that is not ERRORSOCCURED it is undefined what is in the stauts part
		//of the buffer
//		*(DBSTATUS *)((DWORD)m_pData+m_rgBinding[0].obStatus)=DBSTATUS_S_OK; 
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[0].obStatus)=DBSTATUS_S_OK; 
		STATUS_BINDING(m_rgBinding[0],m_pData)=DBSTATUS_S_OK;
		STATUS_BINDING(m_rgBinding[0],pData)=DBSTATUS_S_OK;

		//no data should be changed	
		if(!COMPARE(memcmp(m_pData, pData, (size_t)m_cRowSize), 0))
			goto CLEANUP;

		//no row should be inserted
		if(COMPARE(hRow, NULL))
			fTestPass=TRUE;
	}
	else
	{
		//insert should succeed
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
		
		fTestPass=TRUE;
	}

	
CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	

	//release the accessor handle
	ReleaseAccessorOnRowset();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc The accessor is a parameter accessor.  DB_E_BADACCESSORTYPE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Cursor_Immediate::Variation_7()
{
	DBORDINAL	ulColsToBind	= 1;
	HROW		hRow			= DB_NULL_HROW;
	void		*pData			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	DBPROPID	rgDBPROPID[3];
	HRESULT		hr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;

	if(!m_pIDBCreateCommand)
	{
		return TEST_SKIPPED;  // This variation relies on creating on accessor off the command object.
	}
	//release the rowset
	ReleaseRowsetAndAccessor();

	//create a rowset
	if (S_OK != GetRowsetAndAccessor(SELECT_ALLFROMTBL,3,rgDBPROPID,0,NULL,ON_COMMAND_ACCESSOR,	TRUE,
							DBACCESSOR_PARAMETERDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,       
							1,&ulColsToBind,NO_COLS_OWNED_BY_PROV,DBPARAMIO_INPUT))
	{		
		hRow	= NULL;
		goto CLEANUP;
	}

	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert should fail
	hr	= m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);
	
	//DB_E_INVALIDACCESSOR could be ok here if the rowset chooses not to inherit the parameter accessor from the 
	//command object
	if(	DB_E_BADACCESSORTYPE	!= hr &&
		DB_E_BADACCESSORHANDLE	!= hr)
	{
		goto CLEANUP;
	}
		
	//no row should be inserted
	if(COMPARE(hRow, NULL))
	{
		fTestPass=TRUE;
	}
CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the accessor handle
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
BOOL Invalid_Keyset_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();	

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Invalid_Keyset_Query_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Invalid_Keyset_Query_Immediate - Invalid_Keyset_Query_Immediate
//|	Created:			05/13/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Invalid_Keyset_Query_Immediate::Init()
{
	BOOL		fTestPass=FALSE;
	DBPROPID	rgDBPROPID[2];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	
	if(!TCIRowsetNewRow::Init())
		return FALSE;

	// TestCase name is misleading since the cursor is not necessarily keyset
	//create a rowset and a accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC, NUMELEM(rgDBPROPID), rgDBPROPID,0,NULL,NO_ACCESSOR));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc The accessor is  DBACCCESOR_PASSCOLUMNSBYREF. DB_E_READONLYACCESSOR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Query_Immediate::Variation_1()
{
	return TEST_PASS;
		
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc The status flag specified in a binding structure is neither 	DBSTATUS_S_OK  nor DBSTATUS_S_ISNULL.  	DB_E_BADSTATUSVALU
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Query_Immediate::Variation_2()
{
	HROW		hRow=DB_NULL_HROW;
	ULONG		cCount;
	void		*pData=NULL;
	BOOL		fTestPass=TRUE;
	HRESULT	hr = S_OK;

	//create an accessor which binds to all updatable columns
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
			goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//change the status of columns binding
//	*(DBSTATUS *)((DWORD)pData+m_rgBinding[m_cBinding-1].obStatus)=DBSTATUS_E_INTEGRITYVIOLATION;
	STATUS_BINDING(m_rgBinding[m_cBinding-1],pData)=DBSTATUS_E_INTEGRITYVIOLATION;

	//insert should fail
	hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);
	
	if ( hr != DB_S_ERRORSOCCURRED && hr != DB_E_ERRORSOCCURRED )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//check the status for other data
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		if(cCount==(m_cBinding-1))
			continue;
		if ( hr == DB_E_ERRORSOCCURRED )
		{
			if(!COMPARE(GetStatus(pData, &(m_rgBinding[cCount])), DBSTATUS_E_UNAVAILABLE))
				goto CLEANUP;
		}
		else
		{
			if (GetStatus(pData, &(m_rgBinding[cCount])) != DBSTATUS_S_OK &&
				GetStatus(pData,&(m_rgBinding[cCount])) != DBSTATUS_S_ISNULL )
				goto CLEANUP;

			COMPC(hRow==DB_NULL_HROW, FALSE);			
		}
	}


	if(COMPARE(GetStatus(pData, &(m_rgBinding[m_cBinding-1])), DBSTATUS_E_BADSTATUS))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	

	//release the accessor handle
	ReleaseAccessorOnRowset();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc The accessor only has length binding.  E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Query_Immediate::Variation_3()
{
	ULONG		cCount					= 0;
	HROW		hRow					= NULL;
	void		*pData					= NULL;
	BOOL		fTestPass				= TRUE;
	HRESULT		hr						= E_FAIL;
	DBORDINAL	cRowsInTableBeforeInsert= 0;
	DBORDINAL	cRowsInTableAfterInsert = 0;

	//create an accessor that has length binding
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,	FALSE,DBACCESSOR_ROWDATA,DBPART_LENGTH,UPDATEABLE_COLS_BOUND))
			goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	fTestPass = FALSE;
	//mark the bogus length information
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(ULONG *)((DWORD)pData+m_rgBinding[cCount].obLength)=2;
		LENGTH_BINDING(m_rgBinding[cCount],pData)=2;
	}
	cRowsInTableBeforeInsert = m_pTable->CountRowsOnTable();
	hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);
	cRowsInTableAfterInsert = m_pTable->CountRowsOnTable();

	if (hr==DB_E_ERRORSOCCURRED)
	{
		// check count of rows
		COMPC(cRowsInTableBeforeInsert,cRowsInTableAfterInsert);
		
		//The row handle should be NULL
		COMPC(hRow, NULL);		
	}
	else if (hr==DB_S_ERRORSOCCURRED)
	{
		// check row was added
		COMPC(cRowsInTableBeforeInsert+1,cRowsInTableAfterInsert);
 		
		//no row should be inserted
		COMPC(hRow!=NULL, TRUE);		
	}
	else
		TESTC_(hr, DB_E_ERRORSOCCURRED); // generate error		

	fTestPass = TRUE;
		
CLEANUP:
	//release the memory
	PROVIDER_FREE(pData);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the accessor handle
	ReleaseAccessorOnRowset();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc The iOrdinal is out of range.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Invalid_Keyset_Query_Immediate::Variation_4()
{
	HROW			hRow		=1;
	HACCESSOR		hAccessor	=NULL;
	DBBINDSTATUS *	prgDBBindStatus = NULL;	
	void			*pData		=NULL;
	BOOL			fTestPass	=TRUE;

	//create an accessor which binds to all the updatale columns
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND))
	{
		hRow=NULL;
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	// Allocate binding status array
	SAFE_ALLOC(prgDBBindStatus, DBBINDSTATUS, m_cBinding);


	fTestPass = FALSE;
	//get some data to insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//change to column number of the last binding structure
	m_rgBinding[m_cBinding-1].iOrdinal=m_cBinding+100;

	//create an accessor
	if(FAILED(m_hr=m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBinding, m_rgBinding, 0,&hAccessor,prgDBBindStatus)))
	{
		COMPARE(m_hr, DB_E_ERRORSOCCURRED);
		COMPARE(prgDBBindStatus[m_cBinding-1],DBBINDSTATUS_BADORDINAL);
		hRow=NULL;
		fTestPass=TRUE;
		goto CLEANUP;
	}


	//insert should fail
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pData,&hRow),DB_E_BADORDINAL);

	//no row should be inserted
	if(COMPARE(hRow, NULL))
		fTestPass=TRUE;

CLEANUP:
	//release the memory
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
	//release the accessor handle
	if(hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL), S_OK);

	//release the accessor handle
	ReleaseAccessorOnRowset();

	SAFE_FREE(prgDBBindStatus);

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
BOOL Invalid_Keyset_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Valid_Keyset_Cursor_Immediate)
//*-----------------------------------------------------------------------
//| Test Case:		Valid_Keyset_Cursor_Immediate - Valid_Keyset_Cursor_Immediate
//|	Created:			05/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Valid_Keyset_Cursor_Immediate::Init()
{
	BOOL			fTestPass=FALSE;
	ULONG			cBinding;
	IConvertType	*pIConvertType = NULL;

	m_rgDBPROPID[0]=DBPROP_IRowsetChange;
	m_cDBPROPID=1;

	m_fCanConvertFromArray = TRUE;
	m_fCanConvertFromVector = TRUE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//The cursor does not have to be keyset
	//create a rowset 
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND,FORWARD,NO_COLS_BY_REF));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	if(!VerifyInterface(m_pIRowset, IID_IConvertType,
			ROWSET_INTERFACE,(IUnknown **)&pIConvertType))
			goto CLEANUP;

	for ( cBinding = 0; cBinding < m_cBinding; cBinding++ )
	{
		m_fCanConvertFromArray &= ( S_OK == pIConvertType->CanConvert( m_rgBinding[cBinding].wType | DBTYPE_ARRAY,
														m_rgBinding[cBinding].wType, DBCONVERTFLAGS_COLUMN));
		m_fCanConvertFromVector &= ( S_OK == pIConvertType->CanConvert( m_rgBinding[cBinding].wType | DBTYPE_VECTOR, 
														m_rgBinding[cBinding].wType, DBCONVERTFLAGS_COLUMN));
	}

	TESTC_(m_pTable->DeleteRows(ALLROWS), S_OK);
	TESTC_(m_pTable->Insert(PRIMARY, 1, g_ulRowCount), S_OK);

	fTestPass = TRUE;

CLEANUP:
	SAFE_RELEASE(pIConvertType);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Insert the variable length columns, in forward order of the rowset.  Value & length binding only.
// 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_1()
{
	HROW		hRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	void		*pData		= NULL;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;

	//open a rowset.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,NO_ACCESSOR));

	//value and length binding only
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
							FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert should be successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//hRow should not be NULL
	if(!hRow)
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=NULL;

	ReleaseRowsetAndAccessor();

	//get the rowset again
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,
					TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
					FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY))
		goto CLEANUP;

	
	// the insert should be visible
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}

CLEANUP:
	
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	if(pHRow)
	{
	  	CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Insert the fixed length columns with only value binding.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_2()
{
  	DBORDINAL	*rgColsToBind	= NULL;
	DBORDINAL	cColsNumber		= 0;
	HROW		hRow			= NULL;
	HROW		*pHRow			= NULL;
	DBCOUNTITEM	cRows			= 0;
	void		*pData			= NULL;
	BOOL		fTestPass		= TRUE;
	HRESULT		hr;


	//open a rowset.
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,NO_ACCESSOR));

	//Binds to all fixed length columns.
	if(!GetFixedLengthAndUpdatable(&cColsNumber,&rgColsToBind))
		goto CLEANUP;

	//value binding only
	// Need status bound as well to avoid accessing data when it is NULL.
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS,USE_COLS_TO_BIND_ARRAY,
								FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind))
		goto CLEANUP;

	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
					 
	fTestPass = FALSE;
	
	//insert a row.  Do not care about the result.  It could possiblly fail.
	m_hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow);

	if(FAILED(m_hr))
	{
	   if(!CHECK(m_hr, DB_E_ERRORSOCCURRED))
		   goto CLEANUP;
	   if(!COMPARE(hRow, NULL))
		   goto CLEANUP;
	   fTestPass=TRUE;
	   goto CLEANUP;

	}

	 //hRow should not be NULL
	if(!hRow)
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=NULL;

	ReleaseRowsetAndAccessor();

	//get the rowset again (but use status and length bindings)
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS,USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind))
		goto CLEANUP;

	// the insert should be visible
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}



CLEANUP:
	
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	PROVIDER_FREE(rgColsToBind);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
	if(pHRow)
	{
	  	CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DBTYPE_BYREF binding for all columns.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_3()
{
	void			*pData			= NULL;
	void			*pValidationData= NULL;
	BOOL			fTestPass		= TRUE;
	DBBINDING		*rgBinding		= NULL;
	DBORDINAL		cCol			= 0;
	HROW			*pHRow			= NULL;
	DBORDINAL		*rgColNumber	= NULL;
	HROW			hRow;
	DBCOUNTITEM		ulInputSeed		= g_ulNextRow++;


	//create a rowset and accessor that binds to DBTYPE_BYREF
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,NO_ACCESSOR));
	
	//GetUpdatable columns
	if(!GetFixedLengthAndUpdatable(&cCol, &rgColNumber))
		goto CLEANUP;

	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
					USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_BYREF,cCol,rgColNumber))
		goto CLEANUP;

	fTestPass=FALSE;
	
	//create data to set to the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,ulInputSeed,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//set data
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor, pData,&hRow),S_OK);

	ReleaseInputBindingsMemory(m_cBinding,m_rgBinding,(BYTE*)pData);
	ReleaseAccessorOnRowset();

	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR, TRUE, DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
					USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,DBTYPE_EMPTY,cCol,rgColNumber))
		goto CLEANUP;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pValidationData,ulInputSeed,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

		//get data should succeed
	TESTC_(m_pIRowset->GetData(hRow, m_hAccessor, m_pData),S_OK);
		
	//compare data should be successful
	if(!CompareBuffer(m_pData,pValidationData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE))
		goto CLEANUP;

	ReleaseAccessorOnRowset();
	ReleaseRowsetAndAccessor();

	fTestPass = TRUE;	

CLEANUP:
	
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pValidationData);
	
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	PROVIDER_FREE(rgColNumber);
	PROVIDER_FREE(rgBinding);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DBTYPE_ARRAY binding for all columns.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_4()
{
	HROW	hRow=1;
	ULONG	cCount;
	void	*pData=NULL;
	BOOL	fTestPass=TRUE;

	if ( !m_fCanConvertFromArray )
		return TEST_SKIPPED;

	//open a rowset binds to all updatable columns by DBTYPE_ARRAY
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND,FORWARD,NO_COLS_BY_REF, DBTYPE_ARRAY))
	{
		hRow	= NULL;
		goto CLEANUP;
	}

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass=FALSE;
	//make data for insert
	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
	{
		hRow	= NULL;
		goto CLEANUP;
	}
	//set the status field
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK; 
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;

	}
	//insert should be successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_UNSUPPORTEDCONVERSION);

	//hRow should be NULL
	if(hRow)
	{
		goto CLEANUP;
	}
	fTestPass=TRUE;


CLEANUP:

	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBTYPE_VECTOR binding for all columns.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Cursor_Immediate::Variation_5()
{
	HROW	hRow=1;
	ULONG	cCount;
	void	*pData=NULL;
	BOOL	fTestPass=TEST_FAIL;

	if (!m_fCanConvertFromVector)
		return TEST_SKIPPED;

	//open a rowset binds to all updatable columns by DBTYPE_ARRAY
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND,
							FORWARD,NO_COLS_BY_REF, DBTYPE_VECTOR));
		
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert
	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
		goto CLEANUP;

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
//		*(DBSTATUS *)((DWORD)pData+m_rgBinding[cCount].obStatus)=DBSTATUS_S_OK; 
		STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_OK;
	}
					 
	//insert should be successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_UNSUPPORTEDCONVERSION);

	//hRow should not be NULL
	if(hRow)
		goto CLEANUP;

	fTestPass=TEST_PASS;


CLEANUP:

	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	
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
BOOL Valid_Keyset_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Valid_Keyset_Query_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		Valid_Keyset_Query_Buffered - Valid_Keyset_Query_Buffered
//|	Created:			05/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Valid_Keyset_Query_Buffered::Init()
{
	BOOL		fTestPass=TEST_SKIPPED;

	m_rgDBPROPID[0]=DBPROP_IRowsetChange;
	m_rgDBPROPID[1]=DBPROP_IRowsetUpdate;	
	
	m_cDBPROPID=2;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//Cursor does not have to be keyset
	//create a rowset 
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC, m_cDBPROPID, m_rgDBPROPID,
		0,NULL,NO_ACCESSOR));
	
	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Insert the fixed length data type columns with bogus length information.
// information.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Query_Buffered::Variation_1()
{
  	DBORDINAL	*rgColsToBind		= NULL;
	DBCOUNTITEM	cCount				= 0;
	DBORDINAL	cColsNumber			= 0;
	HROW		hRow				= NULL;
	HROW		*pHRow				= NULL;
	DBROWSTATUS	rgDBRowStatus[1];
	DBROWSTATUS	*pDBRowStatus		= rgDBRowStatus;
	DBCOUNTITEM	cRows				= 0;
	void		*pData				= NULL;
	DBROWSTATUS	DBRowStatus;
	BOOL		fTestPass			= TRUE;
	HRESULT		hr;


	//open a rowset.
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,0,NULL,NO_ACCESSOR))
		goto CLEANUP;

	//Binds to all fixed length columns.
	if(!GetFixedLengthAndUpdatable(&cColsNumber,&rgColsToBind))
		goto CLEANUP;

	//value and length binding only
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
							FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind))
		goto CLEANUP;

	fTestPass=FALSE;
	//make data for insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//set the length for the 1st column as 0
//	*(ULONG *)(dwAddr+m_rgBinding[0].obLength)=0;
	LENGTH_BINDING(m_rgBinding[0],pData)	= 0;

	//insert should be successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//hRow should not be NULL
	if(!hRow)
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL, &DBRowStatus),S_OK);
	hRow=NULL;

	if(!COMPARE(DBRowStatus, DBROWSTATUS_S_PENDINGCHANGES))
		fTestPass=FALSE;

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cCount,&pHRow,&pDBRowStatus),S_OK);

	if(!COMPARE(cCount, 1))
		goto CLEANUP;

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pHRow);
	ReleaseRowsetAndAccessor();

	//get the rowset again
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,
							0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH,USE_COLS_TO_BIND_ARRAY,
							FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, cColsNumber, rgColsToBind))
		goto CLEANUP;
	
	// the insert should be visible
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		FreeMemory();
	}

	
CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColsToBind);
	PROVIDER_FREE(pDBRowStatus);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	if(pHRow)
	{
	  	CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Insert  the whole row with status only.  Set everything to NULL
//  Another call clears the value.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Valid_Keyset_Query_Buffered::Variation_2()
{
	void			*pData			= NULL;
	DBORDINAL		cCols			= 0;
	DBORDINAL		*rgColsToBind	= 0;
	ULONG			cCount			= 0;
	DBORDINAL		cColCount		= 0;
	DBCOUNTITEM		cRows			= 0;
	HROW			hRow			= NULL;
	HROW			*pHRow			= NULL;
	IRowsetChange	*pIRowsetChange	= NULL;
	BOOL			fTestPass		= TRUE;
	HRESULT			hr;

	//get a rowset binds to all updatable columns
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,
		UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//get the nullable and updatable columns
	if(!GetNullableAndUpdatable(&cCols, &rgColsToBind))
		goto CLEANUP;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	fTestPass=FALSE;

	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		for(cColCount=0; cColCount<cCols; cColCount++)
		{
			if(m_rgBinding[cCount].iOrdinal==rgColsToBind[cColCount])
			{
//				*(DBSTATUS *)(dwAddr+m_rgBinding[cCount].obStatus)=DBSTATUS_S_ISNULL;
				*(DBSTATUS*)&STATUS_BINDING(m_rgBinding[cCount],pData)=DBSTATUS_S_ISNULL;
				//exit the inner loop
				cColCount=cCols;
			}
		}
	}

	//insert should be successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//hRow should not be NULL
	if(!hRow)
		goto CLEANUP;

	//release the memory
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);
	pData=NULL;

	//make data for the 1st row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//change the data cach
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,pData),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	hRow=NULL;
	ReleaseRowsetAndAccessor();

	//get the rowset again
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,m_cDBPROPID,m_rgDBPROPID,
							0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	// the insert should be visible
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
	{
		if( cRows ==0)
			break;
		//Get the data for the 10th row handle
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
		//make sure GetData should be able to see the change
		if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
			TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			fTestPass=TRUE;
			FreeMemory();
			break;
		}
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		FreeMemory();
	}



CLEANUP:
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColsToBind);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	if(pHRow)
	{
	  	CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Valid_Keyset_Query_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetNewRow::Terminate());
}


// {{ TCW_TC_PROTOTYPE(Sequence)
//*-----------------------------------------------------------------------
//| Test Case:		Sequence - sequence testing
//|	Created:			05/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Sequence::Init()
{
	if(TCIRowsetNewRow::Init())
		return TRUE;

	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_CanHoldRows.  Set 3 rows in a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_1()
{
	DBPROPID	rgDBPROPID[4];
	ULONG		cCount		= 0;
	DBCOUNTITEM	cRows		= 0;
	void		*rgpData[25];
	HROW		rgHRow[25];
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;
	rgDBPROPID[3]=DBPROP_IRowsetLocate;

	//Init variables
	for(cCount=0; cCount<25; cCount++)
	{
		rgpData[cCount]=NULL;
		rgHRow[cCount]=NULL;
	}

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = FALSE;
		
	for(cCount=0; cCount<25; cCount++)
	{
		//make data for the 1st row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]), (g_ulNextRow++),m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);

		//insert 
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[cCount],&(rgHRow[cCount])),S_OK);

		//hRow should not be NULL
		if(!rgHRow[cCount])
			goto CLEANUP;
	}

	//release the row handles
	for(cCount=0; cCount<25; cCount++)
	{
		if(rgHRow[cCount])
		{
			TESTC_(m_pIRowset->ReleaseRows(1, &(rgHRow[cCount]),NULL,NULL,NULL),S_OK)
			rgHRow[cCount]=NULL;
		}
	}

	//reexecute the rowset
	ReleaseRowsetAndAccessor();

	//get a rowset binds to all updatable columns
	if(S_OK != GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND))
		goto CLEANUP;

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make sure all  rows are inserted
	for(cCount=0; cCount<25; cCount++)
	{
		//get a row handle
		pHRow=&(rgHRow[cCount]);
		if(cCount==0)
		{
			TESTC_(m_pIRowset->GetNextRows(NULL, m_ulTableRows, 1,&cRows, &pHRow),S_OK);
		}
		else
		{
			TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1,&cRows, &pHRow),S_OK);
		}
		
		//get data
		TESTC_(m_pIRowset->GetData(rgHRow[cCount], m_hAccessor, m_pData),S_OK);
		FreeMemory();
	}

	fTestPass=TRUE;

CLEANUP:

	//Release the row handle and memory
	for(cCount=0; cCount<25; cCount++)
	{
		if( rgHRow[cCount] )
			CHECK(m_pIRowset->ReleaseRows(1, &(rgHRow[cCount]), NULL,NULL,NULL),S_OK);
		
		if( rgpData[cCount] )
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)rgpData[cCount],TRUE);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Unset DBPROP_CanHoldrows.  Insert 2 rows.  DB_E_ROWSNOTRELEASED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_2()
{
	DBPROPID	rgDBPROPID[3];
	DBPROPID	DBPropID=DBPROP_CANHOLDROWS;
	ULONG		cCount=0;
	void		*rgpData[3]={NULL,NULL,NULL};
	HROW		rgHRow[3]={NULL,NULL,NULL};
	BOOL		fTestPass=TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
								1, &DBPropID,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	
	if(GetProp(DBPROP_CANHOLDROWS)==TRUE)
		goto CLEANUP;
	
	fTestPass = FALSE;
	//make data for the 1st row handle
	for(cCount=0; cCount<3; cCount++)
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,(BYTE **)&(rgpData[cCount]),
								g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert 
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&(rgHRow[0])),S_OK);

	//insert the second row.  Do not ask for any row handle back.
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[1],NULL),S_OK);

	//insert the third row.  Rows not released.
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&(rgHRow[1])),DB_E_ROWSNOTRELEASED);

	//release the 1st row
	TESTC_(m_pIRowset->ReleaseRows(1, &(rgHRow[0]), NULL,NULL,NULL),S_OK);
	rgHRow[0]=NULL;;

	//insert the third row.  Successful
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&(rgHRow[1])),S_OK);
			
	fTestPass=TRUE;

CLEANUP:

	//Release the row handle and memory
	for(cCount=0; cCount<3; cCount++)
	{
		if(rgHRow[cCount])
			CHECK(m_pIRowset->ReleaseRows(1, &(rgHRow[cCount]), NULL,NULL,NULL),S_OK);
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount],TRUE);
	} 

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Buffered mode.  Unset DBPROP_CanHoldRows.  Insert 2 rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_3()
{
	DBPROPID	rgDBPROPID[4];
	DBPROPID	DBPropID	= DBPROP_CANHOLDROWS;
	DBCOUNTITEM	cCount		= 0;
	void		*rgpData[3]	= {NULL,NULL,NULL};
	HROW		rgHRow[3]	= {NULL,NULL,NULL};
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TRUE;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;
	rgDBPROPID[3]=DBPROP_IRowsetIdentity;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,
									1,&DBPropID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetUpdate)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = FALSE;
	//make data for the 1st row handle
	for(cCount=0; cCount<3; cCount++)
	{
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
			(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert one row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&(rgHRow[0])),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&(rgHRow[0]),&cCount,&pHRow,NULL),S_OK);

	if(!COMPARE(cCount, 1))
		goto CLEANUP;

  	if(StrongIdentity())
		CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, rgHRow[0]),S_OK);

	//release the row handle		
	TESTC_(m_pIRowset->ReleaseRows(1, rgHRow, NULL,NULL,NULL),S_OK);

	rgHRow[0]=NULL;
	PROVIDER_FREE(pHRow);
	
	//insert the second row.  Should succeed
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[1],NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cCount,&pHRow,NULL),S_OK);

	//insert the third row.  Should report Rows not released
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&(rgHRow[2])),DB_E_ROWSNOTRELEASED);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);

	PROVIDER_FREE(pHRow);
	
	//insert the third row.  Should be fine
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&(rgHRow[2])),S_OK);

	//update
	if(CHECK(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL, NULL, NULL),S_OK))
			fTestPass=TRUE;

CLEANUP:

	//Release the row handle and memory
	for(cCount=0; cCount<3; cCount++)
	{

		if(rgHRow[cCount])
			CHECK(m_pIRowset->ReleaseRows(1, &(rgHRow[cCount]), NULL,NULL,NULL),S_OK);

		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount],
			TRUE);
	}

	if(pHRow)
	{			
		CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Insert 50 rows.  Update.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_4()
{
	DBPROPID	rgDBPROPID[4];
	ULONG		cCount		= 0;
	DBCOUNTITEM	cRows		= 0;
	void		*rgpData[STRESS_COUNT];
	HROW		*pHRow		= NULL;
	BOOL		fTestPass	= TRUE;
	ULONG_PTR	ulValue		= 0;

	//init
	for(cCount=0; cCount<STRESS_COUNT; cCount++)
		rgpData[cCount]=NULL;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,4,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));


	GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET,m_pIRowset,&ulValue);
	
	if (!m_pIRowsetUpdate)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass = FALSE;
	
	for(cCount=0; cCount<ulValue && cCount<STRESS_COUNT; cCount++)
	{
		//make data for the 1st row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
		
		//insert 
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[cCount],NULL),S_OK);
	
	}
	
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cRows, &pHRow,NULL),S_OK);
	
	if(COMPARE(cRows,(ulValue>STRESS_COUNT? STRESS_COUNT:ulValue) ))
		fTestPass=TRUE;	


CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<ulValue && cCount<STRESS_COUNT ; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)rgpData[cCount], TRUE);
	}

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Buffered mode,DBPROP_CanHoldRows FALSE,Insert row/release it,insert/get another
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Sequence::Variation_5()
{
	DBPROPID	rgDBPROPID[3];
	DBPROPID	DBPropID	= DBPROP_CANHOLDROWS;
	ULONG		cCount		= 0;
	DBCOUNTITEM	cRows		= 1;
	void		*rgpData[2]	= {NULL,NULL};
	HROW		HRow		= NULL;
	HROW		HRowBad		= NULL;
	HROW		*pHRowBad   = NULL;
	BOOL		fTestPass	= TEST_SKIPPED;
	ULONG		cRowsUp		= 0;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_IRowsetIdentity;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
									1,&DBPropID,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
									DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	if (!m_pIRowsetUpdate)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass = TEST_FAIL;

	//make data for insert
	for(cCount=0; cCount<2; cCount++)
	{
		//make data for the 1st row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	}
	//insert one row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&HRow),S_OK);

	//release the row handle		
	TESTC_(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);

	//insert another.  Should report Rows not released
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&HRowBad),DB_E_ROWSNOTRELEASED);
	COMPARE(HRowBad,NULL);

	//get the 1st row handle.  Should report Rows not released
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowBad),DB_E_ROWSNOTRELEASED);
	COMPARE(pHRowBad,NULL);
	COMPARE(cRows,0);

	//update all changes
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	//insert another row.  Should be fine
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[1],&HRow),S_OK);
				
	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);
	fTestPass=TRUE;
CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<2; cCount++)
	{
		if(rgpData[cCount])
		{
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
		}
	}
	if(HRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);
	}
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
BOOL Sequence::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_IRowsetDelete)
//*-----------------------------------------------------------------------
//| Test Case:		Related_IRowsetDelete - test Related_IRowsetDelete wiht IRowsetNewRow
//|	Created:			05/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowsetDelete::Init()
{
	BOOL		fTestPass=FALSE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	fTestPass = TRUE;

	return fTestPass;

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc In immediately update mode, insert two rows, delete one, the insert another row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowsetDelete::Variation_1()
{
	void		*rgpData[3]	= {NULL,NULL,NULL};
	HROW		rgHRow[3]	= {NULL,NULL,NULL};
	ULONG		cCount;
	DBCOUNTITEM	cRows;
	HROW		*pHRow		= NULL;
	DBROWSTATUS	DBRowStatus;
	BOOL		fTestPass	= TEST_SKIPPED;
	DBPROPID	rgDBPROPID[4];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;

	HRESULT hr = S_OK;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,0,NULL,
								ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert
	for(cCount=0; cCount<3; cCount++)
	{
		//make data for the 1st row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
					(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert 
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&(rgHRow[0])),S_OK);

	//insert another row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[1],&(rgHRow[1])),S_OK);

	//delete the row
	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&(rgHRow[0]),&DBRowStatus),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&(rgHRow[0]),&DBRowStatus),DB_E_ERRORSOCCURRED);
		COMPARE(DBRowStatus, DBROWSTATUS_E_NEWLYINSERTED);
	}


	//insert another row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&(rgHRow[2])),S_OK);

	//restart position
	hr = m_pIRowset->RestartPosition(NULL);
	
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
		goto CLEANUP;

	//get next row.  We have added 2 rows into the rowset//skip # of rows+3 in the rowset.  DB_S_ENDOFROWSET
	TESTC_(m_pIRowset->GetNextRows(NULL,m_ulTableRows+3,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
	
	fTestPass=TRUE;
CLEANUP:	
	//Release the row handle and memory
	for(cCount=0; cCount<3; cCount++)
	{
		if(rgHRow[cCount])
			CHECK(m_pIRowset->ReleaseRows(1, &(rgHRow[cCount]), NULL,NULL,NULL),S_OK);
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	if(pHRow)
	{
	   	CHECK(m_pIRowset->ReleaseRows(1,pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	ReleaseRowsetAndAccessor();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode, insert one row, change it and delete it.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowsetDelete::Variation_2()
{
	void		*rgpData[2]	= {NULL,NULL};
	ULONG		cCount		= 0;
	BOOL		fTestPass	= FALSE;
	HROW		hRow		= NULL;
	DBPROPID	rgDBPROPID[4];
	DBROWSTATUS	DBRowStatus;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;
	rgDBPROPID[2]=DBPROP_IRowsetLocate;

	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
							0,NULL,	ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));	  

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert
	for(cCount=0; cCount<2; cCount++)
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);

	//insert 
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&hRow),S_OK);

	//change the row
	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[1]),S_OK);

		//all columns including the computed columns should be ok
		for(cCount=0; cCount<m_cBinding; cCount++)
		{
//			COMPARE(*(DBSTATUS *)((DWORD)rgpData[1]+m_rgBinding[cCount].obStatus),DBSTATUS_S_OK);
			COMPARE(STATUS_BINDING(m_rgBinding[cCount],rgpData[1]),DBSTATUS_S_OK);
		}
	}
	else
	{
	   	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[1]),DB_E_NEWLYINSERTED);
	}

	// MSDASQL _ SQLServer/Access/Oracle all three behave differently here!!!
	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		if ( GetProp(DBPROP_REMOVEDELETED) )
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),S_OK);
		}
		else 
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),DB_E_ERRORSOCCURRED);
			COMPARE(DBRowStatus, DBROWSTATUS_E_DELETED);
		}
	}
	else 
	{
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),DB_E_ERRORSOCCURRED);
		COMPARE(DBRowStatus, DBROWSTATUS_E_NEWLYINSERTED);
	}

	fTestPass=TRUE;
CLEANUP:
	//Release the row handle and memory
	if(hRow)
	{
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);
	}
	for(cCount=0; cCount<2; cCount++)
	{
		if(rgpData[cCount])
		{
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
		}
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
BOOL Related_IRowsetDelete::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_IRowsetChange)
//*-----------------------------------------------------------------------
//| Test Case:		Related_IRowsetChange - test IRowsetChange and IRowsetNewRow
//|	Created:			05/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowsetChange::Init()
{
	BOOL		fTestPass=FALSE;
	DBPROPID	rgDBPROPID[2];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_OTHERUPDATEDELETE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Ichange the primary key of one row, insert a row with	the original primary key.  S_OK.  Insert another row with the new primary
// S_OK.  Insert another row with the new primary key.  DB_E_INTEGRITYVIOLATION.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowsetChange::Variation_1()
{
	DBCOUNTITEM		cRows			= 0;
	IRowsetChange	*pIRowsetChange	= NULL;
	HROW			hRow			= 1;
	HROW			*pHRow			= NULL;
	void			*pData			= NULL;
	BOOL			fTestPass		= FALSE;

	// This variation tests col uniqueness and this property is available only through non-level 0 interfaces
	// If strict conformance leveling is active, skip this variation
	if (GetModInfo()->IsStrictLeveling())
		return TEST_SKIPPED;

	//make data for the 1st row.  The rowset starts with 10th row.
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//get the 1st row handle
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//QI for IRowsetDelete
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetChange,(LPVOID *)&pIRowsetChange),S_OK);

	//delete the row handle
	TESTC_(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);

	if(GetProp(DBPROP_CANHOLDROWS))
	{
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);
		//release the row handle
		TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
		hRow=NULL;
	}
	else
		//insert the same row.  DB_E_ROWSNOTRELEASED
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),DB_E_ROWSNOTRELEASED);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);
	
	if(GetProp(DBPROP_CANHOLDROWS))
	{
		//insert again
		m_hr=m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL);
		
		if(m_bIndexExists)
		{
			TESTC_(m_hr,DB_E_INTEGRITYVIOLATION);
		}
		else
		{
			TESTC_(m_hr,S_OK);
		}
	}
	else
		//insert
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	fTestPass=TRUE;

CLEANUP:
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData, TRUE);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(hRow)
	   	CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetChange);

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc In immediate update mode, insert two rows, change one of the row.  Verify.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_IRowsetChange::Variation_2()
{
	void	*rgpData[3]={NULL,NULL,NULL};
	HROW	hRow=NULL;
	ULONG	cCount;
	DBROWSTATUS	DBRowStatus;
	BOOL	fTestPass=FALSE;


	//make data for insert && change
	for(cCount=0; cCount<3; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert 
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],NULL),S_OK);

	//insert another row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[1],&hRow),S_OK);

	//change the row
	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[2]),S_OK);
	}
	else
	{
		TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[2]),DB_E_NEWLYINSERTED);
	}

	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		if ( GetProp(DBPROP_REMOVEDELETED) )
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),S_OK);
		}
		else 
		{
			TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),DB_E_ERRORSOCCURRED);
			COMPARE(DBRowStatus, DBROWSTATUS_E_DELETED);
		}
	}
	else
	{
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,&DBRowStatus),DB_E_ERRORSOCCURRED);
		COMPARE(DBRowStatus, DBROWSTATUS_E_NEWLYINSERTED);
	}

	fTestPass=TRUE;


CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<3; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_IRowsetChange::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Transaction)
//*-----------------------------------------------------------------------
//| Test Case:		Transaction - testing zombie state
//|	Created:			05/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Transaction::Init()
{
	m_DBPropSet.rgProperties = NULL;

	if(!CTransaction::Init())                                                 
   		return TEST_SKIPPED;                                                         
                        
	m_DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	m_DBPropSet.cProperties = 0;
	m_DBPropSet.rgProperties=(DBPROP *)PROVIDER_ALLOC(3*sizeof(DBPROP));

	if(!m_DBPropSet.rgProperties)
		return FALSE;

	//Memset to zeros
	memset(m_DBPropSet.rgProperties, 0, (3*sizeof(DBPROP))); 

	//DBPROP_IRowsetChange
   	m_DBPropSet.rgProperties[0].dwPropertyID=DBPROP_IRowsetChange;
   	m_DBPropSet.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
	m_DBPropSet.rgProperties[0].colid		= DB_NULLID;
   	m_DBPropSet.rgProperties[0].vValue.vt=VT_BOOL;
   	V_BOOL(&m_DBPropSet.rgProperties[0].vValue)=VARIANT_TRUE;
	m_DBPropSet.cProperties++;

	//DBPROP_CANHOLDROWS
	m_DBPropSet.rgProperties[1].dwPropertyID=DBPROP_CANHOLDROWS;
   	m_DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;
	m_DBPropSet.rgProperties[1].colid		= DB_NULLID;
   	m_DBPropSet.rgProperties[1].vValue.vt=VT_BOOL;
   	V_BOOL(&m_DBPropSet.rgProperties[1].vValue)=VARIANT_TRUE;
	m_DBPropSet.cProperties++;

	if ( DBPROPFLAGS_WRITE & GetPropInfoFlags(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pIDBCreateSession))
	{
		//DBPROP_UPDATABILITY
		m_DBPropSet.rgProperties[2].dwPropertyID=DBPROP_UPDATABILITY;
   		m_DBPropSet.rgProperties[2].dwOptions=DBPROPOPTIONS_REQUIRED;
   		m_DBPropSet.rgProperties[2].vValue.vt=VT_I4;
		m_DBPropSet.rgProperties[2].colid		= DB_NULLID;
   		V_I4(&m_DBPropSet.rgProperties[2].vValue)=DBPROPVAL_UP_INSERT|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;
		m_DBPropSet.cProperties++;
	}

	// guarantee that we create insert seeds that fit in the transact table
	g_ulNextRow =  ( g_ulNextRow <= TRANSACTION_ROW_COUNT ) ? TRANSACTION_ROW_COUNT+1 : g_ulNextRow;

   	//register interface to be tested                                         
   	if(!RegisterInterface(ROWSET_INTERFACE, IID_IRowsetChange, 1, &m_DBPropSet)) 
   		return FALSE;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=TRUE.  Query based
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_1()
{
	BOOL			fTestPass		= FALSE;
	HACCESSOR		hAccessor		= NULL;
	void			*pSetData		= NULL;
	void			*pSetSecondData	= NULL;
	DBLENGTH		cRowSize		= 0; 
	void			*pGetData		= NULL;
	DBCOUNTITEM		cBinding		= 0;
	DBBINDING		*rgBinding		= NULL;
	IRowsetChange	*pIRowsetChange	= NULL;
	IAccessor		*pIAccessor		= NULL;
	DBCOUNTITEM		cRows			= 0; 
	HROW			hRow			= NULL;
	HROW			hSecondRow		= NULL;
	HROW			*pHRow			= NULL;
	IUnknown		*pIUnknown		= NULL;
	HRESULT			hr				= E_FAIL;


	//start a transaction.  Create a rowset with IRowsetNewRow pointer.

	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,
		1, &m_DBPropSet))
	{
		goto CLEANUP;	
	}
	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding, &cRowSize,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
							FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
							NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//Get a second data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetSecondData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pSetData,NULL),S_OK);

	//insert again.  Error should occur.
	hr = pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pSetData,&hRow);

	// This should be an error indicating that a unique key constraint has been violated.
	COMPC(FAILED(hr), TRUE);	
											    
	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto CLEANUP;

	if(!m_fCommitPreserve)
	{
		hSecondRow	= 1;
		//test zombie
		if(CHECK(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetSecondData,&hSecondRow),E_UNEXPECTED))
		{
			if(COMPARE(hSecondRow, NULL))
				fTestPass=TRUE;
			goto CLEANUP;
		}
	}
		
	//test the rowset should be fully functional
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetSecondData,&hSecondRow),S_OK);

	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto CLEANUP;

	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	PROVIDER_FREE(rgBinding);
	hAccessor=NULL;
		
	//reexcute the command
	TESTC_(m_pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIUnknown),S_OK);

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(pIUnknown,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding, &cRowSize,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
							FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
							NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //get the 1st row of the rowset
	TESTC_(((IRowset *)pIUnknown)->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//allocate memory for GetData
	if(!(pGetData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	//get data
	TESTC_(((IRowset *)pIUnknown)->GetData(*pHRow, hAccessor, pGetData),S_OK);
		
	fTestPass=TRUE;

	//the udpate should not be made as the transaction was aborted.
	//TODO Why is this commented?
//	if(!CompareBuffer(pGetData,pSetData,cBinding,rgBinding,
//		m_pIMalloc,TRUE))
//	{
//		fTestPass=TRUE;
//		PROVIDER_FREE(pSetData);
//	}	
CLEANUP:
	//release GetData buffers
	PROVIDER_FREE(pGetData);

	//release the row handle
	if(pHRow)
	{
		CHECK(((IRowset *)pIUnknown)->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	if(hAccessor)
	{
		//QI for IAccessor.  The accessor could be on the 1st rowset 
		if(pIUnknown)
		{
			if(CHECK(((IRowset *)pIUnknown)->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
			
		}
		else
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	//release the rowsets generated
	SAFE_RELEASE(pIUnknown);

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE*)pSetData, TRUE);

   	if(pSetSecondData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetSecondData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the row handles generated for the second time
	if(hSecondRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hSecondRow, NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetChange);

	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining=FALSE. Cursor based
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_2()
{
	BOOL			fTestPass		= FALSE;
	HACCESSOR		hAccessor		= NULL;
	void			*pSetData		= NULL;
	void			*pGetData		= NULL;
	DBLENGTH		cRowSize		= 0; 
	DBCOUNTITEM		cBinding		= 0;
	DBBINDING		*rgBinding		= NULL;
	IRowsetChange	*pIRowsetChange	= NULL;
	IAccessor		*pIAccessor		= NULL;
	DBCOUNTITEM		cRows			= 0; 
	HROW			hRow			= NULL;
	HROW			hSecondRow		= NULL;
	HROW			*pHRow			= NULL;
	IUnknown		*pIUnknown		= NULL;
	HRESULT			hr				= E_FAIL;


	//start a transaction.  Create a rowset with IRowsetNewRow pointer.

	//Cursor based update
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,1, &m_DBPropSet))
		goto CLEANUP;

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding, &cRowSize,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
								FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
								NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);
		
    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
						(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//insert
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pSetData,&hRow),S_OK);

	//commit the transaction with fRetaining==FALSE
	if(!GetCommit(FALSE))
		goto CLEANUP;

	if(!m_fCommitPreserve)
	{
		hSecondRow	= 1;
		//test zombie
		if(CHECK(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData,&hSecondRow),E_UNEXPECTED))
		{
			if(COMPARE(hSecondRow, NULL))
				fTestPass=TRUE;
			goto CLEANUP;
		}
	}
		
	//test the rowset should be fully functional//we try to set the same value to the 1st column
	//if(m_bIndexExists)
	//Should return constraints violation
	hr = pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData, &hSecondRow);

	// accept either HRESULT since providers differ in their
	// ability to detect which col generated the violation
	COMPC(FAILED(hr), TRUE);

	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	hAccessor=NULL;
	PROVIDER_FREE(rgBinding);
			
	//reexcute the command
	TESTC_(m_pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIUnknown),S_OK);

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(pIUnknown,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding, &cRowSize,
						DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,	
						FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
						NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//get the 1st row of the rowset
	TESTC_(((IRowset *)pIUnknown)->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//allocate memory for GetData
	if(!(pGetData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	//get data
	if(CHECK(((IRowset *)pIUnknown)->GetData(*pHRow, hAccessor, pGetData),S_OK))
		fTestPass=TRUE;

	//the udpate should not be made as the transaction was aborted.
	//TODO Why is this commented?
//	if(!CompareBuffer(pGetData,pSetData,cBinding,rgBinding,
//		m_pIMalloc,TRUE))
//	{
//		fTestPass=TRUE;
//		PROVIDER_FREE(pSetData);
//	}

	
CLEANUP:
	//release GetData buffers
	PROVIDER_FREE(pGetData);

	//release the row handle
	if(pHRow)
	{
		CHECK(((IRowset *)pIUnknown)->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	if(hAccessor)
	{
		//QI for IAccessor.  The accessor could be on the 1st rowset 
		if(pIUnknown)
		{
			if(CHECK(((IRowset *)pIUnknown)->QueryInterface(IID_IAccessor,
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

	//release the rowsets generated
	SAFE_RELEASE(pIUnknown);

	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

	//release the row handles generated for the second time
	if(hSecondRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hSecondRow, NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetChange);

	//clean up
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=TRUE.  Cursor  based.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_3()
{
	BOOL			fTestPass		= FALSE;
	HACCESSOR		hAccessor		= NULL;
	void			*pSetData		= NULL;
	void			*pGetData		= NULL;
	DBLENGTH		cRowSize		= 0; 
	DBCOUNTITEM		cBinding		= 0;
	DBBINDING		*rgBinding		= NULL;
	IRowsetChange	*pIRowsetChange	= NULL;
	IAccessor		*pIAccessor		= NULL;
	DBCOUNTITEM		cRows			= 0; 
	HROW			hRow			= NULL;
	HROW			hSecondRow		= NULL;
	HROW			*pHRow			= NULL;
	IUnknown		*pIUnknown		= NULL;
	HRESULT			hr				= E_FAIL;

	//start a transaction.  Create a rowset with IRowsetNewRow pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,1, &m_DBPropSet))
		goto CLEANUP;

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor,&rgBinding,&cBinding,&cRowSize,
						DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
						FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
						NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//insert
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pSetData,&hRow),S_OK);

	//abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto CLEANUP;

	if(!m_fAbortPreserve)
	{
		hSecondRow	= 1;
		//test zombie
		if(CHECK(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData,&hSecondRow),E_UNEXPECTED))
		{
			if(COMPARE(hSecondRow, NULL))
				fTestPass=TRUE;
			goto CLEANUP;
		}
	}
		
	//test the rowset should be fully functional//we try to set the same value to the 1st column.  Should be 
	//fine as the transaction was aborted
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData,&hSecondRow),S_OK);

	//abort the transaction again
	if(!GetAbort(TRUE))
		goto CLEANUP;

	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	hAccessor=NULL;
	PROVIDER_FREE(rgBinding);
	
	//reexcute the command
	TESTC_(m_pICommand->Execute(NULL,IID_IRowset,NULL,NULL,	&pIUnknown),S_OK);

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(pIUnknown,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding,&cRowSize,
						DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
						FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
						NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //make sure no row is actually inserted get the 1st row
	TESTC_(((IRowset *)pIUnknown)->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//allocate memory for GetData
	if(!(pGetData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	//get data
	if(CHECK(((IRowset *)pIUnknown)->GetData(*pHRow, hAccessor, pGetData),S_OK))
		fTestPass=TRUE;

	//the udpate should not be made as the transaction was aborted.
	//TODO Why is this commented?
//	if(!CompareBuffer(pGetData,pSetData,cBinding,rgBinding,
//		m_pIMalloc,TRUE))
//	{
//		fTestPass=TRUE;
//		PROVIDER_FREE(pSetData);
//	}

CLEANUP:
	//release GetData buffers
	PROVIDER_FREE(pGetData);

	//release the row handle
	if(pHRow)
	{
		CHECK(((IRowset *)pIUnknown)->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	if(hAccessor)
	{
		//QI for IAccessor.  The accessor could be on the 1st rowset 
		if(pIUnknown)
		{
			if(CHECK(((IRowset *)pIUnknown)->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
			
		}
		else
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	//release the rowsets generated
	SAFE_RELEASE(pIUnknown);
	
	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

   	//release the row handle on the 1st rowset
	if(hSecondRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hSecondRow, NULL,NULL,NULL),S_OK);

	SAFE_RELEASE(pIRowsetChange);

	//clean up
	CleanUpTransaction(S_OK);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining=FALSE.  Query based
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_4()
{
	BOOL			fTestPass		= FALSE;
	HACCESSOR		hAccessor		= NULL;
	void			*pSetData		= NULL;
	void			*pGetData		= NULL;
	DBLENGTH		cRowSize		= 0; 
	DBCOUNTITEM		cBinding		= 0;
	DBBINDING		*rgBinding		= NULL;
	IRowsetChange	*pIRowsetChange	= NULL;
	IAccessor		*pIAccessor		= NULL;
	DBCOUNTITEM		cRows			= 0; 
	HROW			hRow			= NULL;
	HROW			hSecondRow		= NULL;
	HROW			*pHRow			= NULL;
	IUnknown		*pIUnknown		= NULL;

	//start a transaction.  Create a rowset with IRowsetNewRow pointer.

	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetChange,1, &m_DBPropSet))
		goto CLEANUP;

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding,&cBinding,&cRowSize,
						DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
						FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
						NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);

    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//Get data to set
	TESTC_(FillInputBindings(m_pCTable,DBACCESSOR_ROWDATA,cBinding,rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
	
	//insert
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,hAccessor,pSetData,&hRow),S_OK);

	//abort the transaction with fRetaining==FALSE
	if(!GetAbort(FALSE))
		goto CLEANUP;

	if(!m_fAbortPreserve)
	{
		hSecondRow	= 1;
		//test zombie
		if(CHECK(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData,&hSecondRow),E_UNEXPECTED))
		{
			if(COMPARE(hSecondRow, NULL))
				fTestPass=TRUE;
			goto CLEANUP;
		}
	}
		
	//test the rowset should be fully functional
	//we try to set the same value to the 1st column.  Should be 
	//fine as the transaction was aborted
	TESTC_(pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pSetData,&hSecondRow),S_OK);

	//release the accessor
	TESTC_(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
	hAccessor=NULL;

	//reexcute the command
	TESTC_(m_pICommand->Execute(NULL,IID_IRowset,NULL,NULL,&pIUnknown),S_OK);

	//create an accessor on the rowset object 
	TESTC_(GetAccessorAndBindings(pIUnknown,DBACCESSOR_ROWDATA,&hAccessor, &rgBinding, &cBinding,&cRowSize,
								DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND,
								FORWARD,NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL,
								NULL,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);


    //if there are no updatable rows
    if (!cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    //make sure one row is actually inserted as fRetaining is FALSE,we are in autocommite mode
	TESTC_(((IRowset *)pIUnknown)->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//allocate memory for GetData
	if(!(pGetData=PROVIDER_ALLOC(cRowSize)))
		goto CLEANUP;

	//get data
	if(CHECK(((IRowset *)pIUnknown)->GetData(*pHRow, hAccessor, pGetData),S_OK))
		fTestPass=TRUE;

	//the udpate should not be made as the transaction was aborted.
	//TODO Why is this commented?
//	if(COMPARE(CompareBuffer(pGetData,pSetData,cBinding,rgBinding,
//		m_pIMalloc,TRUE),TRUE))
//	{
//		fTestPass=TRUE;
//		PROVIDER_FREE(pSetData);
//	}

CLEANUP:
	//release GetData buffers
	PROVIDER_FREE(pGetData);

	//release the row handle
	if(pHRow)
	{
		CHECK(((IRowset *)pIUnknown)->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	//release the accessor
	if(hAccessor)
	{
		//QI for IAccessor.  The accessor could be on the 1st rowset 
		if(pIUnknown)
		{
			if(CHECK(((IRowset *)pIUnknown)->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
			
		}
		else
			if(CHECK(pIRowsetChange->QueryInterface(IID_IAccessor,(LPVOID *)&pIAccessor),S_OK))
			{
				CHECK(pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);
				SAFE_RELEASE(pIAccessor);
			}
	}

	SAFE_RELEASE(pIUnknown);
	
	//release SetData buffers
	if(pSetData)
		ReleaseInputBindingsMemory(cBinding, rgBinding, (BYTE *)pSetData, TRUE);

	//release the binding structure
	PROVIDER_FREE(rgBinding);

	//release the row handle on the 1st rowset
	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);

   	//release the row handle on the 1st rowset
	if(hSecondRow)
		CHECK(m_pIRowset->ReleaseRows(1, &hSecondRow, NULL,NULL,NULL),S_OK);

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
	PROVIDER_FREE(m_DBPropSet.rgProperties);
	return(CTransaction::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(EmptyTable)
//*-----------------------------------------------------------------------
//| Test Case:		EmptyTable - insert a new row into an empty table
//|	Created:			07/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL EmptyTable::Init()
{
	BOOL fTestPass = FALSE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	g_p1RowTable->DeleteRows();

	//set the table.  No need to create a table everytime.
	SetTable(g_p1RowTable,DELETETABLE_NO);	
	fTestPass = TRUE;

	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row into an empty table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyTable::Variation_1()
{
	DBCOUNTITEM	cRows		= 0;
	HROW		*pHRow		= NULL;
	void		*pData		= NULL;
	DBPROPID	rgDBPropID[2];
	BOOL		fTestPass	= FALSE;

	HRESULT hr = S_OK;
	
	rgDBPropID[0]=DBPROP_IRowsetChange;
	rgDBPropID[1]=DBPROP_OTHERINSERT;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								
	
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//fill up buffer for a row
	TESTC_(FillInputBindings(g_p1RowTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//InsertRow should succeed
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK);

	//restartposition
	hr = m_pIRowset->RestartPosition(NULL);
	if(!CHECK(hr==S_OK || hr==DB_S_COMMANDREEXECUTED, TRUE))	
		goto CLEANUP;

	//GetNextRows
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK);
	
	//make sure GetData should be able to see the change
	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE)==TRUE)
		fTestPass=TRUE;

CLEANUP:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	PROVIDER_FREE(pData);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL EmptyTable::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(AppendOnly)
//*-----------------------------------------------------------------------
//| Test Case:		AppendOnly - test DBPROP_APPENDONLY
//|	Created:			08/06/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL AppendOnly::Init()
{
	BOOL		fTestPass	= TEST_SKIPPED;
	DBPROPID	rgDBPropID[1];

	rgDBPropID[0]=DBPROP_APPENDONLY;

	if(!TCIRowsetNewRow::Init())
	{
		return TEST_FAIL;
	}
	if (!SupportedProperty(DBPROP_APPENDONLY, DBPROPSET_ROWSET))
	{
		return TEST_SKIPPED;
	}
	//create a rowset
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,0,NULL,NO_ACCESSOR));

	fTestPass = TRUE;
CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc check props with APPENDONLY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_1()
{
	ULONG	cRows=2;
	HROW	*pHRow=NULL;
	DBPROPID rgDBPropID[1];
	BOOL	fTestPass=TRUE;

	rgDBPropID[0]=DBPROP_APPENDONLY;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

	//make sure DBPROP_APPENDONLY is variant_true
	if(!GetProp(DBPROP_APPENDONLY))
		goto CLEANUP;
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass=TEST_FAIL;
	
	//check the implied properties
	if(!GetProp(DBPROP_IRowsetChange))
		goto CLEANUP;

	if(!GetProp(DBPROP_OWNINSERT))
		goto CLEANUP;

	if(GetProp(DBPROP_OTHERINSERT))
		goto CLEANUP;

	if(!CheckProp(DBPROPVAL_UP_INSERT))
		goto CLEANUP;

	//insert should fail
	if(CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,NULL,NULL,NULL),DB_E_BADACCESSORHANDLE))
		fTestPass=TEST_PASS;
CLEANUP:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc AppendOnly conflict with DBPROP_CANSCROLLBACKWARDS, and OTHERINSERT.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_2()
{
	ICommandProperties	*pICommandProperties=NULL;
	DBPROPSET			DBPropSet;
	DBPROP				rgDBProp[5];
	BOOL				fTestPass=TEST_FAIL;
	//ULONG				cCount;

	DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropSet.cProperties=5;
	DBPropSet.rgProperties=rgDBProp;

	rgDBProp[0].dwPropertyID=DBPROP_APPENDONLY;
	rgDBProp[0].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBProp[0].colid		= DB_NULLID;
	rgDBProp[0].vValue.vt=VT_BOOL;
	V_BOOL(&rgDBProp[0].vValue)=VARIANT_TRUE;

	rgDBProp[1].dwPropertyID=DBPROP_IRowsetChange;
	rgDBProp[1].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBProp[1].colid		= DB_NULLID;
	rgDBProp[1].vValue.vt=VT_BOOL;
	V_BOOL(&rgDBProp[1].vValue)=VARIANT_TRUE;

	rgDBProp[2].dwPropertyID=DBPROP_OTHERINSERT;
	rgDBProp[2].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBProp[2].colid		= DB_NULLID;
	rgDBProp[2].vValue.vt=VT_BOOL;
	V_BOOL(&rgDBProp[2].vValue)=VARIANT_TRUE;

	rgDBProp[3].dwPropertyID=DBPROP_CANFETCHBACKWARDS;
	rgDBProp[3].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBProp[3].colid		= DB_NULLID;
	rgDBProp[3].vValue.vt=VT_BOOL;
	V_BOOL(&rgDBProp[3].vValue)=VARIANT_TRUE;

   	rgDBProp[4].dwPropertyID=DBPROP_CANSCROLLBACKWARDS;
	rgDBProp[4].dwOptions=DBPROPOPTIONS_REQUIRED;
	rgDBProp[4].colid		= DB_NULLID;
	rgDBProp[4].vValue.vt=VT_BOOL;
	V_BOOL(&rgDBProp[4].vValue)=VARIANT_TRUE;

	if(!SUCCEEDED(SetRowsetProperties(&DBPropSet, 1)))
		goto CLEANUP;

	//create the rowset object
	CreateRowsetObject(SELECT_ALLFROMTBL,IID_IRowset,EXECUTE_IFNOERROR);
		
	fTestPass=TEST_PASS;	
	//conflicting
//none fo the above a level zero strict i believe
//	for(cCount=0; cCount<5; cCount++)
//	{
//		if(cCount==1)
//		{
//			if(!COMPARE((DBPropSet.rgProperties)[cCount].dwStatus, DBPROPSTATUS_OK))
//				goto CLEANUP;
//		}
//		else
//		{
//			if(!COMPARE((DBPropSet.rgProperties)[cCount].dwStatus, DBPROPSTATUS_CONFLICTING))
//				goto CLEANUP;
//		}
//	}
CLEANUP:
	SAFE_RELEASE(pICommandProperties);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc the rowset is empty.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_3()
{
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	DBPROPID	rgDBPropID[1];
	BOOL		fTestPass		= TEST_SKIPPED;

	rgDBPropID[0]=DBPROP_APPENDONLY;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    fTestPass = TEST_FAIL;

	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	//get next rows should return DB_E_BADSTARTPOSITION
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
	
	//no row should be retrieved
	if(!COMPARE(cRows, 0))
	{
		goto CLEANUP;
	}
	
	fTestPass=TEST_PASS;
CLEANUP:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc REFRESH/RESYNC, should not bring back old rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_4()
{
	ULONG			cRows				= 0;
	HROW			HRow				= NULL;
	DBPROPID		rgDBPropID[3];
	BOOL			fTestPass			= TEST_SKIPPED;
	void			*pData				= NULL;
	IRowsetResynch	*pIRowsetResynch	= NULL;
	DBCOUNTITEM		cRowsResynched		= -1;
	HROW			*rghRowsResynched	= NULL;


	rgDBPropID[0]=DBPROP_APPENDONLY;
	rgDBPropID[2]=DBPROP_IRowsetChange;
	rgDBPropID[1]=DBPROP_IRowsetResynch;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = TEST_FAIL;

	//get the resynch interface
	if(!VerifyInterface(m_pIRowset, IID_IRowsetResynch,ROWSET_INTERFACE,(IUnknown **)&pIRowsetResynch))
	{
		goto CLEANUP;
	}

	//resync all the rows, since this rowset only show just inserted rows this should come back empty
	TESTC_(pIRowsetResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	
	//no row should be retrieved
	if(!COMPARE(cRowsResynched, 0))
	{
		goto CLEANUP;
	}

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&HRow),S_OK))
	{
		goto CLEANUP;
	}

	//resync all the rows, since this rowset only show just inserted rows this should come back 1
	TESTC_(pIRowsetResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	
	//no row should be retrieved
	if(!COMPARE(cRowsResynched, 1))
	{
		goto CLEANUP;
	}
	fTestPass=TRUE;
CLEANUP:
	//release SetData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	SAFE_RELEASE(pIRowsetResynch);

	//release the row handle
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc RestartPosition/Get fetches only newlyinserted rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_5()
{
	DBCOUNTITEM		cRows				= 0;
	DBPROPID		rgDBPropID[1];
	BOOL			fTestPass			= TEST_SKIPPED;
	void			*pData				= NULL;
	void			*pData1				= NULL;
	HROW			*pHRow				= NULL;


	rgDBPropID[0]=DBPROP_APPENDONLY;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = TEST_FAIL;

	//insert a row at the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

	//position at the start
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	//get next rows should return the row just inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//one row should be retrieved
	if(!COMPARE(cRows, 1))
	{
		goto CLEANUP;
	}

	//GetData
	//get the data from the row
	SAFE_ALLOC(pData1, BYTE, m_cRowSize);
	memset(pData1, 0, (size_t)m_cRowSize);
	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

	//should be able to see the new row first, data buffers should match
	if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
	{
		goto CLEANUP;
	}
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//get next rows should now being moving off the rowsert
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);

	fTestPass = TEST_PASS;
CLEANUP:
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	PROVIDER_FREE(pData1);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc Change newly inserted row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_6()
{
	DBCOUNTITEM	cRows			= 0;
	HROW		HRow			= NULL;
	DBPROPID	rgDBPropID[1];
	BOOL		fTestPass		= TEST_SKIPPED;
	void		*pData			= NULL;
	void		*pData1			= NULL;
	void		*pData2			= NULL;
	BOOL		fChangeRow		= FALSE;
	HROW		hRow			= NULL;
	HROW		*pHRow			= NULL;


	rgDBPropID[0]=DBPROP_APPENDONLY;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(USE_SUPPORTED_SELECT_ALLFROMTBL,1,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = TEST_FAIL;

	//get value of DBPROP_CHANGEINSERTEDROWS for this rowset
	if(GetProp(DBPROP_CHANGEINSERTEDROWS))
	{
		fChangeRow = TRUE;
	}
	else
	{
		fChangeRow = FALSE;
	}
		
	//make a data buffer
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&HRow),S_OK))
	{
		goto CLEANUP;
	}
	
	//make a new buffer
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData1,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	if(fChangeRow)
	{
		//change the row with the new buffer	
		TESTC_(m_pIRowsetChange->SetData(HRow,m_hAccessor,pData1),S_OK);
	}
	else
	{
	   	TESTC_(m_pIRowsetChange->SetData(HRow,m_hAccessor,pData1),DB_E_NEWLYINSERTED);
	}

	//release the row handle
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);

	//re-position at the start
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	//get next rows should return the row just inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

	//one row should be retrieved
	if(!COMPARE(cRows, 1))
	{
		goto CLEANUP;
	}

	//GetData
	//get the data from the row
	SAFE_ALLOC(pData2, BYTE, m_cRowSize);
	memset(pData2, 0, (size_t)m_cRowSize);
	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData2),S_OK);
	
	if(fChangeRow)
	{
		//should be able to see the new row, data bufferes should match
		if(!CompareBuffer(pData2,pData1,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		{
			goto CLEANUP;
		}
		//delete the row
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);
	}
	else
	{
		//should be able to see the new row, data bufferes should match
		if(!CompareBuffer(pData2,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		{
			goto CLEANUP;
		}
		TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),DB_E_NEWLYINSERTED);
	}
	fTestPass	 = TRUE;
CLEANUP:
	//release SetData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);

	//release the row handle
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc update pending,RestartPosition/Get fetches only newlyinserted rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_7()
{
	DBCOUNTITEM		cRows				= 0;
	DBPROPID		rgDBPropID[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	void			*pData				= NULL;
	void			*pData1				= NULL;
	HROW			*pHRow				= NULL;
	BOOL			fReturnPendingInsert= FALSE;


	rgDBPropID[0]=DBPROP_APPENDONLY;
	rgDBPropID[1]=DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	fTestPass = TEST_FAIL;

	//get value of DBPROP_RETURNPENDINGINSERTS for this rowset
	if(GetProp(DBPROP_RETURNPENDINGINSERTS))
	{
		fReturnPendingInsert = TRUE;
	}
	else
	{
		fReturnPendingInsert = FALSE;
	}

	//goto the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

	//position at the start
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	if (fReturnPendingInsert)
	{
		//get next rows should return the row just inserted
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		//one row should be retrieved
		if(!COMPARE(cRows, 1))
		{
			goto CLEANUP;
		}
		//GetData
		//get the data from the row
		SAFE_ALLOC(pData1, BYTE, m_cRowSize);
		memset(pData1, 0, (size_t)m_cRowSize);
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

		//should be able to see the new row first, data buffers should match
		if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		{
			goto CLEANUP;
		}
		PROVIDER_FREE(pData1);
	}
	else
	{
		//get next rows should return the row just inserted
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
		//one row should be retrieved
		if(!COMPARE(cRows, 0))
		{
			goto CLEANUP;
		}
	}
	
	//update (all pendig updates) the insert to the back end
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	//free the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	//restart
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));	
	
	//get next rows should return the row just inserted
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
	//one row should be retrieved
	if(!COMPARE(cRows, 1))
	{
		goto CLEANUP;
	}
	
	//GetData
	//get the data from the row
	SAFE_ALLOC(pData1, BYTE, m_cRowSize);
	memset(pData1, 0, (size_t)m_cRowSize);
	TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

	//should be able to see the new row first, data buffers should match
	if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
	{
		goto CLEANUP;
	}
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	//get next rows should now being moving off the rowsert
	TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);

	fTestPass = TEST_PASS;
CLEANUP:
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	PROVIDER_FREE(pData1);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc update pending,REFRESH/RESYNC, should not bring back old rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_8()
{
	ULONG			cRows				= 0;
	HROW			HRow				= NULL;
	DBPROPID		rgDBPropID[2];
	BOOL			fTestPass			= TEST_SKIPPED;
	void			*pData				= NULL;
	IRowsetResynch	*pIRowsetResynch	= NULL;
	IRowsetRefresh	*pIRowsetRefresh	= NULL;
	DBCOUNTITEM		cRowsResynched		= -1;
	HROW			*rghRowsResynched	= NULL;


	rgDBPropID[0]=DBPROP_APPENDONLY;
	rgDBPropID[1]=DBPROP_IRowsetUpdate;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
							DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));								

	//get the resynch interface or the refresh interface
	if(!VerifyInterface(m_pIRowset, IID_IRowsetResynch,ROWSET_INTERFACE,(IUnknown **)&pIRowsetResynch))
	{
		if(!VerifyInterface(m_pIRowset, IID_IRowsetRefresh,ROWSET_INTERFACE,(IUnknown **)&pIRowsetRefresh))
		{
			goto CLEANUP;
		}
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	fTestPass = TEST_FAIL;

	if (pIRowsetResynch)
	{
		//resync all the rows, since this rowset only show just inserted rows this should come back empty
		TESTC_(pIRowsetResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	}
	else
	{
		//refresh all the rows, since this rowset only show just inserted rows this should come back empty
		TESTC_(pIRowsetRefresh->RefreshVisibleData(0, 0, NULL, TRUE, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	}

	//no row should be retrieved
	if(!COMPARE(cRowsResynched, 0))
	{
		goto CLEANUP;
	}

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&HRow),S_OK))
	{
		goto CLEANUP;
	}

	if (pIRowsetResynch)
	{
		//resync all the rows, since this rowset only show just inserted rows this should come back 1
		TESTC_(pIRowsetResynch->ResynchRows(0, NULL, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	}
	else
	{
		//refresh all the rows, since this rowset only show just inserted rows this should come back 1
		TESTC_(pIRowsetRefresh->RefreshVisibleData(0, 0, NULL, TRUE, &cRowsResynched, &rghRowsResynched, NULL),S_OK);
	}
	
	//no row should be retrieved
	if(!COMPARE(cRowsResynched, 1))
	{
		goto CLEANUP;
	}
	//update (all pending updates) the insert to the back end
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	fTestPass=TRUE;
CLEANUP:
	//release SetData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	SAFE_RELEASE(pIRowsetResynch);
	SAFE_RELEASE(pIRowsetRefresh);

	//release the row handle
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, &HRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc update pending,Commit.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_9()
{
	ITransactionLocal	*pITransactionLocal	= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	HROW				*pHRow				= NULL;
	DBCOUNTITEM			cRows				= 0;
	ULONG				ulTransactionLevel	= 0;
	BOOL				fCommitPerserve		= FALSE;
	DBPROPID			rgDBPropID[2];
	void				*pData				= NULL;
	void				*pData1				= NULL;
	BOOL				fReturnPendingInsert= FALSE;


	rgDBPropID[0]	= DBPROP_APPENDONLY;
	rgDBPropID[1]	= DBPROP_IRowsetUpdate;

	//Get ITransactionLocal on test session
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		odtLog << L"ITransactionLocal not supported.\n";
		return TEST_SKIPPED;
	}
	
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, &ulTransactionLevel),S_OK);	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	
	fTestPass=TEST_FAIL;

	//get value of DBPROP_COMMITPERSERVE for this rowset
	if(GetProp(DBPROP_COMMITPRESERVE))
	{
		fCommitPerserve = TRUE;
	}
	else
	{
		fCommitPerserve = FALSE;
	}

	//get value of DBPROP_RETURNPENDINGINSERTS for this rowset
	if(GetProp(DBPROP_RETURNPENDINGINSERTS))
	{
		fReturnPendingInsert = TRUE;
	}
	else
	{
		fReturnPendingInsert = FALSE;
	}

	//insert a row at the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

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
						odtLog << L"Non-Retaining Commit not supported, returning TEST_SKIPPED. \n";
						fTestPass = TEST_SKIPPED;
						goto CLEANUP;
					}
				}
			}
		}
	}	

	if (fCommitPerserve)
	{
		if (fReturnPendingInsert)
		{
			//get next rows should return the row just inserted
			TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
			//one row should be retrieved
			if(!COMPARE(cRows, 1))
			{
				goto CLEANUP;
			}
			//GetData
			//get the data from the row
			SAFE_ALLOC(pData1, BYTE, m_cRowSize);
			memset(pData1, 0, (size_t)m_cRowSize);
			TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

			//should be able to see the new row first, data buffers should match
			if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
			{
				goto CLEANUP;
			}
			PROVIDER_FREE(pData1);
		}
		else
		{
			//get next rows should return the row just inserted
			TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
			//one row should be retrieved
			if(!COMPARE(cRows, 0))
			{
				goto CLEANUP;
			}
		}
		
		//update (all pendig updates) the insert to the back end
		TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

		//free the row handle
		if(pHRow)
		{
			CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
			PROVIDER_FREE(pHRow);
			pHRow=NULL;
		}

		//restart
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));	
		
		//get next rows should return the row just inserted
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);
		//one row should be retrieved
		if(!COMPARE(cRows, 1))
		{
			goto CLEANUP;
		}
		
		//GetData
		//get the data from the row
		SAFE_ALLOC(pData1, BYTE, m_cRowSize);
		memset(pData1, 0, (size_t)m_cRowSize);
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

		//should be able to see the new row first, data buffers should match
		if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		{
			goto CLEANUP;
		}
		//release the row handle
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//get next rows should now being moving off the rowsert
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);
	}	
	fTestPass=TEST_PASS;	 
CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	PROVIDER_FREE(pData1);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc update pending,Abort.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_10()
{
	ITransactionLocal	*pITransactionLocal	= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	HROW				*pHRow				= NULL;
	DBCOUNTITEM			cRows				= 0;
	ULONG				ulTransactionLevel	= 0;
	BOOL				fAbortPerserve		= FALSE;
	DBPROPID			rgDBPropID[2];
	void				*pData				= NULL;


	rgDBPropID[0]	= DBPROP_APPENDONLY;
	rgDBPropID[1]	= DBPROP_IRowsetUpdate;

	//Get ITransactionLocal on test session
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		odtLog << L"ITransactionLocal not supported.\n";
		return TEST_SKIPPED;
	}
	
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, &ulTransactionLevel),S_OK);	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
	
	fTestPass=TEST_FAIL;

	//get value of DBPROP_ABORTPERSERVE for this rowset
	if(GetProp(DBPROP_ABORTPRESERVE))
	{
		fAbortPerserve = TRUE;
	}
	else
	{
		fAbortPerserve = FALSE;
	}

	//insert a row at the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

	//update (all pending) the insert to the back end
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	//abort the session
	if(!COMPARE(pITransactionLocal->Abort(NULL, FALSE, FALSE),S_OK))
	{
		//abort not retaining not supported
		odtLog << L"Non-Retaining Abort not supported, returning TEST_SKIPPED. \n";
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	

	if (fAbortPerserve)
	{
		//position at the start
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

		//get next rows should not see end-of-rowset, the insert was aborted but the buffer should be there
		if(CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET))
		{
			goto CLEANUP;	 
		}

		//get the data, pending inserts that are aborted should look like a deleted row
		TESTC_(m_pIRowsetUpdate->GetOriginalData(*pHRow,m_hAccessor,pData),DB_E_DELETEDROW);

		//one row should be retrieved
		if(!COMPARE(cRows, 0))
		{
			goto CLEANUP;
		}
	}	
	fTestPass=TEST_PASS;	 
CLEANUP:
	//abort just in case
	pITransactionLocal->Abort(NULL, FALSE, FALSE);
	SAFE_RELEASE(pITransactionLocal);
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc Commit.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_11()
{
	ITransactionLocal	*pITransactionLocal	= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	HROW				*pHRow				= NULL;
	DBCOUNTITEM			cRows				= 0;
	ULONG				ulTransactionLevel	= 0;
	BOOL				fCommitPerserve		= FALSE;
	DBPROPID			rgDBPropID[1];
	void				*pData				= NULL;
	void				*pData1				= NULL;


	rgDBPropID[0]	= DBPROP_APPENDONLY;

	//Get ITransactionLocal on test session
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		odtLog << L"ITransactionLocal not supported.\n";
		return TEST_SKIPPED;
	}
	
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, &ulTransactionLevel),S_OK);	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));


	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	
	fTestPass=TEST_FAIL;

	//get value of DBPROP_COMMITPERSERVE for this rowset
	if(GetProp(DBPROP_COMMITPRESERVE))
	{
		fCommitPerserve = TRUE;
	}
	else
	{
		fCommitPerserve = FALSE;
	}

	//insert a row at the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

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
						odtLog << L"Non-Retaining Commit not supported, returning TEST_SKIPPED. \n";
						fTestPass = TEST_SKIPPED;
						goto CLEANUP;
					}
				}
			}
		}
	}	

	if (fCommitPerserve)
	{
		//position at the start
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

		//get next rows should return the row just inserted
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

		//one row should be retrieved
		if(!COMPARE(cRows, 1))
		{
			goto CLEANUP;
		}

		//GetData
		//get the data from the row
		SAFE_ALLOC(pData1, BYTE, m_cRowSize);
		memset(pData1, 0, (size_t)m_cRowSize);
		TESTC_(m_pIRowset->GetData(*pHRow,m_hAccessor,pData1),S_OK);

		//should be able to see the new row first, data buffers should match
		if(!CompareBuffer(pData1,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		{
			goto CLEANUP;
		}
		//release the row handle
		TESTC_(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);

		//get next rows should now being moving off the rowsert
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),DB_S_ENDOFROWSET);		
	}	
	fTestPass=TEST_PASS;	 
CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	PROVIDER_FREE(pData1);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// {{ TCW_VAR_PROTOTYPE()
//*-----------------------------------------------------------------------
// @mfunc Abort.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AppendOnly::Variation_12()
{
	ITransactionLocal	*pITransactionLocal	= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	HROW				*pHRow				= NULL;
	DBCOUNTITEM			cRows				= 0;
	ULONG				ulTransactionLevel	= 0;
	BOOL				fAbortPerserve		= FALSE;
	DBPROPID			rgDBPropID[2];
	void				*pData				= NULL;


	rgDBPropID[0]	= DBPROP_APPENDONLY;
	rgDBPropID[1]	= DBPROP_RETURNPENDINGINSERTS;

	//Get ITransactionLocal on test session
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown **)&pITransactionLocal))
	{
		odtLog << L"ITransactionLocal not supported.\n";
		return TEST_SKIPPED;
	}
	
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,0, NULL, &ulTransactionLevel),S_OK);	

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPropID,0,NULL,ON_ROWSET_ACCESSOR,	FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND));


	if (!m_pIRowsetChange)
	{
		goto CLEANUP;
	}
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	
	fTestPass=TEST_FAIL;

	//get value of DBPROP_ABORTPERSERVE for this rowset
	if(GetProp(DBPROP_ABORTPRESERVE))
	{
		fAbortPerserve = TRUE;
	}
	else
	{
		fAbortPerserve = FALSE;
	}

	//insert a row at the begining of the rowset
	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	if(!CHECK(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK))
	{
		goto CLEANUP;	 
	}

	//abort the session
	if(!COMPARE(pITransactionLocal->Abort(NULL, FALSE, FALSE),S_OK))
	{
		//abort not retaining not supported
		odtLog << L"Non-Retaining Abort not supported, returning TEST_SKIPPED. \n";
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	

	if (fAbortPerserve)
	{
		//position at the start
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

		//get next rows should the pending inserted row, abort only affects the back end
		TESTC_(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK);

		//one row should be retrieved
		if(!COMPARE(cRows, 1))
		{
			goto CLEANUP;
		}
	}	
	fTestPass=TEST_PASS;	 
CLEANUP:
	pITransactionLocal->Abort(NULL, FALSE, FALSE);
	SAFE_RELEASE(pITransactionLocal);
	//release pData buffers
	if(pData)
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	//release the row handle
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
		pHRow=NULL;
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL AppendOnly::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(BMK_Static_Buffered_OwnInsert)
//*-----------------------------------------------------------------------
//| Test Case:		BMK_Static_Buffered_OwnInsert - Test static cursors capable of seeing their own inserts
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BMK_Static_Buffered_OwnInsert::Init()
{ 
	DBPROPID	rgDBPROPID[4];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;	
	rgDBPROPID[3]=DBPROP_OWNINSERT;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test visibility of newly inserted row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int BMK_Static_Buffered_OwnInsert::Variation_1()
{ 
	void		*pData=NULL;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	HROW		hRow=DB_NULL_HROW;
	BOOL		fTestPass=FALSE;

	//make data for insert. Insert the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//insert a new row.  
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	// Check that the newly inserted row is visible
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWVISIBLE));
	
CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL BMK_Static_Buffered_OwnInsert::Terminate()
{ 
	ReleaseRowsetAndAccessor();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Buffered_ReturnPendingInsert)
//*-----------------------------------------------------------------------
//| Test Case:		Buffered_ReturnPendingInsert - Check ReturnPendingInsert property
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Buffered_ReturnPendingInsert::Init()
{ 
	DBPROPID	rgDBPROPID[3];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;	
	rgDBPROPID[2]=DBPROP_RETURNPENDINGINSERTS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Soft Insert, check visibility
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ReturnPendingInsert::Variation_1()
{ 
	void		*pData=NULL;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	HROW		hRow=DB_NULL_HROW;
	BOOL		fTestPass=FALSE;

	//make data for insert. Insert the first row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	//soft insert a new row.  
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));

	// Check that the newly inserted row is visible
	// Should be visible because DBPROP_RETURNPENDINGINSERTS was requested
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWVISIBLE));

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);

	TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
	// Check that visibility of transmitted row
	if(GetProperty(DBPROP_OWNINSERT, DBPROPSET_ROWSET, m_pIRowset, VARIANT_TRUE))
	{
		TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWVISIBLE));
	}
	else
	{
		TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWNOTVISIBLE));
	}
	
CLEANUP:				
	PROVIDER_FREE(pData);

	if(hRow)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL), S_OK);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Buffered_ReturnPendingInsert::Terminate()
{ 
	ReleaseRowsetAndAccessor();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Immediate_ServerDataOnInsert)
//*-----------------------------------------------------------------------
//| Test Case:		Immediate_ServerDataOnInsert - Test retrieval of bookmarks on newly inserted rows
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Immediate_ServerDataOnInsert::Init()
{ 
	DBPROPID	rgDBPROPID[5];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;	
	rgDBPROPID[2]=DBPROP_SERVERDATAONINSERT;
	rgDBPROPID[3]=DBPROP_OWNINSERT;
	rgDBPROPID[4]=DBPROP_CANHOLDROWS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test visibility of bookmark
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Immediate_ServerDataOnInsert::Variation_1()
{ 
	void		*pData=NULL;
	DBBKMARK	cbBookmark;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pLastBookmark=(BYTE *)&DBBookmark;
	BYTE		*pBookmark=NULL;
	HROW		hRowNew=NULL;
	HROW		*pNewHRow=&hRowNew;
	HROW		HRow=NULL;
	BOOL		fTestPass=FALSE;

	//make data for insert. Insert the the seond row row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,pNewHRow),S_OK);
	
	//Get a bookmark - must be visible since SERVERDATAONINSERT is on.
	TESTC_(GetBookmarkByRow(hRowNew, &cbBookmark, &pBookmark),S_OK);	
	
	//Get the row by IRowsetLocate::GetRowsByBookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
						  
	//GetData
	TESTC_(m_pIRowset->GetData(HRow,m_hAccessor,m_pData),S_OK);
	
	//should be able to see the changes
	if(!CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		goto CLEANUP;

	//IsSameRow should return S_OK
	if(StrongIdentity())
		TESTC_(m_pIRowsetIdentity->IsSameRow(HRow, hRowNew),S_OK);
	
	fTestPass=TRUE;

CLEANUP:
	//release the bookmark
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pData);

	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL,NULL,NULL), S_OK);
	
	if(hRowNew)
		CHECK(m_pIRowset->ReleaseRows(1,&hRowNew, NULL,NULL,NULL), S_OK);

	return fTestPass;	
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Immediate_ServerDataOnInsert::Terminate()
{ 
	ReleaseRowsetAndAccessor();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Buffered_ServerDataOnInsert)
//*-----------------------------------------------------------------------
//| Test Case:		Buffered_ServerDataOnInsert - Test retrieval of bookmarks on newly inserted rows
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Buffered_ServerDataOnInsert::Init()
{ 
	DBPROPID	rgDBPROPID[6];
	BOOL		fTestPass=TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetLocate;	
	rgDBPROPID[2]=DBPROP_IRowsetUpdate;	
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;
	rgDBPROPID[4]=DBPROP_SERVERDATAONINSERT;
	rgDBPROPID[5]=DBPROP_OWNINSERT;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
		0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test visibility of bookmark
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ServerDataOnInsert::Variation_1()
{ 
	void		*pData		= NULL;
	DBBKMARK	cbBookmark	= 0;
	BYTE		*pBookmark	= NULL;
	HROW		HRow		= NULL;
	HROW		hRowNew		= NULL;
	BOOL		fTestPass	= FALSE;

	//make data for insert. Insert the the last row
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert a new row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRowNew),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRowNew,NULL,NULL,NULL),S_OK);

	// Get a bookmark 
	TESTC_(GetBookmarkByRow(hRowNew, &cbBookmark, &pBookmark),S_OK);

	//Get the row by IRowsetLocate::GetRowsByBookmark
	TESTC_(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,(const BYTE **)&pBookmark,&HRow,NULL),S_OK);
					  
	//GetData
	TESTC_(m_pIRowset->GetData(HRow,m_hAccessor,m_pData),S_OK);
	
	//should be able to see the changes
	if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding, m_pIMalloc,TRUE))
		fTestPass=TRUE;
	
CLEANUP:
	//release the bookmark
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(pData);

	if(HRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL,NULL,NULL), S_OK);
	}

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Buffered_ServerDataOnInsert::Terminate()
{ 
	ReleaseRowsetAndAccessor();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Immediate_ChangeInsertedRow)
//*-----------------------------------------------------------------------
//| Test Case:		Immediate_ChangeInsertedRow - Test DBPROP_CHANGEINSERTEDROW
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Immediate_ChangeInsertedRow::Init()
{ 
	BOOL		fTestPass=FALSE;
	DBPROPID	rgDBPROPID[2];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CHANGEINSERTEDROWS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Explicitly request ChangeInsertedRows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Immediate_ChangeInsertedRow::Variation_1()
{ 
	void			*rgpData[2]={NULL,NULL};
	HROW			hRow=NULL;
	ULONG			cCount=0;
	BOOL			fTestPass=FALSE;
	DBPROPID		rgDBPROPID[2];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CHANGEINSERTEDROWS;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert && change
	for(cCount=0; cCount<2; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&hRow),S_OK);

	//change the row	
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[1]),S_OK);

	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check for visibility
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

	TESTC(fTestPass = CheckRowVisible(m_pIRowset, rgpData[1], CHECK_ROWVISIBLE));

CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<2; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

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
BOOL Immediate_ChangeInsertedRow::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Buffered_ChangeInsertedRow)
//*-----------------------------------------------------------------------
//| Test Case:		Buffered_ChangeInsertedRow - Test DBPROP_CHANGEINSERTEDROW in buffered mode
//| Created:  	5-27-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Buffered_ChangeInsertedRow::Init()
{ 
	BOOL		fTestPass=FALSE;
	DBPROPID	rgDBPROPID[3];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CHANGEINSERTEDROWS;

	if(!TCIRowsetNewRow::Init())
		return FALSE;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

	COMPC(BufferedUpdate(), TRUE);
	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Explicitly request ChangeInsertedRows in buffered mode
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ChangeInsertedRow::Variation_1()
{ 
	void			*rgpData[2]={NULL,NULL};
	HROW			hRow=NULL;
	ULONG			cCount=0;
	BOOL			fTestPass=FALSE;
	DBPROPID		rgDBPROPID[3];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CHANGEINSERTEDROWS;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for insert && change
	for(cCount=0; cCount<2; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&hRow),S_OK);

	//change the row	
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,rgpData[1]),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check for visibility
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, rgpData[1], CHECK_ROWVISIBLE));
	

CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<2; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete a newly inserted row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ChangeInsertedRow::Variation_2()
{ 
	void			*pData=NULL;
	HROW			hRow=NULL;
	BOOL			fTestPass=FALSE;
	DBROWSTATUS		*pDBRowStatus=NULL;
	DBPROPID		rgDBPROPID[3];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CHANGEINSERTEDROWS;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for the last row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,&pDBRowStatus),DB_E_ERRORSOCCURRED);
	COMPC(pDBRowStatus[0], DBROWSTATUS_E_DELETED); // deletion of pending insert is a hard-delete
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check that row is not visible
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWNOTVISIBLE));
	

CLEANUP:
	PROVIDER_FREE(pDBRowStatus);
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetData on newly inserted followed by Delete
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ChangeInsertedRow::Variation_3()
{ 
	void			*pData=NULL, *pSetData=NULL;
	HROW			hRow=NULL;
	BOOL			fTestPass=FALSE;
	DBROWSTATUS		*pDBRowStatus=NULL;
	DBPROPID		rgDBPROPID[3];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CHANGEINSERTEDROWS;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for the insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	//make data for the setdata
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pSetData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&hRow),S_OK);

	//change the row	
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,pSetData),S_OK);

	//delete the row
	TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRow,NULL),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,&pDBRowStatus),DB_E_ERRORSOCCURRED);
	COMPC(pDBRowStatus[0], DBROWSTATUS_E_DELETED); // deletion of pending insert is a hard-delete

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check that row is not visible
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pSetData, CHECK_ROWNOTVISIBLE));

	TESTC_(m_pIRowset->RestartPosition(NULL),S_OK);
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pData, CHECK_ROWNOTVISIBLE));

CLEANUP:
	PROVIDER_FREE(pDBRowStatus);
	
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pSetData, TRUE);
	
	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Multiple SetData on newly inserted
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ChangeInsertedRow::Variation_4()
{ 
	void			*pInsertData=NULL, *pFirstSetData=NULL, *pSecondSetData=NULL;
	HROW			hRow=NULL;
	BOOL			fTestPass=FALSE;
	DBPROPID		rgDBPROPID[4];

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CHANGEINSERTEDROWS;
	rgDBPROPID[3]=DBPROP_CANHOLDROWS;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	//make data for the insert
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pInsertData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	// make data for the first setdata
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pFirstSetData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	//make data for the 2nd setdata
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pSecondSetData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pInsertData,&hRow),S_OK);

	//change the row	
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,pFirstSetData),S_OK);

	//change the row again
	TESTC_(m_pIRowsetChange->SetData(hRow,m_hAccessor,pSecondSetData),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,1,&hRow,NULL,NULL,NULL),S_OK);
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRow, NULL,NULL,NULL),S_OK);
	hRow=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check that 2nd insert is visible
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, pSecondSetData, CHECK_ROWVISIBLE));

CLEANUP:
	
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pInsertData, TRUE);
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pFirstSetData, TRUE);
	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pSecondSetData, TRUE);
	
	if(hRow)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRow, NULL,NULL,NULL),S_OK);

	ReleaseRowsetAndAccessor();

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Multilpe newly Inserted and interleaved SetData and Delete ops
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Buffered_ChangeInsertedRow::Variation_5()
{ 
	void			*rgpData[6]		= {NULL,NULL,NULL,NULL,NULL,NULL};
	HROW			hRowFirst		= DB_NULL_HROW,hRowSecond=DB_NULL_HROW,hRowThird=DB_NULL_HROW;
	ULONG			cCount			= 0;
	BOOL			fTestPass		= TEST_SKIPPED;
	DBPROPID		rgDBPROPID[4]	= {DBPROP_IRowsetChange, DBPROP_IRowsetUpdate, DBPROP_CHANGEINSERTEDROWS, DBPROP_CANHOLDROWS};
	ULONG_PTR		cMaxPendingRows	= 0;

	//get a rowset binds to all updatable columns
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    TESTC(GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, m_pIRowsetChange, &cMaxPendingRows));
	if (cMaxPendingRows<3)
	{
		goto CLEANUP;
	}
	fTestPass	= TEST_FAIL;

	//make data for insert && change
	for(cCount=0; cCount<6; cCount++)
	{
		//make data for the last row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&(rgpData[cCount]),g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
	}

	//insert a row
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[0],&hRowFirst),S_OK);

	//change the first row	
	TESTC_(m_pIRowsetChange->SetData(hRowFirst,m_hAccessor,rgpData[1]),S_OK);

	//insert two more rows
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[2],&hRowSecond),S_OK);
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,rgpData[3],&hRowThird),S_OK);

	//change the third row
	TESTC_(m_pIRowsetChange->SetData(hRowThird,m_hAccessor,rgpData[4]),S_OK);

	// delete the second row
	TESTC_(m_pIRowsetChange->DeleteRows(NULL,1,&hRowSecond,NULL),S_OK);

	// change the first row again
	TESTC_(m_pIRowsetChange->SetData(hRowFirst,m_hAccessor,rgpData[5]),S_OK);

	//update
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL),S_OK);
	
	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1, &hRowFirst, NULL,NULL,NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, &hRowSecond, NULL,NULL,NULL),S_OK);
	TESTC_(m_pIRowset->ReleaseRows(1, &hRowThird, NULL,NULL,NULL),S_OK);
	
	hRowFirst=hRowSecond=hRowThird=DB_NULL_HROW;

	ReleaseRowsetAndAccessor();

	// Re-Open and check for visibility
	TESTC_(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,NUMELEM(rgDBPROPID),rgDBPROPID,
								0,NULL,ON_ROWSET_ACCESSOR,TRUE,DBACCESSOR_ROWDATA,
								DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND),S_OK);

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	// Check that hRowFirst visible(was updated twice)
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, rgpData[5], CHECK_ROWVISIBLE));
	
	// Check that hRowThird (update once)
	TESTC_(m_pIRowset->RestartPosition(NULL),S_OK);	
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, rgpData[4], CHECK_ROWVISIBLE));

	// Check that hRowSecond is not visible (was deleted)
	TESTC_(m_pIRowset->RestartPosition(NULL),S_OK);
	TESTC(fTestPass = CheckRowVisible(m_pIRowset, rgpData[2], CHECK_ROWNOTVISIBLE));
	
CLEANUP:
	//Release the row handle and memory
	for(cCount=0; cCount<6; cCount++)
	{
		if(rgpData[cCount])
			ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)rgpData[cCount], TRUE);
	}

	if(hRowFirst)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRowFirst, NULL,NULL,NULL),S_OK);

	if(hRowSecond)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRowSecond, NULL,NULL,NULL),S_OK);

	if(hRowThird)
	  	CHECK(m_pIRowset->ReleaseRows(1,&hRowThird, NULL,NULL,NULL),S_OK);

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
BOOL Buffered_ChangeInsertedRow::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(InsertDefault)
//*-----------------------------------------------------------------------
//| Test Case:		InsertDefault - InsertRow using DBSTATUS_S_DEFAULT
//| Created:  	6/24/98
//*-----------------------------------------------------------------------


//*-----------------------------------------------------------------------
// @mfunc Builds a list of info about the default property of the columns
//
//*-----------------------------------------------------------------------
BOOL InsertDefault::GetDefaultColumns()
{
	CRowset		Rowset;
	BOOL		fTestPass			= FALSE;
	ULONG		i;
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
	DBORDINAL	nCol;
	ULONG		cCol;

	// init the retrictions for COLUMNS Schema Rowset
	for (i=0; i<cRes; i++)
	{
		VariantInit(&rgRes[i]);
	}
	// set the table name restriction
	rgRes[2].vt = DBTYPE_BSTR;
	V_BSTR(&rgRes[2]) = SysAllocString(m_pTable->GetTableName());

	// alloc mem for the output parameters and initialize
	SAFE_ALLOC(m_rgbDefault, BOOL, m_pTable->CountColumnsOnTable());

	for (cCol = 0; cCol < m_pTable->CountColumnsOnTable(); cCol++)
	{
		m_rgbDefault[cCol]	= FALSE;
	}

	// set restrictions and create rowset, accessor and bindings (do not bind bookmark)
	Rowset.SetRestrictions(cRes, rgRes);
	if (S_OK != Rowset.CreateRowset(SELECT_DBSCHEMA_COLUMNS, IID_IRowset, m_pTable, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

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
		if (S_OK != Rowset.GetRowData(hRow, &Rowset.m_pData))
		{
			fTestPass = TEST_SKIPPED;
			goto CLEANUP;
		}
		
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

	fTestPass = TRUE;

CLEANUP:
	// clean restriction array
	for (i=0; i<cRes; i++)
	{
		VariantClear(&rgRes[i]);
	}
	return fTestPass;
} // InsertDefault::GetDefaultColumns




//*-----------------------------------------------------------------------
// @mfunc mask the m_fHasDefault field of columns in m_pTable
//
void InsertDefault::MaskDefColumns(BOOL fMask)
{
	ULONG		iCol;
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
} //InsertDefault::MaskDefColumns




//*-----------------------------------------------------------------------
// @mfunc InsertRow, check it, get data, check it 
//
// @rdesc TRUE or FALSE
//
BOOL InsertDefault::InsertAndCheckDefault(
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
	HROW			hRow;
	ULONG			ulStatus;
	ULONG			ulStatusCC;
	ULONG			ulStatusGet;
	void			*pDefault			= NULL;
	USHORT			cb;
	ULONG			size;
	ULONG			cBinding;
	CCol			col;
	DBORDINAL		cOrdinalPos;
	DBORDINAL		nCols				= m_pTable->CountColumnsOnTable();
	WCHAR			*pwszMakeData		= NULL;
	
	
	// make sure pData is not null
	if (NULL == pData)
	{
		odtLog << "ERROR: pData is NULL\n";
		fRes = FALSE;
		goto CLEANUP;
	}

	// carbon copy of original pData in pDataCC
	SAFE_ALLOC(pDataCC, BYTE, m_cRowSize);
	memcpy(pDataCC, pData, (size_t)m_cRowSize);

	// set data
	hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor,pData,&hRow);

	// check hr
	if (NULL != hrSetData)
	{
		*hrSetData = hr;
	}
	if (fValidate)
	{
		if (S_OK!=hr)
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
				odtLog << "Error in IRowsetChange::SetData return value\n";
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
						odtLog << "Error in IRowsetChange::SetData return value\n";
						COMPARE(1,0);
						fRes = FALSE;
						goto CLEANUP;
					}
					if (DBSTATUS_E_BADSTATUS==	*(ULONG *)(pData+m_rgBinding[cBinding].obStatus)	&&
						DBSTATUS_S_DEFAULT	==	*(ULONG *)(pDataCC+m_rgBinding[cBinding].obStatus))	
					{
						odtLog << "Warning in IRowsetChange:Provider does not support DBSTATUS_S_DEFAULT\n";
						goto CLEANUP;
					}
				}
			}
		}
	}

	//get the data from the row
	SAFE_ALLOC(pDataOne, BYTE, m_cRowSize);
	memset(pDataOne, 0, (size_t)m_cRowSize);

	// get the same row
	if (SUCCEEDED(hr))
	{
		hrGetData = m_pIRowset->GetData(hRow,m_hAccessor,pDataOne);

		// general check for data (data got - pDataOne, against data set - pData
		if (!COMPARE(CompareBuffer(pDataOne, pDataCC, m_cBinding, m_rgBinding, m_pIMalloc, TRUE, 
				FALSE, COMPARE_ONLY, TRUE), TRUE))
			fRes = FALSE;
	}

	m_pTable->SetIndexColumn(1);

	// for the column with the ordinal cCount
	// the provider can return either a status of OK, NULL or UNAVAILABLE
	// for the column marked as DEFAULT going in
	// the provider detected a default value for the column => DBSTATUS_S_OK or DBSTATUS_S_ISNULL
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{	
		//alloc a string for comparing
		pwszMakeData = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * MAXDATALEN);
		pwszMakeData[0]=L'\0';

		// get ordinal of the column
		cOrdinalPos = m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);

		// make sure it is in range
		if (!COMPARE(cOrdinalPos <= nCols, TRUE))
			fRes = FALSE;

		// get statuses
		ulStatus	= *(ULONG*)(pData+m_rgBinding[cBinding].obStatus);
		ulStatusCC	= *(ULONG*)(pDataCC+m_rgBinding[cBinding].obStatus);
		ulStatusGet	= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obStatus);

		// get data size
		size		= *(ULONG*)(pDataOne+m_rgBinding[cBinding].obLength);
		
		// if column status was DBSTATUS_S_DEFAULT... 
		if (DBSTATUS_S_DEFAULT == ulStatusCC)
		{
			// check success in setting data
			if (S_OK == hr && !COMPARE(DBSTATUS_S_DEFAULT, ulStatus))
			{
				odtLog << "ERROR: Bad return status for column " << col.GetColName() << " in SetData\n";
				fRes = FALSE;
			}

			if (S_OK != hr)
				continue;
 
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
							odtLog << L"ERROR comparing the returned value for column " << cOrdinalPos << L" (default)\n";
							fRes = FALSE;
						}
						break;
					case DBSTATUS_E_UNAVAILABLE:						
						break;
					default:
						odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() << "\n";
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
						odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() << "\n";
						fRes = FALSE;
						break;
				}
			}
			SAFE_FREE(pDefault);
		}
		if (pwszMakeData)
		{
			PROVIDER_FREE(pwszMakeData);
		}
	}

CLEANUP:
	SAFE_FREE(pDefault);
	if(hRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	if( pDataOne )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataOne, TRUE);
	SAFE_FREE(pDataCC);

	return fRes;
} //InsertDefault::InsertAndCheckDefault




//*-----------------------------------------------------------------------
// @mfunc InsertDefault::PrepareToInsert
//			Creates a rowset and accessors, fills the input bindings and
//			set the proper status of the given column
//
// @rdesc E_FAIL on insuccess or S_OK on success
//
HRESULT InsertDefault::PrepareForInsert(
	DBORDINAL		cSelectedColumn,	// [in]	the ordinal of the selected column (1 based)
	DBSTATUS		Status,				// [in] the status value for the selected column
	LONG			lRowsOffset,		// [in] row number for data creation
	BYTE			**ppData,			// [out] data buffer
	ULONG			cPropsToUse			// [in] default is 1, set this to 2 to get DBPROP_SERVERDATAONINSERT
)
{
	HRESULT			hr = E_FAIL;
	ULONG			cBinding;
	// properties asked for the rowset related to bindings
	DBPROPID		rgPropertyIDs[] = {DBPROP_IRowsetChange,DBPROP_SERVERDATAONINSERT}; 

	
	
	ASSERT(DBSTATUS_S_DEFAULT == Status || DBSTATUS_S_IGNORE == Status);
	TESTC(NULL != ppData);
	*ppData = NULL;

	//create an accessor on the command object on a updatable column only
	hr = GetRowsetAndAccessor(SELECT_ALLFROMTBL,cPropsToUse,rgPropertyIDs,0,NULL,
										ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH, UPDATEABLE_COLS_BOUND,
										FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,0,NULL);
	if (S_OK!=hr)
	{
		goto CLEANUP;
	}

	//get a new data buffer to set the data
	TESTC_(FillInputBindings(	m_pTable,
								DBACCESSOR_ROWDATA,
								m_cBinding,
								m_rgBinding,
								ppData,
								lRowsOffset+1,					// row number for data creation
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

	hr = S_OK;

CLEANUP:
	return hr;
} // InsertDefault::PrepareForInsert



//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertDefault::Init()
{ 
	BOOL				fRes				= FALSE;
	ULONG				nCol				= 0;
	DBCOLUMNDESC		*pColumnDesc		= NULL;
	ULONG				cColumnDesc			= 1;
	HRESULT				hr;
	ULONG				nDefault			= 0;
	ITableDefinition	*pITableDefinition	= NULL;
	DBORDINAL			cColsOnTable		= 0;

	m_pCustomTable	= new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	m_rgbDefault	= NULL;
	m_nRows			= 12;
	m_cSeed			= g_pTable->CountColumnsOnTable () + 1; 

	// {{ TCW_INIT_BASECLASS_CHECK
	// }}
	if(TCIRowsetNewRow::Init())
	{ 
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
			memset(m_rgbDefault, 0,(size_t)(sizeof(BOOL)*cColsOnTable));

			for (nCol = 0; nCol < cColsOnTable; nCol++)
			{
				SAFE_ALLOC(pColumnDesc, DBCOLUMNDESC, 1);

				// get column, position and column desc for column nCol'th
				CCol& rCol = g_pTable->GetColInfoForUpdate(nCol+1);

				if (g_pTable->SetDefaultValue(rCol, m_cSeed))
				{
					// build the column description
					g_pTable->BuildColumnDesc(pColumnDesc, rCol);

					// set column desc as current column desc for the table
					m_pCustomTable->SetColumnDesc(pColumnDesc, 1);
					m_pCustomTable->SetBuildColumnDesc(FALSE);	// use the pcolumnDesc

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
				SAFE_FREE(pColumnDesc);
			}
		}
		fRes = TEST_PASS;
	} 
CLEANUP:
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
int InsertDefault::Variation_1()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos;
	ULONG			cBinding		= 0;
	CCol			col;
	ULONG			lRowsOffset		= 2;
	// buffer for row data
	BYTE			*pData			= NULL;
	BYTE			*pDataCpy		= NULL;
	ULONG			nDefault		= 0;
	ULONG			cIndexInner		= 0;
	ULONG			cIndexOutter	= 0;


	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);

		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(0, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
		
	// Duplicate the pData
	pDataCpy = (BYTE*) PROVIDER_ALLOC(m_cRowSize);
	TESTC(pDataCpy != NULL);
	memcpy(pDataCpy, pData, (size_t)m_cRowSize);

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
		fTestPass = TEST_SKIPPED;
		odtLog << "Skip variation: No default columns were found\n";
		goto CLEANUP;
	}

	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:
	
	// release buffers
	if( pDataCpy )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataCpy, TRUE);
	PROVIDER_FREE(pData);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a default column; set many columns, one is asked default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_2()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 98;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			ulColAttr			= 0;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// build the custom table
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// get a default column
	ulColAttr = COL_COND_DEFAULT;
	if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
	{
		// there is no explicit default, get a nullable column
		ulColAttr = COL_COND_NULL;
		if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
			odtLog << "No default column was detected\n";
			goto CLEANUP;
		}
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckDefault(pData, S_OK));
	
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not nullable default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_3()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_NOTNULL};
	ULONG			cColAttr			= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	MaskDefColumns(TRUE);

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not find a default nullable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		// retrieve column
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(!col.GetNullable());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def == NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_4()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);

	// get an implicit default (nullable) column (def == null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not find an updateable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		TESTC(col.GetNullable());
		TESTC(!col.GetHasDefault());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;
CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def != NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_5()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	
	// get a default, nullable column (def != NULL)
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UPDATEABLE, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	// reset all the columns to their non default value
	MaskDefColumns(TRUE);
	
	// get a default nullable column (default != null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not identify column with specified attributes\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(col.GetNullable());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_6()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	ULONG	rgColAttr[]	= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NOTNULL};

	if (m_fCustomTables)
	{
		//get a updateable column (not def)
		//this is because a custom table won't have non default or non null cols so this would
		//be skipped if 
		if (!g_pTable->GetColWithAttr(COL_COND_UPDATEABLE, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}
	else
	{
		// get an updateable column (not def)
		if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	InsertAndCheckDefault(pData, DB_E_ERRORSOCCURRED, TRUE, &hr);
	TESTC(DB_E_ERRORSOCCURRED == hr || DB_E_INTEGRITYVIOLATION == hr);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	}
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a unique default value
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_7()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// properties asked for the rowset related to bindings
	DBPROPID		rgPropertyIDs[] = {DBPROP_IRowsetChange}; 
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UNIQUE};
	ULONG			cColAttr			= 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	if (!SettableProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
	
	// reset all the columns to their non default value
	MaskDefColumns(TRUE);

	// get a default column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		// try again, this time looking for a nullable column
		rgColAttr[0] = COL_COND_NULL;
		if (m_fCustomTables || !g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
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
			//if DBPROPSET_COLUMN is set to DBPROP_COL_NULLABLE, change the value to FALSE 
			//instead of adding another DBPROPSET_COLUMN to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_FALSE;
				fPropNULLSet = TRUE;
			}
			//make sure DBPROP_COL_UNIQUE is set to TRUE
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_UNIQUE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_TRUE;
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		// retrieve the column and make sure is has a default value
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckDefault(pData, S_OK));
	InsertAndCheckDefault(pData, S_OK, FALSE, &hr);
	TESTC(DB_E_INTEGRITYVIOLATION == hr || DB_E_ERRORSOCCURRED == hr);
	if (DB_E_ERRORSOCCURRED == hr)
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Set default values on all default columns (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_8()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos;
	ULONG			cBinding		= 0;
	CCol			col;
	ULONG			lRowsOffset		= 2;
	// buffer for row data
	BYTE			*pData			= NULL;
	BYTE			*pDataCpy		= NULL;
	ULONG			nDefault		= 0;
	ULONG			cIndexInner		= 0;
	ULONG			cIndexOutter	= 0;
	HRESULT			hr;


	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);

		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// create rowset and accessor and fill input bindings, turn on SERVERDATAONINSERT
	hr=PrepareForInsert(0, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}

	// Duplicate the pData
	pDataCpy = (BYTE*) PROVIDER_ALLOC(m_cRowSize);
	TESTC(pDataCpy != NULL);
	memcpy(pDataCpy, pData, (size_t)m_cRowSize);

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
		fTestPass = TEST_SKIPPED;
		odtLog << "Skip variation: No default columns were found\n";
		goto CLEANUP;
	}

	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:
	// release buffers
	if( pDataCpy )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataCpy, TRUE);
	PROVIDER_FREE(pData);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a default column; set many columns, one is asked default (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_9()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 98;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			ulColAttr;
	HRESULT			hr;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// build the custom table
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// get a default column
	ulColAttr = COL_COND_DEFAULT;
	if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
	{
		// there is no explicit default, get a nullable column
		ulColAttr = COL_COND_NULL;
		if (!m_pTable->GetColWithAttr(1, &ulColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
			odtLog << "No default column was detected\n";
			goto CLEANUP;
		}
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	TESTC(InsertAndCheckDefault(pData, S_OK));
	
	fTestPass = TEST_PASS;

CLEANUP:
	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not nullable default column (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_10()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_NOTNULL};
	ULONG			cColAttr			= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;
	HRESULT			hr;

	MaskDefColumns(TRUE);

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not find a default nullable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		// retrieve column
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(!col.GetNullable());
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def == NULL) (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_11()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL			cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;
	HRESULT			hr;

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);

	// get an implicit default (nullable) column (def == null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not find an updateable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		TESTC(col.GetNullable());
		TESTC(!col.GetHasDefault());
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;
CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);
	
	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a nullable default column (def != NULL) (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_12()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	
	// get a default, nullable column (def != NULL)
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UPDATEABLE, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;
	HRESULT			hr;


	// reset all the columns to their non default value
	MaskDefColumns(TRUE);
	
	// get a default nullable column (default != null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestPass = TEST_SKIPPED;
		odtLog << "could not identify column with specified attributes\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(col.GetNullable());
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	TESTC(InsertAndCheckDefault(pData, S_OK));
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a not default column (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_13()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;


	ULONG	rgColAttr[]	= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NOTNULL};

	if (m_fCustomTables)
	{
		//get a updateable column (not def)
		//this is because a custom table won't have non default or non null cols so this would
		//be skipped if 
		if (!g_pTable->GetColWithAttr(COL_COND_UPDATEABLE, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}
	else
	{
		// get an updateable column (not def)
		if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	InsertAndCheckDefault(pData, DB_E_ERRORSOCCURRED, TRUE, &hr);
	TESTC(DB_E_ERRORSOCCURRED == hr || DB_E_INTEGRITYVIOLATION == hr);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	}
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Set default value on a unique default value (SDI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertDefault::Variation_14()
{ 
	BOOL			fTestPass		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// properties asked for the rowset related to bindings
	DBPROPID		rgPropertyIDs[] = {DBPROP_IRowsetChange}; 
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	HRESULT			hr;
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UNIQUE};
	ULONG			cColAttr			= 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	if (!SettableProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}	
	
	// reset all the columns to their non default value
	MaskDefColumns(TRUE);

	// get a default column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		// try again, this time looking for a nullable column
		rgColAttr[0] = COL_COND_NULL;
		if (m_fCustomTables || !g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
		{
			fTestPass = TEST_SKIPPED;
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
			//if DBPROPSET_COLUMN is set to DBPROP_COL_NULLABLE, change the value to FALSE 
			//instead of adding another DBPROPSET_COLUMN to the prop set because that won't work!
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_NULLABLE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_FALSE;
				fPropNULLSet = TRUE;
			}
			//make sure DBPROP_COL_UNIQUE is set to TRUE
			if (rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].dwPropertyID==DBPROP_COL_UNIQUE)
			{
				rgColumnDesc[col.GetColNum()-1].rgPropertySets->rgProperties[i].vValue.boolVal=VARIANT_TRUE;
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		// retrieve the column and make sure is has a default value
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
	}

	// create rowset and accessor and fill input bindings.  turn on SERVERDATAONINSERT
	hr=PrepareForInsert(cSelectedColumn, DBSTATUS_S_DEFAULT, lRowsOffset, &pData,2);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

	//if SERVERDATAONINSERT is not supported then skip, any other failure is an error
	if (DB_E_ERRORSOCCURRED	== hr	||	DB_S_ERRORSOCCURRED	== hr	|| S_FALSE	== hr)
	{
		fTestPass	= TEST_SKIPPED;
		goto CLEANUP;
	}
	if(!SUCCEEDED(hr))
	{
		goto CLEANUP;
	}


	TESTC(InsertAndCheckDefault(pData, S_OK));
	InsertAndCheckDefault(pData, S_OK, FALSE, &hr);
	TESTC(DB_E_INTEGRITYVIOLATION == hr || DB_E_ERRORSOCCURRED == hr);
	if (DB_E_ERRORSOCCURRED == hr)
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	fTestPass = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL InsertDefault::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	SAFE_FREE(m_rgbDefault);
	delete m_pCustomTable;
	return(TCIRowsetNewRow::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(InsertIgnore)
//*-----------------------------------------------------------------------
//| Test Case:		InsertIgnore - InsertRow using DBSTATUS_S_IGNORE
//| Created:  	6/24/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertIgnore::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(InsertDefault::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 



//*-----------------------------------------------------------------------
// @mfunc SetData, check it, get data, check it 
//
// @rdesc TRUE or FALSE
//
BOOL InsertIgnore::InsertAndCheckIgnore(
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
	HROW			hRow;

	ULONG			ulStatus;
	ULONG			ulStatusCC;
	ULONG			ulStatusGet;

	ULONG			cBinding;
	CCol			col;
	DBORDINAL		cOrdinalPos;
	DBORDINAL		nCols				= m_pTable->CountColumnsOnTable();

	// make sure pData is not null
	if (NULL == pData)
	{
		odtLog << "ERROR: pData is NULL\n";
		fRes = FALSE;
		goto CLEANUP;
	}

	// carbon copy of original pData in pDataCC
	SAFE_ALLOC(pDataCC, BYTE, m_cRowSize);
	memcpy(pDataCC, pData, (size_t)m_cRowSize);

	// set data from pData
	hr = m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, m_hAccessor, pData, &hRow);

	// check hr
	if (NULL != hrSetData)
		*hrSetData = hr;
	if (fValidate && !CHECK(hr, hrSetDataExpected))
	{
		odtLog << "Error in IRowsetChange::SetData return value\n";
		fRes = FALSE;
	}

	//get the data from the row
	SAFE_ALLOC(pDataOne, BYTE, m_cRowSize);
	memset(pDataOne, 0, (size_t)m_cRowSize);

	// get the same row data in pDataOne
	if (SUCCEEDED(hr))
	{
		hrGetData = m_pIRowset->GetData(hRow,m_hAccessor,pDataOne);

		// general check for data (data got - pDataOne, against data set - pData
		if (!COMPARE(CompareBuffer(pDataOne, pDataCC, m_cBinding, m_rgBinding, m_pIMalloc, TRUE, 
				FALSE, COMPARE_ONLY, TRUE), TRUE))
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
		
		// if column status was DBSTATUS_S_DEFAULT... 
		if (DBSTATUS_S_IGNORE == ulStatusCC)
		{
			// check success in setting data
			if (S_OK == hr && !COMPARE(DBSTATUS_S_IGNORE, ulStatus))
			{
				odtLog << "ERROR: Bad return status for column " << col.GetColName() << " in InsertRow\n";
				fRes = FALSE;
			}

			if (S_OK == hr)
			{
				// status for getting data 		
				switch (ulStatusGet)
				{
					case DBSTATUS_S_OK:
						break;
					case DBSTATUS_S_ISNULL:
						if (!col.GetNullable())
						{
							odtLog << "ERROR: mismatch for get statuses: DBSTATUS_S_ISNULL\n";
							fRes = FALSE;
						}
						break;
					case DBSTATUS_E_UNAVAILABLE:						
						break;
					default:
						odtLog << "ERROR in IRowset::GetData for column " << col.GetColName() << "\n";
						fRes = FALSE;
						break;
				}
			}
		}
	}

CLEANUP:
	if(hRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,&hRow,NULL,NULL,NULL),S_OK);
	
	if( pDataOne )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataOne, TRUE);
	SAFE_FREE(pDataCC);

	return fRes;
} //InsertIgnore::InsertAndCheckIgnore




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore values on all updateable columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_1()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	DBORDINAL		cOrdinalPos;
	ULONG			cBinding;
	ULONG			lRowsOffset		= 21;	
	CCol			col;
	// buffer for row data
	BYTE			*pData			= NULL;
	BYTE			*pDataCpy		= NULL;
	
	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// create a table with maximum number of default values
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(0, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	// Duplicate the pData
	pDataCpy = (BYTE*) PROVIDER_ALLOC(m_cRowSize);
	TESTC(pDataCpy != NULL);
	memcpy(pDataCpy, pData, (size_t)m_cRowSize);

	// bind status to DBSTATUS_S_DEFAULT for all the updateable and default columns
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		// get column number and retrieve column
		cOrdinalPos	= m_rgBinding[cBinding].iOrdinal;
		col			= m_pTable->GetColInfoForUpdate(cOrdinalPos);
		if (col.GetHasDefault() || col.GetNullable())
			*(ULONG *)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;
	}

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pDataCpy )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pDataCpy, TRUE);
	PROVIDER_FREE(pData);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore value on a column; set many columns, one is asked ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_2()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 98;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]		= {COL_COND_DEFAULT, COL_COND_NOTNULL};


	MaskDefColumns(TRUE);

	if (m_fCustomTables)
	{
		// build the custom table
		g_pTable->BuildColumnDescs(&rgColumnDesc);
		m_pCustomTable->SetColumnDesc(rgColumnDesc, cColumnDesc);

		// create a table with as many default columns as possible
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))

	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find a default nullable column\n";
		goto CLEANUP;
	}
	col = g_pTable->GetColInfoForUpdate(cSelectedColumn);

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:
	
	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore value on a not nullable default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_3()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData			= NULL;
	ULONG			lRowsOffset		= 2;
	DBORDINAL		cSelectedColumn	= 0;
	ULONG			rgColAttr[]		= {COL_COND_DEFAULT, COL_COND_NOTNULL};
	ULONG			cColAttr		= m_fCustomTables? 1: 2;
	BOOL			fOldNullable;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	MaskDefColumns(TRUE);

	// get a default not nullable column
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find a default nullable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		// retrieve column
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(!col.GetNullable());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore value on a nullable default column (def == NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_4()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	ULONG			rgColAttr[]			= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	// reset all the columns to their non default value
	MaskDefColumns(FALSE);

	// set an implicit default (nullable) column (def == null)

	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not find an updateable column\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		TESTC(col.GetNullable());
		TESTC(!col.GetHasDefault());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore value on a nullable default column (def != NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_5()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	
	// get a default, nullable column (def != NULL)
	ULONG			rgColAttr[]			= {COL_COND_DEFAULT, COL_COND_UPDATEABLE, COL_COND_NULL};
	ULONG			cColAttr			= 3;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	// reset all the columns to their non default value
	MaskDefColumns(TRUE);

	// get a default nullable column (default != null)
	if (!g_pTable->GetColWithAttr(cColAttr, rgColAttr, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		odtLog << "could not identify column with specified attributes\n";
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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
		col = m_pTable->GetColInfoForUpdate(cSelectedColumn);
		// set status in bindings
		TESTC(col.GetHasDefault());
		TESTC(col.GetNullable());
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;
CLEANUP:
	
	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Insert ignore value on a not default column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_6()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	DBCOLUMNDESC	*rgColumnDesc	= NULL;
	DBORDINAL		cColumnDesc		= g_pTable->CountColumnsOnTable();
	CCol			col;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	DBORDINAL		cSelectedColumn		= 0;
	BOOL			fOldNullable;
	HRESULT			hr;
	BOOL			fPropNULLSet	= FALSE;
	WORD			i;

	ULONG	rgColAttr[]	= {COL_COND_UPDATEABLE, COL_COND_NOTDEFAULT, COL_COND_NOTNULL};

	MaskDefColumns(FALSE);

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
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}
	else
	{
		// get an updateable column (not def)
		if (!g_pTable->GetColWithAttr(NUMELEM(rgColAttr), rgColAttr, &cSelectedColumn))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
		col = g_pTable->GetColInfoForUpdate(cSelectedColumn);
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, DB_E_INTEGRITYVIOLATION, FALSE, &hr));
	TESTC(DB_E_ERRORSOCCURRED == hr || DB_E_INTEGRITYVIOLATION == hr);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC	(	DBSTATUS_E_INTEGRITYVIOLATION	== *(ULONG*)(pData+m_rgBinding[0].obStatus) ||
					DBSTATUS_E_BADSTATUS			== *(ULONG*)(pData+m_rgBinding[0].obStatus)
				);
	}
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->SetColumnDesc(NULL, 0);
	m_pCustomTable->DropTable();
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set status to DBSTATUS_S_IGNORE on a non updateable column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_7()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 23;
	DBORDINAL		cSelectedColumn		= 0;

	if (m_fCustomTables)
	{
		m_pCustomTable->SetBuildColumnDesc(TRUE);	// create ColList again
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}
	// get a non updateable column
	if (!m_pTable->GetColWithAttr(COL_COND_AUTOINC, &cSelectedColumn))
	{
		fTestRes = TEST_SKIPPED;
		goto CLEANUP;
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(cSelectedColumn, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->DropTable();

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Insert all updateable cols, set status to DBSTATUS_S_IGNORE for half of them
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int InsertIgnore::Variation_8()
{ 
	BOOL			fTestRes		= TEST_FAIL;
	// buffer for row data
	BYTE			*pData				= NULL;
	ULONG			lRowsOffset			= 2;
	ULONG			cBinding;
	CCol			col;

	if (m_fCustomTables)
	{
		// create a table with as many default columns as possible
		m_pCustomTable->SetBuildColumnDesc(TRUE);	// create ColList again
		TESTC_(m_pCustomTable->CreateTable(0, 0), S_OK);
	}

	// create rowset and accessor and fill input bindings
	TESTC_(PrepareForInsert(0, DBSTATUS_S_IGNORE, lRowsOffset, &pData), S_OK);
    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestRes = TEST_SKIPPED;
		goto CLEANUP;
    }
		
	// set status in bindings
	for (cBinding = 0; cBinding < m_cBinding; cBinding++)
	{
		if ((col.GetHasDefault() || col.GetNullable()) && cBinding % 2)
		{
			*(ULONG *)(pData+m_rgBinding[cBinding].obStatus) = DBSTATUS_S_IGNORE;
			break;
		}
	}

	TESTC(InsertAndCheckIgnore(pData, S_OK));
	fTestRes = TEST_PASS;

CLEANUP:

	// release buffers
	if( pData )
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding, (BYTE *)pData, TRUE);

	ReleaseRowsetAndAccessor();
	m_pCustomTable->DropTable();

	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL InsertIgnore::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(InsertDefault::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(Sequence)
//*-----------------------------------------------------------------------
//| Test Case:		Sequence - sequence testing
//|	Created:			05/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ErrorCases::Init()
{
	if(TCIRowsetNewRow::Init())
		return TRUE;
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ErrorCases::Variation_1()
{
	return TRUE;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DATAOVERFLOW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ErrorCases::Variation_2()
{
	return TRUE;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_MAXPENDCHANGESEXCEEDED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ErrorCases::Variation_3()
{
	DBPROPID	rgDBPROPID[3];
	ULONG		cCount			= 0;
	DBCOUNTITEM	cRows			= 0;
	void		*pData			= NULL;
	HROW		*pHRow			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	ULONG_PTR	ulValue			= 0;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_IRowsetUpdate;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,3,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET,m_pIRowset,&ulValue);
	
	//if there is no limit this error can not be reached
	if (0==ulValue)
	{
		goto CLEANUP;
	}

	fTestPass = TEST_FAIL;
	
	//insert the max pending rows
	for(cCount=0;cCount<ulValue;cCount++)
	{
		//make data for the row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
		
		//insert 
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),S_OK);
		if(pData)
		{
			PROVIDER_FREE(pData);
		}
	}
	
	//make data for the row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert one more row than allowed into the pending buffer
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),DB_E_MAXPENDCHANGESEXCEEDED);

	//update, should still work with the row currently in the buffer
	TESTC_(m_pIRowsetUpdate->Update(NULL,0,NULL,&cRows, &pHRow,NULL),S_OK);
	
	fTestPass=TRUE;	
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData, TRUE);
	}
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ROWLIMITEDEXCEEDED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ErrorCases::Variation_4()
{
	DBPROPID	rgDBPROPID[2];
	ULONG		cCount			= 0;
	ULONG		cRows			= 0;
	void		*pData			= NULL;
	HROW		*pHRow			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	ULONG_PTR	ulValue			= 0;
	HROW		*hRow			= NULL;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }
	GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET,m_pIRowset,&ulValue);
	
	//if there is no limit this error can not be reached
	if (0==ulValue)
	{
		goto CLEANUP;
	}

	//allocate memory for row handles
	if(!(hRow=(HROW*)PROVIDER_ALLOC(ulValue*sizeof(HROW))))
	{
		goto CLEANUP;
	}

	fTestPass = TEST_FAIL;
	
	//insert the max pending rows
	for(cCount=0;cCount<ulValue;cCount++)
	{
		//make data for the row handle
		TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
								(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
								m_rgTableColOrds,PRIMARY),S_OK);
		
		//insert 
		TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&(hRow[cCount])),S_OK);
		if(pData)
		{
			PROVIDER_FREE(pData);
		}
	}
	
	//make data for the row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert one more row than allowed into the pending buffer
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,&(hRow[cCount])),DB_E_ROWLIMITEXCEEDED);
	
	fTestPass=TRUE;	
CLEANUP:
	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData, TRUE);
	}
	//insert the max pending rows
	for(cCount=0;cCount<ulValue;cCount++)
	{
		if(hRow[cCount])
		{
			CHECK(m_pIRowset->ReleaseRows(cRows,&(hRow[cCount]),NULL,NULL,NULL),S_OK);
		}
	}
	PROVIDER_FREE(hRow);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ErrorCases::Variation_5()
{
	DBPROPID	rgDBPROPID[2];
	void		*pData			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_CANHOLDROWS;

	m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE;

	if (!(DBPROPFLAGS_WRITE &GetPropInfoFlags(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pIDBInitialize)))
	{
		goto CLEANUP;
	}

	//get a rowset binds to all updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,2,rgDBPROPID,
										0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    fTestPass = TEST_FAIL;
	
	//make data for the row handle
	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&pData,g_ulNextRow++,m_cRowsetCols,
							m_rgTableColOrds,PRIMARY),S_OK);
		
	//insert 
	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,pData,NULL),DB_E_NOTSUPPORTED);
	
	fTestPass=TRUE;	
CLEANUP:
	m_ulUpdFlags = DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT|DBPROPVAL_UP_DELETE;

	if(pData)
	{
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData, TRUE);
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
BOOL ErrorCases::Terminate()
{
	return(TCIRowsetNewRow::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(ServerDataOnInsertII)
//*-----------------------------------------------------------------------
//| Test Case:		ServerDataOnInsertII - test Fetch computed columns
//|	Created:			07/06/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ServerDataOnInsertII::Init()
{
	BOOL		fTestPass=FALSE;

	if(!TCIRowsetNewRow::Init())
		return FALSE;
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetData of computed column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ServerDataOnInsertII::Variation_1()
{
	BOOL			fTestPass		= TEST_SKIPPED;
	DBPROPID		rgDBPROPID[3];
	void			*pVisibleData	= NULL;
	HROW			hNewRow			= NULL;
	ULONG			cCount			= 0;
	BYTE			*pbAddr;

	rgDBPROPID[0]=DBPROP_IRowsetChange;
	rgDBPROPID[1]=DBPROP_SERVERDATAONINSERT;
	rgDBPROPID[2]=DBPROP_CANHOLDROWS;

	//this variation wants its own rowset, make sure the one created in the init is gone
	ReleaseRowsetAndAccessor();

	//create an accessor to bind updatable columns
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_COMPUTEDCOLLIST,3,rgDBPROPID,0,NULL,ON_ROWSET_ACCESSOR,FALSE,DBACCESSOR_ROWDATA,
										DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,UPDATEABLE_COLS_BOUND));

    //if there are no updatable rows
    if (!m_cBinding)
    {
        fTestPass = TEST_SKIPPED;
		goto CLEANUP;
    }

    fTestPass	= TEST_FAIL;

	TESTC_(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
							(BYTE **)&m_pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds,PRIMARY),S_OK);

	TESTC_(m_pIRowsetChange->InsertRow(DB_NULL_HCHAPTER,m_hAccessor,m_pData,&hNewRow),S_OK);

	PROVIDER_FREE(m_pData);

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);
	if(!m_pData)
	{
		goto CLEANUP;
	}
	//Get the data for the row handle
	//This should bring back the computed column because SERVERDATAONINSERT is TRUE
	TESTC_(m_pIRowset->GetData(hNewRow,m_hAccessor,m_pData),S_OK);

	pbAddr=(BYTE *)m_pData;

	//the value for this computed column will be zero
//	COMPARE(DBSTATUS_S_OK,(*(DBSTATUS *)(pbAddr+(m_rgBinding[m_cBinding-1].obStatus))));
//	COMPARE(0,(*(ULONG *)(pbAddr+(m_rgBinding[m_cBinding-1].obValue))));

	//release the row handle
	TESTC_(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	hNewRow=NULL;

	fTestPass = TRUE;
CLEANUP:
	PROVIDER_FREE(m_pData);
	if(hNewRow && m_pIRowset)
		CHECK(m_pIRowset->ReleaseRows(1,&hNewRow,NULL,NULL,NULL),S_OK);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetData of default value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ServerDataOnInsertII::Variation_2()
{
//TODO
	BOOL			fTestPass		= TEST_SKIPPED;

	fTestPass = TRUE;
	return fTestPass;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ServerDataOnInsertII::Terminate()
{
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetNewRow::Terminate());
}	// }}

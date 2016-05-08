//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module KAGTEST.CPP | KAGTEST source file for all test modules.
//

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "modstandard.hpp"
#include "KAGTEST.h"

#include "extralib.h"

#define STRINGSIZE	500

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x313f5420, 0xa823, 0x11d0, { 0x92, 0xd1, 0x00, 0x80, 0xc7, 0xe0, 0x4a, 0x81 }};
DECLARE_MODULE_NAME("KageraSpecificTest");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Kagera Specific Test");
DECLARE_MODULE_VERSION(795921705);
// }}
BOOL	g_fSqlServer    = FALSE;

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	LPWSTR				wszProviderName=NULL;

	//CommonModuleInit, Verify IRowset is supported, and Create a table
	if (!CommonModuleInit(pThisTestModule, IID_IRowset, 15))
	{
		return FALSE;
	}
	if (wcscmp(pThisTestModule->m_pwszProviderName, g_wszKageraName))
	{
		odtLog << L"This test only runs against the Kagera Provider.\n";
		return TEST_SKIPPED;
	}

	GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown, &wszProviderName);
	if (wszProviderName && !wcscmp((LPWSTR)wszProviderName, L"Microsoft SQL Server"))
    {
        g_fSqlServer = TRUE;
    }
		PROVIDER_FREE(wszProviderName);
    return TRUE;
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
 				for(cCnt=1; cCnt<=g_pTable->GetRowsOnCTable(); cCnt++)
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//Base Class definition for Kagera specific test.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CKageraTest: Base class for all kagera specific test cases.
class CKageraTest : public CRowsetObject
{
public:
	IDBProperties		*m_pIDBProperties;
	IDBInitialize		*m_pIDBInitialize;
	
	SQLHENV		m_henv;
	SQLHDBC		m_hdbc;
	SQLHSTMT	m_hstmt;
	SQLHDBC		m_hdesc;

	DBPROPSET	*m_rgPropSets;
	ULONG		m_cPropSets;
private:
	BOOL InitODBC(	enum STATE_ENUM	eState,
					SQLPOINTER		Value);

	BOOL InitOLEDB(enum STATE_ENUM eState);
protected:
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

	//@cmember 
	DBORDINAL			m_ulTableRows;
		
	//@cmember 
	BOOL				m_bIndexExists;

	enum STATE_ENUM m_eState;

	BOOL GetMeToState(	enum STATE_ENUM eState,
						SQLPOINTER		Value);
	BOOL FreeState(enum STATE_ENUM eState);

	TESTRESULT TestGetInfo(enum GETINFO_ENUM eInfoValue);

	BOOL Init();
	BOOL Terminate();
		
	//@mfunc: Create a command object and set properties, execute a sql statement,
	//		  and create a rowset object.  Create an accessor on the rowset 
	BOOL GetRowsetAndAccessor
	(	
		EQUERY				eSQLStmt,
		ULONG				cProperties			=0,			
		const DBPROPID		*rgProperties		=NULL,			
		DBPROPSTATUS		dwPropStatus		=DBPROPOPTIONS_REQUIRED	,
		ULONG				cPropertiesUnset	=0,
		const DBPROPID		*rgPropertiesUnset	=NULL,
		BOOL				fBindLongColumn		=FALSE,
		DBACCESSORFLAGS		dwAccessorFlags		=DBACCESSOR_ROWDATA,		
		DBPART				dwPart				=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
		ECOLS_BOUND			eColsToBind			=ALL_COLS_BOUND,			
		ECOLUMNORDER		eBindingOrder		=FORWARD,			
		ECOLS_BY_REF		eColsByRef			=NO_COLS_BY_REF,				
		DBTYPE				dbTypeModifier		=DBTYPE_EMPTY,
		DBORDINAL			cColsToBind			=0,
		ULONG				*rgColsToBind		=NULL,
		ECOLS_MEM_PROV_OWNED 
							eColsMemProvOwned = NO_COLS_OWNED_BY_PROV,	//@paramopt [IN] Which columns' memory is to be owned by the provider
		DBPARAMIO			eParamIO = DBPARAMIO_NOTPARAM				//@paramopt [IN] Parameter type to specify for eParmIO
		);

	//@mfunc: release a rowset object and accessor created on it
	void ReleaseRowsetAndAccessor();

	//@cmember CTOR
	CKageraTest(LPWSTR wszTestCaseName):CRowsetObject(wszTestCaseName) 
	{
		m_pIDBProperties	=NULL;
		m_pIRowset			=NULL;
		m_hAccessor			=NULL;
		m_cRowSize			=0;
		m_cBinding			=0;
		m_rgBinding			=NULL;
		m_rgInfo			=NULL;
		m_pStringsBuffer	=NULL;
		m_pData				=NULL;
		m_bIndexExists		=FALSE;
		m_ulTableRows		=0;
	}

	virtual ~CKageraTest()
	{
		// Free pIDBProperties
		SAFE_RELEASE(m_pIDBProperties)
	}
};

//---------------------------------------------------------------------------
//	CKageraTest::InitODBC
//
//	@mfunc	STATE_ENUM	eState				
//			SQLPOINTER  Value odbc behavior to connect as 
//
//  this function brings the class obdc handles to the requested sState 
//---------------------------------------------------------------------------
BOOL CKageraTest::InitODBC(enum STATE_ENUM	eState,
						   SQLPOINTER		Value)
{
	SQLRETURN	rc;
	WCHAR		*pwszTemp		=	NULL;
	WCHAR		*pwszTemp2		=	NULL;
	WCHAR		*pwszTemp3		=	NULL;
	WCHAR		*pwszDSN		=	NULL;
	WCHAR		*pwszUID		=	NULL;
	WCHAR		*pwszPWD		=	NULL;
	WCHAR		*pToken			=	NULL;
	WCHAR		*pwszInitODBC	=	NULL;
	BOOL		fReturn			=	FALSE;

	if (!m_pwszInitString)
		return FALSE;

	// Extract DSN, UID, PWD from init string
	// DSN
	pwszTemp = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pwszInitString)+1)*sizeof(WCHAR));
	wcscpy(pwszTemp, m_pwszInitString);
	pwszTemp2 = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pwszInitString)+1)*sizeof(WCHAR));
	wcscpy(pwszTemp2, m_pwszInitString);
	pwszDSN=wcsstr(pwszTemp, L"=");
	pwszTemp3=wcstok(pwszTemp2, L"=");

	//if this is using a DSNless connection
	if (!wcscmp(pwszTemp3,L"PROVIDERSTRING"))
	{
	}
	else
	{
		pwszInitODBC = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pwszInitString)+1)*sizeof(WCHAR));
		wcscpy(pwszInitODBC, m_pwszInitString);
		pToken=wcstok(pwszInitODBC, L";"); // Always returns first string
		pwszDSN=wcsstr(pToken, L"=");
		if (!pwszDSN)
			goto CLEANUP;
		pwszDSN++;

		// UID
		pToken=wcstok(NULL, L";");
		if (!pToken)
			goto CLEANUP;
		pwszUID=wcsstr(pToken, L"=");
		if (!pwszUID)
			goto CLEANUP;
		pwszUID++;

		// PWD
		pToken=wcstok(NULL, L";");
		if (!pToken)
			goto CLEANUP;
		pwszPWD=wcsstr(pToken, L"=");
		if (!pwszPWD)
			goto CLEANUP;
		pwszPWD++;
	}

	if (eState != STATE_INTI_ALREADY_UNINIT)
	{
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv))
			goto CLEANUP;

		if (SQL_SUCCESS != SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)Value, SQL_IS_UINTEGER))
			goto CLEANUP;
		if (eState == STATE_NO_DSO)
		{
			fReturn=TRUE;
			goto CLEANUP;
		}

		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc))
			goto CLEANUP;
	}	
	
	if (eState == STATE_UNINITIALIZED_DSO)
	{
		fReturn=TRUE;
		goto CLEANUP;
	}

	//if this is using a DSNless connection
	if (!wcscmp(pwszTemp3,L"PROVIDERSTRING"))
	{
		pwszTemp3=wcstok((pwszDSN+2), L"'");
		rc = SQLDriverConnectW(m_hdbc, NULL, pwszTemp3, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	}
	else
	{
		rc = SQLConnectW(m_hdbc, pwszDSN, SQL_NTS, pwszUID, SQL_NTS, pwszPWD, SQL_NTS);
	}
		

	if (!SQL_SUCCEEDED(rc))
		goto CLEANUP;
	
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt))
	{
		goto CLEANUP;
	}

	if (eState == STATE_INITIALIZED_DSO)
	{
		fReturn=TRUE;
		goto CLEANUP;
	}

CLEANUP:
	SAFE_FREE(pwszTemp)
	SAFE_FREE(pwszTemp2)
	SAFE_FREE(pwszInitODBC)
	return fReturn;
}

//---------------------------------------------------------------------------
//	CKageraTest::InitOLEDB
//
//	@mfunc	STATE_ENUM	eState				
//
// 
//  this function brings Set OLEDB to the desired eState
//---------------------------------------------------------------------------
BOOL CKageraTest::InitOLEDB(enum STATE_ENUM eState)
{
	BOOL			fResults		= FALSE;
	HRESULT			hr				= NOERROR;

	if (eState != STATE_INTI_ALREADY_UNINIT)
	{
		// Get our initial connection to the provider, asking for IDBInitialize since 
		// we must initialize before anything else
//		if (!SUCCEEDED(hr = CoCreateInstance(	m_pThisTestModule->m_ProviderClsid,
//															NULL,
//															m_pThisTestModule->m_clsctxProvider,
//															IID_IDBInitialize,
//															(void **)&m_pIDBInitialize)))
		if(!SUCCEEDED(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&m_pIDBInitialize)))
			goto CLEANUP;
		
		// Get IDBProperties Pointer
		if (FAILED(m_pIDBInitialize->QueryInterface(IID_IDBProperties,(void **)&m_pIDBProperties)))
			goto CLEANUP;	
	}
	if (eState == STATE_UNINITIALIZED_DSO)
	{
		fResults=TRUE;
		goto CLEANUP;
	}

	// Set the properties before we Initialize
	if (FAILED(m_pIDBProperties->SetProperties(m_cPropSets, m_rgPropSets)))
		goto CLEANUP;	
	// Initialize (connect)
	m_pThisTestModule->m_pError->Validate((hr = m_pIDBInitialize->Initialize()), 
										LONGSTRING(__FILE__), __LINE__,S_OK);

	if (!SUCCEEDED(hr))
	{
		odtLog << wszInitializeFailed;	
		goto CLEANUP;
	}
	if (eState == STATE_INITIALIZED_DSO || eState == STATE_INTI_ALREADY_UNINIT)
	{
		fResults=TRUE;
		goto CLEANUP;
	}
	// Requested state was invalid
	fResults=FALSE; 

CLEANUP:
	if (!fResults)
	{
		// Release the Data Source object we created since we've failed
		if (m_pThisTestModule->m_pIUnknown)
			m_pThisTestModule->m_pError->Compare(m_pThisTestModule->m_pIUnknown->Release()==0, LONGSTRING(__FILE__), __LINE__);
	}
	
	return fResults;
}

//---------------------------------------------------------------------------
//	CKageraTest::GetMeToState
//
//	@mfunc	STATE_ENUM	eState				
//
//  Set ODBC and OLEDB to the desired eState
//---------------------------------------------------------------------------
BOOL CKageraTest::GetMeToState(enum STATE_ENUM	eState,
							   SQLPOINTER		Value)
{
	// Initialize OLE DB to the desired state
	if (!InitOLEDB(eState))
		return FALSE;

	// Initialize the ODBC driver to the desired state
	if (!InitODBC(eState,(SQLPOINTER)Value))
		return FALSE;

	// If both initializations were successful then we've achieved the 
	// desired state.
	m_eState=eState;

	return TRUE;
}


//---------------------------------------------------------------------------
//	CKageraTest::FreeState
//
//	@mfunc	STATE_ENUM	eState				
//
//  Set ODBC and OLEDB to the desired eState
//---------------------------------------------------------------------------
BOOL CKageraTest::FreeState(enum STATE_ENUM eState)
{
	if (eState == STATE_INITIALIZED_DSO)
	{
		CHECK(m_pIDBInitialize->Uninitialize(), S_OK);
		COMPARE(SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt), SQL_SUCCESS);
		COMPARE(SQLDisconnect(m_hdbc), SQL_SUCCESS);
	}
	SAFE_RELEASE(m_pIDBInitialize)
	SAFE_RELEASE(m_pIDBProperties)

	COMPARE(SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc), SQL_SUCCESS);
	COMPARE(SQLFreeHandle(SQL_HANDLE_ENV, m_henv), SQL_SUCCESS);

	return TRUE;
}


// Test the given GetInfo value
TESTRESULT CKageraTest::TestGetInfo(enum GETINFO_ENUM eInfoValue)
{
	return TEST_PASS;
}



//---------------------------------------------------------------------------
//	CKageraTest::Init
//				
//  Init this test module
//---------------------------------------------------------------------------
BOOL CKageraTest::Init()
{
	m_cPropSets	 = 0;
	m_rgPropSets = NULL;

	// Setup the arrays needed for good init, based on string TMD passed to us
	TESTC(GetInitProps(&m_cPropSets, &m_rgPropSets));
	return TRUE;

CLEANUP:
	return FALSE;
}

//---------------------------------------------------------------------------
//	CKageraTest::Terminate
//				
//  Terminate this test module
//---------------------------------------------------------------------------
BOOL CKageraTest::Terminate()
{
	// Clean up our variants we used in the init
	FreeProperties(&m_cPropSets,&m_rgPropSets);
	return CTestCases::Terminate();
}

//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	CKageraTest::GetRowsetAndAccessor
(	
	EQUERY				eSQLStmt,				//the SQL Statement to create
	ULONG				cProperties,			//the count of properties
	const DBPROPID		*rgProperties,			//the array of properties to be set
	DBPROPSTATUS		dwPropStatus,			//rgProperties status
	ULONG				cPropertiesUnset,		//the count of properties to be unset
	const DBPROPID		*rgPropertiesUnset,		//the array of properties to be unset	
	BOOL				fBindLongColumn,		//whether to bind LONG columns
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	DBORDINAL			cColsToBind,			//the count of columns to bind
	ULONG				*rgColsToBind,			//the array of column ordinals to bind
	ECOLS_MEM_PROV_OWNED eColsMemProvOwned,		//@paramopt [IN] Which columns' memory is to be owned by the provider
	DBPARAMIO			eParamIO				//@paramopt [IN] Parameter type to specify for eParmIO
)
{
	IColumnsInfo		*pIColumnsInfo	= NULL;
	IRowset				*pIRowset		= NULL;
	ULONG				cRowsObtained	= 0;
	HROW				*pHRow			= NULL;

	ULONG				cDBPropSet		= 1;
	DBPROPSET			rgDBPropSet[2];
	
	ULONG				cProp			= 0;
	
	HRESULT				hr				= S_OK;
	BOOL				fPass			= FALSE;
	BLOBTYPE			blobType;

	ULONG				ulIndex			= 0;
	ULONG				ulUpdValue		= 0;
	ULONG				ulMaxPendRows	= 0;
	ULONG				cExtraProps		= 1;
	ULONG				i				= 0;

	m_pIAccessor	= NULL;

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
		//allocate 
		rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * (cProperties + cPropertiesUnset));

		memset(rgDBPropSet[0].rgProperties,0,sizeof(DBPROP)*(cProperties + cPropertiesUnset));
		if(!rgDBPropSet[0].rgProperties)
		{
			goto CLEANUP;
		}
		//Get Rowset interface so info from RowsetInfo  can be obtainied
		// call IOpenRowset to return a Rowset		
		hr = g_pTable->CreateRowset	(
										USE_OPENROWSET,
										IID_IRowset,
										0,
										NULL,
										(IUnknown**)&pIRowset,
										NULL,
										NULL,
										NULL
									);		
		if (pIRowset)
		{
			//go through the loop to set every DB Property required
			for(i=0; i<cProperties; i++)
			{
				//if the property is supported AND
				//if the property is writeable OR the default is VARIANT_TRUE
				if( SupportedProperty(rgProperties[i],DBPROPSET_PROVIDERROWSET,g_pIDBCreateSession) &&
					(SettableProperty(rgProperties[i],DBPROPSET_PROVIDERROWSET,g_pIDBCreateSession) ||
					 GetProperty(rgProperties[i],DBPROPSET_ROWSET,pIRowset)) )
				{
					rgDBPropSet[0].rgProperties[cProp].dwPropertyID   = rgProperties[i];
					rgDBPropSet[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_REQUIRED;

					rgDBPropSet[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
					rgDBPropSet[0].rgProperties[cProp].colid			=DB_NULLID;
					V_BOOL(&rgDBPropSet[0].rgProperties[cProp].vValue)= VARIANT_TRUE;

					cProp++;
				}
				else
				{
					odtLog<<L"A property neccessary to execute this variation was not settable.\n";
					fPass=FALSE;
					goto CLEANUP;
				}
			}
		}
		else
		{
				odtLog<<L"this provider is useless.\n";
				goto CLEANUP;
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
				odtLog<<L"A property neccessary to execute this variation was not settable.\n";
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
					odtLog<<L"A property neccessary to execute this variation was not settable.\n";
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

	//this has to be DBPROPSET_PROVIDERROWSET because the test is using provider specific commands
	rgDBPropSet[0].guidPropertySet	= DBPROPSET_PROVIDERROWSET;

	if(!SUCCEEDED(SetRowsetProperties(rgDBPropSet, cDBPropSet)))
		goto CLEANUP;

	//create the rowset object
	//May fail due to combinations of properties
	if(m_pIDBCreateCommand==NULL && (eSQLStmt== SELECT_ORDERBYNUMERIC 
									|| eSQLStmt == SELECT_REVCOLLIST
									|| eSQLStmt == SELECT_COLLISTFROMTBL))
	{
		eSQLStmt = SELECT_ALLFROMTBL;
	}

	hr = CreateRowsetObject(eSQLStmt,IID_IRowset,EXECUTE_IFNOERROR);
	if(hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED)
	{
		for (ulIndex=0;ulIndex<rgDBPropSet[0].cProperties;ulIndex++)
		{
			//if multiple props were set they might be conflicting on some providers
			if (DB_E_ERRORSOCCURRED		==	hr												&&
				DBPROPSTATUS_CONFLICTING==	rgDBPropSet[0].rgProperties[ulIndex].dwStatus	&&
				1						<	rgDBPropSet[0].cProperties)
			{
				odtLog<<L"Conflict.\n";
			}
		}
		goto CLEANUP;
	}
	
	TESTC_(hr,S_OK);
		
	//get the IRowset  pointer  
	TESTC_(m_pIAccessor->QueryInterface(IID_IRowset,(LPVOID *)&m_pIRowset),S_OK);
			
	// get the columns infomation
	TESTC_(m_pIAccessor->QueryInterface(IID_IColumnsInfo, (LPVOID *)&pIColumnsInfo),S_OK);
	TESTC_(pIColumnsInfo->GetColumnInfo(&m_cRowsetCols,&m_rgInfo, &m_pStringsBuffer),S_OK);

	//can not create an accessor on a rowset if no IRowset is present
	if(!m_pIRowset)
	{
		goto CLEANUP;
	}

	//create an accessor on the rowset
	hr	= GetAccessorAndBindings(m_pIAccessor,dwAccessorFlags,&m_hAccessor,
									&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
									eColsByRef,NULL,NULL,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *)rgColsToBind,
									NULL,eColsMemProvOwned,eParamIO,blobType);

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);
	if(!m_pData)
	{
		goto CLEANUP;
	}
	fPass=TRUE;
CLEANUP:
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
	return fPass;
}

//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
void CKageraTest::ReleaseRowsetAndAccessor()
{
	IAccessor *pIAccessor = NULL;

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize=0;
	m_cBinding=0;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);

	//free accessor handle, if a rowset Accessor
	if(m_hAccessor && m_pIAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
		m_hAccessor=NULL;
	}
	
	//free accessor handle, if a command Accessor
	if(m_hAccessor && m_pICommand)
	{
		//QI for the accessor handle on the command object
		if(CHECK(m_pICommand->QueryInterface(IID_IAccessor, (void**)&pIAccessor),S_OK))
		{
			CHECK(pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
		}
	}

	//Release accessors
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(m_pIAccessor);
	m_hAccessor=NULL;
	
	//release IRowset pointer
	SAFE_RELEASE(m_pIRowset);

	//free binding structure
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgInfo);
	PROVIDER_FREE(m_pStringsBuffer);

	ReleaseRowsetObject(0);
	ReleaseCommandObject(0);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(DBPROPSET_KAGPROP_GETINFO_Unititialized)
//--------------------------------------------------------------------
// @class Test DBPROPSET_KAGPROP_GETINFO when unitialized
//
class DBPROPSET_KAGPROP_GETINFO_Unititialized : public CKageraTest {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DBPROPSET_KAGPROP_GETINFO_Unititialized,CKageraTest);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SQL_ACCESSIBLE_PROCEDURES
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(DBPROPSET_KAGPROP_GETINFO_Unititialized)
#define THE_CLASS DBPROPSET_KAGPROP_GETINFO_Unititialized
BEG_TEST_CASE(DBPROPSET_KAGPROP_GETINFO_Unititialized, CKageraTest, L"Test DBPROPSET_KAGPROP_GETINFO when unitialized")
	TEST_VARIATION(1,		L"SQL_ACCESSIBLE_PROCEDURES")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(DBPROPSET_KAGPROP_GETINFO_Initialized)
//--------------------------------------------------------------------
// @class Test GetInfo property when initialized
//
class DBPROPSET_KAGPROP_GETINFO_Initialized : public CKageraTest {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DBPROPSET_KAGPROP_GETINFO_Initialized,CKageraTest);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SQL_ACCESSIBLE_PROCEDURES
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(DBPROPSET_KAGPROP_GETINFO_Initialized)
#define THE_CLASS DBPROPSET_KAGPROP_GETINFO_Initialized
BEG_TEST_CASE(DBPROPSET_KAGPROP_GETINFO_Initialized, CKageraTest, L"Test GetInfo property when initialized")
	TEST_VARIATION(1,		L"SQL_ACCESSIBLE_PROCEDURES")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(DriverErrors)
//--------------------------------------------------------------------
// @class Test DriverErrors 
//
class TCDriverErrors : public CKageraTest {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDriverErrors,CKageraTest);
	// }}
 
	USHORT				m_cHeader;
	USHORT				m_cRecord;
	LCID				m_lcid;
	IDBCreateCommand	*m_pIDBCreateCommand;
	IDBCreateSession	*m_pIDBCreateSession;
	ICommandText		*m_pICommandText;
	ICommand			*m_pICommand;
	IErrorInfo			*m_pIErrorInfo;


	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	BOOL	RequestFieldsToCache
			(
				IUnknown	*pIUnknown,			// @parm [IN]	object to QI from for ISQLRequestDiagFields
				KAGREQDIAG	rgReqDiag[],		// @parm [IN]	array of fields
				WORD		cFieldCount			// @parm [IN]	number of fields in array
			);	

	BOOL	GetODBCDiag
			(
				KAGREQDIAG	*rgReqDiag,			// @parm [IN]	array of name of diag fields to cache
				KAGGETDIAG	*rgGetDiag,			// @parm [OUT]	array of info from fields requested
				HRESULT		*rgGetDiagHR,		// @parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
				enum ERROR_HANDLE_ENUM eHandle, // @parm [IN]	odbc handle error is requested on
				ULONG		cArraySize			// @parm [IN]	number of fields in array
			);

	BOOL	OleGetDiagField
			(
				KAGREQDIAG	*rgReqDiag,			// @parm [IN]	array of name of fields to get
				KAGGETDIAG	*rgGetDiag,			// @parm [OUT]	array of info from fields requested
				HRESULT		*rgGetDiagHR,		// @parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
				ULONG		cArraySize			// @parm [IN]	number of fields in array
			);

	BOOL	CauseErrorThroughODBC
			(
				ERROR_HANDLE_ENUM	eHandle		// @parm [IN]	odbc handle error should be requested on
			);

	BOOL	CauseErrorThroughOLEDB
			(
				ERROR_HANDLE_ENUM	eHandle,	// @parm [IN]	odbc handle error should be requested on
				ERROR_NUMBER		eNum		// @parm [IN]	used to determine which error to cause
			);

	BOOL	CreateCommandObject();

	BOOL	FreeCommandObject();


	// {{ TCW_TESTVARS()
	//@cmember HDBC Single Err - Request All, Get All
	int Variation_1();
	//@cmember HDBC Single Err - Request None, Get All
	int Variation_2();
	//@cmember HDBC Single Err - Request All (Header only), Get All
	int Variation_3();
	//@cmember HDBC Single Err - Request All (Record only), Get All
	int Variation_4();
	//@cmember HDBC Single Err - Request One Header where the diag indetifier
	//							returns SQL_ERROR on the error, Get Header,
	//							expect to see no diag custom error object
	//							because no error was produced
	int Variation_5();
	//@cmember HDBC Single Err - Request One Record where the diag indetifier
	//							returns SQL_ERROR on the error, Get Record,
	//							expect to see no diag custom error object
	//							because no error was produced
	int Variation_6();
	//@cmember HDBC Single Err - Request One Header where the diag indetifier
	//							returns SQL_SUCCESS on the error, Get more than one header
	int Variation_7();
	//@cmember HDBC Single Err - Request One Record where the diag indetifier
	//							returns SQL_SUCCESS on the error, Get more than one record
	int Variation_8();
	//@cmember HDBC Multiple Err - Request All, Get All
	int Variation_9();
	//@cmember HDBC Multiple (Stress) Err - Request All, Get All - Stress errors on HDBC
	int Variation_10();
	//@cmember HDBC Single Err - Request All, Get All
	int Variation_11();
	//@cmember HSTMT Single Err - Request None, Get All
	int Variation_12();
	//@cmember HSTMT Single Err - Request All (Header only), Get All
	int Variation_13();
	//@cmember HSTMT Single Err - Request All (Record only), Get All
	int Variation_14();
	//@cmember HSTMT Single Err - Request One Header (all header fields 
	//							pass on a HSTMT), Get Header,
	//							expect to see diag custom error object

	int Variation_15();
	//@cmember HSTMT Single Err - Request One Record (all record fields 
	//							pass on a HSTMT), Get record,
	//							expect to see diag custom error object
	//							

	int Variation_16();
	//@cmember HSTMT Single Err - Request One Header where the diag indetifier
	//							returns SQL_SUCCESS on the error, Get more than one header
	int Variation_17();
	//@cmember HSTMT Single Err - Request One Header where the diag indetifier
	//							returns SQL_SUCCESS on the error, Get more than one record 
	int Variation_18();
	//@cmember HSTMT Multiple Err - Request All, Get All
	int Variation_19();
	//@cmember HSTMT Multiple (Stress) Err - Request All, Get All - Stress errors on HSTMT
	int Variation_20();
	//@cmember Multiple Mixed Handle Err - Request All, Get All
	int Variation_21();
	//@cmember INVALID ARG RequestDiagField
	int Variation_22();
	//@cmember INVALID ARG GetDiagField 
	int Variation_23();
	// }}
};
// {{ TCW_TESTCASE(TCDriverErrors)
#define THE_CLASS TCDriverErrors
BEG_TEST_CASE(TCDriverErrors, CKageraTest, L"Test Driver Errors")

	TEST_VARIATION(1,		L"HDBC Single Err - Request All, Get All")
	TEST_VARIATION(2,		L"HDBC Single Err - Request None, Get All")
	TEST_VARIATION(3,		L"HDBC Single Err - Request All (Header only), Get All")
	TEST_VARIATION(4,		L"HDBC Single Err - Request All (Record only), Get All")
	TEST_VARIATION(5,		L"HDBC Single Err - Request One Header, indetifier returns SQL_ERROR on error")
	TEST_VARIATION(6,		L"HDBC Single Err - Request One Record, indetifier returns SQL_ERROR on error")
	TEST_VARIATION(7,		L"Request One Header where indetifier returns SQL_SUCCESS on error, Get more than one header")
	TEST_VARIATION(8,		L"Request One Record where indetifier returns SQL_SUCCESS on error, Get more than one record")
	TEST_VARIATION(9,		L"HDBC Multiple Err - Request All, Get All")
	TEST_VARIATION(10,		L"HDBC Multiple (Stress) Err - Request All, Get All - Stress errors on HDBC")
	TEST_VARIATION(11,		L"HSTMT Single Err - Request All, Get All")
	TEST_VARIATION(12,		L"HSTMT Single Err - Request None, Get All")
	TEST_VARIATION(13,		L"HSTMT Single Err - Request All (Header only), Get All")
	TEST_VARIATION(14,		L"HSTMT Single Err - Request All (Record only), Get All")
	TEST_VARIATION(15,		L"HSTMT Single Err - Request One Header")
	TEST_VARIATION(16,		L"HSTMT Single Err - Request One Record")
	TEST_VARIATION(17,		L"Request One Header where indetifier returns SQL_SUCCESS on error, Get more than one header")
	TEST_VARIATION(18,		L"Request One Record where indetifier returns SQL_SUCCESS on error, Get more than one record")
	TEST_VARIATION(19,		L"HDBC Multiple Err - Request All, Get All")
	TEST_VARIATION(20,		L"HSTMT Multiple (Stress) Err - Request All, Get All - Stress errors on HDBC")
	TEST_VARIATION(21,		L"Multiple Mixed Handle Err - Request All, Get All")
	TEST_VARIATION(22,		L"INVALID ARG RequestDiagField")
	TEST_VARIATION(23,		L"INVALID ARG GetDiagField")

END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// }} END_DECLARE_TEST_CASES()


// {{ TCW_TEST_CASE_MAP(TCKAGPROPS)
//--------------------------------------------------------------------
// @class Test DBPROPSET_KAGPROP_GETINFO when unitialized
//
class TCKAGPROPS : public CKageraTest {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCKAGPROPS,CKageraTest);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember BLOBSONFOCURSOR OPTIONAL
	int Variation_1();
	// @cmember BLOBSONFOCURSOR REQUIRED
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCKAGPROPS)
#define THE_CLASS TCKAGPROPS
BEG_TEST_CASE(TCKAGPROPS, CKageraTest, L"Test different Kagera specific properties")
	TEST_VARIATION(1,		L"BLOBSONFOCURSOR OPTIONAL")
	TEST_VARIATION(2,		L"BLOBSONFOCURSOR REQUIRED")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(TCKAGADHOC)
//--------------------------------------------------------------------
// @class Test adhoc
//
class TCKAGADHOC : public CKageraTest {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCKAGADHOC,CKageraTest);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember select from large row
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(TCKAGPROPS)
#define THE_CLASS TCKAGADHOC
BEG_TEST_CASE(TCKAGADHOC, CKageraTest, L"Test bugs found through adhoc scenarios")
	TEST_VARIATION(1,		L"select from large row")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1,	DBPROPSET_KAGPROP_GETINFO_Unititialized)
	TEST_CASE(2,	DBPROPSET_KAGPROP_GETINFO_Initialized)
	TEST_CASE(3,	TCDriverErrors)
	TEST_CASE(4,	TCKAGPROPS)
	TEST_CASE(5,	TCKAGADHOC)
END_TEST_MODULE()
// }}




// {{ TCW_TC_PROTOTYPE(DBPROPSET_KAGPROP_GETINFO_Unititialized)
//*-----------------------------------------------------------------------
//|	Test Case:		DBPROPSET_KAGPROP_GETINFO_Unititialized - Test DBPROPSET_KAGPROP_GETINFO when unitialized
//|	Created:			03/29/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DBPROPSET_KAGPROP_GETINFO_Unititialized::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	return CKageraTest::Init();
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SQL_ACCESSIBLE_PROCEDURES
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DBPROPSET_KAGPROP_GETINFO_Unititialized::Variation_1()
{
return TRUE;
	//return TestGetInfo(E_ACCESSIBLE_PROCEDURES);

}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DBPROPSET_KAGPROP_GETINFO_Unititialized::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CKageraTest::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DBPROPSET_KAGPROP_GETINFO_Initialized)
//*-----------------------------------------------------------------------
//|	Test Case:		DBPROPSET_KAGPROP_GETINFO_Initialized - Test GetInfo property when initialized
//|	Created:			04/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DBPROPSET_KAGPROP_GETINFO_Initialized::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	return CKageraTest::Init();
	// }}
}



// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc SQL_ACCESSIBLE_PROCEDURES
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DBPROPSET_KAGPROP_GETINFO_Initialized::Variation_1()
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
BOOL DBPROPSET_KAGPROP_GETINFO_Initialized::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CKageraTest::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(TCDriverErrors)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDriverErrors - Test Driver Errors
//|	Created:			04/16/97
//*-----------------------------------------------------------------------

//---------------------------------------------------------------------------
//	TCDriverErrors::Init
//				
//
//  TestCase Initialization Routine
//---------------------------------------------------------------------------
BOOL TCDriverErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CKageraTest::Init())
	// }}
	{
		ULONG	i	= 0;
		m_lcid = GetSystemDefaultLCID(); 

		m_cHeader=0;
		m_cRecord=0;

		//get the number of headers fields and the number of record fields (a field is one or the other)
		for (i=0;i<g_ccAllDiagFields;i++)
		{
			if (g_rgKagDiagAll[i].ulDiagFlags==KAGREQDIAGFLAGS_HEADER)
			{
				m_cHeader++;
			}
			else
			{
				m_cRecord++;
			}
		}

		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::RequestFieldsToCache 
//				
//	@parm [IN]	object to QI from for ISQLRequestDiagFields
//	@parm [IN]	array of fields
//	@parm [IN]	number of fields in array
// 
//  this calls RequestDiagFields to request which diag identifier field OLDB
//  will cache
//---------------------------------------------------------------------------

BOOL TCDriverErrors::RequestFieldsToCache
(
	IUnknown						*pIUnknown,		//	@parm [IN]	object to QI from for ISQLRequestDiagFields
	KAGREQDIAG						rgReqDiag[],	//	@parm [IN]	array of fields
	WORD							cFieldCount		//	@parm [IN]	number of fields in array
)
{
	ISQLRequestDiagFields	*pISQLRequestDiagFields	= NULL;
	BOOL					fResults				= FALSE;
	HRESULT					hr;

	if (!pIUnknown)
	{
		odtLog << L"Request To Set Error Field Failed.\n";
		goto CLEANUP;
	}
	// Get ISQLRequestDiagFields Pointer
	if (!CHECK(pIUnknown->QueryInterface(IID_ISQLRequestDiagFields,(void **)&pISQLRequestDiagFields),S_OK))
		goto CLEANUP;

	//call request of DIAG constant 
	if (FAILED(hr = pISQLRequestDiagFields->RequestDiagFields(cFieldCount,rgReqDiag)))
	{
		odtLog << L"Request To Set Error Field Failed.\n";
		goto CLEANUP;
	}	
	fResults = TRUE;

CLEANUP:
	SAFE_RELEASE(pISQLRequestDiagFields)
	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::OleGetDiagField 
//				
//	@parm [IN]	array of name of diag fields to cache
//	@parm [OUT]	array of info from fields requested
//	@parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
//	@parm [IN]	odbc handle error is requested on
//	@parm [IN]	number of fields in array
// 
//  this calls GetErrorInfo to GetDiagField to get info on the cusotm
//  error object reporitng on fields that have been cached
//---------------------------------------------------------------------------
BOOL TCDriverErrors::OleGetDiagField
(
	KAGREQDIAG	*rgReqDiag,			//	@parm [IN]	array of name of fields to get
	KAGGETDIAG	*rgGetDiag,			//	@parm [OUT]	array of info from fields requested
	HRESULT		*rgGetDiagHR,		//	@parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
	ULONG		cArraySize			//	@parm [IN]	odbc handle error is requested on
)
{
	BOOL				fResults				=	FALSE;
	BOOL				fFoundCustomErrorObject	=	FALSE;

	IErrorRecords		*pIErrorRecords			=	NULL;
	ISQLErrorInfo		*pISQLErrorInfo			=	NULL;
	IErrorInfo			*pIErrorInfo			=	NULL;

	ISQLGetDiagField	*pIGetDiagField			=	NULL;
	ISQLGetDiagField	*pIGetDiagFieldHead		=	NULL;

	HRESULT				hr;

	ULONG				ulErrorRecCount			=	0;
	ULONG				i						=	0;
	ULONG				j						=	0;	

	WCHAR				wszSQLSTATE[]			= L"00000";
	WCHAR*				wszString				= NULL ;
	BSTR				pbstrDescription		= NULL;
	LONG				lNativeError;


	//Now get our current error object
	if (!CHECK(GetErrorInfo(0, &m_pIErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!m_pIErrorInfo)
	{
		goto CLEANUP;
	}

	//Get the IErrorRecord interface 
	if (!CHECK(m_pIErrorInfo->QueryInterface(IID_IErrorRecords,
			(void **)&pIErrorRecords), S_OK))
	{
		goto CLEANUP;
	}
	if (!pIErrorRecords)
	{
		goto CLEANUP;
	}

	//Find out how many records were created
	if (!CHECK(pIErrorRecords->GetRecordCount(&ulErrorRecCount), S_OK))
	{
		goto CLEANUP;
	}

	//loop through error records and get first custom error object
	//not necessarily the 0 rec (bug 4193)
	for (i=0;i<ulErrorRecCount;i++)
	{
		hr=pIErrorRecords->GetCustomErrorObject(i, IID_ISQLGetDiagField, (IUnknown**) &pIGetDiagField);
		if (hr==S_OK && pIGetDiagField)
		{
			//we got one
			break;
		}
		//if pIGetDiagField is NULL and valid hr then custom error object is empty
		//but there has to an error object somewhere, check to see that it is there
		if ((hr==S_OK&&!pIGetDiagField)||(hr==E_NOINTERFACE&&!pIGetDiagField))
		{
			if (!CHECK(pIErrorRecords->GetCustomErrorObject(i, IID_ISQLErrorInfo,
				(IUnknown**) &pISQLErrorInfo), S_OK)||!pISQLErrorInfo)
			{
				if (!CHECK(pIErrorRecords->GetErrorInfo(i, m_lcid, 
					(IErrorInfo**)&pIErrorInfo), S_OK)||!pIErrorInfo)
				{
					//if we are here there is not an error object
					//GetRecordCount said there was an error but there was none to get anywhere
					goto CLEANUP;
				}
			}
			//free if we just got an interface, we were just checking it was there
			SAFE_RELEASE(pISQLErrorInfo)
			SAFE_RELEASE(pIErrorInfo)
			SAFE_RELEASE(pIGetDiagField);
			continue;
		}
		else
		{
			goto CLEANUP;
		}
	}
	
	//no fields were requested if we are here and pIGetDiagField is NULL and hr is E_NOINTERFACE
	//so return TRUE, if this if evaluates to true and fields were requested there is a error, it will show later.
	if (!pIGetDiagField&&hr==E_NOINTERFACE)
	{
		fResults=TRUE;
		goto CLEANUP;
	}
	//check if this is a header record or a non-header record
	if (!CHECK(pIErrorRecords->GetCustomErrorObject(i, IID_ISQLErrorInfo,
		(IUnknown**) &pISQLErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!CHECK(pISQLErrorInfo->GetSQLInfo(&pbstrDescription,&lNativeError), S_OK))
	{
		goto CLEANUP;
	}
	//if this custom error object is a header record then the SQLSTATE will be 00000
	wszString = BSTR2WSTR(pbstrDescription);
	if(!wcscmp(wszSQLSTATE,wszString))
	{
		if (pbstrDescription)
		{
			SysFreeString(pbstrDescription);
			pbstrDescription=NULL;
			PROVIDER_FREE(wszString);
		}
		SAFE_RELEASE(pISQLErrorInfo)
		//point to this interface with pIHead, and go get the record object
		pIGetDiagFieldHead=pIGetDiagField;
		pIGetDiagField=NULL;

		//since that was head the next custom errror object will be a record
		//check for record object  (if there is one, there doens't have top be if only heads were cached)
		for (j=(i+1);j<ulErrorRecCount;j++)
		{
			if (!CHECK(pIErrorRecords->GetCustomErrorObject(j, IID_ISQLGetDiagField, 
				(IUnknown**) &pIGetDiagField), S_OK) || !pIGetDiagField)
			{
				continue;
			}
			break;
		}
	}
	else
	{
		//if we are here we will have to use the following again so free'em
		if (pbstrDescription)
		{
			SysFreeString(pbstrDescription);
			pbstrDescription=NULL;
			PROVIDER_FREE(wszString);
		}
		SAFE_RELEASE(pISQLErrorInfo)
		//check for header object  (if there is one)
		for (j=(i+1);j<ulErrorRecCount;j++)
		{
			if (!CHECK(pIErrorRecords->GetCustomErrorObject(j, IID_ISQLGetDiagField, 
				(IUnknown**) &pIGetDiagFieldHead), S_OK) || !pIGetDiagFieldHead)
			{
				continue;
			}
			break;
		}
		//if there is an interface
		if (pIGetDiagFieldHead)
		{
			//check if this is a header object or a record object, (it has to be header)
			if (!CHECK(pIErrorRecords->GetCustomErrorObject(j, IID_ISQLErrorInfo,
				(IUnknown**) &pISQLErrorInfo), S_OK))
			{
				goto CLEANUP;
			}
			if (!CHECK(pISQLErrorInfo->GetSQLInfo(&pbstrDescription,&lNativeError), S_OK))
			{
				goto CLEANUP;
			}
			//if this custom error object is a header record then the SQLSTATE will be 00000
			//if it is not a header object then drop it it is an illrelavent custom error object			
			wszString = BSTR2WSTR(pbstrDescription);
			if(wcscmp(wszSQLSTATE,wszString))
			{
				SAFE_RELEASE(pIGetDiagFieldHead)
			}
			if (pbstrDescription)
			{
				SysFreeString(pbstrDescription);
				pbstrDescription=NULL;
				PROVIDER_FREE(wszString);
			}
			SAFE_RELEASE(pISQLErrorInfo)
		}
	}
	//loop through the array of DIAG constants
	for (i=0;i<cArraySize;i++)
	{
		VariantInit(&(rgGetDiag[i].vDiagInfo));
		rgGetDiag[i].ulSize = sizeof(rgGetDiag[i]);
		rgGetDiag[i].sDiagField = rgReqDiag[i].sDiagField;

		if (rgReqDiag[i].ulDiagFlags==KAGREQDIAGFLAGS_HEADER)
		{
			//if test requested a header object then there has to be a header object
			if (!pIGetDiagFieldHead)
			{
				goto CLEANUP;
			}
			if (FAILED(rgGetDiagHR[i] = pIGetDiagFieldHead->GetDiagField(&rgGetDiag[i])))
			{
				goto CLEANUP;
			}
		}
		else
		{
			//if test requested a record object then there has to be a record object
			if (!pIGetDiagField)
			{
				goto CLEANUP;
			}
			if (FAILED(rgGetDiagHR[i] = pIGetDiagField->GetDiagField(&rgGetDiag[i])))
			{
				goto CLEANUP;
			}
		}
	}
	fResults = TRUE;

CLEANUP:
	if (pbstrDescription)
	{
		SysFreeString(pbstrDescription);
		pbstrDescription=NULL;
	}
	SAFE_RELEASE(pIGetDiagField)
	SAFE_RELEASE(pIGetDiagFieldHead)
	SAFE_RELEASE(pISQLErrorInfo)
	SAFE_RELEASE(pIErrorInfo)
	SAFE_RELEASE(pIErrorRecords)
	SAFE_RELEASE(m_pIErrorInfo)

	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::OleGetDiagField 
//				
//	@parm [IN]	array of name of diag fields to cache
//	@parm [OUT]	array of info from fields requested
//	@parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
//	@parm [IN]	odbc handle error is requested on
//	@parm [IN]	number of fields in array
// 
//	determine which handle the error is on loop through the KAGREQDIAG fields 
//  fill them from direct calls to ODBC
//---------------------------------------------------------------------------
BOOL TCDriverErrors::GetODBCDiag
(
	KAGREQDIAG	*rgReqDiag,			//	@parm [IN]	array of name of fields to get
	KAGGETDIAG	*rgGetDiag,			//	@parm [OUT]	array of info from fields requested
	HRESULT		*rgGetDiagHR,		//	@parm [OUT]	array corresponding to KAGGETDIAG saying if the field has any info for the error
	enum ERROR_HANDLE_ENUM eHandle, //	@parm [IN]	odbc handle error is requested on
	ULONG		cArraySize			//	@parm [IN]	number of fields in array
)
{
	SQLLEN		wDiagInfo		= 0;
	TCHAR       szDiagInfo[STRINGSIZE];
	WCHAR		wszDiagInfo[STRINGSIZE];
	SQLSMALLINT	wInfoOctetCount	= 0;
	WORD		cIndex			= 0;

	BOOL		fResults		= FALSE;
	SQLRETURN	rc;
	SHORT		cRecNumber		= 0;

	//loop through fields requested	
	for (cIndex=0;cIndex<cArraySize;cIndex++)
	{
		//headers are found on record zero
		if (rgReqDiag[cIndex].ulDiagFlags==KAGREQDIAGFLAGS_HEADER)
		{
			cRecNumber=0;
		}
		else
		{
			cRecNumber=1;
		}
		VariantInit(&rgGetDiag[cIndex].vDiagInfo);
		V_VT(&rgGetDiag[cIndex].vDiagInfo)	= rgReqDiag[cIndex].vt;
		rgGetDiag[cIndex].sDiagField		= rgReqDiag[cIndex].sDiagField;
		//if the diag info requested returns a string
		if (rgReqDiag[cIndex].vt == VT_BSTR)
		{
			switch (eHandle)
			{
				case eHENV:
					rc = SQLGetDiagField(	SQL_HANDLE_ENV,
											m_henv,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&szDiagInfo,
											STRINGSIZE,
											&wInfoOctetCount);
					break;
				case eHDBC:
					rc = SQLGetDiagField(	SQL_HANDLE_DBC,
											m_hdbc,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&szDiagInfo,
											STRINGSIZE,
											&wInfoOctetCount);
					break;
				case eHSTMT:
					rc = SQLGetDiagField(	SQL_HANDLE_STMT,
											m_hstmt,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&szDiagInfo,
											STRINGSIZE,
											&wInfoOctetCount);
					break;
				case eHDESC:
					rc = SQLGetDiagField(	SQL_HANDLE_DESC,
											m_hdesc,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&szDiagInfo,
											STRINGSIZE,
											&wInfoOctetCount);
					break;
				default:
					break;
			}
			MultiByteToWideChar(	CP_ACP,
									MB_PRECOMPOSED,
									szDiagInfo,
									sizeof(szDiagInfo),
									wszDiagInfo,
									sizeof(wszDiagInfo));

			//if not success field does not apply for error
			if (rc==SQL_SUCCESS)
			{
				V_BSTR(&rgGetDiag[cIndex].vDiagInfo) = SysAllocString(wszDiagInfo);
				if (!V_BSTR(&rgGetDiag[cIndex].vDiagInfo))
				{
						VariantClear(&rgGetDiag[cIndex].vDiagInfo);
						odtLog << L"Out of memory.\n";
						goto CLEANUP;

				}
				rgGetDiagHR[cIndex]	= S_OK;
			}
			else
			{
				V_VT(&rgGetDiag[cIndex].vDiagInfo)	= 0;
				rgGetDiagHR[cIndex]					= S_FALSE;
			}
		}
		//else it returns an int
		else
		{
			switch (eHandle)
			{
				case eHENV:
					rc = SQLGetDiagField(	SQL_HANDLE_ENV,
											m_henv,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&wDiagInfo,
											0,
											NULL);
					break;
				case eHDBC:
					rc = SQLGetDiagField(	SQL_HANDLE_DBC,
											m_hdbc,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&wDiagInfo,
											0,
											NULL);
					break;
				case eHSTMT:
					rc = SQLGetDiagField(	SQL_HANDLE_STMT,
											m_hstmt,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&wDiagInfo,
											0,
											NULL);
					break;
				case eHDESC:
					rc = SQLGetDiagField(	SQL_HANDLE_DESC,
											m_hdesc,
											cRecNumber,
											rgReqDiag[cIndex].sDiagField,
											&wDiagInfo,
											0,
											NULL);
					break;
				default:
					break;
			}

			//if not success field does not apply for error
			if (rc==SQL_SUCCESS)
			{	
				switch(rgReqDiag[cIndex].vt)
				{
					case VT_I4:
						V_I4(&rgGetDiag[cIndex].vDiagInfo)	= (DWORD)wDiagInfo;
						break;
					case VT_UI4:
						V_UI4(&rgGetDiag[cIndex].vDiagInfo)	= (UDWORD)wDiagInfo;
						break;
					case VT_I2:
						V_I2(&rgGetDiag[cIndex].vDiagInfo)	= (WORD)wDiagInfo;
						break;
					case VT_UI2:
						V_UI2(&rgGetDiag[cIndex].vDiagInfo)	= (UWORD)wDiagInfo;
						break;
				}
				rgGetDiagHR[cIndex]	= S_OK;
			}
			else
			{
				V_VT(&rgGetDiag[cIndex].vDiagInfo)	= 0;
				rgGetDiagHR[cIndex]					= S_FALSE;
			}
		}
	}	
	fResults = TRUE;
CLEANUP:
	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::CauseErrorThroughODBC 
//
//	@mfunc	ERROR_HANDLE_ENUM	eHandle	// @parm [IN]	odbc handle error should be requested on
//
//	causes and error on the request eState by directly calling ODBC
//
//  this function expects the calling procedure to be in the correct state for 
//  the Handle it wants the error on
//---------------------------------------------------------------------------
BOOL TCDriverErrors::CauseErrorThroughODBC
(
	ERROR_HANDLE_ENUM	eHandle		// @parm [IN]
)
{		
	SQLRETURN		rc;

	BOOL			fResults			=	FALSE;
	WCHAR			*pwszDSN			=	NULL;
	WCHAR			*pToken				=	NULL;

	WCHAR			*pwszInitODBC		=	NULL;
	WCHAR			*pwszBadInitODBC	=	NULL;
	WCHAR			wszBadODBCInit[]	=	L"DSN=;UID=;PWD=;";
	WCHAR			wszBadSQL[]			=	L"garbage_XXXXO";

	switch (eHandle)
	{
		case eHENV:
			break;
		case eHDBC:
			//PRODUCE ODBC ERROR
			//Connect with NULL DSN, UID and NULL PWD, (error is from dm)
			rc = SQLDriverConnectW(m_hdbc, NULL, wszBadODBCInit, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
			//this should fail, stop test if it does not
			if (!rc)
			{
				COMPARE(SQLDisconnect(m_hdbc), SQL_SUCCESS);
				odtLog << L"ODBC error was not generated on Driver Connect, cannot continue with test case.\n";
				goto CLEANUP;
			}

			break;
		case eHSTMT:

			//PRODUCE ODBC ERROR
			rc = SQLExecDirectW(m_hstmt,wszBadSQL,SQL_NTS);
			//this should fail, stop test if it does not
			if (!rc)
			{
				odtLog << L"ODBC error was not generated on ExecDirect, cannot continue with test case.\n";
				goto CLEANUP;
			}
		case eHDESC:
			break;
		default:
			break;	
	}
	fResults=TRUE;

CLEANUP:
	SAFE_FREE(pwszBadInitODBC)
	SAFE_FREE(pwszInitODBC)
	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::CauseErrorThroughOLEDB 
//
//	@mfunc	ERROR_HANDLE_ENUM	eHandle	// @parm [IN]	odbc handle error should be requested on
//
//	causes and error on the request eState by calling OLEDB
//
//  this function expects the calling procedure to be in the correct state for 
//  the Handle it wants the error on
//---------------------------------------------------------------------------
BOOL TCDriverErrors::CauseErrorThroughOLEDB
(
	ERROR_HANDLE_ENUM	eHandle,		// @parm [IN]
	ERROR_NUMBER		eNum		// @parm [IN]
)
{		
	BOOL				fResults			=	FALSE;
	WORD				i					=	0;
	WCHAR				wszBadSQL[]			=	L"garbage_XXXXO";
	WCHAR				wszBadSQL2[]		=	L"insert into no_table_asifhsad values (1)";	
	HRESULT				hr;
	BSTR				bstrTempU			=	NULL;
	BSTR				bstrTempP			=	NULL;
	BSTR				bstrTempD			=	NULL;

	switch (eHandle)
	{
		case eHENV:
			break;
		case eHDBC:
			switch (eNum)
			{
				case eERRORONE:
					//PRODUCE OLEDB ERROR 1 (IM002)
					for (i=0;i<m_rgPropSets[0].cProperties;i++)
					{
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_DATASOURCE)
						{
							bstrTempD = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_USERID)
						{
							bstrTempU = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_PASSWORD)
						{
							bstrTempP = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_PROVIDERSTRING)
						{
							bstrTempP = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
					}
					break;
				case eERRORTWO:
					//PRODUCE OLEDB ERROR 1 (28000)
					for (i=0;i<m_rgPropSets[0].cProperties;i++)
					{
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_USERID)
						{
							bstrTempU = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_PASSWORD)
						{
							bstrTempP = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_PROVIDERSTRING)
						{
							bstrTempP = SysAllocStringLen(	m_rgPropSets[0].rgProperties[i].vValue.bstrVal,
															(UINT)wcslen(m_rgPropSets[0].rgProperties[i].vValue.bstrVal));
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal	= SysAllocStringLen(L"",(UINT)wcslen(L""));
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
					}
					break;
				default:
					goto CLEANUP;			
			}
			// Set the properties before we Initialize 
			if (FAILED(m_pIDBProperties->SetProperties(m_cPropSets, m_rgPropSets)))
			{
				goto CLEANUP;
			}
			//bad Initialize (connect)
			hr = m_pIDBInitialize->Initialize();
			// Make sure we didn't succeed
			if (hr == ResultFromScode(S_OK))
			{		
				CHECK(m_pIDBInitialize->Uninitialize(), S_OK);
				odtLog << L"ODBC error was not generated by OLEDB initialization, cannot continue with test case.\n";
				//set to true because kagera can only generate one error on acess hdbc
				fResults=TRUE;
				goto CLEANUP;
			}
			switch (eNum)
			{
				case eERRORONE:
					//switch prop setting back so init won't cause  OLEDB ERROR 1 (IM002)
					for (i=0;i<m_rgPropSets[0].cProperties;i++)
					{
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_DATASOURCE)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempD,(UINT)wcslen(bstrTempD));
							SysFreeString(bstrTempD);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_USERID)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempU,(UINT)wcslen(bstrTempU));
							SysFreeString(bstrTempU);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_PASSWORD)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempP,(UINT)wcslen(bstrTempP));
							SysFreeString(bstrTempP);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_PROVIDERSTRING)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempP,(UINT)wcslen(bstrTempP));
							SysFreeString(bstrTempP);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
					}
					break;
				case eERRORTWO:
					//switch prop setting back so init won't cause OLEDB ERROR 1 (28000)
					for (i=0;i<m_rgPropSets[0].cProperties;i++)
					{
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_USERID)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempU,(UINT)wcslen(bstrTempU));
							SysFreeString(bstrTempU);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_AUTH_PASSWORD)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempP,(UINT)wcslen(bstrTempP));
							SysFreeString(bstrTempP);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
						if (m_rgPropSets[0].rgProperties[i].dwPropertyID==DBPROP_INIT_PROVIDERSTRING)
						{
							SysFreeString(m_rgPropSets[0].rgProperties[i].vValue.bstrVal);
							m_rgPropSets[0].rgProperties[i].vValue.bstrVal = SysAllocStringLen(bstrTempP,(UINT)wcslen(bstrTempP));
							SysFreeString(bstrTempP);
							if (!m_rgPropSets[0].rgProperties[i].vValue.bstrVal)
							{
								odtLog << L"Out of memory.\n";
								goto CLEANUP;
							}
						}
					}
					break;
				default:
					goto CLEANUP;			
			}
			break;
		case eHSTMT:
			switch (eNum)
			{
				case eERRORONE:
					if (!SUCCEEDED(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL,wszBadSQL)))
					{
						goto CLEANUP;
					}
					break;
				case eERRORTWO:
					if (!SUCCEEDED(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL,wszBadSQL2)))
					{
						goto CLEANUP;
					}
					break;
				default:
					goto CLEANUP;
			}
			//this works cause 	pICommandText is a child of pICommand
			if (!FAILED(hr = m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL)))
			{
				odtLog << L"ODBC error was not generated by OLEDB execute, cannot continue with test case.\n";
				goto CLEANUP;
			}
			break;
		case eHDESC:
			break;
		default:
			break;	
	}
	fResults=TRUE;

CLEANUP:
	// Clean up our variants we used in the init
	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::CreateCommandObject 
//
//	@mfunc	
//
//  gets a command object
//---------------------------------------------------------------------------
BOOL TCDriverErrors::CreateCommandObject
(
)
{
	BOOL		fResults=FALSE;
	HRESULT		hr;

	m_pIDBCreateSession		= NULL;
	m_pIDBCreateCommand		= NULL;
	m_pICommand				= NULL;
	m_pICommandText			= NULL;

	//GetCommandObject
	if (!SUCCEEDED(hr = m_pIDBInitialize->QueryInterface(	IID_IDBCreateSession, 
												(void**)&m_pIDBCreateSession)))
	{
		goto CLEANUP;
	}
	//start SESSION and while i'm there get a (COMMAND) CreateCommand interface
	if (!SUCCEEDED(hr = m_pIDBCreateSession->CreateSession(	NULL, 
															IID_IDBCreateCommand, 
															(IUnknown **)&m_pIDBCreateCommand)))
	{
		goto CLEANUP;       
	}	
	if (!SUCCEEDED(hr = m_pIDBCreateCommand->CreateCommand(	NULL,
															IID_ICommand,
															(IUnknown**)&m_pICommand)))
	{
		goto CLEANUP;
	}
	if (!SUCCEEDED(hr = m_pICommand->QueryInterface(IID_ICommandText, (void **)&m_pICommandText)))
	{
		goto CLEANUP;
	}
	fResults=TRUE;
CLEANUP:
	return fResults;
}

//---------------------------------------------------------------------------
//	TCDriverErrors::FreeCommandObject 
//
//	@mfunc	
//
//  frees a command object
//---------------------------------------------------------------------------
BOOL TCDriverErrors::FreeCommandObject
(
)
{		
	SAFE_RELEASE(m_pICommandText)
	SAFE_RELEASE(m_pICommand)
	SAFE_RELEASE(m_pIDBCreateCommand)
	SAFE_RELEASE(m_pIDBCreateSession)

	return TRUE;	
}

	
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request All, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_1()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}

	fResult = fTemp;

CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request None, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_2()
{
	BOOL				fResult=FALSE;

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,NULL,0))
	{
		goto CLEANUP;
	}
	//do it twice to make sure it is done one when the cache is empty
	if (!RequestFieldsToCache(m_pIDBInitialize,NULL,0))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//TRY to Get all OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//nothing to look at
	fResult=TRUE;
CLEANUP:
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request All (Header only), Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_3()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGREQDIAG			*rgKagDiag = (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*m_cHeader);

	KAGGETDIAG			*rgGetDiagODBC	= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cHeader);
	HRESULT				*rgGetDiagHRODBC= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cHeader);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cHeader);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cHeader);
	
	ULONG				i;

	//get just the header fields
	for (i=0;i<m_cHeader;i++)
	{
		rgKagDiag[i]=g_rgKagDiagAll[i];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,m_cHeader))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],m_cHeader))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],m_cHeader))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<m_cHeader;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request All (Record only), Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_4()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*m_cRecord);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cRecord);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cRecord);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cRecord);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cRecord);
	
	ULONG				i;
	ULONG				j;

	//get just the record fields
	for (i=0;i<m_cRecord;i++)
	{
		j = i + (m_cHeader);
		rgKagDiag[i]=g_rgKagDiagAll[j];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,m_cRecord))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],m_cRecord))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],m_cRecord))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=m_cHeader;i<m_cRecord;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	for (i=0;i<m_cRecord;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request One Header where the diag indetifier
//							returns SQL_ERROR on the error, Get Header,
//							expect to see no diag custom error object
//							because no error was produced
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_5()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*1);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);
	
	ULONG				i;

	//get a header field, the field doesn't apply to the error on the HDBC here
	for (i=0;i<1;i++)
	{
		rgKagDiag[i]=g_rgKagDiagAll[i];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,1))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],1))
	{
		goto CLEANUP;
	}	
	//nothing to look at
	for (i=0;i<1;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = TRUE;

CLEANUP:
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request One Record where the diag indetifier
//							returns SQL_ERROR on the error, Get Record,
//							expect to see no diag custom error object
//							because no error was produced
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_6()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*1);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);
	
	ULONG				i;
	ULONG				j;

	//get a record field, the field doesn't apply to the error on the HDBC here
	for (i=0;i<1;i++)
	{
		j = i + (m_cHeader+1);//plus one at the end gets us to the first diag record field that fails on this hdbc
		rgKagDiag[i]=g_rgKagDiagAll[j];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,1))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],1))
	{
		goto CLEANUP;
	}	
	//nothing to compare
	for (i=0;i<1;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = TRUE;

CLEANUP:
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request One Header where the diag indetifier
//							returns SQL_SUCCESS on the error, Get more than one header rec
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_7()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	//the following fields are hard coded to be sure what fields were getting in case .h file is updated
	KAGREQDIAG			rgKagDiag[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_HEADER,	VT_BSTR,	SQL_DIAG_DYNAMIC_FUNCTION
											};
	KAGREQDIAG			rgKagDiag2[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_HEADER,	VT_I4,		SQL_DIAG_CURSOR_ROW_COUNT,
											KAGREQDIAGFLAGS_HEADER,	VT_BSTR,	SQL_DIAG_DYNAMIC_FUNCTION
											};

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag2[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,2))
	{
		goto CLEANUP;
	}

	//this might ot might not pass depending on the odbc version, 
	//if it does set it to false becuase it wasn't cached so it should fail oledb
	rgGetDiagHRODBC[0]=S_FALSE;
	rgGetDiagODBC[0].vDiagInfo.vt=0;
	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag2[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],2))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<2;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//--------------------------------------------------------------------
// @mfunc HDBC Single Err - Request One Record where the diag indetifier
//							returns SQL_SUCCESS on the error, Get more than one header rec
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_8()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	//the following fields are hard coded to be sure what fields were getting in case .h file is updated
	KAGREQDIAG			rgKagDiag[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_CONNECTION_NAME
											};
	KAGREQDIAG			rgKagDiag2[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_CONNECTION_NAME,
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_MESSAGE_TEXT
											};

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);
	
	ULONG				i;
	VARTYPE				vTemp;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag2[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,2))
	{
		goto CLEANUP;
	}
	
	//this field should pass, if it does set it to false becuase it wasn't cached so it should fail oledb
	if (rgGetDiagHRODBC[1]==S_OK)
	{
		rgGetDiagHRODBC[1]				= S_FALSE;
		vTemp							= rgGetDiagODBC[1].vDiagInfo.vt;
		rgGetDiagODBC[1].vDiagInfo.vt	= 0;
	}

	//Request DIAG constant Field (single) To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics Fields (plural)
	if (!OleGetDiagField(&rgKagDiag2[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],2))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<2;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		if (1==i)
		{
			rgGetDiagODBC[1].vDiagInfo.vt	= vTemp;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//--------------------------------------------------------------------
// @mfunc HDBC Multiple Err - Request All, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_9()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORTWO))
	{
		goto CLEANUP;
	}
	//Cause different error, this is the error that was caused in odbc above and the one we should see in oledb
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}

	fResult = fTemp;

CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//----------------------------------------------------------------------
// @mfunc HDBC Multiple (Stress) Err - Request All, Get All - Stress errors on HDBC
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_10()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;
	ULONG				ulStressCount		=	100;//0;


	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHDBC,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}

	//USE OLEDB error MUCHO times
	for (i=0;i<=ulStressCount;i++)
	{
		//Cause ODBC error on connection handle through OLEDB
		if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
		{
			goto CLEANUP;
		}
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
			fTemp=FALSE;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo)
	FreeState(STATE_UNINITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request All, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_11()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo, 
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request None, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_12()
{
	BOOL				fResult=FALSE;

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,NULL,0))
	{
		goto CLEANUP;
	}
	//do it twice to make sure it is done one when the cache is empty
	if (!RequestFieldsToCache(m_pICommand,NULL,0))
	{
 		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//TRY to Get all OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//nothing to look at
	fResult=TRUE;
CLEANUP:
	//free Command object
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request All (Header only), Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_13()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGREQDIAG			*rgKagDiag = (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*m_cHeader);

	KAGGETDIAG			*rgGetDiagODBC	= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cHeader);
	HRESULT				*rgGetDiagHRODBC= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cHeader);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cHeader);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cHeader);
	
	ULONG				i;

	//get just the header fields
	for (i=0;i<m_cHeader;i++)
	{
		rgKagDiag[i]=g_rgKagDiagAll[i];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,m_cHeader))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],m_cHeader))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],m_cHeader))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<m_cHeader;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request All (Record only), Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_14()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*m_cRecord);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cRecord);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cRecord);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*m_cRecord);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*m_cRecord);
	
	ULONG				i;
	ULONG				j;

	//get just the record fields
	for (i=0;i<m_cRecord;i++)
	{
		j = i + (m_cHeader);
		rgKagDiag[i]=g_rgKagDiagAll[j];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,m_cRecord))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],m_cRecord))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],m_cRecord))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=m_cHeader;i<m_cRecord;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	for (i=0;i<m_cRecord;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	//free Command object
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request One Header (all header fields 
//							pass on a HSTMT), Get Header,
//							expect to see diag custom error object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_15()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*1);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);
	
	ULONG				i;

	//get a header field, the field doesn't apply to the error on the HSTMT here
	for (i=0;i<1;i++)
	{
		rgKagDiag[i]=g_rgKagDiagAll[i];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,1))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],1))
	{
		goto CLEANUP;
	}	
	//nothing to look at
	for (i=0;i<1;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = TRUE;

CLEANUP:
	//free Command object
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request One Record (all record fields 
//							pass on a HSTMT), Get record,
//							expect to see diag custom error object
//							
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_16()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	KAGREQDIAG			*rgKagDiag			= (KAGREQDIAG*)PROVIDER_ALLOC(sizeof(KAGREQDIAG)*1);

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*1);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*1);
	
	ULONG				i;

	//get a record field, the field doesn't apply to the error on the HSTMT here
	for (i=0;i<1;i++)
	{
		rgKagDiag[i]=g_rgKagDiagAll[1];
	}
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,1))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],1))
	{
		goto CLEANUP;
	}	
	//nothing to compare
	for (i=0;i<1;i++)
	{
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = TRUE;
CLEANUP:
	//free Command object
	SAFE_FREE(rgKagDiag)
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(17)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request One Header where the diag indetifier
//							returns SQL_SUCCESS on the error, Get more than one header rec
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_17()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	//the following fields are hard coded to be sure what fields were getting in case .h file is updated
	KAGREQDIAG			rgKagDiag[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_HEADER,	VT_I4,	SQL_DIAG_NUMBER
											};
	KAGREQDIAG			rgKagDiag2[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_HEADER,	VT_I4,	SQL_DIAG_CURSOR_ROW_COUNT,
											KAGREQDIAGFLAGS_HEADER,	VT_I4,	SQL_DIAG_NUMBER
											};

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag2[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,2))
	{
		goto CLEANUP;
	}

	//this might ot might not pass depending on the odbc version, 
	//if it does set it to false becuase it wasn't cached so it should fail oledb
	rgGetDiagHRODBC[0]=S_FALSE;
	rgGetDiagODBC[0].vDiagInfo.vt=0;
	
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&rgKagDiag2[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],2))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<2;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(18)
//--------------------------------------------------------------------
// @mfunc HSTMT Single Err - Request One Record where the diag indetifier
//							returns SQL_SUCCESS on the error, Get more than one header rec
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_18()
{
	BOOL				fResult				= FALSE;
	BOOL				fTemp				= TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	DWORD				cIndex				= 0;

	//the following fields are hard coded to be sure what fields were getting in case .h file is updated
	KAGREQDIAG			rgKagDiag[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_CONNECTION_NAME
											};
	KAGREQDIAG			rgKagDiag2[]=		{	
											//	field				field flag	field type	
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_CONNECTION_NAME,
											KAGREQDIAGFLAGS_RECORD,	VT_BSTR,	SQL_DIAG_MESSAGE_TEXT
											};

	KAGGETDIAG			*rgGetDiagODBC		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHRODBC	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);

	KAGGETDIAG			*rgGetDiagOLEDB		= (KAGGETDIAG*)PROVIDER_ALLOC(sizeof(KAGGETDIAG)*2);
	HRESULT				*rgGetDiagHROLEDB	= (HRESULT*)PROVIDER_ALLOC(sizeof(HRESULT)*2);
	
	ULONG				i;
	VARTYPE				vTemp;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&rgKagDiag2[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,2))
	{
		goto CLEANUP;
	}
	
	//this field should pass, if it does set it to false becuase it wasn't cached so it should fail oledb
	if (COMPARE(rgGetDiagHRODBC[1],S_OK))
	{
		rgGetDiagHRODBC[1]				= S_FALSE;
		vTemp							= rgGetDiagODBC[1].vDiagInfo.vt;
		rgGetDiagODBC[1].vDiagInfo.vt	= 0;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Field (single) To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,&rgKagDiag[0],1))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics Fields (plural)
	if (!OleGetDiagField(&rgKagDiag2[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],2))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics
	for (i=0;i<2;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		if (1==i)
		{
			rgGetDiagODBC[1].vDiagInfo.vt	= vTemp;
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}
	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_FREE(rgGetDiagODBC)
	SAFE_FREE(rgGetDiagHRODBC)
	SAFE_FREE(rgGetDiagOLEDB)
	SAFE_FREE(rgGetDiagHROLEDB)
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(19)
//--------------------------------------------------------------------
// @mfunc HSTMT Multiple Err - Request All, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_19()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORTWO))
	{
		goto CLEANUP;
	}
	//Cause error that was cause in odbc above, this is the latest one so we should see it in oledb
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}

	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//----------------------------------------------------------------------
// @mfunc HSTMT Multiple (Stress) Err - Request All, Get All - Stress errors on HSTMT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_20()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];
	
	ULONG				i;
	ULONG				ulStressCount		=	1000;


	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}

	//USE OLEDB error MUCHO times
	for (i=0;i<=ulStressCount;i++)
	{
		//Cause ODBC error on connection handle through OLEDB
		if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
		{
			goto CLEANUP;
		}
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}

	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//--------------------------------------------------------------------
// @mfunc Multiple Mixed Handle Err - Request All, Get All
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_21()
{
	BOOL				fResult				=	FALSE;
	BOOL				fTemp				=	TRUE;
	
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	DWORD				cIndex				=	0;

	KAGGETDIAG			rgGetDiagODBC[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHRODBC[g_ccAllDiagFields];

	KAGGETDIAG			rgGetDiagOLEDB[g_ccAllDiagFields];	
	HRESULT				rgGetDiagHROLEDB[g_ccAllDiagFields];

	ULONG				i;
	BOOL				fInitialized		= FALSE;


	//ERROR HDBC
	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);
	//Cause ODBC error on connection handle through ODBC
	if (!CauseErrorThroughODBC(eHDBC))
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}

	//ERROR HSTMT - this is a error on another method so it should clear the previous error from different object
	//Get ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INTI_ALREADY_UNINIT,(SQLPOINTER)SQL_OV_ODBC2);
	fInitialized=TRUE;
	//Cause ODBC error on statement handle through ODBC
	if (!CauseErrorThroughODBC(eHSTMT))
	{
		goto CLEANUP;
	}	
	//Get ODBC diagnotics put into array of KAGGETDIAG just like call to GetDiagField would
	if (!GetODBCDiag(&g_rgKagDiagAll[0], &rgGetDiagODBC[0], &rgGetDiagHRODBC[0], eHSTMT,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//create Command object
	if (!CreateCommandObject())
	{
		goto CLEANUP;
	}	
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pICommand,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause error on different handle, this is the one we should see
	if (!CauseErrorThroughOLEDB(eHSTMT,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pICommand->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ICommand), S_OK))
	{
		goto CLEANUP;
	}	
	//Get OLE DB diagnostics
	if (!OleGetDiagField(&g_rgKagDiagAll[0], &rgGetDiagOLEDB[0], &rgGetDiagHROLEDB[0],g_ccAllDiagFields))
	{
		goto CLEANUP;
	}	
	//Compare OLE DB diagnostics with ODBC diagnostics S_OK
	for (i=0;i<g_ccAllDiagFields;i++)
	{
		if (!COMPARE(rgGetDiagHROLEDB[i],rgGetDiagHRODBC[i]))
		{
			odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result handles.\n";
			fTemp=FALSE;
		}
		if (!CompareVariant(&rgGetDiagOLEDB[i].vDiagInfo,
							&rgGetDiagODBC[i].vDiagInfo))
		{
			//the connectin name field will be different for each connected connection
			if (rgGetDiagOLEDB[i].sDiagField!=SQL_DIAG_CONNECTION_NAME)
			{
				odtLog << L"Mismatch in OLEDB vs ODBC Diagnostics result values.\n";
				fTemp=FALSE;
			}
		}
		VariantClear(&rgGetDiagOLEDB[i].vDiagInfo);
		VariantClear(&rgGetDiagODBC[i].vDiagInfo);
	}

	fResult = fTemp;

CLEANUP:
	//free Command object
	SAFE_RELEASE(pISupportErrorInfo)
	FreeCommandObject();
	if (fInitialized)
	{
		FreeState(STATE_INITIALIZED_DSO);
	}
	else
	{
		FreeState(STATE_UNINITIALIZED_DSO);
	}
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//--------------------------------------------------------------------
// @mfunc INVALID ARG RequestDiagField
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_22()
{
	ISQLRequestDiagFields	*pISQLRequestDiagFields =	NULL;
	BOOL					fResult					=	FALSE;

	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_INITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);

	//Request DIAG constant Fields To Cache in custom error object
	// Get ISQLRequestDiagFields Pointer
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISQLRequestDiagFields,(void **)&pISQLRequestDiagFields),S_OK))
	{
		goto CLEANUP;
	}
	SAFE_RELEASE(pISQLRequestDiagFields)
	//call request with NULL, expect invalid arg
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISQLRequestDiagFields,(void **)&pISQLRequestDiagFields),S_OK))
	{
		goto CLEANUP;
	}
	//rgDiagFields should not be NULL unless cDiagFields is zero
	if (!CHECK(pISQLRequestDiagFields->RequestDiagFields(1,NULL),E_INVALIDARG))
	{
		odtLog << L"RequestDiagFields Not Checking for INVALID ARG.\n";
		goto CLEANUP;
	}
	fResult=TRUE;
CLEANUP:
	SAFE_RELEASE(pISQLRequestDiagFields)
	//free Command object
	FreeState(STATE_INITIALIZED_DSO);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//--------------------------------------------------------------------
// @mfunc	INVALID ARG GetDiagField, test has to do lots to get 
//			CustomErrorObject so it can get to GetDiagField method just to test INVALID_ARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDriverErrors::Variation_23()
{
	BOOL				fResults				=	FALSE;

	IErrorRecords		*pIErrorRecords			=	NULL;
	ISQLErrorInfo		*pISQLErrorInfo			=	NULL;
	IErrorInfo			*pIErrorInfo			=	NULL;
	ISupportErrorInfo	*pISupportErrorInfo =	NULL;

	ISQLGetDiagField	*pIGetDiagField			=	NULL;

	HRESULT				hr;

	ULONG				ulErrorRecCount			=	0;
	ULONG				i						=	0;


	//Get to ODBC to proper state and OLE DB to proper state
	GetMeToState(STATE_UNINITIALIZED_DSO,(SQLPOINTER)SQL_OV_ODBC2);
	//Request DIAG constant Fields To Cache in custom error object
	if (!RequestFieldsToCache(m_pIDBInitialize,g_rgKagDiagAll,g_ccAllDiagFields))
	{
		goto CLEANUP;
	}
	//Cause ODBC error on connection handle through OLEDB
	if (!CauseErrorThroughOLEDB(eHDBC,eERRORONE))
	{
		goto CLEANUP;
	}
	//Check that current interface supports error objects
	if (!CHECK(m_pIDBInitialize->QueryInterface(IID_ISupportErrorInfo,
			(void **)&pISupportErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!pISupportErrorInfo)
	{
		goto CLEANUP;
	}
	if (!CHECK(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDBInitialize), S_OK))
	{
		goto CLEANUP;
	}	
	//Now get our current error object
	if (!CHECK(GetErrorInfo(0, &m_pIErrorInfo), S_OK))
	{
		goto CLEANUP;
	}
	if (!m_pIErrorInfo)
	{
		goto CLEANUP;
	}
	//Get the IErrorRecord interface 
	if (!CHECK(m_pIErrorInfo->QueryInterface(IID_IErrorRecords,
			(void **)&pIErrorRecords), S_OK))
	{
		goto CLEANUP;
	}
	if (!pIErrorRecords)
	{
		goto CLEANUP;
	}
	//Find out how many records were created
	if (!CHECK(pIErrorRecords->GetRecordCount(&ulErrorRecCount), S_OK))
	{
		goto CLEANUP;
	}
	//loop through error records and get first custom error object
	//not necessarily the 0 rec (bug 4193)
	for (i=0;i<ulErrorRecCount;i++)
	{
		hr=pIErrorRecords->GetCustomErrorObject(i, IID_ISQLGetDiagField, (IUnknown**) &pIGetDiagField);
		if (hr==S_OK && pIGetDiagField)
		{
			//we got one
			break;
		}
		//if pIGetDiagField is NULL and valid hr then custom error object is empty
		//but there has to an error object somewhere, check to see that it is there
		if ((hr==S_OK&&!pIGetDiagField)||(hr==E_NOINTERFACE&&!pIGetDiagField))
		{
			if (!CHECK(pIErrorRecords->GetCustomErrorObject(i, IID_ISQLErrorInfo,
				(IUnknown**) &pISQLErrorInfo), S_OK)||!pISQLErrorInfo)
			{
				if (!CHECK(pIErrorRecords->GetErrorInfo(i, m_lcid, 
					(IErrorInfo**)&pIErrorInfo), S_OK)||!pIErrorInfo)
				{
					//if we are here there is not an error object
					//GetRecordCount said there was an error but there was none to get anywhere
					goto CLEANUP;
				}
			}
			//free if we just got an interface, we were just checking it was there
			SAFE_RELEASE(pISQLErrorInfo)
			SAFE_RELEASE(pIErrorInfo)
			continue;
		}
		else
		{
			goto CLEANUP;
		}
	}
	//no fields were requested if we are here and pIGetDiagField is NULL and hr ius E_NOINTERFACE
	//so return TRUE
	if (!pIGetDiagField&&hr==E_NOINTERFACE)
	{
		fResults=TRUE;
		goto CLEANUP;
	}
	//if test requested a record object then there has to be a record object
	if (!pIGetDiagField)
	{
		goto CLEANUP;
	}

	//**CHECK for INVALID arg
	if (!CHECK(pIGetDiagField->GetDiagField(NULL),E_INVALIDARG))
	{
		odtLog << L"GetDiagField Not Checking for INVALID ARG.\n";
		goto CLEANUP;
	}

	fResults=TRUE;
CLEANUP:
	FreeState(STATE_UNINITIALIZED_DSO);
	SAFE_RELEASE(m_pIErrorInfo)
	SAFE_RELEASE(pIGetDiagField);
	SAFE_RELEASE(pISQLErrorInfo);
	SAFE_RELEASE(pIErrorInfo);
	SAFE_RELEASE(pIErrorRecords);
	SAFE_RELEASE(pISupportErrorInfo);

	return fResults;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDriverErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CKageraTest::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(TCKAGPROPS)
//*-----------------------------------------------------------------------
//|	Test Case:		TCKAGPROPS - Test Kagera specific properties
//|	Created:			01/29/99
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCKAGPROPS::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
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

	g_pTable->DoesIndexExist(&m_bIndexExists);
	
	return CKageraTest::Init();
	// }}
}
//	}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc BLOBSONFOCURSOR	OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCKAGPROPS::Variation_1()
{
	DBPROPID			rgPropertyIDs[1];
	BOOL				fTestPass			= TEST_FAIL;

	//initialization
	rgPropertyIDs[0]=KAGPROP_BLOBSONFOCURSOR;

	//get a rowset and accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,DBPROPOPTIONS_OPTIONAL, 0,NULL,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	fTestPass=TEST_PASS;
CLEANUP:
	ReleaseRowsetAndAccessor();	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc BLOBSONFOCURSOR	REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCKAGPROPS::Variation_2()
{
	DBPROPID			rgPropertyIDs[1];
	BOOL				fTestPass			= TEST_FAIL;

	//initialization
	rgPropertyIDs[0]=KAGPROP_BLOBSONFOCURSOR;

	//get a rowset and accessor
	TESTC_PROVIDER(GetRowsetAndAccessor(SELECT_ORDERBYNUMERIC,1,rgPropertyIDs,DBPROPOPTIONS_REQUIRED, 0,NULL,FALSE,
										DBACCESSOR_ROWDATA,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
										UPDATEABLE_COLS_BOUND)); 	
	fTestPass=TEST_PASS;
CLEANUP:
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
BOOL TCKAGPROPS::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();
	CRowsetObject::Terminate();
	return(CKageraTest::Terminate());
}	// }}
// }}
// }}




// {{ TCW_TC_PROTOTYPE(TCKAGADHOC)
//*-----------------------------------------------------------------------
//|	Test Case:		TCKAGADHOC - Test Kagera adhoc scenarios
//|	Created:			05/19/00
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCKAGADHOC::Init()
{
    if (g_fSqlServer)
    {
	    // {{ TCW_INIT_BASECLASS_CHECK
	    //Init baseclass
	    CRowsetObject::Init();
	    
	    //Use the global DSO created in Module init
	    SetDataSourceObject(g_pIDBInitialize); 

	    //Use the global DB Rowset created in Module init
	    SetDBSession(g_pIOpenRowset); 
	    
	    return CKageraTest::Init();
    }
    else
    {
		if (!CKageraTest::Init())
			return TEST_FAIL;
        return TEST_SKIPPED;
    }
// }}
}
//	}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc select from table with large row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCKAGADHOC::Variation_1()
{
	BOOL				fTestPass			= TEST_FAIL;
    IDBCreateCommand    *pIDBCreateCommand  = NULL;
    ICommandProperties  *pICommandProperties= NULL;
    ICommand            *pICommand          = NULL;
    ICommandText        *pICommandText      = NULL;
    IRowset             *pIRowset           = NULL;
	DBROWCOUNT	        cRows               = 0;
	DBCOUNTITEM	        cRowsOut            = 0;
	HROW                *phrow              = NULL;
	DBPROPSET			dbPropSet[1];
	DBPROP				dbProp[2];

	//QI for the accessor handle on the command object
	CHECK(g_pIOpenRowset->QueryInterface(IID_IDBCreateCommand, (void**)&pIDBCreateCommand),S_OK);
    CHECK(pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,(IUnknown**)&pICommand),S_OK);

	CHECK(pICommand->QueryInterface(IID_ICommandText, (void**)&pICommandText),S_OK);

	CHECK(pICommand->QueryInterface(IID_ICommandProperties, (void**)&pICommandProperties),S_OK);

	dbProp[0].dwPropertyID      = DBPROP_OTHERUPDATEDELETE;
	dbProp[0].dwOptions         = DBPROPOPTIONS_REQUIRED;
	dbProp[0].dwStatus          = DBPROPSTATUS_OK;
	dbProp[0].colid             = DB_NULLID;
	dbProp[0].vValue.vt         = VT_BOOL;
	V_BOOL(&dbProp[0].vValue)   = VARIANT_FALSE;
	
	dbProp[1].dwPropertyID      = DBPROP_CANHOLDROWS;
	dbProp[1].dwOptions         = DBPROPOPTIONS_REQUIRED;
	dbProp[1].dwStatus          = DBPROPSTATUS_OK;
	dbProp[1].colid             = DB_NULLID;
	dbProp[1].vValue.vt         = VT_BOOL;
	V_BOOL(&dbProp[1].vValue)   = VARIANT_TRUE;
	
	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 2;
	dbPropSet[0].rgProperties = dbProp;

	CHECK(pICommandProperties->SetProperties(1,dbPropSet),S_OK);

    CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, L"Drop table XLongFieldsX"), S_OK);
    pICommand->Execute(NULL, IID_IRowset, NULL, &cRows, (IUnknown**)&pIRowset);

    CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, L"CREATE TABLE XLongFieldsX ([key] [int] IDENTITY (1, 1) NOT NULL , [1] [varchar] (7000) NULL , [2] [varchar] (7000) NULL , [3] [varchar] (7000) NULL ,[4] [varchar] (7000) NULL , [5] [varchar] (7000) NULL , [6] [varchar] (7000) NULL ,[7] [varchar] (7000) NULL , [8] [varchar] (7000) NULL , [9] [varchar] (7000) NULL ,[10] [varchar] (7000) NULL , [11] [varchar] (7000) NULL , [12] [varchar] (7000) NULL ,[13] [varchar] (7000) NULL , [14] [varchar] (7000) NULL , [15] [varchar] (7000) NULL ,[16] [varchar] (7000) NULL , [17] [varchar] (7000) NULL , [18] [varchar] (7000) NULL ,[19] [varchar] (7000) NULL , [20] [varchar] (7000) NULL)"), S_OK);
    CHECK(pICommand->Execute(NULL, IID_IRowset, NULL, &cRows, (IUnknown**)&pIRowset), S_OK);

    CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, L"Insert into XLongFieldsX values ('abc', 'abc', 'abc', 'abc', 'abc','abc', 'abc', 'abc', 'abc', 'abc','abc', 'abc', 'abc', 'abc', 'abc','abc', 'abc', 'abc', 'abc', 'abc')"), S_OK);
    CHECK(pICommand->Execute(NULL, IID_IRowset, NULL, &cRows, (IUnknown**)&pIRowset), S_OK);

    CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, L"Insert into XLongFieldsX values ('xyz', 'xyz', 'xyz', 'xyz', 'xyz','xyz', 'xyz', 'xyz', 'xyz', 'xyz','xyz', 'xyz', 'xyz', 'xyz', 'xyz','xyz', 'xyz', 'xyz', 'xyz', 'xyz')"), S_OK);
    CHECK(pICommand->Execute(NULL, IID_IRowset, NULL, &cRows, (IUnknown**)&pIRowset), S_OK);
    
    CHECK(pICommandText->SetCommandText(DBGUID_DEFAULT, L"select * from XLongFieldsX"), S_OK);
    CHECK(pICommand->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown**)&pIRowset), S_OK);

    CHECK(pIRowset->GetNextRows(NULL,0,1,&cRowsOut,&phrow), S_OK);

    fTestPass=TEST_PASS;

    SAFE_RELEASE(pICommandProperties);
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pICommand);
    SAFE_RELEASE(pICommandText);
    SAFE_RELEASE(pIDBCreateCommand);

    return fTestPass;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCKAGADHOC::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	ReleaseDBSession();
	ReleaseDataSourceObject();
	CRowsetObject::Terminate();
	return(CKageraTest::Terminate());
}	// }}
// }}
// }}

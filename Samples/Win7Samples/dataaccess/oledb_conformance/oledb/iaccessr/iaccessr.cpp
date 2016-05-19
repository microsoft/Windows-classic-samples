
//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IACCESSR.CPP | Test Module for IAccessor
//
//--------------------------------------------------------------------

#include "MODStandard.hpp"

#define DBINITCONSTANTS		// Must be defined to initialize constants in oledb.h
#define INITGUID			// For IID_ITransactionOptions, etc.

#include "iaccessr.h"


// At least one provider has a problem with row accessors created on commands
// This will help work around...
#define NO_COMMAND_ACCESSOR_HACK

#define SAFE_RELEASE_ACCESSOR(pIAcc, hAcc) {if ((pIAcc) && (hAcc) && \
	CHECK((pIAcc)->ReleaseAccessor((hAcc), NULL), S_OK)) (hAcc) = DB_NULL_HACCESSOR;}

#define FREE_BINDINGS(x,y)	{FreeAccessorBindings(*x,*y); *x = 0; *y = NULL;}

#define IS_BASE_TYPE(wType, wBaseType)	(((wType) & ~(DBTYPE_ARRAY|DBTYPE_BYREF|DBTYPE_VECTOR)) == (wBaseType))

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xabebd540, 0xfb6b, 0x11ce, { 0xa9, 0xe9, 0x00, 0xaa, 0x00, 0x3e, 0x77, 0x8a }};
DECLARE_MODULE_NAME("IAccessor");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IAccessor");
DECLARE_MODULE_VERSION(839294173);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

//Indicate whether the provider is read only
BOOL				g_fReadOnlyProvider = FALSE;
//Indicate whether the provider supports the parameter accessor
BOOL				g_fParamAccessor= TRUE;
//Indicate whether the provider supports the command
BOOL				g_fCmdSupported = TRUE;
//Flag to show output parameter support
BOOL				g_fOutputParam;
//Row number used to create single row in table
UINT				g_uiRowNum = 1;
//Use for creating parameter accessors, all we need is the ordinals in the order they appear in the accessor
const	ULONG		MAX_COLS = 25;
DBORDINAL			g_rgParamOrds[MAX_COLS] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{		
	ULONG_PTR ulPropValue;
	ULONG ulRowCount=0;
	BOOL fNULLS=FALSE;
	IOpenRowset * pIOpenRowset=NULL;
	IDBCreateCommand * pIDBCreateCommand=NULL;
	ICommandWithParameters * pICmdWPar=NULL;

	if (ModuleCreateDBSession(pThisTestModule))
	{
		g_fParamAccessor = FALSE;
		g_uiRowNum = 1;

		// Check whether provider is read only
		g_fReadOnlyProvider = IsProviderReadOnly((IUnknown *)pThisTestModule->m_pIUnknown2);

		//Check whether the provider supports the parameter accessor by creating a command 
		// object and asking for ICommandWithParameters interface back.
		pIOpenRowset=(IOpenRowset *)pThisTestModule->m_pIUnknown2;
		if (VerifyInterface(pIOpenRowset, IID_IDBCreateCommand, SESSION_INTERFACE,
			(IUnknown **)&pIDBCreateCommand))
		{
			// If we can get an ICommandWithParameters interface from a command object we must support
			// parameter accessors
			if (SUCCEEDED(pIDBCreateCommand->CreateCommand(NULL, IID_ICommandWithParameters, (IUnknown **)&pICmdWPar)))
				g_fParamAccessor=TRUE;

			SAFE_RELEASE(pICmdWPar);
			SAFE_RELEASE(pIDBCreateCommand);
		}

		// See if output parameters are supported.  We assume the lowest functionality if 
		// GetProperty fails as this property may not be supported.
		g_fOutputParam = FALSE;
		if (GetProperty(DBPROP_OUTPUTPARAMETERAVAILABILITY, DBPROPSET_DATASOURCEINFO,
			(IUnknown *)pThisTestModule->m_pIUnknown, &ulPropValue) &&
			ulPropValue != DBPROPVAL_OA_NOTSUPPORTED)
			g_fOutputParam = TRUE;

		//Create a table we'll use for the whole test module,
		//store it in pVoid for now
		pThisTestModule->m_pVoid = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2,
			(LPWSTR)gwszModuleName);
			
		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		if(!((CTable *)pThisTestModule->m_pVoid)->GetCommandSupOnCTable() )
			g_fCmdSupported = FALSE;
		else
			g_fCmdSupported = TRUE;

		if (!CHECK(((CTable *)pThisTestModule->m_pVoid)->CreateTable(30), S_OK))
			return FALSE;

		// See if we're able to detect nulls in the table
		if (((CTable *)pThisTestModule->m_pVoid)->GetNull() == NONULLS)
			odtLog << L"Warning: This table doesn't claim to have NULL data values, so NULLs may not be tested.\n";

		//If we made it this far, everything has succeeded
		return TRUE;
	}
	
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
	
	//We still own the table since all of our testcases
	//have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{
		if(g_fCmdSupported)
			((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	return ModuleReleaseDBSession(pThisTestModule);
}	

    
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CAccessor Base Class for all IAccessor Testcases
class CAccessor : public CRowsetObject
{
	public:
		// @cmember Constructor
		CAccessor(LPWSTR wszTestCaseName);
		// @cmember Destructor
		~CAccessor(){};


	protected:	
		// @cmember Common base class initialization
		BOOL Init();
		// @cmember Common base class termination
		BOOL Terminate();

		//@cmember Copies one binding's values to another
		void CopyBindings(
			DBBINDING * dbBindDest, //@parm [IN] Destination array
			DBBINDING * dbBindSrc	//@parm [IN] Sources
		);
		
		//@cmember	If provider supports IRowsetLocate (ie, whether we can bind
		//long data and get it back using ODBC Provider)
		BLOBTYPE	m_fBindLongCols;

		//@cmember Cleans up a rowset object
		void CleanUpRowsetObject(BOOL fReleaseAccessor = TRUE);
		
		//@cmember Creates a rowset object and optionally calls GetNextRows and GetData
		HRESULT OpenRowsetObject(BOOL fGetData = FALSE);

		//@cmember Tries to GetNextRows and GetData with IRowset
		HRESULT	GetRowsAndData(IUnknown * pIUnknown, HACCESSOR hAccessor,
			DBLENGTH cbRowSize, BYTE ** ppData=NULL, IRowset ** ppIRowset=NULL, HROW ** prghRows=NULL);

		//@cmember Verifies return code from CreateAccessor call
		BOOL VerifyError(
			IUnknown * pIUnknown,
			HACCESSOR hAccessor,
			HRESULT ExpectedHr,
			DBBINDSTATUS dbExpectedStatus,
			LONG lStatusIndex,
			DBBINDING * rgBinding,
			enum DEFER_MODE eDeferMode,
			enum FAILURE_MODE eFailMode = MUST_FAIL,
			BOOL fCompareData=TRUE,
			BYTE ** pDataBuf=NULL);

		//@cmember Verifies that GetBindings returns the same binding array
		//as was used to create the accessor
		BOOL	VerifyBindings(
			IAccessor * pIAccessor,			//@parm [IN] Interface to do GetBindings on
			DBBINDING * rgCreateBindings,	//@parm [IN] Array of bindings used to create accessor
			DBCOUNTITEM cCreateBindings,			//@parm [IN] Count of bindin structs in array
			HACCESSOR hAccessor,			//@parm [IN] Handle of accessor created, for which we'll do GetBindings
			DBACCESSORFLAGS dwCreateAccessorFlags//@parm [IN] Value of Accessor flags used to create accessor
		);
								

		// @cmember Gets data with accessor and validates value, status and len were returned correctly
		HRESULT UseRowAccessorAndVerify(
				HACCESSOR hAccessor,	//@parm [IN] Handle of accessor			
				DBLENGTH cbRowSize,		//@parm [IN] Size of buffer needed for row 
				ULONG ulRowNum,			//@parm [IN] Row number needed for MakeData
				DB_LORDINAL * rgColumnsOrd,	//@parm [IN] Back end ordinals corresponding to rowset cols
				DBCOUNTITEM cColumns,			//@parm [IN] Number of ordinals in rgColumnsOrd array
				DBBINDING * rgBinding,	//@parm [IN] Array of bindings for this accessor	
				DBCOUNTITEM cBinding,			//@parm [IN] Number of bindings in rgBinding array
				BOOL fReadColumnsByRef = FALSE,	//@parm [IN] Whether or not accessor is READCOLUMNSBYREF
				BOOL fKeepCurrentRows = FALSE);	//@parm [IN] Specifies if rows are kept for next call or not

		// @cmember Uses parameter accessor for input parameters on select statement,
		// and verifies that the query result was correct
		HRESULT UseParamAccessorAndVerify(
				HACCESSOR  hAccessor,	//@parm [IN] Parameter accessor to use
				DBACCESSORFLAGS dwAccessorFlags, //@parm [IN] Accessor flags
				DBLENGTH cbRowSize,		//@parm [IN] Size for one row of parameter data
				ULONG ulRowNum,			//@parm [IN] Row number needed for MakeData								
				DBBINDING * rgBinding,	//@parm [IN] Array of bindings for this accessor	
				DBCOUNTITEM cBinding,	//@parm [IN] Number of bindings in rgBinding array
				ICommand * pICommand,	//@parm [IN] Command to execute on. Must match accessor command object.
				HRESULT hrExecute=S_OK,	//@parm [IN] Expected hresult from Execute call.
				BOOL fWarn = FALSE);

		//@cmember Find a set of types that satisfy required conversion needs
		BOOL FindConversionTypes(
			DBBINDING * prgBindings,
			DBCOUNTITEM cBindings,
			ULONG * piBackEnd,
			DBTYPE * wOptType,
			DBTYPE * wNonOptType);

		// @cmember Utility function to set all bindings to the given parameter type
		void SetParamIO(DBCOUNTITEM cBindings, DBBINDING * rgBindings, DBPARAMIO	eParamIO);

		//@cmember Changes bindings to the specified memory owner for
		//all the types for which dwMemOwner applies
		void AdjustMemOwner(DBMEMOWNERENUM eMemOwner, DBCOUNTITEM cBindings, DBBINDING * rgBindings);

		//@cmember Accept success but print failure for log if not S_OK on non-ReadOnly provider
		BOOL CheckHr(HRESULT hr);

		//@cmember Verifies required properties are set on rowset
		HRESULT VerifyRowsetProperties(IUnknown * pIUnknown, DBPROPSET * rgPropertySets, ULONG cPropertySets);

		//@cmember Check for provider support of binding fixed length columns BYREF
		enum FIXED_BYREF_SUPPORT SupportFixedByRef(IUnknown * pIUnknown);

		//@cmember Flag indicating whether or not pass by ref accessors are supported
		BOOL	m_fPassByRef;

		//@cmember Array of status for each binding
		DBBINDSTATUS * m_rgStatus;

		//@cmember Count of bindings in m_rgBindings
		DBCOUNTITEM	m_cBindings;

		//@cmember Count of bindings in m_rgBindings2
		DBCOUNTITEM	m_cBindings2;

		//@cmember Array of DBBINDINGS used to create an accessor
		DBBINDING *		m_rgBindings;

		//@cmember Array of DBBINDINGS used to create a second accessor
		DBBINDING *		m_rgBindings2;

		//@cmember Row size used to allocate buffer pointed to by m_pData
		DBLENGTH		m_cbRowSize;

		//@cmember Row size used to allocate buffer pointed to by m_pData2
		DBLENGTH		m_cbRowSize2;

		//@cmember Number of binding which is in error
		ULONG			m_ulErrorBinding;

		//@cmember Array of DBINDINGS retrieved via GetBindings
		DBBINDING *		m_rgGetBindings;

		//@cmember Handle to created a accessor
		HACCESSOR		m_hAccessor;

		//@cmember Handle to created a second accessor
		HACCESSOR		m_hAccessor2;

		//@cmember Pointer to consumer's buffer
		BYTE *			m_pData;

		//@cmember Pointer to consumer's second data buffer
		BYTE *			m_pData2;

		//@cmember Count of columns in table
		DBCOUNTITEM			m_cTableColumns;

		//@cmember Count of rows obtained from GetNextRows
		DBCOUNTITEM 			m_cRowsObtained;

		//@cmember Row handle retrieved from GetNextRows
		HROW			m_hRow;
		
		//@cmember Interface pointer
		IRowsetChange*	m_pIRowsetChange;

		//@cmember Interface pointer
		IRowset *		m_pIRowset;

		//@cmember IAccessor on Command object
		IAccessor *		m_pCmdIAccessor;

		//@cmember Flag indicating if IRowset is supported
		BOOL			m_fIRowset;

		//@cmember Flag indicating if MAYREFERENCE is not set for any Fixed length columns
		//BOOL			m_fMayNotReferenceFixed;
		
		//@cmember Flag indicating if MAYREFERENCE is not set for any Variable length columns
		//BOOL			m_fMayNotReferenceVariable;

		//@cmember Count of searchable columns in rowset, used for parameterized queries
		ULONG		m_cSearchableCols;

		//@cmember Array of ordinals of searchable columns in rowset, used for parameterized queries
		DB_LORDINAL *	m_rgSearchableCols;
	
		//@cmember	Property set for this test case
		DBPROPSET	*	m_pDBPropSetLocate;

		//@cmember	Properties for this test case
		DBPROP		*	m_prgDBPropsLocate;	

		//@cmember	Storage object used for DBTYPE_IUNKNOWN bindings
		DBOBJECT	m_StorageObject;

		//@cmember	Flag to indicate provider supports OLE objects (IUNKNOWN).
		BOOL		m_fOLEOBJECTS;
		
};
		   

//--------------------------------------------------------------------
// @mfunc Constructor
//
CAccessor::CAccessor(LPWSTR wszTestCaseName) : CRowsetObject(wszTestCaseName)
{
		m_ulErrorBinding = 0;
		m_hAccessor = DB_NULL_HACCESSOR;
		m_hAccessor2 = DB_NULL_HACCESSOR;
		m_fIRowset = FALSE;		 
		m_cRowsObtained = 0;		 
		m_cBindings = 0;
		m_cBindings2 = 0;
		m_rgBindings = NULL;
		m_rgBindings2 = NULL;
		m_pData = NULL;
		m_pData2 = NULL;
		m_cbRowSize = 1;
		m_cbRowSize2 = 1;
		m_rgGetBindings = NULL;
		m_cTableColumns = 0;
		m_pIRowsetChange = NULL;
		m_pIRowset = NULL;
		m_hRow = DB_NULL_HROW;
		m_pCmdIAccessor = NULL;
		m_cSearchableCols = 0;
		m_rgSearchableCols = NULL;
		m_rgStatus = NULL;
		m_fPassByRef = FALSE;
		m_pDBPropSetLocate = NULL;
		m_prgDBPropsLocate = NULL;
		m_fOLEOBJECTS = FALSE;
		m_StorageObject.dwFlags=STGM_READ;
		m_StorageObject.iid=IID_ISequentialStream;
}

//--------------------------------------------------------------------
// @mfunc Base class Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CAccessor::Init()
{	
	IDBProperties * pIDBProp = NULL;
	DBPROPIDSET		PropIDSet;
	PROPID			PropID = DBPROP_BYREFACCESSORS;
	ULONG			cPropSets = 0;
	DBPROPSET *		rgPropSets = NULL;
	ICommandProperties * pICmdProps = NULL;
	HRESULT			hr;
	ULONG_PTR		ulPropValue=0;

	// Check for OLE object support
	if (GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		(IUnknown *)m_pThisTestModule->m_pIUnknown, &ulPropValue) &&
		ulPropValue & DBPROPVAL_OO_BLOB)
		m_fOLEOBJECTS=TRUE;

	if (m_fOLEOBJECTS)
	{
		// Find out what storage object is needed
		if (COMPARE(GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO,
			(IUnknown *)m_pThisTestModule->m_pIUnknown, &ulPropValue),TRUE))
		{
			if (ulPropValue & DBPROPVAL_SS_ISEQUENTIALSTREAM)
				m_StorageObject.iid=IID_ISequentialStream;
			else if (ulPropValue & DBPROPVAL_SS_ISTREAM)
				m_StorageObject.iid=IID_IStream;
			else if (ulPropValue & DBPROPVAL_SS_ISTORAGE)
				m_StorageObject.iid=IID_IStorage;
			else if (ulPropValue & DBPROPVAL_SS_ILOCKBYTES)
				m_StorageObject.iid=IID_ILockBytes;
			else
			{
				// This is an error, 
				odtLog << L"OLE Objects supported but no valid structured storage type was returned.\n";
				COMPARE(1, 0);
				m_fOLEOBJECTS = FALSE;
			}
		}
		else
			// We couldn't retrieve the storage type required, drop back to no support
			m_fOLEOBJECTS = FALSE;
	}

	// Initialize storage object flag member
	m_StorageObject.dwFlags=STGM_READ;

	m_pDBPropSetLocate = (DBPROPSET *)PROVIDER_ALLOC(sizeof(DBPROPSET));
	m_prgDBPropsLocate = (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP));

	if (!m_pDBPropSetLocate || !m_prgDBPropsLocate)
	{
		odtLog << wszMemoryAllocationError;
		return FALSE;
	}

	//Prop structs for setting IRowsetLocate
	m_pDBPropSetLocate->guidPropertySet = DBPROPSET_ROWSET;
	m_pDBPropSetLocate->rgProperties = m_prgDBPropsLocate;
	m_pDBPropSetLocate->cProperties = 1;
	m_prgDBPropsLocate[0].dwPropertyID = DBPROP_IRowsetLocate;
	//Note this is required because otherwise ODBC Provider won't
	//consider it cheap and we will lose long data testing
	m_prgDBPropsLocate[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_prgDBPropsLocate[0].colid = DB_NULLID;
	m_prgDBPropsLocate[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_prgDBPropsLocate[0].vValue) = VARIANT_TRUE;

	//Prop struct for asking about BYREFACCESSOR support
	PropIDSet.guidPropertySet = DBPROPSET_DATASOURCEINFO;
	PropIDSet.cPropertyIDs = 1;
	PropIDSet.rgPropertyIDs = &PropID;

	if(CRowsetObject::Init())	
	{
		//Copy the IDBCreateCommand pointer we got at the
		//module level down to the testcase level.
		//Note, this increments the ref count, so we call
		//ReleaseDBSession in the Terminate, but the DBSession
		//does not go away until ModuleTerminate time
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);

		//Have this testcase use the table created in ModuleInit, but don't
		//let table be deleted, since we'll use it for next test case
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

		//Record number of columns in the table
		m_cTableColumns = m_pTable->CountColumnsOnTable();
		
		//Zero is returned if there was an error counting columns
		if (!m_cTableColumns)
			return FALSE;

		//Allocate the max array we'll need for binding status
		//We add one to col total since we may have bookmarks
		m_rgStatus = (DBBINDSTATUS *)m_pIMalloc->Alloc((m_cTableColumns+1) * sizeof(DBBINDSTATUS));
		if (!m_rgStatus)
			return FALSE;

		//Find out if PASS BY REF Accessors are supported		
		if (VerifyInterface((IDBInitialize *)m_pThisTestModule->m_pIUnknown, IID_IDBProperties, DATASOURCE_INTERFACE,
			(IUnknown **)&pIDBProp))
		{
			HRESULT hrGetProp = E_FAIL;
		
			hrGetProp = pIDBProp->GetProperties(1, &PropIDSet, &cPropSets, &rgPropSets);

			// Since we only asked for one prop we must only get one back
			if (hrGetProp == S_OK || hrGetProp == DB_E_ERRORSOCCURRED)
			{
				if (COMPARE(cPropSets, 1) && COMPARE(rgPropSets[0].cProperties, 1))
				{
					if (hrGetProp == S_OK)
					{
						// There is only one set with one property
						COMPARE(rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_OK);
						m_fPassByRef = (VARIANT_TRUE == rgPropSets[0].rgProperties[0].vValue.boolVal);
					}
					else if (hrGetProp == DB_E_ERRORSOCCURRED)
					{
						//The only other acceptable value is not supported
						COMPARE(rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
						m_fPassByRef = FALSE;
					}
				}
			}
			else
				CHECK(hrGetProp, S_OK);
			
			//release property sets 
			FreeProperties(&cPropSets, &rgPropSets);
			SAFE_RELEASE(pIDBProp);
		} 

		//Find out if IRowsetLocate is supported
		SetRowsetProperties(m_pDBPropSetLocate, 1);

		if (SUCCEEDED(hr=CreateRowsetObject(SELECT_VALIDATIONORDER)))
		{

			//Record our ability to bind long columns
			//NOTE:  This is only a restriction for ODBC Provider,
			//other providers may be able to bind long data
			//without the support of IRowsetLocate

			//We assume we have only one property set, so
			//we'll access the zeroth element of m_rgPropSets
			COMPARE(m_cPropSets, 1);
//			if (m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_OK)
				m_fBindLongCols = BLOB_LONG;
//			else
//				m_fBindLongCols = NO_BLOB_COLS;

			//Keep command so that IRowsetLocate property is always set
			//for all rowsets generated.  It is SETIFCHEAP, so if it
			//is not supported, we will still work.
			ReleaseRowsetObject();
			return TRUE;
		}
		else if(hr==DB_E_ERRORSOCCURRED)
		{
			m_fBindLongCols = NO_BLOB_COLS;
			
			// Release the properties which will affect opening rowset later
			if (m_rgPropSets)
			{
				// Walk Set array, freeing all member property arrays
				for(ULONG i=0; i<m_cPropSets; i++)
				{
					PROVIDER_FREE(m_rgPropSets[i].rgProperties);
				}
			
				PROVIDER_FREE(m_rgPropSets);
				m_cPropSets = 0;
			}
			return TRUE;
		}
	}  
	return FALSE;

}


//--------------------------------------------------------------------
// @mfunc Base Case Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CAccessor::Terminate()
{
	// Make sure we don't leak the binding information
	CleanUpRowsetObject();

	//Cleanup as much as we allocated in the base class init	
	ReleaseCommandObject();
	ReleaseDBSession();
	
	// Free property array
	PROVIDER_FREE(m_pDBPropSetLocate);
	PROVIDER_FREE(m_prgDBPropsLocate);

	//Free binding status array
	PROVIDER_FREE(m_rgStatus);

	return(CRowsetObject::Terminate());
}

//--------------------------------------------------------------------
// @mfunc Cleans up member vars associated with rowset object
//
void CAccessor::CleanUpRowsetObject(BOOL fReleaseAccessor)
{
	// We might have bindings already, don't overwrite
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	
	// Reset to 0 and NULL
	m_cBindings = 0;
	m_rgBindings = NULL;

	if (fReleaseAccessor)
	{
		// We might have created an accessor; we don't want to overwrite.
		// Don't check return code because we don't know if the accessor was created on the command
		// or rowset object.
		if (m_pCmdIAccessor && m_hAccessor != DB_NULL_HACCESSOR && 
			S_OK == m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL))
			m_hAccessor = DB_NULL_HACCESSOR;
		if (m_pIAccessor && m_hAccessor != DB_NULL_HACCESSOR && 
			S_OK == m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL))
			m_hAccessor = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pCmdIAccessor);
	SAFE_RELEASE(m_pIRowset);

	// Release the rowset in case it exists.  Note this releases m_pIAccessor for us.
	ReleaseRowsetObject();

	m_fIRowset = FALSE;
}

//--------------------------------------------------------------------
// @mfunc Creates a rowset object and optionally calls GetNextRows and GetData
//
HRESULT CAccessor::OpenRowsetObject(BOOL fGetData)
{
	IRowsetInfo * pIRowsetInfo = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = E_FAIL;

	CleanUpRowsetObject(FALSE);

	//Set m_pIAccessor on a 'select *' rowset  We'll
	//use this interface ptr to do our tests.

hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

#ifdef COMMAND_ACCESSOR_HACK
	// Hack for Service Components
	if (!CHECK(hr, S_OK))
	{
		SAFE_RELEASE(m_pICommand);
		TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand), S_OK);
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);
	}
#endif

	TESTC_(hr, S_OK);

	TESTC(m_pIAccessor != NULL);

	// Get a rowset interface for later use
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
		(IUnknown **)&m_pIRowset));

	m_fIRowset = TRUE;

	//Now get a command if that is applicable
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowsetInfo, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetInfo));

	if(g_fCmdSupported)
	{
		//Note that NULL is put in m_pCmdIAccesor if that fails, which is what
		//we'll do the check on to determine support
		hr = pIRowsetInfo->GetSpecification(IID_IAccessor, (IUnknown **)&m_pCmdIAccessor);

		// Some providers may return S_FALSE from GetSpecification
		if (S_FALSE == hr)
		{
			if (m_pICommand && !VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
			(IUnknown **)&m_pCmdIAccessor))
				g_fCmdSupported = FALSE;
		}
		else
			TESTC_(hr, S_OK);
	}
			
	//Fill in m_rgBindings, m_cBindings and m_cbRowSize with valid values
	TESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);

	//Get Rows to force immediate validation, 
	//we need a valid m_hAccessor for this call
	if (fGetData)
		TESTC_(GetRowsAndData(m_pIAccessor, hAccessor, m_cbRowSize), S_OK);				

	// We'll just use the bindings after this, so release accessor
	// Don't consider this a fatal error if it fails
	if (CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK))
		hAccessor = DB_NULL_HACCESSOR;

	hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pIRowsetInfo);

	// Release the rowset object on error
	if (S_OK != hr)
	{
		SAFE_RELEASE(m_pCmdIAccessor);
		SAFE_RELEASE(m_pIRowset);
		ReleaseRowsetObject();
		m_fIRowset = FALSE;
	}

	return hr;
}

//--------------------------------------------------------------------
// @mfunc Accept success but print failure for log if not S_OK on
// non-ReadOnly provider
//
BOOL CAccessor::CheckHr(HRESULT hr)
{
	// We want to accept DB_S_ERRORSOCCURRED if it is returned, but
	// we should print a failure for the log.  S_OK is the expected result for 
	// non-ReadOnly providers.
	if (!g_fReadOnlyProvider)
		CHECK(hr, S_OK);
	return SUCCEEDED(hr);
}


//--------------------------------------------------------------------
// @mfunc Copies bindings from dbBindSrc to dbBindDest
//
void CAccessor::CopyBindings(DBBINDING * dbBindDest, DBBINDING * dbBindSrc)
{
	ASSERT(dbBindDest);
	ASSERT(dbBindSrc);

	
	dbBindDest->iOrdinal = dbBindSrc->iOrdinal;
	dbBindDest->dwPart = dbBindSrc->dwPart;
	dbBindDest->wType = dbBindSrc->wType;
	dbBindDest->eParamIO = dbBindSrc->eParamIO;
	dbBindDest->pTypeInfo = dbBindSrc->pTypeInfo;
	dbBindDest->obValue = dbBindSrc->obValue;
	dbBindDest->cbMaxLen = dbBindSrc->cbMaxLen;
	if (dbBindSrc->pObject)
	{
		//Allocate a new object for the destination binding
		dbBindDest->pObject = (DBOBJECT *)m_pIMalloc->Alloc(sizeof(DBOBJECT));		
		dbBindDest->pObject->dwFlags = dbBindSrc->pObject->dwFlags;
		dbBindDest->pObject->iid = dbBindSrc->pObject->iid;
	}
	else
		dbBindDest->pObject = NULL;	
	dbBindDest->obLength = dbBindSrc->obLength;
	dbBindDest->obStatus = dbBindSrc->obStatus;
	dbBindDest->dwMemOwner = dbBindSrc->dwMemOwner;
	dbBindDest->pBindExt = dbBindSrc->pBindExt;
	dbBindDest->dwFlags = dbBindSrc->dwFlags;
	dbBindDest->bPrecision = dbBindSrc->bPrecision;
	dbBindDest->bScale = dbBindSrc->bScale;


}

//--------------------------------------------------------------------
// @mfunc Utility function to set each binding to eParamIO
//
void CAccessor::SetParamIO(DBCOUNTITEM cBindings, DBBINDING * rgBindings, DBPARAMIO	eParamIO)
{	
	while (cBindings)
	{
		rgBindings[cBindings-1].eParamIO = eParamIO;
		cBindings--;
	}
}
//--------------------------------------------------------------------
// @mfunc Gets IRowset, calls GetNextRows and GetData on it.
//
HRESULT	CAccessor::GetRowsAndData(IUnknown * pIUnknown, HACCESSOR hAccessor,
	DBLENGTH cbRowSize, BYTE ** ppData, IRowset ** ppIRowset, HROW ** pprghRows)
{
	IRowset *	pIRowset = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	BYTE *		pData = NULL;
	HROW *		prghRows = NULL;

	//Alloc enough for a row buffer	
	pData = (BYTE *)m_pIMalloc->Alloc(cbRowSize);
	if (!pData)
		return ResultFromScode(E_OUTOFMEMORY);

	//Get an IRowset interface 
	if (VerifyInterface(pIUnknown,IID_IRowset,ROWSET_INTERFACE,(IUnknown **)&pIRowset))
	{	
		//We could get a couple of different S codes for the next two calls		
		m_pError->SetErrorLevel(HR_SUCCEED);

		//Start at beginning of rowset
		CHECK(pIRowset->RestartPosition(NULL), S_OK);
		
		//Fetch one row of data		
		CHECK(m_hr = pIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &prghRows), IGNORE);
		m_pError->SetErrorLevel(HR_STRICT);
		
		//In case we get DB_S_ENDOFROWSET, need to also check cRowsObtained
		if (SUCCEEDED(m_hr) && cRowsObtained)		
			m_hr = pIRowset->GetData(*prghRows, hAccessor, pData);				

		if (prghRows)
		{
			if (pprghRows)
				*pprghRows = prghRows;
			else
			{
				pIRowset->ReleaseRows(1, prghRows, NULL, NULL, NULL);
				PROVIDER_FREE(prghRows);
			}
		}

	}
	else
		m_hr = E_FAIL;
	
	if (pData)
	{
		//Either give memory to user
		if (ppData)
			*ppData = pData;
		else
		{
			// Or release it.  Unfortunately for BYREF types we have to know the bindings to release the memory.
			// We could pass in the bindings, but it's better to exercise GetBindings...
			if (pIRowset)
			{
				IAccessor * pIAccessor = NULL;
				DBCOUNTITEM cBindings;
				DBACCESSORFLAGS dwAccessorFlags;
				DBBINDING * pBindings = NULL;

				if (!VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&pIAccessor))
					m_hr = E_FAIL;
				
				CHECK(pIAccessor->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &pBindings), S_OK);

				CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData, TRUE), S_OK);

				CHECK(FreeAccessorBindings(cBindings, pBindings), S_OK);

				SAFE_RELEASE(pIAccessor);
			}
		}
	}

	if (ppIRowset)
		*ppIRowset = pIRowset;
	else
		SAFE_RELEASE(pIRowset);

	//Return GetNextRows or GetData Result
	return m_hr;

}

//--------------------------------------------------------------------
// @mfunc Verifies GetBindings returns the same bindings array
// as was used to create the accessor
//
// @rdesc TRUE or FALSE
// @flag TRUE The Bindings match
// @flag FALSE The Bindings do not match
//
BOOL	CAccessor::VerifyBindings(
				IAccessor * pIAccessor,			//@parm [IN] Interface to do GetBindings on
				DBBINDING * rgCreateBindings,	//@parm [IN] Array of bindings used to create accessor
				DBCOUNTITEM cCreateBindings,			//@parm [IN] Count of bindin structs in array
				HACCESSOR hAccessor,			//@parm [IN] Handle of accessor created, for which we'll do GetBindings
				DBACCESSORFLAGS dwCreateAccessorFlags)//@parm [IN] Value of Accessor flags used to create accessor
				
{
	DBACCESSORFLAGS	dwGetAccessorFlags = 0;
	DBCOUNTITEM			cGetBindings = 0;
	ULONG			i;
	DBBINDING *		rgGetBindings = NULL;
	HRESULT			hr = E_FAIL;

	ASSERT(pIAccessor);

	hr = pIAccessor->GetBindings(hAccessor, &dwGetAccessorFlags, 
		&cGetBindings, &rgGetBindings);

	TESTC_(hr, S_OK);
	
	// Some providers just ignore the DBACCESSOR_OPTIMIZED bit because an optimized accessor
	// is identical to a regular accessor, therefore the bit may not be returned even if set.  Providers are allowed
	// to not return this bit in GetBindings, so make this a warning.
	if (dwCreateAccessorFlags & DBACCESSOR_OPTIMIZED && !(dwGetAccessorFlags & DBACCESSOR_OPTIMIZED))
	{
		odtLog << L"DBACCESSOR_OPTIMIZED bit not returned from GetBindings.\n";
		TESTW(dwCreateAccessorFlags == dwGetAccessorFlags);
		dwCreateAccessorFlags &= ~DBACCESSOR_OPTIMIZED;
	}

	TESTC(dwGetAccessorFlags == dwCreateAccessorFlags);
	TESTC(cGetBindings == cCreateBindings);

	//Set up index for bindings
	if (cGetBindings == 0)
	{
		//This is a null accessor, binding array should be null
		TESTC(rgGetBindings == NULL);
	}
	else
	{
		for (i=0; i < cGetBindings; i++)
		{
			// iOrdinal
			TESTC(rgGetBindings[i].iOrdinal == rgCreateBindings[i].iOrdinal);

			// obValue
			if (rgCreateBindings[i].dwPart & DBPART_VALUE)
			{
				TESTC(rgGetBindings[i].obValue == rgCreateBindings[i].obValue);
			}

			// obLength
			if (rgCreateBindings[i].dwPart & DBPART_LENGTH)
			{
				TESTC(rgGetBindings[i].obLength == rgCreateBindings[i].obLength);
			}

			// obStatus
			if (rgCreateBindings[i].dwPart & DBPART_STATUS)
			{
				TESTC(rgGetBindings[i].obStatus == rgCreateBindings[i].obStatus);
			}

			// pTypeInfo
			TESTC(rgGetBindings[i].pTypeInfo == rgCreateBindings[i].pTypeInfo);
			
			// pObject. Only applies if VALUE bound and type is DBTYPE_UNKNOWN				
			if (rgCreateBindings[i].dwPart & DBPART_VALUE && 
				rgCreateBindings[i].wType == DBTYPE_IUNKNOWN)
			{
				TESTC(rgGetBindings[i].pObject->dwFlags == rgCreateBindings[i].pObject->dwFlags);					
				TESTC(rgGetBindings[i].pObject->iid == rgCreateBindings[i].pObject->iid);				
			}

			// pBindExt
			TESTC(rgGetBindings[i].pBindExt == rgCreateBindings[i].pBindExt);

			// dwPart
			TESTC(rgGetBindings[i].dwPart == rgCreateBindings[i].dwPart);

			// dwMemOwner
			TESTC(rgGetBindings[i].dwMemOwner == rgCreateBindings[i].dwMemOwner);

			// eParamIO
			TESTC(rgGetBindings[i].eParamIO == rgCreateBindings[i].eParamIO);

			// cbMaxLen
			if (rgCreateBindings[i].dwPart & DBPART_VALUE)
			{
				TESTC(rgGetBindings[i].cbMaxLen == rgCreateBindings[i].cbMaxLen);
			}

			// dwFlags: Only valid for string types
			// Providers that don't support DBBINDFLAG_HTML will likely not return it,
			// so don't fail them for that.
			if (IS_BASE_TYPE(rgCreateBindings[i].wType, DBTYPE_STR) ||
				IS_BASE_TYPE(rgCreateBindings[i].wType, DBTYPE_WSTR))
			{

				if (rgCreateBindings[i].dwFlags & DBBINDFLAG_HTML)
				{
					// Warning if flag not set
					COMPAREW(!!(rgGetBindings[i].dwFlags & DBBINDFLAG_HTML), TRUE);

					// Remove the flag from each binding
					rgGetBindings[i].dwFlags &= ~DBBINDFLAG_HTML;
					rgCreateBindings[i].dwFlags &= ~DBBINDFLAG_HTML;

				}
				TESTC(rgGetBindings[i].dwFlags == rgCreateBindings[i].dwFlags);
			}

			//Precision and scale only apply for numeric and decimal types
			//and then only if VALUE is bound
			if ((rgCreateBindings[i].wType == DBTYPE_NUMERIC || 
				rgCreateBindings[i].wType == DBTYPE_DECIMAL) &&
				(rgCreateBindings[i].dwPart & DBPART_VALUE))
			{				
				TESTC(rgGetBindings[i].bPrecision == rgCreateBindings[i].bPrecision);
				TESTC(rgGetBindings[i].bScale == rgCreateBindings[i].bScale);
			}
			
		}
	}


	PROVIDER_FREE(rgGetBindings);

	//If we get here, we must have  passed
	return TRUE;


CLEANUP:
	PROVIDER_FREE(rgGetBindings);
	return FALSE;


}

// Verify error returned from CreateAccessor call. 
BOOL CAccessor::VerifyError(
	IUnknown * pIUnknown,
	HACCESSOR hAccessor,
	HRESULT ExpectedHr,
	DBBINDSTATUS dbExpectedStatus,
	LONG lStatusIndex,
	DBBINDING * rgBinding,
	enum DEFER_MODE eDeferMode,		
	enum FAILURE_MODE eFailMode,
	BOOL fCompareData,
	BYTE ** pDataBuf)
{
	BOOL	fResults = FALSE;
	BYTE * pData = NULL;
	IRowset *	pIRowset = NULL;
	HROW *		prghRows = NULL;

	ASSERT (eDeferMode == IMMEDIATE		||
			eDeferMode == MAY_DEFERR	||
			eDeferMode == MUST_DEFERR);


	if	((eDeferMode == IMMEDIATE && eFailMode == MUST_FAIL) ||
		(eDeferMode == IMMEDIATE && eFailMode == MAY_FAIL && FAILED(m_hr)) ||
		(eDeferMode == MAY_DEFERR && FAILED(m_hr)))
	{
		//CreateAccessor must always return DB_E_ERRORSOCCURRED in immediate mode

		//Verify correct binding is identified as bad and accessor
		//is nulled, in addition to the return code being correct
		fResults = CHECK(m_hr, DB_E_ERRORSOCCURRED) &&
				((lStatusIndex != -1) ? COMPARE(dbExpectedStatus, m_rgStatus[lStatusIndex]) : TRUE) && 
				COMPARE(NULL, hAccessor);
	}
	else
	{
		//If validation is deferred, the CreateAccessor call must have succeeded
		if (CHECK(m_hr, NOERROR))
		{
			//But using the accessor should fail
			m_hr = GetRowsAndData(pIUnknown, hAccessor, m_cbRowSize, &pData, &pIRowset, &prghRows);

			// If we failed here but specified immediate mode then the provider has a bug,
			// For a MAY_FAIL case we should succeed here in IMMEDIATE mode.
			if (eDeferMode == IMMEDIATE && eFailMode == MAY_FAIL && FAILED(m_hr))
				CHECK(m_hr, S_OK);
			//This can be either DB_S_ERRORSOCCURRED
			else if (m_hr == DB_S_ERRORSOCCURRED || m_hr == DB_E_ERRORSOCCURRED)
			{
				//Then the status must be right for the correct binding
				fResults = COMPARE(STATUS_BINDING(rgBinding[lStatusIndex], pData), DBSTATUS_E_BADACCESSOR);
				
				//Further, if we return DB_E_ERRORSOCCURED, we must have done
				//no useful work, therefore it must be the first binding that failed
				if (m_hr == DB_E_ERRORSOCCURRED)
					fResults &= COMPARE(lStatusIndex, 0);
			}
			//Or we can fail outright
			else if (eFailMode == MUST_FAIL || !SUCCEEDED(m_hr)) 
				fResults = CHECK(m_hr, ExpectedHr);
			else
				fResults = CHECK(m_hr, S_OK);
			
			// If we got back data then we didn't get an error.  This may be valid in
			// some cases (MAY_FAIL cases).
			if (fCompareData && SUCCEEDED(m_hr) &&
				STATUS_BINDING(rgBinding[lStatusIndex], pData) == DBSTATUS_S_OK)
			{

				// If we got data back then GetData succeeded
				// Make sure it matches what we expect
				fResults &= COMPARE(CompareData(
						m_cRowsetCols,
						m_rgTableColOrds,
						1,
						pData,
						1,
						&rgBinding[lStatusIndex],
						m_pTable,
						m_pIMalloc,
						PRIMARY,
						(pDataBuf) ? COMPARE_ONLY : COMPARE_FREE,
						COMPARE_ALL,
						TRUE
					), TRUE);
			}

		}
	}

	
	if (pDataBuf)
		*pDataBuf = pData;
	else
		PROVIDER_FREE(pData);

	// Release any rows obtained in GetRowsAndData.  Should be 1 row handle.
	if (pIRowset && prghRows)
	{
		pIRowset->ReleaseRows(1, prghRows, NULL, NULL, NULL);
		PROVIDER_FREE(prghRows);
	}
	SAFE_RELEASE(pIRowset);

	return fResults;
}

void CAccessor::AdjustMemOwner(DBMEMOWNERENUM eMemOwner, DBCOUNTITEM cBindings, DBBINDING * rgBindings)
{
	ULONG	i;
	for (i=0; i<cBindings; i++)
	{
		//Only change dwMemOwner if the type is appropriate

		if (rgBindings[i].wType & DBTYPE_BYREF ||
			rgBindings[i].wType & DBTYPE_VECTOR ||
			rgBindings[i].wType & DBTYPE_ARRAY ||
			//Strip off a possible BYREF to check if its a BSTR
			(~(DBTYPE_BYREF) & rgBindings[i].wType)  == DBTYPE_BSTR)
		{
			//Switch to type of memory owner caller wants
			if (eMemOwner == DBMEMOWNER_PROVIDEROWNED)
				rgBindings[i].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
			else
				rgBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		}
	}
}		

HRESULT CAccessor::VerifyRowsetProperties(IUnknown * pIUnknown, DBPROPSET * rgPropertySets, ULONG cPropertySets)
{
	// We opened a rowset, but we need to make sure the properties we need are set
	IRowsetInfo * pIRowsetInfo = NULL;
	HRESULT hr = S_OK;
	ULONG iPropSet, iProp;
	VARIANT vPropertyVal;

	if (VerifyInterface(pIUnknown, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown **)&pIRowsetInfo))
	{
		for (iPropSet = 0; iPropSet < cPropertySets; iPropSet++)
		{
			for (iProp = 0; iProp < rgPropertySets[iPropSet].cProperties; iProp++)
			{
				if (rgPropertySets[iPropSet].rgProperties[iProp].dwOptions == DBPROPOPTIONS_REQUIRED)
				{
					VariantInit(&vPropertyVal);

					// This is a required property, verify it is set on the rowset
					if (!GetProperty(
						rgPropertySets[iPropSet].rgProperties[iProp].dwPropertyID,
						rgPropertySets[iPropSet].guidPropertySet,
						(IUnknown *)pIRowsetInfo,
						&vPropertyVal))
					{
						// Couldn't read the property
						hr = E_FAIL;
						goto CLEANUP;
					}

					if (rgPropertySets[iPropSet].guidPropertySet == DBPROPSET_ROWSET &&
						rgPropertySets[iPropSet].rgProperties[iProp].dwPropertyID == DBPROP_UPDATABILITY)
					{
						// This is a bitmask property, just make sure the bits we asked for are set
						// Currently there is only one bitmask property
						if (V_I4(&rgPropertySets[iPropSet].rgProperties[iProp].vValue) != 
							(V_I4(&vPropertyVal) & 
							V_I4(&rgPropertySets[iPropSet].rgProperties[iProp].vValue)))
							// if (Rowset & Asked != Asked) then the bits (properties) we
							// needed on the rowset were not there.
						{
							hr = E_FAIL;
							goto CLEANUP;
						}
					}
					else
						// Compare values exactly
						if (!CompareVariant(
							&rgPropertySets[iPropSet].rgProperties[iProp].vValue,
							&vPropertyVal))
						{
							// Variants don't match
							hr = E_FAIL;
							goto CLEANUP;
						}

					
					VariantClear(&vPropertyVal);

				}

			}  // Next property
	
		} // Next property set

	}
	else
		// Couldn't get the IRowsetInfo interface
		hr = E_FAIL;

CLEANUP:
	
	SAFE_RELEASE(pIRowsetInfo);

	VariantClear(&vPropertyVal);
	
	if (!SUCCEEDED(hr))
		odtLog << L"Requested properties not supported.\n";

	return hr;
}

// Note:  Due to the spec issue noted below this function will report FIXED_BYREF_SOME or
// FIXED_BYREF_ALL for tables that contain fixed length binary or char columns.
enum FIXED_BYREF_SUPPORT CAccessor::SupportFixedByRef(IUnknown * pIUnknown)
{
	IConvertType * pIConvertType=NULL;
	IColumnsInfo * pIColumnsInfo = NULL;
	CCol  		TempCol;
	enum FIXED_BYREF_SUPPORT eSupport = FIXED_BYREF_ALL;
	ULONG	cFixedColsByRef = 0;
	DBTYPE wFromType, wToType;
	HRESULT hr;
	DBORDINAL cColumns;
	DBCOLUMNINFO * pColInfo=NULL;
	WCHAR * pStringsBuffer=NULL;

	// We need to obtain an IConvertType interface first
	if(!VerifyInterface(pIUnknown,IID_IConvertType, ROWSET_INTERFACE,
		(IUnknown **)&pIConvertType))
		return FIXED_BYREF_NONE;

	// We also need IColumnsInfo
	if(!VerifyInterface(pIUnknown,IID_IColumnsInfo, ROWSET_INTERFACE,
		(IUnknown **)&pIColumnsInfo))
		return FIXED_BYREF_NONE;

	// Get column information
	if (!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &pColInfo, &pStringsBuffer), S_OK))
		return FIXED_BYREF_NONE;

	// Go through all the columns of the table to verify fixed length BYREF bind support.
	for (DBORDINAL iCol=0; iCol<cColumns; iCol++)
	{
		// If this is a fixed length column
		if(pColInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH)
		{
			// See if we support BYREF conversion
			wFromType = pColInfo[iCol].wType;

			// We always bind natively except for bookmarks which are always bound to DBTYPE_BYTES
			wToType = wFromType;
			if (!pColInfo[iCol].iOrdinal)
				wToType = DBTYPE_BYTES;

			hr = pIConvertType->CanConvert(wFromType, wToType|DBTYPE_BYREF, DBCONVERTFLAGS_COLUMN);

			if (S_OK == hr)
				cFixedColsByRef++;
			else if (S_FALSE == hr)
				eSupport = FIXED_BYREF_SOME;
			else
				CHECK(hr, S_OK);
		}

	}

	if (!cFixedColsByRef)
		eSupport = FIXED_BYREF_NONE;

	// Currently we have no providers that support some fixed cols by ref.  Test 
	// will need updating if this occurs

	// Note: there's currently a spec issue where CanConvert will return S_OK for fixed length
	// binary or char columns even though at this time no providers actually support this conversion.
	// Currently CanConvert doesn't know anything about fixed vs. variable length conversions.
	if (eSupport == FIXED_BYREF_SOME)
		odtLog << L"Please update this test to support SOME fixed length columns bound BYREF.\n";

	SAFE_RELEASE(pIConvertType);
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(pColInfo);
	PROVIDER_FREE(pStringsBuffer);

	return eSupport;

}

//Dummy class only defined so that CRowsetObject can
//be instantiated as an object without inheriting
//from a real testcase
class CSetRowsetObject : public CRowsetObject
{
public:

	//@cmember CTOR
	CSetRowsetObject(LPWSTR tcName, CThisTestModule *pMod, ICommand * pICommand = NULL,
		CTable * pTable = NULL):CRowsetObject(tcName)
	{
		ASSERT(pMod);
		SetOwningMod(0, pMod);
		if (pICommand)
			SetCommandObject(pICommand);
		m_pTable = pTable;
	};

	~CSetRowsetObject()
	{
		// m_pICommand is NULLed by the CCommand constructor in the base class, so
		// it will only be non-NULL if we passed a pointer in the constructor
		SAFE_RELEASE(m_pICommand);	// Because SetCommandObject addrefed it.
	}

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCCrtRtnValsAfterGetRows)
//--------------------------------------------------------------------
// @class Return values for all CreateAccessor error conditions
//
class TCCrtRtnValsAfterGetRows : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCrtRtnValsAfterGetRows,CAccessor);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - Null phAccessor
	int Variation_1();
	// @cmember DB_E_BADBINDINFO - Invalid dwPart
	int Variation_2();
	// @cmember DB_E_BADBINDINFO - wType is DBTYPE_EMPTY
	int Variation_3();
	// @cmember DB_E_BADBINDINFO - PASSBYREF without correct buffer format
	int Variation_4();
	// @cmember DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's
	int Variation_5();
	// @cmember DB_E_BADBINDINFO - PARAMETERDATA and Multiple Input bindings with same ordinal
	int Variation_6();
	// @cmember DB_E_BADACCESSORFLAGS - PASSBYREF when DBPROP_BYREFACCESSORS is FALSE
	int Variation_7();
	// @cmember DB_E_BADBINDINFO - PARAMETERDATA with invalid eParamIO
	int Variation_8();
	// @cmember DB_E_BADACCESSORFLAGS - OPTIMIZED created after GetNextRows
	int Variation_9();
	// @cmember DB_E_BADBINDINFO - Second optimized accessor using same column
	int Variation_10();
	// @cmember DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor
	int Variation_11();
	// @cmember DB_E_BADORDINAL - iOrdinal of largest column number + 1
	int Variation_12();
	// @cmember DB_E_BADORDINAL - iOrdinal of max value for ULONG
	int Variation_13();
	// @cmember DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_EMPTY
	int Variation_14();
	// @cmember DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_NULL
	int Variation_15();
	// @cmember DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_RESERVED
	int Variation_16();
	// @cmember DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_BYREF
	int Variation_17();
	// @cmember DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_VECTOR
	int Variation_18();
	// @cmember DB_E_BADBINDINFO - DBTYPE_VECTOR | DBTYPE_BYREF
	int Variation_19();
	// @cmember S_OK - PASSBYREF and DBMEMOWNER_PROVIDEROWNED
	int Variation_20();
	// @cmember DB_E_BADACCESSORFLAGS - Invalid DBACCESSORFLAGS
	int Variation_21();
	// @cmember DB_E_BADORDINAL - iOrdinal = 0 for PARAMETERDATA accessor
	int Variation_22();
	// @cmember DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks
	int Variation_23();
	// @cmember DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with PARAMETERDATA
	int Variation_24();
	// @cmember S_OK - PARAMETERDATA and Multiple OUTPUT bindings with same ordinal
	int Variation_25();
	// @cmember DB_E_BADBINDINFO - wType is DBTYPE_NULL
	int Variation_26();
	// @cmember E_INVALID - cBindings != 0 and rgBindings = NULL
	int Variation_27();
	// @cmember DB_E_BADACCESSORFLAGS - dwAccessorFlags = OPTIMIZED only
	int Variation_28();
	// @cmember DB_E_BADACCESSORFLAGS - dwAccessorFlags = INVALID only
	int Variation_29();
	// @cmember DB_E_NULLACCESSORNOTSUPPORTED - Null accessor created on Command Object
	int Variation_30();
	// @cmember PARAMETERDATA accessor on Rowset Object
	int Variation_31();
	// @cmember DB_E_ERRORSOCCURRED - PROVIDEROWNED dwMemOwner for non pointer types
	int Variation_32();
	// @cmember DB_E_BADBINDINFO - DBTYPE_IUnknown and NULL pObject
	int Variation_33();
	// @cmember DB_E_BADBINDINFO: DBBINDFLAG_HTML for non-string type
	int Variation_34();
	// @cmember DB_E_BADBINDINFO: dwFlags invalid
	int Variation_35();
	// @cmember DB_E_NULLACCESSORNOTSUPPORTED - Null accessor on rowset with IRowsetChange FALSE
	int Variation_36();
	// @cmember DB_E_BADBINDINFO: dwMemOwner invalid
	int Variation_37();
	// }} TCW_TESTVARS_END

	//@cmember Does CreateAccessor with a command rowdata accessor, expecting
	//the given binding to have the given status
	HRESULT TestCommandAccessor(DBBINDSTATUS status, LONG lBinding, enum DEFER_MODE eDeferMode,
		DBACCESSORFLAGS dwAccessorFlags);
};
// {{ TCW_TESTCASE(TCCrtRtnValsAfterGetRows)
#define THE_CLASS TCCrtRtnValsAfterGetRows
BEG_TEST_CASE(TCCrtRtnValsAfterGetRows, CAccessor, L"Return values for all CreateAccessor error conditions")
	TEST_VARIATION(1, 		L"E_INVALIDARG - Null phAccessor")
	TEST_VARIATION(2, 		L"DB_E_BADBINDINFO - Invalid dwPart")
	TEST_VARIATION(3, 		L"DB_E_BADBINDINFO - wType is DBTYPE_EMPTY")
	TEST_VARIATION(4, 		L"DB_E_BADBINDINFO - PASSBYREF without correct buffer format")
	TEST_VARIATION(5, 		L"DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's")
	TEST_VARIATION(6, 		L"DB_E_BADBINDINFO - PARAMETERDATA and Multiple Input bindings with same ordinal")
	TEST_VARIATION(7, 		L"DB_E_BADACCESSORFLAGS - PASSBYREF when DBPROP_BYREFACCESSORS is FALSE")
	TEST_VARIATION(8, 		L"DB_E_BADBINDINFO - PARAMETERDATA with invalid eParamIO")
	TEST_VARIATION(9, 		L"DB_E_BADACCESSORFLAGS - OPTIMIZED created after GetNextRows")
	TEST_VARIATION(10, 		L"DB_E_BADBINDINFO - Second optimized accessor using same column")
	TEST_VARIATION(11, 		L"DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor")
	TEST_VARIATION(12, 		L"DB_E_BADORDINAL - iOrdinal of largest column number + 1")
	TEST_VARIATION(13, 		L"DB_E_BADORDINAL - iOrdinal of max value for ULONG")
	TEST_VARIATION(14, 		L"DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_EMPTY")
	TEST_VARIATION(15, 		L"DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_NULL")
	TEST_VARIATION(16, 		L"DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_RESERVED")
	TEST_VARIATION(17, 		L"DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_BYREF")
	TEST_VARIATION(18, 		L"DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_VECTOR")
	TEST_VARIATION(19, 		L"DB_E_BADBINDINFO - DBTYPE_VECTOR | DBTYPE_BYREF")
	TEST_VARIATION(20, 		L"S_OK - PASSBYREF and DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(21, 		L"DB_E_BADACCESSORFLAGS - Invalid DBACCESSORFLAGS")
	TEST_VARIATION(22, 		L"DB_E_BADORDINAL - iOrdinal = 0 for PARAMETERDATA accessor")
	TEST_VARIATION(23, 		L"DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks")
	TEST_VARIATION(24, 		L"DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with PARAMETERDATA")
	TEST_VARIATION(25, 		L"S_OK - PARAMETERDATA and Multiple OUTPUT bindings with same ordinal")
	TEST_VARIATION(26, 		L"DB_E_BADBINDINFO - wType is DBTYPE_NULL")
	TEST_VARIATION(27, 		L"E_INVALID - cBindings != 0 and rgBindings = NULL")
	TEST_VARIATION(28, 		L"DB_E_BADACCESSORFLAGS - dwAccessorFlags = OPTIMIZED only")
	TEST_VARIATION(29, 		L"DB_E_BADACCESSORFLAGS - dwAccessorFlags = INVALID only")
	TEST_VARIATION(30, 		L"DB_E_NULLACCESSORNOTSUPPORTED - Null accessor created on Command Object")
	TEST_VARIATION(31, 		L"PARAMETERDATA accessor on Rowset Object")
	TEST_VARIATION(32, 		L"DB_E_ERRORSOCCURRED - PROVIDEROWNED dwMemOwner for non pointer types")
	TEST_VARIATION(33, 		L"DB_E_BADBINDINFO - DBTYPE_IUnknown and NULL pObject")
	TEST_VARIATION(34, 		L"DB_E_BADBINDINFO: DBBINDFLAG_HTML for non-string type")
	TEST_VARIATION(35, 		L"DB_E_BADBINDINFO: dwFlags invalid")
	TEST_VARIATION(36, 		L"DB_E_NULLACCESSORNOTSUPPORTED - Null accessor on rowset with IRowsetChange FALSE")
	TEST_VARIATION(37, 		L"DB_E_BADBINDINFO: dwMemOwner invalid")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCreateValidRowAccessors)
//--------------------------------------------------------------------
// @class Creation of valid row accessors
//
class TCCreateValidRowAccessors : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPSET * m_pDBPropSet;
	DBPROP * m_prgDBProps;

	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateValidRowAccessors,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	//check whether the DBPROP_IRowsetChange and DBPROP_UPDATABILITY
	BOOL	m_fPropertiesSet; 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	
	// {{ TCW_TESTVARS()
	// @cmember Null Accessor
	int Variation_1();
	// @cmember dwPart = DBPART_VALUE
	int Variation_2();
	// @cmember dwPart = DBPART_LENGTH
	int Variation_3();
	// @cmember dwPart = DBPART_STATUS
	int Variation_4();
	// @cmember dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS
	int Variation_5();
	// @cmember dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS | DBCOLUMPART_VALUE
	int Variation_6();
	// @cmember One optmized, one non optimized, using same fields
	int Variation_7();
	// @cmember One non-optimized, one optimized, using same fields - different creation sequence
	int Variation_8();
	// @cmember One optimized, one non-optimized, using different fields
	int Variation_9();
	// @cmember Two optimized, using different fields
	int Variation_10();
	// @cmember Two non-optimized, using same fields
	int Variation_11();
	// @cmember Multiple bindings for same column in one accessor
	int Variation_12();
	// @cmember All types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
	int Variation_13();
	// @cmember Fixed types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
	int Variation_14();
	// @cmember Variable types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
	int Variation_15();
	// @cmember All types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
	int Variation_16();
	// @cmember Fixed types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
	int Variation_17();
	// @cmember Variable types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
	int Variation_18();
	// @cmember No types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
	int Variation_19();
	// @cmember Use accessor after release rows
	int Variation_20();
	// @cmember dwFlags: DBBINDFLAG_HTML
	int Variation_21();
	// @cmember DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED
	int Variation_22();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCreateValidRowAccessors)
#define THE_CLASS TCCreateValidRowAccessors
BEG_TEST_CASE(TCCreateValidRowAccessors, CAccessor, L"Creation of valid row accessors")
	TEST_VARIATION(1, 		L"Null Accessor")
	TEST_VARIATION(2, 		L"dwPart = DBPART_VALUE")
	TEST_VARIATION(3, 		L"dwPart = DBPART_LENGTH")
	TEST_VARIATION(4, 		L"dwPart = DBPART_STATUS")
	TEST_VARIATION(5, 		L"dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS")
	TEST_VARIATION(6, 		L"dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS | DBCOLUMPART_VALUE")
	TEST_VARIATION(7, 		L"One optmized, one non optimized, using same fields")
	TEST_VARIATION(8, 		L"One non-optimized, one optimized, using same fields - different creation sequence")
	TEST_VARIATION(9, 		L"One optimized, one non-optimized, using different fields")
	TEST_VARIATION(10, 		L"Two optimized, using different fields")
	TEST_VARIATION(11, 		L"Two non-optimized, using same fields")
	TEST_VARIATION(12, 		L"Multiple bindings for same column in one accessor")
	TEST_VARIATION(13, 		L"All types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(14, 		L"Fixed types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(15, 		L"Variable types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(16, 		L"All types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(17, 		L"Fixed types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(18, 		L"Variable types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(19, 		L"No types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(20, 		L"Use accessor after release rows")
	TEST_VARIATION(21, 		L"dwFlags: DBBINDFLAG_HTML")
	TEST_VARIATION(22, 		L"DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCreateValidParamAccessors)
//--------------------------------------------------------------------
// @class Creation of valid parameter accessors
//
class TCCreateValidParamAccessors : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember Save global table pointer for the testcase
	CTable *m_pSaveTable;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateValidParamAccessors,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROWDATA and PARAMETERDATA accessor
	int Variation_1();
	// @cmember Null Parameter Accessor
	int Variation_2();
	// @cmember dwPart - DBCOLUMPART_VALUE
	int Variation_3();
	// @cmember dwPart - DBCOLUMPART_LENGTH
	int Variation_4();
	// @cmember dwPart - DBCOLUMPART_STATUS
	int Variation_5();
	// @cmember dwPart - DBCOLUMPART_VALUE | DBPART_STATUS
	int Variation_6();
	// @cmember dwPart - DBCOLUMPART_LENGTH | DBPART_STATUS | DBPART_VALUE
	int Variation_7();
	// @cmember One optimized, one non optimized, using same fields
	int Variation_8();
	// @cmember One non-optimized, one optimized, using same fields - different creation sequence
	int Variation_9();
	// @cmember Two non-optimized, using same fields
	int Variation_10();
	// @cmember All searchable cols BY_REF, without DBMEMOWNER_PROVIDEROWNED
	int Variation_11();
	// @cmember More than 256 parameters
	int Variation_12();
	// @cmember Use DBBINDFLAGS_HTML in parameter accessor
	int Variation_13();
	// @cmember DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED
	int Variation_14();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCreateValidParamAccessors)
#define THE_CLASS TCCreateValidParamAccessors
BEG_TEST_CASE(TCCreateValidParamAccessors, CAccessor, L"Creation of valid parameter accessors")
	TEST_VARIATION(1, 		L"ROWDATA and PARAMETERDATA accessor")
	TEST_VARIATION(2, 		L"Null Parameter Accessor")
	TEST_VARIATION(3, 		L"dwPart - DBCOLUMPART_VALUE")
	TEST_VARIATION(4, 		L"dwPart - DBCOLUMPART_LENGTH")
	TEST_VARIATION(5, 		L"dwPart - DBCOLUMPART_STATUS")
	TEST_VARIATION(6, 		L"dwPart - DBCOLUMPART_VALUE | DBPART_STATUS")
	TEST_VARIATION(7, 		L"dwPart - DBCOLUMPART_LENGTH | DBPART_STATUS | DBPART_VALUE")
	TEST_VARIATION(8, 		L"One optimized, one non optimized, using same fields")
	TEST_VARIATION(9, 		L"One non-optimized, one optimized, using same fields - different creation sequence")
	TEST_VARIATION(10, 		L"Two non-optimized, using same fields")
	TEST_VARIATION(11, 		L"All searchable cols BY_REF, without DBMEMOWNER_PROVIDEROWNED")
	TEST_VARIATION(12, 		L"More than 256 parameters")
	TEST_VARIATION(13, 		L"Use DBBINDFLAGS_HTML in parameter accessor")
	TEST_VARIATION(14, 		L"DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAccessorSequencing)
//--------------------------------------------------------------------
// @class Accessors created and used in various sequences
//
class TCAccessorSequencing : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAccessorSequencing,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember One accessor created on command, one on rowset
	int Variation_1();
	// @cmember One accessor created on command, one on rowset, using different fields
	int Variation_2();
	// @cmember Two ROWDATA accessors on command object, one before and one after execute
	int Variation_3();
	// @cmember Create and use accessor after Setting command
	int Variation_4();
	// @cmember Create and use accessor after reading data via IRowset
	int Variation_5();
	// @cmember Create and use accessor after GetData
	int Variation_6();
	// @cmember ReleaseAccessor at different times
	int Variation_7();
	// @cmember Use Command accessor after freeing command object
	int Variation_8();
	// @cmember CreateAccessor from GetBindings and Release, validate pBinding
	int Variation_9();
	// @cmember CreateAccessor on new command object
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAccessorSequencing)
#define THE_CLASS TCAccessorSequencing
BEG_TEST_CASE(TCAccessorSequencing, CAccessor, L"Accessors created and used in various sequences")
	TEST_VARIATION(1, 		L"One accessor created on command, one on rowset")
	TEST_VARIATION(2, 		L"One accessor created on command, one on rowset, using different fields")
	TEST_VARIATION(3, 		L"Two ROWDATA accessors on command object, one before and one after execute")
	TEST_VARIATION(4, 		L"Create and use accessor after Setting command")
	TEST_VARIATION(5, 		L"Create and use accessor after reading data via IRowset")
	TEST_VARIATION(6, 		L"Create and use accessor after GetData")
	TEST_VARIATION(7, 		L"ReleaseAccessor at different times")
	TEST_VARIATION(8, 		L"Use Command accessor after freeing command object")
	TEST_VARIATION(9, 		L"CreateAccessor from GetBindings and Release, validate pBinding")
	TEST_VARIATION(10, 		L"CreateAccessor on new command object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCBookMarkRowset)
//--------------------------------------------------------------------
// @class Create accessors for rowsets with bookmarks
//
class TCBookMarkRowset : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBookMarkRowset,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Bind bookmark column with ROWDATA
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCBookMarkRowset)
#define THE_CLASS TCBookMarkRowset
BEG_TEST_CASE(TCBookMarkRowset, CAccessor, L"Create accessors for rowsets with bookmarks")
	TEST_VARIATION(1, 		L"Bind bookmark column with ROWDATA")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetBindingsRtnVals)
//--------------------------------------------------------------------
// @class Return values for all GetBindings error conditions
//
class TCGetBindingsRtnVals : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetBindingsRtnVals,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - Null pdwAccessorFlags
	int Variation_1();
	// @cmember E_INVALIDARG - Null pcBindings
	int Variation_2();
	// @cmember E_INVALIDARG - Null prgBindings
	int Variation_3();
	// @cmember DB_E_BADACCESSORHANDLE -  Null hAccessor
	int Variation_4();
	// @cmember DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetBindingsRtnVals)
#define THE_CLASS TCGetBindingsRtnVals
BEG_TEST_CASE(TCGetBindingsRtnVals, CAccessor, L"Return values for all GetBindings error conditions")
	TEST_VARIATION(1, 		L"E_INVALIDARG - Null pdwAccessorFlags")
	TEST_VARIATION(2, 		L"E_INVALIDARG - Null pcBindings")
	TEST_VARIATION(3, 		L"E_INVALIDARG - Null prgBindings")
	TEST_VARIATION(4, 		L"DB_E_BADACCESSORHANDLE -  Null hAccessor")
	TEST_VARIATION(5, 		L"DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCReleaseAccessorRtnVals)
//--------------------------------------------------------------------
// @class Return values for all ReleaseAccessor error conditions
//
class TCReleaseAccessorRtnVals : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCReleaseAccessorRtnVals,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Release Rowset without releasing rowset Accessor
	int Variation_1();
	// @cmember DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor
	int Variation_2();
	// @cmember DB_E_OPENOBJECT - Release command accessor while rowset is open
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCReleaseAccessorRtnVals)
#define THE_CLASS TCReleaseAccessorRtnVals
BEG_TEST_CASE(TCReleaseAccessorRtnVals, CAccessor, L"Return values for all ReleaseAccessor error conditions")
	TEST_VARIATION(1, 		L"S_OK - Release Rowset without releasing rowset Accessor")
	TEST_VARIATION(2, 		L"DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor")
	TEST_VARIATION(3, 		L"DB_E_OPENOBJECT - Release command accessor while rowset is open")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCDeferredColumns)
//--------------------------------------------------------------------
// @class Use of Deferred and CacheDeferred properties
//
class TCDeferredColumns : public CAccessor { 
private:
	
	// @cmember Flag indicating if Deferred column property is supported
	BOOL		m_fDeferredSupported;

	// @cmember Flag indicating if Deferred column caching property is supported
	BOOL		m_fCacheSupported;

	// @cmember Accessor with only fixed length columns bound
	HACCESSOR	m_FixedAccessor;

	// @cmember Accessor with only variable length columns bound
	HACCESSOR	m_VariableAccessor;

	// @cmember Accessor with all rowset columns bound, used on GetRowset
	HACCESSOR	m_GetAllAccessor;
	 			
	// @cmember Accessor with all rowset columns bound, used on SetRowset
	HACCESSOR	m_SetAllAccessor;

	// @cmember Bindings for FixedAccessor
	DBBINDING * m_rgFixedBindings;

	 // @cmember Bindings for VariableAccessor
	DBBINDING * m_rgVariableBindings;

	// @cmember Bindings for GetAllAccessor
	DBBINDING * m_rgGetAllBindings;

	// @cmember Bindings for SetAllAccessor
	DBBINDING * m_rgSetAllBindings;

	// @cmember Count of bindings in m_rgFixedBindings
	DBCOUNTITEM	m_cFixedBindings;

	// @cmember Count of bindings in m_rgVariableBindings
	DBCOUNTITEM	m_cVariableBindings;

	// @cmember Count of bindings in m_rgGetAllBindings
	DBCOUNTITEM	m_cGetAllBindings;

	// @cmember Count of bindings in m_rgSetAllBindings
	DBCOUNTITEM	m_cSetAllBindings;

	// @cmember Count of bytes needed for a single row using m_FixedAccessor
	DBLENGTH	m_cbFixedRowSize;	
	
	// @cmember Count of bytes needed for a single row using m_VariableAccessor
	DBLENGTH	m_cbVariableRowSize;
	
	// @cmember Count of bytes needed for a single row using m_GetAllAccessor 
	DBLENGTH	m_cbGetAllRowSize;

	// @cmember Count of bytes needed for a single row using m_SetAllAccessor 
	DBLENGTH	m_cbSetAllRowSize;

	// @cmember IRowset interface to use for retrieving data
	IRowset *	m_pGetIRowset;

	// @cmember IRowset interface to use for setting data concurrently to back end
	IRowset *	m_pSetIRowset;

	// @cmember IRowsetChange interface for actual setting of data on set rowset
	IRowsetChange *	m_pSetIRowsetChange;

	// @cmember Array of columns ids for rowset
	DBID	* m_rgDBIDs;

	//@cmember Encapsulated rowset object, used to Set Data
	//so the data differs from what is cached.
	CSetRowsetObject	* m_pSetRowset;

	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDeferredColumns,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	//@cmember Sets row of data in table to PRIMARY or SECONDARY
	HRESULT	SetData(EVALUE	eValue=PRIMARY);

	//@cmember Releases current m_pGetIRowset, sets deferred properties requested and generates new rowset
	HRESULT	SetDeferredProperties(BOOL fDeferred, BOOL fCacheDeferred, DBCOUNTITEM cBindings, DBBINDING * rgBindings);

	//@cmember Copies test case info from testcase to encapsulated CRowset object
	void CopyTestCaseInfo(CTestCases * pTC);

	// {{ TCW_TESTVARS()
	// @cmember Deferred on, CacheDeferred off - All Columns
	int Variation_1();
	// @cmember Deferred on, CacheDeferred on - Fixed Columns
	int Variation_2();
	// @cmember Deferred on, CacheDeferred on - Variable Columns
	int Variation_3();
	// @cmember Deferred on, CacheDeferred on - All Columns
	int Variation_4();
	// @cmember Deferred off, CacheDeferred on - All Columns
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCDeferredColumns)
#define THE_CLASS TCDeferredColumns
BEG_TEST_CASE(TCDeferredColumns, CAccessor, L"Use of Deferred and CacheDeferred properties")
	TEST_VARIATION(1, 		L"Deferred on, CacheDeferred off - All Columns")
	TEST_VARIATION(2, 		L"Deferred on, CacheDeferred on - Fixed Columns")
	TEST_VARIATION(3, 		L"Deferred on, CacheDeferred on - Variable Columns")
	TEST_VARIATION(4, 		L"Deferred on, CacheDeferred on - All Columns")
	TEST_VARIATION(5, 		L"Deferred off, CacheDeferred on - All Columns")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCommandAccessorTransactions)
//--------------------------------------------------------------------
// @class Commit/Abort behavior for Command Accessors
//
class TCCommandAccessorTransactions : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCommandAccessorTransactions,CTransaction);
	// }} TCW_DECLARE_FUNCS_END

	//Tests commit/abort with respect to IAccessor on commands
	int TestTxn(ETXN	eTxn, BOOL	fRetaining);
	
	//@cmember IAccessor on command object
	IAccessor * 	m_pIAccessor;

	//@cmember Accessor handle
	HACCESSOR		m_hAccessor;

	//@cmember Array of bindings to use for CreateAccessor
	DBBINDING 		m_rgBindings[1];

	//@cmember Count of bindings in binding array
	DBCOUNTITEM		m_cBindings;

	//@cmember Holds the size of one row of data for the accessor
	DBLENGTH		m_cbRowSize;

	//@cmember Holds flags for the accessor
	ULONG			m_dwFlags;

	//@cmember Pointer to array of bindings from GetBindings
	DBBINDING *		m_rgGetBindings;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit Retaining
	int Variation_1();
	// @cmember Commit Non Retaining
	int Variation_2();
	// @cmember Abort Retaining
	int Variation_3();
	// @cmember Abort Non Retaining
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCommandAccessorTransactions)
#define THE_CLASS TCCommandAccessorTransactions
BEG_TEST_CASE(TCCommandAccessorTransactions, CTransaction, L"Commit/Abort behavior for Command Accessors")
	TEST_VARIATION(1, 		L"Commit Retaining")
	TEST_VARIATION(2, 		L"Commit Non Retaining")
	TEST_VARIATION(3, 		L"Abort Retaining")
	TEST_VARIATION(4, 		L"Abort Non Retaining")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowsetAccessorTransactions)
//--------------------------------------------------------------------
// @class Commit/Abort behavior for Rowset Accessors
//
class TCRowsetAccessorTransactions : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetAccessorTransactions,CTransaction);
	// }} TCW_DECLARE_FUNCS_END

	//@cmember Tests zombie IAccessor states on rowset 
	int TCRowsetAccessorTransactions::TestTxn(ETXN	eTxn, BOOL	fRetaining);

	//@cmember IAccessor on rowset object
	IAccessor * 	m_pIAccessor;

	//@cmember Accessor handle
	HACCESSOR		m_hAccessor;

	//@cmember Array of bindings to use for CreateAccessor
	DBBINDING 		m_rgBindings[1];

	//@cmember Count of bindings in binding array
	DBCOUNTITEM		m_cBindings;

	//@cmember Holds the size of one row of data for the accessor
	ULONG			m_cbRowSize;

	//@cmember Holds flags for the accessor
	ULONG			m_dwFlags;

	//@cmember Pointer to array of bindings from GetBindings
	DBBINDING *		m_rgGetBindings;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit Retaining
	int Variation_1();
	// @cmember Commit Non Retaining
	int Variation_2();
	// @cmember Abort Retaining
	int Variation_3();
	// @cmember Abort Non Retaining
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRowsetAccessorTransactions)
#define THE_CLASS TCRowsetAccessorTransactions
BEG_TEST_CASE(TCRowsetAccessorTransactions, CTransaction, L"Commit/Abort behavior for Rowset Accessors")
	TEST_VARIATION(1, 		L"Commit Retaining")
	TEST_VARIATION(2, 		L"Commit Non Retaining")
	TEST_VARIATION(3, 		L"Abort Retaining")
	TEST_VARIATION(4, 		L"Abort Non Retaining")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IAccessor calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IAccessor calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IAccessor calls with no previous error object existing
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CAccessor, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IAccessor calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IAccessor calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid IAccessor calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCrtRtnValsBeforeGetRows)
//--------------------------------------------------------------------
// @class Return values for all CreateAccessor error conditions
//
class TCCrtRtnValsBeforeGetRows : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCrtRtnValsBeforeGetRows,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_BADBINDINFO - PASSBYREF without correct buffer format
	int Variation_1();
	// @cmember DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's
	int Variation_2();
	// @cmember DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor
	int Variation_3();
	// @cmember DB_E_BADORDINAL - iOrdinal of largest column number + 1
	int Variation_4();
	// @cmember DB_E_BADORDINAL - iOrdinal of max value for ULONG
	int Variation_5();
	// @cmember DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks
	int Variation_6();
	// @cmember DB_E_BADBINDINFO - Some bindings succeeding others failing
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCrtRtnValsBeforeGetRows)
#define THE_CLASS TCCrtRtnValsBeforeGetRows
BEG_TEST_CASE(TCCrtRtnValsBeforeGetRows, CAccessor, L"Return values for all CreateAccessor error conditions")
	TEST_VARIATION(1, 		L"DB_E_BADBINDINFO - PASSBYREF without correct buffer format")
	TEST_VARIATION(2, 		L"DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's")
	TEST_VARIATION(3, 		L"DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor")
	TEST_VARIATION(4, 		L"DB_E_BADORDINAL - iOrdinal of largest column number + 1")
	TEST_VARIATION(5, 		L"DB_E_BADORDINAL - iOrdinal of max value for ULONG")
	TEST_VARIATION(6, 		L"DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks")
	TEST_VARIATION(7, 		L"DB_E_BADBINDINFO - Some bindings succeeding others failing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAddRefAccessor)
//--------------------------------------------------------------------
// @class Test the AddREfAccessor Method
//
class TCAddRefAccessor : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAddRefAccessor,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Regular Addreff on an accessor, S_OK
	int Variation_1();
	// @cmember Addref on Command and Rowset object.
	int Variation_2();
	// @cmember NULL for pcRefCount arguments.
	int Variation_3();
	// @cmember Invalid Accessor
	int Variation_4();
	// @cmember Release accessor on a rowset object.
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAddRefAccessor)
#define THE_CLASS TCAddRefAccessor
BEG_TEST_CASE(TCAddRefAccessor, CAccessor, L"Test the AddREfAccessor Method")
	TEST_VARIATION(1, 		L"Regular Addreff on an accessor, S_OK")
	TEST_VARIATION(2, 		L"Addref on Command and Rowset object.")
	TEST_VARIATION(3, 		L"NULL for pcRefCount arguments.")
	TEST_VARIATION(4, 		L"Invalid Accessor")
	TEST_VARIATION(5, 		L"Release accessor on a rowset object.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAccessorOnAlteredTable)
//--------------------------------------------------------------------
// @class Test to test the validity of accessors once rowset is modified.
//
class TCAccessorOnAlteredTable : public CAccessor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// CLASS VARIABLES

	// @cmember Table object for Alter Table test case
	CTable * m_pAT_Table;

	// @cmember Command text interface pointer.
	ICommandText *m_pATICommandText;

	// @cmember Rowset interface pointer.
	IRowset *m_pATIRowset;

	// @cmember ColumnsRowsetInfo Rowset pointer.
	IRowset *m_pIColumnsRowsetInfo;

	// @cmember IColumnsInfo interface pointer.
	IColumnsInfo *m_pATIColumnsInfo;

	// @cmember IAccessor Interface pointer.
	IAccessor *m_pATIAccessor ;

	// @cmember IColumnsRowset interface pointer.
	IColumnsRowset *m_pATIColumnsRowset;

	// @cmbember Array for storing bindings for the table.
	DBBINDING m_rgATDbBindings[3];

	// @cmember Rowsize for Create accessor.
	DBLENGTH m_cbRowSize ;  // Some safe size.

	// @cmember Number of bindings.
	ULONG m_cATDbBindings;

	// @cmember accessor on the table.
	HACCESSOR m_hATAccessor;

	// @cmember Number of members returned by IcolumnsInfo.
	DBORDINAL m_cColumnsInfo;

	// @cmember Column information returned by IColumnsInfo
	DBCOLUMNINFO *m_rgColumnsInfo;

	// @cmember Strings buffer.
	OLECHAR *m_pStringsBuffer;

	// @cmember Number of rows obtained for ColumnsRowset.
	DBCOUNTITEM m_cColumnsRowsetInfoObtained;

	// @cmember HROWS's for ColumnsRowset.
	HROW m_rgColumnsRowsetInfohRows[3];  // 2 hrows for Now One for Later (after alter table).

	// @cmember Mapping of bindings-to-columns for data comparison.
	DB_LORDINAL m_rgColMap[2];

	// @cmember Object name used, may be View or Procedure name
	WCHAR * m_pwszObjName;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAccessorOnAlteredTable,CAccessor);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Test to verify the validity of accessors on an altered rowset.
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAccessorOnAlteredTable)
#define THE_CLASS TCAccessorOnAlteredTable
BEG_TEST_CASE(TCAccessorOnAlteredTable, CAccessor, L"Test to test the validity of accessors once rowset is modified.")
	TEST_VARIATION(1, 		L"Test to verify the validity of accessors on an altered rowset.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(14, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCCrtRtnValsAfterGetRows)
	TEST_CASE(2, TCCreateValidRowAccessors)
	TEST_CASE(3, TCCreateValidParamAccessors)
	TEST_CASE(4, TCAccessorSequencing)
	TEST_CASE(5, TCBookMarkRowset)
	TEST_CASE(6, TCGetBindingsRtnVals)
	TEST_CASE(7, TCReleaseAccessorRtnVals)
	TEST_CASE(8, TCDeferredColumns)
	TEST_CASE(9, TCCommandAccessorTransactions)
	TEST_CASE(10, TCRowsetAccessorTransactions)
	TEST_CASE(11, TCExtendedErrors)
	TEST_CASE(12, TCCrtRtnValsBeforeGetRows)
	TEST_CASE(13, TCAddRefAccessor)
	TEST_CASE(14, TCAccessorOnAlteredTable)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


HRESULT CAccessor::UseRowAccessorAndVerify(
			HACCESSOR hAccessor,	//@parm [IN] Handle of accessor			
			DBLENGTH cbRowSize,		//@parm [IN] Size of buffer needed for row 
			ULONG ulRowNum,			//@parm [IN] Row number needed for MakeData
			DB_LORDINAL * rgColumnsOrd,	//@parm [IN] Back end ordinals corresponding to rowset cols
			DBCOUNTITEM cColumns,			//@parm [IN] Number of ordinals in rgColumnsOrd array
			DBBINDING * rgBinding,	//@parm [IN] Array of bindings for this accessor	
			DBCOUNTITEM cBinding,			//@parm [IN] Number of bindings in rgBinding array
			BOOL fReadColumnsByRef,	//@parm [IN] Whether or not accessor is READCOLUMNSBYREF
			BOOL fKeepCurrentRows)	//@parm [IN] TRUE indicates hRows are kept for next call,
									//			 FALSE indicates that rows are not kept, and
									//           GetNextRows will be called next time to retrieve
									//			 more rows.
{
	BYTE *		pData = NULL;
	HROW * 		rgOnehRow = &m_hRow;
	ULONG		cRowSkip = 0;
	IRowsetLocate *		pIRowsetLocate = NULL;
	DBBKMARK	rgcbBookmarks[1]={0};
	BYTE *		rgpBookmarks[1]={NULL};

	// Skip any rows before the one(s) we want to verify.
	cRowSkip = ulRowNum-1;	

	m_hr = NOERROR;
	m_cRowsObtained = 0;

	//Allocate a new data buffer
	pData = (BYTE *)m_pIMalloc->Alloc(cbRowSize);
	if (!pData)			
		return ResultFromScode(E_OUTOFMEMORY);
	
	memset(pData, 0, (size_t)cbRowSize);

	//Get the data into our buffer	
	if (m_fIRowset)
	{
		//Only get new rows if user released them last time
		if (!m_hRow)
		{			
			//We could get DB_S_COMMANDREEXECUTED, so just expect success code		
			if (SUCCEEDED(m_pIRowset->RestartPosition(NULL)))
			{
				//Use IRowset to retrieve data
				if (CHECK(m_hr = m_pIRowset->GetNextRows(NULL, cRowSkip, 1, 
					&m_cRowsObtained, (HROW **)&rgOnehRow), S_OK))			
				{	
					CHECK(m_hr = m_pIRowset->GetData(m_hRow, hAccessor, pData),S_OK);
				}
				else
				{
					//So we know that our hRow is no good
					m_hRow = DB_NULL_HROW;
				}
			}
		}
		else
		{
			//Just get data on current row
			m_hr = m_pIRowset->GetData(m_hRow, hAccessor, pData);
			m_cRowsObtained = 1;
		}
			
	}
	else
		//Fail here since IRowset has to be there
		return ResultFromScode(E_NOINTERFACE);

	//Make sure we got the right number of rows, and GetData succeeded, if called
	if (m_cRowsObtained && SUCCEEDED(m_hr))
	{
		//Make sure we have gotten back exactly one row.
		COMPARE(m_cRowsObtained,1);

		// Retrieve any bookmark value here because CompareData frees the BYREF bindngs
		// and because provider-owned memory is freed at ReleaseRows time.
		if (rgBinding && (rgBinding[0].iOrdinal == 0))
		{
			//We need length and value bound for the bookmark column
			if ((rgBinding[0].dwPart & DBPART_LENGTH) && 
				(rgBinding[0].dwPart & DBPART_VALUE))
			{
	
				//Retrieve the bookmark and its length from our row buffer
				rgcbBookmarks[0] = *(LONG *)(pData+rgBinding[0].obLength);
				if (rgBinding[0].wType & DBTYPE_BYREF)
				{
					// We have to allocate a buffer for the bookmark value
					SAFE_ALLOC(rgpBookmarks[0], BYTE, rgcbBookmarks[0]);
					memcpy(rgpBookmarks[0], (BYTE *)*(ULONG **)(pData+rgBinding[0].obValue), (size_t)rgcbBookmarks[0]);
				}
				else
					rgpBookmarks[0] = (BYTE *)(pData+rgBinding[0].obValue);
			}

			//Check status if bound for the bookmark
			if (rgBinding[0].dwPart & DBPART_STATUS) 
			{
				COMPARE(*(ULONG *)(pData+rgBinding[0].obStatus), DBSTATUS_S_OK);
			}
		}
		
		//Verify data value, length and status are what is expected
		if (COMPARE(CompareData(cColumns, rgColumnsOrd, ulRowNum, pData, cBinding, rgBinding,
			m_pTable, m_pIMalloc, PRIMARY), TRUE))
			m_hr = NOERROR;
		else
			m_hr = E_FAIL;

		//Clean up from row retrieval, if user doesn't want to keep the row
		if (m_fIRowset && !fKeepCurrentRows)
		{
			CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL, NULL,NULL),S_OK);		
			m_hRow = DB_NULL_HROW;
		}
		
		//Test bookmark value -- we assume its in the first binding,
		//otherwise we won't validate it.  Note that if the accessor happens to be
		// a NULL accessor here rgBinding can be NULL also.  We skip this section
		// if data validation above failed since likely the bookmark isn't valid.
		if (rgpBookmarks[0] && m_hr == S_OK)
		{
			// We know we have a bookmark, but this doesn't require IRowsetLocate to be supported
			// If it is we'll try it.
			if (!VerifyInterface(m_pIRowset, IID_IRowsetLocate, ROWSET_INTERFACE,
				(IUnknown **)&pIRowsetLocate))
			{
				// We don't report IRowsetLocate support, so we need to exit here with success
				m_hr = S_OK;
				goto CLEANUP;
			}
			
			//We'll get RowsNotReleased if we don't release this held row
			//since we didn't specify CANHOLDROWS.  We'll save the row
			//we get from GetRowsByBookmark so the user still has one.
			if (fKeepCurrentRows)
			{
				CHECK(m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL, NULL,NULL),S_OK);		
				m_hRow = DB_NULL_HROW;
			}

			//Make sure we can get one row back with the bookmark
			if (CHECK(m_hr = pIRowsetLocate->GetRowsByBookmark(NULL, 1, 
				rgcbBookmarks, (const BYTE **)rgpBookmarks, rgOnehRow, NULL),S_OK))
			{
				//Make sure data is from right row
				CHECK(pIRowsetLocate->GetData(rgOnehRow[0], hAccessor, pData), S_OK);
				COMPARE(CompareData(cColumns, rgColumnsOrd, ulRowNum, pData, 
					cBinding, rgBinding, m_pTable, m_pIMalloc, PRIMARY), TRUE);
				
				//Keep this row so we can use it next time if the user has requested it
				if (fKeepCurrentRows)
					m_hRow = rgOnehRow[0];
				else
					CHECK(m_pIRowset->ReleaseRows(1, rgOnehRow, NULL, NULL,NULL),S_OK);		
			}
			
			SAFE_RELEASE(pIRowsetLocate);

			// For BYREF bookmarks we had to allocate a buffer to save the value
			if (rgBinding[0].wType & DBTYPE_BYREF)
				PROVIDER_FREE(rgpBookmarks[0]);
		}

	}

CLEANUP:
	

	//Clean up fixed buffer 
	PROVIDER_FREE(pData);
	
	//Set this to null so next time we will know row has been released		
	if (!fKeepCurrentRows)
		m_hRow = DB_NULL_HROW;

	//Results is either error on set up or results of CompareData()
	return m_hr;


}

HRESULT CAccessor::UseParamAccessorAndVerify(
				HACCESSOR  hAccessor,	//@parm [IN] Accessor to use
				DBACCESSORFLAGS dwAccessorFlags, //@parm [IN] Accessor flags
				DBLENGTH cbRowSize,		//@parm [IN] Size for one row of parameter data
				ULONG ulRowNum,			//@parm [IN] Row number needed for MakeData						
				DBBINDING * rgBindings,	//@parm [IN] Array of bindings for this accessor	
				DBCOUNTITEM cBindings, 		//@parm [IN] Number of bindings in rgBinding array)
				ICommand * pICommand,	//@parm [IN] Command to execute on.This must match accessor command object.
				HRESULT hrExecute,		//@parm [IN] Expected hresult from Execute call.  Default S_OK.
				BOOL fWarn)				//@parm [IN] Whether warning or failure is issued.  Default FALSE.
{	
	DBROWCOUNT cRowsAffected = 0;	
	ULONG iRow, cRows = 2;
	IRowset * rgpRowset[2] = {NULL, NULL};
	DBPARAMS Param;
	BYTE * pData = NULL;	
	DBCOUNTITEM cBindingsFilled = 0;
	
	m_hr = ResultFromScode(E_FAIL);
	
	//Create a command that we will execute with parameters
	TESTC_(m_hr = m_pTable->ExecuteCommand(SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE, IID_IUnknown,
		NULL,NULL,NULL, NULL, EXECUTE_NEVER, 0, NULL, 
		NULL, NULL, &pICommand), S_OK);
	
	//Alloc enough memory to hold a row of parameter data
	pData = (BYTE *)m_pIMalloc->Alloc(cbRowSize);	

	if (!pData)
		goto CLEANUP;
	memset(pData, 0, (size_t)cbRowSize);
	
	for (iRow = 0; iRow < cRows; iRow++)
	{
	
		//Set up parameter input values for selecting row 1
		TESTC_(m_hr = FillInputBindings(m_pTable,  
			dwAccessorFlags, cBindings, rgBindings, &pData, ulRowNum, 
			m_cSearchableCols, m_rgSearchableCols, PRIMARY), S_OK);

		cBindingsFilled = cBindings;
 
		Param.cParamSets = 1;
		Param.hAccessor = hAccessor;
		Param.pData = pData;
		
		m_hr = pICommand->Execute(NULL, IID_IRowset, &Param, 
					&cRowsAffected, (IUnknown **)&rgpRowset[iRow]);

		//Determine which rowset interface is supported by this provider
		//and ask for that interface on the execute
		if (fWarn)
		{
			TESTW_(m_hr, hrExecute);
		}
		else
		{
			TESTC_(m_hr, hrExecute);
		}

		//Init this so only a successful row retrieval will set it back to S_OK
		m_hr = ResultFromScode(E_FAIL);

		if (rgpRowset[iRow])
		{
			// Use IRowset to get rows of data
			
			HROW	* rgOnehRow = (HROW *)m_pIMalloc->Alloc(sizeof(HROW));

			if (!rgOnehRow)
			{
				m_hr = ResultFromScode(E_OUTOFMEMORY);	
				goto CLEANUP;
			}

			TESTC_(rgpRowset[iRow]->GetNextRows(NULL, 0, 1, &m_cRowsObtained, 
				(HROW **)&rgOnehRow),S_OK);

			//One row should have been in rowset
			if (COMPARE(m_cRowsObtained,1))
 				m_hr = NOERROR;
				
			//CLEANUP
			CHECK(rgpRowset[iRow]->ReleaseRows(1,rgOnehRow, NULL, NULL, NULL),S_OK);

			PROVIDER_FREE(rgOnehRow);
		}

		//  Release pData before we set it again.
		ReleaseInputBindingsMemory(cBindingsFilled, rgBindings, pData);
		cBindingsFilled = 0;
	}	

CLEANUP:
			
	ReleaseInputBindingsMemory(cBindingsFilled, rgBindings, pData);
	PROVIDER_FREE(pData);
	SAFE_RELEASE(rgpRowset[0]);
	SAFE_RELEASE(rgpRowset[1]);

	//This should be S_OK if the parameterized query successfully brought back one row
	return m_hr;
	
}

// Finds a combination of data types to match requirements for a particular 
// variation
BOOL CAccessor::FindConversionTypes(
	DBBINDING * prgBindings,
	DBCOUNTITEM cBindings,
	ULONG * piBackEnd,
	DBTYPE * wOptType,
	DBTYPE * wNonOptType)
{
	IConvertType * pIConvertType=NULL;
	IColumnsInfo * pIColumnsInfo=NULL;
	DBORDINAL cColumns=0;
	DBCOLUMNINFO * prgColumnInfo=NULL;
	OLECHAR *	pStringsBuffer=NULL;
	ULONG iOpt, iNonOpt;
	BOOL fFound=FALSE;
	ULONG fNoBookmarks = 1;

	// Get an IConvertType interface
	if (!VerifyInterface(m_pIAccessor, IID_IConvertType, 
		 ROWSET_INTERFACE, (IUnknown **)&pIConvertType))
		return TEST_FAIL;

	// Go through all the provider types
	for (*piBackEnd=0; *piBackEnd < cBindings; (*piBackEnd)++)
	{
		for (iOpt=0; iOpt < cBindings; iOpt++)
		{
			if (S_OK == pIConvertType->CanConvert(prgBindings[*piBackEnd].wType,
				prgBindings[iOpt].wType, DBCONVERTFLAGS_COLUMN))
			{
				for (iNonOpt=0; iNonOpt < cBindings; iNonOpt++)
				{
					/*
					odtLog << L"Conversion from type: " << prgBindings[iOpt].wType <<
						L" to type: " << prgBindings[iNonOpt].wType << L" is supported? " <<
						(BOOL)(pIConvertType->CanConvert(prgBindings[iOpt].wType,
						prgBindings[iNonOpt].wType, DBCONVERTFLAGS_COLUMN) == S_OK) << L"\n";
					*/

					if ((S_FALSE == pIConvertType->CanConvert(prgBindings[iOpt].wType,
						prgBindings[iNonOpt].wType, DBCONVERTFLAGS_COLUMN)) &&
						(prgBindings[*piBackEnd].wType == prgBindings[iNonOpt].wType))
/*
						S_OK	== pIConvertType->CanConvert(prgBindings[*piBackEnd].wType,
						prgBindings[iNonOpt].wType, DBCONVERTFLAGS_COLUMN))
*/
					{
						*wNonOptType=prgBindings[iNonOpt].wType;
						fFound=TRUE;
						goto CLEANUP;
					}
				}
			}
		}
	}

	// None of the provider types worked, try DBTYPE_IUNKNOWN

	// First see if it should be supported (provider supports DBPROP_OLEOBJECTS).
	if (!m_fOLEOBJECTS)
		goto CLEANUP;

	// Now we need to get an IColumnsInfo interface so we can find a LONG column.
	// It is provider specific whether non-LONG cols can be bound to IUNKNOWN, so
	// don't count on it.
	if (!VerifyInterface(m_pIAccessor,IID_IColumnsInfo,ROWSET_INTERFACE,
		(IUnknown **)&pIColumnsInfo))
		goto CLEANUP;

	// Get the columns information
	if (FAILED(pIColumnsInfo->GetColumnInfo(&cColumns, &prgColumnInfo, &pStringsBuffer)))
		goto CLEANUP;

	// See if bookmarks are available on the rowset
	if (!prgColumnInfo[0].iOrdinal)
		fNoBookmarks=0;

	// Find a LONG BLOB column we can bind to IUNKNOWN
	for (*piBackEnd=0; *piBackEnd < cBindings; (*piBackEnd)++)
	{
		if (prgColumnInfo[prgBindings[*piBackEnd].iOrdinal-fNoBookmarks].dwFlags & DBCOLUMNFLAGS_ISLONG &&
			(prgBindings[*piBackEnd].wType == DBTYPE_STR ||
			prgBindings[*piBackEnd].wType == DBTYPE_WSTR ||
			prgBindings[*piBackEnd].wType == DBTYPE_BYTES))
		{
			// Can convert these to IUnknown
			for (iOpt=0; iOpt < cBindings; iOpt++)
			{
				if ((prgBindings[iOpt].wType != DBTYPE_STR &&
					prgBindings[iOpt].wType != DBTYPE_WSTR &&
					prgBindings[iOpt].wType != DBTYPE_BYTES) &&
					S_OK	== pIConvertType->CanConvert(prgBindings[*piBackEnd].wType,
					prgBindings[iOpt].wType, DBCONVERTFLAGS_COLUMN))
				{
					*wNonOptType=DBTYPE_IUNKNOWN;
					fFound=TRUE;
					goto CLEANUP;
				}
			}
		}
	}

CLEANUP:

	SAFE_RELEASE(pIConvertType);
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(prgColumnInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (fFound)
		*wOptType=prgBindings[iOpt].wType;
	
	return fFound;
}


//--------------------------------------------------------------------
// @mfunc Sets the first row of data in the table to the type indicated
// by eValue.  Non updateable columns are skipped.
//
HRESULT	TCDeferredColumns::SetData(EVALUE eValue)
{
	HROW	*	rghRows = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	CCol		CurCol;	
	BYTE *		pData = NULL;
	WCHAR *		wszData = NULL;
		
	//In case we're at the end of the rowset, start from the beginning again
	if (FAILED(m_hr = m_pSetIRowset->RestartPosition(NULL)))
		goto CLEANUP;
	
	//Get a row
	if (!CHECK(m_hr = m_pSetIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&rghRows), S_OK))
		goto CLEANUP;

	pData = (BYTE *)m_pIMalloc->Alloc(m_cbSetAllRowSize);
	if (!pData)
	{
		m_hr = E_OUTOFMEMORY;
		goto CLEANUP;
	}

	//Set data to eValue kind of data, for all updateable columns
	if (!CHECK(m_hr = FillInputBindings(m_pTable, 
		DBACCESSOR_ROWDATA, m_cSetAllBindings, m_rgSetAllBindings, &pData, 
		g_uiRowNum, m_pTable->CountColumnsOnTable(), m_rgTableColOrds, eValue), S_OK))
		goto CLEANUP;

	//This assumes that non updatable cols will just be skipped in the SetData
	if (SUCCEEDED(m_hr = m_pSetIRowsetChange->SetData(rghRows[0], m_SetAllAccessor, pData)))
		//We consider any S hr ok with us
		m_hr = NOERROR;

CLEANUP:
	
	//Cleanup any out of line memory allocated in FillInputBindings
	ReleaseInputBindingsMemory(m_cSetAllBindings, m_rgSetAllBindings, pData);

	if (rghRows)
		m_pSetIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL,NULL);

	PROVIDER_FREE(rghRows);
	PROVIDER_FREE(pData);
	
	return m_hr;

}

// @mfunc Sets Deferred property if the fDeferred flag is set.  
// Sets the fCacheDeferred flag for every column in rgBindings if
// the fCacheDeferred flag is set.
//
// NOTE:  Returns E_FAIL if any of the requested the properties didn't set, else
// it returns S_OK. User should not specify a property which isn't supported
// as the method will return E_FAIL.
//
HRESULT	TCDeferredColumns::SetDeferredProperties(
	BOOL fDeferred,			//Flag indicating if DEFERRED property is set
	BOOL fCacheDeferred,	//Flag indicating if CACHEDEFERRED property is set	
	DBCOUNTITEM cBindings,		//Count of bindings in rgBindings. This is ignored if fCacheDeferred == FALSE.
	DBBINDING * rgBindings	//Bindings corresponding to columns for which CACHEDEFERRED
							//is to be set.  This is ignored if fCacheDeferred == FALSE.
)
{
	
	DBPROPSET	DBPropSet;
	DBPROP	*	rgProps = NULL;
	ULONG		cProperties = 0;
	ULONG		cPropSets = 1;	//This will be one, unless no properties are specified

	//This is the max number of props we'll need,  One for deferred, and
	//one for each binding for cache deferred
	rgProps = (DBPROP *)m_pIMalloc->Alloc((cBindings+1)*sizeof(DBPROP));
	if (!rgProps)
		return ResultFromScode(E_OUTOFMEMORY);
	
	//Free the existing accessors and rowset, so we can start over
	if (m_pIAccessor)
	{
		if (m_GetAllAccessor != DB_NULL_HACCESSOR)			
		{
			m_pIAccessor->ReleaseAccessor(m_GetAllAccessor, NULL);
			m_GetAllAccessor = DB_NULL_HACCESSOR;
		}
		
		if (m_VariableAccessor != DB_NULL_HACCESSOR)			
		{
			m_pIAccessor->ReleaseAccessor(m_VariableAccessor, NULL);
			m_VariableAccessor = DB_NULL_HACCESSOR;
		}
		
		if (m_FixedAccessor != DB_NULL_HACCESSOR)			
		{
			m_pIAccessor->ReleaseAccessor(m_FixedAccessor, NULL);
			m_FixedAccessor = DB_NULL_HACCESSOR;
		}
	}

	if (m_pGetIRowset)
	{
		COMPARE(m_pGetIRowset->Release(), 1);
		m_pGetIRowset = NULL;
	}
	ReleaseRowsetObject();

	//Set up properties used on the CreateRowsetObject call	
	if (fDeferred)
	{	
		//Set properties for Deferred
		rgProps[cProperties].dwPropertyID = DBPROP_DEFERRED;
		rgProps[cProperties].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgProps[cProperties].colid = DB_NULLID;
		rgProps[cProperties].vValue.vt = VT_BOOL;		
		V_BOOL(&(rgProps[cProperties].vValue)) = VARIANT_TRUE;
		//Set deferred for all cols, so we test every type
		rgProps[cProperties].colid=DB_NULLID;
		cProperties++;
	}
	if (fCacheDeferred)
	{
 		//Set properties for CacheDeferred, for each column in the accessor
		while (cBindings)
		{
			
			rgProps[cProperties].dwPropertyID = DBPROP_CACHEDEFERRED;
			rgProps[cProperties].dwOptions = DBPROPOPTIONS_REQUIRED;
			rgProps[cProperties].colid = DB_NULLID;
			rgProps[cProperties].vValue.vt = VT_BOOL;		
			rgProps[cProperties].colid = m_rgDBIDs[(rgBindings[cBindings-1].iOrdinal)-1];
			V_BOOL(&(rgProps[cProperties].vValue)) = VARIANT_TRUE;
			cProperties++;
			cBindings--;
		}
	}	
	
	if (cProperties)
	{
		cPropSets = 1;

		//Set up Prop Set for our rowset properties
		DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
		DBPropSet.cProperties = cProperties;
		DBPropSet.rgProperties = rgProps;
		
	}
	else
		//The SetRowsetProperties call will do nothing,
		//since we have no properties
		cPropSets = 0;

	//Set the properties
	SetRowsetProperties(&DBPropSet, cPropSets);

	//Set m_pIAccessor on a 'select *' rowset with bookmarks
	//We expect every property to be set, so check for S_OK
	if (CHECK(m_hr = CreateRowsetObject(SELECT_VALIDATIONORDER), S_OK))
	{	

		//If we get this far, we've already verified that IRowset is supported
		if (VerifyInterface(m_pIAccessor,IID_IRowset,ROWSET_INTERFACE,
			(IUnknown **)&m_pGetIRowset))
		{
			PROVIDER_FREE(m_rgFixedBindings);

			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_FixedAccessor, &m_rgFixedBindings, &m_cFixedBindings, &m_cbFixedRowSize,			
  				DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
				FIXED_LEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))
			{

				PROVIDER_FREE(m_rgVariableBindings);

				if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
					&m_VariableAccessor, &m_rgVariableBindings, &m_cVariableBindings, &m_cbVariableRowSize,			
  					DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
					VARIABLE_LEN_COLS_BOUND,
					FORWARD, NO_COLS_BY_REF, NULL, NULL,
					NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
					NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
					m_fBindLongCols),S_OK))
				{

					PROVIDER_FREE(m_rgGetAllBindings);
		
					if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
						&m_GetAllAccessor, &m_rgGetAllBindings, &m_cGetAllBindings, &m_cbGetAllRowSize,			
  						DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
						ALL_COLS_BOUND,					
						FORWARD, NO_COLS_BY_REF, NULL, NULL,
						NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
						m_fBindLongCols),S_OK))
					{			
						
						m_hr = NOERROR;
						goto CLEANUP;
					}
				}
			}
		}
	}
	//If we get here, we've fallen through and thus not totally succeeded
	m_hr = ResultFromScode(E_FAIL);				

CLEANUP:
	PROVIDER_FREE(rgProps);
	
	return m_hr;	
}


// {{ TCW_TC_PROTOTYPE(TCCrtRtnValsAfterGetRows)
//*-----------------------------------------------------------------------
//| Test Case:		TCCrtRtnValsAfterGetRows - Return values for all CreateAccessor error conditions
//|	Created:			09/30/95
//*-----------------------------------------------------------------------


//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCrtRtnValsAfterGetRows::Init()
{
	IColumnsInfo * pIColInfo = NULL;
	ULONG	cColumns = 0;
	DBCOLUMNINFO * rgInfo = NULL;
	WCHAR * pStrBuffer = NULL;
	BOOL	fResults = FALSE;
	IRowsetInfo * pIRowsetInfo = NULL;
	IOpenRowset * pIOpenRowset = NULL;

	m_pCmdIAccessor = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		// If m_fBindLongCols is set to BLOB_LONG in CAccessor::Init, then
		// IRowsetLocate is supported on the rowset.  Some providers require
		// IRowsetLocate before BLOB columns can be retrieved via GetData.
		if (m_fBindLongCols == BLOB_LONG)
			SetRowsetProperties(m_pDBPropSetLocate, 1);

		TESTC_(OpenRowsetObject(TRUE), S_OK);

		if (!g_fCmdSupported)
			odtLog << L"The provider does not appear to support commands.\n";

	}

	fResults = TRUE;

CLEANUP:

	return fResults;
}


//--------------------------------------------------------------------
// @mfunc
//Tests creating the accessor given the data members as parameters,
//with the passed in status being expected.
//It is assumed that DB_E_ERRORSOCCURED is always the hr expected
//
// @rdesc TRUE or FALSE
//

HRESULT TCCrtRtnValsAfterGetRows::TestCommandAccessor(DBBINDSTATUS status, LONG lBinding, enum DEFER_MODE eDeferMode,
		DBACCESSORFLAGS dwAccessorFlags)
{

	// Save the value of m_hAccessor since it's used in VerifyError for the row accessor
	// but needs to be populated with the command accessor handle for VerifyError below
	HACCESSOR		hCmdAccessor = DB_NULL_HACCESSOR;
	BOOL			fResults = FALSE;
	ULONG			cRef;
	HRESULT			hrExpect=S_OK;

	//If we don't support a command, just return 
	if (!g_fCmdSupported)
	{
		fResults = TRUE;
		return NOERROR;
	}
	
	// Get the command object ref count
	m_pICommand->AddRef();
	cRef = m_pICommand->Release();

	CSetRowsetObject CommandRowsetObject((LPWSTR)gwszModuleName, m_pThisTestModule, m_pICommand, m_pTable);

	// Tell the rowset object not to delete our table
	CommandRowsetObject.SetTable(m_pTable, DELETETABLE_NO);


	m_hr = m_pCmdIAccessor->CreateAccessor(dwAccessorFlags, m_cBindings,
		m_rgBindings, m_cbRowSize, &hCmdAccessor, m_rgStatus);

	if (eDeferMode == MUST_DEFERR)
	{
		HRESULT hr = E_FAIL;

		switch (status)
		{
			case DBBINDSTATUS_NOINTERFACE:
				hrExpect=E_NOINTERFACE;
				break;
			case DBBINDSTATUS_BADBINDINFO:
				hrExpect=DB_E_BADBINDINFO;
				break;
			case DBBINDSTATUS_BADORDINAL:
				hrExpect=DB_E_BADORDINAL;
				break;
			case DBBINDSTATUS_BADSTORAGEFLAGS:
				hrExpect=DB_E_BADSTORAGEFLAGS;
				break;
			case DBBINDSTATUS_UNSUPPORTEDCONVERSION:
				hrExpect=DB_E_UNSUPPORTEDCONVERSION;
				break;
			default:
				ASSERT(!L"Unknown status for deferred validation.");
		}

		// Before we can attempt to use the accessor we need to open a rowset on the command
		if (SUCCEEDED(hr = CommandRowsetObject.CreateRowsetObject(SELECT_VALIDATIONORDER)))
		{
			// Creating an accessor off the command object never validates against the metadata
			fResults = VerifyError(CommandRowsetObject.m_pIAccessor, hCmdAccessor, hrExpect, DBBINDSTATUS_BADBINDINFO,
				lBinding, m_rgBindings, MUST_DEFERR, MUST_FAIL);
		}
		else
		{
			fResults = CHECK(hr, hrExpect);

		}
	}


	if (eDeferMode == IMMEDIATE)
		// If the error condition can be detected without the metadata it must do so.
		fResults = VerifyError(CommandRowsetObject.m_pIAccessor, hCmdAccessor, hrExpect, status,
			lBinding, m_rgBindings, eDeferMode, MUST_FAIL);

	CommandRowsetObject.ReleaseRowsetObject();
	CommandRowsetObject.ReleaseCommandObject(cRef);
	CommandRowsetObject.ReleaseDBSession();

	if (hCmdAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor(hCmdAccessor, &cRef), S_OK);

	if (fResults)
		return NOERROR;
	else
		return ResultFromScode(E_FAIL);

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - Null phAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_1()
{

	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Null phAccessor should return invalid arg
	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, 1, NULL, m_rgStatus), E_INVALIDARG);

	if (g_fCmdSupported)
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
			m_rgBindings, 1, NULL, m_rgStatus), E_INVALIDARG);
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - Invalid dwPart
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_2()
{
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);
	
	//Set first binding to invalid dwPart
	m_rgBindings[0].dwPart = 0;

	//Expect bad dwPart to cause an error
	if (CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
		fResults = COMPARE(m_rgStatus[0], DBBINDSTATUS_BADBINDINFO);

	//Try a command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO, 0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Reset dwPart to valid value
	m_rgBindings[0].dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}
	  

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - wType is DBTYPE_EMPTY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_3()
{
			
	BOOL	fResults = FALSE;
	DBTYPE	dbType;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Initialize accessor so we can verify it is set to NULL on error
	m_hAccessor = (HACCESSOR)1;

	//Remember valid value for first binding's wType
	dbType = m_rgBindings[0].wType;
	
	//Set first binding wType to DBTYPE_EMPTY 
	m_rgBindings[0].wType = DBTYPE_EMPTY;

	//Expect wType of DBTYPE_EMPTY to cause an error
	if (CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
		fResults = COMPARE(m_rgStatus[0], DBBINDSTATUS_BADBINDINFO);

	//Try a command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO, 0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);

	//Reset wType to previous value
	m_rgBindings[0].wType = dbType;
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - PASSBYREF without correct buffer format
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_4()
{
	
	BOOL	fResults = FALSE;
	DBBYTEOFFSET obValue;
	DBLENGTH	cbMaxLen;	
	
	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//If we support PASSBYREF
	if (m_fPassByRef)
	{
		//Remember the values we will be changing
		obValue = m_rgBindings[0].obValue;
		cbMaxLen = m_rgBindings[0].cbMaxLen;	

		//Ensure our buffer does not match the provider's by
		//setting the cbMaxLen to 0 and the obValue to an offset of 1.
		//A cbMaxLen of 0 should never occur for variable length data.
		//In case of fixed length data, we use an obValue of 1, which is based 
		//on an assumption that no provider will skip one byte and then
		//start the value buffer.	
		m_rgBindings[0].cbMaxLen = 0;
		m_rgBindings[0].obValue = 1;

		//Try PASSBYREF with bindings that don't match provider's buffer layout
		m_hr =m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, 
			m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

		//Verify return code -- should be immediate mode
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			0, m_rgBindings, IMMEDIATE, MUST_FAIL);

		fResults &= SUCCEEDED(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO, 0, MUST_DEFERR,
			DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF));

		//Reset cbMaxLen and value offset to previous values
		m_rgBindings[0].cbMaxLen = cbMaxLen;
		m_rgBindings[0].obValue = obValue;

	}
	else
	{
		//We should fail right away since pass by ref isn't supported
		//Try PASSBYREF with bindings that don't match provider's buffer layout
		CHECKW(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, 
			m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), 
			DB_E_BYREFACCESSORNOTSUPPORTED);
		fResults = TRUE;
	}
			
CLEANUP:
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_5()
{
	
	BOOL	fResults = FALSE;
	DBTYPE	wType;	
	ULONG	i=0;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Find a column which won't convert to IUnknown
	for (i=0; i < m_cBindings; i++)
	{
		if (m_rgBindings[i].wType != DBTYPE_IUNKNOWN &&
			m_rgBindings[i].wType != DBTYPE_IDISPATCH &&
			m_rgBindings[i].wType != DBTYPE_BYTES &&
			m_rgBindings[i].wType != DBTYPE_WSTR &&
			m_rgBindings[i].wType != DBTYPE_STR)
			break;
	}

	//Assume we have at least one non IUnknown convertable column, or we'll fail	
	if (i == m_cBindings)
	{
		odtLog << L"No IUnknown convertable column. \n";
		return TEST_SKIPPED;
	}
	//Remember the wType which we will be changing
	wType = m_rgBindings[i].wType;	

	//Use BYREF on the non IUnknown column's binding, and ensure our requested
	//wType does not match the provider's by using DBTYPE_IUNKNOWN
	m_rgBindings[i].wType = DBTYPE_BYREF | DBTYPE_IUNKNOWN;		
	
	//Make our bindings provider owned for the appropriate column
	m_rgBindings[i].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;

	//Try DBMEMOWNER_PROVIDEROWNED with a binding type which doesn't match the provider's 
	m_hr =m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	
	//Now move it back to client owned for next time
	m_rgBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	
	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		i, m_rgBindings, MAY_DEFERR, MUST_FAIL);
		
	//Reset correct type of binding we changed
	m_rgBindings[i].wType = wType;
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - PARAMETERDATA and Multiple Input bindings with same ordinal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_6()
{	
	BOOL		fResults = FALSE;
	DBBINDING	rgBindings[2];

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//if the provider does not supported the command, skip this test.
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}
	//Build two identical bindings, so they'll have the same ordinal
	//Don't use first binding, in case it's a bookmark
	CopyBindings(&rgBindings[0],&m_rgBindings[1]);	
	CopyBindings(&rgBindings[1],&m_rgBindings[1]);

	//Make sure the ParamIO type is INPUT
	rgBindings[0].eParamIO = DBPARAMIO_INPUT;
	rgBindings[1].eParamIO = DBPARAMIO_INPUT;	

	//Same ordinal for two input parameter bindings should cause error
	if (CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, 2,
		rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))	
		fResults = COMPARE(m_rgStatus[1], DBBINDSTATUS_BADBINDINFO);
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORFLAGS - PASSBYREF when DBPROP_BYREFACCESSORS is FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_7()
{
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

	//If we don't support this, we know what the return code should be 
	if (!m_fPassByRef)			
		CHECKW(m_hr, DB_E_BYREFACCESSORNOTSUPPORTED);

	// If PASSBYREF is supported we don't know the binding layout. 
	// We might fail, or might have given the right layout by accident.

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	//if the provider does not supported the command, skip this test.
	if (g_fCmdSupported)
	{		
		//Try command accessor as well
		if (m_pICommand)
		{	
			m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, m_cBindings,
				m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

			//If we don't support this, we know what the return code should be 
			if (!m_fPassByRef)			
				CHECKW(m_hr, DB_E_BYREFACCESSORNOTSUPPORTED);

			// If PASSBYREF is supported we don't know the binding layout. 
			// We might fail, or might have given the right layout by accident.

			SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_hAccessor);
		}
	}

	//Note that if we do support pass by ref, we'd still need to know the
	//exact buffer format to get CreateAccessor to work correctly, and
	//we don't know that info, so we can't verify anything in this case

	fResults = TRUE;
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - PARAMETERDATA with invalid eParamIO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_8()
{

	BOOL	fResults = FALSE;	

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		return TEST_SKIPPED;
	}

	//if the provider does not supported the command, skip this test.
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}
		
	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Make sure all bindings are valid parameter bindings
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_INPUT);
		
	//Except for one:  Change the first binding we'll use to DBPARAMIO_NOTPARAM
	m_rgBindings[1].eParamIO = DBPARAMIO_NOTPARAM;	
		
	//Now try creating a parameter data accessor, this should fail immediately
	//Make sure we skip the first binding in case it's a bookmark
	if (CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
		m_cBindings-1, &m_rgBindings[1], m_cbRowSize,  
		&m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
		fResults = COMPARE(m_rgStatus[0], DBBINDSTATUS_BADBINDINFO);			
		
	//Now switch back to normal rowdata for the rest of the variations
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_NOTPARAM);
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}
 

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORFLAGS - OPTIMIZED created after GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_9()
{	
	HACCESSOR	hOptAccessor = DB_NULL_HACCESSOR;
	BOOL		fResults = FALSE;
	HRESULT		hr;
	DBBINDING * prgBindings = NULL;
			
	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Try to create an optimized accessor, should fail and set accessor to null
	//Since we have gotten rows in the init function
	if (SUCCEEDED(hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		m_cBindings, m_rgBindings, m_cbRowSize, &hOptAccessor, m_rgStatus)))
	{
		SAFE_RELEASE_ACCESSOR(m_pIAccessor, hOptAccessor);
		odtLog << L"Warning: Creation of an optimized accessor AFTER rows have been retrieved succeeded.\n";
		odtLog << L"         This is usually only valid if the provider doesn't actually support optimized accessors.\n";
		fResults = TRUE;
		goto CLEANUP;
	}
	else
		TESTC_(hr, DB_E_BADACCESSORFLAGS);

	TESTC(NULL == hOptAccessor);
	
	//if the provider does not supported the command, skip this test.
	if (g_fCmdSupported)
	{
		//Command row data accessor should succeed since it is always before get next rows
		//for the rowset which the accessor will be good for
		TESTC_(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			m_cBindings, m_rgBindings, m_cbRowSize, &hOptAccessor, m_rgStatus),
			S_OK);
		
		SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, hOptAccessor);
	}

	if(g_fParamAccessor)
	{
		//Make sure this is ignored for parameter data accessors
		if (g_fCmdSupported)
		{
			DBCOUNTITEM	cBindings=m_cBindings;
			DBLENGTH	cbRowSize=0;

			// We need to fix up the bindings for a parameter accessor.  Cannot bind ordinal 0, nor 
			// can any ordinal be larger than the number of bindings if the provider verifies the ordinal
			// number isn't too large (not all providers do).  Create a parameter binding array.
			if (!(prgBindings = (DBBINDING *)PROVIDER_ALLOC(cBindings * sizeof(DBBINDING))))
			{
				odtLog << L"Out of memory.\n";
				goto CLEANUP;
			}

			// Copy the binding information, but leave out the bookmark binding if it exists.  Otherwise
			// we can get DB_E_BADORDINAL on CreateAccessor.
			if (m_rgBindings[0].iOrdinal == 0)
			{
				cBindings--;
				memcpy(prgBindings, &m_rgBindings[1], (size_t)(cBindings * sizeof(DBBINDING)));
			}
			else
				memcpy(prgBindings, m_rgBindings, (size_t)(cBindings * sizeof(DBBINDING)));

			// Now go through the ordinals and make sure they're sequential in case a hole was left by
			// a long column that wasn't bound.  Otherwise we'll have a parameter ordinal larger than
			// the number of bindings (DB_E_BADORDINAL).
			for (ULONG iBind=0; iBind < cBindings; iBind++)
			{
				prgBindings[iBind].iOrdinal = iBind+1;
				prgBindings[iBind].eParamIO = DBPARAMIO_INPUT;
				cbRowSize+=prgBindings[iBind].cbMaxLen;
			}

			// We finally get to create the accessor
			TESTC_(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
				cBindings, prgBindings, cbRowSize, &hOptAccessor, m_rgStatus), S_OK);

		}
	}

	fResults = TRUE;
	
CLEANUP:

	PROVIDER_FREE(prgBindings);
	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, hOptAccessor);

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - Second optimized accessor using same column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_10()
{

	BOOL		fResults = FALSE;
	HACCESSOR	hOptAccessor = DB_NULL_HACCESSOR;	
	HRESULT		hr;

	// Open the rowset object and don't fetch rows first.
	// Optimized accessors must be created before rows are fetched.
	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Create one optimized accessor
	if (!CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		m_cBindings, m_rgBindings, m_cbRowSize, &hOptAccessor, m_rgStatus),
		S_OK))
		return TEST_FAIL;
	
	
	//Now try to create a second optimized accessor with same first column number
	//This should be detectable immediately.  Note that some providers don't make
	//any distinction between optimized and non-optimized, so this can succeed.
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		1, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
		
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, MAY_DEFERR, MAY_FAIL);

	//Release the first optimized accessor
	if (hOptAccessor && CHECK(m_pIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK))
		hOptAccessor = DB_NULL_HACCESSOR;

	//Release the duplicate HACCESSOR if it was created anyway
	if (m_hAccessor && CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK))
		m_hAccessor = DB_NULL_HACCESSOR;

	//Now try the same thing on a command rowdata accessor
	if (g_fCmdSupported)
	{
		//Create one optimized accessor
		if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			m_cBindings, m_rgBindings, m_cbRowSize, &hOptAccessor, m_rgStatus),
			S_OK))
			return TEST_FAIL;			
		
		//Now try to create a second optimized accessor with same first column number
		//This should be detectable immediately
		m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			1, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
		
		// Now we have to release and recreate the rowset object to make it pick up the new
		// accessors from the command.
		ReleaseRowsetObject();
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		if (!CHECK(hr,S_OK))
			return TEST_FAIL;

		//Verify return code -- should be immediate mode
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			0, m_rgBindings, MAY_DEFERR, MAY_FAIL);

		//Release the first optimized accessor
		if (hOptAccessor && CHECK(m_pCmdIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK))
			hOptAccessor = DB_NULL_HACCESSOR;

		//Release the duplicate HACCESSOR if it was created anyway
		if (m_hAccessor && CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK))
			m_hAccessor = DB_NULL_HACCESSOR;
	}	
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_11()
{
	BOOL		fResults = FALSE;
	HACCESSOR	hOptAccessor = DB_NULL_HACCESSOR;	
	ULONG		iBackEndType;
	DBTYPE		wType, wOptType, wNonOptType;
	HRESULT	    hr;
	BYTE *		pData=NULL;


	// Open the rowset object and don't fetch rows first.
	// Optimized accessors must be created before rows are fetched.
	TESTC_(OpenRowsetObject(FALSE), S_OK);

	/*
	Find binding indexes of bad types for this variation. We need:
		1) Backend DBTYPE can convert to Optimized accessor DBTYPE
		2) Backend DBTYPE can convert to NonOptimized accessor DBTYPE
		3) Optimized accessor DBTYPE can't convert to NonOptimized accessor DBTYPE

	This assumes the existing binding array has all columns bound to have all types
	available.
	*/
	if (!FindConversionTypes(m_rgBindings, m_cBindings, &iBackEndType, &wOptType, &wNonOptType))
	{
		odtLog <<L"Can't find a combination of data types required for this variation.\n";
		return TEST_SKIPPED;
	}

	//Remember value we'll be changing
	wType  = m_rgBindings[iBackEndType].wType;

	// Set the dbtype to a valid conversion but one that can't convert.
	m_rgBindings[iBackEndType].wType = wOptType;

	//Create one optimized accessor with one non convertable column
	if (!CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		1, &m_rgBindings[iBackEndType], m_cbRowSize, &hOptAccessor, m_rgStatus),
		S_OK))
		goto CLEANUP;

	// Now change the type in the binding to other type.
	m_rgBindings[iBackEndType].wType = wNonOptType;

	// If we ended up with a DBTYPE_IUNKNOWN wNonOptType we need to set pObject
	if (DBTYPE_IUNKNOWN == wNonOptType)
		m_rgBindings[iBackEndType].pObject = &m_StorageObject;

	//Now try to create a second accessor for same column, 
	//and use an unsupported coersion
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
		1, &m_rgBindings[iBackEndType], m_cbRowSize, &m_hAccessor, m_rgStatus);
	
	//Verify return code -- may be deferred
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, &m_rgBindings[iBackEndType], MAY_DEFERR, MAY_FAIL);


	//Release the first optimized accessor
	if (hOptAccessor && CHECK(m_pIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK))
		hOptAccessor = DB_NULL_HACCESSOR;

	//Release the duplicate HACCESSOR if it was created anyway
	if (m_hAccessor && CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK))
		m_hAccessor = DB_NULL_HACCESSOR;

	////////////////////////////////////////
	//Now do same thing on command accessor
	////////////////////////////////////////
	if (g_fCmdSupported)
	{
		// Set DBTYPE back for optimized accessor
		m_rgBindings[iBackEndType].wType = wOptType;

		//Create optimized accessor
		if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			1, &m_rgBindings[iBackEndType], m_cbRowSize, &hOptAccessor, m_rgStatus),
			S_OK))
			goto CLEANUP;

		//Set to IUnknown for non-optimized accessor
		m_rgBindings[iBackEndType].wType = wNonOptType;

		//Now try to create a second accessor for same column, 
		//and use an unsupported coersion of DBTYPE_IUNKNOWN
		m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
			1, &m_rgBindings[iBackEndType], m_cbRowSize, &m_hAccessor, m_rgStatus);

		// The rowset opened in the init doesn't know anything about our command accessor
		// so we must release and recreate it
		ReleaseRowsetObject();
		
		if (fResults &= CHECK(hr=CreateRowsetObject(SELECT_VALIDATIONORDER), S_OK))
			// Verify return code -- MAY be deferred even though on command object since metadata not needed.
			// VerifyError needs a pointer to a rowset interface to call GetNextRows and GetData.  
			// CreateRowsetObject places this interface in m_pIAccessor.
			fResults &= VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
				0, &m_rgBindings[iBackEndType], MAY_DEFERR, MAY_FAIL);

		//Release the first optimized accessor
		if (hOptAccessor && CHECK(m_pCmdIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK))
			hOptAccessor = DB_NULL_HACCESSOR;

		//Release the duplicate HACCESSOR if it was created anyway
		if (m_hAccessor && CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK))
			m_hAccessor = DB_NULL_HACCESSOR;
	}

CLEANUP:	

	// Set the binding back to NULL
	if (m_rgBindings)
		m_rgBindings[iBackEndType].pObject = NULL;

	return (fResults) ?  TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal of largest column number + 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_12()
{

	BOOL		fResults = FALSE;
	DBORDINAL	ulRememberOrdinal;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Set column number one too large
	ulRememberOrdinal = m_rgBindings[0].iOrdinal;
	//Number of possible columns plus 1 (for bookmark)
	//plus 1 should be invalid
	m_rgBindings[0].iOrdinal = m_pTable->CountColumnsOnTable()+2;

	//Should be invalid column number
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBindings, 
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	
	//Verify return code
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
		0, m_rgBindings, MAY_DEFERR, MUST_FAIL);

	//Should be OK on Command object since they check no meta data there
	if (g_fCmdSupported)
	{
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBindings, 
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), S_OK);
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	
	//Set correct column number back
	m_rgBindings[0].iOrdinal = ulRememberOrdinal;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal of max value for ULONG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_13()
{
	BOOL		fResults = FALSE;
	ULONG		ulMax;
	DBORDINAL	ulRememberOrdinal;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Get the max number a ULONG can hold
	ulMax = (ULONG)pow(2.0, (double)sizeof(ULONG)*8) - 1;

	//Set column number to max ULONG possible
	ulRememberOrdinal = m_rgBindings[0].iOrdinal;
	m_rgBindings[0].iOrdinal = ulMax;

	//Max number the type can hold should be invalid column number
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

	//Verify return code
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
		0, m_rgBindings, MAY_DEFERR, MUST_FAIL);

	//Should be OK on Command object since they check no meta data there
	if (g_fCmdSupported)
	{
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBindings, 
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), S_OK);
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	//Set correct column number back
	m_rgBindings[0].iOrdinal = ulRememberOrdinal;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_EMPTY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_14()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Try by ref with DBTYPE_EMPTY
	m_rgBindings[0].wType = DBTYPE_BYREF | DBTYPE_EMPTY;
	
	//Create should fail for this combination of dwTypes
	//This error should be detectable immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);	

	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);

	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_15()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Try by ref with DBTYPE_NULL
	m_rgBindings[0].wType = DBTYPE_BYREF | DBTYPE_NULL;

	//Create should fail for this combination of dwTypes
	//This error should be detectable immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	
	//Verify return code -- should be immediate
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);

	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_BYREF with DBTYPE_RESERVED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_16()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Try by ref with DBTYPE_RESERVED
	m_rgBindings[0].wType = DBTYPE_BYREF | DBTYPE_RESERVED;

	//Create should fail for this combination of dwTypes
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	//Verify return code should be immediate
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	
	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_17()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Use mutually exclusive modifiers on simple type
	m_rgBindings[0].wType = DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_UI1;

	//Create should fail for this combination of dwTypes
	//This error should be detectable immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	
	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);

	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_ARRAY | DBTYPE_VECTOR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_18()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Use mutually exclusive modifiers on simple type
	m_rgBindings[0].wType = DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_UI1;

	//Create should fail for this combination of dwTypes
	//This error should be detectable immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);		

	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	
	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_VECTOR | DBTYPE_BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_19()
{
	DBTYPE	wType;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Remember wType we'll be changing
	wType = m_rgBindings[0].wType;

	//Use mutually exclusive modifiers on simple type
	m_rgBindings[0].wType = DBTYPE_BYREF | DBTYPE_VECTOR | DBTYPE_UI1;

	//Create should fail for this combination of dwTypes
	//This error should be detectable immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	
	//Test command accessor as well
	fResults &= CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO,0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);
	
	//Put our old wType back to what is was originally
	m_rgBindings[0].wType = wType;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK - PASSBYREF and DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_20()
{
	//In this variation, we are testing if the DBMEMOWNER_PROVIDEROWNED is
	//ignored when PASSBYREF is specified 
		
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Make all appropriate types bindings provider owned buffers
	AdjustMemOwner(DBMEMOWNER_PROVIDEROWNED, m_cBindings, m_rgBindings);	

	m_hr =m_pIAccessor->CreateAccessor(
		DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF,
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

	// Release the accessor in case it succeeds
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
		
	//We know what to expect if we don't support pass by ref accessors
	if (!m_fPassByRef)								
		CHECKW(m_hr, DB_E_BYREFACCESSORNOTSUPPORTED);

	//but we don't verify anything if we do support them, because
	//we are still required to know the exact buffer layout to succeed and 
	//we don't have that info

	//Do same thing for command accessor
	if (g_fCmdSupported)
	{
		m_hr =m_pCmdIAccessor->CreateAccessor(
			DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF,
			m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
			
		// Release the accessor in case it succeeds
		SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_hAccessor);

		//We know what to expect if we don't support pass by ref accessors
		if (!m_fPassByRef)								
			CHECKW(m_hr, DB_E_BYREFACCESSORNOTSUPPORTED);

		//but we don't verify anything if we do support them, because
		//we are still required to know the exact buffer layout to succeed and 
		//we don't have that info
	}
	
	AdjustMemOwner(DBMEMOWNER_CLIENTOWNED, m_cBindings, m_rgBindings);

	fResults = TRUE;

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORFLAGS - Invalid DBACCESSORFLAGS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_21()
{	
	DBACCESSORFLAGS dwAccessorFlags;
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	if(!g_fParamAccessor)
	{
		//Set up bad value for dwAccessorFlags
		dwAccessorFlags = (ULONG)~(DBACCESSOR_ROWDATA | 
		DBACCESSOR_OPTIMIZED | DBACCESSOR_PASSBYREF);
	}
	else
	{
		//Set up bad value for dwAccessorFlags
		dwAccessorFlags = (ULONG)~(DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA | 
			DBACCESSOR_OPTIMIZED | DBACCESSOR_PASSBYREF);
	}

	//Create should fail for bad DBACCESSORFLAGS - make sure rowdata is included
	//so it gets passed the initial requirement of having the row or paramter bit set.
	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | dwAccessorFlags,
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);
	
	if (g_fCmdSupported)
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | dwAccessorFlags,
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);	
		
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal = 0 for PARAMETERDATA accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_22()
{
	BOOL	fResults = FALSE;	
	DBORDINAL iCol;
	DBTYPE	dbType;

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		return TEST_SKIPPED;
	}

	//Do this on a command object
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}
	
	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Swap in column zero bound to string for first binding
	iCol = m_rgBindings[0].iOrdinal;		
	dbType = m_rgBindings[0].wType;
	m_rgBindings[0].iOrdinal = 0;
	m_rgBindings[0].wType = DBTYPE_STR;
	//Make sure it's valid parameter binding
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_INPUT);
	
	//Column zero should fail immediately 
	m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);	

	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	
	//Set bindings back the way they were
	m_rgBindings[0].iOrdinal = iCol;
	m_rgBindings[0].eParamIO = DBPARAMIO_NOTPARAM;	
	m_rgBindings[0].wType = dbType;
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_NOTPARAM);
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_23()
{
	BOOL	fResults = TRUE;
	DBACCESSORFLAGS dwAccessorFlags;
	DBORDINAL	iCol;
	DBTYPE	dbType;
	DBORDINAL cCols = 0;
	IColumnsInfo * pIColInfo = NULL;
	DBCOLUMNINFO * rgInfo =  NULL;
	WCHAR *	pStrings = NULL;	
	BOOL	fBkmkVisible = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Find out if provider exposes column 0 itself before we go any farther
	if (!VerifyInterface(m_pIAccessor, IID_IColumnsInfo, 
		 ROWSET_INTERFACE, (IUnknown **)&pIColInfo))
		return TEST_FAIL;
	
	if (CHECK(pIColInfo->GetColumnInfo(&cCols, &rgInfo, 
		&pStrings), S_OK))
	{		
		//Bookmark column exists on rowset
		if (rgInfo[0].iOrdinal == 0)
			fBkmkVisible = TRUE;
		
		PROVIDER_FREE(rgInfo);
		PROVIDER_FREE(pStrings);
		SAFE_RELEASE(pIColInfo);

	}
	else
		return TEST_FAIL;

	//Set up parameter flags
	dwAccessorFlags =  DBACCESSOR_ROWDATA;
	
	//Swap in column zero bound to bytes for first binding
	iCol = m_rgBindings[0].iOrdinal;		
	dbType = m_rgBindings[0].wType;
	m_rgBindings[0].iOrdinal = 0;
	m_rgBindings[0].wType = DBTYPE_BYTES;
	
	//Column zero is valid and should succeed
	if (fBkmkVisible)
	{
		if (fResults &= CHECK(m_pIAccessor->CreateAccessor(dwAccessorFlags,
		m_cBindings, m_rgBindings, m_cbRowSize,  
		&m_hAccessor, m_rgStatus), S_OK))
		{
			if (fResults &= CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK))
				m_hAccessor = DB_NULL_HACCESSOR;
		}
	}
	else
	{
		//Column zero should fail as bookmark does not exist
		m_hr = m_pIAccessor->CreateAccessor(dwAccessorFlags,
			m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

		//Verify return code -- should be immediate mode
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
			0, m_rgBindings, IMMEDIATE, MUST_FAIL);
	}

	//Now do accessor on command.  Note that the command can't know for
	//sure if a bookmark will exist on a rowset if IColumnsInfo doesn't
	//report it, yet we should be able to bind that column because
	//a bookmark could exist on the rowset later on
	if (g_fCmdSupported)
		if (CHECK(m_pCmdIAccessor->CreateAccessor(dwAccessorFlags,
			m_cBindings, m_rgBindings, m_cbRowSize,  
			&m_hAccessor, m_rgStatus), S_OK))
			{
				fResults &= CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
				m_hAccessor = DB_NULL_HACCESSOR;
			}


	//Set bindings back the way they were
	m_rgBindings[0].iOrdinal = iCol;
	m_rgBindings[0].wType = dbType;
		

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with PARAMETERDATA
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_24()
{
	BOOL	fResults = FALSE;
	ULONG	i;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	DBBINDING	* rgNoBkMkBindings = NULL;
	DBCOUNTITEM	cNoBkMkBindings = 0;
	DBLENGTH cbRowSize = 0;

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		return TEST_SKIPPED;
	}

	//Do this on a command object
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// We need the bindings bound BYREF for variable length cols
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols), S_OK);

	// Set our bindings to those returned above
	rgNoBkMkBindings = m_rgBindings;
	cNoBkMkBindings = m_cBindings;
	cbRowSize = m_cbRowSize;

	//Skip first binding if it is a bookmark binding
	if (!m_rgBindings[0].iOrdinal)
	{
		rgNoBkMkBindings++;
		cNoBkMkBindings--;
		cbRowSize -= sizeof(DBSTATUS)+sizeof(DBLENGTH)+m_rgBindings[0].cbMaxLen;
	}

	//Make all appropriate types provider owned (this will find
	//all the by ref cols we just created and change them to provider owned)
	AdjustMemOwner(DBMEMOWNER_PROVIDEROWNED, cNoBkMkBindings, rgNoBkMkBindings);

	//Make valid parameter bindings
	SetParamIO(cNoBkMkBindings, rgNoBkMkBindings, DBPARAMIO_INPUT);

	//Now try creating a parameter data accessor, this should fail immediately
	if (CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
		cNoBkMkBindings, rgNoBkMkBindings, cbRowSize,  
		&m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
	{
		//Consider us passed, but COMPARE macro will still increment
		//count for any problems we have
		fResults = TRUE;

		//For each each binding, check status (skipping first binding
		//in case it is a bookmark column, which is not valid for
		//parameterdata accessors and is tested by other variations).
		for (i=0; i<cNoBkMkBindings; i++)
		{
			//For pointer types, we'be changed the memory ownership
			if (rgNoBkMkBindings[i].wType & DBTYPE_BYREF ||
				rgNoBkMkBindings[i].wType & DBTYPE_VECTOR ||
				rgNoBkMkBindings[i].wType & DBTYPE_ARRAY ||
				//Strip off a possible BYREF to check if its a BSTR
				(~(DBTYPE_BYREF) & rgNoBkMkBindings[i].wType)  == DBTYPE_BSTR)

				 fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_BADBINDINFO);						
			else
				//All the other columns' memory owners shouldn't 
				//have been changed, so should be OK
				fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_OK);
		}
	}					
	
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc S_OK - PARAMETERDATA and Multiple OUTPUT bindings with same ordinal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_25()
{
	HRESULT hrExpect=S_OK;
	DBBINDSTATUS stExpect = DBBINDSTATUS_OK;
	DBBINDING	rgBindings[2];
	ULONG		iBind = 0;

	//Do this on a command object
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		hrExpect = DB_E_BADACCESSORFLAGS;
	}
	else if (!g_fOutputParam)
	{
		odtLog << L"This provider doesn't support output parameters\n";
		hrExpect = DB_E_ERRORSOCCURRED;
		stExpect = DBBINDSTATUS_BADBINDINFO;
	}

	// Parameter bindings with ordinal 0 are illegal.
	if (m_rgBindings[iBind].iOrdinal == 0)
		iBind = 1;

	// We need at least one more binding slot 
	if (m_cBindings < iBind + 2)
	{
		odtLog << L"Not enough bindings to complete this variation.\n";
		return TEST_SKIPPED;
	}

	//Build two identical bindings, so they'll have the same ordinal
	//Don't use first binding in case it is a bookmark
	CopyBindings(&rgBindings[0],&m_rgBindings[iBind]);	
	CopyBindings(&rgBindings[1],&m_rgBindings[iBind]);


	// A consumer won't normally bind the same locations, so change them
	// to the next available slot
	rgBindings[1].obValue = m_rgBindings[iBind+1].obValue;
	rgBindings[1].obLength = m_rgBindings[iBind+1].obLength;
	rgBindings[1].obStatus = m_rgBindings[iBind+1].obStatus;

	//Make sure the ParamIO type is OUTPUT
	SetParamIO(2, rgBindings, DBPARAMIO_OUTPUT);

	//Same ordinal for two output parameter bindings should be OK
	if (CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, 2,
		rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus),
		hrExpect))
	{
		// If we got the hresult we expect validate the status
		if (hrExpect != DB_E_BADBINDINFO &&
			COMPARE(m_rgStatus[0], stExpect) &&
			COMPARE(m_rgStatus[1], stExpect))
			return TEST_PASS;	//Everything went as expected

	}

CLEANUP:
	
	return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - wType is DBTYPE_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_26()
{
	BOOL	fResults = FALSE;
	DBTYPE	dbType;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Initialize accessor so we can verify it is set to NULL on error
	m_hAccessor = (HACCESSOR)1;

	//Remember valid value for first binding's wType
	dbType = m_rgBindings[0].wType;
	
	//Set first binding wType to DBTYPE_NULL 
	m_rgBindings[0].wType = DBTYPE_NULL;

	//Expect wType of DBTYPE_NULL to cause an error immediately
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
	
	//Verify return code -- should be immediate mode
	fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, m_rgBindings, IMMEDIATE, MUST_FAIL);

	fResults &=CHECK(TestCommandAccessor(DBBINDSTATUS_BADBINDINFO, 0, IMMEDIATE, DBACCESSOR_ROWDATA), S_OK);

	//Reset wType to previous value
	m_rgBindings[0].wType = dbType;
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc E_INVALID - cBindings != 0 and rgBindings = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_27()
{
	BOOL	fResults = FALSE;	
	
	TESTC_(OpenRowsetObject(TRUE), S_OK);

	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
		NULL, m_cbRowSize, &m_hAccessor, m_rgStatus), E_INVALIDARG);

	if (g_fCmdSupported)
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
			NULL, m_cbRowSize, &m_hAccessor, m_rgStatus), E_INVALIDARG);

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORFLAGS - dwAccessorFlags = OPTIMIZED only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_28()
{
			
	BOOL	fResults = FALSE;	

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_OPTIMIZED, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);

	if (g_fCmdSupported)
		fResults &= CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_OPTIMIZED, m_cBindings,
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORFLAGS - dwAccessorFlags = INVALID only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_29()
{
	BOOL	fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_INVALID, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);

	if (g_fCmdSupported)
		fResults = CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_INVALID, m_cBindings,
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NULLACCESSORNOTSUPPORTED - Null accessor created on Command Object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_30()
{
	
	BOOL		fResults = FALSE;

	//Do this on the command object
	if (!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(TRUE), S_OK);
	
	//Null accessor shouldn't be allowed
	fResults = CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 0,
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), 
			DB_E_NULLACCESSORNOTSUPPORTED);	
		
	
	// Now create a non-null accessor so GetRowsAndData won't fail
	CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), 
			S_OK);

	//Put our state back to having fetched rows for the rest
	//of the variations.  
	CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER), S_OK);

	GetRowsAndData(m_pIAccessor, m_hAccessor, m_cbRowSize);

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc PARAMETERDATA accessor on Rowset Object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_31()
{
	BOOL	fResults = FALSE;

	//if parameter accessor is not supported, skip this variation
	if(!g_fParamAccessor)
	{
		odtLog << wszParamAccesNotSupported;
		return TEST_PASS;
	}

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Make valid parameter bindings
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_INPUT);

	//Skip first binding in case it's a bookmark
	fResults = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings-1,
		&m_rgBindings[1], m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_BADACCESSORFLAGS);

	//Set back to rowdata bindings
	SetParamIO(m_cBindings, m_rgBindings, DBPARAMIO_NOTPARAM);

CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - PROVIDEROWNED dwMemOwner for non pointer types
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsAfterGetRows::Variation_32()
{
	ULONG i;
	BOOL  fResults = FALSE;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Loop thru and make all bindings PROVIDER owned
	for (i=0; i<m_cBindings; i++)
	{
		m_rgBindings[i].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
	}
	
	if (CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))	
	{
		fResults = TRUE;
		for (i=0; i<m_cBindings; i++)	
		{
			//For pointer types, this is valid memory ownership
			if (m_rgBindings[i].wType & DBTYPE_BYREF ||
				m_rgBindings[i].wType & DBTYPE_VECTOR ||
				m_rgBindings[i].wType & DBTYPE_ARRAY ||
				//Strip off a possible BYREF to check if its a BSTR
				(~(DBTYPE_BYREF) & m_rgBindings[i].wType)  == DBTYPE_BSTR)
			{
				//Make sure status is OK
				fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_OK);
			}
			else	//For any non pointer types, we expect an error
			{	
				//Make sure status is BADBINDINFO
				fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_BADBINDINFO);
			}
		}
	
	}
	
	////////////////////////////////////////////
	//Now do the same thing for command accessor
	////////////////////////////////////////////
	if (g_fCmdSupported)
		if (CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))	
		{		
			for (i=0; i<m_cBindings; i++)	
			{
				//For pointer types, this is valid memory ownership
				if (m_rgBindings[i].wType & DBTYPE_BYREF ||
					m_rgBindings[i].wType & DBTYPE_VECTOR ||
					m_rgBindings[i].wType & DBTYPE_ARRAY ||
					//Strip off a possible BYREF to check if its a BSTR
					(~(DBTYPE_BYREF) & m_rgBindings[i].wType)  == DBTYPE_BSTR)
				{
					//Make sure status is OK
					fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_OK);
				}
				else	//For any non pointer types, we expect an error
				{	
					//Make sure status is BADBINDINFO
					fResults &= COMPARE(m_rgStatus[i], DBBINDSTATUS_BADBINDINFO);
				}
			}
	
	}
	
	
	//Change back to CLIENT owned
	for (i=0; i<m_cBindings; i++)
	{
		m_rgBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	}
	
CLEANUP:

	return (fResults) ? TEST_PASS : TEST_FAIL;
}


// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBTYPE_IUnknown and NULL pObject
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCrtRtnValsAfterGetRows::Variation_33()
{ 
			
	TESTRESULT fResults = TEST_FAIL;
	HACCESSOR hCmdAccessor = (HACCESSOR)ULONG_MAX;;
	DBTYPE	dbType = DBTYPE_EMPTY;
	ULONG iCol;
	CCol TempCol;

	// If the provider doesn't support structured storage objects, then we'll skip this test
	if (!m_fOLEOBJECTS)
	{
		odtLog << L"Provider doesn't support binding to structured storage objects.\n";
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}

	//Initialize accessor so we can verify it is set to NULL on error
	m_hAccessor = (HACCESSOR)ULONG_MAX;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	// See if there's a column we can use with IUnknown
	for (iCol=0; iCol < m_cBindings; iCol++)
	{
		// Can't get colinfo for bookmark
		if (!m_rgBindings[iCol].iOrdinal)
			continue;

		// Retrieve colinfo for this column
		CHECK(m_pTable->GetColInfo(m_rgBindings[iCol].iOrdinal, TempCol), S_OK);

		// If this is a long column it should be usable with IUnknown binding
		if (TempCol.GetIsLong())
			break;
	}

	if (iCol == m_cBindings)
	{
		odtLog << L"Couldn't find a LONG column to try with DBTYPE_IUKNOWN.\n";
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Make sure the provider supports binding to IUnknown

	//Remember valid value for long column's wType
	dbType = m_rgBindings[iCol].wType;
	
	//Set long column's binding wType to DBTYPE_IUNKNOWN
	m_rgBindings[iCol].wType = DBTYPE_IUNKNOWN;

	//Expect wType of DBTYPE_IUNKNOWN to cause an error
	// Note: pObject is already NULL at this point
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
		&m_rgBindings[iCol], m_cbRowSize, &m_hAccessor, m_rgStatus);

	//Verify return code -- may be deferred
	TESTC(VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
		0, &m_rgBindings[iCol], MAY_DEFERR, MAY_FAIL));

	// Release the accessor now that we're done with it
	if (m_hAccessor != ULONG_MAX && m_hAccessor != DB_NULL_HACCESSOR)
		SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	//Initialize accessor so we can verify it is set to NULL on error
	m_hAccessor = (HACCESSOR)ULONG_MAX;

	// Try command accessor
	if (m_pCmdIAccessor)
	{
		m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
			&m_rgBindings[iCol], m_cbRowSize, &hCmdAccessor, m_rgStatus);

		// Re-open rowset object to get command accessors inherited on the rowset.
		// Note we CAUSED the error after getnextrows, but re-opening the rowset doesn't
		// need to call GetNextRows again.
		TESTC_(OpenRowsetObject(FALSE), S_OK);

		// Reset binding back to IUnknown for comparison, since OpenRowset changes it back
		m_rgBindings[iCol].wType = DBTYPE_IUNKNOWN;

		// Validate
		TESTC(VerifyError(m_pIAccessor, hCmdAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			0, &m_rgBindings[iCol], MAY_DEFERR, MAY_FAIL));
	}

	fResults = TEST_PASS;
	
CLEANUP:

	// Release the accessor now that we're done with it
	if (m_hAccessor == ULONG_MAX)
		m_hAccessor = DB_NULL_HACCESSOR;
	else 
		SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);	

	if (hCmdAccessor == ULONG_MAX)
		hCmdAccessor = DB_NULL_HACCESSOR;
	else
		SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, hCmdAccessor);	

	//Reset wType to previous value
	if (dbType != DBTYPE_EMPTY && m_rgBindings)
		m_rgBindings[iCol].wType = dbType;

	return fResults;
	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO: DBBINDFLAG_HTML for non-string type
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCrtRtnValsAfterGetRows::Variation_34()
{ 
	BOOL fSuccess = TEST_FAIL;
	BOOL fNonStringType = FALSE;
	ULONG iBind;
	DBBINDSTATUS * pBindStatus = NULL;
	enum FAILURE_MODE eFailMode = MUST_FAIL;

	// Release any previous bindings or accessors
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	
	//Create Accessor with a binding using length, status and value
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, g_rgParamOrds, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols,
		 &pBindStatus),S_OK);

	// Release the accessor we got above so we can change the binding information
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 

	// Set dwFlags to DBBINDFLAG_HTML for all non-string types.
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_STR) &&
			!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_WSTR))
		{
			m_rgBindings[iBind].dwFlags = DBBINDFLAG_HTML;
			fNonStringType = TRUE;
		}
	}

	if (!fNonStringType)
	{
		odtLog << L"No non-string types available to test.\n";
		fSuccess = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Now recreate the accessor.  Note specifying DBBINDFLAGS_HTML is legal even if provider 
	// doesn't support it.  It should be ignored.
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, m_rgStatus);
	 
	// Due to recent spec change need to force this as a warning if it succeeded
	if (S_OK == m_hr)
	{
		CHECKW(m_hr, DB_E_ERRORSOCCURRED);
		// Set failure mode to MAY_FAIL, which will then allow an S_OK on CreateAccessor
		// to proceed and validate the data.  For IMMEDIATE defer mode a failure will be
		// posted if GetNextRows or GetData fail.
		eFailMode = MAY_FAIL; 
	}

	// The hr should be DB_E_ERRORSOCCURRED and all non-string types should have DBBINDSTATUS_BADBINDINFO
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_STR) &&
			!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_WSTR))
		{
			TESTC(VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
				iBind, m_rgBindings, IMMEDIATE, eFailMode));
		}
	}

	fSuccess = TEST_PASS;

CLEANUP:

	//Clean up 
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	SAFE_FREE(pBindStatus)

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO: dwFlags invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCrtRtnValsAfterGetRows::Variation_35()
{ 
	BOOL fSuccess = TEST_FAIL;
	ULONG iBind;
	DBBINDSTATUS * pBindStatus = NULL;
	enum FAILURE_MODE eFailMode = MUST_FAIL;
		
	// Release any previous bindings or accessors
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	//Create Accessor with a binding using length, status and value
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, g_rgParamOrds, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols,
		 &pBindStatus),S_OK);

	// Release the accessor we got above so we can change the binding information
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 

	// Set dwFlags to an invalid value.  This is any currently non-defined value
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		// Set all bits on
		m_rgBindings[iBind].dwFlags = ULONG_MAX;

		// DBBINDFLAGS_HTML is invalid for non-string types, so remove it to 
		// verify the provider checks the other flags.
		if (!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_STR) &&
			!IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_WSTR))
			m_rgBindings[iBind].dwFlags &= ~DBBINDFLAG_HTML;
	}

	// Now recreate the accessor.  Note specifying DBBINDFLAG_HTML is legal even if provider 
	// doesn't support it.  It should be ignored.
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, m_rgStatus);

	// Due to recent spec change need to force this as a warning if it succeeded
	if (S_OK == m_hr)
	{
		CHECKW(m_hr, DB_E_ERRORSOCCURRED);
		// Set failure mode to MAY_FAIL, which will then allow an S_OK on CreateAccessor
		// to proceed and validate the data.  For IMMEDIATE defer mode a failure will be
		// posted if GetNextRows or GetData fail.
		eFailMode = MAY_FAIL; 
	}

	// The hr should be DB_E_ERRORSOCCURRED and all types should have DBBINDSTATUS_BADBINDINFO
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (!VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			iBind, m_rgBindings, IMMEDIATE, eFailMode))
			goto CLEANUP;
	}

	fSuccess = TEST_PASS;

CLEANUP:

	//Clean up 
	// Release any previous bindings or accessors
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	SAFE_FREE(pBindStatus)

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NULLACCESSORNOTSUPPORTED - Null accessor on rowset with IRowsetChange FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCrtRtnValsAfterGetRows::Variation_36()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	BOOL	fResults = FALSE;
	IRowsetChange * pIRowsetChange = NULL;
	HRESULT hr = E_FAIL;

	// Make sure we don't have a rowset open
	CleanUpRowsetObject();

	TESTC_(SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
		DBTYPE_BOOL, (ULONG_PTR)VARIANT_FALSE), S_OK);

	if (FAILED(hr = CreateRowsetObject(SELECT_VALIDATIONORDER)))
	{
		if (m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSETTABLE ||
			m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_CONFLICTING)
		{
			CHECK(hr, DB_E_ERRORSOCCURRED);
			odtLog << L"Unable to turn off IRowsetChange property.\n";
			fResults = TEST_SKIPPED;
			goto CLEANUP;
		}
	}

	TESTC_(hr, S_OK);

	// Make sure we did not get IRowsetChange property
	TESTC(!VerifyInterface(m_pIAccessor,IID_IRowsetChange, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetChange));

	// Should return error
	fResults = CHECKW(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 0,
		m_rgBindings, 1, &hAccessor, m_rgStatus), DB_E_NULLACCESSORNOTSUPPORTED);

CLEANUP:

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hAccessor);
	SAFE_RELEASE(pIRowsetChange);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	CHECK(SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
		DBTYPE_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_OPTIONAL), S_OK);

	return (fResults);

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO: dwMemOwner invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCrtRtnValsAfterGetRows::Variation_37()
{ 
	BOOL fSuccess = TEST_FAIL;
	ULONG iBind;
	DBBINDSTATUS * pBindStatus = NULL;
	enum FAILURE_MODE eFailMode = MUST_FAIL;
		
	// Release any previous object to prevent mem leak.	
	CleanUpRowsetObject();

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	//Create Accessor with a binding using length, status and value
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, g_rgParamOrds, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols,
		 &pBindStatus),S_OK);

	// Release the accessor we got above so we can change the binding information
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 

	// Set dwFlags to an invalid value.  This is any currently non-defined value
	for(iBind = 0; iBind < m_cBindings; iBind++)
		// Set all bits on except PROVIDEROWNED (CLIENTOWNED is 0!)
		m_rgBindings[iBind].dwMemOwner = ULONG_MAX & ~DBMEMOWNER_PROVIDEROWNED;

	// Now recreate the accessor with the invalid dwMemOwner
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, m_rgStatus);

	// Due to recent spec change need to force this as a warning if it succeeded
	if (S_OK == m_hr)
	{
		CHECKW(m_hr, DB_E_ERRORSOCCURRED);
		// Set failure mode to MAY_FAIL, which will then allow an S_OK on CreateAccessor
		// to proceed and validate the data.  For IMMEDIATE defer mode a failure will be
		// posted if GetNextRows or GetData fail.
		eFailMode = MAY_FAIL; 
	}

	// The hr should be DB_E_ERRORSOCCURRED and all types should have DBBINDSTATUS_BADBINDINFO
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (!VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			iBind, m_rgBindings, IMMEDIATE, eFailMode))
			goto CLEANUP;
	}

	fSuccess = TEST_PASS;

CLEANUP:

	//Clean up 
	// Release any previous bindings or accessors
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	SAFE_FREE(pBindStatus)

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCrtRtnValsAfterGetRows::Terminate()
{
	CleanUpRowsetObject();

	return(CAccessor::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCreateValidRowAccessors)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateValidRowAccessors - Creation of valid row accessors
//|	Created:			10/20/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateValidRowAccessors::Init()
{
	DBCOLUMNINFO * rgInfo = NULL;
	WCHAR * pStrBuffer = NULL;
	ULONG cColumns = 0;
	IColumnsInfo * pIColInfo = NULL;
	const ULONG		cProps = 2;
	HRESULT			hr;
	BOOL			fPropsSettable=FALSE;

	m_pDBPropSet = (DBPROPSET *)PROVIDER_ALLOC(sizeof(DBPROPSET));
	m_prgDBProps = (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * cProps);

	if (!m_pDBPropSet || !m_prgDBProps)
	{
		odtLog << wszMemoryAllocationError;
		return FALSE;
	}

	m_prgDBProps[0].dwPropertyID = DBPROP_IRowsetChange;
	m_prgDBProps[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_prgDBProps[0].colid = DB_NULLID;
	m_prgDBProps[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_prgDBProps[0].vValue)) = VARIANT_TRUE;	
	
	m_prgDBProps[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_prgDBProps[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_prgDBProps[1].colid = DB_NULLID;
	m_prgDBProps[1].vValue.vt = VT_I4;		
	m_prgDBProps[1].vValue.lVal = DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;	

	m_pDBPropSet->guidPropertySet = DBPROPSET_ROWSET;
	m_pDBPropSet->cProperties = cProps;
	m_pDBPropSet->rgProperties = m_prgDBProps;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		fPropsSettable=SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE) &
			SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE);

		if (fPropsSettable)
			SetRowsetProperties(m_pDBPropSet, 1);

		//Set m_pIAccessor on a rowset created with 'select *' rowset.  We'll
		//use this interface ptr to do our tests. 
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		if (SUCCEEDED(hr))
		{
			if(SUCCEEDED(VerifyRowsetProperties((IUnknown *)m_pIAccessor, m_pDBPropSet, 1)))
				m_fPropertiesSet = TRUE;
			else
				m_fPropertiesSet = FALSE;
		}

		if (CheckHr(hr))
		{
			//Try to get an IRowset interface 
			if(VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
			{
				//Record that IRowset is supported
				m_fIRowset = TRUE;
				//We got IRowset, that's all we need
				return TRUE;
			}
									
		}
		//since we want to test the row accessor, it is okay if we can not set these properties.
		//the only drawback is that we have to skip var1.
		else if(hr==DB_E_ERRORSOCCURRED)
		{
			// Release the properties which will affect opening rowset later
			if (m_rgPropSets)
			{
				// Walk Set array, freeing all member property arrays
				for(ULONG i=0; i<m_cPropSets; i++)
				{
					PROVIDER_FREE(m_rgPropSets[i].rgProperties);
				}
			
				PROVIDER_FREE(m_rgPropSets);
				m_cPropSets = 0;
			}
			
			hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

			if(hr==S_OK)
				return TRUE;
		}
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Null Accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_1()
{	
	BOOL	fSuccess = TRUE;
	HRESULT hr=S_OK;
	HRESULT hrExp = NOERROR;
	HROW rghRows[1]={DB_NULL_HROW};
	DBROWSTATUS rgRowStatus[1];
	 
	if(!m_fPropertiesSet)
		hr= E_NOINTERFACE;

	//If IRowsetChange is not supported, NULL accessor is not supported.
	if(!VerifyInterface(m_pIAccessor,IID_IRowsetChange, ROWSET_INTERFACE,
		(IUnknown **)&m_pIRowsetChange))
	{
		odtLog <<L"Provider does not support IRowsetChange.\n";
		hrExp = DB_E_NULLACCESSORNOTSUPPORTED;

		if (S_OK == m_pIAccessor->QueryInterface(IID_IRowsetChange, (void **)&m_pIRowsetChange))
		{
			// We're not supposed to use this interface, but it's actually supported. Therefore creating
			// a NULL accessor should actually succeed.  Since this is not a proper condition for testing
			// skip this variation.
			SAFE_RELEASE(m_pIRowsetChange);
			return TEST_SKIPPED;
		}
	}


	//Pass cBindings = 0 to create a NULL accessor, m_rgBindings is null
	//and should be ignored
	fSuccess &= CHECK(hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 0, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, m_rgStatus),hrExp);

	if(hr==S_OK && m_pIRowsetChange)
	{	
		//Try to use NULL Accessor, this may fail if nulls and defaults
		//are not available for a column, which is perfectly valid, so don't
		//check return code.
		
		// Note: This can also fail if a NULL in the key column results in a duplicate primary key.
		hr=m_pIRowsetChange->InsertRow(NULL,m_hAccessor,NULL, &rghRows[0]);

		//GetBindings should return same values we've used to create the accessor
		fSuccess &= VerifyBindings(m_pIAccessor, m_rgBindings, 0, m_hAccessor, DBACCESSOR_ROWDATA);

		// Accessor creation was successful and we're done with it.
		fSuccess &= CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
		
		if (SUCCEEDED(hr))
		{
			// Let the table object know how many rows it's got
			m_pTable->AddRow();

			// TODO: Verify the proper row was added

			// Remove row we've just added

			// We can fail to delete the row when using default values because the provider
			// may not always know which row is referenced by the row handle.  The provider
			// may not actually know the default values, especially with Query Based Udates
			hr = m_pIRowsetChange->DeleteRows(NULL, 1, rghRows, rgRowStatus);

			if (SUCCEEDED(hr))
			{
				m_pTable->SubtractRow();
				fSuccess &= CHECK(hr, S_OK);
				fSuccess &= COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
			}
			else
			{
				// If we didn't delete the row using the row handle above we might have stumbled
				// over the QBU limitation, so don't fail for that, but check for proper error
				// and status.
				fSuccess &= CHECK(hr, DB_E_ERRORSOCCURRED);
				fSuccess &= COMPARE(rgRowStatus[0], DBROWSTATUS_E_NEWLYINSERTED);

				//  Remove row by deleting all rows and re-adding one

				// We can't depend on commands being available here, so delete all the
				// rows using IRowsetChange.  We need the new row in our rowset so we 
				// have to close and re-open the rowset object.
				fSuccess &= CHECK(m_pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL), S_OK);
				rghRows[0] = DB_NULL_HROW;
				SAFE_RELEASE(m_pIRowsetChange);
				SAFE_RELEASE(m_pIRowset);
				ReleaseRowsetObject();

				SetRowsetProperties(m_pDBPropSet, 1);

				if (!(fSuccess &= CHECK(CreateRowsetObject(USE_OPENROWSET), S_OK)))
					goto CLEANUP;

				if(!(fSuccess &= VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
					(IUnknown **)&m_pIRowset)))
					goto CLEANUP;
				if(!(fSuccess &= VerifyInterface(m_pIAccessor,IID_IRowsetChange, ROWSET_INTERFACE,
					(IUnknown **)&m_pIRowsetChange)))
					goto CLEANUP;

				do
				{
					// Get a row from the rowset
					DBCOUNTITEM cRowsObtained=0;
					HROW * prghRows=&rghRows[0];
					hr = m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows);
					if (S_OK == hr)
					{
						if (CHECK(hr = m_pIRowsetChange->DeleteRows(NULL, 1, rghRows, rgRowStatus), S_OK))
						{
							m_pTable->SubtractRow();
						}
						CHECK(hr = m_pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL), S_OK);
						rghRows[0] = DB_NULL_HROW;
					}
				} while (S_OK == hr);

				fSuccess &= CHECK(hr, DB_S_ENDOFROWSET);

				//Now add the single first row back in for the rest of the variations
				if (!(fSuccess &= CHECK(m_pTable->Insert(g_uiRowNum),S_OK)))
					odtLog << wszErrorInserting;
			}
		}

CLEANUP:
		//Clean up
		if (rghRows[0] != DB_NULL_HROW)
			CHECK(m_pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL), S_OK);

		SAFE_RELEASE(m_pIRowsetChange);
		//Release rowset so that for next variations we don't 
		//have an un updateable newly inserted hRow in the rowset
		ReleaseRowsetObject();
		SAFE_RELEASE(m_pIRowset);

		//Create a new rowset for the next variations.
		if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE) &
				SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE))
			SetRowsetProperties(m_pDBPropSet, 1);

		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		fSuccess &= CheckHr(hr);

		if (SUCCEEDED(hr))
		{
				//Try to get an IRowset interface 
				fSuccess &= COMPARE(VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
					(IUnknown **)&m_pIRowset), TRUE);
			}
	}
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc dwPart = DBPART_VALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_2()
{
	BOOL fSuccess = FALSE;

	//Create Accessor with a binding using only DBCOLUMPART_VALUE
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_LENGTH, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))

		// Now go through and remove the length binding for all but varnumeric bindings.  Length
		// is required for varnumeric bindings.
		for (ULONG iBind = 0; iBind < m_cBindings; iBind++)
		{
			m_rgBindings[iBind].dwPart = DBPART_VALUE;
		}

		// Recreate the accessor since we've changed the bindings
		TESTC_(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, m_rgBindings, m_cbRowSize,
			&m_hAccessor, NULL), S_OK);
				
		//Verify the data is correctly brought back using the accessor 
		//(via either GetData or ReadData).  	
		if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK)) 				
		//GetBindings should return same values we've used to create the accessor
 			fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA); 

CLEANUP:

	//Clean up 
	if (m_hAccessor)
	{	
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc dwPart = DBPART_LENGTH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_3()
{
	BOOL fSuccess = TRUE;
	

	//Create Accessor with a binding using only DBCOLUMPART_LENGTH
	if (fSuccess &= CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
				
		if (fSuccess &= CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK))				
			//GetBindings should return same values we've used to create the accessor
			fSuccess &= VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA);				

	//Clean up 
	if (m_hAccessor)
	{
		fSuccess &= CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwPart = DBPART_STATUS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_4()
{
	BOOL fSuccess = FALSE;
	
	
	//Create Accessor with a binding using only DBCOLUMPART_STATUS
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_STATUS, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
				
		//Verify the data is correctly brought back using the accessor 
		//(via either GetData or ReadData).  
		if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK))
			//GetBindings should return same values we've used to create the accessor
			fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA);							

	//Clean up 
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
				    

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_5()
{
	BOOL fSuccess = FALSE;
	
	
	//Create Accessor with a binding using only length and status
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
				
		//Verify the data is correctly brought back using the accessor 
		//(via either GetData or ReadData).  
		if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK))
			//GetBindings should return same values we've used to create the accessor
			fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA);

	//Clean up 
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc dwPart = DBCOLUMPART_LENGTH | DBCOLUMPART_STATUS | DBCOLUMPART_VALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_6()
{
	
	BOOL fSuccess = FALSE;
		
	//Create Accessor with a binding using length, status and value
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, g_rgParamOrds, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols),S_OK))
				
	//Verify the data is correctly brought back using the accessor 
	//(via either GetData or ReadData).  
	if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK))
		//GetBindings should return same values we've used to create the accessor
		fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA);						

	//Clean up 
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc One optmized, one non optimized, using same fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_7()
{
	BOOL fSuccess = FALSE;
	HRESULT	 hr;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	
	SAFE_RELEASE(m_pIRowset);

	if (m_fBindLongCols == BLOB_LONG)
		SetRowsetProperties(m_pDBPropSetLocate, 1);

	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);
	
	if (!CheckHr(hr))
		return TEST_FAIL;
	
	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;	

	//Create an optmized accessor, using all the columns 
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK)
		
		&&
		
		//Create a non optimized accessor using the same columns
		CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
	
				
		//Verify the data is correctly brought back using both accessors 
		//(via either GetData or ReadData).  
		if(g_fCmdSupported)
		{
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings,
				FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call
			{				
				if (CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings2, m_cBindings2),S_OK))
				
					//Check first accessor with GetBindings  
					fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
						DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED);

					//Check second accessor with GetBindings
					fSuccess &= VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_ROWDATA);
			}
			else 
			fSuccess = TRUE;
		//We have to clean up since we kept the row in UseRowAccessorAndVerify
			if(m_hRow && m_fIRowset)
			{
				m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL, NULL,NULL);		
				m_hRow = DB_NULL_HROW;
			}
		}

		else 
			fSuccess = TRUE;
					
	//Clean Up
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc One non-optimized, one optimized, using same fields - different creation sequence
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_8()
{
	BOOL fSuccess = FALSE;
	HRESULT hr;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	
	SAFE_RELEASE(m_pIRowset);
	
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);	

	if (!CheckHr(hr))
		return TEST_FAIL;	

	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;	

	//Create a non optmized accessor, using all the columns 
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK)
		
		&&
		
		//Create an optimized accessor using the same columns
		CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
	
				
		//Verify the data is correctly brought back using both accessors 
		//(via either GetData or ReadData).  NOTE: we use cBindings for
		//number of columns in rowset, since these are the same value here
		if(g_fCmdSupported)
		{
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings,
				FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call							
			{
				if (CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
				m_rgTableColOrds, m_cRowsetCols, m_rgBindings2, m_cBindings2),S_OK))
					//Check first accessor with GetBindings  
					fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA) &&
					//Check second accessor with GetBindings
					VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
					DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED);
			}
			//We have to clean up since we kept the row in UseRowAccessorAndVerify
			if(m_hRow && m_fIRowset)
			{
				m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL, NULL,NULL);		
				m_hRow = DB_NULL_HROW;
			}
		}
		else 
			fSuccess = TRUE;
		
	//Clean Up
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc One optimized, one non-optimized, using different fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_9()
{
	BOOL fSuccess = FALSE;
	DBORDINAL cCols;
	DBORDINAL cCols2;
	HRESULT hr;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pIRowset);

	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (!CheckHr(hr))
		return TEST_FAIL;
	
	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;	


	//Create an optmized accessor, using odd columns
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ODD_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, &cCols,						
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK)
		
		&&
		
		//Create a non optimized accessor using different columns (even ones)
		CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		EVEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, &cCols2,		
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
	{
		//This should be the same number since its the same rowset
		COMPARE(cCols,cCols2);
				
		//Verify the data is correctly brought back using both accessors 
		//(via either GetData or ReadData). 
		if(g_fCmdSupported)
		{
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, cCols, m_rgBindings, m_cBindings,
					FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call
			{
				if (CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
					m_rgTableColOrds, cCols2, m_rgBindings2, m_cBindings2),S_OK))
						//Check first accessor with GetBindings  
						fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
							DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED) &&
						//Check second accessor with GetBindings
						VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_ROWDATA);			
			}
				//We have to clean up since we kept the row in UseRowAccessorAndVerify
				if(m_hRow && m_fIRowset)
				{
					m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow,NULL, NULL,NULL);		
					m_hRow = DB_NULL_HROW;
				}
		}
		else 
			fSuccess = TRUE;
		
	}			

	//Clean Up
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Two optimized, using different fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_10()
{
	BOOL fSuccess = FALSE;
	DBORDINAL cCols;
	DBORDINAL cCols2;
	HRESULT hr;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pIRowset);

	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (!CheckHr(hr))
		return TEST_FAIL;	

	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;		
	
	//Create an optmized accessor, using odd columns
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ODD_COLS_BOUND,  FORWARD, NO_COLS_BY_REF, NULL, &cCols,		
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK)
		
		&&
		
		//Create another optimized accessor using different columns (even ones)
		CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		EVEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, &cCols2,		
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))		
	{
		//This should be the same number since its the same rowset
		COMPARE(cCols,cCols2);
		
		//Verify the data is correctly brought back using both accessors 
		//(via either GetData or ReadData). 
		if(g_fCmdSupported)
		{
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, cCols, m_rgBindings, m_cBindings,
					FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call
			{
				if (CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
					m_rgTableColOrds, cCols2, m_rgBindings2, m_cBindings2),S_OK))
						//Check first accessor with GetBindings  
						fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
							DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED) &&
						//Check second accessor with GetBindings
						VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
							DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED);
			}
				//We have to clean up since we kept the row in UseRowAccessorAndVerify
				if(m_hRow && m_fIRowset)
				{
					m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL,  NULL,NULL);		
					m_hRow = DB_NULL_HROW;
				}
		}
		else 
			fSuccess = TRUE;
		
	}										
	

	//Clean Up
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Two non-optimized, using same fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_11()
{
	BOOL fSuccess = FALSE;
	HRESULT hr;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pIRowset);

	//Create a new rowset for the next variations.
	// We have to reset any properties we wanted on the rowset for IOpenRowset.
	if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE) &
			SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE))
		SetRowsetProperties(m_pDBPropSet, 1);

	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (!CheckHr(hr))
		return TEST_FAIL;
	
	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;	

	
	//Create an optmized accessor, using all columns
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND,											
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK)
		
		&&
		
		//Create another optimized accessor with same columns 
		CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,		
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))

		//Check that GetBindings works here 
		VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
			DBACCESSOR_ROWDATA);
				
		//Verify the data is correctly brought back using both accessors 
		//(via either GetData or ReadData).  NOTE: we use cBindings for
		//number of columns in rowset, since these are the same value here
		if(g_fCmdSupported)
		{
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings,
					FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call							
			{
					//Specify that we don't want to GetNextRows again, so
					//we can read the first row a second time, hence true 
					//as the last param
					if (CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings2, m_cBindings2),S_OK))
				
						//Check first accessor with GetBindings  
						fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
							DBACCESSOR_ROWDATA);			
			}
				//We have to clean up since we kept the row in UseRowAccessorAndVerify
				if(m_hRow && m_fIRowset)
				{
					m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow,NULL, NULL,NULL);		
					m_hRow = DB_NULL_HROW;
				}
		}

		else 
			fSuccess = TRUE;
		
	//Clean Up
	if (m_hAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}

// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multiple bindings for same column in one accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_12()
{
	BOOL fSuccess = FALSE;
	DBBINDING	rgMultiBind[2];
	DBBINDSTATUS * rgStatus = NULL;
	IConvertType * pIConvertType = NULL;
	IColumnsInfo * pIColumnsInfo = NULL;
	HRESULT		hrExp = NOERROR;
	HRESULT		hr = NOERROR;
	DBORDINAL cColumns;
	DBCOLUMNINFO * pColInfo=NULL;
	WCHAR * pStringsBuffer=NULL;

	ULONG	ulBindIdx=0;
	DBLENGTH ulOffset=0;
	DBLENGTH cbRowSize=0;
	DBORDINAL iColIdx=0;

	//Use this routine to build bindings
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,		
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	//Now release accessor since we'll make a different one
	CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);
	m_hAccessor = DB_NULL_HACCESSOR;
	
	if (!VerifyInterface(m_pIRowset, IID_IConvertType, ROWSET_INTERFACE,(IUnknown **) &pIConvertType))
		goto CLEANUP;

	if (!VerifyInterface(m_pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE,(IUnknown **) &pIColumnsInfo))
		goto CLEANUP;

	// Get column information
	if (!CHECK(pIColumnsInfo->GetColumnInfo(&cColumns, &pColInfo, &pStringsBuffer), S_OK))
		goto CLEANUP;

	//Any failure from here on will set this to false,	
	fSuccess = TRUE;

	//For each column, create an accessor with two bindings for same col
	//and make sure we can GetData back correctly using it.
	//NOTE: This assumes that DBTYPE_WSTR will be a mandatory coersion.
	for (ulBindIdx=0; ulBindIdx<m_cBindings; ulBindIdx++)
	{
		//Use the default binding type for one binding
		CopyBindings(&rgMultiBind[0], &m_rgBindings[ulBindIdx]);
		//Adjust offsets to use beginning of data buffer
		rgMultiBind[0].obStatus = offsetof(DATA, sStatus);
		rgMultiBind[0].obLength = offsetof(DATA, ulLength);
		rgMultiBind[0].obValue = offsetof(DATA, bValue);
		
		//Find start of next binding in buffer
		ulOffset = offsetof(DATA, bValue) + rgMultiBind[0].cbMaxLen;
		ulOffset = ROUND_UP(ulOffset, ROUND_UP_AMOUNT);

		//Bind same column, except make bind type wchar
		CopyBindings(&rgMultiBind[1], &m_rgBindings[ulBindIdx]);
		rgMultiBind[1].wType = DBTYPE_WSTR;
		
		//Move offsets for second binding to after first binding
		rgMultiBind[1].obStatus = ulOffset + offsetof(DATA, sStatus);
		rgMultiBind[1].obLength = ulOffset + offsetof(DATA, ulLength);
		rgMultiBind[1].obValue = ulOffset + offsetof(DATA, bValue);
		
		ulOffset = ulOffset + offsetof(DATA,bValue) + rgMultiBind[1].cbMaxLen;
		ulOffset = ROUND_UP(ulOffset, ROUND_UP_AMOUNT);
		
		rgStatus = (DBBINDSTATUS *)m_pIMalloc->Alloc(2 * sizeof(DBBINDSTATUS));
		if (!rgStatus)
			goto CLEANUP;

		// Adjust cbMaxLen and cbRowSize.  In general given a DBTYPE we don't know how much
		// space is needed for the WSTR representation.
		switch(rgMultiBind[0].wType)
		{
			case DBTYPE_WSTR:
				// The binding is already the right size, do nothing
				break;
			case DBTYPE_STR:
				// The binding is half that needed
				rgMultiBind[1].cbMaxLen = rgMultiBind[0].cbMaxLen * 2 + sizeof(WCHAR);
				break;
			case DBTYPE_BYTES:
				// The binding is one fourth that needed
				// One binary digit becomes two wchars, each wchar is two bytes
				rgMultiBind[1].cbMaxLen = rgMultiBind[0].cbMaxLen * 4 + sizeof(WCHAR);
				break;
			default:
				// We'll set it to a minimum of MAX_BIND_LIMIT
				rgMultiBind[1].cbMaxLen = MAX_BIND_LIMIT;
		}

		// Now adjust cbRowSize
		cbRowSize = rgMultiBind[1].obValue - rgMultiBind[0].obStatus + rgMultiBind[1].cbMaxLen;

		// Find the index into the columnsinfo for this column
		iColIdx = rgMultiBind[1].iOrdinal;
		if (pColInfo[0].iOrdinal)
			iColIdx--;	// If we don't have the bookmark column our index is one less

		//Check whether provider supports the coersion
		if(FAILED(hr = pIConvertType->CanConvert(pColInfo[iColIdx].wType, rgMultiBind[1].wType, DBCONVERTFLAGS_COLUMN)))
			goto CLEANUP;
		
		// Conversion to WSTR is mandatory for all providers
		CHECK(hr, S_OK);

		//Now create accessor with these two bindings
		CHECK(hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
			2, rgMultiBind, cbRowSize, &m_hAccessor, rgStatus),hrExp);

		if(hr == DB_E_ERRORSOCCURRED)
		{
			CHECK(rgStatus[0],DBBINDSTATUS_OK);
			CHECK(rgStatus[1],DBBINDSTATUS_UNSUPPORTEDCONVERSION);
			COMPARE(m_hAccessor, 0);
		}
		else
		{
			//Try to use the accessor
			if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, rgMultiBind, 2),S_OK))							
			{		
				fSuccess = FALSE;
				goto CLEANUP;
			}
			
			//If either of these fail, increment the error count,
			//but don't exit variation, since this isn't what
			//we're really testing here
			COMPARE(VerifyBindings(m_pIAccessor, rgMultiBind, 2, m_hAccessor, 
							DBACCESSOR_ROWDATA), TRUE);		

			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);
			m_hAccessor = DB_NULL_HACCESSOR;
		}
		PROVIDER_FREE(rgStatus);
	}		
				
CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	SAFE_RELEASE(pIConvertType);
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(pColInfo);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rgStatus);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}

// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc All types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_13()
{
	BOOL fSuccess = FALSE;

	//Get accessor with all cols BY_REF
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, SUPPORTED_COLS_BY_REF,
		NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);
			
	//Verify the data is correctly brought back using accessors 	
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA));

	fSuccess = TRUE;
				
CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Fixed types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_14()
{
	BOOL fSuccess = FALSE;
	DBORDINAL cCols;

	//Get accessor with fixed data type cols bound BY_REF
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
 		ALL_COLS_BOUND, FORWARD, SUPPORTED_FIXED_LEN_COLS_BY_REF, NULL, &cCols,						
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);

	//Verify the data is correctly brought back using accessors 
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, cCols, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA));


	fSuccess = TRUE;


CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Variable types bound BY_REF, without DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_15()
{
	BOOL fSuccess = FALSE;
	DBORDINAL cCols;

	//Get accessor with variable data type cols bound BY_REF
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF, NULL, &cCols,		
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_fBindLongCols),S_OK);

	//Verify the data is correctly brought back using accessors 
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, cCols, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA));

	fSuccess = TRUE;


CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc All types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_16()
{
	BOOL fSuccess = FALSE;
	HRESULT hr = E_FAIL;

	//Get accessor with all cols BY_REF and owned by provider
	hr = GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, SUPPORTED_COLS_BY_REF, NULL,
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		SUPPORTED_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_fBindLongCols);

	// It is now legal for providers not to support provider-owned memory.  At a later time a 
	// property will be added to indicate support but at this time it's just DB_E_ERRORSOCCURRED.
	if (hr == DB_E_ERRORSOCCURRED)
	{
		FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
		odtLog << L"Couldn't create an accessor with provider-owned memory.\n";
		return TEST_SKIPPED;
	}

	TESTC_(hr, S_OK);
			
	//Verify the data is correctly brought back using accessors 
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA));

	fSuccess = TRUE;
	
CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Fixed types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_17()
{
	//This is no longer a valid variation, CreateAccessor should fail
	//for this scenario, and another variation covers this error condition
	odtLog << L"No longer a valid variation.  Left as a placeholder.\n";
	return TEST_SKIPPED;

}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Variable types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_18()
{
	BOOL fSuccess = FALSE;
	DBCOLUMNINFO * rgColInfo = NULL;
	DBORDINAL		cCols = 0;
	ULONG			i,j;
	DBBINDSTATUS * rgStatus = NULL;
	ULONG			cFoundLongCols = 0;
	OLECHAR *		pStringsBuffer = NULL;
	IColumnsInfo *  pIColInfo = NULL;
	HRESULT			hr;
	
	//Get accessor with variable data type cols bound BY_REF
	//and all columns provider owned -- do not bind long columns.  	
	hr = GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		VARIABLE_LEN_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		SUPPORTED_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS);


	// Some providers may not support variable length columns, in which case m_cBindings = 0.
	// The accessor created will be a NULL accessor if such is supported, otherwise hr will be 
	// DB_E_NULLACCESSORNOTSUPPORTED.
	if(m_cBindings == 0 && hr==DB_E_NULLACCESSORNOTSUPPORTED)
		return TEST_SKIPPED;

	// It is now legal for providers not to support provider-owned memory.  At a later time a 
	// property will be added to indicate support but at this time it's just DB_E_ERRORSOCCURRED.
	if (hr == DB_E_ERRORSOCCURRED)
	{
		FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
		odtLog << L"Couldn't create an accessor with provider-owned memory.\n";
		return TEST_SKIPPED;
	}

	// Any other case should be successful
	TESTC_(hr, S_OK);

	//Verify the data is correctly brought back using accessors 
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA));

	//Since long columns are illegal for provider owned memory,
	//try it here 
	if (m_fBindLongCols == BLOB_LONG)
	{

		//clean up from last call
			
		if (m_hAccessor)
		{
			m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
			m_hAccessor = DB_NULL_HACCESSOR;
		}

		FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

		//Just get the bindings for long columns, don't specify MemOwner yet
		//since then this function will fail
		TESTC_(GetAccessorAndBindings(m_pIAccessor, 
			DBACCESSOR_ROWDATA,
			&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			VARIABLE_LEN_COLS_BOUND, FORWARD, VARIABLE_LEN_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_fBindLongCols), S_OK);

		//We only wanted the bindings
		if (m_hAccessor)
		{
			m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
			m_hAccessor = DB_NULL_HACCESSOR;
		}

		//Make all possible bindings provider owned
		AdjustMemOwner(DBMEMOWNER_PROVIDEROWNED, m_cBindings, m_rgBindings);

		//Get the col info
		TESTC(VerifyInterface(m_pIAccessor, IID_IColumnsInfo, ROWSET_INTERFACE,(IUnknown **) &pIColInfo));
	
		TESTC_(pIColInfo->GetColumnInfo(&cCols,	
			&rgColInfo,	&pStringsBuffer), S_OK);
		
		rgStatus = (DBBINDSTATUS *)m_pIMalloc->Alloc(m_cBindings * sizeof(DBBINDSTATUS));

		TESTC(rgStatus != NULL);

		//Now try create the accessor with long cols
		m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
			m_rgBindings, m_cbRowSize, &m_hAccessor, rgStatus);
		
		//Make sure that each binding has the correct status
		for (i=0; i<m_cBindings; i++)
		{
				//Loop thru the colinfo...
			for (j=0; j<cCols; j++)
			{
			
				 //...looking for the one which matches this binding
				if (m_rgBindings[i].iOrdinal == rgColInfo[j].iOrdinal)
				{
					//Long data is invalid
					if (rgColInfo[j].dwFlags & DBCOLUMNFLAGS_ISLONG)
					{
						if (S_OK != m_hr)
							COMPARE(rgStatus[i], DBBINDSTATUS_BADBINDINFO);
						else
							COMPARE(rgStatus[i], DBBINDSTATUS_OK);

						//Mark that we found at least one long col
						cFoundLongCols++;
					}
					else
						//Everything else should be fine
						COMPARE(rgStatus[i], DBBINDSTATUS_OK);

					
					//We found our colinfo for this binding, so stop looking
					break;
				}
			}		
		}
		
		//Now check our return code
		if (cFoundLongCols)
		{
			////////////////////////////////////////////////////////////////////////////////
			// ODBC Provider SPECIFIC NOTE:
			//
			//Note that this will only fail if the accessor is invalid for SetData as well;
			//Right now Provider Owned memory is allowed for SetData, thus another reason
			//for invalid SetData must exist for the accessor creation to actually fail.
			//On SQL Server, we bind some non updateable columns, so SetData fails, and
			//GetData fails for the Provider Owned memory case we our testing.  For Brazos,
			//we don't bind any non updateable columns, so even though the GetData
			//validation may fail, SetData is still valid, thus accessor creation succeeds.
			//For this reason, this line will fail on Brazos, but succeed on SQL Server.
			// TODO:  Figure out a way to guarantee the SetData part of validation always
			//fails w/o affecting GetData validation.

			// This can succeed if the provider thinks it can support this on LONG cols.
			if (S_OK != m_hr)
			{
				TESTC_(m_hr, DB_E_ERRORSOCCURRED);
			}
			else
			{
				// Try to use the accessor

				//Verify the data is correctly brought back using accessors 
				TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK);

				//Check first accessor with GetBindings  
				TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
					DBACCESSOR_ROWDATA));
			}
		}
		else
			TESTC_(m_hr, S_OK);

	}

	fSuccess = TRUE;

CLEANUP:

	SAFE_RELEASE(pIColInfo);

	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(rgStatus);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc No types bound BY_REF, with DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_19()
{
	//BSTRS are	the only type which can be Provider Owned without being BYREF
	//NOTE, ODBC Provider does not support BSTRS, so this
	//variation is not applicable to ODBC Provider.
	
	HRESULT hr = E_FAIL;
	BOOL fSuccess = FALSE;


	//Get accessor with no data type cols bound BY_REF --
	//and all columns provider owned.  The DBMEMOWNER_PROVIDEROWNED 
	//should be a no op.
	hr = GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		SUPPORTED_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_fBindLongCols);

	// It is now legal for providers not to support provider-owned memory.  At a later time a 
	// property will be added to indicate support but at this time it's just DB_E_ERRORSOCCURRED.
	if (hr == DB_E_ERRORSOCCURRED)
	{
		odtLog << L"Couldn't create an accessor with provider-owned memory.\n";
		return TEST_SKIPPED;
	}

	TESTC_(hr, S_OK);

	//Verify the data is correctly brought back using accessors 
	TESTC_(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK);

	//Check first accessor with GetBindings  
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA));

	fSuccess = TRUE;
					
CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_rgBindings)
	{
		m_pIMalloc->Free(m_rgBindings);
		m_rgBindings = NULL;
	}	

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
		

}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//--------------------------------------------------------------------
// @mfunc Use accessor after release rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidRowAccessors::Variation_20()
{
	BOOL fSuccess = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *pHRow = NULL;
	HRESULT hr;

	//Create Accessor
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_STATUS|DBPART_LENGTH|DBPART_VALUE, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
	{
		// Make sure we're at the beginning of the rowset
		// Some providers will need to re-execute the query to get back to the beginning
		// so just check for success code. GetNextRows will return ENDOFROWSET if it fails.
		if (!SUCCEEDED(hr = m_pIRowset->RestartPosition(NULL)))
			CHECK(hr, S_OK);

		if( CHECK(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow), S_OK))
		{
			if( CHECK(m_pIRowset->ReleaseRows(cRowsObtained, pHRow, NULL, NULL, NULL), S_OK))
			{					
				//Verify the data is correctly brought back using the accessor 
				//(via either GetData or ReadData).  
				if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK)) 				
					//GetBindings should return same values we've used to create the accessor
 					fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA);
			}
			PROVIDER_FREE(pHRow);
		}
	}
	//Clean up 
	if (m_hAccessor)
	{	
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc dwFlags: DBBINDFLAG_HTML
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateValidRowAccessors::Variation_21()
{ 
	
	BOOL fSuccess = TEST_FAIL;
	BOOL fStringType = FALSE;
	ULONG iBind;
	DBBINDSTATUS * pBindStatus = NULL;
		
	//Create Accessor with a binding using length, status and value
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, g_rgParamOrds, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, m_fBindLongCols,
		 &pBindStatus),S_OK);

	// Release the accessor we got above so we can change the binding information
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 

	// Set dwFlags to DBBINDFLAG_HTML for all string types.
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_STR) ||
			IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_WSTR))
		{
			m_rgBindings[iBind].dwFlags = DBBINDFLAG_HTML;
			fStringType = TRUE;
		}
	}

	if (!fStringType)
	{
		odtLog << L"No string types available to test.\n";
		fSuccess = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Now recreate the accessor.  Note specifying DBBINDFLAGS_HTML is legal even if provider 
	// doesn't support it.  It should be ignored.
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, pBindStatus), S_OK);
	 
	 //Verify the data is correctly brought back using the accessor 
	//(via either GetData or ReadData).  
	if(g_fCmdSupported)
	{
		CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK);
	}

	//GetBindings should return same values we've used to create the accessor
	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA));

	fSuccess = TEST_PASS;

CLEANUP:

	//Clean up 
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);	
	SAFE_FREE(pBindStatus);
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateValidRowAccessors::Variation_22()
{ 
	BOOL fSuccess = FALSE;
	DBORDINAL cCols;
	DBORDINAL cCols2;
	HRESULT hr;
	HACCESSOR hCmdAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hAccInherited = m_hAccessor;
	HACCESSOR hAccInherited2 = m_hAccessor2;
	DBBINDSTATUS  * pBindStatus;
	DBBINDSTATUS * pBindStatus2;
	DBACCESSORFLAGS dwFlags = DBACCESSOR_ROWDATA | DBACCESSOR_INHERITED;
	DBACCESSORFLAGS dwFlags2 = dwFlags | DBACCESSOR_OPTIMIZED;

	//Start with a clean rowset so optimized accessor creation is legal
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pIRowset);

	TESTC_(CreateRowsetObject(SELECT_VALIDATIONORDER), S_OK);
	
	if(!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))
		return TEST_FAIL;	

	//Create an optmized accessor, using odd columns
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ODD_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, &cCols,						
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);
		
		//Create a non optimized accessor using different columns (even ones)
	TESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		EVEN_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, &cCols2,		
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);

	//This should be the same number since its the same rowset
	TESTC(cCols==cCols2);

	// Specify the input arguments - the original accessor handles
	hAccInherited = m_hAccessor;
	hAccInherited2 = m_hAccessor2;

	// Allocate memory for DBBINDSTATUS arrays
	SAFE_ALLOC(pBindStatus, DBSTATUS, m_cBindings);
	SAFE_ALLOC(pBindStatus2, DBSTATUS, m_cBindings2);

	// Now create new accessors specifying DBACCESSOR_INHERITED flag.  Note that phAccessor
	// is an input argument.
	hr = m_pIAccessor->CreateAccessor(dwFlags,	m_cBindings2, m_rgBindings2, m_cbRowSize2,
		&hAccInherited2, pBindStatus2);

	// If the provider doesn't support inherited accessors then DB_E_BADACCESSORFLAGS is expected.
	if (DB_E_BADACCESSORFLAGS == hr)
	{
		odtLog << L"Provider does not support DBACCESSOR_INHERITED flag.\n";

		// Make sure original accessors still work
		hAccInherited = m_hAccessor;
		hAccInherited2 = m_hAccessor2;
		m_hAccessor = DB_NULL_HACCESSOR;
		m_hAccessor2 = DB_NULL_HACCESSOR;
		dwFlags &= ~DBACCESSOR_INHERITED;
		dwFlags2 &= ~DBACCESSOR_INHERITED;
		hr = S_OK;
	}
	else
		TESTC_(m_pIAccessor->CreateAccessor	(dwFlags2,	m_cBindings, m_rgBindings,
				m_cbRowSize, &hAccInherited, pBindStatus), S_OK);

	TESTC_(hr, S_OK);
	
	//Verify the data is correctly brought back using both accessors 
	//(via either GetData or ReadData). 
	TESTC_(UseRowAccessorAndVerify(hAccInherited, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, cCols, m_rgBindings, m_cBindings,
			FALSE, TRUE),S_OK);	//Keep same rows to be gotten by next call

	TESTC_(UseRowAccessorAndVerify(hAccInherited2, m_cbRowSize2, g_uiRowNum, 
			m_rgTableColOrds, cCols2, m_rgBindings2, m_cBindings2),S_OK);

	//Check first accessor with GetBindings  
	VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, hAccInherited, 
		dwFlags2);

	//Check second accessor with GetBindings
	VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, hAccInherited2, 
		dwFlags);		

	//We have to clean up since we kept the row in UseRowAccessorAndVerify
	if(m_hRow && m_fIRowset)
	{
		m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow,NULL, NULL,NULL);		
		m_hRow = DB_NULL_HROW;
	}

	fSuccess = TRUE;

CLEANUP:

	//Clean Up
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hAccInherited);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hAccInherited2);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor2);

	SAFE_FREE(pBindStatus);
	SAFE_FREE(pBindStatus2);

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//					  
BOOL TCCreateValidRowAccessors::Terminate()
{
	PROVIDER_FREE(m_pDBPropSet);
	PROVIDER_FREE(m_prgDBProps);

	//Release the interface we allocated in Init
	SAFE_RELEASE(m_pIRowset);
	ReleaseRowsetObject();	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}
			 

// {{ TCW_TC_PROTOTYPE(TCCreateValidParamAccessors)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateValidParamAccessors - Creation of valid parameter accessors
//|	Created:			11/11/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateValidParamAccessors::Init()
{	
	IColumnsInfo * pIColumnsInfo = NULL;
	ICommandPrepare * pICommandPrepare = NULL;
	BOOL fResult = TEST_FAIL;
	CCol	TempCol(m_pIMalloc);
	ULONG	i, iSearchable;
	DBORDINAL cColumns;
	DBCOLUMNINFO * pColumnInfo = NULL;
	WCHAR * pStringsBuffer = NULL;

	// Initialize m_pSaveTable since it's used in Terminate.
	m_pSaveTable = NULL;

	//if command is not supported, skip this test.
	if(!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//if parameter accessor is not supported, skip this test.
	if(!g_fParamAccessor)
	{
		odtLog << L"Parameter accessor is not supported."<<ENDL;
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		m_pSaveTable = m_pTable;
		
		//Get a command object to generate parameterized queries on,
		//and the rowset to find out what the columns will be 
		if (CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER), S_OK))
		{			

			//Create a table without using nulls because select fails on a null value.
			m_pTable = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
				(LPWSTR)gwszModuleName, NONULLS);

			if (!m_pTable)
			{	
				odtLog << wszMemoryAllocationError;
				goto CLEANUP;
			}		

			TESTC_(m_pTable->CreateTable(0), S_OK);

			//Determine what interface will be needed to retreive rows 
			//in UseParamAccessorAndVerify
			if(VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&m_pIRowset))			
			{
				//Record that IRowset is supported
				m_fIRowset = TRUE;
				//Don't need interface
				SAFE_RELEASE(m_pIRowset);
			}

			//Only need command after this
			ReleaseRowsetObject();
		
			/////////////////////////////////////////////////////////
			// Build array containing col ordinals for all searchable
			// cols so we can use them in parameterized queries
			/////////////////////////////////////////////////////////

			//Get memory to hold array of all col numbers.  NOTE:  This 
			//is the max possible, we won't necessarily use them all.
			m_rgSearchableCols = (DB_LORDINAL *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(DB_LORDINAL));
			if (!m_rgSearchableCols)
			{
				odtLog << wszMemoryAllocationError;
				goto CLEANUP;
			}
						
			// While there is a searchable flag in CCol it is derived from IColumnsRowset and as
			// such is not populated when using an ini file or if the provider doesn't support
			// IColumnsRowset.  So we'll use a select to give us the searchable colummns.
			TESTC_(m_pTable->ExecuteCommand(SELECT_SEARCHABLE, IID_IRowset,
				NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
				0, NULL, NULL, NULL, &m_pICommand), S_OK);

			// Get an IColumnsInfo interface
			TESTC(VerifyInterface(m_pICommand,IID_IColumnsInfo, COMMAND_INTERFACE,
				(IUnknown **)&pIColumnsInfo));

			// If the provider supports Prepare we must before getting col info
			if (VerifyInterface(m_pICommand,IID_ICommandPrepare, COMMAND_INTERFACE,
				(IUnknown **)&pICommandPrepare))
			{
				TESTC_(pICommandPrepare->Prepare(1), S_OK);
			}

			TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &pColumnInfo, &pStringsBuffer), S_OK);

			//We'll use this count as the index to the array as we build it
			m_cSearchableCols = 0;

			// Look through the available columns in the table looking for searchable
			// and updatable columns.  We don't include LONG columns due to limitations
			// of some providers.
			for (i=1; i<=m_pTable->CountColumnsOnTable(); i++)
			{
				TESTC_(m_pTable->GetColInfo(i, TempCol), S_OK);

				//Record the column number in the array
				//if it is searchable
				if (TempCol.GetIsLong())
					continue;

				if (TempCol.GetUpdateable())
				{
					// This is a candidate column, so we need to check searchability.  If it's
					// in the columns info obtained above then it's searchable.

/* Due to change in DBKIND reported by some providers just use the column name to identify
	the column.  This is a hack and is bad because it may not be the same as the base table
	column name.  The other choice would be to just use the name portion of the DBID which
	should be the base table name, but then why have a guid portion if the name is always
	sufficient?
					for (iSearchable=0;
						iSearchable < cColumns && !CompareDBID(pColumnInfo[iSearchable].columnid,
							*TempCol.GetColID());
						iSearchable++);
*/
					for (iSearchable=0;
						iSearchable < cColumns && 
							pColumnInfo[iSearchable].pwszName &&
							TempCol.GetColName() &&
							wcscmp(pColumnInfo[iSearchable].pwszName,
								TempCol.GetColName());
						iSearchable++);

					if (iSearchable < cColumns)
					{
						m_rgSearchableCols[m_cSearchableCols] = TempCol.GetColNum();				
						m_cSearchableCols++;
					}
				}
																	
			}
			

			// When running with ini file we already have rows in the table
			if (!GetModInfo()->GetFileName())
			{
				// Since this is a local table only to this test case lets insert both rows expected 
				// by the test case.
				// Insert one row for Parameter Verification
				TESTC_(m_pTable->InsertWithParams (g_uiRowNum), S_OK);
				
				// Insert another row for Parameter Verification
				TESTC_(m_pTable->InsertWithParams (g_uiRowNum + 1), S_OK);
			}
			
			//We've succeeded if we got this far
			fResult = TRUE;

		}
	}

CLEANUP:

	SAFE_FREE(pColumnInfo);
	SAFE_FREE(pStringsBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrepare);
	
	return fResult;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROWDATA and PARAMETERDATA accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_1()
{	
	BOOL		fSuccess = FALSE;					
	IAccessor * pCmdIAccessor = NULL;			
	
#ifdef COMMAND_ACCESSOR_HACK
	SAFE_RELEASE(m_pICommand);
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand), S_OK);
#endif

	//Get accessor interface on the command object on which we will do the execute
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&pCmdIAccessor))			
		goto CLEANUP;	
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
	
	//Create a ROWDATA and PARAMETERDATA accessor, using all cols
	//so we can verify the rowdata portion on the rowset	
	if (!CHECK(GetAccessorAndBindings(pCmdIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT,
		m_fBindLongCols),S_OK))
		goto CLEANUP;
	
	//Execute the query to generate a rowset object for the rowdata accessor	
	if (!CHECK(m_hr = m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, &m_rgTableColOrds, EXECUTE_ALWAYS, 
		0, NULL, NULL, (IUnknown **)&m_pIRowset, &m_pICommand), S_OK))
		goto CLEANUP;

	//Verify the data is correctly brought back using the accessor 	
	if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_pTable->CountColumnsOnTable(), m_rgBindings, 
			m_cBindings),S_OK))
		goto CLEANUP;

	if (!VerifyBindings(pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA))
		goto CLEANUP;

	//Cleanup rowset so we can use command again
	COMPARE(m_pIRowset->Release(), 0);
	m_pIRowset = NULL;
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	PROVIDER_FREE(m_rgTableColOrds);

	//Release this so we can start with another accessor testing parameters
	pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL);
	m_hAccessor = DB_NULL_HACCESSOR;
		
	//Now create a dual accessor, but use searchable columns 
	//and specify the right ordinals so we can use it for our 
	//parameterized statement
	if (!CHECK(GetAccessorAndBindings(pCmdIAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT, 
		m_fBindLongCols), S_OK))
		goto CLEANUP;
	
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
			DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA,
			m_cbRowSize, g_uiRowNum, m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;

	//Check accessor with GetBindings  			
	fSuccess = VerifyBindings(pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA);				

			
CLEANUP:
	
	if (m_hAccessor != DB_NULL_HACCESSOR)
	{
		pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pCmdIAccessor);

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	PROVIDER_FREE(m_rgTableColOrds);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Null Parameter Accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_2()
{
	BOOL	fSuccess = FALSE;
	ICommand * pICommand = NULL;
	IRowset * pRowset = NULL;

	
	//Get a command set to use parameters, have function return its new ICommand
	if (!CHECK(m_pTable->ExecuteCommand(
					SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,		//Parameterized query
					IID_IAccessor,					//We won't execute, this will be ignored
					NULL, NULL, NULL, NULL,			//Stuff we don't need since we aren't executing
					EXECUTE_NEVER,					//Only want to set the command
					0, NULL, NULL, NULL,			//More stuff we don't need
					&pICommand		 				//Want command object which was set
					), S_OK))
		goto CLEANUP;

	
	//Get accessor interface on the set command object
	if (!VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;	
	
	//It is invalid to create a null accessor on a command object
	fSuccess = CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, 0, NULL, 
		m_cbRowSize, &m_hAccessor, m_rgStatus),DB_E_NULLACCESSORNOTSUPPORTED);
		
							
CLEANUP:

	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(m_pIAccessor);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc dwPart - DBCOLUMPART_VALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_3()
{
	BOOL fSuccess = FALSE;
	CTable *	pSaveTable = NULL;
	ULONG i=0;
	
#ifdef COMMAND_ACCESSOR_HACK
	SAFE_RELEASE(m_pICommand);
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand), S_OK);
#endif

	//Create a table that doesn't use nulls so we never need to 
	//use a parameter's status, since we won't bind it
	pSaveTable = m_pTable;
	m_pTable = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
		(LPWSTR)gwszModuleName, NONULLS);
	if (!m_pTable)
	{	
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}		

	if (FAILED(m_pTable->CreateTable(0)))
		return FALSE;

	//Insert one row based on the number all our routines will expect
	//if (!CHECK(m_pTable->Insert(g_uiRowNum), S_OK))
	if (!CHECK(m_pTable->InsertWithParams(g_uiRowNum), S_OK))	
		goto CLEANUP;

	// Insert another row for Parameter Verification
	if (!CHECK (m_pTable->InsertWithParams (g_uiRowNum + 1), S_OK))
		goto CLEANUP;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;	

		//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Create Accessor with a binding using only DBCOLUMPART_VALUE
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE, USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT,
		m_fBindLongCols),S_OK))
		goto CLEANUP;

		
	// Fixup bindings now.  It's generally an error to not bind the length for
	// variable length types unless:
	//		For char types assume NULL terminated.
	//		For DBTYPE_BYTES assume length is cbMaxLen.
	//		For DBTYPE_VARNUMERIC assume length is cbMaxLength?
	// This was changed recently in the spec.
	for (i = 0; i < m_cBindings; i++ )
	{
		// Remove the length and status bindings
		m_rgBindings[i].dwPart = 0| DBPART_VALUE;
	}

	// Recreate the accessor since we've changed the bindings
	TESTC_(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, NULL), S_OK);

				
	//Verify parameter accessor can be used to successfully retrieve a row
	//NOTE:  We are assuming (since we didn't bind length) that cbMaxLen
	//is equal to the correct data length, so the right length of values
	//are used for the parameter.
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize, g_uiRowNum, 
		m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
		
	//GetBindings should return same values we've used to create the accessor
	// Since we changed the bindings this will fail.
	fSuccess = TRUE;
 	// fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_PARAMETERDATA);	
						
CLEANUP :
	
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
	}

	//Set our table back to the default one for the whole test module	
	m_pTable = pSaveTable;
		
	//Clean up 
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwPart - DBCOLUMPART_LENGTH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_4()
{
	BOOL fSuccess = FALSE;	
	BYTE * pParamData = NULL;
	ICommand * pICommand = NULL;
	DBPARAMS	Param;
	IRowset * pRowset = NULL;
		
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &pICommand), S_OK))
		goto CLEANUP;

	//Get accessor interface on the command object which is set
	if (!VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;
		
	//Create parameter Accessor with a binding using only DBCOLUMPART_LENGTH	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, m_fBindLongCols),S_OK))		
		goto CLEANUP;
	
	//Have function generate a new command object and set it with parameterized query
	if (!CHECK(m_pTable->ExecuteCommand(
				SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,		//Parameterized query
				IID_IAccessor,					//We won't execute, this will be ignored
				NULL, NULL, NULL, NULL,			//Stuff we don't need since we aren't executing
				EXECUTE_NEVER,					//Only want to set the command
				0, NULL, NULL, NULL,			//More stuff we don't need
				&pICommand		 				//Want command object which was set
				), S_OK))
		goto CLEANUP;


	pParamData = (BYTE *)m_pIMalloc->Alloc(m_cbRowSize);
	if (!pParamData)
		goto CLEANUP;

	//Set up our buffer to only have length bound
	if (!CHECK(FillInputBindings(m_pTable,  
		DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, &pParamData, g_uiRowNum, 
		m_cSearchableCols, m_rgSearchableCols, PRIMARY), S_OK))
		goto CLEANUP;

	//Set up parameters to use the accessor
	Param.cParamSets = 1;
	Param.hAccessor = m_hAccessor;
	Param.pData = (BYTE *)pParamData;

	//Verify that since we didn't set any default params,
	//using an accessor with only length bound should fail
	fSuccess = CHECK(pICommand->Execute(NULL, IID_IAccessor, &Param,  
		NULL, (IUnknown **)&pRowset), DB_E_ERRORSOCCURRED);

				
CLEANUP:
	PROVIDER_FREE(pParamData);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(pICommand);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc dwPart - DBCOLUMPART_STATUS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_5()
{
	BOOL fSuccess = FALSE;	
	BYTE * pParamData = NULL;
	ICommand * pICommand = NULL;
	DBPARAMS	Param;
	IRowset * pRowset = NULL;
	ULONG iIndex = 0;
	ULONG i=0;
	CCol TempCol;
		
		//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &pICommand), S_OK))
		goto CLEANUP;


	//Get accessor interface on the command object which is set
	if (!VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;
		
	// Create parameter Accessor with a binding using only DBCOLUMPART_STATUS
	// Currently Set to DBPART_ALL but later change to only DBPART_STATUS
	// This is done so that first we use FillINputBindings for all the bindings 
	// and convert bindings for which columns are nullable to DBSTATUS_S_ISNULL and binding to DBPART_STATUS only.

	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols, 
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols), S_OK))		
		goto CLEANUP;

		//Have function generate a new command object and set it with parameterized query
	if (!CHECK(m_pTable->ExecuteCommand(
				SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,		//Parameterized query
				IID_IAccessor,					//We won't execute, this will be ignored
				NULL, NULL, NULL, NULL,			//Stuff we don't need since we aren't executing
				EXECUTE_NEVER,					//Only want to set the command
				0, NULL, NULL, NULL,			//More stuff we don't need
				&pICommand		 				//Want command object which was set
				), S_OK))
		goto CLEANUP;

	pParamData = (BYTE *)m_pIMalloc->Alloc(m_cbRowSize);
	if (!pParamData)
		goto CLEANUP;

	
	//Set up our buffer to only have length bound
	if (!CHECK(FillInputBindings(m_pTable,  
		DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, &pParamData, g_uiRowNum, 
		m_cSearchableCols, m_rgSearchableCols, PRIMARY), S_OK))
		goto CLEANUP;

	// Now the bindings of nullable columns to DBPART_STATUS and set the status bit appropriately for them.
	for (i = 1; i <= m_pTable->CountColumnsOnTable(); i++)
	{
		CHECK(m_pTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array
		//if it is 
		//Record the column number in the array
		//if it is searchable
		if (TempCol.GetIsLong())
				continue;

		if (TempCol.GetUpdateable() && (TempCol.GetSearchable() != DB_UNSEARCHABLE))
		{

			ASSERT(iIndex < m_cBindings); 

			if (TempCol.GetNullable() == TRUE )
			{
				m_rgBindings[iIndex].dwPart = 0| DBPART_STATUS;	
				// Set pData to DBSTATUS_ISNULL;
				*((DBSTATUS *)((BYTE *)pParamData + m_rgBindings[iIndex].obStatus)) = DBSTATUS_S_ISNULL;

			}
			iIndex++; // For updateable Column.
		}
		
	}

	//Set up parameters to use the accessor
	Param.cParamSets = 1;
	Param.hAccessor = m_hAccessor;
	Param.pData = (BYTE *)pParamData;

	//Verify that query was executed successfully, even though no rows should be found
	// TODO

	fSuccess = CHECK(pICommand->Execute(NULL, IID_IAccessor, &Param, 
		NULL, (IUnknown **)&pRowset),S_OK);


CLEANUP:
	SAFE_RELEASE(pRowset);
	
	PROVIDER_FREE(pParamData);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(pICommand);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc dwPart - DBCOLUMPART_VALUE | DBPART_STATUS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_6()
{
	BOOL fSuccess = FALSE;		
	ULONG i=0;

	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;	

	//Create Accessor with a binding using only DBCOLUMPART_VALUE and STATUS.
	//The length should be determined by cbMaxLen alone.	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY,
		 FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT,
		m_fBindLongCols),S_OK))
		goto CLEANUP;
				
	// Fixup bindings now:
	for (i = 0; i < m_cBindings; i++ )
	{
		// For variable length types the length is required, although this is under spec review.
		// Bind only value and status for fixed length types and DBTYPE_BTYES.  For DBTYPE_BYTES if 
		// the consumer doesn't bind length provider must get length from cbMaxLen.
		if (IsFixedLength(m_rgBindings[i].wType) || DBTYPE_BYTES == m_rgBindings[i].wType)
			// For rest remove the length bindings.
			m_rgBindings[i].dwPart = DBPART_VALUE | DBPART_STATUS;
		
	}

	// Recreate the accessor since we've changed the bindings
	TESTC_(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, NULL), S_OK);

	//Verify 
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize, g_uiRowNum, 
		m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
		
	//GetBindings should return same values we've used to create the accessor
	// Since we changed the bindings this will fail.
	fSuccess = TRUE;
 	// fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_PARAMETERDATA);	
						
CLEANUP :
			
	//Clean up 
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc dwPart - DBCOLUMPART_LENGTH | DBPART_STATUS | DBPART_VALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_7()
{
	BOOL fSuccess = FALSE;	
	ULONG ulBindIdx;

	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;	

	//Create Accessor with a binding using all three parts, DBCOLUMPART_VALUE, LENGTH and STATUS.	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY, 
		 FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT,
		m_fBindLongCols),S_OK))
		goto CLEANUP;
				
	//Set cbMaxLen to a number larger than the length bound will ever be,
	//to ensure that the provider is not using cbMaxLen instead of length
	//bound and thus reading past end of value
	for (ulBindIdx=0; ulBindIdx<m_cBindings; ulBindIdx++)
		m_rgBindings[ulBindIdx].cbMaxLen = MAX_COL_SIZE + 1;

	//Verify 
	// TODO Fails to select.  Execute execute success fully.
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize, g_uiRowNum,	
		m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
		
	fSuccess = TRUE;
	//GetBindings should return same values we've used to create the accessor
	// Since we changed the cbMaxLen this comparision fails.
 	//fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_PARAMETERDATA);	
						
CLEANUP :
			
	//Clean up 
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc One optimized, one non optimized, using same fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_8()
{
	BOOL fSuccess = FALSE;	

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;

	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Create an optmized accessor, using all the columns 	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, m_fBindLongCols),S_OK))
		goto CLEANUP;
				
		//Create a non optimized accessor using the same columns
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;
	
				
	//Verify the first parameter accessor works in an execute statement	
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		m_cbRowSize, g_uiRowNum,	m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;

	//Verify the second parameter accessor works in an execute statement				
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize2, g_uiRowNum, 
		m_rgBindings2, m_cBindings2, m_pICommand),S_OK))
		goto CLEANUP;
					
	//Check first accessor with GetBindings  
	fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED) &&
	//Check second accessor with GetBindings
	VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_PARAMETERDATA);
				

CLEANUP:
			
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);
	
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc One non-optimized, one optimized, using same fields - different creation sequence
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_9()
{
	BOOL fSuccess = FALSE;	
	

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
				
	//Create a non optimized accessor using all the columns	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;
	

	//Create an optmized accessor, using same columns 
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	
	//Verify the first parameter accessor works in an execute statement				
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize2, g_uiRowNum,  
		m_rgBindings2, m_cBindings2, m_pICommand),S_OK))
		goto CLEANUP;
	
	
	//Verify the second parameter accessor works in an execute statement	
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		m_cbRowSize, g_uiRowNum, m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;

						
	//Check second accessor with GetBindings  
	fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED) &&
	//Check first accessor with GetBindings
	VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_PARAMETERDATA);

				
CLEANUP:
			
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;	

}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Two non-optimized, using same fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_10()
{
	BOOL fSuccess = FALSE;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;
	
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Create an optmized accessor, using all columns
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;		
		
		//Create another optimized accessor with same columns 
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

			
	//Verify the first parameter accessor works in an execute statement				
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA,
		m_cbRowSize, g_uiRowNum, m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
		
	//Verify the second parameter accessor works in an execute statement				
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA,
		m_cbRowSize2, g_uiRowNum, m_rgBindings2, m_cBindings2, m_pICommand),S_OK))
		goto CLEANUP;

			
	//Check first accessor with GetBindings  
	fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_PARAMETERDATA)	&&	
	//Check second accessor with GetBindings
	VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_PARAMETERDATA);

				
CLEANUP:
	
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc All searchable cols BY_REF, without DBMEMOWNER_PROVIDEROWNED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_11()
{
	BOOL fSuccess = FALSE;
	
	// Get an accessor interface on our command object	
	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;

	
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
			
	//Get accessor with all searchable data type cols bound BY_REF

	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, ALL_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols, g_rgParamOrds,		 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	//Verify the first parameter accessor works in an execute statement				

	// TODO Fails.  RETURNS DB_E_OVERFLOW.  (parameter accessor doesn't support passbyref).
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA,
		m_cbRowSize, g_uiRowNum, m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
	
	// Check first accessor with GetBindings  

	fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_PARAMETERDATA);
					
CLEANUP:
	
	if (m_pIAccessor)
	{
		if (m_hAccessor)
			m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
		SAFE_RELEASE(m_pIAccessor);
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc More than 256 parameters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCreateValidParamAccessors::Variation_12()
{

	BOOL fSuccess = FALSE;	
	DBCOUNTITEM iBinding, iCol;
	HRESULT hrCreateAccessor=S_OK;
	DBCOUNTITEM cAdditionalBindings=0;
	WCHAR *	pwszStatement=NULL;
	const WCHAR wszFormat[] = L" AND %s = ?";
	CCol TempCol;
	WCHAR * pwszColName=NULL;
	WCHAR * pwszConcatVal=NULL;
	DB_LORDINAL * pParamCols=NULL;
	DBROWCOUNT cRowsAffected=0;
	BYTE * pData = NULL;
	DBPARAMS Param;
	IRowset * pRowset = NULL;
	DBLENGTH ulOffset;

	// Make sure there are some searchable columns
	if (!m_cSearchableCols)
	{
		odtLog << "No searchable columns available.\n";
		return TEST_SKIPPED;
	}

	// Retrieve the name of the first searchable column
	m_pTable->GetColInfo(m_rgSearchableCols[0], TempCol);
	pwszColName=TempCol.GetColName();

	// Allocate some space for the column info we're going to tack on
	if (!(pwszConcatVal=(WCHAR *)m_pIMalloc->Alloc((wcslen(wszFormat)+wcslen(pwszColName)+1)*sizeof(WCHAR))))
		goto CLEANUP; 

	// Fill it
	swprintf(pwszConcatVal, wszFormat, pwszColName);
		
	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE, IID_IRowset,
		NULL, &pwszStatement, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;	

	//Create Accessor with a binding using all three parts, DBCOLUMPART_VALUE, LENGTH and STATUS.	
	hrCreateAccessor=GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, ALL_COLS_BOUND, 
		 FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL,
		NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT,
		m_fBindLongCols);

	if (!CHECK(hrCreateAccessor, S_OK))
		goto CLEANUP;

	// Release the accessor we got previously
	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	// Compute the number of additional bindings required to hit our max
	if (MAX_BIND_LIMIT > m_cBindings)
	{
		cAdditionalBindings=MAX_BIND_LIMIT-m_cBindings;
		// Increase size of binding array
		if (!(m_rgBindings=(DBBINDING *)m_pIMalloc->Realloc(m_rgBindings, 
			MAX_BIND_LIMIT*sizeof(DBBINDING))))
			goto CLEANUP;

		// Increase size of sql stmt
		if (!(pwszStatement=(WCHAR *)m_pIMalloc->Realloc(pwszStatement, 
			(wcslen(pwszStatement)+wcslen(pwszConcatVal)*cAdditionalBindings+1)*sizeof(WCHAR))))
			goto CLEANUP;

		for (iBinding=m_cBindings; iBinding < m_cBindings+cAdditionalBindings; iBinding++)
		{
			ulOffset = ROUND_UP(sizeof(DBSTATUS), ROUND_UP_AMOUNT);

			ulOffset+= sizeof(DBLENGTH)+m_rgBindings[iBinding-1].cbMaxLen;
			
			// Adjust for alignment
			ulOffset = ROUND_UP(ulOffset,ROUND_UP_AMOUNT);

			memcpy(&m_rgBindings[iBinding], &m_rgBindings[0], sizeof(DBBINDING));
			m_rgBindings[iBinding].iOrdinal=iBinding+1;
			m_rgBindings[iBinding].obStatus=m_rgBindings[iBinding-1].obStatus+ulOffset;
			m_rgBindings[iBinding].obLength=m_rgBindings[iBinding-1].obLength+ulOffset;
			m_rgBindings[iBinding].obValue=m_rgBindings[iBinding-1].obValue+ulOffset;

			// Increase the row size
			m_cbRowSize+=ulOffset + m_rgBindings[iBinding].cbMaxLen;

			wcscat(pwszStatement, pwszConcatVal);
		}
		m_cBindings=m_cBindings+cAdditionalBindings;

		// Set the new text value
		m_pTable->BuildCommand(pwszStatement, IID_IRowset, EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &m_pICommand);
	}
	else
		cAdditionalBindings=0;


	hrCreateAccessor=m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, NULL);

	if (!CHECK(hrCreateAccessor, S_OK))
		goto CLEANUP;
		

	// Allocate memory for the parameter columns.  This is the list of searchable
	// columns from the table with the first searchable column repeated at the end
	// to make up the required number of parameters.
	pParamCols = (DB_LORDINAL *)m_pIMalloc->Alloc(m_cBindings*sizeof(DB_LORDINAL));
	if (!pParamCols)
		goto CLEANUP;

	// Copy the searchable columns
	memcpy(pParamCols, m_rgSearchableCols, m_cSearchableCols*sizeof(DB_LORDINAL));
	
	// Set the rest of the columns to be the same as the first searchable one
	for (iCol=m_cSearchableCols; iCol < m_cBindings; iCol++)
		pParamCols[iCol]=pParamCols[0];

	//GetBindings should return same values we've used to create the accessor
 	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_PARAMETERDATA))
		goto CLEANUP;
	
	//Alloc enough memory to hold a row of parameter data
	pData = (BYTE *)m_pIMalloc->Alloc(m_cbRowSize);	

	if (!pData)
		goto CLEANUP;
	memset(pData, 0, (size_t)m_cbRowSize);
	

	//Set up parameter input values for selecting row 1
	if (FAILED(m_hr = FillInputBindings(m_pTable,  
		DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, &pData, 1, 
		m_cBindings, pParamCols, PRIMARY)))
		goto CLEANUP;

	Param.cParamSets = 1;
	Param.hAccessor = m_hAccessor;
	Param.pData = pData;

	// Now try to use the accessor
	if (!CHECK(m_pICommand->Execute(NULL, IID_IRowset, &Param, 
			&cRowsAffected, (IUnknown **)&pRowset),S_OK))
		goto CLEANUP;

/*	TODO: See why this is commented out

	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize, g_uiRowNum,	
		m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;
*/

	odtLog << L"Created " << (ULONG)m_cBindings << L" parameters.\n";

	fSuccess = TRUE;
						
CLEANUP :
			
	//Clean up 
	SAFE_RELEASE(pRowset);

	if (m_hAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_RELEASE(m_pIAccessor);

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	PROVIDER_FREE(pwszStatement);
	PROVIDER_FREE(pwszConcatVal);
	PROVIDER_FREE(pParamCols);
	PROVIDER_FREE(pData);


	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Use DBBINDFLAGS_HTML in parameter accessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateValidParamAccessors::Variation_13()
{ 
	BOOL fSuccess = TEST_FAIL;	
	BOOL fStringType = FALSE;
	ULONG iBind;
	DBBINDSTATUS * pBindStatus = NULL;

	// For providers that support the HTML flag, this is a success variation.
	// For providers that do not support it an error should be returned.  At this
	// point it's not clear whether the error will occur at CreateAccessor time or
	// Execute time.
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK);

	//Get an accessor interface on our command object	
	TESTC(VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor));

	//Create Accessor with a binding using all three parts, DBCOLUMPART_VALUE, LENGTH and STATUS.	
	TESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS, USE_COLS_TO_BIND_ARRAY, 
		 FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT,
		m_fBindLongCols),S_OK);
				
	// Release the accessor we got above so we can change the binding information
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 

	// Set dwFlags to DBBINDFLAG_HTML for all string types.
	for(iBind = 0; iBind < m_cBindings; iBind++)
	{
		if (IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_STR) ||
			IS_BASE_TYPE(m_rgBindings[iBind].wType, DBTYPE_WSTR))
		{
			m_rgBindings[iBind].dwFlags = DBBINDFLAG_HTML;
			fStringType = TRUE;
		}
	}

	if (!fStringType)
	{
		odtLog << L"No string types available to test.\n";
		fSuccess = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Now recreate the accessor.  Note specifying DBBINDFLAGS_HTML is legal even if provider 
	// doesn't support it.  It should be ignored.
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, pBindStatus), S_OK);

	// Verify.  The expected result is a failure for providers that don't support the HTML flag.
	// Per spec, providers that don't support the HTML flag can't use the flag to set data.
	UseParamAccessorAndVerify(m_hAccessor, DBACCESSOR_PARAMETERDATA, m_cbRowSize, g_uiRowNum,	
		m_rgBindings, m_cBindings, m_pICommand, DB_E_BADBINDINFO, TRUE);
		
	//GetBindings should return same values we've used to create the accessor
 	TESTC(VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, DBACCESSOR_PARAMETERDATA));

	fSuccess = TEST_PASS;
						
CLEANUP :
			
	//Clean up 
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor); 
	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);

	return fSuccess;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DBACCESSOR_INHERITED - Or'd with DBACCESSOR_OPTIMIZED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateValidParamAccessors::Variation_14()
{ 
	HACCESSOR hAccInherited = m_hAccessor;
	HACCESSOR hAccInherited2 = m_hAccessor2;
	DBBINDSTATUS  * pBindStatus;
	DBBINDSTATUS * pBindStatus2;
	BOOL fSuccess = FALSE;	
	HRESULT hr = E_FAIL;

	//Get an accessor interface on our command object	
	if (!VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE,
		(IUnknown **)&m_pIAccessor))			
		goto CLEANUP;
	
	//Just set the command so we can do IColumnsInfo to generate accessor
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset,
		NULL, NULL, NULL, NULL, EXECUTE_NEVER, 
		0, NULL, NULL, NULL, &m_pICommand), S_OK);

	//Create an optmized accessor, using all the columns 	
	TESTC_(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, m_fBindLongCols),S_OK);
				
		//Create a non optimized accessor using the same columns
	TESTC_(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_PARAMETERDATA,
		&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY,  FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, m_cSearchableCols, m_rgSearchableCols,
		g_rgParamOrds, NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, 
		m_fBindLongCols),S_OK);

	// Specify the input arguments - the original accessor handles
	hAccInherited = m_hAccessor;
	hAccInherited2 = m_hAccessor2;

	// Allocate memory for DBBINDSTATUS arrays
	SAFE_ALLOC(pBindStatus, DBSTATUS, m_cBindings);
	SAFE_ALLOC(pBindStatus2, DBSTATUS, m_cBindings2);

	// Now create new accessors specifying DBACCESSOR_INHERITED flag.  Note that phAccessor
	// is an input argument.  Since the INHERITED flag is *only* for row accessors this should be 
	// DB_E_BADACCESSORFLAGS.
	TESTC_(m_pIAccessor->CreateAccessor	(
		DBACCESSOR_PARAMETERDATA  | DBACCESSOR_INHERITED,
		m_cBindings2, m_rgBindings2, m_cbRowSize2, &hAccInherited2, pBindStatus2), 
		DB_E_BADACCESSORFLAGS);

	// Try with OPTIMIZED flag also.
	TESTC_(m_pIAccessor->CreateAccessor	(
		DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED | DBACCESSOR_INHERITED,
		m_cBindings, m_rgBindings, m_cbRowSize, &hAccInherited, pBindStatus), 
		DB_E_BADACCESSORFLAGS);

	// Make sure the original accessors still work

	//Verify the first parameter accessor works in an execute statement	
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED,
		m_cbRowSize, g_uiRowNum,	m_rgBindings, m_cBindings, m_pICommand),S_OK))
		goto CLEANUP;

	//Verify the second parameter accessor works in an execute statement				
	if (!CHECK(UseParamAccessorAndVerify(m_hAccessor, 
		DBACCESSOR_PARAMETERDATA, m_cbRowSize2, g_uiRowNum, 
		m_rgBindings2, m_cBindings2, m_pICommand),S_OK))
		goto CLEANUP;
					
	//Check first accessor with GetBindings  
	fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_PARAMETERDATA | DBACCESSOR_OPTIMIZED) &&
	//Check second accessor with GetBindings
	VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, DBACCESSOR_PARAMETERDATA);
				

CLEANUP:

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hAccInherited);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hAccInherited2);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor2);
	SAFE_RELEASE(m_pIAccessor);
	
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	SAFE_FREE(pBindStatus);
	SAFE_FREE(pBindStatus2);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateValidParamAccessors::Terminate()
{
		
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
	}

	//Set our table back to the default one for the whole test module	
	m_pTable = m_pSaveTable;

	PROVIDER_FREE(m_rgSearchableCols);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAccessorSequencing)
//*-----------------------------------------------------------------------
//| Test Case:		TCAccessorSequencing - Accessors created and used in various sequences
//|	Created:			11/11/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessorSequencing::Init()
{		

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		TESTC_(OpenRowsetObject(FALSE), S_OK);

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc One accessor created on command, one on rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_1()
{
	BOOL fSuccess = FALSE;	
	ICommand * pICommand = NULL;

	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}


	//Create an accessor on the command object
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor2, NULL), S_OK))
		goto CLEANUP;

	TESTC_(OpenRowsetObject(FALSE), S_OK);
	
	//Create an accessor on the rowset object
	if (!CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	//Make sure we can GetBindings at this point	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;
		

	//Now use both accessors on rowset 	

	//Try rowset created accessor first
	if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
			m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings,
			FALSE, TRUE),S_OK))	//Keep same rows to be gotten by next call							
		goto CLEANUP;

	if(g_fCmdSupported)
	{
		//Next try command created accessor
		if (!CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings,
				FALSE, TRUE),S_OK))							
			goto CLEANUP;
	}

	//Check bindings on command before we release inherited accessor on the command
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor2, 
		DBACCESSOR_ROWDATA))
		goto CLEANUP;
	
	//Should be able to release inherited accessor on the command without
	//any effect on the rowset.  Note we don't null the handle 'cause we want to
	// use it again.
	if (m_hAccessor2)
	{
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor2, NULL),S_OK);		
	}

	if(g_fCmdSupported)
	{
		//Using inherited command accessor on rowset after it's been released on
		//the command should work
		if (!CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK))							
			goto CLEANUP;
	}

	//Check accessors with GetBindings done on rowset's IAccessor interface
	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;	
	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;				
			
	
	fSuccess = TRUE;
	
CLEANUP:

	//We have to clean up since we kept the row in UseRowAccessorAndVerify
	if(m_hRow && m_fIRowset)
	{
		m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow,NULL, NULL,NULL);		
		m_hRow = DB_NULL_HROW;
	}


	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	CleanUpRowsetObject();

	return (fSuccess) ? TEST_PASS : TEST_FAIL;

}

// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc One accessor created on command, one on rowset, using different fields
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_2()
{
	BOOL fSuccess = FALSE;	
	DBORDINAL cCols;
	
	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);
	
	//Create an accessor on the command object, using 
	//binding info generated in init, which binds all cols	
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Create an accessor on the rowset object, using only odd cols
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor2, &m_rgBindings2, 
		&m_cBindings2, &m_cbRowSize2, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH, ODD_COLS_BOUND,
		FORWARD, NO_COLS_BY_REF, NULL, &cCols,						
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	//Now use both accessors on rowset 	

	//Try rowset created accessor first
	if (!CHECK(UseRowAccessorAndVerify(m_hAccessor2, m_cbRowSize2, g_uiRowNum, 
			m_rgTableColOrds, cCols, m_rgBindings2, m_cBindings2,
			FALSE, TRUE),S_OK))	
		goto CLEANUP;

	//Next try command created accessor	
	if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
		m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK))							
		goto CLEANUP;

	//Check accessors with GetBindings done on rowset's IAccessor interface	
	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;

	if (!VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
			goto CLEANUP;
				
	//Now check command accessor with GetBindings done on command's IAccessor interface	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
			goto CLEANUP;
	
	fSuccess = TRUE;
	
CLEANUP:
	
	if (m_hAccessor2 && m_pIAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL),S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}
	PROVIDER_FREE(m_rgBindings2);

	//We have to clean up since we kept the row in UseRowAccessorAndVerify
	if(m_hRow && m_pIRowset)
	{
		m_pIRowset->ReleaseRows(m_cRowsObtained,&m_hRow, NULL, NULL,NULL);		
		m_hRow = DB_NULL_HROW;
	}


	//Make it clear to other variations that hRow is invalid
	m_hRow = DB_NULL_HROW;

	CleanUpRowsetObject();
	
	return (fSuccess) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Two ROWDATA accessors on command object, one before and one after execute
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_3()
{
	HROW * 		rgOnehRow = &m_hRow;
	BOOL fSuccess = FALSE;		
	BYTE * pData = (BYTE *)1;	//We should error out before this gets touched,
						//so use a bogus value for it

	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Create an accessor on the command object, using 
	//binding info generated in init, which binds all cols.
	//This way we get coverage before a command is set	
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Create another accessor on the command object, after execute	
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor2, NULL), S_OK))
		goto CLEANUP;

	//Make sure we can do GetBindings at this point	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;
		

	if(g_fCmdSupported)
	{
		//Try command accessor	created before execute - should work
		if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings),S_OK))							
			goto CLEANUP;
	}

	//Try accessor created after execute - should fail because it was created after
	//the rowset was
	if (m_fIRowset)
	{
		// This could return DB_S_COMMANDREEXECUTED as well as S_OK.
		if (!SUCCEEDED(m_pIRowset->RestartPosition(NULL)))
			goto CLEANUP;

		if (CHECK(m_hr = m_pIRowset->GetNextRows(NULL, 0, 1, 
			&m_cRowsObtained, (HROW **)&rgOnehRow), S_OK))
		{
			if (!CHECK(m_pIRowset->GetData(m_hRow, m_hAccessor2, pData), DB_E_BADACCESSORHANDLE))
			{	
				CHECK(m_pIRowset->ReleaseRows(1,rgOnehRow, NULL, NULL, NULL), S_OK);
				m_hRow = DB_NULL_HROW;
				goto CLEANUP;
			}

			CHECK(m_pIRowset->ReleaseRows(1,rgOnehRow, NULL, NULL, NULL), S_OK);
			m_hRow = DB_NULL_HROW;
			if(pData!=(BYTE *)1)
				goto CLEANUP;				
		}
		else
			goto CLEANUP;
	}
	else
		goto CLEANUP;

	//Check accessor with GetBindings done on rowset's IAccessor interface	
	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;
		
	//Now check both command accessors with GetBindings done on command's IAccessor interface	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor2, 
		DBACCESSOR_ROWDATA) ||
		!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA))
		goto CLEANUP;
		
	fSuccess = TRUE;

CLEANUP:

	CleanUpRowsetObject();
	
	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Create and use accessor after Setting command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_4()
{
	BOOL fSuccess = FALSE;	
	DBORDINAL	cCols;		
	
	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// Release the rowset but leave the command object
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIAccessor);
	SAFE_FREE(m_rgTableColOrds);
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);
	
	//Set command, but don't execute	
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER,
				IID_IRowset, NULL, NULL, &cCols, &m_rgTableColOrds, EXECUTE_NEVER,
				0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
	
	//Create an accessor on the command object
	if (!CHECK(GetAccessorAndBindings(m_pCmdIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor, &m_rgBindings, 
		&m_cBindings, &m_cbRowSize, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH, EVEN_COLS_BOUND, 
		FORWARD, NO_COLS_BY_REF, NULL, &cCols,						
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	//Make sure we can do GetBindings at this point
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;
	
	//Execute command
	if (!CHECK(m_pICommand->Execute(NULL, IID_IAccessor, NULL, 
		NULL, (IUnknown **)&m_pIAccessor),S_OK))
		goto CLEANUP;

	//Try to get an IRowset interface 
	if(VerifyInterface(m_pIAccessor,IID_IRowset,ROWSET_INTERFACE,
		(IUnknown **)&m_pIRowset))
	{
		//Record that IRowset is supported
		m_fIRowset = TRUE;		
	}

	if(g_fCmdSupported)
	{
			//Now use accessor	
		if (!CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, cCols, m_rgBindings, m_cBindings),S_OK))							
			goto CLEANUP;
	}

	//Check accessors with GetBindings on rowset's IAccessor
	if (VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
	{
		//Now verify bindings using command's IAccessor
		fSuccess = VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA);
	}
	
	
CLEANUP:

	//Release the interface we allocated on rowset object
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIAccessor);

	if (m_hAccessor)
	{
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	
	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Create and use accessor after reading data via IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_5()
{
	BOOL		fSuccess = FALSE;	
	HROW *		rghRow = &m_hRow;
	BYTE *		pData = NULL;
	DBCOUNTITEM	cRows = 1;
	DBROWCOUNT	cRowSkip;

	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// Skip any existing rows in the table and get the last one.
	cRowSkip = m_pTable->GetRowsOnCTable()-1;
	
	//Get a row before creating the accessors
	m_hRow = DB_NULL_HROW;

	if (!CHECK(m_pIRowset->GetNextRows(NULL,cRowSkip, 1, 
		&cRows, &rghRow),S_OK))
		goto CLEANUP;

	//Create an accessor on the command object using bindings we got earlier
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	
		m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	//Create an accessor on the rowset object
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor2, &m_rgBindings2, 
		&m_cBindings2, &m_cbRowSize2, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH, ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	pData = (BYTE *)m_pIMalloc->Alloc(m_cbRowSize2);
	if (!pData)
		goto CLEANUP;

	//Use rowset accessor
	if (!CHECK(m_pIRowset->GetData(m_hRow, m_hAccessor2, pData), S_OK))
		goto CLEANUP;

	//Verify data value, length and status are what is expected
	TESTC(CompareData(m_cBindings, m_rgTableColOrds, g_uiRowNum+cRowSkip, pData, m_cBindings2,
		m_rgBindings2, m_pTable, m_pIMalloc));
	
	//Check accessor with GetBindings on rowset's IAccessor		
	if (!VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;


	//Now check command accessor on the command's IAccessor	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
		DBACCESSOR_ROWDATA))
		goto CLEANUP;
	
	//Everything went OK if we got this far
	fSuccess = TRUE;

CLEANUP:

	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_hAccessor);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor2);

	PROVIDER_FREE(pData);

	if (m_hRow && m_pIRowset)
	{
		m_pIRowset->ReleaseRows(1, &m_hRow, NULL,  NULL, NULL);
		m_hRow = DB_NULL_HROW;
	}

	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);
	CleanUpRowsetObject();
	
	return (fSuccess) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Create and use accessor after GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_6()
{
	BOOL		fSuccess = FALSE;
	HROW *		rghRow = &m_hRow;
	BYTE *		pData = NULL;
	DBCOUNTITEM	cRows = 1;
	DBCOUNTITEM cRowSkip;

	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	// We don't want our default rowset object from init
	CleanUpRowsetObject();

	// Skip any existing rows in the table and get the last one.
	cRowSkip = m_pTable->GetRowsOnCTable()-1;
	
	//Set command, but don't execute	
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER,
				IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER,
				0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	m_hRow = DB_NULL_HROW;

	//Get a row before creating the accessor
	if (!CHECK(m_pIRowset->GetNextRows(NULL, cRowSkip, 1, 
		&cRows, &rghRow),S_OK))
		goto CLEANUP;

	//Create an accessor on the command object
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBindings,
		m_rgBindings, m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;
	
	//Create an accessor on the rowset object		
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor2, &m_rgBindings2, 
		&m_cBindings2, &m_cbRowSize2, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH, ALL_COLS_BOUND,										
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	pData = (BYTE *)m_pIMalloc->Alloc(m_cbRowSize2);
	if (!pData)
		goto CLEANUP;
	
	//Now use rowset accessor
	if (!CHECK(m_pIRowset->GetData(m_hRow, m_hAccessor2, pData), S_OK))
		goto CLEANUP;
	
	//Verify data value, length and status are what is expected
	if (!CompareData(m_cBindings2, m_rgTableColOrds, g_uiRowNum+cRowSkip,pData, m_cBindings2,
		m_rgBindings2, m_pTable, m_pIMalloc))
		goto CLEANUP;

	//Check accessors with GetBindings on rowset's IAccessor	
	if (!VerifyBindings(m_pIAccessor, m_rgBindings2, m_cBindings2, m_hAccessor2, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;

	//Check accessors with GetBindings on command's IAccessor	
	if (!VerifyBindings(m_pCmdIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;


	fSuccess = TRUE;

CLEANUP:
		
	//Should be able to release command accessor before rowset
	//is released in this case, since this command accessor
	//isn't being used by the rowset
	if (m_hAccessor != DB_NULL_HACCESSOR && m_pCmdIAccessor)
	{
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2 != DB_NULL_HACCESSOR && m_pIAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL),S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	PROVIDER_FREE(m_rgBindings2);
	PROVIDER_FREE(pData);

	if (m_hRow && m_fIRowset)
	{
		m_pIRowset->ReleaseRows(1, &m_hRow,  NULL, NULL, NULL);
		m_hRow = DB_NULL_HROW;
	}

	CleanUpRowsetObject();
	
	return (fSuccess) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc ReleaseAccessor at different times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_7()
{

	BOOL		fSuccess = FALSE;
	DBORDINAL	cCols;
	ULONG		ulRefCount = 0;
	
	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// Release the rowset but leave the command object
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIAccessor);
	SAFE_FREE(m_rgTableColOrds);

	//Create an accessor on the command object, using 
	//binding info generated in init, which binds all cols.
	//This way we get coverage before a command is set	
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	//Try release here
	if (!CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, &ulRefCount), S_OK))
	{
		COMPARE(ulRefCount, 0);
		goto CLEANUP;
	}
	else
		m_hAccessor = DB_NULL_HACCESSOR;
	
	//Set command, but don't execute	
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER,
				IID_IRowset, NULL, NULL, &cCols, &m_rgTableColOrds, EXECUTE_NEVER,
				0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
	
	//Create an accessor on the command object with the command set
	if (!CHECK(GetAccessorAndBindings(m_pCmdIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor, NULL, 
		NULL, NULL, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	//Try release here
	if (!CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK))
		goto CLEANUP;
	else
		m_hAccessor = DB_NULL_HACCESSOR;

	//Now get another one before executing
	if (!CHECK(GetAccessorAndBindings(m_pCmdIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor, NULL, 
		NULL, NULL, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
		goto CLEANUP;

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Releasing command accessor with open rowset should succeed.
	//Note we intentionally will not release this accessor on the
	//rowset object, to verify that nothing nasty happens when
	//the rowset is released with rowset accessors still around.
	//The rowset accessors should just be implicitly released at that point.
	if (!CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK))
		goto CLEANUP;	
	
	TESTC_(OpenRowsetObject(FALSE), S_OK);

	//Create an accessor on the command after the execute	
	if (!CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,	m_cBindings, m_rgBindings, 
		m_cbRowSize, &m_hAccessor, NULL), S_OK))
		goto CLEANUP;

	m_hAccessor = DB_NULL_HACCESSOR;
	
	//Create an accessor on the rowset after the execute
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,	&m_hAccessor2, NULL, 
		NULL, NULL, DBPART_VALUE | 
		DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
			goto CLEANUP;


	//Try release here
	if (!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK))
		goto CLEANUP;

	m_hAccessor2 = DB_NULL_HACCESSOR;

	//If we got this far, we succeeded
	fSuccess = TRUE;

CLEANUP:

	//Try release here, 
	CleanUpRowsetObject();

	return (fSuccess) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Use Command accessor after freeing command object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorSequencing::Variation_8()
{	
	BOOL fSuccess = FALSE;
	DBCOUNTITEM cCommandBindings = 0;

	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	//Make this none zero so we know it changed to the expected zero value
	ULONG	ulRefCount = 1;	
	
	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// Release the rowset but leave the command object
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIAccessor);
	FREE_BINDINGS(&m_cBindings, &m_rgBindings);

	//Set command, but don't execute, just so we can call IColumnsInfo	
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER,
				IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER,
				0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
		

	//Create a command object accessor
	if (!CHECK(GetAccessorAndBindings(m_pCmdIAccessor, 
			DBACCESSOR_ROWDATA,	&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize, 
			DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, 			
			ALL_COLS_BOUND,					
			FORWARD, NO_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongCols),S_OK))
		goto CLEANUP;

	cCommandBindings = m_cBindings;
	

	TESTC_(OpenRowsetObject(FALSE), S_OK);

	// Release IAccessor interface obtained in CreateRowsetObject so we won't overwrite it below
	SAFE_RELEASE(m_pIAccessor);

	m_pICommand->Execute(NULL, IID_IAccessor, NULL, NULL, (IUnknown **)&m_pIAccessor); 

	//Try to get an IRowset interface 
	SAFE_RELEASE(m_pIRowset);
	if(VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,
		(IUnknown **)&m_pIRowset))
	{
		//Record that IRowset is supported
		m_fIRowset = TRUE;		
	}
	else
		goto CLEANUP;


	//Now command object should be releaseable, command accessor should
	//go away with no problem, but inherited accessor should still work
	SAFE_RELEASE(m_pCmdIAccessor);
	SAFE_RELEASE(m_pICommand);

	// At this point m_cBindings contains the number of bindings from the ROWSET.
	// If this is different than we got on the command above, then it's a provider
	// bug, but we won't be able to validate data.
	TESTC(cCommandBindings == m_cBindings);
	
	if(g_fCmdSupported)
	{
		//We should still be able to use the accessor we created on command object	
		if (S_OK != UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
				m_rgTableColOrds, m_cBindings, m_rgBindings, m_cBindings))
			goto CLEANUP;
	}

	if (!VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
			DBACCESSOR_ROWDATA))
		goto CLEANUP;	
	
	//We should also be able to release this inherited accessor
	if (!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, &ulRefCount), S_OK))
		goto CLEANUP;

	COMPARE(ulRefCount, 0);
	m_hAccessor = DB_NULL_HACCESSOR;

	//If we got this far, we have succeeded
	fSuccess = TRUE;

CLEANUP:

	CleanUpRowsetObject();
	
	//Get a command object back for any subsequent variations
	CHECK(CreateCommandObject(), S_OK);		

	// Some providers can't retrieve BLOB data without IRowsetLocate on, so here
	// we turn back on since we lost it when recreating the command object.
	if (m_fBindLongCols == BLOB_LONG && SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IRowsetLocate, TRUE), S_OK);

	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc CreateAccessor from GetBindings and Release, validate pBinding
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAccessorSequencing::Variation_9()
{ 
	BOOL	fResults = FALSE;
	DBBINDING * pBinding = NULL;

	TESTC_(OpenRowsetObject(TRUE), S_OK);

	// Save a copy of the binding info for later comparison
	SAFE_ALLOC(pBinding, DBBINDING, m_cBindings * sizeof(DBBINDING));
	memcpy(pBinding, m_rgBindings, (size_t)(m_cBindings * sizeof(DBBINDING)));

	if (g_fCmdSupported)
	{
		TESTC_(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
			pBinding, m_cbRowSize, &m_hAccessor, m_rgStatus), S_OK);

		// Call GetBindings and validate the binding information remains the same
		if (!VerifyBindings(m_pCmdIAccessor, pBinding, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA))
			goto CLEANUP;
		
		SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_hAccessor);

		// Now release the accessor, bindings should remain the same
		TESTC(memcmp(pBinding, m_rgBindings,(size_t)(m_cBindings * sizeof(DBBINDING))) == 0);

	}

	// Now do the same thing for a rowset accessor

	//Null phAccessor should return invalid arg
	TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		pBinding, m_cbRowSize, &m_hAccessor, m_rgStatus), S_OK);

	// Call GetBindings and validate the binding information remains the same
	if (!VerifyBindings(m_pIAccessor, pBinding, m_cBindings, m_hAccessor, DBACCESSOR_ROWDATA))
		goto CLEANUP;

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	// Now release the accessor, bindings should remain the same
	TESTC(memcmp(pBinding, m_rgBindings, (size_t)(m_cBindings * sizeof(DBBINDING))) == 0);

	fResults = TRUE;
	
CLEANUP:

	SAFE_FREE(pBinding);

	return (fResults) ? TEST_PASS : TEST_FAIL;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc CreateAccessor on new command object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAccessorSequencing::Variation_10()
{ 
	BOOL fSuccess = TEST_FAIL;	
	DBORDINAL cCols;		
	ICommand * pICommand = NULL;
	IAccessor * pIAccessor = NULL;
	IRowset * pISaveRowset = m_pIRowset;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DB_LORDINAL * pTableColOrds = NULL;
	
	if (!g_fCmdSupported)
	{
		//provider does not support commands
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	// Create a new command object, don't set any command text.
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&pICommand), S_OK);

	// Some providers can't retrieve BLOB data without IRowsetLocate on.
	if (m_fBindLongCols == BLOB_LONG && SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		TESTC_(SetRowsetProperty(pICommand, DBPROPSET_ROWSET, DBPROP_IRowsetLocate, TRUE), S_OK);

	// Get an accessor interface
	TESTC(VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE,
			(IUnknown **)&pIAccessor));

	// Create an accessor using our known bindings
	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &hAccessor, m_rgStatus), S_OK);

	//Make sure we can do GetBindings at this point
	TESTC(VerifyBindings(pIAccessor, m_rgBindings, m_cBindings, hAccessor, 
			DBACCESSOR_ROWDATA));
	
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER,
				IID_IRowset, NULL, NULL, &cCols, &pTableColOrds, EXECUTE_NEVER,
				0, NULL, NULL, NULL, &pICommand), S_OK);

	//Execute command
	TESTC_(pICommand->Execute(NULL, IID_IRowset, NULL, 
		NULL, (IUnknown **)&m_pIRowset),S_OK);

	//Now use accessor	
	TESTC_(UseRowAccessorAndVerify(hAccessor, m_cbRowSize, g_uiRowNum, 
			pTableColOrds, cCols, m_rgBindings, m_cBindings),S_OK);

	fSuccess = TEST_PASS;
	
CLEANUP:

	SAFE_RELEASE_ACCESSOR(pIAccessor, hAccessor);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(pTableColOrds);
	m_pIRowset = pISaveRowset;

	return fSuccess;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessorSequencing::Terminate()
{	
	CleanUpRowsetObject();
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCBookMarkRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCBookMarkRowset - Create accessors for rowsets with bookmarks
//|	Created:			11/14/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBookMarkRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		DBPROPSET	DBPropSet;
		DBPROP		Prop; 

		//Set IRowsetLocate properties to try to create rowset with bookmarks
		
		Prop.dwPropertyID = DBPROP_IRowsetLocate;
		Prop.dwOptions = DBPROPOPTIONS_REQUIRED;
		Prop.colid = DB_NULLID;
		Prop.vValue.vt = VT_BOOL;
		V_BOOL(&(Prop.vValue)) = VARIANT_TRUE;

		//Set up our property set for rowset properties								
		DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
		DBPropSet.rgProperties = &Prop;
		DBPropSet.cProperties = 1;

		SetRowsetProperties(&DBPropSet, 1);

		//Set m_pIAccessor on a 'select *' rowset with bookmarks
		m_hr=CreateRowsetObject(SELECT_VALIDATIONORDER);	
	
		if (SUCCEEDED(m_hr))
		{						
			//If we got an S code, it should be S_OK
			if (CHECK(m_hr, S_OK))
				//This is the only condition in which
				//the following testcases should be run
				return TRUE;
			
		}		
		//If we got any error, the only acceptable one 
		//is that IRowsetLocate is not supported
		else
		{			
			if (CHECK(m_hr, DB_E_ERRORSOCCURRED))
			{
				//Check the array which the provider was passed by our framework
				//to see what status the provider gave
				if (COMPARE(m_rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSUPPORTED))
				{
					odtLog << L"IRowsetLocate not supported by provider, this variation is not applicable.\n";
					return TEST_SKIPPED;
				}
			}
		}
	} 
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Bind bookmark column with ROWDATA
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCBookMarkRowset::Variation_1()
{
		
	BOOL fSuccess = FALSE;

	
	//We know we support IRowset since we got IRowsetLocate in Init
	m_fIRowset = TRUE;
	if (!VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset))
		return TEST_FAIL;

	//Bind all columns, including col 0
	if (CHECK(GetAccessorAndBindings(m_pIAccessor, 
		DBACCESSOR_ROWDATA,
		&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  		DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,		
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK))
	{
				
		//Verify the data is correctly brought back using the accessor 
		//(via either GetData or ReadData) - note this function also
		//validates that the bookmark retrieved is valid
			if (CHECK(UseRowAccessorAndVerify(m_hAccessor, m_cbRowSize, g_uiRowNum, 
					m_rgTableColOrds, m_cRowsetCols, m_rgBindings, m_cBindings),S_OK))				
				//Check accessor with GetBindings  
				fSuccess = VerifyBindings(m_pIAccessor, m_rgBindings, m_cBindings, m_hAccessor, 
					DBACCESSOR_ROWDATA);
	}

	//Clean up
	if (m_hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL),S_OK);

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	SAFE_RELEASE(m_pIRowset);

	if (fSuccess)
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
BOOL TCBookMarkRowset::Terminate()
{

	ReleaseRowsetObject();	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetBindingsRtnVals)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetBindingsRtnVals - Return values for all GetBindings error conditions
//|	Created:			11/24/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetBindingsRtnVals::Init()
{
	BOOL fSuccess = FALSE;
	HRESULT hr;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		//Set m_pIAccessor on a 'select *' rowset  We'll
		//use this interface ptr to do our tests.
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);	
	
		if (CHECK(hr,S_OK))
		{			
			//ReleaseRowsetObject();
			if(m_pICommand)
			{
				ReleaseRowsetObject();
				//Try to get IAccesor on Command
				if (VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,
					(IUnknown **)&m_pCmdIAccessor))
				{
					//Create an accessor on the command object
					if (CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_ROWDATA,
						&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
						DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
						ALL_COLS_BOUND,					
						FORWARD, NO_COLS_BY_REF, NULL, NULL,
						NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
						m_fBindLongCols),S_OK))
					{
						if (!CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER),S_OK))
							return TEST_FAIL;
					}
				}
			}
			//Create an accessor on the rowset object
			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				ALL_COLS_BOUND,					
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))
					fSuccess = TRUE;
		}
	
	}

	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	return fSuccess;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - Null pdwAccessorFlags
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetBindingsRtnVals::Variation_1()
{
	BOOL fSuccess = TRUE;
	m_cBindings = 8;
	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Try GetBindings on command object, if its supported
	if (m_pCmdIAccessor)
	{
		fSuccess = CHECK(m_pCmdIAccessor->GetBindings(m_hAccessor, NULL, 
			&m_cBindings, &m_rgBindings), E_INVALIDARG);
		//Parameters must be set to 0 and NULL on error
		fSuccess &= COMPARE(m_cBindings, 0);
		fSuccess &= COMPARE(m_rgBindings, NULL);
	}

	m_cBindings = 8;
	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Now do GetBindings on rowset object
	fSuccess &= CHECK(m_pIAccessor->GetBindings(m_hAccessor2, NULL, 
		&m_cBindings, &m_rgBindings), E_INVALIDARG);
	//Parameters must be set to 0 and NULL on error
	fSuccess &= COMPARE(m_cBindings, 0);
	fSuccess &= COMPARE(m_rgBindings, NULL);

	m_cBindings = 0;
	m_rgBindings = NULL;

	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - Null pcBindings
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetBindingsRtnVals::Variation_2()
{

	DBACCESSORFLAGS dwFlags = 1;
	BOOL fSuccess = TRUE;
	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Try GetBindings on command object, if its supported
	if (m_pCmdIAccessor)
	{
		fSuccess = CHECK(m_pCmdIAccessor->GetBindings(m_hAccessor, &dwFlags, 
			NULL, &m_rgBindings), E_INVALIDARG);
		//Parameters must be set to 0 and NULL on error
		fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
		fSuccess &= COMPARE(m_rgBindings, NULL);
	}

	//Reset to 1 so we are sure that method sets it to zero
	dwFlags = 1;

	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Now do GetBindings on rowset object
	fSuccess &= CHECK(m_pIAccessor->GetBindings(m_hAccessor2, &dwFlags,
		NULL, &m_rgBindings), E_INVALIDARG);
	//Parameters must be set to 0 and NULL on error
	fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
	fSuccess &= COMPARE(m_rgBindings, NULL);

	m_rgBindings = NULL;
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - Null prgBindings
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetBindingsRtnVals::Variation_3()
{
	DBACCESSORFLAGS dwFlags = 1;
	BOOL fSuccess = TRUE;
	m_cBindings = 8;


	//Try GetBindings on command object, if its supported
	if (m_pCmdIAccessor)
	{
		fSuccess = CHECK(m_pCmdIAccessor->GetBindings(m_hAccessor, &dwFlags, 
			&m_cBindings, NULL), E_INVALIDARG);
		//Parameters must be set to 0 and NULL on error
		fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
		fSuccess &= COMPARE(m_cBindings, 0);
	}

	//Reset to 1 so we are sure that method sets it to zero
	dwFlags = 1;
	m_cBindings = 8;

	//Now do GetBindings on rowset object
	fSuccess &= CHECK(m_pIAccessor->GetBindings(m_hAccessor2, &dwFlags,
		&m_cBindings, NULL), E_INVALIDARG);
	//Parameters must be set to 0 and NULL on error
	fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
	fSuccess &= COMPARE(m_cBindings, 0);

	m_cBindings = 0;

	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORHANDLE -  Null hAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetBindingsRtnVals::Variation_4()
{
	DBACCESSORFLAGS dwFlags = 1;
	BOOL fSuccess = TRUE;
	m_cBindings = 8;
	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Try GetBindings on command object, if its supported
	if (m_pCmdIAccessor)
	{
		fSuccess = CHECK(m_pCmdIAccessor->GetBindings(NULL, &dwFlags, 
			&m_cBindings, &m_rgBindings), DB_E_BADACCESSORHANDLE);
		//Parameters must be set to 0 and NULL on error
		fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
		fSuccess &= COMPARE(m_rgBindings, NULL);
		fSuccess &= COMPARE(m_cBindings, 0);
	}

	//Reset to 1 so we are sure that method sets it to zero
	dwFlags = 1;
	m_cBindings = 8;
	m_rgBindings = (DBBINDING *)&m_rgBindings;

	//Now do GetBindings on rowset object
	fSuccess &= CHECK(m_pIAccessor->GetBindings(NULL, &dwFlags,
		&m_cBindings, &m_rgBindings), DB_E_BADACCESSORHANDLE);
	//Parameters must be set to 0 and NULL on error
	fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
	fSuccess &= COMPARE(m_rgBindings, NULL);
	fSuccess &= COMPARE(m_cBindings, 0);

	m_cBindings = 0;
	m_rgBindings = NULL;
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetBindingsRtnVals::Variation_5()
{
	DBACCESSORFLAGS dwFlags = 1;
	BOOL fSuccess = TRUE;
	m_cBindings = 8;
	m_rgBindings = (DBBINDING *)&m_rgBindings;


	//Release the accessor
	CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL),S_OK);

	//Now do GetBindings on rowset object
	fSuccess &= CHECK(m_pIAccessor->GetBindings(m_hAccessor2, &dwFlags,
		&m_cBindings, &m_rgBindings), DB_E_BADACCESSORHANDLE);
	//Parameters must be set to 0 and NULL on error
	fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
	fSuccess &= COMPARE(m_rgBindings, NULL);
	fSuccess &= COMPARE(m_cBindings, 0);

	//Set to null so we don't release again in terminate
	m_hAccessor2 = DB_NULL_HACCESSOR;
	
	//Reset to 1 so we are sure that method sets it to zero
	dwFlags = 1;	
	
	//Try GetBindings on command object, if its supported
	if (m_pCmdIAccessor)
	{
		//First release rowset so we don't get an open object error
		ReleaseRowsetObject();

		//Release the accessor
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);

		m_cBindings = 8;
		m_rgBindings = (DBBINDING *)&m_rgBindings;

		fSuccess &= CHECK(m_pCmdIAccessor->GetBindings(m_hAccessor, &dwFlags, 
			&m_cBindings, &m_rgBindings), DB_E_BADACCESSORHANDLE);
		//Parameters must be set to 0 and NULL on error
		fSuccess &= COMPARE(dwFlags, DBACCESSOR_INVALID);
		fSuccess &= COMPARE(m_rgBindings, NULL);
		fSuccess &= COMPARE(m_cBindings, 0);

		//Set to null so we don't release again in terminate
		m_hAccessor = DB_NULL_HACCESSOR;
		//Set us up for next variation
		CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER),S_OK);
	}
	
	//Set us up for next variation
	m_cBindings = 0;
	m_rgBindings = NULL;

	if (fSuccess)
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
BOOL TCGetBindingsRtnVals::Terminate()
{
	
	if (m_hAccessor && m_pIAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL);
		m_hAccessor = DB_NULL_HACCESSOR;
	}
	if (m_hAccessor2 && m_pCmdIAccessor)
	{
		m_pCmdIAccessor->ReleaseAccessor(m_hAccessor2, NULL);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	SAFE_RELEASE(m_pCmdIAccessor);

	ReleaseRowsetObject();	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCReleaseAccessorRtnVals)
//*-----------------------------------------------------------------------
//| Test Case:		TCReleaseAccessorRtnVals - Return values for all ReleaseAccessor error conditions
//|	Created:			11/24/95
//*-----------------------------------------------------------------------
					
//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCReleaseAccessorRtnVals::Init()
{
	BOOL fSuccess = FALSE;
	HRESULT	hr;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		//Set m_pIAccessor on a 'select *' rowset  We'll
		//use this interface ptr to do our tests.
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);
	
		if (CHECK(hr,S_OK))
		{			
			//Get IAccesor on Command
			if(m_pICommand)
			{
				//Make sure we don't get Open Object error
				ReleaseRowsetObject();

				if (VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,
					(IUnknown **)&m_pCmdIAccessor))
				{
					//Create an accessor on the command object
					if (CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_ROWDATA,
						&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
						DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
						ALL_COLS_BOUND,					
						FORWARD, NO_COLS_BY_REF, NULL, NULL,
						NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
						m_fBindLongCols),S_OK))
					{
						//Now create rowset again
						if (!CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER),S_OK))
							return TEST_FAIL;
					}
				}
			}
			
			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				ALL_COLS_BOUND,					
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))
					fSuccess = TRUE;

		}
	
	}
	

	return fSuccess;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Release Rowset without releasing rowset Accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCReleaseAccessorRtnVals::Variation_1()
{
	BOOL fSuccess = FALSE;
	HRESULT hr;

	//Make sure releasing the rowset cleans up the accessor without dying
	ReleaseRowsetObject();

	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	//Create Rowset and accessor again for next vars
		if (CHECK(hr,S_OK))				
		{
			//Create an accessor on the rowset object
			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_hAccessor2, NULL, NULL, NULL,			
				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				ALL_COLS_BOUND,					
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))
				fSuccess = TRUE;
		}
	

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADACCESSORHANDLE - Previously released accessor for hAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCReleaseAccessorRtnVals::Variation_2()
{
	
	BOOL fSuccess = TRUE;

	
	//Do release twice on rowset object
	CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);	
	fSuccess = CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), 
		DB_E_BADACCESSORHANDLE);

	if(m_pCmdIAccessor)
	{
		//Now close rowset so we don't get DB_E_OPENOBJECT when we release cmd accessor
		ReleaseRowsetObject();

		//Release same accessor twice on command object
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		fSuccess &= CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), 
			DB_E_BADACCESSORHANDLE);

		//Now create command accessor again for other variations
		CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_ROWDATA,
		&m_hAccessor, NULL, NULL, NULL, DBPART_VALUE | 
		DBPART_LENGTH | DBPART_STATUS,
		ALL_COLS_BOUND,					
		FORWARD, NO_COLS_BY_REF, NULL, NULL,
		NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
		m_fBindLongCols),S_OK);
	}

	//Get rowset back for other variations
	CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER),S_OK);

	//Now create rowset accessor again for other variations
	CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
	&m_hAccessor2, NULL, NULL, NULL, DBPART_VALUE | 
	DBPART_LENGTH | DBPART_STATUS,
	ALL_COLS_BOUND,					
	FORWARD, NO_COLS_BY_REF, NULL, NULL,
	NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
	NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
	m_fBindLongCols),S_OK);


	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OPENOBJECT - Release command accessor while rowset is open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCReleaseAccessorRtnVals::Variation_3()
{
	
	HACCESSOR	hAccessor;
	
	//if command is not supported
	if(!m_pCmdIAccessor)
		return TEST_PASS;

	//Now create command accessor after rowset is already open
	CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
		m_cBindings, m_rgBindings, m_cbRowSize,
		&hAccessor, NULL),S_OK);
	
	//Try to release command accessor while rowest is open,
	//This should work because the accessor was created after
	//the rowset was opened.  Note, in TCAccessorSequencing
	//we test releasing accessor created before rowset was opened.
	if (CHECK(m_pCmdIAccessor->ReleaseAccessor(hAccessor, NULL), 
		S_OK))
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
BOOL TCReleaseAccessorRtnVals::Terminate()
{
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);

	if (m_hAccessor2 && m_pIAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor2, NULL), S_OK);
		m_hAccessor2 = DB_NULL_HACCESSOR;
	}

	if (m_hAccessor && m_pCmdIAccessor)
	{
		CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		m_hAccessor = DB_NULL_HACCESSOR;
	}

	CleanUpRowsetObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCDeferredColumns)
//*-----------------------------------------------------------------------
//| Test Case:		TCDeferredColumns - Use of Deferred and CacheDeferred properties
//|	Created:			11/28/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc Copies test case info from this class to encapsulated CRowset
//		  
// @rdesc TRUE or FALSE
//
void TCDeferredColumns::CopyTestCaseInfo(CTestCases * pTC)
{
	pTC->SetOwningMod(0, this->m_pThisTestModule);
}

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDeferredColumns::Init()
{
	BOOL			fSuccess = FALSE;
	IRowsetInfo *	pIRowsetInfo = NULL;						
	DBPROPIDSET		PropIDSet;
	const ULONG		cPropertyIDs = 2;
	DBPROPID		rgPropIDs[cPropertyIDs];
	ULONG			cPropSets = 0;
	DBPROPSET *		rgPropSets = NULL;
	IColumnsInfo *	pIColInfo = NULL;
	DBORDINAL		cColumns = 0;
	DBCOLUMNINFO *	rgColInfo = NULL;
	WCHAR *			pStr =	NULL;
	const ULONG		cProperties = 2;
	DBPROP			rgProps[cProperties];
	DBPROPSET		DBPropSet;
	HRESULT			hr;	

	m_pSetRowset = new CSetRowsetObject((LPWSTR)gwszModuleName, m_pThisTestModule);
	if (!m_pSetRowset)
		return FALSE;

	m_GetAllAccessor = DB_NULL_HACCESSOR;
	m_SetAllAccessor = DB_NULL_HACCESSOR;
	m_VariableAccessor = DB_NULL_HACCESSOR;
	m_FixedAccessor = DB_NULL_HACCESSOR;
	m_rgFixedBindings = NULL;
	m_rgVariableBindings = NULL;
	m_rgGetAllBindings = NULL;
	m_rgSetAllBindings = NULL;
	m_pGetIRowset = NULL;	
	m_pSetIRowset = NULL;
	m_pSetIRowsetChange = NULL;	
	m_rgDBIDs = NULL;

	//Set up structures for GetProperties
	PropIDSet.rgPropertyIDs = rgPropIDs;
	PropIDSet.cPropertyIDs = cPropertyIDs;
	PropIDSet.guidPropertySet = DBPROPSET_ROWSET;
	rgPropIDs[0] = DBPROP_DEFERRED;
	rgPropIDs[1] = DBPROP_CACHEDEFERRED;	

	//Bring our encapsulated object up to speed on what 
	//test case info we currently have
//	CopyTestCaseInfo(m_pSetRowset);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{		
		
		//Set up a Property for SetData
		//We'll use this part for both rowsets
		rgProps[0].dwPropertyID = DBPROP_UPDATABILITY;
		rgProps[0].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgProps[0].colid = DB_NULLID;
		rgProps[0].vValue.vt = VT_I4;
		rgProps[0].vValue.lVal = DBPROPVAL_UP_CHANGE;
		
		rgProps[1].dwPropertyID = DBPROP_IRowsetChange;
		rgProps[1].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgProps[1].colid = DB_NULLID;
		rgProps[1].vValue.vt = VT_BOOL;
		V_BOOL(&(rgProps[1].vValue)) = VARIANT_TRUE;
		
		DBPropSet.guidPropertySet = DBPROPSET_ROWSET;
		DBPropSet.cProperties = cProperties;
		DBPropSet.rgProperties = rgProps;		

		//Set the properties for our rowset we'll test
		if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE) &
			SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE))
			SetRowsetProperties(&DBPropSet, 1);

		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		if (SUCCEEDED(hr))
			hr = VerifyRowsetProperties((IUnknown *)m_pIAccessor, &DBPropSet, 1);

		if (!SUCCEEDED(hr))
		{
			odtLog << L"Couldn't create the rowset as requested.\n";
			fSuccess = TEST_SKIPPED;
			goto CLEANUP;
		}

		//Get rowset interface off of our main rowset object
		if (!VerifyInterface(m_pIAccessor,IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pGetIRowset))
			goto CLEANUP;

		//Find out what deferred and cache deferred behavior is supported
		if (!VerifyInterface(m_pIAccessor,IID_IRowsetInfo,ROWSET_INTERFACE,(IUnknown **)&pIRowsetInfo))
			goto CLEANUP;

		//We may get errors occured from not supported properties
		m_hr = pIRowsetInfo->GetProperties(1, &PropIDSet, &cPropSets, &rgPropSets);
		if (m_hr == DB_E_ERRORSOCCURRED)
		{
			// Neither of the properties were supported, we can't continue

			//Make sure that the error we get sets the right status
			//Anything other than NOTSUPPORTED is a bug.  If we 
			//don't have support, we'll just fail and not do this variation.
			COMPARE(rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
			odtLog << L"Required properties not supported, test case skipped.\n";
			fSuccess=TEST_SKIPPED;
			goto CLEANUP;		
		}


		//Any other error is just that, an error!
		if (m_hr != DB_S_ERRORSOCCURRED)
			if (!CHECK(m_hr, S_OK))			
				goto CLEANUP;
		
		//Set our flags so we know what is supported
		if (rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_OK &&
			V_BOOL(&rgPropSets[0].rgProperties[0].vValue) == VARIANT_TRUE)
			m_fDeferredSupported = TRUE;
		else
			m_fDeferredSupported = FALSE;

		if (rgPropSets[0].rgProperties[1].dwStatus == DBPROPSTATUS_OK &&
			V_BOOL(&rgPropSets[0].rgProperties[1].vValue) == VARIANT_TRUE)
			m_fCacheSupported = TRUE;
		else
			m_fCacheSupported = FALSE;
		
		//Use DB Session that we also used for current rowset object
		//for our SetDataRowset
		m_pSetRowset->SetDBSession(m_pThisTestModule->m_pIUnknown2);

		//Have this testcase use the table created in ModuleInit, but don't
		//let table be deleted, since we use it for the other rowset
		m_pSetRowset->SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

		//Ask for SetData on the rowset we'll use to change data
		if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE) &
			SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown *)m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE))
			m_pSetRowset->SetRowsetProperties(&DBPropSet, 1);
		
		//Create the new rowset and place it in m_pSetIRowset
		hr=m_pSetRowset->CreateRowsetObject(SELECT_VALIDATIONORDER);

		if (SUCCEEDED(hr))
			hr = VerifyRowsetProperties((IUnknown *)m_pSetRowset->m_pIAccessor, &DBPropSet, 1);

		if (FAILED(hr))
			goto CLEANUP;

		//If we've gotten this far, we know IRowset is supported, so expect it as an interface
		if (!VerifyInterface(m_pSetRowset->m_pIAccessor,IID_IRowset,ROWSET_INTERFACE,(IUnknown **)&m_pSetIRowset))
			goto CLEANUP;				

		//Get rowsetchange interface off of our SetData rowset object, if this 
		//isn't supported, we are done since we can't test anything without SetData
		if (!VerifyInterface(m_pSetIRowset,IID_IRowsetChange,ROWSET_INTERFACE,
			(IUnknown **)&m_pSetIRowsetChange))		
		{
			//Return but don't increment error count, since we can't 
			//continue without IRowsetChange support			
			fSuccess = FALSE;
			goto CLEANUP;
		}

  
		//Build a colid array based on our rowset
		if (!VerifyInterface(m_pIAccessor,IID_IColumnsInfo,ROWSET_INTERFACE,(IUnknown **)&pIColInfo))
			goto CLEANUP;
		if (!CHECK(pIColInfo->GetColumnInfo(&cColumns, &rgColInfo, &pStr), S_OK))
			goto CLEANUP;
		m_rgDBIDs = (DBID *)m_pIMalloc->Alloc((cColumns) * sizeof(DBID));
		while (cColumns)
		{
			//NOTE:  The index to this array will equal the column number
			//associated with that colid minus one.  It is assumed there 
			//are no bookmarks on this rowset
			cColumns --;
			m_rgDBIDs[cColumns] = rgColInfo[cColumns].columnid;				
		}


		//Get the three accessors we'll be testing with
		if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
			&m_FixedAccessor, &m_rgFixedBindings, &m_cFixedBindings, &m_cbFixedRowSize,			
  			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			FIXED_LEN_COLS_BOUND,							
			FORWARD, NO_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongCols),S_OK))
			goto CLEANUP;

		if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
			&m_VariableAccessor, &m_rgVariableBindings, &m_cVariableBindings, &m_cbVariableRowSize,			
  			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			VARIABLE_LEN_COLS_BOUND,								
			FORWARD, NO_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongCols),S_OK))
			goto CLEANUP;
		
		if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
			&m_GetAllAccessor, &m_rgGetAllBindings, &m_cGetAllBindings, &m_cbGetAllRowSize,			
  			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
			ALL_COLS_BOUND,			
			FORWARD, NO_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongCols),S_OK))
			goto CLEANUP;

		if (!CHECK(GetAccessorAndBindings(m_pSetRowset->m_pIAccessor, DBACCESSOR_ROWDATA,
			&m_SetAllAccessor, &m_rgSetAllBindings, &m_cSetAllBindings, &m_cbSetAllRowSize,			
  			DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE, UPDATEABLE_COLS_BOUND,										
			FORWARD, NO_COLS_BY_REF, NULL, NULL,
			NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
			m_fBindLongCols),S_OK))
			goto CLEANUP;

		//If we got this far everything succeeded
		fSuccess = TRUE;
	}

CLEANUP: 

	FreeProperties(&cPropSets, &rgPropSets);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIColInfo);
	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(pStr);

	return fSuccess;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Deferred on, CacheDeferred off - All Columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDeferredColumns::Variation_1()
{
	BOOL fSuccess = FALSE;
	HROW	rghRow[1] = {NULL};
	HROW * phRow = &rghRow[0];
	DBCOUNTITEM cRowsObtained = 0;
	BYTE * pGetData1 = NULL;
	BYTE * pGetData2 = NULL;

	//Skip this variation if the right properties are not supported
	if (!m_fDeferredSupported)
	{
		odtLog << wszDeferredNotSupported;
		return TEST_PASS;
	}

	//Make sure we start with Primary data for our one row table
	if (!CHECK(SetData(PRIMARY), S_OK))
		goto CLEANUP;

	//Set up the properties for this variation: deferred on, cachedeferred off
	if (!CHECK(SetDeferredProperties(TRUE, FALSE, 0, NULL), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
		goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now get some of the columns with one call, some with the next call to GetData
	pGetData1 = (BYTE *)m_pIMalloc->Alloc(m_cbVariableRowSize);
	if (!pGetData1)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_VariableAccessor, pGetData1), S_OK))
		goto CLEANUP;

	pGetData2 = (BYTE *)m_pIMalloc->Alloc(m_cbFixedRowSize);
	if (!pGetData2)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_FixedAccessor, pGetData2), S_OK))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData1, m_cVariableBindings, 
		m_rgVariableBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData2, m_cFixedBindings, 
		m_rgFixedBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;

	//We got this far, so we have succeeded
	fSuccess = TRUE;


CLEANUP:
	 if (rghRow[0])
		 CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL),S_OK);
	 PROVIDER_FREE(pGetData1);
	 PROVIDER_FREE(pGetData2);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Deferred on, CacheDeferred on - Fixed Columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDeferredColumns::Variation_2()
{
	BOOL fSuccess = FALSE;
	HROW	rghRow[1] = {NULL};
	HROW * phRow = &rghRow[0];
	DBCOUNTITEM	cRowsObtained = 0;
	BYTE * pGetData1 = NULL;	

	//Skip this variation if the right properties are not supported
	if (!m_fDeferredSupported)
	{
		odtLog << wszDeferredNotSupported;
		return TEST_SKIPPED;
	}
	if (!m_fCacheSupported)
	{
		odtLog << wszCacheDeferredNotSupported;
		return TEST_SKIPPED;
	}

	//Make sure we start with Primary data for our one row table
	if (!CHECK(SetData(PRIMARY), S_OK))
		goto CLEANUP;

	//Set up the properties for this variation: deferred on, cachedeferred off
	if (!CHECK(SetDeferredProperties(TRUE, TRUE, m_cFixedBindings, 
		m_rgFixedBindings), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
		goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now get all of the columns, so the deferred ones are cached
	pGetData1 = (BYTE *)m_pIMalloc->Alloc(m_cbGetAllRowSize);
	if (!pGetData1)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	//Now change the data while the first rowset is open
	if (!CHECK(SetData(SECONDARY), S_OK))
		goto CLEANUP;
	
	//Release the row and refetch to make sure we have to reaccess the cached values
	if (!CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL), S_OK))
		goto CLEANUP;
	rghRow[0] = DB_NULL_HROW;
	//Could get DB_S_COMMANDREEXECUTED here
	if (FAILED(m_pGetIRowset->RestartPosition(NULL)))
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
			goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now Get the data and make sure it is using the cached, not the new values		
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_FixedAccessor, pGetData1), S_OK))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData1, m_cFixedBindings, 
		m_rgFixedBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;
	
	//We got this far, so we have succeeded
	fSuccess = TRUE;
	
CLEANUP:
	 if (rghRow[0])
		 CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL),S_OK);
	 PROVIDER_FREE(pGetData1);
	 
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Deferred on, CacheDeferred on - Variable Columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDeferredColumns::Variation_3()
{
	BOOL fSuccess = FALSE;
	HROW	rghRow[1] = {NULL};
	HROW * phRow = &rghRow[0];
	DBCOUNTITEM	cRowsObtained = 0;
	BYTE * pGetData1 = NULL;	

	//Skip this variation if the right properties are not supported
	if (!m_fDeferredSupported)
	{
		odtLog << wszDeferredNotSupported;
		return TEST_SKIPPED;
	}
	if (!m_fCacheSupported)
	{
		odtLog << wszCacheDeferredNotSupported;
		return TEST_SKIPPED;
	}
	//Make sure we start with Primary data for our one row table
	if (!CHECK(SetData(PRIMARY), S_OK))
		goto CLEANUP;

	//Set up the properties for this variation: deferred on, cachedeferred on
	if (!CHECK(SetDeferredProperties(TRUE, TRUE, m_cVariableBindings, 
		m_rgVariableBindings), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
		goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now get all of the columns, so the deferred ones are cached
	pGetData1 = (BYTE *)m_pIMalloc->Alloc(m_cbGetAllRowSize);
	if (!pGetData1)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	//Now change the data while the first rowset is open
	if (!CHECK(SetData(SECONDARY), S_OK))
		goto CLEANUP;

	//Release the row and refetch to make sure we have to reaccess the cached values
	if (!CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL), S_OK))
		goto CLEANUP;
	rghRow[0] = DB_NULL_HROW;
	//Could get DB_S_COMMANDREEXECUTED here
	if (FAILED(m_pGetIRowset->RestartPosition(NULL)))
		goto CLEANUP;	
	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
			goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	
	//Now Get the data and make sure it is using the cached, not the new values		
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_VariableAccessor, pGetData1), S_OK))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData1, m_cVariableBindings, 
		m_rgVariableBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;
	
	//We got this far, so we have succeeded
	fSuccess = TRUE;
	
CLEANUP:
	 if (rghRow[0])
		 CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL),S_OK);
	 PROVIDER_FREE(pGetData1);
	 
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Deferred on, CacheDeferred on - All Columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDeferredColumns::Variation_4()
{
	BOOL fSuccess = FALSE;
	HROW	rghRow[1] = {NULL};
	HROW * phRow = &rghRow[0];
	DBCOUNTITEM	cRowsObtained = 0;
	BYTE * pGetData1 = NULL;	

	//Skip this variation if the right properties are not supported
	if (!m_fDeferredSupported)
	{
		odtLog << wszDeferredNotSupported;
		return TEST_SKIPPED;
	}
	if (!m_fCacheSupported)
	{
		odtLog << wszCacheDeferredNotSupported;
		return TEST_SKIPPED;
	}
	//Make sure we start with Primary data for our one row table
	if (!CHECK(SetData(PRIMARY), S_OK))
		goto CLEANUP;

	//Set up the properties for this variation: deferred on, cachedeferred on
	if (!CHECK(SetDeferredProperties(TRUE, TRUE, m_cGetAllBindings, 
		m_rgGetAllBindings), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
		goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now get all of the columns, so the deferred ones are cached
	pGetData1 = (BYTE *)m_pIMalloc->Alloc(m_cbGetAllRowSize);
	if (!pGetData1)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	//Now change the data while the first rowset is open
	if (!CHECK(SetData(SECONDARY), S_OK))
		goto CLEANUP;

	//Release the row and refetch to make sure we have to reaccess the cached values
	if (!CHECK(m_pGetIRowset->ReleaseRows(1, rghRow,  NULL, NULL, NULL), S_OK))
		goto CLEANUP;
	rghRow[0] = DB_NULL_HROW;
	//Could get DB_S_COMMANDREEXECUTED here
	if (FAILED(m_pGetIRowset->RestartPosition(NULL)))
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
			goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now Get the data and make sure it is using the cached, not the new values		
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData1, m_cGetAllBindings, 
		m_rgGetAllBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;
	
	//We got this far, so we have succeeded
	fSuccess = TRUE;
	
CLEANUP:
	 if (rghRow[0])
		 CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL),S_OK);
	 PROVIDER_FREE(pGetData1);
	 
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Deferred off, CacheDeferred on - All Columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDeferredColumns::Variation_5()
{
	BOOL fSuccess = FALSE;
	HROW	rghRow[1] = {NULL};
	HROW * phRow = &rghRow[0];
	DBCOUNTITEM	cRowsObtained = 0;
	BYTE * pGetData1 = NULL;	

	//Skip this variation if the right properties are not supported
	if (m_fDeferredSupported)
	{
		odtLog << wszDeferredSupported;
		return TEST_SKIPPED;
	}
	if (!m_fCacheSupported)
	{
		odtLog << wszCacheDeferredNotSupported;
		return TEST_SKIPPED;
	}
	//Make sure we start with Primary data for our one row table
	if (!CHECK(SetData(PRIMARY), S_OK))
		goto CLEANUP;

	//Set up the properties for this variation: deferred off, cachedeferred on
	//This should be identical to deferred on, since cachedeferred implies deferred.
	if (!CHECK(SetDeferredProperties(FALSE, TRUE, m_cGetAllBindings, 
		m_rgGetAllBindings), S_OK))
		goto CLEANUP;

	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
		goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now get all of the columns, so the deferred ones are cached
	pGetData1 = (BYTE *)m_pIMalloc->Alloc(m_cbGetAllRowSize);
	if (!pGetData1)
		goto CLEANUP;
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	//Now change the data while the first rowset is open
	if (!CHECK(SetData(SECONDARY), S_OK))
		goto CLEANUP;
	
	//Release the row and refetch to make sure we have to reaccess the cached values
	if (!CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL), S_OK))
		goto CLEANUP;
	rghRow[0] = DB_NULL_HROW;
	//Could get DB_S_COMMANDREEXECUTED here
	if (FAILED(m_pGetIRowset->RestartPosition(NULL)))
		goto CLEANUP;	
	if (!CHECK(m_pGetIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow),S_OK))
			goto CLEANUP;
	COMPARE(cRowsObtained, 1);

	//Now Get the data and make sure it is using the cached, not the new values		
	if (!CHECK(m_pGetIRowset->GetData(rghRow[0], m_GetAllAccessor, pGetData1), S_OK))
		goto CLEANUP;

	if (!CompareData(m_cRowsetCols, m_rgTableColOrds, g_uiRowNum, pGetData1, m_cGetAllBindings, 
		m_rgGetAllBindings, m_pTable, m_pIMalloc, PRIMARY))
		goto CLEANUP;
	
	//We got this far, so we have succeeded
	fSuccess = TRUE;
	
CLEANUP:
	 if (rghRow[0])
		 CHECK(m_pGetIRowset->ReleaseRows(1, rghRow, NULL,  NULL, NULL),S_OK);
	 PROVIDER_FREE(pGetData1);
	 
	if (fSuccess)
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
BOOL TCDeferredColumns::Terminate()
{
	
	//Release accessors
	if (m_FixedAccessor)		
	{
		m_pIAccessor->ReleaseAccessor(m_FixedAccessor, NULL);
		m_FixedAccessor = DB_NULL_HACCESSOR;
	}
	if (m_VariableAccessor)
	{
		m_pIAccessor->ReleaseAccessor(m_VariableAccessor, NULL);
		m_VariableAccessor = DB_NULL_HACCESSOR;
	}


	if (m_GetAllAccessor != DB_NULL_HACCESSOR)
	{
		m_pIAccessor->ReleaseAccessor(m_GetAllAccessor, NULL);
		m_GetAllAccessor = DB_NULL_HACCESSOR;
	}

	if (m_SetAllAccessor != DB_NULL_HACCESSOR)
	{
		m_pSetRowset->m_pIAccessor->ReleaseAccessor(m_SetAllAccessor, NULL);
		m_SetAllAccessor = DB_NULL_HACCESSOR;
	}
	
	//Release memory associated with binding arrays
	PROVIDER_FREE(m_rgFixedBindings);
	PROVIDER_FREE(m_rgVariableBindings);
	PROVIDER_FREE(m_rgGetAllBindings);
	PROVIDER_FREE(m_rgSetAllBindings);

	SAFE_RELEASE(m_pGetIRowset);
	SAFE_RELEASE(m_pSetIRowset);
	SAFE_RELEASE(m_pSetIRowsetChange);

	//We don't need the whole CRowset object anymore, we have an interface to the rowset
	if (m_pSetRowset)
	{
		m_pSetRowset->ReleaseRowsetObject();
		m_pSetRowset->ReleaseCommandObject();
		m_pSetRowset->ReleaseDBSession();
		delete m_pSetRowset;
	}
		
	PROVIDER_FREE(m_rgDBIDs);

	ReleaseRowsetObject();	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCommandAccessorTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCCommandAccessorTransactions - Commit/Abort behavior for Command Accessors
//|	Created:			12/18/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommandAccessorTransactions::Init()
{
	//if command is not supported, skip this test 
	if(!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
	 
		//Set up a simple binding to use
		m_cBindings = 1;
		m_rgBindings[0].dwPart = DBPART_VALUE;
		m_rgBindings[0].eParamIO = DBPARAMIO_INPUT;
		m_rgBindings[0].iOrdinal = 1;
		m_rgBindings[0].dwFlags = 0;
		m_rgBindings[0].wType = DBTYPE_STR;
		m_rgBindings[0].pTypeInfo = NULL;
		m_rgBindings[0].pObject = NULL;
		m_rgBindings[0].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		m_rgBindings[0].bPrecision = 0;
		m_rgBindings[0].bScale = 0;
		m_rgBindings[0].pBindExt = NULL;
		m_rgBindings[0].obValue = 0;
		m_rgBindings[0].cbMaxLen = MAX_COL_SIZE;
		m_rgBindings[0].obLength = 0;
		m_rgBindings[0].obStatus = 0;

		//This is a mandatory interface, it should always succeed
		return COMPARE(RegisterInterface(COMMAND_INTERFACE, IID_IAccessor, 0, NULL), TRUE);
					
	}
	return TEST_SKIPPED;
}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandAccessorTransactions::TestTxn(ETXN	eTxn, BOOL	fRetaining)
{
	
	BOOL		fSuccess = FALSE;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;	
	IAccessor *	pCmdIAccessor = NULL;
	

	if (!StartTransaction(SELECT_VALIDATIONORDER, (IUnknown **)&pCmdIAccessor, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED))
		goto CLEANUP;
		
	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}
		
	//Make sure everything still works after commit or abort
	if (CHECK(pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
			m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK))
	{
		if (CHECK(pCmdIAccessor->GetBindings(hAccessor, &m_dwFlags, 
			&m_cBindings, &m_rgGetBindings), S_OK))
		{
			PROVIDER_FREE(m_rgGetBindings);
			fSuccess = CHECK(pCmdIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);				
			hAccessor = DB_NULL_HACCESSOR;
		}
	}

CLEANUP:

	if (pCmdIAccessor)
	{
		if (hAccessor != DB_NULL_HACCESSOR)
			CHECK(pCmdIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
		
		SAFE_RELEASE(pCmdIAccessor);
	}

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
	

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandAccessorTransactions::Variation_1()
{	
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandAccessorTransactions::Variation_2()
{
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandAccessorTransactions::Variation_3()
{
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandAccessorTransactions::Variation_4()
{
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommandAccessorTransactions::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2	
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRowsetAccessorTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetAccessorTransactions - Commit/Abort behavior for Rowset Accessors
//|	Created:			12/18/95
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetAccessorTransactions::Init()
{
	//if command is not supported, skip this test 
	if(!g_fCmdSupported)
	{
		odtLog << wszCommandNotSupported;
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		//Set up a simple binding to use
		m_cBindings = 1;
		m_rgBindings[0].dwPart = DBPART_VALUE;		
		m_rgBindings[0].eParamIO = DBPARAMIO_NOTPARAM;
		m_rgBindings[0].iOrdinal = 1;
		m_rgBindings[0].dwFlags = 0;
		m_rgBindings[0].wType = DBTYPE_STR;
		m_rgBindings[0].pTypeInfo = NULL;
		m_rgBindings[0].pObject = NULL;
		m_rgBindings[0].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		m_rgBindings[0].bPrecision = 0;
		m_rgBindings[0].bScale = 0;
		m_rgBindings[0].pBindExt = NULL;
		m_rgBindings[0].obValue = 0;
		m_rgBindings[0].cbMaxLen = MAX_COL_SIZE;
		m_rgBindings[0].obLength = 0;
		m_rgBindings[0].obStatus = 0;

		//This is a mandatory interface, it should always succeed
		return COMPARE(RegisterInterface(ROWSET_INTERFACE, IID_IAccessor, 0, NULL), TRUE);		
	}
	return TEST_SKIPPED;
}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort preservation with respect to IAccessor on rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetAccessorTransactions::TestTxn(ETXN	eTxn, BOOL	fRetaining)
{
	
	BOOL		fSuccess = FALSE;
	HACCESSOR	hAccessor1 = DB_NULL_HACCESSOR;	
	HACCESSOR	hAccessor2 = DB_NULL_HACCESSOR;	
	IAccessor *	pIAccessor = NULL;
	

	if (!StartTransaction(SELECT_VALIDATIONORDER, (IUnknown **)&pIAccessor, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED))
		goto CLEANUP;

	//Get an accessor which we can try to release when zombied
	if (!CHECK(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
				m_rgBindings, m_cbRowSize, &hAccessor1, NULL), S_OK))
		goto CLEANUP;		

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}
	
	
	//Make sure everything still works after commit or abort
	if((eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
		(eTxn == ETXN_ABORT && m_fAbortPreserve))
	{	
		if (CHECK(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
				m_rgBindings, m_cbRowSize, &hAccessor2, NULL), S_OK))
		{
			if (CHECK(pIAccessor->GetBindings(hAccessor2, &m_dwFlags, 
				&m_cBindings, &m_rgGetBindings), S_OK))
			{
				PROVIDER_FREE(m_rgGetBindings);
				fSuccess = CHECK(pIAccessor->ReleaseAccessor(hAccessor2, NULL), S_OK);				
				hAccessor2 = DB_NULL_HACCESSOR;
			}
		}

	}
	//Make sure we are zomibified, and can do nothing but release accessor
	else
	{
		if (CHECK(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,
				m_rgBindings, m_cbRowSize, &hAccessor2, NULL), E_UNEXPECTED))
		{
			if (CHECK(pIAccessor->GetBindings(hAccessor2, &m_dwFlags, 
				&m_cBindings, &m_rgGetBindings), E_UNEXPECTED))
			{
				PROVIDER_FREE(m_rgGetBindings);
				//Should still be able to release accessors gotten before zombie
				fSuccess = CHECK(pIAccessor->ReleaseAccessor(hAccessor1, NULL), S_OK);				
				hAccessor1 = DB_NULL_HACCESSOR;
			}
		}

	}
CLEANUP:

	if (pIAccessor)
	{
		if (hAccessor1 != DB_NULL_HACCESSOR)
			CHECK(pIAccessor->ReleaseAccessor(hAccessor1, NULL), S_OK);
		
		if (hAccessor2 != DB_NULL_HACCESSOR)
			CHECK(pIAccessor->ReleaseAccessor(hAccessor2, NULL), S_OK);
		
		SAFE_RELEASE(pIAccessor);
	}

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
	

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetAccessorTransactions::Variation_1()
{
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetAccessorTransactions::Variation_2()
{
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetAccessorTransactions::Variation_3()
{
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetAccessorTransactions::Variation_4()
{
	return TestTxn(ETXN_ABORT, FALSE);

}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetAccessorTransactions::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2	
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			03/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{
	HRESULT hr;

// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{		
		//Set m_pIAccessor on a 'select *' rowset  We'll
		//use this interface ptr to do our tests.
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		if (CHECK(hr,S_OK))
		{			
			
			//Get IAccesor on Command
			//if (VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,
			//	(IUnknown **)&m_pCmdIAccessor))
			//{
				//Create an accessor just to build our bindings
				if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				//if (CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_ROWDATA,
					&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
					DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
					ALL_COLS_BOUND,							
					FORWARD, NO_COLS_BY_REF, NULL, NULL,
					NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
					NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
					m_fBindLongCols),S_OK))
				{

					//Now release the accessor as we don't need it
					CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);			
					//CHECK(m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);			
					m_hAccessor = DB_NULL_HACCESSOR;
					return TRUE;
				}
			//}						
		}
	
	}		
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IAccessor calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	
	BOOL	fResults = FALSE;
	DBACCESSORFLAGS	dwFlags = 0;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IAccessor method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	m_pExtError->CauseError();
	if (CHECK(m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
		m_rgBindings, 0, &m_hAccessor, NULL), S_OK))
	{
		//Do extended check following CreateAccessor
		fResults = XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		
		m_pExtError->CauseError();
		if (CHECK(m_hr = m_pIAccessor->GetBindings(m_hAccessor, &dwFlags, &m_cBindings2,
			&m_rgBindings2), S_OK))
		{
			//Do extended check following GetBindings
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}

		m_pExtError->CauseError();
		if (CHECK(m_hr = m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK))
		{

			//Do extended check following ReleaseAccessor
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}
		else
			fResults = FALSE;

		m_hAccessor = DB_NULL_HACCESSOR;

	}

	
	PROVIDER_FREE(m_rgBindings2);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IAccessor calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL	fResults = FALSE;
	DBACCESSORFLAGS	dwFlags = 0;
	HACCESSOR hAccessor;

	 
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IAccessor method.
	//We then check extended errors to verify the right extended error behavior.
	
	//Set first binding to invalid dwPart
	m_rgBindings[0].dwPart = 0;
	
	m_pExtError->CauseError();

	//if (CHECK(m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
	//	m_rgBindings, 0, &hAccessor, NULL), DB_E_BADBINDINFO))

	if (CHECK(m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
		m_rgBindings, 0, &hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
	{
		fResults = COMPARE(m_rgStatus[0], DBBINDSTATUS_BADBINDINFO);
	    
		//Do extended check following CreateAccessor
		fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		
		//NOTE:  We released m_hAccessor in the Init, so
		//the handle should be invalid

		m_pExtError->CauseError();
		//if (CHECK(m_hr = m_pCmdIAccessor->GetBindings(m_hAccessor, &dwFlags, &m_cBindings2,
		if (CHECK(m_hr = m_pIAccessor->GetBindings(m_hAccessor, &dwFlags, &m_cBindings2,
			&m_rgBindings2), DB_E_BADACCESSORHANDLE))
		{
			//Do extended check following GetBindings
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}

		m_pExtError->CauseError();
		//if (CHECK(m_hr = m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), DB_E_BADACCESSORHANDLE))
		if (CHECK(m_hr = m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), DB_E_BADACCESSORHANDLE))
		{
			//Do extended check following ReleaseAccessor
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}
		else
			fResults = FALSE;

		m_hAccessor = DB_NULL_HACCESSOR;
	}

	
	PROVIDER_FREE(m_rgBindings2);

	//Reset dwPart to valid value
	m_rgBindings[0].dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IAccessor calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	BOOL	fResults = FALSE;
	DBACCESSORFLAGS	dwFlags = 0;
	HACCESSOR	hAccessor;
	DBTYPE	dbType;

   
	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IAccessor method.
	//We then check extended errors to verify the right extended error behavior.

	//Remember valid value for first binding's wType
	dbType = m_rgBindings[0].wType;
	
	//Set first binding wType to DBTYPE_EMPTY 
	m_rgBindings[0].wType = DBTYPE_EMPTY;

	//	if (CHECK(m_hr = m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
	//      m_rgBindings, 0, &hAccessor, NULL), DB_E_BADBINDINFO))

	if (CHECK(m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
		m_rgBindings, 0, &hAccessor, m_rgStatus), DB_E_ERRORSOCCURRED))
	{
		fResults = COMPARE(m_rgStatus[0], DBBINDSTATUS_BADBINDINFO);

		//Do extended check following CreateAccessor
		fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		
		//NOTE:  We released m_hAccessor in the Init, so
		//the handle should be invalid

		//if (CHECK(m_hr = m_pCmdIAccessor->GetBindings(m_hAccessor, &dwFlags, &m_cBindings2,
		if (CHECK(m_hr = m_pIAccessor->GetBindings(m_hAccessor, &dwFlags, &m_cBindings2,
			&m_rgBindings2), DB_E_BADACCESSORHANDLE))
		{
			//Do extended check following GetBindings
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}

		//if (CHECK(m_hr = m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL), DB_E_BADACCESSORHANDLE))
		if (CHECK(m_hr = m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), DB_E_BADACCESSORHANDLE))
		{
			//Do extended check following ReleaseAccessor
			fResults &= XCHECK(m_pIAccessor, IID_IAccessor, m_hr);	
		}
		else
			fResults = FALSE;

		m_hAccessor = DB_NULL_HACCESSOR;
	}

	
	PROVIDER_FREE(m_rgBindings2);
	
	//Reset wType to previous value
	m_rgBindings[0].wType = dbType;
	
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
BOOL TCExtendedErrors::Terminate()
{
	
	SAFE_RELEASE(m_pIAccessor);

	//Release everything we did after CAccessor::Init	
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	ReleaseRowsetObject();	
	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCrtRtnValsBeforeGetRows)
//*-----------------------------------------------------------------------
//| Test Case:		TCCrtRtnValsBeforeGetRows - Return values for all CreateAccessor error conditions
//|	Created:			03/27/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCrtRtnValsBeforeGetRows::Init()
{
	IColumnsInfo * pIColInfo = NULL;
	ULONG	cColumns = 0;
	DBCOLUMNINFO * rgInfo = NULL;
	WCHAR * pStrBuffer = NULL;
	HRESULT hr;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		//Set m_pIAccessor on a 'select *' rowset  We'll
		//use this interface ptr to do our tests.
		if(g_fCmdSupported)
			hr=CreateRowsetObject(SELECT_VALIDATIONORDER);
		else
		{
			// Since deferred validation is only mandatory for accessors created from
			// a command object then without commands we can't test.
			odtLog << L"Commands not supported, can't test deferred validation.\n";
			return TEST_SKIPPED;
		}

		if (CHECK(hr,S_OK))
		{						
			//Fill in m_rgBindings, m_cBindings and m_cbRowSize with valid values
			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
  				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				ALL_COLS_BOUND,							
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))
			{
				//We don't need this accessor, we just wanted to generate
				//bindings to be used in an attempt to create other accessors				
				CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
				m_hAccessor = DB_NULL_HACCESSOR;

				//We want a clean start for every variation, so release rowset here
				ReleaseRowsetObject();
				return TRUE;				
			}					
		}
		else
			return FALSE;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - PASSBYREF without correct buffer format
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_1()
{
	
	BOOL	fResults = FALSE;
	DBBYTEOFFSET obValue;
	DBLENGTH cbMaxLen;	
	HRESULT hr;

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr,S_OK))
	{
		if (m_fPassByRef)
		{
			
			//Remember the values we will be changing
			obValue = m_rgBindings[0].obValue;
			cbMaxLen = m_rgBindings[0].cbMaxLen;	

			//Ensure our buffer does not match the provider's by
			//setting the cbMaxLen to 0 and the obValue to an offset of 1.
			//A cbMaxLen of 0 should never occur for variable length data.
			//In case of fixed length data, we use an obValue of 1, which is based 
			//on an assumption that no provider will skip one byte and then
			//start the value buffer.	
			m_rgBindings[0].cbMaxLen = 0;
			m_rgBindings[0].obValue = 1;
			
			//Try PASSBYREF with bindings that don't match provider's buffer layout
			m_hr =m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, 
				m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

			//Verify return code - may be deferred mode
			fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
				0, m_rgBindings, MAY_DEFERR, MUST_FAIL);
			
			//Reset cbMaxLen and value offset to previous values
			m_rgBindings[0].cbMaxLen = cbMaxLen;
			m_rgBindings[0].obValue = obValue;
		}
		else
		{
			//Should fail outright, without having mismatching buffers
			CHECKW(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, 
				m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus), 
				DB_E_BYREFACCESSORNOTSUPPORTED);

			fResults = TRUE;
		}


		ReleaseRowsetObject();
	}

	SAFE_RELEASE_ACCESSOR(m_pIAccessor, m_hAccessor);

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - DBMEMOWNER_PROVIDEROWNED with DBTYPE not matching provider's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_2()
{
	
	BOOL	fResults = FALSE;
	DBTYPE	wType;	
	ULONG	i=0;
	HRESULT hr;

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr,S_OK))
	{
		//Find a column which own't convert to IUnknown
		for (i=0; i < m_cBindings; i++)
		{
			if (m_rgBindings[i].wType != DBTYPE_IUNKNOWN &&
				m_rgBindings[i].wType != DBTYPE_IDISPATCH &&
				m_rgBindings[i].wType != DBTYPE_BYTES &&
				m_rgBindings[i].wType != DBTYPE_WSTR &&
				m_rgBindings[i].wType != DBTYPE_STR)
				break;
		}

		//Assume we have at least one non IUnknown convertable column, or we'll fail	
		if (i == m_cBindings)
		{
			odtLog << L"No IUnknown convertable column. \n";
			return TEST_SKIPPED;
		}

		//Remember the wType which we will be changing
		wType = m_rgBindings[i].wType;	

		//Use BYREF on the non IUnknown column's binding, and ensure our requested
		//wType does not match the provider's by using DBTYPE_IUNKNOWN
		m_rgBindings[i].wType = DBTYPE_BYREF | DBTYPE_IUNKNOWN;	

		//Make the binding provider owned
		m_rgBindings[i].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;		
		
		//Try DBMEMOWNER_PROVIDEROWNED with a binding type which doesn't match the provider's 
		m_hr =m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
			m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
		
		//Verify return code -- may be deferred
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			i, m_rgBindings, MAY_DEFERR, MUST_FAIL);
			
		//Reset correct type of binding we changed
		m_rgBindings[i].wType = wType;
		//Reset owner to client
		m_rgBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		
		ReleaseRowsetObject();

	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - Accessor with invalid coersion for column in existing optimized accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_3()
{
	TESTRESULT	fResults = TEST_FAIL;
	HACCESSOR	hOptAccessor = DB_NULL_HACCESSOR;	
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;	
	ULONG		iBackEndType;
	DBTYPE		wType = DBTYPE_EMPTY, wOptType, wNonOptType;
	HRESULT	    hr;
	BYTE *		pData=NULL;

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	/*
	Find binding indexes of bad types for this variation. We need:
		1) Backend DBTYPE can convert to Optimized accessor DBTYPE
		2) Backend DBTYPE can convert to NonOptimized accessor DBTYPE
		3) Optimized accessor DBTYPE can't convert to NonOptimized accessor DBTYPE

	This assumes the existing binding array has all columns bound to have all types
	available.
	*/
	if (!FindConversionTypes(m_rgBindings, m_cBindings, &iBackEndType, &wOptType, &wNonOptType))
	{
		odtLog <<L"Can't find a combination of data types required for this variation.\n";
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (CHECK(hr,S_OK))
	{
		//Remember value we'll be changing
		wType  = m_rgBindings[iBackEndType].wType;

		m_rgBindings[iBackEndType].wType = wOptType;

		// Create first optimized accessor.  Note providers that don't support optimized
		// accessors generally ignore the OPTIMIZED flag and just create a regular 
		// accessor.
		TESTC_(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			1, &m_rgBindings[iBackEndType], m_cbRowSize, &hOptAccessor, m_rgStatus),
			S_OK);

		m_rgBindings[iBackEndType].wType = wNonOptType;

		//Now try to create a second accessor for same column, 
		//and use an unsupported coersion of DBTYPE_IUNKNOWN
		m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
			1, &m_rgBindings[iBackEndType], m_cbRowSize, &hAccessor, m_rgStatus);
		
		//Verify return code -- may be deferred
		TESTC(VerifyError(m_pIAccessor, hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			0, &m_rgBindings[iBackEndType], MAY_DEFERR, MAY_FAIL));

	}

	fResults = TEST_PASS;

CLEANUP:

	if (wType != DBTYPE_EMPTY)
		//Set back type we changed in binding
		m_rgBindings[iBackEndType].wType = wType;

	//Release the first optimized accessor
	if (hOptAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK);
	//Release the second accessor
	if (hAccessor)
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor, NULL),S_OK);

	ReleaseRowsetObject();

	return fResults;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal of largest column number + 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_4()
{
	DBORDINAL	ulRememberOrdinal = 0;
	BOOL		fResults = FALSE;
	HRESULT		hr;

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr, S_OK))
	{
		//Set column number one too large
		ulRememberOrdinal = m_rgBindings[0].iOrdinal;
		//Number of possible columns plus 1 (for bookmark)
		//plus 1 should be invalid
		m_rgBindings[0].iOrdinal = m_pTable->CountColumnsOnTable()+2;

		//Should be invalid column number
		m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,m_cBindings, 
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);
		
		//Verify return code -- may be deferred
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
			0, m_rgBindings, MAY_DEFERR, MUST_FAIL);

		//Set correct column number back
		m_rgBindings[0].iOrdinal = ulRememberOrdinal;

		ReleaseRowsetObject();
	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal of max value for ULONG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_5()
{
	BOOL		fResults = FALSE;
	ULONG		ulMax;
	DBORDINAL	ulRememberOrdinal;
	HRESULT		hr;

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr,S_OK))
	{

		//Get the max number a ULONG can hold
		ulMax = ULONG_MAX;

		ulRememberOrdinal = m_rgBindings[0].iOrdinal;

		//Set column number to max ULONG possible
		m_rgBindings[0].iOrdinal = ulMax;

		//Max number the type can hold should be invalid column number
		m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cBindings, 
			m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

		//Verify return code -- may be deferred
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
			0, m_rgBindings, MAY_DEFERR, MUST_FAIL);

		//Set correct column number back
		m_rgBindings[0].iOrdinal = ulRememberOrdinal;

		ReleaseRowsetObject();

	}

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADORDINAL - iOrdinal = 0 for ROWDATA accessor without bookmarks
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_6()
{
	DBACCESSORFLAGS dwAccessorFlags;
	DBORDINAL cCols = 0;
	DBORDINAL iCol = 0;
	DBTYPE	dbType;
	BOOL	fResults = FALSE;
	IColumnsInfo * pIColInfo = NULL;
	DBCOLUMNINFO * rgInfo =  NULL;
	WCHAR *	pStrings = NULL;	
	BOOL	fBkmkVisible = FALSE;
	HRESULT	hr;

	//Start fresh with a rowset where no rows have 
	//been gotten, to force possible deferred validation.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr,S_OK))
	{

		//Find out if provider exposes column 0 itself before we go any farther
		if (!VerifyInterface(m_pIAccessor, IID_IColumnsInfo, 
			 ROWSET_INTERFACE, (IUnknown **)&pIColInfo))
			goto CLEANUP;
		
		if (CHECK(pIColInfo->GetColumnInfo(&cCols, &rgInfo, 
			&pStrings), S_OK))
		{		
			//Bookmark column exists on rowset
			if (rgInfo[0].iOrdinal == 0)
				fBkmkVisible = TRUE;
			
			PROVIDER_FREE(rgInfo);
			PROVIDER_FREE(pStrings);
			SAFE_RELEASE(pIColInfo);

		}
		else
			goto CLEANUP;

		//Set up parameter flags
		dwAccessorFlags = DBACCESSOR_ROWDATA;
	
		//Swap in column zero bound to string for first binding
		iCol = m_rgBindings[0].iOrdinal;		
		dbType = m_rgBindings[0].wType;
		m_rgBindings[0].iOrdinal = 0;
		m_rgBindings[0].wType = DBTYPE_STR;
	
		//Column zero is valid and should succeed
		if (fBkmkVisible)
		{
			if (CHECK(m_pIAccessor->CreateAccessor(dwAccessorFlags,
			m_cBindings, m_rgBindings, m_cbRowSize,  
			&m_hAccessor, m_rgStatus), S_OK))
			{
				fResults = CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
				m_hAccessor = DB_NULL_HACCESSOR;
			}
		}
		else
		{
			//Column zero should fail as bookmark does not exist
			m_hr = m_pIAccessor->CreateAccessor(dwAccessorFlags,
				m_cBindings, m_rgBindings, m_cbRowSize, &m_hAccessor, m_rgStatus);

			//Verify return code -- may be deferred
			fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADORDINAL, DBBINDSTATUS_BADORDINAL,
				0, m_rgBindings, MAY_DEFERR, MUST_FAIL);
		}	

		//Set bindings back the way they were
		m_rgBindings[0].iOrdinal = iCol;
		m_rgBindings[0].wType = dbType;
		
	}

CLEANUP:

	ReleaseRowsetObject();

	if (fResults)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}
			

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO - Some bindings succeeding others failing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCrtRtnValsBeforeGetRows::Variation_7()
{
	BOOL		fResults = FALSE;
	HACCESSOR	hOptAccessor = DB_NULL_HACCESSOR;	
	ULONG		iBackEndType;
	DBTYPE		wType, wOptType, wNonOptType;
	HRESULT	    hr;
	BYTE *		pData=NULL;

	//The point of this variation is to repeat variation 3 , but
	//to use some successful bindings as well as the failing
	//binding

	//Start fresh with a rowset where no rows have 
	//been gotten, to attempt force deferred validation, if applicable.
	hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

	if (CHECK(hr,S_OK))
	{

		/*
		Find binding indexes of bad types for this variation. We need:
			1) Backend DBTYPE can convert to Optimized accessor DBTYPE
			2) Backend DBTYPE can convert to NonOptimized accessor DBTYPE
			3) Optimized accessor DBTYPE can't convert to NonOptimized accessor DBTYPE

		This assumes the existing binding array has all columns bound to have all types
		available.
		*/
		if (!FindConversionTypes(m_rgBindings, m_cBindings, &iBackEndType, &wOptType, &wNonOptType))
		{
			odtLog <<L"Can't find a combination of data types required for this variation.\n";
			return TEST_SKIPPED;
		}

		//Remember value we'll be changing
		wType  = m_rgBindings[iBackEndType].wType;

		m_rgBindings[iBackEndType].wType = wOptType;

		//Create one optimized accessor with one non IUnknown column, and
		//one column of whatever is next in the binding array
		if (!CHECK(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA | DBACCESSOR_OPTIMIZED,
			2, &m_rgBindings[iBackEndType], m_cbRowSize, &hOptAccessor, m_rgStatus),
			S_OK))
			return TEST_FAIL;	

		m_rgBindings[iBackEndType].wType = wNonOptType;

		//Now try to create a second accessor for same column, 
		//and use an unsupported coersion of DBTYPE_IUNKNOWN for
		//binding one, and a valid binding for binding two
		m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
			2, &m_rgBindings[iBackEndType], m_cbRowSize, &m_hAccessor, m_rgStatus);
		
		//Verify return code -- may be deferred
		fResults = VerifyError(m_pIAccessor, m_hAccessor, DB_E_BADBINDINFO, DBBINDSTATUS_BADBINDINFO,
			0, &m_rgBindings[iBackEndType], MAY_DEFERR, MAY_FAIL);

		//Release the first optimized accessor
		if (hOptAccessor)
			CHECK(m_pIAccessor->ReleaseAccessor(hOptAccessor, NULL),S_OK);
		//Set back type we changed in binding
		m_rgBindings[iBackEndType].wType = wType;

		ReleaseRowsetObject();
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
BOOL TCCrtRtnValsBeforeGetRows::Terminate()
{

	//Cleanup everything we created in TCCrtRtnValsBeforeGetRows::Init()
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	ReleaseRowsetObject();	

	return(CAccessor::Terminate());
}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAddRefAccessor)
//*-----------------------------------------------------------------------
//| Test Case:		TCAddRefAccessor - Test the AddREfAccessor Method
//|	Created:			07/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddRefAccessor::Init()
{

	BOOL fSuccess = FALSE;
	HRESULT hr;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		hr=CreateRowsetObject(SELECT_VALIDATIONORDER);

		//Set m_pIAccessor on a 'select *' rowset  We'll
		//use this interface ptr to do our tests.
		if (CHECK(hr,S_OK))
		{			
			if(m_pICommand)
			{
				//Make sure we don't get Open Object error
				ReleaseRowsetObject();

				//Get IAccesor on Command
				if (VerifyInterface(m_pICommand,IID_IAccessor,COMMAND_INTERFACE,
					(IUnknown **)&m_pCmdIAccessor))
					//Create an accessor on the command object
					if (CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_ROWDATA,
						&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cbRowSize,			
						DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
						ALL_COLS_BOUND,							
						FORWARD, NO_COLS_BY_REF, NULL, NULL,
						NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
						m_fBindLongCols),S_OK))
						//Now create rowset again
						if (!CHECK(CreateRowsetObject(SELECT_VALIDATIONORDER),S_OK))
							return TEST_FAIL;
			}
			//Create an accessor on the rowset object
			if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
				&m_hAccessor2, &m_rgBindings2, &m_cBindings2, &m_cbRowSize2,			
				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				ALL_COLS_BOUND,							
				FORWARD, NO_COLS_BY_REF, NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongCols),S_OK))

				fSuccess = TRUE;
		}
	}
	

	return fSuccess;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Regular Addreff on an accessor, S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRefAccessor::Variation_1()
{
	BOOL fSuccess = FALSE;
	ULONG cCmdRefCount = 1;
	ULONG cRowsetRefCount = 1;
	ULONG cCmdRefCountOnRel = 1;
	ULONG cRowsetRefCountOnRel = 1;

	if(m_pCmdIAccessor)
	{
		// Check the refcount on both cmd and rowset accessors;
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK))
			goto CLEANUP;
	
		if (!COMPARE (cCmdRefCount, 2))
			goto CLEANUP;

		//  Do it again.
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK))
			goto CLEANUP;
		
		if (!COMPARE (cCmdRefCount, 3))
			goto CLEANUP;
	}

	// Do it for accessor on rowset.
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, &cRowsetRefCount), S_OK))
		goto CLEANUP;
	
	if (!COMPARE (cRowsetRefCount, 2))
		goto CLEANUP;

	
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, &cRowsetRefCount), S_OK))
		goto CLEANUP;
	
	if (!COMPARE (cRowsetRefCount, 3))
		goto CLEANUP;

	if(m_pCmdIAccessor)
	{
		if (!CHECK (m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, &cCmdRefCountOnRel), S_OK))
			goto CLEANUP;

		if (!COMPARE (cCmdRefCountOnRel, cCmdRefCount -1))
			goto CLEANUP;

		cCmdRefCount--;
		
		if (!CHECK (m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, &cCmdRefCountOnRel), S_OK))
			goto CLEANUP;

		if (!COMPARE (cCmdRefCountOnRel, cCmdRefCount -1))
			goto CLEANUP;

		cCmdRefCount--;
	}

	if (!CHECK (m_pIAccessor->ReleaseAccessor(m_hAccessor2, &cRowsetRefCountOnRel), S_OK ))
		goto CLEANUP;

	if (!COMPARE (cRowsetRefCountOnRel, cRowsetRefCount - 1))
		goto CLEANUP;

	cRowsetRefCount--;
	
	if (!CHECK (m_pIAccessor->ReleaseAccessor(m_hAccessor2, &cRowsetRefCountOnRel), S_OK ))
		goto CLEANUP;
	
	if (!COMPARE (cRowsetRefCountOnRel, cRowsetRefCount - 1))
		goto CLEANUP;

	cRowsetRefCount--;
	
	fSuccess = TRUE;

CLEANUP:

	while (cRowsetRefCount > 1)
		m_pIAccessor->ReleaseAccessor (m_hAccessor2, &cRowsetRefCount);

	while (m_pCmdIAccessor && cCmdRefCountOnRel > 1)
		m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, &cCmdRefCount);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Addref on Command and Rowset object.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRefAccessor::Variation_2()
{
	BOOL fSuccess = FALSE;

	ULONG cCmdRefCount = 1;
	ULONG cRowsetRefCount = 1;
	
	// Now m_hAccessor is a command accessor.  In should have been copied
	// individual accessors.
	if(g_fCmdSupported)
	{
		TESTC_(m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK);
	
		COMPARE (cCmdRefCount, 2);

		TESTC_(m_pCmdIAccessor->ReleaseAccessor (m_hAccessor, &cCmdRefCount), S_OK);

		COMPARE (cCmdRefCount, 1);
	}


	//  Do it  for the rowset accessor.
	TESTC_(m_pIAccessor->AddRefAccessor (m_hAccessor2, &cRowsetRefCount), S_OK);
	
	COMPARE (cRowsetRefCount, 2);

	TESTC_(m_pIAccessor->ReleaseAccessor (m_hAccessor2, &cRowsetRefCount), S_OK);

	COMPARE (cRowsetRefCount, 1);

	fSuccess = TRUE;

CLEANUP:
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc NULL for pcRefCount arguments.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRefAccessor::Variation_3()
{
	ULONG cCmdRefCount = 1;
	ULONG cRowsetRefCount = 1;
	BOOL fSuccess = FALSE;
	
	if(g_fCmdSupported)
	{
		// Make ref count 2.
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, NULL), S_OK))
			goto CLEANUP;

		// Make ref count 3
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, NULL), S_OK))
			goto CLEANUP;

		// Make ref count 4.
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK))
			goto CLEANUP;
		
		if (!COMPARE (cCmdRefCount, 4))
			goto CLEANUP;
	}

	//  Do it  for the copied accessor.
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, NULL), S_OK))
		goto CLEANUP;
	
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, NULL), S_OK))
		goto CLEANUP;
	
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, &cRowsetRefCount), S_OK))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// Retain the original ref count.
	while (cRowsetRefCount > 1)
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, &cRowsetRefCount);

	if(g_fCmdSupported)
	{
		while (m_pCmdIAccessor && cCmdRefCount > 1)
			m_pCmdIAccessor->ReleaseAccessor (m_hAccessor, &cCmdRefCount);
	}

	return TEST_PASS;

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid Accessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRefAccessor::Variation_4()
{
	BOOL fResult = TEST_PASS;

	if (!CHECK(m_pIAccessor->AddRefAccessor(DB_NULL_HACCESSOR, NULL), DB_E_BADACCESSORHANDLE ) )
			fResult = TEST_FAIL;

	if(g_fCmdSupported)
	{
		if(!m_pCmdIAccessor)
			return TEST_FAIL;

		if (!CHECK (m_pCmdIAccessor->AddRefAccessor(DB_NULL_HACCESSOR, NULL), DB_E_BADACCESSORHANDLE ) )
			fResult = TEST_FAIL;
	}

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Release accessor on a rowset object.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRefAccessor::Variation_5()
{
	// In this test case Add reff cmd accessor and release rowset accessor.  
	// Verify Release returns S_OK on the Last ref count.
	
	BOOL fSuccess = FALSE;
	ULONG cCmdRefCount = 1;
	ULONG cRowsetRefCount = 1;

	if(g_fCmdSupported)
	{
		// Now m_hAccessor is a command accessor.  In should have been copied
		// individual accessors.
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK))
			goto CLEANUP;
			
		if (!COMPARE (cCmdRefCount, 2))
			goto CLEANUP;
	}

	//  Do it  for the copied accessor.
	if (!CHECK (m_pIAccessor->AddRefAccessor (m_hAccessor2, &cRowsetRefCount), S_OK))
		goto CLEANUP;
	
	if (!COMPARE (cRowsetRefCount, 2))
		goto CLEANUP;

	// Make sure only once ref count is left on Rowset accessor.
	while (cRowsetRefCount > 1)
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, &cRowsetRefCount);

	// Now addRef the CmdAccessor and Check for RefCount to be 3.
	if(m_pCmdIAccessor)
	{
		if (!CHECK (m_pCmdIAccessor->AddRefAccessor (m_hAccessor, &cCmdRefCount), S_OK))
			goto CLEANUP;
		
		if (!COMPARE (cCmdRefCount, 3))
			goto CLEANUP;
	}

	fSuccess = TRUE;
CLEANUP:
	
	// Retain the original Ref count.
	while (m_pCmdIAccessor && cCmdRefCount > 1)
		m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, &cCmdRefCount);

	while (cRowsetRefCount > 1)
		m_pIAccessor->ReleaseAccessor(m_hAccessor2, &cRowsetRefCount);

	return TEST_PASS;

}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddRefAccessor::Terminate()
{	
	FREE_BINDINGS(&m_cBindings,  &m_rgBindings);
	FREE_BINDINGS(&m_cBindings2, &m_rgBindings2);
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pCmdIAccessor);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAccessorOnAlteredTable)
//*-----------------------------------------------------------------------
//| Test Case:		TCAccessorOnAlteredTable - Test to test the validity of accessors once rowset is modified.
//|	Created:			08/05/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessorOnAlteredTable::Init()
{

	BOOL fSucceed = FALSE;
	BOOL fProc = FALSE;		// Whether we're using procs or views to cause columns changed
	ULONG i;
	DBLENGTH ulOffset = 0;
	HROW *hRowPtr = NULL;
	ICommandText *pIAlterCommandText = NULL;
	DBCOUNTITEM		cRowsObtained;
	HROW 			rghRows[1];
	HROW *			prghRows=rghRows;
	CCol TempCol;
	WCHAR * pwszCol1 = NULL;
	WCHAR * pwszCol2 = NULL;
	DBTYPE wtype1, wtype2;
	DBLENGTH ulPrec1, ulPrec2;
	WCHAR wszSQL[1000];
	WCHAR * pwszColList = NULL;

	// Initialize class variables.CLASS VARIABLES
	m_pATICommandText = NULL;
	m_pATIRowset = NULL;
	m_pIColumnsRowsetInfo = NULL;
	m_pATIColumnsInfo = NULL;
	m_pATIAccessor = NULL;
	m_pATIColumnsRowset = NULL;
	m_cbRowSize = 0 ;  // Some safe size.
	m_cATDbBindings = 2;
	m_hATAccessor = DB_NULL_HACCESSOR;
	m_cColumnsInfo = 0;
	m_rgColumnsInfo = NULL;
	m_pStringsBuffer = NULL;
	m_cColumnsRowsetInfoObtained = 0;
	m_rgColumnsRowsetInfohRows[3];  // 2 hrows for Now One for Later (after alter table).
	m_pwszObjName = NULL;

	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAccessor::Init())
	// }}
	{
		//command is not supported
		if(!m_pIDBCreateCommand)
		{
			odtLog << wszCommandNotSupported;
			return TEST_SKIPPED;
		}

		// Create the command objects
		if(!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown **)&m_pATICommandText), S_OK))
			goto CLEANUP;

		// Create a unique view or proc name (object name) rather than a "hard-coded" one.  Make it the
		// same length as the table name.
		m_pwszObjName = MakeObjectName(L"IAccObj", wcslen(m_pTable->GetTableName()));

		if (!m_pwszObjName)
			goto CLEANUP;

		// Go through the columns of the table and find two with different data types.
		for (ULONG iCol=1; iCol<=m_pTable->CountColumnsOnTable() && !pwszCol2; iCol++)
		{
			CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK);

			if (iCol == 1)
			{
				wtype1 = TempCol.GetProviderType();
				ulPrec1 = TempCol.GetMaxSize();
				if (wtype1 == DBTYPE_STR)
					ulPrec1+=sizeof(CHAR);	// Allow for null terminator
				if (wtype1 == DBTYPE_WSTR)
					ulPrec1+=sizeof(WCHAR);	// Allow for null terminator
				pwszCol1 = wcsDuplicate(TempCol.GetColName());
				m_rgColMap[0] = iCol;
			}
			else if (TempCol.GetProviderType() != wtype1)
			{
				wtype2 = TempCol.GetProviderType();
				ulPrec2 = TempCol.GetMaxSize();
				if (wtype2 == DBTYPE_STR)
					ulPrec2+=sizeof(CHAR);	// Allow for null terminator
				if (wtype2 == DBTYPE_WSTR)
					ulPrec2+=sizeof(WCHAR);	// Allow for null terminator
				pwszCol2 = wcsDuplicate(TempCol.GetColName());
				m_rgColMap[1] = iCol;
			}
		}

		if (!pwszCol2)
		{
			// We couldn't find two different data types in the table, skip this test
			odtLog << L"This test case needs two different data types.\n";
			fSucceed = TEST_SKIPPED;
			goto CLEANUP;
		}

		// The row size is the size of each data item plus the status and length
		m_cbRowSize = ulPrec1 + ulPrec2 + 2*sizeof(DBSTATUS) + 2*sizeof(DBLENGTH);

		SAFE_ALLOC(pwszColList, WCHAR, wcslen(pwszCol1)+wcslen(pwszCol2)+wcslen(L", ")+sizeof(WCHAR));
		swprintf(pwszColList, L"%s, %s", pwszCol1, pwszCol2);

		swprintf(wszSQL, wszCREATE_VIEW, m_pwszObjName, pwszColList, m_pTable->GetTableName());

		// Now try to create a view on the table in ordinal order
		if (!CHECK (m_pATICommandText->SetCommandText (DBGUID_DBSQL, wszSQL), S_OK))
			goto CLEANUP;

		// Create the view
		if (FAILED(m_pATICommandText->Execute (NULL, IID_NULL, NULL, NULL, NULL)))
		{
			// Create view failed, create a proc on the table in ordinal order instead
			swprintf(wszSQL, wszCREATE_PROC, m_pwszObjName, pwszColList, m_pTable->GetTableName());
			if (!CHECK (m_pATICommandText->SetCommandText (DBGUID_DBSQL, wszSQL), S_OK))
				goto CLEANUP;

			if (FAILED(m_pATICommandText->Execute (NULL, IID_NULL, NULL, NULL, NULL)))
			{
				// Skip test if we can't create views or procedures for some reason.  Not all providers support this.
				odtLog << "Can't create the view or procedure needed for this test case.\n";
				fSucceed = TEST_SKIPPED;
				goto CLEANUP;
			}

			fProc = TRUE;
		}
		else
			m_pTable->SetViewName(m_pwszObjName);

		// Now generate the rowset
		if (!fProc)
		{
			// Select from the view
			swprintf(wszSQL, wszSELECT_COLLISTFROMTBL, L"*", m_pwszObjName);
		}
		else
		{
			// {call procname}
			swprintf(wszSQL, wszEXEC_PROC, m_pwszObjName);
		}

		if (!CHECK (m_pATICommandText->SetCommandText (DBGUID_DBSQL, wszSQL), S_OK))
			goto CLEANUP;

		if (!CHECK (m_pATICommandText->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&m_pATIRowset), S_OK))
			goto CLEANUP;

		// Now we have the rowset
		// Lets create the accessor;
		if (!VerifyInterface(m_pATIRowset, IID_IAccessor,
				ROWSET_INTERFACE,(IUnknown **)&m_pATIAccessor))
		{
			goto CLEANUP;
		}

		// Verify other properties.
		if (!VerifyInterface(m_pATIRowset, IID_IColumnsInfo,
				ROWSET_INTERFACE,(IUnknown **)&m_pATIColumnsInfo))
		{
			goto CLEANUP;
		}

		// Create the bindings and the create handle to accessor.
		for (i = 0; i < m_cATDbBindings; i++ )
		{
			m_rgATDbBindings[i].dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
			m_rgATDbBindings[i].eParamIO = DBPARAMIO_NOTPARAM;
			m_rgATDbBindings[i].iOrdinal = i+1;
			m_rgATDbBindings[i].pTypeInfo = NULL;
			
			m_rgATDbBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
			m_rgATDbBindings[i].pBindExt = NULL;
			m_rgATDbBindings[i].bPrecision = 0;
			m_rgATDbBindings[i].bScale = 0;
		}

		// Set Binding values of individual members
		// For data type 1
		m_rgATDbBindings[0].obValue = offsetof (DATA, bValue);
		m_rgATDbBindings[0].obLength = offsetof (DATA, ulLength);
		m_rgATDbBindings[0].obStatus = offsetof (DATA, sStatus);
		m_rgATDbBindings[0].cbMaxLen = ulPrec1;
		m_rgATDbBindings[0].wType = wtype1; 
		m_rgATDbBindings[0].dwFlags = 0; 

		ulOffset = sizeof (DATA) + ulPrec1;
		ulOffset = ROUND_UP(ulOffset, ROUND_UP_AMOUNT);
		
		// For data type 2
		m_rgATDbBindings[1].obValue = ulOffset + offsetof (DATA, bValue);
		m_rgATDbBindings[1].obLength =  ulOffset + offsetof (DATA, ulLength);
		m_rgATDbBindings[1].obStatus =  ulOffset + offsetof (DATA, sStatus);
		m_rgATDbBindings[1].cbMaxLen = ulPrec2;
		m_rgATDbBindings[1].wType = wtype2; 
		m_rgATDbBindings[1].dwFlags = 0; 

		ulOffset+=sizeof (DATA) + ulPrec2;
		ulOffset = ROUND_UP(ulOffset, ROUND_UP_AMOUNT);

		m_cbRowSize = ulOffset;

		// Call create accessor.
		if (!CHECK (m_pATIAccessor->CreateAccessor( 
			DBACCESSOR_ROWDATA, m_cATDbBindings, m_rgATDbBindings, m_cbRowSize,
			&m_hATAccessor, NULL), S_OK))
		{
		
			goto CLEANUP;
		}

		if (!CHECK (m_pATIColumnsInfo->GetColumnInfo ( &m_cColumnsInfo, &m_rgColumnsInfo, &m_pStringsBuffer), S_OK))
			goto CLEANUP;

		// We have to retrieve data before DB_S_COLUMNSCHANGED will be returned.
		if (!CHECK(m_pATIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows), S_OK))
			goto CLEANUP;

		if (!CHECK(m_pATIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL), S_OK))
			goto CLEANUP;
	
		if(FAILED(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown **)&pIAlterCommandText)))
		{
			odtLog << "Create command  Failed\n";
			goto CLEANUP;
		}

		// Switch the order of the columns
		swprintf(pwszColList, L"%s, %s", pwszCol2, pwszCol1);

		// Now alter the rowset underneath
		if (!fProc)
		{
			if (FAILED(m_pTable->ExecuteCommand(DROP_VIEW, IID_NULL, m_pwszObjName,	NULL, 0, NULL,
				EXECUTE_IFNOERROR)))
			{
				if (FAILED(m_pTable->ExecuteCommand(DROP_TABLE, IID_NULL, m_pwszObjName, NULL, 0, NULL,
					EXECUTE_IFNOERROR)))
				{
					odtLog << L"Couldn't alter rowset while open.\n";
					goto CLEANUP;
				}
			}


			// Now create the same view on the table in reverse ordinal order
			swprintf(wszSQL, wszCREATE_VIEW, m_pwszObjName, pwszColList, m_pTable->GetTableName());
			if (!CHECK (pIAlterCommandText->SetCommandText (DBGUID_DBSQL, wszSQL), S_OK))
				goto CLEANUP;

		}
		else
		{
			if (FAILED(m_pTable->ExecuteCommand(DROP_PROC, IID_NULL, m_pwszObjName, NULL, 0, NULL,
				EXECUTE_IFNOERROR)))
			{
				odtLog << L"Couldn't alter rowset while open.\n";
				goto CLEANUP;
			}

			// Now create the same proc on the table in reverse ordinal order
			swprintf(wszSQL, wszCREATE_PROC, m_pwszObjName, pwszColList, m_pTable->GetTableName());
			if (!CHECK (pIAlterCommandText->SetCommandText (DBGUID_DBSQL, wszSQL), S_OK))
				goto CLEANUP;

		}

		// Create the view or proc again
		if (!CHECK (pIAlterCommandText->Execute (NULL, IID_NULL, NULL, NULL, NULL), S_OK ))
			goto CLEANUP;
	

		fSucceed = TRUE;
	}

	
CLEANUP:

	SAFE_FREE(pwszCol1);
	SAFE_FREE(pwszCol2);
	SAFE_FREE(pwszColList);

	SAFE_RELEASE(pIAlterCommandText);

	return fSucceed;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test to verify the validity of accessors on an altered rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessorOnAlteredTable::Variation_1()
{
	DBCOUNTITEM cRowsObtained;
	HROW * phRow = NULL;
	BYTE * pData=NULL;
	BOOL fSuccess = FALSE;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBORDINAL ulTemp;
	HRESULT hrRestart = E_FAIL;

	// Call RestartPosition and Get DB_S_COLUMNSCHANGED.
	hrRestart = m_pATIRowset->RestartPosition(NULL);
	
	if (FAILED(hrRestart))
	{
		// This will always return E_FAIL on Kagers; Kagera can't return DB_S_COLUMNSCHANGED
		TESTC_(hrRestart, DB_S_COLUMNSCHANGED);
	}
	
	// Allocate space for data buffer
	SAFE_ALLOC(pData, BYTE, m_cbRowSize);
	memset(pData, 0, (size_t)m_cbRowSize);

	// If the columns changed we have to rebuild the accessor to set the ordinals oppositely
	if (hrRestart == DB_S_COLUMNSCHANGED)
	{
		// Since we changed the column ordering it's the consumer's responsibility to 
		// correctly modify the accessor.
		ulTemp = m_rgColMap[0];
		m_rgColMap[0] = m_rgColMap[1];
		m_rgColMap[1] = ulTemp;

		ulTemp = m_rgATDbBindings[0].iOrdinal;
		m_rgATDbBindings[0].iOrdinal = m_rgATDbBindings[1].iOrdinal;
		m_rgATDbBindings[1].iOrdinal = ulTemp;
	}

	// Now create the accessor
	TESTC_(m_pATIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, m_cATDbBindings, m_rgATDbBindings, m_cbRowSize,
		&hAccessor, NULL), S_OK);
	
	// Call GetNextRows to fetch the first row
	TESTC_(m_pATIRowset->GetNextRows(NULL,0,1,&cRowsObtained, &phRow), S_OK);

	// Call GetData to retrieve the data valuus
	TESTC_(m_pATIRowset->GetData(*phRow, hAccessor, pData), S_OK);

	//Verify data value, length and status are what is expected
	TESTC(CompareData(m_cATDbBindings, m_rgColMap, 1, pData, m_cATDbBindings, m_rgATDbBindings,
		m_pTable, m_pIMalloc, PRIMARY));

	fSuccess=TRUE;

CLEANUP:

	SAFE_RELEASE_ACCESSOR(m_pATIAccessor, hAccessor);

	if (m_pATIRowset && phRow)
		m_pATIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL);

	SAFE_RELEASE(m_pATIRowset);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(phRow);

	return (fSuccess) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessorOnAlteredTable::Terminate()
{
	SAFE_RELEASE(m_pATIColumnsInfo);
	SAFE_RELEASE(m_pATIAccessor );
	PROVIDER_FREE(m_pStringsBuffer);
	PROVIDER_FREE(m_rgColumnsInfo );

	if (m_pIColumnsRowsetInfo && m_cColumnsRowsetInfoObtained > 0)
			m_pIColumnsRowsetInfo->ReleaseRows(m_cColumnsRowsetInfoObtained, m_rgColumnsRowsetInfohRows,
				NULL, NULL, NULL);

	SAFE_RELEASE(m_pIColumnsRowsetInfo);

	// Try to drop the view with both "drop view" and "drop table" syntax.  We don't check the return
	// code because Terminate will be called even if the creation in init failed.
	if (m_pATICommandText && m_pwszObjName && FAILED(m_pTable->ExecuteCommand(DROP_VIEW, IID_NULL, m_pwszObjName,
		NULL, 0, NULL, EXECUTE_IFNOERROR)))
	{
		// Some providers use "Drop Table" syntax to drop a view
		if (FAILED(m_pTable->ExecuteCommand(DROP_TABLE, IID_NULL, m_pwszObjName, NULL, 0, NULL, EXECUTE_IFNOERROR)))
		{
			// If views aren't supported we may use procedures
			m_pTable->ExecuteCommand(DROP_PROC, IID_NULL, m_pwszObjName, NULL, 0, NULL, EXECUTE_IFNOERROR);
		}
	}

	if (m_pATIAccessor)
		m_pATIAccessor->ReleaseAccessor (m_hATAccessor, NULL);

	SAFE_FREE(m_pwszObjName);
	SAFE_RELEASE(m_pATIColumnsRowset);
	SAFE_RELEASE(m_pATIColumnsInfo);
	SAFE_RELEASE(m_pATIAccessor );
	SAFE_RELEASE(m_pATICommandText);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAccessor::Terminate());
}	// }}
// }}
// }}

//-----------------------------------------------------------------------
// Microsoft OLE DB
//
// Copyright (C) 1997-2000 Microsoft Corporation
//
// @doc  
//
// @module QUIKTEST.CPP | This test module performs minimum testing 
// of an OLE DB provider by calling each method in the API.
//

#include "MODStandard.hpp"	//Standard headers to be precompiled in MODStandard.cpp			
#include "QuikTest.h"
#include "ExtraLib.h"
#include "msdasc.h"         //IID_IDataInitialize
#include "msdadc.h"			//IID_IRowPosition
#include <direct.h>         //Needed for _getcwd (current working directory)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xca9539f1, 0xd59f, 0x11ce, { 0xa9, 0xda, 0x00, 0xaa, 0x00, 0x3e, 0x77, 0x8a }};
DECLARE_MODULE_NAME("Quiktest");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("OLE DB Quik Test");
DECLARE_MODULE_VERSION(810940912);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END
			 

//----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	return CreateModInfo(pThisTestModule);
}	

	 
//----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	return ReleaseModInfo(pThisTestModule);
}	


//----------------------------------------------------------------------
//Prints the error messages when run as a stand-alone executable.
//
HRESULT OutputText(const WCHAR* pwszFmt, ...)
{
	va_list	marker;

	va_start(marker, pwszFmt);
	vwprintf(pwszFmt, marker);
	va_end(marker);

	return S_OK;
} //OutputText



////////////////////////////////////////////////////////////////////////
// CBaseError Implementation
//
////////////////////////////////////////////////////////////////////////

//This class has the IError interface. Below is the implementation of the
//methods of the IError interface and methods specific to the CBaseError
//class. This class will be used as the error object when quiktest is
//run as a stand-alone executable.

//Constructor for CBaseError.
CBaseError::CBaseError()
{
	//Errors
	m_cModErrors = 0;		
	m_cCaseErrors = 0;		
	m_cVarErrors = 0;				

	//Warnings
	m_cModWarnings = 0;		
	m_cCaseWarnings = 0;		
	m_cVarWarnings = 0;				

	m_ErrorLevel = HR_STRICT;
	m_ExpectedHr = NULL;
	m_ActualHr = NULL;

	m_cRef = 0;
}

//Destructor (virtual) for CBaseError.
CBaseError::~CBaseError()
{
}

//QI for a supported interface.
STDMETHODIMP CBaseError::QueryInterface(REFIID riid, void **ppvObject)
{
	if(ppvObject == NULL)
		return E_INVALIDARG;
	*ppvObject = NULL;

	if(riid == IID_IUnknown)
		*ppvObject = (IUnknown*)this;
	else if(riid == IID_IError)
		*ppvObject = (IError*)this;
	else 
		return E_NOINTERFACE;

	((IUnknown*)*ppvObject)->AddRef();
	return S_OK;
}

//Add reference count.
STDMETHODIMP_(DWORD) CBaseError::AddRef(void)
{
	return InterlockedIncrement((LONG*)&m_cRef);
}

//Decrement the reference count. If it goes down to zero delete 
//this object.
STDMETHODIMP_(DWORD) CBaseError::Release(void)
{
	InterlockedDecrement((LONG*)&m_cRef);
	if(m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

STDMETHODIMP CBaseError::ResetModErrors(void) 
{	
	m_cModErrors = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::ResetModWarnings(void) 
{	
	m_cModWarnings = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::ResetCaseErrors(void) 
{	
	m_cCaseErrors = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::ResetCaseWarnings(void) 
{	
	m_cCaseWarnings = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::ResetVarErrors(void) 
{	
	m_cVarErrors = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::ResetVarWarnings(void) 
{	
	m_cVarWarnings = 0; 
	return S_OK; 
}

STDMETHODIMP CBaseError::GetVarErrors(LONG *pdw)
{
	*pdw = m_cVarErrors; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetVarWarnings(LONG *pdw)
{
	*pdw = m_cVarWarnings; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetCaseErrors(LONG *pdw)
{
	*pdw = m_cCaseErrors; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetCaseWarnings(LONG *pdw)
{
	*pdw = m_cCaseWarnings; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetModErrors(LONG *pdw)
{
	*pdw = m_cModErrors; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetModWarnings(LONG *pdw)
{
	*pdw = m_cModWarnings; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetActualHr(LONG *pHR)
{
	*pHR = m_ActualHr; 
	return S_OK;
}

STDMETHODIMP CBaseError::SetErrorLevel(ERRORLEVEL eLevel)
{
	m_ErrorLevel = eLevel; 
	return S_OK;
}

STDMETHODIMP CBaseError::GetErrorLevel(ERRORLEVEL *pLevel) 
{
	*pLevel = m_ErrorLevel; 
	return S_OK;
}

//Print the expected HRESULT.
STDMETHODIMP CBaseError::LogExpectedHr(LONG hrExpected)
{
	//Try to find the Error
	WCHAR* pwszError = GetErrorName(hrExpected);
	if(pwszError)
		return OutputText(L"\t\t%s %s\n", wszExpectedHR, pwszError);
	else
		return OutputText(L"\t\t%s 0x%08x\n", wszExpectedHR, hrExpected);
}

//Print the received HRESULT, file name, and line number.
STDMETHODIMP CBaseError::LogReceivedHr(LONG hrExpected, BSTR bstrFile, LONG udwLine)
{
	//Try to find the Error
	WCHAR* pwszError = GetErrorName(hrExpected);
	if(pwszError)
		OutputText(L"\t\t%s %s\n", wszReceivedHR, pwszError);
	else
		OutputText(L"\t\t%s 0x%08x\n", wszReceivedHR, hrExpected);

	//Print File and Line Number...
	return OutputText(L"\t\t%s %s  %s %d\n\n", wszFile, bstrFile, wszLine, udwLine);
}

//Compare the actual HR with the expected HR and increment error or 
//warning count accordingly. Print appropriate message.
STDMETHODIMP CBaseError::Validate(

	LONG ActualHr,	 						  					  
	BSTR bstrFile,		
	LONG udwLine,		
	LONG ExpectedHr,
	VARIANT_BOOL *pfResult)
{	
	//Record the HRESULT received by the function in question
	m_ActualHr = ActualHr;
	
	//Now we decide what to do about it
	switch (m_ErrorLevel)
	{
		case HR_OPTIONAL:	//We don't care what the received result is					
			*pfResult = TRUE;
			return S_OK;

		case HR_WARNING:	//Expected must match received.
		case HR_STRICT:		//NOTE: we differentiate between warning 
							//and strict in Increment and Logxxxxx.									
			if(ActualHr != ExpectedHr)	
			{
				//Display the Header
				if(HR_WARNING == m_ErrorLevel)
				{
					Transmit((WCHAR*)wszWarningTitle);
				}
				else
				{
					Transmit((WCHAR*)wszErrorTitle);
				}
							 	
				//Log what we expected
				Increment();
				LogExpectedHr(ExpectedHr);
			}
			else
			{
				*pfResult = TRUE;
				return S_OK;
			}
			break;

		case HR_SUCCEED:	//Received result must be success code
			if(!SUCCEEDED(ActualHr))
			{
				//Log what we expected
				OutputText(L"\t\t%s\n", wszExpectedSuccess);
			}
			else
			{
				*pfResult = TRUE;
				return S_OK;
			}
			break;

		case HR_FAIL:	//Received result must be failure code
			if(!FAILED(ActualHr))
			{
				//Log what we expected
				Transmit("Expected a Failing HRESULT\n");
			}
			else
			{
				*pfResult = TRUE;
				return S_OK;
			}
			break;

		default:
			ASSERT(FALSE);
	}

	//If we got this far, the HRESULT received didn't match
	//what was expected, so log what was received and where
	LogReceivedHr(ActualHr, bstrFile, udwLine);
	*pfResult = FALSE;
	return S_OK;
}

//If objects compared were not equal, increment the error or warning 
//count. Print appropriate message.
STDMETHODIMP CBaseError::Compare(

		VARIANT_BOOL	fEqual,		
		BSTR			bstrFile,	
		LONG			udwLine,
		VARIANT_BOOL *	pfResult)
{
	//If the objects where not equal, increment error and print message
	if(!fEqual)
	{						 	
		Increment();
		OutputText(L"\t\t%s\n", wszCompareFailed);
		OutputText(L"\t\t%s %s  %s %d\n\n", wszFile, bstrFile, wszLine, udwLine);
	}	

	*pfResult = fEqual;
	return S_OK;
}

//Increment the error or warning counters.
STDMETHODIMP CBaseError::Increment(void)
{
	if(HR_WARNING == m_ErrorLevel)
	{
		//Increment all warning counters, from variation level on up
		m_cVarWarnings++;
		m_cCaseWarnings++;
		m_cModWarnings++;
	}
	else
	{
		//Increment all error counters, from variation level on up
		m_cVarErrors++;
		m_cCaseErrors++;
		m_cModErrors++;
	}

	return S_OK;
}

//Print text which is passed in as a CHAR*
HRESULT CBaseError::Transmit(CHAR* pszString)
{
	WCHAR* pwszString = NULL;
	pwszString = ConvertToWCHAR(pszString);
	OutputText(L"\t\t");
	OutputText(pwszString);
	SAFE_FREE(pwszString);
	return S_OK;
};

//Print text which is passed in as a BSTR.
HRESULT CBaseError::Transmit(BSTR bstrString)
{
	OutputText(L"\t\t");
	return OutputText(bstrString);
};

STDMETHODIMP CBaseError::Initialize(void)
{
	return S_OK;
}



////////////////////////////////////////////////////////////////////////
// CQuickTest Class
//
////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------
// @class A base class used to do common work for all interface testcases,
// such as verifying the interface is supported, and storage and cleanup of 
// interface pointers.  It is assumed that any interfaces/objects obtained
// by the user outside member functions are released by the user before
// the next attempt to obtain new objects/interfaces via the member 
// functions.

class CQuickTest : public CSessionObject
{
protected:

//VARIABLES

	//@cmember	DBPROP_SQLSUPPORT
	ULONG_PTR	m_ulSQLSup;

	//@cmember	Maximum number of open rows (DBPROP_MAXOPENROWS)
	ULONG_PTR	m_ulMaxOpenRows;

	//@cmember	Can Fetch bachwards (DBPROP_CANFETCHBACKWARDS)
	BOOL		m_fFetchBackwards;

	//@cmember	File name for persisted file 
	WCHAR		m_wszFile[2*PATH_SIZE];

	//@cmember	Number of property Info sets.
	ULONG		m_cPropInfoSets;

	//@cmember	Array of property Info Sets.
	DBPROPINFOSET*	m_rgPropInfoSets;

	//@cmember	Number of property ID sets.
	ULONG		m_cPropIDSets;

	//@cmember	Array of property ID Sets.
	DBPROPIDSET*	m_rgPropIDSets;

	//@cmember	URL to a rowset (the rowset dumped into the INI file).
	WCHAR*		m_pwszRowsetURL;

	//@cmember	Pointer to a CRowset object.
	CRowset*	m_pCRowset;

//INTERFACES

	//@cmember	Pointer to IpersistFile interface
	IPersistFile*	m_pIPersistFile;

	//@cmember	Pointer to IDBProperties interface
	IDBProperties*	m_pIDBProperties;

	//@cmember	Pointer to IRowset interface
	IRowset*	m_pIRowset;

	//@cmember	Pointer to ICommand interface
	ICommand*	m_pICommand;

	//@cmember	Pointer to IBindResource (Binder) interface
	IBindResource*	m_pIBindResource;

	//@cmember	Pointer to IRow interface
	IRow*		m_pIRow;
	
private:

	//List of Provider types
	DBTYPE*		m_rgProviderTypes;

	//Count of List of Provider types
	ULONG		m_cProviderTypes;

	//@cmember Path to current working directory
	CHAR		m_szPath[PATH_SIZE];
		
protected:
	
	//@cmember	Constructor - Takes the derived TestCase name as parameter
	CQuickTest(LPWSTR wszTestCaseName);		//[IN] Testcase Name
	
	//@cmember	Destructor
	virtual		~CQuickTest();	
	
	//@cmember Saves object to file in current working directory
	HRESULT		PersistToFile();

	//@cmember	Adds another DBPROPINFO or DBPROPINFOSET to the dynamic 
	//			array m_rgPropInfoSets.
	HRESULT		SetPropInfo( 
					DBPROPID	propID,       //[IN] Property ID
					const GUID&	guidPropSet,  //[IN] Property Set
					VARTYPE		varType		  //[IN] Variant type
					);

	//@cmember	Adds another DBPROPID or DBPROPIDSET to the dynamic 
	//			array m_rgPropIDSets.
	HRESULT		SetPropID( 
					DBPROPID	propID,       //[IN] Property ID
					const GUID&	guidPropSet   //[IN] Property Set
					);

	//@cmember	Initializes m_rgPropIDSets and m_rgPropInfoSets with some
	//			selected properties.
	void		InitPropIDStructs();

	//@cmember	Set the flags of the given property info.
	BOOL		SetFlags(
					DBPROPID	propID,       //[IN] Property ID
					const GUID&	guidPropSet,  //[IN] Property Set
					DBPROPFLAGS	dwFlags		  //[IN] Prop Flags
					);
	
	//@cmember	Call GetPropertyInfo and sets the Flag of the properties
	//			in m_rgPropInfoSets. This can be used later to check if 
	//			the property is supported.
	void		InitSupInfo();

	//@cmember	Check if the I2 value is positive. Used for checking 
	//			value of DBPROP_INIT_PROMPT.
	BOOL		checkPositive_I2(VARIANT* pv);

	//@cmember	Wrapper for IRowset::GetNextRows.
	HRESULT		GetNextRows(
					IRowset*	pIRowset,		//[IN] Pointer to rowset			
					HCHAPTER*	hChapter,		//[IN] Handle to chapter
					DBROWOFFSET		lRowsOffset,	//[IN] Offset to row
					DBROWCOUNT		cRows,			//[IN] Number of rows to fetch
					DBCOUNTITEM*		pcRowsObtained,	//[OUT] Number of rows fetched
					HROW**		prghRows		//[OUT] Handle to fetched rows.
					);

	//@cmember	Wrapper for IRowset::ReleaseRows.
	HRESULT		ReleaseRows(
					IRowset**	ppIRowset,	//[IN] Pointer to rowset
					DBCOUNTITEM*	pcRows,		//[IN] Num of rows to release
					HROW**		prghRows		//[IN] HROW of rows to release
					);

	//@cmember	Wrapper for IRowset::RestartPosition.
	HRESULT		RestartPosition(IRowset* pIRowset) ;

	//@cmember	Release the HACCESSOR and free the bindings. Release
	//			IAccessor if pbReleaseIAcc is TRUE.
	HRESULT		FreeAccessorAndBindings(
					IAccessor**		ppIAccessor,		//[IN] Pointer to IAccessor
					HACCESSOR*		phAccessor,			//[IN] Handle to accessor
					DBCOUNTITEM*	pcBindings,			//[IN] Number of bindings
					DBBINDING**		prgBindings,		//[IN] Binding Structs.
					BOOL			pbReleaseIAcc=TRUE	//[IN] Release IAccessor?
					);

	//@cmember	Append a property and its value to the Init String.
	HRESULT		AppendToInitString(
					WCHAR**		ppwszInitString, 
					DBPROPID	dwPropertyID, 
					GUID		guidPropertySet, 
					VARIANT*	pVariant
					);

	//@cmember	Generate a rowset (call OpenRowset) on the given table.
	HRESULT		CreateOpenRowset(
					CTable*		pTable,            //[IN] Table on which to open the rowset.
					const IID&	riid,              //[IN] IID of requested interface on rowset.
					IUnknown**	ppRowset,          //[OUT] Pointer to rowset interface.
					IUnknown*	pUnkOuter = NULL   //[IN] For aggregation.
					);

	//@cmember	Verify the bindings obtained by GetBindings is same as 
	//			that used for creating the accessor.
	BOOL		VerifyBindings(
					IAccessor*		pIAccessor,           //[IN] Pointer to IAccessor. 
					HACCESSOR		hAccessor,            //[IN] handle to accessor
					DBCOUNTITEM		cBindings,            //[IN] Number of binding structs
					DBBINDING*		rgBindings,           //[IN] Binding structures
					DBACCESSORFLAGS dwCreateAccessorFlags //[IN] Value of Accessor flags used to create accessor
					);	

	//@cmember	Fetch the specified rows, get their data and compare.
	BOOL		GetDataAndCompare(
					DBROWOFFSET		cSkipRows,		  //[IN] num of rows to skip.
					DBROWCOUNT		cGetRows,		  //[IN] num of rows to fetch.
					DBCOUNTITEM		numFirstRowInSet, //[IN] num of the first row in the set of rows to be fetched.
					DBORDINAL		cColumns,		  //[IN] num of columns.
					DB_LORDINAL*	rgColumnsOrd,	  //[IN] list of column ordinals.
					IRowset*	pIRowset,		  //[IN] Pointer to IRowset.
					HACCESSOR	hAccessor,        //[IN] handle to accessor
					DBCOUNTITEM		cBindings,        //[IN] Number of bindings
					DBBINDING*	rgBindings,       //[IN] Binding structs
					DBLENGTH		cbRowSize,        //[IN] row size
					CTable*		pTable = NULL     //[IN] pointer to base table
					);

	//@cmember	Get the bookmark for the given row.
	BOOL		GetBookmark(
					DBCOUNTITEM	ulRow,         //[IN] row number for which to get bookmark (1 based)
					DBLENGTH	cbRowSize,     //[IN] row size
					HACCESSOR	hAccessor,     //[IN] handle to accessor
					DBCOUNTITEM	cBindings,     //[IN] number of bindings
					DBBINDING*	rgBindings,    //[IN] binding structs
					IUnknown*	pIUnkRowset,   //[IN] pointer to the rowset
					DBBKMARK	*pcbBookmark,  //[OUT] size of bookmark
					BYTE		**ppBookmark   //[OUT] value of bookmark
					);

	//@cmember	Check if the two DBCOLUMNDESC are equal.
	BOOL		CompareColumnDesc(
					DBCOLUMNDESC* pCD1,  //[IN] column description
					DBCOLUMNDESC* pCD2   //[IN] column description
					);

	//@cmember	Get the supported DBTYPEs from CTable's CCol structs.
	void		GetProviderTypes(CTable* pTable);

	//@cmember	Check if a given Type exists in CTable's CCol structs.
	BOOL		IsSupportedType(DBTYPE dwType);

	//@cmember	Check if the given index exists in the given table.
	TESTRESULT	DoesIndexExist(
					IDBSchemaRowset*	pIDSR,    //[IN] pointer to interface  
					DBID*				pTableID, //[IN] table ID
					DBID*				pIndexID, //[IN] Index ID
					BOOL*				pfExists  //[OUT] does index exist
					);

	//@cmember	Get the available optional columns through IColumnsRowset.
	HRESULT		GetAvailableColumns(
					IUnknown*	pUnk,			//[IN] Pointer to the object.
					DBORDINAL*	pcOptCols,		//[OUT] Number of optional columns.
					DBID**		prgOptCols		//[OUT] DBIDs of optional columns.
					);

	//@cmember	Check the list of optional metadata columns returned by 
	//			IColumnsRowset::GetAvailableColumns.
	BOOL		CheckOptColumns(
					const DBORDINAL	cOptCols,  //[IN] num of optional columns
					const DBID*	rgOptCols  //[IN] array of optional columns
					);

	//@cmember	Check the columns rowset returned by IColumnsRowset::
	//			GetColumnsRowset.
	BOOL		CheckColumnsRowset(
					IUnknown*	pIUnk,                //[IN] pointer to the columns rowset
					const DBORDINAL	cOptCols,             //[IN] num of optional columns
					const BOOL	bOrgRowsetHasBookmark //[IN] does the original rowset have bookmark column
					);

	//@cmember	Create an instance of the Root Binder, requesting 
	//			IBindResource. Then QI for IDBBinderProperties and 
	//			set the initialization properties.
	BOOL		CreateRootBinder();

	//@cmember	Test IDBProperties::Getproperties method.
	TESTRESULT	testGetProperties();

	//@cmember	Test IDBProperties::Getproperties method with (0,NULL).
	TESTRESULT	testGetAllProperties(IDBProperties* pIDBProp);

	//@cmember	Test IDBProperties::GetPropertyInfo method.
	TESTRESULT	testGetPropInfo();

	//@cmember	Test IDBProperties::GetPropertyInfo method with (0,NULL).
	TESTRESULT	testGetAllPropInfo(IDBProperties* pIDBProp);

	//@cmember	Test IDBProperties::SetProperties method (set to a valid
	//			value).
	TESTRESULT	testSetProperties1();
	
	//@cmember	Test IDBProperties::SetProperties method (set to VT_EMPTY)
	TESTRESULT	testSetProperties2();

	//@cmember	Test GetProperties and GetPropertyInfo while DSO is 
	//			uninitialized.
	TESTRESULT	testGetPropBeforeInit();

	//@cmember	Test IDBInfo::GetKeywords method.
	TESTRESULT	testGetKeywords();

	//@cmember	Test IDBInfo::GetLiteralInfo method.
	TESTRESULT	testGetLiteralInfo();

	//@cmember	Test ISourcesRowset interface.
	TESTRESULT	testISrcRowset();

	//@cmember	Test IDataInitialize::CreateDBInstance method.
	TESTRESULT	testIDataIzCreateDBIns();

	//@cmember	Test IDataInitialize::GetDataSource method.
	TESTRESULT	testIDataIzGetDS();

	//@cmember	Test IGetDataSource::GetDataSource method.
	TESTRESULT	testGetDataSource();

	//@cmember	Test IOpenRowset::OpenRowset method.
	TESTRESULT	testOpenRowset();

	//@cmember	Test ISessionProperties::GetProperties method.
	TESTRESULT	testSessGetProp();

	//@cmember	Test ISessionProperties::SetProperties method.
	TESTRESULT	testSessSetProp();

	//@cmember	Test IDBCreateCommand interface.
	TESTRESULT	testCreateCommand();

	//@cmember	Test IDBSchemaRowset::GetSchemas method.
	TESTRESULT	testGetSchemas();

	//@cmember	Test the COLUMNS rowset obtained by calling
	//			IDBSchemaRowset::GetRowset.
	TESTRESULT	testColumnsSchemaRowset();

	//@cmember	Test the TABLES rowset obtained by calling
	//			IDBSchemaRowset::GetRowset.
	TESTRESULT	testTablesSchemaRowset();

	//@cmember	Test the PROVIDER_TYPES rowset obtained by calling
	//			IDBSchemaRowset::GetRowset.
	TESTRESULT	testProvTypesSchemaRowset();

	//@cmember	Test ITableDefinition::AddColumn method.
	TESTRESULT	testAddColumn();

	//@cmember	Test ITableDefinition::CreateTable method.
	TESTRESULT	testCreateTable();

	//@cmember	Test ITableDefinition::DropColumn method.
	TESTRESULT	testDropColumn();

	//@cmember	Test ITableDefinition::DropTable method.
	TESTRESULT	testDropTable();

	//@cmember	Test IIndexDefinition methods.
	TESTRESULT	testIIndexDef();

	//@cmember	Test IAlterIndex::AlterIndex method.
	TESTRESULT	testAlterIndex();

	//@cmember	Test IAlterTable::AlterColumn method.
	TESTRESULT	testAlterColumn();

	//@cmember	Test IAlterTable::AlterTable method.
	TESTRESULT	testAlterTable();

	//@cmember	Test ITableDefinitionWithConstraints::AddConstraint & DropConstraint methods.
	TESTRESULT	testAddAndDropConstraint();

	//@cmember	Test ITableDefinitionWithConstraints::CreateTableWithConstraints method.
	TESTRESULT	testCreateTableWithConstraints();

	//@cmember	Test aggregation of Session.
	TESTRESULT	testAggregateSession(IDBCreateSession* pIDBCreateSession);

	//@cmember	Test IConvertType method.
	TESTRESULT	testIConvertType();

	//@cmember	Test IColumnsInfo::GetColumnInfo method.
	TESTRESULT	testGetColumnInfo(IUnknown* pIUnknown);

	//@cmember	Test IColumnsInfo::MapColumnIDs method.
	TESTRESULT	testMapColumnIDs(IUnknown* pIUnknown);

	//@cmember	Test the IAccessor interface methods.
	TESTRESULT	testIAccessor();

	//@cmember	Test the IRowsetInfo::GetProperties method
	//			or the ICommandProperties::GetProperties method.
	TESTRESULT	testRowsetGetProp(IUnknown* pIUnknown);

	//@cmember	Test the IRowsetInfo::GetReferencedRowset method
	TESTRESULT	testIRowsetInfoGetRefRowset();

	//@cmember	Test the IRowsetInfo::GetSpecification method on a
	//			rowset generated from Session.
	TESTRESULT	testIRowsetInfoGetSpec();

	//@cmember	Test the IRowsetInfo::GetSpecification method on a
	//			rowset generated from Command.
	TESTRESULT	testIRowsetInfoGetSpecCmd();

	//@cmember	Test the IRowset (No props set)
	TESTRESULT	testIRowset2();

	//@cmember	Test the IRowset (Scroll_Back and CANHOLDROWS set)
	TESTRESULT	testIRowset3();

	//@cmember	Test the IRowsetLocate::Compare
	TESTRESULT	testIRowsetLocateCompare();

	//@cmember	Test the IRowsetLocate::GetRowsAt
	TESTRESULT	testIRowsetLocateGetRowsAt();

	//@cmember	Test the IRowsetLocate::GetRowsByBookmark
	TESTRESULT	testIRowsetLocateGetRowsByBkm();

	//@cmember	Test the IRowsetLocate::Hash
	TESTRESULT	testIRowsetLocateHash();

	//@cmember	Test the IRowsetFind::FindNextRow
	TESTRESULT	testFindNextRow();

	//@cmember	Test the IRowsetScroll::GetApproximatePosition
	TESTRESULT	testGetApproxPos();

	//@cmember	Test the IRowsetScroll::GetRowsAtRatio
	TESTRESULT	testGetRowsAtRatio();

	//@cmember	Test the IRowPosition methods.
	TESTRESULT	testIRowPosition();

	//@cmember	Test aggregation of Rowset.
	TESTRESULT	testAggregateRowset();

	//@cmember	Test the IRowsetIdentity methods.
	TESTRESULT	testIRowsetIdentity();

	//@cmember	Test the IRowset methods (various properties set)
	TESTRESULT	testIRowset();

	//@cmember	Test the IRowsetChange::SetData method.
	TESTRESULT	testIRowsetChangeSet();

	//@cmember	Test the IRowsetChange::DeleteRows method.
	TESTRESULT	testIRowsetChangeDelete();

	//@cmember	Test the IRowsetChange::Insert method.
	TESTRESULT	testIRowsetChangeInsert();

	//@cmember	Test the IRowsetUpdate interface.
	TESTRESULT	testIRowsetUpdate();

	//@cmember	Test the IRowsetResynch methods.
	TESTRESULT	testIRowsetResynch();

	//@cmember	Test the IRowsetRefresh methods.
	TESTRESULT	testIRowsetRefresh();

	//@cmember	Test the IAccessor (Command interface) methods.
	TESTRESULT	testCmdIAccessor();

	//@cmember	Test the ICommandProperties::SetProperties method.
	TESTRESULT	testSetCmdProp();

	//@cmember	Test the ICommandText interface.
	TESTRESULT	testICmdText();

	//@cmember	Test the ICommand::GetDBSession method.
	TESTRESULT	testICmdGetDBSession();

	//@cmember	Test the ICommand::Execute method (set, exe, set, exe,
	//			set, exe).
	TESTRESULT	testICmdExec1();

	//@cmember	Test the ICommand::Execute method (set, prepare, exe,
	//			exe, exe).
	TESTRESULT	testICmdExec2();

	//@cmember	Test the ICommandPrepare::Prepare method.
	TESTRESULT	testCmdPrep();

	//@cmember	Test the ICommandPrepare::UnPrepare method.
	TESTRESULT	testCmdUnprep();

	//@cmember	Make the parameter bindings to be used in testICmdWParam.
	BOOL		MakeParamBindings(
					CTable*			pTable,
					DBORDINAL			cColumnsOnTable,
					DBORDINAL*			pcParamColMap,
					LONG_PTR*		rgParamColMap,
					DB_UPARAMS*			pcDbParamBindInfo,
					DBPARAMBINDINFO*	rgDbParamBindInfo,
					DB_UPARAMS*			rgParamOrdinals,
					DBCOLUMNINFO*	rgTableInfo);

	//@cmember	Test GetParameterInfo for ICommandWithParameters.
	TESTRESULT	testGetParamInfo(
					ICommandWithParameters* pICWP,
					CTable*					pTable,
					DB_UPARAMS				cDbParamBindInfo,
					DBPARAMBINDINFO*		rgDbParamBindInfo,
					LONG_PTR*				rgParamColMap);

	//@cmember	Test the ICommandWithParameters methods.
	TESTRESULT	testICmdWParam();

	//@cmember	Test the IMultipleResults.
	TESTRESULT	testIMultipleResults();

	//@cmember	Test the Error interfaces.
	TESTRESULT	testIError();

	//@cmember	Test transaction commit (fRetaining=FALSE).
	TESTRESULT	testTransactionCommit();

	//@cmember	Test transaction abort (fRetaining=FALSE).
	TESTRESULT	testTransactionAbort();

	//@cmember	Test the Storage Object (ISequentialStream).
	TESTRESULT	testStorageObj();

	//@cmember	Test the IRow methods.
	BOOL		testIRow(IRow* pIRow, DBCOUNTITEM ulRowNum=0);

	//@cmember	Test the IColumnsInfo methods on a Row object. If 
	//			bTestResColInfo is TRUE, then it also tests the 
	//			method GetRestrictedColumnInfo.
	BOOL		testColInfo2(
					CRowObject*		pCRowObj,
					BOOL			bTestResColInfo = FALSE);

	//@cmember	Test the IRowChange method SetColumns on a Row object. 
	BOOL		testIRowChange(CRowObject* pCRowObj);

	//@cmember	Test the IRowSchemaChange methods on a Row object. 
	BOOL		testIRowSchemaChange(CRowObject* pCRowObj);

};


////////////////////////////////////////////////////////////////////////
// CQuickTest Implementation
//
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// @mfunc Connection base class constructor, must take testcase name
//        as parameter.
//
CQuickTest::CQuickTest(LPWSTR wszName)	//Takes TestCase name as parameter
						: CSessionObject(wszName) 
{
	//Empty file so we know it hasn't been set yet
	m_wszFile[0]		= L'\0' ;
	m_szPath[0]			= '\0';
	m_cProviderTypes	= 0;
	m_rgProviderTypes	= NULL;
	m_pwszRowsetURL		= NULL;
	m_pCRowset			= NULL;

	m_ulSQLSup			= 0;
	m_ulMaxOpenRows		= 0;
	m_fFetchBackwards	= FALSE;

	m_cPropSets			= 0;  
	m_cPropInfoSets		= 0;
	m_cPropIDSets		= 0;
	m_rgPropSets		= NULL;
	m_rgPropInfoSets	= NULL;
	m_rgPropIDSets		= NULL;
	m_pIPersistFile		= NULL;
	m_pIDBProperties	= NULL;
	m_pIRowset			= NULL;
	m_pICommand			= NULL;
	m_pIBindResource	= NULL;
	m_pIRow				= NULL;
}


//----------------------------------------------------------------------
//@mfunc Destructor for CQuickTest class.
//
CQuickTest::~CQuickTest()
{
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	FreeProperties(&m_cPropInfoSets, &m_rgPropInfoSets);
	FreeProperties(&m_cPropIDSets, &m_rgPropIDSets);

	SAFE_FREE(m_rgProviderTypes);
	SAFE_FREE(m_pwszRowsetURL);

	//Release here if we haven't before		
	SAFE_RELEASE(m_pIPersistFile);
	SAFE_RELEASE(m_pIDBProperties);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIBindResource);
	SAFE_RELEASE(m_pIRow);
}


//--------------------------------------------------------------------
// @mfunc Find the current working directory and use it to set
// the path and file name for persisting files. Then attempt to
// save to that file. An instance of CLocaleInfo is created temporarily,
// only to use its member function 'MakeUnicodeIntlString'.
//
HRESULT	CQuickTest::PersistToFile()
{
	WCHAR			wszSaveFile[2*PATH_SIZE];
	HRESULT			hr = E_FAIL;
	CLocaleInfo*	pCLI=NULL;

	m_szPath[0] = '\0';

	//If file is set, we've saved already, so just return NO_ERROR.
	if (m_wszFile[0] != L'\0')
		return  NO_ERROR;

	//Get current working directory to be used for creating files
	if(!_getcwd(m_szPath,PATH_SIZE))
	{
		odtLog << wszErrorFindingCurrentPath;
		return  ResultFromScode(E_FAIL);
	}

	//Build whole file path and name
	MultiByteToWideChar(CP_ACP,0,(LPCSTR)m_szPath,-1,m_wszFile,PATH_SIZE);
	
	//For International strings.
	BOOL	fIntl = FindIntlSetting();

	if(!fIntl)
		wcscat(m_wszFile,L"\\QuikTest long file name.tst");
	else
	{
		pCLI = new CLocaleInfo(GetUserDefaultLCID());
		if(pCLI==NULL) goto CLEANUP;
		pCLI->MakeUnicodeIntlString(wszSaveFile, 16);
		wcscat(m_wszFile, L"\\");
		wcscat(m_wszFile, wszSaveFile);
		wcscat(m_wszFile, L".tst");
	}

	//Save to the specified file.
	hr = m_pIPersistFile->Save((LPOLESTR)m_wszFile,TRUE);
	if(hr != S_OK)
		m_wszFile[0] = L'\0';

CLEANUP:
	SAFE_DELETE(pCLI)
	return hr;	
} //PersistToFile


//-----------------------------------------------------------------------
//@mfunc Append another DBPROPINFO and/or DBPROPINFOSET to m_rgPropInfoSets
//and increment the counter m_cPropInfoSets. The variant is left empty
//and dwFlags is initalized to NOT_SUPPORTED.
//
HRESULT CQuickTest::SetPropInfo( 
	DBPROPID		propID,       //[IN] Property ID
	const GUID&		guidPropSet,  //[IN] Property Set
	VARTYPE			varType		  //[IN] Variant type
	)
{
	ASSERT(propID);

	ULONG cPropertyInfos = 0;
	DBPROPINFO* rgPropertyInfos = NULL;

	ULONG cPropInfoSets = m_cPropInfoSets;
	DBPROPINFOSET* rgPropInfoSets = m_rgPropInfoSets;

	//Find the propertyInfo Set in which to insert the new DBPROPINFO.
	ULONG iPropInfoSet = ULONG_MAX;
	for(ULONG i=0; i<cPropInfoSets; i++)
		if(guidPropSet == rgPropInfoSets[i].guidPropertySet)
			iPropInfoSet = i;

	//If propertyInfo Set is not found, create one.
	if(iPropInfoSet == ULONG_MAX)
	{
		iPropInfoSet = cPropInfoSets;
		SAFE_REALLOC(rgPropInfoSets, DBPROPINFOSET, cPropInfoSets+1);
		rgPropInfoSets[iPropInfoSet].cPropertyInfos = 0;
		rgPropInfoSets[iPropInfoSet].rgPropertyInfos = NULL;
		rgPropInfoSets[iPropInfoSet].guidPropertySet = guidPropSet;
		cPropInfoSets++;
	}

	//Fill values into DBPROPINFOSET.
	cPropertyInfos = rgPropInfoSets[iPropInfoSet].cPropertyInfos;
	rgPropertyInfos = rgPropInfoSets[iPropInfoSet].rgPropertyInfos;

	SAFE_REALLOC(rgPropertyInfos, DBPROPINFO, cPropertyInfos+1);

	//Fill values into DBPROPINFO.
	rgPropertyInfos[cPropertyInfos].pwszDescription = L"\0";
	rgPropertyInfos[cPropertyInfos].dwPropertyID = propID;
	rgPropertyInfos[cPropertyInfos].vtType = varType;
	VariantInit(&(rgPropertyInfos[cPropertyInfos].vValues));
	rgPropertyInfos[cPropertyInfos].dwFlags = DBPROPFLAGS_NOTSUPPORTED;

	cPropertyInfos++;

CLEANUP:
	rgPropInfoSets[iPropInfoSet].cPropertyInfos = cPropertyInfos;
	rgPropInfoSets[iPropInfoSet].rgPropertyInfos = rgPropertyInfos;
	m_cPropInfoSets = cPropInfoSets;
	m_rgPropInfoSets = rgPropInfoSets;
	return S_OK;
} //SetPropInfo


//----------------------------------------------------------------------
//@mfunc Append another DBPROPID and/or DBPROPIDSET to m_rgPropIDSets 
//and increment the counter m_cPropIDSets.
//
HRESULT CQuickTest::SetPropID( 
	DBPROPID		propID,       //[IN] Property ID
	const GUID&		guidPropSet   //[IN] Property Set
	)   
{
	ASSERT(propID);

	ULONG cPropertyIDs = 0;
	DBPROPID* rgPropertyIDs = NULL;

	ULONG cPropIDSets = m_cPropIDSets;
	DBPROPIDSET* rgPropIDSets = m_rgPropIDSets;

	//Find the propertyID Set in which to insert the new DBPROPID.
	ULONG iPropIDSet = ULONG_MAX;
	for(ULONG i=0; i<cPropIDSets; i++)
		if(guidPropSet == rgPropIDSets[i].guidPropertySet)
			iPropIDSet = i;

	//If propertyID Set is not found, create one.
	if(iPropIDSet == ULONG_MAX)
	{
		iPropIDSet = cPropIDSets;
		SAFE_REALLOC(rgPropIDSets, DBPROPIDSET, cPropIDSets+1);
		rgPropIDSets[iPropIDSet].cPropertyIDs = 0;
		rgPropIDSets[iPropIDSet].rgPropertyIDs = NULL;
		rgPropIDSets[iPropIDSet].guidPropertySet = guidPropSet;
		cPropIDSets++;
	}

	//Fill values into DBPROPIDSET.
	cPropertyIDs = rgPropIDSets[iPropIDSet].cPropertyIDs;
	rgPropertyIDs = rgPropIDSets[iPropIDSet].rgPropertyIDs;

	SAFE_REALLOC(rgPropertyIDs, DBPROPID, cPropertyIDs+1);

	//Fill in DBPROPID.
	rgPropertyIDs[cPropertyIDs] = propID;

	cPropertyIDs++;

CLEANUP:
	rgPropIDSets[iPropIDSet].cPropertyIDs = cPropertyIDs;
	rgPropIDSets[iPropIDSet].rgPropertyIDs = rgPropertyIDs;
	m_cPropIDSets = cPropIDSets;
	m_rgPropIDSets = rgPropIDSets;
	return S_OK;
} //SetPropID


//-----------------------------------------------------------------------
//@mfunc Initialize m_rgPropIDSets and m_rgPropInfoSets with some chosen 
//properties. For this it uses the function calls to SetPropID and 
//SetPropInfo. Properties may be added or removed from m_rgPropIDSets 
//and m_rgPropInfoSets by adding or removing calls to SetPropID and 
//SetPropInfo below. 
//
void CQuickTest::InitPropIDStructs()
{
	m_cPropInfoSets = 0;
	m_cPropIDSets = 0;
	m_rgPropInfoSets = NULL;
	m_rgPropIDSets = NULL;

//DBPROPSET_DATASOURCEINFO

	SetPropInfo(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_CATALOGLOCATION, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_CATALOGLOCATION, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_COLUMNDEFINITION, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_COLUMNDEFINITION, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_DATASOURCENAME, DBPROPSET_DATASOURCEINFO, VT_BSTR);
	SetPropID(DBPROP_DATASOURCENAME, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, VT_BOOL);
	SetPropID(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, VT_BSTR);
	SetPropID(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_DBMSVER, DBPROPSET_DATASOURCEINFO, VT_BSTR);
	SetPropID(DBPROP_DBMSVER, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_IDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_IDENTIFIERCASE, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_MAXROWSIZE, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_MAXROWSIZE, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_MAXTABLESINSELECT, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_MAXTABLESINSELECT, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, VT_BSTR);
	SetPropID(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_QUOTEDIDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_QUOTEDIDENTIFIERCASE, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_ROWSETCONVERSIONSONCOMMAND, DBPROPSET_DATASOURCEINFO, VT_BOOL);
	SetPropID(DBPROP_ROWSETCONVERSIONSONCOMMAND, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, VT_I4);
	SetPropID(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO);

	SetPropInfo(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO, VT_BSTR);
	SetPropID(DBPROP_USERNAME, DBPROPSET_DATASOURCEINFO);

//DBPROPSET_DBINIT

	SetPropInfo(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, VT_BSTR);
	SetPropID(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_AUTH_USERID, DBPROPSET_DBINIT, VT_BSTR);
	SetPropID(DBPROP_AUTH_USERID, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, VT_BSTR);
	SetPropID(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_LCID, DBPROPSET_DBINIT, VT_I4);
	SetPropID(DBPROP_INIT_LCID, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_LOCATION, DBPROPSET_DBINIT, VT_BSTR);
	SetPropID(DBPROP_INIT_LOCATION, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_MODE, DBPROPSET_DBINIT, VT_I4);
	SetPropID(DBPROP_INIT_MODE, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, VT_I2);
	SetPropID(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, VT_BSTR);
	SetPropID(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT);

	SetPropInfo(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, VT_I4);
	SetPropID(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT);

//DBPROPSET_ROWSET

	SetPropInfo(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, VT_BOOL);

	return;
} //InitPropIDStructs


//----------------------------------------------------------------------
//@mfunc Set the flag of the given property in m_rgPropInfoSets to the 
//given value. If the property was not found in m_rgPropInfoSets, then
//return FALSE. If the property was found and the flag was set, return
//TRUE.
//
BOOL CQuickTest::SetFlags(
	DBPROPID		propID,       //[IN] Property ID
	const GUID&		guidPropSet,  //[IN] Property Set
	DBPROPFLAGS		dwFlags		  //[IN] Prop Flags
	)
{
	ULONG	iSet = 0;
	ULONG	iInfo = 0;

	for(iSet=0; iSet<m_cPropInfoSets; iSet++)
	{
		if(m_rgPropInfoSets[iSet].guidPropertySet == guidPropSet)
		{
			for(iInfo=0; iInfo<m_rgPropInfoSets[iSet].cPropertyInfos; iInfo++)
			{
				if(m_rgPropInfoSets[iSet].rgPropertyInfos[iInfo].dwPropertyID ==
					propID)
				{
					m_rgPropInfoSets[iSet].rgPropertyInfos[iInfo].dwFlags =
						dwFlags;

					return TRUE;
				}
			}
		}
	}

	return FALSE;
} //SetFlags


//----------------------------------------------------------------------
//@mfunc Call GetPropertyInfo and then initalize the property flags in
//m_rgPropInfoSets to that of those obtained through the GetPropertyInfo
//call. This can be used later to verify if the property is supported.
//The function SetFlags is called to locate the property in 
//m_rgPropInfoSets and set the flag. All the properties in this function
//have been added to m_rgPropInfoSets in the function InitPropIDStructs.
//
void CQuickTest::InitSupInfo()
{
	ULONG			iSet=0, iInfo=0;
	ULONG			cPropInfoSets = 0;
	DBPROPINFO*		pInfo = NULL;
	DBPROPINFOSET*	pSet = NULL;
	DBPROPINFOSET*	rgPropInfoSets = NULL;

	if(m_pIDBProperties==NULL)
		goto CLEANUP;
	if(!CHECK(m_pIDBProperties->GetPropertyInfo(0,NULL,&cPropInfoSets,
		&rgPropInfoSets,NULL), S_OK))
		goto CLEANUP;

	//At least CANHOLDROWS has to be supported.
	TESTC(cPropInfoSets !=0) 
	TESTC(rgPropInfoSets !=NULL)

	for(iSet=0; iSet<cPropInfoSets; iSet++)
	{
		pSet = &(rgPropInfoSets[iSet]) ;

		for(iInfo=0; iInfo<pSet->cPropertyInfos; iInfo++)
		{
			if(pSet->guidPropertySet==DBPROPSET_DATASOURCEINFO)
			{
				pInfo = &(pSet->rgPropertyInfos[iInfo]) ;

				switch(pInfo->dwPropertyID)
				{
				case DBPROP_ACTIVESESSIONS:
					COMPARE(SetFlags(DBPROP_ACTIVESESSIONS, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_CATALOGLOCATION:
					COMPARE(SetFlags(DBPROP_CATALOGLOCATION, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_COLUMNDEFINITION:
					COMPARE(SetFlags(DBPROP_COLUMNDEFINITION, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_DATASOURCENAME:
					COMPARE(SetFlags(DBPROP_DATASOURCENAME, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_DATASOURCEREADONLY:
					COMPARE(SetFlags(DBPROP_DATASOURCEREADONLY, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_DBMSNAME:
					COMPARE(SetFlags(DBPROP_DBMSNAME, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_DBMSVER:
					COMPARE(SetFlags(DBPROP_DBMSVER, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_IDENTIFIERCASE:
					COMPARE(SetFlags(DBPROP_IDENTIFIERCASE, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_MAXROWSIZE:
					COMPARE(SetFlags(DBPROP_MAXROWSIZE, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_MAXTABLESINSELECT:
					COMPARE(SetFlags(DBPROP_MAXTABLESINSELECT, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_PROVIDERNAME:
					COMPARE(SetFlags(DBPROP_PROVIDERNAME, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_QUOTEDIDENTIFIERCASE:
					COMPARE(SetFlags(DBPROP_QUOTEDIDENTIFIERCASE, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_ROWSETCONVERSIONSONCOMMAND:
					COMPARE(SetFlags(DBPROP_ROWSETCONVERSIONSONCOMMAND, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_SQLSUPPORT:
					COMPARE(SetFlags(DBPROP_SQLSUPPORT, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_USERNAME:
					COMPARE(SetFlags(DBPROP_USERNAME, 
						DBPROPSET_DATASOURCEINFO, pInfo->dwFlags),TRUE);
					break;
				default:
					break;
				}//switch
			}
			else if(pSet->guidPropertySet==DBPROPSET_DBINIT)
			{
				pInfo = &(pSet->rgPropertyInfos[iInfo]) ;

				switch(pInfo->dwPropertyID)
				{
				case DBPROP_AUTH_PASSWORD:
					COMPARE(SetFlags(DBPROP_AUTH_PASSWORD, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_AUTH_USERID: 
					COMPARE(SetFlags(DBPROP_AUTH_USERID, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_DATASOURCE:
					COMPARE(SetFlags(DBPROP_INIT_DATASOURCE, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_LCID:
					COMPARE(SetFlags(DBPROP_INIT_LCID, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_LOCATION:
					COMPARE(SetFlags(DBPROP_INIT_LOCATION, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_MODE:
					COMPARE(SetFlags(DBPROP_INIT_MODE, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_PROMPT: 
					COMPARE(SetFlags(DBPROP_INIT_PROMPT, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_PROVIDERSTRING:
					COMPARE(SetFlags(DBPROP_INIT_PROVIDERSTRING, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				case DBPROP_INIT_TIMEOUT:
					COMPARE(SetFlags(DBPROP_INIT_TIMEOUT, 
						DBPROPSET_DBINIT, pInfo->dwFlags),TRUE);
					break;
				default:
					break;
				}//switch
			}
			else if(pSet->guidPropertySet==DBPROPSET_ROWSET)
			{
				pInfo = &(pSet->rgPropertyInfos[iInfo]) ;

				switch(pInfo->dwPropertyID)
				{
				case DBPROP_CANHOLDROWS:
					COMPARE(SetFlags(DBPROP_CANHOLDROWS, 
						DBPROPSET_ROWSET, pInfo->dwFlags),TRUE);
					break;
				default:
					break;
				}//switch
			}
		}
	}

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets);
	return;
} //InitSupInfo


//----------------------------------------------------------------------
//@mfunc Verify that the VT_I2 variant has a non-negative value.
//
BOOL CQuickTest::checkPositive_I2(VARIANT* pv)
{
	if((V_VT(pv) == VT_I2) && (V_I2(pv) >= 0))
		return TRUE;
	else
		return FALSE;
} //checkPositive_I2


//----------------------------------------------------------------------
//@mfunc Wrapper for IRowset::GetNextRows.
//
HRESULT CQuickTest::GetNextRows(
	IRowset*		pIRowset,		//[IN] Pointer to rowset			
	HCHAPTER*		hChapter,		//[IN] Handle to chapter
	DBROWOFFSET		lRowsOffset,	//[IN] Offset to row
	DBROWCOUNT		cRows,			//[IN] Number of rows to fetch
	DBCOUNTITEM*	pcRowsObtained,	//[OUT] Number of rows fetched
	HROW**			prghRows		//[OUT] Handle to fetched rows.
	)
{
	DBCOUNTITEM cRowsObtained = NO_ROWS;
	DBCOUNTITEM i=0;
	HROW* rghRowsInput = NULL;
	HRESULT hr = S_OK;
	HRESULT hrRet = E_FAIL;

	TESTC(prghRows != NULL)
	TESTC(pcRowsObtained != NULL)
	TESTC(pIRowset != NULL)
	
	//Record if we passed in consumer allocated array...
	rghRowsInput = *prghRows;

	//Fetch the rows.
	hr = pIRowset->GetNextRows(NULL, lRowsOffset, cRows, &cRowsObtained, prghRows);
	
	//Verify correct values are returned
	if(hr == S_OK)
	{
		TESTC(cRowsObtained == (DBCOUNTITEM) ABS(cRows));

		//Verify row array
		for(i=0; i<cRowsObtained; i++)
		{
			TESTC(*prghRows != NULL);
			TESTC((*prghRows)[i]!=DB_NULL_HROW)
		}
	}
	else
	{
		TESTC(cRowsObtained <= (DBCOUNTITEM) ABS(cRows));
		if(cRowsObtained>0)
		{
			TESTC(*prghRows != NULL)
			TESTC(**prghRows != DB_NULL_HROW)
		}
	}

	//Verify output array, depending upon whether it is consumer or 
	//provider allocated...
	if(rghRowsInput)
	{
		//This is a user allocated static array.
		//This had better not be nulled out by the provider, if non-null
		//on input.
		TESTC(*prghRows == rghRowsInput);
	}
	else
	{
		TESTC(cRowsObtained ? *prghRows != NULL : *prghRows == NULL);
	}

	hrRet = hr;

CLEANUP:
	if(pcRowsObtained)
		*pcRowsObtained = cRowsObtained;
	return hrRet;
} //GetNextRows


//----------------------------------------------------------------------
//@mfunc Wrapper for IRowset::ReleaseRows.
//
HRESULT CQuickTest::ReleaseRows(
	IRowset**		ppIRowset,	//[IN] Pointer to rowset
	DBCOUNTITEM*	pcRows,		//[IN] Num of rows to release
	HROW**			prghRows	//[IN] HROW of rows to release
	)
{
	HRESULT hr=S_OK;

	if(ppIRowset==NULL || pcRows==NULL || prghRows==NULL)
		return E_FAIL;

	if(*ppIRowset && *prghRows)
		hr = (*ppIRowset)->ReleaseRows(*pcRows, *prghRows, NULL, NULL, 
		NULL);
	*pcRows = 0;
	PROVIDER_FREE(*prghRows);  //*prghRows = NULL

	return hr;
} //ReleaseRows


//----------------------------------------------------------------------
//@mfunc Wrapper for IRowset::RestartPosition.
//
HRESULT CQuickTest::RestartPosition(IRowset* pIRowset)
{
	HRESULT hr = E_FAIL;

	TESTC(pIRowset != NULL)
	hr = pIRowset->RestartPosition(NULL);

	//Mask the hr.
	if(hr==DB_S_COMMANDREEXECUTED)
	{
		hr = S_OK;
		odtLog<<L"INFO: Got DB_S_COMMANDREEXECUTED for IRowset::RestartPosition.\n";
	}

CLEANUP:
	return hr;
} //RestartPosition


//----------------------------------------------------------------------
//@mfunc Release the hAccessor and free the bindings. If bReleaseIAcc is
//TRUE(this is the default), then release IAccessor.
//
HRESULT	CQuickTest::FreeAccessorAndBindings(
	IAccessor**		ppIAccessor,	//[IN] Pointer to IAccessor
	HACCESSOR*		phAccessor,		//[IN] Handle to accessor
	DBCOUNTITEM*	pcBindings,		//[IN] Number of bindings
	DBBINDING**		prgBindings,	//[IN] Binding Structs.
	BOOL			bReleaseIAcc    //[IN] Release IAccessor ?
	)
{
	HRESULT hr;

	if(!ppIAccessor || !phAccessor || !pcBindings || !prgBindings)
		return E_FAIL;

	if(*ppIAccessor && *phAccessor)
		hr = (*ppIAccessor)->ReleaseAccessor(*phAccessor, NULL);
	*phAccessor = DB_NULL_HACCESSOR;
	hr = FreeAccessorBindings(*pcBindings, *prgBindings);
	*pcBindings = 0;
	*prgBindings = NULL;
	if(bReleaseIAcc)
		SAFE_RELEASE(*ppIAccessor);
	return hr;
} //FreeAccessorAndBindings


//----------------------------------------------------------------------
//@mfunc Appends a property and its value to the Init String. Returns a
//NULL if an error occurs.
//
HRESULT CQuickTest::AppendToInitString(WCHAR** ppwszInitString, DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT* pVariant)
{
	TBEGIN
	ASSERT(ppwszInitString);
	WCHAR wszBuffer[MAX_QUERY_LEN+100];
	wszBuffer[0] = L'\0';
	WCHAR* pwszNewString = NULL;
	HRESULT hr = S_OK;

	//Now get the Keyword and Value
	WCHAR* pwszKeyword	= GetPropDesc(dwPropertyID, guidPropertySet, m_pIDBInitialize);
	TESTC_(hr = VariantToString(pVariant, wszBuffer, MAX_QUERY_LEN+100, FALSE),S_OK);

	//Don't neccessaryly need a value, "keyword=;", but we
	//do need a keyword.  This should map to a non-NULL property description
	TESTC(pwszKeyword != NULL);

	//Add this to the String
	pwszNewString = CreateString(L"%s%s =", *ppwszInitString ? L";" : L"", pwszKeyword);
	hr = AppendString(ppwszInitString, pwszNewString);
	SAFE_FREE(pwszNewString);

	//We might need to quote the items if there is an embedded semi
	if(wcschr(wszBuffer, L';'))
		pwszNewString = CreateString(L"\"%s\"", wszBuffer);
	else
		pwszNewString = CreateString(L"%s", wszBuffer);
	hr = AppendString(ppwszInitString, pwszNewString);

CLEANUP:
	SAFE_FREE(pwszKeyword);
	SAFE_FREE(pwszNewString);
	return hr;
} //AppendToInitString


//----------------------------------------------------------------------
//@mfunc Call OpenRowset and verify the returned IRowset. This method 
//implicity uses m_cPropSets/m_rgPropSets. So if you want particular
//properties set, just simply set them before calling this method.
//
HRESULT CQuickTest::CreateOpenRowset(
	CTable*		pTable,     //[IN] Table on which to open the rowset.
	const IID&	riid,       //[IN] IID of requested interface on rowset.
	IUnknown**	ppRowset,   //[OUT] Pointer to rowset interface.
	IUnknown*	pUnkOuter   //[IN] For aggregation.
	)
{
	HRESULT hr = E_FAIL;
	DBID*	pTableID = NULL;

	TESTC(pTable != NULL)
	TESTC(m_pIOpenRowset != NULL)

	pTableID = &(pTable->GetTableIDRef()) ;
    //Call OpenRowset  
	hr = m_pIOpenRowset->OpenRowset(pUnkOuter,pTableID,NULL,
		riid,m_cPropSets,m_rgPropSets,ppRowset);
    
    if(hr==S_OK || hr==DB_S_ERRORSOCCURRED)
    {
        //Verify Usable Rowset
        if(ppRowset && riid!=IID_NULL)  
        {  
            TESTC(*ppRowset!=NULL)
        } 
        
        //Verify no errors
        if(hr==S_OK) 
		{
			TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, 
				DBPROPSTATUS_OK))
		}

		if(riid==IID_NULL)
		{
			//Verify no rowset returned
			if(ppRowset) 
				TESTC(*ppRowset==NULL)
		}
    }
	else  //OpenRowset failed.
	{
		//Verify no rowset returned
        if(ppRowset) 
            TESTC(*ppRowset==NULL)
	}

CLEANUP:
	return hr;
} //CreateOpenRowset


//----------------------------------------------------------------------
// @mfunc This a test for IAccessor::GetBindings method. It checks if
// GetBindings returns the same bindings array as the ones used to create
// the accessor.
//
BOOL CQuickTest::VerifyBindings(
	IAccessor*		pIAccessor,           //[IN] Pointer to IAccessor. 
	HACCESSOR		hAccessor,            //[IN] handle to accessor
	DBCOUNTITEM		cBindings,            //[IN] Number of binding structs
	DBBINDING*		rgBindings,           //[IN] Binding structures
	DBACCESSORFLAGS dwCreateAccessorFlags //[IN] Value of Accessor flags used to create accessor
	)
{
	BOOL			bSuccess = FALSE;
	DBCOUNTITEM		cGetBindings = 0;
	DBCOUNTITEM		ulIndex=0;
	DBBINDING*		pGetBind = NULL;
	DBBINDING*		pBind = NULL;
	DBBINDING*		rgGetBindings = NULL;
	DBACCESSORFLAGS	dwGetAccessorFlags = 0;

	TESTC(pIAccessor != NULL)

	//Obtained the bindings.
	TESTC_(pIAccessor->GetBindings(hAccessor, &dwGetAccessorFlags, 
		&cGetBindings, &rgGetBindings),S_OK)

	//Verify the Accessor Flags and number of bindings.
	COMPARE(dwGetAccessorFlags , dwCreateAccessorFlags);
	COMPARE(cGetBindings , cBindings);

	//Verify the binding stuctures.
	if (cGetBindings == 0)
	{
		//This is a null accessor, binding array should be null
		TESTC(rgGetBindings == NULL)
	}
	else
	{
		TESTC(cGetBindings != 0)
		TESTC(rgGetBindings != NULL)

		for(ulIndex=0; ulIndex<cGetBindings; ulIndex++)
		{
			pGetBind = &(rgGetBindings[ulIndex]);
			pBind = &(rgBindings[ulIndex]);

			COMPARE(pGetBind->dwPart , pBind->dwPart);
			COMPARE(pGetBind->iOrdinal , pBind->iOrdinal);
			COMPARE(pGetBind->wType , pBind->wType);
			COMPARE(pGetBind->eParamIO , pBind->eParamIO);				
			COMPARE(pGetBind->pTypeInfo , pBind->pTypeInfo);
							
			//Precision and scale only apply for numeric and decimal 
			//types and then only if VALUE is bound.
			if ((pBind->wType == DBTYPE_NUMERIC || 
				pBind->wType == DBTYPE_DECIMAL ||
				pBind->wType == DBTYPE_DBTIMESTAMP) &&
				(pBind->dwPart & DBPART_VALUE))
			{				
				COMPARE(pGetBind->bPrecision , 
					pBind->bPrecision);
				COMPARE(pGetBind->bScale , pBind->bScale);
			}
			
			//These only apply if value is bound
			if (pBind->dwPart & DBPART_VALUE)
			{
				COMPARE(pGetBind->obValue , pBind->obValue);					
				COMPARE(pGetBind->cbMaxLen , pBind->cbMaxLen);
			}
			
			//These only apply if type is DBTYPE_UNKNOWN				
			if ((pBind->wType == DBTYPE_IUNKNOWN) &&
				(pGetBind->pObject != NULL))
			{
				COMPARE(pGetBind->pObject->dwFlags , 
					pBind->pObject->dwFlags)	;					
				COMPARE(pGetBind->pObject->iid , 
					pBind->pObject->iid);
			}

			//These only apply if length is bound
			if (pBind->dwPart & DBPART_LENGTH)
				COMPARE(pGetBind->obLength , pBind->obLength);
			
			//These only apply if status is bound
			if (pBind->dwPart & DBPART_STATUS)
				COMPARE(pGetBind->obStatus , pBind->obStatus);
		}//For Loop
	}

	//If we get here, we must have  passed
	bSuccess = TRUE;

CLEANUP:
	FreeAccessorBindings(cGetBindings, rgGetBindings);
	return bSuccess;
} //VerifyBindings


//-----------------------------------------------------------------------
//@mfunc Get the specified rows. Obtain their data and verify it using the
//function CompareData. Then release all the obtained rows. It is left to 
//the caller of this function to make sure that the rows asked for exist
//and check (and set) the values of properties DBPROP_CANHOLDROWS and 
//DBPROP_MAXOPENROWS if required. This function does not call 
//RestartPosition.
//
BOOL CQuickTest::GetDataAndCompare(
	DBROWOFFSET	cSkipRows,		  //[IN] num of rows to skip.
	DBROWCOUNT	cGetRows,		  //[IN] num of rows to fetch.
	DBCOUNTITEM	numFirstRowInSet, //[IN] num of the first row in the set of rows to be fetched.
	DBORDINAL	cColumns,		  //[IN] num of columns.
	DB_LORDINAL*	rgColumnsOrd,	  //[IN] list of column ordinals.
	IRowset*	pIRowset,		  //[IN] Pointer to IRowset.
	HACCESSOR	hAccessor,        //[IN] handle to accessor
	DBCOUNTITEM	cBindings,        //[IN] Number of bindings
	DBBINDING*	rgBindings,       //[IN] Binding structs
	DBLENGTH	cbRowSize,        //[IN] row size
	CTable*		pTable            //[IN] pointer to base table
	)
{
	BOOL		bSuccess = FALSE;
	DBCOUNTITEM	ulIndex = 0;
	DBCOUNTITEM	rowNum = 0;	  //Row number of row to be compared.
	DBCOUNTITEM	cRowsObtained = 0;
	HRESULT		hr = S_OK;
	BYTE*		pData = NULL;
	HROW*		rghRows = NULL;

	TESTC(pIRowset != NULL)

	if(pTable == NULL)
		pTable = m_pTable;

	TESTC(pTable != NULL)

	//Use IRowset to retrieve data
	TESTC_(GetNextRows(pIRowset, NULL, cSkipRows, cGetRows, 
		&cRowsObtained, (HROW **)&rghRows), S_OK)

	TESTC(cRowsObtained == (DBCOUNTITEM) ABS(cGetRows));

	//Allocate a new data buffer. 
	SAFE_ALLOC(pData, BYTE, cbRowSize)

	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
	{
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		if(!CHECK(hr = pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData),S_OK))
			continue;

		//adjust the rowNum according to the fetch direction.
		if(cGetRows >= 0)
			rowNum = numFirstRowInSet+ulIndex;
		else
			rowNum = numFirstRowInSet-ulIndex;

		//Verify data value, length and status are what is expected.
		TESTC(CompareData(cColumns, rgColumnsOrd, rowNum,
			pData, cBindings, rgBindings, pTable,
			m_pIMalloc, PRIMARY, COMPARE_ONLY))

		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings,
			pData), S_OK);
	}

	bSuccess = TRUE;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	return bSuccess;
} //GetDataAndCompare


//----------------------------------------------------------------------
//@mfunc Get the bookmark for a given row number. Row nums are 1 based.
//Memory allocated for bookmark is freed by caller of this function.
//
BOOL CQuickTest::GetBookmark(
	DBCOUNTITEM		ulRow,         //[IN] row number for which to get bookmark (1 based)
	DBLENGTH		cbRowSize,     //[IN] row size
	HACCESSOR		hAccessor,     //[IN] handle to accessor
	DBCOUNTITEM		cBindings,     //[IN] number of bindings
	DBBINDING*		rgBindings,    //[IN] binding structs
	IUnknown*		pIUnkRowset,   //[IN] pointer to the rowset
	DBBKMARK		*pcbBookmark,  //[OUT] size of bookmark
	BYTE			**ppBookmark   //[OUT] value of bookmark
	)
{
	BOOL		bSuccess = FALSE;
	HRESULT		hr = S_OK;
	HROW*		rghRows = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	DBBKMARK	dwAddr = 0;
	IRowset*	pIRowset = NULL;
	BYTE*		pData = NULL;

	if(!pcbBookmark || !ppBookmark)
		return FALSE;

	//Row numbers are 1 based.
	if(ulRow < 1)
		return FALSE;

	//VerifyInterface
	if(!VerifyInterface(pIUnkRowset,IID_IRowset,
		ROWSET_INTERFACE,(IUnknown**)&pIRowset))
	{
		odtLog<<L"INFO: Could not get IRowset interface.\n";
		return FALSE;
	}

	//Allocate a new data buffer. 
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	//restart the cursor position
	TESTC_(RestartPosition(pIRowset), S_OK)

	//fetch the row
	TESTC_(GetNextRows(pIRowset, NULL,(ulRow-1),1,&cRowsObtained,&rghRows),S_OK)

	//get the data
	TESTC_(pIRowset->GetData(*rghRows, hAccessor, pData),S_OK)

	//make sure the 0 column is for bookmark
	TESTC(rgBindings[0].iOrdinal == 0)

	//get the starting pointer of the consumer's buffer
	dwAddr = (DBBKMARK) pData;

	//get the length of the bookmark
	*pcbBookmark = *((DBBKMARK *)(dwAddr+rgBindings[0].obLength));

	//allocate memory for bookmark
	SAFE_ALLOC(*ppBookmark, BYTE, *pcbBookmark);

	if(!(*ppBookmark))
		goto CLEANUP;

	//copy the value of the bookmark into the consumer's buffer
	memcpy(*ppBookmark, (void *)(dwAddr+rgBindings[0].obValue), (size_t)*pcbBookmark);

	bSuccess=TRUE;

CLEANUP:
	if(!bSuccess)
		odtLog<<L"INFO: Could not get bookmark for row "<<ulRow<<L".\n";
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	//restart the cursor position
	if(!CHECK(RestartPosition(pIRowset), S_OK))
		bSuccess = FALSE;
	SAFE_RELEASE(pIRowset);
	return bSuccess;
} //GetBookmark


//----------------------------------------------------------------------
//@mfunc Compare 2 column descriptions.
//
BOOL CQuickTest::CompareColumnDesc(
	DBCOLUMNDESC* oldCD,   //[IN] column description
	DBCOLUMNDESC* newCD    //[IN] column description
	)
{
	ULONG		cPropSet, cProp;
	BOOL		bSuccess = FALSE;
	DBPROPSET	*pNewPropSet = NULL;
	DBPROPSET	*pOldPropSet = NULL;
	DBPROP		*pNewProp = NULL;
	DBPROP		*pOldProp = NULL;

	//compare everything in the structure
	TESTC(oldCD->pTypeInfo == newCD->pTypeInfo);
	TESTC(oldCD->ulColumnSize == newCD->ulColumnSize);
	TESTC(oldCD->wType == newCD->wType);
	TESTC(!IsNumericType(oldCD->wType) || oldCD->bPrecision == newCD->bPrecision);
	TESTC((oldCD->wType != DBTYPE_NUMERIC && oldCD->wType != DBTYPE_DECIMAL) 
			|| oldCD->bScale == newCD->bScale);
	TESTC(CompareDBID(oldCD->dbcid, newCD->dbcid));
	
	//compare the properties
	TESTC(oldCD->cPropertySets == newCD->cPropertySets);

	for(cPropSet=0; cPropSet<oldCD->cPropertySets; cPropSet++)
	{
		pOldPropSet = &(oldCD->rgPropertySets[cPropSet]);
		pNewPropSet = &(newCD->rgPropertySets[cPropSet]);

		//compare the props in that prop set
		TESTC(pOldPropSet->guidPropertySet == pNewPropSet->guidPropertySet);
		TESTC(pOldPropSet->cProperties == pNewPropSet->cProperties);

		for(cProp = 0; cProp < pOldPropSet->cProperties; cProp++)
		{
			pOldProp = &(pOldPropSet->rgProperties[cProp]);
			pNewProp = &(pNewPropSet->rgProperties[cProp]);

			//there are no info for some of the properties
			if (	(pNewProp->dwPropertyID == DBPROP_COL_AUTOINCREMENT ) 
				||	(pNewProp->dwPropertyID == DBPROP_COL_DEFAULT ) 
				||	(pNewProp->dwPropertyID == DBPROP_COL_UNIQUE )
			   )
				continue;

			TESTC(pOldProp->dwPropertyID ==	pNewProp->dwPropertyID);
			TESTC(CompareVariant(&(pOldProp->vValue), &(pNewProp->vValue)));
		}
	}
	bSuccess = TRUE;

CLEANUP:
	return bSuccess;
} //CompareColumnDesc


//----------------------------------------------------------------------
//@mfunc Allocate memory for m_rgProviderTypes, and fill it with the
//provider types obtained from the tables column info. Each time this 
//function is called, the caller has to free the memory for 
//m_rgProviderTypes. This func is used by IConvertType test.
//
void CQuickTest::GetProviderTypes(CTable* pTable)
{
	DBORDINAL	cColumns = 0;
	DBORDINAL	iCol = 0;
	CCol		colTmp;

	//If m_rgProviderTypes exists, then return.
	if(m_rgProviderTypes)
		return;

	cColumns = pTable->CountColumnsOnTable();

	//Allocate memory for storing the provider types.
	SAFE_ALLOC(m_rgProviderTypes, DBTYPE, cColumns);
	m_cProviderTypes = (ULONG) cColumns;   // small no.

	//Copy the provider types from the tables column info.
	for(iCol=0; iCol<cColumns; iCol++)
	{
		if(S_OK == pTable->GetColInfo(iCol+1, colTmp))
		{
			m_rgProviderTypes[iCol] = colTmp.GetProviderType();
		}
	}

CLEANUP:
	return;
} //GetProviderTypes


//----------------------------------------------------------------------
//@mfunc Check if the given type is in the list of provider types
//m_rgProviderTypes. This func is used by IConvertType test.
//
BOOL CQuickTest::IsSupportedType(DBTYPE dwType)
{
	ULONG	ulIndex = 0;

	if(m_rgProviderTypes == NULL)
		return FALSE;

	for(ulIndex=0; ulIndex<m_cProviderTypes; ulIndex++)
	{
		if(m_rgProviderTypes[ulIndex] == dwType)
			return TRUE;
	}
	return FALSE;
} //IsSupportedType


//----------------------------------------------------------------------
//@mfunc Use the DBSCHEMA_INDEXES alongwith the restrictions to determine 
//if the given index exists. Return codes are interpreted as follows:
//TEST_PASS: check fExists to see if Index exists or not.
//TEST_SKIPPED: the schema or restrictions were not supported.
//TEST_FAIL: an unexpected error occurred.
//
TESTRESULT CQuickTest::DoesIndexExist(
	IDBSchemaRowset*	pIDSR,    //[IN] pointer to interface  
	DBID*				pTableID, //[IN] table ID
	DBID*				pIndexID, //[IN] Index ID
	BOOL*				pfExists  //[OUT] does index exist
	)
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG				ulIndex = 0;   //no. of schemas
	ULONG				cRest = 5;
	DBCOUNTITEM			cRowsObtained = 0;
	HROW*				rghRows = NULL;
	ULONG				cSchema	= 0;
	ULONG				*prgRestrictions= NULL;
	GUID				*prgSchemas	= NULL;
	BOOL				bIsSchemaSupported = FALSE;
	VARIANT				rgRestrictIndexes[5];
	IRowset*			pIndexRowset = NULL;

	//Call VariantInit.
	for(ulIndex=0;ulIndex<cRest;ulIndex++)
		VariantInit(&rgRestrictIndexes[ulIndex]);

	TESTC(pfExists != NULL)
	*pfExists = FALSE;

	//Check to see if the schema is supported
	TESTC_(hr = pIDSR->GetSchemas(&cSchema, &prgSchemas, 
		&prgRestrictions),S_OK)

	//Check to see if DBSCHEMA_INDEXES is supported
	for(ulIndex=0, bIsSchemaSupported=FALSE; ulIndex<cSchema && !bIsSchemaSupported;)
	{
		if(prgSchemas[ulIndex] == DBSCHEMA_INDEXES)
			bIsSchemaSupported = TRUE;
		else 
			ulIndex++;
	}

	//The schema has to be supported. Also the table name and Index
	//name restrictions have to be supported.
	if(!bIsSchemaSupported || !(prgRestrictions[ulIndex] & 0x4) || !(prgRestrictions[ulIndex] & 0x10))
	{
		odtLog<<L"Index Schema Rowset or the required constraints are not supported\n";
		tTestResult = TEST_SKIPPED;  
		goto CLEANUP;
	}

	//Set the restrictions.
	rgRestrictIndexes[2].vt 	 = VT_BSTR;
	rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);

	//Get the Index schema rowset.
	TESTC_(hr = pIDSR->GetRowset(NULL, DBSCHEMA_INDEXES, cRest,	
		rgRestrictIndexes, IID_IRowset, 0, NULL, (IUnknown**)
		&pIndexRowset),S_OK)

	TESTC(pIndexRowset != NULL)

	//Fetch rows. There should be only 1 row or no rows. 
	hr = GetNextRows(pIndexRowset, 0, 0, 3, &cRowsObtained, &rghRows) ;
	if(FAILED(hr))
	{
		CHECK(hr, S_OK);
		goto CLEANUP;    //TEST_FAIL.
	}
	
	if(cRowsObtained == 0 && rghRows == NULL)
	{
		//Index does not exist.
		tTestResult = TEST_PASS; 
		*pfExists = FALSE;
		goto CLEANUP;
	}

	if(cRowsObtained == 0 || rghRows == NULL)
	{
		//An error occurred.
		COMPARE(cRowsObtained, 1);
		COMPARE(rghRows != NULL, TRUE);
		tTestResult = TEST_FAIL;  
		goto CLEANUP;
	}

	//If we got here, then Index exists.
	TESTC(cRowsObtained == 1 && rghRows)
	TESTC(*rghRows != DB_NULL_HROW)
	*pfExists = TRUE;
	tTestResult = TEST_PASS; 

CLEANUP:
	CHECK(ReleaseRows(&pIndexRowset, &cRowsObtained, &rghRows), S_OK);
	for(ulIndex=0; ulIndex<5; ulIndex++)
		VariantClear(&rgRestrictIndexes[ulIndex]);
	PROVIDER_FREE(prgSchemas);
	PROVIDER_FREE(prgRestrictions);
	SAFE_RELEASE(pIndexRowset);
	return tTestResult;
} //DoesIndexExist	


//-------------------------------------------------------------------
//@mfunc Get the optional columns through IColumnsRowset.
//
HRESULT CQuickTest::GetAvailableColumns(
	IUnknown*	pUnk,			//[IN] Pointer to the object.
	DBORDINAL*	pcOptCols,		//[OUT] Number of optional columns.
	DBID**		prgOptCols		//[OUT] DBIDs of optional columns.
	)
{
	HRESULT				hrRet = E_FAIL;
	IColumnsRowset*		pIColumnsRowset = NULL;

	HRESULT hr = pUnk->QueryInterface(IID_IColumnsRowset, (void **)
		&pIColumnsRowset);

	if(hr == E_NOINTERFACE)
	{
		odtLog<<L"IColumnsRowset is not supported.\n";
		return E_NOINTERFACE;
	}

	TESTC_(hr, S_OK)
	TESTC(pIColumnsRowset != NULL)

	TESTC_(hrRet=pIColumnsRowset->GetAvailableColumns(pcOptCols, 
		prgOptCols), S_OK)

	if((*pcOptCols==0) && (*prgOptCols==NULL))
	{
		//Issue a warning.
		COMPAREW(*pcOptCols, 1);
		odtLog<<"WARNING: No Optional Metadata columns were returned.\n";
	}
	else
	{
		TESTC((*pcOptCols !=0) && (*prgOptCols !=NULL))
	}

CLEANUP:
	SAFE_RELEASE(pIColumnsRowset);
	return hrRet;
} //GetAvailableColumns


//----------------------------------------------------------------------
//@mfunc Check if there are any mandatory columns in the list of 
//columns returned by IColumnsRowset::GetAvailableColumns.
//
BOOL CQuickTest::CheckOptColumns(
	const DBORDINAL		cOptCols,  //[IN] num of optional columns
	const DBID*		rgOptCols  //[IN] array of optional columns
	)
{
	BOOL	bSuccess = TRUE;
	DBORDINAL	iOpt = 0;
	ULONG		iMand = 0;  //there are a fixed no. of mand cols

	// Check if a mandatory column was returned
	for(iOpt=0; iOpt<cOptCols; iOpt++)
	{
		for(iMand=0 ;iMand<g_cMandCols; iMand++)
		{
			if(memcmp(&(rgOptCols[iOpt]),g_rgMandCols[iMand].pDbid,sizeof(DBID)) == 0)
			{
				odtLog<<L"ERROR: Mandatory Column found at " << iOpt<< ENDL;
				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
} //CheckOptColumns


//----------------------------------------------------------------------
//@mfunc For the given columns rowset, fetch one row at a time and verify
//them. 
//
BOOL CQuickTest::CheckColumnsRowset(
	IUnknown*		pIUnk,                //[IN] pointer to the columns rowset
	const DBORDINAL	cOptCols,             //[IN] num of optional columns
	const BOOL		bOrgRowsetHasBookmark //[IN] does the original rowset have bookmark column
	)
{
	BOOL			bSuccess = FALSE;
	BOOL			bStatus = FALSE;
	HRESULT			hr = S_OK;
	HACCESSOR		hAccessor = NULL;
	USHORT			usBookmark = 0;  // 0 = Doesn't exist, 1 = Exists.
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		iRow = 0;
	DBORDINAL		iCol = 0;
	DBCOUNTITEM		iBind = 0;
	DBCOUNTITEM		cRowsObtained = 0;	
	DBCOUNTITEM		cTotalRows = 0;
	DBCOUNTITEM		cBindings = 0;
	DBORDINAL		cColumns = 0;
	CCol			col;
	DBCOLUMNINFO*	rgInfo = NULL;
	DBBINDING *		rgBindings = NULL;	
	BYTE *			pData = NULL;
	DATA*			pColumn = NULL;
	HROW*			rghRows = NULL;
	WCHAR*			pStrBuf = NULL;
	IRowset *		pIRowset = NULL;
	IAccessor *		pIAccessor = NULL;

	//Obtain IAccessor interface.
	if(!VerifyInterface(pIUnk,IID_IAccessor, 
		ROWSET_INTERFACE,(IUnknown **)&pIAccessor))
		return TEST_FAIL;

	//Obtain IRowset interface.
	TESTC(VerifyInterface(pIUnk,IID_IRowset, 
		ROWSET_INTERFACE,(IUnknown **)&pIRowset))

	// Get bindings and column info
	TESTC_(hr=GetAccessorAndBindings(pIUnk,	DBACCESSOR_ROWDATA,					
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,							
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH, ALL_COLS_BOUND,							
		FORWARD, NO_COLS_BY_REF, &rgInfo, &cColumns, &pStrBuf), S_OK) 

	TESTC(rgInfo != NULL)

	//Check if the columns rowset has bookmark column.
	if(rgInfo[0].iOrdinal == 0)
		usBookmark = 1;

	//Check the number of columns in the columns rowset.
	TESTC(cColumns == g_cMandCols + cOptCols + usBookmark)

	//For each mandatory column ...
	for(iCol=0; iCol<g_cMandCols; iCol++)
	{
		//Check the type.
		COMPARE(rgInfo[iCol+usBookmark].wType , g_rgMandCols[iCol].wType);

		//Check the name.
		TESTC(wcscmp(rgInfo[iCol+usBookmark].pwszName,
			g_rgMandCols[iCol].pwszName) == 0)
	}

	SAFE_ALLOC(pData, BYTE, cbRowSize)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get one row at a time from the columns rowset.
	while(S_OK==(hr=GetNextRows(pIRowset, NULL,0,1,&cRowsObtained,&rghRows)))
	{
		//If it is a row corresponding to a bookmark column in the 
		//original rowset, then release the row and fetch next row.
		if((iRow==0)&&(bOrgRowsetHasBookmark))
			goto RELEASEROW;

		memset(pData, 0, (size_t) cbRowSize);

		//From the table, get the column info for the column corresponding
		//to this row.
		TESTC_(hr=m_pTable->GetColInfo(iRow+ !bOrgRowsetHasBookmark, col), S_OK)

		//Get the row data.
		TESTC_(hr=pIRowset->GetData(rghRows[0],hAccessor, pData),S_OK)

		//Check the first 6 mandatory columns of this row.
		for(iBind=0; iBind < cBindings; iBind++)
		{
			if((iBind==0) && usBookmark)
				continue;

			//Grab one column.
			pColumn = (DATA *) (pData + rgBindings[iBind].obStatus);

			// Mandatory columns :-

			if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_IDNAME))
			{
				if( (DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK )
				{
					bStatus = TRUE;
					COMPARE(wcscmp(col.GetColName(), 
						(WCHAR*)pColumn->bValue),0) ;
				}
			}
			else if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_GUID))
			{
				if( (DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK )
				{
					bStatus = TRUE;
					COMPARE((col.GetColID())->uGuid.guid,
						*((GUID*)pColumn->bValue)) ;
				}
			}
			else if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_PROPID))
			{
				if( (DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK )
				{
					bStatus = TRUE;
					COMPARE((col.GetColID())->uName.ulPropid,
						*((ULONG*)pColumn->bValue)) ; //propid is ULONG
				}
			}
			else if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_NAME))
			{
				if((DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK)
				{
					if(!COMPAREW(wcscmp(col.GetColName(), 
						(WCHAR*)pColumn->bValue),0))
					{
						odtLog<<L"WARNING: Column names don't match.\n"
							<<L"Expected: "<<col.GetColName()
							<<L" Got: "<<(WCHAR*)pColumn->bValue<<"\n";
					}

				}
			}
			else if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_NUMBER))
			{
				TESTC((DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK)
				COMPARE(*((ULONG*)pColumn->bValue) , 
					iRow+ !bOrgRowsetHasBookmark);
			}
			else if( CompareDBID(rgInfo[iBind].columnid, DBCOLUMN_TYPE))
			{
				TESTC((DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK)
				COMPARE(*((USHORT*)pColumn->bValue),
					col.GetProviderType()) ;
			}
		}

		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);

		//This is used to check if at least one of the three columns
		//(DBCOLUMN_IDNAME, DBCOLUMN_GUID and DBCOLUMN_PROPID) has a
		//status of DBSTATUS_S_OK.
		TESTC(bStatus)

RELEASEROW:
		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
		bStatus = FALSE;
		cTotalRows++;
		iRow++;
	}

	TESTC_(hr, DB_S_ENDOFROWSET)

	//Check the total number of rows fetched from the columns rowset.
	TESTC(cTotalRows == m_pTable->CountColumnsOnTable()+ bOrgRowsetHasBookmark)

	bSuccess = TRUE;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStrBuf);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	return bSuccess;
} //CheckColumnsRowset


//----------------------------------------------------------------------
//@mfunc Create an instance of the Root Binder, requesting IBindResource.
//Then QI for IDBBinderProperties and set the initialization properties,
//and some rowset properties. Then get the rowset URL from INI file.
//
BOOL CQuickTest::CreateRootBinder()
{
	TBEGIN
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	IBindResource*			pIBR = NULL;
	IDBBinderProperties*	pIDBBProp = NULL;

	pIBR = GetModInfo()->GetRootBinder();
	TESTC_PROVIDER(pIBR != NULL)

	if(GetModInfo()->GetParseObject()->GetURL(ROWSET_INTERFACE))
		m_pwszRowsetURL = wcsDuplicate(GetModInfo()->GetParseObject()->GetURL(ROWSET_INTERFACE));
	else if(GetModInfo()->GetRootURL())
		m_pwszRowsetURL = wcsDuplicate(GetModInfo()->GetRootURL());

	//There may not be an INI file, or the INI file may not have a 
	//[URL] section. In these cases we would want to skip.
	TESTC_PROVIDER(m_pwszRowsetURL != NULL)

	//URLs are of the form "scheme:<prefix>". Cannot have a valid URL 
	//with less than 2 characters.
	TESTC(wcslen(m_pwszRowsetURL) > 1)

	TESTC(VerifyInterface(pIBR, IID_IBindResource,
		BINDER_INTERFACE,(IUnknown**)&m_pIBindResource))

	TESTC(VerifyInterface(m_pIBindResource, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBProp))

	TESTC(GetInitProps(&cPropSets, &rgPropSets))

	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, 
		DBPROPOPTIONS_OPTIONAL) ;

	//SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, 
	//	&rgPropSets) ;

	TESTC_(pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBBProp);
	TRETURN
} //CreateRootBinder


//-----------------------------------------------------------------------
//@mfunc Get the properties specified in the property ID sets created in 
//the function InitPropIDStructs. Check for the property ID and status of
//returned property sets. For the properties which show NOTSUPPORTED, 
//verify that they are indeed not supported.
//
TESTRESULT CQuickTest::testGetProperties()
{
	TESTRESULT		tTestResult = TEST_FAIL;  
	HRESULT			hr = S_OK;
	ULONG			ulSupported = 0;
	ULONG			iSet=0, iProp=0 ;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;

	if(m_pIDBProperties == NULL)
	{
		odtLog<<L"IDBProperties pointer is NULL.\n";
		return TEST_FAIL;
	}

	//IDBProperties::GetProperties may return S_OK if all properties in 
	//our DBPROPIDSET structures are supported. If some of them are not 
	//supported it may return DB_S_ERRORSOCCURRED. 
	hr = m_pIDBProperties->GetProperties(m_cPropIDSets, 
		m_rgPropIDSets, &cPropSets, &rgPropSets) ;

	if(hr==DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"WARNING: None of the properties used were supported.\n";
		CHECKW(hr, S_OK);
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	if((hr != S_OK) && (hr != DB_S_ERRORSOCCURRED))
	{
		CHECK(hr, S_OK);
		goto CLEANUP;
	}

	// Verify returned variables have valid values
	if(!COMPARE(cPropSets, m_cPropIDSets) || 
		(rgPropSets==NULL) || 
		!COMPARE(rgPropSets[0].cProperties, m_rgPropIDSets[0].cPropertyIDs) || 
		(rgPropSets[0].rgProperties==NULL) ||
		!COMPARE(rgPropSets[1].cProperties, m_rgPropIDSets[1].cPropertyIDs) || 
		(rgPropSets[1].rgProperties==NULL ))
		goto CLEANUP;

	//Compare property IDs and check status for verification.
	for(iSet=0; iSet<cPropSets; iSet++)
	{
		for(iProp=0; iProp<rgPropSets[iSet].cProperties ; iProp++)
		{
			//Comparing property IDs
			TESTC(rgPropSets[iSet].rgProperties[iProp].dwPropertyID ==
				m_rgPropIDSets[iSet].rgPropertyIDs[iProp])

			//Check if the variant type matches the type we had set
			//to in InitPropIDStructs, or VT_EMPTY.
			COMPARE(((V_VT(& rgPropSets[iSet].rgProperties[iProp].vValue) ==
				m_rgPropInfoSets[iSet].rgPropertyInfos[iProp].vtType) ||
				(V_VT(& rgPropSets[iSet].rgProperties[iProp].vValue) ==
				VT_EMPTY)), TRUE) ;

			//Check status.
			if(hr==S_OK)
			{
				//The status should be DBPROPSTATUS_OK.
				COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus,
					DBPROPSTATUS_OK) ;
			}
			if(hr==DB_S_ERRORSOCCURRED)
			{
				//The status should be either DBPROPSTATUS_OK or 
				//DBPROPSTATUS_NOTSUPPORTED. 
				COMPARE(((rgPropSets[iSet].rgProperties[iProp].dwStatus==
					DBPROPSTATUS_OK) ||
					(rgPropSets[iSet].rgProperties[iProp].dwStatus==
					DBPROPSTATUS_NOTSUPPORTED)), TRUE) ;

				if(rgPropSets[iSet].rgProperties[iProp].dwStatus==
					DBPROPSTATUS_OK)
					ulSupported++;
			}

			//If the obtained property shows that it is not supported,
			//then verify that the flag was set to NOT_SUPPORTED in
			//InitSupInfo.
			if(rgPropSets[iSet].rgProperties[iProp].dwStatus ==
				DBPROPSTATUS_NOTSUPPORTED) 
			{
				COMPARE(m_rgPropInfoSets[iSet].rgPropertyInfos[iProp].dwFlags,
				DBPROPFLAGS_NOTSUPPORTED) ;
			}
		}
	}

	if(hr==DB_S_ERRORSOCCURRED)
		TESTC(ulSupported > 0)

	tTestResult = TEST_PASS;
	
CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	return tTestResult;
} //testGetProperties


//----------------------------------------------------------------------
//@mfunc Get ALL properties supported by a provider. For the returned 
//properties, check if the status is DBPROPSTATUS_OK. For a bunch of 
//properties check if the variant type of the variant in DBPROP is the 
//same as property type or VT_EMPTY. 
//
TESTRESULT CQuickTest::testGetAllProperties(IDBProperties* pIDBProperties)
{
	TESTRESULT		tTestResult = TEST_FAIL;  
	HRESULT			hr = S_OK;
	ULONG			iSet=0, iProp=0 ;
	ULONG			cPropSets = 0;
	DBPROP*			pProp = NULL;
	DBPROPSET*		rgPropSets = NULL;

	if(pIDBProperties == NULL)
	{
		odtLog<<L"IDBProperties pointer is NULL.\n";
		return TEST_FAIL;
	}

	//IDBProperties::GetProperties(0,NULL)
	TESTC_(hr=pIDBProperties->GetProperties(0, NULL, &cPropSets, 
		&rgPropSets), S_OK)

	//At least CANHOLDROWS has to be supported.
	TESTC((cPropSets != 0) && (rgPropSets != NULL))

	//Check the DBPROP structures for certain selected properties in the
	//Data Source Information and Data Source Initialization groups. 
	for(iSet=0; iSet<cPropSets; iSet++)
	{
		//If the property belongs to the Data Source Information group.
		if(rgPropSets[iSet].guidPropertySet==DBPROPSET_DATASOURCEINFO)
		{
			for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
			{
				pProp = &(rgPropSets[iSet].rgProperties[iProp]);

				//Check if status of property is DBPROPSTATUS_OK.
				COMPARE(pProp->dwStatus, DBPROPSTATUS_OK) ;

				//The variant type of the vValue field in DBPROP should 
				//either match the type of the property, or VT_EMPTY.
				switch(pProp->dwPropertyID)
				{
				case DBPROP_ACTIVESESSIONS:
				case DBPROP_MAXROWSIZE:
				case DBPROP_MAXTABLESINSELECT:
				case DBPROP_SQLSUPPORT:
					COMPARE(VT_I4, V_VT(& pProp->vValue)) ;
					break;

				case DBPROP_CATALOGLOCATION:
					COMPARE(VT_I4, V_VT(& pProp->vValue)) ;
					COMPARE((V_I4(&pProp->vValue)==DBPROPVAL_CL_START ||
						V_I4(&pProp->vValue)==DBPROPVAL_CL_END), TRUE) ;
					break;

				case DBPROP_COLUMNDEFINITION:
					COMPARE(VT_I4, V_VT(& pProp->vValue)) ;
					COMPARE((V_I4(&pProp->vValue)==0 ||
						V_I4(&pProp->vValue)==DBPROPVAL_CD_NOTNULL), TRUE) ;
					break;

				case DBPROP_DATASOURCENAME:
				case DBPROP_DBMSNAME:
				case DBPROP_DBMSVER:
				case DBPROP_PROVIDERNAME:
				case DBPROP_USERNAME:
					COMPARE((VT_BSTR == V_VT(&pProp->vValue) ||
						VT_EMPTY == V_VT(&pProp->vValue)),TRUE);
					//COMPARE(VT_BSTR, V_VT(& pProp->vValue)); 
					COMPARE(CheckVariant(& pProp->vValue),TRUE);
					break;

				case DBPROP_DATASOURCEREADONLY:
				case DBPROP_ROWSETCONVERSIONSONCOMMAND:
					COMPARE(VT_BOOL, V_VT(& pProp->vValue));
					COMPARE(CheckVariant(& pProp->vValue),TRUE);
					break;

				case DBPROP_IDENTIFIERCASE:
					COMPARE(VT_I4, V_VT(& pProp->vValue));
					COMPARE((V_I4(&pProp->vValue)==DBPROPVAL_IC_UPPER ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_LOWER ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_SENSITIVE ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_MIXED),TRUE);
					break;

				case DBPROP_QUOTEDIDENTIFIERCASE:
					COMPARE(VT_I4, V_VT(& pProp->vValue));
					COMPARE((V_I4(&pProp->vValue)==DBPROPVAL_IC_UPPER ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_LOWER ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_SENSITIVE ||
						V_I4(&pProp->vValue)==DBPROPVAL_IC_MIXED),TRUE);
					break;

				default:
					break;
				}
			}
		}
		//If property belongs to the Data Source Initialization group.
		else if(rgPropSets[iSet].guidPropertySet==DBPROPSET_DBINIT)
		{
			for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
			{
				pProp = &(rgPropSets[iSet].rgProperties[iProp]);

				//Check if status of property is DBPROPSTATUS_OK. 
				COMPARE(pProp->dwStatus, DBPROPSTATUS_OK);

				//The variant type of the vValue field in DBPROP should 
				//either match the type of the property, or VT_EMPTY.
				switch(pProp->dwPropertyID)
				{
				case DBPROP_AUTH_PASSWORD:
				case DBPROP_AUTH_USERID:
				case DBPROP_INIT_DATASOURCE:
				case DBPROP_INIT_LOCATION:
				case DBPROP_INIT_PROVIDERSTRING:
					COMPARE((VT_BSTR == V_VT(&pProp->vValue) ||
						VT_EMPTY == V_VT(&pProp->vValue)),TRUE);
					if(VT_EMPTY != V_VT(&pProp->vValue))
						COMPARE(CheckVariant(&pProp->vValue),TRUE);
					break;

				case DBPROP_INIT_LCID:
				case DBPROP_INIT_MODE:
				case DBPROP_INIT_TIMEOUT:
					COMPARE((VT_I4 == V_VT(&pProp->vValue) ||
						VT_EMPTY == V_VT(&pProp->vValue)),TRUE);
					break;

				case DBPROP_INIT_PROMPT: 
					COMPARE((VT_I2 == V_VT(&pProp->vValue) ||
						VT_EMPTY == V_VT(&pProp->vValue)),TRUE);
					if(VT_EMPTY != V_VT(&pProp->vValue))
						COMPARE(checkPositive_I2(&pProp->vValue),TRUE);
					break;

				default:
					break;
				}
			}
		}
		//for any other property sets (and provider specific sets)
		else
		{
			for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
			{
				pProp = &(rgPropSets[iSet].rgProperties[iProp]);

				//Check if status of property is DBPROPSTATUS_OK. 
				COMPARE(pProp->dwStatus, DBPROPSTATUS_OK);

				//Check the variant value for some variant types.
				COMPARE(CheckVariant(&pProp->vValue), TRUE);
			}
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	return tTestResult;
} //testGetAllProperties


//----------------------------------------------------------------------
//@mfunc Get the property info for the property ID sets created in 
//the function InitPropIDStructs. Check the property ID and flags of
//returned property Info sets. 
// 
TESTRESULT CQuickTest::testGetPropInfo()
{
	TESTRESULT		tTestResult = TEST_FAIL; 
	HRESULT			hr = S_OK;
	ULONG			iSet=0, iInfo=0 ;
	ULONG			cPropInfoSets  = 0;
	DBPROPINFOSET * rgPropInfoSets = NULL;
	WCHAR*			pwszStringBuffer = NULL;

	if(m_pIDBProperties == NULL)
	{
		odtLog<<L"IDBProperties pointer is NULL.\n";
		return TEST_FAIL;
	}

	//IDBPropertiesGetPropertyInfo may return S_OK if all properties in 
	//our DBPROPIDSET structures are supported. If some of them are not
	//supported it may return DB_S_ERRORSOCCURRED. 
	hr = m_pIDBProperties->GetPropertyInfo(m_cPropIDSets, m_rgPropIDSets,
		&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer) ;

	if(hr==DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"CANHOLDROWS has to be supported.\n";
		tTestResult = TEST_FAIL;
		goto CLEANUP;
	}
	if((hr != S_OK) && (hr != DB_S_ERRORSOCCURRED))
	{
		CHECK(hr, S_OK);
		goto CLEANUP;
	}
		
	//Check if the returned variables have valid values
	if(!COMPARE(cPropInfoSets,m_cPropIDSets) || 
		(rgPropInfoSets==NULL) || 
		!COMPARE(rgPropInfoSets[0].cPropertyInfos,m_rgPropIDSets[0].cPropertyIDs) || 
		(rgPropInfoSets[0].rgPropertyInfos==NULL) ||
		!COMPARE(rgPropInfoSets[1].cPropertyInfos,m_rgPropIDSets[1].cPropertyIDs) || 
		(rgPropInfoSets[1].rgPropertyInfos==NULL) )
		goto CLEANUP;

	//Compare Descriptions and property IDs for verification.
	for(iSet=0; iSet<cPropInfoSets; iSet++)
	{
		for(iInfo=0; iInfo<rgPropInfoSets[iSet].cPropertyInfos ; iInfo++)
		{
			//Check property ID
			COMPARE(rgPropInfoSets[iSet].rgPropertyInfos[iInfo].dwPropertyID,
				m_rgPropIDSets[iSet].rgPropertyIDs[iInfo]);

			//Verify the flags are the same as that initialized in the
			//function InitSupInfo.
			COMPARE(rgPropInfoSets[iSet].rgPropertyInfos[iInfo].dwFlags,
				m_rgPropInfoSets[iSet].rgPropertyInfos[iInfo].dwFlags);
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	return tTestResult;
} //testGetPropInfo


//------------------------------------------------------------------------
//@mfunc Get property Info for all properties supported by a provider. For
//the returned properties, check the flags in the field dwFlags of 
//DBPROPINFO. For a bunch of properties check if the variant type (vtType) 
//of the DBPROPINFO matches that of the property.
//
TESTRESULT CQuickTest::testGetAllPropInfo(IDBProperties* pIDBProperties)
{
	TESTRESULT		tTestResult = TEST_FAIL; 
	HRESULT			hr = S_OK;
	ULONG			iSet=0, iInfo=0 ;
	ULONG			cPropInfoSets  = 0;
	DBPROPINFO*		pPropInfo = NULL;
	DBPROPINFOSET * rgPropInfoSets = NULL;
	WCHAR*			pwszStringBuffer = NULL;

	if(pIDBProperties == NULL)
	{
		odtLog<<L"IDBProperties pointer is NULL.\n";
		return TEST_FAIL;
	}

	//IDBProperties::GetPropertyInfo(0,NULL)
	TESTC_(hr=pIDBProperties->GetPropertyInfo(0, NULL, &cPropInfoSets, 
		&rgPropInfoSets, &pwszStringBuffer), S_OK)

	//At least CANHOLDROWS has to be supported.
	TESTC((cPropInfoSets!=0) && (rgPropInfoSets!=NULL))

	//for all property info sets...
	for(iSet=0; iSet<cPropInfoSets; iSet++)
	{
		//If property belongs to the Data Source Information group. 
		if(rgPropInfoSets[iSet].guidPropertySet==DBPROPSET_DATASOURCEINFO)
		{
			for(iInfo=0; iInfo<rgPropInfoSets[iSet].cPropertyInfos; iInfo++)
			{
				pPropInfo = &(rgPropInfoSets[iSet].rgPropertyInfos[iInfo]);

				// Check flag field of DBPROPINFO struct.
				COMPARE((pPropInfo->dwFlags & DBPROPFLAGS_DATASOURCEINFO), 
					DBPROPFLAGS_DATASOURCEINFO);

				//Verify description exists.
				COMPARE((wcslen(pPropInfo->pwszDescription)>0),TRUE);

				//Check if the variant type in DBPROPINFO matches that of 
				//the property.
				switch(pPropInfo->dwPropertyID)
				{
				case DBPROP_ACTIVESESSIONS:
				case DBPROP_CATALOGLOCATION:
				case DBPROP_COLUMNDEFINITION:
				case DBPROP_IDENTIFIERCASE:
				case DBPROP_MAXROWSIZE:
				case DBPROP_MAXTABLESINSELECT:
				case DBPROP_QUOTEDIDENTIFIERCASE:
				case DBPROP_SQLSUPPORT:
					COMPARE(VT_I4, pPropInfo->vtType);
					break;

				case DBPROP_DATASOURCENAME:
				case DBPROP_DBMSNAME:
				case DBPROP_DBMSVER:
				case DBPROP_PROVIDERNAME:
				case DBPROP_USERNAME:
					COMPARE(VT_BSTR, pPropInfo->vtType);
					break;

				case DBPROP_DATASOURCEREADONLY:
				case DBPROP_ROWSETCONVERSIONSONCOMMAND:
					COMPARE(VT_BOOL, pPropInfo->vtType);
					break;

				default:
					break;
				}
			}
		}
		//If property belongs to the Data Source Initialization group.
		else if(rgPropInfoSets[iSet].guidPropertySet==DBPROPSET_DBINIT)
		{
			for(iInfo=0; iInfo<rgPropInfoSets[iSet].cPropertyInfos; iInfo++)
			{
				pPropInfo = &(rgPropInfoSets[iSet].rgPropertyInfos[iInfo]);

				// Check flag field of DBPROPINFO struct.
				COMPARE((pPropInfo->dwFlags & DBPROPFLAGS_DBINIT) , 
					DBPROPFLAGS_DBINIT);

				//Verify description exists.
				COMPARE((wcslen(pPropInfo->pwszDescription)>0),TRUE);

				//Check if the variant type in DBPROPINFO matches that of 
				//the property. 
				switch(pPropInfo->dwPropertyID)
				{
				case DBPROP_AUTH_PASSWORD: 
				case DBPROP_AUTH_USERID:
				case DBPROP_INIT_DATASOURCE:
				case DBPROP_INIT_LOCATION:
				case DBPROP_INIT_PROVIDERSTRING:
					COMPARE(VT_BSTR, pPropInfo->vtType);
					break;

				case DBPROP_INIT_LCID:
				case DBPROP_INIT_MODE:
				case DBPROP_INIT_TIMEOUT:
					COMPARE(VT_I4, pPropInfo->vtType);
					break;

				case DBPROP_INIT_PROMPT: 
					COMPARE(VT_I2, pPropInfo->vtType);
					break;

				default:
					break;
				}
			}
		}
		//for other property groups (and provider specific groups)
		else
		{
			for(iInfo=0; iInfo<rgPropInfoSets[iSet].cPropertyInfos; iInfo++)
				COMPARE((wcslen(rgPropInfoSets[iSet].rgPropertyInfos[iInfo].pwszDescription)>0),TRUE);
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	return tTestResult;
} //testGetAllPropInfo


//----------------------------------------------------------------------
//@mfunc Set the property DBPROP_INIT_PROMPT to DBPROMPT_NOPROMPT and
//DBPROP_INIT_ASYNCH to 0. Then call GetProperty to verify.
//
TESTRESULT CQuickTest::testSetProperties1()
{
	TESTRESULT		tTestResult = TEST_FAIL; 
	HRESULT			hr = S_OK;
	ULONG			ulSupported = 0;
	SHORT			sPrompt = DBPROMPT_NOPROMPT;
	LONG			lAsynch = 0;
	ULONG			cPropSetsEx=0;
	DBPROPSET*		rgPropSetsEx = NULL;
	IDBInitialize*	pIDBInitialize = NULL;
	IDBProperties*	pIDBProperties = NULL;

	//Initialize the variant to pass as an [OUT] parameter to GetProperty.
	VARIANT		vVar;
	VariantInit(&vVar);
	VARIANT		vVar2;
	VariantInit(&vVar2);

	//Create a new DSO.
	TESTC_(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, 
		(IUnknown**)&pIDBInitialize), S_OK)
	if(pIDBInitialize==NULL)
	{
		odtLog<<L"INFO: Could not create another DSO.\n";
		return TEST_FAIL;
	}

	//Used in the Extralib property functions.
	g_pIDBInitialize = pIDBInitialize;

	//Obtain the IDBProperties interface.
	if(!VerifyInterface(pIDBInitialize, IID_IDBProperties, 
		DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
		goto CLEANUP;

	//Get the initialization properties, and set them.
	GetInitProps(&cPropSetsEx, &rgPropSetsEx);
	TESTC_(hr = pIDBProperties->SetProperties(cPropSetsEx, rgPropSetsEx), 
		S_OK)

	//Set the 2 properties.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, 
		&m_cPropSets, &m_rgPropSets,
		(void*)sPrompt, DBTYPE_I2);
	SetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)lAsynch, DBTYPE_I4); //cast modified

	//Call SetProperties method.
	hr = pIDBProperties->SetProperties(m_cPropSets, m_rgPropSets);

	//check status.
	if(hr==S_OK)
	{
		//Status should be OK for both properties.
		COMPARE(m_rgPropSets[0].rgProperties[0].dwStatus, 
			DBPROPSTATUS_OK);

		COMPARE(m_rgPropSets[0].rgProperties[1].dwStatus, 
			DBPROPSTATUS_OK);
	}
	else if(hr==DB_S_ERRORSOCCURRED)
	{
		//If the status was not OK, then make sure the property was
		//not supported or settable.
		if(m_rgPropSets[0].rgProperties[0].dwStatus != DBPROPSTATUS_OK)
		{
			COMPARE(SettableProperty(DBPROP_INIT_PROMPT, 
				DBPROPSET_DBINIT), FALSE);
		}
		else
		{
			ulSupported++;
		}

		if(m_rgPropSets[0].rgProperties[1].dwStatus != DBPROPSTATUS_OK)
		{
			COMPARE(SettableProperty(DBPROP_INIT_ASYNCH, 
				DBPROPSET_DBINIT), FALSE);
		}
		else
		{
			ulSupported++;
		}
	}
	else
	{
		//Both properties were not supported or settable.
		TESTC_(hr, DB_E_ERRORSOCCURRED)

		COMPARE(SettableProperty(DBPROP_INIT_PROMPT, 
			DBPROPSET_DBINIT), FALSE);

		COMPARE(SettableProperty(DBPROP_INIT_ASYNCH, 
			DBPROPSET_DBINIT), FALSE);

		odtLog<<L"INFO: None of the properties used (DBPROP_INIT_PROMPT & DBPROP_INIT_ASYNCH) were supported or settable.\n";
	}

	if(hr==DB_S_ERRORSOCCURRED)
		COMPARE(ulSupported, 1);

	if(m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_OK)
	{
		//Call GetProperty to get the variant value of DBPROP_INIT_PROMPT.
		TESTC(GetProperty(DBPROP_INIT_PROMPT,DBPROPSET_DBINIT, 
			(IUnknown*)pIDBInitialize, &vVar))

		//Compare the variant value got by above call to GetProperty and the 
		//one created (pVar) to pass in to SetProperty.
		COMPARE(V_VT(&vVar), DBTYPE_I2);
		COMPARE(V_I2(&vVar), DBPROMPT_NOPROMPT);
	}

	if(m_rgPropSets[0].rgProperties[1].dwStatus == DBPROPSTATUS_OK)
	{
		//Call GetProperty to get the variant value of DBPROP_INIT_ASYNCH.
		TESTC(GetProperty(DBPROP_INIT_ASYNCH,DBPROPSET_DBINIT, 
			(IUnknown*)pIDBInitialize, &vVar2))

		//Compare the variant value got by above call to GetProperty and the 
		//one created (pVar) to pass in to SetProperty.
		COMPARE(V_VT(&vVar2), DBTYPE_I4);
		COMPARE(V_I4(&vVar2), 0);
	}

	//Initialize the DSO.
	CHECK(hr = pIDBInitialize->Initialize(), S_OK);
	if(hr != S_OK)
		goto CLEANUP;

	tTestResult = TEST_PASS;

CLEANUP:
	VariantClear(&vVar);
	VariantClear(&vVar2);
	FreeProperties(&cPropSetsEx, &rgPropSetsEx);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	//Release the DSO.
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	return tTestResult;
} //testSetProperties1


//-----------------------------------------------------------------------
//@mfunc Set the variant of properties DBPROP_INIT_PROMPT and 
//DBPROP_INIT_ASYNCH to VT_EMPTY. When SetProperties is called,
//these should be set to their default values. Call GetProperty to 
//verify.
//
TESTRESULT CQuickTest::testSetProperties2()
{
	TESTRESULT		tTestResult = TEST_FAIL; //Test Return Value
	HRESULT			hr = S_OK;
	ULONG			ulSupported = 0;
	ULONG			cPropSets = 1;
	DBPROPSET		rgPropSets[1];
	ULONG			cPropSetsEx=0;
	DBPROPSET*		rgPropSetsEx = NULL;
	DBPROP			dwProp[2];
	IDBInitialize*	pIDBInitialize = NULL;
	IDBProperties*	pIDBProperties = NULL;

	//Initialize the variant to pass as an [OUT] parameter to GetProperty.
	VARIANT		vVar;
	VariantInit(&vVar);
	VARIANT		vVar2;
	VariantInit(&vVar2);

	//Create a new DSO.
	TESTC_(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, 
		(IUnknown**)&pIDBInitialize), S_OK)
	if(pIDBInitialize==NULL)
	{
		odtLog<<L"INFO: Could not create another DSO.\n";
		return TEST_FAIL;
	}

	//Used in the Extralib property functions.
	g_pIDBInitialize = pIDBInitialize;

	//Obtain the IDBProperties interface.
	if(!VerifyInterface(pIDBInitialize, IID_IDBProperties, 
		DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
		goto CLEANUP;

	//Get the initialization properties, and set them.
	GetInitProps(&cPropSetsEx, &rgPropSetsEx);
	TESTC_(hr = pIDBProperties->SetProperties(cPropSetsEx, rgPropSetsEx), 
		S_OK)

	//Set DBPROP for PROMPT.
	memset(&dwProp[0], 0, sizeof(DBPROP)) ;
	dwProp[0].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[0].vValue);

	//Set DBPROP for ASYNCH.
	memset(&dwProp[1], 0, sizeof(DBPROP)) ;
	dwProp[1].dwPropertyID = DBPROP_INIT_ASYNCH;
	dwProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[1].vValue);

	//Set the DBPROPSET.
	rgPropSets[0].rgProperties = dwProp;
	rgPropSets[0].cProperties = 2;
	rgPropSets[0].guidPropertySet = DBPROPSET_DBINIT;

	//Call SetProperties method.
	hr = pIDBProperties->SetProperties(cPropSets, rgPropSets);

	//check status.
	if(hr==S_OK)
	{
		//status should be OK for both props.
		COMPARE(rgPropSets[0].rgProperties[0].dwStatus, 
			DBPROPSTATUS_OK);

		COMPARE(rgPropSets[0].rgProperties[1].dwStatus, 
			DBPROPSTATUS_OK);
	}
	else if(hr==DB_S_ERRORSOCCURRED)
	{
		//if status is not OK, then make sure the property was not
		//supported or settable.
		if(rgPropSets[0].rgProperties[0].dwStatus != DBPROPSTATUS_OK)
		{
			COMPARE(SettableProperty(DBPROP_INIT_PROMPT, 
				DBPROPSET_DBINIT), FALSE);
		}
		else
		{
			ulSupported++;
		}

		if(rgPropSets[0].rgProperties[1].dwStatus != DBPROPSTATUS_OK)
		{
			COMPARE(SettableProperty(DBPROP_INIT_ASYNCH, 
				DBPROPSET_DBINIT), FALSE);
		}
		else
		{
			ulSupported++;
		}
	}
	else
	{
		//Both properties were not supported or settable.
		TESTC_(hr, DB_E_ERRORSOCCURRED)

		COMPARE(SettableProperty(DBPROP_INIT_PROMPT, 
			DBPROPSET_DBINIT), FALSE);

		COMPARE(SettableProperty(DBPROP_INIT_ASYNCH, 
			DBPROPSET_DBINIT), FALSE);

		odtLog<<L"INFO: None of the properties used (DBPROP_INIT_PROMPT & DBPROP_INIT_ASYNCH) were supported or settable.\n";
	}

	if(hr==DB_S_ERRORSOCCURRED)
		COMPARE(ulSupported, 1);

	if(rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_OK)
	{
		//Call GetProperty to get the variant value of DBPROP_INIT_PROMPT.
		TESTC(GetProperty(DBPROP_INIT_PROMPT,DBPROPSET_DBINIT, 
			(IUnknown*)pIDBInitialize, &vVar))

		//The variant should be set to its default value, which will be
		//one of the following.
		COMPARE(V_VT(&vVar) , DBTYPE_I2);

		COMPARE((V_I2(&vVar) == DBPROMPT_PROMPT) ||
					(V_I2(&vVar) == DBPROMPT_COMPLETE) ||
					(V_I2(&vVar) == DBPROMPT_COMPLETEREQUIRED) ||
					(V_I2(&vVar) == DBPROMPT_NOPROMPT), TRUE);
	}

	if(rgPropSets[0].rgProperties[1].dwStatus == DBPROPSTATUS_OK)
	{
		//Call GetProperty to get the variant value of DBPROP_INIT_ASYNCH.
		TESTC(GetProperty(DBPROP_INIT_ASYNCH,DBPROPSET_DBINIT, 
			(IUnknown*)pIDBInitialize, &vVar2))

		COMPARE(V_VT(&vVar2) , DBTYPE_I4);

		//The variant should be set to its default value.
		COMPARE(V_I4(&vVar2) >= 0 , TRUE);
	}

	//Initialize the DSO.
	CHECK(hr = pIDBInitialize->Initialize(), S_OK);
	if(hr != S_OK)
		goto CLEANUP;

	tTestResult = TEST_PASS;

CLEANUP:
	VariantClear(&vVar);
	VariantClear(&vVar2);
	FreeProperties(&cPropSetsEx, &rgPropSetsEx);
	//Release the DSO.
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	return tTestResult;
} //testSetProperties2


//----------------------------------------------------------------------
//@mfunc Test the GetProperties and GetPropertyInfo methods when the DSO
//is in Uninitialized state.
//
TESTRESULT CQuickTest::testGetPropBeforeInit()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	IDBProperties*		pIDBProp = NULL;

	//Create a new DSO.
	TESTC_(hr = GetModInfo()->CreateProvider(NULL, IID_IDBProperties, 
		(IUnknown**)&pIDBProp), S_OK)

	if(pIDBProp==NULL)
	{
		odtLog<<L"INFO: Could not create another DSO.\n";
		return TEST_FAIL;
	}

	//Conduct Property Tests while DSO is uninitialized.
	TESTC(testGetAllProperties(pIDBProp)==TEST_PASS)

	TESTC(testGetAllPropInfo(pIDBProp)==TEST_PASS)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBProp);
	return tTestResult;
} //testGetPropBeforeInit


//----------------------------------------------------------------------
//@mfunc Get a comma-separated list of provider-specific keywords. Make
//sure the list does not contain any keywords from OLE DB.
//
TESTRESULT CQuickTest::testGetKeywords()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			i = 0;
	WCHAR*			pwszKeywords = NULL;
	WCHAR*			wtoken = NULL;
	IDBInfo*		pIDBInfo = NULL;

	//Get pointer to IDBInfo.
	if(!VerifyInterface(m_pIDBInitialize,IID_IDBInfo,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBInfo))
	{
		odtLog<<L"IDBInfo is not supported.\n";
		return TEST_SKIPPED;
	}

	//Call GetKeywords method.
	TESTC_(hr=pIDBInfo->GetKeywords(&pwszKeywords), S_OK)

	if(!pwszKeywords) 
	{
		//Issue a warning.
		odtLog<<L"The Provider returned no Keywords.\n" ;
		COMPAREW(pwszKeywords!=NULL, TRUE);
		tTestResult = TEST_PASS;
		goto CLEANUP;
	}
	
	// Find the first keyword and loop thru all the Keywords
	wtoken = wcstok(pwszKeywords, L",");
	
	//If pwszKeywords is an empty string, FAIL test.
	if(!wtoken) 
	{
		odtLog<<L"The Provider should have returned a NULL instead of <EMPTY> for pwszKeywords." <<L"\n";
		goto CLEANUP;
	}

	// Loop thru all the Keywords
	while(wtoken)
	{
		// Static Keywords should not be in the list.
		for( i=0; i < (sizeof(g_rgpwszKeywords)/sizeof(g_rgpwszKeywords[0])); i++)
		{
			if(!(_wcsicmp(g_rgpwszKeywords[i], wtoken))) 
			{
				odtLog<<ENDL <<g_rgpwszKeywords[i] 
					<<L" should not be returned in the list." <<ENDL;
				goto CLEANUP;
			} 
		}

		//Get the next keyword
		wtoken = wcstok(NULL, L",");
	}

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(pwszKeywords);
	SAFE_RELEASE(pIDBInfo);
	return tTestResult;
} //testGetKeywords


//----------------------------------------------------------------------
//@mfunc Get the Literal information through IDBInfo and verify it.
//
TESTRESULT CQuickTest::testGetLiteralInfo()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			cErrors = 0;
	ULONG			ulIndex=0;
	ULONG			ulSupported = 0;
	ULONG			cLiteralInfo = 0;
	DBLITERALINFO*	rgLiteralInfo = NULL;
	WCHAR*			pCharBuffer = NULL;
	IDBInfo*		pIDBInfo = NULL;

	if(!VerifyInterface(m_pIDBInitialize,IID_IDBInfo,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBInfo))
	{
		odtLog<<L"IDBInfo is not supported.\n";
		return TEST_SKIPPED;
	}

	//Get information about ALL supported literals.
	TESTC_(hr=pIDBInfo->GetLiteralInfo(0, NULL, &cLiteralInfo,
			&rgLiteralInfo, &pCharBuffer), S_OK)
	
	if((cLiteralInfo==0) && (rgLiteralInfo==NULL))
	{
		//Issue a warning.
		odtLog<<L"WARNING:No literals are supported by this provider.\n";
		COMPAREW(cLiteralInfo > 0, TRUE);
		tTestResult = TEST_PASS;
		goto CLEANUP;
	}

	TESTC((cLiteralInfo !=0) && (rgLiteralInfo !=NULL))

	// Count the supported Literals
	for(ulIndex=0; ulIndex < cLiteralInfo; ulIndex++)
		if(rgLiteralInfo[ulIndex].fSupported)
			ulSupported++;

	//All the fSupported flags should have been TRUE.
	COMPARE(cLiteralInfo, ulSupported);

	// Check the pCharBuffer
	COMPARE(!pCharBuffer, NULL);

	// Check the Data returned
	for(ulIndex=0; ulIndex < cLiteralInfo; ulIndex++)
	{
		// These are the only 8 that should modify pwszLiteralValue and cchMaxLen, 
		// and not modify pwszInvalidChar and pwszInvalidStartingChar.
		if( (rgLiteralInfo[ulIndex].lt == DBLITERAL_CATALOG_SEPARATOR) ||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_PERCENT_PREFIX)||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_PERCENT_SUFFIX)||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_UNDERSCORE_PREFIX)||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_ESCAPE_UNDERSCORE_SUFFIX)||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_LIKE_PERCENT)      ||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_LIKE_UNDERSCORE)   ||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_QUOTE_PREFIX)      ||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_QUOTE_SUFFIX)	   ||
			(rgLiteralInfo[ulIndex].lt == DBLITERAL_SCHEMA_SEPARATOR) )
		{
			// Check pwszLiteralValue exceptions 
			if(!rgLiteralInfo[ulIndex].pwszLiteralValue)
			{
				odtLog<<L"ERROR: "<<g_rgpwszLiterals[rgLiteralInfo[ulIndex].lt]
					<<L" should not be NULL."<<"\n";
				cErrors++;
			}

			// Check pwszInvalidChar exceptions
			if(rgLiteralInfo[ulIndex].pwszInvalidChars)
			{
				odtLog<<L"ERROR: "<<g_rgpwszLiterals[rgLiteralInfo[ulIndex].lt]
					<<L" should be NULL."<<"\n";
				cErrors++;
			}

			// Check pwszInvalidStartingChar exceptions
			if(rgLiteralInfo[ulIndex].pwszInvalidStartingChars)
			{
				odtLog<<L"ERROR: "<<g_rgpwszLiterals[rgLiteralInfo[ulIndex].lt]
					<<L" should be NULL."<<"\n";
				cErrors++;
			}

			// Check cchMaxLen exceptions
			if( (rgLiteralInfo[ulIndex].pwszLiteralValue) && 
				(rgLiteralInfo[ulIndex].cchMaxLen != wcslen(rgLiteralInfo[ulIndex].pwszLiteralValue)))
			{
				odtLog<<L"ERROR: "<<g_rgpwszLiterals[rgLiteralInfo[ulIndex].lt]
					<<L" expect " <<rgLiteralInfo[ulIndex].cchMaxLen 
					<<L" but returned "<< wcslen(rgLiteralInfo[ulIndex].pwszLiteralValue)
					<<L" character(s)."<<"\n";
				cErrors++;
			}
		}
		else if(rgLiteralInfo[ulIndex].pwszLiteralValue)
		{
			odtLog<<L"ERROR: "<<g_rgpwszLiterals[rgLiteralInfo[ulIndex].lt]
				<<L" should be NULL."<<"\n";
			cErrors++;
		}
	}

	TESTC(cErrors == 0)
		
	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(pCharBuffer);
	PROVIDER_FREE(rgLiteralInfo);
	SAFE_RELEASE(pIDBInfo);
	return tTestResult;
} //testGetLiteralInfo


//----------------------------------------------------------------------
//@mfunc Get a sources rowset and verify its columns and rows..
//
TESTRESULT CQuickTest::testISrcRowset()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	USHORT				usBkmExists = 0; // 0=FALSE, 1=TRUE.
	USHORT				cCol = 0;
	DBORDINAL			cColumns = 0;
	DBCOLUMNINFO*		rgInfo = NULL;
	WCHAR*				pStringsBuffer = NULL;
	CLSID				clsid = CLSID_OLEDB_ENUMERATOR;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	DBBINDING*			rgBindings = NULL;
	BYTE*				pData = NULL;
	DBCOUNTITEM			cRowsObtained = 0;
	HROW*				rghRows = NULL;
	WCHAR*				pwszSourceParseName = NULL;
	USHORT				usSourceType = 0;
	DBCOUNTITEM			cTotalRows = 0;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	IColumnsInfo*		pICI = NULL;
	ISourcesRowset*		pISR = NULL;

		
	//Create an enumerator Object.
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
		IID_ISourcesRowset,(void **)&pISR) ;

	if(hr != S_OK)
	{
		TESTC(!pISR)
		odtLog<<L"Could not create enumerator object.\n";
		return TEST_SKIPPED;
	}

	TESTC(pISR != NULL)

	//Open Sources Rowset and obtain IColumnsInfo. 
	TESTC_(hr = pISR->GetSourcesRowset(NULL, IID_IColumnsInfo,
		0, NULL, (IUnknown **) &pICI), S_OK)

	TESTC(VerifyInterface(pICI,IID_IRowset,
		ROWSET_INTERFACE,(IUnknown**)&pIRowset))

	TESTC(VerifyInterface(pICI,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC_(pICI->GetColumnInfo(&cColumns, &rgInfo,
		&pStringsBuffer), S_OK)

	// Check to see if the Bookmark Property is on
	if (rgInfo[0].iOrdinal == 0)
	{
		COMPARE(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
			pICI), TRUE);
		usBkmExists = 1;
	}

	//Skip bookmark column, check each column info.
	//For each column check the position, column name and type.
	for(cCol=0; cCol<cColumns; cCol++)
	{
		switch(rgInfo[cCol].iOrdinal)
		{
		case 0:
			//Bookmark Column. Do nothing.
			break;
		case 1:
			COMPARE(cCol, 0+usBkmExists);
			COMPARE(wcscmp(rgInfo[cCol].pwszName, L"SOURCES_NAME"),0);
			COMPARE(rgInfo[cCol].wType, DBTYPE_WSTR);
			break;
		case 2:
			COMPARE(cCol, 1+usBkmExists);
			COMPARE(wcscmp(rgInfo[cCol].pwszName, L"SOURCES_PARSENAME"),0);
			COMPARE(rgInfo[cCol].wType, DBTYPE_WSTR);
			break;
		case 3:
			COMPARE(cCol, 2+usBkmExists);
			COMPARE(wcscmp(rgInfo[cCol].pwszName, L"SOURCES_DESCRIPTION"),0);
			COMPARE(rgInfo[cCol].wType, DBTYPE_WSTR);
			break;
		case 4:
			COMPARE(cCol, 3+usBkmExists);
			COMPARE(wcscmp(rgInfo[cCol].pwszName, L"SOURCES_TYPE"),0);
			COMPARE(rgInfo[cCol].wType, DBTYPE_UI2);
			break;
		case 5:
			COMPARE(cCol, 4+usBkmExists);
			COMPARE(wcscmp(rgInfo[cCol].pwszName, L"SOURCES_ISPARENT"),0);
			COMPARE(rgInfo[cCol].wType, DBTYPE_BOOL);
			break;
		default:
			break;
		}//switch
	}

	//Create an accessor on the sources rowset and make bindings
	TESTC_(GetAccessorAndBindings(pIAccessor,DBACCESSOR_ROWDATA,&hAccessor,
		&rgBindings,&cBindings,&cbRowSize,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_EXCEPTBOOKMARK,FORWARD,NO_COLS_BY_REF,NULL,NULL,
		NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,NO_BLOB_COLS),S_OK)

	// Allocate memory for the row data.
	SAFE_ALLOC(pData, BYTE, cbRowSize);

	// Loop over the rows and get data.
	while(GetNextRows(pIRowset, NULL,0,1,&cRowsObtained,&rghRows) == S_OK)
	{
		//Clear pData.
		memset(pData, 0, (size_t) cbRowSize);

		// Get the next row 
		TESTC_(hr=pIRowset->GetData(rghRows[0],hAccessor,pData), S_OK)

		cTotalRows++;
		
		//Get name, description and type of the source.
		pwszSourceParseName = (WCHAR*)(pData + rgBindings[1].obValue) ;
		usSourceType = *(USHORT*)(pData + rgBindings[3].obValue) ;

		TESTC(wcslen(pwszSourceParseName) > 0)

		TESTC(usSourceType==DBSOURCETYPE_DATASOURCE ||
			  usSourceType==DBSOURCETYPE_ENUMERATOR ||
			  usSourceType==DBSOURCETYPE_DATASOURCE_TDP ||
			  usSourceType==DBSOURCETYPE_DATASOURCE_MDP	)

		//Release the row handle
		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	}//while ends.

	odtLog<<L"INFO: Total rows in Sources Rowset = "<<cTotalRows<<"\n" ;

	TESTC(cTotalRows >= 1)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICI);
	SAFE_RELEASE(pISR);
	return tTestResult;
} //testISrcRowset


//----------------------------------------------------------------------
//@mfunc Create a data source object using IDataInitialize::CreateDBInstance
//and verify it.
//
TESTRESULT CQuickTest::testIDataIzCreateDBIns()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	CLSID				clsidProv, clsid;
	ULONG				cPropInfoSets = 0;
	DBPROPINFOSET*		rgPropInfoSets = NULL;
	IDataInitialize*	pIDI = NULL;
	IDBInitialize*		pIDBI = NULL;
	IDBProperties*		pIDBProperties = NULL;
	IPersist*			pIPersist = NULL;

	hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDataInitialize, 
		(void**)&pIDI) ;

	if(hr != S_OK)
	{
		TESTC(!pIDI)
		odtLog<<L"Could not create the DataInitialize object.\n";
		return TEST_SKIPPED;
	}

	TESTC(pIDI != NULL)

	TESTC(VerifyOutputInterface(S_OK, IID_IDataInitialize, 
		(IUnknown**)&pIDI))

	clsidProv = GetModInfo()->GetThisTestModule()->m_ProviderClsid ;

	//Create the DSO. Request IDBInitialize.
	TESTC_(hr = pIDI->CreateDBInstance(clsidProv, NULL, 
		CLSCTX_INPROC_SERVER, NULL, IID_IDBInitialize, 
		(IUnknown**)&pIDBI), S_OK)

	TESTC(pIDBI != NULL)

	//Get IDBProperties.
	TESTC(VerifyInterface(pIDBI,IID_IDBProperties,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBProperties))

	TESTC(pIDBProperties != NULL)

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	//Obtain Required Init Properties (from the Init String)
	TESTC(GetModInfo()->GetInitProps(&m_cPropSets, &m_rgPropSets))

	//Set the Init properties.
	TESTC_(hr = pIDBProperties->SetProperties(m_cPropSets, m_rgPropSets),
		S_OK)

	//Initialize the DSO.
	TESTC_(hr = pIDBI->Initialize(), S_OK)

	//GetPropertyInfo.
	TESTC_(hr = pIDBProperties->GetPropertyInfo(0, NULL, &cPropInfoSets,
		&rgPropInfoSets, NULL), S_OK)

	if(cPropInfoSets == 0 && rgPropInfoSets == NULL)
	{	
		odtLog<<L"No PropertyInfo sets were returned.\n";
		COMPAREW(cPropInfoSets, 1);
	}
	else
	{
		TESTC(cPropInfoSets > 0 && rgPropInfoSets != NULL)
	}

	//Create another DSO. This time request IPersist.
	TESTC_(hr = pIDI->CreateDBInstance(clsidProv, NULL, 
		CLSCTX_INPROC_SERVER, NULL, IID_IPersist, 
		(IUnknown**)&pIPersist), S_OK)

	TESTC(pIPersist != NULL)

	//Get class ID and verify that CLSID returned is identical to 
	//the Provider CLSID.			
	TESTC_(pIPersist->GetClassID(&clsid), S_OK)

	TESTC(clsid == GetModInfo()->GetThisTestModule()->m_ProviderClsid)	

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	FreeProperties(&cPropInfoSets, &rgPropInfoSets);
	SAFE_RELEASE(pIDBI);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIPersist);
	SAFE_RELEASE(pIDI);
	return tTestResult;
} //testIDataIzCreateDBIns


//----------------------------------------------------------------------
//@mfunc Construct an initialization string from the init properties and
//pass that in to IDataInitialize::GetDataSource. Check the obtained DSO.
//Then call ::GetInitializationString (requesting password) and make sure
//it contains the password.
//
TESTRESULT CQuickTest::testIDataIzGetDS()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG				i, j;
	CLSID				clsid;
	BOOL				bDefault = FALSE;
	DBPROPSET*			pPropSet = NULL;
	DBPROP*				pProp = NULL;
	ULONG				cPropInfoSets = 0;
	DBPROPINFOSET*		rgPropInfoSets = NULL;
	ULONG				cPropSets = 0;
	DBPROPSET*			rgPropSets = NULL;
	WCHAR*				pwszProgID = NULL;
	WCHAR*				pwszInitStr1 = NULL;
	WCHAR*				pwszInitStr2 = NULL;
	WCHAR*				pwszInitString = NULL;
	WCHAR*				pwszValue = NULL;
	WCHAR*				pwszValue1 = NULL;
	WCHAR*				pwszDefPassword = NULL;
	IDataInitialize*	pIDI = NULL;
	IDBInitialize*		pIDBI = NULL;
	IDBProperties*		pIDBProperties = NULL;
	IUnknown*			pIUnknown = NULL;

	hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDataInitialize, 
		(void**)&pIDI) ;

	if(hr != S_OK)
	{
		TESTC(!pIDI)
		odtLog<<L"Could not create the DataInitialize object.\n";
		return TEST_SKIPPED;
	}

	TESTC(pIDI != NULL)

	TESTC(VerifyOutputInterface(S_OK, IID_IDataInitialize, 
		(IUnknown**)&pIDI))

	//Get Init props to construct the Init string.
	TESTC(GetModInfo()->GetInitProps(&cPropSets, &rgPropSets))

	//Required for the function SetSupportedProperty.
	g_pIDBInitialize = m_pIDBInitialize ;

	//Required for obtaining the Password. This will be added
	//to the init string. If this property is not supported,
	//then the password will be visible anyway.
	SetSettableProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO,
		DBPROPSET_DBINIT, &cPropSets, &rgPropSets) ;

	clsid = GetModInfo()->GetThisTestModule()->m_ProviderClsid ;

	TESTC_(ProgIDFromCLSID(clsid, &pwszProgID),S_OK);

	//Add the provider keyword and value to Init string.
	pwszInitString = CreateString(L"Provider = %s; ", pwszProgID);

	//Loop through specified properties and construct the String
	for(i=0; i<cPropSets; i++)
	{
		pPropSet = &rgPropSets[i];
		for(j=0; j<pPropSet->cProperties; j++)
		{
			pProp = &pPropSet->rgProperties[j];
			
			//Now Add this Property
			AppendToInitString(&pwszInitString, pProp->dwPropertyID, pPropSet->guidPropertySet, &pProp->vValue);
			TESTC(pwszInitString != NULL);
		}
	}

	//If no properties, just use an empty string, since GetDataSource 
	//fails on NULL
	AppendString(&pwszInitString, cPropSets ? L";" : L"");

	

	TESTC(pwszInitString != NULL)
	odtLog<<L"Init String: "<<pwszInitString<<L"\n";
	//Create the DSO.
	TESTC_(hr = pIDI->GetDataSource(NULL, CLSCTX_INPROC_SERVER,
		pwszInitString, IID_IDBInitialize, (IUnknown**) &pIDBI), S_OK)

	TESTC(pIDBI != NULL)

	//Call Initilize().
	TESTC_(hr = pIDBI->Initialize(), S_OK)

	//Get IDBProperties.
	TESTC(VerifyInterface(pIDBI,IID_IDBProperties,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBProperties))

	TESTC(pIDBProperties != NULL)

	//Call GetPropertyInfo.
	TESTC_(hr = pIDBProperties->GetPropertyInfo(0, NULL, &cPropInfoSets,
		&rgPropInfoSets, NULL), S_OK)

	//Check returned values.
	if(cPropInfoSets == 0 && rgPropInfoSets == NULL)
	{
		COMPAREW(cPropInfoSets, 1);
		odtLog<<L"No PropertyInfo sets were returned.\n";
	}
	else
	{
		TESTC(cPropInfoSets > 0 && rgPropInfoSets != NULL)
	}

	//Call GetInitializationString with fIncludePassword=TRUE.
	TESTC_(hr = pIDI->GetInitializationString((IUnknown*)pIDBI,
		TRUE, &pwszInitStr1), S_OK)

	TESTC(pwszInitStr1 != NULL)
	TESTC(wcslen(pwszInitStr1) > 0)

	//Make sure the password is the same as the one we had used to 
	//create the DSO.
	if(GetModInfo()->GetStringKeywordValue(pwszInitString, L"Password",
		&pwszValue) && pwszValue)
	{
		TESTC_(GetModInfo()->CreateProvider(clsid, NULL, IID_IUnknown, &pIUnknown),S_OK);
		
		//Verify this property is the default on the DataSource
		COMPARE(GetProperty(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, pIUnknown, &pwszDefPassword), TRUE);

		if((wcscmp(pwszValue, L"")==0) ||  
			(pwszDefPassword && (wcscmp(pwszDefPassword, pwszValue)==0)))
			bDefault = TRUE;

		if(!bDefault)
		{
			TESTC(GetModInfo()->GetStringKeywordValue(pwszInitStr1, 
				L"Password", &pwszValue1))

			TESTC(wcscmp(pwszValue, pwszValue1) == 0)
		}
	}

	//Write the Init string into a file.
	TESTC_(hr = pIDI->WriteStringToStorage(L"This is a long file name for QuikTest.tst",
		pwszInitStr1, CREATE_ALWAYS), S_OK)

	//Load the init string from the file.
	TESTC_(hr = pIDI->LoadStringFromStorage(L"This is a long file name for QuikTest.tst",
		&pwszInitStr2), S_OK)

	//Make sure they are the same.
	TESTC(wcscmp(pwszInitStr1, pwszInitStr2) == 0)

	tTestResult = TEST_PASS;

CLEANUP:
	remove("This is a long file name for QuikTest.tst");
	FreeProperties(&cPropInfoSets, &rgPropInfoSets);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszDefPassword);
	SAFE_FREE(pwszProgID);
	SAFE_FREE(pwszInitString);
	SAFE_FREE(pwszInitStr1);
	SAFE_FREE(pwszInitStr2);
	SAFE_FREE(pwszValue);
	SAFE_FREE(pwszValue1);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIDBI);
	SAFE_RELEASE(pIDI);
	return tTestResult;
} //testIDataIzGetDS


//----------------------------------------------------------------------
//@mfunc Use the IGetDataSource::GetDataSource method to obtain pointers
//to IDBInitialize, IDBProperties and IPersist interfaces. Verify the 
//obtained interfaces. Check if the obtained IDBInitialize is the same 
//the one we had used to initialize the Data Source Object. Use the 
//obtained IPersist interface to get the class ID and compare it to the 
//provider class ID. Use the IDBProperties to get the value of 
//DBPROP_INIT_PROMPT property.
//
TESTRESULT CQuickTest::testGetDataSource()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	CLSID			clsid;
	IGetDataSource*	pIGetDataSource = NULL;
	IDBInitialize*	pIDBInitialize=NULL;
	IDBProperties*	pIDBProperties=NULL;
	IDBProperties*	pIDBProp2=NULL;
	IPersist*		pIPersist=NULL;

	VARIANT vVar;
	VariantInit(&vVar);

	//Obtain the IGetDataSource interface.
	if(!VerifyInterface(m_pIOpenRowset,IID_IGetDataSource, 
		SESSION_INTERFACE,(IUnknown **)&pIGetDataSource))
	{
		odtLog<<L"INFO: Could not get IGetDataSource interface.\n";
		return TEST_FAIL;
	}

	//Use method to get IDBInitialize.
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBInitialize,
		(IUnknown**)&pIDBInitialize), S_OK)

	//The IDBInitialize obtained through this method should be the
	//same as m_pIDBInitialize.
	COMPARE(VerifyEqualInterface((IUnknown*)pIDBInitialize, 
		(IUnknown*)m_pIDBInitialize), TRUE);

	//Use method to get IPersist.
	TESTC_(pIGetDataSource->GetDataSource(IID_IPersist,
		(IUnknown**)&pIPersist), S_OK)

	//Use the obtained IPersist interface to get class ID.
	//Verify that CLSID returned is identical to the Provider CLSID			
	CHECK(pIPersist->GetClassID(&clsid), S_OK);
	COMPARE(clsid, GetModInfo()->GetThisTestModule()->m_ProviderClsid);

	//Use method to get IDBProperties. Then use the obtained IDBProperties
	//to get the DBPROP_INIT_PROMPT property and check its variant value.
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBProperties,
		(IUnknown**)&pIDBProperties), S_OK)

	//Obtain the IGetDataSource interface.
	TESTC(VerifyInterface(m_pIDBInitialize,IID_IDBProperties, 
		DATASOURCE_INTERFACE,(IUnknown **)&pIDBProp2))

	//Verify the interface. 
	COMPARE(VerifyEqualInterface(pIDBProperties, pIDBProp2), TRUE);

	//Try to get the PROMPT property using the obtained IDBProperties.
	if(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT,
		pIDBProperties, &vVar))
		TESTC(checkPositive_I2(&vVar))

	tTestResult = TEST_PASS;

CLEANUP:
	VariantClear(&vVar);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBProp2);
	SAFE_RELEASE(pIPersist);
	SAFE_RELEASE(pIDBInitialize);
	return tTestResult;
} //testGetDataSource


//----------------------------------------------------------------------
//@mfunc Create a table. Set two properties (BOOKMARKS, CANFETCHBACKWARDS).
//Generate a rowset on the table with these properties. Then set three 
//properties (BOOKMARKS, CANSCROLLBACKWARDS, OWNUPDATEDELETE). Generate 
//a second rowset on the same table, with these three properties. Verify
//the two rowsets exist and are different. Also open rowsets with each 
//of the mandatory interfaces of Rowset object.
//
TESTRESULT CQuickTest::testOpenRowset()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	IRowset*			pIRowset = NULL;
	IRowsetInfo*		pIRowsetInfo = NULL;
	IAccessor*			pIAccessor = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;
	IConvertType*		pIConvertType = NULL;

	TESTC(m_pTable != NULL)

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	//Set 2 properties.
    SetSettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets,
		&m_rgPropSets);
	SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets);
    
    //Call OpenRowset #1
    hr = CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)&pIRowset);
	CHECK(hr, S_OK);

	FreeProperties(&m_cPropSets, &m_rgPropSets);

    //Set 3 properties
    SetSettableProperty(DBPROP_OWNUPDATEDELETE, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets);
    SetSettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets);
    SetSettableProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets);
    
    //Call OpenRowset #2
    hr = CreateOpenRowset(m_pTable, IID_IRowsetInfo,(IUnknown**)
		&pIRowsetInfo);
	CHECK(hr, S_OK);
    
    //Verify different rowset pointers
	COMPARE(VerifyEqualInterface(pIRowset, pIRowsetInfo), FALSE);

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	//Call OpenRowset #3
    hr = CreateOpenRowset(m_pTable, IID_IAccessor,(IUnknown**)
		&pIAccessor);
	CHECK(hr, S_OK);

	//Call OpenRowset #4
    hr = CreateOpenRowset(m_pTable, IID_IColumnsInfo,(IUnknown**)
		&pIColumnsInfo);
	CHECK(hr, S_OK);

	//Call OpenRowset #5
    hr = CreateOpenRowset(m_pTable, IID_IConvertType,(IUnknown**)
		&pIConvertType);
	CHECK(hr, S_OK);

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIConvertType);
	return tTestResult;
} //testOpenRowset


//---------------------------------------------------------------------
//@mfunc Get the DBPROP_SESS_AUTOCOMMITISOLEVELS property by creating 
//an appropriate PROPIDSET and passing it to the GetProperties method 
//call. Verify the returned property and values. Then call the 
//GetProperties method with (0,NULL) to get all supported session 
//properties. Verify the returned values and properties.
//
TESTRESULT CQuickTest::testSessGetProp()
{
	TESTRESULT	tTestResult = TEST_FAIL;
	HRESULT		hr = S_OK;
	ULONG		cPropSets = 0;
	ULONG		iSet=0, iProp=0 ;
	DBPROP*		pProp = NULL;
	DBPROPSET*	rgPropSets = NULL;
	ISessionProperties*	pISessProp = NULL;

	//Obtain ISessionProperties interface.
	if(!VerifyInterface(m_pIOpenRowset,IID_ISessionProperties, 
		SESSION_INTERFACE,(IUnknown **)&pISessProp))
	{
		odtLog<<L"Could not get ISessionProperties interface.\n";
		return TEST_FAIL;
	}

	//make DBPROPIDSET with one property in it.
	SetPropID(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION);

	//Call GetProperties with the above made DBPROPIDSET.
	hr = pISessProp->GetProperties(m_cPropIDSets, m_rgPropIDSets, 
		&cPropSets, &rgPropSets) ;

	//Verify that 1 DBPROPSET was returned.
	TESTC((cPropSets==1)&&(rgPropSets != NULL))

	//Verify the guid of PropertySet.
	COMPARE(rgPropSets[0].guidPropertySet, DBPROPSET_SESSION);

	//Verify number of properties.
	COMPARE(rgPropSets[0].cProperties, 1);

	if(hr==S_OK)
	{
		//Verify status.
		COMPARE(rgPropSets[0].rgProperties[0].dwStatus,
			DBPROPSTATUS_OK);

		//Verify the variant.
		COMPARE((V_VT(&rgPropSets[0].rgProperties[0].vValue)
			== VT_I4), TRUE);
		
		//The value is a bitmask. So check if it is >= 0.
		COMPARE((V_I4(&rgPropSets[0].rgProperties[0].vValue)
			>= 0), TRUE);
	}
	else
	{
		odtLog<<L"DBPROP_SESS_AUTOCOMMITISOLEVELS is not supported.\n";
		CHECK(hr, DB_E_ERRORSOCCURRED);
		COMPARE(SupportedProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, 
			DBPROPSET_SESSION), FALSE);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	//Get ALL session properties.
	TESTC_(pISessProp->GetProperties(0, NULL, &cPropSets, 
		&rgPropSets), S_OK)

	//Check return values.
	if(cPropSets==0 && rgPropSets==NULL)
	{
		//Issue a warning since no Property Sets were returned.
		odtLog<<L"WARNING: No Property Sets were returned.\n";
		COMPAREW(cPropSets, 1);
		tTestResult = TEST_PASS;
		goto CLEANUP;
	}
	TESTC((cPropSets!=0) && (rgPropSets!=NULL))

	//Loop through the property sets.
	for(iSet=0; iSet<cPropSets; iSet++)
	{
		//Check to see if the Provider has returned an invalid PropSets
		if( (rgPropSets[iSet].guidPropertySet == DBPROPSET_COLUMN) || 
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCE)    ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCEINFO)||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DBINIT) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_INDEX)  ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_ROWSET) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_TABLE) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCEALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_ROWSETALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_SESSIONALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DBINITALL))
		{
			odtLog<<L"ERROR: An Invalid OLEDB PropertySet has been returned by ISessionProperties::GetProperties!\n";
			goto CLEANUP;
		}

		if(rgPropSets[iSet].guidPropertySet != DBPROPSET_SESSION)
		{
			odtLog<<L"INFO: Found a provider specific property set.\n";
		}

		//Loop through the properties.
		for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(rgPropSets[iSet].rgProperties[iProp]) ;

			//Verify if status is DBPROPSTATUS_OK.
			COMPARE(pProp->dwStatus, DBPROPSTATUS_OK);

			//For the property DBPROP_SESS_AUTOCOMMITISOLEVELS, verify
			//if the variant type is VT_I4 or VT_EMPTY.
			if((rgPropSets[iSet].guidPropertySet == DBPROPSET_SESSION)&&
				(pProp->dwPropertyID==DBPROP_SESS_AUTOCOMMITISOLEVELS))
			{
				COMPARE((V_VT(&pProp->vValue) == VT_I4), TRUE);
			}

			//For certain variant types do some checking.
			COMPARE(CheckVariant(&pProp->vValue), TRUE);
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&m_cPropIDSets, &m_rgPropIDSets);
	SAFE_RELEASE(pISessProp);
	return tTestResult;
} //testSessGetProp


//---------------------------------------------------------------------
//@mfunc Set the property DBPROP_SESS_AUTOCOMMITISOLEVELS to a valid 
//value. After setting the property call GetProperty on it and verify.
//
TESTRESULT CQuickTest::testSessSetProp()
{
	TESTRESULT		tTestResult = TEST_FAIL; //Test Return Value
	HRESULT			hr = S_OK;
	ULONG			cPropSets = 1;
	DBPROP			dwProp;
	DBPROPSET		rgPropSets[1];
	ISessionProperties*	pISessProp = NULL;

	//Initialize the variant to pass as an [OUT] parameter to GetProperty.
	VARIANT		vVar;
	VariantInit(&vVar);

	//Obtain ISessionProperties Interface.
	if(!VerifyInterface(m_pIOpenRowset,IID_ISessionProperties, 
		SESSION_INTERFACE,(IUnknown **)&pISessProp))
	{
		odtLog<<L"Could not get ISessionProperties interface.\n";
		return TEST_FAIL;
	}

	//Create and initialize the DBPROPSET to pass in to SetProperties.
	memset(&dwProp, 0, sizeof(DBPROP)) ;
	dwProp.dwPropertyID = DBPROP_SESS_AUTOCOMMITISOLEVELS;
	dwProp.dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp.vValue);
	V_VT(&dwProp.vValue) = DBTYPE_I4;
	V_I4(&dwProp.vValue) = DBPROPVAL_TI_READCOMMITTED;
	rgPropSets[0].rgProperties = &dwProp;
	rgPropSets[0].cProperties = 1;
	rgPropSets[0].guidPropertySet = DBPROPSET_SESSION;

	//Set the property.
	hr = pISessProp->SetProperties(cPropSets, rgPropSets);

	if(hr==S_OK)
	{
		//Check if the status is DBPROPSTATUS_OK.
		COMPARE(rgPropSets[0].rgProperties[0].dwStatus,
			DBPROPSTATUS_OK);

		//Call GetProperty to get the variant value of 
		//DBPROP_SESS_AUTOCOMMITISOLEVELS.
		COMPARE(GetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS,
			DBPROPSET_SESSION, (IUnknown*)pISessProp, &vVar), TRUE);

		//Compare the variant value got by above call to GetProperty 
		//and the one created (pVar) to pass in to SetProperty.
		COMPARE(CompareVariant(&vVar, &dwProp.vValue), TRUE);
	}
	else
	{
		odtLog<<L"DBPROP_SESS_AUTOCOMMITISOLEVELS is not supported or not settable.\n";

		TESTC_(hr, DB_E_ERRORSOCCURRED)

		//The status should be NOTSUPPORTED or NOTSETTABLE.
		COMPARE((rgPropSets[0].rgProperties[0].dwStatus == 
			DBPROPSTATUS_NOTSUPPORTED) ||
			(rgPropSets[0].rgProperties[0].dwStatus == 
			DBPROPSTATUS_NOTSETTABLE), TRUE);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	VariantClear(&vVar);
	SAFE_RELEASE(pISessProp);
	return tTestResult;
} //testSessSetProp


//----------------------------------------------------------------------
//@mfunc Create multiple command objects using method CreateCommand. Each
//time request a different Command interface. Do this for all mandatory
//command interfaces.
//
TESTRESULT CQuickTest::testCreateCommand()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ICommand*		pICommand = NULL;
	ICommandText*	pICommandText = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;
	IAccessor*		pIAccessor = NULL;
	IConvertType*	pIConvertType = NULL;
	ICommandProperties*	pICommandProperties = NULL;

	if(m_pIDBCreateCommand == NULL)
	{
		odtLog<<L"IDBCreateCommand pointer is NULL.\n";
		return TEST_FAIL;
	}

	// CreateCommand with IID_ICommand
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICommand), S_OK);
	TESTC(pICommand != NULL)

	// CreateCommand with IID_ICommandText
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, 
		(IUnknown**)&pICommandText), S_OK);
	TESTC(pICommandText != NULL)

	// CreateCommand with IID_IColumnsInfo
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_IColumnsInfo, 
		(IUnknown**)&pIColumnsInfo), S_OK);
	TESTC(pIColumnsInfo != NULL)

	// CreateCommand with IID_IAccessor
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_IAccessor, 
		(IUnknown**)&pIAccessor), S_OK);
	TESTC(pIAccessor != NULL)

	// CreateCommand with IID_IConvertType
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_IConvertType, 
		(IUnknown**)&pIConvertType), S_OK);
	TESTC(pIConvertType != NULL)

	// CreateCommand with IID_ICommandProperties
	CHECK(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandProperties, 
		(IUnknown**)&pICommandProperties), S_OK);
	TESTC(pICommandProperties != NULL)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIConvertType);
	SAFE_RELEASE(pICommandProperties);
	return tTestResult;
} //testCreateCommand


//----------------------------------------------------------------------
//@mfunc Get the supported schemas. Make sure the 3 mandatory ones 
//(DBSCHEMA_TABLES, DBSCHEMA_COLUMNS and DBSCHEMA_PROVIDER_TYPES) are
//there.
//
TESTRESULT CQuickTest::testGetSchemas()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	ULONG				fRequired = 0; //a bit mask to mark existing schema.
	ULONG				ulIndex = 0;
	GUID *				rgSchemas = NULL;
	ULONG				cSchemas = 0;
	ULONG *				rgRestrictionSupport = NULL;
	IDBSchemaRowset*	pIDBSR = NULL;

	//Obtain IDBSchemaRowset interface 
    if(!VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDBSR))
	{
		odtLog<<L"IDBSchemaRowset is not supported.\n";
		return TEST_SKIPPED;
	}

	//Call the method.
	TESTC_(pIDBSR->GetSchemas(&cSchemas,&rgSchemas,
		&rgRestrictionSupport),S_OK)

	TESTC((cSchemas > 2) && (rgSchemas) && (rgRestrictionSupport))

	for(ulIndex=0; ulIndex<cSchemas; ulIndex++)
	{
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_TABLES))
		{
			COMPARE(fRequired & 0x1, 0);

			fRequired |= 0x1 ; //Mark "This schema exists".

			//There can be a maximum of 4 restrictions. Hence the 
			//bitmask can have a max value of 15.
			COMPARE(rgRestrictionSupport[ulIndex] < 16, TRUE);
		}
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_COLUMNS))
		{
			COMPARE(fRequired & 0x2, 0);

			fRequired |= 0x2 ;
			//There can be a maximum of 4 restrictions. Hence the 
			//bitmask can have a max value of 15.
			COMPARE(rgRestrictionSupport[ulIndex] < 16, TRUE);
		}
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_PROVIDER_TYPES))
		{
			COMPARE(fRequired & 0x4, 0);

			fRequired |= 0x4 ;
			//There can be a maximum of 2 restrictions. Hence the 
			//bitmask can have a max value of 3.
			COMPARE(rgRestrictionSupport[ulIndex] < 4, TRUE);
		}
	}

	//Make sure all 3 required schemas were found.
	TESTC(fRequired == 7)

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	SAFE_RELEASE(pIDBSR);
	return tTestResult;
} //testGetSchemas


//----------------------------------------------------------------------
//@mfunc Get the DBSCHEMA_COLUMNS rowset with certain restrictions. 
//Verify the obtained rowset.
//
TESTRESULT CQuickTest::testColumnsSchemaRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cRowsObtained = 0;
	DBCOUNTITEM		cGotRows = 0;
	DBCOUNTITEM		ulIndex = 0;
	HROW*			rghRows = NULL;
	BYTE*			pData = NULL;
	LONG_PTR		rgColsToBind[1];
	VARIANT			rgRestrictions[4];
	WCHAR*			pwszTableName = NULL;
	ULONG			ulRestrictCOL = 0;
	GUID *			rgSchemas = NULL;
	ULONG			cSchemas = 0;
	ULONG *			rgRestrictionSupport = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*		rgBindings = NULL;
	IAccessor*		pIAccessor = NULL;
	IRowset*		pIRowset = NULL;
	IDBSchemaRowset*	pIDBSR = NULL;

	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantInit(&rgRestrictions[ulIndex]);

	//Obtain IDBSchemaRowset interface 
    if(!VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDBSR))
	{
		odtLog<<L"IDBSchemaRowset is not supported.\n";
		return TEST_SKIPPED;
	}

	//Call the method.
	TESTC_(hr=pIDBSR->GetSchemas(&cSchemas,&rgSchemas,
		&rgRestrictionSupport),S_OK)

	TESTC((cSchemas > 2) && (rgSchemas) && (rgRestrictionSupport))

	//Get the restiction support.
	for(ulIndex=0; ulIndex<cSchemas; ulIndex++)
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_COLUMNS))
			ulRestrictCOL = rgRestrictionSupport[ulIndex];

	TESTC(m_pTable != NULL)

	//Table name to be used as restriction.
	pwszTableName = m_pTable->GetTableName();

	//Bind only 3rd column. This is the column on which we will have the
	//restriction.
	rgColsToBind[0] = 3; 

	//If the table name restriction is supported ...
	if(ulRestrictCOL & 0x4)
	{
		rgRestrictions[2].vt = VT_BSTR;

		//Allocate memory for BSTR. Will be freed by VariantClear.
		SAFE_SYSALLOC(rgRestrictions[2].bstrVal, pwszTableName); 
	}

	//Get the schema rowset with 1 restriction (if supported).
	TESTC_(hr=pIDBSR->GetRowset(NULL, DBSCHEMA_COLUMNS,4,rgRestrictions, 
		IID_IRowset, 0, NULL, (IUnknown**)
		&pIRowset), S_OK)

	TESTC(pIRowset != NULL)

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using only value. 
	//This binds the 3rd column only.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 1, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)

	while(S_OK==(hr=GetNextRows(pIRowset, NULL, 0, 1, &cRowsObtained, &rghRows)))
	{
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		TESTC_(hr = pIRowset->GetData(*rghRows, hAccessor, 
			pData),S_OK)
		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);

		//If the restriction was supported, make sure it was followed.
		if(ulRestrictCOL & 0x4)
			TESTC(wcsstr((WCHAR*)((BYTE *)pData+rgBindings[0].obValue), 
				pwszTableName) != NULL)

		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings,
			pData), S_OK);

		cGotRows++;
	}

	TESTC_(hr, DB_S_ENDOFROWSET)
	TESTC(cGotRows > 0)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantClear(&rgRestrictions[ulIndex]);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSR);
	return tTestResult;
} //testColumnsSchemaRowset


//----------------------------------------------------------------------
//@mfunc Get the DBSCHEMA_TABLES rowset with certain restrictions. 
//Verify the obtained rowset.
//
TESTRESULT CQuickTest::testTablesSchemaRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cRowsObtained = 0;
	DBCOUNTITEM		cGotRows = 0;
	DBCOUNTITEM		ulIndex = 0;
	HROW*			rghRows = NULL;
	BYTE*			pData = NULL;
	LONG_PTR		rgColsToBind[2];
	VARIANT			rgRestrictions[4];
	WCHAR*			pwszTableName = NULL;
	ULONG			ulRestrictTBL = 0;
	GUID *			rgSchemas = NULL;
	ULONG			cSchemas = 0;
	ULONG *			rgRestrictionSupport = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*		rgBindings = NULL;
	IAccessor*		pIAccessor = NULL;
	IRowset*		pIRowset = NULL;
	IDBSchemaRowset*	pIDBSR = NULL;

	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantInit(&rgRestrictions[ulIndex]);

	//Obtain IDBSchemaRowset interface 
    if(!VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDBSR))
	{
		odtLog<<L"IDBSchemaRowset is not supported.\n";
		return TEST_SKIPPED;
	}

	//Call the method.
	TESTC_(hr=pIDBSR->GetSchemas(&cSchemas,&rgSchemas,
		&rgRestrictionSupport),S_OK)

	TESTC((cSchemas > 2) && (rgSchemas) && (rgRestrictionSupport))

	//Get the restriction support.
	for(ulIndex=0; ulIndex<cSchemas; ulIndex++)
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_TABLES))
			ulRestrictTBL = rgRestrictionSupport[ulIndex];

	TESTC(m_pTable != NULL)

	//Table name to use for restriction.
	pwszTableName = m_pTable->GetTableName();

	//Bind only 3rd and 4th columns. 
	rgColsToBind[0] = 3;
	rgColsToBind[1] = 4;

	//If the table name restriction is supported ...
	if(ulRestrictTBL & 0x4)
	{
		rgRestrictions[2].vt = VT_BSTR;

		//Allocate memory for BSTR. Will be freed by VariantClear.
		SAFE_SYSALLOC(rgRestrictions[2].bstrVal, pwszTableName); 
	}

	//If the table type restriction is supported ...
	if(ulRestrictTBL & 0x8)
	{
		rgRestrictions[3].vt = VT_BSTR;

		//Allocate memory for BSTR. Will be freed by VariantClear.
		SAFE_SYSALLOC(rgRestrictions[3].bstrVal, L"TABLE"); 
	}

	//Get the schema rowset with 2 restrictions (if supported).
	TESTC_(hr=pIDBSR->GetRowset(NULL, DBSCHEMA_TABLES,4,rgRestrictions, 
		IID_IRowset, 0, NULL, (IUnknown**)
		&pIRowset), S_OK)

	TESTC(pIRowset != NULL)

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using only value. 
	//This binds the 3rd column only.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 2, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)

	while(S_OK==(hr=GetNextRows(pIRowset, NULL, 0, 1, &cRowsObtained, &rghRows)))
	{
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		TESTC_(hr = pIRowset->GetData(*rghRows, hAccessor, 
			pData),S_OK)
		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);

		//If the restriction was supported, make sure it was followed.
		if(ulRestrictTBL & 0x4)
			TESTC(wcsstr((WCHAR*)((BYTE *)pData+rgBindings[0].obValue), 
				pwszTableName) != NULL)

		//If the restriction was supported, make sure it was followed.
		if(ulRestrictTBL & 0x8)
			TESTC(wcsstr((WCHAR*)((BYTE *)pData+rgBindings[1].obValue), 
				L"TABLE") != NULL)

		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings,
			pData), S_OK);

		cGotRows++;
	}

	TESTC_(hr, DB_S_ENDOFROWSET)
	if(cGotRows == 0)
	{
		odtLog<<L"WARNING: The number of rows in the Tables Schema Rowset with restrictions TableName = "<<pwszTableName<<L" and TableType = TABLE was "<<cGotRows<<".\n";
		COMPAREW(cGotRows > 0, TRUE);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantClear(&rgRestrictions[ulIndex]);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSR);
	return tTestResult;
} //testTablesSchemaRowset


//----------------------------------------------------------------------
//@mfunc Get the DBSCHEMA_PROVIDER_TYPES rowset with certain restrictions. 
//Verify the obtained rowset.
//
TESTRESULT CQuickTest::testProvTypesSchemaRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	BYTE*			pData = NULL;
	LONG_PTR		rgColsToBind[1];
	VARIANT			rgRestrictions[2];
	ULONG			ulIndex = 0;    //index for no. of schemas
	ULONG			ulRestrictPTY = 0;
	GUID *			rgSchemas = NULL;
	ULONG			cSchemas = 0;
	ULONG *			rgRestrictionSupport = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*		rgBindings = NULL;
	IAccessor*		pIAccessor = NULL;
	IRowset*		pIRowset = NULL;
	IDBSchemaRowset*	pIDBSR = NULL;

	VariantInit(&rgRestrictions[0]);
	VariantInit(&rgRestrictions[1]);

	//Obtain IDBSchemaRowset interface 
    if(!VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDBSR))
	{
		odtLog<<L"IDBSchemaRowset is not supported.\n";
		return TEST_SKIPPED;
	}

	//Call the method.
	TESTC_(hr=pIDBSR->GetSchemas(&cSchemas,&rgSchemas,
		&rgRestrictionSupport),S_OK)

	TESTC((cSchemas > 2) && (rgSchemas) && (rgRestrictionSupport))

	for(ulIndex=0; ulIndex<cSchemas; ulIndex++)
		if(IsEqualGUID(rgSchemas[ulIndex], DBSCHEMA_PROVIDER_TYPES))
			ulRestrictPTY = rgRestrictionSupport[ulIndex];

	//Bind only 2nd column.
	rgColsToBind[0] = 2;  

	//If the data type restriction is supported.
	if(ulRestrictPTY & 0x1)
	{
		rgRestrictions[0].vt = VT_UI2;
		V_UI2(&(rgRestrictions[0])) = DBTYPE_BYTES ; 
	}

	//Get the schema rowset with 1 restriction (if supported).
	TESTC_(hr=pIDBSR->GetRowset(NULL, DBSCHEMA_PROVIDER_TYPES,2,
		rgRestrictions, IID_IRowset, 0, NULL, (IUnknown**)
		&pIRowset), S_OK)

	TESTC(pIRowset != NULL)

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using only value.
	//Bind only the 2nd column.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 1, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)

	while(S_OK==(hr=GetNextRows(pIRowset, NULL, 0, 1, &cRowsObtained, &rghRows)))
	{
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		TESTC_(hr = pIRowset->GetData(*rghRows, hAccessor, 
			pData),S_OK)
		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);

		//If the restriction was supported, make sure it was followed.
		if(ulRestrictPTY & 0x1)
			TESTC(*(USHORT*)((BYTE *)pData+rgBindings[0].obValue) 
				== DBTYPE_BYTES)

		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings,
			pData), S_OK);
	}

	TESTC_(hr, DB_S_ENDOFROWSET)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	VariantClear(&rgRestrictions[0]);
	VariantClear(&rgRestrictions[1]);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSR);
	return tTestResult;
} //testProvTypesSchemaRowset


//----------------------------------------------------------------------
//@mfunc Add a column using ITableDefinition and verify that it was
//added.
//
TESTRESULT CQuickTest::testAddColumn()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = S_OK;
	BOOL					fNewColumn=FALSE;
	DBORDINAL				cOldCol = 0;
	DBORDINAL				cNewCol = 0;
	DBID*					pColumnID = NULL;
	DBID*					pTableID = NULL;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBCOLUMNDESC*			rgColDesc1	= NULL;
	DBCOLUMNDESC*			rgColDesc2	= NULL;
	DBCOLUMNDESC*			pColumnDesc = NULL;
	DBORDINAL				cColumnDesc = 0;
	DBORDINAL				cColDesc1 = 0;
	DBORDINAL				cColDesc2 = 0;
	CTable*					pTable = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ITableDefinition*		pITableDefinition = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinition,
		SESSION_INTERFACE,(IUnknown**)&pITableDefinition))
	{
		odtLog<<L"ITableDefinition is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	//Create column information, initializes column list with columns 
	//of specified types. 
	TESTC_(hr=pTable->CreateColInfo(ListNativeTemp, ListDataTypes, 
		ALLTYPES), S_OK)

	//build the column description array
	hr=pTable->BuildColumnDescs(&rgColumnDesc);
	TESTC(SUCCEEDED(hr))

	cColumnDesc = pTable->CountColumnsOnTable();

	//Set cColumnDesc-1 column descriptions.
	pTable->SetColumnDesc(rgColumnDesc, cColumnDesc-1);
	pTable->SetBuildColumnDesc(FALSE);

	//Create table with one less column.
	TESTC_(pTable->CreateTable(0, 0), S_OK)

	pTableID = &(pTable->GetTableIDRef()) ;

	hr=pTable->GetTableColumnInfo(pTableID);
	TESTC(SUCCEEDED(hr))

	//Get column descs before calling AddColumn.
	TESTC(pTable->ColList2ColumnDesc(&rgColDesc1, &cColDesc1))

	//The column to be added.
	pColumnDesc = &rgColumnDesc[cColumnDesc-1];

	//Add the new column.
	TESTC_(hr = pITableDefinition->AddColumn(pTableID, 
		pColumnDesc, &pColumnID), S_OK)
	TESTC(pColumnID != NULL)

	hr=pTable->GetTableColumnInfo(pTableID);
	TESTC(SUCCEEDED(hr))

	//Get column descs after AddColumn.
	TESTC(pTable->ColList2ColumnDesc(&rgColDesc2, &cColDesc2))

	//Number of columns should have increased by 1.
	TESTC(cColDesc2 == cColDesc1+1);

	//LOOP cColDesc1+1 times.
	for (cOldCol=cNewCol=0; cNewCol<cColDesc2; cNewCol++)
	{
		//Its an old column.
		//Added an extra condition cOldCol < cColDesc1 because that would prevent reading
		//past rgColDesc1 max index. 

		if (cOldCol < cColDesc1 && CompareDBID(rgColDesc1[cOldCol].dbcid, 
			rgColDesc2[cNewCol].dbcid))
		{
			TESTC(CompareColumnDesc(&rgColDesc1[cOldCol], 
				&rgColDesc2[cNewCol]));
			cOldCol++;
		}
		//It should be the new column which was added.
		else
		{
			//Make sure the new column has not been found already.
			TESTC(!fNewColumn);
			//Check if this new column is the one we had added.
			TESTC(pColumnDesc->pTypeInfo == rgColDesc2[cOldCol].pTypeInfo);
			TESTC(pColumnDesc->wType == rgColDesc2[cOldCol].wType);
			TESTC(CompareDBID(*pColumnID, rgColDesc2[cOldCol].dbcid));
			//The new column has been found.
			fNewColumn = TRUE;
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	if(pTable)
		pTable->DropTable();

	//rgColumnDesc gets freed above when pTable is deleted.
	pTable->SetNoOfColumnDesc(cColumnDesc);
	SAFE_DELETE(pTable);
	ReleaseColumnDesc(rgColDesc1, cColDesc1);
	ReleaseColumnDesc(rgColDesc2, cColDesc2);
	ReleaseDBID(pColumnID, TRUE);
	SAFE_RELEASE(pITableDefinition);
	return tTestResult;
} //testAddColumn


//----------------------------------------------------------------------
//@mfunc Create a table using ITableDefinition, and verify.
//
TESTRESULT CQuickTest::testCreateTable()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = S_OK;
	BOOL					fExists = FALSE;
	DBID*					pNewTableID = NULL;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBORDINAL				cColumnDesc = 0;
	CTable*					pTable = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	IRowsetInfo*			pIRowsetInfo = NULL;
	IOpenRowset*			pIOpenRowset = NULL;
	ITableDefinition*		pITableDefinition = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinition,
		SESSION_INTERFACE,(IUnknown**)&pITableDefinition))
	{
		odtLog<<L"ITableDefinition is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	//Create column information, initializes column list with columns 
	//of specified types. Can get DB_S_ENDOFROWSET.
	TESTC_(hr=pTable->CreateColInfo(ListNativeTemp, ListDataTypes, 
	ALLTYPES), S_OK)

	//build the column description array
	TESTC_(hr=pTable->BuildColumnDescs(&rgColumnDesc), S_OK)

	cColumnDesc = pTable->CountColumnsOnTable();

	//Setting the following property is not required.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &m_cPropSets, 
		&m_rgPropSets) ;

	//Create table with the column descriptions obtained from pTable.
	TESTC_(hr = pITableDefinition->CreateTable(NULL, NULL, cColumnDesc, 
		rgColumnDesc, IID_IRowsetInfo, m_cPropSets, m_rgPropSets, 
		&pNewTableID, (IUnknown**)&pIRowsetInfo), S_OK)

	//Make sure the table exists.
	CHECK(hr = pTable->DoesTableExist(pNewTableID, &fExists), S_OK) ;
	TESTC(fExists)

	TESTC(pIRowsetInfo != NULL)

	//Make sure that the IRowsetInfo obtained from CreateTable is
	//actually usable.
	hr = pIRowsetInfo->GetSpecification(IID_IOpenRowset, (IUnknown**)
		&pIOpenRowset) ;

	if(hr==S_FALSE)
	{
		odtLog<<L"IRowsetInfo::GetSpecification returned S_FALSE.\n";
		CHECKW(hr, S_OK);
	}
	else
	{
		TESTC_(hr, S_OK)
		TESTC(VerifyEqualInterface(pIOpenRowset, m_pIOpenRowset))
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIOpenRowset);
	if(pNewTableID)
	{
		hr = pITableDefinition->DropTable(pNewTableID);
		ReleaseDBID(pNewTableID, TRUE);
		if(hr != S_OK)
		{
			CHECK(hr, S_OK);
			tTestResult = TEST_FAIL;
		}
	}
	SAFE_DELETE(pTable);
	SAFE_RELEASE(pITableDefinition);
	return tTestResult;
} //testCreateTable


//----------------------------------------------------------------------
//@mfunc Drop a column from the table using ITableDefinition, and verify
//that it got removed.
//
TESTRESULT CQuickTest::testDropColumn()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBORDINAL		ulIndex = 0;
	BOOL			bFound = FALSE;
	DBID*			pTableID = NULL ;
	WCHAR*			pwszNewName = NULL;
	DBCOLUMNDESC	*rgColumnDesc = NULL;
	DBCOLUMNDESC	*rgColDesc1	= NULL;
	DBORDINAL		cColumnDesc = 0;
	DBORDINAL		cColDesc1 = 0;
	CTable*			pTable = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ITableDefinition*	pITableDefinition = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinition,
		SESSION_INTERFACE,(IUnknown**)&pITableDefinition))
	{
		odtLog<<L"ITableDefinition is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	//Create column information, initializes column list with columns 
	//of specified types. Can get DB_S_ENDOFROWSET.
	TESTC_(hr=pTable->CreateColInfo(ListNativeTemp, ListDataTypes, 
		ALLTYPES), S_OK)

	//build the column description array
	hr=pTable->BuildColumnDescs(&rgColumnDesc);
	TESTC(SUCCEEDED(hr))

	cColumnDesc = pTable->CountColumnsOnTable();

	//Create a table with the modified column description.
	pTable->SetBuildColumnDesc(FALSE);
	pTable->SetColumnDesc(rgColumnDesc, cColumnDesc);
	TESTC_(hr = pTable->CreateTable(0, 0), S_OK);

	pTableID = &(pTable->GetTableIDRef()) ;

	//Drop the column whose DBID we had modified earlier.
	hr = pITableDefinition->DropColumn(pTableID, 
		&(rgColumnDesc[0].dbcid)) ;

	if(hr==E_NOTIMPL)
	{
		odtLog<<L"WARNING/FAIL: The method returned E_NOTIMPL. This is OK only if you are running against SQLServer6.5. Against anything else, this would be a FAILURE.\n";
		CHECKW(hr, S_OK);
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC_(hr, S_OK)

	//Obtain the column info and descriptions after dropping a 
	//column.
	hr=pTable->GetTableColumnInfo(pTableID);
	TESTC(SUCCEEDED(hr))

	TESTC(pTable->ColList2ColumnDesc(&rgColDesc1, &cColDesc1))

	//Make sure the column which was dropped does not exist in the
	//new column desc list.
	for(ulIndex=0; (ulIndex<cColDesc1 && bFound == FALSE); ulIndex++)
	{
		if(CompareDBID(rgColumnDesc[0].dbcid, rgColDesc1[ulIndex].dbcid))
			bFound = TRUE;
	}

	TESTC(!bFound)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColDesc1, cColDesc1);
	if(pTable)
		pTable->DropTable();
	SAFE_DELETE(pTable);
	//rgColumnDesc gets freed above when pTable is deleted.
	SAFE_RELEASE(pITableDefinition);
	return tTestResult;
} //testDropColumn


//----------------------------------------------------------------------
//@mfunc Drop a table using ITableDefinition and verify.
//
TESTRESULT CQuickTest::testDropTable()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	BOOL				fExists = TRUE;
	CTable*				pTable = NULL;
	ITableDefinition*	pITableDefinition = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinition,
		SESSION_INTERFACE,(IUnknown**)&pITableDefinition))
	{
		odtLog<<L"ITableDefinition is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	pTable->SetBuildColumnDesc(TRUE);

	//Create a table.
	TESTC_(hr = pTable->CreateTable((DBCOUNTITEM)0, (DBORDINAL)0, NULL), S_OK);

	TESTC_(hr = pITableDefinition->DropTable(&pTable->GetTableID()), 
		S_OK);

	//Check if the table still exists.
	CHECK(hr = pTable->DoesTableExist(&pTable->GetTableID(), &fExists),
		S_OK) ;

	TESTC(!fExists)

	tTestResult = TEST_PASS;

CLEANUP:
	if(pTable)
		pTable->DropTable();
	SAFE_DELETE(pTable);
	SAFE_RELEASE(pITableDefinition);
	return tTestResult;
} //testDropTable


//----------------------------------------------------------------------
//@mfunc Create an Index and verify that it exists.
//
TESTRESULT CQuickTest::testIIndexDef()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	TESTRESULT				tTR = TEST_FAIL;
	HRESULT					hr = S_OK;
	BOOL					bIndexExists = FALSE;
	DBORDINAL				cColumnDesc = 0;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBINDEXCOLUMNDESC		rgIndexColumnDesc[1];
	DBINDEXCOLUMNDESC*		rgIndexColumnDescOut = NULL;
	DBID*					pTableID = NULL;
	DBID*					pIndexID = NULL;
	CTable*					pTable = NULL;
	IIndexDefinition*		pIID = NULL;
	IDBSchemaRowset*		pIDSR = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_IIndexDefinition,
		SESSION_INTERFACE,(IUnknown**)&pIID))
	{
		odtLog<<L"IIndexDefinition is not supported.\n";
		return TEST_SKIPPED;
	}

	//Initialize this local variable to avoid releasing garbage pointer.
	rgIndexColumnDesc[0].pColumnID = NULL;

	pTable	= new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName, NONULLS);
	TESTC(pTable != NULL)

	//Create a table with no index.
	TESTC(SUCCEEDED(hr = pTable->CreateTable(MIN_ROWS, 0)))

	//Get the column descs.
	TESTC_(pTable->BuildColumnDescs(&rgColumnDesc), S_OK)
	cColumnDesc = pTable->CountColumnsOnTable();
	pTableID = &(pTable->GetTableIDRef());

	//Create the index column desc.
	SAFE_ALLOC(rgIndexColumnDesc[0].pColumnID, DBID, 1);
	DuplicateDBID(rgColumnDesc[0].dbcid, rgIndexColumnDesc[0].pColumnID);
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;

	//Create the index.
	TESTC_(hr = pIID->CreateIndex(pTableID, NULL, 1, rgIndexColumnDesc, 
		0, NULL, &pIndexID), S_OK)

	TESTC(pIndexID != NULL)

	if(!VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDSR))
	{
		odtLog<<L"INFO: IDBSchemaRowset is not supported.\n";
		pIDSR = NULL;
	}

	//check if the index exists.
	if(pIDSR)
	{
		tTR = DoesIndexExist(pIDSR, pTableID, pIndexID, &bIndexExists);
		if(tTR==TEST_PASS)
			TESTC(bIndexExists)
		else if(tTR==TEST_FAIL)
			COMPARE(tTR, TEST_PASS);	
	}

	//Try to create a duplicate index to verify.
	TESTC_(hr = pIID->CreateIndex(pTableID, pIndexID, 1, rgIndexColumnDesc, 
		0, NULL, NULL), DB_E_DUPLICATEINDEXID)

	//Drop the index.
	TESTC_(hr = pIID->DropIndex(pTableID, pIndexID), S_OK)

	//check if the index has been dropped.
	if(pIDSR)
	{
		tTR = DoesIndexExist(pIDSR, pTableID, pIndexID, &bIndexExists);
		if(tTR==TEST_PASS)
			TESTC(!bIndexExists)
		else if(tTR==TEST_FAIL)
			COMPARE(tTR, TEST_PASS);	
	}

	//Try to drop the index again, to verify.
	TESTC_(hr = pIID->DropIndex(pTableID, pIndexID), DB_E_NOINDEX)

	tTestResult = TEST_PASS;

CLEANUP:
	if(pTable)
		pTable->DropTable();
	SAFE_DELETE(pTable);
	ReleaseDBID(pIndexID, TRUE);
	ReleaseDBID(rgIndexColumnDesc[0].pColumnID, TRUE);
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	PROVIDER_FREE(rgIndexColumnDescOut);
	SAFE_RELEASE(pIDSR);
	SAFE_RELEASE(pIID);
	return tTestResult;
} //testIIndexDef


//----------------------------------------------------------------------
//@mfunc Test IAlterIndex::AlterIndex.
//
TESTRESULT CQuickTest::testAlterIndex()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	TESTRESULT				tTR = TEST_FAIL;
	HRESULT					hr = E_FAIL;
	HRESULT					hr1 = E_FAIL;
	BOOL					bIndexExists = FALSE;
	DBORDINAL				cColumnDesc = 0;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBINDEXCOLUMNDESC		rgIndexColumnDesc[1];
	DBID*					pTableID = NULL;
	DBID*					pIndexID = NULL;
	DBID					dbidNewIndex;
	IAlterIndex*			pIAI = NULL;
	IIndexDefinition*		pIID = NULL;
	IDBSchemaRowset*		pIDSR = NULL;
	IRowsetIndex*			pIRI = NULL;

	DBORDINAL				cKeyColumns = 0;
	DBINDEXCOLUMNDESC *		rgIndexColumnDesc2 = NULL;
	ULONG					cIndexProperties = 0;
	DBPROPSET *				rgIndexProperties = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_IAlterIndex,
		SESSION_INTERFACE,(IUnknown**)&pIAI))
	{
		odtLog<<L"IAlterIndex is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC(VerifyInterface(m_pIOpenRowset,IID_IIndexDefinition,
		SESSION_INTERFACE,(IUnknown**)&pIID))

	//Get the column descs.
	TESTC_(m_pTable->BuildColumnDescs(&rgColumnDesc), S_OK)
	cColumnDesc = m_pTable->CountColumnsOnTable();
	pTableID = &(m_pTable->GetTableIDRef());

	//Create the index column desc.
	SAFE_ALLOC(rgIndexColumnDesc[0].pColumnID, DBID, 1);
	DuplicateDBID(rgColumnDesc[0].dbcid, rgIndexColumnDesc[0].pColumnID);
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;

	//See if IDBSchemaRowset is supported.
	VerifyInterface(m_pIOpenRowset,IID_IDBSchemaRowset,
		SESSION_INTERFACE,(IUnknown**)&pIDSR);

	//Create the index.
	TESTC_(hr = pIID->CreateIndex(pTableID, NULL, 1, rgIndexColumnDesc, 
		0, NULL, &pIndexID), S_OK)
	TESTC(pIndexID != NULL)

	//Duplicate index id.
	DuplicateDBID(*pIndexID, &dbidNewIndex);

	//check if the index exists.
	if(pIDSR)
	{
		tTR = DoesIndexExist(pIDSR, pTableID, pIndexID, &bIndexExists);
		if(tTR==TEST_PASS)
			TESTC(bIndexExists)
		else if(tTR==TEST_FAIL)
			COMPARE(tTR, TEST_PASS);	
	}

	//Modify the index name.
	if(dbidNewIndex.eKind == DBKIND_GUID_NAME ||
		dbidNewIndex.eKind == DBKIND_NAME ||
		dbidNewIndex.eKind == DBKIND_PGUID_NAME)
	{
		TESTC(_wcsset(dbidNewIndex.uName.pwszName, 'x') != NULL)
	}
	else
	{
		dbidNewIndex.uName.ulPropid++;
	}

	//Set some props.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, 
		&m_rgPropSets, (void*)66, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL) ;

	TEST2C_(hr1 = pIAI->AlterIndex(pTableID, pIndexID, &dbidNewIndex, 
		m_cPropSets, m_rgPropSets), S_OK, DB_S_ERRORSOCCURRED)

	//check if the new index exists and old one doesn't exist.
	if(pIDSR)
	{
		tTR = DoesIndexExist(pIDSR, pTableID, &dbidNewIndex, &bIndexExists);
		if(tTR==TEST_PASS)
			TESTC(bIndexExists)

		tTR = DoesIndexExist(pIDSR, pTableID, pIndexID, &bIndexExists);
		if(tTR==TEST_PASS)
			TESTC(!bIndexExists)
	}

	//Verify using IRowsetIndex, if it is supported.
	TEST3C_(hr=m_pIOpenRowset->OpenRowset(NULL, pTableID, &dbidNewIndex,
		IID_IRowsetIndex, 0, NULL, (IUnknown**)&pIRI), S_OK, E_NOINTERFACE, 
		DB_E_NOINDEX)

	if(hr == S_OK)
	{
		TESTC_(pIRI->GetIndexInfo(&cKeyColumns, &rgIndexColumnDesc2, 
			&cIndexProperties, &rgIndexProperties), S_OK)

		COMPARE(cKeyColumns, 1);
		COMPARE(rgIndexColumnDesc2[0].eIndexColOrder, rgIndexColumnDesc[0].eIndexColOrder);
		COMPARE(CompareDBID(*rgIndexColumnDesc2[0].pColumnID, *rgIndexColumnDesc[0].pColumnID), TRUE);

		if(hr1 == S_OK)
		{
			for(ULONG i=0; i<cIndexProperties; i++)
			{
				if(rgIndexProperties[i].guidPropertySet == DBPROPSET_INDEX)
					for(ULONG j=0; j<rgIndexProperties[i].cProperties; j++)
					{
						if(rgIndexProperties[i].rgProperties[j].dwPropertyID == DBPROP_INDEX_FILLFACTOR)
							COMPARE(V_I4(&rgIndexProperties[i].rgProperties[j].vValue), 66);
					}
			}
		}
	}

	/*****
	TEST2C_(hr = pIAI->AlterIndex(pTableID, &dbidNewIndex, pIndexID, 
		0, NULL), S_OK, DB_E_TABLEINUSE)

	if(hr == DB_E_TABLEINUSE)
	{
		SAFE_RELEASE(pIRI);
		TESTC_(hr = pIAI->AlterIndex(pTableID, &dbidNewIndex, pIndexID, 
			0, NULL), S_OK)
	}
	*****/

	SAFE_RELEASE(pIRI);
	TESTC_(hr = pIAI->AlterIndex(pTableID, &dbidNewIndex, pIndexID, 
		0, NULL), S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(pIndexID, TRUE);
	ReleaseDBID(&dbidNewIndex, FALSE);
	ReleaseDBID(rgIndexColumnDesc[0].pColumnID, TRUE);
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	FreeProperties(&cIndexProperties, &rgIndexProperties);
	SAFE_FREE(rgIndexColumnDesc2);
	SAFE_RELEASE(pIRI);
	SAFE_RELEASE(pIID);
	SAFE_RELEASE(pIDSR);
	SAFE_RELEASE(pIAI);
	return tTestResult;
} //testAlterIndex


//----------------------------------------------------------------------
//@mfunc Test IAlterTable::AlterColumn.
//
TESTRESULT CQuickTest::testAlterColumn()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = E_FAIL;
	HRESULT					hr1 = E_FAIL;
	DBID*					pTableID = NULL;
	DBID					dbidOld;
	ULONG_PTR				ulColFlags = 0;
	DBORDINAL				ulCol = 0;
	DBORDINAL				cColumns = 0;
	WCHAR*					pStringsBuffer = NULL;
    DBCOLUMNINFO*			rgInfo = NULL;
	DBCOLUMNDESCFLAGS		dwColFlags = 0;
	DBCOLUMNDESC			colDesc;
	IColumnsInfo*			pICI = NULL;
	IAlterTable*			pIAT = NULL;

	memset(&colDesc, 0, sizeof(DBCOLUMNDESC));

	if(!VerifyInterface(m_pIOpenRowset,IID_IAlterTable,
		SESSION_INTERFACE,(IUnknown**)&pIAT))
	{
		odtLog<<L"IAlterTable is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC(GetProperty(DBPROP_ALTERCOLUMN, DBPROPSET_DATASOURCEINFO, 
		m_pIDBInitialize, &ulColFlags))

	pTableID = &(m_pTable->GetTableIDRef());

	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pTableID, NULL, IID_IColumnsInfo, 
		0, NULL, (IUnknown**)&pICI), S_OK)

	//Call GetColumnInfo.
    TESTC_(pICI->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK)
    TESTC(cColumns!=0 && rgInfo!=NULL && pStringsBuffer!=NULL)

	if(cColumns>1)
		ulCol = 1;

	if(ulColFlags & DBCOLUMNDESCFLAGS_PROPERTIES)
		dwColFlags |= DBCOLUMNDESCFLAGS_PROPERTIES;
	if(ulColFlags & DBCOLUMNDESCFLAGS_DBCID)
		dwColFlags |= DBCOLUMNDESCFLAGS_DBCID;

	//Duplicate column id.
	DuplicateDBID(rgInfo[ulCol].columnid, &colDesc.dbcid);
	DuplicateDBID(rgInfo[ulCol].columnid, &dbidOld);

	//Modify the column id.
	if(rgInfo[ulCol].columnid.eKind == DBKIND_GUID_NAME ||
		rgInfo[ulCol].columnid.eKind == DBKIND_NAME ||
		rgInfo[ulCol].columnid.eKind == DBKIND_PGUID_NAME)
	{
		TESTC(_wcsset(colDesc.dbcid.uName.pwszName, 'x') != NULL)
	}
	else
	{
		colDesc.dbcid.uName.ulPropid++;
	}

	//Set a column property.
	SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, &colDesc.cPropertySets, 
		&colDesc.rgPropertySets, (void*)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL) ;

	//Release rowset so that the table is not in use.
	SAFE_RELEASE(pICI);

	TEST2C_(hr1 = pIAT->AlterColumn(pTableID, &rgInfo[ulCol].columnid, dwColFlags,
		&colDesc), S_OK, DB_S_ERRORSOCCURRED)

	TEST2C_(hr = pIAT->AlterColumn(pTableID, &rgInfo[ulCol].columnid, dwColFlags,
		&colDesc), DB_E_NOCOLUMN, DB_E_DUPLICATECOLUMNID)

	//Release old column info.
	SAFE_FREE(rgInfo);
	SAFE_FREE(pStringsBuffer);

	//Call GetColumnInfo.
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pTableID, NULL, IID_IColumnsInfo, 
		0, NULL, (IUnknown**)&pICI), S_OK)
    TESTC_(pICI->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK)
    TESTC(cColumns!=0 && rgInfo!=NULL && pStringsBuffer!=NULL)

	COMPARE(CompareDBID(rgInfo[ulCol].columnid, colDesc.dbcid), TRUE);
	if(hr1 == S_OK)
		COMPARE(rgInfo[ulCol].dwFlags & DBCOLUMNFLAGS_ISNULLABLE, 0);

	//Remove props from the col desc, and set to old col ID.
	FreeProperties(&colDesc.cPropertySets, &colDesc.rgPropertySets);
	colDesc.cPropertySets = 0;
	ReleaseDBID(&colDesc.dbcid, FALSE);
	DuplicateDBID(dbidOld, &colDesc.dbcid);

	SAFE_RELEASE(pICI);

	TESTC_(hr = pIAT->AlterColumn(pTableID, &rgInfo[ulCol].columnid, dwColFlags,
		&colDesc), S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&dbidOld, FALSE);
	ReleaseDBID(&colDesc.dbcid, FALSE);
	FreeProperties(&colDesc.cPropertySets, &colDesc.rgPropertySets);
	SAFE_FREE(rgInfo);
	SAFE_FREE(pStringsBuffer);
	SAFE_RELEASE(pICI);
	SAFE_RELEASE(pIAT);
	return tTestResult;
} //testAlterColumn


//----------------------------------------------------------------------
//@mfunc Test IAlterTable::AlterTable.
//
TESTRESULT CQuickTest::testAlterTable()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = E_FAIL;
	HRESULT					hr1 = E_FAIL;
	DBID*					pTableID = NULL;
	DBID					dbidNew;
	IAlterTable*			pIAT = NULL;
	IRowset*				pIR = NULL;

	if(!VerifyInterface(m_pIOpenRowset,IID_IAlterTable,
		SESSION_INTERFACE,(IUnknown**)&pIAT))
	{
		odtLog<<L"IAlterTable is not supported.\n";
		return TEST_SKIPPED;
	}

	pTableID = &(m_pTable->GetTableIDRef());

	//Duplicate index id.
	DuplicateDBID(*pTableID, &dbidNew);

	//Modify the index name.
	if(dbidNew.eKind == DBKIND_GUID_NAME ||
		dbidNew.eKind == DBKIND_NAME ||
		dbidNew.eKind == DBKIND_PGUID_NAME)
	{
		TESTC(_wcsset(dbidNew.uName.pwszName, 'x') != NULL)
	}
	else
	{
		dbidNew.uName.ulPropid++;
	}

	//Set some props.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, &m_cPropSets, 
		&m_rgPropSets, (void*)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL) ;

	TEST3C_(hr1 = pIAT->AlterTable(pTableID, &dbidNew, 
		m_cPropSets, m_rgPropSets), S_OK, DB_S_ERRORSOCCURRED, 
		DB_E_DUPLICATETABLEID)

	//If table with that name exists, try once more with another name.
	if(hr1 == DB_E_DUPLICATETABLEID)
	{
		if(dbidNew.eKind == DBKIND_GUID_NAME ||
			dbidNew.eKind == DBKIND_NAME ||
			dbidNew.eKind == DBKIND_PGUID_NAME)
		{
			odtLog<<L"INFO: A table with name "<<dbidNew.uName.pwszName<<L" already exists.\n";
			TESTC(_wcsset(dbidNew.uName.pwszName, 'z') != NULL)
		}
		else
		{
			odtLog<<L"INFO: A table with propid "<<dbidNew.uName.ulPropid<<L" already exists.\n";
			dbidNew.uName.ulPropid++;
		}

		TEST3C_(hr1 = pIAT->AlterTable(pTableID, &dbidNew, 
			m_cPropSets, m_rgPropSets), S_OK, DB_S_ERRORSOCCURRED, 
			DB_E_DUPLICATETABLEID)

		if(hr1 == DB_E_DUPLICATETABLEID)
		{
			if(dbidNew.eKind == DBKIND_GUID_NAME ||
				dbidNew.eKind == DBKIND_NAME ||
				dbidNew.eKind == DBKIND_PGUID_NAME)
				odtLog<<L"INFO: A table with name "<<dbidNew.uName.pwszName<<L" already exists.\n";
			else
				odtLog<<L"INFO: A table with propid "<<dbidNew.uName.ulPropid<<L" already exists.\n";

			tTestResult = TEST_PASS;
			goto CLEANUP;
		}
	}

	//check if the new table exists and old one doesn't exist.

	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pTableID, NULL,
		IID_IRowset, 0, NULL, (IUnknown**)&pIR), DB_E_NOTABLE)
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, &dbidNew, NULL,
		IID_IRowset, 0, NULL, (IUnknown**)&pIR), S_OK)

	/*****
	//Restore old table name.
	TEST2C_(hr1 = pIAT->AlterTable(&dbidNew, pTableID, 
		0, NULL), S_OK, DB_E_TABLEINUSE)

	if(hr == DB_E_TABLEINUSE)
	{
		SAFE_RELEASE(pIR);
		TESTC_(hr1 = pIAT->AlterTable(&dbidNew, pTableID, 
			0, NULL), S_OK)
	}
	*****/

	SAFE_RELEASE(pIR);
	TESTC_(hr1 = pIAT->AlterTable(&dbidNew, pTableID, 
		0, NULL), S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&dbidNew, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(pIR);
	SAFE_RELEASE(pIAT);
	return tTestResult;
} //testAlterTable


//----------------------------------------------------------------------
//@mfunc Test the ITableDefinitionWithConstraints interface.
//
TESTRESULT CQuickTest::testAddAndDropConstraint()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = E_FAIL;
	HRESULT					hr1 = E_FAIL;
	DBID*					pTableID = NULL;
	DBID					dbidCons;
	DBCONSTRAINTDESC		dbConDesc;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBORDINAL				cColumnDesc = 0;
	CTable*					pTable = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ITableDefinitionWithConstraints*	pITDWC = NULL;

	memset(&dbConDesc, 0, sizeof(DBCONSTRAINTDESC));

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinitionWithConstraints,
		SESSION_INTERFACE,(IUnknown**)&pITDWC))
	{
		odtLog<<L"ITableDefinitionWithConstraints is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	//Create column information, initializes column list with columns 
	//of specified types. Can get DB_S_ENDOFROWSET.
	TESTC_(hr=pTable->CreateColInfo(ListNativeTemp, ListDataTypes, 
	ALLTYPES), S_OK)

	//build the column description array
	TESTC_(hr=pTable->BuildColumnDescs(&rgColumnDesc), S_OK)

	cColumnDesc = pTable->CountColumnsOnTable();

	//Create table with the column descriptions obtained from pTable.
	TESTC_(hr = pITDWC->CreateTable(NULL, NULL, cColumnDesc, 
		rgColumnDesc, IID_IRowsetInfo, 0, NULL, 
		&pTableID, NULL), S_OK)

	TESTC(pTableID != NULL)

	//Build the DBCONSTRAINTDESC structure.
	dbidCons.eKind = DBKIND_NAME;
	dbidCons.uName.pwszName = L"QuikTest_CN1";
	dbConDesc.pConstraintID = &dbidCons;
	dbConDesc.ConstraintType = DBCONSTRAINTTYPE_UNIQUE;
	dbConDesc.cColumns = 1;
	dbConDesc.rgColumnList = &(rgColumnDesc[0].dbcid);

	TESTC_(hr1 = pITDWC->AddConstraint(pTableID, &dbConDesc), S_OK)

	if(cColumnDesc>1)
	{
		dbConDesc.rgColumnList = &(rgColumnDesc[1].dbcid);
		CHECK(hr1 = pITDWC->AddConstraint(pTableID, &dbConDesc), DB_E_DUPLICATECONSTRAINTID);
	}

	TESTC_(hr1 = pITDWC->DropConstraint(pTableID, &dbidCons), S_OK)
	TESTC_(hr1 = pITDWC->DropConstraint(pTableID, &dbidCons), DB_E_NOCONSTRAINT)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	if(pTableID)
	{
		CHECK(hr=pITDWC->DropTable(pTableID), S_OK);
		ReleaseDBID(pTableID, TRUE);
	}
	SAFE_RELEASE(pITDWC);
	if(pTable)
		pTable->DropTable();
	SAFE_DELETE(pTable);
	return tTestResult;
} //testAddAndDropConstraint


//----------------------------------------------------------------------
//@mfunc Test the ITableDefinitionWithConstraints interface.
//
TESTRESULT CQuickTest::testCreateTableWithConstraints()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = E_FAIL;
	DBID					dbidCons;
	DBID					dbidCons2;
	DBCONSTRAINTDESC		rgdbConDesc[2];
	BOOL					fExists = FALSE;
	DBID*					pNewTableID = NULL;
	DBCOLUMNDESC*			rgColumnDesc = NULL;
	DBORDINAL				cColumnDesc = 0;
	CTable*					pTable = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ITableDefinitionWithConstraints*	pITDWC = NULL;

	memset(rgdbConDesc, 0, 2*sizeof(DBCONSTRAINTDESC));

	if(!VerifyInterface(m_pIOpenRowset,IID_ITableDefinitionWithConstraints,
		SESSION_INTERFACE,(IUnknown**)&pITDWC))
	{
		odtLog<<L"ITableDefinitionWithConstraints is not supported.\n";
		return TEST_SKIPPED;
	}

	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
	TESTC(pTable != NULL)

	//Create column information, initializes column list with columns 
	//of specified types. Can get DB_S_ENDOFROWSET.
	TESTC_(hr=pTable->CreateColInfo(ListNativeTemp, ListDataTypes, 
	ALLTYPES), S_OK)

	//build the column description array
	TESTC_(hr=pTable->BuildColumnDescs(&rgColumnDesc), S_OK)

	cColumnDesc = pTable->CountColumnsOnTable();

	//Setting the following property is not required.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &m_cPropSets, 
		&m_rgPropSets) ;

	//Build the DBCONSTRAINTDESC structure.
	dbidCons.eKind = DBKIND_NAME;
	dbidCons.uName.pwszName = L"QuikTest_CN1";
	rgdbConDesc[0].pConstraintID = &dbidCons;
	rgdbConDesc[0].ConstraintType = DBCONSTRAINTTYPE_UNIQUE;
	rgdbConDesc[0].cColumns = 1;
	rgdbConDesc[0].rgColumnList = &(rgColumnDesc[0].dbcid);

	dbidCons2.eKind = DBKIND_NAME;
	dbidCons2.uName.pwszName = L"QuikTest_CN2";
	rgdbConDesc[1].pConstraintID = &dbidCons2;
	rgdbConDesc[1].ConstraintType = DBCONSTRAINTTYPE_UNIQUE;
	rgdbConDesc[1].cColumns = 1;
	rgdbConDesc[1].rgColumnList = &(rgColumnDesc[1].dbcid);

	//Create table with the column descriptions obtained from pTable.
	TESTC_(hr = pITDWC->CreateTableWithConstraints(NULL, NULL, 
		cColumnDesc, rgColumnDesc, 2, rgdbConDesc, IID_NULL, 0, NULL, 
		&pNewTableID, NULL), S_OK)

	//Make sure the table exists.
	CHECK(hr = pTable->DoesTableExist(pNewTableID, &fExists), S_OK) ;
	TESTC(fExists)

	TESTC_(hr = pITDWC->DropConstraint(pNewTableID, &dbidCons), S_OK)
	TESTC_(hr = pITDWC->DropConstraint(pNewTableID, &dbidCons2), S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	if(pNewTableID)
	{
		CHECK(pITDWC->DropTable(pNewTableID), S_OK);
		ReleaseDBID(pNewTableID, TRUE);
	}
	SAFE_DELETE(pTable);
	SAFE_RELEASE(pITDWC);
	return tTestResult;
} //testCreateTableWithConstraints


//----------------------------------------------------------------------
//@mfunc Aggregate a session object and test it.
//
TESTRESULT CQuickTest::testAggregateSession(IDBCreateSession* pIDBCreateSession)
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	IUnknown*	pIUnkInner = NULL;
	
	CAggregate Aggregate(pIDBCreateSession);

	//Aggregation
	hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Check if aggregation is supported.
	if(hr==DB_E_NOAGGREGATION)
	{
		odtLog<<"Aggregation is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	//Verify Aggregation for this session...
	TESTC(Aggregate.VerifyAggregationQI(hr, IID_IOpenRowset))

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	return tTestResult;
} //testAggregateSession


//----------------------------------------------------------------------
//@mfunc Obtain the IConvertType interface from the IRowset interface or
//from ICommand interface. Use its CanConvert method to check for 
//certain conversions.
//
TESTRESULT CQuickTest::testIConvertType()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	HRESULT			Exphr = S_OK;
	BOOL			fRowsetCnvtOnCmd = FALSE;
	BOOL			bDBTIME = FALSE;
	BOOL			bNUMERIC = FALSE;
	BOOL			bSTR = FALSE;
	BOOL			bBYTES = FALSE;
	IConvertType*	pICT = NULL;

	//COMMAND object.
	if(m_pICommand)
	{
		if(!VerifyInterface(m_pICommand,IID_IConvertType, 
			COMMAND_INTERFACE,(IUnknown **)&pICT))
			return TEST_FAIL;

		// Check to see if conversion is supported on the command
		if(GetProperty(DBPROP_ROWSETCONVERSIONSONCOMMAND, 
			DBPROPSET_DATASOURCEINFO, m_pIDBInitialize))
			fRowsetCnvtOnCmd = TRUE;
	}
	//ROWSET object.
	else if(m_pIRowset)
	{
		if(!VerifyInterface(m_pIRowset,IID_IConvertType, 
			ROWSET_INTERFACE,(IUnknown **)&pICT))
			return TEST_FAIL;

		fRowsetCnvtOnCmd = TRUE;
	}
	//ROW object.
	else
	{
		TESTC(m_pIRow != NULL)

		if(!VerifyInterface(m_pIRow,IID_IConvertType, 
			ROW_INTERFACE,(IUnknown **)&pICT))
			return TEST_FAIL;

		fRowsetCnvtOnCmd = TRUE;
	}

	TESTC(m_pTable != NULL)

	GetProviderTypes(m_pTable);

	if(IsSupportedType(DBTYPE_DBTIME))
		bDBTIME = TRUE;
	if(IsSupportedType(DBTYPE_NUMERIC))
		bNUMERIC = TRUE;
	if(IsSupportedType(DBTYPE_STR))
		bSTR = TRUE;
	if(IsSupportedType(DBTYPE_BYTES))
		bBYTES = TRUE;

//S_FALSE: The following conversion should not be supported.
	if(fRowsetCnvtOnCmd)
		Exphr = S_FALSE;
	else
		Exphr = DB_E_BADCONVERTFLAG;

	if(bNUMERIC && bDBTIME)
		CHECK(hr=pICT->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME,
		DBCONVERTFLAGS_COLUMN), Exphr);

//S_OK: The following conversions should be supported.
	if(fRowsetCnvtOnCmd)
		Exphr = S_OK;
	else
		Exphr = DB_E_BADCONVERTFLAG;

	CHECK(hr=pICT->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
		DBCONVERTFLAGS_COLUMN),Exphr);

	if(bSTR)
		CHECK(hr=pICT->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
		DBCONVERTFLAGS_COLUMN),Exphr);

	if(bBYTES)
		CHECK(hr=pICT->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
		DBCONVERTFLAGS_COLUMN),Exphr);

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(m_rgProviderTypes);
	SAFE_RELEASE(pICT);
	return tTestResult;
} //testIConvertType


//----------------------------------------------------------------------
//@mfunc The IColumnsInfo interface passed in could belong to either a
//rowset or a command object. Get the column info by using the
//GetColumnInfo method. Verify the number of columns. Also verify the 
//values in the DBCOLUMNINFO structures. Obtain CCol structures by 
//calling CTable::GetCloInfo and use these to verify the column info 
//returned by the GetColumnInfo method call.
//
TESTRESULT CQuickTest::testGetColumnInfo(IUnknown* pIUnknown)
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	BOOL			fColIDWarning = FALSE;
	DBORDINAL		ulIndex = 0;
    DBORDINAL		cColumns = 0;
	CCol			rgCol;
	WCHAR*			pStringsBuffer = NULL;
    DBCOLUMNINFO*	rgInfo = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;

	TESTC(pIUnknown != NULL)

	//Get IColumnsInfo.
	TESTC_(hr = pIUnknown->QueryInterface(IID_IColumnsInfo, (void**)
		&pIColumnsInfo), S_OK)

    //Call GetColumnInfo.
    TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer)
		,S_OK)
    TESTC(cColumns!=0 && rgInfo!=NULL && pStringsBuffer!=NULL)

    //Verify the number of columns.
	if(rgInfo[0].iOrdinal==0)
		TESTC(cColumns-1 == m_pTable->CountColumnsOnTable())
	else
		TESTC(cColumns == m_pTable->CountColumnsOnTable())

	//Verify the DBCOLUMNINFO values.
	for(ulIndex=0; ulIndex<cColumns; ulIndex++)
	{
		//Ignore the bookmarks column if it exists.
		if((ulIndex==0)&&(rgInfo[ulIndex].iOrdinal==0))
		{
			// Check for the Bookmark FLAG.
			TESTC(rgInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)
			
			continue;
		}
	
		//Verify the ordinal value.
		TESTC(rgInfo[ulIndex].iOrdinal == ulIndex+rgInfo[0].iOrdinal)

		//Get the CCol structure for this column.
		TESTC_(hr=m_pTable->GetColInfo(rgInfo[ulIndex].iOrdinal, rgCol),
			S_OK)

		//Verify the column name.
		if((rgInfo[ulIndex].pwszName)&&(rgCol.GetColName()))
			TESTC(wcscmp(rgInfo[ulIndex].pwszName, 
				rgCol.GetColName()) == 0)
		else
			TESTC((rgInfo[ulIndex].pwszName == NULL)&&
				(rgCol.GetColName() == NULL))

		//ITypeInfo is reserved for future use.
		TESTC(rgInfo[ulIndex].pTypeInfo == rgCol.GetTypeInfo())

		//Verify the DBTYPE.
		TESTC(rgInfo[ulIndex].wType == rgCol.GetProviderType())

		//Compare the column IDs.
		if(!CompareDBID(rgInfo[ulIndex].columnid, *(rgCol.GetColID())))
		{
			TESTC(rgInfo[ulIndex].columnid.eKind !=
				(*(rgCol.GetColID())).eKind)
			fColIDWarning = TRUE;
		}
	}
	if(fColIDWarning)
	{
		//Issue a warning.
		COMPAREW(fColIDWarning, FALSE);
		odtLog<<L"WARNING: ColID returned is of different DBKIND.\n";
	}

	tTestResult = TEST_PASS;

CLEANUP:
    SAFE_RELEASE(pIColumnsInfo);
    PROVIDER_FREE(rgInfo);
    PROVIDER_FREE(pStringsBuffer);
	return tTestResult;
} //testGetColumnInfo


//----------------------------------------------------------------------
//@mfunc The IColumnsInfo interface passed in could belong to either a
//rowset or a command object. Get the column info and use 
//it to make an array of DBIDs. The method MapColumnIDs is called with 
//the constructed array of DBIDs. Verify the returned list of ordinals.
//
TESTRESULT CQuickTest::testMapColumnIDs(IUnknown* pIUnknown)
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBORDINAL		ulIndex = 0;
	DBORDINAL		cColumns = 0;
	DBCOLUMNINFO*	rgInfo = NULL;
	WCHAR*			pStringsBuffer = NULL;
	DBID*			rgColumnIDs = NULL;
	DBORDINAL*		rgColumns = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;

	TESTC(pIUnknown != NULL)

	//Get IColumnsInfo.
	TESTC_(hr = pIUnknown->QueryInterface(IID_IColumnsInfo, (void**)
		&pIColumnsInfo), S_OK)

	//Get the Column DBIDs
	TESTC_(hr=pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo,
		&pStringsBuffer),S_OK)

	//Make array of DBIDs
	TESTC((cColumns>0) && (rgInfo!=NULL))

	//Allocate memory for the array of DBID's
	SAFE_ALLOC(rgColumnIDs, DBID, sizeof(DBID) * cColumns)

	//Copy values into members
	for(ulIndex=0; ulIndex<cColumns; ulIndex++)
		memcpy(&(rgColumnIDs[ulIndex]),&(rgInfo[ulIndex].columnid),
		sizeof(DBID));

	//Allocate memory for the array of DBID's
	SAFE_ALLOC(rgColumns, DBORDINAL, sizeof(DBORDINAL) * cColumns)

	//Initialize the values to invalid values.
	for(ulIndex=0; ulIndex<cColumns; ulIndex++)
		rgColumns[ulIndex] = DB_INVALIDCOLUMN;

	//run testing interface, validate params
	TESTC_(hr=pIColumnsInfo->MapColumnIDs(cColumns, 
			rgColumnIDs, rgColumns),S_OK)

	//Loop thru the IDs to see if they are valid.
	for(ulIndex = 0; ulIndex<cColumns; ulIndex++)
	{
		TESTC(rgColumns[ulIndex] != DB_INVALIDCOLUMN)
		TESTC(rgColumns[ulIndex] < ULONG_MAX)
		TESTC(rgColumns[ulIndex] == rgInfo[ulIndex].iOrdinal)
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rgColumnIDs);
	PROVIDER_FREE(rgColumns);
	return tTestResult;
} //testMapColumnIDs


//----------------------------------------------------------------------
//@mfunc Obtain the IAccesssor interface through the IRowset interface.
//Create bindings and then create an accessor with this binding. The 
//length, status and value fields are bound. All columns are bound 
//including BLOB_LONG. Verify the status of the bindings after creating
//the accessor. Use this accessor to fetch all the rows of the table and
//verify their data. Then call the method GetBindings and verify that 
//they are the same as the ones used to create the accessor. After the 
//verifications, call the method ReleaseAccessor.
//
TESTRESULT CQuickTest::testIAccessor()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		ulIndex = 0;
	DBROWCOUNT		cRows = 0;
	DBORDINAL		cCols = 0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBBINDSTATUS*	rgStatus = NULL;
	DBCOLUMNINFO*	rgInfo = NULL;
	WCHAR*			pStringsBuffer = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;

	//Obtain the IAccessor interface.
	if(!VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))
		return TEST_FAIL;

	//Create Accessor with a binding using length, status and value.
	//All columns are bound including BLOB_LONG.
	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, &rgInfo, 
		 &cCols, &pStringsBuffer, DBTYPE_EMPTY, 0, NULL, NULL, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG, 
		 &rgStatus),S_OK)

	//Verify status of bindings.
	for(ulIndex=0; ulIndex<cBindings; ulIndex++)
		TESTC(rgStatus[ulIndex] == DBBINDSTATUS_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Get number of rows.
	cRows = m_pTable->CountRowsOnTable();

	//Donot call RestartPosition. We should be at the beginning of
	//the rowset.

	//For each row in the table, fetch the row, compare the data in the 
	//row and release the row.
	for(ulIndex=0; ulIndex< (DBCOUNTITEM)cRows; ulIndex++)
		TESTC(GetDataAndCompare(0, 1, ulIndex+1, cRowsetCols, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Call the IAccessor::GetBindings method and verify.
	TESTC(VerifyBindings(pIAccessor, hAccessor, cBindings, rgBindings,
		DBACCESSOR_ROWDATA))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rgStatus);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	return tTestResult;
} //testIAccessor


//----------------------------------------------------------------------
//@mfunc This function is used to test IRowsetInfo::GetProperties and 
//ICommandProperties::GetProperties. Obtain the appropriate interface,
//IRowsetInfo or ICommandProperties. Construct a DBPROPIDSET with four
//properties. Call the GetProperties method with this DBPROPIDSET. Verify
//the returned values and properties. Then call the GetProperties method
//again with (0,NULL) to get ALL supported rowset properties. Verify the
//returned values and properties.
//
TESTRESULT CQuickTest::testRowsetGetProp(IUnknown* pIUnknown)
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			ulSupported = 0;
	ULONG			iSet=0, iProp=0;
	ULONG			cPropSets = 0;
	DBPROP*			pProp = NULL;
	DBPROPSET*		rgPropSets = NULL;

	//IUnknown could either be IRowset or ICommand.
	IRowsetInfo* pIRowsetInfo = NULL;
	ICommandProperties* pICommandProperties = NULL;

	if(!pIUnknown)
		return TEST_FAIL;

	//Get FOUR properties
	FreeProperties(&m_cPropIDSets, &m_rgPropIDSets);

	//make DBPROPIDSET with FOUR properties in it.
	SetPropID(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET);
	SetPropID(DBPROP_OWNUPDATEDELETE, DBPROPSET_ROWSET);
	SetPropID(DBPROP_OWNINSERT, DBPROPSET_ROWSET);
	SetPropID(DBPROP_REMOVEDELETED, DBPROPSET_ROWSET);

	//Call GetProperties with the above made DBPROPIDSET.

	// IID_IRowsetInfo
	if((hr=pIUnknown->QueryInterface(IID_IRowsetInfo, (void **)
		&pIRowsetInfo))==S_OK)
		hr = pIRowsetInfo->GetProperties(m_cPropIDSets, m_rgPropIDSets, 
		&cPropSets, &rgPropSets);
	// IID_ICommandProperties
	else if((hr=pIUnknown->QueryInterface(IID_ICommandProperties, 
		(void **)&pICommandProperties))==S_OK)
		hr = pICommandProperties->GetProperties(m_cPropIDSets, 
		m_rgPropIDSets, &cPropSets, &rgPropSets);

	//Verify that 1 DBPROPSET was returned.
	TESTC((cPropSets==1)&&(rgPropSets != NULL))

	//Verify the guid of property set and count.
	COMPARE(rgPropSets[0].guidPropertySet, DBPROPSET_ROWSET);
	COMPARE(rgPropSets[0].cProperties, m_rgPropIDSets[0].cPropertyIDs);

	//Check the status of properties against the return code.
	if(hr == S_OK)
	{
		for(iProp=0; iProp<rgPropSets[0].cProperties; iProp++)
		{
			pProp = &(rgPropSets[0].rgProperties[iProp]) ;

			//Verify status.
			COMPARE(pProp->dwStatus, DBPROPSTATUS_OK);
	
			//Verify the variant.
			COMPARE((V_VT(&pProp->vValue) == VT_BOOL) ||
				(V_VT(&pProp->vValue) == VT_EMPTY), TRUE);
		}
	}
	else if(hr == DB_S_ERRORSOCCURRED)
	{
		for(iProp=0; iProp<rgPropSets[0].cProperties; iProp++)
		{
			pProp = &(rgPropSets[0].rgProperties[iProp]) ;

			//Verify status.
			if(pProp->dwStatus != DBPROPSTATUS_OK)
			{
				//SupportedProperty() should return FALSE.
				COMPARE(SupportedProperty(pProp->dwPropertyID,
					DBPROPSET_ROWSET), FALSE);

				//CANHOLDROWS cannot be among the not supported props.
				COMPARE(pProp->dwPropertyID != DBPROP_CANHOLDROWS, TRUE);
			}
			else
				ulSupported++;
		}

		//At least one prop has to be supported for this return code.
		//Also, all props cannot be supported for this return code.
		COMPARE(ulSupported >0, TRUE);
		COMPARE(ulSupported <4, TRUE);
	}
	else
	{
		//At least CANHOLDROWS has to be supported.
		TESTC_(hr, S_OK)
	}
	FreeProperties(&cPropSets, &rgPropSets);

	//Get ALL properties.

	if(pIRowsetInfo != NULL)
		hr = pIRowsetInfo->GetProperties(0, NULL, &cPropSets, &rgPropSets);
	// IID_ICommandProperties
	else 
	{
		TESTC(pICommandProperties != NULL)
		hr = pICommandProperties->GetProperties(0, NULL, &cPropSets, 
		&rgPropSets);
	}

	//At least CANHOLDROWS should be supported.
	TESTC_(hr, S_OK)

	//Check return values.
	TESTC((cPropSets!=0) && (rgPropSets!=NULL))

	//Loop through the property sets.
	for(iSet=0; iSet<cPropSets; iSet++)
	{
		//Check to see if the Provider has returned an invalid PropSets
		if( (rgPropSets[iSet].guidPropertySet == DBPROPSET_COLUMN) || 
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCE) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCEINFO) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DBINIT) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_INDEX)  ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_PROPERTIESINERROR) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_SESSION)||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_TABLE) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DATASOURCEALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_ROWSETALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_SESSIONALL) ||
			(rgPropSets[iSet].guidPropertySet == DBPROPSET_DBINITALL) )
		{
			odtLog<<L"ERROR: An Invalid OLEDB PropertySet has been returned by GetProperties!\n";
			goto CLEANUP;
		}

		if(rgPropSets[iSet].guidPropertySet != DBPROPSET_ROWSET)
		{
			odtLog<<L"INFO: Found a provider specific property set.\n";
		}

		//Loop through the properties.
		for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(rgPropSets[iSet].rgProperties[iProp]);

			//Verify if status is DBPROPSTATUS_OK.
			COMPARE(pProp->dwStatus, DBPROPSTATUS_OK);

			//For the properties DBPROP_CANHOLDROWS, DBPROP_OWNINSERT,
			//DBPROP_OWNUPDATEDELETE and DBPROP_REMOVEDELETED verify
			//if the variant type is VT_BOOL or VT_EMPTY.
			if((rgPropSets[iSet].guidPropertySet == DBPROPSET_ROWSET)&&
				((pProp->dwPropertyID==DBPROP_CANHOLDROWS) ||
				(pProp->dwPropertyID==DBPROP_OWNUPDATEDELETE) ||
				(pProp->dwPropertyID==DBPROP_OWNINSERT) ||
				(pProp->dwPropertyID==DBPROP_REMOVEDELETED)))
			{
				if(V_VT(&pProp->vValue) == VT_BOOL)
				{
					COMPARE(CheckVariant(&pProp->vValue), TRUE);
				}
				else 
				{
					COMPARE(V_VT(&pProp->vValue), VT_EMPTY);
				}
			}

			//For certain variant types do some checking.
			COMPARE(CheckVariant(&pProp->vValue), TRUE);
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&m_cPropIDSets, &m_rgPropIDSets);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommandProperties);
	return tTestResult;
} //testRowsetGetProp


//----------------------------------------------------------------------
//@mfunc Obtain the IRowsetInfo interface. Call the method 
//GetReferencedRowset. Pass it the ordinal of the bookmark column which
//is used to obtain the rowset corresponding to these 
//bookmarks. Verify that the obtained IRowset interface is the same as
//the one we got on generating this rowset.
//
TESTRESULT CQuickTest::testIRowsetInfoGetRefRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	BOOL			bBookmarksSupp = FALSE;
	IRowset*		pIRowset = NULL;
	IRowsetInfo*	pIRI = NULL;

	//Obtain the IRowsetInfo interface.
	if(!VerifyInterface(m_pIRowset,IID_IRowsetInfo,
		ROWSET_INTERFACE,(IUnknown**)&pIRI))
		return TEST_FAIL;

	if(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		bBookmarksSupp = TRUE;

	//Use the method to obtain the IRowset interface and verify it is
	//the same as the one we got on creating the rowset.
	hr = pIRI->GetReferencedRowset(0, IID_IRowset, (IUnknown**)
		&pIRowset) ;
	
	if(bBookmarksSupp)
	{
		TESTC_(hr, S_OK)
		TESTC(VerifyEqualInterface((IUnknown*)pIRowset,(IUnknown*)m_pIRowset))
	}
	else
	{
		odtLog<<"Bookmarks are not supported.\n";
		TESTC_(hr, DB_E_BADORDINAL)
		tTestResult = TEST_SKIPPED;
		goto CLEANUP ;
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRI);
	return tTestResult;
} //testIRowsetInfoGetRefRowset


//----------------------------------------------------------------------
//@mfunc Obtain the IRowsetInfo interface. Use its GetSpecification 
//method to obtain the IOpenRowset interface. Verify that this is the 
//same interface which we had obtained on creating this DB Session. Use
//the GetSpecification method again to obtain the IGetDataSource 
//interface. Verify this interface by using it to obtain the 
//IDBInitialize interface (using IGetDataSource::GetDataSource) and 
//verifying that it is the same as the IDBInitialize interface which 
//was used to initialize the Data Source Object.
//
TESTRESULT CQuickTest::testIRowsetInfoGetSpec()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	IOpenRowset*	pIOR = NULL;
	IGetDataSource*	pIGDS = NULL;
	IDBInitialize*	pIDBI = NULL;
	IRowsetInfo*	pIRI = NULL;

	//Obtain IRowsetInfo interface.
	if(!VerifyInterface(m_pIRowset,IID_IRowsetInfo,
		ROWSET_INTERFACE,(IUnknown**)&pIRI))
		return TEST_FAIL;

	//Use the method to obtain the IOpenRowset Interface.
	TEST2C_(hr = pIRI->GetSpecification(IID_IOpenRowset, 
		(IUnknown**)&pIOR), S_OK, S_FALSE)

	//The method can return S_FALSE if the provider does not have an 
	//object that created the rowset.
	if(hr == S_OK)
	{
		COMPARE(VerifyEqualInterface((IUnknown*)pIOR, 
			(IUnknown*)m_pIOpenRowset), TRUE);
	}
	else
	{
		odtLog<<L"WARNING: S_FALSE was returned when asking for IOpenRowset.\n";
		CHECKW(hr, S_OK);
		COMPARE(pIOR, NULL);
	}

	//Use the method to obtain the IGetDataSource interface. Use that
	//to then obtain the IDBInitialize interface.
	TEST2C_(hr = pIRI->GetSpecification(IID_IGetDataSource, 
		(IUnknown**)&pIGDS), S_OK, S_FALSE)

	if(hr == S_OK)
	{
		TESTC_(pIGDS->GetDataSource(IID_IDBInitialize, (IUnknown**)&pIDBI),
			S_OK)
		COMPARE(VerifyEqualInterface((IUnknown*)pIDBI, 
			(IUnknown*)m_pIDBInitialize), TRUE);
	}
	else
	{
		odtLog<<L"WARNING: S_FALSE was returned when asking for IGetDataSource.\n";
		CHECKW(hr, S_OK);
		COMPARE(pIGDS, NULL);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRI);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIGDS);
	SAFE_RELEASE(pIDBI);
	return tTestResult;
} //testIRowsetInfoGetSpec


//----------------------------------------------------------------------
//@mfunc Obtain the IRowsetInfo interface by executing a command. Use 
//its GetSpecification method to obtain the IColumnsInfo interface. 
//Verify that this belongs to the same command object which was used to 
//create the rowset.
//
TESTRESULT CQuickTest::testIRowsetInfoGetSpecCmd()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	IRowsetInfo*	pIRI = NULL;
	ICommand*		pICommand = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;

	if(m_pIDBCreateCommand)
	{
		TESTC_(hr = m_pIDBCreateCommand->CreateCommand(NULL, 
			IID_ICommand,(IUnknown **)&pICommand),S_OK) 
	}
	else
	{
		odtLog<<L"Commands are NOT supported.\n";
		return TEST_SKIPPED;
	}
	TESTC(pICommand != NULL)

	TESTC(m_pTable != NULL)

	//Create SQL stmt and set command text and execute.
	CHECK(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL,
		IID_IRowsetInfo, NULL, NULL, NULL, NULL, 
		EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIRI, 
		&pICommand), S_OK);

	TESTC(pIRI != NULL)

	//Use the method to obtain the IColumnsInfo Interface.
	TEST2C_(hr = pIRI->GetSpecification(IID_IColumnsInfo, 
		(IUnknown**)&pIColumnsInfo), S_OK, S_FALSE)

	//The method can return S_FALSE if the provider does not have an 
	//object that created the rowset.
	if(hr == S_OK)
	{
		TESTC(VerifyEqualInterface(pICommand, pIColumnsInfo))
	}
	else
	{
		odtLog<<L"WARNING: S_FALSE was returned.\n";
		CHECKW(hr, S_OK);
		COMPARE(pIColumnsInfo, NULL);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRI);
	return tTestResult;
} //testIRowsetInfoGetSpec


//----------------------------------------------------------------------
//@mfunc Generate a rowset on the table with NO properties set. Get the
//accessor and bindings. Call the method RestartPosition to make sure 
//the cursor is on the first row. Fetch one row at a time, skipping rows
//in the forward direction. This is done using the method GetNextRows. 
//Each time a row is fetched, obtain its data using the method GetData 
//and verify it. Release the row using method ReleaseRows before fetching
//another row.
//
TESTRESULT CQuickTest::testIRowset2()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cRows = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowset*		pIRowset = NULL;

	//Make sure NO properties are set.
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	//Open a rowset with NO properties set.
	if(!CHECK(hr=CreateOpenRowset(m_pTable, IID_IRowset,
		(IUnknown**)&pIRowset), S_OK))
		return TEST_FAIL;

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, 
	//status and value. This accessor binds the Index
	//column also.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Count number of rows on the table
	cRows = m_pTable->CountRowsOnTable();
	TESTC(cRows >= MIN_ROWS)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Skip the 1st row and fetch the 2nd row.
	TESTC(GetDataAndCompare(1, 1, 2, cRowsetCols, 
			rgColumnsOrd, pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Skip 2 more rows and fetch the 5th row.
	TESTC(GetDataAndCompare(2, 1, 5, cRowsetCols, 
			rgColumnsOrd, pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Restart position.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Skip 3 rows and fetch the 4th row.
	TESTC(GetDataAndCompare(3, 1, 4, cRowsetCols, 
		rgColumnsOrd, pIRowset,  hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Skip 0 and fetch the 5th row.
	TESTC(GetDataAndCompare(0, 1, 5, cRowsetCols, 
		rgColumnsOrd, pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Restart position.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Try to open more than 2 rows at a time if the MAXOPENROWS property
	//permits this.
	if(m_ulMaxOpenRows>1 || m_ulMaxOpenRows==0)
	{
		TESTC_(hr = GetNextRows(pIRowset, NULL, 0, 2, &cRowsObtained,
			(HROW**)&rghRows), S_OK)

		CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	}
	
	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	return tTestResult;
} //testIRowset2


//----------------------------------------------------------------------
//@mfunc Set the properties CANHOLDROWS and CANSCROLLBACKWARDS. Generate
//a rowset on the table using these properties. Get the accessor and 
//bindings. Call method RestartPosition. Skip 4 rows in the forward 
//direction and fetch the 5th row (method GetNextRows). Get its data 
//(method GetData) and compare. Scroll backwards 2 rows and fetch the 
//3rd row. Compare its data. Scroll backwards again 1 row to fetch the 
//2nd row. Compare its data. Finally release all the rows we obtained 
//above.
//
TESTRESULT CQuickTest::testIRowset3()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cRows = 0;
	DBORDINAL		cColumns = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;

	//The properties DBPROP_CANHOLDROWS and DBPROP_CANSCROLLBACKWARDS
	//have been set. 

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, 
	//status and value. This accessor binds the Index
	//column also.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Count number of rows on the table
	cRows = m_pTable->CountRowsOnTable();
	TESTC(cRows >= MIN_ROWS);

	//Make sure we are on the first row.
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Skip 4 rows and fetch the 5th row. Compare its data.
	TESTC(GetDataAndCompare(4, 1, 5, cColumns, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Scroll backwards 2 rows and fetch the 3rd row. Compare its data.
	TESTC(GetDataAndCompare(-3, 1, 3, cColumns, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))
	
	//Scroll backwards 1 row and fetch the 2rd row. Compare its data.
	TESTC(GetDataAndCompare(-2, 1, 2, cColumns, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))
	
	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	return tTestResult;
} //testIRowset3


//----------------------------------------------------------------------
//@mfunc Get bookmarks for rows 1, 2 and 4. Use IRowsetLocate::Compare
//to compare them.
//
TESTRESULT	CQuickTest::testIRowsetLocateCompare()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBBKMARK		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	DBBOOKMARK		dbBkm = DBBMK_FIRST;
	BYTE*			pBkm = (BYTE*) &dbBkm;
	DBBOOKMARK		dbBkm2 = DBBMK_LAST;
	BYTE*			pBkm2 = (BYTE*) &dbBkm;
	DBCOMPARE		dwComparison1 = DBCOMPARE_NOTCOMPARABLE;
	DBCOMPARE		dwComparison2 = DBCOMPARE_NOTCOMPARABLE;
	DBCOMPARE		dwComparison3 = DBCOMPARE_NOTCOMPARABLE;
	DBCOMPARE		dwComparison4 = DBCOMPARE_NOTCOMPARABLE;
	DBCOMPARE		dwComparison5 = DBCOMPARE_NOTCOMPARABLE;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowsetLocate*	pIRowsetLocate = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetLocate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetLocate))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	//get the bookmark for the 1st row
	TESTC(GetBookmark(1,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//get the bookmark for the 2nd row 
	TESTC(GetBookmark(2,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[1],&rgpBookmarks[1]))

	//get the bookmark for the 4th row 
	TESTC(GetBookmark(4,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[2],&rgpBookmarks[2]))

	//Compare rows 2 and 4.
	TESTC_(hr=pIRowsetLocate->Compare(NULL,rgcbBookmarks[1],rgpBookmarks[1],
		rgcbBookmarks[2],rgpBookmarks[2],&dwComparison1),S_OK)
	//Compare rows 4 and 1.
	TESTC_(hr=pIRowsetLocate->Compare(NULL,rgcbBookmarks[2],rgpBookmarks[2],
		rgcbBookmarks[0],rgpBookmarks[0],&dwComparison2),S_OK)
	//Compare rows 2 and DBBMK_FIRST.
	TESTC_(hr=pIRowsetLocate->Compare(NULL,rgcbBookmarks[1],rgpBookmarks[1],
		STD_BOOKMARKLENGTH,pBkm,&dwComparison3),S_OK)
	//Compare rows 4 and 4.
	TESTC_(hr=pIRowsetLocate->Compare(NULL,rgcbBookmarks[2],rgpBookmarks[2],
		rgcbBookmarks[2],rgpBookmarks[2],&dwComparison4),S_OK)
	//Compare rows 2 and DBBMK_LAST.
	TESTC_(hr=pIRowsetLocate->Compare(NULL,rgcbBookmarks[1],rgpBookmarks[1],
		STD_BOOKMARKLENGTH,pBkm2,&dwComparison5),S_OK)

	//check the comparisons.

	//If ordered-bookmarks are supported.
	if(GetProperty(DBPROP_ORDEREDBOOKMARKS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		COMPARE(dwComparison1, DBCOMPARE_LT);
		COMPARE(dwComparison2, DBCOMPARE_GT);
		COMPARE(dwComparison3, DBCOMPARE_NE);
		COMPARE(dwComparison4, DBCOMPARE_EQ);
		COMPARE(dwComparison5, DBCOMPARE_NE);
	}
	else
	{
		COMPARE(dwComparison1, DBCOMPARE_NE);
		COMPARE(dwComparison2, DBCOMPARE_NE);
		COMPARE(dwComparison3, DBCOMPARE_NE);
		COMPARE(dwComparison4, DBCOMPARE_EQ);
		COMPARE(dwComparison5, DBCOMPARE_NE);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	PROVIDER_FREE(rgpBookmarks[2]);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetLocate);
	return tTestResult;
} //testIRowsetLocateCompare


//----------------------------------------------------------------------
//@mfunc Use IRowsetLocate::GetRowsAt to fetch rows with respect to a
//row whose bookmark is provided.
//
TESTRESULT	CQuickTest::testIRowsetLocateGetRowsAt()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			ulIndex = 0;   //small no.
	DBBKMARK		rgcbBookmarks[1];
	BYTE			*rgpBookmarks[1] = {NULL};
	DBBOOKMARK		dbBkm = DBBMK_FIRST;
	BYTE*			pBkm = (BYTE*) &dbBkm;
	BYTE*			pData = NULL;
	HROW*			rghRows = NULL;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowsetLocate*	pIRowsetLocate = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetLocate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetLocate))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL,NULL,NULL,NULL))

	//get the bookmark for the 2nd row
	TESTC(GetBookmark(2,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//Fetch 3rd and 4th rows.
	TESTC_(hr=pIRowsetLocate->GetRowsAt(NULL, NULL, rgcbBookmarks[0],
		rgpBookmarks[0], 1, 2, &cRowsObtained, &rghRows), S_OK)

	SAFE_ALLOC(pData, BYTE, cbRowSize);

	TESTC(cRowsObtained==2 && rghRows)

	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
	{
		memset(pData, 0, (size_t) cbRowSize);

		TESTC_(hr=m_pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData), S_OK)

		TESTC(CompareData(cColumns, rgColumnsOrd, 3+ulIndex, pData, 
			cBindings, rgBindings, m_pTable, m_pIMalloc, PRIMARY))

		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	}

	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);

	//Fetch the 3rd and 4th rows using standard bookmarks.
	TESTC_(hr=pIRowsetLocate->GetRowsAt(NULL, NULL, STD_BOOKMARKLENGTH,
		pBkm, 2, 2, &cRowsObtained, &rghRows), S_OK)
	
	TESTC(cRowsObtained==2 && rghRows)

	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
	{
		memset(pData, 0, (size_t) cbRowSize);

		TESTC_(hr=m_pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData), S_OK)

		TESTC(CompareData(cColumns, rgColumnsOrd, 3+ulIndex, pData, 
			cBindings, rgBindings, m_pTable, m_pIMalloc, PRIMARY))

		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	}

	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);

	//SCROLL_BACKWARDS

	//Fetch 1st, 2nd, and 3rd rows using SCROLL_BACKWARDS.
	hr = pIRowsetLocate->GetRowsAt(NULL, NULL, rgcbBookmarks[0],
		rgpBookmarks[0], -1, 3, &cRowsObtained, &rghRows) ;

	if(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		TESTC_(hr, S_OK)
		TESTC(cRowsObtained==3 && rghRows)
	}
	else
	{
		CHECK(hr, DB_E_CANTSCROLLBACKWARDS);
		tTestResult = TEST_PASS;
		goto CLEANUP;
	}

	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
	{
		memset(pData, 0, (size_t) cbRowSize);

		TESTC_(hr=m_pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData), S_OK)

		TESTC(CompareData(cColumns, rgColumnsOrd, 1+ulIndex, pData, 
			cBindings, rgBindings, m_pTable, m_pIMalloc, PRIMARY))

		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetLocate);
	return tTestResult;
} //testIRowsetLocateGetRowsAt


//----------------------------------------------------------------------
//@mfunc Get bookmarks to some rows and pass this in to IRowsetLocate::
//GetRowsByBookmark. Verify the returned rows.
//
TESTRESULT	CQuickTest::testIRowsetLocateGetRowsByBkm()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			ulIndex = 0;  // max 3
	DBCOUNTITEM		cGotRows = 0;
	DBCOUNTITEM		iRowNum[3] = {2,4,5};  //Row numbers of rows to get.
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	BYTE*			pData = NULL;
	DBBKMARK		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	DBROWSTATUS		rgRowStatus[3];		//Not required to initialize.
	HROW			rghRows[3] ;		//Not required to initialize.
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowsetLocate*	pIRowsetLocate = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetLocate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetLocate))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL,NULL,NULL,NULL))

	//get the bookmark for the 2nd row
	TESTC(GetBookmark(2,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//get the bookmark for the 4th row 
	TESTC(GetBookmark(4,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[1],&rgpBookmarks[1]))

	//get the bookmark for the 5th row 
	TESTC(GetBookmark(5,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[2],&rgpBookmarks[2]))

	TESTC_(hr=pIRowsetLocate->GetRowsByBookmark(NULL, 3, rgcbBookmarks,
		(const BYTE**)rgpBookmarks, rghRows, rgRowStatus), S_OK)

	for(ulIndex=0; ulIndex<3; ulIndex++)
	{
		TESTC(rghRows[ulIndex] != DB_NULL_HROW)
		cGotRows++;
		COMPARE(rgRowStatus[ulIndex] , DBROWSTATUS_S_OK);
	}

	SAFE_ALLOC(pData, BYTE, cbRowSize);

	for(ulIndex=0; ulIndex<3; ulIndex++)
	{
		memset(pData, 0, (size_t) cbRowSize);

		TESTC_(hr=m_pIRowset->GetData(rghRows[ulIndex], hAccessor, 
			pData), S_OK)

		COMPARE(CompareData(cColumns, rgColumnsOrd, iRowNum[ulIndex], 
			pData, cBindings, rgBindings, m_pTable, m_pIMalloc, 
			PRIMARY), TRUE);

		ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(m_pIRowset->ReleaseRows(3, rghRows, NULL,NULL,NULL), S_OK);
	PROVIDER_FREE(rgColumnsOrd);
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	PROVIDER_FREE(rgpBookmarks[2]);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetLocate);
	return tTestResult;
} //testIRowsetLocateGetRowsByBkm


//----------------------------------------------------------------------
//@mfunc Get hash values for certain bookmarks.
//
TESTRESULT	CQuickTest::testIRowsetLocateHash()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG			ulIndex = 0;  //max 3
	DBBKMARK		rgcbBookmarks[3];
	BYTE			*rgpBookmarks[3]={NULL,NULL,NULL};
	DBHASHVALUE		rgHashedValues[3];    //Not required to initialize.
	DBROWSTATUS		rgBookmarkStatus[3];  //Not required to initialize.
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowsetLocate*	pIRowsetLocate = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetLocate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetLocate))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	//get the bookmark for the 1st row
	TESTC(GetBookmark(1,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//get the bookmark for 1st row again
	TESTC(GetBookmark(1,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[1],&rgpBookmarks[1]))

	//get the bookmark for the 4th row 
	TESTC(GetBookmark(4,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[2],&rgpBookmarks[2]))

	//Hash returns different values
	TESTC_(hr = pIRowsetLocate->Hash(NULL,3,rgcbBookmarks,
		(const BYTE **)rgpBookmarks, rgHashedValues,rgBookmarkStatus),
		S_OK)

	for(ulIndex=0; ulIndex<3; ulIndex++)
	{
		COMPARE(rgBookmarkStatus[ulIndex] , DBROWSTATUS_S_OK);
	}

	//The first 2 hash values are for the first row, hence they
	//should be equal. The third one should be different.
	COMPARE(rgHashedValues[0] == rgHashedValues[1], TRUE);
	COMPARE(rgHashedValues[1] != rgHashedValues[2], TRUE);
	COMPARE(rgHashedValues[0] != rgHashedValues[2], TRUE);

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	PROVIDER_FREE(rgpBookmarks[2]);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetLocate);
	return tTestResult;
} //testIRowsetLocateHash


//----------------------------------------------------------------------
//@mfunc Make data corresponding to a certain row and column (choosing a
//fixed length column). Use this as the search criterion for FindNextRow.
//Verify the obtained rows.
//
TESTRESULT CQuickTest::testFindNextRow()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBORDINAL		ulIndex = 0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	DBCOUNTITEM		ulRowNum = 0;
	DB_LORDINAL		ulColNum = 0;
	DBLENGTH		cbDataLength = 0;
	DBCOLUMNINFO*	rgInfo = NULL;
	WCHAR*			pStrBuf = NULL;
	WCHAR			wszData[2000];
	BYTE*			pMakeData = NULL;
	void*			pvData = NULL;
	BYTE*			pFindValue = NULL;
	BYTE*			pData = NULL;
	DBTYPE			wColType = DBTYPE_EMPTY;
	DBTYPE			wVariantType = DBTYPE_EMPTY;
	DBBKMARK		rgcbBookmarks[1];
	BYTE			*rgpBookmarks[1]={NULL};
	HROW*			rghRows = NULL ;
	DBCOUNTITEM		cRowsObtained = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;
	IRowsetFind*	pIRowsetFind = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetFind,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetFind))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	//Bookmark will be got for the following row num.
	ulRowNum = 2; 

	//get the bookmark for the 'ulRowNum' row
	TESTC(GetBookmark(ulRowNum,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//Release hAccessor and free bindings. But don't release IAccessor.
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings,
		FALSE);

	TESTC(VerifyInterface(m_pIRowset,IID_IColumnsInfo,
		ROWSET_INTERFACE,(IUnknown**)&pIColumnsInfo))

	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgInfo, 
		&pStrBuf), S_OK)

	//Get a fixed-length column whose data we will use for the FindRow.
	for(ulIndex=1; ulIndex<cColumns; ulIndex++)
	{
		if(IsFixedLength(rgInfo[ulIndex].wType))
			break;
	}
	if(ulIndex==cColumns)
	{
		odtLog<<L"Could not find a fixed length column to bind to for the findValue.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	wColType = rgInfo[ulIndex].wType ;
	ulColNum = rgInfo[ulIndex].iOrdinal;

	//MakeData will generate data for the following row num.
	ulRowNum = 3;

	//MakeData to be used for FindNextRow.
	TESTC_(hr=m_pTable->MakeData(wszData, ulRowNum, ulColNum, PRIMARY, 
		wColType, TRUE, &wVariantType), S_OK)

	//If it is a variant, then use correct variant type.
	pMakeData = (BYTE *)WSTR2DBTYPE(wszData, ( wColType == 
		DBTYPE_VARIANT ? wVariantType : wColType) ,NULL );

	cbDataLength = GetDBTypeSize(wColType) ;

	SAFE_ALLOC(pFindValue, BYTE, cbDataLength+offsetof(DATA, bValue))

	//If the find value is to be a variant then convert the data to 
	//a variant.
	if ( wColType == DBTYPE_VARIANT )
		pvData = DBTYPE2VARIANT(pMakeData, wVariantType);
	else 
		pvData = pMakeData;

	//Create the binding for the find value.
	memcpy(pFindValue+offsetof(DATA, bValue), pvData, (size_t) cbDataLength);

	*(DBSTATUS *)(pFindValue+offsetof(DATA, sStatus)) = DBSTATUS_S_OK;
		
	*(DBLENGTH *)(pFindValue+offsetof(DATA, ulLength)) = cbDataLength;		

	//Create an accessor and binding for one column (ulColNum).
	TESTC_(hr = GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA, &hAccessor, &rgBindings, &cBindings, 
		&cbRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,  NULL, 
		NULL, NULL, DBTYPE_EMPTY, 1, 
		&ulColNum, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG ), S_OK)

	//Call the method (FindNextRow) to be tested.
	hr = pIRowsetFind->FindNextRow(NULL, hAccessor, pFindValue, 
		DBCOMPAREOPS_EQ, rgcbBookmarks[0], rgpBookmarks[0], 0, 2, 
		&cRowsObtained, &rghRows) ;

	//Make sure we got back 2 rows. 
	TESTC_(hr, S_OK)
	TESTC(cRowsObtained == 2)

	SAFE_ALLOC(pData, BYTE, cbRowSize);
	memset(pData, 0, (size_t) cbRowSize);

	//Get data for the first of the 2 rows. The data in this should
	//match our search data.
	TESTC_(hr = m_pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

	TESTC(CompareBuffer((void*)pData, (void*)pFindValue, 1, rgBindings,
		m_pIMalloc, FALSE, FALSE, COMPARE_ONLY))

	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);

	memset(pData, 0, (size_t) cbRowSize);

	//Get the data for the second of the 2 rows. This should be the
	//row which comes right after the row which matched the search 
	//data.
	TESTC_(hr = m_pIRowset->GetData(rghRows[1], hAccessor, pData),
		S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(CompareData(cRowsetCols,rgColumnsOrd,ulRowNum+1,pData,1,
		rgBindings, m_pTable, m_pIMalloc, PRIMARY))

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(pStrBuf);
	PROVIDER_FREE(pvData);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(rgColumnsOrd);
	SAFE_FREE(pFindValue);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIRowsetFind);
	return tTestResult;
} //testFindNextRow


//----------------------------------------------------------------------
//@mfunc Get the bookmarks for some rows and obtain their approximate 
//position by calling the method GetApproximatePosition. Verify that
//the positions returned have reasonable approximations.
//
TESTRESULT CQuickTest::testGetApproxPos()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBBKMARK			rgcbBookmarks[3];
	BYTE				*rgpBookmarks[3]={NULL,NULL,NULL};
	DBCOUNTITEM			cRowsOnTable = 0;
	DBCOUNTITEM			cRows = 0;
	DBCOUNTITEM			ulPosition = 0;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	DBBINDING*			rgBindings = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	IAccessor*			pIAccessor = NULL;
	IRowsetScroll*		pIRS = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetScroll,
		ROWSET_INTERFACE,(IUnknown**)&pIRS))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor.
	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_LONG, NULL),S_OK)

	//get the bookmark for the 1st row
	TESTC(GetBookmark(1,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[0],&rgpBookmarks[0]))

	//get the bookmark for the 3rd row 
	TESTC(GetBookmark(3,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[1],&rgpBookmarks[1]))

	//get the bookmark for the 5th row 
	TESTC(GetBookmark(5,cbRowSize,hAccessor,cBindings, rgBindings,
		(IUnknown*)m_pIRowset, &rgcbBookmarks[2],&rgpBookmarks[2]))

	cRowsOnTable = m_pTable->CountRowsOnTable();

	//Bookmark for row 1.
	TESTC_(hr = pIRS->GetApproximatePosition(NULL, rgcbBookmarks[0],
		rgpBookmarks[0], &ulPosition, &cRows), S_OK)
	TESTC(cRows >= ulPosition)
	odtLog<<L"INFO: Position returned for row 1 = "<<ulPosition<<L".\n";

	//Bookmark for row 3.
	TESTC_(hr = pIRS->GetApproximatePosition(NULL, rgcbBookmarks[1],
		rgpBookmarks[1], &ulPosition, &cRows), S_OK)
	TESTC(cRows >= ulPosition)
	odtLog<<L"INFO: Position returned for row 3 = "<<ulPosition<<L".\n";

	//Bookmark for row 5.
	TESTC_(hr = pIRS->GetApproximatePosition(NULL, rgcbBookmarks[2],
		rgpBookmarks[2], &ulPosition, &cRows), S_OK)
	TESTC(cRows >= ulPosition)
	odtLog<<L"INFO: Position returned for row 5 = "<<ulPosition<<L".\n";

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgpBookmarks[0]);
	PROVIDER_FREE(rgpBookmarks[1]);
	PROVIDER_FREE(rgpBookmarks[2]);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRS);
	return tTestResult;
} //testGetApproxPos


//----------------------------------------------------------------------
//@mfunc Call the method GetRowsAtRatio using 0 for numerator and the
//total number of rows for the denominator. This should fetch rows 
//starting with the first row in the rowset. Verify the obtained rows.
//
TESTRESULT CQuickTest::testGetRowsAtRatio()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG				ulIndex = 0;  //small no.
	DBCOUNTITEM			cRowsOnTable = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBCOUNTITEM			cRowsObtained2 = 0;
	HROW*				rghRows = NULL;
	HROW*				rghRows2 = NULL;
	IRowsetScroll*		pIRS = NULL;
	IRowsetIdentity*	pIRI = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetScroll,
		ROWSET_INTERFACE,(IUnknown**)&pIRS))

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetIdentity,
		ROWSET_INTERFACE,(IUnknown**)&pIRI))

	cRowsOnTable = m_pTable->CountRowsOnTable();

	//Pass in 0 for numerator and cRowsOnTable for denominator. This
	//is guaranteed to fetch the first 3 rows.
	TESTC_(hr = pIRS->GetRowsAtRatio(NULL, NULL, 0, cRowsOnTable, 3, 
		&cRowsObtained, &rghRows), S_OK)
		
	TESTC(cRowsObtained == 3)
	TESTC(rghRows != NULL)

	//Position cursor to beginning of rowset.
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Fetch the first 3 rows using GetNextRows.
	TESTC_(hr = GetNextRows(m_pIRowset, NULL, 0, 3, &cRowsObtained2,
		&rghRows2), S_OK)

	//Verify that the rows returned by GetRowsAtRatio are the same as
	//that fetched by GetNextRows.
	for(ulIndex=0; ulIndex<cRowsObtained; ulIndex++)
		TESTC_(hr = pIRI->IsSameRow(rghRows[ulIndex], rghRows2[ulIndex]),
			S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained2, &rghRows2), S_OK);
	SAFE_RELEASE(pIRI);
	SAFE_RELEASE(pIRS);
	return tTestResult;
} //testGetRowsAtRatio


//----------------------------------------------------------------------
//@mfunc Initialize IRowPosition with m_pIRowset. Then call the various
//methods of IRowPosition and verify their functionality.
//
TESTRESULT CQuickTest::testIRowPosition()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	HROW*				rghRows = NULL;
	HROW				hRows2 = DB_NULL_HROW;
	DBCOUNTITEM			cRowsObtained = 0;
	VARIANT_BOOL		bValue = VARIANT_FALSE;
	IRowsetInfo*		pIRI = NULL;
	IRowPosition*		pIRowPosition = NULL;

	hr = CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, 
		CLSCTX_INPROC_SERVER, IID_IRowPosition, 
		(LPVOID *)&pIRowPosition);

	if(hr != S_OK)
	{
		odtLog<<L"Could not create an instance of the row position library.\n";
		return TEST_SKIPPED;
	}

	TESTC(pIRowPosition != NULL)

	//The positioning will be done on the m_pIRowset rowset.
	TESTC_(hr = pIRowPosition->Initialize((IUnknown*)m_pIRowset), S_OK)

	//Call GetRowset to obtain IRowsetInfo.
	TESTC_(hr = pIRowPosition->GetRowset(IID_IRowsetInfo, (IUnknown**)
		&pIRI), S_OK)

	TESTC(pIRI != NULL)

	//Use the obtained IRowsetInfo interface.
	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (IUnknown*)
		pIRI, &bValue))

	//Clear any existing row position.
	TESTC_(hr = pIRowPosition->ClearRowPosition(), S_OK)

	//Position cursor to beginning of rowset.
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Get the handle to the 2nd row.
	TESTC_(hr = GetNextRows(m_pIRowset, NULL, 1, 1, &cRowsObtained,
		&rghRows), S_OK)

	//Set the row position to the 2nd row.
	TESTC_(hr = pIRowPosition->SetRowPosition(NULL, rghRows[0], 
		DBPOSITION_OK), S_OK)

	TESTC_(hr = m_pIRowset->ReleaseRows(1,rghRows, NULL, NULL, 
		NULL),S_OK);

	//Get the current row position.
	TESTC_(hr = pIRowPosition->GetRowPosition(NULL, &hRows2, 
		NULL), S_OK)

	//Make sure it is same as the one we had set.
	TESTC(rghRows[0] == hRows2)

	TESTC_(hr = m_pIRowset->ReleaseRows(1,rghRows, NULL, NULL, 
		NULL),S_OK);

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	SAFE_RELEASE(pIRI);
	SAFE_RELEASE(pIRowPosition);
	return tTestResult;
} //testIRowPosition


//----------------------------------------------------------------------
//@mfunc Aggregate the rowset and test it.
//
TESTRESULT CQuickTest::testAggregateRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr;
	IUnknown*		pIUnkInner = NULL;

	CAggregate Aggregate(m_pIOpenRowset);

	TESTC(m_pTable && m_pIOpenRowset)

	//Aggregation
	hr = CreateOpenRowset(m_pTable, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Check if aggregation is supported.
	if(hr==DB_E_NOAGGREGATION)
	{
		odtLog<<"Aggregation is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//Verify Aggregation for this rowset...
	TESTC(Aggregate.VerifyAggregationQI(hr, IID_IRowset))

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	return tTestResult;
} //testAggregateRowset


//----------------------------------------------------------------------
//@mfunc Use the IRowset interface to obtain the IRowsetIdentity 
//interface. Fetch the first 2 rows. If we can fetch backwards, then 
//fetch the second row again. Use the method IsSameRow to compare the 
//row handles. Verify the results. 
//
TESTRESULT CQuickTest::testIRowsetIdentity()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	HROW*				rghRows1 = NULL;
	HROW*				rghRows2 = NULL;
	DBCOUNTITEM			cRowsObtained1 = 0;
	DBCOUNTITEM			cRowsObtained2 = 0;
	IRowsetIdentity*	pIRId = NULL;

	if(m_ulMaxOpenRows == 0)
		m_ulMaxOpenRows = m_pTable->CountRowsOnTable();

	//Obtain IRowsetIdentity interface.
	if(!VerifyInterface(m_pIRowset,IID_IRowsetIdentity,
		ROWSET_INTERFACE,(IUnknown**)&pIRId))
		return TEST_FAIL;
	
	//Position cursor at beginning of rowset.
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Fetch the first 2 rows.
	TEST2C_(hr = GetNextRows(m_pIRowset, NULL, 0, 2, &cRowsObtained1, 
		(HROW **)&rghRows1), S_OK, DB_S_ROWLIMITEXCEEDED)

	if(hr==S_OK)
	{
		TESTC(cRowsObtained1==2 && rghRows1)
	}
	else
	{
		//hr is DB_S_ROWLIMITEXCEEDED.
		COMPARE(m_ulMaxOpenRows<2, TRUE);
		COMPARE(cRowsObtained1, 1);
	}

	//If we can fetch backwards, get the 2nd row again.
	if(m_fFetchBackwards &&(m_ulMaxOpenRows>2))
	{
		//Use IRowset to get 2nd row again.
		TESTC_(hr = GetNextRows(m_pIRowset, NULL, 0, -1, 
			&cRowsObtained2, (HROW **)&rghRows2), S_OK)

		//The 2nd row fetched first time is compared to the 
		//2nd row fetched second time.
		CHECK(pIRId->IsSameRow(rghRows1[1], *rghRows2), S_OK);

		//The 1st row is compared to the 2nd row fetched second time.
		CHECK(pIRId->IsSameRow(rghRows1[0], *rghRows2), S_FALSE);
	}

	//Compare the 1st row with the 2nd row.
	if(cRowsObtained1 == 2)
		CHECK(pIRId->IsSameRow(rghRows1[0], rghRows1[1]), S_FALSE);

	//Compare the 1st row with itself.
	CHECK(pIRId->IsSameRow(rghRows1[0], rghRows1[0]), S_OK);

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained1, &rghRows1), S_OK);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained2, &rghRows2), S_OK);
	SAFE_RELEASE(pIRId);
	return tTestResult;
} //testIRowsetIdentity


//----------------------------------------------------------------------
//@mfunc Call the method RestartPosition. Then fetch all the rows in 
//the table (sequentially) one at a time (in the forward direction). 
//Compare data for that row and release it. Determine the MAXOPENROWS.
//Restart position and open the maximum number of rows all at once. 
//Compare their data. Release them all. Call RestartPosition again. If 
//fetching backwards is supported, then skip 4 rows forward and fetch 
//backwards 3 rows. So we should get the 4th, 3rd and 2nd rows. Compare
//their data and release them.
//
TESTRESULT CQuickTest::testIRowset()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRows = 0, cMultRows = 0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, 
	//status and value. This accessor binds the Index
	//column also.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Get number of rows on table.
	cRows = m_pTable->CountRowsOnTable();

	//Position cursor on first row.
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Get one row at a time, compare and release.
	for(ulIndex=0; ulIndex<cRows; ulIndex++)
		TESTC(GetDataAndCompare(0, 1, ulIndex+1, cRowsetCols, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Determine how many rows we can open at a time.
	if(cRows >= m_ulMaxOpenRows)
		cMultRows = m_ulMaxOpenRows;
	if(cRows < m_ulMaxOpenRows)
		cMultRows = cRows;
	if(m_ulMaxOpenRows == 0)
		cMultRows = cRows;
	
	//Restart position. 
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Now get multiple rows, compare and then release all.
	TESTC(GetDataAndCompare(0, cMultRows, 1, cRowsetCols, 
		rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	//Restart position. 
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//If fetch backwards is supported, then skip 2 rows and fetch
	//backwards 2 rows. We will get rows 2 and 1. Verify their data.
	if((cMultRows>4) && (m_fFetchBackwards))
		TESTC(GetDataAndCompare(2, -2, 2, cRowsetCols, 
			rgColumnsOrd, m_pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	return tTestResult;
} //testIRowset


//----------------------------------------------------------------------
//@mfunc Check if Updateability and DBPROPVAL_UP_CHANGE are supported.
//Create an accessor binds all updateable, non-index columns. Fetch the 
//first row. Set it's data. Call GetData on the first row. Verify the
//data obtained is same as the one we had set.
//
TESTRESULT CQuickTest::testIRowsetChangeSet()
{
	TESTRESULT	tTestResult = TEST_FAIL;
	HRESULT		hr = S_OK;
	ULONG_PTR	updtFlag = 0;
	DBLENGTH	cbRowSize = 0;
	DBORDINAL	cRowsetCols = 0;
	DBCOUNTITEM	cRowsObtained = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	HROW*		rghRows = NULL;
	BYTE*		pData = NULL;
	BYTE*		pData2 = NULL;
	DBCOUNTITEM	cBindings = 0;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*	rgBindings = NULL;
	IAccessor*	pIAccessor = NULL;
	IRowsetChange*	pIRC = NULL;

	if(!IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange))
	{
		odtLog<<L"IID_IRowsetChange is not required.\n";
		return TEST_SKIPPED;
	}

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		m_pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		return TEST_SKIPPED;
	}

	if(!(updtFlag & DBPROPVAL_UP_CHANGE))
	{
		odtLog<<L"CHANGE is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetChange,
		ROWSET_INTERFACE,(IUnknown**)&pIRC))

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Restart position. 
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Allocate 2 new data buffers
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	SAFE_ALLOC(pData2, BYTE, cbRowSize)

	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);

	//Get the first row.
	TESTC_(GetNextRows(m_pIRowset, NULL,0,1,&cRowsObtained,
		&rghRows),S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	//Set data of first row.
	TESTC_(hr=pIRC->SetData(rghRows[0],hAccessor,pData),S_OK)

	//Get the data for the first row
	TESTC_(m_pIRowset->GetData(rghRows[0],hAccessor,pData2),S_OK)

	//make sure GetData should be able to see the change
	TESTC(CompareBuffer(pData2,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2, TRUE);
	PROVIDER_FREE(rgColumnsOrd);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRC);
	return tTestResult;
} //testIRowsetChangeSet


//----------------------------------------------------------------------
//@mfunc @mfunc Check if Updateability and DBPROPVAL_UP_DELETE are 
//supported. Delete the first row. Make sure it was deleted.
//
TESTRESULT CQuickTest::testIRowsetChangeDelete()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ULONG_PTR		updtFlag = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	DBROWSTATUS		rgRowStatus[1]; 
	BYTE*			pData = NULL;
	DBCOUNTITEM		cBindings = 0;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*		rgBindings = NULL;
	IAccessor*		pIAccessor = NULL;
	IRowsetChange*	pIRC = NULL;

	if(!IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange))
	{
		odtLog<<L"IID_IRowsetChange is not required.\n";
		return TEST_SKIPPED;
	}

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		m_pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		return TEST_SKIPPED;
	}

	if(!(updtFlag & DBPROPVAL_UP_DELETE))
	{
		odtLog<<L"DELETE is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetChange,
		ROWSET_INTERFACE,(IUnknown**)&pIRC))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Restart position. 
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Get the first row.
	TESTC_(GetNextRows(m_pIRowset, NULL,0,1,&cRowsObtained,
		&rghRows),S_OK)

	//Delete the first row.
	TESTC_(pIRC->DeleteRows(NULL,1,rghRows,rgRowStatus), S_OK)
	m_pTable->SubtractRow();

	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK)

	SAFE_ALLOC(pData, BYTE, cbRowSize)

	//Call GetData on the deleted row. This should fail.
	hr = m_pIRowset->GetData(rghRows[0], hAccessor, pData);
	CHECKW(hr, DB_E_DELETEDROW);

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pData);
	CHECK(ReleaseRows(&m_pIRowset, &cRowsObtained, &rghRows), S_OK);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRC);
	return tTestResult;
} //testIRowsetChangeDelete


//----------------------------------------------------------------------
//@mfunc Check if Updateability and DBPROPVAL_UP_INSERT are supported.
//Create an accessor binds all updateable, non-index columns. Create
//data for a new row and insert it. Verify that the new row was inserted
//into the rowset.
//
TESTRESULT CQuickTest::testIRowsetChangeInsert()
{
	TESTRESULT	tTestResult = TEST_FAIL;
	HRESULT		hr = S_OK;
	ULONG_PTR	updtFlag = 0;
	DBORDINAL	cRowsetCols = 0;
	ULONG		cRowsInserted = 0;  //small no.
	DB_LORDINAL*	rgColumnsOrd = NULL;
	HROW		hNewRow = DB_NULL_HROW;
	BYTE*		pData = NULL;
	BYTE*		pData2 = NULL;
	DBLENGTH	cbRowSize = 0;
	DBCOUNTITEM	cBindings = 0;
	HACCESSOR	hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*	rgBindings = NULL;
	IAccessor*	pIAccessor = NULL;
	IRowsetChange*	pIRC = NULL;

	if(!IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange))
	{
		odtLog<<L"IID_IRowsetChange is not required.\n";
		return TEST_SKIPPED;
	}

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		m_pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		return TEST_SKIPPED;
	}

	if(!(updtFlag & DBPROPVAL_UP_INSERT))
	{
		odtLog<<L"INSERT is not supported.\n";
		return TEST_SKIPPED;
	}

	TESTC(VerifyInterface(m_pIRowset,IID_IRowsetChange,
		ROWSET_INTERFACE,(IUnknown**)&pIRC))

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(m_pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Restart position. 
	TESTC_(RestartPosition(m_pIRowset), S_OK)

	//Allocate 2 new data buffers
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	SAFE_ALLOC(pData2, BYTE, cbRowSize)

	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData,++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	//Insert the row.
	TESTC_(pIRC->InsertRow(NULL,hAccessor,pData,&hNewRow),S_OK)
	TESTC(hNewRow != DB_NULL_HROW)
	m_pTable->AddRow();
	cRowsInserted++;

	//get the data for the new row
	TESTC_(m_pIRowset->GetData(hNewRow,hAccessor,pData2),S_OK)

	//Compare the data.
	TESTC(CompareBuffer(pData2,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2, TRUE);
	PROVIDER_FREE(rgColumnsOrd);
	CHECK(m_pIRowset->ReleaseRows(cRowsInserted, &hNewRow, NULL,NULL,
		NULL), S_OK);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRC);
	return tTestResult;
} //testIRowsetChangeInsert


//----------------------------------------------------------------------
//@mfunc Create a rowset with IRowsetUpdate. Modify the rowset. Use the
//methods of IRowsetUpdate to Undo and Update changes. Verify results
//of the various methods of this interface.
//
TESTRESULT CQuickTest::testIRowsetUpdate()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG_PTR			updtFlag = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	HROW*				rghRows = NULL;
	BYTE*				pData = NULL;
	BYTE*				pData2 = NULL;
	BYTE*				pOrgData = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	DBCOUNTITEM			cRowsOut = 0;
	DBCOUNTITEM			cPendingRows = 0;
	HROW*				rghRowsOut = NULL;
	HROW*				rgPendingRows = NULL;
	DBROWSTATUS*		rgRowStatus = NULL;
	DBPENDINGSTATUS		pendingStatus = DBPENDINGSTATUS_NEW | 
										DBPENDINGSTATUS_CHANGED |
										DBPENDINGSTATUS_DELETED ;
	DBPENDINGSTATUS*	rgPendingStatus = NULL;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowsetUpdate*		pIRowsetUpdate = NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
						  DBPROPVAL_UP_DELETE|
						  DBPROPVAL_UP_INSERT  ;

	//Set the following properties (ALL REQUIRED).
	SetProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets);

	SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Open a rowset with above properties set.
	hr=CreateOpenRowset(m_pTable, IID_IRowset,
		(IUnknown**)&pIRowset);

	if(hr==DB_E_ERRORSOCCURRED)
	{
		DumpPropertyErrors(m_cPropSets, m_rgPropSets);
		FreeProperties(&m_cPropSets, &m_rgPropSets);
		TESTC(!pIRowset)
		return TEST_SKIPPED;
	}

	TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED)

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//We need DBPROPVAL_UP_CHANGE and DBPROPVAL_UP_DELETE to be supported
	//for this test.
	if(!(updtFlag & DBPROPVAL_UP_CHANGE) || !(updtFlag & DBPROPVAL_UP_DELETE))
	{
		odtLog<<L"DBPROPVAL_UP_CHANGE and/or DBPROPVAL_UP_DELETE is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC(VerifyInterface(pIRowset,IID_IRowsetUpdate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetUpdate))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Donot call RestartPosition. We should be at the beginning of
	//the rowset.

	//Allocate 2 new data buffers
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	SAFE_ALLOC(pData2, BYTE, cbRowSize)
	SAFE_ALLOC(pOrgData, BYTE, cbRowSize)

	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);
	memset(pOrgData, 0, (size_t) cbRowSize);

	//Fetch the 2nd row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,1,1,&cRowsObtained,
		&rghRows),S_OK)

	TESTC_(hr = pIRowset->GetData(rghRows[0],hAccessor,pOrgData),S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	//Set data of 2nd row.
	TESTC_(hr = pIRowsetUpdate->SetData(rghRows[0],hAccessor,pData),S_OK)

	//GetPendingRows should return the 2nd row.
	TESTC_(hr = pIRowsetUpdate->GetPendingRows(NULL, pendingStatus,
		&cPendingRows, &rgPendingRows, &rgPendingStatus), S_OK)

	//Check return values. Pending status should be _CHANGED.
	TESTC(cPendingRows==1 && rgPendingRows && rgPendingStatus)
	TESTC(rgPendingRows[0] != DB_NULL_HROW)
	TESTC(rgPendingStatus[0] == DBPENDINGSTATUS_CHANGED)

	rgPendingStatus[0] = DBPENDINGSTATUS_INVALIDROW;

	//Call GetRowsStatus on rgPendingRows obtained from the call
	//to GetPendingRows. Pending status should be _CHANGED.
	TESTC_(hr = pIRowsetUpdate->GetRowStatus(NULL, 1, rgPendingRows,
		rgPendingStatus), S_OK)
	TESTC(rgPendingStatus[0] == DBPENDINGSTATUS_CHANGED)

	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);

	//Get data for the 2nd row.
	TESTC_(hr = pIRowset->GetData(rghRows[0],hAccessor,pData2),S_OK)

	//The data should be the new one we had set it to.
	TESTC(CompareData(cRowsetCols, rgColumnsOrd, g_ulNextRow, pData2, cBindings,
		rgBindings, m_pTable, m_pIMalloc, PRIMARY, COMPARE_ONLY))

	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2);
	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);

	//This should return the original data for the 2nd row.
	TESTC_(hr = pIRowsetUpdate->GetOriginalData(rghRows[0], hAccessor,
		pData2), S_OK)

	//Make sure it is the orginal data.
	TESTC(CompareBuffer(pOrgData,pData2,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	//Undo the change made to the 2nd row.
	TESTC_(hr = pIRowsetUpdate->Undo(NULL, 1, rghRows, &cRowsOut,
		&rghRowsOut, &rgRowStatus), S_OK)

	TESTC(cRowsOut==1 && rghRowsOut && rgRowStatus && 
		rgRowStatus[0]==DBROWSTATUS_S_OK)

	TESTC(rghRowsOut[0] != DB_NULL_HROW)

	//Get the data after undoing the change.
	TESTC_(hr = pIRowset->GetData(rghRowsOut[0],hAccessor,pData),S_OK)

	//This should be the same as the original data for 2nd row.
	TESTC(CompareBuffer(pData2,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	PROVIDER_FREE(rghRowsOut);
	PROVIDER_FREE(rgRowStatus);

	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2);
	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);

	//Release the row handle.
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);

	//Get the 3rd row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,0,1,&cRowsObtained,
		&rghRows),S_OK)

	//Delete the 3rd row.
	TESTC_(pIRowsetUpdate->DeleteRows(NULL,1,rghRows,rgRowStatus), S_OK)

	//GetPendingRows should give 1 row with status _DELETED.
	TESTC_(hr = pIRowsetUpdate->GetPendingRows(NULL, pendingStatus,
		&cPendingRows, &rgPendingRows, &rgPendingStatus), S_OK)

	TESTC(cPendingRows==1 && rgPendingRows && rgPendingStatus)
	TESTC(rgPendingStatus[0] == DBPENDINGSTATUS_DELETED)

	//Update the change.
	TESTC_(hr = pIRowsetUpdate->Update(NULL, 1, rghRows, &cRowsOut, 
		&rghRowsOut,&rgRowStatus), S_OK)

	TESTC(cRowsOut==1 && rghRowsOut && rgRowStatus)
	TESTC(rghRowsOut[0] != DB_NULL_HROW)
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK)

	//Try to call GetData on the row which was deleted and updated.
	//This should fail. The expected return code is DB_E_DELETEDROW,
	//although this is provider specific.
	hr = pIRowset->GetData(rghRows[0],hAccessor,pData) ;
	CHECKW(hr, DB_E_DELETEDROW) ;

	cRowsObtained = 0;

	tTestResult = TEST_PASS;

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pOrgData);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_FREE(pData);
	SAFE_FREE(pData2);
	SAFE_FREE(pOrgData);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(rghRowsOut);
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);
	PROVIDER_FREE(rgColumnsOrd);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowsetUpdate);
	SAFE_RELEASE(pIRowset);
	return tTestResult;
} //testIRowsetUpdate


//----------------------------------------------------------------------
//@mfunc Open 2 rowsets on the same table. Fetch the first rows of each 
//rowset and keep the row handles. Call SetData to modify the data of
//the first row of rowset1. Then call IRowsetResynch::ResynchRows on
//rowset2. Now the data of rowset2 should reflect the change made in
//rowset1. Verify this.
//
TESTRESULT CQuickTest::testIRowsetResynch()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBCOUNTITEM			cRowsObtained1 = 0;
	DBCOUNTITEM			cRowsObtained2 = 0;
	DBCOUNTITEM			cRowsOut = 0;
	ULONG_PTR			updtFlag = 0;
	HROW*				rghRows1 = NULL;
	HROW*				rghRows2 = NULL;
	HROW*				rghRowsOut = NULL;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	BYTE*				pData = NULL;
	BYTE*				pData2 = NULL;
	DBROWSTATUS*		rgRowStatus = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowsetUpdate*		pIRowsetUpdate = NULL;
	IRowsetChange*		pIRowsetChange = NULL;
	IRowsetResynch*		pIRowsetResynch = NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
						  DBPROPVAL_UP_DELETE|
						  DBPROPVAL_UP_INSERT  ;
						  
	//Set the following properties.
	SetProperty(DBPROP_IRowsetResynch, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets) ;
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets) ;

	SetSettableProperty(DBPROP_OTHERUPDATEDELETE, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets) ;

	SetSettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Open a rowset with above properties set.
	hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)
		&pIRowset1);

	if(hr==DB_E_ERRORSOCCURRED)
	{
		DumpPropertyErrors(m_cPropSets, m_rgPropSets);
		FreeProperties(&m_cPropSets, &m_rgPropSets);
		TESTC(!pIRowset1)
		return TEST_SKIPPED;
	}

	TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED)

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset1, &updtFlag))
	{
		odtLog<<L"Could not get the value of DBPROP_UPDATABILITY.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	if(!(updtFlag & DBPROPVAL_UP_CHANGE))
	{
		odtLog<<L"CHANGE is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//If IRowsetUpdate exists then fail the test.
	if(VerifyInterface(pIRowset1,IID_IRowsetUpdate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetUpdate))
	{
		odtLog<<L"Requested to be in immediate-update mode, but currently in buffered mode.\n";
		goto CLEANUP;
	}

	//Open another rowset with above properties set.
	TEST2C_(hr = CreateOpenRowset(m_pTable, IID_IRowset,
			(IUnknown**)&pIRowset2), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(VerifyInterface(pIRowset1,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC(VerifyInterface(pIRowset1,IID_IRowsetChange,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetChange))

	TESTC(VerifyInterface(pIRowset2,IID_IRowsetResynch,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetResynch))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds updateable non-index columns. 
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Allocate 2 new data buffers
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	SAFE_ALLOC(pData2, BYTE, cbRowSize)

	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);

	//Restart position for rowset1.
	TESTC_(RestartPosition(pIRowset1), S_OK)

	//Fetch first row of rowset1.
	TESTC_(hr = GetNextRows(pIRowset1, NULL,0,1,&cRowsObtained1,
		&rghRows1),S_OK)

	//Restart position for rowset2. 
	TESTC_(RestartPosition(pIRowset2), S_OK)

	//Fetch first row of rowset2.
	TESTC_(hr = GetNextRows(pIRowset2, NULL,0,1,&cRowsObtained2,
		&rghRows2),S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	//Set data of 1st row of rowset1 to that of row (cTotalRows+1).
	TESTC_(hr = pIRowsetChange->SetData(rghRows1[0],hAccessor,pData),S_OK)

	//Release accessor and bindings.
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);

	TESTC(VerifyInterface(pIRowset2,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds the updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get the backend data for first row of rowset2.
	TESTC_(hr = pIRowsetResynch->GetVisibleData(rghRows2[0], hAccessor,
		pData2), S_OK)
	
	//This should be same as the new data that the first row of rowset1
	//was set to.
	TESTC(CompareBuffer(pData2,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2);
	memset(pData2, 0, (size_t) cbRowSize);

	//Resynch the rows of rowset2.
	TESTC_(hr = pIRowsetResynch->ResynchRows(1, rghRows2, &cRowsOut,
		&rghRowsOut, &rgRowStatus), S_OK)

	TESTC(cRowsOut==1 && rghRowsOut && rgRowStatus)
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK)

	//Get the data for the first row of rowset2.
	TESTC_(hr = pIRowset2->GetData(rghRowsOut[0], hAccessor, pData2),
		S_OK)

	//It should be the same as the data which was set in rowset1.
	TESTC(CompareBuffer(pData2,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2, TRUE);
	CHECK(ReleaseRows(&pIRowset1, &cRowsObtained1, &rghRows1), S_OK);
	CHECK(ReleaseRows(&pIRowset2, &cRowsObtained2, &rghRows2), S_OK);
	PROVIDER_FREE(rghRowsOut);
	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetUpdate);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRowsetResynch);
	return tTestResult;
} //testIRowsetResynch


//----------------------------------------------------------------------
//@mfunc Open a rowset with IRowsetRefresh (in the buffered update mode).
//Fetch the first row of the rowset and keep the row handle. Call
//SetData to modify the data of the first row. Then, without calling
//IRowsetUpdate::Update, call IRowsetRefresh::RefreshVisibleData. Then
//call GetData on the first row. This should give the original data
//corresponding to the first row.
//
TESTRESULT CQuickTest::testIRowsetRefresh()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBCOUNTITEM			cRowsObtained = 0;
	DBCOUNTITEM			cRowsOut = 0;
	ULONG_PTR			updtFlag = 0;
	HROW*				rghRows = NULL;
	HROW*				rghRowsOut = NULL;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	BYTE*				pData = NULL;
	BYTE*				pOrgData = NULL;
	BYTE*				pData2 = NULL;
	DBROWSTATUS*		rgRowStatus = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowsetUpdate*		pIRowsetUpdate = NULL;
	IRowsetRefresh*		pIRowsetRefresh = NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
						  DBPROPVAL_UP_DELETE|
						  DBPROPVAL_UP_INSERT  ;
						  
	//Set the following properties.
	SetProperty(DBPROP_IRowsetRefresh, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets) ;

	SetProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets,
		(void*)VARIANT_TRUE, DBTYPE_BOOL,
		DBPROPOPTIONS_REQUIRED) ;

	SetSettableProperty(DBPROP_OTHERUPDATEDELETE, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets) ;

	SetSettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Open a rowset with above properties set.
	hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)&pIRowset);

	if(hr==DB_E_ERRORSOCCURRED)
	{
		DumpPropertyErrors(m_cPropSets, m_rgPropSets);
		FreeProperties(&m_cPropSets, &m_rgPropSets);
		TESTC(!pIRowset);
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
		
	}

	TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED)

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//Make sure CHANGE is supported.
	if(!(updtFlag & DBPROPVAL_UP_CHANGE))
	{
		odtLog<<L"CHANGE is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC(VerifyInterface(pIRowset,IID_IRowsetUpdate,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetUpdate))

	TESTC(VerifyInterface(pIRowset,IID_IRowsetRefresh,
		ROWSET_INTERFACE,(IUnknown**)&pIRowsetRefresh))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds updateable non-index columns. 
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Allocate 2 new data buffers
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	SAFE_ALLOC(pData2, BYTE, cbRowSize)
	SAFE_ALLOC(pOrgData, BYTE, cbRowSize)

	memset(pData, 0, (size_t) cbRowSize);
	memset(pData2, 0, (size_t) cbRowSize);
	memset(pOrgData, 0, (size_t) cbRowSize);

	//Restart position.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Fetch first row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,0,1,&cRowsObtained,
		&rghRows),S_OK)

	//Get data of the first row.
	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pOrgData),
		S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	//Set data of 1st row.
	TESTC_(hr = pIRowsetUpdate->SetData(rghRows[0],hAccessor,pData),S_OK)

	//Get the backend data for first row.
	TESTC_(hr = pIRowsetRefresh->GetLastVisibleData(rghRows[0], hAccessor,
		pData2), S_OK)
	
	//This should be same as the original data of first row.
	TESTC(CompareBuffer(pData2,pOrgData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2);
	memset(pData2, 0, (size_t) cbRowSize);

	//Resynch the first row.
	TESTC_(hr = pIRowsetRefresh->RefreshVisibleData(NULL, 1, rghRows, 
		TRUE, &cRowsOut, &rghRowsOut, &rgRowStatus), S_OK)

	TESTC(cRowsOut==1 && rghRowsOut && rgRowStatus)
	TESTC(rgRowStatus[0] == DBROWSTATUS_S_OK)

	//Get the data for the first row.
	TESTC_(hr = pIRowset->GetData(rghRowsOut[0], hAccessor, pData2),
		S_OK)

	//It should be the same as the original data of first row.
	TESTC(CompareBuffer(pData2,pOrgData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData2, TRUE);
	ReleaseInputBindingsMemory(cBindings, rgBindings, pOrgData, TRUE);
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	PROVIDER_FREE(rghRowsOut);
	PROVIDER_FREE(rgRowStatus);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetUpdate);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetRefresh);
	return tTestResult;
} //testIRowsetRefresh


//-----------------------------------------------------------------------
//@mfunc Obtain the IAccesssor interface through the ICommand interface.
//Create bindings and then create an accessor with this binding. The 
//length, status and value fields are bound. All columns are bound 
//including BLOB_LONG. Verify the status of the bindings after creating
//the accessor. Then call the method GetBindings and verify that 
//they are the same as the ones used to create the accessor. Use the 
//accessor to fetch rows and verify them. Call  ReleaseAccessor.
//
TESTRESULT CQuickTest::testCmdIAccessor()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRows = 0;
	DBROWCOUNT		cRowsAffected = 0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBBINDSTATUS*	rgStatus = NULL;
	DBLENGTH		cbRowSize = 0;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	IRowset*		pIRowset = NULL;
	ICommand*		pICommand = NULL;

	//Set this property.
	SetSettableProperty(DBPROP_IRowsetLocate,
		DBPROPSET_ROWSET, &cPropSets, &rgPropSets) ;

	//Create SQL stmt and set command text.
	TEST2C_(hr = m_pTable->ExecuteCommand(SELECT_ALLFROMTBL,
		IID_ICommand, NULL, NULL, &cRowsetCols, &rgColumnsOrd, 
		EXECUTE_NEVER, cPropSets, rgPropSets, NULL, 
		NULL, &pICommand), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(pICommand != NULL)

	//Obtain the IAccessor (command) interface.
	if(!VerifyInterface(pICommand,IID_IAccessor,
		COMMAND_INTERFACE,(IUnknown**)&pIAccessor))
		return TEST_FAIL;

	//Create Accessor with a binding using length, status and value.
	//All columns are bound including BLOB_LONG.
	TESTC_(hr = GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG, 
		 &rgStatus),S_OK)

	//Verify status of bindings.
	for(ulIndex=0; ulIndex<cBindings; ulIndex++)
		TESTC(rgStatus[ulIndex] == DBBINDSTATUS_OK)

	//Call the IAccessor::GetBindings method and verify.
	TESTC(VerifyBindings(pIAccessor, hAccessor, cBindings, rgBindings,
		DBACCESSOR_ROWDATA))

	//cRowsAffected is undefined on output.
	TESTC_(hr = pICommand->Execute(NULL, IID_IRowset, NULL, &cRowsAffected,
		(IUnknown**)&pIRowset), S_OK)

	TESTC(pIRowset != NULL)

	//Get number of rows.
	cRows = m_pTable->CountRowsOnTable();

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//For each row in the table, fetch the row, compare the data in the 
	//row and release the row.
	for(ulIndex=0; ulIndex<cRows; ulIndex++)
		TESTC(GetDataAndCompare(0, 1, ulIndex+1, cRowsetCols, 
			rgColumnsOrd, pIRowset, hAccessor, cBindings,
			rgBindings, cbRowSize))

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	PROVIDER_FREE(rgStatus);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommand);
	return tTestResult;
} //testCmdIAccessor


//----------------------------------------------------------------------
//@mfunc Obtain the ICommandProperties interface. Attempt to set three 
//properties. If the method call returns S_OK then verify that all the
//three properties were correctly set. If method returns DB_S_ERRORSOCCURRED
//then verify that the relevant property is not supported or not settable.
//
TESTRESULT CQuickTest::testSetCmdProp()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	USHORT				usSupported = 0;
	ULONG				ulIndex = 0;
	ICommandProperties*	pICommProp = NULL;

	//Obtain the ICommandProperties interface.
	if(!VerifyInterface(m_pICommand,IID_ICommandProperties,
		COMMAND_INTERFACE,(IUnknown**)&pICommProp))
		return TEST_FAIL;

	//Set the following four properties.
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &m_cPropSets,
		&m_rgPropSets);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &m_cPropSets,
		&m_rgPropSets);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &m_cPropSets,
		&m_rgPropSets);

	//Call the SetProperties method for the above properties.
	TEST2C_(hr = pICommProp->SetProperties(m_cPropSets, m_rgPropSets),
		S_OK, DB_S_ERRORSOCCURRED)

	//Loop through all the properties.
	for(ulIndex=0; ulIndex < m_rgPropSets[0].cProperties; ulIndex++)
	{
		//If status shows DBPROPSTATUS_OK then make sure the value 
		//of that property was set to VARIANT_TRUE. the hr in this
		//case has to be either S_OK or DB_S_ERRORSOCCURRED.
		if(m_rgPropSets[0].rgProperties[ulIndex].dwStatus == 
			DBPROPSTATUS_OK)
		{
			COMPARE(GetProperty(m_rgPropSets[0].rgProperties[ulIndex].dwPropertyID,
				m_rgPropSets[0].guidPropertySet, pICommProp), TRUE);

			if(hr==DB_S_ERRORSOCCURRED)
				usSupported++;
			else
				TESTC_(hr, S_OK)
		}

		//If return value was DB_S_ERRORSOCCURRED then.....
		if(hr==DB_S_ERRORSOCCURRED)
		{
			//If status shows DBPROPSTATUS_NOTSUPPORTED then make sure
			//that the property is not supported.
			if(m_rgPropSets[0].rgProperties[ulIndex].dwStatus == 
				DBPROPSTATUS_NOTSUPPORTED)
			{
				COMPARE(SupportedProperty(m_rgPropSets[0].rgProperties[ulIndex].dwPropertyID,
					m_rgPropSets[0].guidPropertySet), FALSE);
			}
			//If the status shows DBPROPSTATUS_NOTSETTABLE then make sure
			//that the property is not settable.
			else if(m_rgPropSets[0].rgProperties[ulIndex].dwStatus == 
				DBPROPSTATUS_NOTSETTABLE)
			{
				COMPARE(SettableProperty(m_rgPropSets[0].rgProperties[ulIndex].dwPropertyID,
					m_rgPropSets[0].guidPropertySet), FALSE);
			}
		}
	}

	//If DB_S_ERRORSOCCURRED was returned, then the number of properties
	//set successfully should be >0 and <cProperties.
	if(hr==DB_S_ERRORSOCCURRED)
		TESTC(usSupported>0 && usSupported<m_rgPropSets[0].cProperties)

	tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(pICommProp);
	return tTestResult;
} //testSetCmdProp


//-----------------------------------------------------------------------
//@mfunc Obtain the ICommandText interface. Create an SQL stmt and set 
//the command text. Then get the command text. Verify that the stmt got
//is the same that was used to set the command text. Create another SQL 
//stmt. Set the command text, prepare it and execute it. Then get the 
//command text and verify. 
//
TESTRESULT CQuickTest::testICmdText()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	WCHAR*			pSQLSet = NULL;
	WCHAR*			pSQLGet = NULL;
	DBROWCOUNT		cRowsAffected=0;
	GUID			guidDialect = DB_NULLGUID;
	ICommandText*	pICT = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;

	//Create a command object and get the ICommand interface.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, 
		(IUnknown**)&pICT), S_OK)

	//Create the following SQL Stmt.
	TESTC(SUCCEEDED(m_pTable->CreateSQLStmt(SELECT_UPDATEABLE, 
		NULL, &pSQLSet, NULL, NULL)))

	//Set the command text to the above SQL stmt.
	TESTC_(pICT->SetCommandText(DBGUID_DEFAULT, pSQLSet), S_OK)

	//Get the command text.
	TEST2C_(hr = pICT->GetCommandText(&guidDialect, &pSQLGet), S_OK,
		DB_S_DIALECTIGNORED)

	//Verify that SQL stmt got is the same that was used to set the
	//command text.
	TESTC(wcscmp(pSQLSet, pSQLGet) == 0)

	//Free command text
	PROVIDER_FREE(pSQLSet);
	PROVIDER_FREE(pSQLGet);
	
	//Create another SQL Stmt. 
	TESTC(SUCCEEDED(m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL,NULL,
											&pSQLSet,NULL,NULL)))

	//Set the command text to the above SQL stmt.									
	TESTC_(pICT->SetCommandText(DBGUID_DEFAULT, pSQLSet), S_OK)

	//If ICommandPrepare is supported, prepare the command.
	//It is not required to prepare the command for Execute.
	if(VerifyInterface(pICT,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		TESTC(pICommandPrepare->Prepare(1) == S_OK)
	}

	//Execute the command. cRowsAffected is undefined on output.
	TESTC_(pICT->Execute(NULL, IID_NULL, NULL, &cRowsAffected, 
		NULL),S_OK)

	//Get the command text after command execution. 
	TEST2C_(hr = pICT->GetCommandText(&guidDialect, &pSQLGet), S_OK,
		DB_S_DIALECTIGNORED)

	//Verify that SQL stmt got is the same that was used to set the
	//command text before execution.
	TESTC(wcscmp(pSQLSet, pSQLGet) == 0)

	tTestResult = TEST_PASS;	

CLEANUP:
	PROVIDER_FREE(pSQLSet);
	PROVIDER_FREE(pSQLGet);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICT);
	return tTestResult;
} //testICmdText


//----------------------------------------------------------------------
//@mfunc Create a command on the session. Call the GetDBSession method
//to obtain the IOpenRowset interface. S_FALSE may be returned by this 
//method if the provider did not have an object that created the command.
//If S_OK is returned, verify the interface obtained. Set the command
//text and execute it asking for the IRowsetInfo interface. Call the
//GetSpecification method on it to obtain the ICommand interface. Verify
//that it should be the same as the first command interface. Call 
//GetDBSession again, this time to get IDBCreateCommand. Verify this
//interface. 
//
TESTRESULT CQuickTest::testICmdGetDBSession()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	ICommand*		pICmd1 = NULL;
	ICommand*		pICmd2 = NULL;
	IOpenRowset*	pIOR = NULL;
	IRowsetInfo*	pIRowsetInfo = NULL;
	IDBCreateCommand*	pIDBCC = NULL;

	//Create a command object and get the ICommand interface.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICmd1), S_OK)
	
	//Call the GetDBSession method.
	TEST2C_(hr = pICmd1->GetDBSession(IID_IOpenRowset, (IUnknown**)&pIOR),
		S_OK, S_FALSE)

	if(hr==S_FALSE)
	{
		COMPARE(pIOR, NULL);
		//Issue a warning.
		odtLog<<L"WARNING: S_FALSE was returned for ICommand::GetDBSession.\n";
		CHECKW(hr, S_OK);
	}

	//Verify the obtained interface.
	if(hr == S_OK)
		TESTC(VerifyEqualInterface(pIOR, m_pIOpenRowset))

	//Set the command text for execution.
	TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL, eSELECT,
		SELECT_ALLFROMTBL, NULL), S_OK)

	//Execute the command and request IRowsetInfo.
	TESTC_(hr=pICmd1->Execute(NULL,IID_IRowsetInfo, NULL, NULL, 
		(IUnknown**)&pIRowsetInfo), S_OK)

	//Call GetSpecification method of IRowsetInfo. 
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_ICommand, (IUnknown**)
		&pICmd2), S_OK, S_FALSE)

	if(hr==S_FALSE)
	{
		COMPARE(pICmd2, NULL);
		//Issue a warning.
		odtLog<<L"WARNING: S_FALSE was returned for IRowsetInfo::GetSpecification.\n";
		CHECKW(hr, S_OK);
	}

	if(hr==S_OK)
	{
		//Verify obtained interface.
		TESTC(VerifyEqualInterface(pICmd1, pICmd2))

		//Call GetDBSession on the obtained ICommand interface, this time
		//requesting IDBCreateCommand.
		m_hr = pICmd2->GetDBSession(IID_IDBCreateCommand, (IUnknown**)
			&pIDBCC) ;

		if(m_hr==S_FALSE)
		{
			//Issue a warning.
			odtLog<<L"WARNING: GetDBSession returned S_FALSE.\n";
			CHECKW(m_hr, S_OK);
		}
		else
			TESTC_(m_hr, S_OK)
		
		//Verify obtained interface.
		TESTC(VerifyEqualInterface(pIDBCC, m_pIDBCreateCommand))
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIDBCC);
	return tTestResult;
} //testICmdGetDBSession


//----------------------------------------------------------------------
//@mfunc Check if the provider is Read-only. Create a command object and
//obtain the ICommand interface. Set the command text to insert and 
//execute the command. Again set the command text to insert and execute
//the command. The set the command text to select all from table and 
//execute this command, asking for IRowset. The previous 2 calls to
//execute did not request any interface (passed in IID_NULL).
//
TESTRESULT CQuickTest::testICmdExec1()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	BOOL			fIsProvReadOnly = FALSE;
	WCHAR*			pwszCmdText = NULL;
	HRESULT			hr = E_FAIL;
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cTotalRows = 0;
	DBROWCOUNT		cRowsAffected=0;
	DBORDINAL		cRowsetCols = 0;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	IAccessor*		pIAccessor = NULL;
	ICommandText*	pICmd1 = NULL;
	IRowset*		pIRowset = NULL;

	//Check if provider is Read-only.
	if(IsProviderReadOnly((IUnknown*)m_pIOpenRowset))
		fIsProvReadOnly = TRUE;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, 
		(IUnknown**)&pICmd1), S_OK)

	if(!fIsProvReadOnly)
	{
		//Set the command text to insert.
		TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL,eINSERT,
			NO_QUERY, NULL,NULL,NULL,NULL,&pwszCmdText), S_OK)

		//Execute the command.
		TEST2C_(hr=pICmd1->Execute(NULL, IID_NULL, NULL, &cRowsAffected, 
			NULL),S_OK, DB_E_NOCOMMAND)

		if(pwszCmdText)
		{
			TESTC_(hr, S_OK)
			//Check affected rows.
			TESTC(cRowsAffected==1 || cRowsAffected==DB_COUNTUNAVAILABLE)
			//Insert adds a row.
			m_pTable->AddRow();
		}
		else
			TESTC_(hr, DB_E_NOCOMMAND)

		SAFE_FREE(pwszCmdText);

		//Set the command text to insert again. The command text has to 
		//be set again to insert a unique row.
		TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL,eINSERT,
			NO_QUERY, NULL,NULL,NULL,NULL,&pwszCmdText), S_OK)

		//Execute the command.
		TEST2C_(hr=pICmd1->Execute(NULL, IID_NULL, NULL, &cRowsAffected, 
			NULL),S_OK, DB_E_NOCOMMAND)

		if(pwszCmdText)
		{
			TESTC_(hr, S_OK)
			//Check affected rows.
			TESTC(cRowsAffected==1 || cRowsAffected==DB_COUNTUNAVAILABLE)
			//Insert adds a row.
			m_pTable->AddRow();
		}
		else
			TESTC_(hr, DB_E_NOCOMMAND)
	}

	//Set command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	//Execute this command.
	TESTC_(hr=pICmd1->Execute(NULL, IID_IRowset, NULL,NULL, (IUnknown**)
		&pIRowset),S_OK)

	//Verify that pIRowset is not NULL.
	TESTC(pIRowset != NULL)

	//Obtain the ICommandProperties interface.
	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	cTotalRows = m_pTable->CountRowsOnTable();

	//Create Accessor with a binding using length, status and value.
	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, NO_BLOB_COLS),
		 S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	//Get data from the rowset and verify.
	for(ulIndex=0; ulIndex<cTotalRows; ulIndex++)
	{
		TESTC(GetDataAndCompare(0,1,ulIndex+1, cRowsetCols, 
			rgColumnsOrd, pIRowset, hAccessor, cBindings, rgBindings,
			cbRowSize))
	}

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(pwszCmdText);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICmd1);
	return tTestResult;
} //testICmdExec1


//----------------------------------------------------------------------
//@mfunc Create a command object. Set the command text to select all 
//from table. Prepare the command (if supported). Execute this command
//thrice. The first time no interface is requested (IID_NULL). The second
//time an IRowset interface is requested. The third time an IRowsetInfo
//is requested. Verify that the 2 interfaces returned belong to different
//rowset objects.
//
TESTRESULT CQuickTest::testICmdExec2()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBCOUNTITEM		cTotalRows = 0;
	DBROWCOUNT		cRowsAffected=0;
	ICommand*		pICmd1 = NULL;
	IRowset*		pIRowset = NULL;
	IRowsetInfo*	pIRowsetInfo = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICmd1), S_OK)

	//Set the command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	//If ICommandPrepare is supported, prepare the command (Not required) 
	if(VerifyInterface(pICmd1,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		TESTC(pICommandPrepare->Prepare(3) == S_OK)
	}

	cTotalRows = m_pTable->CountRowsOnTable();

	//Execute the command, not requesting any interface. cRowsAffected
	//is undefined on output.
	CHECK(hr=pICmd1->Execute(NULL, IID_NULL, NULL, &cRowsAffected, 
		NULL),S_OK);

	//Execute command again, requesting an IRowset interface.
	CHECK(hr=pICmd1->Execute(NULL, IID_IRowset, NULL,NULL, (IUnknown**)
		&pIRowset),S_OK);

	//Execute command again, requesting an IRowsetInfo interface.
	CHECK(hr=pICmd1->Execute(NULL, IID_IRowsetInfo, NULL,NULL, (IUnknown**)
		&pIRowsetInfo),S_OK);

	//Verify that the 2 interfaces returned are not for the same rowset
	//object.
	COMPARE(VerifyEqualInterface(pIRowset, pIRowsetInfo), FALSE);

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICmd1);
	return tTestResult;
} //testICmdExec2


//----------------------------------------------------------------------
//@mfunc Call prepare without having set the command text. This should
//fail. Set the command text to select all from table. Call
//GetColumnInfo before preparing the command. Check for expected return
//code. Then execute the command to open a rowset. Calling Prepare now
//should fail again as the rowset object is open. Now release the rowset,
//prepare the command and then call GetColumnInfo. It should succeed
//this time.
//
TESTRESULT CQuickTest::testCmdPrep()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	DBORDINAL		cColumns = 0;
	WCHAR*			pStringsBuffer = NULL;
    DBCOLUMNINFO*	rgInfo = NULL;
	IRowset*		pIRowset = NULL;
	IColumnsInfo*	pIColumnsInfo = NULL;
	ICommand*			pICmd1 = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICmd1), S_OK)

	if(!VerifyInterface(pICmd1,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		odtLog<<L"ICommandPrepare is not supported.\n";
		SAFE_RELEASE(pICmd1);
		return TEST_SKIPPED;
	}

	//Call prepare without having set the command text.
	TESTC_(hr=pICommandPrepare->Prepare(1), DB_E_NOCOMMAND)

	//Set the command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	TESTC(VerifyInterface(pICmd1,IID_IColumnsInfo,
		COMMAND_INTERFACE,(IUnknown**)&pIColumnsInfo))

    //Call GetColumnInfo without preparing the command.
    TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer)
		,DB_E_NOTPREPARED)

	//Execute the command to open a rowset.
	TESTC_(hr=pICmd1->Execute(NULL, IID_IRowset, NULL,NULL, (IUnknown**)
		&pIRowset),S_OK)

	//Call Prepare with the rowset open.
	TESTC_(hr=pICommandPrepare->Prepare(1), DB_E_OBJECTOPEN)

	//Release the rowset.
	SAFE_RELEASE(pIRowset);

	//Prepare the command.
	TESTC_(hr=pICommandPrepare->Prepare(1), S_OK)

	//Call GetColumnInfo. This should succeed.
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer)
		,S_OK)

	TESTC(cColumns>0 && rgInfo && pStringsBuffer)

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICmd1);
	return tTestResult;
} //testCmdPrep


//----------------------------------------------------------------------
//@mfunc Set the command text to select all from table. Call Unprepare.
//This should succeed. Now prepare the command and execute it to open
//a rowset. Calling Unprepare now would fail due to the rowset being
//open. Now release the rowset and then call Unprepare.
//
TESTRESULT CQuickTest::testCmdUnprep()
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr = S_OK;
	IRowset*		pIRowset = NULL;
	ICommand*			pICmd1 = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICmd1), S_OK)

	if(!VerifyInterface(pICmd1,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		odtLog<<L"ICommandPrepare is not supported.\n";
		SAFE_RELEASE(pICmd1);
		return TEST_SKIPPED;
	}

	//Set the command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICmd1, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	//Calling Unprepare without preparing should succeed.
	TESTC_(hr=pICommandPrepare->Unprepare(), S_OK)

	//Prepare the command.
	TESTC_(hr=pICommandPrepare->Prepare(1), S_OK)

	//Execute the command
	TESTC_(hr=pICmd1->Execute(NULL, IID_IRowset, NULL,NULL, (IUnknown**)
		&pIRowset),S_OK)

	//Unprepare should fail as the rowset is open.
	TESTC_(hr=pICommandPrepare->Unprepare(), DB_E_OBJECTOPEN)

	//Release the rowset.
	SAFE_RELEASE(pIRowset);

	//Now Unprepare should succeed.
	TESTC_(hr=pICommandPrepare->Unprepare(), S_OK)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICmd1);
	return tTestResult;
} //testCmdUnprep


//----------------------------------------------------------------------
//@mfunc Make the parameter bindings to be used in testICmdWParam.
//
BOOL CQuickTest::MakeParamBindings(
	CTable*			pTable,
	DBORDINAL			cColumnsOnTable,
	DBORDINAL*			pcParamColMap,
	LONG_PTR*		rgParamColMap,
	DB_UPARAMS*			pcDbParamBindInfo,
	DBPARAMBINDINFO*	rgDbParamBindInfo,
	DB_UPARAMS*			rgParamOrdinals,
	DBCOLUMNINFO*	rgTableInfo
	)
{
	DBORDINAL	ulIndex;
	BOOL	bSuccess = FALSE;
	DBORDINAL	ulSearchable;
	CCol	TempCol;

	TESTC(pTable && pcParamColMap && rgParamColMap && pcDbParamBindInfo
		&& rgDbParamBindInfo && rgParamOrdinals && rgTableInfo)

	//Make the parameter bindings for all searchable columns in 
	//the table.
	for (ulIndex = 0; ulIndex < cColumnsOnTable; ulIndex++)
	{
		TESTC_(pTable->GetColInfo(ulIndex+1, TempCol), S_OK)

		ulSearchable = TempCol.GetSearchable() ;

		//SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE includes only 
		//those columns in the command text which agree with the
		//following condition.
		if((ulSearchable == DB_LIKE_ONLY ||
			ulSearchable == DB_ALL_EXCEPT_LIKE ||
			ulSearchable == DB_SEARCHABLE ) &&
			(TempCol.GetUpdateable())       &&
			(!TempCol.GetIsLong())
			)
		{
			//Record the column number in the array if it is 
			//searchable & updateable.
			rgParamColMap[*pcParamColMap] = (LONG_PTR) TempCol.GetColNum();				
			(*pcParamColMap)++;

			//Build Information for SetParameterInfo.
			rgParamOrdinals[*pcDbParamBindInfo] = *pcDbParamBindInfo + 1;  
			rgDbParamBindInfo[*pcDbParamBindInfo].pwszDataSourceType = 
				wcsDuplicate(TempCol.GetProviderTypeName());
		
			rgDbParamBindInfo[*pcDbParamBindInfo].pwszName = NULL;
		
			rgDbParamBindInfo[*pcDbParamBindInfo].dwFlags = 
				DBPARAMFLAGS_ISINPUT;
			if (TempCol.GetNullable())
				rgDbParamBindInfo[*pcDbParamBindInfo].dwFlags |= 
				DBPARAMFLAGS_ISNULLABLE;

			rgDbParamBindInfo[*pcDbParamBindInfo].ulParamSize = 
				rgTableInfo[ulIndex].ulColumnSize;
			rgDbParamBindInfo[*pcDbParamBindInfo].bScale = 
				(BYTE)rgTableInfo[ulIndex].bScale;
			rgDbParamBindInfo[*pcDbParamBindInfo].bPrecision = 
				(BYTE)rgTableInfo[ulIndex].bPrecision;
			(*pcDbParamBindInfo)++;
		}
	}

	bSuccess = TRUE;

CLEANUP:
	return bSuccess;
} //MakeParamBindings


//----------------------------------------------------------------------
//@mfunc Test GetParameterInfo for ICommandWithParameters.
//
TESTRESULT CQuickTest::testGetParamInfo(
	ICommandWithParameters* pICWP,
	CTable*					pTable,
	DB_UPARAMS				cDbParamBindInfo,
	DBPARAMBINDINFO*		rgDbParamBindInfo,
	LONG_PTR*				rgParamColMap
	)
{
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr;
	DB_UPARAMS		ulIndex;
	DB_UPARAMS		cParams = 0;
	DBPARAMINFO*	rgLocalParamInfo = NULL;
	WCHAR*			pwszNamesBuffer = NULL;
	CCol			TempCol;

	TESTC(pICWP && pTable && rgDbParamBindInfo && rgParamColMap)

	//Call GetParameterInfo.
	TESTC_(hr = pICWP->GetParameterInfo(&cParams, &rgLocalParamInfo, 
		&pwszNamesBuffer), S_OK)

	TESTC(cParams == cDbParamBindInfo)
	TESTC(rgLocalParamInfo != NULL)

	//We did not give any names to the parameters when we called
	//SetParameterInfo. Hence, the names buffer should be NULL.
	if(pwszNamesBuffer != NULL)
		odtLog<<L"INFO: All parameter names were NULL. Hence, the names buffer should be NULL. Instead of NULL, the names buffer contains "<<pwszNamesBuffer<<L".\n";

	//Check the parameter info obtained by calling GetParameterInfo
	//above, with the paramBindInfo which we had set earlier (before
	//execution).
	for(ulIndex=0; ulIndex<cParams; ulIndex++)
	{
		TESTC((rgLocalParamInfo[ulIndex].dwFlags & DBPARAMFLAGS_ISINPUT)
			== DBPARAMFLAGS_ISINPUT) 

		TESTC(rgLocalParamInfo[ulIndex].iOrdinal == ulIndex+1)
		TESTC(!rgLocalParamInfo[ulIndex].pwszName)

		TESTC_(pTable->GetColInfo(rgParamColMap[ulIndex], TempCol), 
			S_OK)

		COMPAREW(rgLocalParamInfo[ulIndex].pTypeInfo, TempCol.GetTypeInfo());

		if(rgLocalParamInfo[ulIndex].wType != TempCol.GetProviderType())
		{
			odtLog<<L"INFO: wType returned from GetParamInfo was "<<GetDBTypeName(rgLocalParamInfo[ulIndex].wType)<<" instead of "<<GetDBTypeName(TempCol.GetProviderType())<<".\n";
		}
		else
		{
			if(rgLocalParamInfo[ulIndex].ulParamSize != rgDbParamBindInfo[ulIndex].ulParamSize)
				odtLog<<L"INFO: For "<<GetDBTypeName(rgLocalParamInfo[ulIndex].wType)<<" ParamSize returned from GetParamInfo was "<<rgLocalParamInfo[ulIndex].ulParamSize<<" instead of "<<rgDbParamBindInfo[ulIndex].ulParamSize<<".\n";

			if(rgLocalParamInfo[ulIndex].bPrecision != rgDbParamBindInfo[ulIndex].bPrecision)
				odtLog<<L"INFO: For "<<GetDBTypeName(rgLocalParamInfo[ulIndex].wType)<<" Precision returned from GetParamInfo was "<<rgLocalParamInfo[ulIndex].bPrecision<<" instead of "<<rgDbParamBindInfo[ulIndex].bPrecision<<".\n";

			if(rgLocalParamInfo[ulIndex].bScale != rgDbParamBindInfo[ulIndex].bScale)
				odtLog<<L"INFO: For "<<GetDBTypeName(rgLocalParamInfo[ulIndex].wType)<<" Scale returned from GetParamInfo was "<<rgLocalParamInfo[ulIndex].bScale<<" instead of "<<rgDbParamBindInfo[ulIndex].bScale<<".\n";
		}
	}

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(pwszNamesBuffer);
	PROVIDER_FREE(rgLocalParamInfo);
	return tTestResult;
} //testGetParamInfo


//----------------------------------------------------------------------
//@mfunc Make the parameter bindings for all searchable columns in 
//the table. Get the bindings for the Searchable & Updateable columns.
//Set the command text for searching with parameters. Prepare the command
//and call GetParameterInfo. Verify. Then call SetParameterInfo with
//the parameter bindings we made. Make parameter data and execute the 
//command with these parameters. Verify the obtained rowset. Now call
//GetParameterInfo again. Compare this with the paramBindInfo which we 
//had used for SetParameterInfo.
//
TESTRESULT CQuickTest::testICmdWParam()
{
	TESTRESULT				tTestResult = TEST_FAIL;
	HRESULT					hr = S_OK;
	HRESULT					hrGetParam = S_OK;
	ULONG_PTR				ulIndex = 0;
	DBORDINAL				cColumnsOnTable = 0;
	LONG_PTR*				rgParamColMap = NULL;
	DB_UPARAMS*				rgParamOrdinals = NULL;
	DBPARAMBINDINFO*		rgDbParamBindInfo = NULL;
	DB_UPARAMS				cParams = 0;
	DBPARAMINFO*			rgLocalParamInfo = NULL;
	DBORDINAL				cParamColMap = 0;
	DB_UPARAMS				cDbParamBindInfo = 0;
	DBORDINAL				cTableInfo = 0;
	DBCOLUMNINFO*			rgTableInfo = NULL;
	LPWSTR					pTableStringsBuffer = NULL;
	DBLENGTH				cbRowSize = 0;
	DBCOUNTITEM				cBindings = 0;
	DBBINDING*				rgBindings = NULL;
	HACCESSOR				hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS				dbParams;
	BYTE*					pData = NULL;
	DBROWCOUNT				cRowsAffected = 0;
	WCHAR*					pwszSqlStmt = NULL;
	DBCOUNTITEM				cRowsObtained = 0;
	HROW*					rghRows = NULL;
	CTable*					pTable = NULL;
	ICommandPrepare*		pICommandPrepare = NULL;
	ICommandWithParameters*	pICWP = NULL;
	ICommand*				pICommand = NULL;
	IAccessor*				pIAccessor = NULL;
	IRowset*				pIRowset = NULL;
	IColumnsInfo*			pIColumnsInfo = NULL;

	//Create a new command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICommand), S_OK)

	//Obtain pointer to ICommandWithParameters, if it is supported.
	if(!VerifyInterface(pICommand,IID_ICommandWithParameters,
		COMMAND_INTERFACE,(IUnknown**)&pICWP))
	{
		odtLog<<L"ICommandWithParameters is not supported.\n";
		SAFE_RELEASE(pICommand);
		return TEST_SKIPPED;
	}

	//Get pointer to IAccessor (command accessor).
	TESTC(VerifyInterface(pICommand,IID_IAccessor,
		COMMAND_INTERFACE,(IUnknown**)&pIAccessor))

	//Get pointer to ICommandPrepare, if it is supported.
	if(!VerifyInterface(pICommand,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
		pICommandPrepare = NULL;

	//Create a new table.
	pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName, NONULLS);
	TESTC(pTable != NULL)
	TESTC_(pTable->CreateTable(MIN_ROWS), S_OK)

	//Set command text to "select * from [table]"
	TESTC_(pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IUnknown, 
		NULL, NULL, NULL, NULL,  EXECUTE_NEVER, 0, NULL, NULL, NULL, 
		&pICommand), S_OK)

	//Prepare the command, if ICommandPrepare is supported. Preparing 
	//the command is required for calling GetColumnInfo.
	if(pICommandPrepare)
	{
		TESTC_(pICommandPrepare->Prepare(1), S_OK)
	}

	//Get pointer to IColumnInfo.
	TESTC(VerifyInterface(pICommand,IID_IColumnsInfo,
		COMMAND_INTERFACE,(IUnknown**)&pIColumnsInfo))

	//Get the column info (cmd text set to select all from table).
	TESTC_(pIColumnsInfo->GetColumnInfo(&cTableInfo, 
		&rgTableInfo, &pTableStringsBuffer), S_OK);

	SAFE_RELEASE(pIColumnsInfo);

	cColumnsOnTable = pTable->CountColumnsOnTable();

	SAFE_ALLOC(rgParamColMap, LONG_PTR, cColumnsOnTable)
	SAFE_ALLOC(rgParamOrdinals, DB_UPARAMS, cColumnsOnTable)
	SAFE_ALLOC(rgDbParamBindInfo, DBPARAMBINDINFO, cColumnsOnTable)

	memset(rgDbParamBindInfo, 0, (size_t) (cColumnsOnTable*sizeof(DBPARAMBINDINFO)));
	memset(rgParamColMap, 0, (size_t) (cColumnsOnTable*sizeof(LONG_PTR)));
	memset(rgParamOrdinals, 0, (size_t) (cColumnsOnTable*sizeof(DB_UPARAMS)));

	//Make the parameter bindings.
	TESTC(MakeParamBindings(pTable, cColumnsOnTable, &cParamColMap,
		rgParamColMap, &cDbParamBindInfo, rgDbParamBindInfo,
		rgParamOrdinals, rgTableInfo))
	
	//Get the Bindings for the Searchable & Updateable columns by
	//passing an array containing the column ordinals for searchable 
	//& updateable columns only. This is a command acessor.
	TESTC_(hr = GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_PARAMETERDATA, &hAccessor, &rgBindings, &cBindings, 
		&cbRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, 
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,  NULL, 
		NULL, NULL, DBTYPE_EMPTY, cParamColMap, 
		rgParamColMap, rgParamOrdinals, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, BLOB_LONG ), S_OK)

	// Set the command text to a statement like "select col2,col5,... 
	// from [table] where col2=?,col5=?,...".
	TESTC_(hr = pTable->ExecuteCommand(
		SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE, IID_IUnknown, NULL, 
		&pwszSqlStmt, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, 
		&pICommand), S_OK)

	//Make sure the command text was built.
	TESTC(pwszSqlStmt != NULL)
	
	//Prepare the command. Required for GetParameterInfo.
	if(pICommandPrepare)
	{
		TESTC_(pICommandPrepare->Prepare(2), S_OK)
	}

	//GetParameterInfo will return DB_E_PARAMUNAVAILABLE if provider 
	//cannot derive parameter information from the command. Otherwise,
	//it returns S_OK.
	TEST2C_(hrGetParam = pICWP->GetParameterInfo(&cParams, &rgLocalParamInfo, 
		NULL), S_OK, DB_E_PARAMUNAVAILABLE)

	if (hrGetParam == DB_E_PARAMUNAVAILABLE)
	{
		TESTC(cParams==0 && rgLocalParamInfo==NULL)
		odtLog<<L"INFO: Provider cannot derive parameter information from the command (SetParameterInfo has not been called).\n";
	}
	else
	{
		TESTC(cParams==cParamColMap && rgLocalParamInfo)
		PROVIDER_FREE(rgLocalParamInfo);
	}

	//Set the parameter info. If the Provider can derive parameter 
	//information from the command (i.e., if the above call to
	//GetParameterInfo returned S_OK), then SetParameterInfo will
	//return DB_S_TYPEINFOOVERRIDDEN.
	TEST2C_(hr = pICWP->SetParameterInfo(cDbParamBindInfo, rgParamOrdinals, 
		rgDbParamBindInfo), S_OK, DB_S_TYPEINFOOVERRIDDEN)

	if(hrGetParam == DB_E_PARAMUNAVAILABLE)
		CHECK(hr, S_OK);
	if(hrGetParam == S_OK)
		CHECK(hr, DB_S_TYPEINFOOVERRIDDEN);

	//Make the parameter data (corresponding to row 1).
	TESTC_(hr = FillInputBindings(pTable, DBACCESSOR_PARAMETERDATA, 
		cBindings, rgBindings, &pData, 1, cParamColMap, rgParamColMap), 
		S_OK)

	//Set the DBPARAMS structure to contain the parameter data "pData".
	dbParams.pData = pData;
	dbParams.cParamSets = 1;
	dbParams.hAccessor = hAccessor;

	//Execute command with parameters.
	hr = pICommand->Execute(NULL, IID_IRowset, &dbParams, &cRowsAffected,
		(IUnknown**)&pIRowset) ;

	if(FAILED(hr))
	{
		odtLog<<L"WARNING: Failure in executing command with parameters. The syntax used may not supported by the provider.\n";
		CHECKW(hr, S_OK);
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC_(hr, S_OK)

	TESTC(pIRowset != NULL)

	//Release pData, accessor and bindings. These will be reused.
	ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	pData = NULL;
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);

	//Obtain IAccessor on the rowset.
	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create an accessor and bindings, with all columns of the rowset
	//bound. This is a rowset accessor.
	TESTC_(hr = GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA, &hAccessor, &rgBindings, &cBindings, 
		&cbRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, 
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,  NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL,  NULL ,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG ), S_OK)

	//Allocate pData.
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData,0, (size_t) cbRowSize);

	//Position cursor at beginning of rowset.
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Fetch 3 rows. The rowset, however, should have only 1 row,
	//because the parameter data was built to match the data for
	//1 row only (the first row of the base table).
	TEST2C_(hr = GetNextRows(pIRowset, NULL, 0,3, &cRowsObtained, 
		&rghRows), DB_S_ENDOFROWSET, DB_S_ROWLIMITEXCEEDED)

	TESTC(cRowsObtained == 1)

	//Get data of the first row.
	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

	//The data has to match the data of the first row of base table.
	TESTC(CompareData(cParamColMap, rgParamColMap, 1, pData,
		cBindings, rgBindings, pTable, m_pIMalloc))

	//Now test GetParameterInfo.
	TESTC(testGetParamInfo(pICWP, pTable, cDbParamBindInfo,
		rgDbParamBindInfo, rgParamColMap) == TEST_PASS)

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	for(ulIndex=0; ulIndex<cDbParamBindInfo; ulIndex++)
	{
		PROVIDER_FREE(rgDbParamBindInfo[ulIndex].pwszName);
		PROVIDER_FREE(rgDbParamBindInfo[ulIndex].pwszDataSourceType);
	}
	PROVIDER_FREE(rgTableInfo);
	PROVIDER_FREE(pTableStringsBuffer);
	PROVIDER_FREE(rgParamColMap);
	PROVIDER_FREE(rgParamOrdinals);
	PROVIDER_FREE(rgDbParamBindInfo) ;
	PROVIDER_FREE(pwszSqlStmt);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICWP);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsInfo);
	if(pTable)
		pTable->DropTable();
	SAFE_DELETE(pTable);
	return tTestResult;
} //testICmdWParam


//---------------------------------------------------------------------
//@mfunc Make a sql select stmt with list of updateable columns.
//Set the command text to this statement.
//Execute command and request IMultipleResults. Get the first 
//rowset and verify. Release this rowset. Make sure there was only 1 
//rowset in the results.
//
TESTRESULT CQuickTest::testIMultipleResults()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBCOUNTITEM			ulIndex = 0;
	DBCOUNTITEM			cRows = 0;
	DBORDINAL			cColumns = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	DBROWCOUNT			cRowsAffected = 0;
	WCHAR*				pwszTableName = NULL;
	WCHAR*				pwszSelectUpdt = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	DBBINDING*			rgBindings = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	IAccessor*			pIAccessor = NULL;
	IRowset*			pIRowset1 = NULL;
	IRowset*			pIRowset2 = NULL;
	ICommandText*		pICmdText = NULL;
	IMultipleResults*	pIMR = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, 
		(IUnknown**)&pICmdText), S_OK)

	TESTC(m_pTable != NULL)

	pwszTableName = m_pTable->GetTableName();

	//Get number of rows.
	cRows = m_pTable->CountRowsOnTable();

	//Make a sql select stmt with list of updateable columns.
	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_UPDATEABLE, pwszTableName,
		&pwszSelectUpdt, &cColumns, &rgColumnsOrd), S_OK)

	TESTC(pwszSelectUpdt != NULL)

	//Set the command text to the multiple stmt.
	TESTC_(hr = pICmdText->SetCommandText(DBGUID_DEFAULT, pwszSelectUpdt),
		S_OK)

	//Execute command and request IMultipltResults.
	TESTC_(hr = pICmdText->Execute(NULL, IID_IMultipleResults, NULL, 
		&cRowsAffected, (IUnknown**)&pIMR), S_OK)

	//Get the first rowset and verify. cRowsAffected is undefined
	//on output.
	TESTC_(hr = pIMR->GetResult(NULL, 0, IID_IRowset, &cRowsAffected,
		(IUnknown**)&pIRowset1), S_OK)

	TESTC(VerifyInterface(pIRowset1,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	TESTC_(hr=GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Make sure we are on the first row.
	TESTC_(RestartPosition(pIRowset1), S_OK)

	//For each row in the table, fetch the row, compare the data in the 
	//row and release the row.
	for(ulIndex=0; ulIndex<cRows; ulIndex++)
		TESTC(GetDataAndCompare(0, 1, ulIndex+1, cColumns, 
			rgColumnsOrd, pIRowset1, hAccessor, cBindings,
			rgBindings, cbRowSize, m_pTable))

	if(pIAccessor && hAccessor)
		CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);
	hAccessor = DB_NULL_HACCESSOR;
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset1);

	//Calling GetResult a second time should return DB_S_NORESULT,
	//as there was only 1 sql stmt in the command text.
	TESTC_(hr = pIMR->GetResult(NULL, 0, IID_IRowset, &cRowsAffected,
		(IUnknown**)&pIRowset2), DB_S_NORESULT)

	TESTC(pIRowset2 == NULL)

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(pwszSelectUpdt);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIMR);
	SAFE_RELEASE(pICmdText);
	return tTestResult;
} //testIMultipleResults


//----------------------------------------------------------------------
//@mfunc Check if IDBCreateSession has error support. Call CreateSession
//with an invalid argument. Get the error records and get an IErrorInfo 
//for every record. Loop through each record and verify the various
//information returned by the methods of IErrorInfo.
//
TESTRESULT	CQuickTest::testIError()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG				ulIndex = 0;
	LCID				lcid ;
	GUID				guid ;
	DWORD				dwHelpContext = 0;
	ULONG				cRecords = 0;
	BSTR				bstrDescription;
	BSTR				bstrHelpFile;
	BSTR				bstrSource ;
	WCHAR*				wszString = NULL ;
	IErrorInfo**		rgIErrorInfo = NULL;
	IErrorInfo*			pIEI = NULL;
	IErrorRecords*		pIER = NULL;
	IErrorLookup*		pIEL = NULL;
	ISupportErrorInfo*	pISEI = NULL;
	IDBCreateSession*	pIDBCS = NULL;

	//Get ISupportErrorInfo.
	if (!VerifyInterface(m_pIDBInitialize, IID_ISupportErrorInfo, 
		DATASOURCE_INTERFACE, (IUnknown **)&pISEI))
	{
		odtLog << L"Interface ISupportErrorInfo is not supported.\n";
		return TEST_SKIPPED;
	}
	
	//Check if IDBCreateSession has error support.
	hr = pISEI->InterfaceSupportsErrorInfo(IID_IDBCreateSession);
	
	if (hr == S_FALSE)
	{
		SAFE_RELEASE(pISEI);
		odtLog<<L"IDBCreateSession does not have error support.\n";
		return TEST_SKIPPED;
	}

	TESTC_(hr, S_OK)

	TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
		DATASOURCE_INTERFACE, (IUnknown **)&pIDBCS)) 

	//Call CreateSession with an invalid argument.
	hr = pIDBCS->CreateSession(NULL, IID_IOpenRowset, NULL);

	TESTC_(hr, E_INVALIDARG)

	//Validate the correct behavior for the given error situation.
	TESTC(XCHECK((IUnknown*)pIDBCS, IID_IDBCreateSession, hr))

	hr = pIDBCS->CreateSession(NULL, IID_IOpenRowset, NULL);

	TESTC_(hr, E_INVALIDARG)

	//Get the IErrorInfo interface.
	hr = GetErrorInfo(0, &pIEI) ;
	if(hr == S_FALSE)
	{
		odtLog<<L"ErrorInfo is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC(hr==S_OK && (pIEI))

	//Get IErrorRecords interface.
	TESTC(VerifyInterface(pIEI, IID_IErrorRecords, ERROR_INTERFACE,
		(IUnknown **)&pIER))

	//get the number of error records.
	if(!CHECK(pIER->GetRecordCount(&cRecords), S_OK))
		goto CLEANUP;

	//There has to be at least one error record.
	if (cRecords == 0)
	{
		odtLog << L"No error records were found in the error object.\n";
		goto CLEANUP;
	}

	//Now get an IErrorInfo for every record
	rgIErrorInfo = (IErrorInfo **)PROVIDER_ALLOC(sizeof(IErrorInfo *) * cRecords);
	TESTC(rgIErrorInfo != NULL)

	lcid = GetSystemDefaultLCID();
	
	//For each record...
	for (ulIndex=0; ulIndex<cRecords; ulIndex++)
	{
		//Place each interface in an element of our array
		TESTC_(pIER->GetErrorInfo(ulIndex, lcid, 
			(IErrorInfo**)&rgIErrorInfo[ulIndex]), S_OK)
	}

	//Loop through each error record.
	for (ulIndex = cRecords; ulIndex > 0; ulIndex--)
	{
		//GetDescription.
		if(CHECK(hr = ((IErrorInfo *)rgIErrorInfo[ulIndex-1])->GetDescription(
			&bstrDescription), S_OK))
		{
			if (wszString = BSTR2WSTR(bstrDescription, TRUE))		
			{
				TESTC(wcslen(wszString) > 0) 
				odtLog << L"INFO: Description for record " << ulIndex-1 << L":" 
					<< wszString << L"\n" ;
				PROVIDER_FREE(wszString);
			}
		}
		
		//GetGUID.
		if(CHECK(hr = ((IErrorInfo *)rgIErrorInfo[ulIndex-1])->GetGUID(
			&guid), S_OK))
		{
			//Make sure the right interface was returned
			TESTC(guid == IID_IDBCreateSession)
		}

		//GetHelpContext
		CHECK(hr = ((IErrorInfo *)rgIErrorInfo[ulIndex-1])->GetHelpContext(
			&dwHelpContext), S_OK) ;

		//GetHelpFile
		if(CHECK(hr = ((IErrorInfo *)rgIErrorInfo[ulIndex-1])->GetHelpFile(
			&bstrHelpFile), S_OK))
		{
			if (wszString = BSTR2WSTR(bstrHelpFile, TRUE))		
			{
				odtLog << L"INFO: Help File for record " << ulIndex-1 << L":" 
					<< wszString << L"\n";
				odtLog << L"INFO: Help Context = "<<dwHelpContext<<L".\n";
				PROVIDER_FREE(wszString);
			}
		}

		//GetSource
		if (CHECK(hr = ((IErrorInfo *)rgIErrorInfo[ulIndex-1])->GetSource(
			&bstrSource), S_OK))
		{
			if (wszString = BSTR2WSTR(bstrSource, TRUE))		
			{
				TESTC(wcslen(wszString) > 0)
				odtLog << L"INFO: Source for record " << ulIndex-1 << L":" 
					<< wszString << L"\n";
				PROVIDER_FREE(wszString);
			}
		}

	} //For-Loop Ends.

	tTestResult = TEST_PASS;

CLEANUP:
	if (rgIErrorInfo)
	{
		//Release each records IErrorInfo interface
		for (ulIndex=0; ulIndex<cRecords; ulIndex++)
			SAFE_RELEASE((IErrorInfo*)rgIErrorInfo[ulIndex]);

		//Delete array of interface pointers
		PROVIDER_FREE(rgIErrorInfo);
	}
	PROVIDER_FREE(wszString);
	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pISEI);
	SAFE_RELEASE(pIEL);
	SAFE_RELEASE(pIER);
	SAFE_RELEASE(pIEI);
	return tTestResult;
} //testIError


//----------------------------------------------------------------------
//@mfunc Start a transaction. Open a rowset with updateable properties.
//Get the third row and set its data to some new data. Now commit the
//transaction (fPreserve=FALSE). If DBPROP_COMMITPRESERVE is FALSE, then
//release the rowset and open another rowset. Verify that the third row
//has the new data.
//
TESTRESULT	CQuickTest::testTransactionCommit()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG_PTR			updtFlag = 0;
	BOOL				bProvUpdateable = TRUE;
	BOOL				bCommitPreserve = FALSE;
	ULONG				ulTransLevel = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	HROW*				rghRows = NULL;
	BYTE*				pData = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowsetChange*		pIRowsetChange = NULL;
	ITransactionLocal*	pITL = NULL;

	//Get ITransactionLocal.
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, 
		SESSION_INTERFACE, (IUnknown **)&pITL))
	{
		odtLog << L"ITransactionLocal is not supported.\n";
		return TEST_SKIPPED;
	}

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
						  DBPROPVAL_UP_DELETE|
						  DBPROPVAL_UP_INSERT  ;

	//Set the following properties.
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets,
		(void*)VARIANT_TRUE, DBTYPE_BOOL,
		DBPROPOPTIONS_OPTIONAL) ;

	SetSettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Start a transaction.
	TESTC_(hr = pITL->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,
		0, NULL, &ulTransLevel), S_OK)

	//Open a rowset with above properties set.
	TEST2C_(hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)
		&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		bProvUpdateable = FALSE;
	}
	else
	{
		if(!(updtFlag & DBPROPVAL_UP_CHANGE))
		{
			odtLog<<L"CHANGE is not supported.\n";
			bProvUpdateable = FALSE;
		}
	}

	if(!GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset))
	{
		odtLog<<L"IRowsetChange is not supported.\n";
		bProvUpdateable = FALSE;
	}

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	if(bProvUpdateable)
	{
		TESTC(VerifyInterface(pIRowset,IID_IRowsetChange,
			ROWSET_INTERFACE,(IUnknown**)&pIRowsetChange))
	}

	//Create Accessor with a binding using length, status and value.
	//This accessor binds the updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get the value of the property DBPROP_COMMITPRESERVE.
	if(GetProperty(DBPROP_COMMITPRESERVE, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset))
		bCommitPreserve = TRUE;

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	//Restart position. 
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get third row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,2,1,&cRowsObtained,
		&rghRows),S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	if(bProvUpdateable)
	{
		//Set data corresponding to row (g_ulNextRow+1).
		TESTC_(hr = pIRowsetChange->SetData(rghRows[0],hAccessor,
			pData),S_OK)
	}

	//Release...cleanup...
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	pData = NULL;
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetChange);

	//pIRowset has not been freed.

	//Commit the transaction with fRetaining=FALSE.
	//FUTURE ATTENTION required here.
	if((hr=pITL->Commit(FALSE, XACTTC_SYNC, 0)) == XACT_E_NOTSUPPORTED)
		if((hr=pITL->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0)) == XACT_E_NOTSUPPORTED)
			if((hr=pITL->Commit(FALSE, XACTTC_ASYNC_PHASEONE, 0)) == XACT_E_NOTSUPPORTED)
				if((hr=pITL->Commit(FALSE, XACTTC_SYNC_PHASEONE, 0)) == XACT_E_NOTSUPPORTED)
					if((hr=pITL->Commit(FALSE, 0, 0)) == XACT_E_NOTSUPPORTED)
					{
						odtLog<<L"None of the XACTTC commit flag values were supported.\n";
						goto CLEANUP;
					}

	//The Commit should have succeeded.
	TESTC_(hr, S_OK)

	//If DBPROP_COMMITPRESERVE was FALSE, then release pIRowset and 
	//open a rowset again. Otherwise, the current rowset is valid, so
	//proceed.
	if(!bCommitPreserve)
	{
		//Verify the rowset is in zombie state.
		TESTC_(RestartPosition(pIRowset), E_UNEXPECTED)

		SAFE_RELEASE(pIRowset);
		TEST2C_(hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)
			&pIRowset), S_OK, DB_S_ERRORSOCCURRED)
	}

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds the updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	//Restart position. 
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get third row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,2,1,&cRowsObtained,
		&rghRows),S_OK)

	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

	if(bProvUpdateable)
	{
		//The first row should contain the new data which was set in
		//the transaction before committing.
		TESTC(CompareData(cRowsetCols, rgColumnsOrd, g_ulNextRow, 
			pData, cBindings, rgBindings, m_pTable, m_pIMalloc, PRIMARY, 
			COMPARE_ONLY))
	}

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	if(pData)
		ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);
	if((hr=pITL->Abort(NULL, FALSE, FALSE)) == XACT_E_NOTSUPPORTED)
		hr=pITL->Abort(NULL, FALSE, TRUE) ;
	SAFE_RELEASE(pITL);
	return tTestResult;
} //testTransactionCommit


//----------------------------------------------------------------------
//@mfunc Start a transaction. Open a rowset with updateable properties.
//Get the third row and set its data to some new data. Now abort the
//transaction (fPreserve=FALSE). If DBPROP_ABORTPRESERVE is FALSE, then
//release the rowset and open another rowset. Verify that the third row
//has the old data.
//
TESTRESULT	CQuickTest::testTransactionAbort()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	ULONG_PTR			updtFlag = 0;
	BOOL				bProvUpdateable = TRUE;
	BOOL				bAbortPreserve = FALSE;
	ULONG				ulTransLevel = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	HROW*				rghRows = NULL;
	BYTE*				pData = NULL;
	BYTE*				pOrgData = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	IRowsetChange*		pIRowsetChange = NULL;
	ITransactionLocal*	pITL = NULL;

	//Get ITransactionLocal.
	if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionLocal, 
		SESSION_INTERFACE, (IUnknown **)&pITL))
	{
		odtLog << L"ITransactionLocal is not supported.\n";
		return TEST_SKIPPED;
	}

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
						  DBPROPVAL_UP_DELETE|
						  DBPROPVAL_UP_INSERT  ;

	//Set the following properties.
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets,
		(void*)VARIANT_TRUE, DBTYPE_BOOL,
		DBPROPOPTIONS_OPTIONAL) ;

	SetSettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		&m_cPropSets, &m_rgPropSets,
		(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Start a transaction.
	TESTC_(hr = pITL->StartTransaction(ISOLATIONLEVEL_READCOMMITTED,
		0, NULL, &ulTransLevel), S_OK)

	//Open a rowset with above properties set.
	TEST2C_(hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)
		&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	//Get value of the DBPROP_UPDATABILITY property.
	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset, &updtFlag))
	{
		odtLog<<L"INFO: Could not get the value of DBPROP_UPDATABILITY.\n";
		bProvUpdateable = FALSE;
	}
	else 
	{
		if(!(updtFlag & DBPROPVAL_UP_CHANGE))
		{
			odtLog<<L"CHANGE is not supported.\n";
			bProvUpdateable = FALSE;
		}
	}

	if(!GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset))
	{
		odtLog<<L"IRowsetChange is not supported.\n";
		bProvUpdateable = FALSE;
	}

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	if(bProvUpdateable)
	{
		TESTC(VerifyInterface(pIRowset,IID_IRowsetChange,
			ROWSET_INTERFACE,(IUnknown**)&pIRowsetChange))
	}

	//Create Accessor with a binding using length, status and value.
	//This accessor binds the updateable non-index columns.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Get the value of the property DBPROP_ABORTPRESERVE.
	if(GetProperty(DBPROP_ABORTPRESERVE, DBPROPSET_ROWSET, (IUnknown*)
		pIRowset))
		bAbortPreserve = TRUE;

	//Allocate a new data buffer
	SAFE_ALLOC(pOrgData, BYTE, cbRowSize)
	memset(pOrgData, 0, (size_t) cbRowSize);

	//Restart position. 
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get third row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,2,1,&cRowsObtained,
		&rghRows),S_OK)

	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pOrgData), S_OK)

	//Make data corresponding to row (g_ulNextRow+1).
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_ROWDATA,
		cBindings, rgBindings, &pData, ++g_ulNextRow, cRowsetCols,
		rgColumnsOrd, PRIMARY),S_OK)

	if(bProvUpdateable)
	{
		//Set data corresponding to row (g_ulNextRow+1).
		TESTC_(hr = pIRowsetChange->SetData(rghRows[0],hAccessor,
			pData),S_OK)
	}

	//Release...cleanup...
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	pData = NULL;
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetChange);

	//Abort the transaction with fRetaining=FALSE.
	if((hr=pITL->Abort(NULL, FALSE, FALSE)) == XACT_E_NOTSUPPORTED)
		if((hr=pITL->Abort(NULL, FALSE, TRUE)) == XACT_E_NOTSUPPORTED)
		{
			odtLog<<L"INFO: Failure when calling Abort.\n";
			goto CLEANUP;
		}

	//The transaction should have aborted successfully.
	TESTC_(hr, S_OK)

	//If DBPROP_ABORTPRESERVE was FALSE, then release pIRowset and 
	//open a rowset again. Otherwise, the current rowset is valid, so
	//proceed.
	if(!bAbortPreserve)
	{
		//Verify the rowset is in zombie state.
		TESTC_(RestartPosition(pIRowset), E_UNEXPECTED)

		SAFE_RELEASE(pIRowset);
		TEST2C_(hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)
			&pIRowset), S_OK, DB_S_ERRORSOCCURRED)
	}

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds the updateable non-index columns.
	TESTC_(hr = GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_NONINDEX_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	//Restart position. 
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get third row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,2,1,&cRowsObtained,
		&rghRows),S_OK)

	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

	//The data should correspond to the original data of first row.
	TESTC(CompareBuffer(pOrgData,pData,cBindings,rgBindings,
		m_pIMalloc,TRUE, FALSE, COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	if(pData)
		ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	if(pOrgData)
		ReleaseInputBindingsMemory(cBindings, rgBindings,pOrgData, TRUE);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);
	if((hr=pITL->Abort(NULL, FALSE, FALSE)) == XACT_E_NOTSUPPORTED)
		hr=pITL->Abort(NULL, FALSE, TRUE) ;
	SAFE_RELEASE(pITL);
	return tTestResult;
} //testTransactionAbort


//----------------------------------------------------------------------
//@mfunc Create an accessor binding all columns. Look for a binding of
//type DBTYPE_IUNKNOWN. Fetch the first row and get its data. Compare 
//the data for the DBTYPE_IUNKNOWN binding. CompareData gets the 
//ISequentialStream interface and calls read. It then verifies the data 
//read through ISequentialStream.
//
TESTRESULT CQuickTest::testStorageObj()
{
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBCOUNTITEM			ulBinding = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBORDINAL			cRowsetCols = 0;
	DB_LORDINAL*		rgColumnsOrd = NULL;
	HROW*				rghRows = NULL;
	BYTE*				pData = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	IRowset*			pIRowset = NULL;
	IAccessor*			pIAccessor = NULL;
	ISequentialStream*	pISS = NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	//Set the following properties. IRowsetLocate is required if
	//we have bound BLOB columns and are going to fetch the data.
	SetSupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET,
		&m_cPropSets, &m_rgPropSets) ;

	SetProperty(DBPROP_CANHOLDROWS,
		DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

	//Open a rowset with above properties set.
	hr=CreateOpenRowset(m_pTable, IID_IRowset,(IUnknown**)&pIRowset);

	if(hr == DB_E_ERRORSOCCURRED)
	{
		TESTC(!pIRowset)
		DumpPropertyErrors(m_cPropSets, m_rgPropSets);
		FreeProperties(&m_cPropSets, &m_rgPropSets);
		TESTC(!pIRowset)
		return TEST_SKIPPED;
	}

	TESTC_(hr, S_OK)

	//Get list of ordinals.
	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cRowsetCols, 
		&rgColumnsOrd, NULL, NULL, NULL, NULL))

	TESTC(VerifyInterface(pIRowset,IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	//Create Accessor with a binding using length, status and value.
	//This accessor binds all columns. Include BLOB_IID_ISEQSTREAM
	//and BLOB_BIND_STR.
	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA,&hAccessor, &rgBindings,
		&cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		BLOB_IID_ISEQSTREAM|BLOB_BIND_STR, NULL),S_OK)

	//There should be a binding of type DBTYPE_IUNKNOWN for this test.
	if(!FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding))
	{
		odtLog<<L"No column of type DBTYPE_IUNKNOWN in the rowset.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	//Restart position. 
	TESTC_(RestartPosition(pIRowset), S_OK)

	//Get first row.
	TESTC_(hr = GetNextRows(pIRowset, NULL,0,1,&cRowsObtained,
		&rghRows),S_OK)

	//Get data for first row.
	TESTC_(hr = pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

	TESTC_(hr = GetStorageData(cBindings, rgBindings, pData, NULL, 
		NULL, IID_ISequentialStream, (IUnknown**)&pISS), S_OK)

	TESTC(pISS != NULL)

	//Compare the data for the DBTYPE_IUNKNOWN binding. This function
	//gets the ISequentialStream interface and calls read. It then
	//verifies the data read through ISequentialStream.
	TESTC(CompareData(cRowsetCols, rgColumnsOrd, 1, pData, 1,
		&rgBindings[ulBinding], m_pTable, m_pIMalloc, PRIMARY, 
		COMPARE_ONLY))

	tTestResult = TEST_PASS;

CLEANUP:
	CHECK(ReleaseRows(&pIRowset, &cRowsObtained, &rghRows), S_OK);
	if(pData)
		ReleaseInputBindingsMemory(cBindings, rgBindings,pData, TRUE) ;
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	PROVIDER_FREE(rgColumnsOrd);
	FreeAccessorAndBindings(&pIAccessor,&hAccessor,&cBindings,&rgBindings);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIRowset);
	return tTestResult;
} //testStorageObj


//----------------------------------------------------------------------
// Test the IRow interface of the row object.
//
BOOL CQuickTest::testIRow(IRow* pIRow, DBCOUNTITEM ulRowNum)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	CRowObject	CRow;
	IRowset*	pIRowset = NULL;
	IAccessor*	pIAccessor = NULL;

	if(!pIRow || !m_pTable)
		return FALSE;

	//Try to get the source rowset of this row object.
	TEST2C_(hr=pIRow->GetSourceRowset(IID_IRowset, (IUnknown**)&pIRowset,
		NULL), S_OK, DB_E_NOSOURCEOBJECT)

	//Try to get the source rowset again, with a different iid.
	if(hr==S_OK)
		TESTC_(hr=pIRow->GetSourceRowset(IID_IAccessor, (IUnknown**)&pIAccessor,
			NULL), S_OK)
	else
		TESTC_(hr=pIRow->GetSourceRowset(IID_IAccessor, (IUnknown**)&pIAccessor,
			NULL), DB_E_NOSOURCEOBJECT)

	if(hr==S_OK)
		TESTC((pIAccessor != NULL) && (pIRowset!= NULL))
	else
		odtLog<<L"INFO: There is no rowset object as source for the row.\n";

	//If a row number (of this row in parent rowset) was passed in, 
	//use that to verify data.
	if(ulRowNum)
	{
		TESTC_(CRow.SetRowObject(pIRow), S_OK)

		TESTC(CRow.VerifyGetColumns(ulRowNum, m_pTable, ALL_COLS_BOUND,
			NO_BLOB_COLS, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, 0, 
			NULL))
	}

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} //testIRow


//----------------------------------------------------------------------
//@mfunc Test the IColumnsInfo2 interface of a row object.
//
BOOL CQuickTest::testColInfo2( 
	CRowObject*		pCRowObj,
	BOOL			bTestResColInfo)
{
	TBEGIN
	HRESULT			hr;
	DBORDINAL		ulIndex = 0;
	BOOL			fColIDWarning = FALSE;
	DBID			rgColIDMasks[4];
	DBORDINAL		ulCols=0, ulCols2=0;
	DBORDINAL		cColsInBaseTable = 0;
	DBID*			rgColIDs = NULL;
	DBCOLUMNINFO*	rgColInfo = NULL;
	DBCOLUMNINFO*	rgColInfo2 = NULL;
	WCHAR*			pStrBuf = NULL;
	WCHAR*			pStrBuf2 = NULL;
	CCol			rgCol;
	IColumnsInfo*	pICI = NULL;
	IColumnsInfo2*	pICI2 = NULL;

	TESTC((pCRowObj!=NULL) && (m_pTable!=NULL))

	//Test MapColumnIDs.
	TESTC(testMapColumnIDs(pCRowObj->pIRow()))

	if(bTestResColInfo)
		TESTC(VerifyInterface(pCRowObj->pIRow(), IID_IColumnsInfo2,
			ROW_INTERFACE, (IUnknown**)&pICI2))
	else
		TESTC(VerifyInterface(pCRowObj->pIRow(), IID_IColumnsInfo,
			ROW_INTERFACE, (IUnknown**)&pICI))

	//Test GetColumnInfo method.

	if(pICI2)
		TESTC_(pICI2->GetColumnInfo(&ulCols, &rgColInfo, &pStrBuf), S_OK)
	else
		TESTC_(pICI->GetColumnInfo(&ulCols, &rgColInfo, &pStrBuf), S_OK)

    TESTC(ulCols!=0 && rgColInfo!=NULL && pStrBuf!=NULL)

    //Verify the number of columns.
	if(rgColInfo[0].iOrdinal==0)
		TESTC(ulCols-1 >= m_pTable->CountColumnsOnTable())
	else
		TESTC(ulCols >= m_pTable->CountColumnsOnTable())

	//Verify the DBCOLUMNINFO values.
	for(ulIndex=0; ulIndex<ulCols; ulIndex++)
	{
		//Ignore the bookmarks column if it exists.
		if((ulIndex==0)&&(rgColInfo[ulIndex].iOrdinal==0))
		{
			// Check for the Bookmark FLAG.
			TESTC(rgColInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)
			continue;
		}
	
		//Verify the ordinal value.
		TESTC(rgColInfo[ulIndex].iOrdinal == ulIndex+rgColInfo[0].iOrdinal)

		//Get the CCol structure for this column.
		if(m_pTable->GetColInfo(rgColInfo[ulIndex].iOrdinal, rgCol) != S_OK)
			continue;

		cColsInBaseTable++;

		//Verify the column name.
		if((rgColInfo[ulIndex].pwszName)&&(rgCol.GetColName()))
			TESTC(wcscmp(rgColInfo[ulIndex].pwszName, 
				rgCol.GetColName()) == 0)
		else
			TESTC((rgColInfo[ulIndex].pwszName == NULL)&&
				(rgCol.GetColName() == NULL))

		//ITypeInfo is reserved for future use.
		TESTC(rgColInfo[ulIndex].pTypeInfo == rgCol.GetTypeInfo())

		//Verify the DBTYPE.
		TESTC(rgColInfo[ulIndex].wType == rgCol.GetProviderType())

		//Compare the column IDs.
		if(!CompareDBID(rgColInfo[ulIndex].columnid, *(rgCol.GetColID())))
		{
			if(ulIndex>1)
				TESTC(fColIDWarning)
			TESTC(rgColInfo[ulIndex].columnid.eKind != (*(rgCol.GetColID())).eKind)
			fColIDWarning = TRUE;
		}
	}

	TESTC(cColsInBaseTable == m_pTable->CountColumnsOnTable())

	if(fColIDWarning)
	{
		//Issue a warning.
		odtLog<<L"WARNING: ColID returned is of different DBKIND.\n";
		COMPAREW(fColIDWarning, FALSE);
	}

	//If we do not have to test IColumnsInfo2::GetRestrictedColumnInfo,
	//then skip the following portion.

	if(!bTestResColInfo)
		goto CLEANUP;

	//rgColumnIDMasks should be ignoed when cColumnIDMasks=0.
	TESTC_(pICI2->GetRestrictedColumnInfo(0, INVALID(DBID*), 0, &ulCols2,
		&rgColIDs, &rgColInfo2, &pStrBuf2), S_OK)

	//The columns obtained from GetRestrictedColumnInfo with zero
	//restrictions has to be same as that obtained from GetColumnInfo method.

	TESTC(ulCols2 == ulCols)
	TESTC(rgColIDs!=NULL && rgColInfo2!=NULL && pStrBuf2!=NULL)

	for(ulIndex=0; ulIndex<ulCols; ulIndex++)
	{
		TESTC(rgColInfo2[ulIndex].iOrdinal == ulIndex+rgColInfo2[0].iOrdinal)
		TESTC(CompareDBID(rgColInfo2[ulIndex].columnid, rgColInfo[ulIndex].columnid))
		TESTC(CompareDBID(rgColInfo2[ulIndex].columnid, rgColIDs[ulIndex]))
	}

	SAFE_FREE(rgColInfo2);
	SAFE_FREE(pStrBuf2);
	SAFE_FREE(rgColIDs);
	ulCols2 = 0;

	//Build restrictions for GetRestrictedColumnInfo.

	for(ulIndex=0; ulIndex<4; ulIndex++)
		rgColIDMasks[ulIndex].eKind = DBKIND_NAME;

	rgColIDMasks[0].uName.pwszName = L"C";
	rgColIDMasks[1].uName.pwszName = L"c";
	rgColIDMasks[2].uName.pwszName = L"N";
	rgColIDMasks[3].uName.pwszName = L"n";

	//Call GetRestrictedColumnInfo with above restrictions
	//and verify.

	TEST2C_(hr=pICI2->GetRestrictedColumnInfo(4, rgColIDMasks, 0, &ulCols2,
		&rgColIDs, &rgColInfo2, &pStrBuf2), S_OK, DB_E_NOCOLUMN)

	if(hr==S_OK)
		TESTC(ulCols2>0 && rgColIDs!=NULL && rgColInfo2!=NULL && pStrBuf2!=NULL)
	else
		TESTC(ulCols2==0 && rgColIDs==NULL && rgColInfo2==NULL && pStrBuf2==NULL)

	for(ulIndex=0; ulIndex<ulCols2; ulIndex++)
	{
		TESTC(rgColInfo2[ulIndex].columnid.uName.pwszName != NULL)
		TESTC((_wcsnicmp(rgColInfo2[ulIndex].columnid.uName.pwszName,
			L"c",1)==0) ||
			(_wcsnicmp(rgColInfo2[ulIndex].columnid.uName.pwszName,
			L"n",1)==0) )
	}

CLEANUP:
	SAFE_FREE(rgColIDs);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(rgColInfo2);
	SAFE_FREE(pStrBuf);
	SAFE_FREE(pStrBuf2);
	SAFE_RELEASE(pICI);
	SAFE_RELEASE(pICI2);
	TRETURN
} //testColInfo2


//----------------------------------------------------------------------
// @mfunc Test IRowChange interface.
//
BOOL CQuickTest::testIRowChange(CRowObject*	pCRowObj)
{
	TBEGIN
	DBLENGTH			cbRowSize = 0;
	DBORDINAL			cColAccess = 0;
	DBCOLUMNACCESS*		rgColAccess = NULL;
	void*				pData = NULL;

	TESTC((pCRowObj!=NULL) && (m_pTable!=NULL))

	//Create the DBCOLUMNACCESS struct and fill in data to be set on the row.

	TESTC_(pCRowObj->CreateColAccess(&cColAccess, &rgColAccess, &pData, 
		&cbRowSize, UPDATEABLE_NONINDEX_COLS_BOUND), S_OK)

	TESTC_(pCRowObj->FillColAccess(m_pTable, cColAccess, rgColAccess, 
		2, PRIMARY), S_OK)

	//Call SetColumns to change the data in the row object.
	TESTC_(pCRowObj->SetColumns(cColAccess, rgColAccess), S_OK)

	//Verify new data in row object.
	TESTC(pCRowObj->VerifyGetColumns(2, m_pTable, UPDATEABLE_NONINDEX_COLS_BOUND))

	//Replace the original data by calling SetColumns again.

	TESTC_(pCRowObj->FillColAccess(m_pTable, cColAccess, rgColAccess, 
		1, PRIMARY), S_OK)

	TESTC_(pCRowObj->SetColumns(cColAccess, rgColAccess), S_OK)

	TESTC(pCRowObj->VerifyGetColumns(1, m_pTable, UPDATEABLE_NONINDEX_COLS_BOUND))

CLEANUP:
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	TRETURN
} //testIRowChange


//----------------------------------------------------------------------
// @mfunc Test the IRowSchemaChange interface.
//
BOOL CQuickTest::testIRowSchemaChange(CRowObject* pCRowObj)
{
	TBEGIN
	DBORDINAL			ulIndex = 0;
	DBLENGTH			cbRowSize = 0;
	DBORDINAL			cColAccess = 0;
	DBCOLUMNACCESS*		rgColAccess = NULL;
	DBCOLUMNINFO*		rgNewColInfo = NULL;
	void*				pData = NULL;
	DBORDINAL			cColInfo2 = 0;
	DBORDINAL			cColInfo = 0;
	DBCOLUMNINFO*		rgColInfo = NULL;
	WCHAR*				pwszBuff = NULL;
	DBSTATUS			dwStatus = 0;
	IColumnsInfo*		pICI = NULL;
	IRowSchemaChange*	pIRSC = NULL;

	TESTC((pCRowObj!=NULL) && (m_pTable!=NULL))

	//Create the DBCOLUMNACCESS struct and fill in data. One
	//column from this will be modified and used for AddColumn.

	TESTC_(pCRowObj->CreateColAccess(&cColAccess, &rgColAccess, &pData, 
		&cbRowSize, UPDATEABLE_NONINDEX_COLS_BOUND), S_OK)

	TESTC_(pCRowObj->FillColAccess(m_pTable, cColAccess, rgColAccess, 
		2, PRIMARY), S_OK)

	//Allocate a new DBCOLUMNINFO struct to pass in to AddColumns.
	SAFE_ALLOC(rgNewColInfo, DBCOLUMNINFO, 1);
	memset(rgNewColInfo, 0, sizeof(DBCOLUMNINFO));

	TESTC(VerifyInterface(pCRowObj->pIRow(), IID_IColumnsInfo,
		ROW_INTERFACE, (IUnknown**)&pICI))

	TESTC_(pICI->GetColumnInfo(&cColInfo, &rgColInfo, &pwszBuff), S_OK)

	//Get the column info for same column as in rgColAccess[0].
	for(ulIndex=0; ulIndex<cColInfo; ulIndex++)
	{
		if(CompareDBID(rgColInfo[ulIndex].columnid, rgColAccess[0].columnid))
			break;
	}

	//Make sure we did find it before hitting end of the FOR loop.
	TESTC(ulIndex<cColInfo);

	//copy.
	rgNewColInfo[0] = rgColInfo[ulIndex];

	//Free and null string pointers.
	SAFE_FREE(pwszBuff);
	rgNewColInfo[0].pwszName = NULL;
	if(rgNewColInfo[0].columnid.eKind == DBKIND_GUID_NAME ||
		rgNewColInfo[0].columnid.eKind == DBKIND_NAME)
		rgNewColInfo[0].columnid.uName.pwszName = NULL;

	//Create a unique name for the columnid.
	switch(rgNewColInfo[0].columnid.eKind)
	{
		case DBKIND_GUID_PROPID:
		case DBKIND_GUID:
			//Create a new guid (fairly unique!)
			TESTC_(CoCreateGuid(&rgNewColInfo[0].columnid.uGuid.guid),S_OK);
			break;
			
		case DBKIND_PROPID:
			rgNewColInfo[0].columnid.uName.ulPropid++;
			break;

		case DBKIND_GUID_NAME:
		case DBKIND_NAME:
		{
			GUID guid;
			CoCreateGuid(&guid);
			TESTC_(StringFromCLSID(guid, &rgNewColInfo[0].columnid.uName.pwszName),S_OK)
			rgNewColInfo[0].pwszName = rgNewColInfo[0].columnid.uName.pwszName;
			break;
		}

		default:
			TESTC(FALSE);
			break;
	};

	//NOTE: No need to modify rgColAccess[0].columnid. 
	//      This should be ignored. We will let it remain
	//      different from rgNewColInfo[0].columnid just to
	//      make sure it is ignored.

	TESTC(VerifyInterface(pCRowObj->pIRow(), IID_IRowSchemaChange,
		ROW_INTERFACE, (IUnknown**)&pIRSC))

	//Add the new column.
	TESTC_(pIRSC->AddColumns(1, rgNewColInfo, rgColAccess), S_OK)
	TESTC(rgColAccess[0].dwStatus == DBSTATUS_S_OK)

	SAFE_FREE(rgColInfo);

	//Verify the new column was added.
	TESTC_(pICI->GetColumnInfo(&cColInfo2, &rgColInfo, &pwszBuff), S_OK)
	TESTC(cColInfo2 = cColInfo+1)

	for(ulIndex=0; ulIndex<cColInfo2; ulIndex++)
		if(CompareDBID(rgColInfo[ulIndex].columnid, rgNewColInfo[0].columnid))
			break;

	TESTC(ulIndex<cColInfo2)
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pwszBuff);

	//Delete the new column.
	TESTC_(pIRSC->DeleteColumns(1, &rgNewColInfo[0].columnid, &dwStatus), S_OK)
	TESTC(dwStatus == DBSTATUS_S_OK)

	//Verify the new column is deleted.

	TESTC_(pICI->GetColumnInfo(&cColInfo2, &rgColInfo, &pwszBuff), S_OK)
	TESTC(cColInfo2 = cColInfo)

	for(ulIndex=0; ulIndex<cColInfo2; ulIndex++)
		if(CompareDBID(rgColInfo[ulIndex].columnid, rgNewColInfo[0].columnid))
			break;

	TESTC(ulIndex == cColInfo2)

CLEANUP:
	if(rgNewColInfo)
		SAFE_FREE(rgNewColInfo[0].pwszName);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pwszBuff);
	SAFE_FREE(rgNewColInfo);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_RELEASE(pICI);
	SAFE_RELEASE(pIRSC);
	TRETURN
} //testIRowSchemaChange



////////////////////////////////////////////////////////////////////////
// Test Case Section
//
///////////////////////////////////////////////////////////////////////


// {{ TCW_TEST_CASE_MAP(TCDataSource)
//---------------------------------------------------------------------
// @class DataSource Interfaces
//
class TCDataSource : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDataSource,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	

	// {{ TCW_TESTVARS()
	// @cmember IDBInitialize::Initialize
	int Variation_1();
	// @cmember IDBInitialize::Uninitialize
	int Variation_2();
	// @cmember IDBCreateSession
	int Variation_3();
	// @cmember IDBProperties::GetProperties
	int Variation_4();
	// @cmember IDBProperties::GetProperties(ALL)
	int Variation_5();
	// @cmember IDBProperties::GetPropertyInfo
	int Variation_6();
	// @cmember IDBProperties::GetPropertyInfo(ALL)
	int Variation_7();
	// @cmember IDBProperties::SetProperties
	int Variation_8();
	// @cmember Get properties before initialization
	int Variation_9();
	// @cmember IDBInfo::GetKeywords
	int Variation_10();
	// @cmember IDBInfo::GetLiteralInfo
	int Variation_11();
	// @cmember ISourcesRowset
	int Variation_12();
	// @cmember IDataInitialize::CreateDBInstance
	int Variation_13();
	// @cmember IDataInitialize::GetDataSource
	int Variation_14();
	// @cmember IPersist::GetClassID
	int Variation_15();
	// @cmember IPersistFile::GetClassID
	int Variation_16();
	// @cmember IPersistFile::IsDirty
	int Variation_17();
	// @cmember IPersistFile::Save
	int Variation_18();
	// @cmember IPersistFile::SaveCompleted
	int Variation_19();
	// @cmember IPersistFile::Load
	int Variation_20();
	// @cmember IPersistFile::GetCurFile
	int Variation_21();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCDataSource)
#define THE_CLASS TCDataSource
BEG_TEST_CASE(TCDataSource, CQuickTest, L"DataSource Interfaces")
	TEST_VARIATION(1, 		L"IDBInitialize::Initialize")
	TEST_VARIATION(2, 		L"IDBInitialize::Uninitialize")
	TEST_VARIATION(3, 		L"IDBCreateSession")
	TEST_VARIATION(4, 		L"IDBProperties::GetProperties")
	TEST_VARIATION(5, 		L"IDBProperties::GetProperties(ALL)")
	TEST_VARIATION(6, 		L"IDBProperties::GetPropertyInfo")
	TEST_VARIATION(7, 		L"IDBProperties::GetPropertyInfo(ALL)")
	TEST_VARIATION(8, 		L"IDBProperties::SetProperties")
	TEST_VARIATION(9, 		L"Get properties before initialization")
	TEST_VARIATION(10, 		L"IDBInfo::GetKeywords")
	TEST_VARIATION(11, 		L"IDBInfo::GetLiteralInfo")
	TEST_VARIATION(12, 		L"ISourcesRowset")
	TEST_VARIATION(13, 		L"IDataInitialize::CreateDBInstance")
	TEST_VARIATION(14, 		L"IDataInitialize::GetDataSource")
	TEST_VARIATION(15, 		L"IPersist::GetClassID")
	TEST_VARIATION(16, 		L"IPersistFile::GetClassID")
	TEST_VARIATION(17, 		L"IPersistFile::IsDirty")
	TEST_VARIATION(18, 		L"IPersistFile::Save")
	TEST_VARIATION(19, 		L"IPersistFile::SaveCompleted")
	TEST_VARIATION(20, 		L"IPersistFile::Load")
	TEST_VARIATION(21, 		L"IPersistFile::GetCurFile")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(TCSession)
//*---------------------------------------------------------------------
// @class Session Interfaces
//
class TCSession : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSession,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IGetDataSource
	int Variation_1();
	// @cmember IOpenRowset
	int Variation_2();
	// @cmember ISessionProperties::GetProperties
	int Variation_3();
	// @cmember ISessionProperties::SetProperties
	int Variation_4();
	// @cmember IDBCreateCommand
	int Variation_5();
	// @cmember IDBSchemaRowset::GetSchemas
	int Variation_6();
	// @cmember IDBSchemaRowset::GetRowset(DBSCHEMA_COLUMNS)
	int Variation_7();
	// @cmember IDBSchemaRowset::GetRowset(DBSCHEMA_TABLES)
	int Variation_8();
	// @cmember IDBSchemaRowset::GetRowset(DBSCHEMA_PROVIDER_TYPES)
	int Variation_9();
	// @cmember ITableDefinition::AddColumn
	int Variation_10();
	// @cmember ITableDefinition::CreateTable
	int Variation_11();
	// @cmember ITableDefinition::DropColumn
	int Variation_12();
	// @cmember ITableDefinition::DropTable
	int Variation_13();
	// @cmember IIndexDefinition
	int Variation_14();
	// @cmember IAlterIndex
	int Variation_15();
	// @cmember IAlterTable::AlterColumn
	int Variation_16();
	// @cmember IAlterTable::AlterTable
	int Variation_17();
	// @cmember ITableDefinitionWithConstraints - AddConstraint & DropConstraint
	int Variation_18();
	// @cmember ITableDefinitionWithConstraints::CreateTableWithConstraints
	int Variation_19();
	// @cmember AggregateSession
	int Variation_20();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCSession)
#define THE_CLASS TCSession
BEG_TEST_CASE(TCSession, CQuickTest, L"Session Interfaces")
	TEST_VARIATION(1, 		L"IGetDataSource")
	TEST_VARIATION(2, 		L"IOpenRowset")
	TEST_VARIATION(3, 		L"ISessionProperties::GetProperties")
	TEST_VARIATION(4, 		L"ISessionProperties::SetProperties")
	TEST_VARIATION(5, 		L"IDBCreateCommand")
	TEST_VARIATION(6, 		L"IDBSchemaRowset::GetSchemas")
	TEST_VARIATION(7, 		L"IDBSchemaRowset::GetRowset(DBSCHEMA_COLUMNS)")
	TEST_VARIATION(8, 		L"IDBSchemaRowset::GetRowset(DBSCHEMA_TABLES)")
	TEST_VARIATION(9, 		L"IDBSchemaRowset::GetRowset(DBSCHEMA_PROVIDER_TYPES)")
	TEST_VARIATION(10, 		L"ITableDefinition::AddColumn")
	TEST_VARIATION(11, 		L"ITableDefinition::CreateTable")
	TEST_VARIATION(12, 		L"ITableDefinition::DropColumn")
	TEST_VARIATION(13, 		L"ITableDefinition::DropTable")
	TEST_VARIATION(14, 		L"IIndexDefinition")
	TEST_VARIATION(15, 		L"IAlterIndex")
	TEST_VARIATION(16, 		L"IAlterTable::AlterColumn")
	TEST_VARIATION(17, 		L"IAlterTable::AlterTable")
	TEST_VARIATION(18, 		L"ITableDefinitionWithConstraints - AddConstraint & DropConstraint")
	TEST_VARIATION(19, 		L"ITableDefinitionWithConstraints::CreateTableWithConstraints")
	TEST_VARIATION(20, 		L"AggregateSession")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCRowsetNotUpdateable)
//*---------------------------------------------------------------------
// @class Rowset Interfaces (Non-Updateable Rowset)
//
class TCRowsetNotUpdateable : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetNotUpdateable,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IConvertType
	int Variation_1();
	// @cmember IColumnsInfo::GetColumnInfo
	int Variation_2();
	// @cmember IColumnsInfo::MapColumnIDs
	int Variation_3();
	// @cmember IAccessor
	int Variation_4();
	// @cmember IRowsetInfo::GetProperties
	int Variation_5();
	// @cmember IRowsetInfo::GetReferencedRowset
	int Variation_6();
	// @cmember IRowsetInfo::GetSpecification (Session)
	int Variation_7();
	// @cmember IRowsetInfo::GetSpecification (Command)
	int Variation_8();
	// @cmember IRowset (No Properties)
	int Variation_9();
	// @cmember IRowset (Scroll_Back & Canholdrows)
	int Variation_10();
	// @cmember IColumnsRowset::GetAvailableColumns
	int Variation_11();
	// @cmember IColumnsRowset::GetColumnsRowset
	int Variation_12();
	// @cmember IRowsetLocate::Compare
	int Variation_13();
	// @cmember IRowsetLocate::GetRowsAt
	int Variation_14();
	// @cmember IRowsetLocate::GetRowsByBookmark
	int Variation_15();
	// @cmember IRowsetLocate::Hash
	int Variation_16();
	// @cmember IRowsetFind
	int Variation_17();
	// @cmember IRowsetScroll::GetApproximatePosition
	int Variation_18();
	// @cmember IRowsetScroll::GetRowsAtRatio
	int Variation_19();
	// @cmember IRowPosition
	int Variation_20();
	// @cmember AggregateRowset
	int Variation_21();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRowsetNotUpdateable)
#define THE_CLASS TCRowsetNotUpdateable
BEG_TEST_CASE(TCRowsetNotUpdateable, CQuickTest, L"Rowset Interfaces (Non-Updateable Rowset)")
	TEST_VARIATION(1, 		L"IConvertType")
	TEST_VARIATION(2, 		L"IColumnsInfo::GetColumnInfo")
	TEST_VARIATION(3, 		L"IColumnsInfo::MapColumnIDs")
	TEST_VARIATION(4, 		L"IAccessor")
	TEST_VARIATION(5, 		L"IRowsetInfo::GetProperties")
	TEST_VARIATION(6, 		L"IRowsetInfo::GetReferencedRowset")
	TEST_VARIATION(7, 		L"IRowsetInfo::GetSpecification (Session)")
	TEST_VARIATION(8, 		L"IRowsetInfo::GetSpecification (Command)")
	TEST_VARIATION(9, 		L"IRowset (No Properties)")
	TEST_VARIATION(10, 		L"IRowset (Scroll_Back & Canholdrows)")
	TEST_VARIATION(11, 		L"IColumnsRowset::GetAvailableColumns")
	TEST_VARIATION(12, 		L"IColumnsRowset::GetColumnsRowset")
	TEST_VARIATION(13, 		L"IRowsetLocate::Compare")
	TEST_VARIATION(14, 		L"IRowsetLocate::GetRowsAt")
	TEST_VARIATION(15, 		L"IRowsetLocate::GetRowsByBookmark")
	TEST_VARIATION(16, 		L"IRowsetLocate::Hash")
	TEST_VARIATION(17, 		L"IRowsetFind")
	TEST_VARIATION(18, 		L"IRowsetScroll::GetApproximatePosition")
	TEST_VARIATION(19, 		L"IRowsetScroll::GetRowsAtRatio")
	TEST_VARIATION(20, 		L"IRowPosition")
	TEST_VARIATION(21, 		L"AggregateRowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCRowsetUpdateable)
//*---------------------------------------------------------------------
// @class Rowset Interfaces (Updateable Rowset)
//
class TCRowsetUpdateable : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetUpdateable,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IRowsetIdentity
	int Variation_1();
	// @cmember IRowset (various properties set)
	int Variation_2();
	// @cmember IRowsetChange::SetData
	int Variation_3();
	// @cmember IRowsetChange::DeleteRows
	int Variation_4();
	// @cmember IRowsetChange::InsertRow
	int Variation_5();
	// @cmember IRowsetUpdate
	int Variation_6();
	// @cmember IRowsetResynch
	int Variation_7();
	// @cmember IRowsetRefresh
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRowsetUpdateable)
#define THE_CLASS TCRowsetUpdateable
BEG_TEST_CASE(TCRowsetUpdateable, CQuickTest, L"Rowset Interfaces (Updateable Rowset)")
	TEST_VARIATION(1, 		L"IRowsetIdentity")
	TEST_VARIATION(2, 		L"IRowset (various properties set)")
	TEST_VARIATION(3, 		L"IRowsetChange::SetData")
	TEST_VARIATION(4, 		L"IRowsetChange::DeleteRows")
	TEST_VARIATION(5, 		L"IRowsetChange::InsertRow")
	TEST_VARIATION(6, 		L"IRowsetUpdate")
	TEST_VARIATION(7, 		L"IRowsetResynch")
	TEST_VARIATION(8, 		L"IRowsetRefresh")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCommand)
//*-----------------------------------------------------------------------
// @class Command Interfaces
//
class TCCommand : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCommand,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IAccessor
	int Variation_1();
	// @cmember IColumnsInfo::GetColumnInfo
	int Variation_2();
	// @cmember IColumnsInfo::MapColumnIDs
	int Variation_3();
	// @cmember ICommandProperties::GetProperties
	int Variation_4();
	// @cmember ICommandProperties::SetProperties
	int Variation_5();
	// @cmember IConvertType
	int Variation_6();
	// @cmember ICommandText
	int Variation_7();
	// @cmember ICommand::GetDBSession
	int Variation_8();
	// @cmember ICommand::Execute (set,exe,set,exe,set,exe)
	int Variation_9();
	// @cmember ICommand::Execute (set,exe,exe,exe)
	int Variation_10();
	// @cmember IColumnsRowset::GetAvailableColumns
	int Variation_11();
	// @cmember IColumnsRowset::GetColumnsRowset
	int Variation_12();
	// @cmember ICommandPrepare::Prepare
	int Variation_13();
	// @cmember ICommandPrepare::Unprepare
	int Variation_14();
	// @cmember ICommandWithParameters
	int Variation_15();
	// @cmember IMultipleResults
	int Variation_16();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCommand)
#define THE_CLASS TCCommand
BEG_TEST_CASE(TCCommand, CQuickTest, L"Command Interfaces")
	TEST_VARIATION(1, 		L"IAccessor")
	TEST_VARIATION(2, 		L"IColumnsInfo::GetColumnInfo")
	TEST_VARIATION(3, 		L"IColumnsInfo::MapColumnIDs")
	TEST_VARIATION(4, 		L"ICommandProperties::GetProperties")
	TEST_VARIATION(5, 		L"ICommandProperties::SetProperties")
	TEST_VARIATION(6, 		L"IConvertType")
	TEST_VARIATION(7, 		L"ICommandText")
	TEST_VARIATION(8, 		L"ICommand::GetDBSession")
	TEST_VARIATION(9, 		L"ICommand::Execute (set,exe,set,exe,set,exe)")
	TEST_VARIATION(10, 		L"ICommand::Execute (set,exe,exe,exe)")
	TEST_VARIATION(11, 		L"IColumnsRowset::GetAvailableColumns")
	TEST_VARIATION(12, 		L"IColumnsRowset::GetColumnsRowset")
	TEST_VARIATION(13, 		L"ICommandPrepare::Prepare")
	TEST_VARIATION(14, 		L"ICommandPrepare::Unprepare")
	TEST_VARIATION(15, 		L"ICommandWithParameters")
	TEST_VARIATION(16, 		L"IMultipleResults")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCOtherObjects)
//*-----------------------------------------------------------------------
// @class Other Objects
//
class TCOtherObjects : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOtherObjects,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember ErrorInterfaces
	int Variation_1();
	// @cmember Transaction(Commit)
	int Variation_2();
	// @cmember Transaction(Abort)
	int Variation_3();
	// @cmember StorageInterface
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCOtherObjects)
#define THE_CLASS TCOtherObjects
BEG_TEST_CASE(TCOtherObjects, CQuickTest, L"Other Objects")
	TEST_VARIATION(1, 		L"ErrorInterfaces")
	TEST_VARIATION(2, 		L"Transaction(Commit)")
	TEST_VARIATION(3, 		L"Transaction(Abort)")
	TEST_VARIATION(4, 		L"StorageInterface")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBinder)
//*-----------------------------------------------------------------------
// @class Binder Interfaces
//
class TCBinder : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBinder,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IBindResource
	int Variation_1();
	// @cmember ICreateRow
	int Variation_2();
	// @cmember IDBBinderProperties - Set rowset props
	int Variation_3();
	// @cmember IDBBinderProperties - Reset
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBinder)
#define THE_CLASS TCBinder
BEG_TEST_CASE(TCBinder, CQuickTest, L"Binder Interfaces")
	TEST_VARIATION(1, 		L"IBindResource")
	TEST_VARIATION(2, 		L"ICreateRow")
	TEST_VARIATION(3, 		L"IDBBinderProperties - Set rowset props")
	TEST_VARIATION(4, 		L"IDBBinderProperties - Reset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCRow)
//*-----------------------------------------------------------------------
// @class Row Interfaces
//
class TCRow : public CQuickTest { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRow,CQuickTest);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IRow
	int Variation_1();
	// @cmember IGetSession
	int Variation_2();
	// @cmember IConvertType
	int Variation_3();
	// @cmember IColumnsInfo
	int Variation_4();
	// @cmember IColumnsInfo2
	int Variation_5();
	// @cmember IGetRow
	int Variation_6();
	// @cmember IGetSourceRow
	int Variation_7();
	// @cmember IRowChange
	int Variation_8();
	// @cmember IRowSchemaChange
	int Variation_9();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRow)
#define THE_CLASS TCRow
BEG_TEST_CASE(TCRow, CQuickTest, L"Row Interfaces")
	TEST_VARIATION(1, 		L"IRow")
	TEST_VARIATION(2, 		L"IGetSession")
	TEST_VARIATION(3, 		L"IConvertType")
	TEST_VARIATION(4, 		L"IColumnsInfo")
	TEST_VARIATION(5, 		L"IColumnsInfo2")
	TEST_VARIATION(6, 		L"IGetRow")
	TEST_VARIATION(7, 		L"IGetSourceRow")
	TEST_VARIATION(8, 		L"IRowChange")
	TEST_VARIATION(9, 		L"IRowSchemaChange")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(8, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCDataSource)
	TEST_CASE(2, TCSession)
	TEST_CASE(3, TCRowsetNotUpdateable)
	TEST_CASE(4, TCRowsetUpdateable)
	TEST_CASE(5, TCCommand)
	TEST_CASE(6, TCOtherObjects)
	TEST_CASE(7, TCBinder)
	TEST_CASE(8, TCRow)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TCDataSource)
//*---------------------------------------------------------------------
//| Test Case:		TCDataSource - DataSource Interfaces
//|	Created:			08/24/95
//*---------------------------------------------------------------------

//----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine. Gets the pointers to 
//IPersistFile and IDBProperties interfaces. Also initializes m_rgPropIDSets
//m_rgPropInfoSets with the selected properties.
//
// @rdesc TRUE or FALSE
//
BOOL TCDataSource::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{
		if(CHECK(m_hr = CreateDataSourceObject(),S_OK))
		{
			VerifyInterface(m_pIDBInitialize,IID_IPersistFile, 
				DATASOURCE_INTERFACE,(IUnknown **)&m_pIPersistFile) ;
			VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
				DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties) ;

			if(m_pIDBProperties == NULL)
			{
				odtLog<<L"INFO: Could not get IDBProperties (level-0) interface.\n";
				return FALSE;
			}

			InitPropIDStructs();

			if(CHECK(InitializeDSO(), S_OK))
			{
				InitSupInfo();
				return TRUE;
			}
		}
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc IDBInitialize::Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
//Create another DSO. Get the initialization properties and initialize
//it. To make sure that it was actually initialized call CreateSession.
//
int TCDataSource::Variation_1()
{ 
	IDBInitialize*		pIDBInitialize = NULL;
	IDBCreateSession*	pIDBCreateSession = NULL;
	IOpenRowset*		pIOpenRowset = NULL;
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr;

	//Create a new Data Source Object and initialize it.
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, 
		(IUnknown**)&pIDBInitialize), S_OK)

	//Obtain the IDBCreateSession interface.
	if(!VerifyInterface(pIDBInitialize, IID_IDBCreateSession, 
		DATASOURCE_INTERFACE, (IUnknown**)&pIDBCreateSession))
		goto CLEANUP;

	//CreateSession should succeed if the DSO was properly initialized.
	if(!CHECK(hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset,
		(IUnknown**)&pIOpenRowset), S_OK))
		goto CLEANUP;

	if(hr == S_OK)
		tTestResult = TEST_PASS;

CLEANUP:
	//Release Session.
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIDBCreateSession);
	//Release the DSO.
	SAFE_RELEASE(pIDBInitialize);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc IDBInitialize::Uninitialize
//and then calls Uninitialize on it.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_2()
{
	IDBInitialize*	pIDBInitialize = NULL;
	IDBProperties*	pIDBProperties = NULL;
	ULONG			cPropSets =0;
	DBPROPSET*		rgPropSets = NULL;
	TESTRESULT		tTestResult = TEST_FAIL;
	HRESULT			hr;

	//Create a new DSO.
	CHECK(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, 
		(IUnknown**)&pIDBInitialize), S_OK);
	if(pIDBInitialize==NULL)
	{
		odtLog<<L"INFO: Could not create another DSO.\n";
		goto CLEANUP;
	}

	//Obtain the IDBProperties interface.
	if(!VerifyInterface(pIDBInitialize, IID_IDBProperties, 
		DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
		goto CLEANUP;

	//Get the initialization properties and set them.
	GetInitProps(&cPropSets, &rgPropSets);
	CHECK(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets), 
		S_OK);

	//Calling Uninitialize before Initialize should succeed.
	CHECK(hr = pIDBInitialize->Uninitialize(), S_OK);
	if(hr != S_OK)
	{
		odtLog<<L"INFO: Failure on Uninitialize (before Initialize).\n";
		goto CLEANUP;
	}

	//Initialize DSO.
	CHECK(hr = pIDBInitialize->Initialize(), S_OK);
	if(hr != S_OK) 
	{
		odtLog<<L"INFO: Failure on Initialize.\n";
		goto CLEANUP;
	}

	//Call Uninitialize on the DSO. This should succeed.
	CHECK(hr = pIDBInitialize->Uninitialize(), S_OK);
	if(hr == S_OK)
		tTestResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	//Release the DSO.
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc IDBCreateSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_3()
{ 
	TESTRESULT			tTestResult = TEST_FAIL;
	IDBCreateSession*	pIDBCreateSession = NULL;
	IOpenRowset*		pIOpenRowset = NULL;

	//Make sure that DSO is initialized before creating session.
	if(!m_fInitialized)
		CHECK(InitializeDSO(REINITIALIZE_NO), S_OK) ;

	if(!VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
		DATASOURCE_INTERFACE,(IUnknown**) &pIDBCreateSession))
	{
		odtLog<<L"Error getting IDBCreateSession Interface.\n";
		return TEST_FAIL;
	}

	TESTC_(pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, 
		(IUnknown**)&pIOpenRowset), S_OK)

	TESTC(pIOpenRowset != NULL)

	tTestResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIOpenRowset) ;
	SAFE_RELEASE(pIDBCreateSession) ;
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc IDBProperties::GetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_4()
{
	return testGetProperties() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc IDBProperties::GetProperties(ALL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_5()
{ 
	return testGetAllProperties(m_pIDBProperties) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc IDBProperties::GetPropertyInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_6()
{ 
	return testGetPropInfo() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*---------------------------------------------------------------------
// @mfunc IDBProperties::GetPropertyInfo(ALL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_7()
{
	return testGetAllPropInfo(m_pIDBProperties) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*---------------------------------------------------------------------
// @mfunc IDBProperties::SetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_8()
{
	return ((testSetProperties1() == TEST_PASS) && (testSetProperties2() == TEST_PASS)) ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*---------------------------------------------------------------------
// @mfunc Get properties before initialization
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_9()
{
	return testGetPropBeforeInit() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc IDBInfo::GetKeywords
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_10()
{ 
	return testGetKeywords();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc IDBInfo::GetLiteralInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_11()
{ 
	return testGetLiteralInfo();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ISourcesRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_12()
{ 
	return testISrcRowset();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc IDataInitialize::CreateDBInstance
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_13()
{ 
	return testIDataIzCreateDBIns();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc IDataInitialize::GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_14()
{ 
	return testIDataIzGetDS();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*---------------------------------------------------------------------
// @mfunc IPersist::GetClassID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_15()
{
	CLSID			clsid;
	IPersist * 	pIPersist;

	//Return if this interface is not supported 
	if (!VerifyInterface(m_pIDBInitialize,IID_IPersist, 
		DATASOURCE_INTERFACE,(IUnknown **)&pIPersist))
		return TEST_FAIL;

	//Verify that CLSID returned is identical to the Provider CLSID			
	CHECK(pIPersist->GetClassID(&clsid), S_OK);

	SAFE_RELEASE(pIPersist);

	return (clsid == GetModInfo()->GetThisTestModule()->m_ProviderClsid) ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::GetClassID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_16()
{
	CLSID			clsid;

	//Return if this interface is not supported 
	if(m_pIPersistFile == NULL)
		return TEST_SKIPPED;

	//Verify that CLSID returned is identical to the Provider CLSID			
	CHECK(m_pIPersistFile->GetClassID(&clsid), S_OK);

	return (clsid == GetModInfo()->GetThisTestModule()->m_ProviderClsid) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::IsDirty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_17()
{
	if(m_pIPersistFile == NULL) 
		return TEST_SKIPPED;

	//We haven't called Save yet, so we should be in a dirty state.
	//S_FALSE is only returned when the object had been saved and has
	//maintained the same state since. 
	CHECK(m_hr = m_pIPersistFile->IsDirty(),S_OK);

	return (m_hr == S_OK) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::Save
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_18()
{	 	
	if(m_pIPersistFile == NULL)
		return TEST_SKIPPED;

	//Test if the save to file succeeds.
	m_hr = PersistToFile();

	if (m_hr ==S_OK)
	{
		//If SAVE was successful, then the IsDirty should be FALSE.
		CHECK(m_pIPersistFile->IsDirty(),S_FALSE);
		return TEST_PASS;
	}
	else if (m_hr == E_NOTIMPL)
	{
		COMPARE((GetModInfo()->UseServiceComponents() & SERVICECOMP_INVOKE), SERVICECOMP_INVOKE);
		return TEST_SKIPPED;
	}

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::SaveCompleted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_19()
{
	BOOL			fResults = FALSE;
	
	if(m_pIPersistFile == NULL)
		return TEST_SKIPPED;

	//Do the save	
	m_hr = PersistToFile();

	if (m_hr == S_OK)
	{
		//Save completed should now return S_OK
		if (CHECK(m_pIPersistFile->SaveCompleted(m_wszFile),S_OK))
			fResults = TRUE;
		return (fResults) ? TEST_PASS : TEST_FAIL;
	}
	else if (m_hr == E_NOTIMPL)
	{
		COMPARE((GetModInfo()->UseServiceComponents() & SERVICECOMP_INVOKE), SERVICECOMP_INVOKE);
		return TEST_SKIPPED;
	}

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::Load
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_20()
{
	BOOL			fResults = FALSE;
	
	if(m_pIPersistFile == NULL)
		return TEST_SKIPPED;

	//Do the save
	m_hr = PersistToFile();

	if (m_hr == S_OK)
	{
		//We must release the current session for uninitialize to work
		if(m_pIOpenRowset != NULL || m_pIDBCreateCommand != NULL)
			ReleaseDBSession();
			
		//Uninitialize.
		if (CHECK(UninitializeDSO(), S_OK))
		{			
			//Now Load should succeed
			if (CHECK(m_pIPersistFile->Load((LPOLESTR)m_wszFile,
				STGM_READWRITE),S_OK))				
			{
				// Initialize the Provider with Loaded Properties
				if (CHECK(InitializeDSO(), S_OK))
					fResults = TRUE;
			}
		}
		return (fResults) ? TEST_PASS : TEST_FAIL;
	}
	else if (m_hr == E_NOTIMPL)
	{
		COMPARE((GetModInfo()->UseServiceComponents() & SERVICECOMP_INVOKE), SERVICECOMP_INVOKE);
		return TEST_SKIPPED;
	}

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*---------------------------------------------------------------------
// @mfunc IPersistFile::GetCurFile
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_21()
{
	LPOLESTR		pwszCurFile = NULL;
	BOOL			fResults = FALSE;
	
	if(m_pIPersistFile == NULL)
		return TEST_SKIPPED;

	//Do the save
	m_hr = PersistToFile();

	if (m_hr == S_OK)
	{
		if(!CHECK(m_pIPersistFile->GetCurFile(&pwszCurFile),S_OK))
			return TEST_FAIL;
	
		//If any of these is NULL, fail test.
		if(pwszCurFile && m_wszFile)
		{
			//Returns 0 when identical
			fResults = COMPARE(wcscmp(pwszCurFile,m_wszFile),0); 
		}
		
		PROVIDER_FREE(pwszCurFile);
		return (fResults) ? TEST_PASS : TEST_FAIL;
	}
	else if (m_hr == E_NOTIMPL)
	{
		COMPARE((GetModInfo()->UseServiceComponents() & SERVICECOMP_INVOKE), SERVICECOMP_INVOKE);
		return TEST_SKIPPED;
	}

	return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//----------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TEST_PASS or TEST_FAIL
//
BOOL TCDataSource::Terminate()
{
	CHAR*	pszFile = NULL;

	//Delete file we used for persistence
	if (m_wszFile[0] != L'\0')
	{
		pszFile = ConvertToMBCS(m_wszFile);
		remove(pszFile);
	}

	PROVIDER_FREE(pszFile);

	FreeProperties(&m_cPropInfoSets, &m_rgPropInfoSets);
	FreeProperties(&m_cPropIDSets, &m_rgPropIDSets);
	SAFE_RELEASE(m_pIPersistFile);
	SAFE_RELEASE(m_pIDBProperties);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
}  // }}
// }} TCW_TERMINATE_METHOD_END 
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCSession)
//*---------------------------------------------------------------------
//| Test Case:		TCSession - Session Interfaces
//| Created:  	3/18/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSession::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{
		if(CHECK(m_hr = CreateDBSession(),S_OK))
		{
			CTable* pTable = NULL;

			//Required for using some ExtraLib functions.
			g_pIDBInitialize = m_pIDBInitialize;
			g_pIOpenRowset = m_pIOpenRowset;

			//Create a table with MIN_ROWS rows.
			if(CreateTable(&pTable, MIN_ROWS))
			{
				SetTable(pTable, DELETETABLE_NO);
				if(m_pTable)
					return TRUE;
			}
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc IGetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_1()
{
	return testGetDataSource() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc IOpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_2()
{ 
	return testOpenRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc ISessionProperties::GetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_3()
{
	return testSessGetProp() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc ISessionProperties::SetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_4()
{
	return testSessSetProp() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IDBCreateCommand
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_5()
{
	if(!m_pIDBCreateCommand)
	{
		odtLog<<L"Commands are not supported.\n";
		return TEST_SKIPPED;
	}

	return testCreateCommand() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IDBSchemaRowset::GetSchemas
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_6()
{
	return testGetSchemas() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc IDBSchemaRowset::GetRowset(DBSCHEMA_COLUMNS)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_7()
{ 
	return testColumnsSchemaRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IDBSchemaRowset::GetRowset(DBSCHEMA_TABLES)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_8()
{ 
	return testTablesSchemaRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc IDBSchemaRowset::GetRowset(DBSCHEMA_PROVIDER_TYPES)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_9()
{ 
	return testProvTypesSchemaRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinition::AddColumn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_10()
{ 
	return testAddColumn() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinition::CreateTable
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_11()
{ 
	return testCreateTable() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinition::DropColumn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_12()
{ 
	return testDropColumn() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinition::DropTable
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_13()
{ 
	return testDropTable() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc IIndexDefinition
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_14()
{ 
	return testIIndexDef() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc IAlterIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_15()
{ 
	return testAlterIndex() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc IAlterTable::AlterColumn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_16()
{ 
	return testAlterColumn() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc IAlterTable::AlterTable
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_17()
{ 
	return testAlterTable() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinitionWithConstraints - AddConstraint & DropConstraint
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_18()
{ 
	return testAddAndDropConstraint() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ITableDefinitionWithConstraints::CreateTableWithConstraints
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_19()
{ 
	return testCreateTableWithConstraints() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc AggregateSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSession::Variation_20()
{ 
	TESTRESULT	tTestResult = TEST_FAIL;
	IDBInitialize* pIDBI = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;

	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, (IUnknown**)
		&pIDBI), S_OK)

	TESTC(VerifyInterface(pIDBI,IID_IDBCreateSession,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBCreateSession))

	tTestResult = testAggregateSession(pIDBCreateSession);

CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIDBI);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCSession::Terminate()
{
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCRowsetNotUpdateable)
//*---------------------------------------------------------------------
//| Test Case:		TCRowsetNotUpdateable - Rowset Interfaces (Non-Updateable Rowset)
//| Created:  	3/24/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetNotUpdateable::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{
		if(CHECK(m_hr = CreateDBSession(),S_OK))
		{
			CTable* pTable = NULL;

			//Required for using some ExtraLib functions without using
			//the ExtraLib classes.
			g_pIDBInitialize = m_pIDBInitialize;
			g_pIOpenRowset = m_pIOpenRowset;

			//Create a table with MIN_ROWS rows.
			if(CreateTable(&pTable, MIN_ROWS))
			{
				SetTable(pTable, DELETETABLE_NO);

				//Set the following properties.
				SetProperty(DBPROP_IRowsetScroll,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_IRowsetFind,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_IColumnsRowset,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_ORDEREDBOOKMARKS,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_CANSCROLLBACKWARDS,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_IRowsetLocate,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_BOOKMARKS,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;
				SetProperty(DBPROP_IRowsetIdentity,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;
				SetProperty(DBPROP_CANHOLDROWS,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

				//Open a rowset with above properties set.
				m_hr=CreateOpenRowset(m_pTable, IID_IRowset,
					(IUnknown**)&m_pIRowset);

				if(FAILED(m_hr))
					DumpPropertyErrors(m_cPropSets, m_rgPropSets);

				if(m_hr==S_OK || m_hr==DB_S_ERRORSOCCURRED)
				{
					FreeProperties(&m_cPropSets, &m_rgPropSets) ;

					VARIANT vVar;
					VariantInit(&vVar);

					//Get the values of following properties.
					if(GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET,
						m_pIRowset, &vVar))
						m_ulMaxOpenRows = V_I4(&vVar);
					else
						m_ulMaxOpenRows = 0;

					VariantClear(&vVar);

					return TRUE;
				}
			}
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_1()
{
	return testIConvertType() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc IColumnsInfo::GetColumnInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_2()
{
	return testGetColumnInfo((IUnknown*)m_pIRowset) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc IColumnsInfo::MapColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_3()
{
	return testMapColumnIDs((IUnknown*)m_pIRowset) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_4()
{
	return testIAccessor() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc IRowsetInfo::GetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_5()
{
	return testRowsetGetProp((IUnknown*)m_pIRowset) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc IRowsetInfo::GetReferencedRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_6()
{ 
	return testIRowsetInfoGetRefRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*---------------------------------------------------------------------
// @mfunc IRowsetInfo::GetSpecification (Session)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_7()
{ 
	return testIRowsetInfoGetSpec() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IRowsetInfo::GetSpecification (Command)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_8()
{ 
	return testIRowsetInfoGetSpecCmd() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*---------------------------------------------------------------------
// @mfunc IRowset (No Properties)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_9()
{ 
	return testIRowset2() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*---------------------------------------------------------------------
// @mfunc IRowset (Scroll_Back & Canholdrows)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_10()
{ 
	BOOL	fPropSup = TRUE;

	if(!GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"CANHOLDROWS (level-0 prop) is not supported.\n";
		return TEST_FAIL;
	}

	if(!GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		fPropSup = FALSE;

	if(!fPropSup)
	{
		odtLog<<L"CANSCROLLBACKWARDS is not supported.\n";
		return TEST_SKIPPED;
	}

	return testIRowset3() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc IColumnsRowset::GetAvailableColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_11()
{ 
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBORDINAL			cOptCols = 0;
	DBID*				rgOptCols = NULL;

	hr=GetAvailableColumns((IUnknown*)m_pIRowset, &cOptCols, 
		&rgOptCols) ;

	if(hr==E_NOINTERFACE)
		return TEST_SKIPPED;
	else
		TESTC_(hr, S_OK)

	TESTC(CheckOptColumns(cOptCols, rgOptCols))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgOptCols);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc IColumnsRowset::GetColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_12()
{ 
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	BOOL				bBookmark = FALSE;
	DBORDINAL			cOptCols = 0;
	DBID*				rgOptCols = NULL;
	IColumnsRowset*		pIColumnsRowset = NULL;
	IRowset*			pIRowset = NULL;

	if(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		bBookmark = TRUE;

	if(!VerifyInterface(m_pIRowset,IID_IColumnsRowset,
		ROWSET_INTERFACE,(IUnknown**)&pIColumnsRowset))
	{
		odtLog<<L"IColumnsRowset is not supported.\n";
		return TEST_SKIPPED;
	}
	TESTC(pIColumnsRowset != NULL)

	TESTC_(hr=GetAvailableColumns((IUnknown*)m_pIRowset, &cOptCols, 
		&rgOptCols), S_OK)

	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, cOptCols, rgOptCols,
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK)

	TESTC(CheckColumnsRowset((IUnknown*) pIRowset, cOptCols, bBookmark))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgOptCols);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate::Compare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_13()
{ 
	//Check if the provider supports IRowsetLocate interface.
	if(!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetLocate is not supported\n";
		return TEST_SKIPPED;
	}
	return testIRowsetLocateCompare();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate::GetRowsAt
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_14()
{ 
	//Check if the provider supports IRowsetLocate interface.
	if(!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetLocate is not supported\n";
		return TEST_SKIPPED;
	}
	return testIRowsetLocateGetRowsAt();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate::GetRowsByBookmark
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_15()
{  
	//Check if the provider supports IRowsetLocate interface.
	if(!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetLocate is not supported\n";
		return TEST_SKIPPED;
	}
	return testIRowsetLocateGetRowsByBkm();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate::Hash
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_16()
{ 
	//Check if the provider supports IRowsetLocate interface.
	if(!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetLocate is not supported\n";
		return TEST_SKIPPED;
	}
	return testIRowsetLocateHash();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_17()
{ 
	//Check if the provider supports IRowsetLocate interface.
	if(!GetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetFind is not supported\n";
		return TEST_SKIPPED;
	}
	return testFindNextRow();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc IRowsetScroll::GetApproximatePosition
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_18()
{ 
	//Check if the provider supports IRowsetScroll interface.
	if(!GetProperty(DBPROP_IRowsetScroll, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetScroll is not supported\n";
		return TEST_SKIPPED;
	}
	return testGetApproxPos();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc IRowsetScroll::GetRowsAtRatio
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_19()
{ 
	//Check if the provider supports IRowsetScroll interface.
	if(!GetProperty(DBPROP_IRowsetScroll, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
	{
		odtLog<<L"IRowsetScroll is not supported\n";
		return TEST_SKIPPED;
	}
	return testGetRowsAtRatio();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc IRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_20()
{ 
	return testIRowPosition();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc AggregateRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetNotUpdateable::Variation_21()
{ 
	return testAggregateRowset();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRowsetNotUpdateable::Terminate()
{ 
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(m_pIRowset);
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCRowsetUpdateable)
//*---------------------------------------------------------------------
//| Test Case:		TCRowsetUpdateable - Rowset Interfaces (Updateable Rowset)
//| Created:  	4/1/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetUpdateable::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{ 
		if(CHECK(m_hr = CreateDBSession(),S_OK))
		{
			CTable* pTable = NULL;

			//Required for using some ExtraLib functions without using
			//the ExtraLib classes.
			g_pIDBInitialize = m_pIDBInitialize;
			g_pIOpenRowset = m_pIOpenRowset;

			//Create table with MIN_ROWS rows.
			if(CreateTable(&pTable, MIN_ROWS))
			{
				SetTable(pTable, DELETETABLE_NO);

				LONG ulUpdateFlags = DBPROPVAL_UP_CHANGE|
									  DBPROPVAL_UP_DELETE|
									  DBPROPVAL_UP_INSERT  ;

				//Set the following properties.
				SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET,
					&m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;

				SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET,
					&m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;

				SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET,
					&m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;

				SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET,
					&m_cPropSets, &m_rgPropSets) ;

				SetProperty(DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET, 
					&m_cPropSets, &m_rgPropSets,
					(void*)VARIANT_TRUE, DBTYPE_BOOL, 
					DBPROPOPTIONS_OPTIONAL) ;

				SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
					&m_cPropSets, &m_rgPropSets,
					(void*)(LONG_PTR)ulUpdateFlags, DBTYPE_I4, //cast modified
					DBPROPOPTIONS_OPTIONAL) ;

				SetProperty(DBPROP_CANHOLDROWS,
					DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets) ;

				//Open a rowset with above properties set.
				m_hr=CreateOpenRowset(m_pTable, IID_IRowset,
					(IUnknown**)&m_pIRowset);

				if(FAILED(m_hr))
					DumpPropertyErrors(m_cPropSets, m_rgPropSets);

				if(m_hr==S_OK || m_hr==DB_S_ERRORSOCCURRED)
				{
					FreeProperties(&m_cPropSets, &m_rgPropSets);

					//ExtraLib global.
					g_ulNextRow = m_pTable->CountRowsOnTable() + 10;

					//Get the values of following properties.
					if(!GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET,
						m_pIRowset, &m_ulMaxOpenRows))
						m_ulMaxOpenRows = 0;

					if(GetProperty(DBPROP_CANFETCHBACKWARDS,
						DBPROPSET_ROWSET, m_pIRowset))
						m_fFetchBackwards = TRUE;

					return TRUE;	
				}
			}
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc IRowsetIdentity
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_1()
{
	return testIRowsetIdentity() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc IRowset (various properties set)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_2()
{
	return testIRowset() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc IRowsetChange::SetData
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_3()
{
	//Check if the provider supports IRowsetChange interface.
	if(GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		return testIRowsetChangeSet() ;

	odtLog<<L"IRowsetChange is not supported.\n";
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange::DeleteRows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_4()
{ 
	//Check if the provider supports IRowsetChange interface.
	if(GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		return testIRowsetChangeDelete() ;

	odtLog<<L"IRowsetChange is not supported.\n";
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange::InsertRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_5()
{ 
	//Check if the provider supports IRowsetChange interface.
	if(GetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
		(IUnknown*)m_pIRowset))
		return testIRowsetChangeInsert() ;

	odtLog<<L"IRowsetChange is not supported.\n";
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_6()
{ 
	return testIRowsetUpdate();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc IRowsetResynch
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_7()
{ 
	return testIRowsetResynch();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IRowsetRefresh
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRowsetUpdateable::Variation_8()
{ 
	return testIRowsetRefresh();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRowsetUpdateable::Terminate()
{ 
	BOOL bUseInsertWithParams = FALSE;
	ICommandWithParameters*	pICommandWithParameters=NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(m_pIRowset);
	DBCOUNTITEM ulRows = 0;

	//Determine if we are going to use insert with literals
	//or insert with parameters.
	if((!GetModInfo()->IsUsingITableDefinition() ||
		(GetModInfo()->IsUsingITableDefinition() && !SupportedProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pIDBInitialize,ROWSET_INTERFACE)) ||
		GetModInfo()->GetInsert()==INSERT_WITHPARAMS) &&
		GetModInfo()->GetInsert()!=INSERT_COMMAND &&
		m_pIDBCreateCommand &&
		(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandWithParameters,(IUnknown**)&pICommandWithParameters)==S_OK))
		bUseInsertWithParams = TRUE;

	SAFE_RELEASE(pICommandWithParameters);

	//Cleanup and refresh the table. Delete all the rows and
	//insert original rows again, hence restoring table to
	//a fresh state.
	if(m_pTable)
	{
		ulRows = m_pTable->GetRowsOnCTable();
		m_hr = m_pTable->DeleteRows(ALLROWS);
		if(m_hr == S_OK)
			for(DBCOUNTITEM i=0; i<ulRows; i++)
			{
				if(bUseInsertWithParams)
					CHECK(m_pTable->InsertWithParams(i+1, PRIMARY), S_OK);
				else
					CHECK(m_pTable->Insert(i+1, PRIMARY), S_OK);
			}
		m_pTable->DropTable();
	}
	SAFE_DELETE(m_pTable);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCCommand)
//*---------------------------------------------------------------------
//| Test Case:		TCCommand - Command Interfaces
//| Created:  	4/13/98
//*---------------------------------------------------------------------

//*---------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommand::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{
		HRESULT hr = S_OK;
		IDBCreateCommand* pIDBCreateCommand = NULL;

		// Get DB Session
		if(CHECK(hr = CreateDBSession(), S_OK))
		{
			if(m_pIDBCreateCommand)
			{
				if(CHECK(hr = m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,(IUnknown **)&m_pICommand),S_OK))
				{
					// Make sure DB Session returned is correct
					COMPARE(SUCCEEDED(hr=m_pICommand->GetDBSession(IID_IDBCreateCommand, (IUnknown **)&pIDBCreateCommand)),TRUE);
					if(hr==S_OK)
						COMPARE(VerifyEqualInterface(m_pIDBCreateCommand, pIDBCreateCommand), TRUE);
					SAFE_RELEASE(pIDBCreateCommand);

					//For ExtraLib functions.
					g_pIDBInitialize = m_pIDBInitialize;
					g_pIOpenRowset = m_pIOpenRowset;

					CTable* pTable = NULL;

					//Create table with MIN_ROWS rows.
					if(CreateTable(&pTable, MIN_ROWS))
					{
						GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &m_ulSQLSup);

						SetTable(pTable, DELETETABLE_NO);
						if(!m_pTable)
							return FALSE;

						//Create SQL stmt and set command text.
						if(!CHECK(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL,
							IID_ICommand, NULL, NULL, NULL, NULL, 
							EXECUTE_NEVER, 0, NULL, NULL, NULL, 
							&m_pICommand), S_OK))
							return FALSE;
						
						return TRUE;
					}
					else
					{
						odtLog<<L"INFO: Failure in CreateTable(...).\n";
					}
				}
			}
			else
			{
				COMPARE(m_pIDBInitialize != NULL, TRUE);
				COMPARE(m_pIOpenRowset != NULL, TRUE);
				odtLog<<L"WARNING: Commands not supported. SKIP all variations\n";
				return TEST_SKIPPED;
			}
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*---------------------------------------------------------------------
// @mfunc IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_1()
{ 
	return testCmdIAccessor() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*---------------------------------------------------------------------
// @mfunc IColumnsInfo::GetColumnInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_2()
{ 
	ICommandPrepare*	pICommandPrepare = NULL;

	if(VerifyInterface(m_pICommand,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		if(!CHECK(pICommandPrepare->Prepare(1), S_OK))
		{
			SAFE_RELEASE(pICommandPrepare);
			return TEST_FAIL;
		}
		SAFE_RELEASE(pICommandPrepare);
	}

	return testGetColumnInfo((IUnknown*)m_pICommand) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*---------------------------------------------------------------------
// @mfunc IColumnsInfo::MapColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_3()
{ 
	ICommandPrepare*	pICommandPrepare = NULL;

	if(VerifyInterface(m_pICommand,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		if(!CHECK(pICommandPrepare->Prepare(1), S_OK))
		{
			SAFE_RELEASE(pICommandPrepare);
			return TEST_FAIL;
		}
		SAFE_RELEASE(pICommandPrepare);
	}

	return testMapColumnIDs((IUnknown*)m_pICommand) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*---------------------------------------------------------------------
// @mfunc ICommandProperties::GetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_4()
{ 
	return testRowsetGetProp((IUnknown*)m_pICommand) ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*---------------------------------------------------------------------
// @mfunc ICommandProperties::SetProperties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_5()
{ 
	return testSetCmdProp() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*---------------------------------------------------------------------
// @mfunc IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_6()
{ 
	return testIConvertType() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*---------------------------------------------------------------------
// @mfunc ICommandText
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_7()
{ 
	return testICmdText() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*---------------------------------------------------------------------
// @mfunc ICommand::GetDBSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_8()
{ 
	return testICmdGetDBSession() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*---------------------------------------------------------------------
// @mfunc ICommand::Execute (set,exe,set,exe,set,exe)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_9()
{ 
	return testICmdExec1() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*---------------------------------------------------------------------
// @mfunc ICommand::Execute (set,exe,exe,exe)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_10()
{ 
	return testICmdExec2() ;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc IColumnsRowset::GetAvailableColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_11()
{ 
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	DBORDINAL			cOptCols = 0;
	DBID*				rgOptCols = NULL;
	ICommand*			pICommand = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICommand), S_OK)

	//Set the command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICommand, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	//If ICommandPrepare is supported, prepare the command. 
	if(VerifyInterface(pICommand,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		TESTC_(pICommandPrepare->Prepare(1), S_OK)
	}

	hr=GetAvailableColumns((IUnknown*)pICommand, &cOptCols, 
		&rgOptCols) ;

	if(hr==E_NOINTERFACE)
	{
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	else
		TESTC_(hr, S_OK)

	TESTC(CheckOptColumns(cOptCols, rgOptCols))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgOptCols);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICommand);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc IColumnsRowset::GetColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_12()
{ 
	TESTRESULT			tTestResult = TEST_FAIL;
	HRESULT				hr = S_OK;
	BOOL				bBookmark = FALSE;
	DBORDINAL			cOptCols = 0;
	DBID*				rgOptCols = NULL;
	ICommand*			pICommand = NULL;
	ICommandPrepare*	pICommandPrepare = NULL;
	IColumnsRowset*		pIColumnsRowset = NULL;
	IRowset*			pIRowset = NULL;

	//Create a command object.
	TESTC_(hr=m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
		(IUnknown**)&pICommand), S_OK)

	//Set the command text to select all from table.
	TESTC_(SetCommandText(m_pIMalloc, pICommand, m_pTable, NULL, eSELECT, 
		SELECT_ALLFROMTBL, NULL), S_OK)

	//If ICommandPrepare is supported, prepare the command. 
	if(VerifyInterface(pICommand,IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown**)&pICommandPrepare))
	{
		TESTC_(pICommandPrepare->Prepare(1), S_OK)
	}

	if(!VerifyInterface(pICommand,IID_IColumnsRowset,
		COMMAND_INTERFACE,(IUnknown**)&pIColumnsRowset))
	{
		odtLog<<L"IColumnsRowset is not supported.\n";
		tTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	TESTC(pIColumnsRowset != NULL)

	//Check for property DBPROP_BOOKMARKS on this command object.
	if(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
		(IUnknown*)pICommand))
		bBookmark = TRUE;

	TESTC_(hr=GetAvailableColumns((IUnknown*)pICommand, &cOptCols, 
		&rgOptCols), S_OK)

	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, cOptCols, rgOptCols,
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK)

	TESTC(CheckColumnsRowset((IUnknown*) pIRowset, cOptCols, bBookmark))

	tTestResult = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rgOptCols);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pICommand);
	return tTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ICommandPrepare::Prepare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_13()
{ 
	return testCmdPrep();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ICommandPrepare::Unprepare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_14()
{ 
	return testCmdUnprep();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ICommandWithParameters
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_15()
{ 
	return testICmdWParam();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc IMultipleResults
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommand::Variation_16()
{ 
	VARIANT		vVar;
	VariantInit(&vVar);

	if(!GetProperty(DBPROP_MULTIPLERESULTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &vVar))
	{
		odtLog<<L"Multiple Results are not supported.\n";
		return TEST_SKIPPED;
	}
	if(!(V_I4(&vVar) & DBPROPVAL_MR_SUPPORTED))
	{
		odtLog<<L"Multiple Results Objects are not supported.\n";
		return TEST_SKIPPED;
	}

	return testIMultipleResults();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*---------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCommand::Terminate()
{ 
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(m_pICommand);
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCOtherObjects)
//*-----------------------------------------------------------------------
//| Test Case:		TCOtherObjects - Other Objects
//| Created:  	6/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOtherObjects::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{ 
		if(CHECK(m_hr = CreateDBSession(), S_OK))
		{
			CTable* pTable = NULL;

			g_pIDBInitialize = m_pIDBInitialize;
			g_pIOpenRowset = m_pIOpenRowset;

			//Create table with MIN_ROWS rows.
			if(CreateTable(&pTable, MIN_ROWS))
			{
				SetTable(pTable, DELETETABLE_NO);
				if(m_pTable)
				{
					g_ulNextRow = m_pTable->CountRowsOnTable() + 10;
					return TRUE;
				}
			}
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ErrorInterfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOtherObjects::Variation_1()
{ 
	return testIError();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Transaction(Commit)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOtherObjects::Variation_2()
{ 
	return testTransactionCommit();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Transaction(Abort)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOtherObjects::Variation_3()
{ 
	return testTransactionAbort();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc StorageInterface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOtherObjects::Variation_4()
{ 
	return testStorageObj();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCOtherObjects::Terminate()
{ 
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBinder)
//*-----------------------------------------------------------------------
//| Test Case:		TCBinder - Binder Interfaces
//| Created:  	9/22/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBinder::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{ 
		TBEGIN
		BOOL	bTest;
		CTable*	pTable = NULL;
		IGetDataSource*	pIGetDataSource = NULL;

		//Get the root binder.
		bTest = CreateRootBinder();
		if(bTest != TEST_PASS)
			return bTest;
			
		TESTC(DefaultObjectTesting(m_pIBindResource, BINDER_INTERFACE))

		//bind to a session.
		TEST2C_(m_pIBindResource->Bind(NULL, m_pwszRowsetURL, 
			DBBINDURLFLAG_READ, DBGUID_SESSION, IID_IOpenRowset, 
			NULL, NULL, NULL, (IUnknown**)&m_pIOpenRowset), S_OK,
			DB_S_ERRORSOCCURRED)

		TESTC(VerifyInterface(m_pIOpenRowset,IID_IGetDataSource,
			SESSION_INTERFACE,(IUnknown**)&pIGetDataSource))

		//get a dso.
		TESTC_(pIGetDataSource->GetDataSource(IID_IDBInitialize,
			(IUnknown**)&m_pIDBInitialize), S_OK)

		g_pIDBInitialize = m_pIDBInitialize;
		g_pIOpenRowset = m_pIOpenRowset;

		TESTC(CreateTable(&pTable, 5))

		SetTable(pTable, DELETETABLE_NO);

CLEANUP:
		SAFE_RELEASE(pIGetDataSource);
		TRETURN
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IBindResource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBinder::Variation_1()
{ 
	TBEGIN 

	SAFE_RELEASE(m_pIRowset);  //Should be NULL anyway.

	//bind to a rowset.
	TEST2C_(m_pIBindResource->Bind(NULL, m_pwszRowsetURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
		NULL, NULL, NULL, (IUnknown**)&m_pIRowset), S_OK, 
		DB_S_ERRORSOCCURRED)
	TESTC(m_pIRowset != NULL)

	TESTC(DefaultObjectTesting(m_pIRowset, ROWSET_INTERFACE))

	//Get the values of following properties.
	if(!GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET,
		m_pIRowset, &m_ulMaxOpenRows))
		m_ulMaxOpenRows = 0;

	if(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, 
		m_pIRowset))
		m_fFetchBackwards = TRUE;

	TESTC(testIRowset())

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ICreateRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBinder::Variation_2()
{ 
	TBEGIN
	HRESULT		hr;
	ULONG_PTR	ulVal = 0;
	WCHAR*		pwszNewURL = NULL;
	IRow*		pIRow = NULL;
	ICreateRow*	pICreateRow = NULL;

	TESTC(VerifyInterface(m_pIBindResource,IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&pICreateRow))

	hr=pICreateRow->CreateRow(NULL, m_pwszRowsetURL, 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS, 
		DBGUID_ROW, IID_IRow, NULL, NULL, NULL, &pwszNewURL, 
		(IUnknown**)&pIRow);

	if(hr==S_OK || hr==DB_S_ERRORSOCCURRED)
	{
		TESTC(DefaultObjectTesting(pIRow, ROW_INTERFACE))
		TESTC(testIRow(pIRow))
		TESTC(pwszNewURL != NULL)
		COMPARE(wcscmp(pwszNewURL, m_pwszRowsetURL), 0);
	}
	else if(hr==DB_E_RESOURCEEXISTS)
		odtLog<<L"INFO: The provider does not support OPENIFEXISTS behaviour on ICreateRow.\n";
	else if(hr==DB_E_NOTSUPPORTED)
		odtLog<<L"INFO: The provider does not support ROW objects.\n";
	else if(hr==E_NOINTERFACE)
		odtLog<<L"INFO: ICreateRow is not supported.\n";
	else
		CHECK(hr, S_OK);

CLEANUP:
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pICreateRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IDBBinderProperties - Set rowset props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBinder::Variation_3()
{ 
	TBEGIN
	HRESULT					hr, hr2;
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	BSTR					pbstrCC = L"QuikTest - DBPROP_CURRENTCATALOG";
	IRowset*				pIRowset = NULL;
	IRowsetInfo*			pIRInfo = NULL;
	IGetDataSource*			pIGDS = NULL;
	IDBProperties*			pIDBProp = NULL;
	IRowsetIdentity*		pIRIdentity = NULL;
	IRowsetLocate*			pIRLocate = NULL;
	IDBBinderProperties*	pIDBBProp = NULL;

	VARIANT	vVar;
	VariantInit(&vVar);

	TESTC(VerifyInterface(m_pIBindResource,IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBProp))

	//Reset binder props.
	TESTC_(pIDBBProp->Reset(), S_OK)

	//Set the init props on binder.
	TESTC(GetInitProps(&cPropSets, &rgPropSets))

	//set a bunch of datasource and rowset props on the root
	//binder.

	SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, 
		&cPropSets, &rgPropSets, (void*)pbstrCC, DBTYPE_BSTR, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, 
		&cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET,
		&cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET,
		&cPropSets, &rgPropSets) ;

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, 
		&rgPropSets) ;

	TESTC_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	//bind to a rowset and verify props.
	TEST3C_(hr = m_pIBindResource->Bind(NULL, m_pwszRowsetURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
		NULL, NULL, NULL, (IUnknown**)&pIRowset), S_OK, 
		DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED)

	if(hr == DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"WARNING: Bind returned DB_E_ERRORSOCCURRED. This could be because setting rowset props is not supported through the Binder.\n";
		CHECKW(hr, S_OK);
		goto CLEANUP;
	}

	TESTC(pIRowset != NULL)

	TESTC(VerifyInterface(pIRowset,IID_IRowsetInfo,
		ROWSET_INTERFACE,(IUnknown**)&pIRInfo))

	//get the dso in order to verify the DATASOURCE props.
	TEST2C_(hr2=pIRInfo->GetSpecification(IID_IGetDataSource, (IUnknown**)&pIGDS), S_OK, S_FALSE)
	if(hr2 == S_OK)
		TESTC_(pIGDS->GetDataSource(IID_IDBProperties, (IUnknown**)&pIDBProp), S_OK)

	COMPARE(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, 
		pIRowset), TRUE);

	COMPARE(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, 
		pIRowset), TRUE);

	COMPARE(VerifyInterface(pIRowset,IID_IRowsetIdentity,
		ROWSET_INTERFACE,(IUnknown**)&pIRIdentity), TRUE);

	if(S_OK == hr)
	{
		COMPARE(GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
			pIRowset), TRUE);
		COMPARE(VerifyInterface(pIRowset,IID_IRowsetLocate,
			ROWSET_INTERFACE,(IUnknown**)&pIRLocate), TRUE);
		COMPARE(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
			pIRowset), TRUE);
		COMPARE(GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, 
			pIRowset), TRUE);

		if(pIDBProp)
		{
			COMPARE(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, 
				pIDBProp), TRUE);
			COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, 
				pIDBProp, &vVar), TRUE);
			COMPARE(wcscmp(V_BSTR(&vVar), pbstrCC), 0);
		}
	}
	else  // hr==DB_S_ERRORSOCCURRED
	{
		if(GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, 
			pIRowset))
			COMPARE(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, 
				pIRowset), TRUE);
		if(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, 
			pIDBProp, &vVar))
			COMPARE(wcscmp(V_BSTR(&vVar), pbstrCC), 0);
	}

CLEANUP:
	VariantClear(&vVar);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIRIdentity);
	SAFE_RELEASE(pIRLocate);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRInfo);
	SAFE_RELEASE(pIGDS);
	SAFE_RELEASE(pIDBProp);
	SAFE_RELEASE(pIDBBProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IDBBinderProperties - Reset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBinder::Variation_4()
{ 
	TBEGIN
	HRESULT					hr;
	ULONG					ulIndex; //small no.
	ULONG_PTR				ulMaxRows = 0;
	ULONG_PTR				ulUpdt = 0;
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	VARIANT					vrgDefault[4];
	IRowset*				pIRowset = NULL;
	IDBBinderProperties*	pIDBBProp = NULL;

	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantInit(&vrgDefault[ulIndex]);

	TESTC(VerifyInterface(m_pIBindResource,IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&pIDBBProp))

	//
	//STEP-1 : Obtain the default values of 4 rowset properties and 
	//store them in vrgDefault.
	//

	TESTC_(pIDBBProp->Reset(), S_OK)

	TESTC(GetInitProps(&cPropSets, &rgPropSets))

	TESTC_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	TEST2C_(m_pIBindResource->Bind(NULL, m_pwszRowsetURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
		NULL, NULL, NULL, (IUnknown**)&pIRowset), S_OK, 
		DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL)

	TESTC(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, 
		pIRowset, &vrgDefault[0]))

	TESTC(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, 
		pIRowset, &vrgDefault[1]))

	if(!GetProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET, 
		pIRowset, &vrgDefault[2]))
		VariantInit(&vrgDefault[2]);

	if(!GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, 
		pIRowset, &vrgDefault[3]))
		VariantInit(&vrgDefault[3]);

	SAFE_RELEASE(pIRowset);

	//
	//STEP-2 : Set the values of the 4 rowset properties. Then call
	//Reset(). After calling Reset(), Bind to a Rowset.
	//

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, 
		&rgPropSets) ;

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET,
		&cPropSets, &rgPropSets) ;

	SetProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET,
		&cPropSets, &rgPropSets, (void*)25, DBTYPE_I4, 
		DBPROPOPTIONS_OPTIONAL) ;

	SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET,
		&cPropSets, &rgPropSets, (void*)7, DBTYPE_I4, 
		DBPROPOPTIONS_OPTIONAL) ;

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK,
		DB_S_ERRORSOCCURRED)

	TESTC_(pIDBBProp->Reset(), S_OK)

	FreeProperties(&cPropSets, &rgPropSets);

	TESTC(GetInitProps(&cPropSets, &rgPropSets))

	TESTC_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	TEST2C_(m_pIBindResource->Bind(NULL, m_pwszRowsetURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
		NULL, NULL, NULL, (IUnknown**)&pIRowset), S_OK, 
		DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL)

	//
	//STEP-3 : Obtain the 4 rowset properties from the rowset. Verify
	//that their values match the default ones stored in vrgDefault.
	//

	if(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, 
		pIRowset))
		TESTC(V_BOOL(&vrgDefault[0]) == VARIANT_TRUE)
	else
		TESTC(V_BOOL(&vrgDefault[0]) == VARIANT_FALSE)

	if(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, 
		pIRowset))
		TESTC(V_BOOL(&vrgDefault[1]) == VARIANT_TRUE)
	else
		TESTC(V_BOOL(&vrgDefault[1]) == VARIANT_FALSE)

	if(GetProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET,
		pIRowset, &ulMaxRows))
		TESTC(((ULONG_PTR)V_I4(&vrgDefault[2])) == ulMaxRows)
	else
		TESTC(V_VT(&vrgDefault[2]) == VT_EMPTY)

	if(GetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET,
		pIRowset, &ulUpdt))
		TESTC(((ULONG_PTR)V_I4(&vrgDefault[3])) == ulUpdt)
	else
		TESTC(V_VT(&vrgDefault[3]) == VT_EMPTY)

CLEANUP:
	for(ulIndex=0; ulIndex<4; ulIndex++)
		VariantClear(&vrgDefault[ulIndex]);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBBProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBinder::Terminate()
{ 
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIDBInitialize);
	SAFE_RELEASE(m_pIBindResource);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCRow)
//*-----------------------------------------------------------------------
//| Test Case:		TCRow - Row Interfaces
//| Created:  	9/22/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRow::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CQuickTest::Init())
	// }}
	{ 
		TBEGIN
		ULONG_PTR	ulOleObj = 0;
		CTable* pTable = NULL;

		TESTC_(m_hr = CreateDBSession(),S_OK)

		//Required for using some ExtraLib functions.
		g_pIDBInitialize = m_pIDBInitialize;
		g_pIOpenRowset = m_pIOpenRowset;

		//Check if provider supports ROW Objects. If not, then SKIP
		//this test case.
		TESTC_PROVIDER(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
			m_pIDBInitialize, &ulOleObj))
		TESTC_PROVIDER((ulOleObj & DBPROPVAL_OO_ROWOBJECT) == 
			DBPROPVAL_OO_ROWOBJECT)

		//Create a table with MIN_ROWS rows.
		TESTC(CreateTable(&pTable, MIN_ROWS))

		SetTable(pTable, DELETETABLE_NO);
		TESTC(m_pTable != NULL)

		//Create the parent Rowset of the row objects used for testing 
		//in the variations.
		m_pCRowset = new CRowset();
		m_pCRowset->SetProperty(DBPROP_CANHOLDROWS);
		m_pCRowset->SetProperty(DBPROP_IRowsetIdentity);
		m_pCRowset->SetSettableProperty(DBPROP_IRowsetLocate);
		TESTC_(m_pCRowset->CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL,
			IID_IRowset, m_pTable),S_OK);

CLEANUP:
		TRETURN
	}
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_1()
{ 
	TBEGIN
	HRESULT				hr;
	DBCOUNTITEM			ulIndex = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	HROW				hRow = DB_NULL_HROW;
	HROW*				rghRows = NULL;
	IRowsetInfo*		pIRowsetInfo = NULL;
	IRowsetIdentity*	pIRIdentity = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		//Get first row and create a row object on it.
		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		TESTC(DefaultObjectTesting(RowObj.pIRow(), ROW_INTERFACE))

		//Veify the GetColumns method.
		TESTC(RowObj.VerifyGetColumns(ulIndex, m_pTable))

		//Call GetSourceRowset and verify that the correct parent 
		//is returned.
		TEST2C_(hr = RowObj.GetSourceRowset(IID_IRowsetInfo, (IUnknown**)&pIRowsetInfo,
			&hRow), S_OK, DB_E_NOSOURCEOBJECT)

		if(S_OK == hr)
		{
			TESTC(hRow != DB_NULL_HROW)
			TESTC(VerifyEqualInterface(pIRowsetInfo, m_pCRowset->pIRowset()))

			TESTC(VerifyInterface(pIRowsetInfo, IID_IRowsetIdentity,
				ROWSET_INTERFACE, (IUnknown**)&pIRIdentity))

			TESTC_(pIRIdentity->IsSameRow(hRow, rghRows[0]), S_OK)

			CHECK(m_pCRowset->ReleaseRows(1, &hRow),S_OK);
			hRow = DB_NULL_HROW;
		}

		CHECK(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK);
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pIRIdentity);
		SAFE_RELEASE(pIRowsetInfo);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRIdentity);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IGetSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_2()
{ 
	TBEGIN
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	IOpenRowset*	pIOR = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		TESTC_(RowObj.GetSession(IID_IOpenRowset, (IUnknown**)&pIOR), S_OK)

		TESTC(VerifyEqualInterface(pIOR, m_pIOpenRowset))

		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pIOR);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIOR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_3()
{ 
	TBEGIN
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;

	//Make sure these are NULL, although hey should be.
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRow);

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		m_pIRow = RowObj.pIRow();
		TESTC(testIConvertType())
		m_pIRow = NULL;

		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	m_pIRow = NULL;
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_4()
{ 
	TBEGIN
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		TESTC(testColInfo2(&RowObj))

		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IColumnsInfo2
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_5()
{ 
	TBEGIN
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	IColumnsInfo2*	pICI2 = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		if(ulIndex==1)
			TESTC_PROVIDER(VerifyInterface(RowObj.pIRow(), IID_IColumnsInfo2,
				ROW_INTERFACE, (IUnknown**)&pICI2))
		else
			TESTC(VerifyInterface(RowObj.pIRow(), IID_IColumnsInfo2,
				ROW_INTERFACE, (IUnknown**)&pICI2))

		TESTC(testColInfo2(&RowObj, TRUE))

		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pICI2);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pICI2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IGetRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_6()
{ 
	TBEGIN
	DBCOUNTITEM	ulIndex = 0;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		rghRows = NULL;
	WCHAR*		pwszURL = NULL;
	IGetRow*	pIGetRow = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	TESTC(VerifyInterface(m_pCRowset->pIRowset(), IID_IGetRow,
		ROWSET_INTERFACE, (IUnknown**)&pIGetRow))

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
		
		//The following tests the GetRowFromHROW method.

		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		TESTC(RowObj.VerifyGetColumns(ulIndex, m_pTable))

		//Now test the GetURLFromHROW method.

		TESTC_(pIGetRow->GetURLFromHROW(rghRows[0], &pwszURL), S_OK)
		TESTC((pwszURL != NULL) && (wcslen(pwszURL)>2))
		TESTC(wcsstr(pwszURL, L":") != NULL)

		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_FREE(pwszURL);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIGetRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc IGetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_7()
{ 
	TBEGIN
	HRESULT			hr;
	DBCOUNTITEM		ulIndex = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	DBID			dbid = DBROWCOL_DEFAULTSTREAM;
	IGetSession*	pIGS = NULL;
	IGetSourceRow*	pIGetSourceRow = NULL;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	for(ulIndex=1; ulIndex<=m_pCRowset->m_ulTableRows; ulIndex++)	
	{
		CRowObject RowObj;

		TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)

		TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
			rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

		TEST2C_(hr=RowObj.Open(NULL, &dbid, DBGUID_STREAM, IID_IGetSourceRow,
			(IUnknown**)&pIGetSourceRow), S_OK, DB_E_BADCOLUMNID)

		if(hr==S_OK)
		{
			TEST2C_(hr=pIGetSourceRow->GetSourceRow(IID_IGetSession, (IUnknown**)
				&pIGS), S_OK, DB_E_NOSOURCEOBJECT)

			if(hr==S_OK)
				TESTC(VerifyEqualInterface(pIGS, RowObj.pIRow()))
			else
				TESTC(!pIGS)
		}
	
		TESTC_(m_pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK)
		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pIGS);
		SAFE_RELEASE(pIGetSourceRow);
	}

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIGS);
	SAFE_RELEASE(pIGetSourceRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IRowChange
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_8()
{ 
	TBEGIN
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	IRowChange*		pIRowChange = NULL;

	CRowObject RowObj;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
	
	TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
		rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

	TESTC_PROVIDER(VerifyInterface(RowObj.pIRow(), IID_IRowChange,
		ROW_INTERFACE, (IUnknown**)&pIRowChange))

	TESTC(testIRowChange(&RowObj))

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowChange);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc IRowSchemaChange
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRow::Variation_9()
{ 
	TBEGIN
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	IRowSchemaChange*	pIRowSchemaChange = NULL;

	CRowObject RowObj;

	TESTC_(m_pCRowset->RestartPosition(),S_OK)

	TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)
	
	TEST2C_(RowObj.CreateRowObject(m_pCRowset->pIRowset(), 
		rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS)

	TESTC_PROVIDER(VerifyInterface(RowObj.pIRow(), IID_IRowSchemaChange,
		ROW_INTERFACE, (IUnknown**)&pIRowSchemaChange))

	TESTC(testIRowSchemaChange(&RowObj))

CLEANUP:
	m_pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowSchemaChange);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRow::Terminate()
{ 
	if(m_pTable)
		m_pTable->DropTable();
	SAFE_DELETE(m_pTable);
	SAFE_DELETE(m_pCRowset);
	ReleaseDBSession();
	ReleaseDataSourceObject();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CQuickTest::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



////////////////////////////////////////////////////////////////////////
// Test Driver
//
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// The test driver (main) function. This runs all the tests (and all 
// variations in them) when QuikTest is run as a stand-alone executable.
//
void __cdecl main(int argc, char* argv[])
{
	HRESULT		hr = S_OK ;
	CHAR*		pszCmdLine = NULL ;
	CHAR*		pszTemp = NULL;
	WCHAR*		pwszCmdLine = NULL ;
	WCHAR*		pwszProvName = NULL ;
	CLSID		clsidProvider ;
	ITestCases* pTC = NULL ;
	IError*		pIError = NULL;
	CBaseError*	pCBError = NULL;
	TESTRESULT	tResult = eVariationStatusNonExistent ; // VARIATION_STATUS

	VARIANT_BOOL	fResult=VARIANT_FALSE;
	LONG	dwResult=0;
	LONG	cTotalVars=0;
	LONG	cCaseCount=0;
	LONG	cVarCount=0;
	LONG	iTestNum=0;
	LONG	iVarNum=0;
	ULONG	cVarPassed=0;
	ULONG	cVarFailed=0;   //Total number of failures; not variations failed.
	ULONG	cVarSkipped=0;
	ULONG	cVarNonExistent=0;

	LONG	cVarErrors = 0;
	LONG	cVarWarnings = 0;
	LONG	cModErrors = 0;
	LONG	cModWarnings = 0;

	BSTR	bstrModuleName = NULL;
	BSTR	bstrModuleDesc = NULL;
	BSTR	bstrCaseName = NULL;
	BSTR	bstrCaseDesc = NULL;
	BSTR	bstrVarDesc = NULL;

	if(hr != OleInitialize(NULL)) goto CLEANUP ;
	SetUseIMallocSpy(FALSE) ;

	//Get the command line.
	pszCmdLine = GetCommandLineA() ;

	//Remove the first arg, which is the executable name.
	//What's left is the Init string.
	if(argc>=2 && argv && argv[0] && argv[1]) 
	{
		pszTemp = strstr(pszCmdLine, argv[0]);
		if(pszTemp)
			pszCmdLine = pszTemp + strlen(argv[0]) + 1 ;
	}
	else
	{
		OutputText(L"Expected at least 2 args on command line.\n");
		goto CLEANUP;
	}

	//Now the command line consists of only the Init string.
	pwszCmdLine = ConvertToWCHAR(pszCmdLine) ;
	if(pwszCmdLine==NULL)
	{
		OutputText(L"Failed to convert Command Line.\n");
		goto CLEANUP;
	}

	//Create a temporary CModInfo to use it's member function
	//GetInitStringValue to get the provider name from the 
	//init string. Donot check for its return value. It will return
	//false since we passed in a NULL.
	CreateModInfo(NULL);
	GetModInfo()->SetInitString(pwszCmdLine) ;
	if(!GetModInfo()->GetInitStringValue(L"PROVIDER", &pwszProvName))
	{
		OutputText(L"Failed to get the Provider Name.\n");
		OutputText(L"Include PROVIDER=... in the Init String.\n");
		goto CLEANUP;
	}
	ReleaseModInfo(NULL);
	//Destroyed the temporary CModInfo.

	//From provider name get class ID.
	if(FAILED(hr = CLSIDFromProgID(pwszProvName , &clsidProvider)))
		hr = CLSIDFromString(pwszProvName , &clsidProvider) ;

	if(FAILED(hr))
	{
		OutputText(L"Failed to get CLSID from Provider Name.\n");
		goto CLEANUP;
	}

	//Initialize the Global Module Data.
	SetGlobalModuleData() ;
	g_pThisTestModule->m_pwszInitString = SYSSTRING_ALLOC(pwszCmdLine) ;
	g_pThisTestModule->m_ProviderClsid = clsidProvider ;

	//This stmt is not required. It gets set to this by default.
	g_pThisTestModule->m_clsctxProvider = CLSCTX_INPROC_SERVER;

	//Create and initialize the CModInfo which will be used throughout
	//the test.
	if(!ModuleInit(g_pThisTestModule))
	{
		OutputText(L"ModuleInit failed.\n");
		goto CLEANUP;
	}

	//Create an error class to be used when run as a stand-alone 
	//executable.
	pCBError = new CBaseError;
	if(pCBError == NULL)
	{
		OutputText(L"Failed to create an Error Object.\n");
		goto CLEANUP;
	}
	//Get the IError interface from our error object.
	pCBError->QueryInterface(IID_IError, (void**)&pIError);
	if(pIError == NULL)
	{
		OutputText(L"Failed to get the IError interface.\n");
		goto CLEANUP;
	}
	//Set the obtained IError as our Error interface to use.
	GetModInfo()->GetErrorObject()->SetErrorInterface(pIError);

	//Get name and description of the module and print it alongwith 
	//the initialization string.
	g_pThisTestModule->GetName(&bstrModuleName);
	g_pThisTestModule->GetDescription(&bstrModuleDesc);
	OutputText(L" \n") ;
	OutputText(L"-------------------------------------------------------------------------------\n") ;
	OutputText(L"ModuleName:     %s\n",bstrModuleName) ;
	OutputText(L"ModuleDesc:     %s\n",bstrModuleDesc) ;
	OutputText(L"Initialization: %s\n",pwszCmdLine) ;
	OutputText(L"-------------------------------------------------------------------------------\n") ;

	//Get number of test cases.
	g_pThisTestModule->GetCaseCount(&cCaseCount);

	//Initialize error and warning counters.
	pCBError->ResetModErrors();
	pCBError->ResetModWarnings();

	//Loop through the Test Cases.
	for( iTestNum=0; iTestNum<cCaseCount; iTestNum++)
	{
		//Initialize error and warning counters.
		pCBError->ResetCaseErrors();
		pCBError->ResetCaseWarnings();

		//Get the test case.
		if(!CHECK(g_pThisTestModule->GetCase(iTestNum,&pTC), S_OK))
			goto CLEANUP;
		if(pTC==NULL)
		{
			OutputText(L"Failed to get test case (%d).\n", iTestNum+1);
			continue;
		}
		pTC->GetVariationCount(&cVarCount) ;
		pTC->GetName(&bstrCaseName);
		pTC->GetDescription(&bstrCaseDesc);
		OutputText(L" \n");
		OutputText(L"%s: %s.\n",bstrCaseName,bstrCaseDesc);

		//Increment total variation count by number of vars in this
		//test case.
		cTotalVars += cVarCount;

		//Call the Init() function of this test case. If fails, goto
		//terminate().
		pTC->Init(&dwResult);
		if(dwResult == (LONG)TEST_FAIL)
		{
			//If Init fails, then all variations within that TestCase
			//are counted as failed.
			cVarFailed += cVarCount;
			OutputText(L"Init failed.\n");
			goto TERMINATE;
		}
		else if(dwResult == (LONG)TEST_SKIPPED)
		{
			//If Init is skipped, then all variations within that 
			//TestCase are counted as skipped.
			cVarSkipped += cVarCount;
			//Skip all variations in this test case.
			goto TERMINATE;
		}

		//Loop through the variations.
		for( iVarNum=0; iVarNum<cVarCount; iVarNum++)
		{
			//Initialize error and warning counters.
			pCBError->ResetVarErrors();
			pCBError->ResetVarWarnings();

			pTC->GetVariationDesc(iVarNum, &bstrVarDesc);
			OutputText(L"\tVariation (%d) %s.\n",iVarNum+1, bstrVarDesc);
			tResult = eVariationStatusNonExistent;

			//Execute the variation. 
			try
			{
				pTC->ExecuteVariation(iVarNum, &tResult) ;
			}
			catch(...)
			{
				OutputText(L"\tEXCEPTION thrown.\n");
				tResult = TEST_FAIL;
			}

			//Get error and warning count of this variation.
			pCBError->GetVarErrors(&cVarErrors);
			pCBError->GetVarWarnings(&cVarWarnings);

			if(cVarErrors > 0)
				tResult = TEST_FAIL;

			//Display Result of the variation.
			switch(tResult)
			{
			case TEST_PASS:
				if(cVarWarnings > 0)
					OutputText(L"\tWARNING\n");
				else
					OutputText(L"\tPASS\n");
				cVarPassed++;
				break;
			case TEST_FAIL:
				OutputText(L"\tFAIL ******** FAIL\n");
				if(cVarErrors==0)
					cVarFailed++;
				else
					cVarFailed += cVarErrors;
				break;
			case TEST_SKIPPED:
				OutputText(L"\tSKIPPED\n");
				cVarSkipped++;
				break;
			default:
				OutputText(L"\tNON-EXISTENT\n");
				cVarNonExistent++;
				break;
			}//switch
			OutputText(L"\n");
			SAFE_SYSFREE(bstrVarDesc);
		}

TERMINATE:
		//Call the terminate() function of this test case.
		pTC->Terminate(&fResult) ;
		SAFE_SYSFREE(bstrCaseName);
		SAFE_SYSFREE(bstrCaseDesc);
		SAFE_RELEASE(pTC);
		cVarCount = 0;
	}

	//Get error and warning count of entire module.
	pCBError->GetModErrors(&cModErrors);
	pCBError->GetModWarnings(&cModWarnings);

	OutputText(L" \n");
	OutputText(L"-------------------------------------------------------------------------------\n") ;
	OutputText(L"FAILURES     = %d\t(Total number of errors in entire test)\n", cVarFailed);
	OutputText(L"Warnings     = %d\t(Total number of warnings in entire test)\n", cModWarnings);
	OutputText(L"Skipped      = %d\t(Number of variations skipped)\n", cVarSkipped);
	OutputText(L"Passed       = %d/%d\t(Number of variations passed and skipped / Number of variations run)\n", cVarPassed+cVarSkipped, cTotalVars);
	OutputText(L"Pass Percent = %d\t(percentage of above ratio)\n", (cVarPassed+cVarSkipped)*100/cTotalVars);
	OutputText(L"-------------------------------------------------------------------------------\n") ;

	//Terminate module. Releases CModInfo.
	if(!ModuleTerminate(g_pThisTestModule))
		OutputText(L"ModuleTerminate Failed.\n");

CLEANUP:
	SAFE_RELEASE(pIError);
	SAFE_FREE(pwszCmdLine);
	SAFE_FREE(pwszProvName);
	SAFE_SYSFREE(bstrModuleName);
	SAFE_SYSFREE(bstrModuleDesc);
	OleUninitialize() ;
}

//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module irowinfo.cpp | Source file for test module IRowsetInfo
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "irowinfo.h"
#include "msdasql.h"
#include <stdarg.h>


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x779ee042, 0x9239, 0x11cf, { 0xb4, 0xb2, 0x00, 0xaa, 0x00, 0xbb, 0xba, 0x1c }};
DECLARE_MODULE_NAME("IRowsetInfo");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test IRowsetInfo");
DECLARE_MODULE_VERSION(829078083);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

/////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////
#define MAX_MULTI_ROWSETS 10

// Constant DBPROP Values
DBPROPID DBPROP_NOTSUPPORTEDPROPERTY;
DBPROPID DBPROP_NOTSUPPORTEDINTERFACE;
DBPROPID DBPROP_NOTSETTABLEPROPERTY;
DBPROPID DBPROP_SETTABLEPROPERTY;

// NOTE:
//  When you remove these defines, please go down to the case statement in
//  CreateRowsetObjectWithInfo (just search for "case USE_GETCOLROWSET:") and
//  remove the commented out block - since these are all the same value
//  it was necessary to do this.  The code was left in
//
#define USE_GETCOLROWSET  ((EQUERY) 9999) // fake EQUERY

//  ENUMSOURCES
//  Apparently, EnumerateSources has been removed V1.
//  TODO Enumertor object is now inplememted, need to update test!
#ifdef ENUMSOURCES
#define USE_ENUMSOURCES   SELECT_EMPTYROWSET // TODO for now (this one doesn't crash)
#endif

//  Increment the error counter
#define RECORD_FAILURE ((*m_pError)++)

//  If commands are not supported, certain tests may not be able to be run
//  The following variable should be set at ModuleInit time and will be
//  queried by those variations which require commands, so that the test
//  will not record a failure when the provider gives a NOT SUPPORTED error.
//
//  Bookmarks are so commonly used that it too gets its own global variable.
BOOL g_fBookmarksOnByDft;
BOOL g_fBookmarksSettable;
BOOL g_fBookmarksSupported;
BOOL g_fSchemaRowSupported;
BOOL g_fColumnRowSupported;


//  Strings for DBPROPERTYERROR values.
//  This is an ordered enumeration starting at 0, so a straight index
//  into the array can be used...
//
const WCHAR *g_szPropertyErrorStrings[] = {
	L"OK",
	L"NOT SUPPORTED",
	L"BAD VALUE",
	L"BAD OPTION",
	L"BAD COLUMN",
	L"NOT ALL SETTABLE",
	L"NOT SETTABLE",
	L"NOT SET",
	L"CONFLICTING"
};


typedef struct tagDBEXPECTEDPROPERTY {
	DBPROPID            dwPropertyID;
	VARIANT             vValue;
	DBID                colid;
	DBPROPOPTIONS       dwOptions;
	LPCWSTR             szPropertyName;
	BOOL                bCheckValue;
	ULONG               uLine;
} DBEXPECTEDPROPERTY;

#define PROPERTY_NOT_FOUND 0xFFFFFFFF

// Will be in the 2.0 Leveling Spec.
#define NUM_MANDATORY_PROPERTIES 5
#define ADD_MANDATORY_PROPERTIES(var,idx) \
	(ADD_TRUE_PROP(&var[(idx)+0], DBPROP_IAccessor), \
	 ADD_TRUE_PROP(&var[(idx)+1], DBPROP_IRowsetInfo), \
	 ADD_TRUE_PROP(&var[(idx)+2], DBPROP_IColumnsInfo), \
	 ADD_TRUE_PROP(&var[(idx)+3], DBPROP_IConvertType), \
	 ADD_TRUE_PROP(&var[(idx)+4], DBPROP_IRowset))
DBEXPECTEDPROPERTY g_rgMandatoryInterfaces[NUM_MANDATORY_PROPERTIES];

//--------------------------------------------------------------------
// @func Sets up the data in a Boolean property entry for SetProperty
//    or to compare the results of GetProperty
//
// The version which sets up an expected property is primarily set
// via macro, so no default exists for its value argument.
//
// @rdesc None
//
//--------------------------------------------------------------------
void SetupBooleanProperty
(
   DBEXPECTEDPROPERTY *pEntry,              // @cparm [in out]: Entry to set
   const DBPROPID &dwPropertyID,            // @cparm [in]: Property to set
   VARIANT_BOOL bValue,                     // @cparm [in]: Value
   WCHAR *szPropertyName,                   // @cparm [in]: Print string for logging
   ULONG uLine                              // @cparm [in]: Where it came from
)
{
	pEntry->dwPropertyID = dwPropertyID;
	pEntry->vValue.vt = VT_BOOL;
	V_BOOL(&pEntry->vValue) = bValue;
	pEntry->szPropertyName = szPropertyName;
	pEntry->bCheckValue = TRUE;
	pEntry->colid = DB_NULLID;
	pEntry->dwOptions = DBPROPOPTIONS_REQUIRED;
	pEntry->uLine = uLine;
}

void SetupBooleanPropertyOptional
(
   DBEXPECTEDPROPERTY *pEntry,              // @cparm [in out]: Entry to set
   const DBPROPID &dwPropertyID,            // @cparm [in]: Property to set
   VARIANT_BOOL bValue,                     // @cparm [in]: Value
   WCHAR *szPropertyName,                   // @cparm [in]: Print string for logging
   ULONG uLine                              // @cparm [in]: Where it came from
)
{
	pEntry->dwPropertyID = dwPropertyID;
	pEntry->vValue.vt = VT_BOOL;
	V_BOOL(&pEntry->vValue) = bValue;
	pEntry->szPropertyName = szPropertyName;
	pEntry->bCheckValue = TRUE;
	pEntry->colid = DB_NULLID;
	pEntry->dwOptions = DBPROPOPTIONS_OPTIONAL;
	pEntry->uLine = uLine;
}

void SetupUndefinedProperty
(
   DBEXPECTEDPROPERTY *pEntry,              // @cparm [in out]: Entry to set
   const DBPROPID &dwPropertyID,            // @cparm [in]: Property to set
   WCHAR *szPropertyName,                   // @cparm [in]: Print string for logging
   ULONG uLine                              // @cparm [in]: Where it came from
)
{
	pEntry->dwPropertyID = dwPropertyID;
	pEntry->szPropertyName = szPropertyName;
	pEntry->bCheckValue = FALSE;
	pEntry->colid = DB_NULLID;
	pEntry->dwOptions = DBPROPOPTIONS_REQUIRED;
	pEntry->uLine = uLine;
}

void SetupUnsupportedProperty
(
   DBEXPECTEDPROPERTY *pEntry,              // @cparm [in out]: Entry to set
   const DBPROPID &dwPropertyID,            // @cparm [in]: Property to set
   WCHAR *szPropertyName,                   // @cparm [in]: Print string for logging
   ULONG uLine                              // @cparm [in]: Where it came from
)
{
	pEntry->dwPropertyID = dwPropertyID;
	pEntry->szPropertyName = szPropertyName;
	pEntry->bCheckValue = FALSE;
	pEntry->colid = DB_NULLID;
	pEntry->dwOptions = DBPROPOPTIONS_REQUIRED; 
	pEntry->uLine = uLine;
	pEntry->vValue.vt = VT_BOOL;
	V_BOOL(&pEntry->vValue) = VARIANT_TRUE; // All of the unsupported properties we choose at this time are boolean. May need to change...

}

#define ADD_BOOL_PROP(entry,prop,value) SetupBooleanProperty(entry,prop,value,L#prop,__LINE__)
#define ADD_TRUE_PROP_OPTIONAL(entry,prop) SetupBooleanPropertyOptional(entry,prop,VARIANT_TRUE,L#prop,__LINE__)
#define ADD_TRUE_PROP(entry,prop) SetupBooleanProperty(entry,prop,VARIANT_TRUE,L#prop,__LINE__)
#define ADD_FALSE_PROP(entry,prop) SetupBooleanProperty(entry,prop,VARIANT_FALSE,L#prop,__LINE__)
#define ADD_UNSUP_PROP(entry,prop) SetupUnsupportedProperty(entry,prop,L#prop,__LINE__)
#define ADD_UNDEF_PROP(entry,prop) SetupUndefinedProperty(entry,prop,L#prop,__LINE__)


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	DBPROP_NOTSUPPORTEDPROPERTY	= 0;
		
	// The following are the mandatory interfaces...
	ADD_MANDATORY_PROPERTIES(g_rgMandatoryInterfaces, 0);

	if(ModuleCreateDBSession(pThisTestModule))
		return TRUE;

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
	// Free the interface we got in ModuleCreateDBSession()
	return ModuleReleaseDBSession(pThisTestModule);
}

//--------------------------------------------------------------------
// @func Is a given property included in a property list?
//
//    Searches a property list returned by IRowsetInfo::GetProperties
//    to determine if a given property is included in the list.
//
//    Currently, the implementation is a linear search.  While not
//    the most efficient, there may not be a great enough number
//    of searches to justify spending much time improving the
//    algorithm.
//
//    COMMENT: Necessary when cProperties is 0 to GetProperties.
//    Not clear if when cProperties is > 0, the properties come back
//    in the same order as they are requested in GetProperties.
//
// @rdesc Returns TRUE if the property is found, false otherwise.
//    If lPropSetIndex is not NULL, it will be set to the property
//    set index of the set containing the property; if lPropIndex is
//    not NULL it will be set to the index within the property set
//    which contains the related property information; if ppProperty
//    is not NULL it is set to a pointer to the matched property.
//
//    If the property is not found, *lPropSetIndex and *lPropIndex
//    will be set to PROPERTY_NOT_FOUND and *ppProperty will be set
//    to NULL.
//
//    In some situations the caller cares merely that a match exists,
//    but does not care about the match data: in these cases, the caller
//    may safely pass NULL as the out arguments.
//
//    CHANGED FOR M8:  The returned property is no longer a list of properties
//    but a list of property lists.  For this test all desired properties fall
//    in the same list.
//
//--------------------------------------------------------------------
BOOL IsPropertyInList
(
   const DBPROPID           dwPropertyID,     // @cparm [in]: Property to look for
   const DBPROPSET          *prgProperties,   // @cparm [in]: Property list
   ULONG                    cNumProperties,   // @cparm [in]: Number of properties in property list
   ULONG                    *lPropSetIndex,   // @cparm [out]: Which prop set contains property
   ULONG                    *lPropIndex,      // @cparm [out]: Which property in prop set
   DBPROP                   **ppProperty      // @cparm [out]: Actual property data
)
{
	//  Loop down the list, checking the property against each entry
	//  until a match is found.
	for (ULONG uIndex1=0; uIndex1 < cNumProperties; uIndex1++) 
	{
		for (ULONG uIndex2=0; uIndex2 < prgProperties[uIndex1].cProperties; uIndex2++) 
		{
			if( (dwPropertyID == prgProperties[uIndex1].rgProperties[uIndex2].dwPropertyID) &&
				(prgProperties[uIndex1].guidPropertySet == DBPROPSET_ROWSET) )
			{
				if (lPropSetIndex) 
					*lPropSetIndex = uIndex1;
				
				if (lPropIndex) 
					*lPropIndex = uIndex2;
				
				if (ppProperty) 
					*ppProperty = &prgProperties[uIndex1].rgProperties[uIndex2];
				
				return TRUE;
			}
		}
	}
	
	if (lPropSetIndex) 
		*lPropSetIndex = NULL;

	if (lPropIndex) 
		*lPropIndex = NULL;
	
	return FALSE;
}

//--------------------------------------------------------------------
// @func Remove a property entry from a property list.
//
//    When a property list is being checked for correctness,
//    each expected property is searched from the list returned
//    from IRowsetInfo::GetProperties.  Matching properties are
//    removed from the list.
//
//    When all required entries have been removed, the list should
//    be empty unless "Return all properties" was requested and
//    the Rowset interface defines its own properties.
//
//    Now its no longer a list of properties, but
//    a list of property lists.  The entry to be deleted is identified
//    by Set Index and Index within the set; as a programming safeguard
//    I pass a pointer to the property to be deleted and check against
//    the delete indeces.
//
//  @rdesc none
//--------------------------------------------------------------------
void RemoveProperty
(
   DBPROPSET *prgPropertyList, // @cparm [in out]: Property list
   ULONG     uIndexMatchSet,   // @cparm [in] Property set containing property to remove
   ULONG     uIndexToRemove    // @cparm [in] 0-based index of entry to remove
)
{
	// Subtract one from the count
	prgPropertyList[uIndexMatchSet].cProperties--;
	
	if (uIndexToRemove < prgPropertyList[uIndexMatchSet].cProperties) 
		memcpy(&prgPropertyList[uIndexMatchSet].rgProperties[uIndexToRemove],
				 &prgPropertyList[uIndexMatchSet].rgProperties[uIndexToRemove + 1],
				 sizeof(DBPROP) * (prgPropertyList[uIndexMatchSet].cProperties - uIndexToRemove));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum {STATE_TEXT_SET, STATE_PREPARED} enumSTATE;

// @class GenTblEntry (General Table Entry)  Support class used to encapsulate
// information found in the test specification tables.  Arrays of these table
// entries comprise the basis for the General tests found under GetProperties
// of the test specification; however, each table entry also includes a test
// for GetReferencedRowset and GetSpecification as well.
//
// A single table entry is one "column entry" of the tables found in the test
// specification.
class GenTblEntry {
  public:

	// @cmember SQL Statement on which to base rowset for this test
	enum EQUERY         m_fSQLQuery;

	// @cmember Properties to ask for before creating rowset
	UWORD               m_cPresetProperties;
	DBEXPECTEDPROPERTY *m_prgPresetProperties;

	// @cmember Additional property: Requested ID
	const GUID         *m_id;
	DBPROPID            m_idRequestedID;
	WCHAR              *m_szRequestedIDString;

	// @cmember State of the Command Object: either "Text Set" or "Prepared"
	enumSTATE           m_eCmdObjState;

	// @cmember Properties which are inquired via GetProperties
	ULONG               m_cInquiredProperties;
	DBEXPECTEDPROPERTY *m_prgInquiredProperties;

	// @cmember Returned value
	ULONG               m_cReturnedProperties;
	DBEXPECTEDPROPERTY *m_prgReturnedProperties;
	BOOL                m_bAllowMoreProperties;  // obsolete - ignored

	// @cmember Column to request from GetReferencedRowset
	ULONG               m_ulColumn;

	// @cmember HRESULT expected from GetReferencedRowset
	HRESULT             m_hrGetReferencedRowset;

	// @cmember IID used to request from GetSpecification
	const GUID         *m_idCommandObjectID;

	// @cmember HRESULT expected from GetSpecification
	HRESULT             m_hrGetSpecification;

	// @cmember String representing SQL Text (EQUERY name)
	// Entered automatically via TEST_TABLE_ENTRY macro
	// from m_szQuery argument; used to identify table
	// entry in test logs
	WCHAR              *m_pszSelectString;

	//
	// @cmember Source line of this table entry
	// Entered automatically via TEST_TABLE_ENTRY macro
	// used so tester can quickly located source of failures
	ULONG               m_uLine;
};

#define TEST_TABLE_ENTRY(a,b,c,d,e,f,g,h,i,j,k,l) \
  {a,\
   (UWORD) ((b) ? (sizeof(b) / sizeof(*((DBEXPECTEDPROPERTY *)(b)))) : 0),(b),\
   &c,d,L#d,e,\
   (ULONG) ((f) ? (sizeof(f) / sizeof(*((DBEXPECTEDPROPERTY *)(f)))) : 0),(f),\
   (ULONG) ((h) ? (sizeof(h) / sizeof(*((DBEXPECTEDPROPERTY *)(h)))) : 0),(h),\
   g,\
   i,j,\
   &k,l,\
   L#a, __LINE__}


// Forward declaration necessary.
class CSubRowset;

// @class CRowsetInfoSupport Base Class for all IRowsetInfo test cases
// provides intermediate functions to verify "universal behavior" of the
// IRowsetInfo functions (i.e., when error occurs, proper fields are reset,
// when successful, proper fields contain data); also provides functions
// which execute a repeated series of steps which constitute the test
// performed by several variations or by multiple table entries within
// a variation.
class CRowsetInfoSupport : public CRowsetObject {

   public:
		// @cmember Constructor
		CRowsetInfoSupport(LPWSTR wszTestCaseName) : CRowsetObject(wszTestCaseName)
		{
				// Initialize
			g_fBookmarksOnByDft   = FALSE;
			g_fBookmarksSettable  = FALSE;
			g_fBookmarksSupported = FALSE;
			g_fSchemaRowSupported = FALSE;
			g_fColumnRowSupported = FALSE;

			m_pszSQLStatement	  = NULL;
			m_pICreator			  = NULL;
			m_pICmdPrepare		  = NULL;
			m_pIRowsetInfo		  = NULL;
		};

		// @cmember Destructor
		virtual ~CRowsetInfoSupport(){};

		// @cmember Common base class initialization
		BOOL Init();
		// @cmember Common base class termination
		BOOL Terminate();

	   // @cmember Calls IRowsetInfo::GetProperties; verifies whenever an
	   // error code is returned, *pcProperties is set to 0 and *prgProperties
	   // is set to NULL.  All test case calls pass through here.
      HRESULT GetProperties
			(
			   ULONG		cProperties,    // @parm [IN] Count of properties for which values are requested
			   DBPROPIDSET	rgProperties[], // @parm [IN] Array of size cProperties containing GUIDs representing properties of interest
			   ULONG *		pcProperties,   // @parm [OUT] Number of properties returned
			   DBPROPSET ** prgProperties   // @parm [OUT] Returned properties and values
			);

	   // @cmember Calls IRowsetInfo::GetProperties
      HRESULT GetProperties
			(
			   ULONG		cProperties,    // @parm [IN] Count of properties for which values are requested
			   DBPROPID *	rgPropIDs,      // @parm [IN] Array of size cProperties containing GUIDs representing properties of interest
			   ULONG *		pcProperties,   // @parm [OUT] Number of properties returned
			   DBPROPSET ** prgProperties   // @parm [OUT] Returned properties and values
			);

	   // @cmember Calls IRowsetInfo::GetReferencedRowset; verifies whenever
		// an error code is returned, *ppReferencedRowset is set to NULL.  All
		// test case calls pass through here.
		HRESULT GetReferencedRowset
			(
			   DBORDINAL	     iOrdinal,        // @parm [IN] The bookmark or chapter column for which to get the related rowset
			   REFIID            riid,            // @parm [IN] ID of interface pointer to return in ppReferencedRowset
			   IUnknown          **ppReferencedRowset // @parm [OUT] Where to return interface pointer on rowset which interprets values from this column
			);

	   // @cmember Calls IRowsetInfo::GetSpecification; verifies whenever an
		// error code is returned, *pcSpecification is set to NULL.  All test
		// case code pass through here.  GetSpecification also always tests
		// the reference count of the creator.
		HRESULT GetSpecification
			(
			   REFIID               riid,           // @parm [IN] ID of interface on which to return pointer
			   IUnknown          ** ppSpecification // @parm [OUT] Object which created the rowset
			);

	   // @cmember Calls CreateRowsetObject and sets up the m_pIRowsetInfo
		// member field for later use by member functions.  If desired,
		// can specify properties to request on OPEN, but these must
		// be successful and the error return array is automatically checked.
		//
		HRESULT CreateRowsetObjectWithInfo
			 (
			  EQUERY             eQuery,		             //@parm [IN] Query to generate rowset with
			  REFIID			 riid,			             //@parm [IN] What type of rowset to create
			  ULONG              cProperties=0,           //@parm [IN] Number of properties to set
			  DBEXPECTEDPROPERTY *rgProperties=NULL,      //@parm [IN] Properties to set
			  enumSTATE          fPrepare=STATE_TEXT_SET  //@parm [IN] STATE_TEXT_SET or STATE_PREPARE
		  );

	   // @cmember Releases a rowset created with an Info object as
		// well as its Info object.
		void ReleaseRowsetObjectWithInfo(BOOL bReleaseEverything = TRUE);

		BOOL CheckPropertyValue
			(const DBEXPECTEDPROPERTY *pExpectedEntry,
			 const DBPROP             *pActualEntry);

		// @cmember Check the property list returned by GetProperties against
		// a set of expected properties
		ULONG ComparePropertyLists
			(DBPROPSET *prgReturnedProperties,  // @cparm [in]: Prop list from GetProperties.
			 ULONG cNumReturnedProperties,      // @cparm [in]: # entries in prgReturnedProperties
			 const DBEXPECTEDPROPERTY *prgRequiredProperties, // @cparm [in]: Prop. which must appear
			 ULONG cNumRequiredProperties,      // @cparm [in] Number of required properties
			 const DBEXPECTEDPROPERTY *prgPermittedProperties, // @cparm [in]: Prop. which MAY appear
			 ULONG cNumPermittedProperties,     // @cparm [in]: Number of permitted properties
			 ULONG uCompareFlags                // @cparm [in]: Flags: see function description
			 );

#define COMPAREPROP_CHECKVALUES     (0x0001)  // check value when a match is found
#define COMPAREPROP_DISALLOWEXTRAS  (0x0002)  // check that no other properties remain
#define COMPAREPROP_NODUPCHECK      (0x0004)  // check each match occurs only once
#define COMPAREPROP_LOGCHECKS       (0x0008)  // print a line for each property checked

		// @cmember Check the interface returned by GetReferencedRowset
		int CheckGetRRRetval(IUnknown *(&pRRRetval));

		// @cmember Check the interface returned by GetSpecification
		int CheckGetSpecRetval(IUnknown *(&pSpecRetval));

		// @cmember Dumps the errors CreateRowsetObject found on SetProperties
		void DumpPropertyErrors(ULONG cNumProperties,
										DBEXPECTEDPROPERTY *rgProperties,
										DBPROPSET *rgPropSet,
										ULONG &cNumErrors,
										ULONG &cNumNotSupported);


		// @cmember Creates an encapsulated rowset
		CSubRowset *MakeNewRowset(HRESULT *phr,
										  EQUERY fQuery,
										  ULONG cProperties = 0,
										  DBEXPECTEDPROPERTY *rgProperties = NULL);

		// @cmember Releases an encapsulated rowset created by MakeNewRowset
		void ReleaseRowset(CSubRowset *(&pRowset));

		// @cmember IRowsetInfo interface to the current rowset (in m_pIRowset)
		IRowsetInfo *	m_pIRowsetInfo;

		// @cmember Count of columns in current rowset
		DBORDINAL		m_cRowsetColumns;

		// @cmember Is there a bookmark column on this rowset.
		BOOL			m_fBookmarksActive;

		// @cmember Who created the rowset created by CreateRowsetObject
		// Used to check return value of GetSpecification
		IUnknown *		m_pICreator;

		// @cmember SQL Text for prepared statements
		WCHAR *			m_pszSQLStatement;

		// @cmember For prepared items, command object
		ICommandPrepare * m_pICmdPrepare;

   private:
};


// @class CSubRowset
//  For those test cases which require more than one rowset, CSubRowset provides
//  a mechanism for declaring multiple rowsets; each rowset gets the same methods
//  as the "implicit" rowset defined for each test case.   (For example, each
//  SubRowset in a set of multiple rowsets can call the RowsetInfoSupport version
//  of GetProperties, which performs certain return value validations).
//
//  CSubRowset looks to C++ like a test case because CRowsetInfoSupport inherits
//  from CTestCase; however, its purpose is not to provide test variations but
//  to support all the functionality encapsulated between CTestCase and a test
//  case class, in an instance where the test case must distinguish between
//  several rowset objects at the same time.
//
class CSubRowset : public CRowsetInfoSupport
{
 public:
	//@cmember CTOR
	CSubRowset(LPWSTR tcName) : CRowsetInfoSupport(tcName)
		{
			m_pReferencedRowset = NULL;
			m_pSpecification = NULL;
		};
	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();
	//@cmember Finds a row and optionally returns the hRow for that row

	//@cmember  Copies normally initialized by test framework test case info
	//into the encapsulated object which inherits from CTestCases.
	void	CopyTestCaseInfo(CTestCases * pTC);

	// @cmember: where to place the result
	IUnknown *m_pReferencedRowset;

	// @cmember: where to place the result
	IUnknown *m_pSpecification;
};

//--------------------------------------------------------------------
// @mfunc Init
// Call the parent Init routine.  CSubRowset has no member fields
// of its own to set up.
//
// Following the call of the parent Init routine, it is still necessary
// to copy test case information.
//--------------------------------------------------------------------
BOOL CSubRowset::Init()
{
	if(CRowsetInfoSupport::Init())
	{
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc CopyTestCaseInfo
// Copies the test case info from the given testcase into this sub-rowset
// object.  This is needed since CSubRowset inherits from CTestCases
// and may need to access some of the data members which are normally
// set in initialization of a testcase, which won't happen because
// we are encapsulating rather than inheriting from CRowsetInfoSupport.
//--------------------------------------------------------------------
void CSubRowset::CopyTestCaseInfo(CTestCases * pTC)
{
	SetOwningMod(0, pTC->m_pThisTestModule);
}

//--------------------------------------------------------------------
// @mfunc Terminate
// Call the parent Terminate routine.  CSubRowset has no member fields
// of its own to clean up.
//--------------------------------------------------------------------
BOOL CSubRowset::Terminate()
{
	return(CRowsetInfoSupport::Terminate());
}


//--------------------------------------------------------------------
// @mfunc Base class Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CRowsetInfoSupport::Init()
{
	if(CRowsetObject::Init())
	{
		HRESULT hr		 = E_FAIL;

		IDBSchemaRowset * pIDBSchemaRowset = NULL;
		IColumnsRowset *  pIColumnsRowset  = NULL;
	
		// Create a new DSO and Session
		CreateDBSession();

		// Create a table we'll use for the whole test module,
		m_pTable = new CTable(m_pIOpenRowset, (LPWSTR)gwszModuleName);
		if(!m_pTable)
			return FALSE;

		// Start with a table with one row
		if (FAILED(hr=m_pTable->CreateTable(10)))
			return FALSE;

		// Display the row number we'll use -- this is for reproing
		odtLog << L"The single row table is using insert seed " <<m_pTable->GetRowsOnCTable() << L" for this run." <<ENDL;

		// See if BOOKMARKS are supported
		CreateRowsetObject(USE_OPENROWSET, IID_IRowset);
		if(SupportedProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIDBInitialize))
		{
			g_fBookmarksSupported = TRUE;
		}
		else
		{
			DBPROP_NOTSUPPORTEDPROPERTY	= DBPROP_BOOKMARKS;
		}

		// See if the BOOKMARKS are settable
		if(SettableProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,m_pIDBInitialize))
			g_fBookmarksSettable = TRUE;

		// See if the default for BOOKMARKS is VARIANT_FALSE
		if(GetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,
				(m_pICommand) ? (IUnknown*)m_pICommand : (IUnknown*)m_pIAccessor))
			g_fBookmarksOnByDft = TRUE;

		if(!SupportedProperty(DBPROP_IStorage,DBPROPSET_ROWSET,m_pIDBInitialize))
		{
			DBPROP_NOTSUPPORTEDINTERFACE	= DBPROP_IStorage;
			DBPROP_NOTSUPPORTEDPROPERTY		= DBPROP_IStorage;
		}
		else
		{
			DBPROP_NOTSUPPORTEDINTERFACE  = 0;
		}
		
		// Fill in Constant DBPROP Values
		if(!SupportedProperty(DBPROP_ROWRESTRICT,DBPROPSET_ROWSET,m_pIDBInitialize))
		{
			DBPROP_NOTSUPPORTEDPROPERTY  = DBPROP_ROWRESTRICT;
		}

		if( (SupportedProperty(DBPROP_STRONGIDENTITY,DBPROPSET_ROWSET,m_pIDBInitialize)) &&
			(!SettableProperty(DBPROP_STRONGIDENTITY,DBPROPSET_ROWSET,m_pIDBInitialize)) )
			DBPROP_NOTSETTABLEPROPERTY  = DBPROP_STRONGIDENTITY;
		else
			DBPROP_NOTSETTABLEPROPERTY  = DBPROP_IConvertType;

		DBPROP_SETTABLEPROPERTY = DBPROP_MAXROWS;

		// Release the created objects
		ReleaseRowsetObject();

		// See if IDBSchemaRowset is supported
		if(VerifyInterface(m_pIOpenRowset, IID_IDBSchemaRowset, 
							SESSION_INTERFACE, (IUnknown **)&pIDBSchemaRowset))
			g_fSchemaRowSupported = TRUE;

		// See if IColumnsRowset is supported
		CreateRowsetObject(USE_OPENROWSET, IID_IColumnsRowset);
		if(VerifyInterface(m_pIAccessor, IID_IColumnsRowset, 
							ROWSET_INTERFACE, (IUnknown **)&pIColumnsRowset))
			g_fColumnRowSupported = TRUE;

		// Release the created objects
		ReleaseRowsetObject();

		SAFE_RELEASE(pIDBSchemaRowset);
		SAFE_RELEASE(pIColumnsRowset);

		return TRUE;
	}
	return FALSE;

}


//--------------------------------------------------------------------
// @mfunc Base Case Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CRowsetInfoSupport::Terminate()
{
	//Cleanup as much as we allocated in the base class init
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	// Clean up the Table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}
	
	return(CRowsetObject::Terminate());
}

//--------------------------------------------------------------------
// @mfunc Create an "encapsulated" rowset, so that test cases can
//   simultaneously process multiple rowsets with each rowset having
//   the functionality of the rowset which is provided as part of
//   all test cases inherited from CRowsetObject.
//
// @rdesc Pointer to the sub-rowset, which the caller is responsible
//   for freeing (by calling ReleaseRowset).  If the creation fails,
//   all resources are released and MakeNewRowset returns NULL.
//
CSubRowset* CRowsetInfoSupport::MakeNewRowset(HRESULT *phr, EQUERY fQuery,
	 ULONG cProperties, DBEXPECTEDPROPERTY *rgProperties)
{
	CSubRowset *pReturnRowset = NULL;

	*phr = S_OK;
	pReturnRowset = new CSubRowset(L"");
	
	if (pReturnRowset) 
	{
		pReturnRowset->CopyTestCaseInfo(this);
		if (pReturnRowset->Init()) 
		{
			*phr = pReturnRowset->CreateRowsetObjectWithInfo(fQuery, IID_IRowset,
    				 cProperties, rgProperties);

			if (*phr == S_OK)
				return pReturnRowset;
			
			//  This is coded so that if required properties are not set, no
			//  failure is logged; but if a failure is logged it will show
			//  we were expecting success.
			//
			if (*phr != DB_S_ERRORSOCCURRED) 
			{
				CHECK(*phr, S_OK);
				odtLog << L"** FAILURE **: Couldn't create sub-rowset" <<ENDL;
			}
					
			pReturnRowset->ReleaseRowsetObjectWithInfo();
		}
		pReturnRowset->Terminate();
		delete pReturnRowset;
		pReturnRowset = NULL;
	}
	
	return FALSE;
}

// @mfunc Release a rowset created by MakeNewRowset
//
// @rdesc none
//
void CRowsetInfoSupport::ReleaseRowset(CSubRowset *(&pRowset))
{
	if (pRowset) {
		pRowset->ReleaseRowsetObjectWithInfo();
		pRowset->Terminate();
		delete pRowset;
		pRowset = NULL;
	}
}


// @mfunc CGetRefRSParameters
// All of the GetReferencedRowset parameter tests are based on this class,
// which merely provides a member variable to hold the return value for
// the GetReferencedRowset call (this would otherwise be a local variable
// common to every test variation).

class CGetRefRSParameters : public CRowsetInfoSupport
{
   public:
		// @cmember Constructor
		CGetRefRSParameters(LPWSTR wszTestCaseName) : CRowsetInfoSupport(wszTestCaseName)
		{};

		// @cmember Destructor
		virtual ~CGetRefRSParameters(){};


   protected:
	   // @cmember: where to place the result
	   IUnknown *m_pReferencedRowset;

};


// @mfunc CGetSpecParameters
// All of the GetSpecification parameter tests are based on this class,
// which merely provides a member variable to hold the return value for
// the GetSpecification call (this would otherwise be a local variable
// common to every test variation).

class CGetSpecParameters : public CRowsetInfoSupport
{
   public:
		// @cmember Constructor
		CGetSpecParameters(LPWSTR wszTestCaseName) : CRowsetInfoSupport(wszTestCaseName)
		{};

		// @cmember Destructor
		virtual ~CGetSpecParameters(){};

   protected:
	   // @cmember: where to place the result
	   IUnknown *m_pSpecification;

};


HRESULT CRowsetInfoSupport::GetProperties
(	ULONG		 cPropertyIDSets,
	DBPROPIDSET	 rgPropertyIDSets[],
	ULONG *		 pcPropertySets,
	DBPROPSET ** prgPropertySets
)
{
	HRESULT hr = E_FAIL;

	//  Initialize to INVALID
	if (pcPropertySets) 
		*pcPropertySets = INVALID(ULONG);
	if (prgPropertySets) 
		*prgPropertySets = INVALID(DBPROPSET *);
	
	// Call GetProperties
	if (m_pIRowsetInfo)
		hr = m_pIRowsetInfo->GetProperties(cPropertyIDSets, rgPropertyIDSets, 
			pcPropertySets, prgPropertySets);

	// Check the return code
	if (FAILED(hr) && (hr != DB_E_ERRORSOCCURRED))
	{
		// Whenever GetProperties returns an error condition
		if ((pcPropertySets && *pcPropertySets) || (prgPropertySets && *prgPropertySets)) {
			odtLog << L"GetProperties failed but did not reset the return values." <<ENDL;
			RECORD_FAILURE;
		}
	}
	else 
	{
		// If *pcPropertySets is 0 on output, the provider does not allocate
		// any memory and ensures that *prgPropertySets is a NULL pointer on output.
		if (((!*pcPropertySets) && (*prgPropertySets)) || 
			((*pcPropertySets) && (!*prgPropertySets)))
		{
			odtLog << L"GetProperties returned values that were unexpected." <<ENDL;
			RECORD_FAILURE;
		}
		else 
		{
			// Check the PropSets returned
			for(ULONG i=0; i < *pcPropertySets; i++)
			{
				// Compare the values
				if(cPropertyIDSets)
					COMPARE(*pcPropertySets, cPropertyIDSets);

				if ((rgPropertyIDSets) && (rgPropertyIDSets[i].cPropertyIDs))
					COMPARE((*prgPropertySets)[i].cProperties, rgPropertyIDSets[i].cPropertyIDs);
				
				// Can only be Rowset GUID or Provider defined
				if (((*prgPropertySets)[i].guidPropertySet == DBPROPSET_COLUMN) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_DATASOURCE) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_DATASOURCEINFO) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_DBINIT) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_INDEX) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_PROPERTIESINERROR) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_SESSION) ||
					((*prgPropertySets)[i].guidPropertySet == DBPROPSET_TABLE))
				{
					odtLog << L"GetProperties returned the wrong PropertySet GUID" <<ENDL;
					RECORD_FAILURE;
				}
			}
		}
	}

	return hr;
}

HRESULT CRowsetInfoSupport::GetProperties(
	 ULONG			cProperties,
	 DBPROPID *		rgPropIDs,
	 ULONG *		pcProperties,
	 DBPROPSET **	prgProperties)
{
	DBPROPIDSET PropertySet;

	PropertySet.rgPropertyIDs   = rgPropIDs;
	PropertySet.cPropertyIDs    = cProperties;
	PropertySet.guidPropertySet = DBPROPSET_ROWSET; 
	return GetProperties(1, &PropertySet, pcProperties, prgProperties);
}

HRESULT CRowsetInfoSupport::GetReferencedRowset
   (DBORDINAL   iOrdinal,
    REFIID       riid,
	IUnknown **  ppReferencedRowset)
{
	HRESULT	hr = E_FAIL;

	//  Write garbage into the ppSpecification so we can be sure that the value
	//  was written by the function.
	if (ppReferencedRowset)
		*ppReferencedRowset = INVALID(IUnknown *);

	hr=m_pIRowsetInfo->GetReferencedRowset(iOrdinal, riid, ppReferencedRowset);

	// Whenever GetReferencedRowset returns an error condition,
	// ppReferencedRowset is to be set to NULL.
	if (FAILED(hr) && (ppReferencedRowset && *ppReferencedRowset))
		odtLog << L"GetReferencedRowset failed but did not reset *ppReferencedRowset" <<ENDL;

	return hr;
}

//--------------------------------------------------------------------
// @func Call GetSpecification
//
//  All calls to GetSpecification are routed through this function.
//  This function verifies, after the call and regardless of the return
//  status, any "absolutes" associated with the function:
//
//   *  If an error is returned and ppSpecification is specified,
//      *ppSpecification should be NULL.
//   *  If no error is returned and a value was returned in
//      *ppSpecification, the reference count of the creator should
//      be incremented.
// @rdesc HRESULT returned by GetSpecification.
//--------------------------------------------------------------------
HRESULT CRowsetInfoSupport::GetSpecification
   (REFIID      riid,
	 IUnknown ** ppSpecification)
{
	HRESULT	hr = E_FAIL;

	//  Write garbage into the ppSpecification so we can be sure that the value
	//  was written by the function.
	if (ppSpecification) {
		*ppSpecification = INVALID(IUnknown *);
	}

	hr=m_pIRowsetInfo->GetSpecification(riid, ppSpecification);

	// Whenever GetReferencedRowset returns an error condition,
	// ppReferencedRowset is to be set to NULL.
	if (FAILED(hr) && (ppSpecification && *ppSpecification)) {
		odtLog << L"GetSpecification failed but did not reset *ppSpecification" <<ENDL;
		RECORD_FAILURE;
	}

	// If the HR was S_FALSE print a message
	if (hr == S_FALSE) 
    {
		if (*ppSpecification)
        {
            COMPARE(0,1);
        }
		odtLog << L"GetSpecification did not return the object that created the rowset." <<ENDL;
	}

	return hr;
}

HRESULT CRowsetInfoSupport::CreateRowsetObjectWithInfo
(
    EQUERY         		eQuery,		      //@parm [IN] Query to generate rowset with
    REFIID				riid,			  //@parm [IN] What type of rowset to create
    ULONG               cProperties,      //@parm [IN] Number of properties to set
    DBEXPECTEDPROPERTY	*rgProperties,    //@parm [IN] Properties to set
    enumSTATE           fPrepare          //@parm [IN] STATE_TEXT_SET or STATE_PREPARED
)
{	
	HRESULT			hr				= S_OK;
	ULONG			cNumErrors		= 0;
	ULONG			ulNotSupported	= 0;
	BOOL			*fNotSupported	= NULL;
	DBPROPSET		PropertySet;
	DBPROP			*prgProperties  = NULL;
	IColumnsInfo	*pIColumnsInfo  = NULL;
	DBCOLUMNINFO	*rgInfo			= NULL;
	WCHAR			*pStringsBuffer = NULL;

	do 
	{
		//  These values cannot take commands.  It is as test error if this
		if (eQuery == USE_OPENROWSET || eQuery == USE_GETCOLROWSET || 
			eQuery == SELECT_DBSCHEMA_TABLE /*|| eQuery == USE_ENUMSOURCES */)
			fPrepare = STATE_TEXT_SET;

		//  Caller can ask that rowset be prepared.
		if(fPrepare == STATE_PREPARED)
		{
			//  Set the text from eQuery
			if(!CHECK(m_pTable->ExecuteCommand(eQuery,IID_IUnknown,NULL,NULL,
					NULL,NULL,EXECUTE_NEVER,0,NULL,NULL,NULL,&m_pICommand), S_OK))
				break;

			//  Prepare the Text
			if(VerifyInterface(m_pICommand, IID_ICommandPrepare,COMMAND_INTERFACE, (IUnknown **) &m_pICmdPrepare))
			{
				if (!CHECK(m_pICmdPrepare->Prepare(1), S_OK))
				{
					break;
				}
			}
		}

		// Set up the Property Structure
		if(cProperties > 0)
		{
			// Allocate memory and fill the property structure
			prgProperties	= (DBPROP *) PROVIDER_ALLOC(sizeof(DBPROP) * cProperties);

			fNotSupported	= (BOOL *) PROVIDER_ALLOC(sizeof(BOOL) * cProperties);
			memset(fNotSupported,0,sizeof(BOOL) * cProperties);


			for(ULONG uIndex = 0; uIndex < cProperties; uIndex++)
			{
				prgProperties[uIndex].dwPropertyID = rgProperties[uIndex].dwPropertyID;
				prgProperties[uIndex].vValue       = rgProperties[uIndex].vValue;
				prgProperties[uIndex].colid        = rgProperties[uIndex].colid;
				prgProperties[uIndex].dwOptions    = rgProperties[uIndex].dwOptions;
				prgProperties[uIndex].dwStatus     = 0;

				// Count the unsupported and unsettable properties
				if(!SupportedProperty(rgProperties[uIndex].dwPropertyID, DBPROPSET_ROWSET, m_pIDBInitialize))
				{
					fNotSupported[uIndex]	= TRUE;
				}
			}

			// Set up the Property Structure
			PropertySet.rgProperties = prgProperties;
			PropertySet.cProperties  = cProperties;
			PropertySet.guidPropertySet = DBPROPSET_ROWSET; 

			// Set the Property and free the memory
			if(!CHECK(hr=SetRowsetProperties(&PropertySet, 1), S_OK)) {
				PROVIDER_FREE(prgProperties);
				break;
			}
		}

		// Free the Properties structure
		PROVIDER_FREE(prgProperties);

		// Get the Rowset Object
		if(eQuery == USE_GETCOLROWSET) 
		{
			hr=CreateRowsetObject(USE_OPENROWSET, IID_IColumnsRowset, EXECUTE_IFNOERROR);
			hr=CreateColumnRowset();
		}
		else
		{
			// Check to see if the IID is supported
			if( ((riid == IID_IRowsetChange) && 
				 (SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,m_pIDBInitialize))) ||
				 ((riid == IID_IRowsetLocate) && 
				 (SupportedProperty(DBPROP_IRowsetLocate,DBPROPSET_ROWSET,m_pIDBInitialize))) )
				hr=CreateRowsetObject(eQuery, riid, EXECUTE_IFNOERROR);
			else
				hr=CreateRowsetObject(eQuery, IID_IRowset, EXECUTE_IFNOERROR);
		}

		if(m_rgPropSets)
		{
			//if a prop is not supported make sure it comes back as not supported
			for(ULONG uIndex = 0; uIndex < cProperties; uIndex++)
			{
				if(fNotSupported[uIndex])
				{
					COMPARE(m_rgPropSets->rgProperties[uIndex].dwStatus,DBPROPSTATUS_NOTSUPPORTED);
				}
			}
		}


		// Check to see if the Provider supports Views
		if((FAILED(hr)) && (eQuery == SELECT_REVCOLLISTFROMVIEW)) {
			odtLog<<L"Create view not supported" <<ENDL;
			hr = ResultFromScode(DB_S_ERRORSOCCURRED);
			break;
		}

		// Check the HRESULT to see if a Property FAILED
		if( hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED) 
		{
			if( PropertySet.cProperties )
			{
				odtLog << L"SetProperties status codes:" <<ENDL;

				// Private library's SetRowsetProperties makes a copy of the
				// PropertySet structure.
				DumpPropertyErrors(cProperties, rgProperties, m_rgPropSets, cNumErrors, ulNotSupported);
				
				if( cNumErrors || ulNotSupported )
				{
					if( cNumErrors )
					{
						RECORD_FAILURE;
						goto cleanup_rowset;
					}
					else if( ulNotSupported )
					{
//						hr = DB_S_ERRORSOCCURRED; // so rest of code sees this is a Not Supported error
						odtLog << L"One or more required properties are not supported or not settable " <<ENDL;
						odtLog << L"Rowset not created; test not run." <<ENDL <<ENDL;
						goto cleanup_rowset;
					}
				}
			}
		}

		// Check CreateRowsetObject results AFTER printing status.
		if(hr != S_OK)
			break;

		//  Need the number of columns in the rowset
		if(VerifyInterface(m_pIAccessor, IID_IColumnsInfo, 
							ROWSET_INTERFACE, (IUnknown **)&pIColumnsInfo))
		{
			CHECK(pIColumnsInfo->GetColumnInfo(&m_cRowsetColumns, 
											&rgInfo, &pStringsBuffer), S_OK);

			if(!m_cRowsetColumns) {
				odtLog << L"** FAILURE **: Unable to get number of columns." <<ENDL;
				RECORD_FAILURE;
			}

			// Subtract the Bookmark column
			if((rgInfo) && (!rgInfo->iOrdinal))
				m_cRowsetColumns--;

			// Release the objects
			SAFE_RELEASE(pIColumnsInfo);

			PROVIDER_FREE(rgInfo);
			PROVIDER_FREE(pStringsBuffer);
		}

		// The rowset we created is stored in Accessor, but the local tools
		// require the copy in m_pIRowsetInfo (so it doesn't always have
		// to figure out where CreateRowsetObject left the darn thing).  It
		// is NOT necessary to release the copy.
		if(VerifyInterface(m_pIAccessor, IID_IRowsetInfo, 
							ROWSET_INTERFACE, (IUnknown **) &m_pIRowsetInfo))
		{
			// Remember who created this rowset, so we can check the return
			// value of GetSpecification
			switch (eQuery) 
			{
				case USE_OPENROWSET:
				case USE_GETCOLROWSET:
				case SELECT_DBSCHEMA_TABLE:
					m_pICreator = m_pIOpenRowset;
				break;

//				case USE_ENUMSOURCES:
//					m_pICreator = m_pIDBCreateCommand;
//				break;
		 
				default:
					m_pICreator = m_pICommand;
			}

			return S_OK;
		}

	} while (FALSE);

 cleanup_rowset:

	// Cleanup from CreateRowsetObject, which succeeded.
	ReleaseRowsetObject();
	SAFE_RELEASE(m_pICmdPrepare);

	// If previous stuff was saved, do not release it
	// during this cleanup.
	if(m_pICommand)
		ReleaseCommandObject();

	return hr;
}

void CRowsetInfoSupport::ReleaseRowsetObjectWithInfo(BOOL bReleaseEverything)
{
	// Release the objects
	SAFE_RELEASE(m_pIRowsetInfo);
	SAFE_RELEASE(m_pColRowset);

	ReleaseRowsetObject();

	if(m_pICmdPrepare)
		CHECK(m_pICmdPrepare->Unprepare(), S_OK);
	
	SAFE_RELEASE(m_pICmdPrepare);

	if (bReleaseEverything) 
		 ReleaseCommandObject();
}

//--------------------------------------------------------------------
// @func Check a property value returned from IRowsetInfo::GetProperties
//       against the expected value.
//
//  @rdesc Success or failure
//       @flag  TEST_PASS | The value was as expected
//       @flag  TEST_FAIL | The value was not correct
//--------------------------------------------------------------------
BOOL CRowsetInfoSupport::CheckPropertyValue(const DBEXPECTEDPROPERTY *pExpectedEntry,const DBPROP *pActualEntry)
{
	// Compare the dwOptions
	if((pExpectedEntry->bCheckValue) &&
		!COMPARE(pExpectedEntry->dwOptions, pActualEntry->dwOptions))
		return FALSE;

	// Compare the colid's
	if(!CompareDBID(pExpectedEntry->colid, pActualEntry->colid)) 
	{
		odtLog <<L"** FAILURE **: COLID check failed for " 
				<<pExpectedEntry->szPropertyName <<ENDL;
		RECORD_FAILURE;
		return FALSE;
	}

	// Compare the value
	if (pExpectedEntry->bCheckValue) 
	{
		if (!CompareVariant((VARIANT *) &pActualEntry->vValue,
								  (VARIANT *) &pExpectedEntry->vValue)) 
		{
			odtLog << L"** FAILURE **: Incorrect value for " 
					<<pExpectedEntry->szPropertyName <<ENDL;
			RECORD_FAILURE;
			return FALSE;
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------
// @func Check a property list returned by IRowsetInfo::GetProperties
//       against the set of expected and permitted values.
//
//  The property list must contain every required property; if any
//  are missing, an error is reported.
//
//  The property list may contain one or more permitted properties;
//  if any permitted properties are missing no error is reported.
//
//  If COMPAREPROP_CHECKVALUES is specified in the uCompareFlags argument,
//  then property values will be checked against expected values
//  where possible.
//
//  If COMPAREPROP_DISALLOWEXTRAS is specified in the uCompareFlags argument,
//  then there can be no properties in the list other than the properties
//  which are either required or permitted.
//
//  It is the application's responsibility to free the property list
//  returned by IRowsetInfo::GetProperties.  This routine will free the
//  property list unless COMPAREPROP_KEEPPROPLIST is specified in the
//  uCompareFlags argument.
//
//  If COMPAREPROP_NODUPCHECK is specified in the uCompareFlags argument,
//  then the function does not make sure when a match is found, that it
//  does not appear in the list a second time.
//
//  If COMPAREPROP_LOGCHECKS is specified in the uCompareFlags argument,
//  then the function writes a line to the output log indicating it is
//  searching for a particular property.  This happens for some tests
//  where the primary purpose is checking for the correct properties;
//  the log will reflect that the properties were or weren't searched.
//  Where the primary purpose is some other behavior, this output would
//  just clutter the log.
//
//  @rdesc Success or failure
//       @flag  TEST_PASS | All test conditions passed
//       @flag  TEST_FAIL | One or more failures were reported.
//--------------------------------------------------------------------

ULONG CRowsetInfoSupport::ComparePropertyLists
(
   DBPROPSET*		  rgReturnedProperties,     // @cparm [in]: Prop list from GetProperties.
   ULONG              cNumReturnedProperties,    // @cparm [in]: # entries in rgReturnedProperties
   const DBEXPECTEDPROPERTY *prgRequiredProperties, // @cparm [in]: Properties which must appear
   ULONG              cNumRequiredProperties,    // @cparm [in] Number of required properties
   const DBEXPECTEDPROPERTY *prgPermittedProperties, // @cparm [in]: Properties which MAY appear
   ULONG              cNumPermittedProperties,   // @cparm [in]: Number of permitted properties
   ULONG              uCompareFlags              // @cparm [in]: Flags: see function description
)
{
	ULONG uMatchSetIndex = 0;
	ULONG uMatchIndex	 = 0;
	ULONG fErrorOccurred = FALSE;
	DBPROP *pMatchProp	 = NULL;

	ULONG uTestIndex;
	//  ** Required Properties **:
	//  Loop through and verify that each required property is present.
	//  If value checking is desired, check the value if a match is found.
	//  When a match is found, the property is removed from the list.
	//  A check is made to make sure the property is not listed twice
	//  (in case "DISALLOWEXTRAS" is not being checked).
	//  If a required entry is not found, it is a test error.
	for (uTestIndex=0; uTestIndex < cNumRequiredProperties; uTestIndex++) 
	{
		if (uCompareFlags & COMPAREPROP_LOGCHECKS)
			odtLog  << "   ...Checking for property "
					<< prgRequiredProperties[uTestIndex].szPropertyName <<ENDL;

		// Check to see if the Property is in the return list
		if (IsPropertyInList(prgRequiredProperties[uTestIndex].dwPropertyID,
									rgReturnedProperties, cNumReturnedProperties,
									&uMatchSetIndex, &uMatchIndex, &pMatchProp)) 
		{
			// Check the property value
			if ((uCompareFlags & COMPAREPROP_CHECKVALUES) && 
				(!CheckPropertyValue(&prgRequiredProperties[uTestIndex],pMatchProp)))
					fErrorOccurred = TRUE;

			RemoveProperty(rgReturnedProperties,uMatchSetIndex,uMatchIndex);

			if ((uCompareFlags & COMPAREPROP_NODUPCHECK) && 
				(IsPropertyInList(prgRequiredProperties[uTestIndex].dwPropertyID,
						rgReturnedProperties, cNumReturnedProperties,NULL, NULL, NULL)) )
			{
				//  Property was found twice!
				odtLog << L"** TEST FAILURE **: Required property found twice: "
					<< prgRequiredProperties[uTestIndex].szPropertyName <<ENDL;
				RECORD_FAILURE;
				fErrorOccurred = TRUE;
			}
		}
		else 
		{
			// error - required property not found in list
			odtLog << L"** TEST FAILURE **: Required property not found: "
				<< prgRequiredProperties[uTestIndex].szPropertyName <<ENDL;
			RECORD_FAILURE;
			fErrorOccurred = TRUE;
		}
	}

	//  ** Permitted Properties **:
	//  Loop through and determine whether each permitted property is present.
	//  If value checking is desired, check the value if a match is found.
	//  When a match is found, the property is removed from the list.
	//  A check is made to make sure the property is not listed twice
	//  (in case "DISALLOWEXTRAS" is not being checked).
	//  If a permitted entry is not found, it is *NOT* a test error.
	for (ULONG uTestIndex1=0; uTestIndex1 < cNumPermittedProperties; uTestIndex1++) 
	{
		if (uCompareFlags & COMPAREPROP_LOGCHECKS)
			odtLog  << "   ...Checking for property "
					<< prgPermittedProperties[uTestIndex1].szPropertyName <<ENDL;

		if (IsPropertyInList(prgRequiredProperties[uTestIndex1].dwPropertyID,
									rgReturnedProperties, cNumReturnedProperties,
									&uMatchSetIndex, &uMatchIndex, &pMatchProp)) 
		{
			// Check the property value
			if ((uCompareFlags & COMPAREPROP_CHECKVALUES) && 
				(!CheckPropertyValue(&prgPermittedProperties[uTestIndex1],pMatchProp)))
					fErrorOccurred = TRUE;

			RemoveProperty(rgReturnedProperties,uMatchSetIndex,uMatchIndex);

			if ((!(uCompareFlags & COMPAREPROP_NODUPCHECK)) && 
				(IsPropertyInList(prgPermittedProperties[uTestIndex1].dwPropertyID,
						rgReturnedProperties, cNumReturnedProperties,NULL, NULL, NULL)) )
			{
				//  Property was found twice!
				odtLog << L"** TEST FAILURE **: Permitted property found twice: "
					<< prgRequiredProperties[uTestIndex].szPropertyName <<ENDL;
				RECORD_FAILURE;
				fErrorOccurred = TRUE;
			}
		}
	}

	//  If the list of expected entries are all that is allowed,
	//  then each property should have been removed from the list.
	//  If any remain, it is a test error.
	if (uCompareFlags & COMPAREPROP_DISALLOWEXTRAS) 
	{
		if ((cNumReturnedProperties > 1) || 
			(rgReturnedProperties[0].cProperties > 0))
		{
			// error - extra properties returned
			odtLog << L"** TEST FAILURE **: List contains "
				<< cNumReturnedProperties 
				<< L" prohibited additional entries." <<ENDL;
			fErrorOccurred = TRUE;
		}
	}

	// Free the Properties
	FreeProperties(&cNumReturnedProperties, &rgReturnedProperties);

	if (fErrorOccurred)
		return TEST_FAIL;
	else
		return TEST_PASS;
}

//--------------------------------------------------------------------
// @func Print the list of properties and the associated error code
//       from the array returned by SetProperties.
//
//  @rdesc None
//--------------------------------------------------------------------
void CRowsetInfoSupport::DumpPropertyErrors
(
    ULONG cNumProperties,
    DBEXPECTEDPROPERTY *rgProperties,
    DBPROPSET *rgPropSet,
    ULONG &cNumErrors,
    ULONG &cNumNotSupported
)
{	
	ULONG uIndex;
	WCHAR szTmpBuf[100];
	DBPROP *pProperty;

   cNumErrors = 0;
	cNumNotSupported = 0;

	for (uIndex = 0; uIndex < cNumProperties; uIndex++) 
	{
		pProperty = &rgPropSet->rgProperties[uIndex];
		if (pProperty->dwStatus != DBPROPSTATUS_OK) 
		{
			if (pProperty->dwStatus == DBPROPSTATUS_NOTSUPPORTED || 
				 pProperty->dwStatus == DBPROPSTATUS_NOTSETTABLE ) 
			{
				cNumNotSupported++;
			}
			else 
			{
				cNumErrors++;
			}
		}

		swprintf(szTmpBuf, L"   ...#%u: %-20s : %s\n",
					uIndex,
					rgProperties[uIndex].szPropertyName,
					g_szPropertyErrorStrings[pProperty->dwStatus]);
		odtLog << szTmpBuf;
	}
}

//--------------------------------------------------------------------
// @func Check the return value from GetReferencedRowset
//
//  This function also frees the value.
//
// @rdesc TEST_PASS or TEST_FAIL
//--------------------------------------------------------------------
BOOL CRowsetInfoSupport::CheckGetRRRetval(IUnknown *(&pRRRetval))
{
	BOOL	  fSuccess	  = FALSE;
	IUnknown *pIUnkActual = NULL;
	IUnknown *pIUnkExpect = NULL;

	// Get IUnknown pointers and compare them
	VerifyInterface(pRRRetval,IID_IUnknown,ROWSET_INTERFACE,&pIUnkActual);
	VerifyInterface(m_pIAccessor,IID_IUnknown,ROWSET_INTERFACE,&pIUnkExpect);
	if(COMPARE(pIUnkActual, pIUnkExpect))
		fSuccess = TRUE;

	SAFE_RELEASE(pIUnkExpect);
	SAFE_RELEASE(pIUnkActual);
	SAFE_RELEASE(pRRRetval);

	return fSuccess;
}

//--------------------------------------------------------------------
// @func Check the return value from GetSpecification
//
//  This function also frees the value.
//
// @rdesc TEST_PASS or TEST_FAIL
//--------------------------------------------------------------------
int CRowsetInfoSupport::CheckGetSpecRetval(IUnknown *(&pSpecRetval))
{
	BOOL	  fSuccess	  = FALSE;
	IUnknown *pIUnkActual = NULL;
	IUnknown *pIUnkExpect = NULL;

	// Check the pointer and return code
	if((!pSpecRetval) && (m_hr == S_FALSE)) {
		odtLog << L"The Provider did not return a object pointer" <<ENDL;
		return TRUE;
	}

	// Get IUnknown pointers and compare them
	VerifyInterface(pSpecRetval,IID_IUnknown,ROWSET_INTERFACE,&pIUnkActual);
	VerifyInterface(m_pICreator,IID_IUnknown,ROWSET_INTERFACE,&pIUnkExpect);
	if(COMPARE(pIUnkActual, pIUnkExpect))
		fSuccess = TRUE;

	SAFE_RELEASE(pIUnkExpect);
	SAFE_RELEASE(pIUnkActual);
	SAFE_RELEASE(pSpecRetval);

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCGeneral)
//--------------------------------------------------------------------
// @class General (see table in test spec
//
class TCGeneral : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember Handles the GetProperties part of each table cell
	BOOL TestPropertiesPart(GenTblEntry *pTableEntry);

	// @cmember Handles the GetReferencedRowset part of each table cell
	BOOL TestRefRowsetPart(GenTblEntry *pTableEntry);

	// @cmember Handles the GetSpecification part of each table cell
	BOOL TestSpecificationPart(GenTblEntry *pTableEntry);

	// @cmember Handles an individual table cell (one column entry) (see the test plan)
	int TestTableEntry(GenTblEntry *pTableEntry);

	// @cmember Tests a set of table entries (group of column entries) (see the test plan)
	// Calls TestTableEntry on each one; returns TEST_FAIL if one or more do not pass
	int TestATable(GenTblEntry *pTestTable, UWORD cNumElements);

	// @cmember Used as a placeholder into which results are read
	IUnknown *m_pReferencedRowset;

	// @cmember Used as a placeholder into which results are read
	IUnknown *m_pSpecification;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGeneral,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember ICommand::Execute(All table entries)
	int Variation_1();
	// @cmember ICommand::Execute(Implied; All table entries)
	int Variation_2();
	// @cmember IOpenRowset::OpenRowset(All table entries)
	int Variation_3();
	// @cmmeber NOTSUPPORTED/OPTIONAL
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGeneral)
#define THE_CLASS TCGeneral
BEG_TEST_CASE(TCGeneral, CRowsetInfoSupport, L"General (see table in test spec")
	TEST_VARIATION(1, 		L"ICommand::Execute(All table entries)")
	TEST_VARIATION(2, 		L"ICommand::Execute(Implied; All table entries)")
	TEST_VARIATION(3, 		L"IOpenRowset::OpenRowset(All table entries)")
	TEST_VARIATION(4, 		L"NOTSUPPORTED/OPTIONAL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetPropBoundary)
//--------------------------------------------------------------------
// @class Get Properties Boundary Tests
//
class TCGetPropBoundary : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember: a valid property list
	DBPROPIDSET *		m_prgProperties;

	// @cmember: a valid place to store the output property count
	ULONG				m_cPropertySets;

	// @cmember: a valid place to return the output property list
	DBPROPSET *			m_rgPropertySets;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetPropBoundary,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember 1-NULL-valid-valid ==> E_INVALIDARG
	int Variation_1();
	// @cmember 0-valid-NULL-valid ==> E_INVALIDARG
	int Variation_2();
	// @cmember 0-valid-valid-NULL ==> E_INVALIDARG
	int Variation_3();
	// @cmember 0-NULL-NULL-NULL ==> E_INVALIDARG
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetPropBoundary)
#define THE_CLASS TCGetPropBoundary
BEG_TEST_CASE(TCGetPropBoundary, CRowsetInfoSupport, L"Get Properties Boundary Tests")
	TEST_VARIATION(1, 		L"1-NULL-valid-valid ==> E_INVALIDARG")
	TEST_VARIATION(2, 		L"0-valid-NULL-valid ==> E_INVALIDARG")
	TEST_VARIATION(3, 		L"0-valid-valid-NULL ==> E_INVALIDARG")
	TEST_VARIATION(4, 		L"0-NULL-NULL-NULL ==> E_INVALIDARG")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetPropParam)
//--------------------------------------------------------------------
// @class GetProperties Parameter tests
//
class TCGetPropParam : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetPropParam,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember First Variation
	int Variation_1();
	// @cmember Second Variation
	int Variation_2();
	// @cmember Third Variation
	int Variation_3();
	// @cmember Fourth Variation
	int Variation_4();
	// @cmember Fifth Variation
	int Variation_5();
	// @cmember Sixth Variation
	int Variation_6();
	// @cmember Seventh Variation
	int Variation_7();
	// @cmember Eighth Variation
	int Variation_8();
	// @cmember Ninth Variation
	int Variation_9();
	// @cmember Tenth Variation
	int Variation_10();
	// @cmember Eleventh Variation
	int Variation_11();
	// @cmember Twelfth Variation
	int Variation_12();
	// @cmember Thirteen Variation
	int Variation_13();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetPropParam)
#define THE_CLASS TCGetPropParam
BEG_TEST_CASE(TCGetPropParam, CRowsetInfoSupport, L"GetProperties Parameter tests")
	TEST_VARIATION(1, 		L"First Variation")
	TEST_VARIATION(2, 		L"Second Variation")
	TEST_VARIATION(3, 		L"Third Variation")
	TEST_VARIATION(4, 		L"Fourth Variation")
	TEST_VARIATION(5, 		L"Fifth Variation")
	TEST_VARIATION(6, 		L"Sixth Variation")
	TEST_VARIATION(7, 		L"Seventh Variation")
	TEST_VARIATION(8, 		L"Eighth Variation")
	TEST_VARIATION(9, 		L"Ninth Variation")
	TEST_VARIATION(10, 		L"Tenth Variation")
	TEST_VARIATION(11, 		L"Eleventh Variation")
	TEST_VARIATION(12, 		L"Twelfth Variation")
	TEST_VARIATION(13, 		L"Thirteen Variation")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetPropSequence)
//--------------------------------------------------------------------
// @class GetProperties Sequence Tests
//
class TCGetPropSequence : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	CSubRowset *m_pCRowset1;
	CSubRowset *m_pCRowset2;
	CSubRowset *m_pCRowset3;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetPropSequence,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember First Variation
	int Variation_1();
	// @cmember Second Variation
	int Variation_2();
	// @cmember Third Variation
	int Variation_3();
	// @cmember Fourth Variation
	int Variation_4();
	// @cmember Fifth Variation
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetPropSequence)
#define THE_CLASS TCGetPropSequence
BEG_TEST_CASE(TCGetPropSequence, CRowsetInfoSupport, L"GetProperties Sequence Tests")
	TEST_VARIATION(1, 		L"First Variation")
	TEST_VARIATION(2, 		L"Second Variation")
	TEST_VARIATION(3, 		L"Third Variation")
	TEST_VARIATION(4, 		L"Fourth Variation")
	TEST_VARIATION(5, 		L"Fifth Variation")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRefRowsetBoundary)
//--------------------------------------------------------------------
// @class GetReferenceRowset Boundary Tests
//
class TCGetRefRowsetBoundary : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember List used to request bookmarks for variations which require them
	DBEXPECTEDPROPERTY *m_rgBookmarkProperties;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRefRowsetBoundary,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Without a Bookmark
	int Variation_1();
	// @cmember Without a Bookmark
	int Variation_2();
	// @cmember Without a Bookmark
	int Variation_3();
	// @cmember With a Bookmark
	int Variation_4();
	// @cmember With a Bookmark
	int Variation_5();
	// @cmember With a Bookmark
	int Variation_6();
	// @cmember Get all madatory IID's
	int Variation_7();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetRefRowsetBoundary)
#define THE_CLASS TCGetRefRowsetBoundary
BEG_TEST_CASE(TCGetRefRowsetBoundary, CRowsetInfoSupport, L"GetReferenceRowset Boundary Tests")
	TEST_VARIATION(1, 		L"Without a Bookmark")
	TEST_VARIATION(2, 		L"Without a Bookmark")
	TEST_VARIATION(3, 		L"Without a Bookmark")
	TEST_VARIATION(4, 		L"With a Bookmark")
	TEST_VARIATION(5, 		L"With a Bookmark")
	TEST_VARIATION(6, 		L"With a Bookmark")
	TEST_VARIATION(7, 		L"Get all madatory IID's")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRefRowsetProp)
//--------------------------------------------------------------------
// @class GetReferencedRowset Param Tests: Properties
//
class TCGetRefRowsetProp : public CGetRefRSParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRefRowsetProp,CGetRefRSParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Entire Test Table
	int Variation_1();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetRefRowsetProp)
#define THE_CLASS TCGetRefRowsetProp
BEG_TEST_CASE(TCGetRefRowsetProp, CGetRefRSParameters, L"GetReferencedRowset Param Tests: Properties")
	TEST_VARIATION(1, 		L"Entire Test Table")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRefRowsetICol)
//--------------------------------------------------------------------
// @class GetReferencedRowset Param Tests: iOrdinal
//
class TCGetRefRowsetICol : public CGetRefRSParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRefRowsetICol,CGetRefRSParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember iOrdinal=# of columns in the Rowset
	int Variation_1();
	// @cmember iOrdinal=# of columns in the rowset + 1
	int Variation_2();
	// @cmember iOrdinal=ULONG_MAX
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetRefRowsetICol)
#define THE_CLASS TCGetRefRowsetICol
BEG_TEST_CASE(TCGetRefRowsetICol, CGetRefRSParameters, L"GetReferencedRowset Param Tests: iOrdinal")
	TEST_VARIATION(1, 		L"iOrdinal=# of columns in the Rowset")
	TEST_VARIATION(2, 		L"iOrdinal=# of columns in the rowset + 1")
	TEST_VARIATION(3, 		L"iOrdinal=ULONG_MAX")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRefRowsetNoBmk)
//--------------------------------------------------------------------
// @class GetReferencedRowset Parameter Tests: No Bookmarks
//
class TCGetRefRowsetNoBmk : public CGetRefRSParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRefRowsetNoBmk,CGetRefRSParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember By ICommand::Execute
	int Variation_1();
	// @cmember By IOpenRowset::OpenRowset
	int Variation_2();
	// @cmember By IDBSchemaRowset::GetRowset
	int Variation_3();
	// @cmember By IDBEnumerateSources::Sources
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetRefRowsetNoBmk)
#define THE_CLASS TCGetRefRowsetNoBmk
BEG_TEST_CASE(TCGetRefRowsetNoBmk, CGetRefRSParameters, L"GetReferencedRowset Parameter Tests: No Bookmarks")
	TEST_VARIATION(1, 		L"By ICommand::Execute")
	TEST_VARIATION(2, 		L"By IOpenRowset::OpenRowset")
	TEST_VARIATION(3, 		L"By IDBSchemaRowset::GetRowset")
	TEST_VARIATION(4, 		L"By IDBEnumerateSources::Sources")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecBoundary)
//--------------------------------------------------------------------
// @class GetSpecification Boundary Tests
//
class TCGetSpecBoundary : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecBoundary,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IUnknown
	int Variation_1();
	// @cmember IID_IRowsetInfo
	int Variation_2();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecBoundary)
#define THE_CLASS TCGetSpecBoundary
BEG_TEST_CASE(TCGetSpecBoundary, CRowsetInfoSupport, L"GetSpecification Boundary Tests")
	TEST_VARIATION(1, 		L"IID_IUnknown")
	TEST_VARIATION(2, 		L"IID_IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecParamByExecute)
//--------------------------------------------------------------------
// @class GetSpecification Parameters: Rowset created by lCommand::Execute
//
class TCGetSpecParamByExecute : public CGetSpecParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecParamByExecute,CGetSpecParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IRowsetInfo
	int Variation_1();
	// @cmember IID_IDBCreateCommand
	int Variation_2();
	// @cmember IID_IUnknown
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecParamByExecute)
#define THE_CLASS TCGetSpecParamByExecute
BEG_TEST_CASE(TCGetSpecParamByExecute, CGetSpecParameters, L"GetSpecification Parameters: Rowset created by lCommand::Execute")
	TEST_VARIATION(1, 		L"IID_IRowsetInfo")
	TEST_VARIATION(2, 		L"IID_IDBCreateCommand")
	TEST_VARIATION(3, 		L"IID_IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecParamByOpenRowset)
//--------------------------------------------------------------------
// @class GetSpecification Parameters: Rowset created by OpenRowset
//
class TCGetSpecParamByOpenRowset : public CGetSpecParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecParamByOpenRowset,CGetSpecParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IRowsetInfo
	int Variation_1();
	// @cmember IID_ICommand
	int Variation_2();
	// @cmember IID_IUnknown
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecParamByOpenRowset)
#define THE_CLASS TCGetSpecParamByOpenRowset
BEG_TEST_CASE(TCGetSpecParamByOpenRowset, CGetSpecParameters, L"GetSpecification Parameters: Rowset created by OpenRowset")
	TEST_VARIATION(1, 		L"IID_IRowsetInfo")
	TEST_VARIATION(2, 		L"IID_ICommand")
	TEST_VARIATION(3, 		L"IID_IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecParamBySchemaRowset)
//--------------------------------------------------------------------
// @class GetSpecification Parameters: rowset created by GetRowset
//
class TCGetSpecParamBySchemaRowset : public CGetSpecParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecParamBySchemaRowset,CGetSpecParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IUnknown
	int Variation_1();
	// @cmember IID_IDBCreateCommand
	int Variation_2();
	// @cmember IID_IDBSchemaRowset
	int Variation_3();
	// @cmember IID_ICommand
	int Variation_4();
	// @cmember IID_IRowsetInfo
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecParamBySchemaRowset)
#define THE_CLASS TCGetSpecParamBySchemaRowset
BEG_TEST_CASE(TCGetSpecParamBySchemaRowset, CGetSpecParameters, L"GetSpecification Parameters: rowset created by GetRowset")
	TEST_VARIATION(1, 		L"IID_IUnknown")
	TEST_VARIATION(2, 		L"IID_IDBCreateCommand")
	TEST_VARIATION(3, 		L"IID_IDBSchemaRowset")
	TEST_VARIATION(4, 		L"IID_ICommand")
	TEST_VARIATION(5, 		L"IID_IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecParamBySources)
//--------------------------------------------------------------------
// @class GetSpecification Parameters: Rowset created by Sources
//
class TCGetSpecParamBySources : public CGetSpecParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecParamBySources,CGetSpecParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IUnknown
	int Variation_1();
	// @cmember IID_IDBProperties
	int Variation_2();
	// @cmember IID_IDBInitialize
	int Variation_3();
	// @cmember IID_IDBEnumerateSources
	int Variation_4();
	// @cmember IID_ICommand
	int Variation_5();
	// @cmember IID_IRowsetInfo
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecParamBySources)
#define THE_CLASS TCGetSpecParamBySources
BEG_TEST_CASE(TCGetSpecParamBySources, CGetSpecParameters, L"GetSpecification Parameters: Rowset created by Sources")
	TEST_VARIATION(1, 		L"IID_IUnknown")
	TEST_VARIATION(2, 		L"IID_IDBProperties")
	TEST_VARIATION(3, 		L"IID_IDBInitialize")
	TEST_VARIATION(4, 		L"IID_IDBEnumerateSources")
	TEST_VARIATION(5, 		L"IID_ICommand")
	TEST_VARIATION(6, 		L"IID_IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetSpecParamByColRowset)
//--------------------------------------------------------------------
// @class GetSpecification Parameters: Rowset created by IColumnsRowset::GetColumnsRowset
//
class TCGetSpecParamByColRowset : public CGetSpecParameters { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetSpecParamByColRowset,CGetSpecParameters);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IID_IUnknown
	int Variation_1();
	// @cmember IID_IDBProperties
	int Variation_2();
	// @cmember IID_IDBInitialize
	int Variation_3();
	// @cmember IID_IDBEnumerateSources
	int Variation_4();
	// @cmember IID_ICommand
	int Variation_5();
	// @cmember IID_IRowsetInfo
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCGetSpecParamByColRowset)
#define THE_CLASS TCGetSpecParamByColRowset
BEG_TEST_CASE(TCGetSpecParamByColRowset, CGetSpecParameters, L"GetSpecification Parameters: Rowset created by IColumnsRowset::GetColumnsRowset")
	TEST_VARIATION(1, 		L"IID_IUnknown")
	TEST_VARIATION(2, 		L"IID_IDBProperties")
	TEST_VARIATION(3, 		L"IID_IDBInitialize")
	TEST_VARIATION(4, 		L"IID_IDBEnumerateSources")
	TEST_VARIATION(5, 		L"IID_ICommand")
	TEST_VARIATION(6, 		L"IID_IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCZombieTests)
//--------------------------------------------------------------------
// @class Zombie State Tests
//
class TCZombieTests : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombieTests,CTransaction);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember Set up a zombie state
	int TestTxn(ETXN eTxn, BOOL fRetaining);

	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IRowsetInfo with fRetaining=FALSE
	int Variation_1();
	// @cmember S_OK - Abort IRowsetInfo with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Commit IRowsetInfo with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit IRowsetInfo with fRetaining=TRUE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCZombieTests)
#define THE_CLASS TCZombieTests
BEG_TEST_CASE(TCZombieTests, CTransaction, L"Zombie State Tests")
	TEST_VARIATION(1, 		L"S_OK - Abort IRowsetInfo with fRetaining=FALSE")
	TEST_VARIATION(2, 		L"S_OK - Abort IRowsetInfo with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Commit IRowsetInfo with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit IRowsetInfo with fRetaining=TRUE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMultiRowsets)
//--------------------------------------------------------------------
// @class Multiple (Concurrent) Rowsets
//
class TCMultiRowsets : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:

	struct {
		EQUERY			m_fSourceQuery;
		DBCOUNTITEM		m_cTblCols;
		LONG_PTR		*m_rgTableColOrds;
		WCHAR           *m_pszSQLStmt;
		IAccessor       *m_pIAccessor;
		ICommand        *m_pICommand;
		IRowset         *m_pIRowset;
		IRowsetInfo     *m_pIRowsetInfo;
		ICommandPrepare *m_pICmdPrepare;
		IUnknown        *m_pICreator;
	} m_RowsetPtrs[MAX_MULTI_ROWSETS];

	// @cmember: where to place the result
	IUnknown *m_pSpecification;

	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMultiRowsets,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Set up 1 or more rowsets
	BOOL SetupMultiRowsets(ULONG uNumRowsets, ...);

	// @cmember Sets one of the rowsets as current:
	// RowsetInfoSupport calls operate on the "current" rowset
	int SetCurrentRowset(ULONG uRowsetNum);

	// @cmember Clean up all rowsets
	BOOL CleanupMultiRowsets(void);

	ULONG m_uNumRowsets;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Multiple Rowsets, 2 Command Objects
	int Variation_1();
	// @cmember Multiple Rowsets, 2 OpenRowset Objects
	int Variation_2();
	// @cmember Multiple Rowsets, 1 Command, 1 OpenRowset Objects
	int Variation_3();
	// @cmember Multiple Rowsets, 1 OpenRowset, 1 Command Objects
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCMultiRowsets)
#define THE_CLASS TCMultiRowsets
BEG_TEST_CASE(TCMultiRowsets, CRowsetInfoSupport, L"Multiple (Concurrent) Rowsets")
	TEST_VARIATION(1, 		L"Multiple Rowsets, 2 Command Objects")
	TEST_VARIATION(2, 		L"Multiple Rowsets, 2 OpenRowset Objects")
	TEST_VARIATION(3, 		L"Multiple Rowsets, 1 Command, 1 OpenRowset Objects")
	TEST_VARIATION(4, 		L"Multiple Rowsets, 1 OpenRowset, 1 Command Objects")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CRowsetInfoSupport { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember List used to request bookmarks for variations which require them
	DBEXPECTEDPROPERTY *m_rgBookmarkProperties;

	// @cmember: a valid place to store the output property count
	ULONG				m_cPropertySets;

	// @cmember: a valid place to return the output property list
	DBPROPSET *			m_rgPropertySets;

	// @cmember: where to place the result
	IUnknown *			m_pSpecification;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CRowsetInfoSupport);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IRowsetInfo calls with previous error object existing.
	int Variation_1();
	// @cmember Valid IRowsetInfo calls with previous error object existing.
	int Variation_2();
	// @cmember valid IRowsetInfo calls with previous error object existing
	int Variation_3();
	// @cmember Invalid IRowsetInfo calls with previous error object existing
	int Variation_4();
	// @cmember Invalid IRowsetInfo calls with previous error object existing
	int Variation_5();
	// @cmember Invalid IRowsetInfo calls with previous error object existing
	int Variation_6();
	// @cmember Invalid IRowsetInfo calls with no error object existing
	int Variation_7();
	// @cmember Invalid IRowsetInfo calls with no previous error object existing
	int Variation_8();
	// @cmember Invalid IRowsetInfo calls with no previous error object existing
	int Variation_9();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CRowsetInfoSupport, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IRowsetInfo calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Valid IRowsetInfo calls with previous error object existing.")
	TEST_VARIATION(3, 		L"valid IRowsetInfo calls with previous error object existing")
	TEST_VARIATION(4, 		L"Invalid IRowsetInfo calls with previous error object existing")
	TEST_VARIATION(5, 		L"Invalid IRowsetInfo calls with previous error object existing")
	TEST_VARIATION(6, 		L"Invalid IRowsetInfo calls with previous error object existing")
	TEST_VARIATION(7, 		L"Invalid IRowsetInfo calls with no error object existing")
	TEST_VARIATION(8, 		L"Invalid IRowsetInfo calls with no previous error object existing")
	TEST_VARIATION(9, 		L"Invalid IRowsetInfo calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END
 

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(17, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGeneral)
	TEST_CASE(2, TCGetPropBoundary)
	TEST_CASE(3, TCGetPropParam)
	TEST_CASE(4, TCGetPropSequence)
	TEST_CASE(5, TCGetRefRowsetBoundary)
	TEST_CASE(6, TCGetRefRowsetProp)
	TEST_CASE(7, TCGetRefRowsetICol)
	TEST_CASE(8, TCGetRefRowsetNoBmk)
	TEST_CASE(9, TCGetSpecBoundary)
	TEST_CASE(10, TCGetSpecParamByExecute)
	TEST_CASE(11, TCGetSpecParamByOpenRowset)
	TEST_CASE(12, TCGetSpecParamBySchemaRowset)
	TEST_CASE(13, TCGetSpecParamBySources)
	TEST_CASE(14, TCGetSpecParamByColRowset)
	TEST_CASE(15, TCZombieTests)
	TEST_CASE(16, TCMultiRowsets)
	TEST_CASE(17, TCExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCGeneral)
//*-----------------------------------------------------------------------
//| Test Case:		TCGeneral - General (see table in test spec
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
BOOL TCGeneral::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Test the GetProperties segment of each table entry specified
//   for the General Scenarios of this autotest.
//
//   THE APPLICABLE ROWSET WILL HAVE ALREADY BEEN CREATED.  This
//   routine calls GetProperties with the arguments specified in
//   the table entry, and checks that the returned properties are
//   as specified.
//
// @rdesc TEST_PASS or TEST_FAIL.
//
//   This part is independent of the other parts of each table entry
//   test.  If this part returns TEST_FAIL, the other parts are still
//   run.
BOOL TCGeneral::TestPropertiesPart(GenTblEntry *pTableEntry)
{
	HRESULT hr = E_FAIL;
	DBPROPID rgPropertiesIn[50];
	ULONG cPropertiesOut;
	DBPROPSET *prgPropertiesOut;

	// Output the Title of the section
	odtLog << L"...GetProperties part" <<ENDL;

	// The table specifies properties for input in a structure
	// which may be shared as the data to check the return value
	// against.  Read the GUIDs out of the structure (if present)
	// into a list of properties to request from GetProperties.
	if (pTableEntry->m_cInquiredProperties) 
		for (ULONG idx = 0; idx < pTableEntry->m_cInquiredProperties; idx++) 
			rgPropertiesIn[idx] = pTableEntry->m_prgInquiredProperties[idx].dwPropertyID;

	// This is the fated test call to GetProperties...
	hr=GetProperties(pTableEntry->m_cInquiredProperties, rgPropertiesIn,
											&cPropertiesOut, &prgPropertiesOut);
	
	// GetProperties can return DB_S/DB_E_ if notsupported properties
	if (FAILED(hr) && (hr != DB_E_ERRORSOCCURRED))
		return FALSE;
	
	// GetProperties should return the same number of properties
	if ((pTableEntry->m_cInquiredProperties) && 
		(pTableEntry->m_cInquiredProperties != prgPropertiesOut->cProperties))
		return FALSE;

	// It is built in to the check routine to dispose of the
	// results once they have been checked.
	if (!ComparePropertyLists(	prgPropertiesOut,
								cPropertiesOut,
								pTableEntry->m_prgReturnedProperties,
								pTableEntry->m_cReturnedProperties,
								NULL,
								0,
								COMPAREPROP_CHECKVALUES |
								COMPAREPROP_LOGCHECKS))
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Test the GetReferencedRowset segment of each table entry
//   specified for the General Scenarios of this autotest.
//
//   THE APPLICABLE ROWSET WILL HAVE ALREADY BEEN CREATED.  This
//   routine calls GetReferencedRowset with the arguments specified
//   in the table entry, and checks that the returned properties are
//   as specified.
//
// @rdesc TEST_PASS or TEST_FAIL.
//
//   This part is independent of the other parts of each table entry
//   test.  If this part returns TEST_FAIL, the other parts are still
//   run.
BOOL TCGeneral::TestRefRowsetPart(GenTblEntry *pTableEntry)
{
	// Output the Title of the section
	odtLog << L"...GetReferencedRowset part" <<ENDL;

	// Figure out the return code for commands
	if( (m_pICommand) && 
		(((GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand)) ||
		  (GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pICommand))) ||
		 ((pTableEntry->m_idRequestedID == DBPROP_IRowsetLocate) &&
		  (SupportedProperty(DBPROP_IRowsetLocate,DBPROPSET_ROWSET,m_pIDBInitialize)))) &&
		(pTableEntry->m_hrGetReferencedRowset == DB_E_BADORDINAL) )
		pTableEntry->m_hrGetReferencedRowset = S_OK;

	// Figure out the return code for commands
	if( (m_pICommand) && 
		(((!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand)) &&
		  (!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pICommand))) &&
		((pTableEntry->m_idRequestedID != DBPROP_IRowsetLocate) ||
		 (!SupportedProperty(DBPROP_IRowsetLocate,DBPROPSET_ROWSET,m_pIDBInitialize)))) &&
		(pTableEntry->m_hrGetReferencedRowset == S_OK) )
		pTableEntry->m_hrGetReferencedRowset = DB_E_BADORDINAL;

	if(!CHECK(GetReferencedRowset(pTableEntry->m_ulColumn, IID_IUnknown,
				&m_pReferencedRowset), pTableEntry->m_hrGetReferencedRowset))
	{
		// Release Object
		if(m_pReferencedRowset != INVALID(IUnknown*))
			SAFE_RELEASE(m_pReferencedRowset);
		return FALSE;
	}
	
	// Check the return value
	if( (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIRowsetInfo) && 
		(pTableEntry->m_hrGetReferencedRowset == S_OK) && 
		(!CheckGetRRRetval(m_pReferencedRowset))) )
	{
		// Release Object
		if(m_pReferencedRowset != INVALID(IUnknown*))
			SAFE_RELEASE(m_pReferencedRowset);
		return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Test the GetSpecification segment of each table entry
//  specified for the General Scenarios of this autotest.
//
//   THE APPLICABLE ROWSET WILL HAVE ALREADY BEEN CREATED.  This
//   routine calls GetSpecification with the arguments specified in
//   the table entry, and checks that the returned properties are
//   as specified.
//
// @rdesc TEST_PASS or TEST_FAIL.
//
//   This part is independent of the other parts of each table entry
//   test.  If this part returns TEST_FAIL, the other parts are still
//   run.
BOOL TCGeneral::TestSpecificationPart(GenTblEntry *pTableEntry)
{
	// Output the Title of the section
	odtLog << L"...GetSpecification part" <<ENDL;

	if(!CHECK(GetSpecification(*pTableEntry->m_idCommandObjectID, 
				&m_pSpecification), pTableEntry->m_hrGetSpecification)) 
		return FALSE;
	
	if( (pTableEntry->m_hrGetSpecification == S_OK) && 
		(!CheckGetSpecRetval(m_pSpecification)) )
		return FALSE;
		
	return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Run the series of tests for each table entry specified for
//  the general scenarios portion of this autotest.
//
//   This function creates a rowset according to the parameters
//   specified in each table entry: a SELECT_* EQUERY is specified,
//   a set of properties which should be requested may be listed,
//   and a flag indicating whether the rowset is to be prepared
//   or not.
//
//   It then calls subroutines to test each of the 3 functions
//   GetProperties, GetReferencedRowset, and GetSpecification.  These
//   tests are independent and each one is run regardless of the success
//   of preceding runs...
//
//   After each section is run, the rowset is released.
//
// @rdesc TEST_PASS or TEST_FAIL.
//
//   If the rowset cannot be created, or if one or more of the three
//   test subparts fails, TEST_FAIL is returned.
int TCGeneral::TestTableEntry(GenTblEntry *pTableEntry)
{
	HRESULT	hr		 = E_FAIL;
	BOOL	fSuccess = FALSE;

	// Print a message identifying the current entry
	// This is because multiple table entries are tested
	// in the same variation.
	odtLog <<L"Testing " <<pTableEntry->m_pszSelectString
		   <<L" from line " <<pTableEntry->m_uLine <<ENDL;

	// Check to see if it is a Command test with no Command support
	if ((!GetCommandSupport()) && 
		(pTableEntry->m_fSQLQuery != USE_OPENROWSET   && 
		 pTableEntry->m_fSQLQuery != USE_GETCOLROWSET && 
		 pTableEntry->m_fSQLQuery != SELECT_DBSCHEMA_TABLE)) 
	{
		odtLog <<L"Commands are not supported." <<ENDL;
		return TEST_PASS;
	}

	// The "Text Set/Prepared" flag is passed to CreateRowsetObjectWithInfo,
	// Create the rowset object for testing...
	hr = CreateRowsetObjectWithInfo(pTableEntry->m_fSQLQuery,
											 *pTableEntry->m_id,
											  pTableEntry->m_cPresetProperties,
											  pTableEntry->m_prgPresetProperties,
											  pTableEntry->m_eCmdObjState);

	// This means required properties were not supported
	if (hr == DB_S_ERRORSOCCURRED)
		return TEST_PASS;

	if (!CHECK(hr, S_OK))
		goto CLEANUP;

	//  The three test parts: All 3 are run regardless of whether an earlier
	//  one succeeded or failed.  If one or more failed, remember because the
	//  overall return code must be failure.
	if (!TestPropertiesPart(pTableEntry))
		goto CLEANUP;

	if (!TestRefRowsetPart(pTableEntry))
		goto CLEANUP;

	if (!TestSpecificationPart(pTableEntry))
		goto CLEANUP;

	// Set the variation to TRUE
	fSuccess = TRUE;

CLEANUP:
	
	// Release the rowset
	ReleaseRowsetObjectWithInfo();
	odtLog <<ENDL;

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Simply calls "TestTableEntry" on each element of an array
//  of General Scenario test table entries.
//
//   This function creates a rowset according to the parameters
//   specified in each table entry: a SELECT_* EQUERY is specified,
//   a set of properties which should be requested may be listed,
//   and a flag indicating whether the rowset is to be prepared
//   or not.
//
//   It then calls subroutines to test each of the 3 functions
//   GetProperties, GetReferencedRowset, and GetSpecification.  These
//   tests are independent and each one is run regardless of the success
//   of preceding runs...
//
//   After each section is run, the rowset is released.
//
// @rdesc TEST_PASS or TEST_FAIL.
//   If one or more table test entries does not pass, the function
//   returns TEST_FAIL.
int TCGeneral::TestATable(GenTblEntry *pTestTable, UWORD cNumElements)
{
	UWORD uPass = 0;
	UWORD uFail = 0;

	// Check all of the Properties
	for (ULONG uIndex = 0; uIndex < cNumElements; uIndex++) 
	{
		if (TestTableEntry(&pTestTable[uIndex]) != TEST_PASS) {
			odtLog <<L"FAILED" <<ENDL <<ENDL;
			uFail++;
		}
		else {
			odtLog <<L"PASSED" <<ENDL <<ENDL;
			uPass++;
		}
	}

	odtLog <<L"Variation summary:"  <<ENDL;
	odtLog <<L"...Passed: " <<uPass <<ENDL;
	odtLog <<L"...Failed: " <<uFail <<ENDL <<ENDL;

	if (uFail)
		return TEST_FAIL;
	else
		return TEST_PASS;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ICommand::Execute(All table entries)
//
// @rdesc TEST_PASS or TEST_FAIL
//   Returns TEST_FAIL if one or more table entry tests failed.
//
int TCGeneral::Variation_1()
{
	// Preset properties
	//
	static DBEXPECTEDPROPERTY rgProps_IColumnsInfo[1];
	static DBEXPECTEDPROPERTY rgProps_IRowsetInfo[1];
	static DBEXPECTEDPROPERTY rgProps_IRowsetChange[1];
	static DBEXPECTEDPROPERTY rgProps_Bookmarks[1];

	// Requested properties / results
	//
	static DBEXPECTEDPROPERTY rgDistinctCollistProps[NUM_MANDATORY_PROPERTIES+2];
	static DBEXPECTEDPROPERTY rgRevCollistProps[NUM_MANDATORY_PROPERTIES+2];
	static DBEXPECTEDPROPERTY rgMandatoryPlusBmk[NUM_MANDATORY_PROPERTIES+1];
	static DBEXPECTEDPROPERTY rgWhereLastInSelect[5];
	static DBEXPECTEDPROPERTY rgColListUnionProps[5];
	static DBEXPECTEDPROPERTY rgCollistTblProps[5];
	static DBEXPECTEDPROPERTY rgABCandCollist[5];

	//  JKE:  The properties IAccessor, IRowset, IColumnsInfo, and IRowsetInfo are
	//  required, and are not settable.  Attempting to set these properties returns
	//  an error, but they are automatically set.
	//
	//  Anyplace the test plan called for these properties to be requested, they have
	//  been removed.
	//
	static GenTblEntry rgTestTable[] = {
		TEST_TABLE_ENTRY(
			SELECT_CHANGECOLNAME,          /* SQL Statement */
			NULL,                          /* Set Properties */
			IID_IAccessor,                 /* IID requested on Rowset Object */
			DBPROP_IAccessor,              /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			NULL,                          /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			g_rgMandatoryInterfaces,       /* Additional Properties     */
			0,                             /* iOrdinal */
			DB_E_BADORDINAL,               /* HRESULT on GetReferenceRowset */
			IID_ICommand,                  /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_REVCOLLISTFROMVIEW,     /* SQL Statement */
			NULL,                          /* Set Properties */
			IID_IAccessor,                 /* IID requested on Rowset Object */
			DBPROP_IAccessor,              /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			g_rgMandatoryInterfaces,       /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
		    g_rgMandatoryInterfaces,       /* Additional Properties     */
			3,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_IAccessor,                 /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_REVCOLLIST,             /* SQL Statement */
			NULL,					       /* Set Properties */ // was rgProps_IRowsetInfo
			IID_IAccessor,                 /* IID requested on Rowset Object */
			DBPROP_IAccessor,              /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			rgRevCollistProps,             /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgRevCollistProps,             /* Additional Properties     */
			1,							   /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_ICommandProperties,        /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_COLLISTGROUPBY,         /* SQL Statement */
			rgProps_Bookmarks,             /* Set Properties */
			IID_IAccessor,                 /* IID requested on Rowset Object */
			DBPROP_IAccessor,              /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			NULL,                          /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
		    rgMandatoryPlusBmk,            /* Additional Properties     */
		    0,                             /* iOrdinal */
			S_OK,                          /* HRESULT on GetReferenceRowset */
			IID_ICommandText,              /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_COLLISTWHERELASTCOLINSELECT, /* SQL Statement */
			NULL,                          /* Set Properties */
			IID_IRowsetChange,             /* IID requested on Rowset Object */
			DBPROP_IRowsetChange,          /* IID requested on Rowset Object */
			STATE_PREPARED,                /* State of command object */
			rgWhereLastInSelect,           /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
		    rgWhereLastInSelect,           /* Additional Properties     */
		    3,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_IColumnsInfo,	           /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_COLLISTFROMTBL,         /* SQL Statement */
			rgProps_IRowsetChange,         /* Set Properties */
			IID_IRowsetLocate,             /* IID requested on Rowset Object */
			DBPROP_IRowsetLocate,          /* IID requested on Rowset Object */
			STATE_PREPARED,                /* State of command object */
			rgColListUnionProps,           /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgColListUnionProps,           /* Additional Properties     */
		    0,                             /* iOrdinal */
			S_OK,                          /* HRESULT on GetReferenceRowset */
			IID_IConvertType,              /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_COLLISTFROMTBL,         /* SQL Statement */
		    NULL,                          /* Set Properties */
			IID_IRowsetChange,             /* IID requested on Rowset Object */
			DBPROP_IRowsetChange,          /* IID requested on Rowset Object */
			STATE_PREPARED,                /* State of command object */
			rgCollistTblProps,             /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgCollistTblProps,             /* Additional Properties     */
		    3,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_ICommand,		           /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_ABCANDCOLLIST,          /* SQL Statement */
			rgProps_IRowsetChange,         /* Set Properties */
			IID_IRowsetLocate,             /* IID requested on Rowset Object */
			DBPROP_IRowsetLocate,          /* IID requested on Rowset Object */
			STATE_PREPARED,                /* State of command object */
			rgABCandCollist,               /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgABCandCollist,               /* Additional Properties     */
		    0,                             /* iOrdinal */
			S_OK,                          /* HRESULT on GetReferenceRowset */
			IID_IColumnsInfo,              /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		)

	};

	// Initialize the property arrays because this cannot be done at compile time
	//
	// Preset Property Tables

	//NOTE:  If your setting duplivates here, they must be the same value.
	//Since SetProperties just uses the last value set, you checking mechanism if 
	//wrong.  IE:  if you do
	//
	// ADD_TRUE_PROP(IRowsetInfo)
	// ADD_FALSE_PROP(IRowsetInfo)
	//
	// And then later check these values, the acutal values returned by GetProperties
	//will be VARIANT_FALSE.

	ADD_TRUE_PROP(&rgProps_IColumnsInfo[0],    DBPROP_IColumnsInfo);
	ADD_TRUE_PROP(&rgProps_IRowsetInfo[0],     DBPROP_IRowsetInfo);
	ADD_TRUE_PROP(&rgProps_IRowsetChange[0],   DBPROP_IRowsetChange);
	ADD_TRUE_PROP(&rgProps_Bookmarks[0],	    DBPROP_BOOKMARKS);

	// SetProperties property tables
	ADD_UNSUP_PROP(&rgDistinctCollistProps[0], DBPROP_NOTSUPPORTEDPROPERTY);
	ADD_UNSUP_PROP(&rgDistinctCollistProps[1], DBPROP_NOTSUPPORTEDINTERFACE);
	ADD_MANDATORY_PROPERTIES(rgDistinctCollistProps, 2);

	ADD_UNDEF_PROP(&rgRevCollistProps[0],      DBPROP_NOTSETTABLEPROPERTY);
	ADD_UNDEF_PROP(&rgRevCollistProps[1],      DBPROP_SETTABLEPROPERTY);
   // deleted supported interface - used mandatory
	ADD_MANDATORY_PROPERTIES(rgRevCollistProps, 2);

	ADD_UNDEF_PROP(&rgMandatoryPlusBmk[0],     DBPROP_BOOKMARKS);
	ADD_MANDATORY_PROPERTIES(rgMandatoryPlusBmk, 1);

	ADD_UNDEF_PROP(&rgWhereLastInSelect[0],    DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgWhereLastInSelect[1],    DBPROP_IRowsetScroll);
	ADD_UNDEF_PROP(&rgWhereLastInSelect[2],    DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgWhereLastInSelect[3],    DBPROP_CANHOLDROWS);
	ADD_UNDEF_PROP(&rgWhereLastInSelect[4],    DBPROP_NOTSETTABLEPROPERTY);

	ADD_UNDEF_PROP(&rgColListUnionProps[0],    DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgColListUnionProps[1],    DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgColListUnionProps[2],    DBPROP_IRowsetUpdate);
	ADD_TRUE_PROP(&rgColListUnionProps[3],     DBPROP_IRowsetInfo);
	ADD_UNSUP_PROP(&rgColListUnionProps[4],    DBPROP_NOTSUPPORTEDPROPERTY);

	ADD_UNSUP_PROP(&rgCollistTblProps[0],      DBPROP_IRowsetIdentity);
	ADD_UNDEF_PROP(&rgCollistTblProps[1],      DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgCollistTblProps[2],      DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgCollistTblProps[3],      DBPROP_CANHOLDROWS);
	ADD_UNSUP_PROP(&rgCollistTblProps[4],      DBPROP_NOTSUPPORTEDPROPERTY);

	ADD_UNDEF_PROP(&rgABCandCollist[0],        DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgABCandCollist[1],        DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgABCandCollist[2],        DBPROP_IRowsetUpdate);
	ADD_TRUE_PROP(&rgABCandCollist[3],         DBPROP_IRowsetInfo);
	ADD_UNSUP_PROP(&rgABCandCollist[4],        DBPROP_NOTSUPPORTEDPROPERTY);

	return TestATable(rgTestTable,
							(sizeof(rgTestTable) / sizeof(rgTestTable[0])));
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ICommand::Execute(Implied; All table entries)
//
// @rdesc TEST_PASS or TEST_FAIL
//   Returns TEST_FAIL if one or more table entry tests failed.
//
int TCGeneral::Variation_2()
{
	// Preset properties
	//
	static DBEXPECTEDPROPERTY rgProps_CanHoldRows[2];
	static DBEXPECTEDPROPERTY rgProps_IUpdAndIScroll[2];
	static DBEXPECTEDPROPERTY rgProps_CanFetchBackward[1];
	static DBEXPECTEDPROPERTY rgProps_IExIFndAndILck[3];

	// Requested properties / results
	//
	static DBEXPECTEDPROPERTY rgColSelRevColProps[NUM_MANDATORY_PROPERTIES+3];
	static DBEXPECTEDPROPERTY rgEmptyRowsetProps[6];
	static DBEXPECTEDPROPERTY rgFetchBckwrdProps[2];
	static DBEXPECTEDPROPERTY rgAllFromTblProps[6];

	static GenTblEntry rgTestTable[] = {
		TEST_TABLE_ENTRY(
			SELECT_COLLISTFROMTBL,         /* SQL Statement */
			rgProps_CanHoldRows,           /* Set Properties */
			IID_IRowsetChange,             /* IID requested on Rowset Object */
		    DBPROP_IRowsetChange,          /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			rgColSelRevColProps,           /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgColSelRevColProps,           /* Additional Properties     */
			0,                             /* iOrdinal */
			DB_E_BADORDINAL,               /* HRESULT on GetReferenceRowset */
			IID_ICommandText,			   /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),


		TEST_TABLE_ENTRY(
			SELECT_COLLISTFROMTBL,         /* SQL Statement */
			rgProps_CanFetchBackward,      /* Set Properties */
			IID_IRowsetLocate,             /* IID requested on Rowset Object */
			DBPROP_IRowsetLocate,          /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			rgFetchBckwrdProps,            /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgFetchBckwrdProps,            /* Additional Properties     */
			0,							   /* iOrdinal */
			S_OK,                          /* HRESULT on GetReferenceRowset */
			IID_ICommand,				   /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			SELECT_ALLFROMTBL,             /* SQL Statement */
		    rgProps_IExIFndAndILck,        /* Set Properties */
			IID_IRowsetChange,             /* IID requested on Rowset Object */
			DBPROP_IRowsetChange,          /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object */
			rgAllFromTblProps,             /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgAllFromTblProps,             /* Additional Properties     */
			3,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_ICommandProperties,		   /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		)
	};

	// Initialize the property arrays because this cannot be done at compile time
	//
	// Preset Property Tables

	ADD_TRUE_PROP(&rgProps_CanHoldRows[0],     DBPROP_CANHOLDROWS);
	ADD_FALSE_PROP(&rgProps_CanHoldRows[1],     DBPROP_BOOKMARKS);
	ADD_TRUE_PROP(&rgProps_IUpdAndIScroll[0],  DBPROP_IRowsetUpdate);
	ADD_UNDEF_PROP(&rgProps_IUpdAndIScroll[1], DBPROP_IRowsetScroll);

	ADD_UNDEF_PROP(&rgProps_IExIFndAndILck[0], DBPROP_IRowsetScroll);
	ADD_TRUE_PROP(&rgProps_IExIFndAndILck[1],  DBPROP_IRowsetUpdate);
	ADD_TRUE_PROP(&rgProps_IExIFndAndILck[2],  DBPROP_IRowsetFind);

	ADD_TRUE_PROP(&rgProps_CanFetchBackward[0],DBPROP_CANFETCHBACKWARDS);

	ADD_UNDEF_PROP(&rgColSelRevColProps[0],    DBPROP_IRowsetChange);
	ADD_UNSUP_PROP(&rgColSelRevColProps[1],    DBPROP_NOTSUPPORTEDINTERFACE);
	ADD_FALSE_PROP(&rgColSelRevColProps[2],     DBPROP_BOOKMARKS);
	ADD_MANDATORY_PROPERTIES(rgColSelRevColProps, 3);

	ADD_UNDEF_PROP(&rgEmptyRowsetProps[0],     DBPROP_IRowsetUpdate);
	ADD_UNDEF_PROP(&rgEmptyRowsetProps[1],     DBPROP_IRowsetScroll);
	ADD_UNDEF_PROP(&rgEmptyRowsetProps[2],     DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgEmptyRowsetProps[3],     DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgEmptyRowsetProps[4],     DBPROP_CANFETCHBACKWARDS);
	ADD_UNSUP_PROP(&rgEmptyRowsetProps[5],     DBPROP_NOTSUPPORTEDPROPERTY);

	ADD_UNDEF_PROP(&rgFetchBckwrdProps[0],     DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgFetchBckwrdProps[1],     DBPROP_CANFETCHBACKWARDS);

	ADD_UNDEF_PROP(&rgAllFromTblProps[0],      DBPROP_IRowsetScroll);
	ADD_UNDEF_PROP(&rgAllFromTblProps[1],      DBPROP_IRowsetFind);
	ADD_UNDEF_PROP(&rgAllFromTblProps[2],      DBPROP_NOTSUPPORTEDINTERFACE);
	ADD_UNDEF_PROP(&rgAllFromTblProps[3],      DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgAllFromTblProps[4],      DBPROP_IRowsetLocate);
	ADD_UNDEF_PROP(&rgAllFromTblProps[5],      DBPROP_CANSCROLLBACKWARDS);

	return TestATable(rgTestTable,
							(sizeof(rgTestTable) / sizeof(rgTestTable[0])));
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IOpenRowset::OpenRowset(All table entries)
//
// @rdesc TEST_PASS or TEST_FAIL
//   Returns TEST_FAIL if one or more table entry tests failed.
//
int TCGeneral::Variation_3()
{
	// Preset properties
	//
	static DBEXPECTEDPROPERTY rgProps_Bookmarks[1];
	static DBEXPECTEDPROPERTY rgProps_RightOJ[1];

	// Requested properties / results
	//
	static DBEXPECTEDPROPERTY rgNull1Props[NUM_MANDATORY_PROPERTIES+1];
	static DBEXPECTEDPROPERTY rgBmkProps[2];
	static DBEXPECTEDPROPERTY rgNull2Props[NUM_MANDATORY_PROPERTIES+2];
	static DBEXPECTEDPROPERTY rgSortBy1Props[5];

	static GenTblEntry rgTestTable[] = {
		TEST_TABLE_ENTRY(
			USE_OPENROWSET,                /* SQL Statement */
			NULL,                          /* Set Properties */
			IID_IAccessor,                 /* IID requested on Rowset Object */
			DBPROP_IAccessor,              /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object - NOT APPLICABLE */
			rgNull1Props,                  /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgNull1Props,                  /* Additional Properties     */
			1,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_IOpenRowset,               /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			USE_OPENROWSET,                /* SQL Statement */
			rgProps_Bookmarks,             /* Set Properties */
			IID_IRowsetChange,             /* IID requested on Rowset Object */
			DBPROP_IRowsetChange,          /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object - NOT APPLICABLE */
			rgBmkProps,                    /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
			rgBmkProps,                    /* Additional Properties     */
			0,                             /* iOrdinal */
			S_OK,                          /* HRESULT on GetReferenceRowset */
			IID_ISessionProperties,        /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
		),

		TEST_TABLE_ENTRY(
			USE_OPENROWSET,                /* SQL Statement */
			NULL,                          /* Set Properties */
			IID_IRowsetLocate,             /* IID requested on Rowset Object */
			DBPROP_IRowsetLocate,          /* IID requested on Rowset Object */
			STATE_TEXT_SET,                /* State of command object - NOT APPLICABLE */
			rgNull2Props,                  /* Properties inquired on GetProperties */
			TRUE,                          /* obsolete */
		    rgNull2Props,                  /* Additional Properties     */
			1,                             /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,      /* HRESULT on GetReferenceRowset */
			IID_IGetDataSource,            /* IID on the command object */
			S_OK                           /* HRESULT on GetSpecification */
	   ),
	};

	// Initialize the property arrays because this cannot be done at compile time
	//
	// Preset Property Tables

	ADD_TRUE_PROP(&rgProps_Bookmarks[0],      DBPROP_BOOKMARKS);
	ADD_TRUE_PROP(&rgProps_RightOJ[0],        DBPROP_IRowsetChange);

	ADD_MANDATORY_PROPERTIES(rgNull1Props, 0);

	ADD_UNDEF_PROP(&rgBmkProps[0],            DBPROP_IRowsetChange);
	ADD_UNDEF_PROP(&rgBmkProps[1],            DBPROP_BOOKMARKS);

	ADD_UNDEF_PROP(&rgNull2Props[0],          DBPROP_IRowsetLocate);
	ADD_UNSUP_PROP(&rgNull2Props[1],          DBPROP_NOTSUPPORTEDINTERFACE);
	ADD_MANDATORY_PROPERTIES(rgNull2Props, 2);

	ADD_TRUE_PROP(&rgSortBy1Props[0],         DBPROP_IAccessor);             // 1 mandatory
	ADD_UNDEF_PROP(&rgSortBy1Props[1],        DBPROP_SETTABLEPROPERTY);
	ADD_UNDEF_PROP(&rgSortBy1Props[2],        DBPROP_NOTSETTABLEPROPERTY);
	ADD_UNSUP_PROP(&rgSortBy1Props[3],        DBPROP_NOTSUPPORTEDPROPERTY);
	ADD_TRUE_PROP(&rgSortBy1Props[4],         DBPROP_IRowset);

	return TestATable(rgTestTable,
							(sizeof(rgTestTable) / sizeof(rgTestTable[0])));
}
// }}
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc NOTSUPPORTED/OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL
//  
//
int TCGeneral::Variation_4()
{
	// Preset properties
	static DBEXPECTEDPROPERTY rgProps_NS_OP[1];

	// Requested properties / results
	static DBEXPECTEDPROPERTY rgNSOPProps[1];

	static GenTblEntry rgTestTable[] = {
		TEST_TABLE_ENTRY(
			USE_OPENROWSET,				/* SQL Statement */
			rgProps_NS_OP,				/* Set Properties */
			IID_IRowset,	            /* IID requested on Rowset Object */
			DBPROP_IRowset,				/* IID requested on Rowset Object */
			STATE_TEXT_SET,             /* State of command object - NOT APPLICABLE */
			rgNSOPProps,				/* Properties inquired on GetProperties */
			TRUE,                       /* obsolete */
			rgNSOPProps,				/* Additional Properties     */
			1,                          /* iOrdinal */
			DB_E_NOTAREFERENCECOLUMN,   /* HRESULT on GetReferenceRowset */
			IID_IOpenRowset,            /* IID on the command object */
			S_OK                        /* HRESULT on GetSpecification */
		)
	};

	// Initialize the property arrays because this cannot be done at compile time
	//
	// Preset Property Tables

	if (DBPROP_NOTSUPPORTEDPROPERTY)
	{
		ADD_TRUE_PROP_OPTIONAL(&rgProps_NS_OP[0],DBPROP_NOTSUPPORTEDPROPERTY);

		return TestATable(rgTestTable,(sizeof(rgTestTable) / sizeof(rgTestTable[0])));
	}
	else
	{
		return TEST_SKIPPED;
	}
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGeneral::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetPropBoundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetPropBoundary - Get Properties Boundary Tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropBoundary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		// Create a rowset with any arbitrary method.
		if(CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc 1-NULL-valid-valid ==> E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropBoundary::Variation_1()
{
	if(!CHECK(GetProperties(1,(DBPROPIDSET *)NULL,
							&m_cPropertySets,&m_rgPropertySets), E_INVALIDARG))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc 0-valid-NULL-valid ==> E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropBoundary::Variation_2()
{
	// Initialize
	m_prgProperties = NULL;
	
	if(!CHECK(GetProperties(0,m_prgProperties,
										NULL,&m_rgPropertySets), E_INVALIDARG))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc 0-valid-valid-NULL ==> E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropBoundary::Variation_3()
{
	// Initialize
	m_prgProperties = NULL;

	if(!CHECK(GetProperties(0,m_prgProperties,
										&m_cPropertySets,NULL), E_INVALIDARG))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc 0-NULL-NULL-NULL ==> E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropBoundary::Variation_4()
{
	if(!CHECK(GetProperties(0,(DBPROPIDSET *)NULL,NULL,NULL),E_INVALIDARG))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropBoundary::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetPropParam)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetPropParam - GetProperties Parameter tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropParam::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
		return TRUE;
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc First Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_1()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[6];
	DBPROPID			rgProperties[6];

	// This variation requires that commands be supported.
	if(!GetCommandSupport())
		return TEST_SKIPPED;

	// rgProperties contains a duplicated mandatory interface, duplicate not
	// settable property, and duplicate support interface.
	ADD_TRUE_PROP(&rgTestProps[0],  DBPROP_IRowsetInfo);
	ADD_UNDEF_PROP(&rgTestProps[1], DBPROP_NOTSETTABLEPROPERTY);
	ADD_TRUE_PROP(&rgTestProps[2],  DBPROP_IRowset);
	ADD_TRUE_PROP(&rgTestProps[3],  DBPROP_IRowsetInfo);
	ADD_UNDEF_PROP(&rgTestProps[4], DBPROP_NOTSETTABLEPROPERTY);
	ADD_TRUE_PROP(&rgTestProps[5],  DBPROP_IRowset);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	// Get a rowset by ICommand::Execute
	if(!CHECK(CreateRowsetObjectWithInfo(SELECT_EMPTYROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(6,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,6,NULL,0,
		(COMPAREPROP_CHECKVALUES | COMPAREPROP_LOGCHECKS)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Second Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_2()
{
#ifdef ENUMSOURCES
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[1];
	DBPROPID			rgProperties[1];

	// Add properties to the list
	ADD_UNSUP_PROP(&rgTestProps[0], DBPROP_IRowsetChange);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IDBEnumerateSources::Sources
	if(!CHECK(CreateRowsetObjectWithInfo(USE_ENUMSOURCES, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(1,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,1,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL;
	return TEST_PASS;
#endif
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc Third Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_3()
{
#ifdef ENUMSOURCES
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[2];
	DBPROPID			rgProperties[2];

	// Add properties to the list
	ADD_TRUE_PROP(&rgTestProps[0], DBPROP_IRowsetInfo);
	ADD_TRUE_PROP(&rgTestProps[1], DBPROP_IColumnsInfo);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IDBEnumerateSources::Sources
	if(!CHECK(CreateRowsetObjectWithInfo(USE_ENUMSOURCES, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(2,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,2,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL;
	return TEST_PASS;
#endif
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc Fourth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_4()
{
#ifdef ENUMSOURCES
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	//  Get a rowset by IDBEnumerateSources::Sources
	if(!CHECK(CreateRowsetObjectWithInfo(USE_ENUMSOURCES, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(0,(DBPROPIDSET *)NULL,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,g_rgMandatoryInterfaces,
				NUM_MANDATORY_PROPERTIES,NULL,0,(COMPAREPROP_CHECKVALUES 
						| COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL;
	return TEST_PASS;
#endif
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc Fifth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_5()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;
	ULONG				uIndex;
	HRESULT				hr			= E_FAIL;

	DBEXPECTEDPROPERTY	rgTestProps[2];
	DBPROPID			rgProperties[2];

	// This variation requires that schemas are supported
	if(!g_fSchemaRowSupported)
		return TEST_SKIPPED;

	// Add properties to the list
	ADD_UNSUP_PROP(&rgTestProps[0], DBPROP_NOTSUPPORTEDPROPERTY);
	ADD_TRUE_PROP(&rgTestProps[1],  DBPROP_IRowsetInfo);

	for(uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IDBSchemaRowset::GetRowset
	if(!CHECK(hr = CreateRowsetObjectWithInfo(SELECT_DBSCHEMA_TABLE, IID_IRowset, uIndex, rgTestProps),DB_E_ERRORSOCCURRED))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if (FAILED(hr))
	{
		// Should not have our IRowsetInfo interface
		TESTC(m_pIRowsetInfo == NULL);
	}

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//--------------------------------------------------------------------
// @mfunc Sixth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_6()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[2];
	DBPROPID			rgProperties[2];

	// This variation requires that schemas are supported
	if(!g_fSchemaRowSupported)
		return TEST_SKIPPED;

	// Add properties to the list
	ADD_TRUE_PROP(&rgTestProps[0], DBPROP_IRowsetInfo);
	ADD_TRUE_PROP(&rgTestProps[1], DBPROP_IAccessor);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IDBSchemaRowset::GetRowset
	if(!CHECK(CreateRowsetObjectWithInfo(SELECT_DBSCHEMA_TABLE, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(2,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,2,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//--------------------------------------------------------------------
// @mfunc Seventh Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_7()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	// This variation requires that schemas are supported
	if(!g_fSchemaRowSupported)
		return TEST_SKIPPED;

	// Get a rowset by IDBSchemaRowset::GetRowset
	if(!CHECK(CreateRowsetObjectWithInfo(SELECT_DBSCHEMA_TABLE, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(0,(DBPROPIDSET *)NULL,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,g_rgMandatoryInterfaces,
				NUM_MANDATORY_PROPERTIES,NULL,0,(COMPAREPROP_CHECKVALUES 
						| COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(8)
//--------------------------------------------------------------------
// @mfunc Eighth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_8()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[1];
	DBPROPID			rgProperties[1];

	// This variation requires that schemas are supported
	if(!g_fColumnRowSupported)
		return TEST_SKIPPED;

	// Add properties to the list
	ADD_UNSUP_PROP(&rgTestProps[0], DBPROP_NOTSUPPORTEDPROPERTY);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by ColumnsRowset::GetColumnsRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_GETCOLROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(1,rgProperties,&uRsltPropCnt,&rgRsltProps), DB_E_ERRORSOCCURRED))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,1,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//--------------------------------------------------------------------
// @mfunc Ninth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_9()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[2];
	DBPROPID			rgProperties[2];

	// This variation requires that schemas are supported
	if(!g_fColumnRowSupported)
		return TEST_SKIPPED;

	// Add properties to the list
	ADD_TRUE_PROP(&rgTestProps[0], DBPROP_IRowsetInfo);
	ADD_TRUE_PROP(&rgTestProps[1], DBPROP_IColumnsInfo);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by ColumnsRowset::GetColumnsRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_GETCOLROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(2,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,2,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//--------------------------------------------------------------------
// @mfunc Tenth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_10()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	// This variation requires that schemas are supported
	if(!g_fColumnRowSupported)
		return TEST_SKIPPED;

	//  Get a rowset by ColumnsRowset::GetColumnsRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_GETCOLROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(0,(DBPROPIDSET *)NULL,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,g_rgMandatoryInterfaces,
				NUM_MANDATORY_PROPERTIES,NULL,0,(COMPAREPROP_CHECKVALUES 
						| COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//--------------------------------------------------------------------
// @mfunc Eleventh Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_11()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[1];
	DBPROPID			rgProperties[1];

	// Add properties to the list
	ADD_UNSUP_PROP(&rgTestProps[0], DBPROP_NOTSUPPORTEDPROPERTY);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IOpenRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(1,rgProperties,&uRsltPropCnt,&rgRsltProps), DB_E_ERRORSOCCURRED))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,1,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc Twelfth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_12()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	DBEXPECTEDPROPERTY	rgTestProps[2];
	DBPROPID			rgProperties[2];

	// Add properties to the list
	ADD_TRUE_PROP(&rgTestProps[0], DBPROP_IRowsetInfo);
	ADD_TRUE_PROP(&rgTestProps[1], DBPROP_IColumnsInfo);

	for(ULONG uIndex=0; uIndex<(sizeof(rgTestProps)/sizeof(rgTestProps[0])); uIndex++)
		rgProperties[uIndex] = rgTestProps[uIndex].dwPropertyID;

	//  Get a rowset by IOpenRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(2,rgProperties,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,rgTestProps,2,NULL,0,
		(COMPAREPROP_DISALLOWEXTRAS | COMPAREPROP_CHECKVALUES | 
							COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//--------------------------------------------------------------------
// @mfunc Thirteen Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropParam::Variation_13()
{
	ULONG				uRsltPropCnt = 0;
	DBPROPSET			*rgRsltProps = NULL;
	BOOL				fSuccess	 = FALSE;

	//  Get a rowset by IOpenRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	// Cleanup is necessary following a failure from this point onward...
	if(!CHECK(GetProperties(0,(DBPROPIDSET *)NULL,&uRsltPropCnt,&rgRsltProps), S_OK))
		goto CLEANUP;

	if(!ComparePropertyLists(rgRsltProps,uRsltPropCnt,g_rgMandatoryInterfaces,
				NUM_MANDATORY_PROPERTIES,NULL,0,(COMPAREPROP_CHECKVALUES 
						| COMPAREPROP_LOGCHECKS | COMPAREPROP_NODUPCHECK)))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	// The property list was cleaned up by Compare.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropParam::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetPropSequence)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetPropSequence - GetProperties Sequence Tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropSequence::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
		return TRUE;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc First Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropSequence::Variation_1()
{
#ifdef ENUMSOURCES
	HRESULT hr1 = E_FAIL;
	HRESULT hr2 = E_FAIL;
	HRESULT hr3 = E_FAIL;

	BOOL		bTestFailed	= FALSE;
	ULONG		cOutProps	= 0;
	DBPROPSET	*rgOutProps = NULL;

	// Initialize the Rowset pointers
	m_pCRowset1 = NULL;
	m_pCRowset1 = MakeNewRowset(&hr1, USE_ENUMSOURCES);

	m_pCRowset2 = NULL;
	m_pCRowset2 = MakeNewRowset(&hr2, USE_ENUMSOURCES);

	m_pCRowset3 = NULL;
	m_pCRowset3 = MakeNewRowset(&hr3, USE_ENUMSOURCES);

	// If any rowsets failed creation, skip the test.
	if(m_pCRowset1 && m_pCRowset2 && m_pCRowset3) 
	{
		// Verify GetProperties returns the correct properties on each rowset.
		// ComparePropertyList will free the property list returned by GetProperties.
		if(!CHECK(m_pCRowset1->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, g_rgMandatoryInterfaces, 
						NUM_MANDATORY_PROPERTIES, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, g_rgMandatoryInterfaces, 
						NUM_MANDATORY_PROPERTIES, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, g_rgMandatoryInterfaces, 
						NUM_MANDATORY_PROPERTIES, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		// Verify GetReferencedRowset returns the correct information
		if(!CHECK(m_pCRowset1->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset1->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetRRRetval(m_pCRowset1->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset2->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetRRRetval(m_pCRowset2->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset3->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetRRRetval(m_pCRowset3->m_pReferencedRowset))
			bTestFailed = TRUE;

		// Verify GetSpecification returns the correct information
		if(FAILED(m_pCRowset1->m_hr=m_pCRowset1->GetSpecification(IID_IUnknown, &m_pCRowset1->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetSpecRetval(m_pCRowset1->m_pSpecification)) 
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset2->m_hr=m_pCRowset2->GetSpecification(IID_IUnknown, &m_pCRowset2->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetSpecRetval(m_pCRowset2->m_pSpecification)) 
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset3->m_hr=m_pCRowset3->GetSpecification(IID_IUnknown, &m_pCRowset3->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetSpecRetval(m_pCRowset3->m_pSpecification)) 
			bTestFailed = TRUE;
	}

	ReleaseRowset(m_pCRowset1);
	ReleaseRowset(m_pCRowset2);
	ReleaseRowset(m_pCRowset3);

   	if(bTestFailed)
		return TEST_FAIL;
	else
		return TEST_PASS;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL;
	return TEST_PASS;
#endif
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Second Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropSequence::Variation_2()
{
	HRESULT hr1 = E_FAIL;
	HRESULT hr2 = E_FAIL;
	HRESULT hr3 = E_FAIL;

	BOOL	  bTestFailed = FALSE;
	ULONG	  cOutProps	  = 0;
	DBPROPSET *rgOutProps = NULL;

	// I set properties I want, but I ask for all properties (cProperties = 0)
	// this means the Input and Output lists will be different.
	DBEXPECTEDPROPERTY rgPropsIn1[1];
	DBEXPECTEDPROPERTY rgPropsIn2[2];
	DBEXPECTEDPROPERTY rgPropsIn3[1];

	DBEXPECTEDPROPERTY rgPropsOutExpect1[1 + NUM_MANDATORY_PROPERTIES];
	DBEXPECTEDPROPERTY rgPropsOutExpect2[2 + NUM_MANDATORY_PROPERTIES];
	DBEXPECTEDPROPERTY rgPropsOutExpect3[1 + NUM_MANDATORY_PROPERTIES];

	// Bookmarks are required to run this test variant.
	if( (!g_fBookmarksSupported) || 
		((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)))
		return TEST_SKIPPED;

	// Create First rowset objects from a single DB session object by
	ADD_TRUE_PROP(&rgPropsIn1[0], DBPROP_BOOKMARKS);
	ADD_TRUE_PROP(&rgPropsOutExpect1[0], DBPROP_BOOKMARKS);
	ADD_MANDATORY_PROPERTIES(rgPropsOutExpect1, 1);
	
	m_pCRowset1=NULL;
	m_pCRowset1=MakeNewRowset(&hr1, USE_OPENROWSET, 1, rgPropsIn1);

	// Create Second rowset objects from a single DB session object by
	if(SettableProperty(DBPROP_LITERALBOOKMARKS,DBPROPSET_ROWSET,m_pIDBInitialize))
	{
		ADD_TRUE_PROP(&rgPropsIn2[0], DBPROP_BOOKMARKS);
		ADD_TRUE_PROP(&rgPropsOutExpect2[0], DBPROP_BOOKMARKS);
		ADD_TRUE_PROP(&rgPropsIn2[1], DBPROP_LITERALBOOKMARKS);  
		ADD_TRUE_PROP(&rgPropsOutExpect2[1], DBPROP_LITERALBOOKMARKS);
		ADD_MANDATORY_PROPERTIES(rgPropsOutExpect2, 2);
		
		m_pCRowset2=NULL;
		m_pCRowset2=MakeNewRowset(&hr2, USE_OPENROWSET, 2, rgPropsIn2);
	}
	else
	{
		ADD_TRUE_PROP(&rgPropsIn2[0], DBPROP_BOOKMARKS);
		ADD_TRUE_PROP(&rgPropsOutExpect2[0], DBPROP_BOOKMARKS);
		ADD_MANDATORY_PROPERTIES(rgPropsOutExpect2, 1);
		
		m_pCRowset2=NULL;
		m_pCRowset2=MakeNewRowset(&hr2, USE_OPENROWSET, 1, rgPropsIn2);
	}

	// Create Third rowset objects from a single DB session object by
	ADD_TRUE_PROP(&rgPropsIn3[0], DBPROP_IRowsetLocate);
	ADD_TRUE_PROP(&rgPropsOutExpect3[0], DBPROP_IRowsetLocate);
	ADD_MANDATORY_PROPERTIES(rgPropsOutExpect3, 1);
	
	m_pCRowset3=NULL;
	m_pCRowset3=MakeNewRowset(&hr3, USE_OPENROWSET, 1, rgPropsIn3);

	// If any rowsets failed creation, skip the test.
	if(m_pCRowset1 && m_pCRowset2 && m_pCRowset3)
	{
		// Verify GetProperties returns the correct properties on each rowset.
		// ComparePropertyList will free the property list returned by GetProperties.
		if(!CHECK(m_pCRowset1->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsOutExpect1, 
				(sizeof(rgPropsIn1)/sizeof(rgPropsIn1[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsOutExpect2, 
				(sizeof(rgPropsIn2)/sizeof(rgPropsIn2[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsOutExpect3, 
				(sizeof(rgPropsIn3)/sizeof(rgPropsIn3[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		// Verify GetReferencedRowset returns the correct information
		if(!CHECK(m_pCRowset1->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset1->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetRRRetval(m_pCRowset1->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset2->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetRRRetval(m_pCRowset2->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset3->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetRRRetval(m_pCRowset3->m_pReferencedRowset))
			bTestFailed = TRUE;

		// Verify GetSpecification returns the correct information
		if(FAILED(m_pCRowset1->m_hr=m_pCRowset1->GetSpecification(IID_IUnknown, &m_pCRowset1->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetSpecRetval(m_pCRowset1->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset2->m_hr=m_pCRowset2->GetSpecification(IID_IUnknown, &m_pCRowset2->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetSpecRetval(m_pCRowset2->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset3->m_hr=m_pCRowset3->GetSpecification(IID_IUnknown, &m_pCRowset3->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetSpecRetval(m_pCRowset3->m_pSpecification))
			bTestFailed = TRUE;
	}

	ReleaseRowset(m_pCRowset1);
	ReleaseRowset(m_pCRowset2);
	ReleaseRowset(m_pCRowset3);

   	if(bTestFailed)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc Third Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropSequence::Variation_3()
{
	HRESULT hr1 = E_FAIL;
	HRESULT hr2 = E_FAIL;
	HRESULT hr3 = E_FAIL;

	BOOL		bTestFailed	= FALSE;
	ULONG		cOutProps	= 0;
	DBPROPSET	*rgOutProps = NULL;

	ULONG				cPropsIn = 1;
	DBEXPECTEDPROPERTY	rgPropsIn[1];

	// This variation requires that schemas are supported
	if (!g_fSchemaRowSupported) {
		odtLog << L"Variation skipped: SchemaRowsets not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Bookmarks are required to run this test variant.
	if( (!g_fBookmarksSupported) || 
		((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)))
		return TEST_SKIPPED;

	ADD_TRUE_PROP(&rgPropsIn[0], DBPROP_BOOKMARKS);

	m_pCRowset1=NULL;
	m_pCRowset1=MakeNewRowset(&hr1, SELECT_DBSCHEMA_TABLE, cPropsIn, rgPropsIn);

	m_pCRowset2=NULL;
	m_pCRowset2=MakeNewRowset(&hr2, SELECT_DBSCHEMA_TABLE, cPropsIn, rgPropsIn);

	m_pCRowset3=NULL;
	m_pCRowset3=MakeNewRowset(&hr3, SELECT_DBSCHEMA_TABLE, cPropsIn, rgPropsIn);

	// If any rowsets failed creation, skip the test.
	if(m_pCRowset1 && m_pCRowset2 && m_pCRowset3)
	{
		// Verify GetProperties returns the correct properties on each rowset.
		// ComparePropertyList will free the property list returned by GetProperties.
		if(!CHECK(m_pCRowset1->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsIn, 
				(sizeof(rgPropsIn)/sizeof(rgPropsIn[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsIn, 
				(sizeof(rgPropsIn)/sizeof(rgPropsIn[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, rgPropsIn, 
				(sizeof(rgPropsIn)/sizeof(rgPropsIn[0])), NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		// Verify GetReferencedRowset returns the correct information
		if(!CHECK(m_pCRowset1->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset1->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetRRRetval(m_pCRowset1->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset2->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetRRRetval(m_pCRowset2->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetReferencedRowset(0,IID_IUnknown, &m_pCRowset3->m_pReferencedRowset), S_OK))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetRRRetval(m_pCRowset3->m_pReferencedRowset))
			bTestFailed = TRUE;

		// Verify GetSpecification returns the correct information
		// Some Prroviders don't know the object that created the rowset
		if(FAILED(m_pCRowset1->m_hr=m_pCRowset1->GetSpecification(IID_IUnknown, &m_pCRowset1->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetSpecRetval(m_pCRowset1->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset2->m_hr=m_pCRowset2->GetSpecification(IID_IUnknown, &m_pCRowset2->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetSpecRetval(m_pCRowset2->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset3->m_hr=m_pCRowset3->GetSpecification(IID_IUnknown, &m_pCRowset3->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetSpecRetval(m_pCRowset3->m_pSpecification))
			bTestFailed = TRUE;
	}

	ReleaseRowset(m_pCRowset1);
	ReleaseRowset(m_pCRowset2);
	ReleaseRowset(m_pCRowset3);

   	if(bTestFailed)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc Fourth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropSequence::Variation_4()
{
	HRESULT hr1 = E_FAIL;
	HRESULT hr2 = E_FAIL;
	BOOL	bTestFailed	= FALSE;

	// This variation requires that schemas are supported
	if (!g_fSchemaRowSupported) {
		odtLog << L"Variation skipped: SchemaRowsets not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	m_pCRowset1=NULL;
	m_pCRowset1=MakeNewRowset(&hr1, SELECT_DBSCHEMA_TABLE);

	m_pCRowset2=NULL;
	m_pCRowset2=MakeNewRowset(&hr2, USE_OPENROWSET);

	// If any rowsets failed creation, skip the test.
	if(m_pCRowset1 && m_pCRowset2)
	{
		// Verify GetSpecification returns the correct information
		if(FAILED(m_pCRowset1->m_hr=m_pCRowset1->GetSpecification(IID_IUnknown, &m_pCRowset1->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetSpecRetval(m_pCRowset1->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset2->m_hr=m_pCRowset2->GetSpecification(IID_IUnknown, &m_pCRowset2->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetSpecRetval(m_pCRowset2->m_pSpecification))
			bTestFailed = TRUE;
	}

	ReleaseRowset(m_pCRowset1);
	ReleaseRowset(m_pCRowset2);

   	if(bTestFailed)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc Fifth Variation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetPropSequence::Variation_5()
{
	HRESULT hr1 = E_FAIL;
	HRESULT hr2 = E_FAIL;
	HRESULT hr3 = E_FAIL;

	BOOL		bTestFailed	= FALSE;
	ULONG		cOutProps	= 0;
	DBPROPSET	*rgOutProps = NULL;

	ULONG				cPropsIn = 0;
	DBEXPECTEDPROPERTY	rgPropsIn[1];

	// This variation requires that commands be supported.
	if (!GetCommandSupport()) {
		odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Bookmarks are required to run this test variant.
	if( (g_fBookmarksSupported) && 
		((g_fBookmarksSettable) || (g_fBookmarksOnByDft)))
	{
		cPropsIn = 1;
		ADD_TRUE_PROP(&rgPropsIn[0], DBPROP_BOOKMARKS);
	}

	m_pCRowset1=NULL;
	m_pCRowset1=MakeNewRowset(&hr1, SELECT_EMPTYROWSET, cPropsIn, rgPropsIn);

	m_pCRowset2=NULL;
	m_pCRowset2=MakeNewRowset(&hr2, SELECT_EMPTYROWSET, cPropsIn, rgPropsIn);

	m_pCRowset3=NULL;
	m_pCRowset3=MakeNewRowset(&hr3, SELECT_EMPTYROWSET, cPropsIn, rgPropsIn);

	// If any rowsets failed creation, skip the test.
	if(m_pCRowset1 && m_pCRowset2 && m_pCRowset3)
	{
		// Verify GetProperties returns the correct properties on each rowset.
		// ComparePropertyList will free the property list returned by GetProperties.
		if(!CHECK(m_pCRowset1->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, 
					rgPropsIn, cPropsIn, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, 
					rgPropsIn, cPropsIn, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetProperties(0, (DBPROPIDSET *)NULL, &cOutProps, &rgOutProps), S_OK))
			bTestFailed = TRUE;
		else if(!ComparePropertyLists(rgOutProps, cOutProps, 
					rgPropsIn, cPropsIn, NULL, 0, (COMPAREPROP_CHECKVALUES)))
			bTestFailed = TRUE;

		// Verify GetReferencedRowset returns the correct information
		if(!CHECK(m_pCRowset1->GetReferencedRowset(0,IID_IUnknown, 
				&m_pCRowset1->m_pReferencedRowset), cPropsIn ? S_OK : DB_E_BADORDINAL))
			bTestFailed = TRUE;
		
		if(cPropsIn && !m_pCRowset1->CheckGetRRRetval(m_pCRowset1->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset2->GetReferencedRowset(0,IID_IUnknown, 
				&m_pCRowset2->m_pReferencedRowset), cPropsIn ? S_OK : DB_E_BADORDINAL))
			bTestFailed = TRUE;
		
		if(cPropsIn && !m_pCRowset2->CheckGetRRRetval(m_pCRowset2->m_pReferencedRowset))
			bTestFailed = TRUE;

		if(!CHECK(m_pCRowset3->GetReferencedRowset(0,IID_IUnknown, 
				&m_pCRowset3->m_pReferencedRowset), cPropsIn ? S_OK : DB_E_BADORDINAL))
			bTestFailed = TRUE;
		
		if(cPropsIn && !m_pCRowset3->CheckGetRRRetval(m_pCRowset3->m_pReferencedRowset))
			bTestFailed = TRUE;

		// Verify GetSpecification returns the correct information
		if(FAILED(m_pCRowset1->m_hr=m_pCRowset1->GetSpecification(IID_IUnknown, &m_pCRowset1->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset1->CheckGetSpecRetval(m_pCRowset1->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset2->m_hr=m_pCRowset2->GetSpecification(IID_IUnknown, &m_pCRowset2->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset2->CheckGetSpecRetval(m_pCRowset2->m_pSpecification))
			bTestFailed = TRUE;

		if(FAILED(m_pCRowset3->m_hr=m_pCRowset3->GetSpecification(IID_IUnknown, &m_pCRowset3->m_pSpecification)))
			bTestFailed = TRUE;
		else if(!m_pCRowset3->CheckGetSpecRetval(m_pCRowset3->m_pSpecification))
			bTestFailed = TRUE;
	}

	ReleaseRowset(m_pCRowset1);
	ReleaseRowset(m_pCRowset2);
	ReleaseRowset(m_pCRowset3);

   	if(bTestFailed)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropSequence::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRefRowsetBoundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRefRowsetBoundary - GetReferenceRowset Boundary Tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetBoundary::Init()
{
	// Allocate a property list of length 1 so we can request bookmarks
	// for those variations which want them.
	m_rgBookmarkProperties = NULL;
	m_rgBookmarkProperties = (DBEXPECTEDPROPERTY *)PROVIDER_ALLOC(sizeof(DBEXPECTEDPROPERTY));

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc Without a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_1()
{
	BOOL	 fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks are turned off.
	if((!g_fBookmarksSettable) && (g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks are always enabled <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_FALSE
	ADD_FALSE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(1, IID_IUnknown, &pReferencedRowset), DB_E_NOTAREFERENCECOLUMN))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Without a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_2()
{
	BOOL	 fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks are turned off.
	if((!g_fBookmarksSettable) && (g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks are always enabled <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_FALSE
	ADD_FALSE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(0, IID_IUnknown, &pReferencedRowset), DB_E_BADORDINAL))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc Without a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_3()
{
	BOOL	 fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks are turned off.
	if((!g_fBookmarksSettable) && (g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks are always enabled <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_FALSE
	ADD_FALSE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(ULONG_MAX, IID_IUnknown, 
										&pReferencedRowset), DB_E_BADORDINAL))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc With a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_4()
{
	BOOL	 fSuccess = FALSE;

	// This variation requires bookmarks be supported.
	if((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}
	
	// Add BOOKMARK Property with VARIANT_TRUE
	ADD_TRUE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(0, IID_IUnknown, NULL), E_INVALIDARG))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc With a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_5()
{
	BOOL	 fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks be supported.
	if((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_TRUE
	ADD_TRUE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(0, IID_ICommand, 
									&pReferencedRowset), E_NOINTERFACE))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//--------------------------------------------------------------------
// @mfunc With a Bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_6()
{
	BOOL	 fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks be supported.
	if((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_TRUE
	ADD_TRUE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(!CHECK(GetReferencedRowset(ULONG_MAX, IID_IUnknown, 
										&pReferencedRowset), DB_E_BADORDINAL))
		goto CLEANUP;

	// Cleanup is required following a failure from this point on...
	if(CHECK(GetReferencedRowset(1, IID_IUnknown, 
								&pReferencedRowset), DB_E_NOTAREFERENCECOLUMN))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();
	
	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//--------------------------------------------------------------------
// @mfunc Get all madatory IID's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetBoundary::Variation_7()
{
	BOOL fSuccess = FALSE;
	IUnknown *pReferencedRowset = INVALID(IUnknown*);

	// This variation requires bookmarks be supported.
	if((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Add BOOKMARK Property with VARIANT_TRUE
	ADD_TRUE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset, 
				(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties),S_OK))
		goto CLEANUP;

	// IID_IConvertType
	if(!CHECK(GetReferencedRowset(0,IID_IConvertType,&pReferencedRowset),S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;
		
	// IID_IUnknown
	if(!CHECK(GetReferencedRowset(0,IID_IUnknown,&pReferencedRowset),S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;

	// IID_IAccessor
	if(!CHECK(GetReferencedRowset(0,IID_IAccessor,&pReferencedRowset),S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;
		
	// IID_IRowset
	if(!CHECK(GetReferencedRowset(0,IID_IRowset,&pReferencedRowset), S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;

	// IID_IColumnsInfo
	if(!CHECK(GetReferencedRowset(0,IID_IColumnsInfo,&pReferencedRowset),S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;

	// IID_IRowsetInfo
	if(!CHECK(GetReferencedRowset(0,IID_IRowsetInfo,&pReferencedRowset),S_OK))
		goto CLEANUP;
	if(!CheckGetRRRetval(pReferencedRowset))
		goto CLEANUP;
		
	fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if(pReferencedRowset != INVALID(IUnknown*))
		SAFE_RELEASE(pReferencedRowset);
	
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
BOOL TCGetRefRowsetBoundary::Terminate()
{
	// Free the memory
	PROVIDER_FREE(m_rgBookmarkProperties);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRefRowsetProp)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRefRowsetProp - GetReferencedRowset Param Tests: Properties
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetProp::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetRefRSParameters::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc Entire Test Table
//
// @rdesc TEST_PASS or TEST_FAIL
//   Returns TEST_FAIL if one or more table entry tests failed.
//
int TCGetRefRowsetProp::Variation_1()
{

#define LAST         0xFFFFFF01ul
#define MIDDLE       0xFFFFFF02ul
#define LAST_MINUS_1 0xFFFFFF03ul
#define LAST_PLUS_1  0xFFFFFF04ul
#define BIG_ORDINAL  0xFFFFFFFFul
#define TEST_ENTRY(a,b,c) {a,b,c,L#a,L#b,L#c,__LINE__}

	//  This test gives a table specifying a rowset created with a given
	//  property, an iOrdinal with which to call GetReferencedRowset, and
	//  an expected result.
	//
	//  The 3 columns of the table are captured by the structure below,
	//  with additional fields used to stringize the data for logging
	//  purposes.
	//
	//  iOrdinal values in the table are integer values; special values
	//  have been created for where the table specifies a column relative
	//  to the table size (i.e., LAST).  In these situations the actual
	//  iOrdinal value will be computed prior to calling GetRefRowset.
	//
	//  The table appears below that, with entries built by a macro which
	//  properly fills the data fields and stringizes the logging fields.
	typedef struct {
		DBPROPID m_idTestProperty;
		ULONG    m_iOrdinal;
		HRESULT  m_hrExpectedResult;

		WCHAR    *m_szTestPropertyString;
		WCHAR    *m_szColumnString;
		WCHAR    *m_szExpectedResultString;
		ULONG    m_uLine;
	} TestCase;

	static TestCase rgTestCases[] = {
		TEST_ENTRY(DBPROP_BOOKMARKS,                  0, S_OK),
		TEST_ENTRY(DBPROP_BOOKMARKS,                  1, DB_E_NOTAREFERENCECOLUMN),
		TEST_ENTRY(DBPROP_BOOKMARKS,        LAST_PLUS_1, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_BOOKMARKS,                300, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_BOOKMARKS,        BIG_ORDINAL, DB_E_BADORDINAL),
		
		TEST_ENTRY(DBPROP_LITERALBOOKMARKS,           0, S_OK), 
		TEST_ENTRY(DBPROP_LITERALBOOKMARKS,LAST_MINUS_1, DB_E_NOTAREFERENCECOLUMN),
		
		TEST_ENTRY(DBPROP_ORDEREDBOOKMARKS,           0, S_OK),
		TEST_ENTRY(DBPROP_ORDEREDBOOKMARKS,      MIDDLE, DB_E_NOTAREFERENCECOLUMN),
		
		TEST_ENTRY(DBPROP_IRowsetLocate,              0, S_OK),
		TEST_ENTRY(DBPROP_IRowsetLocate,              2, DB_E_NOTAREFERENCECOLUMN),

		TEST_ENTRY(DBPROP_IRowsetScroll,			  0, S_OK),
		TEST_ENTRY(DBPROP_IRowsetScroll,		   LAST, DB_E_NOTAREFERENCECOLUMN),

		TEST_ENTRY(DBPROP_IRowsetFind,                0, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_IRowsetFind,                3, DB_E_NOTAREFERENCECOLUMN),
		TEST_ENTRY(DBPROP_IRowsetFind,      LAST_PLUS_1, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_IRowsetFind,              300, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_IRowsetFind,      BIG_ORDINAL, DB_E_BADORDINAL),

		TEST_ENTRY(DBPROP_NOTSUPPORTEDINTERFACE,            0, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_NOTSUPPORTEDINTERFACE, LAST_MINUS_1, DB_E_NOTAREFERENCECOLUMN),
		TEST_ENTRY(DBPROP_NOTSUPPORTEDINTERFACE,  LAST_PLUS_1, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_NOTSUPPORTEDINTERFACE,          300, DB_E_BADORDINAL),
		TEST_ENTRY(DBPROP_NOTSUPPORTEDINTERFACE,  BIG_ORDINAL, DB_E_BADORDINAL)
	};

	static UWORD uNumTestCases =
		sizeof(rgTestCases) / sizeof(rgTestCases[0]);

	HRESULT				hr;
	UWORD				uTestCaseIndex;
	TestCase			*pThisTestCase;
	BOOL				bThisTestPassed;      // if one test fails, others continue
	DBORDINAL			iOrdinal;
	WCHAR				szTmpBuf[128];
	DBEXPECTEDPROPERTY rgTestProperty[1];
	ULONG				uPass = 0;
	ULONG				uFail = 0;
	ULONG				uSkip = 0;

	EQUERY fQueryToUse = USE_OPENROWSET; // arbitrary choice for this test

	for (uTestCaseIndex = 0; uTestCaseIndex < uNumTestCases; uTestCaseIndex++) 
	{

		pThisTestCase = &rgTestCases[uTestCaseIndex];
		bThisTestPassed = FALSE;

		//  TODO Is there a way to find out if the test property is supported, so that
		//  TODO the test won't fail if it is not?

		swprintf(szTmpBuf, L"Testing #%u: %s(%s) -> %s [line %lu]\n",
					uTestCaseIndex,
					pThisTestCase->m_szTestPropertyString,
					pThisTestCase->m_szColumnString,
					pThisTestCase->m_szExpectedResultString,
					pThisTestCase->m_uLine);
		odtLog << szTmpBuf;

		// Create a rowset with any arbitrary method.
		// Set the test property.
		//
		SetupBooleanProperty(&rgTestProperty[0],
									pThisTestCase->m_idTestProperty,
									VARIANT_TRUE,
									pThisTestCase->m_szTestPropertyString,
									pThisTestCase->m_uLine);

		hr = CreateRowsetObjectWithInfo(fQueryToUse,
												  IID_IRowset,
												  1, rgTestProperty);

		if (hr == DB_E_ERRORSOCCURRED) 
		{ 
			// this means required properties were not supported
			uSkip++;
			continue;
		}

		if (CHECK(hr, S_OK)) 
		{
			// This variation requires that we successfully received the
			// columns count.
			if (!m_cRowsetColumns) 
				odtLog << L"Couldn't get column count" <<ENDL;
			else 
			{
				//  Figure out which column the test case wants
				//  Sometimes it wants something in the middle.
				switch (pThisTestCase->m_iOrdinal) 
				{
				  case MIDDLE:
					iOrdinal = (m_cRowsetColumns+1) / 2;
					if (iOrdinal < 1) 
						iOrdinal = 1;
					break;

				  case LAST_MINUS_1:
					iOrdinal = m_cRowsetColumns - 1;
					break;

				  case LAST:
					iOrdinal = m_cRowsetColumns;
					break;

				  case LAST_PLUS_1:
					iOrdinal = m_cRowsetColumns + 1;
					break;

				  default:
					  iOrdinal = pThisTestCase->m_iOrdinal;
				}


				//  Here's the nitty-gritty: Call the function and see that
				//  it did what we wanted.
				//

				hr = GetReferencedRowset(iOrdinal, IID_IUnknown, &m_pReferencedRowset);
				//if BOOKMARKS are not supported, should return DB_E_BADORDINAL
				if(!g_fBookmarksSupported && iOrdinal==0)
				{
					if(CHECK(hr,DB_E_BADORDINAL) && COMPARE(m_pReferencedRowset, NULL))
						bThisTestPassed = TRUE;
				}
				else if(g_fBookmarksSupported && iOrdinal==0)
				{
					// Check if bookmarks are supported on this rowset.
					VARIANT_BOOL bValue;

					if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, this->m_pIRowsetInfo, &bValue))
					{
						if (bValue == VARIANT_TRUE)
						{

							if(CHECK(hr,S_OK) && CheckGetRRRetval(m_pReferencedRowset))
								bThisTestPassed = TRUE;
						}
						else
						{
							if (CHECK(hr, DB_E_BADORDINAL))
								bThisTestPassed = TRUE;
						}
					}
					else
					{
						if (CHECK(hr, DB_E_BADORDINAL))
							bThisTestPassed = TRUE;
					}
				}

				else if(iOrdinal > m_cRowsetColumns)
				{
					if(CHECK(hr,DB_E_BADORDINAL) && COMPARE(m_pReferencedRowset, NULL))
						bThisTestPassed = TRUE;
				}
				else if(CHECK(hr,pThisTestCase->m_hrExpectedResult))
				{
					//  Check returned referenced rowset against anything?
					if (g_fBookmarksSupported && pThisTestCase->m_hrExpectedResult == S_OK) 
					{
						if (CheckGetRRRetval(m_pReferencedRowset))
							bThisTestPassed = TRUE;
					}
					else 
					{
						//  Correct error returned: test passed
						if(FAILED(hr) && COMPARE(m_pReferencedRowset, NULL))
							bThisTestPassed = TRUE;
					}
				}
				else
					assert(0);
			}

			ReleaseRowsetObjectWithInfo();

			if (!bThisTestPassed) 
			{
				odtLog << L"" <<ENDL;
				uFail++;
			}
			else 
			{
				uPass++;
			}
		}
	}

	odtLog <<L"Variation summary:"  <<ENDL;
	odtLog <<L"...Passed: "  <<uPass <<ENDL;
	odtLog <<L"...Failed: "  <<uFail <<ENDL;
	odtLog <<L"...Skipped: " <<uSkip <<ENDL <<ENDL;

	if (uFail)
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetProp::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetRefRSParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRefRowsetICol)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRefRowsetICol - GetReferencedRowset Param Tests: iOrdinal
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetICol::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetRefRSParameters::Init())
	// }}
    {
		if(CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc iOrdinal=# of columns in the Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetICol::Variation_1()
{
	if(CHECK(GetReferencedRowset(m_cRowsetColumns, IID_IUnknown, 
							&m_pReferencedRowset),DB_E_NOTAREFERENCECOLUMN)) 
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc iOrdinal=# of columns in the rowset + 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetICol::Variation_2()
{
	if(CHECK(GetReferencedRowset(m_cRowsetColumns+1, IID_IUnknown, 
									&m_pReferencedRowset),DB_E_BADORDINAL))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc iOrdinal=ULONG_MAX
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetICol::Variation_3()
{
	if(CHECK(GetReferencedRowset(ULONG_MAX, IID_IUnknown, 
									&m_pReferencedRowset),DB_E_BADORDINAL))
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
BOOL TCGetRefRowsetICol::Terminate()
{
	// Release the rowset.
   ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetRefRSParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRefRowsetNoBmk)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRefRowsetNoBmk - GetReferencedRowset Parameter Tests: No Bookmarks
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetNoBmk::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetRefRSParameters::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc By ICommand::Execute
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetNoBmk::Variation_1()
{
	BOOL	fSuccess = FALSE;
	HRESULT ExpHR	 = DB_E_BADORDINAL;

	// This variation requires that commands be supported.
	if(!GetCommandSupport()) {
		odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Create a rowset by ICommand::Execute()
	if(!CHECK(CreateRowsetObjectWithInfo(SELECT_ALLFROMTBL, IID_IRowset),S_OK))
		goto CLEANUP;

	// Figure out the return code for commands
	if( (m_pICommand) && 
		(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand)) )
		ExpHR = S_OK;

	if(CHECK(GetReferencedRowset(0, IID_IUnknown, 
										&m_pReferencedRowset),ExpHR))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	SAFE_RELEASE(m_pReferencedRowset);
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc By IOpenRowset::OpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetNoBmk::Variation_2()
{
	BOOL fSuccess = FALSE;

	// Create a rowset by IOpenRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		goto CLEANUP;

	if(CHECK(GetReferencedRowset(1, IID_IUnknown, 
							&m_pReferencedRowset),DB_E_NOTAREFERENCECOLUMN))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc By IDBSchemaRowset::GetRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetNoBmk::Variation_3()
{
	BOOL fSuccess = FALSE;

	// This variation requires that schemas are supported
	if(!g_fSchemaRowSupported) {
		odtLog << L"Variation skipped: SchemaRowsets not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	// Create a rowset by IIDSchemaRowset
	if(!CHECK(CreateRowsetObjectWithInfo(SELECT_DBSCHEMA_TABLE, IID_IRowset),S_OK))
		goto CLEANUP;

	if(CHECK(GetReferencedRowset(ULONG_MAX, IID_IUnknown, 
										&m_pReferencedRowset),DB_E_BADORDINAL))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//--------------------------------------------------------------------
// @mfunc By IDBEnumerateSources::Sources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRefRowsetNoBmk::Variation_4()
{
#ifdef ENUMSOURCES
	BOOL fSuccess = FALSE;

	// Create a rowset by ISourcesRowset
	if(!CHECK(CreateRowsetObjectWithInfo(USE_ENUMSOURCES, IID_IRowset),S_OK))
		goto CLEANUP;

	if(CHECK(GetReferencedRowset(m_cRowsetColumns, IID_IUnknown, 
							&m_pReferencedRowset),DB_E_NOTAREFERENCECOLUMN))
		fSuccess = TRUE;

CLEANUP:

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL;
	return TEST_PASS;
#endif
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRefRowsetNoBmk::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetRefRSParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecBoundary)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecBoundary - GetSpecification Boundary Tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecBoundary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
   {
		if(CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
			return TRUE;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecBoundary::Variation_1()
{
	if(CHECK(m_hr=GetSpecification(IID_IUnknown, NULL), E_INVALIDARG))
		return TEST_PASS;
	
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecBoundary::Variation_2()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, NULL), E_INVALIDARG))
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
BOOL TCGetSpecBoundary::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecParamByExecute)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecParamByExecute - GetSpecification Parameters: Rowset created by lCommand::Execute
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamByExecute::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetSpecParameters::Init())
	// }}
	{
		// This variation requires that commands are supported
		if(!GetCommandSupport()) {
			odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
			return TEST_SKIPPED;
		}
		
		// Create a rowset from a command object
		if(CHECK(CreateRowsetObjectWithInfo(SELECT_ALLFROMTBL, IID_IRowset),S_OK))
			return TRUE;
	}
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByExecute::Variation_1()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, &m_pSpecification),E_NOINTERFACE))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc IID_IDBCreateCommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByExecute::Variation_2()
{
	if(CHECK(m_hr=GetSpecification(IID_IDBCreateCommand, &m_pSpecification),E_NOINTERFACE))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByExecute::Variation_3()
{
	if(!CHECK(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification), S_OK)) 
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification))
		return TEST_FAIL;
	
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamByExecute::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSpecParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecParamByOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecParamByOpenRowset - GetSpecification Parameters: Rowset created by OpenRowset
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamByOpenRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetSpecParameters::Init())
	// }}
	{
		// Create a rowset with OpenRowset [via Private Library]
		if(CHECK(CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByOpenRowset::Variation_1()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, &m_pSpecification), E_NOINTERFACE))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc IID_ICommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByOpenRowset::Variation_2()
{
	if(CHECK(m_hr=GetSpecification(IID_ICommand, &m_pSpecification), E_NOINTERFACE))
		return TEST_PASS;

	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByOpenRowset::Variation_3()
{
	if(!CHECK(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification), S_OK)) 
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;
	
	return TEST_PASS;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamByOpenRowset::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSpecParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecParamBySchemaRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecParamBySchemaRowset - GetSpecification Parameters: rowset created by GetRowset
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamBySchemaRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetSpecParameters::Init())
	// }}
	{
		// This variation requires that schemas are supported
		if (!g_fSchemaRowSupported) {
			odtLog << L"Variation skipped: SchemaRowsets not supported <not a failure>." <<ENDL;
			return TEST_SKIPPED;
		}

		// Create a rowset by IDBSchemaRowset::GetRowset() [via Private Library]
		if(CHECK(CreateRowsetObjectWithInfo(SELECT_DBSCHEMA_TABLE, IID_IRowset),S_OK))
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySchemaRowset::Variation_1()
{
	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification))) 
		return TEST_FAIL;

	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBCreateCommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySchemaRowset::Variation_2()
{
	// Check to see if Commands are supported
	if(!m_pIDBCreateCommand)
	{
		if(!CHECK(m_hr=GetSpecification(IID_IDBCreateCommand, &m_pSpecification), E_NOINTERFACE)) 
			return TEST_FAIL;
	}
	else
	{
		if(FAILED(m_hr=GetSpecification(IID_IDBCreateCommand, &m_pSpecification))) 
			return TEST_FAIL;

		// check return value...
		if(!CheckGetSpecRetval(m_pSpecification)) 
			return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBSchemaRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySchemaRowset::Variation_3()
{
	if(FAILED(m_hr=GetSpecification(IID_IDBSchemaRowset, &m_pSpecification)))
		return TEST_FAIL;

	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IID_ICommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySchemaRowset::Variation_4()
{
	if(CHECK(m_hr=GetSpecification(IID_ICommand, &m_pSpecification), E_NOINTERFACE)) 
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySchemaRowset::Variation_5()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, &m_pSpecification), E_NOINTERFACE)) 
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
BOOL TCGetSpecParamBySchemaRowset::Terminate()
{
	// Release the rowset object
	ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSpecParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecParamBySources)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecParamBySources - GetSpecification Parameters: Rowset created by Sources
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamBySources::Init()
{
#ifdef ENUMSOURCES
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CGetSpecParameters::Init())
	// }}
	{
		// Create a rowset by IDBSchemaRowset::GetRowset() [via Private Library]
		if(CHECK(CreateRowsetObjectWithInfo(SELECT_USE_ENUMSOURCES, IID_IRowset),S_OK))
			return TRUE;
	}	

	return FALSE;
#else
	odtLog << L"EnumerateSources has been removed and will not be tested." <<ENDL
			<< L"This entire test case will be skipped." <<ENDL;
	return TEST_SKIPPED;
#endif
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_1()
{
	if(!CHECK(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification), S_OK)) 
		return TEST_FAIL;

	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification))
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_2()
{
	if(!CHECK(m_hr=GetSpecification(IID_IDBProperties, &m_pSpecification), S_OK)) 
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBInitialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_3()
{
	if(!CHECK(m_hr=GetSpecification(IID_IDBInitialize, &m_pSpecification), S_OK)) 
		return TEST_FAIL;

	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBEnumerateSources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_4()
{
	if(!CHECK(m_hr=GetSpecification(IID_ISourcesRowset, &m_pSpecification), S_OK)) 
		return TEST_FAIL;

	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IID_ICommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_5()
{
	if(CHECK(m_hr=GetSpecification(IID_ICommand, &m_pSpecification), E_NOINTERFACE))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamBySources::Variation_6()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, &m_pSpecification), E_NOINTERFACE)) 
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
BOOL TCGetSpecParamBySources::Terminate()
{
	// Release the rowset object
   ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSpecParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetSpecParamByColRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetSpecParamByColRowset - GetSpecification Parameters: Rowset created by IColumnsRowset::GetColumnsRowset
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetSpecParamByColRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		// This variation requires that schemas are supported
		if(!g_fColumnRowSupported) {
			odtLog << L"Variation skipped: ColumnsRowset not supported <not a failure>." <<ENDL;
			return TEST_SKIPPED;
		}

		// Create a rowset by IDBSchemaRowset::GetRowset() [via Private Library]
		if(CHECK(CreateRowsetObjectWithInfo(USE_GETCOLROWSET, IID_IRowset),S_OK))
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_1()
{
	if(!CHECK(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification), S_OK))
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBProperties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_2()
{
	// Check to see if Commands are supported
	if(!m_pIDBCreateCommand)
	{
		if(!CHECK(m_hr=GetSpecification(IID_IDBCreateCommand, &m_pSpecification), E_NOINTERFACE))
			return TEST_FAIL;
	}
	else
	{
		if(!CHECK(m_hr=GetSpecification(IID_IDBCreateCommand, &m_pSpecification), S_OK))
			return TEST_FAIL;
		
		// check return value...
		if(!CheckGetSpecRetval(m_pSpecification)) 
			return TEST_FAIL;
	}
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBInitialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_3()
{
	if(!CHECK(m_hr=GetSpecification(IID_IOpenRowset, &m_pSpecification), S_OK))
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IID_IDBEnumerateSources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_4()
{
	if(!CHECK(m_hr=GetSpecification(IID_IDBSchemaRowset, &m_pSpecification), S_OK))
		return TEST_FAIL;
	
	// check return value...
	if(!CheckGetSpecRetval(m_pSpecification)) 
		return TEST_FAIL;
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IID_ICommand
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_5()
{
	if(CHECK(m_hr=GetSpecification(IID_ICommand, &m_pSpecification), E_NOINTERFACE))
		return TEST_PASS;
		
	return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetSpecParamByColRowset::Variation_6()
{
	if(CHECK(m_hr=GetSpecification(IID_IRowsetInfo, &m_pSpecification), E_NOINTERFACE))
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
BOOL TCGetSpecParamByColRowset::Terminate()
{
	// Release the rowset object
   ReleaseRowsetObjectWithInfo();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CGetSpecParameters::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombieTests)
//*-----------------------------------------------------------------------
//| Test Case:		TCZombieTests - Zombie State Tests
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombieTests::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		// Register Interface with Zombie
		if(RegisterInterface(ROWSET_INTERFACE,IID_IRowsetInfo,0,NULL))
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IRowsetInfo with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieTests::Variation_1()
{
	// S_OK - Abort IRowsetInfo with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IRowsetInfo with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieTests::Variation_2()
{
	// S_OK - Abort IRowsetInfo with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IRowsetInfo with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieTests::Variation_3()
{
	// S_OK - Commit IRowsetInfo with fRetaining=FALSE
	return TestTxn(ETXN_COMMIT, FALSE);

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IRowsetInfo with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombieTests::Variation_4()
{
	// S_OK - Commit IRowsetInfo with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombieTests::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCMultiRowsets)
//*-----------------------------------------------------------------------
//| Test Case:		TCMultiRowsets - Multiple (Concurrent) Rowsets
//|	Created:		04/09/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc Set up and save a set of rowsets created concurrently on the same object.
//
// If any rowset cannot be created, all rowsets created to that point are cleared
// and TEST_FAIL is returned.
//
// @rdesc TRUE or FALSE
//
BOOL TCMultiRowsets::SetupMultiRowsets(ULONG uNumRowsets, ...)
{
	ULONG uIndex=0;
	EQUERY fQuery;
	va_list ap;

	va_start(ap, uNumRowsets);

	for(uIndex=0; uIndex < uNumRowsets; uIndex++) 
	{
		fQuery = va_arg(ap, EQUERY);

		if(!CHECK(CreateRowsetObjectWithInfo(fQuery, IID_IRowset),S_OK))
			break;

		m_RowsetPtrs[uIndex].m_fSourceQuery     = fQuery;
		m_RowsetPtrs[uIndex].m_cTblCols         = m_pTable->GetRowsOnCTable();
		m_RowsetPtrs[uIndex].m_rgTableColOrds	= m_rgTableColOrds;
		m_RowsetPtrs[uIndex].m_pszSQLStmt       = m_pszSQLStatement;
		m_RowsetPtrs[uIndex].m_pICommand        = m_pICommand;
		m_RowsetPtrs[uIndex].m_pIAccessor       = m_pIAccessor;
		m_RowsetPtrs[uIndex].m_pIRowsetInfo     = m_pIRowsetInfo;
		m_RowsetPtrs[uIndex].m_pICmdPrepare     = m_pICmdPrepare;
		m_RowsetPtrs[uIndex].m_pICreator        = m_pICreator;

		// Cleared, in case the creation of the next rowset makes
		// CreateRowsetObject cleanup any leftover pointers.
		m_rgTableColOrds  = NULL;
		m_pszSQLStatement = NULL;
		m_pICommand       = NULL;
		m_pIAccessor      = NULL;
		m_pIRowsetInfo    = NULL;
		m_pICmdPrepare    = NULL;
		m_pICreator       = NULL;
	}

	va_end(ap);

	// didn't make all the rowsets: something went wrong in the
	// loop.  Free the rowsets and exit.
	if (uIndex < uNumRowsets) 
	{
		m_uNumRowsets = uIndex;
		CleanupMultiRowsets();
		return FALSE;
	}
	else 
	{
		m_uNumRowsets = uNumRowsets;
		return TRUE;
	}
}

//--------------------------------------------------------------------
// @mfunc Set a given rowset (from the set created by SetupMultiRowsets
//  as the "current" rowset.  The current rowset is the one to which
//  the CRowsetInfoSupport, CRowset, CCommand, and other methods will
//  be applied.
//
// uRowsetNum is a 1-based indication of which rowset to used.
//
// @rdesc TRUE or FALSE
//
BOOL TCMultiRowsets::SetCurrentRowset(ULONG uRowsetNum)
{
	// the row numbers are 1-based, the array 0-based
	--uRowsetNum;

	// fQuery            = m_RowsetPtrs[uRowsetNum].m_fSourceQuery;
	m_rgTableColOrds  = m_RowsetPtrs[uRowsetNum].m_rgTableColOrds;
	m_pszSQLStatement = m_RowsetPtrs[uRowsetNum].m_pszSQLStmt;
	m_pICommand       = m_RowsetPtrs[uRowsetNum].m_pICommand;
	m_pIAccessor      = m_RowsetPtrs[uRowsetNum].m_pIAccessor;
	m_pIRowsetInfo    = m_RowsetPtrs[uRowsetNum].m_pIRowsetInfo;
	m_pICmdPrepare    = m_RowsetPtrs[uRowsetNum].m_pICmdPrepare;
	m_pICreator       = m_RowsetPtrs[uRowsetNum].m_pICreator;

	return TRUE;
}

//--------------------------------------------------------------------
// @mfunc Release the set of rowsets created for this test
//
// @rdesc TRUE or FALSE
//
BOOL TCMultiRowsets::CleanupMultiRowsets(void)
{
	for(ULONG uIndex = 1; uIndex <= m_uNumRowsets; uIndex++) 
	{
		SetCurrentRowset(uIndex);
		ReleaseRowsetObjectWithInfo();
	}

	// Reset to zero
	m_uNumRowsets=0;

	return TRUE;
}



//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMultiRowsets::Init()
{
	m_uNumRowsets = 0;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Rowsets, 2 Command Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultiRowsets::Variation_1()
{
	BOOL		fSuccess	= FALSE;
	ULONG_PTR	ulValue		= 0;

	// Figure out how many sessions are valid
	GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, 
												m_pIDBInitialize, &ulValue);
	if(ulValue == 1) {
		odtLog << L"Variation skipped: Max open sessions is one." <<ENDL;
		return TEST_SKIPPED;
	}

	// This variation requires that commands are supported
	if(!GetCommandSupport()) {
		odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}
	
	if(!SetupMultiRowsets(2, SELECT_ALLFROMTBL, SELECT_ALLFROMTBL))
		goto CLEANUP;

	SetCurrentRowset(1);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	SetCurrentRowset(2);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	CleanupMultiRowsets();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Rowsets, 2 OpenRowset Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultiRowsets::Variation_2()
{
	BOOL		fSuccess	= FALSE;
	ULONG_PTR	ulValue		= 0;

	// Figure out how many sessions are valid
	GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, 
												m_pIDBInitialize, &ulValue);
	if(ulValue == 1) {
		odtLog << L"Variation skipped: Max open sessions is one." <<ENDL;
		return TEST_SKIPPED;
	}

	if(!SetupMultiRowsets(2, USE_OPENROWSET, USE_OPENROWSET))
		goto CLEANUP;

	SetCurrentRowset(1);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	SetCurrentRowset(2);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	CleanupMultiRowsets();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Multiple Rowsets, 1 Command, 1 OpenRowset Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultiRowsets::Variation_3()
{
	BOOL		fSuccess	= FALSE;
	ULONG_PTR	ulValue		= 0;

	// Figure out how many sessions are valid
	GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, 
												m_pIDBInitialize, &ulValue);
	if(ulValue == 1) {
		odtLog << L"Variation skipped: Max open sessions is one." <<ENDL;
		return TEST_SKIPPED;
	}

	// This variation requires that commands are supported
	if(!GetCommandSupport()) {
		odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}
	
	if(!SetupMultiRowsets(2, SELECT_ALLFROMTBL, USE_OPENROWSET))
		goto CLEANUP;

	SetCurrentRowset(1);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	SetCurrentRowset(2);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	CleanupMultiRowsets();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Multiple Rowsets, 1 OpenRowset, 1 Command Objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultiRowsets::Variation_4()
{
	BOOL		fSuccess	= FALSE;
	ULONG_PTR	ulValue		= 0;

	// Figure out how many sessions are valid
	GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, 
												m_pIDBInitialize, &ulValue);
	if(ulValue == 1) {
		odtLog << L"Variation skipped: Max open sessions is one." <<ENDL;
		return TEST_SKIPPED;
	}

	// This variation requires that commands are supported
	if(!GetCommandSupport()) {
		odtLog << L"Variation skipped: Commands not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}

	if(!SetupMultiRowsets(2, USE_OPENROWSET, SELECT_ALLFROMTBL))
		goto CLEANUP;

	SetCurrentRowset(1);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	SetCurrentRowset(2);

	if(FAILED(m_hr=GetSpecification(IID_IUnknown, &m_pSpecification)))
		goto CLEANUP;

	if(!CheckGetSpecRetval(m_pSpecification))
		goto CLEANUP;

	fSuccess = TRUE;

CLEANUP:

	CleanupMultiRowsets();

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
BOOL TCMultiRowsets::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CRowsetInfoSupport::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		07/22/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CRowsetInfoSupport::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}

 
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowsetInfo calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{  
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr=CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;
	
	// Cause an Error
	m_pExtError->CauseError();
	
	// Call GetProperties
	if(CHECK(hr=m_pIRowsetInfo->GetProperties(0, NULL, 
									&m_cPropertySets, &m_rgPropertySets), S_OK))
		fSuccess = TRUE;

	if(!m_cPropertySets || !m_rgPropertySets)
		fSuccess &= FALSE;

	// Do extended check following IsSameRow
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	// Free memory from GetProperties
	FreeProperties(&m_cPropertySets, &m_rgPropertySets);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Valid IRowsetInfo calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;
	IUnknown *ppReferencedRowset = NULL;

	// This variation requires bookmarks be supported.
	if((!g_fBookmarksSettable) && (!g_fBookmarksOnByDft)) {
		odtLog << L"Variation skipped: Bookmarks not supported <not a failure>." <<ENDL;
		return TEST_SKIPPED;
	}
	
	// Allocate a property list of length 1 so we can request bookmarks
	m_rgBookmarkProperties =
		 (DBEXPECTEDPROPERTY *) PROVIDER_ALLOC(sizeof(DBEXPECTEDPROPERTY));
	
	ADD_TRUE_PROP(&m_rgBookmarkProperties[0], DBPROP_BOOKMARKS);

	// For each method of the interface, first create an error object on
	// the current thread, try get a success from the IRowsetInfo method.
	// We then check extended errors to verify the right extended error behavior.
	if(!CHECK(hr=CreateRowsetObjectWithInfo(USE_OPENROWSET,IID_IRowset, 
		(g_fBookmarksSupported) ? 1 : 0, m_rgBookmarkProperties), S_OK))
		goto CLEANUP;

	// Cause an Error
	m_pExtError->CauseError();

	if(CHECK(hr=m_pIRowsetInfo->GetReferencedRowset(0,IID_IUnknown,&ppReferencedRowset),
		(g_fBookmarksSupported) ? S_OK : DB_E_BADORDINAL))
		fSuccess = TRUE;

	// Do extended check following IsSameRow
	fSuccess = XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	

CLEANUP:	

	// Release all of the Interfaces
	SAFE_RELEASE(ppReferencedRowset);
	PROVIDER_FREE(m_rgBookmarkProperties);

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc valid IRowsetInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{  
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;
	
	// Create a rowset with any arbitrary method.
	if(!CHECK(hr=CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;

	// Cause an Error
	m_pExtError->CauseError();
	
	// Call GetSpecification
	if(SUCCEEDED(hr=m_pIRowsetInfo->GetSpecification(IID_IUnknown, 
													&m_pSpecification)))
		fSuccess = TRUE;

	if(!m_pSpecification)
		fSuccess &= FALSE;

	// Do extended error check
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	
	
	// Release all of the Interfaces
	SAFE_RELEASE(m_pSpecification);
	
	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}}



// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{	
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr=CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;
	
	// Cause an Error
	m_pExtError->CauseError();
	
	// Call GetProperties
	if(CHECK(hr=m_pIRowsetInfo->GetProperties(1, NULL, &m_cPropertySets, 
											&m_rgPropertySets), E_INVALIDARG))
		fSuccess = TRUE;
	
	if(m_cPropertySets || m_rgPropertySets)
		fSuccess &= FALSE;

	// Do extended check following IsSameRow
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	
	 	
	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr = CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;

	// Cause an Error
	m_pExtError->CauseError();
	
	if(CHECK(hr=m_pIRowsetInfo->GetReferencedRowset(1, IID_IUnknown, NULL), E_INVALIDARG))
		fSuccess = TRUE;
	
	// Do extended check following IsSameRow
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	
	
	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr = CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;

	// Cause an Error
	m_pExtError->CauseError();
	
	if(CHECK(hr=m_pIRowsetInfo->GetSpecification(IID_IRowsetInfo, 
												&m_pSpecification), E_NOINTERFACE))
		fSuccess = TRUE;

	// Do extended error check
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	

	if(m_pSpecification)
		fSuccess &= FALSE;

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with no error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_7()
{	
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr = CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;
	
	// Call GetProperties
	if(CHECK(hr=m_pIRowsetInfo->GetProperties(1, NULL,
						&m_cPropertySets, &m_rgPropertySets),E_INVALIDARG)) 
		fSuccess = TRUE;

	// Do extended check following IsSameRow
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	
	 	
	if(m_cPropertySets || m_rgPropertySets)
		fSuccess &= FALSE;

	// Release all of the Interfaces
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_8()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr = CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;

	if(CHECK(hr=m_pIRowsetInfo->GetReferencedRowset(1, IID_IUnknown, NULL), E_INVALIDARG))
		fSuccess = TRUE;
	
	// Do extended check following IsSameRow
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	
	
	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetInfo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_9()
{
	BOOL	  fSuccess	 = FALSE;
	HRESULT   hr		 = E_FAIL;

	// Create a rowset with any arbitrary method.
	if(!CHECK(hr = CreateRowsetObjectWithInfo(USE_OPENROWSET, IID_IRowset),S_OK))
		return TEST_FAIL;

	if(CHECK(hr=m_pIRowsetInfo->GetSpecification(IID_IRowsetInfo, 
												&m_pSpecification), E_NOINTERFACE))
		fSuccess = TRUE;
	// Do extended error check
	fSuccess &= XCHECK(m_pIRowsetInfo, IID_IRowsetInfo, hr);	

	if(m_pSpecification)
		fSuccess &= FALSE;

	// Release the rowset.
	ReleaseRowsetObjectWithInfo();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	//ReleaseRowsetObjectWithInfo();
	return(CRowsetInfoSupport::Terminate());
}	// }}

// }}


//--------------------------------------------------------------------
// @mfunc Set up a zombie state
//
// @rdesc TRUE or FALSE
//
int TCZombieTests::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	int					fPassFail			= TEST_FAIL;	// ReturnValue
	HRESULT				ExpectedHr			= E_UNEXPECTED;	// Expected HRESULT
	DBCOUNTITEM			cRowsObtained		= 0;			// Number of rows returned, should be 1
	HROW				*rghRows			= NULL;			// Array of Row Handles
	ULONG				cPropertySets		= 0;			// Number of PropSets
	DBPROPSET			*rgPropertySets		= NULL;			// Array of PropSets
	IRowsetInfo			*pIRowsetInfo		= NULL;			// IRowsetInfo
	IUnknown			*pSpecification		= NULL;			// Back Pointer
	IUnknown			*pSpecification1	= NULL;			// Back Pointer
	IUnknown			*pReferencedRowset	= NULL;			// Rowset Pointer to the rowset

	// Retrieve an Interface pointer to IRowsetInfo within a Transaction
	if(!StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIRowsetInfo))
		goto END;

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT) && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Commit or Abort the transaction, with retention as specified
	if( ((eTxn == ETXN_COMMIT) && (!GetCommit(fRetaining))) ||
	    ((eTxn == ETXN_ABORT)  && (!GetAbort(fRetaining))) )
		goto END;

	// Test zombie
	if(!CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr))
		goto END;
		
	// IRowsetInfo::GetProperties
	if(!CHECK(pIRowsetInfo->GetProperties(0, NULL,
								&cPropertySets, &rgPropertySets),ExpectedHr))
		goto END;

	// IRowsetInfo::GetSpecification (S_OK and S_FALSE valid)
	if( ((ExpectedHr == S_OK) && 
		 (FAILED(pIRowsetInfo->GetSpecification(IID_IUnknown, &pSpecification)))) ||
		(!CHECK(pIRowsetInfo->GetSpecification(IID_IUnknown, &pSpecification1), ExpectedHr)) )
		goto END;

	// IRowsetInfo::GetReferencedRowset
	if( (ExpectedHr == S_OK) && 
		(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, pIRowsetInfo)) )
		ExpectedHr = DB_E_BADORDINAL;

	if(CHECK(pIRowsetInfo->GetReferencedRowset(0, IID_IUnknown, 
												&pReferencedRowset), ExpectedHr))
		fPassFail = TEST_PASS;

END:

	// Release the row handle on the 1st rowset
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Release the Interfaces
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pReferencedRowset);
	SAFE_RELEASE(pSpecification);
	SAFE_RELEASE(pSpecification1);

	// Clear the Property memory
	FreeProperties(&cPropertySets, &rgPropertySets);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	return fPassFail;
}


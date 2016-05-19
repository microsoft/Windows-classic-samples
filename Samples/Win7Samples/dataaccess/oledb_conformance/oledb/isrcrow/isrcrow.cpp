//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module ISrcRow.CPP | Template source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include <process.h>
#include "ISrcRow.h"


const ULONG		nThreads=15;
unsigned WINAPI ThreadProc(LPVOID lpvThreadParam);

class CSourcesRowset;
typedef struct inparam{
	ULONG				i;
	CSourcesRowset	*pObject;
} CInParam;

//macros
#define FILL_PROP_SET(RGPROPSET_EL, NPROP, RGPROP, PROP_GUID)		\
	RGPROPSET_EL.cProperties		= NPROP;						\
	RGPROPSET_EL.rgProperties		= RGPROP;						\
	RGPROPSET_EL.guidPropertySet	= PROP_GUID;					\
	if (NULL != RGPROP)												\
		memset(RGPROP, 0, NPROP*sizeof(DBPROP));

#define FILL_PROP(RGPROP_EL, PROPID, VAR_TYPE, VAR_MACRO, VAR_VALUE, OPTION)		\
	memset(&RGPROP_EL, 0, sizeof(DBPROP));											\
	RGPROP_EL.dwPropertyID			= PROPID;										\
	RGPROP_EL.vValue.vt				= VAR_TYPE;										\
	VAR_MACRO(&RGPROP_EL.vValue)	= VAR_VALUE;									\
	RGPROP_EL.dwOptions				= OPTION;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xf6542601, 0x0bf4, 0x11d0, { 0xa5, 0x5b, 0x00, 0xa0, 0xc9, 0x0d, 0x60, 0x52 }};
DECLARE_MODULE_NAME("ISourcesRowset");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test for ISourcesRowset");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// @gmember Supported property marker
struct DBPropMark
{
	PROP_STATUS	status;
	//BOOL		settable;
	DBPROPID	dwPropertyID;
};

// @gmember Supported property marker
DBPropMark*	g_PropMark = NULL;
// @gmember The name of each column, skip the bookmark column
WCHAR*		g_rgwszColumnName[] = { L"SOURCES_NAME", L"SOURCES_PARSENAME", L"SOURCES_DESCRIPTION", L"SOURCES_TYPE", L"SOURCES_ISPARENT", L"SOURCES_CLSID" };
// @gmember The type of each column, skip the bookmark column
DBTYPE		g_ColumnType[] = {DBTYPE_WSTR, DBTYPE_WSTR, DBTYPE_WSTR, DBTYPE_UI2, DBTYPE_BOOL, DBTYPE_WSTR};
// @gmember An array hold SOURCE TYPE from the Registry
USHORT		g_SourceType[ROW_COUNT];
// @gmember count of clsid that are both enum & dso
ULONG		g_cDSOANDENUM	= 0;
// @gmember An array hold SOURCES_NAME from the Registry
WCHAR		g_rgwszSourceName[ROW_COUNT][REG_BUFFER];
// @gmember An array hold SOURCES_PARSENAME from the Registry
WCHAR		g_rgwszSourceParseName[ROW_COUNT][REG_BUFFER];
// @gmember An array hold SOURCES_DESCRIPTION from the Registry
WCHAR		g_rgwszSourceDescription[ROW_COUNT][REG_BUFFER];
// @gmember Counter of the OLE DB Enumerator and Provider entries in the Registry
ULONG		g_cTotalEntries = 0;
// @gmember Counter of the Providers
ULONG		g_cOLEDBProvNames = 0;
// @gmember Counter of the Providers
ULONG		g_cOLEDBProvMDPNames = 0;
// @gmember Counter of the Enumerators
ULONG		g_cOLEDBEnumNames = 0;
// @gmember Provider name
WCHAR		g_rgwszProvName[MAX_NUM_PROV][REG_BUFFER];
// @gmember Provider parse name					
WCHAR		g_rgwszParseName[MAX_NUM_PROV][REG_BUFFER];

///////////////////////////////////////////////////////////////////////
// Verifies if there are duplicates in the array of strings			 //
// returns true if no duplicates found, false otherwise				 //
///////////////////////////////////////////////////////////////////////
bool VerifyNoDuplicates(WCHAR** rgNames, ULONG cNames);

///////////////////////////////////////////////////////////////////////
// Read OLE DB Enumerator and Data sources entries from the Registry,//
// so we can using this infor to check the rowset returned from the  //
// root enumerator source rowset									 //
///////////////////////////////////////////////////////////////////////
BOOL GetEntriesFromRegistry()
{
	HKEY		hKey1, hKey2, hKey3;
	TCHAR		tszSrcName[REG_BUFFER];
	TCHAR		tszDesc[REG_BUFFER];
	TCHAR		tszQuery[REG_BUFFER];
	TCHAR		tszNameCLSID[REG_BUFFER];
	TCHAR		tszClsid[REG_BUFFER];
	TCHAR		tszProvOrEnum[REG_BUFFER];
	TCHAR		tszProvName[REG_BUFFER];
	TCHAR		tszProvNameMDP[REG_BUFFER];
	TCHAR		tszEnumName[REG_BUFFER];
	DWORD		initSize = REG_BUFFER * sizeof(TCHAR);
	DWORD		cbName = initSize;
	DWORD		cbValue = initSize;
	ULONG		index1 = 0;
	ULONG		index2;
	ULONG		arrayIndex = 0;
	BOOL		Result = TRUE;
	BOOL		fDSOANDENUM	=	FALSE;	//flag to used to check if a clsid has already been read as a datasource or as a enumerator

	// We are searching for the entries of OLE DB providers and enumerators
	_tcscpy(tszNameCLSID, _T("CLSID"));
	_tcscpy(tszProvName, _T("OLE DB Provider"));
	_tcscpy(tszProvNameMDP, _T("OLE DB MD Provider"));
	_tcscpy(tszEnumName, _T("OLE DB Enumerator"));
	// In the Registry it is "No name", so we set an empty string 
	_tcscpy(tszQuery, _T(""));

	// need to set g_cDSOANDENUM to 0
	// if the test is run several times under ltm
	// the global variables are not reinitialized
	g_cDSOANDENUM = 0;
	// Open CLSID subkey first
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, tszNameCLSID, 0, 
							KEY_READ, &hKey1) == ERROR_SUCCESS)
	{
   		// Enumerate CLSID subkey
		while(RegEnumKeyEx(hKey1, index1++, tszClsid, 
							&cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			// Have to reset the buffer size
			cbName=initSize;

			//reset flag
			fDSOANDENUM = FALSE;

			// Open clsid subkey under CLSID subkey
			if (RegOpenKeyEx(hKey1, tszClsid, 0, KEY_READ, &hKey2) == ERROR_SUCCESS)
			{
				// The clsid of this key would be the value of SOURCES_PARSENAME in sources rowet
				// The value of this key would be the value of SOURCES_NAME in sources rowet
				// so I will query this value first.
				// If this entry is not what we want, the value will be overwritten later.
				// If this entry is what we want, the value will be recorded right away.
				RegQueryValueEx(hKey2,tszQuery,NULL,NULL,(LPBYTE)tszSrcName,&cbValue);
									
				// Reset the buffer size
				cbValue = initSize;
				index2=0;
		
				// Enumerate subsubkey under clsid subkey
				while(RegEnumKeyEx(hKey2, index2++, tszProvOrEnum,
							&cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
				{
					// Reset buffer size
					cbName = initSize;

					// If we find an OLE DB provider or enumerator, record it.
					if	(	(!_tcscmp(tszProvOrEnum, tszProvNameMDP))	||
							(!_tcscmp(tszProvOrEnum, tszProvName))	||
							(!_tcscmp(tszProvOrEnum, tszEnumName))
						)
					{
						//if this flag is true then this clsid has already been marked as either a dos or as a enum
						//count it
						if (fDSOANDENUM)
						{
							g_cDSOANDENUM++;
						}
						fDSOANDENUM = TRUE;

						if (!_tcscmp(tszProvOrEnum, tszProvName))
						{
							g_SourceType[arrayIndex] = DBSOURCETYPE_DATASOURCE;
							g_cOLEDBProvNames++;
						}
						if (!_tcscmp(tszProvOrEnum, tszProvNameMDP))
						{
							g_SourceType[arrayIndex] = DBSOURCETYPE_DATASOURCE_MDP;
							g_cOLEDBProvMDPNames++;
						}
						if (!_tcscmp(tszProvOrEnum, tszEnumName))
						{
							g_SourceType[arrayIndex] = DBSOURCETYPE_ENUMERATOR;
							g_cOLEDBEnumNames++;
						}
						
						// Count it
						g_cTotalEntries++;
						
						// Open it and get the value, 
						if (RegOpenKeyEx(hKey2, tszProvOrEnum, 0, KEY_READ, &hKey3) != ERROR_SUCCESS)
							Result = FALSE;
						
						// The value of this key would be the value of SOURCES_DESCRIPTION in sources rowet
						if(RegQueryValueEx(hKey3,tszQuery,NULL,NULL,(LPBYTE)tszDesc,&cbValue) != ERROR_SUCCESS)
							Result = FALSE;

						// If we query the values properly
						if (Result)
						{
							// Record all these values for checking the row info later 
							#ifdef _UNICODE
								wcscpy(g_rgwszSourceName[arrayIndex],tszSrcName);
								wcscpy(g_rgwszSourceParseName[arrayIndex],tszClsid);
								wcscpy(g_rgwszSourceDescription[arrayIndex],tszDesc);
							#else
								MultiByteToWideChar(CP_ACP,0,tszSrcName,-1,g_rgwszSourceName[arrayIndex],REG_BUFFER);
								MultiByteToWideChar(CP_ACP,0,tszClsid,-1,g_rgwszSourceParseName[arrayIndex],REG_BUFFER);
								MultiByteToWideChar(CP_ACP,0,tszDesc,-1,g_rgwszSourceDescription[arrayIndex],REG_BUFFER);
							#endif //_UNICODE
						}
						else
							break;

						// Increment index to record the column data of the next row
						arrayIndex++;

						// Reset buffersize and close registry key
						cbValue = initSize;
						RegCloseKey(hKey3);
					}
				}

				// Close registry key
				RegCloseKey(hKey2);
			}
			else
				Result = FALSE;
		}

		// Close registry key
		RegCloseKey(hKey1);
	}
	else
		return TRUE;

	return Result;
}


///////////////////////////////////////////////////////////////
// Mark properties surpported					             //
///////////////////////////////////////////////////////////////
BOOL MarkSupportedProperties(const GUID clsid = CLSID_OLEDB_ENUMERATOR)
{
	TBEGIN
	HRESULT			hr				= E_FAIL;
	ULONG			ulIndex			= 0;
	ISourcesRowset  *pISrcRow		= NULL;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	// Setup the input param DBPROPIDSET
	ULONG			cPropSets		= 0;
	DBPROPSET		*rgPropSets		= NULL;
	DBPROPIDSET		rgPropIDSets;
	
	// Right now I focus on VT_BOOL type properties
	DBPROPID dbPropid[PROPERTY_COUNT] = {
		DBPROP_DEFERRED, 
		DBPROP_BOOKMARKS, 
		DBPROP_CACHEDEFERRED, 
		DBPROP_CANFETCHBACKWARDS, 
		DBPROP_CANHOLDROWS, 
		DBPROP_CANSCROLLBACKWARDS,
		DBPROP_CHANGEINSERTEDROWS, 
		DBPROP_COLUMNRESTRICT, 
		DBPROP_IRowsetChange, 
		DBPROP_IRowsetIdentity, 
		DBPROP_IRowsetLocate, 
		DBPROP_IRowsetScroll, 
		DBPROP_IRowsetUpdate, 
		DBPROP_LITERALIDENTITY, 
		DBPROP_OTHERINSERT, 
		DBPROP_QUICKRESTART, 
		DBPROP_ROWRESTRICT, 
		DBPROP_TRANSACTEDOBJECT, 
		DBPROP_UPDATABILITY,
		DBPROP_APPENDONLY, 
	};
		
	// Create an enumerator object
	TESTC_(hr=CoCreateInstance(clsid, NULL, 
		GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&pISrcRow), S_OK);

	// Open sources rowset
	TESTC_(hr=pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo, 
											0, NULL, (IUnknown**)&pIRowsetInfo), S_OK);

	TESTC(NULL != pIRowsetInfo);

	// Copy the Properties to the Global
	for(ulIndex=0; ulIndex < PROPERTY_COUNT; ulIndex++)
	{
		g_PropMark[ulIndex].dwPropertyID = dbPropid[ulIndex];
		
		// find out whether the prop is supported or not
		rgPropIDSets.rgPropertyIDs		= &dbPropid[ulIndex];
		rgPropIDSets.guidPropertySet	= DBPROPSET_ROWSET;
		rgPropIDSets.cPropertyIDs		= 1;
		
		// If the Property is VARIANT_TRUE
		hr = pIRowsetInfo->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
		if (COMPARE(NULL != rgPropSets, TRUE) && COMPARE(cPropSets, 1))
		{
			if (DBPROPSTATUS_OK == rgPropSets[0].rgProperties[0].dwStatus && CHECK(hr, S_OK))
				g_PropMark[ulIndex].status = SUPPORTED;
			else
			{
				COMPARE(rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				CHECK(hr, DB_E_ERRORSOCCURRED);
				g_PropMark[ulIndex].status = NOTSUPPORTED;
			}
		}
		FreeProperties(&cPropSets,&rgPropSets);
	}

CLEANUP:

	// Release the objects
	SAFE_RELEASE(pISrcRow);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
}

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * m_pThisTestModule)
{
	g_cOLEDBProvNames		= 0;
	g_cOLEDBProvMDPNames	= 0;
	g_cOLEDBEnumNames		= 0;
	g_cTotalEntries			= 0;

	//Must either call CreateModInfo or CreateModuleDBSession before any testing
	if(!CreateModInfo(m_pThisTestModule))
		return FALSE;
	
	g_PropMark = (DBPropMark*)PROVIDER_ALLOC(PROPERTY_COUNT * sizeof(DBPropMark));
	if (!g_PropMark)
	{
		odtLog << wszMemoryAllocationError;
		return FALSE;
	}
	
	// Initialize the buffer to 0's
	memset(g_PropMark, 0, PROPERTY_COUNT * sizeof(DBPropMark));

	// Read OLE DB enumerator and provider entries from the Registry
	// Mark all supported properties from the Rowset property group
	if (GetEntriesFromRegistry())
		return MarkSupportedProperties();
	else
		return FALSE;
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * m_pThisTestModule)
{
	// Release memory and OLE memory allocator
	PROVIDER_FREE(g_PropMark);

	//Init
	g_cOLEDBProvNames		= 0;
	g_cOLEDBProvMDPNames	= 0;
	g_cOLEDBEnumNames		= 0;
	g_cTotalEntries			= 0;
	return ReleaseModInfo(m_pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CSourcesRowset : public COLEDB {
public:
	CSourcesRowset(WCHAR* pwszTestCaseName = NULL);
	~CSourcesRowset(){};

protected:
	// @cmember enumerator class id
	CLSID				m_clsid;
	// @cmember pointer to ISourcesRowset
	ISourcesRowset *	m_pISrcRow;
	// @cmember pointer to IColumnsInfo
	IColumnsInfo *		m_pIColumnsInfo;
	// @cmember pointer to IRowset
	IRowset	*			m_pIRowset;
	// @cmember pointer to IRowsetInfo
	IRowsetInfo *		m_pIRowsetInfo;
	// @cmember pointer to IAccessor
	IAccessor *			m_pIAccessor;
	// @cmember accessory handle
	HACCESSOR			m_hAccessor;
	// @cmember pointer to the row buffer
	void *				m_pData;
	// @cmember size of a row
	DBLENGTH			m_cRowSize;
	// @cmember	count of binding structure
	DBCOUNTITEM			m_cBinding;
	// @cmember array of binding strucuture
	DBBINDING *			m_rgBinding;
	// @cmember Array of property
	DBPROP				m_rgDBProp[MAXPROP];
	// @cmember Array of Property Sets
	DBPROPSET			m_rgDBPropSets[MAXPROP];
	// @cmember Count of Property Sets
	ULONG				m_cDBPropSets;
	// @cmember	Count of DBPROPINFOSETs
	ULONG				m_cDBPropInfoSets;
	// @cmember Count of column
	DBORDINAL			m_cColumns;
	// @cmember Count of column
	DBCOUNTITEM			m_cRows;
	// @cmember An array of DBCOLUMNINFO
	DBCOLUMNINFO *		m_rgInfo;
	// @cmember String buffer
	WCHAR *				m_pStringsBuffer;
	// @cmember Count of Bindings
	DBCOUNTITEM			m_cDBBINDING;			
	// @cmember Count of columns in rowset
	DBORDINAL			m_cDBCOLUMNINFO;		
	// @cmember Count of supported properties
	ULONG				m_cSupportedProp;		

	// @cmember array of return values for threads
	HRESULT				m_rgResult[nThreads];
	// @cmember array of number of rows in the rowset for each thread
	ULONG				m_rgRowsNo[nThreads];
	// @cmember array of rowset interfaces for each thread
	IRowset				*m_rgRowset[nThreads];

	WCHAR**				m_rgSourcesNames[nThreads];
	ULONG				m_rgMaxSourcesNames[nThreads];



	// methods
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember SupportedProperty
	//
	// This function should return TRUE if the fuction succeeded and 
	// the property is supported by the Provider.
	BOOL SupportedProperty(
		DBPROPID	PropertyID, 
		const GUID	guidPropertySet, 
		IUnknown*	pIUnknown,
		EINTERFACE	eCoType = DATASOURCE_INTERFACE
	);


	// @cmember SettableProperty
	//
	// This function should return TRUE if the fuction succeeded and 
	// the property is settable by the Provider.
	BOOL SettableProperty(
		DBPROPID	PropertyID, 
		GUID		guidPropertySet, 
		IUnknown*	pIUnknown, 
		EINTERFACE	eCoType = DATASOURCE_INTERFACE
	);

	// @cmember Set one property from rowset property group
	void SetOneProperty(	
							DBPROPID		dwPropertyID, 
							DBPROPOPTIONS	dwOptions, 
							ULONG			index, 
							VARENUM			eType = VT_BOOL
						);
	// @cmember Check column info of the sources rowset
	BOOL CheckColumnsInfo(ULONG ColCount, IUnknown *pRowset);
	// @cmember Check rowset info of the sources rowset
	BOOL CheckRowsInfo();
	// @cmember Read Parse Name from sourse Rowset of an enumerator
	BOOL ReadParseName(const CLSID clsid);
	// @cmember Compare data in each column in one row
	BOOL CompareData(ULONG *pcRowIndex);
	// @cmember Free accessor handle and binding structure memories
	BOOL FreeAccessorAndBindings();
	// @cmember Check whether the property DBPROP_CANHOLDROWS is supported
	BOOL NoHoldRows(IRowset *pIRowset);
	// @cmember Check whether the IParseDisplayName is valid
	BOOL VerifyParseDisplayName_root(IParseDisplayName *pIParseDisplayName);
	// @cmember Check whether the IParseDisplayName is valid
	BOOL VerifyParseDisplayName_std(IParseDisplayName *pIParseDisplayName);
	// @cmember Tries calling GetNextRows without releasing all row handles
	ULONG GetNextRowsWithoutRelAllRows();
	// @cmember Verify how an invalid colid is treated
	ULONG VerifyInvalidColID(DBPROPOPTIONS);
	// @cmember Verify how an non rowset property set is treated
	ULONG VerifyNonRowsetPropSet(ULONG);

	// @cmember Verify aggregation on sources rowset, ask for something different from IID_IUnknown => DB_E_NOAGGREGATION
	BOOL VerifyNotIUnknownInAggregation();
	// @cmember Verify sources rowset aggregation
	BOOL VerifySourcesRowsetAggregation();
	// @cmember Verify IRowsetInfo::GetSpecification on sources rowset, aggregated enumerator
	BOOL VerifyGetSpec_AggregatedEnum();

	// @cmember Count the number of rows in the rowset (assume init position)
	HRESULT				CountNoOfRows(IRowset *pIRowset, ULONG *pulRowsNo);
	HRESULT				CountNoOfRowsAndGetSourcesNames(IRowset *pIRowset, ULONG *pulRowsNo, WCHAR** rgwszSourcesNames, ULONG cMaxSourcesNames);


	// @cmember Execute the thread method
	unsigned virtual	MyThreadProc(ULONG iThread);
	unsigned virtual	MyThreadProc2(ULONG iThread);

	friend unsigned WINAPI ThreadProc(LPVOID lpvThreadParam);
	friend unsigned WINAPI ThreadProc2(LPVOID lpvThreadParam);
};


///////////////////////////////////////////////////////////////
// Constructor							                 //
///////////////////////////////////////////////////////////////
CSourcesRowset::CSourcesRowset(WCHAR* pwszTestCaseName) : COLEDB(pwszTestCaseName) 
{
	m_pISrcRow		= NULL;
	m_pIColumnsInfo = NULL;
	m_pIRowset		= NULL;
	m_pIRowsetInfo	= NULL;
	m_pIAccessor	= NULL;
	m_cColumns		= 0;
	m_cRows			= 0;
	m_cRowSize		= 0;
	m_cDBPropSets	= 0;
	m_cDBPropInfoSets=0;
	m_cDBBINDING	= 0;			
	m_cDBCOLUMNINFO = 0;		
	m_cSupportedProp= 0;		
	m_cBinding		= 0;
	m_rgBinding		= NULL;
	m_pData			= NULL;
	m_rgInfo		= NULL;
	m_pStringsBuffer= NULL;
	m_hAccessor		= DB_NULL_HACCESSOR;
	m_clsid			= CLSID_OLEDB_ENUMERATOR;
}


///////////////////////////////////////////////////////////////
// Initialization							                 //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::Init()
{
	return COLEDB::Init();
}

///////////////////////////////////////////////////////////////
// Termination							                 //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::Terminate()
{
	return COLEDB::Terminate();
}	



//--------------------------------------------------------------------
// @func BOOL | SupportedProperty
//
// This function should return TRUE if the fuction succeeded and 
// the property is supported by the Provider.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
BOOL CSourcesRowset::SupportedProperty(
	DBPROPID	PropertyID, 
	const GUID	guidPropertySet, 
	IUnknown*	pIUnknown,
	EINTERFACE	eCoType
)
{
	IDBProperties	*pIDBProperties	= NULL;
	ULONG			cProp;
	DBPROPIDSET		rgPropIDSets;
	HRESULT			hr;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	DBPROPSET		*rgPropSets		= NULL;
	ULONG			cPropSets		= 0;
	BOOL			fResult			= FALSE;

	// if a IDBProperties interface can be retrieved, call 
	if (VerifyInterface(pIUnknown, IID_IDBProperties, eCoType, (IUnknown**)&pIDBProperties))
	{
		SAFE_RELEASE(pIDBProperties);
		return ::SupportedProperty(PropertyID, guidPropertySet, pIUnknown, eCoType);
	}

	// if the prop is an already checked one, just return the result
	for (cProp = 0; cProp < PROPERTY_COUNT; cProp++)
	{
		if (g_PropMark[cProp].dwPropertyID == PropertyID)
			return (DBPROPSET_ROWSET == guidPropertySet) && (SUPPORTED == g_PropMark[cProp].status);
	}

	// make sure the ISourcesRowsetInterface is there
	if (!m_pISrcRow)
		return ::SupportedProperty(PropertyID, guidPropertySet, pIUnknown, eCoType);

	// find out whether the prop is supported or not
	rgPropIDSets.rgPropertyIDs		= &PropertyID;
	rgPropIDSets.guidPropertySet	= guidPropertySet;
	rgPropIDSets.cPropertyIDs		= 1;

	// Open sources rowset
	hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo);
	TESTC_(hr, S_OK);
	TESTC(NULL != pIRowsetInfo);

	// If the Property is VARIANT_TRUE
	hr = pIRowsetInfo->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
	fResult =		COMPARE(NULL != rgPropSets, TRUE) && COMPARE(cPropSets, 1) 
				&&	(DBPROPSTATUS_OK == rgPropSets[0].rgProperties[0].dwStatus);

CLEANUP:
	FreeProperties(&cPropSets,&rgPropSets);
	SAFE_RELEASE(pIRowsetInfo);
	return fResult;
}



//--------------------------------------------------------------------
// @func BOOL | SettableProperty
//
// This function should return TRUE if the fuction succeeded and 
// the property is settable by the Provider.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
BOOL CSourcesRowset::SettableProperty(
	DBPROPID	PropertyID, 
	GUID		guidPropertySet, 
	IUnknown*	pIUnknown, 
	EINTERFACE	eCoType
)
{
	IDBProperties	*pIDBProperties	= NULL;
	HRESULT			hr;
	DBPROPSET		rgPropSets;
	DBPROP			rgProp;
	BOOL			fResult			= FALSE;
	IRowsetInfo		*pIRowsetInfo	= NULL;
	VARIANT			vValue;

	if (!SupportedProperty(PropertyID, guidPropertySet, pIUnknown, eCoType))
		return FALSE;

	// if a IDBProperties interface can be retrieved, call 
	if (VerifyInterface(pIUnknown, IID_IDBProperties, eCoType, (IUnknown**)&pIDBProperties))
	{
		fResult = ::SettableProperty(PropertyID, guidPropertySet, pIUnknown, eCoType);
		goto CLEANUP;
	}

	// make sure the ISourcesRowsetInterface is there
	if (!m_pISrcRow)
		return ::SupportedProperty(PropertyID, guidPropertySet, pIUnknown, eCoType);

	// Open sources rowset
	TESTC_(hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo, 
		0, NULL, (IUnknown**)&pIRowsetInfo), S_OK);
	VariantInit(&vValue);
	TESTC(GetProperty(PropertyID, guidPropertySet, pIRowsetInfo, &vValue));
	SAFE_RELEASE(pIRowsetInfo);

	// find out whether the prop is supported or not
	rgPropSets.rgProperties		= &rgProp;
	rgPropSets.cProperties		= 1;
	rgPropSets.guidPropertySet	= guidPropertySet;

	rgProp.dwPropertyID			= PropertyID;
	rgProp.dwOptions			= DBPROPOPTIONS_REQUIRED;
	rgProp.colid				= DB_NULLID;
	rgProp.vValue.vt			= vValue.vt;
	if (VT_BOOL == vValue.vt)
		rgProp.vValue.boolVal	= VARIANT_TRUE == vValue.boolVal? VARIANT_FALSE: VARIANT_TRUE;	// anything else than what seems to be the default value
	else
		rgProp.vValue.intVal		= vValue.intVal + 11;	// anything else than what seems to be the default value

	TESTC(VT_I4 == vValue.vt || VT_BOOL == vValue.vt); 

	// Open sources rowset
	hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo, 1, &rgPropSets, (IUnknown**)&pIRowsetInfo);

	fResult =		(DBPROPSTATUS_NOTSUPPORTED		!= rgPropSets.rgProperties[0].dwStatus) 
				&&	(DBPROPSTATUS_NOTSETTABLE		!= rgPropSets.rgProperties[0].dwStatus)
				&&	(DBPROPSTATUS_NOTALLSETTABLE	!= rgPropSets.rgProperties[0].dwStatus)
				&&	(DBPROPSTATUS_NOTSET			!= rgPropSets.rgProperties[0].dwStatus)
				&&	(DBPROPSTATUS_BADVALUE			!= rgPropSets.rgProperties[0].dwStatus);

	if (S_OK != hr)
		TESTC_(hr, DB_E_ERRORSOCCURRED);

	
CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIRowsetInfo);
	return fResult;
}



///////////////////////////////////////////////////////////////
// set one property from rowset property group.  Right now we//
// only set property with BOOL value						 //
///////////////////////////////////////////////////////////////
void CSourcesRowset::SetOneProperty(	
							DBPROPID		dwPropertyID, 
							DBPROPOPTIONS	dwOptions, 
							ULONG			index, 
							VARENUM			eType
						)
{
	ASSERT(index<MAXPROP);
	m_rgDBProp[index].dwPropertyID = dwPropertyID;
	m_rgDBProp[index].dwOptions = dwOptions;
	m_rgDBProp[index].colid = DB_NULLID;
	m_rgDBProp[index].vValue.vt = eType;
	switch (eType)
	{
		case VT_I4:
			V_I4(&(m_rgDBProp[index].vValue)) = rand();
			break;
		default:
			V_BOOL(&(m_rgDBProp[index].vValue)) = VARIANT_FALSE;
	}

	m_cDBPropSets = 1;
	m_rgDBPropSets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPropSets[0].cProperties = index+1;
	m_rgDBPropSets[0].rgProperties = m_rgDBProp;	
}

///////////////////////////////////////////////////////////////
// Check column information of the sources rowset            //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::CheckColumnsInfo(ULONG ColCount, IUnknown *pRowset)
{
	BOOL	fResult = FALSE;
	ULONG	cCol;
	ULONG	cOrdinal;

	// Make sure we have valid pointer
	TESTC(NULL != pRowset);
	
	// Get a IColumnsInfo pointer
	TESTC(VerifyInterface(pRowset, IID_IColumnsInfo, 
		ROWSET_INTERFACE,(IUnknown **)&m_pIColumnsInfo));
	
	TESTC_(m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer), S_OK);
	
	// Check to see if the Bookmark Property is on
	if (!m_rgInfo[0].iOrdinal)
	{
		TESTC(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, pRowset));
		TESTC(m_cColumns == ColCount);
	}
	else
		TESTC(m_cColumns == ColCount-1);

	cCol		= (0 == m_rgInfo[0].iOrdinal) ? 1 : 0;
	cOrdinal	= 1;

	// if there is a bookmark column then Skip the bookmark column
	for(; cCol < m_cColumns; cCol++, cOrdinal++)
	{
		if (!memcmp(m_rgInfo[cCol].pwszName, 
					g_rgwszColumnName[cOrdinal-1], sizeof(g_rgwszColumnName[cOrdinal-1])))
		{
			TESTC(m_rgInfo[cCol].iOrdinal == cOrdinal);
			TESTC(m_rgInfo[cCol].wType == g_ColumnType[cOrdinal-1]);
		}
	}
	fResult = TRUE;

CLEANUP:
	SAFE_FREE(m_pStringsBuffer);
	SAFE_FREE(m_rgInfo);
	SAFE_RELEASE(m_pIColumnsInfo);
	return fResult;
}


///////////////////////////////////////////////////////////////
// Check row information of the sources rowset               //
// 1.	Get accessor and bindings.							 //
// 2.	Compare the data in each column row by row to make   //
//		sure that the entries we are interested in are in the//
//		rowset returned.									 // 
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::CheckRowsInfo()
{
	BOOL		fResult			= FALSE;
	HROW		hRow[1]			= {NULL};
	HROW		*pHRow			= hRow;
	DBCOUNTITEM	cRow			= 0;
	ULONG		cRowIndex		= 0;
	BOOL		*frgPresentSrc	= NULL;
	ULONG		cFoundRowIndex;
	ULONG		cRemainingRows	= g_cTotalEntries;
	
	// Make sure we have valid pointer
	if (!m_pIRowset)
		return FALSE;	

	// Create an accessor on the sources rowset and get bindings
	if (!CHECK(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND,FORWARD,NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,NO_BLOB_COLS),S_OK))
			return FALSE;

	// Allocate memory for the row
	SAFE_ALLOC(m_pData, BYTE, m_cRowSize);
	SAFE_ALLOC(frgPresentSrc, BOOL, g_cTotalEntries);
	
	for (cRowIndex=0;cRowIndex<g_cTotalEntries;cRowIndex++)
	{
		frgPresentSrc[cRowIndex] = FALSE;
	}
	
	m_hr = m_pIRowset->RestartPosition(0);

	// Loop through the rowset, retrieve one row at a time
	for(cRowIndex=0;cRowIndex<g_cTotalEntries;cRowIndex++)
	{
		// Get the next row 
		if (!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow),S_OK))
			break;

		if (CHECK(m_pIRowset->GetData(hRow[0],m_hAccessor,m_pData), S_OK))
		{
			// Compare the rowset data with the data retrieved from Registry
			fResult = CompareData(&cFoundRowIndex); // if fResult is FALSE break the loop
			if (!COMPARE(cFoundRowIndex == g_cTotalEntries, FALSE))
				fResult = FALSE;
			else
			{
				// check that the source wasn't found before
				if (!frgPresentSrc[cFoundRowIndex])
				{
					// decrease the number of sources not yet found in the rowset
					cRemainingRows--;
					frgPresentSrc[cFoundRowIndex] = TRUE;
				}
				else
					odtLog << "WARNING, " << g_rgwszParseName[cFoundRowIndex]
						<< "already found with the same type!\n";
			}
		}

		// Release the row handle
		CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);
		// if comparison was ok continue the loop
		TESTC(fResult);
	}

 	// The cursor should be at the end of the rowset
	if (fResult)
		CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow),DB_S_ENDOFROWSET);
	
CLEANUP:
	// Release the rowset created
	if (!FreeAccessorAndBindings())
		fResult = FALSE;
	SAFE_FREE(frgPresentSrc);
	return fResult && (0 == cRemainingRows);
}


///////////////////////////////////////////////////////////////
// Compare data in each column of one row			         //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::CompareData(ULONG *pcRowIndex)
{
	BOOL			fErrorOccured		= FALSE;
	BOOL			fCheckForTruncation	= FALSE;
	BOOL			fCheckForNULL		= FALSE;
	BOOL			fCheckForLength		= FALSE;
	BOOL			fColumnError		= FALSE;
	void			*pConsumerData		= NULL;
	ULONG			cCount				= 0;
	USHORT			uswSize				= 0;
	LONG			lDBTypeSize			= 0;
	ULONG			cRowIndex			= g_cTotalEntries;

	const ULONG		nSrcName			= 1;
	const ULONG		nSrcParseName		= 2;
	const ULONG		nSrcDescription		= 3;
	const ULONG		nSrcType			= 4;
	const ULONG		nSrcIsParent		= 5;
	const ULONG		nSrcCLSID			= 6;
	
	ULONG			ulRowIndex;
	WCHAR			*wszParseName		= NULL;
	unsigned short	ulSrcType;
	ULONG			cBndgParseName		= m_rgBinding[0].iOrdinal? 1: 2;
	ULONG			cBndgType			= m_rgBinding[0].iOrdinal? 3: 4;


	// Input validation
	if ((!m_rgBinding) || (!m_pData) || (!pcRowIndex))
		return FALSE;
	*pcRowIndex = g_cTotalEntries;

	// identify the datasource/enumerator to be compared against the current row in the sources rowset
	// get the parse name
//	switch (*((DBSTATUS *)(dwAddrData + m_rgBinding[cBndgParseName].obStatus)))
    switch	(STATUS_BINDING(m_rgBinding[cBndgParseName],m_pData))
	{
		case DBSTATUS_S_ISNULL:
			break;
		case DBSTATUS_S_OK:
			wszParseName = (WCHAR*)&VALUE_BINDING(m_rgBinding[cBndgParseName],m_pData);
			break;
		default:
			return FALSE;
	}

	// get source type
	if (!COMPARE(STATUS_BINDING(m_rgBinding[cBndgType],m_pData), DBSTATUS_S_OK))
		return FALSE;
	ulSrcType = *(unsigned short*)(&VALUE_BINDING(m_rgBinding[cBndgType],m_pData));

	// identify the source
	for (ulRowIndex = 0; ulRowIndex < g_cTotalEntries; ulRowIndex++)
	{
		if (	(NULL == wszParseName && NULL != g_rgwszSourceParseName[ulRowIndex])
			||	(NULL != wszParseName && NULL == g_rgwszSourceParseName[ulRowIndex])
			||  (0 != wcscmp(wszParseName, g_rgwszSourceParseName[ulRowIndex])))
				continue;
		// check if the type is the same
		if (g_SourceType[ulRowIndex] != ulSrcType)
			continue;
		// the target source was found
		cRowIndex = ulRowIndex;
		break;
	}
	if (ulRowIndex >= g_cTotalEntries)
		return FALSE;

	// Compare the data column by column
	// Status, Length, and Valid binding are checked
	for(cCount=0; cCount<m_cBinding; cCount++)
	{
		// If previous bindings have failed exit now
		if (fErrorOccured)
			break;

		// Init
		fCheckForTruncation	= FALSE;
		fCheckForNULL		= FALSE;
		fCheckForLength		= FALSE;
		fColumnError		= FALSE;
		
		//////////////////////////////
		// check the status binding //
		//////////////////////////////
		if ((m_rgBinding[cCount].dwPart) & DBPART_STATUS)
		{
//			switch (*((DBSTATUS *)(dwAddrData + m_rgBinding[cCount].obStatus)))
            switch(STATUS_BINDING(m_rgBinding[cCount],m_pData))
			{
				// Return FALSE if any error flags is set.  
				case DBSTATUS_E_SIGNMISMATCH:
				case DBSTATUS_E_CANTCONVERTVALUE:
				case DBSTATUS_E_CANTCREATE:
				case DBSTATUS_E_UNAVAILABLE:
				case DBSTATUS_E_DATAOVERFLOW:
				case DBSTATUS_E_BADACCESSOR:
				case DBSTATUS_E_INTEGRITYVIOLATION:
				case DBSTATUS_E_SCHEMAVIOLATION:
					fErrorOccured = TRUE;
					continue;			
	
				// If the data is truncated, 
				// we need to check the length binding if appropriate
				case DBSTATUS_S_TRUNCATED:
						fCheckForTruncation = TRUE;
					break;
				
				// If the data is NULL, 
				// we need to check the length binding if appropriate
				case DBSTATUS_S_ISNULL:
						fCheckForNULL = TRUE;
					break;
				
				case DBSTATUS_S_OK:
					break;
				
				default:
					fErrorOccured = TRUE;
			}
		}	// end if status bindings

		//////////////////////////////////
		// check for the length binding //
		//////////////////////////////////
		if ((m_rgBinding[cCount].dwPart) & DBPART_LENGTH)
		{
			// If cbMaxLen > length, 
			// no truncation should occure
			if (fCheckForTruncation)
			{
				if ((LENGTH_BINDING(m_rgBinding[cCount],m_pData)) < m_rgBinding[cCount].cbMaxLen)
					fErrorOccured = TRUE;
				continue;			
			}

			// If status is NULL, skip length and value checks
			if (fCheckForNULL)
				continue;
					
			// Length should be the same as the sizeof(DBTYPE) for fixed length data type
			// Length should be number of bytes of the data for variable length data type,
			switch(m_rgBinding[cCount].wType)
			{
				// No way to check variable lendth data type without a value binding
				case DBTYPE_BYTES:
				case DBTYPE_STR:
				case DBTYPE_WSTR:
						fCheckForLength = TRUE;
					break;
				default:
						// For fixed length data type, make sure the length value is 
						// the same as the size of the data type
						// get the size of the dta type
						lDBTypeSize=GetDBTypeSize(m_rgBinding[cCount].wType);
						
						if ((lDBTypeSize == 0) || (lDBTypeSize == INVALID_DBTYPE_SIZE))
						{
							fColumnError = TRUE;
							break;
						}

						// Compare the data length with length binding
//						if (*((ULONG *)(dwAddrData + m_rgBinding[cCount].obLength)) !=
						if ((LENGTH_BINDING(m_rgBinding[cCount],m_pData)) !=
							(ULONG)lDBTypeSize)
						{
							fColumnError = TRUE;
							break;
						}

					break;
			}

			// Free the memory allocated by the provider, goto next binding structure
			if (fColumnError)
			{
				fErrorOccured=TRUE;
				continue;
			}
		}	// end if negth bindings

		/////////////////////////////
		// check for value binding //
		/////////////////////////////
		if ((m_rgBinding[cCount].dwPart) & DBPART_VALUE )
		{	
			// Skip checking the value binding for BOOKMARKS
			if (!m_rgBinding[cCount].iOrdinal)
				continue;

			// Get the data in the consumer's buffer
			pConsumerData=(void *)(&VALUE_BINDING(m_rgBinding[cCount],m_pData));

			// Compare with data selected from the Registry
			// I can only verify four columnn now: sources name, sources parse name and
			// description and type.  
			// For root enumerator, the sources clsid is the same as parse name.
			
			// Right now, the column number is cCount, the row number is cRowIndex
			switch(m_rgBinding[cCount].iOrdinal)
			{
				case nSrcName: 
					if (wcscmp(g_rgwszSourceName[cRowIndex], (WCHAR *)pConsumerData))
						fErrorOccured = TRUE;
					break;
				case nSrcParseName:
					if (wcscmp(g_rgwszSourceParseName[cRowIndex], (WCHAR *)pConsumerData))
						fErrorOccured = TRUE;
					break;
				case nSrcDescription:
					if (wcscmp(g_rgwszSourceDescription[cRowIndex], (WCHAR *)pConsumerData))
						fErrorOccured = TRUE;
					break;
				case nSrcType:
					// this should be done more agressive after specs merge in 2.1
					switch (*(USHORT *)pConsumerData)
					{
					case DBSOURCETYPE_ENUMERATOR:
						if (*(USHORT *)pConsumerData != g_SourceType[cRowIndex])
							fErrorOccured = TRUE;					
						break;
					case DBSOURCETYPE_DATASOURCE:
					case DBSOURCETYPE_DATASOURCE_MDP:
						// the actual g_SourceType[cRowIndex] value could be either 
						// DBSOURCETYPE_DATASOURCE or DBSOURCETYPE_DATASOURCE_MDP
						if (DBSOURCETYPE_ENUMERATOR == g_SourceType[cRowIndex])
							fErrorOccured = TRUE;					
						break;
					default:
						fErrorOccured = TRUE;					
					}
					break;
				case nSrcCLSID:
					if (wcscmp(g_rgwszSourceParseName[cRowIndex], (WCHAR *)pConsumerData))
						fErrorOccured=TRUE;
					break;
				case nSrcIsParent:
					break;
				default:
					ASSERT(FALSE);
					break;
			}

			// Make sure for variable length data, the length in m_rgBinding[cCount] contains
			// correct information
			if (fCheckForLength)	
			{	
				switch(m_rgBinding[cCount].wType)
				{	
					case DBTYPE_WSTR:	
						if ((LENGTH_BINDING(m_rgBinding[cCount],m_pData)) 	
						!= (wcslen((WCHAR *)pConsumerData)*sizeof(WCHAR)) )	
							fColumnError = TRUE;	
						break;	
					case DBTYPE_BYTES:	
						if ((LENGTH_BINDING(m_rgBinding[cCount],m_pData)) 	
							!= uswSize)	
							fColumnError = TRUE;	
						break;
					default:	
						break;	
				}	

				// Output an error message if the length binding fails
				if (fColumnError)	
					fErrorOccured=TRUE;	
			}	

		}	// end of value binding

	}//end of the main loop

	*pcRowIndex = cRowIndex;
	return !fErrorOccured;
}


///////////////////////////////////////////////////////////////
// Read parse name from the sources rowset of root enumerator//
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::ReadParseName(const CLSID clsid)
{
	BOOL			fResult		= FALSE;
	HRESULT			hr			= E_FAIL;
	HROW			hRow[1]		= {NULL};
	HROW			*pHRow		= hRow;
	DBCOUNTITEM		cRow		= 0;
	ULONG			ulIndex		= 0;
	ULONG			ulIndexMDP	= 0;
	ULONG			ulIndexE	= 0;
	const ULONG		nName		= 0;
	const ULONG		nParseName	= 1;
	const ULONG		nType		= 3;
	
	// Check pointer
	if (!m_pISrcRow)
	{
		return FALSE;
	}

	m_pIRowset = NULL;

	// Open sources rowset 
	if (!CHECK(hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,
											(IUnknown **)&m_pIRowset), S_OK))
		return FALSE;

	// Make sure we have valid pointer
	if (!m_pIRowset)
		return FALSE;

	// Create an accessor on the sources rowset and get bindings
	TESTC_(GetAccessorAndBindings(m_pIRowset,DBACCESSOR_ROWDATA,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_EXCEPTBOOKMARK,FORWARD,NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,NO_BLOB_COLS),S_OK);

	// Allocate memory for the row
	SAFE_ALLOC(m_pData, BYTE, m_cRowSize);

	// Loop over the rows, collecting providers and discarding 
	// enumerators.
	while(	(S_OK == (hr=m_pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow)))
		&&	(ulIndex < MAX_NUM_PROV))
	{
		// initialize fResult
		fResult = FALSE;

		// Get the next row 
		if (!CHECK(hr=m_pIRowset->GetData(hRow[0],m_hAccessor,m_pData), S_OK))
			goto LOOP;
		
		// Collect the data source row
		// Store Provider Name and Parse Name

		//bookmark columns are optional for the enumerator rowset.  check to see if this rowset
		//returned with a bookmark colum
		ASSERT(m_rgBinding[0].iOrdinal != 0);

		if (	DBSOURCETYPE_DATASOURCE == *((USHORT*)((BYTE *)m_pData + m_rgBinding[nType].obValue))
			||	DBSOURCETYPE_DATASOURCE_MDP == *((USHORT*)((BYTE *)m_pData + m_rgBinding[nType].obValue)))
		{
			wcscpy(g_rgwszProvName[ulIndex], (WCHAR*)((BYTE *)m_pData + m_rgBinding[nName].obValue));
			wcscpy(g_rgwszParseName[ulIndex], (WCHAR*)((BYTE *)m_pData + m_rgBinding[nParseName].obValue));
			ulIndex++;
		}

		//just a test to see if the source type if enumerator
		if (DBSOURCETYPE_ENUMERATOR == *((USHORT*)((BYTE *)m_pData + m_rgBinding[nType].obValue)))
		{
			ulIndexE++;
		}

		fResult = TRUE;

LOOP:
		// Release the row handle
		CHECK(hr=m_pIRowset->ReleaseRows(cRow,hRow,NULL,NULL,NULL),S_OK);

		TESTC(fResult);
	} // end while

	if (MAX_NUM_PROV <= ulIndex)
		TWARNING(L"There wasn't enough room to store all the rows provided by enumerator\n");

	// The cursor should be at the end of the rowset
	if (clsid == CLSID_OLEDB_ENUMERATOR)
		COMPARE((ulIndex+ulIndexE+ulIndexMDP), (g_cOLEDBProvNames+g_cOLEDBEnumNames+g_cOLEDBProvMDPNames));
	
CLEANUP:
	// Release the rowset created
	if (!FreeAccessorAndBindings())
		fResult = FALSE;
	
	SAFE_RELEASE(m_pIRowset);

	SAFE_FREE(m_pData);

	// If no provider, something is wrong
	if (!ulIndex)
		return FALSE;

	return fResult;
}


///////////////////////////////////////////////////////////////
// Free accessor handle and binding structure memories       //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::FreeAccessorAndBindings()
{
	BOOL Result = FALSE;

	//free binding structure
	FreeAccessorBindings(m_cBinding, m_rgBinding); 
	
	// Free the consumer buffer
	SAFE_FREE(m_pData);

	// Free accessor handle
	if (m_hAccessor)
	{
		// Get an IAccessor pointer if it was not yet a valid pointer 
		if (!m_pIAccessor)
			CHECK(m_pIRowset->QueryInterface(IID_IAccessor,
											(LPVOID *)&m_pIAccessor),S_OK);
		
		if (m_pIAccessor)
		{
			if (CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				Result = TRUE;
		}
		// Release the IAccessor interface pointer
		SAFE_RELEASE(m_pIAccessor);
		m_hAccessor = NULL;
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////
// Check whether the property DBPROP_CANHOLDROWS is supported			             //
///////////////////////////////////////////////////////////////
BOOL CSourcesRowset::NoHoldRows(IRowset *pIRowset)
{
	BOOL			fResult			= FALSE;			
	ULONG			cDBPropSets		= 0;
	DBPROPSET *		prgDBPropSets	= NULL;
	ULONG			cDBPropIDSets	= 0;
	DBPROPIDSET		DBPropIDSets;
	DBPROPID		rgPropertyIDs;
	IRowsetInfo *	pIRowsetInfo	= NULL;
	
	rgPropertyIDs = DBPROP_CANHOLDROWS;
	cDBPropIDSets = 1;
	DBPropIDSets.rgPropertyIDs = &rgPropertyIDs;
	DBPropIDSets.cPropertyIDs = 1;
	DBPropIDSets.guidPropertySet = DBPROPSET_ROWSET;

	TESTC(VerifyInterface(pIRowset, IID_IRowsetInfo, 
				ROWSET_INTERFACE,(IUnknown **)&pIRowsetInfo));

	// Verify that DBPROP_CANHOLDROWS is VARIANT_FALSE
	pIRowsetInfo->GetProperties(cDBPropIDSets, &DBPropIDSets, &cDBPropSets, &prgDBPropSets);
	TESTC(NULL != prgDBPropSets && 1 == cDBPropSets);

	if (	(DBPROPSTATUS_OK != prgDBPropSets->rgProperties[0].dwStatus) 
		||	(VARIANT_FALSE == V_BOOL(&prgDBPropSets->rgProperties[0].vValue)))
		fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	FreeProperties(&cDBPropSets, &prgDBPropSets);
	return fResult;
}

//////////////////////////////////////////////////////////////////
// Verify IParseDisplayName										//
// 1. Parse a data source names into a moniker					//
// 2. Binds that moniker with the interface on data source object/
// 3. Get the DSO initialized successfully						//
//////////////////////////////////////////////////////////////////
BOOL CSourcesRowset::VerifyParseDisplayName_root(IParseDisplayName	* pIParseDisplayName)
{
	BOOL			fResult			= FALSE;
	BOOL			fBindInitProvDSO= FALSE;
	ULONG			chEaten			= 0;
	ULONG			cPropSets		= 0;
	LPWSTR			wszClsid		= NULL;
	DBPROPSET *		rgPropSets		= NULL;
	IMoniker*		pIMoniker		= NULL;
	IDBInitialize * pIDBInit		= NULL;
	IDBProperties *	pIDBProperties	= NULL;
	ULONG i=0;

	if(!pIParseDisplayName)
		return FALSE;

	// Check each data source
	for(i=0; i<(g_cOLEDBProvNames+g_cOLEDBProvMDPNames); i++)
	{
		// Get a moniker
		odtLog << g_rgwszProvName[i];
		if (0 == wcscmp(L"MSDAIPP.DSO", g_rgwszProvName[i]))
		{
			odtLog << "\tSkipped\n";
			continue;
		}
		if (CHECK(m_hr=pIParseDisplayName->ParseDisplayName(NULL,
								g_rgwszParseName[i],&chEaten,&pIMoniker), S_OK))
		{
			odtLog << "\tgot the moniker";
			fResult = FALSE;
			if (pIMoniker)
			{
				// Binds the moniker with the IDBInitialize interface
				CHECK(BindMoniker(pIMoniker,0,
									IID_IDBInitialize,(LPVOID*)&pIDBInit), S_OK);

				odtLog << "\tbound the moniker";
				// We must have pIDBInit
				if (!pIDBInit)
					goto LOOP;

				// Get the Provider String from the CLSID
				StringFromCLSID(m_ProviderClsid, &wszClsid);
				
				// If it is not the current provider, skip the initialization
				if (_wcsicmp(g_rgwszParseName[i], wszClsid))
				{
					fResult = TRUE;
					goto LOOP;
				}
	
				odtLog << "\ttry to set props on this";

				// Build our init options from string passed to us from TMD for this provider
				if (!GetInitProps(&cPropSets,&rgPropSets))
					goto LOOP;

				// Get IDBProperties Pointer
				if (!VerifyInterface(pIDBInit, IID_IDBProperties, 
								ENUMERATOR_INTERFACE,(IUnknown **)&pIDBProperties))
					goto LOOP;

				// Set the properties before we Initialize
				if (!CHECK(pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK))
					goto LOOP;

				//Initialize
				if (CHECK(m_hr = pIDBInit->Initialize(), S_OK))
				{
					fResult = TRUE;
					fBindInitProvDSO = TRUE;
				}
				odtLog << "\tinitialized";
			}
		}
		else
						odtLog << "\tfailed to get the moniker";
	LOOP:
		odtLog << "\n";
		// Release the pointers
		SAFE_RELEASE(pIDBInit);
		SAFE_RELEASE(pIMoniker);
		SAFE_RELEASE(pIDBProperties);
		
		// Free Properties we got back	
		FreeProperties(&cPropSets,&rgPropSets);
		SAFE_FREE(wszClsid);
		TESTC(fResult);
	}
	fResult = TRUE;

CLEANUP:
	return fResult && fBindInitProvDSO;
}


//////////////////////////////////////////////////////////////////
// Verify IParseDisplayName										//
// 1. Parse a data source names into a moniker					//
// 2. Binds that moniker with the interface on data source object/
//////////////////////////////////////////////////////////////////
BOOL CSourcesRowset::VerifyParseDisplayName_std(IParseDisplayName	* pIParseDisplayName)
{
	BOOL			fResult			= FALSE;
	ULONG			chEaten			= 0;
	ULONG			cPropSets		= 0;
	DBPROPSET *		rgPropSets		= NULL;
	IMoniker*		pIMoniker		= NULL;
	IDBInitialize * pIDBInit		= NULL;
	ULONG i=0;

	if (!pIParseDisplayName)
		return FALSE;

	// Check each data source
	for(i=0; i<(g_cOLEDBProvNames+g_cOLEDBProvMDPNames); i++)
	{
		// Get moniker
		odtLog << g_rgwszProvName[i] << "\n";
		if (CHECK(m_hr=pIParseDisplayName->ParseDisplayName(NULL,
								g_rgwszParseName[i],&chEaten,&pIMoniker), S_OK))
		{	
			if (pIMoniker)
			{
				// Binds the moniker with the IDBInitialize interface
				CHECK(BindMoniker(pIMoniker, 0, IID_IDBInitialize, (LPVOID*)&pIDBInit), S_OK);

				// We must have pIDBInit
				TESTC(NULL != pIDBInit);
			}
		}

		SAFE_RELEASE(pIDBInit);
		SAFE_RELEASE(pIMoniker);
		
		// Free Properties we got back	
		FreeProperties(&cPropSets,&rgPropSets);
	}
	fResult = TRUE;
	
CLEANUP:
	SAFE_RELEASE(pIDBInit);
	SAFE_RELEASE(pIMoniker);

	// Make sure we loop all Providers
	COMPARE((g_cOLEDBProvNames+g_cOLEDBProvMDPNames), i);

	return fResult;
}


//////////////////////////////////////////////////////////////////
// Verify IRowset::GetNextRows on sources rowset				//
// 1. Create the sources rowset, asking for DBPROP_CANHOLDROWS	//
// to be VARIANT_FALSE if it is supported and writable			//
// 2. Get a chunk of the rowset									//
// 3. Call IRowset::GetNextRows again, without releasing the	//
// handles
//////////////////////////////////////////////////////////////////
ULONG CSourcesRowset::GetNextRowsWithoutRelAllRows()
{
	ULONG		fTestRes			= TEST_FAIL;
	DBCOUNTITEM	cRows				= 0;
	HROW		rghRows[FETCH_ROW_STD];
	HROW		*prghRows			= rghRows;
	DBCOUNTITEM	cRows1				= 0;
	HROW		rghRows1[FETCH_ROW_STD];
	HROW		*prghRows1			= rghRows1;
	IRowset		*pIRowset			= NULL;
	DBPROPSET	*rgPropSet			= NULL;
	ULONG		cPropSet			= 0;
	BOOL		fContinue			= TRUE;

	// Make sure we have ISrcRow pointer
	// This is either on root enumerator or on the provider enumerator
	TESTC(NULL != m_pISrcRow);
	
	if (SettableProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
		SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSet, &rgPropSet, VT_BOOL, (ULONG_PTR) VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);

	TESTC_(m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, cPropSet, rgPropSet,(IUnknown **) &pIRowset), S_OK);

	// If the Property is Not Supported
	if (cPropSet)
	{
		TESTC(NoHoldRows(pIRowset));
	}
	else 
	{
		if (!NoHoldRows(pIRowset))
		{
			fTestRes = TEST_SKIPPED;
			goto CLEANUP;
		}
	}

	// go through all the rowset
	while((m_hr=pIRowset->GetNextRows(NULL,0,FETCH_ROW_STD,&cRows,(HROW**)&prghRows)) == S_OK)
	{
		// verify that the proper return value is generated when the rows are not released
		m_hr=pIRowset->GetNextRows(NULL,0,FETCH_ROW_STD, &cRows1,(HROW**)&prghRows1);

		// release all rows now so in case verification fails we do not leak
		fContinue = CHECK(pIRowset->ReleaseRows(cRows, rghRows, NULL, 0, NULL), S_OK);
		if (SUCCEEDED(m_hr))
		{
			fContinue = fContinue && CHECK(pIRowset->ReleaseRows(cRows1, rghRows1, NULL, 0, NULL), S_OK);
		}

		//now do the verification
		TESTC(cRows == FETCH_ROW_STD);
		if (SUCCEEDED(m_hr))
		{
			TESTC(cRows1==FETCH_ROW_STD || m_hr==DB_S_ENDOFROWSET);
			TEST2C_(m_hr, DB_S_ENDOFROWSET, S_OK);
		}
		else
		{
			TESTC_(m_hr, DB_E_ROWSNOTRELEASED);
		}
		//if there were failures do not continue
		if(!fContinue)
			goto CLEANUP;
	}
	
	// Release rows from the last call to GetNextRows: even if it returned DB_S_ENDOFROWSET several rows still could be fetched
	if (SUCCEEDED(m_hr) && cRows)
	{
		CHECK(pIRowset->ReleaseRows(cRows, rghRows, NULL, 0, NULL), S_OK);
	}

	// check that you have gone through the whole rowset
	TESTC_(m_hr, DB_S_ENDOFROWSET);

	fTestRes= TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	FreeProperties(&cPropSet, &rgPropSet);
	return fTestRes;
} //CSourcesRowset::GetNextRowsWithoutRelAllRows



//////////////////////////////////////////////////////////////////
// Checks how ISourcesRowset::GetSourcesRowset deals with		//
// invalid colid passed in the DBPROP stru of a property		//
//////////////////////////////////////////////////////////////////
ULONG CSourcesRowset::VerifyInvalidColID(DBPROPOPTIONS dbPropOption)
{
	// there is not really much to be tested
	// there is no info about which property
	// is column specific that can ge obtained 
	// from the enumerator
	BOOL		 fResult	  = TEST_FAIL;
	DBPROPSTATUS dbPropStatus = DBPROPSTATUS_NOTSUPPORTED;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set the correct status
	if (!SettableProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
	{
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Set a property which can be set to individual column
	// Set an invalid colid
	SetOneProperty(DBPROP_DEFERRED, dbPropOption, 0);
	m_rgDBPropSets[0].rgProperties[0].colid = DBCOLUMN_MAYSORT;

	m_hr = m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset);

	switch (m_hr)
	{
		case S_OK:
			TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
			break;

		case DB_S_ERRORSOCCURRED:
			TESTC(DBPROPOPTIONS_OPTIONAL == dbPropOption);
			TESTC(NULL != m_pIRowset);
			// Make sure the property status was set correctly
			TESTC(DBPROPSTATUS_BADCOLUMN == m_rgDBPropSets[0].rgProperties[0].dwStatus);
			break;

		case DB_E_ERRORSOCCURRED:
			TESTC(DBPROPOPTIONS_REQUIRED == dbPropOption);
			TESTC(NULL == m_pIRowset);
			// Make sure the property status was set correctly
			TESTC(DBPROPSTATUS_BADCOLUMN == m_rgDBPropSets[0].rgProperties[0].dwStatus);
			break;

		default:
			TESTC(FALSE);
			break;
	}

	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
} //CSourcesRowset::VerifyInvalidColID



//////////////////////////////////////////////////////////////////
// Checks how ISourcesRowset::VerifyNonRowsetPropSet deals with		//
// non rowset property set asked								//
//////////////////////////////////////////////////////////////////
ULONG CSourcesRowset::VerifyNonRowsetPropSet(ULONG nColNo)
{
	BOOL			fResult	= TEST_FAIL;
	DBPROPSET		rgPropSet[1];
	DBPROP			rgProp[1];

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set the property in Column property group
	memset(rgProp, 0, sizeof(rgProp));
	memset(rgProp, 0, sizeof(rgProp));
	rgPropSet[0].rgProperties		= rgProp;
	rgPropSet[0].cProperties		= 1;
	rgPropSet[0].guidPropertySet	= DBPROPSET_COLUMN;
	rgProp[0].dwPropertyID			= DBPROP_COL_AUTOINCREMENT;
	rgProp[0].dwOptions				= DBPROPOPTIONS_OPTIONAL;
	rgProp[0].vValue.vt				= VT_BOOL;
	V_BOOL(&rgProp[0].vValue)		= VARIANT_TRUE;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		1,rgPropSet,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(nColNo, m_pIRowset));

	// Make sure the property status was set correctly
	TESTC(rgPropSet[0].rgProperties[0].dwStatus != DBPROPSTATUS_OK);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
} //CSourcesRowset::VerifyNonRowsetPropSet




//////////////////////////////////////////////////////////////////
// Checks how ISourcesRowset::CountNoOfRows 					//
// Counts the number of rows in the rowset (assume init position)//
//////////////////////////////////////////////////////////////////
HRESULT CSourcesRowset::CountNoOfRows(IRowset *pIRowset, ULONG *pulRowsNo)
{
	HRESULT		hr = E_FAIL;
	DBCOUNTITEM	cRowsObtained;
	HROW		rghRows[1];
	HROW		*prghRows = rghRows;

	if (NULL == pIRowset || NULL == pulRowsNo)
		return E_FAIL;
	
	*pulRowsNo = 0;

	for (; DB_S_ENDOFROWSET != hr; (*pulRowsNo)++)
	{
		TEST2C_(hr = pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, (HROW**)&prghRows), S_OK, DB_S_ENDOFROWSET);
		TESTC(S_OK != hr || S_OK == pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL));
	}

	(*pulRowsNo)--;
	hr = S_OK;

CLEANUP:
	return hr;
} //CSourcesRowset::CountNoOfRows



//////////////////////////////////////////////////////////////////
// Checks how ISourcesRowset::CountNoOfRows 					//
// Counts the number of rows in the rowset (assume init position) and stores first cMaxSourcesNames SOURCES_NAME in //
//////////////////////////////////////////////////////////////////
HRESULT CSourcesRowset::CountNoOfRowsAndGetSourcesNames(IRowset *pIRowset, ULONG *pulRowsNo, WCHAR** rgwszSourcesNames, ULONG cMaxSourcesNames)
{
	HRESULT		hr = E_FAIL;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW		*pHRow	= NULL;
	HACCESSOR			hAccessor=DB_NULL_HACCESSOR;
	void *				pData=NULL;
	DBLENGTH			cRowSize;
	DBCOUNTITEM			cBinding=0;
	DBBINDING *			rgBinding=NULL;
	DB_LORDINAL			colOrdinal = 1; // we're interested in first column, SOURCES_NAME
	WCHAR*				wszSourceName = NULL;

	if (NULL == pIRowset || NULL == pulRowsNo)
		return E_FAIL;
	
	*pulRowsNo = 0;

	if (cMaxSourcesNames && rgwszSourcesNames)
	{
		// Create an accessor on the sources rowset and get bindings
		TESTC_(hr=GetAccessorAndBindings(pIRowset,DBACCESSOR_ROWDATA,&hAccessor,
		&rgBinding,&cBinding,&cRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,FORWARD,NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,1,&colOrdinal,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,NO_BLOB_COLS),S_OK)
		
		// Allocate memory for the row
		SAFE_ALLOC(pData, BYTE, cRowSize);
	}

	for (; DB_S_ENDOFROWSET != hr; (*pulRowsNo)++)
	{
		TEST2C_(hr = pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, (HROW**)&pHRow), S_OK, DB_S_ENDOFROWSET);
		
		if (hr==S_OK && cMaxSourcesNames && rgwszSourcesNames && *pulRowsNo<cMaxSourcesNames && CHECK(pIRowset->GetData(*pHRow,hAccessor,pData), S_OK))
		{
			if (STATUS_BINDING(rgBinding[0],pData)==DBSTATUS_S_OK)
			{
				SAFE_ALLOC(rgwszSourcesNames[*pulRowsNo], WCHAR, wcslen((WCHAR*)&VALUE_BINDING(rgBinding[0],pData))+1); 
				wcscpy(rgwszSourcesNames[*pulRowsNo], (WCHAR*)&VALUE_BINDING(rgBinding[0],pData));
			}
			else
				rgwszSourcesNames[*pulRowsNo]=NULL;
		}

		if (pHRow)
		{
			CHECK(pIRowset->ReleaseRows(1, pHRow, NULL, NULL, NULL), S_OK);
			pHRow = NULL;
		}
	}

	(*pulRowsNo)--;

	hr = S_OK;

CLEANUP:
	SAFE_FREE(pData);
	FreeAccessorBindings(cBinding,rgBinding);
	if(hAccessor && pIRowset)
	{
		IAccessor * pIAccessor;

		CHECK(pIRowset->QueryInterface(IID_IAccessor, (LPVOID *)&pIAccessor),S_OK);
		if (pIAccessor)
		{
			pIAccessor->ReleaseAccessor(hAccessor, NULL);
			SAFE_RELEASE(pIAccessor);
		}
	}
	return hr;
} //CSourcesRowset::CountNoOfRows




//------------------------------------------------------------------
//
//	thread function
//------------------------------------------------------------------
unsigned CSourcesRowset::MyThreadProc(ULONG iThread)
{
	HRESULT		hr;
	IRowset		*pIRowset = NULL;

	if (iThread >= nThreads)
		return 0;

	Sleep(0);
	if (NULL != m_pISrcRow)
	{
		hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset);
		hr = SUCCEEDED(hr)? CountNoOfRows(pIRowset, &m_rgRowsNo[iThread]): hr;
	}
	else
		hr = E_FAIL;

	Sleep(0);
	m_rgResult[iThread] = hr;
	m_rgRowset[iThread] = pIRowset;
	return 1;
} //CSourcesRowset::MyThreadProc


//------------------------------------------------------------------
//
//	thread function
//------------------------------------------------------------------
unsigned CSourcesRowset::MyThreadProc2(ULONG iThread)
{
	HRESULT		hr;
	IRowset		*pIRowset = NULL;

	if (iThread >= nThreads)
		return 0;

	Sleep(0);
	if (NULL != m_pISrcRow)
	{
		hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset);
		hr = SUCCEEDED(hr)? CountNoOfRowsAndGetSourcesNames(pIRowset, &m_rgRowsNo[iThread], m_rgSourcesNames[iThread], m_rgMaxSourcesNames[iThread]): hr;
	}
	else
		hr = E_FAIL;

	Sleep(0);
	m_rgResult[iThread] = hr;
	m_rgRowset[iThread] = pIRowset;
	return 1;
} //CSourcesRowset::MyThreadProc


//----------------------------------------------------------------------------------------------------------
//
// Verify aggregation on sources rowset, ask for something different from IID_IUnknown => DB_E_NOAGGREGATION
//
//----------------------------------------------------------------------------------------------------------
BOOL CSourcesRowset::VerifyNotIUnknownInAggregation()
{
	TBEGIN
	IUnknown	*pIUnkOuter = NULL;
	IUnknown	*pIUnkInner	= NULL;
	HRESULT		hr;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Create an outer rowset object
	if (CHECK(m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, &pIUnkOuter), S_OK))
	{
		CAggregate Aggregate(pIUnkOuter);

		// Dirty the output params
		pIUnkInner = INVALID(IUnknown*);

		// Create an inner rowset object on IID_IRowset
		CHECK(hr = m_pISrcRow->GetSourcesRowset(&Aggregate, IID_IRowset, 0, 
			NULL, &pIUnkInner), DB_E_NOAGGREGATION);

		//Inner object cannot RefCount the outer object - COM rule for CircularRef
		COMPARE(Aggregate.GetRefCount(), 1);
		COMPARE(pIUnkInner == NULL, TRUE);
	}

CLEANUP:
	// Release the outer rowset object
	SAFE_RELEASE_(pIUnkOuter);
	// No rowset returned
	COMPARE(pIUnkInner, NULL);
	TRETURN
} // CSourcesRowset::VerifyNotIUnknownInAggregation



//---------------------------------------------------
//
// Verify sources rowset aggregation
//
//---------------------------------------------------
BOOL CSourcesRowset::VerifySourcesRowsetAggregation()
{
	TBEGIN
	IUnknown	*pIUnkOuter	= NULL;
	IUnknown	*pIUnkInner	= NULL;
	HRESULT		hr;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Create an outer rowset object 
	if (CHECK(hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
												0, NULL, &pIUnkOuter), S_OK))
	{
		CAggregate Aggregate(pIUnkOuter);

		// Dirty the output params
		pIUnkInner = INVALID(IUnknown*);

		// Create an inner rowset object on IID_IUnknown
		hr = m_pISrcRow->GetSourcesRowset(&Aggregate, IID_IUnknown, 0, NULL, &pIUnkInner);
		
		// Check the return code
		if (FAILED(hr))
		{
			CHECK(hr, DB_E_NOAGGREGATION);
			COMPARE(NULL == pIUnkInner, TRUE);
		}
		else
		{
			Aggregate.SetUnkInner(pIUnkInner);
			CHECK(hr, S_OK);
			COMPARE(NULL != pIUnkInner, TRUE);
			COMPARE(Aggregate.VerifyAggregationQI(hr, IID_IRowset), TRUE);
		}
	}

CLEANUP:
	// Release the outer rowset object
	SAFE_RELEASE_(pIUnkOuter);
	// Release the inner rowset object
	SAFE_RELEASE_(pIUnkInner);
	TRETURN
} //CSourcesRowset::VerifySourcesRowsetAggregation



//------------------------------------------------------------------------------
//
// Verify IRowsetInfo::GetSpecification on sources rowset, aggregated enumerator
//
//------------------------------------------------------------------------------
BOOL CSourcesRowset::VerifyGetSpec_AggregatedEnum()
{
	TBEGIN
    ISourcesRowset	*pISourcesRowset	= NULL;
	IRowsetInfo		*pIRowsetInfo		= NULL;
	IUnknown		*pIAggregate		= NULL;
	IUnknown		*pIUnkInner			= NULL;
	ULONG			ulRefCountBefore;
	ULONG			ulRefCountAfter;
	HRESULT			hr;

	CAggregate Aggregate(m_pISrcRow);

	//Obtain the Enumerator  (note: Enumerator is Aggregated, not rowset)
	//NOTE:  The ParseDisplayName interface has no way to do aggregation!
	//Thus I'm reallying upon the Root Enumerator wszParseName to be the CLSID
	//which it is ans will never change, but just a note as to why I'm not
	//calling IParseDisplayName::ParseDisplayName on the Enum wszParseName...
	hr = CoCreateInstance(m_clsid, &Aggregate, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset));

	//ISourcesRowset::GetRowset
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pISourcesRowset->GetSourcesRowset(NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(S_OK == hr)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pISourcesRowset));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		COMPARE(ulRefCountAfter > ulRefCountBefore, TRUE);
	}
	else
	{
		COMPARE(NULL == pIAggregate, TRUE);
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}

CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} //CSourcesRowset::VerifyGetSpec_AggregatedEnum





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCSourcesRowset_RootEnumerator)
//--------------------------------------------------------------------
// @class ISourcesRowset::GetSourcesRowset test for root enumerator
//
class TCSourcesRowset_RootEnumerator : public CSourcesRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSourcesRowset_RootEnumerator,CSourcesRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember QI with NULL -- E_INVALIDARG
	int Variation_1();
	// @cmember PropertySets N, NULL -- E_INVALIDARG
	int Variation_2();
	// @cmember Property N, NULL -- E_INVALIDARG
	int Variation_3();
	// @cmember riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE
	int Variation_4();
	// @cmember ppSourcesRowset = NULL -- E_INVALIDARG
	int Variation_5();
	// @cmember QI IDBInitialize -- E_NOINTERFACE
	int Variation_6();
	// @cmember QI IDBProperties -- E_NOINTERFACE
	int Variation_7();
	// @cmember open sources rowset on IRowsetUpdate -- E_NOINTERFACE
	int Variation_8();
	// @cmember open sources rowset on IDBCreateSession -- E_NOINTERFACE
	int Variation_9();
	// @cmember riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE
	int Variation_10();
	// @cmember riid = IID_NULL, valid property -- E_NOINTERFACE
	int Variation_11();
	// @cmember pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION
	int Variation_12();
	// @cmember Call GetSourcesRowset while uninitialized -- E_UNEXPECTED
	int Variation_13();
	// @cmember pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION
	int Variation_14();
	// @cmember QI IParseDisplayName -- S_OK
	int Variation_15();
	// @cmember QI ISupportErrorInfo -- S_OK
	int Variation_16();
	// @cmember create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK
	int Variation_17();
	// @cmember pISourcesRowset->AddRef, Release -- S_OK
	int Variation_18();
	// @cmember PropertySets 0, NULL -- S_OK
	int Variation_19();
	// @cmember PropertySets 0, valid -- S_OK
	int Variation_20();
	// @cmember PropertySets N, valid -- S_OK
	int Variation_21();
	// @cmember DBPROP_COMMANDTIMEOUT -- S_OK
	int Variation_22();
	// @cmember Property 0, NULL -- S_OK
	int Variation_23();
	// @cmember Property 0, valid -- S_OK
	int Variation_24();
	// @cmember Property N, valid -- S_OK
	int Variation_25();
	// @cmember one prop, DBOPTION_REQUIRED -- S_OK
	int Variation_26();
	// @cmember create enumerator object on IParseDisplayName -- S_OK
	int Variation_27();
	// @cmember open sources rowset on IAccessor -- S_OK
	int Variation_28();
	// @cmember open sources rowset on IColunmsInfo -- S_OK
	int Variation_29();
	// @cmember open sources rowset on IRowsetInfo -- S_OK
	int Variation_30();
	// @cmember open sources rowset on IUnknown -- S_OK
	int Variation_31();
	// @cmember IRowsetInfo->GetSpecification -- S_FALSE
	int Variation_32();
	// @cmember open sources rowset, getnextrow, release, getnextrow -- S_OK
	int Variation_33();
	// @cmember property was not supported -- DB_S_ERRORSOCCURRED
	int Variation_34();
	// @cmember property was not in Rowset property group -- DB_S_ERRORSOCCURRED
	int Variation_35();
	// @cmember property set was not supported - DB_S_ERRORSOCCURRED
	int Variation_36();
	// @cmember property was not cheap to set -- DB_S_ERRORSOCCURRED
	int Variation_37();
	// @cmember invalid colid -- DB_S_ERRORSOCCURRED
	int Variation_38();
	// @cmember invalid dwOptions -- DB_E_ERRORSOCCURRED
	int Variation_39();
	// @cmember invalid vValue -- DB_S_ERRORSOCCURRED
	int Variation_40();
	// @cmember incorrect data type -- DB_S_ERRORSOCCURRED
	int Variation_41();
	// @cmember vValue = VT_EMPTY -- S_OK
	int Variation_42();
	// @cmember open sources rowset twice -- S_OK
	int Variation_43();
	// @cmember open sources rowset, close, open, open -- S_OK
	int Variation_44();
	// @cmember property was not applied to all columns -- DB_S_ERRORSOCCURRED
	int Variation_45();
	// @cmember property was not supported -- DB_E_ERRORSOCCURRED
	int Variation_46();
	// @cmember property was not in Rowset property group -- DB_E_ERRORSOCCURRED
	int Variation_47();
	// @cmember property set was not supported -- DB_E_ERRORSOCCURRED
	int Variation_48();
	// @cmember invalid colid -- DB_E_ERRORSOCCURRED
	int Variation_49();
	// @cmember invalid dwOptions -- DB_E_ERRORSOCCURRED
	int Variation_50();
	// @cmember invalid vValue -- DB_E_ERRORSOCCURRED
	int Variation_51();
	// @cmember incorrect data type -- DB_E_ERRORSOCCURRED
	int Variation_52();
	// @cmember vValue = VT_EMPTY -- S_OK
	int Variation_53();
	// @cmember IParseDisplayName::ParseDisplayName without rowset open -- S_OK
	int Variation_54();
	// @cmember IParseDisplayName::ParseDisplayName -- S_OK
	int Variation_55();
	// @cmember ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT
	int Variation_56();
	// @cmember ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT
	int Variation_57();
	// @cmember ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED
	int Variation_58();
	// @cmember IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE
	int Variation_59();
	// @cmember open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED
	int Variation_60();
	// @cmember check that all mutual dso & enum are in rowset -- S_OK
	int Variation_61();
	// @cmember Ask IID_IRowsetChange on sources rowset
	int Variation_62();
	// @cmember Ask IID_IRowsetUpdate on sources rowset
	int Variation_63();
	// @cmember Ask DBPROP_IRowsetChange on sources rowset
	int Variation_64();
	// @cmember Ask for DBPROP_IRowsetUpdate on sources rowset
	int Variation_65();
	// @cmember QI for IRowsetChange and IRowsetUpdate on a common sources rowset
	int Variation_66();
	// @cmember ParseDisplayName: unknown displayName (not a proper string) -- MK_E_NOOBJECT
	int Variation_67();
	// @cmember ParseDisplayName: not null pbc (IBindCtx) => S_OK
	int Variation_68();
	// @cmember Multiple threads retrieve the sources rowset: check the number of rows
	int Variation_69();
	// @cmember Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification
	int Variation_70();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSourcesRowset_RootEnumerator)
#define THE_CLASS TCSourcesRowset_RootEnumerator
BEG_TEST_CASE(TCSourcesRowset_RootEnumerator, CSourcesRowset, L"ISourcesRowset::GetSourcesRowset test for root enumerator")
	TEST_VARIATION(1, 		L"QI with NULL -- E_INVALIDARG")
	TEST_VARIATION(2, 		L"PropertySets N, NULL -- E_INVALIDARG")
	TEST_VARIATION(3, 		L"Property N, NULL -- E_INVALIDARG")
	TEST_VARIATION(4, 		L"riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE")
	TEST_VARIATION(5, 		L"ppSourcesRowset = NULL -- E_INVALIDARG")
	TEST_VARIATION(6, 		L"QI IDBInitialize -- E_NOINTERFACE")
	TEST_VARIATION(7, 		L"QI IDBProperties -- E_NOINTERFACE")
	TEST_VARIATION(8, 		L"open sources rowset on IRowsetUpdate -- E_NOINTERFACE")
	TEST_VARIATION(9, 		L"open sources rowset on IDBCreateSession -- E_NOINTERFACE")
	TEST_VARIATION(10, 		L"riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE")
	TEST_VARIATION(11, 		L"riid = IID_NULL, valid property -- E_NOINTERFACE")
	TEST_VARIATION(12, 		L"pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION")
	TEST_VARIATION(13, 		L"Call GetSourcesRowset while uninitialized -- E_UNEXPECTED")
	TEST_VARIATION(14, 		L"pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION")
	TEST_VARIATION(15, 		L"QI IParseDisplayName -- S_OK")
	TEST_VARIATION(16, 		L"QI ISupportErrorInfo -- S_OK")
	TEST_VARIATION(17, 		L"create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK")
	TEST_VARIATION(18, 		L"pISourcesRowset->AddRef, Release -- S_OK")
	TEST_VARIATION(19, 		L"PropertySets 0, NULL -- S_OK")
	TEST_VARIATION(20, 		L"PropertySets 0, valid -- S_OK")
	TEST_VARIATION(21, 		L"PropertySets N, valid -- S_OK")
	TEST_VARIATION(22, 		L"DBPROP_COMMANDTIMEOUT -- S_OK")
	TEST_VARIATION(23, 		L"Property 0, NULL -- S_OK")
	TEST_VARIATION(24, 		L"Property 0, valid -- S_OK")
	TEST_VARIATION(25, 		L"Property N, valid -- S_OK")
	TEST_VARIATION(26, 		L"one prop, DBOPTION_REQUIRED -- S_OK")
	TEST_VARIATION(27, 		L"create enumerator object on IParseDisplayName -- S_OK")
	TEST_VARIATION(28, 		L"open sources rowset on IAccessor -- S_OK")
	TEST_VARIATION(29, 		L"open sources rowset on IColunmsInfo -- S_OK")
	TEST_VARIATION(30, 		L"open sources rowset on IRowsetInfo -- S_OK")
	TEST_VARIATION(31, 		L"open sources rowset on IUnknown -- S_OK")
	TEST_VARIATION(32, 		L"IRowsetInfo->GetSpecification -- S_FALSE")
	TEST_VARIATION(33, 		L"open sources rowset, getnextrow, release, getnextrow -- S_OK")
	TEST_VARIATION(34, 		L"property was not supported -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(35, 		L"property was not in Rowset property group -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(36, 		L"property set was not supported - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(37, 		L"property was not cheap to set -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(38, 		L"invalid colid -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(39, 		L"invalid dwOptions -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(40, 		L"invalid vValue -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(41, 		L"incorrect data type -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(42, 		L"vValue = VT_EMPTY -- S_OK")
	TEST_VARIATION(43, 		L"open sources rowset twice -- S_OK")
	TEST_VARIATION(44, 		L"open sources rowset, close, open, open -- S_OK")
	TEST_VARIATION(45, 		L"property was not applied to all columns -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(46, 		L"property was not supported -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(47, 		L"property was not in Rowset property group -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(48, 		L"property set was not supported -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(49, 		L"invalid colid -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(50, 		L"invalid dwOptions -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(51, 		L"invalid vValue -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(52, 		L"incorrect data type -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(53, 		L"vValue = VT_EMPTY -- S_OK")
	TEST_VARIATION(54, 		L"IParseDisplayName::ParseDisplayName without rowset open -- S_OK")
	TEST_VARIATION(55, 		L"IParseDisplayName::ParseDisplayName -- S_OK")
	TEST_VARIATION(56, 		L"ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT")
	TEST_VARIATION(57, 		L"ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT")
	TEST_VARIATION(58, 		L"ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED")
	TEST_VARIATION(59, 		L"IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE")
	TEST_VARIATION(60, 		L"open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED")
	TEST_VARIATION(61, 		L"check that all mutual dso & enum are in rowset -- S_OK")
	TEST_VARIATION(62, 		L"Ask IID_IRowsetChange on sources rowset")
	TEST_VARIATION(63, 		L"Ask IID_IRowsetUpdate on sources rowset")
	TEST_VARIATION(64, 		L"Ask DBPROP_IRowsetChange on sources rowset")
	TEST_VARIATION(65, 		L"Ask for DBPROP_IRowsetUpdate on sources rowset")
	TEST_VARIATION(66, 		L"QI for IRowsetChange and IRowsetUpdate on a common sources rowset")
	TEST_VARIATION(67, 		L"ParseDisplayName: unknown displayName (not a proper string) -- MK_E_NOOBJECT")
	TEST_VARIATION(68, 		L"ParseDisplayName: not null pbc (IBindCtx) => S_OK")
	TEST_VARIATION(69, 		L"Multiple threads retrieve the sources rowset: check the number of rows")
	TEST_VARIATION(70, 		L"Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_RootEnumerator)
//--------------------------------------------------------------------
// @class Extended error test for root enumerator
//
class TCExtendedErrors_RootEnumerator : public CSourcesRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_RootEnumerator,CSourcesRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK -- with previous error existing
	int Variation_1();
	// @cmember E_INVALIDARG -- with previous error existing
	int Variation_2();
	// @cmember E_NOINTERFACE -- with no previous error existing
	int Variation_3();
	// @cmember DB_S_ERRORSOCCURRED -- with previous error existing
	int Variation_4();
	// @cmember DB_E_ERRORSOCCURRED -- with no previous error existing
	int Variation_5();
	// @cmember E_INVALIDARG -- with previous error existing
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors_RootEnumerator)
#define THE_CLASS TCExtendedErrors_RootEnumerator
BEG_TEST_CASE(TCExtendedErrors_RootEnumerator, CSourcesRowset, L"Extended error test for root enumerator")
	TEST_VARIATION(1, 		L"S_OK -- with previous error existing")
	TEST_VARIATION(2, 		L"E_INVALIDARG -- with previous error existing")
	TEST_VARIATION(3, 		L"E_NOINTERFACE -- with no previous error existing")
	TEST_VARIATION(4, 		L"DB_S_ERRORSOCCURRED -- with previous error existing")
	TEST_VARIATION(5, 		L"DB_E_ERRORSOCCURRED -- with no previous error existing")
	TEST_VARIATION(6, 		L"E_INVALIDARG -- with previous error existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSourcesRowset_ProviderEnumerator)
//--------------------------------------------------------------------
// @class ISourcesRowset::GetSourcesRowset for standard enumerator
//
class TCSourcesRowset_ProviderEnumerator : public CSourcesRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	// @cmember a string to hold SOURCES_PARSENAME for an enumerator assoc with the provider
	WCHAR		*m_rgwszSourceParseName;

	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSourcesRowset_ProviderEnumerator,CSourcesRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember QI with NULL -- E_INVALIDARG
	int Variation_1();
	// @cmember PropertySets N, NULL -- E_INVALIDARG
	int Variation_2();
	// @cmember Property N, NULL -- E_INVALIDARG
	int Variation_3();
	// @cmember riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE
	int Variation_4();
	// @cmember ppSourcesRowset = NULL -- E_INVALIDARG
	int Variation_5();
	// @cmember QI IDBInitialize -- E_NOINTERFACE
	int Variation_6();
	// @cmember QI IDBProperties -- E_NOINTERFACE
	int Variation_7();
	// @cmember open sources rowset on IRowsetUpdate -- E_NOINTERFACE
	int Variation_8();
	// @cmember open sources rowset on IDBCreateSession -- E_NOINTERFACE
	int Variation_9();
	// @cmember riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE
	int Variation_10();
	// @cmember riid = IID_NULL, valid property -- E_NOINTERFACE
	int Variation_11();
	// @cmember pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION
	int Variation_12();
	// @cmember Call GetSourcesRowset while uninitialized -- E_UNEXPECTED
	int Variation_13();
	// @cmember pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION
	int Variation_14();
	// @cmember QI IParseDisplayName -- S_OK
	int Variation_15();
	// @cmember QI ISupportErrorInfo -- S_OK
	int Variation_16();
	// @cmember create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK
	int Variation_17();
	// @cmember pISourcesRowset->AddRef, Release -- S_OK
	int Variation_18();
	// @cmember PropertySets 0, NULL -- S_OK
	int Variation_19();
	// @cmember PropertySets 0, valid -- S_OK
	int Variation_20();
	// @cmember PropertySets N, valid -- S_OK
	int Variation_21();
	// @cmember DBPROP_COMMANDTIMEOUT -- S_OK
	int Variation_22();
	// @cmember Property 0, NULL -- S_OK
	int Variation_23();
	// @cmember Property 0, valid -- S_OK
	int Variation_24();
	// @cmember Property N, valid -- S_OK
	int Variation_25();
	// @cmember one prop, DBOPTION_REQUIRED -- S_OK
	int Variation_26();
	// @cmember create enumerator object on IParseDisplayName -- S_OK
	int Variation_27();
	// @cmember open sources rowset on IAccessor -- S_OK
	int Variation_28();
	// @cmember open sources rowset on IColunmsInfo -- S_OK
	int Variation_29();
	// @cmember open sources rowset on IRowsetInfo -- S_OK
	int Variation_30();
	// @cmember open sources rowset on IUnknown -- S_OK
	int Variation_31();
	// @cmember IRowsetInfo->GetSpecification -- S_FALSE
	int Variation_32();
	// @cmember open sources rowset, getnextrow, release, getnextrow -- S_OK
	int Variation_33();
	// @cmember property was not supported -- DB_S_ERRORSOCCURRED
	int Variation_34();
	// @cmember property was not in Rowset property group -- DB_S_ERRORSOCCURRED
	int Variation_35();
	// @cmember property set was not supported - DB_S_ERRORSOCCURRED
	int Variation_36();
	// @cmember property was not cheap to set -- DB_S_ERRORSOCCURRED
	int Variation_37();
	// @cmember invalid colid -- DB_S_ERRORSOCCURRED
	int Variation_38();
	// @cmember invalid dwOptions -- DB_E_ERRORSOCCURRED
	int Variation_39();
	// @cmember invalid vValue -- DB_S_ERRORSOCCURRED
	int Variation_40();
	// @cmember incorrect data type -- DB_S_ERRORSOCCURRED
	int Variation_41();
	// @cmember vValue = VT_EMPTY -- S_OK
	int Variation_42();
	// @cmember open sources rowset twice -- S_OK
	int Variation_43();
	// @cmember open sources rowset, close, open, open -- S_OK
	int Variation_44();
	// @cmember property was not applied to all columns -- DB_S_ERRORSOCCURRED
	int Variation_45();
	// @cmember property was not supported -- DB_E_ERRORSOCCURRED
	int Variation_46();
	// @cmember property was not in Rowset property group -- DB_E_ERRORSOCCURRED
	int Variation_47();
	// @cmember property set was not supported -- DB_E_ERRORSOCCURRED
	int Variation_48();
	// @cmember invalid colid -- DB_E_ERRORSOCCURRED
	int Variation_49();
	// @cmember invalid dwOptions -- DB_E_ERRORSOCCURRED
	int Variation_50();
	// @cmember invalid vValue -- DB_E_ERRORSOCCURRED
	int Variation_51();
	// @cmember incorrect data type -- DB_E_ERRORSOCCURRED
	int Variation_52();
	// @cmember vValue = VT_EMPTY -- S_OK
	int Variation_53();
	// @cmember IParseDisplayName::ParseDisplayName without rowset open -- S_OK
	int Variation_54();
	// @cmember IParseDisplayName::ParseDisplayName -- S_OK
	int Variation_55();
	// @cmember ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT
	int Variation_56();
	// @cmember ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT
	int Variation_57();
	// @cmember ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED
	int Variation_58();
	// @cmember IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE
	int Variation_59();
	// @cmember open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED
	int Variation_60();
	// @cmember Multiple threads retrieve the sources rowset: check the number of rows
	int Variation_61();
	// @cmember Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification
	int Variation_62();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSourcesRowset_ProviderEnumerator)
#define THE_CLASS TCSourcesRowset_ProviderEnumerator
BEG_TEST_CASE(TCSourcesRowset_ProviderEnumerator, CSourcesRowset, L"ISourcesRowset::GetSourcesRowset for standard enumerator")
	TEST_VARIATION(1, 		L"QI with NULL -- E_INVALIDARG")
	TEST_VARIATION(2, 		L"PropertySets N, NULL -- E_INVALIDARG")
	TEST_VARIATION(3, 		L"Property N, NULL -- E_INVALIDARG")
	TEST_VARIATION(4, 		L"riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE")
	TEST_VARIATION(5, 		L"ppSourcesRowset = NULL -- E_INVALIDARG")
	TEST_VARIATION(6, 		L"QI IDBInitialize -- E_NOINTERFACE")
	TEST_VARIATION(7, 		L"QI IDBProperties -- E_NOINTERFACE")
	TEST_VARIATION(8, 		L"open sources rowset on IRowsetUpdate -- E_NOINTERFACE")
	TEST_VARIATION(9, 		L"open sources rowset on IDBCreateSession -- E_NOINTERFACE")
	TEST_VARIATION(10, 		L"riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE")
	TEST_VARIATION(11, 		L"riid = IID_NULL, valid property -- E_NOINTERFACE")
	TEST_VARIATION(12, 		L"pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION")
	TEST_VARIATION(13, 		L"Call GetSourcesRowset while uninitialized -- E_UNEXPECTED")
	TEST_VARIATION(14, 		L"pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION")
	TEST_VARIATION(15, 		L"QI IParseDisplayName -- S_OK")
	TEST_VARIATION(16, 		L"QI ISupportErrorInfo -- S_OK")
	TEST_VARIATION(17, 		L"create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK")
	TEST_VARIATION(18, 		L"pISourcesRowset->AddRef, Release -- S_OK")
	TEST_VARIATION(19, 		L"PropertySets 0, NULL -- S_OK")
	TEST_VARIATION(20, 		L"PropertySets 0, valid -- S_OK")
	TEST_VARIATION(21, 		L"PropertySets N, valid -- S_OK")
	TEST_VARIATION(22, 		L"DBPROP_COMMANDTIMEOUT -- S_OK")
	TEST_VARIATION(23, 		L"Property 0, NULL -- S_OK")
	TEST_VARIATION(24, 		L"Property 0, valid -- S_OK")
	TEST_VARIATION(25, 		L"Property N, valid -- S_OK")
	TEST_VARIATION(26, 		L"one prop, DBOPTION_REQUIRED -- S_OK")
	TEST_VARIATION(27, 		L"create enumerator object on IParseDisplayName -- S_OK")
	TEST_VARIATION(28, 		L"open sources rowset on IAccessor -- S_OK")
	TEST_VARIATION(29, 		L"open sources rowset on IColunmsInfo -- S_OK")
	TEST_VARIATION(30, 		L"open sources rowset on IRowsetInfo -- S_OK")
	TEST_VARIATION(31, 		L"open sources rowset on IUnknown -- S_OK")
	TEST_VARIATION(32, 		L"IRowsetInfo->GetSpecification -- S_FALSE")
	TEST_VARIATION(33, 		L"open sources rowset, getnextrow, release, getnextrow -- S_OK")
	TEST_VARIATION(34, 		L"property was not supported -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(35, 		L"property was not in Rowset property group -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(36, 		L"property set was not supported - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(37, 		L"property was not cheap to set -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(38, 		L"invalid colid -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(39, 		L"invalid dwOptions -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(40, 		L"invalid vValue -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(41, 		L"incorrect data type -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(42, 		L"vValue = VT_EMPTY -- S_OK")
	TEST_VARIATION(43, 		L"open sources rowset twice -- S_OK")
	TEST_VARIATION(44, 		L"open sources rowset, close, open, open -- S_OK")
	TEST_VARIATION(45, 		L"property was not applied to all columns -- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(46, 		L"property was not supported -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(47, 		L"property was not in Rowset property group -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(48, 		L"property set was not supported -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(49, 		L"invalid colid -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(50, 		L"invalid dwOptions -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(51, 		L"invalid vValue -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(52, 		L"incorrect data type -- DB_E_ERRORSOCCURRED")
	TEST_VARIATION(53, 		L"vValue = VT_EMPTY -- S_OK")
	TEST_VARIATION(54, 		L"IParseDisplayName::ParseDisplayName without rowset open -- S_OK")
	TEST_VARIATION(55, 		L"IParseDisplayName::ParseDisplayName -- S_OK")
	TEST_VARIATION(56, 		L"ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT")
	TEST_VARIATION(57, 		L"ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT")
	TEST_VARIATION(58, 		L"ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED")
	TEST_VARIATION(59, 		L"IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE")
	TEST_VARIATION(60, 		L"open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED")
	TEST_VARIATION(61, 		L"Multiple threads retrieve the sources rowset: check the number of rows")
	TEST_VARIATION(62, 		L"Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(TCExtendedErrors_ProviderEnumerator)
//--------------------------------------------------------------------
// @class Extended error test for standard enumerator
//
class TCExtendedErrors_ProviderEnumerator : public CSourcesRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember a string to hold SOURCES_PARSENAME for an enumerator assoc with the provider
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors_ProviderEnumerator,CSourcesRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK -- with previous error existing
	int Variation_1();
	// @cmember E_INVALIDARG -- with previous error existing
	int Variation_2();
	// @cmember E_NOINTERFACE -- with no previous error existing
	int Variation_3();
	// @cmember DB_S_ERRORSOCCURRED -- with previous error existing
	int Variation_4();
	// @cmember DB_E_ERRORSOCCURRED -- with no previous error existing
	int Variation_5();
	// @cmember E_INVALIDARG -- with previous error existing
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors_ProviderEnumerator)
#define THE_CLASS TCExtendedErrors_ProviderEnumerator
BEG_TEST_CASE(TCExtendedErrors_ProviderEnumerator, CSourcesRowset, L"Extended error test for standard enumerator")
	TEST_VARIATION(1, 		L"S_OK -- with previous error existing")
	TEST_VARIATION(2, 		L"E_INVALIDARG -- with previous error existing")
	TEST_VARIATION(3, 		L"E_NOINTERFACE -- with no previous error existing")
	TEST_VARIATION(4, 		L"DB_S_ERRORSOCCURRED -- with previous error existing")
	TEST_VARIATION(5, 		L"DB_E_ERRORSOCCURRED -- with no previous error existing")
	TEST_VARIATION(6, 		L"E_INVALIDARG -- with previous error existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCSourcesRowset_RootEnumerator)
	TEST_CASE(2, TCExtendedErrors_RootEnumerator)
	TEST_CASE(3, TCSourcesRowset_ProviderEnumerator)
	TEST_CASE(4, TCExtendedErrors_ProviderEnumerator)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCSourcesRowset_RootEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		TCSourcesRowset_RootEnumerator - ISourcesRowset::GetSourcesRowset test for root enumerator
//|	Created:			09/11/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSourcesRowset_RootEnumerator::Init()
{
	BOOL	fResult = TEST_FAIL;

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(CSourcesRowset::Init());

	m_clsid = CLSID_OLEDB_ENUMERATOR;
		
	// Create an enumerator Object
	// Read the parse name from root enumerator sources rowset to a buffer
	TESTC_(m_hr=CoCreateInstance(m_clsid, NULL, 
		GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&m_pISrcRow), S_OK);
	TESTC(ReadParseName(CLSID_OLEDB_ENUMERATOR));
	fResult = TRUE;

CLEANUP:
	return fResult;
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc QI with NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_1()
{
	TBEGIN;

	// Make sure we have ISrcRow pointer
	if (!m_pISrcRow)
		return TEST_FAIL;
	
	// Pass a NULL pointer 
	//	Note: From MSDN: QueryInterface returns S_OK if the interface is supported, E_NOINTERFACE if not
	//		QueryInterface does not require to return E_INVALIDARG on NULL pointer
	TEST2C_(m_hr=m_pISrcRow->QueryInterface(IID_IParseDisplayName,NULL), E_INVALIDARG, E_POINTER);
CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc PropertySets N, NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_2()
{
	BOOL fResult = TEST_FAIL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// cPropertySets (3, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
							3, NULL, (IUnknown **) &m_pIRowset), E_INVALIDARG);
	// No rowset returned
	TESTC(NULL == m_pIRowset);

	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Property N, NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_3()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	m_cDBPropSets = 1;
	m_rgDBPropSets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPropSets[0].cProperties = 2;
	m_rgDBPropSets[0].rgProperties = NULL;

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// cProperty (N, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,m_cDBPropSets, 
						m_rgDBPropSets, (IUnknown **) &m_pIRowset), E_INVALIDARG);

	// No rowset returned
	TESTC(NULL == m_pIRowset);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_4()
{
	TBEGIN

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL, cPropertySets (N, NULL) 
	TEST2C_(m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,3, 
		NULL, (IUnknown **) &m_pIRowset), E_INVALIDARG, E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ppSourcesRowset = NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_5()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	if (!m_pISrcRow)
		return TEST_FAIL;

	// ppSourcesRowset is NULL 
	if (CHECK(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
												0, NULL, NULL), E_INVALIDARG))
		fResult = TEST_PASS;

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc QI IDBInitialize -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_6()
{
	BOOL			fResult			= TEST_FAIL;
	IDBInitialize * pIDBInitialize	= NULL;

	// Make sure we have ISrcRow pointer
	if (!m_pISrcRow)
		return TEST_FAIL;
	
	// Dirty the output params
	pIDBInitialize = (IDBInitialize*)0x12345678;

	// Enumerator does not require IDBInitialize verifyInterface
	if (!VerifyInterface(m_pISrcRow, IID_IDBInitialize, 
						ENUMERATOR_INTERFACE, (IUnknown **)&pIDBInitialize))
	{	
		// If not supported 
		fResult = TEST_PASS;
		odtLog << L"IDBInitialize is not supported by the Enumerator." << ENDL;
	}
	else
		fResult = TEST_PASS;

	SAFE_RELEASE(pIDBInitialize);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc QI IDBProperties -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_7()
{
	BOOL			fResult			= TEST_FAIL;
	IDBProperties * pIDBProperties	= NULL;

	// Make sure we have ISrcRow pointer
	if (!m_pISrcRow)
		return TEST_FAIL;
	
	// Dirty the output params
	pIDBProperties = (IDBProperties*)0x12345678;

	// Enumerator do not require IDBProperties verifyInterface
	if (!VerifyInterface(m_pISrcRow, IID_IDBProperties, 
						ENUMERATOR_INTERFACE, (IUnknown **)&pIDBProperties))
	{	
		// If not supported 
		fResult = TEST_PASS;
		odtLog << L"IDBProperties is not supported by the Enumerator." << ENDL;
	}
	else
		fResult = TEST_PASS;

	SAFE_RELEASE(pIDBProperties);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IRowsetUpdate -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_8()
{
	BOOL			fResult			= TEST_FAIL;
	IRowsetUpdate	*pIRowsetUpdate = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	pIRowsetUpdate = (IRowsetUpdate*)0x12345678;

	// Sources rowset is read-only 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetUpdate,
						0, NULL, (IUnknown **) &pIRowsetUpdate), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == pIRowsetUpdate);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IDBCreateSession -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_9()
{
	BOOL				fResult			  = TEST_FAIL;
	IDBCreateSession*	pIDBCreateSession = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	pIDBCreateSession = (IDBCreateSession*)0x12345678;

	// For a session object interface 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IDBCreateSession,
						0, NULL, (IUnknown **) &pIDBCreateSession), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == pIDBCreateSession);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_10()
{
	TBEGIN

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,
							0, NULL, (IUnknown **) &m_pIRowset), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, valid property -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//											   
int TCSourcesRowset_RootEnumerator::Variation_11()
{
	TBEGIN
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL, cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,m_cDBPropSets,
					m_rgDBPropSets, (IUnknown **) &m_pIRowset), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_12()
{
	TBEGIN
	IUnknown	*pIUnkOuter = NULL;
	IUnknown	*pIUnkInner	= NULL;
	HRESULT		hr;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Create an outer rowset object
	if (CHECK(m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, &pIUnkOuter), S_OK))
	{
		CAggregate Aggregate(pIUnkOuter);

		// Dirty the output params
		pIUnkInner = INVALID(IUnknown*);

		// Create an inner rowset object on IID_IRowset
		CHECK(hr = m_pISrcRow->GetSourcesRowset(&Aggregate, IID_IRowset, 0, 
			NULL, &pIUnkInner), DB_E_NOAGGREGATION);

		//Inner object cannot RefCount the outer object - COM rule for CircularRef
		COMPARE(Aggregate.GetRefCount(), 1);
		COMPARE(pIUnkInner == NULL, TRUE);
	}

CLEANUP:
	// Release the outer rowset object
	SAFE_RELEASE_(pIUnkOuter);
	// No rowset returned
	COMPARE(pIUnkInner, NULL);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Call GetSourcesRowset while uninitialized -- E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_13()
{
	BOOL			fResult			= TEST_FAIL;
	IDBInitialize * pIDBInitialize	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Check to see if IDBInitialize is supported
	if (!VerifyInterface(m_pISrcRow, IID_IDBInitialize, 
					ENUMERATOR_INTERFACE,	(IUnknown **)&pIDBInitialize))
	{
		odtLog<<L"IDBInitialize not supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), E_UNEXPECTED);

	// No rowset returned
	TESTC(NULL == m_pIRowset);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_14()
{
	TBEGIN
	IUnknown	*pIUnkOuter	= NULL;
	IUnknown	*pIUnkInner	= NULL;
	HRESULT		hr;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Create an outer rowset object 
	if (CHECK(hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
												0, NULL, &pIUnkOuter), S_OK))
	{
		CAggregate Aggregate(pIUnkOuter);

		// Dirty the output params
		pIUnkInner = INVALID(IUnknown*);

		// Create an inner rowset object on IID_IUnknown
		hr = m_pISrcRow->GetSourcesRowset(&Aggregate, IID_IUnknown, 0, NULL, &pIUnkInner);
		
		// Check the return code
		if (FAILED(hr))
		{
			CHECK(hr, DB_E_NOAGGREGATION);
			COMPARE(NULL == pIUnkInner, TRUE);
		}
		else
		{
			Aggregate.SetUnkInner(pIUnkInner);
			CHECK(hr, S_OK);
			COMPARE(NULL != pIUnkInner, TRUE);
			COMPARE(Aggregate.VerifyAggregationQI(hr, IID_IRowset), TRUE);
		}
	}

CLEANUP:
	// Release the outer rowset object
	SAFE_RELEASE_(pIUnkOuter);
	// Release the inner rowset object
	SAFE_RELEASE_(pIUnkInner);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc QI IParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_15()
{
	BOOL				fResult			   = TEST_FAIL;
	IParseDisplayName * pIParseDisplayName = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Should be able to QI IParseDisplayName 
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
						ENUMERATOR_INTERFACE, (IUnknown **)&pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc QI ISupportErrorInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_16()
{
	BOOL				fResult			   = TEST_FAIL;
	ISupportErrorInfo * pISupportErrorInfo = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Should be able to QI ISupportErrorInfo 
	TESTC(VerifyInterface(m_pISrcRow, IID_ISupportErrorInfo, 
							ENUMERATOR_INTERFACE, (IUnknown **)&pISupportErrorInfo));
	TESTC(NULL != pISupportErrorInfo);
	TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ISourcesRowset), S_OK, S_FALSE);
	TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ISupportErrorInfo), S_OK, S_FALSE);
	TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IParseDisplayName), S_OK, S_FALSE);
	TESTC_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IRowset), S_FALSE);
	
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_17()
{
	IUnknown *		pIUnknown = NULL;
	ISourcesRowset *pISrcRow  = NULL;
	BOOL			fResult	  = TEST_FAIL;

	// Create an enumerator Object
	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, 
				GetModInfo()->GetClassContext(), IID_IUnknown,(void **)&pIUnknown), S_OK);

	TESTC(VerifyInterface(pIUnknown, IID_ISourcesRowset,
									ENUMERATOR_INTERFACE,(IUnknown **)&pISrcRow));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pISrcRow);
	
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc pISourcesRowset->AddRef, Release -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_18()
{
	ISourcesRowset	*pISourcesRowset = NULL;
	BOOL			fResult = TEST_FAIL;

	// Create an enumerator Object
	// Read the parse name from root enumerator sources rowset to a buffer
	TESTC_(m_hr=CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, 
		GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&pISourcesRowset), S_OK);

	// Make sure we have enumerate object
	TESTC(NULL != pISourcesRowset); 

	// increment and decrement the ref counter on pISourcesRowset
	pISourcesRowset->AddRef();
	pISourcesRowset->Release();
	pISourcesRowset->AddRef();
	pISourcesRowset->AddRef();
	pISourcesRowset->Release();
	pISourcesRowset->Release();

	// check that in the end the reference counter is 0
	if (0 != pISourcesRowset->Release())
	{
		odtLog << "The ref counter is not 0; make sure this is not a bug\n";
	}
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc PropertySets 0, NULL -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_19()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
									0, NULL, (IUnknown **) &m_pIRowset), S_OK);
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc PropertySets 0, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_20()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_BOOKMARKS, DBPROPOPTIONS_REQUIRED, 0);

	// cPropertySets (0, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
						0, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc PropertySets N, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_21()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// cPropertySets (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
					m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COMMANDTIMEOUT -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_22()
{
	BOOL		fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_COMMANDTIMEOUT, DBPROPOPTIONS_OPTIONAL, 0, VT_I4);
	
	m_hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset);

	TESTC(S_OK == m_hr || DB_S_ERRORSOCCURRED == m_hr);
	if (SettableProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
	{
		TESTC_(m_hr, S_OK);
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	}
	else if (!SupportedProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
	{
		TESTC_(m_hr, DB_S_ERRORSOCCURRED);
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	}
	else
	{
		// supported but not settable
		// Make sure the property is set correctly
		if (S_OK == m_hr)
		{
			TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
		}
		else
		{
			TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
		}
	}

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Property 0, NULL -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_23()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Set property to 0 NULL in the rgProperties Structure
	SetOneProperty(NULL, DBPROPOPTIONS_OPTIONAL, 0);
	m_rgDBPropSets[0].cProperties = 0;
	m_rgDBPropSets[0].rgProperties = NULL;

	// cProperty (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Property 0, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_24()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_BOOKMARKS, DBPROPOPTIONS_REQUIRED, 0);
	m_rgDBPropSets[0].cProperties = 0;

	// cProperty (0, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Property N, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_25()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 0);
			break;
		}
	}

	// cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc one prop, DBOPTION_REQUIRED -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_26()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc create enumerator object on IParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_27()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName*	pIParseDisplayName	= NULL;
	ISourcesRowset *	pISrcRow			= NULL;

	// Create an enumerator Object
	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, GetModInfo()->GetClassContext(),
					IID_IParseDisplayName,(void **)&pIParseDisplayName), S_OK);
	
	TESTC(VerifyInterface(pIParseDisplayName, IID_ISourcesRowset,
									ENUMERATOR_INTERFACE,(IUnknown **)&pISrcRow));

	// Verify the IParseDisplayName
	TESTC(VerifyParseDisplayName_root(pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(pISrcRow);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IAccessor -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_28()
{
	BOOL		fResult		= TEST_FAIL;
	IAccessor *	pIAccessor	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IAccessor,
									0, NULL, (IUnknown **) &pIAccessor), S_OK);

	// Check column data and data in each row
	TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
							ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, pIAccessor));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIAccessor);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IColunmsInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_29()
{
	BOOL	fResult  = TEST_FAIL;
	ULONG	cCol;
	ULONG	ColCount = 7;		// For root enumerator, there is 7 columns

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IColumnsInfo,
								0, NULL, (IUnknown **) &m_pIColumnsInfo), S_OK);

	TESTC(NULL != m_pIColumnsInfo);

	TESTC_(m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
											&m_rgInfo, &m_pStringsBuffer), S_OK);

	// Check to see if the Bookmark Property is on
	if (!m_rgInfo[0].iOrdinal)
		TESTC(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColumnsInfo));

	// Skip bookmark column, check each column info
	for(cCol=1; cCol<ColCount; cCol++)
	{
		if (!memcmp(m_rgInfo[cCol].pwszName, 
						g_rgwszColumnName[cCol-1], sizeof(g_rgwszColumnName[cCol-1])))
		{
			TESTC(m_rgInfo[cCol].iOrdinal == cCol);
			TESTC(m_rgInfo[cCol].wType == g_ColumnType[cCol-1]);
		}
	}
	
	fResult = TEST_PASS;

CLEANUP:

	SAFE_FREE(m_pStringsBuffer);
	SAFE_FREE(m_rgInfo);
	SAFE_RELEASE(m_pIColumnsInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IRowsetInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_30()
{
	BOOL			fResult		 = TEST_FAIL;
	IRowsetInfo	*	pIRowsetInfo = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo,
								0, NULL, (IUnknown **) &pIRowsetInfo), S_OK);

	// Check column data and data in each row
	TESTC(VerifyInterface(pIRowsetInfo, IID_IRowset, 
						ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, pIRowsetInfo));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IUnknown -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_31()
{
	BOOL		fResult   = TEST_FAIL;
	IUnknown *	pIUnknown = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset is returned
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IUnknown,
												0, NULL, &pIUnknown), S_OK);
	// Check column data and data in each row
	TESTC(VerifyInterface(pIUnknown, IID_IRowset, 
						ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, pIUnknown));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIUnknown);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc IRowsetInfo->GetSpecification -- S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_32()
{
	BOOL			fResult			= TEST_FAIL;
	IRowsetInfo	*	pIRowsetInfo	= NULL;
	IUnknown	*	pSpecification	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo,
									0, NULL, (IUnknown **) &pIRowsetInfo), S_OK);

	TESTC(NULL != pIRowsetInfo);

	// Check column data and data in each row
	TESTC(VerifyInterface(pIRowsetInfo, IID_IRowset, 
						ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, pIRowsetInfo));
	TESTC(CheckRowsInfo());

	// The provider does not have an object that created the rowset
	// The pointer to the interface should be NULL
	TESTC_(pIRowsetInfo->GetSpecification(IID_IUnknown, (IUnknown **)&pSpecification), S_FALSE);
	TESTC(NULL == pSpecification);
	fResult = TEST_PASS;

CLEANUP:

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pSpecification);
	
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, getnextrow, release, getnextrow -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_33()
{
	BOOL		fResult		= TEST_FAIL;
	HROW		hRow[1]		= {NULL};
	HROW *		pHRow		= hRow;
	DBCOUNTITEM	cRow		= 0;
	ULONG		cRowCounter = 0;
	IRowset *	pIRowset	= NULL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	TESTC_(m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
									0, NULL,(IUnknown **) &pIRowset), S_OK);

	fResult = TEST_PASS;
	// count rows in the rowset
	while(S_OK == (m_hr=pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow)))
	{
		cRowCounter++;
		if (	!COMPARE(cRow, 1)	// check the number of returned rows
			||	!CHECK(pIRowset->ReleaseRows(cRow, hRow,NULL,NULL,NULL),S_OK))
		{
			fResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	if (	!COMPARE(DB_S_ENDOFROWSET, m_hr)
		||	!COMPARE(cRow, 0)
		||	!CHECK(pIRowset->RestartPosition(NULL), S_OK))
	{
			fResult = TEST_FAIL;
			goto CLEANUP;
	}

	while((m_hr=pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow)) == S_OK)
	{
		cRowCounter--;
		if (	!COMPARE(cRow, 1)
			||	!CHECK(pIRowset->ReleaseRows(cRow, hRow,NULL,NULL,NULL),S_OK))
		{
			fResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	if (	!COMPARE(DB_S_ENDOFROWSET, m_hr)
		||	!COMPARE(cRowCounter, 0)
		||	!COMPARE(cRow, 0))
		fResult = TEST_FAIL;
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc property was not supported -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_34()
{
	BOOL	fResult		= TEST_FAIL;
	ULONG	count		= 0;
	ULONG	cProp;
	HRESULT	hExpected	= DB_S_ERRORSOCCURRED;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((!SupportedProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}

	if (0 >= count)
		hExpected = S_OK;

	// try to call ISourcesRowset::GetSourcesRowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), hExpected);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status were set correctly
	TESTC((count < 2)  || DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	TESTC((count == 0) || DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc property was not in Rowset property group -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_35()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPOPTIONS_OPTIONAL, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status was set correctly
	if (count == 2 ) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc property set was not supported - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_36()
{
	return VerifyNonRowsetPropSet(COLUMN_COUNT_ROOT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc property was not cheap to set -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_37()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc invalid colid -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_38()
{
	return VerifyInvalidColID(DBPROPOPTIONS_OPTIONAL);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc invalid dwOptions -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_39()
{
	BOOL			fResult			= TEST_FAIL;
	HRESULT			Exphr			= DB_E_ERRORSOCCURRED;
	DBPROPSTATUS	dbPropStatus	= DBPROPSTATUS_NOTSUPPORTED;
	DBPROPOPTIONS	InvalidOptions	= -997;
	ULONG			count			= 0;
	ULONG			cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{	
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<1)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}

	// Set a settable property with dwOptions InvalidOptions
	SetOneProperty(DBPROP_IRowsetLocate, InvalidOptions, count);

	// Set the correct return code and status
	if (SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		dbPropStatus = DBPROPSTATUS_BADOPTION;
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(dbPropStatus == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc invalid vValue -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_40()
{
	BOOL		fResult = TEST_FAIL;
	ULONG		cProp	= 0;
	ULONG		count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if(SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			count++;
			// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Set next property which is settalbe
	for(++cProp; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
		{
			// Set this property with dwOptions DBPROPOPTIONS_OPTIONAL
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 1);
			count++;
			break;
		}
	}

	if (2 > count)
	{
		odtLog <<L"No second Settable Properties where supported by the Enumerator.\n";
	}

	// set a bad variant type for this property
	VariantInit(&m_rgDBPropSets[0].rgProperties[count-1].vValue);
	m_rgDBPropSets[0].rgProperties[count-1].vValue.vt		= VT_BOOL;
	m_rgDBPropSets[0].rgProperties[count-1].vValue.intVal	= 20;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status was set correctly
	TESTC(count < 2 || DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc incorrect data type -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_41()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			count++;
			// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Set next property which is settalbe
	for(++cProp; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
		{
			// Set this property with dwOptions DBPROPOPTIONS_OPTIONAL
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 1);
			count++;
			break;
		}
	}

	if (2 > count)
	{
		odtLog <<L"No second Settable Properties where supported by the Enumerator.\n";
	}

	m_rgDBPropSets[0].rgProperties[count-1].vValue.vt = VT_R4;	// none of the possible props is DBTYPE_R4
	V_R4(&m_rgDBPropSets[0].rgProperties[count-1].vValue) = (float)0.28;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status was set correctly
	TESTC(count < 2 || DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc vValue = VT_EMPTY -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_42()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_PASS;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_OPTIONAL
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 0);
	m_rgDBPropSets[0].rgProperties[0].vValue.vt = VT_EMPTY;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset twice -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_43()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open one rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Open another rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetInfo,0,NULL,(IUnknown **) &m_pIRowsetInfo), S_OK);
	fResult = TEST_PASS;;

CLEANUP:
	// Close the first rowset
	SAFE_RELEASE_(m_pIRowset);
	// Close the second rowset
	SAFE_RELEASE_(m_pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, close, open, open -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_44()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open one rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Close the rowset
	SAFE_RELEASE_(m_pIRowset);

	// Open rowset again
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Open another rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetInfo,0,NULL,(IUnknown **) &m_pIRowsetInfo), S_OK);
	fResult = TEST_PASS;

CLEANUP:
	// Close the first rowset
	SAFE_RELEASE_(m_pIRowset);
	// Close the second rowset
	SAFE_RELEASE_(m_pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc property was not applied to all columns -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_45()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc property was not supported -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_46()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((!SupportedProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	if (count < 1)
	{
		odtLog << "could not identify 2 unsupported properties\n";
		return TEST_PASS;
	}

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status were set correctly
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc property was not in Rowset property group -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_47()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_AUTH_CACHE_AUTHINFO, DBPROPOPTIONS_REQUIRED, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count == 2) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc property set was not supported -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_48()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPOPTIONS_REQUIRED, count);
	m_rgDBPropSets[0].guidPropertySet = DBPROPSET_INDEX;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count == 2) 
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc invalid colid -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_49()
{
	return VerifyInvalidColID(DBPROPOPTIONS_REQUIRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc invalid dwOptions -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_50()
{
	BOOL			fResult			= TEST_FAIL;
	DBPROPSTATUS	dbPropStatus	= DBPROPSTATUS_NOTSUPPORTED;
	DBPROPOPTIONS	InvalidOptions	= -997;
	ULONG			count			= 0;
	ULONG			cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<1)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	// Set a settable property with dwOptions InvalidOptions
	SetOneProperty(DBPROP_IRowsetLocate, InvalidOptions, count);

	// Set the correct return code and status
	if (SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		dbPropStatus = DBPROPSTATUS_BADOPTION;
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(dbPropStatus == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc invalid vValue -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_51()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
			break;
	}

	if (cProp >= PROPERTY_COUNT)
	{
		odtLog <<L"No second Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
	VariantInit(&m_rgDBPropSets[0].rgProperties[1].vValue);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = DBTYPE_DBTIMESTAMP;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);

	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc incorrect data type -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_52()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settable
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
			break;
	}

	if (count >= PROPERTY_COUNT)
	{
		odtLog <<L"No second Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = VT_R4;
	V_R4(&m_rgDBPropSets[0].rgProperties[1].vValue) = (float)3.14;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc vValue = VT_EMPTY -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_53()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}
	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
	m_rgDBPropSets[0].rgProperties[0].vValue.vt = VT_EMPTY;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName without rowset open -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_54()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName * pIParseDisplayName	= NULL;
	IMoniker *			pIMoniker			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
		ENUMERATOR_INTERFACE, (IUnknown **)&pIParseDisplayName));

	TESTC(VerifyParseDisplayName_root(pIParseDisplayName));
	fResult = TEST_PASS;
    
CLEANUP:
	SAFE_RELEASE(pIMoniker);
	SAFE_RELEASE(pIParseDisplayName);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_55()
{
	BOOL fResult = TEST_FAIL;
	IParseDisplayName* pIParseDisplayName = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), S_OK);
	
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	TESTC(VerifyParseDisplayName_root(pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);	
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_56()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName * pIParseDisplayName	= NULL;
	IMoniker *			pIMoniker			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	// Display name is NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL,
										NULL,&chEaten,&pIMoniker), MK_E_NOOBJECT);

	TESTC(NULL == pIMoniker);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_57()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName * pIParseDisplayName	= NULL;
	IMoniker *			pIMoniker			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	// pchEaten = NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL,
								g_rgwszParseName[0],NULL,&pIMoniker), MK_E_NOOBJECT);

	TESTC(NULL == pIMoniker);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_58()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName*	pIParseDisplayName	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));
	// ppIMoniker = NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL, 
								g_rgwszParseName[0],&chEaten,NULL), E_INVALIDARG);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_59()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName * pIParseDisplayName	= NULL;
	IMoniker *			pIMoniker			= NULL;
	IDBCreateCommand *	pIDBCrtCmd			= NULL;
	IUnknown *			pIUnknown			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
										0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(NULL != m_pIRowset);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
				ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));
	
	// Get moniker
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL,
									g_rgwszParseName[0],&chEaten,&pIMoniker), S_OK);
	TESTC(NULL != pIMoniker);
	TESTC(VerifyInterface(pIMoniker, IID_IUnknown, 
				UNKNOWN_INTERFACE,&pIUnknown));
	
	// Binds the moniker with the IDBCreateCommand interface
	TESTC_(BindMoniker(pIMoniker,0,IID_IDBCreateCommand,
					(LPVOID*)&pIDBCrtCmd), E_NOINTERFACE);

	// We should not have pIDBCrtCmd
	TESTC(NULL == pIDBCrtCmd);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIMoniker);
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);	
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_60()
{
	// uses it's very own m_pISrcRowset to check that 
	// calling IRowset::GetNextRows() without releasing
	// the previously retrieved row handles results in
	// DB_E_ROWSNOTRELEASED if DBPROP_CANHOLDROWSET is VARIANT_FALSE
	return GetNextRowsWithoutRelAllRows();
}
// }}

// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc check that all mutual dso & enum are in rowset -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_RootEnumerator::Variation_61()
{
	BOOL		fResult			= TEST_FAIL;
	HROW		*pHRow			= NULL;
	DBCOUNTITEM	cRows			= 0;
	IRowset		*pIRowset		= NULL;
	HACCESSOR	hAccessor		= NULL;
	DBLENGTH	cRowSize		= 0;
	DBCOUNTITEM	cBinding		= 0;
	DBBINDING	*rgBinding		= NULL;	
	LONG_PTR	rgColToBind[1];
	void		*pData1			=NULL;
	void		*pData2			=NULL;
	ULONG		cCount			= 0;
	HRESULT		hr;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	rgColToBind[0]=1;	// bind the first column, SOURCES_NAME

	TESTC_(m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
									0, NULL,(IUnknown **) &pIRowset), S_OK);

	TESTC_(GetAccessorAndBindings(
					pIRowset,
					DBACCESSOR_ROWDATA,
					&hAccessor,
					&rgBinding,
					&cBinding,
					&cRowSize,
					DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
					USE_COLS_TO_BIND_ARRAY,
					FORWARD,
					NO_COLS_BY_REF,
					NULL,
					NULL,
					NULL,
					DBTYPE_EMPTY,
					1,
					(LONG_PTR *)rgColToBind),S_OK);

	fResult = TEST_PASS;
	//prime loop with first row
	if (S_OK==(hr = pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || CHECK(hr, DB_S_ENDOFROWSET) )
	{
		//allocate memory 
		SAFE_ALLOC(pData1, BYTE, cRowSize);
		SAFE_ALLOC(pData2, BYTE, cRowSize);
		while (S_OK == hr)
		{
			//get the data of the current row
			if (	!CHECK(pIRowset->GetData(*pHRow,hAccessor,pData1),S_OK)
				||	!CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
				fResult = TEST_FAIL;
			PROVIDER_FREE(pHRow);
			//goto the next row if it is there
			if (S_OK==(hr = pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
			{
				if(cRows==0)
				{
					break;
				}
				//get the data of the new current row
				if (!CHECK(pIRowset->GetData(*pHRow,hAccessor,pData2),S_OK))
					fResult = TEST_FAIL;
				//compare SOURCES_NAME for each row, there will be double where the clsid is both an enum and a dso
				if (CompareBuffer(	pData1,		pData2, cBinding,	rgBinding,
									m_pIMalloc,	FALSE,	FALSE,		COMPARE_ONLY))
				{
					cCount++;
				}
			}
		}
		//free mem
		if (pHRow)
		{
			if (!CHECK(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
				fResult = TEST_FAIL;
			PROVIDER_FREE(pHRow);
		}
	}
	//compare the number of doubles listed in Sources Rowset with the number of doubles
	//read form the keys
	if (!COMPARE(cCount, g_cDSOANDENUM))
		fResult = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	SAFE_RELEASE(pIRowset);
	FreeAccessorBindings(cBinding,rgBinding);
	if(hAccessor && m_pISrcRow)
	{
		IAccessor * pIAccessor;
		ULONG		cRefCounts=ULONG_MAX;

		VerifyInterface(m_pISrcRow, IID_IAccessor, ENUMERATOR_INTERFACE, (IUnknown**)&pIAccessor);
		if (pIAccessor)
		{
			pIAccessor->ReleaseAccessor(hAccessor, &cRefCounts);
			SAFE_RELEASE(pIAccessor);
			COMPARE(cRefCounts,0);
		}
	}
	return fResult;
}
// }}




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Ask IID_IRowsetChange on sources rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_62()
{ 
	BOOL			fResult = TEST_FAIL;
	IRowsetChange	*pIRowsetChange = NULL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetChange,
			0, NULL, (IUnknown **) &pIRowsetChange), E_NOINTERFACE);

	TESTC(NULL == pIRowsetChange);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Ask IID_IRowsetUpdate on sources rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_63()
{ 
	BOOL			fResult = TEST_FAIL;
	IRowsetUpdate	*pIRowsetUpdate = NULL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetUpdate,
			0, NULL, (IUnknown **) &pIRowsetUpdate), E_NOINTERFACE);

	TESTC(NULL == pIRowsetUpdate);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Ask DBPROP_IRowsetChange on sources rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_64()
{ 
	BOOL			fResult			= TEST_FAIL;
	IRowsetChange	*pIRowsetChange = NULL;
	const ULONG		cPropSets		= 1;	
	DBPROPSET		rgPropSets[cPropSets];
	const ULONG		cProperties		= 1;
	DBPROP			rgProperties[cProperties];

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// set DBPROP_IRowsetChange to VARIANT_TRUE, required
	FILL_PROP_SET(rgPropSets[0], cProperties, rgProperties, DBPROPSET_ROWSET);
	FILL_PROP(rgProperties[0], DBPROP_IRowsetChange, VT_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);


	// set DBPROP_IRowsetChange to VARIANT_TRUE, optional
	FILL_PROP(rgProperties[0], DBPROP_IRowsetChange, VT_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL);
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	TESTC(NULL != m_pIRowset);

	TESTC(!VerifyInterface(m_pIRowset, IID_IRowsetChange, ROWSET_INTERFACE, 
		(IUnknown**)&pIRowsetChange));
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// set DBPROP_IRowsetChange to VARIANT_FALSE, required
	FILL_PROP_SET(rgPropSets[0], cProperties, rgProperties, DBPROPSET_ROWSET);
	FILL_PROP(rgProperties[0], DBPROP_IRowsetChange, VT_BOOL, V_BOOL, 
		VARIANT_FALSE, DBPROPOPTIONS_REQUIRED);
	
	TEST2C_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), S_OK, DB_E_ERRORSOCCURRED);
	TESTC(S_OK == m_hr || NULL == m_pIRowset);
	TESTC(S_OK == m_hr || DBPROPSTATUS_NOTSUPPORTED == rgProperties[0].dwStatus);

	// Check column data and data in each row
	TESTC(S_OK != m_hr || CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(S_OK != m_hr || CheckRowsInfo());

	// set DBPROP_IRowsetChange to VARIANT_FALSE, optional
	FILL_PROP(rgProperties[0], DBPROP_IRowsetChange, VT_BOOL, V_BOOL, 
		VARIANT_FALSE, DBPROPOPTIONS_OPTIONAL);
	
	TEST2C_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(NULL != m_pIRowset);
	TESTC(S_OK == m_hr || DBPROPSTATUS_NOTSUPPORTED == rgProperties[0].dwStatus);

	TESTC(!VerifyInterface(m_pIRowset, IID_IRowsetChange, ROWSET_INTERFACE, 
		(IUnknown**)&pIRowsetChange));
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Ask for DBPROP_IRowsetUpdate on sources rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_65()
{ 
	BOOL			fResult = TEST_FAIL;
	IRowsetUpdate	*pIRowsetUpdate = NULL;
	const ULONG		cPropSets		= 1;	
	DBPROPSET		rgPropSets[cPropSets];
	const ULONG		cProperties		= 1;
	DBPROP			rgProperties[cProperties];
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	FILL_PROP_SET(rgPropSets[0], cProperties, rgProperties, DBPROPSET_ROWSET);
	FILL_PROP(rgProperties[0], DBPROP_IRowsetUpdate, VT_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	FILL_PROP(rgProperties[0], DBPROP_IRowsetUpdate, VT_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL);
	
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			cPropSets, rgPropSets, (IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	TESTC(NULL != m_pIRowset);

	TESTC(!VerifyInterface(m_pIRowset, IID_IRowsetUpdate, ROWSET_INTERFACE, 
		(IUnknown**)&pIRowsetUpdate));

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc QI for IRowsetChange and IRowsetUpdate on a common sources rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_66()
{ 
	BOOL	fResult = TEST_FAIL;
	IRowsetChange	*pIRowsetChange = NULL;
	IRowsetUpdate	*pIRowsetUpdate = NULL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
									0, NULL, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	
	TESTC(!VerifyInterface(m_pIRowset, IID_IRowsetChange, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowsetChange));
	TESTC(!VerifyInterface(m_pIRowset, IID_IRowsetUpdate, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowsetUpdate));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName: unknown displayName (not a proper string) -- MK_E_NOOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_67()
{ 
	TBEGIN
	IParseDisplayName	*pIParseDisplayName	= NULL;
	ULONG				chEaten;
	IMoniker			*pmkOut				= NULL; //Address of output variable that receives the 
													//resulting IMoniker interface pointer

	WCHAR				pwszParseName[3000]	= L"";

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
		
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	if (0 < g_cOLEDBProvNames + g_cOLEDBProvMDPNames)
	{
		wcscpy(pwszParseName, g_rgwszParseName[0]);
		wcscat(pwszParseName, L"!new");
		m_hr = pIParseDisplayName->ParseDisplayName(NULL, pwszParseName, 
			&chEaten, &pmkOut);
		if (MK_E_NOOBJECT != m_hr)
		{
			CHECK(m_hr, MK_E_SYNTAX);
		}
	}

	TEST2C_(m_hr = pIParseDisplayName->ParseDisplayName(NULL, L"Unsignificant", 
		&chEaten, &pmkOut), MK_E_NOOBJECT, MK_E_SYNTAX);
	TESTC(MK_E_NOOBJECT != m_hr || 0 == chEaten);
	TESTC(NULL == pmkOut);

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(pmkOut);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName: not null pbc (IBindCtx) => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_68()
{ 
	TBEGIN
	IParseDisplayName	*pIParseDisplayName	= NULL;
	ULONG				chEaten;
	IBindCtx			*pbc				= NULL;
	IDBInitialize		*pIDBInitialize		= NULL;
	IMoniker			*pmkOut				= NULL; //Address of output variable that receives the 
													//resulting IMoniker interface pointer

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
		
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
							ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));
	
	TESTC_(m_hr = CreateBindCtx(NULL, &pbc), S_OK);
 
	TESTC_PROVIDER(0 < g_cOLEDBProvNames + g_cOLEDBProvMDPNames);

	TESTC_(m_hr = pIParseDisplayName->ParseDisplayName(pbc, g_rgwszParseName[0], 
		&chEaten, &pmkOut), S_OK);
	TESTC(NULL != pmkOut);

	TESTC_(m_hr = pmkOut->BindToObject(pbc, NULL, IID_IDBInitialize, 
		(void**)&pIDBInitialize), S_OK);

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(pmkOut);
	SAFE_RELEASE(pbc);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Multiple threads retrieve the sources rowset: check the number of rows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_69()
{ 
	TBEGIN
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	ULONG			nIndex;
	ULONG			cRefRowsNo	= 0;
	IRowset			*pIRowset	= NULL;

	SAFE_RELEASE(m_pIRowset);
	TESTC_(m_hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 
		0, NULL, (IUnknown**)&pIRowset), S_OK);
	TESTC_(m_hr = CountNoOfRows(pIRowset, &cRefRowsNo), S_OK);

	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgResult[nIndex]			= E_FAIL;
		m_rgRowsNo[nIndex]			= 0;
		m_rgRowset[nIndex]			= NULL;

		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		TESTC(hThread[nIndex] != 0);
	}
	
	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		CloseHandle(hThread[nIndex]);
		CHECK(m_rgResult[nIndex], S_OK);
		COMPARE(m_rgRowsNo[nIndex] == cRefRowsNo, TRUE);
		m_pIRowset = m_rgRowset[nIndex];
		TESTC(CheckRowsInfo());
	}

CLEANUP:
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		SAFE_RELEASE(m_rgRowset[nIndex]);
	}
	m_pIRowset = NULL;
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_RootEnumerator::Variation_70()
{ 
	return VerifyGetSpec_AggregatedEnum();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSourcesRowset_RootEnumerator::Terminate()
{
	// Delete enumerator object
	SAFE_RELEASE(m_pISrcRow);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSourcesRowset::Terminate());
} //}}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors_RootEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors_RootEnumerator - Extended error test for root enumerator
//|	Created:			09/18/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_RootEnumerator::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if (CSourcesRowset::Init())
	{
		m_clsid = CLSID_OLEDB_ENUMERATOR;
		
		// Create an enumerator Object
		if (CHECK(m_hr = CoCreateInstance(m_clsid, NULL, 
			GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&m_pISrcRow), S_OK))
			return TRUE;
	}
	
	return FALSE;
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_1()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
								0, NULL, (IUnknown **) &m_pIRowset), S_OK);
	
	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
		
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_2()
{
	BOOL	fResult = TEST_FAIL;
	
	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// ppSourcesRowset is NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, 0, NULL, NULL), E_INVALIDARG);
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	fResult = TEST_PASS;	

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE -- with no previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_3()
{
	BOOL			fResult = TEST_FAIL;
	IRowsetUpdate*	pIRowsetUpdate = NULL;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) &&  (NULL != m_pExtError));

	// Sources rowset is read-only 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetUpdate, 
		0,NULL, (IUnknown **)&pIRowsetUpdate), E_NOINTERFACE);
	// No rowset should be returned
	TESTC(NULL == pIRowsetUpdate);
	fResult = XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr) ? TEST_PASS: TEST_FAIL;	

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_4()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count = 0;
	ULONG	cProp;
	HRESULT	hExpected;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((!SupportedProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}
	hExpected = (0 == count)? S_OK: DB_S_ERRORSOCCURRED;
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), hExpected);

	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_ROOT, m_pIRowset));
	TESTC(CheckRowsInfo());

	// Make sure the property status were set correctly
	if (count == 2 ) 
		TESTC(m_rgDBPropSets[0].rgProperties[count-2].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
	if (count >= 1 ) 
		TESTC(m_rgDBPropSets[0].rgProperties[count-1].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED -- with no previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_5()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) || (NULL != m_pExtError));

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow, ENUMERATOR_INTERFACE))
			break;
	}

	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	if (cProp < PROPERTY_COUNT)
	{
		SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
		m_rgDBPropSets[0].rgProperties[1].vValue.vt = VT_R4;
		V_R4(&m_rgDBPropSets[0].rgProperties[1].vValue) = (float)11.23;
	}
	else
	{
		m_rgDBPropSets[0].rgProperties[0].vValue.vt = VT_R4;
		V_R4(&m_rgDBPropSets[0].rgProperties[0].vValue) = (float)11.23;
	}

	m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset);

	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	TESTC_(m_hr, DB_E_ERRORSOCCURRED);
	// Check that a Rowset was not generated
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_RootEnumerator::Variation_6()
{
	BOOL	fResult = TEST_FAIL;
	
	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// ppSourcesRowset is NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,
		IID_IRowset, 1, NULL, (IUnknown **) &m_pIRowset), E_INVALIDARG);
	TESTC(NULL == m_pIRowset);
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	fResult = TEST_PASS;	

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_RootEnumerator::Terminate()
{
	// Delete enumerator object
	SAFE_RELEASE(m_pISrcRow);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSourcesRowset::Terminate());
} //}}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSourcesRowset_ProviderEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		TCSourcesRowset_ProviderEnumerator - ISourcesRowset::GetSourcesRowset for standard enumerator
//|	Created:			09/26/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSourcesRowset_ProviderEnumerator::Init()
{
	WCHAR			*wszProviderName		= NULL;	
	WCHAR			*wszDBMSName			= NULL;	
	WCHAR			*wszDBMSNameCopy		= NULL;	
	ULONG			cPropSets				= 0;
	const ULONG		cPropIDs				= 2;
	DBPROPID		rgPropIDs[2];
	DBPROPIDSET		PropIDSet;
	DBPROPSET		*rgSupportedPropSets	= NULL;
	BOOL			fResult					= TEST_FAIL;
	ULONG			i;
	HRESULT			hr;
	BOOL			fProviderFound			= FALSE;
	DBPROPSET		*rgInitPropSets			= NULL;
	ULONG			cInitPropSets			= 0;
	IDBProperties	*pIDBProperties			= NULL;
	IDBInitialize	*pIDBInitialize			= NULL;
	WCHAR			*pwszEnumName			= NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	m_rgwszSourceParseName = NULL;
	TESTC(CSourcesRowset::Init());
	
	//Create Session Object so test can see what provider it was asked to run against
	TESTC(SUCCEEDED(hr = CoCreateInstance(m_pThisTestModule->m_ProviderClsid,NULL,
		GetModInfo()->GetClassContext(),IID_IDBInitialize,(void **)&pIDBInitialize)));
	// Get IDBProperties Pointer
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));
	// Setup the arrays needed for init, based on string LTM passed to us
	TESTC(GetInitProps(&cInitPropSets, &rgInitPropSets));
	// Set the properties before we Initialize
	TESTC(SUCCEEDED(pIDBProperties->SetProperties(cInitPropSets, rgInitPropSets)));
	// Initialize (connect so i can get the provider name)
	m_pThisTestModule->m_pError->Validate((hr = pIDBInitialize->Initialize()), 
										LONGSTRING(__FILE__), __LINE__,S_OK);

	//set prop that will Get Provider Name
	rgPropIDs[0] = DBPROP_PROVIDERNAME;
	//set prop that will Get Provider Name
	rgPropIDs[1] = DBPROP_DBMSNAME;

	PropIDSet.cPropertyIDs		= cPropIDs;
	PropIDSet.rgPropertyIDs		= rgPropIDs;
	PropIDSet.guidPropertySet	= DBPROPSET_DATASOURCEINFO;

	//get provider name
	hr = pIDBProperties->GetProperties(1, &PropIDSet, &cPropSets, &rgSupportedPropSets);
	
	if (	DB_S_ERRORSOCCURRED != hr
		&&	DB_E_ERRORSOCCURRED != hr)
		TESTC_(hr, S_OK);
	TESTC(1==cPropSets);
	//disconnect 
	TESTC_(hr = pIDBInitialize->Uninitialize(), S_OK);

	//loop through the array of source name looking for the one with the provider's name and the word enumerator
	if (GetModInfo()->GetEnumerator())
		for (i=0;i< g_cTotalEntries;i++)
		{
				//look for the provider's enumerator in key read from registry now that test knows which provider to run against
				if (!g_rgwszSourceName)
					continue;
				if	(	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), L" ENUMERATOR")==0)
					&&	(wcscmp(_wcsupr(g_rgwszSourceName[i]), _wcsupr(GetModInfo()->GetEnumerator()))==0)	
					)
				{
					// Get the CLSID from Provider String 
					fProviderFound	=	TRUE;
					CLSIDFromString(g_rgwszSourceParseName[i],&m_clsid);
					m_rgwszSourceParseName = wcsDuplicate(g_rgwszSourceParseName[i]);
					pwszEnumName = wcsDuplicate(GetModInfo()->GetEnumerator());
					break;
				}
		}

	if (!fProviderFound)
	{
		odtLog << "Enumerator name, as indicated through ENUMNAME in init string could not be found\n";
		odtLog << "Find an enumerator with similar name to the name of provider\n";

		//get wchar of provider name
		if (DBPROPSTATUS_OK == rgSupportedPropSets[0].rgProperties[0].dwStatus)
			wszProviderName = wcstok(BSTR2WSTR(rgSupportedPropSets[0].rgProperties[0].vValue.bstrVal ), L"."); // Always returns first string
		else
			wszProviderName = NULL;
		//get wchar of dbms name
		if (DBPROPSTATUS_OK == rgSupportedPropSets[0].rgProperties[0].dwStatus)
			wszDBMSName = wcstok(BSTR2WSTR(rgSupportedPropSets[0].rgProperties[1].vValue.bstrVal ), L"."); // Always returns first string
		else
			wszDBMSName = NULL;

		//hack
		if (wszDBMSName)
		{
			//Copy the pointer
			wszDBMSNameCopy = wszDBMSName;

			for (i=0;i<3;i++)
				wszDBMSName++;
		}

		//make Enum string  
		//this assumes that the enumerator name will have the provider name and the word 'Enumerator' in it's name
		for (i=0; !fProviderFound && i< g_cTotalEntries;i++)
		{
				//look for the provider's enumerator in key read from registry now that test knows which provider to run against
				if	(	
						(!(wcsstr(_wcsupr(g_rgwszSourceName[i]), L" ENUMERATOR")==0))
					&&	(	(	wszProviderName 
							&&	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), _wcsupr(wszProviderName))==0))	
						||	(wszDBMSName
							&&	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), _wcsupr(wszDBMSName))==0)))	
					)
				{
					// Get the CLSID from Provider String 
					fProviderFound	=	TRUE;
					CLSIDFromString(g_rgwszSourceParseName[i],&m_clsid);
					m_rgwszSourceParseName = wcsDuplicate(g_rgwszSourceParseName[i]);;
					pwszEnumName = wcsDuplicate(g_rgwszSourceName[i]);
					break;
				}
		}
	}
	
	if (!fProviderFound)
	{
		odtLog << L"An Enumerator was not found for this Provider. " << ENDL;
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	else
		odtLog << "Provider to be tested: " << pwszEnumName << "\n";

	// Create an enumerator Object of provider test will run against
	// Read the parse name from Provider's enumerator sources rowset to a buffer
	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, 
		GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&m_pISrcRow), S_OK);
	TESTC(ReadParseName(m_clsid));
	TESTC(MarkSupportedProperties(m_clsid));
	fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	SAFE_FREE(pwszEnumName);

	PROVIDER_FREE(wszProviderName);
	PROVIDER_FREE(wszDBMSNameCopy);

	FreeProperties(&cInitPropSets, &rgInitPropSets);
	FreeProperties(&cPropSets, &rgSupportedPropSets);
	return fResult;	
	// }}
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc QI with NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_1()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Pass a NULL pointer 
	TESTC_(m_hr=m_pISrcRow->QueryInterface(IID_IParseDisplayName,NULL), E_INVALIDARG);
	fResult = TRUE;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc PropertySets N, NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_2()
{
	BOOL fResult = TEST_FAIL;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// cPropertySets (3, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
							3, NULL, (IUnknown **) &m_pIRowset), E_INVALIDARG);
	// No rowset returned
	TESTC(NULL == m_pIRowset);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Property N, NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_3()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	m_cDBPropSets = 1;
	m_rgDBPropSets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPropSets[0].cProperties = 2;
	m_rgDBPropSets[0].rgProperties = NULL;

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// cProperty (N, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,m_cDBPropSets, 
						m_rgDBPropSets, (IUnknown **) &m_pIRowset), E_INVALIDARG);
	// No rowset returned
	TESTC(NULL == m_pIRowset);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, invalid property set -- E_INVALIDARG or E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_4()
{
	TBEGIN

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL, cPropertySets (N, NULL) 
	TEST2C_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,
		3, NULL, (IUnknown **) &m_pIRowset), E_INVALIDARG, E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ppSourcesRowset = NULL -- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_5()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// ppSourcesRowset is NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, NULL), E_INVALIDARG);
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc QI IDBInitialize -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_6()
{
	BOOL			fResult			= TEST_FAIL;
	IDBInitialize * pIDBInitialize	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Dirty the output params
	pIDBInitialize = (IDBInitialize*)0x12345678;

	
	fResult = TEST_PASS;
	// Enumerator do not require IDBInitialize verifyInterface
	if (!VerifyInterface(m_pISrcRow, IID_IDBInitialize, 
						ENUMERATOR_INTERFACE, (IUnknown **)&pIDBInitialize))
		// If not supported 
		odtLog << L"IDBInitialize is not supported by the Enumerator." << ENDL;

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc QI IDBProperties -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_7()
{
	BOOL			fResult			= TEST_FAIL;
	IDBProperties * pIDBProperties	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Dirty the output params
	pIDBProperties = (IDBProperties*)0x12345678;

	fResult = TEST_PASS;
	// Enumerator do not require IDBProperties verifyInterface
	if (!VerifyInterface(m_pISrcRow, IID_IDBProperties, 
						ENUMERATOR_INTERFACE, (IUnknown **)&pIDBProperties))
		// If not supported 
		odtLog << L"IDBProperties is not supported by the Enumerator." << ENDL;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IRowsetUpdate -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_8()
{
	BOOL			fResult			= TEST_FAIL;
	IRowsetUpdate	*pIRowsetUpdate = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	pIRowsetUpdate = (IRowsetUpdate*)0x12345678;

	// Sources rowset is read-only 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetUpdate,
			0, NULL, (IUnknown **) &pIRowsetUpdate), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == pIRowsetUpdate);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IDBCreateSession -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_9()
{
	BOOL				fResult			  = TEST_FAIL;
	IDBCreateSession*	pIDBCreateSession = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	pIDBCreateSession = (IDBCreateSession*)0x12345678;

	// For a session object interface 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IDBCreateSession,
						0, NULL, (IUnknown **) &pIDBCreateSession), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == pIDBCreateSession);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, valid ppSourcesRowset -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_10()
{
	TBEGIN

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,
			0, NULL, (IUnknown **) &m_pIRowset), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc riid = IID_NULL, valid property -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//											   
int TCSourcesRowset_ProviderEnumerator::Variation_11()
{
	TBEGIN
	ULONG	cProp;
	
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}
	}

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// IID_NULL, cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_NULL,m_cDBPropSets,
					m_rgDBPropSets, (IUnknown **) &m_pIRowset), E_NOINTERFACE);

	// No rowset returned
	TESTC(NULL == m_pIRowset);

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc pUnkOuter not NULL, IID_IRowset for pUnkInner -- E_NOINTERFACE or DB_E_NOAGGREGATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_12()
{
	return VerifyNotIUnknownInAggregation();
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Call GetSourcesRowset while uninitialized -- E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_13()
{
	BOOL			fResult			= TEST_FAIL;
	IDBInitialize * pIDBInitialize	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Check to see if IDBInitialize is supported
	if (!VerifyInterface(m_pISrcRow, IID_IDBInitialize, ENUMERATOR_INTERFACE,	(IUnknown **)&pIDBInitialize))
	{
		odtLog<<L"IDBInitialize not supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Dirty the output params
	m_pIRowset = (IRowset*)0x12345678;

	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, 0, NULL,(IUnknown **)&m_pIRowset), E_UNEXPECTED);

	// No rowset returned
	TESTC(NULL == m_pIRowset);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc pUnkOuter not NULL -- S_OK or DB_E_NOAGGREGATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_14()
{
	return VerifySourcesRowsetAggregation();
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc QI IParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_15()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName	*pIParseDisplayName	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Should be able to QI IParseDisplayName 
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
		ENUMERATOR_INTERFACE, (IUnknown **)&pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc QI ISupportErrorInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_16()
{
	BOOL				fResult				= TEST_FAIL;
	ISupportErrorInfo	*pISupportErrorInfo = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Should be able to QI ISupportErrorInfo 
	if (VerifyInterface(m_pISrcRow, IID_ISupportErrorInfo, 
					ENUMERATOR_INTERFACE, (IUnknown **)&pISupportErrorInfo))
	{
		TESTC(NULL != pISupportErrorInfo);
		TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ISourcesRowset), S_OK, S_FALSE);
		TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_ISupportErrorInfo), S_OK, S_FALSE);
		TEST2C_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IParseDisplayName), S_OK, S_FALSE);
		TESTC_(m_hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IRowset), S_FALSE);
	}
	else
	{
		TESTC(NULL == pISupportErrorInfo);
	}
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc create the enumerator object on IUnknown and QI ISourcesRowset -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_17()
{
	IUnknown *		pIUnknown = NULL;
	ISourcesRowset *pISrcRow  = NULL;
	BOOL			fResult	  = TEST_FAIL;

	// Create an enumerator Object
	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, 
		GetModInfo()->GetClassContext(), IID_IUnknown,(void **)&pIUnknown), S_OK);

	TESTC(VerifyInterface(pIUnknown, IID_ISourcesRowset, ENUMERATOR_INTERFACE, (IUnknown **)&pISrcRow));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnknown);	
	SAFE_RELEASE(pISrcRow);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc pISourcesRowset->AddRef, Release -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_18()
{

	ISourcesRowset		*pISourcesRowset	= NULL;
	IParseDisplayName	*pIParseDisplayName	= NULL;
	IMoniker			*pIMoniker			= NULL;
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;

	// Create an enumerator Object
	// Read the parse name from root enumerator sources rowset to a buffer
	TESTC_(m_hr=CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, 
		GetModInfo()->GetClassContext(), IID_IParseDisplayName,(void **)&pIParseDisplayName), S_OK);
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL, m_rgwszSourceParseName, 
		&chEaten, &pIMoniker), S_OK);
	TESTC_(BindMoniker(pIMoniker, 0, IID_ISourcesRowset, (LPVOID*)&pISourcesRowset), S_OK);

	// Make sure we have enumerate object
	TESTC(NULL != pISourcesRowset); 

	// increment and decrement the ref counter on pISourcesRowset
	SAFE_RELEASE(pIMoniker);
	pISourcesRowset->AddRef();
	pISourcesRowset->Release();
	pISourcesRowset->AddRef();
	pISourcesRowset->AddRef();
	pISourcesRowset->Release();
	pISourcesRowset->Release();

	// check that in the end the reference counter is 0
	if (0 != pISourcesRowset->Release())
	{
		odtLog << "The ref counter is not 0; make sure this is not a bug\n";
	}
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(pIMoniker);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc PropertySets 0, NULL -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_19()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc PropertySets 0, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_20()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_BOOKMARKS, DBPROPOPTIONS_REQUIRED, 0);

	// cPropertySets (0, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 
		0, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	fResult = TRUE;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc PropertySets N, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_21()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}

	// cPropertySets (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
		m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COMMANDTIMEOUT -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_22()
{
	BOOL			fResult					= TEST_FAIL;
		
	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_COMMANDTIMEOUT, DBPROPOPTIONS_OPTIONAL, 0, VT_I4);
	
	m_hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset);

	TESTC(S_OK == m_hr || DB_S_ERRORSOCCURRED == m_hr);
	if (SettableProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
	{
		TESTC_(m_hr, S_OK);
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	}
	else if (!SupportedProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
	{
		TESTC_(m_hr, DB_S_ERRORSOCCURRED);
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	}
	else
	{
		// supported but not settable => read only
		// Make sure the property is set correctly
		if (S_OK == m_hr)
		{
			TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
		}
		else
		{
			//
			TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[0].dwStatus
				||	DBPROPSTATUS_NOTSET == m_rgDBPropSets[0].rgProperties[0].dwStatus
				||	DBPROPSTATUS_NOTSETTABLE == m_rgDBPropSets[0].rgProperties[0].dwStatus
				);
		}
	}

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Property 0, NULL -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_23()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Set property to 0 NULL in the rgProperties Structure
	SetOneProperty(0, DBPROPOPTIONS_OPTIONAL, 0);
	m_rgDBPropSets[0].cProperties = 0;
	m_rgDBPropSets[0].rgProperties = NULL;

	// cProperty (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);

	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Property 0, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_24()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property
	SetOneProperty(DBPROP_BOOKMARKS, DBPROPOPTIONS_REQUIRED, 0);
	m_rgDBPropSets[0].cProperties = 0;

	// cProperty (0, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
		m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Property N, valid -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_25()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 0);
			break;
		}

	// cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
		m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc one prop, DBOPTION_REQUIRED -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_26()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Initialize the properties
	m_cDBPropSets = 0;

	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
			break;
		}

	// cProperty (N, valid) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
		m_cDBPropSets, m_rgDBPropSets, (IUnknown **) &m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property is set correctly
	if (m_cDBPropSets)
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc create enumerator object on IParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_27()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName*	pIParseDisplayName	= NULL;
	ISourcesRowset *	pISrcRow			= NULL;

	// Create an enumerator Object
	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, GetModInfo()->GetClassContext(),
		IID_IParseDisplayName,(void **)&pIParseDisplayName), S_OK);
	
	TESTC(VerifyInterface(pIParseDisplayName, IID_ISourcesRowset,
		ENUMERATOR_INTERFACE,(IUnknown **)&pISrcRow));

	// Verify the IParseDisplayName
	TESTC(VerifyParseDisplayName_std(pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);	
	SAFE_RELEASE(pISrcRow);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IAccessor -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_28()
{
	BOOL		fResult		= TEST_FAIL;
	IAccessor *	pIAccessor	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IAccessor, 0, NULL, (IUnknown **) &pIAccessor), S_OK);
	
	// Check column data and data in each row
	TESTC(VerifyInterface(pIAccessor, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, pIAccessor));
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIAccessor);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IColunmsInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_29()
{
	BOOL	fResult  = TEST_FAIL;
	ULONG	ColCount = COLUMN_COUNT_STD;		// For provider enumerator, there are 6 columns
	ULONG	cCol;
	ULONG	cOrdinal;	// the ordinal of the column compared

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IColumnsInfo, 
		0, NULL, (IUnknown **) &m_pIColumnsInfo), S_OK);

	TESTC(NULL != m_pIColumnsInfo);

	TESTC_(m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer), S_OK);

	cCol		= 0;
	cOrdinal	= 1;
	if (!m_rgInfo[0].iOrdinal)
	{
		// Check to see if the Bookmark Property is on
		TESTC(GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIColumnsInfo));
		TESTC(m_cColumns == ColCount);
		cCol = 1;
	}
	else
		TESTC(m_cColumns == ColCount-1);

	// if there is a bookmark column then Skip the bookmark column
	for(; cCol < m_cColumns; cCol++, cOrdinal++)
	{
		if (!memcmp(m_rgInfo[cCol].pwszName, 
					g_rgwszColumnName[cOrdinal-1], sizeof(g_rgwszColumnName[cOrdinal-1])))
		{
			TESTC(m_rgInfo[cCol].iOrdinal == cOrdinal);
			TESTC(m_rgInfo[cCol].wType == g_ColumnType[cOrdinal-1]);
		}
	}
	fResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(m_pStringsBuffer);
	SAFE_FREE(m_rgInfo);
	SAFE_RELEASE(m_pIColumnsInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IRowsetInfo -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_30()
{
	BOOL			fResult		 = TEST_FAIL;
	IRowsetInfo	*	pIRowsetInfo = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo,
			0, NULL, (IUnknown **) &pIRowsetInfo), S_OK);
	
	// Check column data and data in each row
	TESTC(VerifyInterface(pIRowsetInfo, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, pIRowsetInfo));
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset on IUnknown -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_31()
{
	BOOL		fResult   = TEST_FAIL;
	IUnknown *	pIUnknown = NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset is returned
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IUnknown, 0, NULL, &pIUnknown), S_OK);
	
	// Check column data and data in each row
	TESTC(VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, pIUnknown));
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIUnknown);	
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc IRowsetInfo->GetSpecification -- S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_32()
{
	BOOL			fResult			= TEST_FAIL;
	IRowsetInfo	*	pIRowsetInfo	= NULL;
	IUnknown	*	pSpecification	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Sources rowset returned 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowsetInfo, 
			0, NULL, (IUnknown **) &pIRowsetInfo), S_OK);

	TESTC(NULL != pIRowsetInfo);

	// Check column data and data in each row
	TESTC(VerifyInterface(pIRowsetInfo, IID_IRowset, ROWSET_INTERFACE,(IUnknown **)&m_pIRowset));
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, pIRowsetInfo));

	// The provider does not have an object that created the rowset
	// The pointer to the interface should be NULL
	TESTC_(pIRowsetInfo->GetSpecification(IID_IUnknown, (IUnknown **)&pSpecification), S_FALSE);
	TESTC(NULL == pSpecification);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pSpecification);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, getnextrow, release, getnextrow -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_33()
{
	BOOL		fResult		= TEST_FAIL;
	HROW		hRow[1]		= {NULL};
	HROW		*pHRow		= hRow;
	DBCOUNTITEM	cRow		= 0;
	ULONG		cRowCounter = 0;
	IRowset		*pIRowset	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	TESTC_(m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
				0, NULL,(IUnknown **) &pIRowset), S_OK);

	fResult = TEST_PASS;
	while((m_hr=pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow)) == S_OK)
	{
		cRowCounter++;
		if (	!COMPARE(cRow, 1)
			||	!CHECK(pIRowset->ReleaseRows(cRow, hRow,NULL,NULL,NULL),S_OK))
		{
			fResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	if (	!CHECK(m_hr, DB_S_ENDOFROWSET)
		||	!COMPARE(cRow, 0)
		||	!CHECK(pIRowset->RestartPosition(NULL), S_OK))
	{
		fResult = TEST_FAIL;
		goto CLEANUP;
	}

	while((m_hr=pIRowset->GetNextRows(NULL,0,1,&cRow,&pHRow)) == S_OK)
	{
		cRowCounter--;
		if (	!COMPARE(cRow, 1)
			||	!CHECK(pIRowset->ReleaseRows(cRow, hRow,NULL,NULL,NULL),S_OK))
		{
			fResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	if (	!CHECK(m_hr, DB_S_ENDOFROWSET)
		||	!COMPARE (cRowCounter, 0)
		||	!COMPARE(cRow, 0))
		fResult	= TEST_FAIL;
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc property was not supported -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_34()
{
	BOOL			fResult			= TEST_FAIL;
	ULONG			count			= 0;
	IDBProperties*  pIDBProperties	= NULL;
	ULONG			cProp			= 0;
	HRESULT			hExpected		= DB_S_ERRORSOCCURRED;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((!SupportedProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}

	if (0 == count)
		hExpected = S_OK;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), hExpected);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	
	// Make sure the property status were set correctly
	if (count == 2 ) 
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count >= 1 ) 
		TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:	
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc property was not in Rowset property group -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_35()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPOPTIONS_OPTIONAL, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property status was set correctly
	if (count == 2 ) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc property set was not supported - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_36()
{
	return VerifyNonRowsetPropSet(COLUMN_COUNT_STD);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc property was not cheap to set -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_37()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc invalid colid -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_38()
{
	return VerifyInvalidColID(DBPROPOPTIONS_OPTIONAL);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc invalid dwOptions -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_39()
{
	BOOL			fResult			= TEST_FAIL;
	DBPROPSTATUS	dbPropStatus	= DBPROPSTATUS_NOTSUPPORTED;
	DBPROPOPTIONS	InvalidOptions	= 99;
	ULONG			count			= 0;
	IDBProperties	*pIDBProperties	= NULL;
	ULONG			cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one unsupported property
	//supported propert assumes IDBProperties is available for the INTERFACE requested
	//this is an optional interface of the SourcesRowset enumerator
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<1)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
			count++;
		}
	}
	// Set a settable property with dwOptions InvalidOptions
	SetOneProperty(DBPROP_IRowsetLocate, InvalidOptions, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	
	// Set the correct return code and status
	if (SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		dbPropStatus = DBPROPSTATUS_BADOPTION;

	// Make sure the property status was set correctly
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(dbPropStatus == m_rgDBPropSets[0].rgProperties[count].dwStatus);

	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc invalid vValue -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_40()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
		{
			// Set this property with dwOptions DBPROPOPTIONS_OPTIONAL
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 1);
			count++;
			break;
		}

	// it's sure that the prop is of type bool
	m_rgDBPropSets[0].rgProperties[count-1].vValue.vt = VT_BOOL;
	m_rgDBPropSets[0].rgProperties[count-1].vValue.intVal = 12; // some other value
	m_rgDBPropSets[0].rgProperties[count-1].dwOptions = DBPROPOPTIONS_OPTIONAL; // some other value
	// than VARIANT_TRUE and VARIANT_FALSE

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	
	// Make sure the property status was set correctly
	TESTC(count < 2 || DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc incorrect data type -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_41()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
		{
			count++;
			break;
		}
	}
	if (2 > count)
	{
		odtLog <<L"No seccond Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Set this property with dwOptions DBPROPOPTIONS_OPTIONAL
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 1);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = VT_R4;
	V_R4(&m_rgDBPropSets[0].rgProperties[1].vValue) = (float)26.6;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc vValue = VT_EMPTY -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_42()
{
	BOOL	fResult = TEST_FAIL;

	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_OPTIONAL
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, 0);
	m_rgDBPropSets[0].rgProperties[0].vValue.vt = VT_EMPTY;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;

CLEANUP:	
	SAFE_RELEASE(m_pIRowset);

	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset twice -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_43()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open one rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Open another rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetInfo,0,NULL,(IUnknown **) &m_pIRowsetInfo), S_OK);
	fResult = TRUE;

CLEANUP:
	// Close the first rowset
	SAFE_RELEASE_(m_pIRowset);
	// Close the second rowset
	SAFE_RELEASE_(m_pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, close, open, open -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_44()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	m_pIRowset		= NULL;
	m_pIRowsetInfo	= NULL;

	// Open one rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Close the rowset
	SAFE_RELEASE_(m_pIRowset);

	// Open rowset again
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **) &m_pIRowset), S_OK);
	
	// Open another rowset
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetInfo,0,NULL,(IUnknown **) &m_pIRowsetInfo), S_OK);
	fResult = TEST_PASS;

CLEANUP:
	// Close the first rowset
	SAFE_RELEASE_(m_pIRowset);
	// Close the second rowset
	SAFE_RELEASE_(m_pIRowsetInfo);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc property was not applied to all columns -- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_45()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc property was not supported -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_46()
{
	BOOL			fResult			= TEST_FAIL;
	ULONG			count			= 0;
	ULONG			cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	//supported propert assumes IDBProperties is available for the INTERFACE requested
	//this is an optional interface of the SourcesRowset enumerator
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((!SupportedProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}
	if (0 == count)
	{
		odtLog << "Could not find a not supported property\n";
		return TEST_SKIPPED;
	}

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	
	TESTC(NULL == m_pIRowset);

	// Make sure the property status were set correctly
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(count == 0 || DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc property was not in Rowset property group -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_47()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_AUTH_CACHE_AUTHINFO, DBPROPOPTIONS_REQUIRED, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count == 2) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc property set was not supported -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_48()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	count	= 0;
	ULONG	cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two supported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<2)) {
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
		}
	}

	// Set the property not in the Rowset property group
	SetOneProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPOPTIONS_REQUIRED, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	if (count == 2) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-2].dwStatus);
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(DBPROPSTATUS_NOTSUPPORTED == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc invalid colid -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_49()
{
	return VerifyInvalidColID(DBPROPOPTIONS_REQUIRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc invalid dwOptions -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_50()
{
	BOOL			fResult			= TEST_FAIL;
	DBPROPSTATUS	dbPropStatus	= DBPROPSTATUS_NOTSUPPORTED;
	DBPROPOPTIONS	InvalidOptions	= 99;
	ULONG			count			= 0;
	IDBProperties	*pIDBProperties	= NULL;
	ULONG			cProp;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set two unsupported properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if ((SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) &&
			(count<1)) 
		{
			SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, count);
			count++;
			break;
		}
	}

	// Set a settable property with dwOptions InvalidOptions
	SetOneProperty(DBPROP_IRowsetLocate, InvalidOptions, count);

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	// No rowset should be returned
	TESTC(NULL == m_pIRowset);

	// Set the correct return code and status
	if (SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		dbPropStatus = DBPROPSTATUS_BADOPTION;

	// Make sure the property status was set correctly
	if (count > 0) 
		TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[count-1].dwStatus);
	TESTC(dbPropStatus == m_rgDBPropSets[0].rgProperties[count].dwStatus);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc invalid vValue -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_51()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
			break;
	}
	if (cProp >= PROPERTY_COUNT)
	{
		odtLog << L"No seccond Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}

	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
	VariantInit(&m_rgDBPropSets[0].rgProperties[1].vValue);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = DBTYPE_DBTIMESTAMP;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(NULL == m_pIRowset);

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);

	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);

	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc incorrect data type -- DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_52()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) {
			count++;
			break;
		}
	}
	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
			break;
	}
	if (cProp >= PROPERTY_COUNT)
	{
		odtLog <<L"No second Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = VT_R4;
	V_R4(&m_rgDBPropSets[0].rgProperties[1].vValue) = (float)2.14;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	TESTC( NULL == m_pIRowset);

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	TESTC(DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[1].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc vValue = VT_EMPTY -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_53()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			count++;
			break;
		}
	}
	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);
	m_rgDBPropSets[0].rgProperties[0].vValue.vt = VT_EMPTY;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
		m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), S_OK);
	
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property status was set correctly
	TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[0].dwStatus);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName without rowset open -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_54()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName	*pIParseDisplayName	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
		ENUMERATOR_INTERFACE, (IUnknown **)&pIParseDisplayName));
	
	TESTC(VerifyParseDisplayName_std(pIParseDisplayName));
	fResult = TEST_PASS;
    
CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName -- S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_55()
{
	BOOL				fResult				= TEST_FAIL;
	IParseDisplayName	*pIParseDisplayName	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, 0,NULL,(IUnknown **)&m_pIRowset), S_OK);
	
	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
			ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	TESTC(VerifyParseDisplayName_std(pIParseDisplayName));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName: displayName = NULL -- MK_E_NOOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_56()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName	*pIParseDisplayName	= NULL;
	IMoniker			*pIMoniker			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset, 0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
		ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	// Display name is NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL, NULL, &chEaten, &pIMoniker), MK_E_NOOBJECT);	

	TESTC(NULL == pIMoniker);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName:pchEaten=NULL -- MK_E_NOOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_57()
{
	BOOL				fResult					= TEST_FAIL;
	IParseDisplayName	* pIParseDisplayName	= NULL;
	IMoniker			*pIMoniker				= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);

	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
		ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));

	// pchEaten = NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL,g_rgwszParseName[0],NULL,&pIMoniker), MK_E_NOOBJECT);
	TESTC(NULL == pIMoniker);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc ParseDisplayName:pIMoniker = NULL -- E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_58()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName	*pIParseDisplayName	= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName, 
		ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));
	
	// ppIMoniker = NULL
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL, 
		g_rgwszParseName[0],&chEaten,NULL), E_UNEXPECTED);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc IParseDisplayName::ParseDisplayName, bind Moniker to session object -- E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSourcesRowset_ProviderEnumerator::Variation_59()
{
	BOOL				fResult				= TEST_FAIL;
	ULONG				chEaten				= 0;
	IParseDisplayName * pIParseDisplayName	= NULL;
	IMoniker *			pIMoniker			= NULL;
	IDBCreateCommand *	pIDBCrtCmd			= NULL;
	IUnknown *			pIUnknown			= NULL;

	// Make sure we have ISrcRow pointer
	TESTC(NULL != m_pISrcRow);
	
	// Open sources rowset 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,0,NULL,(IUnknown **)&m_pIRowset), S_OK);

	TESTC(NULL != m_pIRowset);

	TESTC(VerifyInterface(m_pISrcRow, IID_IParseDisplayName,
		ENUMERATOR_INTERFACE,(IUnknown **)&pIParseDisplayName));
	
	// Get moniker
	TESTC_(m_hr=pIParseDisplayName->ParseDisplayName(NULL,g_rgwszParseName[0],&chEaten,&pIMoniker), S_OK);
	
	TESTC(NULL != pIMoniker);
	
	TESTC(VerifyInterface(pIMoniker, IID_IUnknown, UNKNOWN_INTERFACE,&pIUnknown));
	
	// Binds the moniker with the IDBCreateCommand interface
	TESTC_(BindMoniker(pIMoniker,0,IID_IDBCreateCommand,(LPVOID*)&pIDBCrtCmd), E_NOINTERFACE);

	// We should not have pIDBCrtCmd
	TESTC(NULL == pIDBCrtCmd);
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIUnknown);	
	SAFE_RELEASE(pIMoniker);
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc open sources rowset, getnextrow, not release all rows, getnextrow -- DB_E_ROWSNOTRELEASED
//
// @rdesc TEST_PASS or TEST_FAIL
//

int TCSourcesRowset_ProviderEnumerator::Variation_60()
{
	// uses it's very own m_pISrcRowset to check that 
	// calling IRowset::GetNextRows() without releasing
	// the previously retrieved row handles results in
	// DB_E_ROWSNOTRELEASED if DBPROP_CANHOLDROWSET is VARIANT_FALSE
	return GetNextRowsWithoutRelAllRows();
}
// }}




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Multiple threads retrieve the sources rowset: check the number of rows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_ProviderEnumerator::Variation_61()
{ 
	TBEGIN
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	ULONG			nIndex, i;
	ULONG			cRefRowsNo	= 0;
	IRowset			*pIRowset	= NULL;

	TESTC_(m_hr = m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 
		0, NULL, (IUnknown**)&pIRowset), S_OK);
	TESTC_(m_hr = CountNoOfRows(pIRowset, &cRefRowsNo), S_OK);

	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgResult[nIndex]			= E_FAIL;
		m_rgRowsNo[nIndex]			= 0;
		m_rgRowset[nIndex]			= NULL;
		m_rgMaxSourcesNames[nIndex]	= 100; // get data for the first 100 rows
		SAFE_ALLOC(m_rgSourcesNames[nIndex], WCHAR*, m_rgMaxSourcesNames[nIndex]);
		memset(m_rgSourcesNames[nIndex], 0, sizeof(m_rgSourcesNames[nIndex]));


		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc2,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		TESTC(hThread[nIndex] != 0);
	}
	
	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		CloseHandle(hThread[nIndex]);
		CHECK(m_rgResult[nIndex], S_OK);
		if (!COMPARE((1-0.25)*cRefRowsNo<=m_rgRowsNo[nIndex] && m_rgRowsNo[nIndex]<=(1+0.25)*cRefRowsNo, TRUE))
		{
			odtLog << "Thread " << nIndex << " retrieved " << m_rgRowsNo[nIndex] << " rows; ";
			odtLog << "expected: " << (1-0.25)*cRefRowsNo << "<=RowsCount<=" << (1+0.25)*cRefRowsNo << "\n";
		}

		if (m_rgResult[nIndex]==S_OK && 
			!VerifyNoDuplicates(m_rgSourcesNames[nIndex], m_rgRowsNo[nIndex]<m_rgMaxSourcesNames[nIndex] ? m_rgRowsNo[nIndex] : m_rgMaxSourcesNames[nIndex]))
		{
			odtLog << "Thread " << nIndex << ": GetSourcesRowset returned duplicate sources\n";
			// Dump sources names returned by GetSourcesRowset
			//	for(i=0; i< m_rgMaxSourcesNames[nIndex] && i< m_rgRowsNo[nIndex]; i++)
			//		odtLog << L"\t\tRow " << i << L": " << (m_rgSourcesNames[nIndex][i]? m_rgSourcesNames[nIndex][i] : L"NULL") <<  L"\n";

			// Increment failure count 
			COMPARE(TRUE, FALSE);
		}

	}

CLEANUP:
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		SAFE_RELEASE(m_rgRowset[nIndex]);
		for(i=0; i< m_rgMaxSourcesNames[nIndex] && i< m_rgRowsNo[nIndex]; i++)
		{
			SAFE_FREE(m_rgSourcesNames[nIndex][i]);
		}
		SAFE_FREE(m_rgSourcesNames[nIndex]);
	}
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Aggregate The enumerator, get the sources rowset and call IRowsetInfo::GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSourcesRowset_ProviderEnumerator::Variation_62()
{
	return VerifyGetSpec_AggregatedEnum();
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSourcesRowset_ProviderEnumerator::Terminate()
{
	// Delete enumerator object
	SAFE_RELEASE(m_pISrcRow);
	SAFE_FREE(m_rgwszSourceParseName);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSourcesRowset::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors_ProviderEnumerator)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors_ProviderEnumerator - Extended error test for standard enumerator
//|	Created:			09/26/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_ProviderEnumerator::Init()
{
	WCHAR			*wszProviderName		= NULL;	
	WCHAR			*wszDBMSName			= NULL;	
	WCHAR			*wszDBMSNameCopy		= NULL;	
	ULONG			cPropSets				= 0;
	const ULONG		cPropIDs				= 2;
	DBPROPID		rgPropIDs[2];
	DBPROPIDSET		PropIDSet;
	DBPROPSET		*rgSupportedPropSets	= NULL;
	BOOL			fResult					= TEST_FAIL;
	ULONG			i;
	HRESULT			hr;
	BOOL			fProviderFound			= TEST_FAIL;
	DBPROPSET		*rgInitPropSets			= NULL;
	ULONG			cInitPropSets			= 0;
	IDBProperties	*pIDBProperties			= NULL;
	IDBInitialize	*pIDBInitialize			= NULL;
	WCHAR			*pwszEnumName			= NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(CSourcesRowset::Init());

	//Create Session Object so test can see what provider it was asked to run against
	TESTC(SUCCEEDED(hr = CoCreateInstance(m_pThisTestModule->m_ProviderClsid,NULL,
		GetModInfo()->GetClassContext(),
		IID_IDBInitialize,(void **)&pIDBInitialize)));

	// Get IDBProperties Pointer
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));
	fResult = TEST_FAIL;
	// Setup the arrays needed for init, based on string LTM passed to us
	TESTC(GetInitProps(&cInitPropSets, &rgInitPropSets));
	// Set the properties before we Initialize
	TESTC(SUCCEEDED(pIDBProperties->SetProperties(cInitPropSets, rgInitPropSets)));
	// Initialize (connect so i can get the provider name)
	m_pThisTestModule->m_pError->Validate((hr = pIDBInitialize->Initialize()), 
										LONGSTRING(__FILE__), __LINE__,S_OK);

	//set prop that will Get Provider Name
	rgPropIDs[0] = DBPROP_PROVIDERNAME;
	//set prop that will Get Provider Name
	rgPropIDs[1] = DBPROP_DBMSNAME;

	PropIDSet.cPropertyIDs		= cPropIDs;
	PropIDSet.rgPropertyIDs		= rgPropIDs;
	PropIDSet.guidPropertySet	= DBPROPSET_DATASOURCEINFO;

	//get provider name
	hr = pIDBProperties->GetProperties(1, &PropIDSet, &cPropSets, &rgSupportedPropSets);
	
	if (	DB_S_ERRORSOCCURRED != hr
		&&	DB_E_ERRORSOCCURRED != hr)
		TESTC_(hr, S_OK);
	TESTC(1 == cPropSets);
	//disconnect 
	TESTC_(hr = pIDBInitialize->Uninitialize(), S_OK);

	//loop through the array of source name looking for the one with the provider's name and the word enumerator
	if (GetModInfo()->GetEnumerator())
		for (i=0;i< g_cTotalEntries;i++)
		{
				//look for the provider's enumerator in key read from registry now that test knows which provider to run against
				if (!g_rgwszSourceName)
					continue;
				if	(	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), L" ENUMERATOR")==0)
					&&	(wcscmp(_wcsupr(g_rgwszSourceName[i]), _wcsupr(GetModInfo()->GetEnumerator()))==0)	
					)
				{
					// Get the CLSID from Provider String 
					fProviderFound	=	TRUE;
					CLSIDFromString(g_rgwszSourceParseName[i],&m_clsid);
					pwszEnumName = wcsDuplicate(GetModInfo()->GetEnumerator());
					break;
				}
		}

	if (!fProviderFound)
	{
		odtLog << "Enumerator name, as indicated through ENUMNAME in init string could not be found\n";
		odtLog << "Find an enumerator with similar name to the name of provider\n";

		//get wchar of provider name
		if (DBPROPSTATUS_OK == rgSupportedPropSets[0].rgProperties[0].dwStatus)
			wszProviderName = wcstok(BSTR2WSTR(rgSupportedPropSets[0].rgProperties[0].vValue.bstrVal ), L"."); // Always returns first string
		else
			wszProviderName = NULL;
		//get wchar of dbms name
		if (DBPROPSTATUS_OK == rgSupportedPropSets[0].rgProperties[0].dwStatus)
			wszDBMSName = wcstok(BSTR2WSTR(rgSupportedPropSets[0].rgProperties[1].vValue.bstrVal ), L"."); // Always returns first string
		else
			wszDBMSName = NULL;
		
		//hack
		if (wszDBMSName)
		{
			//Copy the pointer
			wszDBMSNameCopy = wszDBMSName;

			for (i=0;i<3;i++)
				wszDBMSName++;
		}

		//make Enum string  
		//this assumes that the enumerator name will have the provider name and the word 'Enumerator' in its name

		//loop through the array of source name looking for the one with the provider's name and the word enumerator
		for (i=0;i< g_cTotalEntries;i++)
		{
				//look for the provider's enumerator in key read from registry now that test knows which provider to run against
				if	(	
						(!(wcsstr(_wcsupr(g_rgwszSourceName[i]), L" ENUMERATOR")==0))
					&&	(	(	wszProviderName 
							&&	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), _wcsupr(wszProviderName))==0))	
						||	(wszDBMSName
							&&	!(wcsstr(_wcsupr(g_rgwszSourceName[i]), _wcsupr(wszDBMSName))==0)))	
					)
				{
					// Get the CLSID from Provider String 
					fProviderFound	=	TRUE;
					CLSIDFromString(g_rgwszSourceParseName[i],&m_clsid);
					pwszEnumName = wcsDuplicate(g_rgwszSourceName[i]);
					break;
				}
		}
	}
	
	if (!fProviderFound)
	{
		odtLog << L"An Enumerator was not found for this Provider. " << ENDL;
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	else
		odtLog << "Provider to be tested: " << pwszEnumName << "\n";
	
	// Create an enumerator Object

	TESTC_(m_hr = CoCreateInstance(m_clsid, NULL, 
		GetModInfo()->GetClassContext(), IID_ISourcesRowset,(void **)&m_pISrcRow), S_OK);
	TESTC(MarkSupportedProperties(m_clsid));
	fResult = TRUE;
	
CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	SAFE_FREE(pwszEnumName);

	PROVIDER_FREE(wszProviderName);
	PROVIDER_FREE(wszDBMSNameCopy);

	FreeProperties(&cInitPropSets, &rgInitPropSets);
	FreeProperties(&cPropSets, &rgSupportedPropSets);
	return fResult;	
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_1()
{
	BOOL fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// cPropertySets (0, NULL) 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset,
			0, NULL, (IUnknown **) &m_pIRowset), S_OK);

	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));	
		
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_2()
{
	BOOL	fResult = TEST_FAIL;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// ppSourcesRowset is NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, NULL), E_INVALIDARG);
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	fResult = TEST_PASS;

CLEANUP:
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE -- with no previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_3()
{
	BOOL			fResult = TEST_FAIL;
	IRowsetUpdate*	pIRowsetUpdate = NULL;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));

	// Sources rowset is read-only 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowsetUpdate,
				0,NULL, (IUnknown **)&pIRowsetUpdate), E_NOINTERFACE);
	// No rowset should be returned
	TESTC(NULL == pIRowsetUpdate);
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_4()
{
	BOOL			fResult			= TEST_FAIL;
	ULONG			count			= 0;
	IDBProperties	*pIDBProperties	= NULL;
	ULONG			cProp			= 0;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));

	// Set settable properties 
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
		{
				SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_OPTIONAL, count);
				count++;
		}
	}
	if (0 == count)
		return TEST_SKIPPED;

	// Cause an error on the current thread
	m_pExtError->CauseError();
	count = count > 2 ? count - 2: 0;
	m_rgDBPropSets[0].rgProperties[count].vValue.vt = VT_NULL;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_S_ERRORSOCCURRED);

	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));	
		
	// Check column data and data in each row
	TESTC(CheckColumnsInfo(COLUMN_COUNT_STD, m_pIRowset));

	// Make sure the property status were set correctly
	for (cProp=0; cProp<m_rgDBPropSets[0].cProperties; cProp++) 
	{
		if (count == cProp ) 
		{
			TESTC(	DBPROPSTATUS_BADVALUE == m_rgDBPropSets[0].rgProperties[cProp].dwStatus
				||	DBPROPSTATUS_NOTSET == m_rgDBPropSets[0].rgProperties[cProp].dwStatus);
		}
		else
		{
			TESTC(DBPROPSTATUS_OK == m_rgDBPropSets[0].rgProperties[cProp].dwStatus);
		}
	}
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED -- with no previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_5()
{
	BOOL	fResult = TEST_FAIL;
	ULONG	cProp	= 0;
	ULONG	count	= 0;

	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));

	// Set one property which is settalbe
	for(cProp=0; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE)) 
		{
			count++;
			break;
		}
	}

	// if count is 0 no settable properties
	if (!count)
	{
		odtLog <<L"No Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	// Set a settable property with dwOptions DBPROPOPTIONS_REQUIRED
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 0);

	// Set next property which is settalbe
	for(cProp++; cProp<PROPERTY_COUNT; cProp++)
	{
		if (SettableProperty(g_PropMark[cProp].dwPropertyID, DBPROPSET_ROWSET, m_pISrcRow,ENUMERATOR_INTERFACE))
			break;
	}

	// Set this property with dwOptions DBPROPOPTIONS_REQUIRED
	if (cProp >= PROPERTY_COUNT)
	{
		odtLog << L"No second Settable Properties where supported by the Enumerator.\n";
		return TEST_SKIPPED;
	}
	SetOneProperty(g_PropMark[cProp].dwPropertyID, DBPROPOPTIONS_REQUIRED, 1);
	m_rgDBPropSets[0].rgProperties[1].vValue.vt = VT_I4;

	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL,IID_IRowset,
			m_cDBPropSets,m_rgDBPropSets,(IUnknown **)&m_pIRowset), DB_E_ERRORSOCCURRED);
	
	// Check that a Rowset was not generated
	TESTC(NULL == m_pIRowset);

	// Do extended check
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));

	// Make sure the property status was set correctly
	TESTC(m_rgDBPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_OK);
	TESTC(m_rgDBPropSets[0].rgProperties[1].dwStatus == DBPROPSTATUS_BADVALUE);
	fResult = TEST_PASS;
	
CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG -- with previous error existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors_ProviderEnumerator::Variation_6()
{
	BOOL	fResult = TEST_FAIL;
	
	// Make sure we have ISrcRow pointer
	TESTC((NULL != m_pISrcRow) && (NULL != m_pExtError));
	
	// Cause an error on the current thread
	m_pExtError->CauseError();

	// ppSourcesRowset is NULL 
	TESTC_(m_hr=m_pISrcRow->GetSourcesRowset(NULL, IID_IRowset, 1, NULL, 
		(IUnknown **) &m_pIRowset), E_INVALIDARG);
	TESTC(NULL == m_pIRowset);
	TESTC(XCHECK(m_pISrcRow, IID_ISourcesRowset, m_hr));
	fResult = TEST_PASS;

CLEANUP:
	SAFE_RELEASE(m_pIRowset);
	return fResult;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors_ProviderEnumerator::Terminate()
{
	// Delete enumerator object
	SAFE_RELEASE(m_pISrcRow);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CSourcesRowset::Terminate());
}	// }}
// }}
// }}


// Hack to defile CoInitializeEx
WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

unsigned WINAPI ThreadProc(LPVOID lpvThreadParam)
{
	CoInitializeEx(NULL, 0);
	CSourcesRowset	*pObject = ((CInParam*)lpvThreadParam)->pObject;
	pObject->MyThreadProc(((CInParam*)lpvThreadParam)->i);
	CoUninitialize();
	return 0;
} //ThreadProc
unsigned WINAPI ThreadProc2(LPVOID lpvThreadParam)
{
	CoInitializeEx(NULL, 0);
	CSourcesRowset	*pObject = ((CInParam*)lpvThreadParam)->pObject;
	pObject->MyThreadProc2(((CInParam*)lpvThreadParam)->i);
	CoUninitialize();
	return 0;
} //ThreadProc

bool VerifyNoDuplicates(WCHAR** rgNames, ULONG cNames)
{

	bool fDuplicates = false;
	
	for(ULONG i=0; i<cNames-1 && !fDuplicates; i++)
	{
		for(ULONG j=i+1; rgNames[i] && j< cNames && !fDuplicates; j++)
		{
			if(fDuplicates=(rgNames[j] && wcscmp(rgNames[i], rgNames[j])==0))
				odtLog << "Found duplicate for: " << rgNames[i] << "\n";
		}
	}

	return !fDuplicates;
}

//*-----------------------------------------------------------------------
//
//   WARNING:
//          PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//
//   Copyright (C) 1994-2000 Microsoft Corporation
//*-----------------------------------------------------------------------


#include "MODStandard.hpp"
#include "RootBinder.h"		// RootBinder testmodule header
#include "ExtraLib.h"


//*-----------------------------------------------------------------------
// Module Values
//*-----------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xdbbf95f0, 0xe258, 0x11d2, { 0x89, 0x0f, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("RootBinder");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Root Binder Test");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


//----------------------------------------------------------------
//Verify that if Wininet.dll is missing, the Root Binder will 
//fail gracefully (E_FAIL).
BOOL TestNoWininet()
{
	BOOL				bRet = FALSE;
	CHAR				szBuf[115];
	CHAR				szNewName[] = "c:\\wininet.dll";
	ULONG				ulLen = 0;
	ULONG				ulRename = 0;
	CLSID				clsidTemp;
	CLSID				clsidProv = DBGUID_DSO;
	WCHAR*				pwszURL = NULL;
	IDBProperties*		pIProp = NULL;
	IBindResource*		pIBR = NULL;
	IRegisterProvider*	pIRP = NULL;
	
	//The system directory is where wininet.dll resides.
	ulLen = GetSystemDirectory(szBuf, 115);

	//Enforce that the system directory name be of a length less
	//than 100, since we need to leave some space for adding
	//"\wininet.dll" at the end of the string.
	TESTC(ulLen>0  && ulLen<100); 
	strcat(szBuf, "\\wininet.dll");

	//Move winnet.dll away from the system directory.
	ulRename = rename(szBuf, szNewName);

	//If the moving fails due to some reason, fail the test.
	if(ulRename)
		goto CLEANUP;  

	pIBR = GetModInfo()->GetRootBinder();

	TESTC(VerifyInterface(pIBR, IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&pIRP))

	CHECKW(pIRP->GetURLMapping(L"http", 0, &clsidTemp), E_FAIL);
	CHECKW(pIRP->SetURLMapping(L"X-IRPabcd", 0, clsidProv), E_FAIL);
	CHECKW(pIRP->UnregisterProvider(L"X-IRPabcd", 0, clsidProv), E_FAIL);
	CHECKW(pIRP->UnregisterProvider(NULL, 0, clsidProv), E_FAIL);

	pwszURL = GetModInfo()->GetRootURL();
	if(!pwszURL)
		pwszURL = GetModInfo()->GetParseObject()->GetURL(ROWSET_INTERFACE);

	CHECKW(pIBR->Bind(NULL, pwszURL, DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), E_FAIL);

	ulRename = rename(szNewName, szBuf);
	CHECKW(ulRename, 0);

	bRet = TRUE;

CLEANUP:
	if(!bRet)
		rename(szNewName, szBuf);
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRP);
	return bRet;
}


//*-----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
  	TBEGIN
	IBindResource* pIBR = NULL;
	ULONG_PTR ulOleObj	= 0; 

	//TESTC(CreateModInfo(pThisTestModule))
	TESTC(ModuleCreateDBSession(pThisTestModule))

	//Check if provider supports direct binding. If the provider doesn't support 
	//direct binding then we skip all test cases. As per the OLE DB spec if the provider sets
	//DBPROPVAL_OO_DIRECTBIND value of the DBPROP_OLE_OBJECTS, then the consumer
	//can assume that direct binding is supported.
	TESTC_PROVIDER(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		(IUnknown*)pThisTestModule->m_pIUnknown, &ulOleObj) &&
		((ulOleObj & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND));

	if(!IsUsableInterface(BINDER_INTERFACE, IID_IBindResource))
	{
		odtLog<<L"SKIP: CONF_STRICT specified and IBindResource, ICreateRow, etc. are Level-1 interfaces.\n";
		return TEST_SKIPPED;
	}

	//Verify that if Wininet.dll is missing, the Root Binder will fail gracefully (E_FAIL).
	COMPAREW(TestNoWininet(), TRUE);

	pIBR = GetModInfo()->GetRootBinder();
	TESTC_PROVIDER(pIBR != NULL)

CLEANUP:
	TRETURN
}

//*-----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	return ModuleReleaseDBSession(pThisTestModule);
}


////////////////////////////////////////////////////////////////////////////
//  TCBase
//
////////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(TC_ONETHREAD); }

	//methods
	virtual void SetTestCaseParam(ETESTCASE eTestCase = TC_ONETHREAD)
	{
		m_eTestCase = eTestCase;
		switch(eTestCase)
		{
			case TC_ONETHREAD:
				break;
			
			case TC_SINGLERB:
				break;

			case TC_MULTIPLERB:
				break;

			default:
				ASSERT(!L"Unhandled Type...");
				break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
};


////////////////////////////////////////////////////////////////////////
//TCRootBinder Class
//
////////////////////////////////////////////////////////////////////////
class TCRootBinder : public CSessionObject, public TCBase
{
public:

	//Constructor
	TCRootBinder(WCHAR* pwszTestCaseName);

	//Destructor
	virtual ~TCRootBinder();

protected:

//VARIABLES...

	LONG				m_lGenerateURL;
	WCHAR*				m_rgURLs[INVALID_OBJECT];
	WCHAR*				m_rgNewURLs[NEWURLS];
	//URLs that map to 3 different providers.
	WCHAR*				m_rgRowsetURL1;
	WCHAR*				m_rgRowsetURL2;
	WCHAR*				m_rgRowsetURL3;

//INTERFACES...

	IBindResource*			m_rgIBR[MAX_THREADS];
	ICreateRow*				m_rgICR[MAX_THREADS];
	IDBBinderProperties*	m_rgIDBBProp[MAX_THREADS];
	IRegisterProvider*		m_rgIRP[MAX_THREADS];

//METHODS...

	//Release all member pointers to interfaces.
	BOOL	ReleaseAll();

	//Initialize routine for most test cases.
	BOOL	InitTC();

	//Terminate routine for most test cases.
	BOOL	TermTC();

	//Get the Root Binder, requesting IBindResource.
	//Then QI for IDBBinderProperties and ICreateRow.
	BOOL	GetRootBinder();

	//Get URL from the INI file.
	WCHAR*	GetURL(EINTERFACE eInterface);

	//Set m_rgURLs.
	BOOL	InitializeURLs();

	//Get a session object (m_pIOpenRowset and m_pIDBInitialize). 
	//Required for calling CreateTable(...).
	BOOL	GetSession();

	//Set INIT props on Root Binder to make sure that a subsequent
	//call to Bind doesn't fail.
	BOOL	SetInitProps(IDBBinderProperties* pIDBBindProp);

	BOOL	CreateRBForThread(ULONG cThread);

	BOOL	ReleaseMultipleRBs(ULONG cThread);

	void	PrintPropsNotSet(HRESULT hr, ULONG cPropSets, DBPROPSET* rgPropSets);

	BOOL	FindIDinSets(DBPROPID dwPropID, ULONG cPropSets, DBPROPSET* rgPropSets);

	BOOL	IsDocSource(IBindResource* pIBR, WCHAR* pwszURL);

	BOOL	GetMarkedForOffline(IRowset* pIRowset);

	BOOL	GetColOrd(IRowset* pIRowset, LONG_PTR* pulOrd, DBID* pcolid, WCHAR* pwszColName=NULL);
	
	static ULONG WINAPI SetPropOPTIONAL(LPVOID pv);
	static ULONG WINAPI SetPropOPTIONALCR(LPVOID pv);
	static ULONG WINAPI SetPropREQUIRED(LPVOID pv);
	static ULONG WINAPI SetPropREQUIREDCR(LPVOID pv);
	static ULONG WINAPI GetIsMarkedFOL(LPVOID pv);
	static ULONG WINAPI ReadStreamInInc(LPVOID pv);
	static ULONG WINAPI PropsInError(LPVOID pv);
	static ULONG WINAPI PropsInErrorCR(LPVOID pv);
	static ULONG WINAPI ClearErrorProp(LPVOID pv);
	static ULONG WINAPI ClearErrorPropCR(LPVOID pv);
	static ULONG WINAPI PropBINDFLAGS(LPVOID pv);
	static ULONG WINAPI ColISCOLLECTION(LPVOID pv);
	static ULONG WINAPI BindToDiffrPBs(LPVOID pv);
	static ULONG WINAPI BindInDiffrSeq(LPVOID pv);
	static ULONG WINAPI BindWAITFORINIT(LPVOID pv);
	static ULONG WINAPI BindDiffrObject(LPVOID pv);
	static ULONG WINAPI RegSamePairs(LPVOID pv);
	static ULONG WINAPI ExtendURLAndReg(LPVOID pv);
	static ULONG WINAPI RegDiffrPairs(LPVOID pv);
	static ULONG WINAPI RegURLToDiffrPBs(LPVOID pv);
	static ULONG WINAPI UnregDiffrPairs(LPVOID pv);
	static ULONG WINAPI UnregAllURLsOfSamePB(LPVOID pv);
	static ULONG WINAPI UnregAllURLsOfDiffrPB(LPVOID pv);
	static ULONG WINAPI SetMODEandBINDFLAGS(LPVOID pv);
	static ULONG WINAPI SetMODEandBINDFLAGSCR(LPVOID pv);
	static ULONG WINAPI BinderRegKey(LPVOID pv);

};


////////////////////////////////////////////////////////////////////////
//TCRootBinder Implementation
//
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
//  TCRootBinder::TCRootBinder
//
TCRootBinder::TCRootBinder(WCHAR * pwszTestCaseName)	: CSessionObject(pwszTestCaseName) 
{
	ULONG	i;

	for(i=0;i<NUMELEM(m_rgURLs); i++)
		m_rgURLs[i]	= NULL;

	for(i=0;i<NUMELEM(m_rgNewURLs); i++)
		m_rgNewURLs[i]	= NULL;

	for(i=0;i<MAX_THREADS; i++)
	{
		m_rgIBR[i]	= NULL;
		m_rgICR[i]	= NULL;
		m_rgIDBBProp[i]	= NULL;
		m_rgIRP[i]	= NULL;
	}

	m_rgRowsetURL1			= NULL;
	m_rgRowsetURL2			= NULL;
	m_rgRowsetURL3			= NULL;

	m_lGenerateURL			= 0;
	m_pIOpenRowset			= NULL;
	m_pIDBInitialize		= NULL;
}

//----------------------------------------------------------------------
//  TCRootBinder::~TCRootBinder
//
TCRootBinder::~TCRootBinder()
{
	ReleaseAll();
}

//----------------------------------------------------------------------
// TCRootBinder::ReleaseAll
//
BOOL TCRootBinder::ReleaseAll()
{
	ULONG i;
	for(i=0;i<NUMELEM(m_rgNewURLs); i++)
		SAFE_FREE(m_rgNewURLs[i]);

	for(i=0;i<MAX_THREADS; i++)
	{
		SAFE_RELEASE(m_rgIBR[i]);
		SAFE_RELEASE(m_rgICR[i]);
		SAFE_RELEASE(m_rgIDBBProp[i]);
		SAFE_RELEASE(m_rgIRP[i]);
	}

	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIDBInitialize);
	return TRUE;
} //ReleaseAll

//----------------------------------------------------------------------
// TCRootBinder::InitTC
//
BOOL TCRootBinder::InitTC()
{
	TBEGIN
	CTable*		pTable = NULL;

	TESTC(GetRootBinder())

	TESTC(InitializeURLs())

	TESTC(GetSession())

	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;

	if(GetRootTable())
		SetTable(GetRootTable(), DELETETABLE_NO);
	else
	{
		TESTC(CreateTable(&pTable, 5))
		SetTable(pTable, DELETETABLE_NO);
	}

	if(m_eTestCase == TC_MULTIPLERB)
	{
		SAFE_RELEASE(m_rgIBR[0]);
		SAFE_RELEASE(m_rgICR[0]);
		SAFE_RELEASE(m_rgIDBBProp[0]);
		SAFE_RELEASE(m_rgIRP[0]);
	}

CLEANUP:
	TRETURN
} //InitTC

//----------------------------------------------------------------------
// TCRootBinder::TermTC
//
BOOL TCRootBinder::TermTC()
{
	if(m_pTable && (!GetRootTable()))
	{
		m_pTable->DropTable();
		SAFE_DELETE(m_pTable);
	}
	ReleaseAll();

	return TRUE;
} //TermTC

//----------------------------------------------------------------------
// TCRootBinder::GetRootBinder
//
BOOL TCRootBinder::GetRootBinder()
{
	TBEGIN
	IBindResource*	pIBR = NULL;

	pIBR = GetModInfo()->GetRootBinder();

	TESTC(VerifyInterface(pIBR, IID_IBindResource,
		BINDER_INTERFACE,(IUnknown**)&m_rgIBR[0]))
	TESTC(VerifyInterface(m_rgIBR[0], IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&m_rgICR[0]))
	TESTC(VerifyInterface(m_rgIBR[0], IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_rgIDBBProp[0]))
	TESTC(VerifyInterface(m_rgIBR[0], IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&m_rgIRP[0]))
	
CLEANUP:
	TRETURN
} //GetRootBinder

//----------------------------------------------------------------------
// TCRootBinder::GetURL
//
WCHAR* TCRootBinder::GetURL(EINTERFACE eInterface)
{
	WCHAR*	pwszURL = GetModInfo()->GetParseObject()->GetURL(eInterface);

	//URLs are of the form "scheme:<path>". Cannot have a valid URL 
	//with less than 2 characters.
	if(pwszURL && wcslen(pwszURL) < 2)
		pwszURL = NULL;

	return pwszURL;
} //GetURL

//----------------------------------------------------------------------
// TCRootBinder::InitializeURLs
//
BOOL TCRootBinder::InitializeURLs()
{
	m_rgURLs[STREAM]	= GetURL(STREAM_INTERFACE);	
	m_rgURLs[ROWSET]	= GetURL(ROWSET_INTERFACE);	
	m_rgURLs[ROW]		= GetURL(ROW_INTERFACE);
	m_rgURLs[SESSION]	= GetURL(SESSION_INTERFACE);
	m_rgURLs[DSO]		= GetURL(DATASOURCE_INTERFACE);	

	for(ULONG ulIndex=0; ulIndex<NUMELEM(m_rgURLs); ulIndex++)
		if((m_rgURLs[ulIndex] == NULL) || (wcslen(m_rgURLs[ulIndex]) < 2))
			return FALSE;

	//We can put URLs that map to different providers in the 
	//URL_DSO, URL_SESSION and URL_ROWSET keys of INI file. If 
	//they all map to same provider, then also the test will
	//work, only less testing will be obtained.
	m_rgRowsetURL1 = m_rgURLs[ROWSET];
	m_rgRowsetURL2 = m_rgURLs[DSO];
	m_rgRowsetURL3 = m_rgURLs[SESSION];

	return TRUE;
} //InitializeURLs

//----------------------------------------------------------------------
// TCRootBinder::GetSession
//
BOOL TCRootBinder::GetSession()
{
	TBEGIN
	IGetDataSource*	pIGetDataSource = NULL;

	if(!m_rgIBR[0] || !m_rgIDBBProp[0])
		return FALSE;

	TESTC(SetInitProps(m_rgIDBBProp[0]))

	TESTC_(m_rgIBR[0]->Bind(NULL, m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ, DBGUID_SESSION, IID_IOpenRowset, 
		NULL, NULL, NULL, (IUnknown**)
		&m_pIOpenRowset), S_OK)

	TESTC(VerifyInterface(m_pIOpenRowset,IID_IGetDataSource,
		SESSION_INTERFACE,(IUnknown**)&pIGetDataSource))

	TESTC_(pIGetDataSource->GetDataSource(IID_IDBInitialize,
		(IUnknown**)&m_pIDBInitialize), S_OK)

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	TRETURN
} //GetSession

//----------------------------------------------------------------------
// TCRootBinder::SetInitProps
//
BOOL TCRootBinder::SetInitProps(IDBBinderProperties* pIDBBindProp)
{
	BOOL		bRet = FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;

	if(!pIDBBindProp)
		return FALSE;

	CHECK(pIDBBindProp->Reset(), S_OK);

	if(COMPARE(GetInitProps(&cPropSets, &rgPropSets), TRUE))
	{
		if(CHECK(pIDBBindProp->SetProperties(cPropSets, rgPropSets), 
			S_OK))
			bRet = TRUE;
	}

	FreeProperties(&cPropSets, &rgPropSets);
	return bRet;
} //SetInitProps

//----------------------------------------------------------------------
// TCRootBinder::CreateRBForThread
//
BOOL TCRootBinder::CreateRBForThread(ULONG cThread)
{
	TBEGIN
	CLSID	clsid;

	if(FAILED(CLSIDFromProgID(L"MSDAURL.Binder", &clsid)))
		CLSIDFromString(L"MSDAURL.Binder", &clsid);

	TESTC_(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
		IID_IBindResource, (void**)&m_rgIBR[cThread]), S_OK)

	TESTC(VerifyInterface(m_rgIBR[cThread], IID_ICreateRow,
		BINDER_INTERFACE,(IUnknown**)&m_rgICR[cThread]))

	TESTC(VerifyInterface(m_rgIBR[cThread], IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_rgIDBBProp[cThread]))

	TESTC(VerifyInterface(m_rgIBR[cThread], IID_IRegisterProvider,
		BINDER_INTERFACE,(IUnknown**)&m_rgIRP[cThread]))

	TESTC(SetInitProps(m_rgIDBBProp[cThread]))

CLEANUP:
	TRETURN
} //CreateRBForThread

//----------------------------------------------------------------------
// TCRootBinder::ReleaseMultipleRBs
//
BOOL TCRootBinder::ReleaseMultipleRBs(ULONG cThread)
{
	TBEGIN

	if(m_eTestCase != TC_MULTIPLERB)
		return TRUE;

	SAFE_RELEASE(m_rgIBR[cThread]);
	SAFE_RELEASE(m_rgICR[cThread]);
	SAFE_RELEASE(m_rgIDBBProp[cThread]);
	SAFE_RELEASE(m_rgIRP[cThread]);

	TRETURN
} //ReleaseMultipleRBs

//----------------------------------------------------------------------
// TCRootBinder::PrintPropsNotSet
//
void TCRootBinder::PrintPropsNotSet(HRESULT hr, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	BOOL bFound=FALSE;

	//Printing in multiple thread cases produces a lot of clutter.
	if(m_eTestCase != TC_ONETHREAD)
		return;

	for(ULONG iSet=0; iSet<cPropSets; iSet++)
		for(ULONG iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
			if(rgPropSets[iSet].rgProperties[iProp].dwStatus != DBPROPSTATUS_OK)
			{
				if(!bFound)
					odtLog<<L"HRESULT = "<<GetErrorName(hr)<<L".\n";
				odtLog<<L"PropNotSet:  "<<GetPropertyName(rgPropSets[iSet].rgProperties[iProp].dwPropertyID, rgPropSets[iSet].guidPropertySet)<<L" , "<<GetPropStatusName(rgPropSets[iSet].rgProperties[iProp].dwStatus)<<L".\n";
				bFound = TRUE;
			}

	return;
} //PrintPropsNotSet

//----------------------------------------------------------------------
// TCRootBinder::FindIDinSets
//
BOOL TCRootBinder::FindIDinSets(DBPROPID dwPropID, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	BOOL bFound=FALSE;

	for(ULONG iSet=0; iSet<cPropSets; iSet++)
		for(ULONG iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
			if(rgPropSets[iSet].rgProperties[iProp].dwPropertyID == dwPropID)
			{
				bFound = TRUE;
				break;
			}

	return bFound;
} //FindIDinSets

//----------------------------------------------------------------------
// TCRootBinder::IsDocSource
//
BOOL TCRootBinder::IsDocSource(IBindResource* pIBR, WCHAR* pwszURL)
{
	BOOL			bRet = FALSE;
	ULONG_PTR		ulVal = 0;
	IDBProperties*	pIDBP = NULL;

	if(pIBR->Bind(NULL, pwszURL, DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIDBP) == S_OK)
	{
		if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO, pIDBP, &ulVal) 
			&& (ulVal & DBPROPVAL_DST_DOCSOURCE))
			bRet = TRUE;
		SAFE_RELEASE(pIDBP);
	}

	return bRet;
} //IsDocSource

//----------------------------------------------------------------------
// TCRootBinder::GetMarkedForOffline
//
BOOL TCRootBinder::GetMarkedForOffline(IRowset* pIRowset)
{
	TBEGIN
	HRESULT				hr = S_OK;
	WCHAR*				pwszBool;
	DBCOUNTITEM			cRowsObtained = 0;
	DBID				dbid=DB_NULLID;
	DBID				dbidName = DBROWCOL_PARSENAME;
	HROW*				rghRows = NULL;
	BYTE*				pData = NULL;
	DBLENGTH			cbRowSize = 0;
	DBCOUNTITEM			cBindings = 0;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDING*			rgBindings = NULL;
	DBORDINAL			cColsToBind = 2;
	LONG_PTR			rgColsToBind[2];
	IAccessor*			pIAccessor = NULL;

	TESTC(VerifyInterface(pIRowset, IID_IAccessor,
		ROWSET_INTERFACE,(IUnknown**)&pIAccessor))

	if(!GetColOrd(pIRowset, &(rgColsToBind[0]), &dbidName, NULL))
	{
		odtLog<<L"INFO: Failed to find RESOURCE_PARSENAME column.\n";
		goto CLEANUP;
	}

	if(!GetColOrd(pIRowset, &(rgColsToBind[1]), &dbid, L"RESOURCE_ISMARKEDFOROFFLINE"))
	{
		odtLog<<L"INFO: Failed to find RESOURCE_ISMARKEDFOROFFLINE column.\n";
		goto CLEANUP;
	}

	TESTC_(GetAccessorAndBindings(pIAccessor, 
		DBACCESSOR_ROWDATA, &hAccessor, &rgBindings,
		&cBindings, &cbRowSize,	DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, 
		NULL, NULL, DBTYPE_EMPTY, cColsToBind, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	SAFE_ALLOC(pData, BYTE, cbRowSize)
	memset(pData, 0, (size_t) cbRowSize);

	while(S_OK == pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&rghRows))
	{
		TESTC(cRowsObtained == 1)

		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData), S_OK)

		if (*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[1], pData) == VARIANT_TRUE)
			pwszBool = L"True";
		else
			pwszBool = L"False";

		if(m_eTestCase == TC_ONETHREAD)
		{
			odtLog<<L"Name = "<<(LPWSTR)&VALUE_BINDING(rgBindings[0], pData);
			odtLog<<L"\t\tIsMarkedForOffline = "<<pwszBool<<".\n";
		}

		cRowsObtained = 0;
		memset(pData, 0, (size_t) cbRowSize);
		SAFE_FREE(rghRows);
	}

CLEANUP:
	if(pIAccessor && hAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
	FreeAccessorBindings(cBindings, rgBindings);
	ReleaseDBID(&dbid, FALSE);
	SAFE_FREE(rghRows);
	SAFE_FREE(pData);
	SAFE_RELEASE(pIAccessor);
	TRETURN
} //GetMarkedForOffline

//----------------------------------------------------------------------
// TCRootBinder::GetColOrd
//
BOOL TCRootBinder::GetColOrd(IRowset* pIRowset, LONG_PTR* pulOrd, DBID* pcolid, WCHAR* pwszColName)
{
	BOOL			bRet = FALSE;
	DBORDINAL		cColumns = 0, ulIndex=0;
	WCHAR*			pStringsBuffer = NULL;
    DBCOLUMNINFO*	rgInfo = NULL;
	IColumnsInfo*	pICI = NULL;

	TESTC(pIRowset!=NULL && pulOrd!=NULL)

	TESTC(VerifyInterface(pIRowset, IID_IColumnsInfo,
		ROWSET_INTERFACE,(IUnknown**)&pICI))

	TESTC_(pICI->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK)

	for(ulIndex=0; ulIndex<cColumns; ulIndex++)
	{
		if(pwszColName)
		{
			if(wcscmp(rgInfo[ulIndex].pwszName, pwszColName))
			{
				*pulOrd = rgInfo[ulIndex].iOrdinal;
				if(pcolid)
				{
					pcolid->eKind = rgInfo[ulIndex].columnid.eKind;
					DuplicateDBID(rgInfo[ulIndex].columnid, pcolid);
				}
				bRet = TRUE;
			}
		}
		else if(pcolid)
		{
			if(CompareDBID(rgInfo[ulIndex].columnid, *pcolid))
			{
				*pulOrd = rgInfo[ulIndex].iOrdinal;
				bRet = TRUE;
			}
		}
	}

CLEANUP:
	SAFE_FREE(rgInfo);
    SAFE_FREE(pStringsBuffer);
	SAFE_RELEASE(pICI);
	return bRet;
} //GetColOrd

//----------------------------------------------------------------------
// TCRootBinder::SetPropOPTIONAL
// Set some properties (OPTIONAL) from each property group. Call
// SetProperty and the Bind. Also get PROPERTIESINERROR.
//
ULONG TCRootBinder::SetPropOPTIONAL(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			iSet, iProp;
	ULONG_PTR		ulVal=0;
	WCHAR*			pwszVal=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	BSTR			pbstrCC = L"RootBinder Test - DBPROP_CURRENTCATALOG";
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DB_BINDFLAGS_OPENIFEXISTS, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DBPROMPT_COMPLETE, DBTYPE_I2, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)pbstrCC, DBTYPE_BSTR, DBPROPOPTIONS_OPTIONAL) ;
	SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL) ;
	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, &cPropSets, &rgPropSets, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_S_ERRORSOCCURRED)
	if(hr == DB_S_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(hr == DB_S_ERRORSOCCURRED)
		TESTC_(hrBind, DB_S_ERRORSOCCURRED)

	TESTC_(hr=pIDBBProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	if(hrBind==S_OK)
		for(iSet=0; iSet<cPropSets; iSet++)
			for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
				COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);

	FreeProperties(&cPropSets, &rgPropSets);

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	if(cPropSets)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	if(hrBind==S_OK)
		TESTC(!cPropSets && !rgPropSets)
	else
		TESTC(cPropSets>0 && rgPropSets!=NULL)

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), S_OK, DB_S_ERRORSOCCURRED)

	if(!pThis->FindIDinSets(DBPROP_INIT_BINDFLAGS, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_BINDFLAGS_OPENIFEXISTS);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_MODE, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_PROMPT, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DBPROMPT_COMPLETE);
	}

	if(!pThis->FindIDinSets(DBPROP_CURRENTCATALOG, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pIProp, &pwszVal), TRUE);
		COMPARE(wcscmp(pwszVal, L"RootBinder Test - DBPROP_CURRENTCATALOG"), 0);
	}

	if(!pThis->FindIDinSets(DBPROP_MULTIPLECONNECTIONS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIProp), TRUE);

	if(!pThis->FindIDinSets(DBPROP_CANHOLDROWS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, pIRowset), TRUE);

	if(!pThis->FindIDinSets(DBPROP_IRowsetIdentity, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, pIRowset), TRUE);

	if(!pThis->FindIDinSets(DBPROP_CANSCROLLBACKWARDS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, pIRowset), TRUE);

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_FREE(pwszVal);
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetPropOPTIONAL

//----------------------------------------------------------------------
// TCRootBinder::SetPropOPTIONALCR
// Set some properties (OPTIONAL) from each property group. Call
// SetProperty and then CreateRow. Also get PROPERTIESINERROR.
//
ULONG TCRootBinder::SetPropOPTIONALCR(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			iSet, iProp;
	ULONG_PTR		ulVal=0;
	WCHAR*			pwszVal=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	BSTR			pbstrCC = L"RootBinder Test - DBPROP_CURRENTCATALOG";
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DB_BINDFLAGS_OPENIFEXISTS, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DBPROMPT_COMPLETE, DBTYPE_I2, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)pbstrCC, DBTYPE_BSTR, DBPROPOPTIONS_OPTIONAL) ;
	SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL) ;
	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, &cPropSets, &rgPropSets, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_S_ERRORSOCCURRED)
	if(hr == DB_S_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(hr == DB_S_ERRORSOCCURRED)
		TESTC_(hrBind, DB_S_ERRORSOCCURRED)

	TESTC_(hr=pIDBBProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	if(hrBind==S_OK)
		for(iSet=0; iSet<cPropSets; iSet++)
			for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
				COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);

	FreeProperties(&cPropSets, &rgPropSets);

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	if(cPropSets)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	if(hrBind==S_OK)
		TESTC(!cPropSets && !rgPropSets)
	else
		TESTC(cPropSets>0 && rgPropSets!=NULL)

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL,
		(IUnknown**)&pIProp), S_OK, DB_S_ERRORSOCCURRED)

	if(!pThis->FindIDinSets(DBPROP_INIT_BINDFLAGS, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_BINDFLAGS_OPENIFEXISTS);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_MODE, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_PROMPT, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DBPROMPT_COMPLETE);
	}

	if(!pThis->FindIDinSets(DBPROP_CURRENTCATALOG, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pIProp, &pwszVal), TRUE);
		COMPARE(wcscmp(pwszVal, L"RootBinder Test - DBPROP_CURRENTCATALOG"), 0);
	}

	if(!pThis->FindIDinSets(DBPROP_MULTIPLECONNECTIONS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIProp), TRUE);

	if(!pThis->FindIDinSets(DBPROP_CANHOLDROWS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, pIRowset), TRUE);

	if(!pThis->FindIDinSets(DBPROP_IRowsetIdentity, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, pIRowset), TRUE);

	if(!pThis->FindIDinSets(DBPROP_CANSCROLLBACKWARDS, cPropSets, rgPropSets))
		COMPARE(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, pIRowset), TRUE);

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_FREE(pwszVal);
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetPropOPTIONALCR

//----------------------------------------------------------------------
// TCRootBinder::SetPropREQUIRED
// Set some properties (REQUIRED) from each property group. Call
// SetProperty and the Bind. Also get PROPERTIESINERROR.
//
ULONG TCRootBinder::SetPropREQUIRED(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG_PTR		ulVal=0;
	WCHAR*			pwszVal=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	BSTR			pbstrCC = L"RootBinder Test - DBPROP_CURRENTCATALOG";
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)pbstrCC, DBTYPE_BSTR) ;
	SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL) ;
	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, &cPropSets, &rgPropSets, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)
	if(hr == DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	if(cPropSets)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	if(hrBind != S_OK)
		goto CLEANUP;  //Passed.

	TESTC(DefTestInterface(pIRowset, TRUE))

	TESTC_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), S_OK)

	COMPARE(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
	COMPARE(ulVal, DBPROMPT_COMPLETE);

	COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pIProp, &pwszVal), TRUE);
	COMPARE(wcscmp(pwszVal, L"RootBinder Test - DBPROP_CURRENTCATALOG"), 0);

	COMPARE(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIProp), TRUE);

	COMPARE(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, pIRowset), TRUE);
	COMPARE(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, pIRowset), TRUE);
	COMPARE(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, pIRowset), TRUE);

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_FREE(pwszVal);
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetPropREQUIRED

//----------------------------------------------------------------------
// TCRootBinder::SetPropREQUIREDCR
// Set some properties (REQUIRED) from each property group. Call
// SetProperty and then CreateRow. Also get PROPERTIESINERROR.
//
ULONG TCRootBinder::SetPropREQUIREDCR(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG_PTR		ulVal=0;
	WCHAR*			pwszVal=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	BSTR			pbstrCC = L"RootBinder Test - DBPROP_CURRENTCATALOG";
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)pbstrCC, DBTYPE_BSTR) ;
	SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL) ;
	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, &cPropSets, &rgPropSets, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)
	if(hr == DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	if(cPropSets)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	if(hrBind != S_OK)
		goto CLEANUP;  //Passed.

	TESTC(DefTestInterface(pIRowset, TRUE))

	TESTC_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), S_OK)

	COMPARE(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
	COMPARE(ulVal, DBPROMPT_COMPLETE);

	COMPARE(GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pIProp, &pwszVal), TRUE);
	COMPARE(wcscmp(pwszVal, L"RootBinder Test - DBPROP_CURRENTCATALOG"), 0);

	COMPARE(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, pIProp), TRUE);

	COMPARE(GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, pIRowset), TRUE);
	COMPARE(GetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, pIRowset), TRUE);
	COMPARE(GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, pIRowset), TRUE);

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_FREE(pwszVal);
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetPropREQUIREDCR

//----------------------------------------------------------------------
// TCRootBinder::GetIsMarkedFOL
// Get value of ISMARKEDFOROFFLINE column.
//
ULONG TCRootBinder::GetIsMarkedFOL(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	IDBInitialize*	pIDBI = NULL;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	TEST3C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_WAITFORINIT,
		DBGUID_ROWSET, IID_IDBInitialize, NULL, NULL, NULL, 
		(IUnknown**)&pIDBI), S_OK, E_NOINTERFACE, E_INVALIDARG)

	if(hr == S_OK)
	{
		TESTC_(pIDBI->Initialize(), S_OK)
		TESTC(VerifyInterface(pIDBI, IID_IRowset,
			ROWSET_INTERFACE,(IUnknown**)&pIRowset))
	}
	else
	{
		odtLog<<L"INFO: WAITFORINIT is not supported.\n";

		TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
			DBBINDURLFLAG_READWRITE,
			DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
			(IUnknown**)&pIRowset), S_OK)
	}

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(!pThis->IsDocSource(pIBR, pThis->m_rgURLs[ROWSET]))
		goto CLEANUP;  //Passed

	TESTC(pThis->GetMarkedForOffline(pIRowset))

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBI);
	THREAD_RETURN
} //GetIsMarkedFOL

//----------------------------------------------------------------------
// TCRootBinder::ReadStreamInInc
// Bind to a STREAM and read it in increments of 399, 400 and 401
// bytes.
//
ULONG TCRootBinder::ReadStreamInInc(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	ULONG			cBytesRead=0, cTotal=0;
	void*			pBuffer=NULL;
	IDBInitialize*	pIDBI = NULL;
	ISequentialStream*	pISS = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	ThreadSwitch(); //Let the other thread(s) catch up

	TEST3C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[STREAM], 
		DBBINDURLFLAG_WRITE|DBBINDURLFLAG_WAITFORINIT,
		DBGUID_STREAM, IID_IDBInitialize, NULL, NULL, NULL, 
		(IUnknown**)&pIDBI), S_OK, E_NOINTERFACE, E_INVALIDARG)

	if(hr == S_OK)
	{
		TESTC_(pIDBI->Initialize(), S_OK)
		TESTC(VerifyInterface(pIDBI, IID_ISequentialStream,
			STREAM_INTERFACE,(IUnknown**)&pISS))
	}
	else
	{
		TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[STREAM], 
			DBBINDURLFLAG_WRITE, DBGUID_STREAM, 
			IID_ISequentialStream, NULL, NULL, NULL, 
			(IUnknown**)&pISS), S_OK)
	}

	SAFE_ALLOC(pBuffer, BYTE, 500);
	while(S_OK == pISS->Read(pBuffer, 400, &cBytesRead))
	{
		COMPAREW(cBytesRead, 400);
		cTotal += cBytesRead;
	}
	odtLog<<L"INFO: Total Bytes in the stream = "<<cTotal<<".\n";

	while(S_OK == pISS->Read(pBuffer, 399, &cBytesRead))
		COMPAREW(cBytesRead, 399);

	while(S_OK == pISS->Read(pBuffer, 401, &cBytesRead))
		COMPAREW(cBytesRead, 401);

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIDBI);
	THREAD_RETURN
} //ReadStreamInInc

//----------------------------------------------------------------------
// TCRootBinder::PropsInError
// Set some rowset properties (REQUIRED). Call SetProperty and 
// the Bind. Also get PROPERTIESINERROR. Then Reset and Set only
// those properties which were NOT in error. Verify that Bind now
// returns S_OK.
//
ULONG TCRootBinder::PropsInError(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			cPropSetsGot=0;
	DBPROPSET*		rgPropSetsGot=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)
	if(hr == DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	SAFE_RELEASE(pIRowset);

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)
	if(cPropSetsGot)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSetsGot, rgPropSetsGot);
	}

	TESTC(pThis->SetInitProps(pIDBBProp))

	if(!pThis->FindIDinSets(DBPROP_CANHOLDROWS, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	if(!pThis->FindIDinSets(DBPROP_IRowsetIdentity, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	if(!pThis->FindIDinSets(DBPROP_CANSCROLLBACKWARDS, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TESTC_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	TESTC_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK)

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //PropsInError

//----------------------------------------------------------------------
// TCRootBinder::PropsInErrorCR
// Set some rowset properties (REQUIRED). Call SetProperty and 
// then CreateRow. Also get PROPERTIESINERROR. Then Reset and Set only
// those properties which were NOT in error. Verify that CreateRow now
// returns S_OK.
//
ULONG TCRootBinder::PropsInErrorCR(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			cPropSetsGot=0;
	DBPROPSET*		rgPropSetsGot=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)
	if(hr == DB_E_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	SAFE_RELEASE(pIRowset);

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)
	if(cPropSetsGot)
	{
		odtLog<<L"INFO: Properties in error are shown below :-\n";
		pThis->PrintPropsNotSet(hr, cPropSetsGot, rgPropSetsGot);
	}

	TESTC(pThis->SetInitProps(pIDBBProp))

	if(!pThis->FindIDinSets(DBPROP_CANHOLDROWS, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	if(!pThis->FindIDinSets(DBPROP_IRowsetIdentity, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	if(!pThis->FindIDinSets(DBPROP_CANSCROLLBACKWARDS, cPropSetsGot, rgPropSetsGot))
		SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TESTC_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK)

	TESTC_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK)

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //PropsInErrorCR

//----------------------------------------------------------------------
// TCRootBinder::ClearErrorProp
// 
ULONG TCRootBinder::ClearErrorProp(LPVOID pv)
{
		THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			cPropSetsGot=0;
	DBPROPSET*		rgPropSetsGot=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	SAFE_RELEASE(pIRowset);

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)

	FreeProperties(&cPropSetsGot, &rgPropSetsGot);

	TESTC(pThis->SetInitProps(pIDBBProp))

	TESTC_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)
	TESTC(!cPropSetsGot && !rgPropSetsGot)

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //ClearErrorProp

//----------------------------------------------------------------------
// TCRootBinder::ClearErrorPropCR
// 
ULONG TCRootBinder::ClearErrorPropCR(LPVOID pv)
{
		THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG			cPropSetsGot=0;
	DBPROPSET*		rgPropSetsGot=NULL;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)

	SAFE_RELEASE(pIRowset);

	if(hr == DB_E_ERRORSOCCURRED)
		TESTC_(hrBind, DB_E_ERRORSOCCURRED)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)

	FreeProperties(&cPropSetsGot, &rgPropSetsGot);

	TESTC(pThis->SetInitProps(pIDBBProp))

	TESTC_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_OPENIFEXISTS|DBBINDURLFLAG_COLLECTION,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), S_OK)
	TESTC(!cPropSetsGot && !rgPropSetsGot)

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //ClearErrorPropCR

//----------------------------------------------------------------------
// TCRootBinder::PropBINDFLAGS
// Set DBPROP_INIT_BINDFLAGS to invalid value and call Bind with
// flags=0. Should fail. Then call Bind again with READ flag. This
// should override the property. Also, reset properties and call
// Bind with WAITFORINIT. Release without initializing.
//
ULONG TCRootBinder::PropBINDFLAGS(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	IDBInitialize*	pIDBI = NULL;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_BINDFLAGS_OPENIFEXISTS|DB_BINDFLAGS_DELAYFETCHCOLUMNS), DBTYPE_I4);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_S_ERRORSOCCURRED)
	if(hr == DB_S_ERRORSOCCURRED)
	{
		odtLog<<L"INFO: The following props were not set on SetProperties call to Root Binder :-\n";
		pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);
	}

	TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 0,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), E_INVALIDARG)

	//If DBPROP_INIT_BINDFLAGS is not supported, we can get
	//DB_S_ERRORSOCCURRED also.
	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(pThis->SetInitProps(pIDBBProp))

	TEST3C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE|DBBINDURLFLAG_WAITFORINIT,
		DBGUID_ROWSET, IID_IDBInitialize, NULL, NULL, NULL, 
		(IUnknown**)&pIDBI), S_OK, E_NOINTERFACE, E_INVALIDARG)

	if(S_OK == hr)
		SAFE_RELEASE(pIDBI)
	else
		odtLog<<L"INFO: WAITFORINIT is not supported.\n";

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBI);
	THREAD_RETURN
} //PropBINDFLAGS

//----------------------------------------------------------------------
// TCRootBinder::ColISCOLLECTION
// Bind to a rowset. Check ISCOLLECTION column of each row, and
// for that are TRUE, Bind to it as a ROWSET.
//
ULONG TCRootBinder::ColISCOLLECTION(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrExp;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW*			rghRows = NULL;
	DBORDINAL		cColumns = 1;
	DBCOLUMNACCESS	rgColumns[1];
	WCHAR*			pwszURL = NULL;
	BYTE*			pData = NULL;
	IRow*			pIRow = NULL;
	IGetRow*		pIGR = NULL;
	IRowset*		pIRowset = NULL;
	IRowset*		pIRowset2 = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READWRITE,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK)

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(!VerifyInterface(pIRowset, IID_IGetRow,
		ROWSET_INTERFACE,(IUnknown**)&pIGR))
	{
		odtLog<<L"INFO: Row objects are not supported.\n";
		goto CLEANUP;
	}

	if(!pThis->IsDocSource(pIBR, pThis->m_rgURLs[ROWSET]))
		goto CLEANUP;  //Passed

	while(S_OK == pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&rghRows))
	{
		TESTC(cRowsObtained == 1)
		
		TESTC_(pIGR->GetRowFromHROW(NULL, rghRows[0], IID_IRow, (IUnknown**)&pIRow), S_OK)
		TESTC(pIRow != NULL)
		
		SAFE_ALLOC(rgColumns[0].pData, BYTE, sizeof(VARIANT_BOOL));
		memset(rgColumns[0].pData, 0, sizeof(VARIANT_BOOL));

		rgColumns[0].columnid = DBROWCOL_ISCOLLECTION;
		rgColumns[0].cbMaxLen = sizeof(VARIANT_BOOL);
		rgColumns[0].wType = DBTYPE_BOOL;

		TESTC_(pIRow->GetColumns(cColumns, rgColumns), S_OK)

		if(*(VARIANT_BOOL *)(rgColumns[0].pData) == VARIANT_TRUE)
			hrExp = S_OK;
		else
			hrExp = DB_E_NOTCOLLECTION;

		TESTC_(pIGR->GetURLFromHROW(rghRows[0], &pwszURL), S_OK)
		
		TESTC_(hr=pIBR->Bind(NULL, pwszURL, DBBINDURLFLAG_READ,
			DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
			(IUnknown**)&pIRowset2), hrExp)

		CHECK(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL), S_OK);
		cRowsObtained = 0;
		SAFE_RELEASE(pIRowset2);
		SAFE_RELEASE(pIRow);
		SAFE_FREE(rghRows);
		SAFE_FREE(pData);
		SAFE_FREE(pwszURL);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	if (pIRowset) CHECK(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL), S_OK);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRow);
	SAFE_FREE(rghRows);
	SAFE_FREE(pData);
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGR);
	RELEASERB
	THREAD_RETURN
} //ColISCOLLECTION

//----------------------------------------------------------------------
// TCRootBinder::BindToDiffrPBs
// Bind to different Provider Binders on different threads.
//
ULONG TCRootBinder::BindToDiffrPBs(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	TESTC(pThis->SetInitProps(pIDBBProp))

	ThreadSwitch(); //Let the other thread(s) catch up

	switch(cThread%3)
	{
		case 0:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
		case 1:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[SESSION], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
		case 2:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //BindToDiffrPBs

//----------------------------------------------------------------------
// TCRootBinder::BindInDiffrSeq
// 
ULONG TCRootBinder::BindInDiffrSeq(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	TESTC(pThis->SetInitProps(pIDBBProp))

	ThreadSwitch(); //Let the other thread(s) catch up

	switch(cThread%3)
	{
		case 0:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[SESSION], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}
			break;
		case 1:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[SESSION], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}
			break;
		case 2:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}

			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[SESSION], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
			{
				TESTC(pIRowset!=NULL)
				SAFE_RELEASE(pIRowset);
			}
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	RELEASERB
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //BindInDiffrSeq

//----------------------------------------------------------------------
// TCRootBinder::BindWAITFORINIT
// 
ULONG TCRootBinder::BindWAITFORINIT(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	IRowset*		pIRowset = NULL;
	IDBInitialize*	pIDBI = NULL;

	INITFUNC;

	TESTC(pThis->SetInitProps(pIDBBProp))

	ThreadSwitch(); //Let the other thread(s) catch up

	switch(cThread%3)
	{
		case 0:  //Without WAITFORINIT
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
		case 1:  //With WAITFORINIT
			TEST3C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
				DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT,
				DBGUID_ROWSET, IID_IDBInitialize, NULL, NULL, NULL, 
				(IUnknown**)&pIDBI), S_OK, E_INVALIDARG, E_NOINTERFACE)
			if(hr==S_OK)
				TESTC(pIDBI!=NULL)
			break;
		case 2:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	RELEASERB
	SAFE_RELEASE(pIDBI);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //BindWAITFORINIT

//----------------------------------------------------------------------
// TCRootBinder::BindDiffrObject
// 
ULONG TCRootBinder::BindDiffrObject(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT				hr=E_FAIL;
	IDBProperties*		pIDBP = NULL;
	ISessionProperties*	pISP = NULL;
	IRowset*			pIRowset = NULL;
	IRow*				pIRow = NULL;
	ISequentialStream*	pISS = NULL;

	INITFUNC;

	TESTC(pThis->SetInitProps(pIDBBProp))

	ThreadSwitch(); //Let the other thread(s) catch up

	switch(cThread%5)
	{
		case 0:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROW], 
				DBBINDURLFLAG_READ,
				DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
				(IUnknown**)&pIDBP), S_OK)
			TESTC(pIDBP!=NULL)
			break;
		case 1:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROW], 
				DBBINDURLFLAG_READ,
				DBGUID_SESSION, IID_ISessionProperties, NULL, NULL, NULL, 
				(IUnknown**)&pISP), S_OK)
			TESTC(pISP!=NULL)
			break;
		case 2:
			TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROW], 
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
				(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
			if(hr==S_OK)
				TESTC(pIRowset!=NULL)
			break;
		case 3:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROW], 
				DBBINDURLFLAG_READ,
				DBGUID_ROW, IID_IRow, NULL, NULL, NULL, 
				(IUnknown**)&pIRow), S_OK)
			TESTC(pIRow!=NULL)
			break;
		case 4:
			TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROW], 
				DBBINDURLFLAG_READ,
				DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, NULL, 
				(IUnknown**)&pISS), S_OK)
			TESTC(pISS!=NULL)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	RELEASERB
	SAFE_RELEASE(pIDBP);
	SAFE_RELEASE(pISP);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pISS);
	THREAD_RETURN
} //BindDiffrObject

//----------------------------------------------------------------------
// TCRootBinder::RegSamePairs
// Register same URL on all threads and Bind to it multiple times.
//
ULONG TCRootBinder::RegSamePairs(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	ULONG			ulIndex=0;
	CLSID			clsid;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	for(ulIndex=0; ulIndex<5; ulIndex++)
	{
		TESTC_(pIRP->GetURLMapping(pThis->m_rgURLs[ROWSET], 0, &clsid), S_OK)
		TESTC_(pIRP->SetURLMapping(pThis->m_rgURLs[ROWSET], 0, clsid), S_OK)
	}

	for(ulIndex=0; ulIndex<5; ulIndex++)
	{
		TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
			DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, 
			NULL, NULL, (IUnknown**)&pIRowset), S_OK)

		SAFE_RELEASE(pIRowset);
	}

	TEST2C_(pIRP->UnregisterProvider(pThis->m_rgURLs[ROWSET], 0, clsid), S_OK, S_FALSE)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(pThis->m_rgURLs[ROWSET], 0, clsid);
	SAFE_RELEASE(pIRowset);
	RELEASERB
	THREAD_RETURN
} //RegSamePairs

//----------------------------------------------------------------------
// TCRootBinder::ExtendURLAndReg
// Get the clsid of a URL is mapped to. Extend that URL and 
// register it to same provider (multiple times). Call Bind. 
// Now, unregister the new entry.
//
ULONG TCRootBinder::ExtendURLAndReg(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	ULONG			ulIndex=0;
	CLSID			clsid;
	WCHAR*			pwszNewURL = NULL;
	IRow*			pIRow = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	TESTC_(pIRP->GetURLMapping(pThis->m_rgURLs[ROW], 0, &clsid), S_OK)

	SAFE_ALLOC(pwszNewURL, WCHAR, wcslen(pThis->m_rgURLs[ROWSET])+wcslen(L"/123456789")+sizeof(WCHAR));
	wcscpy(pwszNewURL, pThis->m_rgURLs[ROWSET]);
	wcscat(pwszNewURL, L"/123456789");

	for(ulIndex=0; ulIndex<5; ulIndex++)
		TESTC_(pIRP->SetURLMapping(pwszNewURL, 0, clsid), S_OK)

	for(ulIndex=0; ulIndex<5; ulIndex++)
	{
		TESTC_(hr=pIBR->Bind(NULL, pwszNewURL, 
			DBBINDURLFLAG_READ, DBGUID_ROW, IID_IRow, NULL, 
			NULL, NULL, (IUnknown**)&pIRow), DB_E_NOTFOUND)
	}

	for(ulIndex=0; ulIndex<5; ulIndex++)
		TEST2C_(pIRP->UnregisterProvider(pwszNewURL, 0, clsid), S_OK, S_FALSE)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(pwszNewURL, 0, clsid);
	SAFE_FREE(pwszNewURL);
	SAFE_RELEASE(pIRow);
	RELEASERB
	THREAD_RETURN
} //ExtendURLAndReg

//----------------------------------------------------------------------
// TCRootBinder::RegDiffrPairs
//
ULONG TCRootBinder::RegDiffrPairs(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	CLSID			clsid1, clsid2, clsid3;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	switch(cThread%3)
	{
		case 0:
		{
			TESTC_(pIRP->GetURLMapping(pThis->m_rgURLs[DSO], 0, &clsid1), S_OK)
			TESTC_(pIRP->SetURLMapping(pThis->m_rgURLs[DSO], 0, clsid1), S_OK)
			break;
		}
		case 1:
		{
			TESTC_(pIRP->GetURLMapping(pThis->m_rgURLs[SESSION], 0, &clsid2), S_OK)
			TESTC_(pIRP->SetURLMapping(pThis->m_rgURLs[SESSION], 0, clsid2), S_OK)
			break;
		}
		case 2:
		{
			TESTC_(pIRP->GetURLMapping(pThis->m_rgURLs[ROWSET], 0, &clsid3), S_OK)
			TESTC_(pIRP->SetURLMapping(pThis->m_rgURLs[ROWSET], 0, clsid3), S_OK)
			break;
		}
	}

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[DSO], 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, 
		NULL, NULL, (IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
	SAFE_RELEASE(pIRowset);

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[SESSION], 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, 
		NULL, NULL, (IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION)
	SAFE_RELEASE(pIRowset);

	TESTC_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, 
		NULL, NULL, (IUnknown**)&pIRowset), S_OK)
	SAFE_RELEASE(pIRowset);

	TEST2C_(pIRP->UnregisterProvider(pThis->m_rgURLs[DSO], 0, clsid1), S_OK, S_FALSE)
	TEST2C_(pIRP->UnregisterProvider(pThis->m_rgURLs[SESSION], 0, clsid2), S_OK, S_FALSE)
	TEST2C_(pIRP->UnregisterProvider(pThis->m_rgURLs[ROWSET], 0, clsid3), S_OK, S_FALSE)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(pThis->m_rgURLs[DSO], 0, clsid1);
	pIRP->UnregisterProvider(pThis->m_rgURLs[SESSION], 0, clsid2);
	pIRP->UnregisterProvider(pThis->m_rgURLs[ROWSET], 0, clsid3);
	SAFE_RELEASE(pIRowset);
	RELEASERB
	THREAD_RETURN
} //RegDiffrPairs

//----------------------------------------------------------------------
// TCRootBinder::RegURLToDiffrPBs
//
ULONG TCRootBinder::RegURLToDiffrPBs(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr1=E_FAIL,hr2=E_FAIL,hr3=E_FAIL;
	CLSID			clsidTemp = DBGUID_DSO;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	switch(cThread%3)
	{
		case 0:
			TEST2C_(hr1=pIRP->SetURLMapping(URL1, 0, CLSID_PROV1), S_OK, DB_E_RESOURCEEXISTS)
			break;
		case 1:
			TEST2C_(hr2=pIRP->SetURLMapping(URL1, 0, CLSID_PROV2), S_OK, DB_E_RESOURCEEXISTS)
			break;
		case 2:
			TEST2C_(hr3=pIRP->SetURLMapping(URL1, 0, CLSID_PROV3), S_OK, DB_E_RESOURCEEXISTS)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_OK)
	COMPARE(clsidTemp==CLSID_PROV1 || clsidTemp==CLSID_PROV2 ||
		clsidTemp==CLSID_PROV3, TRUE);

	switch(cThread%3)
	{
		case 0:
			if(hr1 == S_OK)
				COMPARE(clsidTemp, CLSID_PROV1);
			break;
		case 1:
			if(hr2 == S_OK)
				COMPARE(clsidTemp, CLSID_PROV2);
			break;
		case 2:
			if(hr3 == S_OK)
				COMPARE(clsidTemp, CLSID_PROV3);
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(URL1, 0, CLSID_PROV1);
	pIRP->UnregisterProvider(URL1, 0, CLSID_PROV2);
	pIRP->UnregisterProvider(URL1, 0, CLSID_PROV3);
	RELEASERB
	THREAD_RETURN
} //RegURLToDiffrPBs

//----------------------------------------------------------------------
// TCRootBinder::UnregDiffrPairs
//
ULONG TCRootBinder::UnregDiffrPairs(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	CLSID			clsidTemp;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->SetURLMapping(URL1, 0, CLSID_PROV1), S_OK)
	TESTC_(pIRP->SetURLMapping(URL2, 0, CLSID_PROV2), S_OK)
	TESTC_(pIRP->SetURLMapping(URL3, 0, CLSID_PROV3), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_OK)
	COMPARE(clsidTemp, CLSID_PROV1);
	TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_OK)
	COMPARE(clsidTemp, CLSID_PROV2);
	TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_OK)
	COMPARE(clsidTemp, CLSID_PROV3);

	switch(cThread%3)
	{
		case 0:
			TEST2C_(pIRP->UnregisterProvider(URL1, 0, CLSID_PROV1), S_OK, S_FALSE)
			TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_FALSE)
			break;
		case 1:
			TEST2C_(pIRP->UnregisterProvider(URL2, 0, CLSID_PROV2), S_OK, S_FALSE)
				TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_FALSE)
			break;
		case 2:
			TEST2C_(pIRP->UnregisterProvider(URL3, 0, CLSID_PROV3), S_OK, S_FALSE)
				TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_FALSE)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(URL1, 0, CLSID_PROV1);
	pIRP->UnregisterProvider(URL2, 0, CLSID_PROV2);
	pIRP->UnregisterProvider(URL3, 0, CLSID_PROV3);
	SAFE_RELEASE(pIRowset);
	RELEASERB
	THREAD_RETURN
} //UnregDiffrPairs

//----------------------------------------------------------------------
// TCRootBinder::UnregAllURLsOfSamePB
//
ULONG TCRootBinder::UnregAllURLsOfSamePB(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	ULONG			ulIndex=0;
	CLSID			clsidTemp;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->SetURLMapping(URL1, 0, CLSID_PROV1), S_OK)
	TESTC_(pIRP->SetURLMapping(URL2, 0, CLSID_PROV1), S_OK)
	TESTC_(pIRP->SetURLMapping(URL3, 0, CLSID_PROV1), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV1)
	TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV1)
	TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV1)

	ThreadSwitch(); //Let the other thread(s) catch up

	TEST2C_(pIRP->UnregisterProvider(NULL, 0, CLSID_PROV1), S_OK, S_FALSE)

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_FALSE)
	TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_FALSE)
	TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_FALSE)

CLEANUP:
	pIRP->UnregisterProvider(NULL, 0, CLSID_PROV1);
	SAFE_RELEASE(pIRowset);
	RELEASERB
	THREAD_RETURN
} //UnregAllURLsOfSamePB

//----------------------------------------------------------------------
// TCRootBinder::UnregAllURLsOfDiffrPB
//
ULONG TCRootBinder::UnregAllURLsOfDiffrPB(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	ULONG			ulIndex=0;
	CLSID			clsidTemp;
	IRowset*		pIRowset = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->SetURLMapping(URL1, 0, CLSID_PROV1), S_OK)
	TESTC_(pIRP->SetURLMapping(URL2, 0, CLSID_PROV2), S_OK)
	TESTC_(pIRP->SetURLMapping(URL3, 0, CLSID_PROV3), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV1)
	TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV2)
	TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_OK)
	TESTC(clsidTemp == CLSID_PROV3)

	switch(cThread%3)
	{
		case 0:
			TEST2C_(pIRP->UnregisterProvider(NULL, 0, CLSID_PROV1), S_OK, S_FALSE)
			TESTC_(pIRP->GetURLMapping(URL1, 0, &clsidTemp), S_FALSE)
			break;
		case 1:
			TEST2C_(pIRP->UnregisterProvider(NULL, 0, CLSID_PROV2), S_OK, S_FALSE)
			TESTC_(pIRP->GetURLMapping(URL2, 0, &clsidTemp), S_FALSE)
			break;
		case 2:
			TEST2C_(pIRP->UnregisterProvider(NULL, 0, CLSID_PROV3), S_OK, S_FALSE)
			TESTC_(pIRP->GetURLMapping(URL3, 0, &clsidTemp), S_FALSE)
			break;
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pIRP->UnregisterProvider(NULL, 0, CLSID_PROV1);
	pIRP->UnregisterProvider(NULL, 0, CLSID_PROV2);
	pIRP->UnregisterProvider(NULL, 0, CLSID_PROV3);
	SAFE_RELEASE(pIRowset);
	RELEASERB
	THREAD_RETURN
} //UnregAllURLsOfDiffrPB

//----------------------------------------------------------------------
// TCRootBinder::SetMODEandBINDFLAGS
//
ULONG TCRootBinder::SetMODEandBINDFLAGS(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG_PTR		ulVal=0;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)DB_BINDFLAGS_RECURSIVE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE), DBTYPE_I4);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_S_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 0,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, 
		(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(hr == DB_S_ERRORSOCCURRED)
		TESTC_(hrBind, DB_S_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);

	if(hrBind==S_OK)
		TESTC(!cPropSets && !rgPropSets)
	else
		TESTC(cPropSets>0 && rgPropSets!=NULL)

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 0,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), S_OK, DB_S_ERRORSOCCURRED)

	if(!pThis->FindIDinSets(DBPROP_INIT_BINDFLAGS, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_BINDFLAGS_RECURSIVE);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_MODE, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE);
		ulVal = 0;
	}

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetMODEandBINDFLAGS

//----------------------------------------------------------------------
// TCRootBinder::SetMODEandBINDFLAGSCR
//
ULONG TCRootBinder::SetMODEandBINDFLAGSCR(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	HRESULT			hrBind=E_FAIL;
	ULONG_PTR		ulVal=0;
	ULONG			cPropIDSets=1;
	DBPROPIDSET		rgPropIDSets[1];
	IRowset*		pIRowset = NULL;
	IDBProperties*	pIProp = NULL;

	INITFUNC;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC(pThis->SetInitProps(pIDBBProp))

	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_BINDFLAGS_OPENIFEXISTS|DB_BINDFLAGS_RECURSIVE|DB_BINDFLAGS_COLLECTION), DBTYPE_I4);
	SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)(DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE), DBTYPE_I4);

	TEST2C_(hr=pIDBBProp->SetProperties(cPropSets, rgPropSets), S_OK, DB_S_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	TEST2C_(hrBind=pICR->CreateRow(NULL, pThis->m_rgURLs[ROWSET], 0,
		DBGUID_ROWSET, IID_IRowset, NULL, NULL, NULL, NULL,
		(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED)

	TESTC(DefTestInterface(pIRowset, TRUE))

	if(hr == DB_S_ERRORSOCCURRED)
		TESTC_(hrBind, DB_S_ERRORSOCCURRED)

	FreeProperties(&cPropSets, &rgPropSets);

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC_(hr=pIDBBProp->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	pThis->PrintPropsNotSet(hr, cPropSets, rgPropSets);

	if(hrBind==S_OK)
		TESTC(!cPropSets && !rgPropSets)
	else
		TESTC(cPropSets>0 && rgPropSets!=NULL)

	TEST2C_(hr=pIBR->Bind(NULL, pThis->m_rgURLs[ROWSET], 0,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL, 
		(IUnknown**)&pIProp), S_OK, DB_S_ERRORSOCCURRED)

	if(!pThis->FindIDinSets(DBPROP_INIT_BINDFLAGS, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_BINDFLAGS_RECURSIVE);
		ulVal = 0;
	}

	if(!pThis->FindIDinSets(DBPROP_INIT_MODE, cPropSets, rgPropSets))
	{
		COMPARE(GetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, pIProp, &ulVal), TRUE);
		COMPARE(ulVal, DB_MODE_READWRITE|DB_MODE_SHARE_DENY_WRITE);
		ulVal = 0;
	}

	TESTC_(pIDBBProp->Reset(), S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	RELEASERB
	SAFE_RELEASE(pIProp);
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
} //SetMODEandBINDFLAGSCR

//----------------------------------------------------------------------
// TCRootBinder::BinderRegKey
// Verify "OLE DB Binder" key is present.
//
ULONG TCRootBinder::BinderRegKey(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	HRESULT			hr=E_FAIL;
	BOOL			bFound = FALSE;
	CHAR			szKeyName[200];
	LONG			lResult;
	HKEY			hKey = NULL;
	DWORD			dwIndex;
	CHAR			szValueName[100];
	DWORD			cbValueName;
	CHAR			szData[100];
	DWORD			cbData;
	DWORD			dwType;

	INITFUNC;

	//if(pThis->m_eTestCase != TC_ONETHREAD)
	//	goto CLEANUP;

	strcpy(szKeyName, "CLSID\\");
	strcat(szKeyName, "{FF151822-B0BF-11D1-A80D-000000000000}\\");
	strcat(szKeyName, "OLE DB Binder");

	//Open the key for "OLE DB Binder". Success of this call
	//will confirm presence of this key.
	lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
						   szKeyName,
						   0,
						   KEY_READ,
						   &hKey);
	TESTC( lResult == ERROR_SUCCESS )

	for( dwIndex = 0; ; dwIndex++ )
	{
		cbValueName	= sizeof(szValueName);
		cbData		= sizeof(szData);

		lResult = RegEnumValue(hKey,
							   dwIndex,
							   szValueName,
							   &cbValueName,
							   NULL,
							   &dwType,
							   (LPBYTE)szData,
							   &cbData);
		TESTC( lResult == ERROR_SUCCESS )

		// Stop if we hit "Microsoft OLE DB Root Binder"
		if(!strcmp(szData, "Microsoft OLE DB Root Binder"))
		{
			bFound = TRUE;
			break;
		}
	}

	TESTC(bFound)

CLEANUP:
	RegCloseKey(hKey);
	RELEASERB
	THREAD_RETURN
} //BinderRegKey




//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------

// {{ TCW_TEST_CASE_MAP(TCGeneralScenarios)
//*-----------------------------------------------------------------------
// @class Test common scenarios under single and multiple threads.
//
class TCGeneralScenarios : public TCRootBinder { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGeneralScenarios,TCRootBinder);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Set props (OPTIONAL) from all sets, Bind to rowset.
	int Variation_1();
	// @cmember Set props (OPTIONAL) from all sets, CreateRow a rowset.
	int Variation_2();
	// @cmember Set props (REQUIRED) from all sets, Bind to rowset.
	int Variation_3();
	// @cmember Set props (REQUIRED) from all sets, CreateRow a rowset.
	int Variation_4();
	// @cmember Get value of ISMARKEDFOROFFLINE column.
	int Variation_5();
	// @cmember Bind to a STREAM and read in increments.
	int Variation_6();
	// @cmember Get PROPERTIESINERROR, Reset and Set (BIND)
	int Variation_7();
	// @cmember Get PROPERTIESINERROR, Reset and Set (CREATEROW)
	int Variation_8();
	// @cmember Bind, GetProperties in error, and Bind again.
	int Variation_9();
	// @cmember CreateRow, GetProperties in error, and CreateRow again.
	int Variation_10();
	// @cmember Set DBPROP_INIT_BINDFLAGS to invalid value.
	int Variation_11();
	// @cmember Check ISCOLLECTION column and Bind.
	int Variation_12();
	// @cmember Bind to diffr PBs on diffr threads.
	int Variation_13();
	// @cmember Call Bind on various PBs in diffr sequence.
	int Variation_14();
	// @cmember Call Bind on same URL with & without WAITFORINIT flag.
	int Variation_15();
	// @cmember Bind a diffr object on each thread.
	int Variation_16();
	// @cmember Register same URL on all threads.
	int Variation_17();
	// @cmember Extend URL and register to same provider.
	int Variation_18();
	// @cmember Register diffr pairs on diffr threads.
	int Variation_19();
	// @cmember Register same URL to diffr PBs on diffr threads.
	int Variation_20();
	// @cmember Unreg diffr mappings on diffr threads.
	int Variation_21();
	// @cmember Unreg all URLs of same PB on diffr threads.
	int Variation_22();
	// @cmember Unreg all URLs of diffr PB on diffr threads.
	int Variation_23();
	// @cmember Set MODE and BINDFLAGS props (BIND)
	int Variation_24();
	// @cmember Set MODE and BINDFLAGS props (CREATEROW)
	int Variation_25();
	// @cmember Verify OLE DB Binder key
	int Variation_26();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGeneralScenarios)
#define THE_CLASS TCGeneralScenarios
BEG_TEST_CASE(TCGeneralScenarios, TCRootBinder, L"Test common scenarios under single and multiple threads.")
	TEST_VARIATION(1, 		L"Set props (OPTIONAL) from all sets, Bind to rowset.")
	TEST_VARIATION(2, 		L"Set props (OPTIONAL) from all sets, CreateRow a rowset.")
	TEST_VARIATION(3, 		L"Set props (REQUIRED) from all sets, Bind to rowset.")
	TEST_VARIATION(4, 		L"Set props (REQUIRED) from all sets, CreateRow a rowset.")
	TEST_VARIATION(5, 		L"Get value of ISMARKEDFOROFFLINE column.")
	TEST_VARIATION(6, 		L"Bind to a STREAM and read in increments.")
	TEST_VARIATION(7, 		L"Get PROPERTIESINERROR, Reset and Set (BIND)")
	TEST_VARIATION(8, 		L"Get PROPERTIESINERROR, Reset and Set (CREATEROW)")
	TEST_VARIATION(9, 		L"Bind, GetProperties in error, and Bind again.")
	TEST_VARIATION(10, 		L"CreateRow, GetProperties in error, and CreateRow again.")
	TEST_VARIATION(11, 		L"Set DBPROP_INIT_BINDFLAGS to invalid value.")
	TEST_VARIATION(12, 		L"Check ISCOLLECTION column and Bind.")
	TEST_VARIATION(13, 		L"Bind to diffr PBs on diffr threads.")
	TEST_VARIATION(14, 		L"Call Bind on various PBs in diffr sequence.")
	TEST_VARIATION(15, 		L"Call Bind on same URL with & without WAITFORINIT flag.")
	TEST_VARIATION(16, 		L"Bind a diffr object on each thread.")
	TEST_VARIATION(17, 		L"Register same URL on all threads.")
	TEST_VARIATION(18, 		L"Extend URL and register to same provider.")
	TEST_VARIATION(19, 		L"Register diffr pairs on diffr threads.")
	TEST_VARIATION(20, 		L"Register same URL to diffr PBs on diffr threads.")
	TEST_VARIATION(21, 		L"Unreg diffr mappings on diffr threads.")
	TEST_VARIATION(22, 		L"Unreg all URLs of same PB on diffr threads.")
	TEST_VARIATION(23, 		L"Unreg all URLs of diffr PB on diffr threads.")
	TEST_VARIATION(24, 		L"Set MODE and BINDFLAGS props (BIND)")
	TEST_VARIATION(25, 		L"Set MODE and BINDFLAGS props (CREATEROW)")
	TEST_VARIATION(26, 		L"Verify OLE DB Binder key")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()



////////////////////////////////////////////////////////////////
// Duplicating Test Cases
////////////////////////////////////////////////////////////////

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


//Copy the single thread test case for -
//(1) One RB multiple thread, and
//(2) Multiple RBs.
COPY_TEST_CASE(TCGeneralScenarios_SRB, TCGeneralScenarios)
COPY_TEST_CASE(TCGeneralScenarios_MRB, TCGeneralScenarios)



#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(1, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGeneralScenarios)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(3, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGeneralScenarios)
	TEST_CASE_WITH_PARAM(2, TCGeneralScenarios_SRB, TC_SINGLERB)
	TEST_CASE_WITH_PARAM(3, TCGeneralScenarios_MRB, TC_MULTIPLERB)
END_TEST_MODULE()
#endif



// {{ TCW_TC_PROTOTYPE(TCGeneralScenarios)
//*-----------------------------------------------------------------------
//| Test Case:		TCGeneralScenarios - Test common scenarios under single and multiple threads.
//| Created:  	3/30/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGeneralScenarios::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRootBinder::Init())
	// }}
	{ 
		return InitTC();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set props (OPTIONAL) from all sets, Bind to rowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_1()
{ 
	TBEGIN
	RUNVAR(SetPropOPTIONAL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set props (OPTIONAL) from all sets, CreateRow a rowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_2()
{ 
	TBEGIN
	RUNVAR(SetPropOPTIONALCR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set props (REQUIRED) from all sets, Bind to rowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_3()
{ 
	TBEGIN
	RUNVAR(SetPropREQUIRED);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set props (REQUIRED) from all sets, CreateRow a rowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_4()
{ 
	TBEGIN
	RUNVAR(SetPropREQUIREDCR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Get value of ISMARKEDFOROFFLINE column.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_5()
{ 
	TBEGIN
	RUNVAR(GetIsMarkedFOL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Bind to a STREAM and read in increments.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_6()
{ 
	TBEGIN
	RUNVAR(ReadStreamInInc);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Get PROPERTIESINERROR, Reset and Set (BIND)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_7()
{ 
	TBEGIN
	RUNVAR(PropsInError);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Get PROPERTIESINERROR, Reset and Set (CREATEROW)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_8()
{ 
	TBEGIN
	RUNVAR(PropsInErrorCR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Bind, GetProperties in error, and Bind again.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_9()
{ 
	TBEGIN
	RUNVAR(ClearErrorProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc CreateRow, GetProperties in error, and CreateRow again.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_10()
{ 
	TBEGIN
	RUNVAR(ClearErrorPropCR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INIT_BINDFLAGS to invalid value.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_11()
{ 
	TBEGIN
	RUNVAR(PropBINDFLAGS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Check ISCOLLECTION column and Bind.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_12()
{ 
	TBEGIN
	RUNVAR(ColISCOLLECTION);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Bind to diffr PBs on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_13()
{ 
	TBEGIN
	RUNVAR(BindToDiffrPBs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Call Bind on various PBs in diffr sequence.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_14()
{ 
	TBEGIN
	RUNVAR(BindInDiffrSeq);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Call Bind on same URL with & without WAITFORINIT flag.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_15()
{ 
	TBEGIN
	RUNVAR(BindWAITFORINIT);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Bind a diffr object on each thread.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_16()
{ 
	TBEGIN
	RUNVAR(BindDiffrObject);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Register same URL on all threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_17()
{ 
	TBEGIN
	RUNVAR(RegSamePairs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Extend URL and register to same provider.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_18()
{ 
	TBEGIN
	RUNVAR(ExtendURLAndReg);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Register diffr pairs on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_19()
{ 
	TBEGIN
	RUNVAR(RegDiffrPairs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Register same URL to diffr PBs on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_20()
{ 
	TBEGIN
	RUNVAR(RegURLToDiffrPBs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Unreg diffr mappings on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_21()
{ 
	TBEGIN
	RUNVAR(UnregDiffrPairs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Unreg all URLs of same PB on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_22()
{ 
	TBEGIN
	RUNVAR(UnregAllURLsOfSamePB);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Unreg all URLs of diffr PB on diffr threads.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_23()
{ 
	TBEGIN
	RUNVAR(UnregAllURLsOfDiffrPB);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Set MODE and BINDFLAGS props (BIND)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_24()
{ 
	TBEGIN
	RUNVAR(SetMODEandBINDFLAGS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Set MODE and BINDFLAGS props (CREATEROW)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_25()
{ 
	TBEGIN
	RUNVAR(SetMODEandBINDFLAGSCR);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Verify OLE DB Binder key
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGeneralScenarios::Variation_26()
{ 
	TBEGIN
	RUNVAR(BinderRegKey);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGeneralScenarios::Terminate()
{ 
	TermTC();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRootBinder::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

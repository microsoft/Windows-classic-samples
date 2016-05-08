//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IDBINIT.CPP | OLE DB IDBInitialize tests for Provider.
//

#include "modstandard.hpp"	// Standard headers, precompiled in modcore.cpp			
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "idbinit.h"		// Testcase's header 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x10814ad0, 0xbbc6, 0x11cf, { 0x99, 0x17, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("IDBInitialize");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IDBInitialize Interface.");
DECLARE_MODULE_VERSION(833647380);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

//global for DBINIT_DBPROP's
struct DBInitDBProps
{
	DBPROPID			dwPropertyID;	// The DBPROP of DBINIT
	BOOL				fSupported;		// Whether the DBPROP is supported
	BOOL				fSettable;		// Whether the DBPROP is settable
	BOOL				fRequired;		// Whether the DBPROP is required
	VARTYPE				vtPropType;		// The type of the DBPROP
	VARIANT				vValue;			// The value of the DBPROP
	WCHAR				wszDescBuff[50];// The description of the DBPROP
}g_rgDBInitDBProps[]={

//
//	guidProperty						fSupported	fSettable	fRequired	vtPropType	vValue
//	============						==========	==========	========	========	=======	
DBPROP_AUTH_CACHE_AUTHINFO,				FALSE,		FALSE,		FALSE,		VT_BOOL,	{VT_EMPTY,0,0,0,0},	L"Cache Authentication",
DBPROP_AUTH_ENCRYPT_PASSWORD,			FALSE,		FALSE,		FALSE,		VT_BOOL,	{VT_EMPTY,0,0,0,0},	L"Encrypt Password",
DBPROP_AUTH_INTEGRATED,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Integrated Security",
DBPROP_AUTH_MASK_PASSWORD,				FALSE,		FALSE,		FALSE,		VT_BOOL,	{VT_EMPTY,0,0,0,0},	L"Mask Password",
DBPROP_AUTH_PASSWORD,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Password",
DBPROP_AUTH_PERSIST_ENCRYPTED,			FALSE,		FALSE,		FALSE,		VT_BOOL,	{VT_EMPTY,0,0,0,0},	L"Persist Encrypted",
DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO,	FALSE,		FALSE,		FALSE,		VT_BOOL,	{VT_EMPTY,0,0,0,0},	L"Persist Security Info",
DBPROP_AUTH_USERID,						FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"User ID",
DBPROP_INIT_ASYNCH,						FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Asynchronous Processing",
DBPROP_INIT_CATALOG,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Initial Catalog",
DBPROP_INIT_DATASOURCE,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Data Source",
DBPROP_INIT_GENERALTIMEOUT,				FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"General Timeout",
#ifdef _WIN64
DBPROP_INIT_HWND,						FALSE,		FALSE,		FALSE,		VT_I8,		{VT_EMPTY,0,0,0,0},	L"Window Handle",
#else
DBPROP_INIT_HWND,						FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Window Handle",
#endif
DBPROP_INIT_IMPERSONATION_LEVEL,		FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Impersonation Level",
DBPROP_INIT_LCID,						FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Locale Identifier",
DBPROP_INIT_LOCATION,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Location",
DBPROP_INIT_MODE,						FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Mode",
DBPROP_INIT_PROMPT,						FALSE,		FALSE,		FALSE,		VT_I2,		{VT_EMPTY,0,0,0,0},	L"Prompt",
DBPROP_INIT_PROTECTION_LEVEL,			FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Protection Level",
DBPROP_INIT_PROVIDERSTRING,				FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Extended Properties",
DBPROP_INIT_TIMEOUT,					FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Connect Timeout",
DBPROP_INIT_OLEDBSERVICES,				FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"OLE DB Services",
DBPROP_INIT_BINDFLAGS,					FALSE,		FALSE,		FALSE,		VT_I4,		{VT_EMPTY,0,0,0,0},	L"Bind Flags",
DBPROP_INIT_LOCKOWNER,					FALSE,		FALSE,		FALSE,		VT_BSTR,	{VT_EMPTY,0,0,0,0},	L"Lock Owner",
};																	

ULONG g_cMaxPropertyInfoSets = NUMELEM(g_rgDBInitDBProps);

// For IDBInintialize::Initialize
ULONG		g_cPropertySets;
DBPROPSET *	g_rgPropertySets;

// For IDBProperties::GetPropertyInfo
ULONG	g_cPropertyInfoSets = 0;
ULONG	g_cPropertyInfo		= 0;
ULONG	g_ulProviderSpecific= 0;

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	IDBProperties *	pIDBProperties		= NULL;
	ULONG			ulIndex				= 0;
	ULONG			ulIndex1			= 0;
	ULONG			ulSupported			= 0;

	DBPROPIDSET		rgPropertyIDSets;
	ULONG			cPropertyInfoSets	= 0;
	DBPROPINFOSET *	prgProperyInfoSets	= NULL;
	OLECHAR *		pDescBuffer			= NULL;

	if(ModuleCreateDBSession(pThisTestModule))
	{
		// Clear out the fSupported flags in the Global array
		g_cPropertyInfoSets = 0;
		g_cPropertyInfo		= 0;
		g_ulProviderSpecific= 0;

		for(ULONG i=0; i<g_cMaxPropertyInfoSets; i++)
		{
			g_rgDBInitDBProps[i].fSupported = FALSE;
			g_rgDBInitDBProps[i].fRequired  = FALSE;
		}
		
		// Build our init options from string passed to us from TMD for this provider
		TESTC(GetInitProps(&g_cPropertySets, &g_rgPropertySets));

		// Get the IDBProperties Interface
		TESTC(VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBProperties, 
								DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

		// Set the DBPROPIDSET Structure
		rgPropertyIDSets.cPropertyIDs    = 0;
		rgPropertyIDSets.rgPropertyIDs   = NULL;
		rgPropertyIDSets.guidPropertySet = DBPROPSET_DBINITALL;

		// Get the DBPROPSET_DBINITALL options supported
		if(SUCCEEDED(pIDBProperties->GetPropertyInfo(1, &rgPropertyIDSets, 
				&cPropertyInfoSets, &prgProperyInfoSets, &pDescBuffer)) )
		{
			// Set number of supported Properties
			g_cPropertyInfoSets = cPropertyInfoSets;
			g_cPropertyInfo = prgProperyInfoSets[0].cPropertyInfos;

			// Adjust for the provider specific propset
			if( cPropertyInfoSets > 1 && 
				prgProperyInfoSets[0].guidPropertySet != DBPROPSET_DBINIT )
				g_cPropertyInfo = prgProperyInfoSets[1].cPropertyInfos;

			// Check to see if 0 properties were returned
			if(	!g_cPropertyInfo )
				odtLog << L"Providers supports NO DBINIT Properties."<< ENDL;

			// Check DBPROPINFO
			for(ulIndex=0; ulIndex < prgProperyInfoSets->cPropertyInfos; ulIndex++)
			{
				// If PROPID doesn't match continue
				for(ulIndex1=0; ulIndex1 < g_cMaxPropertyInfoSets; ulIndex1++)
				{
					if( !memcmp(&g_rgDBInitDBProps[ulIndex1].dwPropertyID, 
								&prgProperyInfoSets->rgPropertyInfos[ulIndex].dwPropertyID, sizeof(DBPROPID)) )
					{	
						// ALL Properties returned with non 0 should be supported
						if( prgProperyInfoSets->rgPropertyInfos[ulIndex].dwFlags )
						{
							// Set the Supported Flag
							odtLog << prgProperyInfoSets->rgPropertyInfos[ulIndex].pwszDescription<< L" is a SUPPORTED Property."<< ENDL;
							g_rgDBInitDBProps[ulIndex1].fSupported = TRUE;
							ulSupported++;

							// Set the Settable Flag
							if( prgProperyInfoSets->rgPropertyInfos[ulIndex].dwFlags & DBPROPFLAGS_WRITE )
								g_rgDBInitDBProps[ulIndex1].fSettable = TRUE;

							// Set the Required Flag
							if( prgProperyInfoSets->rgPropertyInfos[ulIndex].dwFlags & DBPROPFLAGS_REQUIRED )
							{
								g_rgDBInitDBProps[ulIndex1].fRequired = TRUE;
								odtLog << prgProperyInfoSets->rgPropertyInfos[ulIndex].pwszDescription<< L" is a REQUIRED Property."<< ENDL;
							}
						}

						break;
					}
				}
			}
		}

		// Print out the descriptions of the Provider Specific Properties
		if(cPropertyInfoSets > 1)
		{
			// Loop thru the PropertyInfo Sets
			for(ulIndex=0; ulIndex < cPropertyInfoSets; ulIndex++)
			{
				if (prgProperyInfoSets[ulIndex].guidPropertySet == DBPROPSET_DBINIT)
					continue;

				// Display all of the Provider Specific Properties
				g_ulProviderSpecific++;

				for(ulIndex1=0; ulIndex1 < prgProperyInfoSets[ulIndex].cPropertyInfos; ulIndex1++)
					odtLog  << prgProperyInfoSets[ulIndex].rgPropertyInfos[ulIndex1].pwszDescription
							<< L" is a SUPPORTED Provider Specific Property."<< ENDL;
			}
		}

		// Check to see if all of the Properties where supported
		if(ulSupported != g_cPropertyInfo)
			odtLog << L"ALL DBPROP's returned should be supported and were not."<< ENDL;

		// Free prgProperyInfoSets
		FreeProperties(&cPropertyInfoSets, &prgProperyInfoSets, &pDescBuffer);
		SAFE_RELEASE(pIDBProperties);

		//Free the interface we got in ModuleCreateDBSession()
		ModuleReleaseDBSession(pThisTestModule);
		CreateModInfo(pThisTestModule);
		return TRUE;
	}

CLEANUP:
	//Free the interface we got in ModuleCreateDBSession()
	ModuleReleaseDBSession(pThisTestModule);
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
	//Cleanup the values array for Initialization
	FreeProperties(&g_cPropertySets, &g_rgPropertySets);
	return ReleaseModInfo(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCIDBInit Base Class for IDBInitialize:Initialize Testcases
class TCIDBInit : public CRowsetObject
{
	public:
		// @cmember Constructor
		TCIDBInit(LPWSTR wstrTestCaseName) : CRowsetObject(wstrTestCaseName)
		{
			m_hr					= E_FAIL;
			m_pIDBCreateSession		= NULL;
			m_pIDBInitialize		= NULL;
			m_pIDBProperties		= NULL;
			m_cPropertyIDSets		= 0;
			m_cPropertyInfoSets		= 0;
			m_prgPropertyInfoSets	= NULL;
			m_pDescBuffer			= NULL;
			m_ulIndex				= 0;
			m_lValue				= 0;
			m_rgPropertyIDs			= NULL;
		};

		// @cmember Destructor
		virtual ~TCIDBInit(){};

	protected:
		// @cmember IDBCreateSession Interface
		IDBCreateSession*	m_pIDBCreateSession;
		// @cmember IDBProperties Interface
		IDBProperties*		m_pIDBProperties;
		// @cmember number of PropertyIDSets
		ULONG				m_cPropertyIDSets;
		// @cmember array of PropertyIDSets
		DBPROPIDSET			m_rgPropertyIDSets[20];
		// @cmember number of PropertyInfoSets returned
		ULONG				m_cPropertyInfoSets;
		// @cmember array of ProperyInfoSets for Initialize
		DBPROPINFOSET *		m_prgPropertyInfoSets;
		// @cmember array of description values for Initialize
		OLECHAR *			m_pDescBuffer;
		// @cmember array of DBPROPIDs
		DBPROPID *			m_rgPropertyIDs;
		// @cmember Index
		ULONG				m_ulIndex;
		// @cmember ulong Value
		ULONG_PTR			m_lValue;
		// @cmember ulong Value
		WCHAR				m_wszBuffer[255];
		// @func	SetDBProperties
		HRESULT				CheckDBProperty(DBPROPID dwPropertyID, void* pv, BOOL fBadValue=FALSE);

};


// @class TCZOMBIE Base Class for IDBInitialize::Zombie Testcases
class TCZOMBIE : public CTransaction
{
	public:
		// @cmember Constructor
		TCZOMBIE(LPWSTR wstrTestCaseName) : CTransaction(wstrTestCaseName)
		{
			m_pIDBInitialize		= NULL;
			m_cPropertyInfoSets		= 0;
			m_prgPropertyInfoSets	= NULL;
			m_pDescBuffer			= NULL;
			m_ulIndex				= 0;
			m_rgPropertyIDs			= NULL;
		};

		// @cmember Destructor
		virtual ~TCZOMBIE(){};

	protected:
		// @cmember IDBInitialize Interface
		IDBInitialize*		m_pIDBInitialize;
		// @cmember number of PropertyInfoSets returned
		ULONG				m_cPropertyInfoSets;
		// @cmember array of ProperyInfoSets for Initialize
		DBPROPINFOSET *		m_prgPropertyInfoSets;
		// @cmember array of description values for Initialize
		WCHAR *				m_pDescBuffer;
		// @cmember array of DBPROPIDs
		DBPROPID *			m_rgPropertyIDs;
		// @cmember Index
		ULONG				m_ulIndex;
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIDBInit_GetPropertyInfo)
//--------------------------------------------------------------------
// @class IDBProperties::GetPropertyInfo
//
class TCIDBInit_GetPropertyInfo : public TCIDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_GetPropertyInfo,TCIDBInit);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember Check the DBPROPINFOSET and DescBuffer
	void CheckDBPropInfoSet(BOOL fInitAll=FALSE);
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - DBPROPSET_DBINIT with NULL pcPropertyInfoSets
	int Variation_1();
	// @cmember E_INVALIDARG - DBPROPSET_DBINIT with NULL prgProperyInfoSets
	int Variation_2();
	// @cmember E_INVALIDARG - DBPROPSET_DBINIT with cPropertyIDSets = 1 and a NULL rgPropertyIDSets
	int Variation_3();
	// @cmember E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0
	int Variation_4();
	// @cmember E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet
	int Variation_5();
	// @cmember S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a NULL array of OPT's
	int Variation_6();
	// @cmember S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a valid array of OPT's
	int Variation_7();
	// @cmember S_OK - DBPROPSET_DBINIT with ALL valid OPT's
	int Variation_8();
	// @cmember DB_E_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL invalid OPT's
	int Variation_9();
	// @cmember DB_S_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL OPT's
	int Variation_10();
	// @cmember E_INVALIDARG - DBPROPSET_DBINITALL with NULL pcPropertyInfoSets
	int Variation_11();
	// @cmember E_INVALIDARG - DBPROPSET_DBINITALL with NULL prgProperyInfoSets
	int Variation_12();
	// @cmember E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0
	int Variation_13();
	// @cmember E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet
	int Variation_14();
	// @cmember S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a NULL array of OPT's
	int Variation_15();
	// @cmember S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a valid array of OPT's
	int Variation_16();
	// @cmember S_OK - DBPROPSET_DBINITALL with ALL valid OPT's
	int Variation_17();
	// @cmember S_OK - DBPROPSET_DBINITALL with ALL invalid OPT's
	int Variation_18();
	// @cmember S_OK - DBPROPSET_DBINITALL with ALL OPT's
	int Variation_19();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_GetPropertyInfo)
#define THE_CLASS TCIDBInit_GetPropertyInfo
BEG_TEST_CASE(TCIDBInit_GetPropertyInfo, TCIDBInit, L"IDBProperties::GetPropertyInfo")
	TEST_VARIATION(1, 		L"E_INVALIDARG - DBPROPSET_DBINIT with NULL pcPropertyInfoSets")
	TEST_VARIATION(2, 		L"E_INVALIDARG - DBPROPSET_DBINIT with NULL prgProperyInfoSets")
	TEST_VARIATION(3, 		L"E_INVALIDARG - DBPROPSET_DBINIT with cPropertyIDSets = 1 and a NULL rgPropertyIDSets")
	TEST_VARIATION(4, 		L"E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0")
	TEST_VARIATION(5, 		L"E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet")
	TEST_VARIATION(6, 		L"S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a NULL array of OPT's")
	TEST_VARIATION(7, 		L"S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a valid array of OPT's")
	TEST_VARIATION(8, 		L"S_OK - DBPROPSET_DBINIT with ALL valid OPT's")
	TEST_VARIATION(9, 		L"DB_E_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL invalid OPT's")
	TEST_VARIATION(10, 		L"DB_S_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL OPT's")
	TEST_VARIATION(11, 		L"E_INVALIDARG - DBPROPSET_DBINITALL with NULL pcPropertyInfoSets")
	TEST_VARIATION(12, 		L"E_INVALIDARG - DBPROPSET_DBINITALL with NULL prgProperyInfoSets")
	TEST_VARIATION(13, 		L"E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0")
	TEST_VARIATION(14, 		L"E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet")
	TEST_VARIATION(15, 		L"S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a NULL array of OPT's")
	TEST_VARIATION(16, 		L"S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a valid array of OPT's")
	TEST_VARIATION(17, 		L"S_OK - DBPROPSET_DBINITALL with ALL valid OPT's")
	TEST_VARIATION(18, 		L"S_OK - DBPROPSET_DBINITALL with ALL invalid OPT's")
	TEST_VARIATION(19, 		L"S_OK - DBPROPSET_DBINITALL with ALL OPT's")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBInit_Initialize)
//--------------------------------------------------------------------
// @class IDBInitialize::Initialize
//
class TCIDBInit_Initialize : public TCIDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_Initialize,TCIDBInit);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - NULL prgPropertySets
	int Variation_1();
	// @cmember E_INVALIDARG - NULL rgProperties and cProperties != 0
	int Variation_2();
	// @cmember E_INVALIDARG - NULL rgProperties and cProperties != 0 in 2nd PropertySet
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_Initialize)
#define THE_CLASS TCIDBInit_Initialize
BEG_TEST_CASE(TCIDBInit_Initialize, TCIDBInit, L"IDBInitialize::Initialize")
	TEST_VARIATION(1, 		L"E_INVALIDARG - NULL prgPropertySets")
	TEST_VARIATION(2, 		L"E_INVALIDARG - NULL rgProperties and cProperties != 0")
	TEST_VARIATION(3, 		L"E_INVALIDARG - NULL rgProperties and cProperties != 0 in 2nd PropertySet")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBInit_Uninitialize)
//--------------------------------------------------------------------
// @class IDBInitialize::Uninitialize
//
class TCIDBInit_Uninitialize : public TCIDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_Uninitialize,TCIDBInit);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_OBJECTOPEN - Uninitialize with a DBSession Open
	int Variation_1();
	// @cmember DB_E_OBJECTOPEN - Uninitialize with a Command Open
	int Variation_2();
	// @cmember DB_E_OBJECTOPEN - Uninitialize with a Rowset Open
	int Variation_3();
	// @cmember S_OK - Uninitialize after an Initialize
	int Variation_4();
	// @cmember S_OK - Uninitialize after an Uninitialize
	int Variation_5();
	// @cmember S_OK - Uninitialize a clean IDBInitailize Pointer
	int Variation_6();
	// @cmember S_OK - Using two IDBInitialize Pointers
	int Variation_7();
	// @cmember DB_E_ERRORSOCCURRED - Ask for DBPROPSET_DATASOURCEINFO after Uninitialize
	int Variation_8();
	// @cmember DB_S_ERRORSOCCURRED - Set an extra Initialize property
	int Variation_9();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_Uninitialize)
#define THE_CLASS TCIDBInit_Uninitialize
BEG_TEST_CASE(TCIDBInit_Uninitialize, TCIDBInit, L"IDBInitialize::Uninitialize")
	TEST_VARIATION(1, 		L"DB_E_OBJECTOPEN - Uninitialize with a DBSession Open")
	TEST_VARIATION(2, 		L"DB_E_OBJECTOPEN - Uninitialize with a Command Open")
	TEST_VARIATION(3, 		L"DB_E_OBJECTOPEN - Uninitialize with a Rowset Open")
	TEST_VARIATION(4, 		L"S_OK - Uninitialize after an Initialize")
	TEST_VARIATION(5, 		L"S_OK - Uninitialize after an Uninitialize")
	TEST_VARIATION(6, 		L"S_OK - Uninitialize a clean IDBInitailize Pointer")
	TEST_VARIATION(7, 		L"S_OK - Using two IDBInitialize Pointers")
	TEST_VARIATION(8, 		L"DB_E_ERRORSOCCURRED - Ask for DBPROPSET_DATASOURCEINFO after Uninitialize")
	TEST_VARIATION(9, 		L"DB_S_ERRORSOCCURRED - Set an extra Initialize property")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBInit_InitProperties)
//--------------------------------------------------------------------
// @class IDBInitialize::Initialize
//
class TCIDBInit_InitProperties : public TCIDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_InitProperties,TCIDBInit);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Set and Get DBPROP_AUTH_CACHE_AUTHINFO
	int Variation_1();
	// @cmember S_OK - Set and Get DBPROP_AUTH_ENCRYPT_PASSWORD
	int Variation_2();
	// @cmember S_OK - Set and Get DBPROP_AUTH_INTEGRATED
	int Variation_3();
	// @cmember S_OK - Set and Get DBPROP_AUTH_MASK_PASSWORD
	int Variation_4();
	// @cmember S_OK - Set and Get DBPROP_AUTH_PASSWORD
	int Variation_5();
	// @cmember S_OK - Set and Get DBPROP_AUTH_PERSIST_ENCRYPTED
	int Variation_6();
	// @cmember S_OK - Set and Get DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO
	int Variation_7();
	// @cmember S_OK - Set and Get DBPROP_AUTH_USERID
	int Variation_8();
	// @cmember S_OK - Set and Get DBPROP_INIT_ASYNCH
	int Variation_9();
	// @cmember S_OK - Set and Get DBPROP_INIT_CATALOG
	int Variation_10();
	// @cmember S_OK - Set and Get DBPROP_INIT_DATASOURCE
	int Variation_11();
	// @cmember S_OK - Set and Get DBPROP_INIT_HWND
	int Variation_12();
	// @cmember S_OK - Set and Get DBPROP_INIT_IMPERSONATION_LEVEL
	int Variation_13();
	// @cmember S_OK - Set and Get DBPROP_INIT_LCID
	int Variation_14();
	// @cmember S_OK - Set and Get DBPROP_INIT_LOCATION
	int Variation_15();
	// @cmember S_OK - Set and Get DBPROP_INIT_MODE
	int Variation_16();
	// @cmember S_OK - Set and Get DBPROP_INIT_PROMPT
	int Variation_17();
	// @cmember S_OK - Set and Get DBPROP_INIT_PROTECTION_LEVEL
	int Variation_18();
	// @cmember S_OK - Set and Get DBPROP_INIT_PROVIDERSTRING
	int Variation_19();
	// @cmember S_OK - Set and Get DBPROP_INIT_TIMEOUT
	int Variation_20();
	// @cmember S_OK - Set and Get DBPROP_INIT_OLEDBSERVICES
	int Variation_21();
	// @cmember S_OK - Set and Get DBPROP_INIT_BINDFLAGS
	int Variation_22();
	// @cmember S_OK - Set and Get DBPROP_INIT_LOCKOWNER
	int Variation_23();
	// @cmember S_OK - Set and Get DBPROP_INIT_GENERALTIMEOUT
	int Variation_24();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_InitProperties)
#define THE_CLASS TCIDBInit_InitProperties
BEG_TEST_CASE(TCIDBInit_InitProperties, TCIDBInit, L"IDBProperties::SetProperties")
	TEST_VARIATION(1, 		L"S_OK - Set and Get DBPROP_AUTH_CACHE_AUTHINFO")
	TEST_VARIATION(2, 		L"S_OK - Set and Get DBPROP_AUTH_ENCRYPT_PASSWORD")
	TEST_VARIATION(3, 		L"S_OK - Set and Get DBPROP_AUTH_INTEGRATED")
	TEST_VARIATION(4, 		L"S_OK - Set and Get DBPROP_AUTH_MASK_PASSWORD")
	TEST_VARIATION(5, 		L"S_OK - Set and Get DBPROP_AUTH_PASSWORD")
	TEST_VARIATION(6, 		L"S_OK - Set and Get DBPROP_AUTH_PERSIST_ENCRYPTED")
	TEST_VARIATION(7, 		L"S_OK - Set and Get DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO")
	TEST_VARIATION(8, 		L"S_OK - Set and Get DBPROP_AUTH_USERID")
	TEST_VARIATION(9, 		L"S_OK - Set and Get DBPROP_INIT_ASYNCH")
	TEST_VARIATION(10, 		L"S_OK - Set and Get DBPROP_INIT_CATALOG")
	TEST_VARIATION(11, 		L"S_OK - Set and Get DBPROP_INIT_DATASOURCE")
	TEST_VARIATION(12, 		L"S_OK - Set and Get DBPROP_INIT_HWND")
	TEST_VARIATION(13, 		L"S_OK - Set and Get DBPROP_INIT_IMPERSONATION_LEVEL")
	TEST_VARIATION(14, 		L"S_OK - Set and Get DBPROP_INIT_LCID")
	TEST_VARIATION(15, 		L"S_OK - Set and Get DBPROP_INIT_LOCATION")
	TEST_VARIATION(16, 		L"S_OK - Set and Get DBPROP_INIT_MODE")
	TEST_VARIATION(17, 		L"S_OK - Set and Get DBPROP_INIT_PROMPT")
	TEST_VARIATION(18, 		L"S_OK - Set and Get DBPROP_INIT_PROTECTION_LEVEL")
	TEST_VARIATION(19, 		L"S_OK - Set and Get DBPROP_INIT_PROVIDERSTRING")
	TEST_VARIATION(20, 		L"S_OK - Set and Get DBPROP_INIT_TIMEOUT")
	TEST_VARIATION(21, 		L"S_OK - Set and Get DBPROP_INIT_OLEDBSERVICES")
	TEST_VARIATION(22, 		L"S_OK - Set and Get DBPROP_INIT_BINDFLAGS")
	TEST_VARIATION(23, 		L"S_OK - Set and Get DBPROP_INIT_LOCKOWNER")
	TEST_VARIATION(24, 		L"S_OK - Set and Get DBPROP_INIT_GENERALTIMEOUT")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBInit_Zombie)
//--------------------------------------------------------------------
// @class IDBInitialize::Zombie
//
class TCIDBInit_Zombie : public TCZOMBIE { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_Zombie,TCZOMBIE);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IDBInitialize::Initialize with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Commit IDBInitialize::Initialize with fRetaining=TRUE
	int Variation_2();
	// @cmember S_OK - Abort IDBInitialize::Initialize with fRetaining=FALSE
	int Variation_3();
	// @cmember S_OK - Commit IDBInitialize::Initialize with fRetaining=FALSE
	int Variation_4();
	// @cmember S_OK - Abort IDBInitialize::Uninitialize with fRetaining=TRUE
	int Variation_5();
	// @cmember S_OK - Commit IDBInitialize::Uninitialize with fRetaining=TRUE
	int Variation_6();
	// @cmember S_OK - Abort IDBInitialize::Uninitialize with fRetaining=FALSE
	int Variation_7();
	// @cmember S_OK - Commit IDBInitialize::Uninitialize with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_Zombie)
#define THE_CLASS TCIDBInit_Zombie
BEG_TEST_CASE(TCIDBInit_Zombie, TCZOMBIE, L"IDBInitialize::Zombie")
	TEST_VARIATION(1, 		L"S_OK - Abort IDBInitialize::Initialize with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Commit IDBInitialize::Initialize with fRetaining=TRUE")
	TEST_VARIATION(3, 		L"S_OK - Abort IDBInitialize::Initialize with fRetaining=FALSE")
	TEST_VARIATION(4, 		L"S_OK - Commit IDBInitialize::Initialize with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"S_OK - Abort IDBInitialize::Uninitialize with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"S_OK - Commit IDBInitialize::Uninitialize with fRetaining=TRUE")
	TEST_VARIATION(7, 		L"S_OK - Abort IDBInitialize::Uninitialize with fRetaining=FALSE")
	TEST_VARIATION(8, 		L"S_OK - Commit IDBInitialize::Uninitialize with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIDBInit_ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCIDBInit_ExtendedErrors : public TCIDBInit_Uninitialize { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIDBInit_ExtendedErrors,TCIDBInit_Uninitialize);
	// }} TCW_DECLARE_FUNCS_END
 	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Valid Initialize and Uninitialize calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid Initialize and Uninitialize calls with previous error object existing
	int Variation_2();
	// @cmember Invalid Initialize and Uninitialize calls with no previous error object existing
	int Variation_3();
	// @cmember Initialize with no properties set
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIDBInit_ExtendedErrors)
#define THE_CLASS TCIDBInit_ExtendedErrors
BEG_TEST_CASE(TCIDBInit_ExtendedErrors, TCIDBInit_Uninitialize, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid Initialize and Uninitialize calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid Initialize and Uninitialize calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid Initialize and Uninitialize calls with no previous error object existing")
	TEST_VARIATION(4, 		L"Initialize with no properties set")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(6, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIDBInit_GetPropertyInfo)
	TEST_CASE(2, TCIDBInit_Initialize)
	TEST_CASE(3, TCIDBInit_Uninitialize)
	TEST_CASE(4, TCIDBInit_InitProperties)
	TEST_CASE(5, TCIDBInit_Zombie)
	TEST_CASE(6, TCIDBInit_ExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIDBInit_GetPropertyInfo)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_GetPropertyInfo - IDBProperties::GetPropertyInfo
//|	Created:		06/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_GetPropertyInfo::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBInit::Init())
	// }}
	{
		// Create DataSource Object
		TESTC_(CreateDataSourceObject(), S_OK);

		// QI for a Session Object off of the m_pIDBProperties pointer
		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties));

		// Set the DBPROPIDSET Structure
		m_rgPropertyIDSets[0].cPropertyIDs    = 0;
		m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
		m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

		// Call the Method to see if it is supported
		m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
				&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);
			
		if( FAILED(m_hr) && !m_prgPropertyInfoSets->cPropertyInfos )
			odtLog <<L"Providers supports NO DBINIT Properties." <<ENDL;
		
		// Compare with Global Count
		COMPARE(g_cPropertyInfo, m_prgPropertyInfoSets->cPropertyInfos);

		// Free m_prgPropertyInfoSets
		FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINIT with NULL pcPropertyInfoSets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_1()
{	
	TBEGIN;

	// Initialize output variables
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for pcPropertyInfoSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
				NULL,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINIT with NULL prgProperyInfoSets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_2()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for rgPropertyInfoSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
				&m_cPropertyInfoSets,NULL,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINIT with cPropertyIDSets = 1 and a NULL rgPropertyIDSets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_3()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Pass in a NULL for rgPropertyIDSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,NULL,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_4()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 1;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for rgPropertyIDSets->rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINIT with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_5()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	m_rgPropertyIDSets[1].cPropertyIDs    = 1;
	m_rgPropertyIDSets[1].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[1].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for rgPropertyIDSets[1].rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(2,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a NULL array of OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_6()
{
	TBEGIN;
	HRESULT	Exphr = S_OK;

	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfo ) 
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a 0, NULL, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, NULL, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet();

CLEANUP:

	// Cleanup and return
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINIT with cOptions set to 0 with a valid array of OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_7()
{
	TBEGIN;
	ULONG	cPropertyIDs = 0;
	HRESULT	Exphr		 = S_OK;
	
	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfo )
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a 0, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet();

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINIT with ALL valid OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_8()
{
	TBEGIN;
	ULONG	cPropertyIDs = 0;
	HRESULT	Exphr		 = S_OK;
	
	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfo )
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet();

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL invalid OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_9()
{
	TBEGIN;
	ULONG cPropertyIDs = 0;

	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( !g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	if( !cPropertyIDs )
	{
		// Cleanup and return
		odtLog << L"ALL DBPROP's where supported."<< ENDL;
		PROVIDER_FREE(m_rgPropertyIDs);
		TRETURN;
	}

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), DB_E_ERRORSOCCURRED);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == 1);
	TESTC(m_prgPropertyInfoSets != NULL);
	TESTC(m_pDescBuffer == NULL);

	// Check information
	CheckDBPropInfoSet();

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - DBPROPSET_DBINIT with ALL OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_10()
{
	TBEGIN;
	ULONG	cPropertyIDs= 0;
	ULONG   cSupported  = 0;
	HRESULT ExpHR		= DB_S_ERRORSOCCURRED;

	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;

		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			cSupported++;
	}
	
	// Figure out the HResult
	if( g_cMaxPropertyInfoSets == cSupported )
		ExpHR = S_OK;

	if( !cSupported )
		ExpHR = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	
	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), ExpHR);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet();

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINITALL with NULL pcPropertyInfoSets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_11()
{
	TBEGIN;

	// Initialize output variables
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a NULL for pcPropertyInfoSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
				NULL,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINITALL with NULL prgProperyInfoSets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_12()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a NULL for rgPropertyInfoSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
				&m_cPropertyInfoSets,NULL,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_13()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 1;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a NULL for rgPropertyIDSets->rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - DBPROPSET_DBINITALL with NULL rgProperyIDs anc cProperty != 0 in 2nd PropertyIDSet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_14()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	m_rgPropertyIDSets[1].cPropertyIDs    = 1;
	m_rgPropertyIDSets[1].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[1].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a NULL for rgPropertyIDSets[1].rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(2,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a NULL array of OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_15()
{
	TBEGIN;
	HRESULT	Exphr = S_OK;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfoSets ) 
		Exphr = DB_E_ERRORSOCCURRED;

	// Pass in a 0, NULL, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, NULL, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == g_ulProviderSpecific + 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet(TRUE);

CLEANUP:

	// Cleanup and return
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINITALL with cOptions set to 0 with a valid array of OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_16()
{
	TBEGIN;
	ULONG	cPropertyIDs = 0;
	HRESULT	Exphr		 = S_OK;
	
	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfoSets )
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a 0, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == g_ulProviderSpecific + 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet(TRUE);

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINITALL with ALL valid OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_17()
{
	TBEGIN;
	ULONG	cPropertyIDs = 0;
	HRESULT	Exphr		 = S_OK;
	
	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfoSets )
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == g_ulProviderSpecific + 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet(TRUE);

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINITALL with ALL invalid OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_18()
{
	TBEGIN;
	ULONG	cPropertyIDs = 0;
	HRESULT	Exphr		 = S_OK;

	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( !g_rgDBInitDBProps[m_ulIndex].fSupported )
			m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;
	}
	
	// DB_E_ERRORSOCCURRED if no DBINIT properties are supported
	if( !g_cPropertyInfoSets )
		Exphr = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;

	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), Exphr);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == g_ulProviderSpecific + 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet(TRUE);

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK - DBPROPSET_DBINITALL with ALL OPT's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_GetPropertyInfo::Variation_19()
{
	TBEGIN;
	ULONG	cPropertyIDs= 0;
	ULONG   cSupported  = 0;
	HRESULT ExpHR		= S_OK;

	// Allocate memory for the array of DBPROPID's
	m_rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_cMaxPropertyInfoSets);
	TESTC(m_rgPropertyIDs != NULL);

	// Fill in the array with all valid OPT's
	for(m_ulIndex=0; m_ulIndex<g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		m_rgPropertyIDs[cPropertyIDs++] = g_rgDBInitDBProps[m_ulIndex].dwPropertyID;

		if( g_rgDBInitDBProps[m_ulIndex].fSupported )
			cSupported++;
	}

	// Figure out the HResult
	if( !g_cPropertyInfoSets )
		ExpHR = DB_E_ERRORSOCCURRED;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = cPropertyIDs;
	m_rgPropertyIDSets[0].rgPropertyIDs   = m_rgPropertyIDs;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINITALL;
	
	// Pass in a COUNT, VALID, PROPSET to get all supported DBINIT Properties 
	TESTC_(m_hr=m_pIDBProperties->GetPropertyInfo(1, m_rgPropertyIDSets, 
		&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer), ExpHR);

	// All output variables should be set since 0, VALID, DBPROPSET_DBINIT
	TESTC(VerifyPropertyInfo(m_hr, m_cPropertyInfoSets, m_prgPropertyInfoSets, m_pDescBuffer));
	TESTC(m_cPropertyInfoSets == g_ulProviderSpecific + 1);
	TESTC(m_prgPropertyInfoSets != NULL);

	// Check information
	CheckDBPropInfoSet(TRUE);

CLEANUP:

	// Cleanup and return
	PROVIDER_FREE(m_rgPropertyIDs);
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_GetPropertyInfo::Terminate()
{
	// Relase Objects
	SAFE_RELEASE(m_pIDBProperties);
	SAFE_RELEASE(m_pIDBInitialize);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBInit::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBInit_Initialize)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_Initialize - IDBInitialize::Initialize
//|	Created:		06/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Initialize::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBInit::Init())
	// }}
	{
		// Create DataSource Object
		TESTC_(CreateDataSourceObject(), S_OK);

		// QI for a Session Object off of the m_pIDBProperties pointer
		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties));
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL prgPropertySets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Initialize::Variation_1()
{
	TBEGIN;
	
	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Pass in a NULL for rgPropertyIDSets 
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,NULL,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);

	// Not setting any Properties for Initialize
	m_hr=m_pIDBInitialize->Initialize();
		
	// Check DBPROPSET_PROPERTIESINERROR
	TESTC(VerifyPropertiesInError(m_hr, m_pIDBInitialize));

	// Check the ReturnCode
	TEST4C_(m_hr, S_OK, E_FAIL, DB_SEC_E_AUTH_FAILED, DB_E_ERRORSOCCURRED);

CLEANUP:
	// Uninitialize just in case for next variation
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	// Free the properties
	FreeProperties(&m_cPropertyInfoSets, &m_prgPropertyInfoSets, &m_pDescBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperties and cProperties != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Initialize::Variation_2()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 1;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for rgPropertyIDSets->rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(1,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);

	// Not setting any Properties for Initialize
	m_hr=m_pIDBInitialize->Initialize();
	
	// Check DBPROPSET_PROPERTIESINERROR
	TESTC(VerifyPropertiesInError(m_hr, m_pIDBInitialize));

	// Check the ReturnCode
	TEST4C_(m_hr, S_OK, E_FAIL, DB_SEC_E_AUTH_FAILED, DB_E_ERRORSOCCURRED);

CLEANUP:

	// Uninitialize just in case for next variation
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperties and cProperties != 0 in 2nd PropertySet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Initialize::Variation_3()
{
	TBEGIN;

	// Initialize output variables
	m_cPropertyInfoSets   = INVALID(ULONG);
	m_prgPropertyInfoSets = INVALID(DBPROPINFOSET*);
	m_pDescBuffer         = INVALID(OLECHAR*);

	// Set the DBPROPIDSET Structure
	m_rgPropertyIDSets[0].cPropertyIDs    = 0;
	m_rgPropertyIDSets[0].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	m_rgPropertyIDSets[1].cPropertyIDs    = 1;
	m_rgPropertyIDSets[1].rgPropertyIDs   = NULL;
	m_rgPropertyIDSets[1].guidPropertySet = DBPROPSET_DBINIT;

	// Pass in a NULL for rgPropertyIDSets[1].rgPropertyIDs
	TESTC_(m_pIDBProperties->GetPropertyInfo(2,m_rgPropertyIDSets,
		&m_cPropertyInfoSets,&m_prgPropertyInfoSets,&m_pDescBuffer), E_INVALIDARG);

	// All output variables should be reset on E_INVALIDARG
	TESTC(!m_cPropertyInfoSets);
	TESTC(!m_prgPropertyInfoSets);
	TESTC(!m_pDescBuffer);

	// Not setting any Properties for Initialize
	m_hr=m_pIDBInitialize->Initialize();

	// Check DBPROPSET_PROPERTIESINERROR
	TESTC(VerifyPropertiesInError(m_hr, m_pIDBInitialize));

	// Check the ReturnCode
	TEST4C_(m_hr, S_OK, E_FAIL, DB_SEC_E_AUTH_FAILED, DB_E_ERRORSOCCURRED);

CLEANUP:

	// Uninitialize just in case for next variation
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Initialize::Terminate()
{
	// Relase Objects
	SAFE_RELEASE(m_pIDBProperties);
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBInit::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBInit_Uninitialize)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_Uninitialize - IDBInitialize::Uninitialize
//|	Created:		06/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Uninitialize::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBInit::Init())
	// }}
	{
		// Create DataSource Object
		TESTC_(CreateDataSourceObject(), S_OK);

		// Initialize the DSO
		TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

		// QI for a Session Object off of the m_pIDBInitialize pointer
		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession));
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Uninitialize with a DBSession Open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_1()
{
	TBEGIN;
	IUnknown* pDBSession = NULL;

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a DBSession
	TESTC_(m_pIDBCreateSession->CreateSession(NULL,IID_IOpenRowset,
											 (IUnknown**)&pDBSession), S_OK);

	// Uninitialize a DSO with a open Session
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:
	
	// Release Session and Uninitialize
	SAFE_RELEASE(pDBSession);
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Uninitialize with a Command Open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_2()
{
	TBEGIN;
	IDBCreateCommand* pIDBCreateCommand	= NULL;
	ICommand*		  pICommand			= NULL;

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a DBSession
	m_hr=m_pIDBCreateSession->CreateSession(NULL,IID_IDBCreateCommand,
										   (IUnknown**)&pIDBCreateCommand);
	
	// Commands are supported
	if( SUCCEEDED(m_hr) )
	{
		// Create a Command
		TESTC_(pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
											   (IUnknown**)&pICommand), S_OK);
		// Release Session
		SAFE_RELEASE(pIDBCreateCommand);

		// Uninitialize a DSO with a open Command
		TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);
	}
	else
	{
		TESTC_(m_hr, E_NOINTERFACE );
		TESTC(pIDBCreateCommand == NULL);
	}

CLEANUP:

	// Release Session
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommand);

	// Uninitialize after releasing the Session
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN - Uninitialize with a Rowset Open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_3()
{
	TBEGIN;

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a DBSession
	TESTC_(m_pIDBCreateSession->CreateSession(NULL,IID_IOpenRowset,
										(IUnknown**)&m_pIOpenRowset), S_OK);

	// Create the rowset object.
	TESTC_(CreateRowsetObject(USE_OPENROWSET), S_OK);

	// Uninitialize a DSO with a open Command
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:

	// Cleanup the Rowset Objects created
	ReleaseRowsetObject();
	ReleaseDBSession();

	// Clean up the Table
	if( m_pTable )
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// Uninitialize
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Uninitialize after an Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_4()
{
	TBEGIN;

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));
		
	// Uninitialize a clean DSO
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Uninitialize after an Uninitialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_5()
{
	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));
		
	// Uninitialize a clean DSO
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);

	// Uninitialize a non Initialized DSO
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Uninitialize a clean IDBInitailize Pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_6()
{
	TBEGIN;
	IDBInitialize* pIDBInitialize = NULL;

	// Create a new Session Object
	TESTC_(CoCreateInstance(m_ProviderClsid, NULL, 
			m_clsctxProvider, IID_IDBInitialize,(void **)&pIDBInitialize), S_OK);

	// Uninitialize a Clean DSO
	TESTC_(pIDBInitialize->Uninitialize(), S_OK);

CLEANUP:

	// Release IID_IDBInitialize and Uninitialize
	SAFE_RELEASE(pIDBInitialize);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Using two IDBInitialize Pointers
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_7()
{
	TBEGIN;
	IDBInitialize*	pIDBInitialize = NULL;
	IDBProperties*	pIDBProperties = NULL;

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a new Session Object
	TESTC_(CoCreateInstance(m_ProviderClsid, NULL, 
			m_clsctxProvider, IID_IDBInitialize,(void **)&pIDBInitialize), S_OK);

	// Get the IDBProperties Interface
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, 
					DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Set the IDBProperties Interface
	TESTC_(pIDBProperties->SetProperties(g_cPropertySets, g_rgPropertySets), S_OK);

	// Call Initialize with nothing set
	m_hr=pIDBInitialize->Initialize();

	// Check the ReturnCode
	TEST3C_(m_hr, S_OK, E_FAIL, DB_SEC_E_AUTH_FAILED);

	// Uninitialize a Clean DSO
	TESTC_(pIDBInitialize->Uninitialize(), S_OK);

	// Initialize the old DSO
	TESTC_(m_pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Release IID_IDBProperties
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - Ask for DBPROPSET_DATASOURCEINFO after Uninitialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_8()
{
	TBEGIN;
	DBPROPID	rgPropertyIDs   = DBPROP_PROVIDERNAME;
	ULONG		pcPropertySets  = 0;
	DBPROPSET *	prgPropertySets = NULL;
	
	// Setup the PropertyIDSet
	m_rgPropertyIDSets->cPropertyIDs    = 1;
	m_rgPropertyIDSets->rgPropertyIDs   = &rgPropertyIDs;
	m_rgPropertyIDSets->guidPropertySet = DBPROPSET_DATASOURCEINFO;
	
	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Uninitialize just in case for next variation
	TESTC_(m_pIDBInitialize->Uninitialize(), S_OK);

	// Get the IDBProperties Interface
	TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
					DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties));

	// Call GetProperties 
	TESTC_(m_hr=m_pIDBProperties->GetProperties(1, m_rgPropertyIDSets,
					&pcPropertySets, &prgPropertySets), DB_E_ERRORSOCCURRED);

	TESTC(VerifyProperties(m_hr, pcPropertySets, prgPropertySets, FALSE));
	
	// Everything should get set
	TESTC(pcPropertySets == 1);
	TESTC(prgPropertySets != NULL);
	TESTC(prgPropertySets->cProperties == 1);
	TESTC(prgPropertySets->rgProperties != NULL);
	TESTC(prgPropertySets->rgProperties->dwPropertyID == DBPROP_PROVIDERNAME);
	TESTC(prgPropertySets->rgProperties->dwStatus == DBPROPSTATUS_NOTSUPPORTED);
	TESTC(prgPropertySets->rgProperties->vValue.vt == VT_EMPTY);

CLEANUP:

	// Uninitialize just in case for next variation
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	//Cleanup Memory
	FreeProperties(&pcPropertySets, &prgPropertySets);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - Set an extra Initialize property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Uninitialize::Variation_9()
{
	TBEGIN;

	HRESULT			ExpHR			= DB_S_ERRORSOCCURRED;
	IDBInitialize*	pIDBInitialize	= NULL;
	IDBProperties*	pIDBProperties	= NULL;
	ULONG			cPropSets		= 0;
	DBPROPSET*		rgPropSets		= NULL;
	WCHAR*			wszProvInitStr	= NULL;

	// Create a new Session Object
	TESTC_(m_hr=CoCreateInstance(m_ProviderClsid, NULL, 
			m_clsctxProvider, IID_IDBInitialize,(void **)&pIDBInitialize), S_OK);

	// Get the IDBProperties Interface
	TESTC(SUCCEEDED(pIDBInitialize->QueryInterface(IID_IDBProperties,
												  (LPVOID*)&pIDBProperties)));

	// Get the Init Properties and add 1 not supported property
	TESTC(GetInitProps(&cPropSets, &rgPropSets));

	// Change the HResult
	if( !cPropSets )
		ExpHR = DB_E_ERRORSOCCURRED;

	SetProperty(1, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, 
				DBTYPE_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);

	// Set the IDBProperties Interface
	TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets), ExpHR);

	// Initialize the DSO
	TESTC_(pIDBInitialize->Initialize(), S_OK);

	// Check the DBPROP_INIT_PROVIDERSTRING property
	if(GetProperty(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT,
							pIDBInitialize, &wszProvInitStr) && !wszProvInitStr)
		TWARNING("DBPROP_INIT_PROVIDERSTRING = You may want to fill in this property after initialize.");

CLEANUP:
	
	// Release IID_IDBProperties
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	PROVIDER_FREE(wszProvInitStr);

	//Cleanup Memory
	FreeProperties(&cPropSets, &rgPropSets);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Uninitialize::Terminate()
{
	// Release Objects
	ReleaseDataSourceObject();

	SAFE_RELEASE(m_pIDBProperties);
	SAFE_RELEASE(m_pIDBCreateSession);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBInit::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc CheckDBPropInfoSet.  It should be called after GetDBInitProperties.
// The function checks information returned in DBPROPINFOSET.
// 
// @rdesc Success or Failure
//		@flag TRUE | CleanUp was successful.
//--------------------------------------------------------------------
HRESULT TCIDBInit::CheckDBProperty(DBPROPID dwPropertyID, void* pv, BOOL fBadValue)
{	
	HRESULT		hr		   = E_FAIL;
	ULONG		cPropSets  = 0;
	DBPROPSET*	rgPropSets = NULL;

	// Set the properties that will initialize the Provider
	TESTC_(m_pIDBProperties->SetProperties(g_cPropertySets, g_rgPropertySets), S_OK);

	// Find the correct Property ID
	for(m_ulIndex=0; m_ulIndex < g_cMaxPropertyInfoSets; m_ulIndex++)
	{
		if( g_rgDBInitDBProps[m_ulIndex].dwPropertyID == dwPropertyID )
			break;
	}
	
	// Assert on an unknown PropertyID
	TESTC(m_ulIndex < g_cMaxPropertyInfoSets);

	// Set to VARIANT_TRUE and DBPROPOPTIONS_REQUIRED
	TESTC_(SetProperty(g_rgDBInitDBProps[m_ulIndex].dwPropertyID, DBPROPSET_DBINIT, 
						&cPropSets, &rgPropSets,
						g_rgDBInitDBProps[m_ulIndex].vtPropType,pv,DBPROPOPTIONS_REQUIRED),S_OK);

	// Set the IDBProperties Interface
	TEST2C_(hr=m_pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED);
	
	if( !g_rgDBInitDBProps[m_ulIndex].fSupported )
	{
		TESTC_(hr, DB_E_ERRORSOCCURRED);
		TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSUPPORTED);
	}
	else if( !g_rgDBInitDBProps[m_ulIndex].fSettable )
	{
		if( fBadValue )
		{
			TESTC_(hr, DB_E_ERRORSOCCURRED);
			if( rgPropSets->rgProperties->dwStatus != DBPROPSTATUS_BADVALUE &&
				rgPropSets->rgProperties->dwStatus != DBPROPSTATUS_NOTSETTABLE )
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_BADVALUE);
		}
		else
		{
			VARIANT vVariant;
			VariantInit(&vVariant);

			GetProperty(g_rgDBInitDBProps[m_ulIndex].dwPropertyID, 
							DBPROPSET_DBINIT, m_pIDBProperties, &vVariant);
			
			if(	(V_VT(&vVariant) == VT_BSTR && wcscmp(V_BSTR(&vVariant), (WCHAR*)pv)) ||
				(V_VT(&vVariant) == VT_BOOL && V_BOOL(&vVariant) != *(VARIANT_BOOL*)pv)       ||
				(V_VT(&vVariant) == VT_I2 && V_I2(&vVariant) != *(SHORT*)pv)          ||
				(V_VT(&vVariant) == VT_I4 && V_I4(&vVariant) != *(LONG*)pv) )
			{
				TESTC_(hr, DB_E_ERRORSOCCURRED);
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE);
			}
			else
			{
				TESTC_(hr, S_OK);
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_OK);
			}
		}
	}
	else
	{
		if( fBadValue || FAILED(hr) )
		{
			TESTC_(hr, DB_E_ERRORSOCCURRED);
			TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_BADVALUE);
		}
		else
		{
			TESTC_(hr, S_OK);
			TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_OK);
		}
	}

	FreeProperties(&cPropSets, &rgPropSets);

	// Set to VARIANT_TRUE and DBPROPOPTIONS_OPTIONAL
	TESTC_(SetProperty(g_rgDBInitDBProps[m_ulIndex].dwPropertyID, DBPROPSET_DBINIT, 
						&cPropSets, &rgPropSets,
						g_rgDBInitDBProps[m_ulIndex].vtPropType,pv,DBPROPOPTIONS_OPTIONAL),S_OK);

	TEST2C_(hr=m_pIDBProperties->SetProperties(cPropSets, rgPropSets), S_OK, DB_E_ERRORSOCCURRED);
	if( !g_rgDBInitDBProps[m_ulIndex].fSupported )
	{
		TESTC_(hr, DB_E_ERRORSOCCURRED);
		TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSUPPORTED);
	}
	else if( !g_rgDBInitDBProps[m_ulIndex].fSettable )
	{
		if( fBadValue )
		{
			TESTC_(hr, DB_E_ERRORSOCCURRED);
			if( rgPropSets->rgProperties->dwStatus != DBPROPSTATUS_BADVALUE &&
				rgPropSets->rgProperties->dwStatus != DBPROPSTATUS_NOTSET )
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_BADVALUE);
		}
		else
		{
			VARIANT vVariant;
			VariantInit(&vVariant);

			GetProperty(g_rgDBInitDBProps[m_ulIndex].dwPropertyID, 
							DBPROPSET_DBINIT, m_pIDBProperties, &vVariant);

			if(	(V_VT(&vVariant) == VT_BSTR && wcscmp(V_BSTR(&vVariant), (WCHAR*)pv)) ||
				(V_VT(&vVariant) == VT_BOOL && V_BOOL(&vVariant) != *(VARIANT_BOOL*)pv)       ||
				(V_VT(&vVariant) == VT_I2 && V_I2(&vVariant) != *(SHORT*)pv)          ||
				(V_VT(&vVariant) == VT_I4 && V_I4(&vVariant) != *(LONG*)pv) )
			{
				TESTC_(hr, DB_E_ERRORSOCCURRED);
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSET || 
					  rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE);
			}
			else
			{
				TESTC_(hr, S_OK);
				TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_OK);
			}
		}
	}
	else
	{
		if( fBadValue || FAILED(hr) )
		{
			TESTC_(hr, DB_E_ERRORSOCCURRED);
			TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_BADVALUE);
		}
		else
		{
			TESTC_(hr, S_OK);
			TESTC(rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_OK);
		}
	}

	FreeProperties(&cPropSets, &rgPropSets);

	//Everything worked correctly
	hr = S_OK;

CLEANUP:

	//Cleanup Memory
	FreeProperties(&cPropSets, &rgPropSets);

	return hr;
}


// {{ TCW_TC_PROTOTYPE(TCIDBInit_InitProperties)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_InitProperties - IDBProperties::SetProperties
//|	Created:		06/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_InitProperties::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBInit::Init())
	// }}
	{
		// Create DataSource Object
		TESTC_(CreateDataSourceObject(), S_OK);

		// QI for a Session Object off of the m_pIDBProperties pointer
		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties));
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_CACHE_AUTHINFO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_1()
{
	TBEGIN;
	
	m_lValue = VARIANT_TRUE;
	CheckDBProperty(DBPROP_AUTH_CACHE_AUTHINFO, &m_lValue);
	
	m_lValue = VARIANT_FALSE;
	CheckDBProperty(DBPROP_AUTH_CACHE_AUTHINFO, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_ENCRYPT_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_2()
{
	TBEGIN;
	
	m_lValue = VARIANT_TRUE;
	CheckDBProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, &m_lValue);

	m_lValue = VARIANT_FALSE;
	CheckDBProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_AUTH_INTEGRATED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_3()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"SSPI");
	CheckDBProperty(DBPROP_AUTH_INTEGRATED, &m_wszBuffer);
	
	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_AUTH_INTEGRATED, &m_wszBuffer);
	
	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_AUTH_INTEGRATED, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_MASK_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_4()
{
	TBEGIN;
	
	m_lValue = VARIANT_TRUE;
	CheckDBProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, &m_lValue);

	m_lValue = VARIANT_FALSE;
	CheckDBProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_5()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"PASSWORD");
	CheckDBProperty(DBPROP_AUTH_PASSWORD, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_AUTH_PASSWORD, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_AUTH_PASSWORD, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_AUTH_PERSIST_ENCRYPTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_6()
{
	TBEGIN;
	
	m_lValue = VARIANT_TRUE;
	CheckDBProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, &m_lValue);

	m_lValue = VARIANT_FALSE;
	CheckDBProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_7()
{
	TBEGIN;
	
	m_lValue = VARIANT_TRUE;
	CheckDBProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, &m_lValue);

	m_lValue = VARIANT_FALSE;
	CheckDBProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_AUTH_USERID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_8()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"sa");
	CheckDBProperty(DBPROP_AUTH_USERID, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_AUTH_USERID, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_AUTH_USERID, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_9()
{
	TBEGIN;
	
	m_lValue = DBPROPVAL_ASYNCH_INITIALIZE;
	CheckDBProperty(DBPROP_INIT_ASYNCH, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_ASYNCH, &m_lValue, TRUE);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_ASYNCH, &m_lValue, TRUE);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_ASYNCH, &m_lValue, TRUE);

	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_ASYNCH, &m_lValue);


	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_CATALOG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_10()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"pubs");
	CheckDBProperty(DBPROP_INIT_CATALOG, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"c:\bogus");
	CheckDBProperty(DBPROP_INIT_CATALOG, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_INIT_CATALOG, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_DATASOURCE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_11()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"DataSource");
	CheckDBProperty(DBPROP_INIT_DATASOURCE, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_INIT_DATASOURCE, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_INIT_DATASOURCE, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_HWND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_12()
{
	TBEGIN;
	
	m_lValue = (ULONG_PTR)GetDesktopWindow();
	CheckDBProperty(DBPROP_INIT_HWND, &m_lValue);

	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_HWND, &m_lValue);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_HWND, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_IMPERSONATION_LEVEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_13()
{
	TBEGIN;
	
	m_lValue = DB_IMP_LEVEL_ANONYMOUS;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue);
	
	m_lValue = DB_IMP_LEVEL_IDENTIFY;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue);
	
	m_lValue = DB_IMP_LEVEL_IMPERSONATE;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue);
	
	m_lValue = DB_IMP_LEVEL_DELEGATE;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue, TRUE);
	
	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue, TRUE);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_IMPERSONATION_LEVEL, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_LCID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_14()
{
	TBEGIN;
	
	m_lValue = LOCALE_SYSTEM_DEFAULT;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = LOCALE_USER_DEFAULT;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);
	
	m_lValue = LOCALE_NEUTRAL;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = GetSystemDefaultLCID();
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = GetUserDefaultLCID();
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_LCID, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_LOCATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_15()
{
	TBEGIN;
	VARIANT		vValue;
	HRESULT		hr;
	const ULONG	cProp		= 1;
	const ULONG	cPropSet	= 1;
	DBPROP		rgProp[cProp];
	DBPROPSET	rgPropSet[cPropSet];

	wcscpy(m_wszBuffer, L"pubs");
	CheckDBProperty(DBPROP_INIT_LOCATION, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_INIT_LOCATION, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_INIT_LOCATION, &m_wszBuffer);

	// get the the value of DBPROP_INIT_DATASOURCE and use it as DBPROP_INIT_LOCATION
	// try to initialize
	// the call should complete
	VariantInit(&vValue);
	TESTC(GetProperty(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, m_pIDBProperties, &vValue));
	if (VT_BSTR == vValue.vt)
	{
		CheckDBProperty(DBPROP_INIT_LOCATION, V_BSTR(&vValue));
	
		// reset DBPROP_INIT_DATASOURCE
		memset(rgProp, 0, sizeof(DBPROP));
		rgProp[0].dwPropertyID	= DBPROP_INIT_DATASOURCE;
		rgProp[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
		VariantInit(&rgProp[0].vValue);
		rgPropSet[0].cProperties		= cProp;
		rgPropSet[0].guidPropertySet	= DBPROPSET_DBINIT;
		rgPropSet[0].rgProperties		= rgProp;

		TESTC_(m_pIDBProperties->SetProperties(cPropSet, rgPropSet), S_OK);

		hr = m_pIDBInitialize->Initialize();
	}

CLEANUP:
	CHECK(hr = m_pIDBInitialize->Uninitialize(), S_OK);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_MODE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_16()
{
	TBEGIN;
	
	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_READ;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_WRITE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_READWRITE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_DENY_READ;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_DENY_WRITE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_EXCLUSIVE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_DENY_NONE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_READ|DB_MODE_WRITE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_DENY_READ|DB_MODE_SHARE_DENY_WRITE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_SHARE_EXCLUSIVE|DB_MODE_SHARE_DENY_NONE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = DB_MODE_READ|DB_MODE_WRITE|DB_MODE_READWRITE|DB_MODE_SHARE_DENY_READ|DB_MODE_SHARE_DENY_WRITE|DB_MODE_SHARE_EXCLUSIVE|DB_MODE_SHARE_DENY_NONE;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue, TRUE);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue, TRUE);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_MODE, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_PROMPT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_17()
{
	TBEGIN;

	m_lValue = DBPROMPT_PROMPT;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue);

	m_lValue = DBPROMPT_COMPLETE;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue);

	m_lValue = DBPROMPT_COMPLETEREQUIRED;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue);

	m_lValue = DBPROMPT_NOPROMPT;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue);

	m_lValue = SHRT_MIN;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue, TRUE);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue, TRUE);

	m_lValue = SHRT_MAX;
	CheckDBProperty(DBPROP_INIT_PROMPT, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Set and Get DBPROP_INIT_PROTECTION_LEVEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_18()
{
	TBEGIN;

	m_lValue = DB_PROT_LEVEL_NONE;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = DB_PROT_LEVEL_CONNECT;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = DB_PROT_LEVEL_CALL;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = DB_PROT_LEVEL_PKT;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = DB_PROT_LEVEL_PKT_INTEGRITY;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = DB_PROT_LEVEL_PKT_PRIVACY;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue, TRUE);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue, TRUE);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_PROTECTION_LEVEL, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_PROVIDERSTRING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_19()
{
	TBEGIN;

	wcscpy(m_wszBuffer, L"ProviderString");
	CheckDBProperty(DBPROP_INIT_PROVIDERSTRING, &m_wszBuffer);
	
	wcscpy(m_wszBuffer, L"BOGUS");
	CheckDBProperty(DBPROP_INIT_PROVIDERSTRING, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_INIT_PROVIDERSTRING, &m_wszBuffer);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_TIMEOUT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_20()
{
	TBEGIN;

	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_TIMEOUT, &m_lValue);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_TIMEOUT, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_TIMEOUT, &m_lValue);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_TIMEOUT, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_OLEDBSERVICES
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_21()
{
	TBEGIN;
	
	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_RESOURCEPOOLING;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_TXNENLISTMENT;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_AGR_AFTERSESSION;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_CLIENTCURSOR;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_ENABLEALL;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_DISABLEALL;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = DBPROPVAL_OS_RESOURCEPOOLING|DBPROPVAL_OS_TXNENLISTMENT|DBPROPVAL_OS_CLIENTCURSOR;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_OLEDBSERVICES, &m_lValue);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_BINDFLAGS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_22()
{
	TBEGIN;
	
	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_DELAYFETCHCOLUMNS;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_DELAYFETCHSTREAM;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_RECURSIVE;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_OUTPUT;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_COLLECTION;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_OPENIFEXISTS;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_OVERWRITE;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = DB_BINDFLAGS_ISSTRUCTUREDDOCUMENT;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue, TRUE);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue, TRUE);

	m_lValue = DB_BINDFLAGS_OVERWRITE|DB_BINDFLAGS_OPENIFEXISTS;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue, TRUE);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_BINDFLAGS, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_LOCKOWNER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_23()
{
	TBEGIN;
	
	wcscpy(m_wszBuffer, L"UserID");
	CheckDBProperty(DBPROP_INIT_LOCKOWNER, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"Bogus");
	CheckDBProperty(DBPROP_INIT_LOCKOWNER, &m_wszBuffer);

	wcscpy(m_wszBuffer, L"");
	CheckDBProperty(DBPROP_INIT_LOCKOWNER, &m_wszBuffer);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Set and Get DBPROP_INIT_GENERALTIMEOUT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_InitProperties::Variation_24()
{
	TBEGIN;
	
	m_lValue = 0;
	CheckDBProperty(DBPROP_INIT_GENERALTIMEOUT, &m_lValue);

	m_lValue = LONG_MAX;
	CheckDBProperty(DBPROP_INIT_GENERALTIMEOUT, &m_lValue);

	m_lValue = LONG_MIN;
	CheckDBProperty(DBPROP_INIT_GENERALTIMEOUT, &m_lValue);

	m_lValue = -1;
	CheckDBProperty(DBPROP_INIT_GENERALTIMEOUT, &m_lValue, TRUE);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_InitProperties::Terminate()
{
	// Relase Objects
	SAFE_RELEASE(m_pIDBProperties);
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBInit::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBInit_Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_Zombie - IDBInitialize::Zombie
//|	Created:		06/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Zombie::Init()
{
	// Check to see if Transactions are usable
	TESTC(IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal));

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCZOMBIE::Init())
	// }}
	{
		// Register Interface with Zombie
		if( RegisterInterface(DATASOURCE_INTERFACE,	// Object
							  IID_IDBInitialize,	// IID
							  0,					// # Prop's
							  NULL) )				// Prop's
			return TRUE;
	}
	
	// Check to see if ITransaction is supported
    QTESTC(m_pITransactionLocal != NULL);
	m_pITransactionLocal = NULL;

	return FALSE;
	
CLEANUP:

	return TEST_SKIPPED;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInitialize::Initialize with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_1()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Abort the transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE));

	// Initialize
	TESTC_(m_pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(S_OK);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInitialize::Initialize with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_2()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Commit the transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE));

	// Initialize
	TESTC_(m_pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInitialize::Initialize with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_3()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Abort the transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE));

	// Initialize
	TESTC_(m_pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInitialize::Initialize with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_4()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Commit the transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE));

	// Initialize
	TESTC_(m_pIDBInitialize->Initialize(), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInitialize::Uninitialize with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_5()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Abort the transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE));

	// Uninitialize
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInitialize::Uninitialize with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_6()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Commit the transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE));

	// Uninitialize
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IDBInitialize::Uninitialize with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_7()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Abort the transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE));

	// Uninitialize
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IDBInitialize::Uninitialize with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_Zombie::Variation_8()
{
	TBEGIN;

	// Retrieve an Interface pointer to IDBInitialize within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, 
						  (IUnknown**)&m_pIDBInitialize,0, NULL));

	// Commit the transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE));

	// Uninitialize
	TESTC_(m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

CLEANUP:

	// Cleanup Transactions
	SAFE_RELEASE(m_pIDBInitialize);
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_Zombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCZOMBIE::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIDBInit_ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCIDBInit_ExtendedErrors - Extended Errors
//|	Created:		07/12/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCIDBInit_ExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIDBInit_Uninitialize::Init())
	// }}
		return TRUE;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid Initialize and Uninitialize calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_ExtendedErrors::Variation_1()
{
	TBEGIN;
	IDBInitialize* pIDBInitialize = NULL;
	IDBProperties* pIDBProperties = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IDBInitialize method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a new Session Object
	TESTC_(CoCreateInstance(m_ProviderClsid, NULL, 
			m_clsctxProvider, IID_IDBInitialize,(void **)&pIDBInitialize), S_OK);

	m_pExtError->CauseError();
	
	// Get the IDBProperties Interface
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, 
					DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Set the IDBProperties Interface
	TESTC_(pIDBProperties->SetProperties(g_cPropertySets, g_rgPropertySets), S_OK);

	//Do extended check following Initialize
	TESTC_(m_hr=pIDBInitialize->Initialize(), S_OK);
	TESTC(XCHECK(pIDBInitialize, IID_IDBInitialize, m_hr));
	
	m_pExtError->CauseError();
	
	// Uninitialize a Clean DSO
	TESTC_(m_hr=pIDBInitialize->Uninitialize(), S_OK);

	// Set the IDBProperties Interface
	TESTC_(pIDBProperties->SetProperties(g_cPropertySets, g_rgPropertySets), S_OK);

	//initialize again so can QI ISupportErrorInfo
	TESTC_(m_hr=pIDBInitialize->Initialize(), S_OK);
	TESTC(XCHECK(pIDBInitialize, IID_IDBInitialize, m_hr));

CLEANUP:

	// Release objects
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid Initialize and Uninitialize calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_ExtendedErrors::Variation_2()
{
	TBEGIN;
	IUnknown* pDBSession = NULL;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the IDBInitialize method.
	//We then check extended errors to verify the right extended error behavior.
	
	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a DBSession
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
					IID_IOpenRowset, (IUnknown**)&pDBSession), S_OK);

	m_pExtError->CauseError();
  
	// Uninitialize a DSO with a open Session
	TESTC_(m_hr=m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

	//Do extended check following Uninitialize
	TESTC(XCHECK(m_pIDBInitialize, IID_IDBInitialize, m_hr));

CLEANUP:

	// Release Session and Uninitialize
	SAFE_RELEASE(pDBSession);
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid Initialize and Uninitialize calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_ExtendedErrors::Variation_3()
{
	TBEGIN;
	IUnknown* pDBSession = NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IDBInitialize method.
	//We then check extended errors to verify the right extended error behavior.
  
	// Initialize the DSO
	TESTC(SUCCEEDED(InitializeDSO(REINITIALIZE_YES)));

	// Create a DBSession
	TESTC_(m_pIDBCreateSession->CreateSession(NULL, 
					IID_IOpenRowset, (IUnknown**)&pDBSession), S_OK);
	
	// Uninitialize a DSO with a open Session
	TESTC_(m_hr=m_pIDBInitialize->Uninitialize(), DB_E_OBJECTOPEN);

	//Do extended check following Uninitialize
	TESTC(XCHECK(m_pIDBInitialize, IID_IDBInitialize, m_hr));

CLEANUP:

	// Release Session and Uninitialize
	SAFE_RELEASE(pDBSession);
	CHECK(m_pIDBInitialize->Uninitialize(), S_OK);

	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Initialize with no properties set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIDBInit_ExtendedErrors::Variation_4()
{
	TBEGIN;
	IDBInitialize *	pIDBInitialize = NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IDBInitialize method.
	//We then check extended errors to verify the right extended error behavior.
  
	TESTC_(GetModInfo()->CreateProvider(NULL,
			IID_IDBInitialize,(IUnknown**)&pIDBInitialize), S_OK);

	// Initialize the DSO
	m_hr=pIDBInitialize->Initialize();

	//Do extended check following Uninitialize
	TESTC(XCHECK(pIDBInitialize, IID_IDBInitialize, m_hr));

CLEANUP:

	// Release IDBInitialize
	SAFE_RELEASE(pIDBInitialize);

	TRETURN;

}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIDBInit_ExtendedErrors::Terminate()
{
	// Relase Objects
	SAFE_RELEASE(m_pIDBInitialize);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIDBInit_Uninitialize::Terminate());

}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc CheckDBPropInfoSet.  It should be called after GetDBInitProperties.
// The function checks information returned in DBPROPINFOSET.
// 
// @rdesc Success or Failure
//		@flag TRUE | CleanUp was successful.
//--------------------------------------------------------------------
void TCIDBInit_GetPropertyInfo::CheckDBPropInfoSet(BOOL fInitAll)
{
	ULONG i,j,ulCount = 0;

	// Check to see if the Provider supports any DBINIT Properties
	if(!g_cPropertyInfoSets)
	{
		COMPARE(m_cPropertyInfoSets, 1);
		COMPARE(m_prgPropertyInfoSets->cPropertyInfos, m_rgPropertyIDSets->cPropertyIDs);
		if(m_prgPropertyInfoSets->cPropertyInfos)
			COMPARE(!!m_prgPropertyInfoSets->rgPropertyInfos, TRUE);
		else
			COMPARE(m_prgPropertyInfoSets->rgPropertyInfos, NULL);
		if(fInitAll)
			COMPARE(m_prgPropertyInfoSets->guidPropertySet, DBPROPSET_DBINITALL);
		else
			COMPARE(m_prgPropertyInfoSets->guidPropertySet, DBPROPSET_DBINIT);
		COMPARE(m_pDescBuffer, NULL);
		return;
	}

	ULONG ulPropSet;
	// If the Provider returns more than 1 PropertySet find the DBPROPSET_DBINIT
	for(ulPropSet=0; ulPropSet<m_cPropertyInfoSets; ulPropSet++)
	{
		if(m_prgPropertyInfoSets[ulPropSet].guidPropertySet == DBPROPSET_DBINIT)
			break;
	}

	// Make sure that we have a DBINIT PROPSET
	TESTC(ulPropSet < m_cPropertyInfoSets);

	// Provider should only return 1 DBPROPSET
	if( fInitAll && m_cPropertyInfoSets > 1 )
		odtLog << L"The Provider has a Provider Specific DBINIT PropSet."<< ENDL;
	else
		COMPARE(m_cPropertyInfoSets, 1);

	// Check PropertyInfoSets info
	COMPARE(DBPROPSET_DBINIT, m_prgPropertyInfoSets[ulPropSet].guidPropertySet);

	// Check DBPROPINFO
	for(i=0; i<m_prgPropertyInfoSets[ulPropSet].cPropertyInfos; i++)
	{
		// If PROPID doesn't match continue
		for(j=0; j<g_cMaxPropertyInfoSets; j++)
		{
			if(	g_rgDBInitDBProps[j].dwPropertyID != 
				m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwPropertyID )
				continue;
			
			if(g_rgDBInitDBProps[j].fSupported)
			{
				// Check the Description
				COMPARE(0, wcscmp(g_rgDBInitDBProps[j].wszDescBuff, m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].pwszDescription));
				
				// Check the PropertyID
				COMPARE(g_rgDBInitDBProps[j].dwPropertyID, m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwPropertyID);
				
				// Check the DBPROPFLAGS
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_COLUMN), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_DATASOURCE), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_DATASOURCECREATE), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_DATASOURCEINFO), 0);
				COMPARE(((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_DBINIT) > 0), 1);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_INDEX), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_ROWSET), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_SESSION), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_TABLE), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_COLUMNOK), 0);
				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_TRUSTEE), 0);
//				COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_VIEW), 0);
				COMPARE(((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_READ) > 0), 1);
				
				if(g_rgDBInitDBProps[j].fRequired)
					COMPARE(((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_REQUIRED) > 0), 1);
				else
					COMPARE((m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags & DBPROPFLAGS_REQUIRED), 0);
				
				// Check the VT_TYPE
				COMPARE(g_rgDBInitDBProps[j].vtPropType, m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vtType);
				
				// Check the vValue
				if(m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vValues.vt & VT_ARRAY)
					COMPARE(g_rgDBInitDBProps[j].vtPropType, m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vValues.vt & ~VT_ARRAY);
				else
					COMPARE(VT_EMPTY, m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vValues.vt);
			}
			else
			{
				COMPARE(m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].pwszDescription, NULL);
				COMPARE(m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].dwFlags, DBPROPFLAGS_NOTSUPPORTED);
				COMPARE(m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vtType == VT_EMPTY, TRUE);
				COMPARE(m_prgPropertyInfoSets[ulPropSet].rgPropertyInfos[i].vValues.vt == VT_EMPTY, TRUE);
			}

			// Increment count of PropertyInfos
			ulCount++;
			break;
		}
	}

	// Check PropertyInfo
	COMPARE(ulCount, m_prgPropertyInfoSets[ulPropSet].cPropertyInfos);

CLEANUP:

	return;
}

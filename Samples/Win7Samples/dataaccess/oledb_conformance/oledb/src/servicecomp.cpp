//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ServiceComp Implementation Module | 	This module contains definition information
//					for the CServiceComp class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

#include "modstandard.hpp"
#include "privstd.h"
#include "privlib.h"
#include "ServiceComp.h"

extern const wchar_t		*gwszModuleName;	
extern CSourcesSet			*g_pSourcesSet;
extern const GUID			CLSID_MSDASQL;

LPFGetComputerName	GetComputerNameI	= GetComputerNameWRAP;

//////////////////////////////////////////////////////////////////////////////////////
//#Definitions
//
//because mtxdm.h keeps this hidden somewhat, I define IID_IDispenserManagerShutdownGuarantee here
//#ifdef USE_UUIDOF_FOR_IID_
#define  IID_IDispenserManagerShutdownGuarantee   __uuidof(IDispenserManagerShutdownGuarantee)
//#endif

LPWSTR				g_swzDispManLib	= L"mtxdm.dll";
LPSTR				g_szDispManLib	= "mtxdm.dll";
HINSTANCE			g_hLoadedLibDM	= NULL;

ULONG				CServiceComp::s_SCRef = 0;

LONG GetServerConnNo();

BOOL CServiceComp::IsMTSPresent()
{
	BOOL			fPresent	= FALSE;
	HMODULE			hMod		= NULL;
	IUnknown		*pUnk		= NULL;
	typedef HRESULT (_cdecl *PFNGetObject)(IUnknown**);

	try
	{

		// First try if we are running under mtx.exe
		hMod=GetModuleHandleW(L"mtx.exe");

		if (hMod != NULL)
			fPresent = TRUE;
		else
		{
			HMODULE hMod=GetModuleHandleW(L"MTXEX.dll");
				
			if (hMod != NULL)
			{
				// we should test that there is a context for this object
				if (hMod != NULL)
				{
					hMod=LoadLibraryW(L"MTXEX.dll");
					if (hMod != NULL)
					{
						PFNGetObject pfnGetObject= (PFNGetObject)GetProcAddress(hMod,"GetObjectContext");
						
						if (pfnGetObject!=NULL)
						{
							IUnknown * pUnk=NULL;
							pfnGetObject(&pUnk);
							
							if (pUnk!=NULL)
							{
								fPresent = TRUE;
								pUnk->Release();
								
							}
						}
						FreeLibrary(hMod);
					}
				}
			}
		}
	}

	catch(...)
	{
	}

	return fPresent;
} //CServiceComp::IsMTSPresent




BOOL CServiceComp::IsWindows2000()
{
	return (LOBYTE(LOWORD(GetVersion())) >= 5);
} //CServiceComp::sWindows2000




BOOL CServiceComp::IsIISPresent()
{
	HMODULE	hMod		= GetModuleHandleW(L"inetinfo.exe");

	if (NULL != hMod)
		hMod = GetModuleHandleW(L"dllhost.exe");

	return (hMod != NULL && IsWindows2000());
} //CServiceComp::IsIISPresent




HRESULT CServiceComp::GetDispenserManager(REFCLSID rclsid, IDispenserManager **ppIDispenserMan)
{

	HRESULT hr = E_FAIL;

	// Finaly COM+ made dispenser manager be cocreatable!
	// We could try to hold on to the class factory but when do we release it?
	// So we just call CoCreateInstance directly
	hr=CoCreateInstance(	rclsid, 
							NULL, 
							CLSCTX_INPROC_SERVER,
							IID_IDispenserManager, 
							(void**)ppIDispenserMan);	
	return hr;
} //CServiceComp::GetDispenserManager




HRESULT CServiceComp::GetDispenserManager(LPSTR szLibName, IDispenserManager **ppIDispenserMan)
{
	typedef HRESULT (*_TPFN_GETDISPMAN)(IDispenserManager** );
	_TPFN_GETDISPMAN	pfnGetDispManager		= NULL;
	HRESULT				hr						= E_FAIL;
	HINSTANCE			g_hLoadedLibDM			= LoadLibraryA(szLibName);

	try
	{
		if(g_hLoadedLibDM)
		{	
			pfnGetDispManager = reinterpret_cast<_TPFN_GETDISPMAN>(GetProcAddress(g_hLoadedLibDM,"GetDispenserManager"));
			
			if (pfnGetDispManager)
				hr = pfnGetDispManager(ppIDispenserMan);
		}
	}

	catch(...)
	{
		// Oops something went wrong.
		if(SUCCEEDED(hr))
			hr=E_FAIL;
	}
	
	if (FAILED(hr) && g_hLoadedLibDM)
		FreeLibrary(g_hLoadedLibDM);

	return hr;
} //CServiceComp::GetDispenserManager




// get a dispenser manager, either by CoCreate-ing it or by API
HRESULT CServiceComp::GetDispMan(IDispenserManager **ppIDispenserMan)
{
	HRESULT hr = E_FAIL;

	// First try to CoCreate dispenser manager
	hr=GetDispenserManager(CLSID_DispenserManager, ppIDispenserMan);

	if (FAILED(hr))
		hr=GetDispenserManager(g_szDispManLib, ppIDispenserMan);

	return hr;
} //CServiceComp::GetDispMan




BOOL CServiceComp::CanGetShutdownGuaranteed()
{
	BOOL	fRes = FALSE;
	IDispenserManager					*pIDispenserMan = NULL;
	IDispenserManagerShutdownGuarantee	*pIDMSG			= NULL;

	TESTC_(GetDispMan(&pIDispenserMan), S_OK);
	TESTC(NULL != pIDispenserMan);
	
	if (S_OK == pIDispenserMan->QueryInterface(
			IID_IDispenserManagerShutdownGuarantee, (void**)&pIDMSG))
	{
//		pIDMSG->IsComPlusApp(&fRes); // this is the new name of the method
		pIDMSG->ShutdownGuarantee(&fRes);
	}
	else 
	return fRes = (IsMTSPresent() || IsIISPresent());

CLEANUP:
	if (g_hLoadedLibDM)
	{
		FreeLibrary(g_hLoadedLibDM);
		g_hLoadedLibDM = NULL;
	}	
	SAFE_RELEASE(pIDispenserMan);
	SAFE_RELEASE(pIDMSG);
	return fRes;
} //CServiceComp::CanGetShutdownGuaranteed








IDataInitialize *CServiceComp::pIDataInitialize()
{
	IDataInitialize	*pIDataInit = NULL;

	if (!CHECK(CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, 
			CLSCTX_INPROC_SERVER, IID_IDataInitialize, (void**)&pIDataInit), S_OK))
	{
		odtLog << "Failed getting an IDataInitialize interface\n";
	}
	else
		s_SCRef++;

	return pIDataInit;
} //CServiceComp::pIDataInitialize




IDBPromptInitialize	*CServiceComp::pIDBPromptInitialize()
{
	IDBPromptInitialize	*pIDBPromptInit = NULL;

	if (!CHECK(CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_INPROC_SERVER, 
		IID_IDBPromptInitialize, (void**)&pIDBPromptInit), S_OK))

	{
		odtLog << "Failed getting an IDBPromptInitialize interface\n";
	}
	else
		s_SCRef++;

	return pIDBPromptInit;
} //CServiceComp::pIDBPromptInitialize



// This function returns the number of connection to server from the current machine 
// If the server is not SQL Server, the return will be -1L meaning no result
LONG CServiceComp::GetServerConnNo()
{
	LONG				lConn = -1;
	CTable				*pTable		= NULL;
	CLightRowset		Rowset;
	IRowset				*pIRowset	= NULL;
	ULONG				ulColOffset = 0;
	DBORDINAL			ulHostName	= 5;	// the column ordinal, should be 4 per spec -> MDAC bug 47663
	WCHAR				*pwszHostName	= NULL;
	HRESULT				hr;
	HROW				hRow;
	const DWORD			dwCNMaxLen = max(128, MAX_COMPUTERNAME_LENGTH);
	DWORD				dwCNLen = dwCNMaxLen;
	WCHAR				wszComputerName[dwCNMaxLen+1] = L"";
	WCHAR				wszCompName[dwCNMaxLen+1];
	IOpenRowset			*pIOpenRowset = NULL;
	DBID				dbidHostName;
	
	ULONG				cPropSets	= 0;
	DBPROPSET			*rgPropSets	= NULL;

	CLightDataSource	DataSource;

	TESTC(GetModInfo()->GetInitProps(&cPropSets, &rgPropSets));

	// create a connection to the server using default props
	TESTC_(CoCreateDSO(&DataSource, GetModInfo()->GetProviderCLSID(),
							IID_IDBInitialize,
							cPropSets, rgPropSets,
							TRUE), S_OK);

	TESTC_(((IDBCreateSession*)DataSource)->CreateSession(NULL, 
		IID_IOpenRowset, (IUnknown**) &pIOpenRowset), S_OK);

	TESTC(0 != GetComputerNameI(wszComputerName, &dwCNLen));
	swprintf(wszCompName, L"%-128s", wszComputerName);

	// create a new connection and check that this is the only connection from this
	// machine to the server
	pTable = new CTable(pIOpenRowset, (LPWSTR)gwszModuleName);

	pTable->SetExistingTable(L"ceva");
	if (S_OK != (hr = pTable->BuildCommand(L"EXEC sp_who", IID_IRowset,
		EXECUTE_ALWAYS, 0, NULL, NULL, NULL, (IUnknown**)&pIRowset, NULL)))
		goto CLEANUP;

	TESTC_(Rowset.Attach(IID_IRowset, pIRowset), S_OK);

	//Try to find the specified row with this table...
	dbidHostName.eKind = DBKIND_NAME;
	dbidHostName.uName.pwszName = L"hostname";
	CHECK(((IColumnsInfo*)Rowset)->MapColumnIDs(1, &dbidHostName, &ulHostName), S_OK);

	lConn = 0;
	for (; S_OK == Rowset.GetNextRows(&hRow); )
	{
		TESTC_(hr = Rowset.GetData(hRow),S_OK);

		pwszHostName = (WCHAR*)Rowset.GetColumnValue(ulHostName);

		if (pwszHostName && 0 == wcscmp(wszCompName, pwszHostName))
			lConn++;
		
		Rowset.ReleaseRows(hRow);	
	}

CLEANUP:
	SAFE_RELEASE(pIOpenRowset);
	FreeProperties(&cPropSets, &rgPropSets);
	delete pTable;
	return lConn;
} //CServiceComp::GetServerConnNo



HRESULT CServiceComp::CreateDBInstance(
	CLightDataSource		*pDPO,
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets	/*= 0*/, 
	DBPROPSET			*rgPropSets	/*= NULL*/, 
	BOOL				fInitialize /*= FALSE*/
)
{
	HRESULT			hr					= E_FAIL;
	IUnknown		*pIUnknown			= NULL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();

	TESTC(NULL != pDPO);

	// co create the provider
	hr = GetIDataInitialize()->CreateDBInstance(clsidProvider, NULL, 
		pThisTestModule->m_clsctxProvider, NULL, 
		riid, &pIUnknown);
	
	if (FAILED(hr))
	{
		COMPARE(NULL == pIUnknown, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != pIUnknown);

	TESTC_(pDPO->Attach(riid, pIUnknown, CREATIONMETHODS_CREATEDBINSTANCE), S_OK);
	TESTC(pDPO->SetProviderCLSID(clsidProvider));

	TESTC(0 == cPropSets || NULL != rgPropSets);

	if (0 < cPropSets)
	{
		hr = pDPO->SetProperties(cPropSets, rgPropSets);
	}

	// initialize
	if (fInitialize)
	{
		hr = pDPO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	return hr;
} //CServiceComp::CreateDBInstance



HRESULT	CServiceComp::CreateDBInstanceEx(
	CLightDataSource		*pDPO, 
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets, 
	DBPROPSET			*rgPropSets, 
	BOOL				fInitialize /*= FALSE*/
)
{
	HRESULT			hr					= E_FAIL;
	IUnknown		*pIUnknown			= NULL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();
	MULTI_QI mq;
	
	mq.pIID = &riid;
	mq.pItf = NULL;
	mq.hr	= S_OK;

	TESTC(NULL != pDPO);

	// co create the provider
	hr = GetIDataInitialize()->CreateDBInstanceEx(clsidProvider, NULL, 
		pThisTestModule->m_clsctxProvider, NULL, NULL, 1, &mq);
	
	if (FAILED(hr))
	{
		COMPARE(NULL == mq.pItf, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != mq.pItf);
	pIUnknown = mq.pItf;

	TESTC_(pDPO->Attach(riid, pIUnknown, CREATIONMETHODS_CREATEDBINSTANCEEX), S_OK);
	TESTC(pDPO->SetProviderCLSID(clsidProvider));

	TESTC(0 == cPropSets || NULL != rgPropSets);

	if (0 < cPropSets)
	{
		hr = pDPO->SetProperties(cPropSets, rgPropSets);
	}

	// initialize
	if (fInitialize)
	{
		hr = pDPO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	return hr;
} //CServiceComp::CreateDBInstanceEx



HRESULT	CServiceComp::GetDataSource(
	CLightDataSource	*pDPO, 
	REFIID				riid,
	WCHAR				*pwszInitString,
	BOOL				fInitialize /*= FALSE*/
)
{
	HRESULT			hr					= E_FAIL;
	IUnknown		*pIUnknown			= NULL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();
	WCHAR			*pwszProvProgID		= NULL;
	CLSID			clsidProvider;

	TESTC(NULL != pDPO);

	// create the provider
	hr = GetIDataInitialize()->GetDataSource(NULL, pThisTestModule->m_clsctxProvider, 
		pwszInitString, riid, &pIUnknown);
	
	if (FAILED(hr))
	{
		COMPARE(NULL == pIUnknown, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != pIUnknown);

	TESTC_(pDPO->Attach(riid, pIUnknown, CREATIONMETHODS_GETDATASOURCE), S_OK);

	// look for provider progID in pwszInitString
	CModInfo::GetStringKeywordValue(pwszInitString, L"Provider", &pwszProvProgID);
	if (pwszProvProgID)
	{
		CHECK(CLSIDFromProgID(pwszProvProgID, &clsidProvider), S_OK);
	}
	else
		clsidProvider = CLSID_MSDASQL;


	pDPO->SetProviderCLSID(clsidProvider);

	// initialize
	if (fInitialize)
	{
		hr = pDPO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszProvProgID);
	return hr;
} //CServiceComp::GetDataSource




HRESULT CServiceComp::GetDataSource(
	CLightDataSource		*pDPO,
	REFIID				riid,
	CLSID				clsidProvider, 
	CPropSets			*pPropSets,
	BOOL				fInitialize /*= FALSE*/
)
{
	CWString		string;
	CPropSets		PropSets;
	IUnknown		*pIUnknown			= NULL;
	WCHAR			*pwszProgID			= NULL;
	HRESULT			hr					= E_FAIL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();

	TESTC(NULL != pDPO);

	PropSets.m_pPropInfoSets = pPropSets->m_pPropInfoSets;

	// if clsidProvider was indicated, add it to string
	if (GUID_NULL != clsidProvider)
	{
		ProgIDFromCLSID(clsidProvider, &pwszProgID);
		string + L"Provider=";
		string + pwszProgID;
		string + L"; ";
	}
	// otherwise do not set a provider value and go for default one (MSDASQL)

	// add properties to string
	string + pPropSets->ConvertToCWString();

	// create the DPO
	hr = GetIDataInitialize()->GetDataSource(NULL, 
		pThisTestModule->m_clsctxProvider, 
		string, riid, &pIUnknown);
	
	if (FAILED(hr))
	{
		COMPARE(NULL == pIUnknown, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != pIUnknown);

	TESTC(pDPO->SetProviderCLSID(clsidProvider));
	TESTC_(pDPO->Attach(riid, pIUnknown, CREATIONMETHODS_GETDATASOURCE), S_OK);

	// make sure we got the required properties
	TESTC_(pDPO->GetInitProperties(&PropSets), S_OK);

	// initialize
	if (fInitialize)
	{
		hr = pDPO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszProgID);
	return hr;
} //CServiceComp::GetDataSource



/*
HRESULT	CServiceComp::PromptDataSource(
	CLightDataSource		*pDPO, 
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets, 
	DBPROPSET			*rgPropSets, 
	BOOL				fInitialize //= FALSE
)
{
	HRESULT			hr					= E_FAIL;
	IUnknown		*pIUnknown			= NULL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();
	CDSL_Dialog		Dialog(GetIDBPromptInitialize(), NULL, NULL, DBPROMPTOPTIONS_PROPERTYSHEET,
						0, NULL, NULL, riid, &pIUnknown);

	TESTC(NULL != pDPO);

	// co create the dialog
	TESTC(NULL != m_pIDBPromptInit);
	TESTC(0 < cPropSets || NULL == rgPropSets);

	TESTC(Dialog.SelectProvider(clsidProvider));
	TESTC(Dialog.GotoAllPage());
	if (0 < cPropSets)
		TESTC(Dialog.SetProperties(cPropSets, rgPropSets));

	TESTC(Dialog.KillThread());
	TESTC(Dialog.IsResult(S_OK));
	hr = Dialog.GetHRESULT();

	if (FAILED(hr))
	{
		COMPARE(NULL == pIUnknown, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != pIUnknown);

	TESTC_(pDPO->Attach(riid, pIUnknown, CREATIONMETHODS_PROMPTINITIALIZE), S_OK);
	TESTC(pDPO->SetProviderCLSID(clsidProvider));

	// initialize
	if (fInitialize)
	{
		hr = pDPO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	return hr;
} //CServiceComp::PromptDataSource
*/



// this method doesn't invoke SC
HRESULT CServiceComp::CoCreateDSO(
	CLightDataSource		*pDSO,
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets, 
	DBPROPSET			*rgPropSets, 
	BOOL				fInitialize /*= FALSE*/
)
{
	HRESULT			hr					= E_FAIL;
	IUnknown		*pIUnknown			= NULL;
	CThisTestModule	*pThisTestModule	= GetModInfo()->GetThisTestModule();

	// co create the provider
	TESTC(NULL != pDSO);

	// call CoCreate helper function in miscfunc.cpp
	// it will use either CoCreateInstance or CoCreateInstanceEx
	hr = CoCreate(clsidProvider, NULL, pThisTestModule->m_clsctxProvider, 
				riid, (LPVOID*)&pIUnknown);
	
	if (FAILED(hr))
	{
		COMPARE(NULL == pIUnknown, TRUE);
		goto CLEANUP;
	}

	TESTC(NULL != pIUnknown);

	TESTC_(pDSO->Attach(riid, pIUnknown, CREATIONMETHODS_COCREATEDSO), S_OK);
	TESTC(pDSO->SetProviderCLSID(clsidProvider));

	TESTC(0 == cPropSets || NULL != rgPropSets);

	if (0 < cPropSets)
	{
		hr = pDSO->SetProperties(cPropSets, rgPropSets);
	}

	// initialize
	if (fInitialize)
	{
		hr = pDSO->Initialize();
	}
	else
		hr = S_OK;

CLEANUP:
	return hr;
} //CServiceComp::CoCreateDSO





HRESULT	CServiceComp::CreateDPO(
	CREATIONMETHODSENUM	dwCreationMethod,
	CLightDataSource		*pDPO,
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets	/*= 0*/, 
	DBPROPSET			*rgPropSets	/*= NULL*/, 
	BOOL				fInitialize /*= FALSE*/
)
{
	HRESULT		hr = E_FAIL;
	CWString	string;
	CWString	stringProps;
	WCHAR		*pwszProgID			= NULL;
	CPropSets	PropSets(cPropSets, rgPropSets);

	switch (dwCreationMethod)
	{
		case CREATIONMETHODS_GETDATASOURCE:
			ProgIDFromCLSID(clsidProvider, &pwszProgID);
			string + L"Provider=";
			string + pwszProgID;
			string + L"; ";

			PropSets.m_pPropInfoSets = &(*g_pSourcesSet)[clsidProvider].m_PropInfoSets;

			// get properties
			string + PropSets.ConvertToCWString();

			hr = GetDataSource(pDPO, riid, (LPWSTR)(LPCWSTR)string, fInitialize);
			//COMPARE(pDPO->SetProviderCLSID(clsidProvider), TRUE);
			break;

		case CREATIONMETHODS_CREATEDBINSTANCE:
			hr = CreateDBInstance(pDPO, clsidProvider, riid, cPropSets, rgPropSets, fInitialize);
			break;

		case CREATIONMETHODS_CREATEDBINSTANCEEX:
			hr = CreateDBInstanceEx(pDPO, clsidProvider, riid, cPropSets, rgPropSets, fInitialize);
			break;

		case CREATIONMETHODS_PROMPTINITIALIZE:
			ASSERT(FALSE);
			hr = TRUE;
//			hr = PromptDataSource(pDPO, clsidProvider, riid, cPropSets, rgPropSets, fInitialize);
			break;


		default:
			hr = E_FAIL;
	}

	PropSets.Detach();
	SAFE_FREE(pwszProgID);
	return hr;
} //CServiceComp::CreateDPO




HRESULT	CServiceComp::CreateDPO(
	CLightDataSource		*pDPO,
	CLSID				clsidProvider, 
	REFIID				riid,
	ULONG				cPropSets	/*= 0*/, 
	DBPROPSET			*rgPropSets	/*= NULL*/, 
	BOOL				fInitialize /*= FALSE*/
)
{
	CREATIONMETHODSENUM	dwCreationMethod = (CREATIONMETHODSENUM)(rand() % g_cCreationMethods + 1);

	// IDBPromptInitialize is not meant for performance
	if (CREATIONMETHODS_PROMPTINITIALIZE == dwCreationMethod)
		dwCreationMethod = CREATIONMETHODS_CREATEDBINSTANCE;
	return CreateDPO(dwCreationMethod, pDPO, clsidProvider, riid, cPropSets, rgPropSets, fInitialize);
} //CServiceComp::CreateDPO





/////////////////////////////////////////////////////////////////////////////
// IsUnicode
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsUnicode()
{
	static BOOL fInitialized = FALSE;
	static BOOL fUnicode = TRUE;

	//NOTE:  Don't call another other helper functions from within this function
	//that might also use the IsUnicode flag, otherwise this will be an infinite loop...
	
	if(!fInitialized)
	{
		HKEY hkJunk = HKEY_CURRENT_USER;

		// Check to see if we have win95's broken registry, thus we do not have Unicode support
		if((RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && hkJunk == HKEY_CURRENT_USER)
		{
			// Try the ANSI version
			if((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && (hkJunk != HKEY_CURRENT_USER))
			{
				fUnicode = FALSE;
			}
		}

		if(hkJunk != HKEY_CURRENT_USER)
			RegCloseKey(hkJunk);
		fInitialized = TRUE;
	}

	return fUnicode;
}


BOOL __stdcall GetComputerNameANSI(LPWSTR lpBuffer, LPDWORD lpnSize)
{
	if (!lpBuffer || !lpnSize)
		return FALSE;

	DWORD	nMaxCountA = *lpnSize + 1;
	LPSTR	lpStringA = NULL;
	int		result;

	SAFE_ALLOC(lpStringA, CHAR, nMaxCountA);
	result = GetComputerNameA(lpStringA, &nMaxCountA);
	if (0 != result)
		*lpnSize = (S_OK == ConvertToWCHAR(lpStringA, lpBuffer, *lpnSize))? (int)wcslen(lpBuffer): 0;

CLEANUP:
	SAFE_FREE(lpStringA);
	return result;
} //GetComputerNameANSI

// the independent version of GetComputerNameWRAP
BOOL __stdcall	GetComputerNameWRAP(LPWSTR lpBuffer, LPDWORD lpnSize)
{
	GetComputerNameI = (IsUnicode()) ? GetComputerNameW : GetComputerNameANSI;

	return GetComputerNameI(lpBuffer, lpnSize);
} // GetComputerNameWRAP


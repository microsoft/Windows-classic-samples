//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CModInfo Implementation Module| 	
// This module contains implementation information
// for Provider functions for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 01-20-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CModInfo Elements|
//
// @subindex CModInfo
//
//---------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "privstd.h"
#include "CModInfo.hpp"
#include "MiscFunc.h"
#include "Strings.h"
#include "MSDASC.H"  //Service Components Header
#include "CStorage.hpp"





//////////////////////////////////////////////////////////////////////////
// CModInfo::CModInfo
//
//////////////////////////////////////////////////////////////////////////
CModInfo::CModInfo()
{	
	m_pCThisTestModule		= NULL;
	
	m_dwProviderLevel		= CONF_LEVEL_0;
	m_fStrictConformance	= FALSE;
	m_fTableCreation		= TRUE; // tables are now created through ITableDefinition
	m_fRowsetIndex			= TRUE; // use IRowsetIndex if available
	m_dwInsert				= DEFAULT_INSERT;
	m_dwServiceComponents	= 0;
	m_dwWarningLevel		= WARNINGLEVEL_ALL;
	m_dwDebugMode			= DEBUGMODE_OFF;
	m_fCompareReadOnlyCols	= TRUE;
	
	//Other items
	m_pwszTableName			= NULL;
	m_dwTableOpts			= 0;

	m_pwszFileName			= NULL;
	m_pwszDefaultQuery		= NULL;
	m_pwszRowScopedQuery	= NULL;
	m_pwszEnumerator		= NULL;
	m_ClassContext			= CLSCTX_INPROC_SERVER;
	m_pwszRemoteMachine		= NULL;

	//Initialization
	m_fParsedProps			= FALSE;
	m_pwszInitString		= NULL;
	m_cInitPropSets			= 0;
	m_rgInitPropSets		= NULL;
	m_fIsWin9x				= FALSE;
	m_pwszBackend			= NULL;
	m_MajorBackendVer		= 0;
	m_pwszBackendVer		= NULL;
	m_pwszProviderVer		= NULL;

	//Rowset Creation
	m_eRowsetQuery			= NO_QUERY;

	m_pCError				= new CError();
	m_pCParseInitFile		= new CParseInitFile();
	m_pCPoolManager			= new CPoolManager;

	m_pwszRootBinderProgID	= NULL;
	m_pwszRootURL			= NULL;
	m_pIBindResource		= NULL;

	// Create CLocaleInfo object instance
	// Use international data only wiht the appropriate registry settings
	// FindIntlSetting looks up the appropriate registry key
	m_pCLocaleInfo = NULL;
	if(FindIntlSetting())
		m_pCLocaleInfo = new CLocaleInfo( GetUserDefaultLCID() );
	m_fUseIntlIdentifier = TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::~CModInfo
//
//////////////////////////////////////////////////////////////////////////
CModInfo::~CModInfo()
{	
	//If using a "static" INI File provided table and the user really 
	//wants to drop it (ie: automation), then we will drop it if indicated in the initstring
	//NOTE: Some providers do not support dropping tables, commands or ITableDef (readonly)
	DropTable();
	
	//Initialization
	PROVIDER_FREE(m_pwszInitString);
	FreeProperties(&m_cInitPropSets, &m_rgInitPropSets);
	m_fParsedProps = FALSE;

	//SAFE_RELEASE(m_pIBindResource);
	if(m_pIBindResource)
		COMPAREW(m_pIBindResource->Release(), 0);

//CLEANUP:
	SAFE_RELEASE(m_pCThisTestModule);
	PROVIDER_FREE(m_pwszTableName);
	PROVIDER_FREE(m_pwszFileName);
	PROVIDER_FREE(m_pwszDefaultQuery);
	PROVIDER_FREE(m_pwszRowScopedQuery);
	PROVIDER_FREE(m_pwszEnumerator);
	PROVIDER_FREE(m_pwszRootBinderProgID);
	PROVIDER_FREE(m_pwszRootURL);
	PROVIDER_FREE(m_pwszBackend);
	PROVIDER_FREE(m_pwszBackendVer);
	PROVIDER_FREE(m_pwszProviderVer);

	SAFE_DELETE(m_pCError);
	SAFE_DELETE(m_pCParseInitFile);
	SAFE_DELETE(m_pCLocaleInfo);
	SAFE_DELETE(m_pCPoolManager);
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::Init
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::Init(CThisTestModule* pCThisTestModule)
{	
   OSVERSIONINFO osverinfo;
	TRACE_CALL(L"PRIVLIB: CModInfo::Init.\n");

	SAFE_ADDREF(pCThisTestModule);
	m_pCThisTestModule = pCThisTestModule;

   osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&osverinfo);
   if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	  m_fIsWin9x = TRUE;

	//Setup the InitString
	//We want CModInfo to be able to operate outside of the LTM Framework,
	//So if we have a TestModule will just that InitString, otherwise
	//the standalone consumer may have setup the inti string themselves..
	if(pCThisTestModule)
	{
		SetInitString(m_pCThisTestModule->m_pwszInitString);
		SetProviderCLSID(m_pCThisTestModule->m_ProviderClsid);
		SetClassContext(m_pCThisTestModule->m_clsctxProvider);
		SetRemoteMachine(m_pCThisTestModule->m_pwszMachineName);

		if(ParseAll())
			return CreateRootBinder();
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::CreateRootBinder
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::CreateRootBinder()
{
	CLSID	clsid;
	HRESULT	hr;

	//If a Root Binder other than MSDAURL.Binder is to be used than 
	//its ProgID will be given by m_pwszRootBinderProgID.

	TRACE_CALL(L"PRIVLIB: CModInfo::CreateRootBinder.\n");

	if(m_pwszRootBinderProgID)
	{
		if(FAILED(CLSIDFromProgID(m_pwszRootBinderProgID, &clsid)))
			if(FAILED(CLSIDFromString(m_pwszRootBinderProgID, &clsid)))
				return FALSE;

		if((hr = CoCreateInstance(clsid, NULL, 
			GetClassContext(),   //-  TO DO: Change later (sirajl)
			//CLSCTX_INPROC_SERVER,
			IID_IBindResource, (void**)&m_pIBindResource)) == S_OK)
			return TRUE;
	}
	else
	{
		//There is no error checking in this part. This is to avoid
		//the pre 2.5 tests from failing.

		if(FAILED(CLSIDFromProgID(L"MSDAURL.Binder", &clsid)))
			CLSIDFromString(L"MSDAURL.Binder", &clsid);

		hr = CoCreateInstance(clsid, NULL, 
			GetClassContext(),     //-  TO DO: Change later (sirajl)
			//CLSCTX_INPROC_SERVER,
			IID_IBindResource, (void**)&m_pIBindResource);

		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::CreateProvider
//
//////////////////////////////////////////////////////////////////////////
HRESULT CModInfo::CreateProvider(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::CreateProvider.\n");

	REFCLSID clsidProvider = GetProviderCLSID();
	
	//Error Checking
	//Its fairly common for first time users of LTM and our tests to forget to select
	//a provider.  If we just call CoCreateInstance with CLSID_NULL its not a very useful 
	//error, just provide more info as this is a common senario.  LTM doesn't display an
	//error since many tests (SQL Server, RDS, etc) do not require a provider or run against...
	if(clsidProvider == CLSID_NULL)
	{
		TERROR(L"You have not selected a provider to run against.\n" << 
			L"Create an Alias in LTM with a selected provider, and then select it.");
		return REGDB_E_CLASSNOTREG;
	}
	
	//delegate
	return CreateProvider(clsidProvider, pIUnkOuter, riid, ppIUnknown, dwOptions);
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::CreateProvider
//
//////////////////////////////////////////////////////////////////////////
HRESULT CModInfo::CreateProvider(CLSID clsid, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions)
{
	HRESULT hr = S_OK;
	IDataInitialize* pIDataInitialize = NULL;

	TRACE_CALL(L"PRIVLIB: CModInfo::CreateProvider.\n");

	//Create an Instance of the Provider 
	//This wrapper should always be used instead of directly using CoCreateInstance!!!
	
	//1. It allows us to use the Correct provider CLSID
	//2. It allows us to use the correct CLSCTX (local, or remote server)
	//3. It allows us to hook up service components, if needed...

	//ServiceComponents
	if(UseServiceComponents())
	{
		//Create Instance of Service Components (first)
		QTESTC_(hr = CoCreate(CLSID_MSDAINITIALIZE, NULL, GetClassContext(), IID_IDataInitialize, (LPVOID *)&pIDataInitialize),S_OK);

		//Now obtain instance of Provider
		//Always create the provider relative to the service components.
		QTESTC_(hr = pIDataInitialize->CreateDBInstance(clsid, pIUnkOuter, CLSCTX_INPROC_SERVER, NULL, dwOptions ? IID_IUnknown : riid, (IUnknown**)ppIUnknown),S_OK);
	}
	else
	{
		//Create Instance of Provider (directly)
		QTESTC_(hr = CoCreate(clsid, pIUnkOuter, GetClassContext(), dwOptions ? IID_IUnknown : riid, (LPVOID *)ppIUnknown),S_OK);
	}

	//Initialize the DSO (if requested)
	if(dwOptions && ppIUnknown)
	{
		hr = InitializeDataSource(*ppIUnknown, dwOptions);

		//Now obtain the requested interface.
		if(!VerifyInterface(*ppIUnknown, riid, DATASOURCE_INTERFACE, ppIUnknown))
			QTESTC_(hr = E_NOINTERFACE, S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIDataInitialize);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CModInfo::InitializeDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CModInfo::InitializeDataSource(IUnknown* pDataSource, DWORD dwOptions)
{
	ASSERT(pDataSource);

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBInitialize* pIDBInitialize = NULL; 
	IDBProperties* pIDBProperties = NULL; 
	HRESULT hr = S_OK;

	//SetProperties (if requested)
	if(dwOptions & CREATEDSO_SETPROPERTIES)
	{
		//Build our init options from string passed to us from the LTM InitString
		GetInitProps(&cPropSets,&rgPropSets);

		if(!VerifyInterface(pDataSource, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
			QTESTC_(hr = E_NOINTERFACE, S_OK);
		QTESTC_(hr = pIDBProperties->SetProperties(cPropSets,rgPropSets),S_OK)
	}
	
	//Initialize (if requested)
	if(dwOptions & CREATEDSO_INITIALIZE)
	{
		if(!VerifyInterface(pDataSource, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize))
			QTESTC_(hr = E_NOINTERFACE, S_OK);
		QTESTC_(hr = pIDBInitialize->Initialize(),S_OK)
	}

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);
	FreeProperties(&cPropSets,&rgPropSets);
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::DropTable
//
//////////////////////////////////////////////////////////////////////////
HRESULT CModInfo::DropTable()
{	
	IOpenRowset* pIOpenRowset = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;
	HRESULT hr = S_OK;

	//If using a "static" INI File provided table and the user really 
	//wants to drop it (ie: automation), then we will drop it if indicated in the initstring
	if(GetFileName() && GetTableName() && (GetTableOpts() & TABLE_DROPALWAYS))
	{
		//In order to drop the table we need a connection to the Provider
		//We could do this in ReleaseDBSession (where we already have a connection), but some tests
		//do not call this (don't need a session), or some test call it nested or multiple times.
		QTESTC_(hr = GetModInfo()->CreateProvider(NULL, IID_IDBCreateSession, (IUnknown**)&pIDBCreateSession, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
		QTESTC_(hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)&pIOpenRowset),S_OK);

		//Create the CTable object (using the Privlib), so we can re-use the "DropTable" functionality
		CTable sCTable(pIOpenRowset);
		sCTable.SetTableName(GetTableName());
		hr = sCTable.DropTable(TRUE/*fDropAlways*/);
	}

CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIOpenRowset);
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::ParseAll
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::ParseAll()
{	
	TRACE_CALL(L"PRIVLIB: CModInfo::ParseAll.\n");

	//No-op
	if(m_pwszInitString == NULL)
		return TRUE;

	//1.  We need to obtain the FILE=filename from the InitString
	if(!ParseInitFileName())
		return FALSE;
	
	//2.  We should parse the INI FileFirst.  This allows use to copy the 
	//InitString from the file and append to our InitString before parsing it
	if(!ParseInitFile())
		return FALSE;

	//3.  Now we can parse the InitString (fully)
	if(!ParseInitString())
		return FALSE;

	//4. Initialize backend info
	if(!InitBackendInfo())
		return FALSE;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::SetTableName
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::SetTableName(WCHAR* pwszTableName)
{
	ASSERT(pwszTableName);

	TRACE_CALL(L"PRIVLIB: CModInfo::SetTableName.\n");

	//Reassign the TableName
	PROVIDER_FREE(m_pwszTableName);
	m_pwszTableName = wcsDuplicate(pwszTableName);

	//Inidicate success
	return m_pwszTableName != NULL;
}

//////////////////////////////////////////////////////////////////////////
// CModInfo::ResetIniFile
//
//////////////////////////////////////////////////////////////////////////
void CModInfo::ResetIniFile()
{
	PROVIDER_FREE(m_pwszTableName);
	PROVIDER_FREE(m_pwszFileName);
}

//////////////////////////////////////////////////////////////////////////
// CModInfo::SetDefaultQuery
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::SetDefaultQuery(WCHAR* pwszDefaultQuery)
{
	ASSERT(pwszDefaultQuery);

	TRACE_CALL(L"PRIVLIB: CModInfo::SetDefaultQuery.\n");

	//Reassign the TableName
	PROVIDER_FREE(m_pwszDefaultQuery);
	m_pwszDefaultQuery = wcsDuplicate(pwszDefaultQuery);

	//Inidicate success
	return m_pwszDefaultQuery != NULL;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::SetRowScopedQuery
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::SetRowScopedQuery(WCHAR* pwszRowScopedQuery)
{
	ASSERT(pwszRowScopedQuery);

	TRACE_CALL(L"PRIVLIB: CModInfo::SetRowScopedQuery.\n");

	//Reassign the value
	PROVIDER_FREE(m_pwszRowScopedQuery);
	m_pwszRowScopedQuery = wcsDuplicate(pwszRowScopedQuery);

	//Inidicate success
	return m_pwszRowScopedQuery != NULL;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::SetInitString
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::SetInitString(WCHAR* pwszString)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::SetInitString.\n");

	//Add the passed in string to the current InitString Value.
	if(pwszString == NULL)
		return FALSE;

	PROVIDER_FREE(m_pwszInitString);
	m_pwszInitString = wcsDuplicate(pwszString);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::SetRootURL
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::SetRootURL(WCHAR* pwszRootURL)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::SetRootURL.\n");

	//Add the passed in string to the current InitString Value.
	if(pwszRootURL == NULL)
		return FALSE;

	PROVIDER_FREE(m_pwszRootURL);
	m_pwszRootURL = wcsDuplicate(pwszRootURL);
	return m_pwszRootURL != NULL;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::AddToInitString
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::AddToInitString(CHAR* pszString)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::AddToInitString.\n");

	//no-op
	if(pszString == NULL)
		return TRUE;

	//Convert to WCHAR
	WCHAR* pwszNewString = ConvertToWCHAR(pszString);

	//no-op (trival)
	if(m_pwszInitString == NULL)
	{
		m_pwszInitString = pwszNewString;
		return TRUE;
	}

	//Take the existing Init string and append the additional information.
	//If there was an exsiting string, we need to add a semicolon to seperate
	//the previous items from the new ones, (in case the original string didn't
	//have a ending ";")

	//Calulcate the new length... (include the null terminator and additional
	//room for a trailing semicolon and an extra space...
	size_t dwLength = wcslen(m_pwszInitString) + wcslen(pwszNewString) + 1 + 2;
	
	//Grow the existing string
	m_pwszInitString = (WCHAR*)PROVIDER_REALLOC(m_pwszInitString, dwLength*sizeof(WCHAR));
	//Add the trailing Semicolon and extra space
	wcscat(m_pwszInitString, L"; ");
	//Append the string
	wcscat(m_pwszInitString, pwszNewString);
		
	PROVIDER_FREE(pwszNewString);
	return TRUE;
}

	
//////////////////////////////////////////////////////////////////////////
// CModInfo::GetFriendlyNameValue
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::GetFriendlyNameValue(DBPROPID dwPropertyID, WCHAR* pwszName, LONG* plValue)
{
	ASSERT(pwszName);
	ASSERT(plValue);
	LONG lValue = 0;
	BOOL bFound = TRUE;

	TRACE_CALL(L"PRIVLIB: CModInfo::GetFriendlyNameValue.\n");

	//Special property friendly names
	switch(dwPropertyID)
	{
		case DBPROP_INIT_PROMPT:
		{
			bFound = TRUE;
			if(_wcsicmp(pwszName, L"DBPROMPT_PROMPT")==0 || _wcsicmp(pwszName, L"PROMPT")==0)
				lValue =  DBPROMPT_PROMPT;
			else if(_wcsicmp(pwszName, L"DBPROMPT_COMPLETEREQUIRED")==0 || _wcsicmp(pwszName, L"COMPLETEREQUIRED")==0)
				lValue =  DBPROMPT_COMPLETEREQUIRED;	
			else if(_wcsicmp(pwszName, L"DBPROMPT_COMPLETE")==0 || _wcsicmp(pwszName, L"COMPLETE")==0)
				lValue =  DBPROMPT_COMPLETE;						
			else if(_wcsicmp(pwszName, L"DBPROMPT_NOPROMPT")==0 || _wcsicmp(pwszName, L"NOPROMPT")==0)
				lValue =  DBPROMPT_NOPROMPT;
			else
				bFound = FALSE;
			break;
		}

		case DBPROP_INIT_HWND:
		{
			if(_wcsicmp(pwszName, L"DESKTOP")==0)
				lValue =  HandleToLong(GetDesktopWindow());
			else
				bFound = FALSE;
			break;
		}

		case DBPROP_INIT_MODE:
		{
			//Orable options are a pain, expecially when some or'd names are
			//contained with other or'd names...
			//This may be a slow solution, but works for all cases...
			WCHAR* pwszDupString = wcsDuplicate(pwszName);
			WCHAR* pwszToken = wcstok( pwszDupString, L"|");

			while(pwszToken) 
			{
				//Skip any Leading whitespace
				pwszToken = SkipWhiteSpace(pwszToken, NULL, L" ");

				//Skip any trailing whitespace
				WCHAR* pwszEnd = SkipWhiteSpace(pwszToken+wcslen(pwszToken)-1, pwszToken, L" ");
				if(pwszEnd)
					(++pwszEnd)[0] = L'\0';

				if(_wcsicmp(pwszToken, L"DB_MODE_READ")==0 || _wcsicmp(pwszToken, L"READ")==0)
					lValue |= DB_MODE_READ;
				else if(_wcsicmp(pwszToken, L"DB_MODE_WRITE")==0 || _wcsicmp(pwszToken, L"WRITE")==0)
					lValue |= DB_MODE_WRITE;
				else if(_wcsicmp(pwszToken, L"DB_MODE_READWRITE")==0 || _wcsicmp(pwszToken, L"READWRITE")==0)
					lValue |= DB_MODE_READWRITE;
				else if(_wcsicmp(pwszToken, L"DB_MODE_SHARE_DENY_NONE")==0 || _wcsicmp(pwszToken, L"SHARE DENY NONE")==0)
					lValue |= DB_MODE_SHARE_DENY_NONE;
				else if(_wcsicmp(pwszToken, L"DB_MODE_SHARE_DENY_READ")==0 || _wcsicmp(pwszToken, L"SHARE DENY READ")==0)
					lValue |= DB_MODE_SHARE_DENY_READ;
				else if(_wcsicmp(pwszToken, L"DB_MODE_SHARE_DENY_WRITE")==0 || _wcsicmp(pwszToken, L"SHARE DENY WRITE")==0)
					lValue |= DB_MODE_SHARE_DENY_WRITE;
				else if(_wcsicmp(pwszToken, L"DB_MODE_SHARE_EXCLUSIVE")==0 || _wcsicmp(pwszToken, L"SHARE EXCLUSIVE")==0)
					lValue |= DB_MODE_SHARE_EXCLUSIVE;
			
				//Get the next token
				pwszToken = wcstok(NULL, L"|");
			}

			if(lValue == 0)
				bFound = FALSE;
			SAFE_FREE(pwszDupString);
			break;
		}
				
		case DBPROP_INIT_IMPERSONATION_LEVEL:
		{
			if(_wcsicmp(pwszName, L"DB_IMP_LEVEL_ANONYMOUS")==0)
				lValue = DB_IMP_LEVEL_ANONYMOUS;
			else if(_wcsicmp(pwszName, L"DB_IMP_LEVEL_IDENTIFY")==0)
				lValue = DB_IMP_LEVEL_IDENTIFY;
			else if(_wcsicmp(pwszName, L"DB_IMP_LEVEL_IMPERSONATE")==0)
				lValue = DB_IMP_LEVEL_IMPERSONATE;
			else if(_wcsicmp(pwszName, L"DB_IMP_LEVEL_DELEGATE")==0)
				lValue = DB_IMP_LEVEL_DELEGATE;
			else 
				bFound = FALSE;
			break;
		}

		case DBPROP_INIT_PROTECTION_LEVEL:
		{
			if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_NONE")==0 || _wcsicmp(pwszName, L"NONE")==0)
				lValue = DB_PROT_LEVEL_NONE;
			else if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_CONNECT")==0 || _wcsicmp(pwszName, L"CONNECT")==0)
				lValue = DB_PROT_LEVEL_CONNECT;
			else if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_CALL")==0 || _wcsicmp(pwszName, L"CALL")==0)
				lValue = DB_PROT_LEVEL_CALL;
			else if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_PKT")==0 || _wcsicmp(pwszName, L"PKT")==0)
				lValue = DB_PROT_LEVEL_PKT;
			else if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_PKT_INTEGRITY")==0 || _wcsicmp(pwszName, L"PKT INTEGRITY")==0)
				lValue = DB_PROT_LEVEL_PKT_INTEGRITY;
			else if(_wcsicmp(pwszName, L"DB_PROT_LEVEL_PKT_PRIVACY")==0 || _wcsicmp(pwszName, L"PKT_PRIVACY")==0)
				lValue = DB_PROT_LEVEL_PKT_PRIVACY;
			else
				bFound = FALSE;
			break;
		}

		case DBPROP_INIT_ASYNCH:
		{
			if(_wcsicmp(pwszName, L"DBPROPVAL_ASYNCH_INITIALIZE")==0 || _wcsicmp(pwszName, L"INITIALIZE")==0)
				lValue = DBPROPVAL_ASYNCH_INITIALIZE;
			else 
				bFound = FALSE;
			break;
		}

		default:
			bFound = FALSE;
			break;

	};

	//If unable to find Friendly name
	//We can try other conversions...
	if(!bFound)
	{
		//Handle Boolean cases...
		bFound = TRUE;
		if(_wcsicmp(pwszName, L"VARIANT_TRUE")==0 || _wcsicmp(pwszName, L"TRUE")==0)
			lValue = VARIANT_TRUE;
		else if(_wcsicmp(pwszName, L"VARIANT_FALSE")==0 || _wcsicmp(pwszName, L"FALSE")==0)
			lValue = VARIANT_FALSE;
		else
		{
			//Property Currently doesn't have an existing
			//or known freindly name, we will just try to return the integer
			//equivilent if we can, otherwise FALSE...
			WCHAR* pwszEnd = NULL;
			lValue = wcstol(pwszName, &pwszEnd, 0);
			
			//Conversion failure...
			if(pwszEnd== NULL || pwszEnd[0]!=L'\0')
				bFound = FALSE;
		}
	}

	*plValue = lValue;
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::GetInitProps
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::GetInitProps(ULONG*	pcPropSets, DBPROPSET**	prgPropSets)
{
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);

	TRACE_CALL(L"PRIVLIB: CModInfo::GetInitProps.\n");

	BOOL	bValue = FALSE;
	WCHAR*  pwszValue = NULL;
	WCHAR*  pwszProvProp = NULL;
	WCHAR*  pwszStart = NULL;
	
	DWORD	i,dwPropertyID;
	DBTYPE	wType;
	GUID	guidPropertySet;
	VARIANT vVariant;
	VariantInit(&vVariant);
	
	HRESULT hr = S_OK;
	IDataInitialize* pIDataInitialize = NULL;
	IDBProperties* pIDBProperties = NULL;

	//Null output params
	*pcPropSets = 0;
	*prgPropSets = NULL;

	//no-op
	if(m_pwszInitString == NULL)
	{
		bValue = TRUE;
		goto CLEANUP;
	}

	//Performance Optimization:
	//If we have already parsed the init string once, no need to redo it again.  Just duplicate
	//the properties and return them...
	if(m_fParsedProps)
	{
		bValue = DuplicatePropertySets(m_cInitPropSets, m_rgInitPropSets, pcPropSets, prgPropSets);
		goto CLEANUP;
	}

	//First see if the user provided a ConnectionString="..."
	//(similar to ADO were we pass whatever they use to DSL which parses this based upon property
	//descriptions).
	if(GetInitStringValue(L"CONNECTIONSTRING", &pwszValue) && pwszValue)
	{
		CLSID clsidProvider = GetProviderCLSID();

		//First Create DSL (Service Components)
		TESTC_(hr = CoCreate(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, IID_IDataInitialize, (void**)&pIDataInitialize),S_OK);
		
		//Now create the provider...
		//By passing in an existing DataSource to IDataInitialize::GetDataSource, we solve two major issues.  
		//1.  This indicates which provider to use so we don't have to add this to the 
		//		InitString (which would require reallocation, getting the progID, etc)
		//2.  It only sets these properties, and we don't get an SC DataSource back which supports
		//		more properties than the provider and would cause problems at SetProperties
		//		when not using Service Components.
		//3.  Note: A provider passed in is not required, the user may want to default to MSDASQL
		//		or it may have the provider keyword in the InitString...
		if(clsidProvider != GUID_NULL)
			TESTC_(hr = CreateProvider(clsidProvider, NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties),S_OK);

		//IDataInitialize::GetDataSource passing in the existing DataSource and the 
		//string the user specified...  SC will parse all the properties, and set those properties
		//on the DataSource we passed in...
		TESTC_(hr = pIDataInitialize->GetDataSource(NULL, CLSCTX_INPROC_SERVER, pwszValue, IID_IDBProperties, (IUnknown**)&pIDBProperties),S_OK);

		//Obtain all the properties on the datasource
		TESTC_(hr = pIDBProperties->GetProperties(0, NULL, pcPropSets, prgPropSets),S_OK);

		//NOTE:  We don't want all the "default" empty properties that GetProperties returns.
		//1.  Since we connected using DSL it will create a DataSource that is wrapped with SC
		//		  which unfortunatly supports additional properties than the provider.  This causes
		//		  a problem if we turn arround and pass these properties to the provider without
		//		  Service Components.  Thankfully before Initialization these "extra" DSL properties
		//		  are returned as VT_EMPTY, if the user has not set them, so we can "easily" remove them
		//2.  Its just a waste of time and extra overhead to always pass-arround and 
		//		  set properties that are the default anyway...
		TESTC_(CompactProperties(pcPropSets, prgPropSets, DBTYPE_EMPTY),S_OK);
		PROVIDER_FREE(pwszValue);
	}

	//Loop over all possible Init Options
	for(i=0; i<g_cInitPropInfoMap; i++)
	{
		//Grab the value from the init string
		//Allow either the property name or the property description...
		if(!GetInitStringValue(g_rgInitPropInfoMap[i].pwszName, &pwszValue) && 
			!GetInitStringValue(g_rgInitPropInfoMap[i].pwszDesc, &pwszValue))
			continue;
		
		VariantInit(&vVariant);
		
		//NOTE: pwszValue = NULL means the keyword was specified, but as Keyword=; meaning
		//VT_EMPTY, as opposed to Keyword=""; which would return a string but null terminated.
		if(pwszValue)
		{
			V_VT(&vVariant) = g_rgInitPropInfoMap[i].vt;

			// Convert the string value the user passed to the correct variant type
			switch(V_VT(&vVariant))
			{				
				case VT_BOOL:
					GetFriendlyNameValue(g_rgInitPropInfoMap[i].dwPropertyID, pwszValue, (LONG*)&V_BOOL(&vVariant));
					break;
				
				case VT_BSTR:
					V_BSTR(&vVariant) = SysAllocString(pwszValue);
					break;

				// We will ignore anything after a valid match, such
				// as white spaces, or any other non ";" character.
				case VT_I2:
					GetFriendlyNameValue(g_rgInitPropInfoMap[i].dwPropertyID, pwszValue, (LONG*)&V_I2(&vVariant));
					break;
										
				// We will ignore anything after a valid match, such
				// as white spaces, or any other non ";" character.
				case VT_I4:
					GetFriendlyNameValue(g_rgInitPropInfoMap[i].dwPropertyID, pwszValue, &V_I4(&vVariant));
				
				default:
					GetFriendlyNameValue(g_rgInitPropInfoMap[i].dwPropertyID, pwszValue, &V_I4(&vVariant));
					break;
			}
		}

		//Now set the Property
		SetProperty(g_rgInitPropInfoMap[i].dwPropertyID, DBPROPSET_DBINIT, pcPropSets, prgPropSets, DBTYPE_VARIANT, &vVariant); 
		VariantClear(&vVariant);
		PROVIDER_FREE(pwszValue);
	}

	//Check for any provider specific properties to the set
	pwszStart = FindSubString(m_pwszInitString, L"PROVPROP");
	GetStringKeywordValue(pwszStart, L"PROVPROP", &pwszProvProp);
	while(pwszProvProp)
	{
		VariantInit(&vVariant);
		
		//Find DBPROPID=
		TESTC(GetStringKeywordValue(pwszProvProp, L"DBPROPID", &pwszValue));
		dwPropertyID = wcstoul(pwszValue, NULL, 10);
		PROVIDER_FREE(pwszValue);

		//Find DBTYPE=
		TESTC(GetStringKeywordValue(pwszProvProp, L"DBTYPE", &pwszValue));
		wType = GetDBType(pwszValue);
		PROVIDER_FREE(pwszValue);

		//Find VALUE=
		TESTC(GetStringKeywordValue(pwszProvProp, L"VALUE", &pwszValue));
		TESTC_(StringToVariant(pwszValue, wType, &vVariant),S_OK);
		PROVIDER_FREE(pwszValue);

		//Find DBPROPSET=
		TESTC(GetStringKeywordValue(pwszProvProp, L"DBPROPSET", &pwszValue));
		TESTC_(CLSIDFromString(pwszValue, &guidPropertySet),S_OK);

		//Now add this property to the existing propsets...
		SetProperty(dwPropertyID, guidPropertySet, pcPropSets, prgPropSets, DBTYPE_VARIANT, &vVariant);
		VariantClear(&vVariant);

		//Find the next ProviderSpecific Property
		PROVIDER_FREE(pwszProvProp);
		pwszStart = FindSubString(++pwszStart, L"PROVPROP");
		GetStringKeywordValue(pwszStart, L"PROVPROP", &pwszProvProp);
		PROVIDER_FREE(pwszValue);
	}

	//If we have made it this far, save the properties
	bValue = DuplicatePropertySets(*pcPropSets, *prgPropSets, &m_cInitPropSets, &m_rgInitPropSets);
	if(bValue)
		m_fParsedProps = TRUE;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDataInitialize);

	VariantClear(&vVariant);
	PROVIDER_FREE(pwszValue);
	PROVIDER_FREE(pwszProvProp);
	return bValue;	
}	



//////////////////////////////////////////////////////////////////////////
// CModInfo::GetInitStringValue
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::GetInitStringValue(WCHAR* pwszKeyword, WCHAR** ppwszValue)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::GetInitStringValue.\n");
	//Delegate
	return GetStringKeywordValue(m_pwszInitString, pwszKeyword, ppwszValue);
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::FindStringValue
//
//////////////////////////////////////////////////////////////////////////
WCHAR* CModInfo::FindStringValue(WCHAR* pwszString, WCHAR* pwszKeyword)
{
	TRACE_CALL(L"PRIVLIB: CModInfo::FindStringValue.\n");

	if(!pwszKeyword)
		return NULL;

	//Find the Keyword within the specified string.
	//This function will always take the last specified keyword...
	//Once found the Keyword, return a pointer to the start of the 
	//corresponding value...

	//No-op
	if(pwszString == NULL)
		return pwszString;

	WCHAR* pwszFound	= NULL;
	WCHAR* pwszToken	= NULL;
	WCHAR* pwszNext		= pwszString;

	//Continue to Loop until we find the last Keyword...
	//(That is a correct match)
	while(pwszNext)
	{
		//Find the required Keyword (case-insensitive)
		pwszToken = pwszNext = FindSubString(pwszNext, pwszKeyword);
		if(pwszNext == NULL)
			continue;

		//Now that we found a keyword
		//Make sure its really a keyword and we didn't just find the string in
		//the middle of another value, should be (Keyword = Value;)
		WCHAR* pwszBefore = pwszNext == pwszString ? pwszNext : pwszNext - 1;
		pwszNext = pwszNext + wcslen(pwszKeyword);
	
		//1.  Make sure we have the following "="
		pwszNext = SkipWhiteSpace(pwszNext, NULL, L" \t\n\r");
		if(pwszNext == NULL || pwszNext[0] != L'=')
			continue;

		//2.  Make sure its preceded by a ";", " ", or is the first Keyword of the string
		pwszBefore = SkipWhiteSpace(pwszBefore, pwszString, L" \t\n\r");
		if(!(pwszBefore[0] == L';' || pwszBefore[0] == L' ' || pwszBefore == pwszString))
			continue;

		//3.  Make sure its not in an embedded "" value, where you are allowed to
		//	  to have keywords that won't get flagged upon...
		WCHAR chQuote = 0;
		ULONG cQuotes = 0;
		pwszBefore = pwszString;
		while(pwszBefore < pwszToken)
		{
			//Quote Character
			if(pwszBefore[0] == '\"' || pwszBefore[0] == '\'')
			{
				//First Quote - remember which quote char used
				if(cQuotes==0)
					chQuote = pwszBefore[0];
			
				//Count this qoute
				if(pwszBefore[0] == chQuote)
					cQuotes = cQuotes ? cQuotes-1 : cQuotes+1;
			}
			pwszBefore++;
		}		
		
		//If we have any unbalanced quotes (cQuotes != 0)
		//Then we know that this keyword is inside or embedded in quotes, so 
		//its not a valid keyword as its part of something else...
		if(cQuotes)
			continue;

		//Make sure we don't have "==", this is not allowed, as it not allowed
		//by ServiceComponents either, Keyword==, is treated as if the actual
		//keyword contains a equal sign, (double it to indicate this...)
		pwszNext++;
		if(pwszNext[0] == L'=')
			continue;

		//Skip over any following whitespace after the equal sign...
		pwszNext = SkipWhiteSpace(pwszNext);

		//If we have made it through all the above checks
		//And we still have a valid string, we have found a correct keyword...
		if(pwszNext)
			pwszFound = pwszNext;
	}

	return pwszFound;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::GetStringKeywordValue
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::GetStringKeywordValue(WCHAR* pwszString, WCHAR* pwszKeyword, WCHAR** ppwszValue)
{
	ASSERT(pwszKeyword);
	ASSERT(ppwszValue);
	*ppwszValue = NULL;

	TRACE_CALL(L"PRIVLIB: CModInfo::GetStringKeywordValue.\n");

	//Find the Start of the Value
	WCHAR*	pwszStart		= FindStringValue(pwszString, pwszKeyword);
	WCHAR*	pwszEndToken	= NULL;
	BOOL	fInsideQuotes   = FALSE;

	//Unable to find Keyword
	if(pwszStart == NULL)
		return FALSE;
	
	//Handle the case where there is no final ";", not really
	//needed we will just use the terminator...
	pwszEndToken = wcsstr(pwszStart, L";");
	if(pwszEndToken == NULL)
		pwszEndToken = pwszStart + wcslen(pwszStart);
			
	//Remove any trailing Whitespace...
	//IE:  Keyword=value ; 
	if(pwszEndToken > pwszStart)
	{
		pwszEndToken = SkipWhiteSpace(pwszEndToken-1, pwszStart, L" \t\n\r");
		pwszEndToken++;
	}


	if(pwszEndToken)
	{
		WCHAR wszQuoteChar[3] = { 0, L';', L'\0'};
		
		//If the string is contained in quotes, 
		//then ignore everything in between the quotes
		if(pwszStart[0] == L'\"' || pwszStart[0] == L'\'') 
		{					
			fInsideQuotes = TRUE;

			//Pull out the string which is enclosed in quotes
			//Must have (";) to indicate the end, since the string itself
			//is probably going to have semicolons embedding (the reason its being quoted...)
			wszQuoteChar[0] = pwszStart[0];
			pwszStart++;
			WCHAR* pwszEndQuote = wcsstr(pwszStart, wszQuoteChar);
			
			//If there is no closing quote (";) then its an error,
			//(unmatched quotes!), just use the end of line char, since they may
			//have forgot the required trailing semicolon...
			if(pwszEndQuote)
			{	
				pwszEndToken = pwszEndQuote;
			}
			else
			{
				pwszEndToken = pwszStart + wcslen(pwszStart);
				
				//Skip any trailing whitespace
				pwszEndToken = SkipWhiteSpace(pwszEndToken, pwszStart, L" ;\t\n\r");
				if(pwszEndToken[0] != wszQuoteChar[0])
					pwszEndToken++;
			}
		}

		ULONG_PTR ulLength = pwszEndToken - pwszStart;
		ASSERT(pwszEndToken >= pwszStart);
		
		//If the length = 0 (ie: Keyword=;) treat this as VT_EMPTY...
		//The only exception is if they really want a NULL string, and this is 
		//represented by using Quote chars (ie: Keyword="";)
		if(!fInsideQuotes && ulLength == 0)
			return TRUE;
		
		//Copy value to the output string, (plus null terminator)
		*ppwszValue = (WCHAR*)PROVIDER_ALLOC((ulLength+1)*sizeof(WCHAR));
		wcsncpy(*ppwszValue, pwszStart, (size_t)ulLength);

		//Null terminator
		(*ppwszValue)[ulLength] = L'\0';
		
		//The IDataInitialize spec indicates all doubled Quote chars will be
		//reduced, if they are within the same outside quote character...
		if(wszQuoteChar[0])
		{
			wszQuoteChar[1] = wszQuoteChar[0];
			wszQuoteChar[2] = L'\0';
			pwszStart = *ppwszValue;
			while(pwszStart = wcsstr(pwszStart, wszQuoteChar))
			{
				ReplaceString(pwszStart, &wszQuoteChar[0], &wszQuoteChar[1]);
				pwszStart++;
			}
		}

		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::ParseInitFileName
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::ParseInitFileName()
{	
	TRACE_CALL(L"PRIVLIB: CModInfo::ParseInitFileName.\n");

	//FILE - name of "ini" file
	GetInitStringValue(L"FILE", &m_pwszFileName);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::ParseInitFile
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::ParseInitFile()
{	
	TRACE_CALL(L"PRIVLIB: CModInfo::ParseInitFile.\n");

	//Parse the "ini" file
	ASSERT(m_pCParseInitFile);
	if(m_pwszFileName)
		return GetParseObject()->Init(m_pwszFileName);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CModInfo::ParseInitString
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::ParseInitString()
{	
	WCHAR* pwszValue = NULL;
	
	TRACE_CALL(L"PRIVLIB: CModInfo::ParseInitString.\n");

	if(m_pwszInitString == NULL)
		return FALSE;

	//TABLE - name of the table is using SetExistingTable()
	if(!GetInitStringValue(L"TABLE", &m_pwszTableName))
		GetInitStringValue(L"TABLENAME", &m_pwszTableName);
	
	//TABLE_OPTIONS
	m_dwTableOpts = 0;
	if(GetInitStringValue(L"TABLEOPT", &pwszValue))
	{
		if(FindSubString(pwszValue, L"DROP"))
			m_dwTableOpts |= TABLE_DROPALWAYS;
		PROVIDER_FREE(pwszValue);
	}

	//DEFAULTQUERY - name of DefaultQuery
	GetInitStringValue(L"DEFAULTQUERY", &m_pwszDefaultQuery);
	if(m_pwszDefaultQuery && m_pwszTableName==NULL)
	{
		odtLog << "ERROR:  InitString DEFAULTQUERY= also requires TABLENAME=\n";
		return FALSE;
	}

	//ROWSCOPEDQUERY - name of Row Scoped Query
	GetInitStringValue(L"ROWQUERY", &m_pwszRowScopedQuery);

	//LEVEL - level of the provider
	if(GetInitStringValue(L"CONFLEVEL", &pwszValue))
	{
		m_dwProviderLevel = 0;

		//Levels
		if(FindSubString(pwszValue, L"CONF_LEVEL_0"))
			m_dwProviderLevel |= CONF_LEVEL_0;
		if(FindSubString(pwszValue, L"CONF_LEVEL_1"))
			m_dwProviderLevel |= CONF_LEVEL_1;
		if(FindSubString(pwszValue, L"CONF_LEVEL_2"))
			m_dwProviderLevel |= CONF_LEVEL_2;

		//Additional Functionality
		if(FindSubString(pwszValue, L"CONF_UPDATEABLE"))
			m_dwProviderLevel |= CONF_UPDATEABLE;
		if(FindSubString(pwszValue, L"CONF_TRANSACTIONS"))
			m_dwProviderLevel |= CONF_TRANSACTIONS;
		if(FindSubString(pwszValue, L"CONF_COMMANDS"))
			m_dwProviderLevel |= CONF_COMMANDS;
		if(FindSubString(pwszValue, L"CONF_FILTERS"))
			m_dwProviderLevel |= CONF_FILTERS;
		if(FindSubString(pwszValue, L"CONF_INDEXES"))
			m_dwProviderLevel |= CONF_INDEXES;

		//Get the Leveling Strictness
		if(FindSubString(pwszValue, L"CONF_STRICT"))
			m_fStrictConformance = TRUE;
		PROVIDER_FREE(pwszValue);
	}


	//TABLE_CREATION - the method used to create table
	if(GetInitStringValue(L"CREATE_TBL", &pwszValue))
	{
		// property
		if(_wcsicmp(pwszValue, L"ITableDefinition")==0)
			m_fTableCreation = TRUE;
		else if(_wcsicmp(pwszValue, L"Command")==0)
			m_fTableCreation = FALSE;
		PROVIDER_FREE(pwszValue);
	}

	//ROWSET_INDEX - the method used to create table
	if(GetInitStringValue(L"ROWSET_INDEX", &pwszValue))
	{
		// property
		if(_wcsicmp(pwszValue, L"NO")==0)
			m_fTableCreation = FALSE;
		PROVIDER_FREE(pwszValue);
	}

	//INSERT_METHOD - the method used to create table
	if(GetInitStringValue(L"INSERT_METHOD", &pwszValue))
	{
		// property
		if(_wcsicmp(pwszValue, L"InsertWithParameters")==0)
			m_dwInsert = INSERT_WITHPARAMS;
		else if(_wcsicmp(pwszValue, L"Command")==0)
			m_dwInsert = INSERT_COMMAND;
		else if(_wcsicmp(pwszValue, L"RowsetChange")==0)
			m_dwInsert = INSERT_ROWSETCHANGE;
		PROVIDER_FREE(pwszValue);
	}

	//SERVICECOMP - use Service Components
	if(GetInitStringValue(L"SERVICECOMP", &pwszValue))
	{
		m_dwServiceComponents = 0;
		if(FindSubString(pwszValue, L"INVOKE"))
			m_dwServiceComponents |= SERVICECOMP_INVOKE;
		if(FindSubString(pwszValue, L"CURSORENGINE"))
			m_dwServiceComponents |= (SERVICECOMP_INVOKE | SERVICECOMP_CURSORENGINE);
		PROVIDER_FREE(pwszValue);
	}

	//WARNINGLEVEL - indicates Warning Level
	if(GetInitStringValue(L"WARNINGLEVEL", &pwszValue))
	{
		m_dwWarningLevel = WARNINGLEVEL_ALL;
		if(_wcsicmp(pwszValue, L"NONE")==0)
			m_dwWarningLevel = WARNINGLEVEL_NONE;
		else if(_wcsicmp(pwszValue, L"0")==0)
			m_dwWarningLevel = WARNINGLEVEL_NONE;
		else if(_wcsicmp(pwszValue, L"1")==0)
			m_dwWarningLevel = WARNINGLEVEL_1;
		else if(_wcsicmp(pwszValue, L"2")==0)
			m_dwWarningLevel = WARNINGLEVEL_2;
		else if(_wcsicmp(pwszValue, L"3")==0)
			m_dwWarningLevel = WARNINGLEVEL_3;
		else if(_wcsicmp(pwszValue, L"4")==0)
			m_dwWarningLevel = WARNINGLEVEL_ALL;
		else if(_wcsicmp(pwszValue, L"ALL")==0)
			m_dwWarningLevel = WARNINGLEVEL_ALL;
		else if(_wcsicmp(pwszValue, L"ERROR")==0)
			m_dwWarningLevel = WARNINGLEVEL_ERROR;
		PROVIDER_FREE(pwszValue);
	}

	//DEBUGMODE - indicates Debugging Mode
	SetUseIMallocSpy(FALSE);
	if(GetInitStringValue(L"DEBUGMODE", &pwszValue))
	{
		m_dwDebugMode = DEBUGMODE_OFF;
		if(FindSubString(pwszValue, L"DIALOGS"))
			m_dwDebugMode |= DEBUGMODE_DIALOGS;
		if(FindSubString(pwszValue, L"POOLING"))
			m_dwDebugMode |= DEBUGMODE_POOLING;
		// Use when normally limited for automation:
		if(FindSubString(pwszValue, L"FULL"))
			m_dwDebugMode |= DEBUGMODE_FULL;
		
		//Show memory allocations or not...
		if(FindSubString(pwszValue, L"MEMORY"))
		{
			m_dwDebugMode |= DEBUGMODE_MEMORY;
			SetUseIMallocSpy(TRUE);
		}
		PROVIDER_FREE(pwszValue);
	}

	//When using an .ini file, this keyword controls
	//whether data in read-only columns will be checked
	if(GetInitStringValue(L"COMPARE_READONLYCOLS", &pwszValue))
	{
		if(_wcsicmp(pwszValue, L"FALSE")==0 || wcscmp(pwszValue, L"OFF")==0)
			m_fCompareReadOnlyCols = FALSE;
		else
			m_fCompareReadOnlyCols = TRUE;
		PROVIDER_FREE(pwszValue);
	}

	//ENUMERATOR - indicates the name of the enumerator to be tested
	GetInitStringValue(L"ENUMERATOR", &m_pwszEnumerator);

	//ROOTBINDER - indicates the name of binder to be used as root
	GetInitStringValue(L"ROOTBINDER", &m_pwszRootBinderProgID);

	//ROOTURL - indicates the URL of the row to be used as the tree root
	GetInitStringValue(L"ROOT_URL", &m_pwszRootURL);

	//ROWSET_SOURCE - indicates how to create the rowset
	//Lookup up this string (string must map to an EQUERY ie: SELECT_ALLFROMTBL)
	if(GetInitStringValue(L"ROWSET_SOURCE", &pwszValue))
	{
		m_eRowsetQuery = GetSQLTokenValue(pwszValue);
		PROVIDER_FREE(pwszValue);
	}

	//ALLOWED_TYPES - limits PROVIDER_TYPES rowset values to those in the comma delimited
	//list of allowed types.  Since this may be dangerous in terms of overlooking failures
	//we post a failure.
	if(GetInitStringValue(L"ALLOWED_TYPES", &pwszValue))
	{
		odtLog << L"***********************************************************************\n";
		odtLog << L"Currently using a limited set of columns in the test table.\n";
		odtLog << L"Types: " << pwszValue << L"\n";
		odtLog << L"Post failure since we can't pass the test with limited columns.\n";
		COMPARE(pwszValue, NULL);
		odtLog << L"***********************************************************************\n";
		PROVIDER_FREE(pwszValue);
	}
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// BOOL CModInfo::UseITableDefinition(BOOL flag)
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::UseITableDefinition(BOOL fUseITableDef)
{
	BOOL fOldFlag		= m_fTableCreation;
	m_fTableCreation	= fUseITableDef;
	return fOldFlag;
}



//////////////////////////////////////////////////////////////////////////
// BOOL CModInfo::UseIRowsetIndex(BOOL flag)
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::UseIRowsetIndex(BOOL fRowsetIndex)
{
	BOOL fOldFlag	= m_fRowsetIndex;
	m_fRowsetIndex	= fRowsetIndex;
	return fOldFlag;
}

//////////////////////////////////////////////////////////////////////////
// BOOL CModInfo::InitBackendInfo
//
//////////////////////////////////////////////////////////////////////////
BOOL CModInfo::InitBackendInfo(IDBInitialize *pIDBInitialize)
{
	WCHAR	*pwszValue		=	NULL;
	ULONG iStart = 0;
	ULONG iEnd = 0;
	IUnknown *pIUnknown = NULL;
	BOOL fRelease = FALSE;
	BOOL bSuccess = TRUE;

	if(pIDBInitialize == NULL)
	{
		// Must have TestModule interface
		if ( m_pCThisTestModule == NULL )
			return TRUE;
		
		pIUnknown = m_pCThisTestModule->m_pIUnknown;
	}
	else
		pIUnknown = pIDBInitialize;


	if (!pIUnknown)
	{
		fRelease = TRUE;
		if (FAILED (CreateProvider(NULL, IID_IDBInitialize, &pIUnknown, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE)))
		{
			bSuccess = FALSE;
			goto CLEANUP;
		}
	}

	PROVIDER_FREE(m_pwszBackend);
	PROVIDER_FREE(m_pwszBackendVer);
	PROVIDER_FREE(m_pwszProviderVer);

	if (SupportedProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pIUnknown))
	{
		TESTC(GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pIUnknown, &m_pwszBackend));
	}
	else
		m_pwszBackend = wcsDuplicate(L"No backend name info, DBPROP_DBMSNAME property not supported");

	if (SupportedProperty(DBPROP_PROVIDERVER, DBPROPSET_DATASOURCEINFO, pIUnknown))
	{
		TESTC(GetProperty(DBPROP_PROVIDERVER, DBPROPSET_DATASOURCEINFO, pIUnknown, &pwszValue));
		m_pwszProviderVer = wcsDuplicate(pwszValue);
		SAFE_FREE(pwszValue);
	}	

	if (SupportedProperty(DBPROP_DBMSVER, DBPROPSET_DATASOURCEINFO, pIUnknown))
	{
		TESTC(GetProperty(DBPROP_DBMSVER, DBPROPSET_DATASOURCEINFO, pIUnknown, &pwszValue));
		m_pwszBackendVer = wcsDuplicate(pwszValue);

		// Skipp leading '0'
		for (iStart = 0; pwszValue+iStart && pwszValue[iStart]==L'0'; iStart++);

		// Find the position of the end of main part:ex, "09.00.0352" iStart=1, iEnd=2
		for (iEnd=iStart; pwszValue+iEnd && pwszValue[iEnd]>=L'0' && pwszValue[iEnd]<=L'9'; iEnd++);
		
		//put Null terminator at iEnd position if it's not already there
		if (pwszValue+iEnd) pwszValue[iEnd] = L'\0';

		if (pwszValue+iStart)
			m_MajorBackendVer = _wtol(pwszValue+iStart);
	}
	else
	{
		m_pwszBackendVer = wcsDuplicate(L"No backend version info, DBPROP_DBMSVER property not supported");
		m_MajorBackendVer = -1;
	}
	odtLog << L"DBMS Name :\t " << m_pwszBackend << L"\nDBMS Version :\t " << m_pwszBackendVer << ENDL;
	
	//11/20/02 - adding this comment to prevent the test from silently passing with out any warning message for yukon types
	if (m_MajorBackendVer>8) odtLog << L"*WARNING*: Running against Yukon server.Please note that the test currently does not use all new Yukon types\n";

CLEANUP:
	PROVIDER_FREE (pwszValue);
	if (fRelease)
		SAFE_RELEASE(pIUnknown);

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
// DisplayPooling
//
//////////////////////////////////////////////////////////////////////////
HRESULT DisplayPooling(IUnknown* pDataSource, BOOL fEnumPools)
{
	TBEGIN


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// GetPoolState
//
//////////////////////////////////////////////////////////////////////////
DWORD GetPoolState(IUnknown* pDataSource)
{	
	TBEGIN
	DWORD dwPoolState = 0;


	return dwPoolState;
}


//////////////////////////////////////////////////////////////////////////
// DrawnFromPool
//
//////////////////////////////////////////////////////////////////////////
BOOL DrawnFromPool(IUnknown* pDataSource)
{	
	BOOL fValue = FALSE;


	return fValue;
}


//////////////////////////////////////////////////////////////////////////
// CanBePooled
//
//////////////////////////////////////////////////////////////////////////
BOOL CanBePooled(IUnknown* pDataSource)
{	
	BOOL fValue = FALSE;


	return fValue;
}


//////////////////////////////////////////////////////////////////////////
// CreatedFromPool
//
//////////////////////////////////////////////////////////////////////////
BOOL CreatedFromPool(IUnknown* pDataSource)
{	
	BOOL fValue = FALSE;


	return fValue;
}




////////////////////////////////////////////////////////////////////////
// CPool::CPool
//
////////////////////////////////////////////////////////////////////////
CPool::CPool(IUnknown* pObject)
{
	ulInUse				= 0;
	ulIdle				= 0;
	m_pwszInitString	= NULL;
	m_fHooks			= TRUE;
	m_pUnkPoolInfo		= NULL;

	//Delegate
	if(pObject)
		Create(pObject);
}


////////////////////////////////////////////////////////////////////////
// CPool::~CPool
//
////////////////////////////////////////////////////////////////////////
CPool::~CPool()
{
	SAFE_FREE(m_pwszInitString);
	SAFE_RELEASE(m_pUnkPoolInfo);
}


////////////////////////////////////////////////////////////////////////
// CPool::Create
//
////////////////////////////////////////////////////////////////////////
HRESULT CPool::Create(IUnknown* pObject)
{
	ASSERT(pObject);
	HRESULT hr = E_NOTIMPL;


	return hr;
}


//////////////////////////////////////////////////////////////////////////
// CPool::GetPoolID
//
//////////////////////////////////////////////////////////////////////////
WCHAR* CPool::GetPoolID()
{	
	TBEGIN
	WCHAR* pwszPoolID = NULL;


	return pwszPoolID;
}


//////////////////////////////////////////////////////////////////////////
// CPool::GetPoolTimeout
//
//////////////////////////////////////////////////////////////////////////
DWORD CPool::GetPoolTimeout()
{	
	TBEGIN
	DWORD dwValue = 0;
	HRESULT hr = S_OK;


	return dwValue;
}


//////////////////////////////////////////////////////////////////////////
// CPool::GetRetryTimeout
//
//////////////////////////////////////////////////////////////////////////
DWORD CPool::GetRetryTimeout()
{	
	TBEGIN
	DWORD dwValue = 0;
	HRESULT hr = S_OK;


	return dwValue;
}


//////////////////////////////////////////////////////////////////////////
// CPool::GetExpBackOff
//
//////////////////////////////////////////////////////////////////////////
DWORD CPool::GetExpBackOff()
{	
	TBEGIN
	DWORD dwValue = 0;
	HRESULT hr = S_OK;


	return dwValue;
}


///////////////////////////////////////////////////////////////////////
// CPoolManager::CPoolManager
//
////////////////////////////////////////////////////////////////////////
CPoolManager::CPoolManager()
{
	m_pUnkMngrInfo	= NULL;
}

////////////////////////////////////////////////////////////////////////
// CPoolManager::~CPoolManager
//
////////////////////////////////////////////////////////////////////////
CPoolManager::~CPoolManager()
{
	SAFE_RELEASE(m_pUnkMngrInfo);
}


////////////////////////////////////////////////////////////////////////
// CPoolManager::Create
//
////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::Create()
{
	HRESULT hr = S_OK;
	

	return hr;
}


//////////////////////////////////////////////////////////////////////////
// CPoolManager::EnumPools
//
//////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::EnumPools(ULONG* pcPools, IUnknown** prgpPoolInfo[])
{	
	TBEGIN
	HRESULT hr = E_NOTIMPL;


	return hr;
}

	
//////////////////////////////////////////////////////////////////////////
// CPoolManager::ReleasePools
//
//////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::ReleasePools(ULONG cPools, IUnknown** rgpPoolInfo)
{	

	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
// CPoolManager::FindPool
//
//////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::FindPool(WCHAR* pwszPoolID, IUnknown** ppIPoolInfo)
{	
	TBEGIN
	HRESULT hr = E_NOTIMPL;
	IUnknown** rgpPoolInfo = NULL;


	return *ppIPoolInfo ? S_OK : DB_E_NOTFOUND;
}


//////////////////////////////////////////////////////////////////////////
// CPoolManager::DisplayPools
//
//////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::DisplayPools()
{	
	TBEGIN
	HRESULT hr = E_NOTIMPL;


	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CPoolManager::ReleaseObject
//
////////////////////////////////////////////////////////////////////////
HRESULT CPoolManager::ReleaseObject(IUnknown** ppObject)
{
	ASSERT(ppObject);
	if(!*ppObject)
		return E_INVALIDARG;
	HRESULT hr = S_OK;

	
	//Release it so it gets put into the pool...
	//NOTE: We pass in the address so it gets nulled out for return so
	//the caller doesn't continue to use it...
	SAFE_RELEASE(*ppObject);


	//Make sure the DataSource gets released, even on error
	SAFE_RELEASE(*ppObject);
	return hr;
}



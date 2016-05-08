//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CENUM.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CEnumerator::CEnumerator
//
/////////////////////////////////////////////////////////////////
CEnumerator::CEnumerator(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CPropertiesBase(eCEnumerator, pCMainWindow, pCMDIChild)
{
	//[MANDATORY]
	m_pISourcesRowset			= NULL;		//Enumerator interface
	m_pIParseDisplayName		= NULL;		//Enumerator interface

	//[OPTIONAL]

	//Enumerator ProvierInfo
	m_cEnumInfo				= 0;	
	m_rgEnumInfo			= NULL;
}


/////////////////////////////////////////////////////////////////
// CEnumerator::~CEnumerator
//
/////////////////////////////////////////////////////////////////
CEnumerator::~CEnumerator()
{
	ReleaseObject(0);

	m_cEnumInfo			= 0;	
	SAFE_FREE(m_rgEnumInfo);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CEnumerator::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CEnumerator::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(ISourcesRowset);
	HANDLE_GETINTERFACE(IParseDisplayName);

	//Otherwise delegate
	return CPropertiesBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CEnumerator::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CEnumerator::AutoRelease()
{
	//[MANDATORY]
	RELEASE_INTERFACE(ISourcesRowset);
	RELEASE_INTERFACE(IParseDisplayName);

	//[OPTIONAL]

	//Delegate
	return CPropertiesBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CEnumerator::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CPropertiesBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IParseDisplayName);
		OBTAIN_INTERFACE(ISourcesRowset);
	}

	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		//[OPTIONAL]
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CEnumerator::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CEnumerator::GetObjectDesc()
{
	if(!m_strObjectDesc && m_guidSource != GUID_NULL)
	{
		if(m_guidSource == CLSID_OLEDB_ENUMERATOR)
			m_strObjectDesc.CopyFrom(L"Root Enumerator");
		else
			m_strObjectDesc.CopyFrom(GetProgID(m_guidSource));
	}

	return m_strObjectDesc;
}


////////////////////////////////////////////////////////////////
// CEnumerator::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void	CEnumerator::OnDefOperation() 
{ 
	if(m_pISourcesRowset)
		m_pCMainWindow->DisplayDialog(IDD_GETSOURCESROWSET, GetFocus(), CMainWindow::GetSourcesRowsetProc, this);
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::ParseDisplayName
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEnumerator::ParseDisplayName(WCHAR* pwszParseName, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown, CBase** ppCSource, DWORD dwConnectOpts, WCHAR* pwszRemoteServer)
{
	ASSERT(pwszParseName);

	CComPtr<IMoniker>	spMoniker;
	CComPtr<IBindCtx>	spBindCtx;
	ULONG chEaten = 0;
	CLSID clsid;
	HRESULT hr = E_FAIL;

	CBase* pCSource = NULL;

	//Try and use ParseDisplayName
	//Otherwise we may be forced to just try CoCreateInstance if not installed...
	//Also cannot use ParseDisplayName if trying to aggregate ServiceComponents
	if(m_pIParseDisplayName && !(dwConnectOpts & CREATE_USESERVICECOMP))
	{
		if(dwCLSCTX)
		{
			//Setup BIND_OPTS
			BIND_OPTS2 BindOps2;
			BindOps2.cbStruct		= sizeof(BIND_OPTS2);
			BindOps2.grfFlags		= BIND_MAYBOTHERUSER;
			BindOps2.grfMode		= STGM_READWRITE;
			BindOps2.dwTickCountDeadline = 0;
			BindOps2.dwTrackFlags	= 1;
			BindOps2.dwClassContext = dwCLSCTX;
			BindOps2.locale			= GetSystemDefaultLCID();
			BindOps2.pServerInfo	= NULL; 
			
			//Setup ServerInfo just incase we need it...
			COSERVERINFO	ServerInfo	= { 0, pwszRemoteServer, NULL, 0};

			//Setup RemoteServer is specified
			if(dwCLSCTX & CLSCTX_REMOTE_SERVER)
				BindOps2.pServerInfo = &ServerInfo;

			//If this fails for some reason, we can still continue...
			hr = CreateBindCtx(0, &spBindCtx);
			if(SUCCEEDED(hr) && spBindCtx)
				hr = spBindCtx->SetBindOptions(&BindOps2);

			//If it does fail we will just release the Bind Context and pass
			//IParseDisplayName NULL and let it choose the CLXCTX...
			if(FAILED(hr))
				spBindCtx.Release();
		}
		
		//ParseDisplayName
		//Don't display error, since we have our own retry logic...
		hr = m_pIParseDisplayName->ParseDisplayName(spBindCtx, pwszParseName, &chEaten, &spMoniker);
		TRACE_METHOD(hr, L"IParseDisplayName::ParseDisplayName(0x%p, \"%s\", &%d, &0x%p)", spBindCtx, pwszParseName, chEaten, spMoniker);
		
		if(SUCCEEDED(hr))
		{
			XTEST(hr = BindMoniker(spMoniker, 0, riid, (void**)ppIUnknown));
			TESTC(TRACE_METHOD(hr, L"BindMoniker(0x%p, 0, %s, &0x%p)", spBindCtx, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));
			pCSource = this;
		}
	}
	
	//If Not using RootEnum or for some reason the RootEnum failed, 
	if(m_pIParseDisplayName == NULL || FAILED(hr))
	{
		//Find the CLSID from the ParseName
		//The user may have entered a ProgID or a CLSID 
		//Since the ParseName is a CLSID, try the common case first...
		if(FAILED(hr = CLSIDFromString(pwszParseName, &clsid)))
			XTESTC(hr = CLSIDFromProgID(pwszParseName, &clsid));
				
		//If using Service Components
		if(dwConnectOpts & CREATE_USESERVICECOMP)
		{
			if(dwCLSCTX == CLSCTX_INPROC_SERVER)
			{
				//Can just reuse our Main DataLink object...
				TESTC(hr = m_pCMainWindow->m_pCServiceComp->CreateDBInstance(clsid, NULL, CLSCTX_INPROC_SERVER, riid, ppIUnknown));
				pCSource = m_pCMainWindow->m_pCServiceComp;
			}
			else
			{
				//If not inproc we just create a stand alone Data Link object
				//We do this again here, incase you want Remoted Providers...
				CServiceComp* pCServiceComp = new CServiceComp(m_pCMainWindow);
				TESTC(pCServiceComp->Create(NULL, dwCLSCTX, pwszRemoteServer));
			
				//The provider is always "inproc" to wherever the ServiceComponents are:
				TESTC(hr = pCServiceComp->CreateDBInstance(clsid, NULL, CLSCTX_INPROC_SERVER, riid, ppIUnknown));
				pCSource = pCServiceComp;
			}
		}
		else
		{
			//Delegate
			XTESTC(hr = CreateInstance(NULL, clsid, dwCLSCTX, riid, ppIUnknown, pwszRemoteServer));
			pCSource = NULL;
		}
	}
	
CLEANUP:
	if(ppCSource)
		*ppCSource = pCSource;
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::GetSourcesRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CEnumerator::GetSourcesRowset(CAggregate* pCAggregate, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	
	if(m_pISourcesRowset)
	{
		XTEST_(hr = m_pISourcesRowset->GetSourcesRowset(pCAggregate, riid, cPropSets, rgPropSets, ppIUnknown),S_OK);
		TRACE_METHOD(hr, L"ISourcesRowset::GetSourcesRowset(0x%p, %s, %d, 0x%p, &0x%p)", pCAggregate, GetInterfaceName(riid), cPropSets, rgPropSets, ppIUnknown ? *ppIUnknown : NULL);
	}

	//Display Errors (if occurred)
	TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;
}




WINOLEAPI CoCreateInstanceEx(REFCLSID, IUnknown*, DWORD, COSERVERINFO*, DWORD, MULTI_QI*);
/////////////////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::CreateInstance
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEnumerator::CreateInstance(CAggregate* pCAggregate, REFCLSID clsid, DWORD dwCLSCTX, REFIID riid, IUnknown** ppIUnknown, WCHAR* pwszRemoteServer)
{
	HRESULT hr = S_OK;

	//CLSID to String (just for display purposes...)
	WCHAR* pwszProgID = GetProgID(clsid);


	//Determine which method to use...
	if(dwCLSCTX & CLSCTX_REMOTE_SERVER)
	{
		//Setup ServerInfo...
		COSERVERINFO	ServerInfo	= { 0, pwszRemoteServer, NULL, 0};
		MULTI_QI		MultiQI		= { &riid, NULL, S_OK};

		//CoCreateInstanceEx
		hr = CoCreateInstanceEx(clsid, pCAggregate, dwCLSCTX, &ServerInfo, 1, &MultiQI);
		TESTC(TRACE_METHOD(hr, L"CoCreateInstanceEx(%s, 0x%p, 0x%08x, 0x%p, 1, 0x%p)", pwszProgID, pCAggregate, dwCLSCTX, &ServerInfo, &MultiQI));
		
		//Output pointer from the MultiQI
		if(ppIUnknown)
			*ppIUnknown = MultiQI.pItf;
	}
	else
	{
		//CoCreateInstance
		hr = CoCreateInstance(clsid, pCAggregate, dwCLSCTX, riid, (void**)ppIUnknown);
		TESTC(TRACE_METHOD(hr, L"CoCreateInstance(%s, 0x%p, 0x%08x, %s, &0x%p)", pwszProgID, pCAggregate, dwCLSCTX, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));
	}

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	
CLEANUP:
	SAFE_FREE(pwszProgID);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::Create
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEnumerator::Create(REFCLSID clsid)
{
	HRESULT hr = S_OK;
	CComPtr<IParseDisplayName>	spParseDisplayName;

	//Initialize the Enumerator and Obtain rowset
	m_guidSource = clsid;
	TESTC(hr = CreateInstance(NULL, clsid, CLSCTX_ALL, IID_IParseDisplayName, (IUnknown**)&spParseDisplayName));

	//Now Delegate to our generic Enumerator method
	TESTC(hr = CreateObject(NULL, IID_IParseDisplayName, spParseDisplayName));
	
CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::CreateEnumInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEnumerator::CreateEnumInfo(REFCLSID clsid, BOOL fRefresh)
{
	HRESULT hr = S_OK;

	//Create the Enumerator, if we haven't already...
	if(clsid!=GUID_NULL && !m_pIParseDisplayName)
		TESTC(hr = Create(clsid));

	//Free Previous Info
	if(fRefresh || !m_cEnumInfo)
	{
		m_cEnumInfo = 0;
		SAFE_FREE(m_rgEnumInfo)
	
		//Obtain the Enumerator Info from the rowset...	
		TESTC(hr = EnumerateInfo(&m_cEnumInfo, &m_rgEnumInfo));
	}

CLEANUP:
	return hr;
}

	
/////////////////////////////////////////////////////////////////////////////
// HRESULT CEnumerator::EnumerateInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEnumerator::EnumerateInfo(ULONG* pcEnumInfo, ENUMINFO** prgEnumInfo)
{
	HRESULT hr = E_FAIL;
	ASSERT(pcEnumInfo);
	ASSERT(prgEnumInfo);

	HROW rghRows[MAX_BLOCK_SIZE];
	HROW* phRows = rghRows;
	DBCOUNTITEM cRowsObtained = 0;
	CComPtr<IRowset> spRowset = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	CRowset cRowset(m_pCMainWindow);

	//Make our lives easier
	ULONG cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
	
	// Bind the user and table name for the list
	const static DBCOUNTITEM cBindings = 5;
	const static DBBINDING rgBindings[cBindings] = 
		{
			1,	 			
			offsetof(ENUMINFO, wszName),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				

			2,	 			
			offsetof(ENUMINFO, wszParseName),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				

			3,	 			
			offsetof(ENUMINFO, wszDescription),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				
	
			4,	 			
			offsetof(ENUMINFO, eType),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(USHORT),
			0, 				
			DBTYPE_UI2, 	
			0,	
			0, 				

			5,	 			
			offsetof(ENUMINFO, fIsParent),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				
	};

	//ISourcesRowset::GetSourcesRowset
	TESTC(hr = GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&spRowset));
	TESTC(hr = cRowset.CreateObject(this, IID_IRowset, spRowset));

	//Create Accessor
	TESTC(hr = cRowset.CreateAccessor(DBACCESSOR_ROWDATA, cBindings, (DBBINDING*)rgBindings, 0, &hAccessor));

	//Loop through the entire returned rowset
	while(TRUE)
	{
		TESTC(hr = cRowset.GetNextRows(0, MAX_BLOCK_SIZE, &cRowsObtained, &phRows));
		
		//ENDOFROWSET
		if(cRowsObtained==0) 
			break;
		
		//Alloc room for ProviderInfo (in chunks)
		SAFE_REALLOC(rgEnumInfo, ENUMINFO, cEnumInfo + cRowsObtained);
		memset(&rgEnumInfo[cEnumInfo], 0, sizeof(ENUMINFO) * (size_t)cRowsObtained);

		//Loop over rows obtained and get ProviderInfo
		for(ULONG i=0; i<cRowsObtained; i++) 
		{	
			//Get the Data
			TESTC(hr = cRowset.GetData(rghRows[i], hAccessor, (void*)&rgEnumInfo[cEnumInfo]));
			cEnumInfo++;
		}
			
		//Release all the rows
		TESTC(hr = cRowset.ReleaseRows(cRowsObtained, rghRows));
	}

	//Special handling for Root Enumerator
	if(m_guidSource == CLSID_OLEDB_ENUMERATOR)
	{
		//NOTE:  The Root Enumerator doesn't include itself in the List of Enumerators
		//This is good for the case of tree controls would be never ending, but bad
		//for apps like this one, where you might want to see the "rowset" of the Root
		//Enum.  So to get arround this I will just add it to the list manually

		//Alloc room for extra MSDAENUM additon
		SAFE_REALLOC(rgEnumInfo, ENUMINFO, cEnumInfo + 2);
		StringCopy(rgEnumInfo[cEnumInfo].wszName,		L"MSDAENUM", MAX_NAME_LEN);
		StringCopy(rgEnumInfo[cEnumInfo].wszParseName,	L"{c8b522d0-5cf3-11ce-ade5-00aa0044773d}",	MAX_NAME_LEN);
		StringCopy(rgEnumInfo[cEnumInfo].wszDescription,L"Microsoft OLE DB Root Enumerator", MAX_NAME_LEN);
		rgEnumInfo[cEnumInfo].eType						= DBSOURCETYPE_ENUMERATOR;
		rgEnumInfo[cEnumInfo].fIsParent					= VARIANT_TRUE;
		cEnumInfo++;

		//Also add the RootBinder to the list of valid progids.
		StringCopy(rgEnumInfo[cEnumInfo].wszName,		L"MSDAURL.Binder", MAX_NAME_LEN);
		StringCopy(rgEnumInfo[cEnumInfo].wszParseName,	L"{FF151822-B0BF-11D1-A80D-000000000000}", MAX_NAME_LEN);
		StringCopy(rgEnumInfo[cEnumInfo].wszDescription,L"Microsoft OLE DB Root Binder for Internet Publishing", MAX_NAME_LEN);
		rgEnumInfo[cEnumInfo].eType						= DBSOURCETYPE_ENUMERATOR;
		rgEnumInfo[cEnumInfo].fIsParent					= VARIANT_TRUE;
		cEnumInfo++;
	}

CLEANUP:
	//Now update the output params
	*pcEnumInfo = cEnumInfo;
	*prgEnumInfo = rgEnumInfo;

	cRowset.ReleaseAccessor(&hAccessor);
	return hr;
}



//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ProviderInfo Implementation Module | 	This module contains definition information
//			for the CSourceInfo, CSourcesSet, CModifyRegistry and CProvPropSets classes
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

// file ProviderInfo.cpp
#include "MODStandard.hpp"	// Standard headers	
#include "privstd.h"	// Private library common precompiled header
#include "privlib.h"
#include "msdaguid.h"
#include "CVectorEx.hpp"
#include "ServiceComp.h"
#include "ProviderInfo.h"

const ULONG	cMaxName	= 300;
CSourcesSet		*g_pSourcesSet	= NULL;


CSourceInfo::operator CLSID()
{
	CLSID	clsid;

	CLSIDFromString((LPWSTR)(LPCWSTR)m_pwszParseName, &clsid);
	return clsid;
} // CSourceInfo::operator CLSID







CSourcesSet::CSourcesSet()
{
	m_fInitialized			= 0;
} //CSourcesSet::CSourcesSet


BOOL CSourcesSet::FinalInit()
{
	TBEGIN
	HRESULT					hr;
	IServiceProvider		*pIServiceProvider		= NULL;
	IUnknown				*pIUnknown				= NULL;
	ISpecifyPropertyPages	*pISpecify				= NULL;
	CLSID					clsid;
	BOOL					*rgExtendsDSL			= NULL;
	CAUUID					Pages;
	WCHAR					wszSubKey[cMaxName]		= L"";
	CHAR					szSubKey[cMaxName]		= "";
	LONG					lRes;
	HKEY					hKey = 0;
	CHAR					szValueName[cMaxName]	= "OLEDB_SERVICES";
	DWORD					dwServices;
	DWORD					cLen					= sizeof(DWORD);
	DWORD					dwType;
	IDataInitialize			*pIDataInit				= NULL;
	ISourcesRowset			*pISourcesRowset		= NULL;
	IRowset					*pIRowset				= NULL;
	CLightRowset			Rowset;
	HROW					hRow;

	if (TRUE == m_fInitialized)
		return TRUE;

	pIDataInit = CServiceComp::pIDataInitialize();
	TESTC(NULL != pIDataInit);

	// get sources rowset
	TESTC_(hr = CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, 
		IID_ISourcesRowset, (void**)&pISourcesRowset),S_OK);

	//Obtain ISourcesRowset interface
	TESTC_(hr = pISourcesRowset->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset),S_OK);
	TESTC_(Rowset.Attach(IID_IRowset, pIRowset), S_OK);


	Pages.pElems = NULL;
	for (; S_OK == (hr = Rowset.GetNextRows(&hRow)); )
	{
		CSourceInfo		SourceInfo;
		DBORDINAL		ulSourcesName			= 1;
		DBORDINAL		ulSourcesParseName		= 2;
		DBORDINAL		ulSourcesDescription	= 3;
		DBORDINAL		ulSourcesType			= 4;
		DBORDINAL		ulSourcesIsParent		= 5;
		DBTYPE			*pwType;
		VARIANT_BOOL	*pfIsParent;

		// go for the default accessor, all columns, no bookmarks, no blobs
		TESTC_(Rowset.GetData(hRow), S_OK);


		SourceInfo.ResetValue();

		SourceInfo.m_pwszName			= (WCHAR*)Rowset.GetColumnValue(ulSourcesName);
		SourceInfo.m_pwszParseName		= (WCHAR*)Rowset.GetColumnValue(ulSourcesParseName);
		SourceInfo.m_pwszDescription	= (WCHAR*)Rowset.GetColumnValue(ulSourcesDescription);
		pwType							= (USHORT*)Rowset.GetColumnValue(ulSourcesType);
		TESTC(NULL != pwType);
		SourceInfo.m_wType				= *pwType;
		pfIsParent						= (VARIANT_BOOL*)Rowset.GetColumnValue(ulSourcesIsParent);
		TESTC(NULL != pfIsParent);
		SourceInfo.m_fIsParent			= *pfIsParent;
		TESTC(NULL != (LPCWSTR)SourceInfo.m_pwszName);
		TESTC(NULL != (LPCWSTR)SourceInfo.m_pwszParseName);
		TESTC(NULL != (LPCWSTR)SourceInfo.m_pwszDescription);

		//if the source is not a provider, skip the checking
		if (	DBSOURCETYPE_DATASOURCE_TDP == SourceInfo.m_wType
			||	DBSOURCETYPE_DATASOURCE_MDP == SourceInfo.m_wType)
		{
		
				
			// get info about services supported
			wcscpy(wszSubKey, L"CLSID\\");
			wcscat(wszSubKey, (LPCWSTR)SourceInfo.m_pwszParseName);
			ConvertToMBCS(wszSubKey, szSubKey, cMaxName);

			lRes = 	RegOpenKeyExA(
				HKEY_CLASSES_ROOT,         // handle to open key
				szSubKey,  // address of name of subkey to open
				NULL,   // reserved
				KEY_QUERY_VALUE , // security access mask
				&hKey    // address of handle to open key
			);
			TESTC(ERROR_SUCCESS == lRes);

			lRes = RegQueryValueExA(
				hKey,							// handle to key to query
				szValueName,					// name of subkey to query
				NULL,							// reserved
				&dwType,						// address of buffer for value type
				(LPBYTE)&dwServices,			// buffer for returned string
				&cLen							// receives size of returned string
				);

			RegCloseKey(hKey);

			if (ERROR_SUCCESS == lRes)
			{
				SourceInfo.m_Services = dwServices & (DBPROPVAL_OS_RESOURCEPOOLING 
					| DBPROPVAL_OS_TXNENLISTMENT | DBPROPVAL_OS_CLIENTCURSOR);	
			}

			if (!CHECK(CLSIDFromString((LPWSTR)(LPCWSTR)SourceInfo.m_pwszParseName, &clsid), S_OK))
				goto LOOP;
		
			odtLog << SourceInfo.m_pwszDescription << "\n";
			if (!CHECK(pIDataInit->CreateDBInstance(clsid, NULL, CLSCTX_ALL, NULL,
				IID_IUnknown, &pIUnknown), S_OK))
				goto LOOP;
			
			// look for customized pages
			if (!VerifyInterface(pIUnknown, IID_IServiceProvider, DATASOURCE_INTERFACE, (IUnknown**)&pIServiceProvider))
				goto LOOP;

			if (S_OK != pIServiceProvider->QueryService(OLEDB_SVC_DSLPropertyPages,
				IID_ISpecifyPropertyPages,(LPVOID *)&pISpecify))
				goto LOOP;

			TESTC_(hr = pISpecify->GetPages(&Pages), S_OK);

			TESTC(3 > Pages.cElems);
			if (0 == Pages.cElems)
				goto LOOP;

			TESTC(2 <= Pages.cElems
				|| Pages.pElems[0] != Pages.pElems[1]);
			
			if (1 <= Pages.cElems && GUID_NULL != Pages.pElems[0])
				SourceInfo.m_CustomUI |= CUSTOM_CONN;
			if (2 <= Pages.cElems && GUID_NULL != Pages.pElems[1])
				SourceInfo.m_CustomUI |= CUSTOM_ADV;
			}

LOOP:
		// get the list of property init descriptions 
		SourceInfo.m_PropInfoSets.CreatePropInfoSet(pIUnknown);
	
		CHECK(m_rgSourcesInfo.AddElement(&SourceInfo), S_OK);

		SAFE_RELEASE(pIUnknown);
		SAFE_RELEASE(pIServiceProvider);
		SAFE_RELEASE(pISpecify);
		SAFE_FREE(Pages.pElems);

		CHECK(Rowset.ReleaseRows(hRow), S_OK);
	}

	TESTC_(hr, DB_S_ENDOFROWSET);
	m_fInitialized = TRUE;

CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	CServiceComp::ReleaseSCInterface(pIDataInit);
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIServiceProvider);
	SAFE_RELEASE(pISpecify);
	SAFE_FREE(Pages.pElems);
	TRETURN
} //CSourcesSet::FinalInit



CSourcesSet::~CSourcesSet()
{
} //CSourcesSet::~CSourcesSet


CSourceInfo	&CSourcesSet::operator[] (ULONG nIndex)
{
	if (Count() <= nIndex)
		throw;

	return this->m_rgSourcesInfo[nIndex];
} //CSourcesSet::operator[]

		
CSourceInfo	&CSourcesSet::operator[] (WCHAR *pwszDescription)
{
	ULONG		nIndex;
	
	if (!pwszDescription)
		throw;
	
	for (nIndex=0; nIndex < Count(); nIndex++)
	{
		if (	m_rgSourcesInfo[nIndex].m_pwszDescription 
			&&	0 == wcscmp(m_rgSourcesInfo[nIndex].m_pwszDescription, pwszDescription))
		{
			return m_rgSourcesInfo[nIndex];
		}
	}

	throw;
	return m_rgSourcesInfo[0];
} //CSourcesSet::operator[]

		
CSourceInfo	&CSourcesSet::operator[] (CLSID clsidProvider)
{
	ULONG		nIndex;
	CLSID		clsidCrtProv;

	for (nIndex=0; nIndex < Count(); nIndex++)
	{
		CLSIDFromString((LPWSTR)(LPCWSTR)m_rgSourcesInfo[nIndex].m_pwszParseName, &clsidCrtProv);
		if (clsidProvider == clsidCrtProv)
		{
			return m_rgSourcesInfo[nIndex];
		}
	}

	throw;
	return m_rgSourcesInfo[0];
} //CSourcesSet::operator[]

		
// set filter for the iterator and the first position
BOOL CSourcesSet::SetFilter(
	ULONG			cAllowedSourceTypes, 
	DBTYPE			*rgAllowedSourceTypes,
	ULONG			cExceptedSourceTypes,
	DBTYPE			*rgExceptedSourceTypes,
	CCustomUI		*pCustomUIFilter,
	COLEDBServices	*pServicesFilter
)
{
	m_ulPos = 0;

	// do not copy the filter values; the client should mantain them
	m_cAllowedSourceTypes	= cAllowedSourceTypes;
	m_rgAllowedSourceTypes	= rgAllowedSourceTypes;
	m_cExceptedSourceTypes	= cExceptedSourceTypes;
	m_rgExceptedSourceTypes	= rgExceptedSourceTypes;
	m_pServicesFilter		= pServicesFilter;
	m_pCustomUIFilter		= pCustomUIFilter;

	// get the first, starting with this position
	m_ulPos	= GetNext();
	return m_ulPos < Count();
} //CSourcesSet::SetFilter


// get the current source information and advance to the next position
BOOL CSourcesSet::GetCurrent(CSourceInfo *pSourceInfo)
{
	if (!pSourceInfo)
		return FALSE;

	m_ulPos++;
	m_ulPos	= GetNext();
	
	if (m_ulPos < Count())
	{
		//CopyToCSourceInfo(m_ulPos, pSourceInfo);
		pSourceInfo = &m_rgSourcesInfo[m_ulPos];
		return TRUE;
	}
	else
		return FALSE;
} //CSourcesSet::GetCurrent



ULONG CSourcesSet::GetNext()
{
	ULONG	ulIndx;

	for (; m_ulPos < Count(); m_ulPos++)
	{
		if (DBSOURCETYPE_DATASOURCE_TDP == m_rgSourcesInfo[m_ulPos].m_wType
			|| DBSOURCETYPE_DATASOURCE_TDP == m_rgSourcesInfo[m_ulPos].m_wType)
		{
			// check the customization
			if (m_pCustomUIFilter && *m_pCustomUIFilter != m_rgSourcesInfo[m_ulPos].m_CustomUI)
				continue;
			// service is at least the required one
			if (m_pServicesFilter && 
				*m_pServicesFilter != (*m_pServicesFilter & m_rgSourcesInfo[m_ulPos].m_Services))
				continue;
		}

		// check whether this is among the allowed sources
		if (0 < m_cAllowedSourceTypes && m_rgAllowedSourceTypes)
		{
			// try to identify current source among the allowed ones
			for (ulIndx=0; ulIndx < m_cAllowedSourceTypes; ulIndx++)
			{
				if (m_rgAllowedSourceTypes[ulIndx] == m_rgSourcesInfo[m_ulPos].m_wType)
					break;
			}
			if (ulIndx < m_cAllowedSourceTypes)
				break;		// do not look in the second list, they shouldn't both exist
		}
		else
		// check whether this is among the allowed sources
		if (0 < m_cExceptedSourceTypes && m_rgExceptedSourceTypes)
		{
			// try to identify current source among the allowed ones
			for (ulIndx=0; ulIndx < m_cExceptedSourceTypes; ulIndx++)
			{
				if (m_rgExceptedSourceTypes[ulIndx] == m_rgSourcesInfo[m_ulPos].m_wType)
					break;
			}
			if (ulIndx >= m_cExceptedSourceTypes)
				break;		// do not look in the second list, they shouldn't both exist
		}
	}

	return m_ulPos;
} //CSourcesSet::GetNext




ULONG CSourcesSet::GetNoOfProviders()
{
	ULONG	ulProvNo	= 0;
	ULONG	ulIndex;

	for (ulIndex = 0; ulIndex < Count(); ulIndex++)
	{
		if (	DBSOURCETYPE_DATASOURCE_TDP == m_rgSourcesInfo[ulIndex].m_wType
			||	DBSOURCETYPE_DATASOURCE_MDP == m_rgSourcesInfo[ulIndex].m_wType)
			ulProvNo++;
	}

	return ulProvNo;
} //CSourcesSet::GetNoOfProviders




// Finds a property of a given type
BOOL CSourcesSet::FindProperty(
	VARTYPE			vtType,						// [in]  the type of the sought property
	DBPROPINFO		**ppPropInfo,				// [out] the property pointer
	GUID			*pguidPropSet,				// [out] the propset guid
	GUID			*pguidProvider,			// [out] the provider guid
	ULONG			cExclProp/* = 0*/,			// [in]  the number of props to be excluded 
	DBPROPID		*rgExclPropID/* = NULL*/,	// [in]  the props to be excluded
	GUID			*rgExclPropSet/* = NULL*/	// [in]  the propset of the corresponding props
)
{
	ULONG			ulProv;

	TESTC(NULL != ppPropInfo);
	TESTC(NULL != pguidPropSet);
	TESTC(NULL != pguidProvider);

	*ppPropInfo		= NULL;
	memset(pguidPropSet, 0, sizeof(GUID));
	memset(pguidProvider, 0, sizeof(GUID));

	for (ulProv = 0; ulProv < Count(); ulProv++)
	{
		if (	DBSOURCETYPE_DATASOURCE_TDP != m_rgSourcesInfo[ulProv].m_wType
			&&	DBSOURCETYPE_DATASOURCE_MDP != m_rgSourcesInfo[ulProv].m_wType)
			continue;

		if (m_rgSourcesInfo[ulProv].m_PropInfoSets.FindProperty(vtType, ppPropInfo, pguidPropSet,
				cExclProp, rgExclPropID, rgExclPropSet))
		{
			*pguidProvider	= (CLSID)((CSourceInfo)((*this)[ulProv]));
			return TRUE;
		}
	}

CLEANUP:
	return FALSE;
} //CSourcesSet::FindProperty



///////////////////////////////////////////////////////////////////////////////
//CModifyRegistry
//	Class for altering OLEDBSERVICES
//

CModifyRegistry::CModifyRegistry(REFCLSID rclsid) 
	: m_clsidProv(rclsid)
{
	if (GUID_NULL == rclsid) 
		memcpy(	(LPVOID)&m_clsidProv, 
				(LPVOID)&GetModInfo()->GetThisTestModule()->m_ProviderClsid, 
				sizeof(CLSID));

	// create and cache the key 
	DWORD	dwIgnored	(0);
	HKEY	hKeyTemp	(0);
	BOOL	fRetVal		(FALSE);

	CLSID	clsid		(rclsid==GUID_NULL ? m_clsidProv : rclsid );

	OLECHAR wszKey[]	= L"CLSID\\{00000000-0000-0000-0000-000000000000}";
	
	TESTC(NULL != StringFromGUID2(clsid, &wszKey[6], (sizeof(wszKey) / sizeof(OLECHAR)) - 6));

	m_hKey = NULL;

	TESTC(ERROR_SUCCESS==RegCreateKeyExW(HKEY_CLASSES_ROOT, &wszKey[0], 0
									   , NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS
									   , NULL, &m_hKey, &dwIgnored));

	// read and save the registry values for OLE DB SERVICES and SP Timeout
	PreserveValue("OLEDB_SERVICES");	
	PreserveValue("SPTimeout");	

CLEANUP:
	return;
} //CModifyRegistry::CModifyRegistry



BOOL CModifyRegistry::QueryValue(
	HKEY	hKey, 
	char	*szValName, 
	DWORD	*pdwVal, 
	BOOL	fGetBackupValue /* = FALSE */
)
{
	//prepare the value, if they want a backup val
	char	szVal[500] = "BAK_";
	DWORD	dwValType	= REG_DWORD;
	DWORD	dwBufSize	= sizeof(DWORD);

	strcpy(&szVal[fGetBackupValue ? 4 : 0], &szValName[0]);

	if (pdwVal)
		*pdwVal = 0;

	if (fGetBackupValue)
		RegQueryValueEx(hKey, &szValName[0], NULL, &dwValType, (LPBYTE)pdwVal, &dwBufSize);

	return (ERROR_SUCCESS == RegQueryValueEx(hKey, &szVal[0], NULL, 
		&dwValType, (LPBYTE)pdwVal, &dwBufSize));
} //CModifyRegistry::QueryValue



BOOL CModifyRegistry::SetValue(HKEY hKey, char * szValName, DWORD dwVal, BOOL fSetBackupVal)
{
	char	szVal[500] = "BAK_";
	DWORD	dwValType	= REG_DWORD;
	DWORD	dwBufSize	= sizeof(DWORD);

	strcpy(&szVal[fSetBackupVal ? 4 : 0], &szValName[0]);

	return (ERROR_SUCCESS == RegSetValueEx(hKey, szVal, 0, 
		REG_DWORD, (BYTE*) &dwVal, sizeof(DWORD)));
} //CModifyRegistry::SetValue


BOOL CModifyRegistry::PreserveValue(HKEY hKey, char * szValName)
{
	DWORD	dwVal	(0);

	//is it already preserved?
	if (QueryValue(hKey, szValName, &dwVal, TRUE))
		return TRUE;

	return( (QueryValue(hKey, szValName, &dwVal, FALSE)) &&
			(SetValue(hKey, szValName, dwVal, TRUE))    );
} //CModifyRegistry::PreserveValue


BOOL CModifyRegistry::RemoveValue(HKEY hKey, char * szValName)
{
	return ((PreserveValue(hKey, szValName)) && 
		    (ERROR_SUCCESS == RegDeleteValue(hKey, szValName)));
} //CModifyRegistry::RemoveValue



BOOL CModifyRegistry::RestoreValue(HKEY hKey, char * szValName)
{
	DWORD	dwVal	(0);

	//if there's a backup value, set it as the main value.
	if(QueryValue(hKey, szValName, &dwVal, TRUE))
	{
		if(!SetValue(hKey, szValName, dwVal, FALSE))
			return FALSE;
	}
	else
	{
		// remove the key
		return (ERROR_SUCCESS == RegDeleteValue(hKey, szValName));
	}

	return TRUE;
} //CModifyRegistry::RestoreValue



BOOL CModifyRegistry::SetServices(DWORD dwServices)
{
	char szValName[] = "OLEDB_SERVICES";

	BOOL fRetVal	(FALSE);

	// if Service Components are in memory, there is no use to 
	// modify provider's registry, SC won't refresh the information
	if (CServiceComp::Get_SCRef())
		return FALSE;

	if(dwServices == CMR_REMOVE)
	{
		fRetVal = RemoveValue(szValName);
	}
	else
	{
		// set to a new value
		fRetVal = SetValue(&szValName[0], dwServices, FALSE);
	}

	return fRetVal;
} //CModifyRegistry::SetServices



BOOL CModifyRegistry::Set_SPTimeout(DWORD dwSPTimeout)
{
	BOOL fRetVal	 = FALSE;

	PreserveValue("SPTimeout");

	if(dwSPTimeout == CMR_REMOVE)
	{
		fRetVal = RemoveValue("SPTimeout");
	}
	else
	{
		fRetVal = SetValue("SPTimeout", dwSPTimeout); 
	}
	return fRetVal;
} //CModifyRegistry::Set_SPTimeout



BOOL CModifyRegistry::IsResourcePoolingAllowed()
{
	DWORD dwServices;	

	return GetServices(&dwServices) && (dwServices & DBPROPVAL_OS_RESOURCEPOOLING);
} //CModifyRegistry::IsResourcePoolingAllowed



BOOL CModifyRegistry::IsTransactionEnlistmentAllowed()
{
	DWORD dwServices;	

	return GetServices(&dwServices) && (dwServices & DBPROPVAL_OS_TXNENLISTMENT);
} //CModifyRegistry::IsTransactionEnlistmentAllowed



BOOL CModifyRegistry::Set_RetryWait(DWORD dwRetryWait)
{

	HKEY	hKey		= 0;
	BOOL	fRetVal		= FALSE;
	DWORD	dwIgnored	(0);

	const WCHAR wszKeyMSDAINIT[]=L"SOFTWARE\\Microsoft\\DataAccess\\Session Pooling";

	//create key for Retry Wait
	if(ERROR_SUCCESS != RegCreateKeyExW(HKEY_LOCAL_MACHINE, &wszKeyMSDAINIT[0], 0
		, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS
		, NULL, &hKey, &dwIgnored))
		return FALSE;
	
	PreserveValue(hKey, "Retry Wait");

	if(dwRetryWait == CMR_REMOVE)
	{
		fRetVal = RemoveValue(hKey, "Retry Wait");
	}
	else
	{
		fRetVal = SetValue(hKey, "Retry Wait", dwRetryWait); 
	}
	SAFE_CLOSEKEY(hKey);
	return fRetVal;
} //CModifyRegistry::Set_RetryWait






HRESULT CProvPropSets::SetProperty(
	GUID			guidProvider, 
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	ULONG_PTR		ulValue, 
	DBPROPOPTIONS	dwOptions/* = DBPROPOPTIONS_REQUIRED*/, 
	DBID			colid/* = DB_NULLID*/
)
{
	if(wType == DBTYPE_BOOL)
		ulValue = ulValue ? VARIANT_TRUE : VARIANT_FALSE;
	return SetProperty(guidProvider, PropertyID, guidPropertySet, wType, &ulValue, dwOptions, colid);
} //CProvPropSets::SetProperty




HRESULT CProvPropSets::SetProperty(
	GUID			guidProvider, 
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	void			*pv, 
	DBPROPOPTIONS	dwOptions/* = DBPROPOPTIONS_REQUIRED*/, 
	DBID			colid/* = DB_NULLID*/
)
{
	LONG		index = GetProviderIndex(guidProvider);

	if (-1 >= index)
	{
		return E_FAIL;
	}
	
	return m_rgPropSets[index].SetProperty(PropertyID, guidPropertySet, wType, pv, dwOptions, colid);
} //CProvPropSets::SetProperty




DBPROP *CProvPropSets::FindProperty(
	GUID		guidProvider, 
	DBPROPID	PropertyID, 
	GUID		guidPropertySet
)
{
	LONG	index = GetProviderIndex(guidProvider);

	return (-1 < index) ? m_rgPropSets[index].FindProperty(PropertyID, guidPropertySet): NULL;
} //CProvPropSets::FindProperty
	



DBPROPSET *CProvPropSets::FindPropertySet(
	GUID	guidProvider, 
	GUID	guidPropertySet
)
{
	LONG	index = GetProviderIndex(guidProvider);

	return (-1 < index) ? m_rgPropSets[index].FindPropertySet(guidPropertySet): NULL;
} //CProvPropSets::FindPropertySet




DBPROPSET *CProvPropSets::FindProvPropertySets(GUID guidProvider)
{
	LONG	index = GetProviderIndex(guidProvider);

	return (-1 < index) ? m_rgPropSets[index].pPropertySets(): NULL;
} //CProvPropSets::FindProvPropertySets




CPropSets *CProvPropSets::FindProvCPropSets(GUID guidProvider)
{
	LONG	index = GetProviderIndex(guidProvider);

	return (-1 < index) ? &m_rgPropSets[index]: NULL;
} //CProvPropSets::FindProvCPropSets




LONG CProvPropSets::GetProviderIndex(GUID guidProvider)
{
	ULONG	index;

	for (index = 0; index < m_cNoOfProviders; index++)
	{
		if (guidProvider == m_rgProviderGUIDs[index])
			return index;
	}

	return -1;
} //CProvPropSets::GetProviderIndex



BOOL CProvPropSets::AddProvider(
	GUID			guidProvider,
	ULONG			cPropSets/*	= 0*/,
	DBPROPSET		*rgPropSets/* = NULL*/
)
{
	TBEGIN

	// check whether the provider already exist
	if (-1 != GetProviderIndex(guidProvider))
		return FALSE;

	m_rgProviderGUIDs.AddElement(&guidProvider);
	m_rgPropSets.AddElement();
	m_rgPropSets[m_cNoOfProviders].Attach(cPropSets, rgPropSets);
	m_cNoOfProviders++;
	TRETURN
} //CProvPropSets::AddProvider




BOOL CProvPropSets::AddProviderWithDefPropValues(
	GUID clsidProvider
)
{
	TBEGIN
	IDBProperties	*pIDBProperties	= NULL;
	ULONG			cPropSets		= 0;
	DBPROPSET		*rgPropSets		= NULL;
	IDataInitialize	*pIDataInit		= CServiceComp::pIDataInitialize();

	ASSERT(pIDataInit);

	TESTC_(pIDataInit->CreateDBInstance(clsidProvider, NULL, CLSCTX_ALL, NULL,
		IID_IDBProperties, (IUnknown**)&pIDBProperties), S_OK);

	TESTC_(pIDBProperties->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK);
	TESTC(AddProvider(clsidProvider, cPropSets, rgPropSets));

CLEANUP:
	CServiceComp::ReleaseSCInterface(pIDataInit);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
} //CProvPropSets::AddProviderWithDefPropValues


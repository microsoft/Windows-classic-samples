//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ProviderInfo Header Module | 	This module contains header information
//			for the CSourceInfo, CSourcesSet, CModifyRegistry and CProvPropSets classes
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

// file ProviderInfo.h

#ifndef __PROVIDER_INFO
#define __PROVIDER_INFO


/////////////////////////////////////////////////////////////////////////////
// Enumerator Operations
//
/////////////////////////////////////////////////////////////////////////////
#define SAFE_CLOSEHANDLE(hIn) if(hIn)  {CloseHandle(hIn);  hIn =NULL;}
#define SAFE_CLOSEKEY(hKey)    if(hKey) {RegCloseKey(hKey); hKey=NULL;}


typedef LONG COLEDBServices;
typedef DWORD CCustomUI;

static const CCustomUI	CUSTOM_NONE	= 0;
static const CCustomUI	CUSTOM_CONN = 1;
static const CCustomUI	CUSTOM_ADV	= 2;
static const CCustomUI	CUSTOM_BOTH = CUSTOM_CONN | CUSTOM_ADV;

static const DWORD		CMR_REMOVE	= 0x01010101;//special value, SetValue with this actually removes value (good for iterations)

// Here are classes used to store info about providers
class CSourceInfo{
public:
	CWString		m_pwszName;
	CWString		m_pwszParseName;
	CWString		m_pwszDescription;
	DBTYPE			m_wType;
	VARIANT_BOOL	m_fIsParent;
	CCustomUI		m_CustomUI;
	COLEDBServices	m_Services;
	CPropInfoSets	m_PropInfoSets;


	CSourceInfo(
		WCHAR			*pwszName			= NULL, 
		WCHAR			*pwszParseName		= NULL, 
		WCHAR			*pwszDescription	= NULL,
		DBTYPE			wType				= 0, 
		VARIANT_BOOL	fIsParent			= VARIANT_FALSE, 
		CCustomUI		CustomUI			= CUSTOM_NONE, 
		COLEDBServices	Services			= 0,
		ULONG			m_cPropInfoSets		= 0
		): 
		m_pwszName(pwszName), m_pwszParseName(pwszParseName), 
		m_pwszDescription(pwszDescription), m_wType(wType), 
		m_fIsParent(fIsParent), m_CustomUI(CustomUI), 
		m_Services(Services){
	}

	CSourceInfo(CSourceInfo &SourceInfo) {
		m_pwszName			= SourceInfo.m_pwszName;
		m_pwszParseName		= SourceInfo.m_pwszParseName; 
		m_pwszDescription	= SourceInfo.m_pwszDescription;
		m_wType				= SourceInfo.m_wType; 
		m_fIsParent			= SourceInfo.m_fIsParent;
		m_CustomUI			= SourceInfo.m_CustomUI;
		m_Services			= SourceInfo.m_Services;
		m_PropInfoSets		= SourceInfo.m_PropInfoSets;

	}

	~CSourceInfo() {
		Free();
	}

	void ResetValue() {
		m_pwszName			= NULL;
		m_pwszParseName		= NULL;
		m_pwszDescription	= NULL;
		m_wType				= 0;
		m_fIsParent			= VARIANT_FALSE;
		m_CustomUI			= CUSTOM_NONE;
		m_Services			= 0;
		m_PropInfoSets.Free();
	}

	operator CLSID();
	// the next operator is only intended for descriptions
	operator WCHAR*() {
		return (LPWSTR)(LPCWSTR)m_pwszDescription;}
	operator DBTYPE() { 
		return m_wType;}
	
	void			Free() {
						// the only thing that is allocated here
	}

	CSourceInfo &	operator = (CSourceInfo &SourceInfo) {
						Free();
						m_pwszName			= SourceInfo.m_pwszName;
						m_pwszParseName		= SourceInfo.m_pwszParseName; 
						m_pwszDescription	= SourceInfo.m_pwszDescription;
						m_wType				= SourceInfo.m_wType; 
						m_fIsParent			= SourceInfo.m_fIsParent;
						m_CustomUI			= SourceInfo.m_CustomUI;
						m_Services			= SourceInfo.m_Services;
						m_PropInfoSets		= SourceInfo.m_PropInfoSets;


						return *this;
	}

}; //CSourceInfo




class CSourcesSet{
	protected:
		CVectorEx<CSourceInfo>		m_rgSourcesInfo;
		BOOL			m_fInitialized;

		ULONG			m_ulPos;
		
		// for filtering
		COLEDBServices	*m_pServicesFilter;
		CCustomUI		*m_pCustomUIFilter;
		// source types that are looked for
		ULONG			m_cAllowedSourceTypes;
		DBTYPE			*m_rgAllowedSourceTypes;
		// forbidden source types
		ULONG			m_cExceptedSourceTypes;
		DBTYPE			*m_rgExceptedSourceTypes;

		// looks for the next valid position (according to the filter)
		// it starts fromthe current position
		ULONG		GetNext();

	public:
		CSourcesSet();
		~CSourcesSet();
		
		ULONG		Count(){
			return m_rgSourcesInfo.GetCount();}

		BOOL		FinalInit();

		CSourceInfo	&operator[] (ULONG nIndex);
		CSourceInfo	&operator[] (WCHAR *pwszDescription);
		CSourceInfo	&operator[] (CLSID clsidProv);
		

		// set filter for the iterator and the first position
		BOOL			SetFilter(
			ULONG			cAllowedSourceTypes, 
			DBTYPE			*rgAllowedSourceTypes,
			ULONG			cExceptedSourceTypes,
			DBTYPE			*rgExceptedSourceTypes,
			CCustomUI		*pCustomUIFilter,
			COLEDBServices	*pServicesFilter
		);

		// get the current source information and advance to the next position
		BOOL			GetCurrent(CSourceInfo*);

		ULONG			GetNoOfProviders();

		// @ cmember Finds a property of a given type
		BOOL				FindProperty(
			VARTYPE			vtType,					// [in]  the type of the sought property
			DBPROPINFO		**ppPropInfo,			// [out] the property pointer
			GUID			*pguidPropSet,			// [out] the propset guid
			GUID			*pguidProvider,			// [out] the provider guid
			ULONG			cExclProp = 0,			// [in]  the number of props to be excluded 
			DBPROPID		*rgExclPropID = NULL,	// [in]  the props to be excluded
			GUID			*rgExclPropSet = NULL	// [in]  the propset of the corresponding props
		);
}; //CSourcesSet




// Helper class for modifying provider's keys
// as well as the Retry Wait key of the SC
// to avoid registry corruption problems a global variable will
// store and restore these data
class CModifyRegistry
{
private:
	const CLSID		m_clsidProv;
	HKEY			m_hKey;

	// static functions, may be called with any HKEY
	static BOOL		QueryValue(HKEY hKey, char * szValName, DWORD * pdwVal, BOOL fGetBackupValue=FALSE);
	static BOOL		SetValue(HKEY hKey, char * szValName, DWORD dwVal, BOOL fSetBackupVal = FALSE);
	static BOOL		PreserveValue(HKEY hKey, char * szValName);
	static BOOL		RemoveValue(HKEY hKey, char * szValName);
	static BOOL		RestoreValue(HKEY hKey, char * szValName);

	// read from the registry (either a value or its backup (BAK_name)
	BOOL			QueryValue(char * szValName, DWORD * pdwVal, BOOL fGetBackupValue=FALSE){
						return QueryValue(m_hKey, szValName, pdwVal, fGetBackupValue);
	}
	BOOL			GetBackUpValue(char * szValName, DWORD * pdwVal) {
						return QueryValue(m_hKey, szValName, pdwVal, TRUE);
	}


	// set a value to the registry
	BOOL			SetValue(char * szValName, DWORD dwVal, BOOL fSetBackupVal = FALSE) {
						return SetValue(m_hKey, szValName, dwVal, fSetBackupVal);
	}
	BOOL			SetBackUpValue(char * szValName, DWORD dwVal) { 
						return SetValue(m_hKey, szValName, dwVal, TRUE); 
	}

	
	BOOL			PreserveValue(char * szValName) {
						return PreserveValue(m_hKey, szValName);
	}
	BOOL			RemoveValue(char * szValName) {
						return RemoveValue(m_hKey, szValName);
	}
	BOOL			RestoreValue(char * szValName) {
						return RestoreValue(m_hKey, szValName);
	}

public:
	
					CModifyRegistry(REFCLSID rclsid = GUID_NULL);
				
					~CModifyRegistry(){
						//COMPARE(Restore_SPTimeout(), TRUE);	
						//COMPARE(RestoreServices(), TRUE);	
						SAFE_CLOSEKEY(m_hKey);
					}

	//Members for accessing OLEDB_SERVICES registry entry
	BOOL			BackupServices(){
						return PreserveValue("OLEDB_SERVICES");
	}
	BOOL			GetServices(DWORD *pdwServices) {
						return QueryValue("OLEDB_SERVICES", pdwServices, FALSE);
	}
	BOOL			SetServices(DWORD dwServices);
	BOOL			RemoveServices() {
						//Remove the registry of the given provider
						return RemoveValue("OLEDB_SERVICES");
	}
	BOOL			RestoreServices() {
						return RestoreValue("OLEDB_SERVICES");
	}
	BOOL			IsResourcePoolingAllowed();
	BOOL			IsTransactionEnlistmentAllowed();


	//Members for accessing SPTimeout (for session pooling interval)
	BOOL			Get_SPTimeout(DWORD *pdwSPTimeout) {
						return QueryValue("SPTimeout", pdwSPTimeout);
	}
	BOOL			Set_SPTimeout(DWORD dwSPTimeout);
	BOOL			Remove_SPTimeout() {
						return RemoveValue("SPTimeout"); 
	}
	BOOL			Restore_SPTimeout() {
						return RestoreValue("SPTimeout"); 
	}


	static BOOL		Set_RetryWait(DWORD dwRetryWait);
}; //CModifyRegistry


class CProvPropSets{
	CVectorEx<GUID>					m_rgProviderGUIDs;
	CVectorEx<CPropSets>				m_rgPropSets;
	ULONG								m_cNoOfProviders;

  public:
				CProvPropSets() {m_cNoOfProviders = 0;}
				~CProvPropSets() {m_cNoOfProviders = 0;}

	void		Free() {
		m_rgProviderGUIDs.Free();
		m_rgPropSets.Free();
		m_cNoOfProviders = 0;
	}

	HRESULT		SetProperty(
		GUID			guidProvider, 
		DBPROPID		PropertyID, 
		GUID			guidPropertySet, 
		VARIANT			*pv, 
		DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED, 
		DBID			colid = DB_NULLID
	);

	HRESULT		SetProperty(
		GUID			guidProvider, 
		DBPROPID		PropertyID, 
		GUID			guidPropertySet, 
		DBTYPE			wType, 
		ULONG_PTR		ulValue, 
		DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED, 
		DBID			colid = DB_NULLID
	);

	HRESULT		SetProperty(
		GUID			guidProvider, 
		DBPROPID		PropertyID, 
		GUID			guidPropertySet, 
		DBTYPE			wType, 
		void			*pv, 
		DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED, 
		DBID			colid = DB_NULLID
	);

	DBPROP				*FindProperty(
		GUID			guidProvider, 
		DBPROPID		PropertyID, 
		GUID			guidPropertySet
	);
	
	DBPROPSET			*FindPropertySet(
		GUID			guidProvider, 
		GUID			guidPropertySet
	);

	DBPROPSET			*FindProvPropertySets(
		GUID			guidProvider
	);

	CPropSets			*FindProvCPropSets(
		GUID			guidProvider
	);

	LONG				GetProviderIndex(
		GUID			guidProvider
	);

	BOOL				AddProvider(
		GUID			guidProvider,
		ULONG			cPropSets	= 0,
		DBPROPSET		*rgPropSets	= NULL
	);

	BOOL				AddProviderWithDefPropValues(
		GUID			guidProvider
	);

}; //CProvPropSets






#endif //__PROVIDER_INFO

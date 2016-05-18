//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module DataSource Implementation Module | 	This module contains definition information
//					for the CLightDataSource, ClightSession and CLightRowset classes
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


#include "DataSource.hpp"
#include "allocator.cpp"
#include "CPropSet.cpp"

#include "mtxdm.h"

extern const GUID CLSID_MSDASQL;

const ULONG cGetPropertyInfo_SucceedsForNonInitVals				= 1;
const ULONG cGetProperties_SucceedsForNonInitVals				= 2;
const ULONG cQI_ForIDBCreateSessionAllowed						= 4;
const ULONG cIfIPersistFileExists_ReturnsAlreadyInitialized		= 8;
// make sure this is the last value
const ULONG cAll												= 
			cGetPropertyInfo_SucceedsForNonInitVals | cGetProperties_SucceedsForNonInitVals
		|	cQI_ForIDBCreateSessionAllowed | cIfIPersistFileExists_ReturnsAlreadyInitialized;

CSourcesSet	*CLightDataSource::s_pSourcesSet = NULL;
ULONG	g_ulTag = 1;

const IID	*rgDSOInterfaces[] = {
	&IID_IUnknown,
	&IID_IDBCreateSession,
	&IID_IDBInitialize,
	&IID_IDBProperties,
	&IID_IPersist,
	&IID_IConnectionPointContainer,
	&IID_IDBAsynchStatus,
	&IID_IDBDataSourceAdmin,
	&IID_IDBInfo,
	&IID_IPersistFile,
	&IID_ISupportErrorInfo,
};



BOOL CLightDataSource::SetProviderCLSID(CLSID clsidProvider)
{
	TBEGIN

	if (m_pModifyRegistry && m_pclsidProvider && *m_pclsidProvider != clsidProvider)
		SAFE_DELETE(m_pModifyRegistry);

	if (NULL == m_pclsidProvider)
		SAFE_ALLOC(m_pclsidProvider, CLSID, 1);

	*m_pclsidProvider = clsidProvider;

CLEANUP:
	TRETURN
} //CLightDataSource::SetProviderCLSID



IUnknown *CLightDataSource::pIUnknown()
{
	IUnknown	*pIUnk = NULL;

#define INIT_ACT(Interface)						\
	if (m_p##Interface)							\
		pIUnk = (Interface*)m_p##Interface;	

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		ASSERT(0 == m_lRef);

	// look for an interface and return it
	APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
	return pIUnk;
} //CLightDataSource::pIUnknown



HRESULT CLightDataSource::Attach(
	REFIID				riid,				// [in] Interface to be cached
	IUnknown			*pIUnknown,			// [in] Pointer to that interface
	CREATIONMETHODSENUM	dwCreationMethod	// [in] the method used to create the object
)
{
	// release from previous
	ReleaseAll();

#define INIT_ACT(Interface)							\
	if (IID_##Interface == riid)					\
	{												\
		ASSERT( NULL == m_p##Interface);						\
		m_p##Interface = (Interface*)pIUnknown;		\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	AddRef();
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT

	// the DSO interface should be attached as soon as created
	// we suppose it wasn't initialized
	m_fInitialized = FALSE;

	if (!IsNativeInterface(riid))
		m_fIsDirty = TRUE;

	m_dwCreationMethod = dwCreationMethod;

	CHECK(GetProperties(&m_PropSets), S_OK);

	return S_OK;
} //CLightDataSource::Attach



HRESULT CLightDataSource::CacheInterface(
	REFIID	riid		// [in] Interface to be cached
)
{
	HRESULT	hr		= S_OK;
	HRESULT	hres	= E_FAIL;

#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			TEST2C_(hr, S_OK, E_NOINTERFACE);									\
			TESTC(SUCCEEDED(hr)? NULL != m_p##Interface : NULL == m_p##Interface);	\
			hres = hr;															\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		else hres = S_OK;														\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	if ( (S_OK == hres) && !m_fInitialized && !IsNativeInterface(riid))
		m_fIsDirty = TRUE;

CLEANUP:
	return hres;
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightDataSource::CacheInterface



HRESULT CLightDataSource::QueryInterface(REFIID riid, LPVOID *ppInterface)
{
	IUnknown	*pIUnk	= pIUnknown();
	HRESULT		hr		= S_OK;

	if (!ppInterface)
		return E_INVALIDARG;
	else
		*ppInterface = NULL;

	ASSERT(pIUnk);


#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		*ppInterface = m_p##Interface;											\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	if (*ppInterface)
	{
		((IUnknown*)(*ppInterface))->AddRef();

		if (!IsNativeInterface(riid))
			m_fIsDirty = TRUE;
	}
	return hr;

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightDataSource::QueryInterface
 


HRESULT CLightDataSource::ReleaseInterface(REFIID riid)
{
#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)		\
	{									\
		if (m_p##Interface)				\
			Release();					\
		SAFE_RELEASE(m_p##Interface);	\
	}

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	Release();
	return S_OK;
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightDataSource::ReleaseInterface




HRESULT CLightDataSource::SetProperties(ULONG cPropSets, DBPROPSET *rgPropSets)
{
	HRESULT			hr				= E_FAIL;
	IDBProperties	*pIDBProperties	= NULL;
	DBORDINAL		ulPropSets;
	DBORDINAL		ulProp;
	DBPROPSET		*pPropSets;
	DBPROP			*pProp;

	//set properties
	CacheInterface(IID_IDBProperties);
	hr = m_pIDBProperties->SetProperties(cPropSets, rgPropSets);

	if (SUCCEEDED(hr))
	{
		TESTC((0 == cPropSets) || (NULL != rgPropSets));

		// iterate the property sets
		pPropSets = rgPropSets;
		for (ulPropSets = 0; ulPropSets < cPropSets; ulPropSets++, pPropSets++)
		{
			TESTC((0 == pPropSets->cProperties) || (NULL != pPropSets->rgProperties));
			
			// iterate through all the properties
			pProp = pPropSets->rgProperties;
			for (ulProp=0; ulProp < pPropSets->cProperties; ulProp++, pProp++)
			{
				// if the property was set update the cache mentained for the DSO
				if (DBPROPSTATUS_OK == pProp->dwStatus)
				{
					m_PropSets.SetProperty(pProp->dwPropertyID, pPropSets->guidPropertySet,
						&pProp->vValue, pProp->dwOptions, pProp->colid);
				}
			}
		}
	}

CLEANUP:
	return hr;
} // CLightDataSource::SetProperties



HRESULT CLightDataSource::GetPropertyInfo(
	ULONG				cPropertyIDSets,
	const DBPROPIDSET	rgPropertyIDSets[],
	ULONG				*pcPropertyInfoSets,
	DBPROPINFOSET		**prgPropertyInfoSets,
	OLECHAR				**ppDescBuffer
)
{
	HRESULT		hr;
	HRESULT		rgValidRes[]	= {	
									S_OK,
									DB_S_ERRORSOCCURRED,
									//E_FAIL,
									E_INVALIDARG,
									E_OUTOFMEMORY, 
									DB_E_ERRORSOCCURRED,
								};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	CacheInterface(IID_IDBProperties);

	hr = m_pIDBProperties->GetPropertyInfo(cPropertyIDSets, rgPropertyIDSets, 
		pcPropertyInfoSets, prgPropertyInfoSets, ppDescBuffer); 
	
	// basic checking on method's activity

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	if (	(NULL == pcPropertyInfoSets) 
		||	(NULL == prgPropertyInfoSets)
		||	((0 < cPropertyIDSets) && (NULL == rgPropertyIDSets))
		)
		CHECK(hr, E_INVALIDARG);

	if (FAILED(hr) && (DB_E_ERRORSOCCURRED != hr))
	{
		if (prgPropertyInfoSets)
			COMPARE(NULL == *prgPropertyInfoSets, TRUE);
		if (pcPropertyInfoSets)
			COMPARE(0 == *pcPropertyInfoSets, TRUE);
	}

	if (ppDescBuffer && (FAILED(hr) || (0 == *pcPropertyInfoSets)))
		COMPARE(NULL == *ppDescBuffer, TRUE);
	return hr;

} //CLightDataSource::GetPropertyInfo




HRESULT CLightDataSource::GetProperties(
	ULONG		cPropertyIDSets,
	DBPROPIDSET	*rgPropertyIDSets, 
	ULONG		*pcPropSets, 
	DBPROPSET	**prgPropSets
)
{
	HRESULT		hr;
	HRESULT		rgValidRes[]	= {	S_OK,
									DB_S_ERRORSOCCURRED,
									E_INVALIDARG,
									E_OUTOFMEMORY,
									DB_E_ERRORSOCCURRED,
//									E_FAIL,
									};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	CacheInterface(IID_IDBProperties);

	hr = m_pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets); 
	
	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	if (NULL == pcPropSets || NULL == prgPropSets)
		CHECK(hr, E_INVALIDARG);

	return hr;
} //CLightDataSource::GetProperties



HRESULT CLightDataSource::GetProperties(CPropSets *pPropSets)
{
	ULONG		cPropSets = 0;
	DBPROPSET	*rgPropSets = NULL;
	HRESULT		hr = E_FAIL;
	
	if (!pPropSets)
		return E_INVALIDARG;

	hr = GetProperties(&cPropSets, &rgPropSets);
	pPropSets->Attach(cPropSets, rgPropSets);
	return hr;
} //CLightDataSource::GetProperties



HRESULT CLightDataSource::GetInitProperties(CPropSets *pPropSets)
{
	ULONG		cPropertyIDSets		= 0;
	DBPROPIDSET	*rgPropertyIDSets	= NULL;
	HRESULT		hr					= E_FAIL;
	ULONG		cPropSets			= 0;
	DBPROPSET	*rgPropSets			= NULL;
	ULONG		index;

	if (!pPropSets)
		return E_INVALIDARG;

	// build the propID set
	if (pPropSets->m_pPropInfoSets)
	{
		cPropertyIDSets = pPropSets->m_pPropInfoSets->cPropInfoSets();
		SAFE_ALLOC(rgPropertyIDSets, DBPROPIDSET, cPropertyIDSets);
		for (index = 0; index < cPropertyIDSets; index++)
		{
			rgPropertyIDSets[index].guidPropertySet = (*(pPropSets->m_pPropInfoSets))[index].guidPropertySet;
			rgPropertyIDSets[index].cPropertyIDs	= 0;
			rgPropertyIDSets[index].rgPropertyIDs	= NULL;
		}
	}
	else
	{
		cPropertyIDSets = 1;
		SAFE_ALLOC(rgPropertyIDSets, DBPROPIDSET, cPropertyIDSets);
		rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
		rgPropertyIDSets[0].cPropertyIDs	= 0;
		rgPropertyIDSets[0].rgPropertyIDs	= NULL;
	}

	hr = GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
	pPropSets->Attach(cPropSets, rgPropSets);

CLEANUP:
	return hr;
} //CLightDataSource::GetInitProperties




BOOL CLightDataSource::CheckPropertyValues()
{
	// compare the properties cached by CLightDataSource
	// with the datasource properties
	TBEGIN
	HRESULT		hr;
	ULONG		ulPropIDSets = 0;
	DBPROPIDSET	*rgPropIDSets = NULL;
	ULONG		index;
	ULONG		cProp;
	ULONG		ulPropSets = 0;
	DBPROPSET	*rgPropSets = NULL;
	DBPROP		*pProp	= NULL;
	DBPROP		*pProp2	= NULL;

	SAFE_ALLOC(rgPropIDSets, DBPROPIDSET, (DBLENGTH)m_PropSets.cPropertySets());
	memset(rgPropIDSets, 0, (size_t)m_PropSets.cPropertySets() * sizeof(DBPROPIDSET));
	ulPropIDSets = m_PropSets.cPropertySets();

	for (index = 0; index < m_PropSets.cPropertySets(); index++)
	{
		SAFE_ALLOC(rgPropIDSets[index].rgPropertyIDs, DBPROPID, m_PropSets[index].cProperties);
		rgPropIDSets[index].cPropertyIDs = m_PropSets[index].cProperties;
		rgPropIDSets[index].guidPropertySet = m_PropSets[index].guidPropertySet;

		for (cProp=0; cProp < m_PropSets[index].cProperties; cProp++)
		{
			rgPropIDSets[index].rgPropertyIDs[cProp] = m_PropSets[index].rgProperties[cProp].dwPropertyID;
			
			if (DBPROP_INIT_OLEDBSERVICES == m_PropSets[index].rgProperties[cProp].dwPropertyID)
			{
				pProp = &m_PropSets[index].rgProperties[cProp];
			}
		}
	}

	TEST2C_(hr = GetProperties(ulPropIDSets, rgPropIDSets, &ulPropSets, &rgPropSets), DB_S_ERRORSOCCURRED, S_OK);

	// compare everything
	for (index = 0; index < m_PropSets.cPropertySets(); index++)
	{
		pProp	= m_PropSets[index].rgProperties;
		pProp2	= rgPropSets[index].rgProperties;

		ASSERT(m_PropSets[index].cProperties == rgPropSets[index].cProperties);

		for (cProp=0; cProp < m_PropSets[index].cProperties; cProp++, pProp++, pProp2++)
		{
			ASSERT(pProp->dwPropertyID == pProp2->dwPropertyID);
			
			if (	(DBPROP_INIT_OLEDBSERVICES == pProp->dwPropertyID)
				&&	DBPROPSTATUS_NOTSUPPORTED == pProp2->dwStatus)
			{
				CHECK(hr, DB_S_ERRORSOCCURRED);	// supposing that we don't set just this prop
				//COMPARE(ShouldBePooled(), FALSE);
				continue;
			}

			COMPARE(DBPROPSTATUS_OK == pProp2->dwStatus, TRUE);

			if (DBPROP_AUTH_PASSWORD == pProp->dwPropertyID)
				continue;

			if (VT_EMPTY == pProp->vValue.vt)
				continue;

			ASSERT(pProp->vValue.vt == pProp2->vValue.vt);

			if (	(VT_EMPTY != pProp->vValue.vt)
				&&	!COMPARE(CompareVariant(&pProp->vValue, &pProp2->vValue), TRUE))
			{
				WCHAR	wszExpectedVal[cMaxName];
				WCHAR	wszActualVal[cMaxName];
				//g_nTabs++;
				//Ident();
				odtLog << "Prop: " << GetPropertyName(pProp->dwPropertyID, rgPropSets[index].guidPropertySet) << "\n";

				CHECK(VariantToString(&pProp->vValue, wszExpectedVal, cMaxName), S_OK);
				CHECK(VariantToString(&pProp2->vValue, wszActualVal, cMaxName), S_OK);
				odtLog << " expected_value = " << wszExpectedVal << " actual_value = " << wszActualVal << "\n";
				//g_nTabs--;
			}

		}
	}

CLEANUP:
	FreeProperties(&ulPropIDSets, &rgPropIDSets);
	TRETURN
} // CLightDataSource::CheckPropertyValues




HRESULT CLightDataSource::Initialize()
{
	HRESULT			hr				= E_FAIL;
	IDBInitialize	*pIDBInitialize	= NULL;
	HRESULT			rgValidRes[]	= {
										S_OK,
										DB_S_ASYNCHRONOUS,
										DB_S_ERRORSOCCURRED,
										E_FAIL,
										E_OUTOFMEMORY,
										E_UNEXPECTED,
										DB_E_ALREADYINITIALIZED,
										DB_E_CANCELED,
										DB_E_ERRORSOCCURRED,
										DB_SEC_E_AUTH_FAILED,
									};
	DBORDINAL		cValidRes		= NUMELEM(rgValidRes);
	DWORD			dwStatus = 0;

	TESTC_(CacheInterface(IID_IDBInitialize), S_OK);
	
	hr = m_pIDBInitialize->Initialize();

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	// make sure error is retrieved when the datasource is already initialized
	if (m_fInitialized)
		CHECK(hr, DB_E_ALREADYINITIALIZED);

	// Note: If the IDBInitialize::Initialize fails, session pooling marks
	// the DPO as dirty and does not try to pool it anymore
	if (FAILED(hr))
	{
	}
	else
	{
		// was DBPROP_INIT_PROMPT set for the DSO?
		DBPROP	*pProp = m_PropSets.FindProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT);

		if (pProp && (VT_EMPTY != pProp->vValue.vt))
		{
			ASSERT(VT_I2 == pProp->vValue.vt);
			if (DBPROMPT_NOPROMPT != V_I2(&pProp->vValue))
				m_fIsDirty = TRUE;
		}
	}

	m_fInitialized = m_fInitialized || SUCCEEDED(hr);

	// call DPO status method for basic checking

CLEANUP:
	return hr;
} // CLightDataSource::Initialize



HRESULT CLightDataSource::Uninitialize()
{
	HRESULT			hr				= E_FAIL;
	IDBInitialize	*pIDBInitialize	= NULL;
	HRESULT			rgValidRes[]	= {
										S_OK,
										E_FAIL,
										E_UNEXPECTED,
										DB_E_OBJECTOPEN,
									 };
	DBORDINAL		cValidRes		= NUMELEM(rgValidRes);


	// make sure the interface is available
	TESTC_(CacheInterface(IID_IDBInitialize), S_OK);
	
	hr = m_pIDBInitialize->Uninitialize();

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	// if datasource was not initialized, unitialize should suceed
	if (!m_fInitialized)
		CHECK(hr, S_OK);

	if (m_fInitialized && SUCCEEDED(hr))
	{
		// trace in m_fIsDirty that the data source was once initialized
		m_fIsDirty = TRUE;
		// data source is not initialized
		m_fInitialized = FALSE;
	}

CLEANUP:
	return hr;
} // CLightDataSource::Uninitialize




HRESULT CLightDataSource::CreateSession(
	IUnknown	*pUnkOuter,
	REFIID		riid,
	IUnknown	**ppDBSession
)
{
	HRESULT			hr				= E_FAIL;
	IDBInitialize	*pIDBInitialize	= NULL;
	HRESULT			rgValidRes[]	= {
										S_OK,
										//E_FAIL,
										E_INVALIDARG,
										E_NOINTERFACE,
										E_OUTOFMEMORY,
										E_UNEXPECTED,
										DB_E_NOAGGREGATION,
										DB_E_OBJECTCREATIONLIMITREACHED,
										DB_E_OBJECTOPEN,
									};
	DBORDINAL		cValidRes		= NUMELEM(rgValidRes);


	// make sure the interface is available
	TESTC_(CacheInterface(IID_IDBCreateSession), S_OK);
	
	hr = m_pIDBCreateSession->CreateSession(pUnkOuter, riid, ppDBSession);

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	// some basic checking
	if (!m_fInitialized)
		CHECK(hr, E_UNEXPECTED);

	if (E_UNEXPECTED == hr)
		COMPARE(FALSE == m_fInitialized, TRUE);

	if (ppDBSession && FAILED(hr))
		COMPARE(NULL == *ppDBSession, TRUE);

	if (NULL == ppDBSession)
		CHECK(hr, E_INVALIDARG);

CLEANUP:
	return hr;
} //CLightDataSource::CreateSession




HRESULT CLightDataSource::CreateSession(
	IUnknown	*pUnkOuter,
	REFIID		riid,
	CLightSession	*pCLightSession
)
{
	HRESULT		hr;
	IUnknown	*pIUnknown	= NULL;

	if (NULL == pCLightSession)
		return E_INVALIDARG;

	TESTC_(CacheInterface(IID_IDBCreateSession), S_OK);

	hr = m_pIDBCreateSession->CreateSession(pUnkOuter, riid, &pIUnknown);

	TESTC_(pCLightSession->Attach(riid, pIUnknown), S_OK);
	if (SUCCEEDED(hr))
		TESTC(pCLightSession->CheckTransactionEnlistment(this));

CLEANUP:
	return hr;
} //CLightDataSource::CreateSession




CLightDataSource *CLightDataSource::operator = (IUnknown *pIUnk)
{
	ReleaseAll();

	Attach(IID_IUnknown, pIUnk, CREATIONMETHODS_UNKNOWN);
	// unlike Attach, the reference counter of the interface is incremented here
	pIUnk->AddRef();

	return this;
} // CLightDataSource::operator = 


//	CLightDataSource::GetDSOStatus
//	Returns TRUE if there is any indication that this Datasource is initialized
//	Returns FALSE if all indications of Initialization are false
//	Details in what way it seems Initialized using the INIT_INDICATORS structure
//	Does following tests:
//		1) GetPropertyInfo for non Initialization values
//		2) GetProperties for non Initialization values
//		3) QI for IDBCreateSession
//		4) IPersistFile::Load returns AlreadyInitialized, if IPersistFile is supported
BOOL CLightDataSource::GetDSOStatus()
{
	ULONG			initIndicators		= 0;
	IUnknown		*pIUnknown2			= NULL;

	//GetPropertyInfo params
	DBPROPIDSET		propidsetNonInit2		={NULL, 0, GUID_MEMBERS(DBPROPSET_DATASOURCEINFO)};
	ULONG			cpropinfosetsOut		(0ul);
	DBPROPINFOSET	(*rgpropinfosetsOut)	(NULL);

	//GetProperties params
	DBPROPIDSET		propidsetNonInit		={NULL, 0, GUID_MEMBERS(DBPROPSET_DATASOURCE)}; 
	ULONG			cPropsetsOut			(0ul);
	DBPROPSET		(*rgPropsetsOut)		(NULL);

	IUnknown			*pIUnk				= pIUnknown();	// remmeber not to release this interface!
	IDBProperties		*pIDBProperties		= NULL;
	IDBCreateSession	*pIDBCreateSession	= NULL;
	IPersistFile		*pIPersistFile		= NULL;

	TESTC(NULL != pIUnk);
	TESTC_(pIUnk->QueryInterface(IID_IDBProperties, (LPVOID*)&pIDBProperties), S_OK);
	pIUnk->QueryInterface(IID_IDBCreateSession, (LPVOID*)&pIDBCreateSession);
	pIUnk->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

	// - Actual Tests - 
	//1) GetPropertyInfo for non Initialization values
	if (SUCCEEDED(pIDBProperties->GetPropertyInfo(1, &propidsetNonInit, &cpropinfosetsOut, &rgpropinfosetsOut, NULL)))
		initIndicators += cGetPropertyInfo_SucceedsForNonInitVals;

	//2) GetProperties for non Initialization values
	if (SUCCEEDED(pIDBProperties->GetProperties(1, &propidsetNonInit2, &cPropsetsOut, &rgPropsetsOut)))
		initIndicators += cGetProperties_SucceedsForNonInitVals;

	//3) QI for IDBCreateSession
	if (pIDBCreateSession)
	{
		if (E_UNEXPECTED != pIDBCreateSession->CreateSession(NULL, IID_IUnknown, &pIUnknown2))
			initIndicators += cQI_ForIDBCreateSessionAllowed;
	}

	//4) IPersistFile::Load returns AlreadyInitialized, if IPersistFile is supported
	if (	pIPersistFile
		&&	(DB_E_ALREADYINITIALIZED == pIPersistFile->Load(L"", 0)))
		initIndicators += cIfIPersistFileExists_ReturnsAlreadyInitialized;

CLEANUP:
	//Cleanup
	FreeProperties(&cpropinfosetsOut, &rgpropinfosetsOut);
	FreeProperties(&cPropsetsOut,     &rgPropsetsOut);
	SAFE_RELEASE(pIUnknown2);
	SAFE_RELEASE(pIPersistFile);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIDBProperties);

	//if any indicators are true, return TRUE, otherwise return FALSE
	if (0 != initIndicators && cAll != initIndicators)
		odtLog << initIndicators;

	return 0 != initIndicators;
} //CLightDataSource::GetDSOStatus




//---------------------------------------------------------------
//
//		CLightDataSource::SetCurrentCatalog
//
// Sets the current catalog
//---------------------------------------------------------------
HRESULT CLightDataSource::SetCurrentCatalog(WCHAR *pwszCatalogName)
{
	HRESULT			hr = E_FAIL;
	CPropSets		PropSets;

	TESTC_(CacheInterface(IID_IDBProperties), S_OK);
	TESTC_(PropSets.AddProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, VT_BSTR, (LPVOID)pwszCatalogName), S_OK);
	hr = SetProperties(&PropSets);

CLEANUP:
	return hr;
} // CLightDataSource::SetCurrentCatalog




WCHAR *CLightDataSource::GetCurrentCatalog()
{
	HRESULT			hr = E_FAIL;
	CPropSets		PropSets;
	DBPROP			*pInitCatalog;
	WCHAR			*pwszCrtCat = NULL;

	TESTC_(CacheInterface(IID_IDBProperties), S_OK);
	TESTC_(GetProperties(&PropSets), S_OK);

	pInitCatalog = PropSets.FindProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE);
	TESTC(NULL != pInitCatalog);
	TESTC(VT_BSTR == pInitCatalog->vValue.vt);

	pwszCrtCat = wcsDuplicate(V_BSTR(&pInitCatalog->vValue));

CLEANUP:
	return pwszCrtCat;
} // CLightDataSource::GetCurrentCatalog








// session class 
HRESULT CLightSession::CacheInterface(
	REFIID	riid		// [in] Interface to be cached
)
{
	HRESULT	hr		= S_OK;
	HRESULT	hres	= E_FAIL;

#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			COMPARE((SUCCEEDED(hr)? NULL != m_p##Interface : NULL == m_p##Interface), TRUE);	\
			hres = hr;															\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		else hres = S_OK;														\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT

	return hres;
} //CLightSession::CacheInterface



IUnknown *CLightSession::pIUnknown()
{
	IUnknown	*pIUnk = NULL;

#define INIT_ACT(Interface)						\
	if (m_p##Interface)							\
		pIUnk = (Interface*)m_p##Interface;	

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		ASSERT(0 == m_lRef);

	// look for an interface and return it
	APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
	return pIUnk;
} //CLightSession::pIUnknown



HRESULT CLightSession::ReleaseInterface(REFIID riid)
{
#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)		\
	{									\
		if (m_p##Interface)				\
			Release();					\
		SAFE_RELEASE(m_p##Interface);	\
	}

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	Release();
	return S_OK;
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightSession::ReleaseInterface




HRESULT CLightSession::QueryInterface(REFIID riid, LPVOID *ppInterface)
{
	IUnknown	*pIUnk	= pIUnknown();
	HRESULT		hr			= S_OK;

	if (!ppInterface)
		return E_INVALIDARG;
	else
		*ppInterface = NULL;

	ASSERT(pIUnk);


#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		*ppInterface = m_p##Interface;											\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	if (*ppInterface)
		((IUnknown*)(*ppInterface))->AddRef();
	return hr;

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightSession::QueryInterface
 


HRESULT CLightSession::Attach(
	REFIID		riid,		// [in] Interface to be cached
	IUnknown	*pIUnknown	// [in] Pointer to that interface
)
{
	// release from previous
	ReleaseAll();

#define INIT_ACT(Interface)							\
	if (IID_##Interface == riid)					\
	{												\
		ASSERT( NULL == m_p##Interface);						\
		m_p##Interface = (Interface*)pIUnknown;		\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	AddRef();
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT

	return S_OK;
} //CLightSession::Attach



CLightSession *CLightSession::operator = (IUnknown *pIUnk)
{
	ReleaseAll();

	Attach(IID_IUnknown, pIUnk);
	// unlike Attach, the reference counter of the interface is incremented here
	pIUnk->AddRef();

	return this;
} // CLightSession::operator = 



// check that the session was enlisted if required and possible
BOOL CLightSession::CheckTransactionEnlistment(CLightDataSource *pDSO)
{
	TBEGIN
	BOOL	fIsInTransaction	= FALSE;
	BOOL	fHasRejoined		= TRUE;

	TESTC(NULL != pDSO);


CLEANUP:
	TRETURN
} //CLightSession::CheckTransactionEnlistment



HRESULT	CLightSession::GetDataSource(
	REFIID		riid,
	IUnknown	**ppDataSource
)
{
	HRESULT	hr = E_FAIL;

	TESTC_(hr = CacheInterface(IID_IGetDataSource), S_OK);

	hr = m_pIGetDataSource->GetDataSource(riid, ppDataSource);

CLEANUP:
	return hr;
} //CLightSession::GetDataSource



HRESULT CLightSession::GetDataSource(REFIID riid, CLightDataSource &rDataSource)
{
	IUnknown	*pIUnknown;
	HRESULT		hr = GetDataSource(riid, (IUnknown**)&pIUnknown);
	
	rDataSource.Attach(riid, pIUnknown, CREATIONMETHODS_UNKNOWN);
	return hr;
} //CLightSession::GetDataSource



// session class 
HRESULT CLightRowset::CacheInterface(
	REFIID	riid		// [in] Interface to be cached
)
{
	HRESULT	hr		= S_OK;
	HRESULT	hres	= E_FAIL;

#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			TEST2C_(hr, S_OK, E_NOINTERFACE);									\
			TESTC(SUCCEEDED(hr) ? (NULL != m_p##Interface) : (NULL == m_p##Interface));	\
			hres = hr;															\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		else hres = S_OK;														\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

CLEANUP:
	return hres;
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightRowset::CacheInterface



IUnknown *CLightRowset::pIUnknown()
{
	IUnknown	*pIUnk = NULL;

#define INIT_ACT(Interface)						\
	if (m_p##Interface)							\
		pIUnk = (Interface*)m_p##Interface;	

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		ASSERT(0 == m_lRef);

	// look for an interface and return it
	APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
	return pIUnk;
} //CLightRowset::pIUnknown



HRESULT CLightRowset::ReleaseInterface(REFIID riid)
{
#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)		\
	{									\
		if (m_p##Interface)				\
			Release();					\
		SAFE_RELEASE(m_p##Interface);	\
	}

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	ASSERT(0 < m_lRef);

	// cache the interface pointer retrieved
	APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	Release();
	return S_OK;
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightRowset::ReleaseInterface




HRESULT CLightRowset::QueryInterface(REFIID riid, LPVOID *ppInterface)
{
	IUnknown	*pIUnk	= pIUnknown();
	HRESULT		hr			= S_OK;

	if (!ppInterface)
		return E_INVALIDARG;
	else
		*ppInterface = NULL;

	ASSERT(pIUnk);


#define INIT_ACT(Interface)														\
	if (IID_##Interface == riid)												\
	{																			\
		if (!m_p##Interface)													\
		{																		\
			hr = pIUnknown()->QueryInterface(riid, (LPVOID*)&m_p##Interface);	\
			if (S_OK == hr)														\
				AddRef();														\
		}																		\
		*ppInterface = m_p##Interface;											\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	if (*ppInterface)
		((IUnknown*)(*ppInterface))->AddRef();
	return hr;

#undef INIT_ACT
#undef ACT
#undef FINAL_ACT
} //CLightRowset::QueryInterface
 


HRESULT CLightRowset::Attach(
	REFIID		riid,		// [in] Interface to be cached
	IUnknown	*pIUnknown	// [in] Pointer to that interface
)
{
	// release from previous
	ReleaseAll();

#define INIT_ACT(Interface)							\
	if (IID_##Interface == riid)					\
	{												\
		ASSERT( NULL == m_p##Interface);						\
		m_p##Interface = (Interface*)pIUnknown;		\
	}								

#define	ACT(Interface)							\
	else INIT_ACT(Interface)

#define FINAL_ACT(Interface)					\
		ACT(Interface)							\
	else										\
		return E_FAIL;

	// cache the interface pointer retrieved
	APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

	AddRef();
#undef INIT_ACT
#undef ACT
#undef FINAL_ACT

	return S_OK;
} //CLightRowset::Attach



CLightRowset *CLightRowset::operator = (IUnknown *pIUnk)
{
	ReleaseAll();

	Attach(IID_IUnknown, pIUnk);
	// unlike Attach, the reference counter of the interface is incremented here
	pIUnk->AddRef();

	return this;
} // CLightRowset::operator = 



HRESULT CLightRowset::RestartPosition(HCHAPTER hChapter)
{
	HRESULT	hr = E_FAIL;
	HRESULT		rgValidRes[]	= {	
									S_OK,
									DB_S_COLUMNSCHANGED,
									DB_S_COMMANDREEXECUTED,
									E_UNEXPECTED,
									DB_E_BADCHAPTER,
									DB_E_CANCELED,
									DB_E_CANNOTRESTART,
									DB_E_NOTREENTRANT,
									DB_E_ROWSNOTRELEASED,
									DB_SEC_E_PERMISSIONDENIED,
									E_FAIL,
								};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	TESTC_(hr = CacheInterface(IID_IRowset), S_OK);

	hr = m_pIRowset->RestartPosition(hChapter);

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	// if schema has changed, current bindings might be out of synch => drop them, accessor and row data
	if (DB_S_COLUMNSCHANGED == hr)
		TESTC(ReleaseRowData());;

CLEANUP:
	return hr;
} //CLightRowset::RestartPosition 



HRESULT CLightRowset::GetNextRows(
	HCHAPTER		hChapter, 
	DBROWOFFSET		lOffset, 
	DBROWCOUNT		cRows, 
	DBCOUNTITEM		*pcRowsObtained, 
	HROW			**prghRows
)
{
	ASSERT(prghRows);
	TBEGIN
	
	HRESULT		hr = E_FAIL;
	DBCOUNTITEM	iRow=0;	
	//Record if we passed in consumer allocated array...
	HROW		*rghRowsInput	= *prghRows;
	HRESULT		rgValidRes[]	= {	
									S_OK,
									DB_S_ENDOFROWSET,
									DB_S_ROWLIMITEXCEEDED,
									DB_S_STOPLIMITREACHED,
									E_INVALIDARG,
									E_OUTOFMEMORY, 
									E_UNEXPECTED,
									DB_E_BADCHAPTER,
									DB_E_BADSTARTPOSITION,
									DB_E_CANCELED,
									DB_E_CANTFETCHBACKWARDS,
									DB_E_CANTSCROLLBACKWARDS,
									DB_E_NOTREENTRANT,
									DB_E_ROWSNOTRELEASED,
									DB_SEC_E_PERMISSIONDENIED,
									E_FAIL,
								};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	TESTC_(CacheInterface(IID_IRowset), S_OK);
	
	//GetNextRows
	hr = m_pIRowset->GetNextRows(hChapter, lOffset, cRows, pcRowsObtained, prghRows);
	
	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	//Verify Correct values returned
	if(SUCCEEDED(hr))
	{
		if(hr == S_OK)
		{
			TESTC(!pcRowsObtained || (*pcRowsObtained==(DBCOUNTITEM)ABS(cRows)));
		}
		else
		{
			TESTC(!pcRowsObtained || (*pcRowsObtained < (DBCOUNTITEM)ABS(cRows)));
		}

		//Verify row array
		for(iRow=0; pcRowsObtained && (iRow<*pcRowsObtained); iRow++)
		{
			TESTC(*prghRows != NULL);
			TESTC((*prghRows)[iRow]!=DB_NULL_HROW)
		}
	}
	else
	{
		TESTC(!pcRowsObtained || (0 == *pcRowsObtained));
	}

	//Verify output array, depending upon consumer or provider allocated...
	if(rghRowsInput)
	{
		//This is a users allocated static array,
		//This had better not be nulled out by the provider, if non-null on input
		TESTC(*prghRows == rghRowsInput);
	}
	else
	{
		TESTC(!pcRowsObtained || ((*pcRowsObtained) ? NULL != *prghRows : NULL == *prghRows));
	}

CLEANUP:
	return hr;
} //CLightRowset::GetNextRows



HRESULT CLightRowset::GetData(
	HROW		hRow, 
	HACCESSOR	hAccessor, 
	void		*pData
)
{
	TBEGIN
	DBACCESSORFLAGS	dwAccessorFlags;
	DBCOUNTITEM		cBindings		= 0;
	DBBINDING		*rgBindings		= NULL;
	HRESULT			hr				= E_FAIL;
	HRESULT			hrAcc			= E_FAIL;
	HRESULT			rgValidRes[]	= {	
										S_OK,
										DB_S_ERRORSOCCURRED,
										E_INVALIDARG,
										E_UNEXPECTED,
										DB_E_BADACCESSORHANDLE,
										DB_E_BADACCESSORTYPE,
										DB_E_BADROWHANDLE,
										DB_E_DELETEDROW,
										DB_E_ERRORSOCCURRED,
										E_FAIL,
									};
	DBORDINAL		cValidRes		= NUMELEM(rgValidRes);

	TESTC_(hr = CacheInterface(IID_IRowset), S_OK);

	//Obtain the accessor bindings
	CHECK(hrAcc = GetBindings(hAccessor, &dwAccessorFlags, 
		&cBindings, &rgBindings), S_OK);

	//Get the Data for row hRow
	hr = m_pIRowset->GetData(hRow, hAccessor, pData);

	if (DB_E_BADACCESSORHANDLE == hrAcc)
		CHECK(hr, DB_E_BADACCESSORHANDLE);

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	
	//Display any binding errors and status
	if (SUCCEEDED(hrAcc))
		TESTC(VerifyBindings(hr, cBindings, rgBindings, pData));

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
} //CLightRowset::GetData



BOOL CLightRowset::ReleaseRowData()
{
	TBEGIN
		
	if (!m_hAccessor)
	{
		TESTC(0 == m_cBindings);
		TESTC(NULL == m_rgBindings);
		TESTC(0 == m_cRowSize);
		TESTC(NULL == m_pData);
		goto CLEANUP;
	}

	TESTC(0 < m_cBindings);
	TESTC(NULL != m_rgBindings);
	TESTC(0 < m_cRowSize);
	TESTC(NULL != m_pData);

	//Release outofline memory, created from FillInputBindings
	TESTC_(ReleaseInputBindingsMemory(m_cBindings, m_rgBindings,(BYTE*)m_pData, TRUE),S_OK)

CLEANUP:
	FreeAccessorBindings(m_cBindings, m_rgBindings);
	m_hAccessor		= NULL;
	m_cBindings		= 0;
	m_rgBindings	= NULL;
	m_cRowSize		= 0;
	m_pData			= NULL;
	TRETURN
} //CLightRowset::ReleaseRowData



BOOL CLightRowset::FindBinding(DBCOUNTITEM cColumnOrdinal, DBCOUNTITEM *pcBinding)
{
	TBEGIN
	DBCOUNTITEM	cBinding;

	TESTC(m_cBindings && m_rgBindings);
	TESTC(NULL != pcBinding);

	for (cBinding = 0; cBinding < m_cBindings; cBinding++)
	{
		if (cColumnOrdinal == m_rgBindings[cBinding].iOrdinal)
			break;
	}
	
	if (cBinding < m_cBindings)
		*pcBinding = cBinding;

CLEANUP:
	TRETURN
} //CLightRowset::FindBinding



// this stores row data accordig to hRow and current accessor/bindings
HRESULT CLightRowset::GetData(HROW hRow)
{
	HRESULT				hr = E_FAIL;
	HRESULT				hrTemp;

	// if there is no current accessor and bindings, create them
	if (NULL == m_hAccessor)
	{
		ASSERT(!m_pData);
		ASSERT(0 == m_cBindings);
		ASSERT(!m_cBindings);
		ASSERT(0 == m_cRowSize);

		// if there is no current accessor and bindings, create them
		TESTC_(hrTemp = GetAccessorAndBindings(pIUnknown(), DBACCESSOR_ROWDATA, 
			&m_hAccessor, &m_rgBindings, &m_cBindings, &m_cRowSize, 
			DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS, 
			ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF, 
			NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS), 
			S_OK);

		//Alloc scratch buffer
		SAFE_ALLOC(m_pData, BYTE, m_cRowSize);
		memset(m_pData, 0, (size_t)m_cRowSize * sizeof(BYTE));
		m_fEmptyBuffer = TRUE;
		if(m_cRowSize) 
			TESTC(m_pData!=NULL)
	}
	else
	{
		ASSERT(m_pData);
		ASSERT(0 != m_cBindings);
		ASSERT(m_cBindings);
		ASSERT(0 != m_cRowSize);

		// release current content, but keep settings
		TESTC_(ReleaseInputBindingsMemory(m_cBindings, m_rgBindings,
			(BYTE*)m_pData, FALSE),S_OK);
	}

	hr = GetData(hRow, m_hAccessor, m_pData);
	
	m_fEmptyBuffer = m_fEmptyBuffer && (S_OK != hr) && (DB_E_ERRORSOCCURRED != hr) && (DB_E_ERRORSOCCURRED != hr);

CLEANUP:
	return hr;
} //CLightRowset::GetData



HRESULT CLightRowset::GetBindings(
	HACCESSOR			hAccessor,
	DBACCESSORFLAGS		*pdwAccessorFlags,
	DBCOUNTITEM			*pcBindings,
	DBBINDING			**prgBindings
)
{
	HRESULT	hr = E_FAIL;
	HRESULT		rgValidRes[]	= {	
									S_OK,
									E_INVALIDARG,
									E_OUTOFMEMORY,
									E_UNEXPECTED,
									DB_E_BADACCESSORHANDLE,
									DB_E_NOTREENTRANT,
									E_FAIL,
								};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	TESTC_(hr = CacheInterface(IID_IAccessor), S_OK);

	hr = m_pIAccessor->GetBindings(hAccessor, pdwAccessorFlags, pcBindings, prgBindings);

	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

CLEANUP:
	return hr;
} //CLightRowset::GetBindings



HRESULT CLightRowset::ReleaseRows(
	DBCOUNTITEM	cRows, 
	HROW		*rghRow, 
	DBREFCOUNT	**prgRefCounts, 
	DBROWSTATUS	**prgRowStatus
)
{
	DBREFCOUNT	*rgRefCounts	= PROVIDER_ALLOC_(cRows, DBREFCOUNT);
	DBROWSTATUS	*rgRowStatus	= PROVIDER_ALLOC_(cRows, DBROWSTATUS);
	HRESULT		hr				= ReleaseRows(cRows, rghRow, rgRefCounts, rgRowStatus);

	if(prgRefCounts)
		*prgRefCounts = rgRefCounts;
	else
		PROVIDER_FREE(rgRefCounts);  

	//RowStatus
	if(prgRowStatus)
		*prgRowStatus = rgRowStatus;
	else
		PROVIDER_FREE(rgRowStatus);  

	return hr;	
} //CLightRowset::ReleaseRows



HRESULT CLightRowset::ReleaseRows(DBCOUNTITEM cRows, HROW* rghRow, DBREFCOUNT* rgRefCounts, DBROWSTATUS* rgRowStatus)
{
	TBEGIN 
	HRESULT		hr;
	HRESULT		rgValidRes[]	= {	
									S_OK,
									DB_S_ERRORSOCCURRED,
									E_INVALIDARG,
									E_UNEXPECTED,
									DB_E_ERRORSOCCURRED,
									DB_E_NOTREENTRANT,
									E_FAIL,
								};
	DBORDINAL	cValidRes		= NUMELEM(rgValidRes);

	TESTC_(CacheInterface(IID_IRowset), S_OK);

	//ReleaseRows
	hr = m_pIRowset->ReleaseRows(cRows, rghRow, NULL, rgRefCounts, rgRowStatus);
	
	// check the returned value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	//Verify Status Array
	if(hr == S_OK && rgRowStatus)
	{
		//Since S_OK for ReleaseRows can return either DBROWSTATUS_S_OK or 
		//DBROWSTATUS_S_PENDINGCHANGES, we have to check for both
		//Instead of just calling VerifyArray()
		for(DBCOUNTITEM i=0; i<cRows; i++)
		{
			COMPARE(	DBROWSTATUS_S_OK == rgRowStatus[i] 
					||	DBROWSTATUS_S_PENDINGCHANGES == rgRowStatus[i], TRUE);
		}
	}

CLEANUP:
	return hr;	
} //CLightRowset::ReleaseRows



BOOL CLightRowset::SetDefaultAccessor(HACCESSOR hAccessor)
{
	TBEGIN
	DBACCESSORFLAGS	dwAccessorFlags;

	// release previous default accessor
	ReleaseRowData();

	if (NULL != hAccessor)
	{
		m_hAccessor = hAccessor;

		ASSERT(!m_pData);
		ASSERT(0 == m_cBindings);
		ASSERT(!m_cBindings);
		ASSERT(0 == m_cRowSize);

		TESTC_(GetBindings(hAccessor, &dwAccessorFlags, &m_cBindings, &m_rgBindings), S_OK);

		m_cRowSize = m_rgBindings[m_cBindings-1].obValue;
		if (m_rgBindings[m_cBindings-1].wType & DBTYPE_BYREF)
			m_cRowSize += sizeof(LPVOID);
		else
			m_cRowSize += m_rgBindings[m_cBindings-1].cbMaxLen;

		//Alloc scratch buffer
		SAFE_ALLOC(m_pData, BYTE, m_cRowSize);
		memset(m_pData, 0, (size_t)m_cRowSize * sizeof(BYTE));

		m_fEmptyBuffer = TRUE;
		if(m_cRowSize) 
			TESTC(m_pData!=NULL)
	}

CLEANUP:
	TRETURN
} //CLightRowset::SetDefaultAccessor

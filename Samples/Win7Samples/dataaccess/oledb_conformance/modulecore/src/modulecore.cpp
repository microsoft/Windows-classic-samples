//-----------------------------------------------------------------------------
// Microsoft Local Test Manager (LTM)
// Copyright (C) 1997 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module MODULECORE.CPP
//
//-----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "MODStandard.hpp"

#include <initguid.h>
#include "DTMGuids.hpp"

DWORD g_cLock = 0;
CSuperLog odtLog;


/////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////
#define CHECK_MEMORY(pv)				if(!pv) goto CLEANUP
#define SAFE_ALLOC(pv, type, cb)		{ pv = CoTaskMemAlloc((cb)*sizeof(type)); CHECK_MEMORY(pv);		}
#define SAFE_REALLOC(pv, type, cb)		{ pv = (type*)CoTaskMemRealloc(pv, (cb)*sizeof(type)); CHECK_MEMORY(pv);	}
#define SAFE_SYSALLOC(pv, bstr)			{ pv = SysAllocString(bstr); CHECK_MEMORY(pv);												}		

#define SAFE_FREE(pv)					{ CoTaskMemFree(pv); pv = NULL;						}
#define SAFE_SYSFREE(bstr)				{ SysFreeString(bstr); bstr = NULL;					}

//IUnknown->Release Wrapper
#define SAFE_ADDREF(pv)					if(pv) { (pv)->AddRef();							}
#define SAFE_RELEASE(pv)				if(pv) { (pv)->Release(); (pv) = NULL;				}  
#define SAFE_DELETE(pv)					if(pv) { delete pv; pv = NULL;	}  

//Error Checking (tracing)
#ifdef _TRACING
#define TRACE(string)					OutputDebugStringA(string)
#else
#define TRACE(string)
#endif //_TRACING


///////////////////////////////////////////////////////////////////////////
// CTestModuleClassFactory
//
//////////////////////////////////////////////////////////////////////////
CTestModuleClassFactory::CTestModuleClassFactory(GlobalModuleData *pTTM)
{
	m_cRef = 0;
	m_pGlobData = pTTM;
}


CTestModuleClassFactory::~CTestModuleClassFactory()
{
}


STDMETHODIMP CTestModuleClassFactory::QueryInterface(

	REFIID riid,                //@parm [in] Interface ID to query for
    LPVOID* ppv)                //@parm [out] Interface pointer for interface ID

{
	if(ppv == NULL)
		return E_INVALIDARG;
	*ppv = NULL;

	// Check if we support the interface asked for
	if(riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else if(riid == IID_IClassFactory)
		*ppv = (IClassFactory*)this;
	else
		return E_NOINTERFACE;
	
	((IUnknown *)*ppv)->AddRef();
	return S_OK;
}


STDMETHODIMP_(DWORD) CTestModuleClassFactory::AddRef()
{
	return InterlockedIncrement((LONG*)&m_cRef);
}


STDMETHODIMP_(DWORD) CTestModuleClassFactory::Release()
{
	InterlockedDecrement((LONG*)&m_cRef);
	if(m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


STDMETHODIMP CTestModuleClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,        //@parm [in] Pointer to the controlling IUNKNOWN
    REFIID riid,                //@parm [in] Interface ID for object initialization
    LPVOID * ppvObj)            //@parm [out] Interface pointer for Interface ID
{
	if(ppvObj == NULL)
		return E_INVALIDARG;
	*ppvObj = NULL;

	// Do we support the interface?
	if(pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	CThisTestModule* pCThisTestModule = new CThisTestModule(m_pGlobData);
	if(pCThisTestModule == NULL)
		return E_OUTOFMEMORY;

 	HRESULT hr = pCThisTestModule->QueryInterface(riid, ppvObj);
	if(FAILED(hr))
		delete pCThisTestModule;

	return hr;
}


STDMETHODIMP  CTestModuleClassFactory::LockServer(
    BOOL fLock)                 //@parm [in] Increment lock count(TRUE/FALSE)
{
    if (fLock)
        g_cLock++;              // TRUE increment lock count
    else
        g_cLock--;              // FALSE decrement lock count
    
    return NOERROR;
}



///////////////////////////////////////////////////////////////////////////
// CThisTestModule
//
//////////////////////////////////////////////////////////////////////////
CThisTestModule::CThisTestModule(GlobalModuleData *pgmd)
{
	m_cRef = 0;
	
	m_gmd = *pgmd;
	m_nTestCount = 0;
	m_pwszProviderName = NULL;
	m_pwszProviderFName = NULL;
	m_pwszInitString = NULL;
	m_ProviderClsid = GUID_NULL;
	m_pwszMachineName = NULL;
	m_clsctxProvider = CLSCTX_INPROC_SERVER;

	m_pError = NULL;
	m_pIProviderInfo = NULL;
	
	m_pIUnknown = NULL;
	m_pIUnknown2 = NULL;
	m_pVoid = NULL;
	m_pVoid2 = NULL;
}


CThisTestModule::~CThisTestModule()
{
	SAFE_DELETE(m_pError);
	SAFE_RELEASE(m_pIProviderInfo);

	SAFE_SYSFREE(m_pwszProviderName);
	SAFE_SYSFREE(m_pwszProviderFName);
	SAFE_SYSFREE(m_pwszInitString);
	SAFE_SYSFREE(m_pwszMachineName);
}


STDMETHODIMP CThisTestModule::QueryInterface(

	REFIID riid,
	void **ppvObject)

{
	if(ppvObject == NULL)
		return E_INVALIDARG;
	*ppvObject = NULL;

	if(riid == IID_IUnknown)
		*ppvObject = (IUnknown*)this;
	else if(riid == IID_ITestModule)
		*ppvObject = (ITestModule*)this;
	else
		return E_NOINTERFACE;
	
   ((IUnknown *)*ppvObject)->AddRef();
	return S_OK;
}

STDMETHODIMP_(DWORD) CThisTestModule::AddRef(void)
{
	return InterlockedIncrement((LONG*)&m_cRef);
}

STDMETHODIMP_(DWORD) CThisTestModule::Release(void)
{
	InterlockedDecrement((LONG*)&m_cRef);
	if(m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


STDMETHODIMP CThisTestModule::GetName(BSTR *pbstrName)
{
	TRACE("CThisTestModule::GetName\n");

	if(pbstrName == NULL)
		return E_INVALIDARG;

	*pbstrName = SysAllocString(m_gmd.m_wszModuleName);
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetDescription(BSTR *pbstrDescription)
{
	TRACE("CThisTestModule::GetDescription\n");

	if(pbstrDescription == NULL)
		return E_INVALIDARG;

	*pbstrDescription = SysAllocString(m_gmd.m_wszDescription);
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetOwnerName(BSTR *pbstrOwner)
{
	TRACE("CThisTestModule::GetOwnerName\n");

	if(pbstrOwner == NULL)
		return E_INVALIDARG;

	*pbstrOwner = SysAllocString(m_gmd.m_wszModuleOwner);
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetCLSID(BSTR* pGUID)
{
	TRACE("CThisTestModule::GetCLSID\n");

	if(pGUID == NULL)
		return E_INVALIDARG;
	
	WCHAR* pwszClsid = NULL;
	StringFromCLSID(*(m_gmd.m_pguidModuleCLSID), &pwszClsid);
	*pGUID = SysAllocString(pwszClsid);
	SAFE_FREE(pwszClsid);
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetVersion(LONG *plVer)
{
	TRACE("CThisTestModule::GetVersion\n");

	if(plVer == NULL)
		return E_INVALIDARG;
	*plVer = m_gmd.m_dwVersion;
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetProviderInterface(IProviderInfo** ppIProviderInfo)
{
	TRACE("CThisTestModule::GetProviderInterface\n");

	if(ppIProviderInfo == NULL)
		return E_INVALIDARG;

	if(m_pIProviderInfo)
		return m_pIProviderInfo->QueryInterface(IID_IProviderInfo, (void**)ppIProviderInfo);

	*ppIProviderInfo = NULL;
	return E_FAIL;
}


STDMETHODIMP CThisTestModule::SetProviderInterface(IProviderInfo *pIProviderInfo)
{
	TRACE("CThisTestModule::SetProviderInterface\n");

	HRESULT hr = S_OK;
	if(!pIProviderInfo)
		return E_INVALIDARG;
	
	SAFE_RELEASE(m_pIProviderInfo);
	SAFE_SYSFREE(m_pwszProviderName);
	SAFE_SYSFREE(m_pwszProviderFName);
	SAFE_SYSFREE(m_pwszInitString);
	SAFE_SYSFREE(m_pwszMachineName);

	//Obtain IProviderInfo from the user...
	if(FAILED(hr = pIProviderInfo->QueryInterface(IID_IProviderInfo, (void**)&m_pIProviderInfo)))
		return hr;

	//ProviderInfo
	pIProviderInfo->GetName(&m_pwszProviderName);
	pIProviderInfo->GetFriendlyName(&m_pwszProviderFName);
	pIProviderInfo->GetInitString(&m_pwszInitString);
	pIProviderInfo->GetMachineName(&m_pwszMachineName);

	//CLSID
	BSTR bstr = NULL;
	pIProviderInfo->GetCLSID(&bstr);
	CLSIDFromString(bstr, &m_ProviderClsid);
	SAFE_SYSFREE(bstr);

	//CLSCTX
	pIProviderInfo->GetCLSCTX((LONG*)&m_clsctxProvider);
	return hr;
}


STDMETHODIMP CThisTestModule::SetErrorInterface(IError *pIError)
{
	TRACE("CThisTestModule::SetErrorInterface\n");

	SAFE_DELETE(m_pError);
	m_pError = new CError;
	if(m_pError == NULL)
		return E_OUTOFMEMORY;

	//Delegate
	return m_pError->SetErrorInterface(pIError);
}


STDMETHODIMP CThisTestModule::GetErrorInterface(IError **ppIError)
{
	TRACE("CThisTestModule::GetErrorInterface\n");

	if(ppIError == NULL)
		return E_INVALIDARG;
	
	//Delegate
	if(m_pError)
		return m_pError->GetErrorInterface(ppIError);

	*ppIError = NULL;
	return E_FAIL;
}


STDMETHODIMP CThisTestModule::Init(LONG *pdwResult)
{
	TRACE("CThisTestModule::Init\n");

	LONG dwResult = m_gmd.m_pfnModuleInit(this);

	if(pdwResult)
		*pdwResult = dwResult;
	return S_OK;
}


STDMETHODIMP CThisTestModule::Terminate(VARIANT_BOOL* pbResult)
{
	TRACE("CThisTestModule::Terminate\n");

	VARIANT_BOOL bResult = (VARIANT_BOOL)m_gmd.m_pfnModuleTerminate(this);

	if(pbResult)
		*pbResult = bResult;
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetCaseCount(LONG *pc)
{
	TRACE("CThisTestModule::GetCaseCount\n");

	if(pc == NULL)
		return E_INVALIDARG;
	*pc = m_gmd.m_wTestCount;
	return S_OK;
}


STDMETHODIMP CThisTestModule::GetCase(LONG iCase, ITestCases** ppITestCases)
{
	TRACE("CThisTestModule::GetCase\n");

	//Range Checking
	if(iCase < 0 || iCase >= m_gmd.m_wTestCount || ppITestCases == NULL)
	{
		if(ppITestCases)
			*ppITestCases = NULL;
		return E_INVALIDARG;
	}

	ITestCases* pITestCases = (ITestCases*)m_gmd.m_pfnModuleGetCase(iCase + 1, this);
	if(pITestCases)
		return pITestCases->QueryInterface(IID_ITestCases, (void**)ppITestCases);
	
	*ppITestCases = NULL;
	return E_FAIL;
}





///////////////////////////////////////////////////////////////////////////
// CTestCases
//
//////////////////////////////////////////////////////////////////////////
CTestCases::CTestCases(const WCHAR* pwszTestCaseName) 
{
	m_cRef = 0;

	m_dwTestCaseNumber = 0;
	m_pwszCLSID = NULL;
	m_pwszTestCaseName = NULL;
	m_pwszTestCaseDesc = NULL;
	
	m_pThisTestModule = NULL;
	m_pError = NULL;

	m_pwszProviderName = NULL;
	m_pwszProviderFName = NULL;
	m_pwszInitString = NULL;
	m_ProviderClsid = GUID_NULL;
	m_pwszMachineName = NULL;
	m_clsctxProvider = CLSCTX_INPROC_SERVER;

	m_pIStats = NULL;
	m_pTmdSpy = NULL;
}

CTestCases::~CTestCases(void)
{
	SAFE_RELEASE(m_pThisTestModule);
	SAFE_DELETE(m_pError);
	DeleteProviderInfo();
}


HRESULT CTestCases::SetOwningMod(LONG i, CThisTestModule* pCThisTestModule)
{
	TRACE("CTestCases::SetOwningMod\n");

	SAFE_RELEASE(m_pThisTestModule);
	m_pThisTestModule = pCThisTestModule;
	SAFE_ADDREF(m_pThisTestModule);

	SAFE_DELETE(m_pError);
	m_pError = new CError;

	m_dwTestCaseNumber = i;
	return SyncProviderInterface();
}


void CTestCases::DeleteProviderInfo(void)
{
	TRACE("CTestCases::DeleteProviderInfo\n");

	SAFE_SYSFREE(m_pwszProviderName);
	SAFE_SYSFREE(m_pwszProviderFName);
	SAFE_SYSFREE(m_pwszInitString);
	SAFE_SYSFREE(m_pwszMachineName);
}	


STDMETHODIMP CTestCases::QueryInterface(

	REFIID riid,                //@parm [in] Interface ID to query for
    LPVOID* ppv)                //@parm [out] Interface pointer for interface ID

{
	if(ppv == NULL)
		return E_INVALIDARG;
	*ppv = NULL;

	// Check if we support the interface asked for
    if(riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else if(riid == IID_ITestCases)
		*ppv = (ITestCases*)this;
	else
		return E_NOINTERFACE;

	((IUnknown *)*ppv)->AddRef();
	return S_OK;
}


STDMETHODIMP_(DWORD) CTestCases::AddRef()
{
	return InterlockedIncrement((LONG*)&m_cRef);
}


STDMETHODIMP_(DWORD) CTestCases::Release()
{
	InterlockedDecrement((LONG*)&m_cRef);
	if(m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


STDMETHODIMP CTestCases::GetName(BSTR *pbstrName)
{
	TRACE("CTestCases::GetName\n");

	if(pbstrName == NULL)
		return E_INVALIDARG;

	*pbstrName = SysAllocString(GetCaseName());
	return S_OK;
}


STDMETHODIMP CTestCases::GetDescription(BSTR *pbstrDesc)
{
	TRACE("CTestCases::GetDescription\n");

	if(pbstrDesc == NULL)
		return E_INVALIDARG;

	*pbstrDesc = SysAllocString(GetCaseDesc());
	return S_OK;
}


STDMETHODIMP CTestCases::GetProviderInterface(IProviderInfo** ppIProviderInfo)
{
	TRACE("CTestCases::GetProviderInterface\n");

	if(ppIProviderInfo == NULL)
		return E_INVALIDARG;

	if(m_pThisTestModule)
		return m_pThisTestModule->GetProviderInterface(ppIProviderInfo);

	*ppIProviderInfo = NULL;
	return E_FAIL;
}

STDMETHODIMP CTestCases::GetOwningITestModule(ITestModule** ppITestModule)
{
	TRACE("CTestCases::GetOwningITestModule\n");

	if(ppITestModule == NULL)
		return E_INVALIDARG;

	if(m_pThisTestModule)
		return m_pThisTestModule->QueryInterface(IID_ITestModule, (void**)ppITestModule);

	*ppITestModule = NULL;
	return E_FAIL;
}


STDMETHODIMP CTestCases::Init(LONG *pdwResult)
{
	TRACE("CTestCases::Init\n");

	//Delegate to the derived class...
	LONG dwResult = Init();

	if(pdwResult)
		*pdwResult = dwResult;
	return S_OK;
}


STDMETHODIMP CTestCases::Terminate(VARIANT_BOOL* pbResult)
{
	TRACE("CTestCases::Terminate\n");

	//Delegate to the derived class...
	VARIANT_BOOL bResult = (VARIANT_BOOL)Terminate();
	
	if(pbResult)
		*pbResult = bResult ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


STDMETHODIMP CTestCases::GetVariationCount(LONG *pc)
{
	TRACE("CTestCases::GetVariationCount\n");

	if(pc == NULL)
		return E_INVALIDARG;
	
	*pc = GetVarCount();
	return S_OK;
}


STDMETHODIMP CTestCases::ExecuteVariation(LONG iVariation, VARIATION_STATUS *pdwResult)
{
	TRACE("CTestCases::ExecuteVariation\n");
	if(pdwResult)
		*pdwResult = eVariationStatusNonExistent;

	//Range Checking
	if(iVariation < 0 || iVariation >= (LONG)GetVarCount())
		return E_INVALIDARG;

	//Find the correct Variation funcion to execute
	VARINFO* rgVarInfo = (VARINFO*)GetVarInfoArray();
	PFNVARIATION pVarFunction = rgVarInfo[iVariation].pfnVariation;

	//TODO: (compiler file 'E:\8168\vc98\p2\src\P2\main.c', line 494)
    //Please choose the Technical Support command on the Visual C++
    //Help menu, or open the Technical Support help file for more information
	//if(!pVarFunction)
	if(pVarFunction == NULL)
		return E_FAIL;

	//Execute the Variation...
	VARIATION_STATUS dwResult = (VARIATION_STATUS)((this->*pVarFunction)());
	
	//Result
	if(pdwResult)
		*pdwResult = dwResult;

	return S_OK;
}


STDMETHODIMP CTestCases::GetVariationDesc(LONG iVariation, BSTR *pbstrDesc)
{
	TRACE("CTestCases::GetVariationDesc\n");

	//Range Checking
	if(iVariation < 0 || iVariation >= (LONG)GetVarCount() || pbstrDesc == NULL)
	{
		if(pbstrDesc)
			*pbstrDesc = NULL;
		return E_INVALIDARG;
	}

	const VARINFO* rgVarInfo = GetVarInfoArray();
	*pbstrDesc = SysAllocString(rgVarInfo[iVariation].wszDescription);
	return S_OK;
}


STDMETHODIMP CTestCases::GetVariationID(LONG iVariation, LONG *pdwID)
{
	TRACE("CTestCases::GetVariationID\n");

	//Range Checking
	if(iVariation < 0 || iVariation >= (LONG)GetVarCount() || pdwID == NULL)
	{
		if(pdwID)
			*pdwID = 0;
		return E_INVALIDARG;
	}

	const VARINFO* rgVarInfo = GetVarInfoArray();
	*pdwID = rgVarInfo[iVariation].id;
	return S_OK;
}


STDMETHODIMP CTestCases::SyncProviderInterface(void)
{
	TRACE("CTestCases::SyncProviderInterface\n");

	IProviderInfo* pIProviderInfo = NULL;
	HRESULT hr = S_OK;

	if(m_pThisTestModule == NULL)
		return E_FAIL;

	//Obtain the CProviderInfo from the TestModule
	if(FAILED(hr = m_pThisTestModule->GetProviderInterface(&pIProviderInfo)))
		return hr;

	DeleteProviderInfo();
	if(pIProviderInfo)
	{
		//ProviderInfo
		pIProviderInfo->GetName(&m_pwszProviderName);
		pIProviderInfo->GetFriendlyName(&m_pwszProviderFName);
		pIProviderInfo->GetInitString(&m_pwszInitString);
		pIProviderInfo->GetMachineName(&m_pwszMachineName);

		//CLSID
		BSTR bstr = NULL;
		pIProviderInfo->GetCLSID(&bstr);
		CLSIDFromString(bstr, &m_ProviderClsid);
		SAFE_SYSFREE(bstr);

		//CLSCTX
		pIProviderInfo->GetCLSCTX((LONG*)&m_clsctxProvider);
	}

	SAFE_RELEASE(pIProviderInfo);
	return S_OK;
}

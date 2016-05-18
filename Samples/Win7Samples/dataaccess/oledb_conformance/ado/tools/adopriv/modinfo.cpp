// ModInfo.cpp : Implementation of CADOPrivApp and DLL registration.

#include <oledb.h>

#include "stdafx.h"
#ifndef __USEPRIVLIB
#include "privcnst.h" //from ed's projects -Local
#else
#include <privlib.h>
#endif

#include "ADOPriv.h"
#include "ModInfo.h"

#ifndef __CModuleInfo_HPP_ //remove this when command is implemented

#define INSERT_COMMAND		0
#define INSERT_WITHPARAMS	1
#define INSERT_ROWSETCHANGE	2
#endif

/////////////////////////////////////////////////////////////////////////////
//

CModuleInfo::CModuleInfo() 
{
	ATLTRACE(_T("CModuleInfo::CModuleInfo()"));
	m_pParseFile = NULL;
	m_pModInfo = NULL;
	m_bstrInitString = SysAllocString(OLESTR(""));
	::SetUseIMallocSpy(FALSE); //Remove this later
	fInit();
}

CModuleInfo::~CModuleInfo() 
{	
	SysFreeString(m_bstrInitString);
	if (m_pParseFile)
	{
		LONG i= m_pParseFile->Release();
	}
	if (m_pModInfo)
		::ReleaseModInfo(NULL);
}

STDMETHODIMP CModuleInfo::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IModInfo,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CModuleInfo::ParseInitString(VARIANT_BOOL * pfResult)
{
	ASSERT(m_pModInfo != NULL);

	WCHAR*			pwszValue = NULL;
	VARIANT_BOOL	fBool = FALSE;
	
	*pfResult = (VARIANT_BOOL) m_pModInfo->ParseInitString();

	return S_OK;
}

STDMETHODIMP CModuleInfo::ParseInitFile(VARIANT_BOOL * pfResult)
{
	ASSERT(m_pModInfo != NULL);

	BOOL	fResult = TRUE;
	HRESULT	hr = E_FAIL;

	*pfResult = (VARIANT_BOOL) m_pModInfo->ParseInitFile();

	return hr;
}


STDMETHODIMP CModuleInfo::GetInitStringValue(BSTR Keyword, BSTR * Value, VARIANT_BOOL * pfResult)
{
	ASSERT(m_pModInfo != NULL);

	WCHAR*	pwszOption		= NULL;
	WCHAR*	pwszEndToken	= NULL;
	WCHAR*	pwszValue		= NULL;
	WCHAR*	pwszKeyword		= (WCHAR*)Keyword;

	*pfResult = (VARIANT_BOOL) m_pModInfo->GetInitStringValue(pwszKeyword, &pwszValue);
	if (pwszValue)
	{
		SysFreeString(*Value);
		*Value = SysAllocString(pwszValue);
	}

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_TableName(BSTR * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = SysAllocString(m_pModInfo->GetTableName());

	return S_OK;
}

STDMETHODIMP CModuleInfo::put_TableName(BSTR newVal)
{
	ASSERT(m_pModInfo != NULL);

	BOOL fResult = m_pModInfo->SetTableName((WCHAR*)newVal);

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_DefaultQuery(BSTR * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = SysAllocString(m_pModInfo->GetDefaultQuery());
	return S_OK;
}

STDMETHODIMP CModuleInfo::put_DefaultQuery(BSTR newVal)
{
	ASSERT(m_pModInfo != NULL);

	BOOL fResult = m_pModInfo->SetDefaultQuery((WCHAR *)newVal);

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_ProviderLevel(long * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = (LONG)m_pModInfo->GetProviderLevel();

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_FileName(BSTR * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = SysAllocString(m_pModInfo->GetFileName());

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_InitString(BSTR * pVal)
{
	ASSERT(m_pModInfo != NULL);

	WCHAR* pwsz = m_pModInfo->GetInitString();
	*pVal = SysAllocString(pwsz);

	return S_OK;
}

STDMETHODIMP CModuleInfo::put_InitString(BSTR newVal)
{
	ASSERT(m_pModInfo != NULL);
	BOOL fResult = m_pModInfo->SetInitString((WCHAR *)newVal);

	return S_OK;
}

STDMETHODIMP CModuleInfo::get_Insert(long * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = (LONG) m_pModInfo->GetInsert();
	return S_OK;
}

STDMETHODIMP CModuleInfo::put_Insert(long newVal)
{
	ASSERT(m_pModInfo != NULL);

	BOOL fResult = m_pModInfo->SetInsert((ULONG)newVal);

	return S_OK;
}

STDMETHODIMP CModuleInfo::IsStrictLeveling(VARIANT_BOOL * pVal)
{
	ASSERT(m_pModInfo != NULL);

	*pVal = (VARIANT_BOOL) m_pModInfo->IsStrictLeveling();

	return S_OK;
}

STDMETHODIMP CModuleInfo::Init(BOOL * pVal)
{
	ASSERT(m_pModInfo != NULL);

	HRESULT	hr = S_OK;
	VARIANT_BOOL	fResult = TRUE;

	if (m_pModInfo)
	{
		m_pParseFile = this->CreateParseInitFile();
		hr = m_pModInfo->Init(NULL); 
		m_pModInfo->ParseAll();
	}
	else
		hr = E_OUTOFMEMORY;


	return hr;
}

IParseInitFile* CModuleInfo::CreateParseInitFile()
{
	HRESULT			hr = S_OK;
	IParseInitFile*	pParseFile = NULL;
	
	hr = CoCreateInstance(CLSID_ParseInitFile, NULL, CLSCTX_INPROC_SERVER, IID_IParseInitFile,(void **)&pParseFile); 
	
	if (!FAILED(hr))
	{
		hr = pParseFile->putref_ModInfo((IModInfo*)this);
	}

	return(pParseFile);

}

STDMETHODIMP CModuleInfo::get_ParseObject(IParseInitFile * * pVal)
{
	LONG	lCount  = 0;

	if (m_pParseFile)
	{
		*pVal = m_pParseFile;
		lCount = ((IParseInitFile *)m_pParseFile)->AddRef(); 
		return(S_OK);
	}

	return S_FALSE;
}

HRESULT CModuleInfo::fInit()
{
	HRESULT	hr = S_OK;

	ATLTRACE(_T("CModuleInfo::fInit()"));

	const GUID CLSID_Error = {0xFB066086, 0xA6F8, 0x11d0,{0xA5,0xBC,0x00,0xC0,0x4F,0xC2,0xCA,0xBA}};
	if (m_pModInfo == NULL)
	{
		IError*	pIError = NULL;
		
 // 		pIError = (IError *)TlsGetValue(0);
 // 		if (pIError == NULL)
 // 		{
 // 			hr = CoCreateInstance(CLSID_Error, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&pIError);
 // 			if (FAILED(hr))
 // 				return(hr);
 // 
 // 			TlsSetValue(0, pIError);
 // 		}

		CreateModInfo(NULL);
		m_pModInfo = ::GetModInfo();
	

		if (m_pModInfo == NULL)
			hr = E_OUTOFMEMORY;
		
	}
	ATLTRACE(_T("CModuleInfo::fInit() - done"));
	return(hr);
}

CModInfo* CModuleInfo::GetModInfo()
{
	return(m_pModInfo);
}

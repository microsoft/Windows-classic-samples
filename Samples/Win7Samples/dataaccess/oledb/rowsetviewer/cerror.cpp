//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATASOURCE.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CError::CError
//
/////////////////////////////////////////////////////////////////
CError::CError(CMainWindow* pCMainWindow) 
	: CBase(eCError, pCMainWindow, NULL)
{
	//OLE DB Interfaces
	m_pIErrorInfo				= NULL;		//Error interface
	m_pIErrorRecords			= NULL;		//Error interface
}

/////////////////////////////////////////////////////////////////
// CError::~CError
//
/////////////////////////////////////////////////////////////////
CError::~CError()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CError::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CError::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IErrorInfo);
	HANDLE_GETINTERFACE(IErrorRecords);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CError::AutoRelease()
{
	//Error
	RELEASE_INTERFACE(IErrorInfo);
	RELEASE_INTERFACE(IErrorRecords);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CError::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IErrorInfo);
		OBTAIN_INTERFACE(IErrorRecords);
	}
	
	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CError::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CError::GetObjectDesc()
{
	if(!m_strObjectDesc)
	{
		CComBSTR bstr;
		if(SUCCEEDED(GetDescription(&bstr)) && bstr)
			m_strObjectDesc.CopyFrom(bstr);
	}	

	return m_strObjectDesc;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetDescription
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetDescription(BSTR* pbstrDescription)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorInfo)
	{
		//IErrorInfo::GetDescription
		XTEST(hr = m_pIErrorInfo->GetDescription(pbstrDescription));
		TRACE_METHOD(hr, L"IErrorInfo::GetDescription(&\"%s\")", pbstrDescription ? *pbstrDescription : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetSource
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetSource(BSTR* pbstrSource)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorInfo)
	{
		//IErrorInfo::GetSource
		XTEST(hr = m_pIErrorInfo->GetSource(pbstrSource));
		TRACE_METHOD(hr, L"IErrorInfo::GetSource(&\"%s\")", pbstrSource ? *pbstrSource : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetHelpFile
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetHelpFile(BSTR* pbstrHelpFile)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorInfo)
	{
		//IErrorInfo::GetHelpFile
		XTEST(hr = m_pIErrorInfo->GetHelpFile(pbstrHelpFile));
		TRACE_METHOD(hr, L"IErrorInfo::GetHelpFile(&\"%s\")", pbstrHelpFile ? *pbstrHelpFile : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetHelpContext
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetHelpContext(DWORD* pdwHelpContext)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorInfo)
	{
		//IErrorInfo::GetHelpContext
		XTEST(hr = m_pIErrorInfo->GetHelpContext(pdwHelpContext));
		TRACE_METHOD(hr, L"IErrorInfo::GetHelpContext(&0x%08x)", pdwHelpContext ? *pdwHelpContext : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetGUID
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetGUID(GUID* pGuid)
{
	HRESULT	hr = S_OK;
	WCHAR* pwszProgID = NULL;

	if(m_pIErrorInfo)
	{
		//IErrorInfo::GetGUID
		XTEST(hr = m_pIErrorInfo->GetGUID(pGuid));
		
		if(pGuid)
			pwszProgID = GetProgID(*pGuid);
		TRACE_METHOD(hr, L"IErrorInfo::GetGuid(&\"%s\")", pwszProgID);
	}

	SAFE_FREE(pwszProgID);
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetRecordCount
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetRecordCount(ULONG* pulCount)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorRecords)
	{
		//IErrorRecords::GetRecordCount
		XTEST(hr = m_pIErrorRecords->GetRecordCount(pulCount));
		TRACE_METHOD(hr, L"IErrorRecords::GetRecordCount(&%d)", pulCount ? *pulCount : NULL);
	}

	return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CError::GetCustomErrorObject
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetCustomErrorObject(ULONG ulRecordNum,	REFIID riid, IUnknown** ppObject)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorRecords)
	{
		//IErrorRecords::GetCustomErrorObject
		XTEST(hr = m_pIErrorRecords->GetCustomErrorObject(ulRecordNum, riid, ppObject));
		TRACE_METHOD(hr, L"IErrorRecords::GetCustomErrorObject(%lu, %s, &%p)", ulRecordNum, GetInterfaceName(riid), ppObject ? *ppObject : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CError::GetErrorInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CError::GetErrorInfo(ULONG ulRecordNum,	LCID lcid, IErrorInfo** ppErrorInfo)
{
	HRESULT	hr = S_OK;

	if(m_pIErrorRecords)
	{
		//IErrorRecords::GetErrorInfo
		XTEST(hr = m_pIErrorRecords->GetErrorInfo(ulRecordNum, lcid, ppErrorInfo));
		TRACE_METHOD(hr, L"IErrorRecords::GetErrorInfo(%lu, %ld, &%p)", ulRecordNum, lcid, ppErrorInfo ? *ppErrorInfo : NULL);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// CCustomError::CCustomError
//
/////////////////////////////////////////////////////////////////
CCustomError::CCustomError(CMainWindow* pCMainWindow) 
	: CBase(eCCustomError, pCMainWindow, NULL)
{
	//OLE DB Interfaces
	m_pISQLErrorInfo			= NULL;		//Error interface
}

/////////////////////////////////////////////////////////////////
// CCustomError::~CCustomError
//
/////////////////////////////////////////////////////////////////
CCustomError::~CCustomError()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CCustomError::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CCustomError::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(ISQLErrorInfo);

	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// HRESULT CCustomError::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CCustomError::AutoRelease()
{
	//Error
	RELEASE_INTERFACE(ISQLErrorInfo);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CCustomError::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CCustomError::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
	}
	
	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(ISQLErrorInfo);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CCustomError::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CCustomError::GetObjectDesc()
{
	if(!m_strObjectDesc)
	{
		CComBSTR bstr;
		LONG lNativeError = 0;

		//ISQLErrorInfo::GetSQLInfo
		if(SUCCEEDED(GetSQLInfo(&bstr, &lNativeError)) && bstr)
		{
			m_strObjectDesc.CopyFrom(bstr);
		}	
	}

	return m_strObjectDesc;
}


/////////////////////////////////////////////////////////////////
// HRESULT CCustomError::GetSQLInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CCustomError::GetSQLInfo(BSTR* pbstrSQLState, LONG* plNativeError)
{
	HRESULT	hr = S_OK;

	if(m_pISQLErrorInfo)
	{
		//ISQLErrorInfo::GetSQLInfo
		XTEST(hr = m_pISQLErrorInfo->GetSQLInfo(pbstrSQLState, plNativeError));
		TRACE_METHOD(hr, L"ISQLErrorInfo::GetSQLInfo(&\"%s\", &%ld)", pbstrSQLState ? *pbstrSQLState : NULL, plNativeError ? *plNativeError : NULL);
	}

	return hr;
}

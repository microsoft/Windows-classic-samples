//-----------------------------------------------------------------------------
// Microsoft Local Test Manager (LTM)
// Copyright (C) 1997 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module MODERROR.CPP
//
//-----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "ModError.hpp"
#include "CSuperLog.hpp"
#include "DTMGuids.hpp"


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


/////////////////////////////////////////////////////////////////////////
// CError
//
/////////////////////////////////////////////////////////////////////////
CError::CError(ERRORLEVEL eLevel)
{
	m_fTriedErrorAlready = FALSE;
	m_pIError = NULL;

	//Set the Initial Error Level...
	if(pIError())
		m_pIError->SetErrorLevel(eLevel);
}


CError::~CError()
{
	SAFE_RELEASE(m_pIError);
}

//Private:  Internal use only.
//Does not addref output pointer...
IError* CError::pIError()
{
	if(/*!m_fTriedErrorAlready &&*/ m_pIError == NULL)
	{
		//Maybe the odtLog has a set Error Object to use?
		HRESULT hr = odtLog.GetErrorInterface(&m_pIError);

		//Try to obtain ErrorObject from LTM
		if(FAILED(hr))
		{
			hr = CoCreateInstance(CLSID_LTMLITE, NULL, CLSCTX_INPROC_SERVER, IID_IError, (void **)&m_pIError);
		
			//Initialize the Error Object if needed...
			if(SUCCEEDED(hr) && m_pIError)
				m_pIError->Initialize();
		}
		m_fTriedErrorAlready = TRUE;
	}
	
	//Return this to the user...
	return m_pIError;
}


HRESULT CError::SetErrorInterface(IError* pIError)
{
	HRESULT hr = S_OK;
	if(pIError == NULL)
		return E_INVALIDARG;

	//Replace our Error Object...
	SAFE_RELEASE(m_pIError)
	if(FAILED(hr = pIError->QueryInterface(IID_IError, (void**)&m_pIError)))
		return hr;
	
	//Also replace the odtLog Error Object...
	return odtLog.SetErrorInterface(pIError);
}


HRESULT CError::GetErrorInterface(IError** ppIError)
{
	if(pIError())
		return m_pIError->QueryInterface(IID_IError, (void**)ppIError);

	return E_FAIL;
}

void CError::SetErrorLevel(ERRORLEVEL eLevel)
{
	if(pIError())
		m_pIError->SetErrorLevel(eLevel);
}


ERRORLEVEL CError::GetErrorLevel(void)
{
	ERRORLEVEL dwErrorLevel = HR_STRICT;

	if(pIError())
		m_pIError->GetErrorLevel(&dwErrorLevel);

	return dwErrorLevel;
}


HRESULT	CError::GetActualHr(void) 
{
	HRESULT hr = S_OK;

	if(pIError())
		m_pIError->GetActualHr(&hr);

	return hr;
}


BOOL CError::Validate(HRESULT hrActual, WCHAR* pwszFile, DWORD udwLine, HRESULT hrExpected)
{
	VARIANT_BOOL fResult = VARIANT_TRUE;

	if(pIError())
	{
		m_pIError->Validate(hrActual, pwszFile, udwLine, hrExpected, &fResult);
		return fResult!=VARIANT_FALSE;
	}
	
	//Otherwise from some reason we are unable to connect to the error object
	return (hrActual == hrExpected);
}


BOOL CError::Compare(BOOL fEqual, WCHAR* pwszFile, DWORD udwLine)
{
	VARIANT_BOOL fResult = VARIANT_TRUE;

	if(pIError())
		m_pIError->Compare((VARIANT_BOOL)fEqual, pwszFile, udwLine, &fResult);

	return fEqual;
}


void CError::LogExpectedHr(HRESULT hrExpected)
{
	if(pIError())
		m_pIError->LogExpectedHr(hrExpected);
}


void CError::LogReceivedHr(HRESULT ReceivedHr, WCHAR* pwszFile, DWORD udwLine)
{
	if(pIError())
		m_pIError->LogReceivedHr(ReceivedHr, pwszFile, udwLine);
}


void CError::ResetModErrors() 
{	
	if(pIError())
		m_pIError->ResetModErrors();
}


void CError::ResetCaseErrors() 
{	
	if(pIError())
		m_pIError->ResetCaseErrors();
}


void CError::ResetVarErrors() 
{	
	if(pIError())
		m_pIError->ResetVarErrors();
}


DWORD CError::GetModErrors() 
{ 
	LONG dw = 0;

	if(pIError())
		m_pIError->GetModErrors(&dw);

	return dw;
}


DWORD CError::GetCaseErrors() 
{
	LONG dw = 0;

	if(pIError())
		m_pIError->GetCaseErrors(&dw);

	return dw;
}

	
DWORD CError::GetVarErrors() 
{
	LONG dw = 0;

	if(pIError())
		m_pIError->GetVarErrors(&dw);

	return dw;
}


CError & CError::operator ++(int)
{
	if(pIError())
		m_pIError->Increment();

	return *this;
}
//-----------------------------------------------------------------------------
// Microsoft Local Test Manager (LTM)
// Copyright (C) 1997 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module CSUPERLOG.CPP
//
//-----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "CSuperLog.hpp"
#include "DTMGuids.hpp"
#include <wchar.h>


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


///////////////////////////////////////////////////////////////////////////
// CSuperLog
//
//////////////////////////////////////////////////////////////////////////
CSuperLog::CSuperLog(void)
{
	m_nTabLevel = 0;
	
	//Error Object
	//Our way of transmitting the text to LTM
	m_fTriedErrorAlready = FALSE;
	m_pIError = NULL;
}


CSuperLog::~CSuperLog()
{
	SAFE_RELEASE(m_pIError);
}

void CSuperLog::SetTabLevel(int iLevel)
{
	m_nTabLevel = iLevel;
}


CSuperLog & CSuperLog::operator << (const BYTE bVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_itow(bVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const ULONGLONG ulVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_ui64tow(ulVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const LONGLONG slVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_i64tow(slVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const unsigned long ulVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_ultow(ulVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const signed long slVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_ltow(slVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const unsigned short usVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_ultow(usVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const signed short ssVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_itow(ssVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const signed sVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_itow(sVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const unsigned uVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	_ultow(uVal, wszBuffer, 10);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (char const *szVal)
{
	WCHAR wszBuffer[256];
	size_t len = szVal ? strlen(szVal) : 0;

	if(len < 256)
	{
		//static buffer
		mbstowcs(wszBuffer, szVal, 256);
		OutputText(wszBuffer);
	}
	else
	{
		//dynamically alloc
		WCHAR* pwsz = (WCHAR*)CoTaskMemAlloc((len+1) * sizeof(WCHAR));
		mbstowcs(pwsz, szVal, len+1);
		OutputText(pwsz);
		SAFE_FREE(pwsz);
	}

	return *this;
}


CSuperLog & CSuperLog::operator << (wchar_t const * wszVal)
{
	OutputText((WCHAR*)wszVal);
	return *this;
}


CSuperLog & CSuperLog::operator << (const double rVal)
{
	WCHAR wszBuffer[50];

	//Convert to String...
	swprintf(wszBuffer, 50, L"%e", rVal);
	OutputText(wszBuffer);
	return *this;
}


CSuperLog & CSuperLog::operator << (const VARIANT vVariant)
{
	//no-op - bstr
	switch(V_VT(&vVariant))
	{
		//no-op - no need to copy data.
		case VT_BSTR:
			OutputText(V_BSTR(&vVariant));
			break;

		default:
		{
			VARIANT vVarTemp;
			VariantInit(&vVarTemp);

			//Delgate
			HRESULT hr = VariantChangeType(
				&vVarTemp,				// Destination (convert not in place)
				(VARIANT*)&vVariant,	// Source
				0,						// dwFlags
				VT_BSTR);

			if(SUCCEEDED(hr))
				OutputText(V_BSTR(&vVarTemp));
			VariantClear(&vVarTemp);
			break;
		}
	};
	
	return *this;
}


int CSuperLog::ScreenLogging(BOOL fScreen)
{
	return fScreen;
}


HRESULT CSuperLog::SetErrorInterface(IError* pIError)
{
	if(pIError == NULL)
		return E_INVALIDARG;

	SAFE_RELEASE(m_pIError)
	return pIError->QueryInterface(IID_IError, (void**)&m_pIError);
}


HRESULT CSuperLog::GetErrorInterface(IError** ppIError)
{
	if(m_pIError)
		return m_pIError->QueryInterface(IID_IError, (void**)ppIError);

	return E_FAIL;
}


HRESULT CSuperLog::OutputText(WCHAR* pwszText)
{
	if(pwszText)
	{
		if(/*!m_fTriedErrorAlready &&*/ m_pIError == NULL)
		{
			//Try to find LTM
			HRESULT hr = CoCreateInstance(CLSID_LTMLITE, NULL, CLSCTX_INPROC_SERVER, IID_IError, (void **)&m_pIError);
			if(SUCCEEDED(hr) && m_pIError)
				m_pIError->Initialize();
			m_fTriedErrorAlready = TRUE;
		}
		
		if(m_pIError == NULL)
			return E_FAIL;
		
		//Unfortunately for marshalling the string must be a "real"
		//BSTR, and not just a WCHAR*, since it will be marsalled accross boundaries...
		BSTR bstrString = NULL;
		SAFE_SYSALLOC(bstrString, pwszText);
		
		//Transmit the string using the IError::Transmit method...
		m_pIError->Transmit(bstrString);
		SAFE_SYSFREE(bstrString);		
	}
	
CLEANUP:
	return S_OK;
}

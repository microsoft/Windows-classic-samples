//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module COMMON.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
#define DBINITCONSTANTS
#define INITGUID


/////////////////////////////////////////////////////////////////////////////
// Include
//
/////////////////////////////////////////////////////////////////////////////
#include "Headers.h"
#include <errno.h>		//ERANGE



/////////////////////////////////////////////////////////////////////////////
// Externs
//
//////////////////////////////////////////////////////////////////////////////
CComPtr<IDataConvert>	g_spDataConvert;



/////////////////////////////////////////////////////////////////////////////
// IsUnicodeOS
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsUnicodeOS()
{
	static BOOL fInitialized = FALSE;
	static BOOL fUnicode = TRUE;

	//NOTE:  Don't call another other helper functions from within this function
	//that might also use the IsUnicodeOS flag, otherwise this will be an infinite loop...
	
	if(!fInitialized)
	{
		HKEY hkJunk = HKEY_CURRENT_USER;

		// Check to see if we have win95's broken registry, thus we do not have Unicode support
		if((RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && hkJunk == HKEY_CURRENT_USER)
		{
			// Try the ANSI version
			if((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && (hkJunk != HKEY_CURRENT_USER))
			{
				fUnicode = FALSE;
			}
		}

		if(hkJunk != HKEY_CURRENT_USER)
			RegCloseKey(hkJunk);
		fInitialized = TRUE;
	}

	return fUnicode;
}


/////////////////////////////////////////////////////////////////////////////
// WCHAR* StringCopy
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* StringCopy(WCHAR* pwszDest, LPCWSTR pwszSource, DBLENGTH cMaxLen)
{
	//Safe guard against overflow...
	if(cMaxLen)
	{	//copy at most cMaxLen-1 letters, 
		//pwszDest is null terminated when function returns
		wcsncpy_s(pwszDest, cMaxLen, pwszSource, cMaxLen-1);
	}

	return pwszDest;
}

/////////////////////////////////////////////////////////////////////////////
// WCHAR* StackFormat
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* StackFormat(LPCWSTR pwszFmt, ...)
{
	va_list		marker;

	ASSERT(pwszFmt);
	static WCHAR wszBuffer[MAX_QUERY_LEN];
	wszBuffer[0] = wEOL;

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, NUMELE(wszBuffer), _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[NUMELE(wszBuffer)-1] = wEOL;
	return wszBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// WCHAR* StringFormat
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* StringFormat(WCHAR* pwszDest, DBLENGTH ulStrLen, LPCWSTR pwszFmt, ...)
{
	ASSERT(pwszDest);
	ASSERT(ulStrLen);
	ASSERT(pwszFmt);
	
	va_list		marker;

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(pwszDest, ulStrLen, _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	pwszDest[ulStrLen-1] = wEOL;
	return pwszDest;
}


/////////////////////////////////////////////////////////////////////////////
// StringCompare
//
/////////////////////////////////////////////////////////////////////////////
BOOL StringCompare(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
	//Safe guard against NULL...
	if(pwsz1 && pwsz2)
		return wcscmp(pwsz1, pwsz2)==0;
	
	return pwsz1 == pwsz2;
}


/////////////////////////////////////////////////////////////////////////////
// StringCompareI
//
/////////////////////////////////////////////////////////////////////////////
BOOL StringCompareI(LPCWSTR pwsz1, LPCWSTR pwsz2)
{
	//Safe guard against NULL...
	if(pwsz1 && pwsz2)
		return _wcsicmp(pwsz1, pwsz2)==0;
	
	return pwsz1 == pwsz2;
}

/////////////////////////////////////////////////////////////////////////////
// CHAR* StringCopy
//
/////////////////////////////////////////////////////////////////////////////
CHAR* StringCopy(CHAR* pszDest, LPCSTR pszSource, DBLENGTH cMaxLen)
{
	//Safe guard against overflow...
	if(cMaxLen)
	{	//copy at most cMaxLen-1 letters, 
		//pwszDest is null terminated when function returns
		strncpy_s(pszDest, cMaxLen, pszSource, cMaxLen-1);
	}
	
	return pszDest;
}


/////////////////////////////////////////////////////////////////////////////
// ConvertToMBCS
//
/////////////////////////////////////////////////////////////////////////////
INT ConvertToMBCS(LPCWSTR pwsz, CHAR* psz, INT cchDst)
{ 
	return ConvertToMBCS(pwsz, -1, psz, cchDst);
}


/////////////////////////////////////////////////////////////////////////////
// INT ConvertToMBCS
//
/////////////////////////////////////////////////////////////////////////////
INT ConvertToMBCS(LPCWSTR pwsz, INT cchSrc, CHAR* psz, INT cchDst)
{
	//NOTE: The user can call this function with the Destination as NULL to indicate
	//just provide the length of the required conversion

	//No source
	if(!pwsz && psz)
	{
		//Conveience for the caller:
		//always make sure the result is null-terminated - if their is enough room
		if(cchDst >= 1)
		{
			psz[0] = EOL;
			return 1;
		}
		return 0;
	}

	//NOTE: WideCharToMultiByte destination buffer is really in (cb) count of bytes, 
	//but to make things simplier since MultiByteToWideChar is opposite for the output
	//buffer, we will operate everything in cch - this is not a perf hit either since we don't
	//need a conversion since for CHAR -> cch == cb
	ASSERT(1/*cch*/ == sizeof(CHAR)/*cb*/);

	//Convert the string to MBCS
	INT cchWritten = WideCharToMultiByte(CP_ACP, 0, pwsz, cchSrc/*cch*/, psz, cchDst/*cb*/, NULL, NULL);

	//Since the buffer we passed in may be smaller than the actual data returned
	//we may not actually get to copy the NULL termintor, so make sure there is one at least
	//at the end of the buffer...
	if(cchDst && (cchDst < cchSrc))
	{
		cchWritten = cchDst;
		psz[cchWritten-1] = '\0';
	}

	return cchWritten;
}


/////////////////////////////////////////////////////////////////////////////
// ConvertToMBCS
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
CHAR* ConvertToMBCS(LPCWSTR pwsz)
{
	HRESULT hr = S_OK;
	
	//no-op case
	if(!pwsz)
		return NULL;
	
	//Determine the space required for the conversion
	INT iLen = ConvertToMBCS(pwsz, -1, NULL, 0);

	//Allocate space for the string
	CHAR* pszBuffer = NULL;
	SAFE_ALLOC(pszBuffer, CHAR, iLen+1);

	//Now convert the string
	ConvertToMBCS(pwsz, pszBuffer, iLen+1);

CLEANUP:
	return pszBuffer;
}


/////////////////////////////////////////////////////////////////////////////
// ConvertToWCHAR
//
/////////////////////////////////////////////////////////////////////////////
INT ConvertToWCHAR(LPCSTR psz, WCHAR* pwsz, INT cchDst)
{
	//Delegate
	return ConvertToWCHAR(psz, -1, pwsz, cchDst);
}



/////////////////////////////////////////////////////////////////////////////
// ConvertToWCHAR
//
/////////////////////////////////////////////////////////////////////////////
INT ConvertToWCHAR(LPCSTR psz, INT cchSrc, WCHAR* pwsz, INT cchDst)
{
	//NOTE: The user can call this function with the Destination as NULL to indicate
	//just provide the length of the required conversion
	
	//No source
	if(!psz && pwsz)
	{
		//Conveience for the caller:
		//always make sure the result is null-terminated - if their is enough room
		if(cchDst >= 1)
		{
			pwsz[0] = wEOL;
			return 1;
		}
		return 0;
	}

	//NOTE: WideCharToMultiByte destination buffer is really in (cb) count of bytes, 
	//but to make things simplier since MultiByteToWideChar is opposite for the output
	//buffer, we will operate everything in cch - this is not a perf hit either since we don't
	//need a conversion since for CHAR -> cch == cb
	ASSERT(1/*cch*/ == sizeof(CHAR)/*cb*/);

	//Convert the string to MBCS
	INT cchWritten = MultiByteToWideChar(CP_ACP, 0, psz, cchSrc/*cb*/, pwsz, cchDst/*cch*/);

	//Since the buffer we passed in may be smaller than the actual data returned
	//we may not actually get to copy the NULL termintor, so make sure there is one at least
	//at the end of the buffer...
	if(cchDst && (cchDst < cchSrc))
	{
		cchWritten = cchDst;
		pwsz[cchWritten-1] = L'\0';
	}

	return cchWritten;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
WCHAR* ConvertToWCHAR(LPCSTR psz)
{
	HRESULT hr = S_OK;

	//no-op case
	if(!psz)
		return NULL;
	
	//Determine the space required for the conversion
	INT iLen = ConvertToWCHAR(psz, -1, NULL, 0);

	//Allocate space for the string
	WCHAR* pwszBuffer = NULL;
	SAFE_ALLOC(pwszBuffer, WCHAR, iLen+1);

	//Now convert the string
	ConvertToWCHAR(psz, pwszBuffer, iLen+1);

CLEANUP:
	return pwszBuffer;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT PostProcessString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT PostProcessString(WCHAR* pwsz, DBLENGTH ulMaxSize, DWORD dwFlags)
{
	ASSERT(pwsz);
	ASSERT(ulMaxSize);
	WCHAR* pwszStop = NULL;
	LONG lValue = 0;

	//Currently only deals with HEX or DECIMAL
	ASSERT(dwFlags & CONV_HEX || dwFlags & CONV_DECIMAL);

	//Convert String to integer
	if(!ConvertToLONG(pwsz, &lValue, LONG_MIN, LONG_MAX, 0/*0 base indicates WinAPI to interpret*/))
		return E_FAIL;

	//Convert Integer back to requested Format
	if(dwFlags & CONV_HEX)
		StringFormat(pwsz, ulMaxSize, L"0x%x", lValue);
	else
		StringFormat(pwsz, ulMaxSize, L"%d", lValue);

	return S_OK;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT DataConvert
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DataConvert
(
	DBSTATUS		 dbSrcStatus,		// @parm IN | source data status
	DBLENGTH		 cbSrcLength,		// @parm IN | size of source data
	DBLENGTH		 cbSrcMaxLength,	// @parm IN | max size of input buffer
	DBTYPE			 wSrcType,			// @parm IN | type of source data
	void*			 pSrcValue,			// @parm IN | ptr to source data
	BYTE			 bPrecision,		// @parm IN | precision to use for dst
	BYTE			 bScale,			// @parm IN | scale to use for dst

	DBTYPE			 wDstType,			// @parm IN | type of output data
	DBSTATUS*		 pdbDstStatus,		// @parm OUT | ptr to output data status
	DBLENGTH*		 pcbDstLength,		// @parm OUT | ptr to size of output data
	void*			 pDstValue,			// @parm OUT | ptr to output data
	DBLENGTH		 cbDstMaxLength,	// @parm IN | max size of output buffer
	DWORD			 dwConvFlags		// @parm IN | Conversion flags
)
{
	HRESULT hr = S_OK;
	if(pdbDstStatus)
		*pdbDstStatus = DBSTATUS_S_OK;
	if(pcbDstLength)
		*pcbDstLength = 0;
	

	//NOTE: cbSrcLength passed in is the actual length of the data (untruncated).
	//cbSrcMaxLength is the maximum size of the source (read) buffer thats valid.  This is useful to
	//seperate into two seperate items, since for String data this doesn't include the null terminator
	//and for fixed length it is the maximum.  But all data source is bound by cbSrcMaxLength...
	cbSrcLength = (wSrcType & DBTYPE_BYREF) ? cbSrcLength : min(cbSrcLength, cbSrcMaxLength);
	
	//STATUS...
	switch(dbSrcStatus)
	{
		case DBSTATUS_S_ISNULL:
		case DBSTATUS_S_DEFAULT:
		case DBSTATUS_S_IGNORE:
			hr = S_OK;
			goto CLEANUP;

		case DBSTATUS_S_TRUNCATED:
			hr = S_OK;
			break;

		case DBSTATUS_S_OK:
			hr = S_OK;
			break;

		default:
			hr = DB_E_CANTCONVERTVALUE;
			goto CLEANUP;
	};

	//LENGTH...

	//VALUE...
	if(!pSrcValue || !pDstValue)
		goto CLEANUP;
	
	//Can we convert this "locally" without bringing in DC?
	if(!(dwConvFlags & CONV_MSDADC_ONLY))
	{
		switch(DBTYPE_SRC_DST(wSrcType, wDstType))
		{
			case DBTYPE_SRC_DST(DBTYPE_WSTR, DBTYPE_WSTR):
			{
				//cbSrcLength - doesn't account for Null terminator
				DBLENGTH cbSrcLengthPlusTerm	= min(cbSrcLength + sizeof(WCHAR), cbSrcMaxLength);

				//VALUE
				//NOTE: We don't use wcscpy since user data could have embedded NULLs
				memcpy(pDstValue, pSrcValue, (size_t)(min(cbSrcLengthPlusTerm, cbDstMaxLength)));
				
				//Since the buffer we passed in may be smaller than the actual data returned
				//we may not actually get to copy the NULL termintor, so make sure there is one at least
				//at the end of the buffer...
				if(cbDstMaxLength && (cbDstMaxLength < cbSrcLengthPlusTerm))
					((WCHAR*)pDstValue)[cbDstMaxLength/sizeof(WCHAR)-1] = L'\0';

				//LENGTH
				//NOTE: This doesn't include the NULL terminator
				if(pcbDstLength)
					*pcbDstLength	= min(cbSrcLength, cbDstMaxLength);
				break;
			}

			case DBTYPE_SRC_DST(DBTYPE_STR, DBTYPE_WSTR):
			{
				//cbSrcLength - doesn't account for Null terminator
				DBLENGTH cbSrcLengthPlusTerm	= min(cbSrcLength + sizeof(CHAR), cbSrcMaxLength);

				//VALUE
				INT chWritten = ConvertToWCHAR((CHAR*)pSrcValue, (INT)cbSrcLengthPlusTerm/sizeof(CHAR), (WCHAR*)pDstValue, (INT)cbDstMaxLength/sizeof(WCHAR));
				
				//NOTE: LENGTH doesn't include the NULL terminator
				if(pcbDstLength && chWritten)
					*pcbDstLength	= (chWritten * sizeof(WCHAR)) - sizeof(WCHAR);
				break;
			}
					
			case DBTYPE_SRC_DST(DBTYPE_WSTR, DBTYPE_STR):
			{
				//cbSrcLength - doesn't account for Null terminator
				DBLENGTH cbSrcLengthPlusTerm	= min(cbSrcLength + sizeof(WCHAR), cbSrcMaxLength);

				//VALUE
				INT chWritten = ConvertToMBCS((WCHAR*)pSrcValue, (INT)cbSrcLengthPlusTerm/sizeof(WCHAR), (CHAR*)pDstValue, (INT)cbDstMaxLength/sizeof(CHAR));

				//NOTE: LENGTH doesn't include the NULL terminator
				if(pcbDstLength && chWritten)
					*pcbDstLength   = (chWritten * sizeof(CHAR)) - sizeof(CHAR);
				break;
			}

			case DBTYPE_SRC_DST(DBTYPE_VARIANT, DBTYPE_WSTR):
			{
				hr = VariantToString((VARIANT*)pSrcValue, (WCHAR*)pDstValue, cbDstMaxLength/sizeof(WCHAR), dwConvFlags);

				//LENGTH
				//NOTE: This doesn't include the NULL terminator
				if(pcbDstLength)
					*pcbDstLength   = wcslen((WCHAR*)pDstValue)*sizeof(WCHAR);
				break;
			}

			case DBTYPE_SRC_DST(DBTYPE_WSTR, DBTYPE_VARIANT):
			{
				VARIANT vVariant;
				V_VT(&vVariant) = VT_BSTR;
				V_BSTR(&vVariant) = SysAllocString((WCHAR*)pSrcValue);
				memcpy(pDstValue, &vVariant, sizeof(VARIANT));

				//LENGTH
				if(pcbDstLength)
					*pcbDstLength   = sizeof(VARIANT);
				break;
			}

			case DBTYPE_SRC_DST(DBTYPE_HCHAPTER, DBTYPE_WSTR):
			{
				//TODO
				IUnknown* pIUnknown = *(IUnknown**)pSrcValue;
				StringFormat((WCHAR*)pDstValue, cbDstMaxLength/sizeof(WCHAR), L"0x%p", pIUnknown);

				//LENGTH
				//NOTE: This doesn't include the NULL terminator
				if(pcbDstLength)
					*pcbDstLength   = wcslen((WCHAR*)pDstValue)*sizeof(WCHAR);
				break;
			}

			case DBTYPE_SRC_DST(DBTYPE_WSTR, DBTYPE_HCHAPTER):
			{
				//TODO - I have no clue how to update a hChapter columns?
				//But since we allow binding in the Accessor as native HCHAPTER we
				//need to support the SetData case, otherwise it will assert...

				//LENGTH
				if(pcbDstLength)
					*pcbDstLength   = sizeof(HCHAPTER);
				break;
			}

			default:
			{
				if(wSrcType & DBTYPE_VECTOR && wDstType == DBTYPE_WSTR)
				{
					hr = VectorToString((DBVECTOR*)pSrcValue, wSrcType, (WCHAR*)pDstValue, cbDstMaxLength/sizeof(WCHAR));

					//LENGTH
					if(pcbDstLength)
						*pcbDstLength   = wcslen((WCHAR*)pDstValue)*sizeof(WCHAR);
					break;
				}
				else if(wDstType & DBTYPE_VECTOR && wSrcType == DBTYPE_WSTR)
				{
					hr = StringToVector((WCHAR*)pSrcValue, wDstType, (DBVECTOR*)pDstValue);

					//LENGTH
					if(pcbDstLength)
						*pcbDstLength   = sizeof(DBVECTOR);
					break;
				}
				else if(wSrcType & DBTYPE_ARRAY && wDstType == DBTYPE_WSTR)
				{
					hr = SafeArrayToString(*(SAFEARRAY**)pSrcValue, wSrcType, (WCHAR*)pDstValue, cbDstMaxLength/sizeof(WCHAR));

					//LENGTH
					if(pcbDstLength)
						*pcbDstLength   = wcslen((WCHAR*)pDstValue)*sizeof(WCHAR);
					break;
				}
				else if(wDstType & DBTYPE_ARRAY && wSrcType == DBTYPE_WSTR)
				{
					hr = StringToSafeArray((WCHAR*)pSrcValue, wDstType, (SAFEARRAY**)pDstValue);

					//LENGTH
					if(pcbDstLength)
						*pcbDstLength   = sizeof(DBVECTOR);
					break;
				}
				else
				{
					//Unhandled Conversion?
					hr = DB_E_CANTCONVERTVALUE;
				}
			}
		};
	}	
					
	//Otherwise use the DataConversion Library...
	//Either we didn't support the conversion, or the user explcitly requested DC
	if((dwConvFlags & CONV_MSDADC_ONLY) || FAILED(hr))
	{
		//Obtain DataConversion library
		if(!g_spDataConvert)
			TESTC(hr = g_spDataConvert.CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY));

		//Unable to obtain DC or not allowed to use it...
		if(!g_spDataConvert)
			TESTC(hr = DB_E_CANTCONVERTVALUE);

		//We need to convert from the consumers type to ours...
		hr = g_spDataConvert->DataConvert(
				wSrcType,							// wSrcType
				wDstType,							// wDstType
				cbSrcLength,						// cbSrcLength
				pcbDstLength,						// pcbDstLength
				pSrcValue,							// pSrc
				pDstValue,							// pDst
				cbDstMaxLength,						// cbDstMaxLength
				dbSrcStatus,						// dbsSrcStatus
				pdbDstStatus,						// pdbsStatus
				bPrecision,							// bPrecision
				bScale,								// bScale
				0L );								// dwFlags
	}


CLEANUP:
	return hr;
}
	



///////////////////////////////////////////////////////////////
// CVariant::CVariant
//
///////////////////////////////////////////////////////////////
CVariant::CVariant()
{
	Init();
}

///////////////////////////////////////////////////////////////
// CVariant::~CVariant
//
///////////////////////////////////////////////////////////////
CVariant::~CVariant()
{
	Clear();
}

///////////////////////////////////////////////////////////////
// CVariant::Init
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::Init()
{
	return VariantInitFast(this);
}

///////////////////////////////////////////////////////////////
// CVariant::CVariant
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::Clear()
{
	return VariantClearFast(this);
}


///////////////////////////////////////////////////////////////
// CVariant::Attach
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::Attach(VARIANT* pSrc)
{
	ASSERT(pSrc);
	
	//Clear the previous value
	HRESULT hr = Clear();
	if(SUCCEEDED(hr))
	{
		memcpy(this, pSrc, sizeof(VARIANT));
		pSrc->vt = VT_EMPTY;
	}

	return hr;
}


///////////////////////////////////////////////////////////////
// CVariant::Detach
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::Detach(VARIANT* pDest)
{
	ASSERT(pDest);

	//Clear the previous value
	HRESULT hr = VariantClearFast(pDest);
	if(SUCCEEDED(hr))
	{
		memcpy(pDest, this, sizeof(VARIANT));
		vt = VT_EMPTY;
	}

	return hr;
}


///////////////////////////////////////////////////////////////
// CVariant::Copy
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::Copy(VARIANT* pSrc)
{
	ASSERT(pSrc);
	return VariantCopyFast(pSrc, this);
}


///////////////////////////////////////////////////////////////
// CVariant::ChangeType
//
///////////////////////////////////////////////////////////////
HRESULT CVariant::ChangeType(VARTYPE vt, VARIANT* pSrc)
{
	//If NULL, we are doing an inplace conversion
	if(!pSrc)
		pSrc = this;

	return VariantChangeFast(this, pSrc, ULONG_MAX, 0, vt);
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT VariantInitFast
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VariantInitFast(VARIANT* pVariant)
{
	ASSERT(pVariant);
	V_VT(pVariant) = VT_EMPTY;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT VariantClearFast
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VariantClearFast(VARIANT* pVariant)
{
	ASSERT(pVariant);
	HRESULT hr = S_OK;

	switch(V_VT(pVariant))
	{
		case VT_EMPTY:
			break;
		
		case VT_NULL:
		case VT_I8:
			V_VT(pVariant) = VT_EMPTY;
			break;

		default:
			//Nice to know if this fails, display error
			XTESTC(hr = VariantClear(pVariant));
			break;
	}

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT VariantCopyFast
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VariantCopyFast(VARIANT* pVarDest, VARIANT* pVarSrc)
{
	ASSERT(pVarDest);
	ASSERT(pVarSrc);
	HRESULT hr = S_OK;

	//First Free the Destination.
	//NOTE: This is the same symmantics as VariantCopy.  We do this always since
	//we know how to clear other types as well...
	TESTC(hr = VariantClearFast(pVarDest));

	//Now do the copy...
	switch(V_VT(pVarSrc))
	{
		case VT_EMPTY:
		case VT_NULL:
			V_VT(pVarDest) = V_VT(pVarSrc);
			break;

		case VT_I8:
			V_VT(pVarDest) = V_VT(pVarSrc);
			pVarDest->ullVal = pVarSrc->ullVal;
			break;

		default:
			//Nice to know if this fails, display error
			XTESTC(hr = VariantCopy(pVarDest, pVarSrc));
			break;
	};

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// VariantChangeFast
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	VariantChangeFast(VARIANT* pVarDest, VARIANT* pVarSrc, LCID lcid, USHORT wFlags, VARTYPE vt)
{
	HRESULT hr = S_OK;
	static LCID lcidDef = GetSystemDefaultLCID();
	
	//Delegate...
	TESTC(hr = VariantChangeTypeEx(
		pVarDest,	// Destination (convert in place)
		pVarSrc,	// Source
		lcid != ULONG_MAX ? lcid : lcidDef,		// LCID
		wFlags,		// dwFlags
		vt ));
	

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT VariantToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VariantToString(VARIANT* pVariant, WCHAR* pwsz, DBLENGTH ulMaxSize, DWORD dwFlags)
{
	//No-op
	if(!ulMaxSize)
		return S_OK;
	
	//Convert a VARIANT to a WCHAR
	ASSERT(pVariant);
	ASSERT(pwsz);
	pwsz[0] = EOL;
	
	//Find the VariantType
	DBTYPE wType = V_VT(pVariant);
	CVariant cVariant;
	HRESULT hr = S_OK;

	//VT_ARRAY is not handled by VariantChangeType
	if(wType & VT_ARRAY)
	{
		TESTC(hr = SafeArrayToString(V_ARRAY(pVariant), wType, pwsz, ulMaxSize));
		goto CLEANUP;
	}

	switch(wType)
	{
		case VT_NULL:
		case VT_EMPTY:
			//pwsz is already set to null terminator above
			break;

		case VT_BOOL:
			if(V_BOOL(pVariant) == VARIANT_TRUE)
				StringFormat(pwsz, ulMaxSize, L"%s", dwFlags & CONV_VARBOOL ? L"VARIANT_TRUE" : L"True");
			else if(V_BOOL(pVariant) == VARIANT_FALSE)
				StringFormat(pwsz, ulMaxSize, L"%s", dwFlags & CONV_VARBOOL ? L"VARIANT_FALSE" : L"False");
			else
				StringFormat(pwsz, ulMaxSize, L"%d", V_BOOL(pVariant));
			break;

		case VT_ERROR:
			StringFormat(pwsz, ulMaxSize, L"%d", V_ERROR(pVariant));
			break;

		case VT_I8:
		{
			//TODO64: OLEAUT doesn't have support for this type yet, so we have to special case...
//			StringFormat(pwsz, ulMaxSize, L"%ld", V_I8(pVariant));
			StringFormat(pwsz, ulMaxSize, L"%lld", pVariant->ullVal);
			break;
		}

		default:
		{
			TESTC(hr = VariantChangeFast(
				&cVariant,	// Destination (convert not in place)
				pVariant,	// Source
				ULONG_MAX,	// LCID
				0,			// dwFlags
				VT_BSTR ));

			StringCopy(pwsz, V_BSTR(&cVariant), ulMaxSize);

			//May need Hex postprocessing
			if(dwFlags & CONV_HEX)
			{
				switch(V_VT(pVariant))
				{
					case DBTYPE_I4:
						//"0x" + 10 characters
						if(ulMaxSize >= 12)
							StringFormat(pwsz, ulMaxSize, L"0x%x", V_I4(pVariant));
						break;
				}
			}
		}
	};

CLEANUP:
	pwsz[ulMaxSize ? ulMaxSize-1 : 0] = wEOL;
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToVariant
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToVariant(WCHAR* pwsz, VARTYPE vt, VARIANT* pVariant, DWORD dwFlags)
{
	//Convert a VARIANT to a WCHAR
	ASSERT(pVariant);
	HRESULT hr = S_OK;

	//Assign the type...
	V_VT(pVariant) = vt;

	//VT_ARRAY is not handled by VariantChangeType
	if(vt & VT_ARRAY)
	{
		SAFEARRAY* pSafeArray = NULL;
		TESTC(hr = StringToSafeArray(pwsz, vt, &pSafeArray));
		V_ARRAY(pVariant) = pSafeArray;
		goto CLEANUP;
	}

	//VariantChangeType seems to handle most types,
	//except the following cases...
	switch(vt)
	{
		case VT_NULL:
		case VT_EMPTY:
			break;

		case VT_BOOL:
		{	
			//This type requires a string
			if(!pwsz)
				TESTC(hr = E_INVALIDARG);

			if(dwFlags & CONV_VARBOOL && StringCompareI(pwsz, L"VARIANT_TRUE"))
				V_BOOL(pVariant) = VARIANT_TRUE;
			else if(dwFlags & CONV_VARBOOL && StringCompareI(pwsz, L"VARIANT_FALSE"))
				V_BOOL(pVariant) = VARIANT_FALSE;
			else if(dwFlags & CONV_ALPHABOOL && StringCompareI(pwsz, L"True"))
				V_BOOL(pVariant) = VARIANT_TRUE;
			else if(dwFlags & CONV_ALPHABOOL && StringCompareI(pwsz, L"False"))
				V_BOOL(pVariant) = VARIANT_FALSE;
			else
			{
				LONG lValue = 0;
				if(!ConvertToLONG(pwsz, &lValue, SHRT_MIN, SHRT_MAX, 0/*Base*/))
				{
					hr = E_FAIL;
					goto CLEANUP;
				}
				V_BOOL(pVariant) = (VARIANT_BOOL)lValue;
			}
			break;
		}

		case VT_I4:
		case VT_ERROR:
		case VT_UI4:
		{	
			//This type requires a string
			if(!pwsz)
				TESTC(hr = E_INVALIDARG);
			
			//We handle this case seperatly since we want to handle HEX values
			WCHAR* pwszStop = NULL;
			BOOL fSigned = (vt != VT_UI4);
			errno = 0;
			
			//Convert 
			ULONG ulValue = vt==VT_UI4 ? wcstoul(pwsz, &pwszStop, 0) : wcstol(pwsz, &pwszStop, 0);
			
			//Function stopped converting prematurely...
			if(pwszStop==NULL || pwszStop[0]!=wEOL || errno==ERANGE)
				TESTC(hr = E_FAIL);

			V_UI4(pVariant) = ulValue;
			break;
		}

		case VT_I8:
		{
			//This type requires a string
			if(!pwsz)
				TESTC(hr = E_INVALIDARG);

			//TODO64: OLEAUT doesn't have support for this type yet, so we have to special case...
			LONGLONG llValue = _wtoi64(pwsz);
//			V_I8(pVariant) = ulValue;
			pVariant->ullVal = llValue;
			break;
		}

		case VT_VARIANT:
		{
			//Place the string into the BSTR of the VARIANT
			V_VT(pVariant) = VT_BSTR;
			V_BSTR(pVariant) = SysAllocString(pwsz);
			break;
		}

		default:
		{
			//Place the string into the BSTR of the VARIANT
			V_VT(pVariant) = VT_BSTR;
			V_BSTR(pVariant) = SysAllocString(pwsz);
			
			//Now delegate to VariantChangeType...
			TESTC(hr = VariantChangeFast(
				pVariant,	// Destination (convert in place)
				pVariant,	// Source
				ULONG_MAX,	// LCID
				0,			// dwFlags
				vt ));
		}
	};

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SafeArrayToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SafeArrayToString(SAFEARRAY* pSafeArray, DBTYPE wType, WCHAR* pwszBuffer, DBLENGTH ulMaxSize)
{
	ASSERT(pSafeArray);
	ASSERT(pwszBuffer);

	//No-op
	if(!ulMaxSize)
		return S_OK;

	//This method is capable of handling N-Dimenstions of Data!!
	//We need to take N-Dimensions of Data and convert it to a string

	//For example:  We have a 3D data, where z-dim has 2 elements, y-Dim has 3 ele
	//and x-dim has 2 elements we end up with the following matrix of points
	
	//	(z, y, x)  => value	[[[
	//	(1, 1, 1)  => 1
	//	(1, 1, 2)  => 2
	//	(1, 2, 1)  => 3		][
	//	(1, 2, 2)  => 4
	//	(1, 3, 1)  => 5		][
	//	(1, 3, 2)  => 6
	//	(2, 1, 1)  => 7		]][[
	//	(2, 1, 2)  => 8
	//	(2, 2, 1)  => 9		][
	//	(2, 2, 2)  => A
	//	(2, 3, 1)  => B		][
	//	(2, 3, 2)  => C
	//						]]]

	//So what we need to generate is a string of:
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]

	//The algorythm we are using is bascially based upon a "ripple" counter.
	//We keep a counter for each dimension.  So when CounterDim[0] hits the Upper
	//Limit, we increment CounterDim[1] and set CounterDim[0] back to LowerLimit.  
	//This continues until all have reached the upper limit together:
	//{CounterDim[n-1]==UpperLimt, ... Dim[0]==UpperLimit}

	//The braces are determined by rising/falling edges of the ripple counter.
	//Everytime a dimension restarts its value from Upper->Lower we see a "][".
	//So we have a pre and a post set of - "[[[" braces "]]]" for the number of dimensions.
	//You'll notices the set of braces in the above example on the rising/falling
	//edges of the ripple counter....

	HRESULT hr = S_OK;

	CVariant cVariant;
	WCHAR* pwsz = pwszBuffer;
	WCHAR* pwszEnd = pwsz + ulMaxSize;

	ULONG i,iDim, ulInnerElements = 0;
	ULONG cDimensions = SafeArrayGetDim(pSafeArray);
	BOOL bDone = FALSE;

	//No-op
	if(!cDimensions)
		return E_FAIL;

	//Make sure there is no Array in the type
	wType &= ~VT_ARRAY;
	pwsz[0] = wEOL;

	LONG* rgIndices = NULL;
	SAFE_ALLOC(rgIndices, LONG, cDimensions);

	//Loop over all dimenstions and fill in "pre" info...
	for(iDim=0; iDim<cDimensions; iDim++)
	{
		//Fill in lower bound
		LONG lLowerBound = 0;
		SafeArrayGetLBound(pSafeArray, iDim+1, &lLowerBound);
		rgIndices[iDim] = lLowerBound;
		
		//Fill in outer dimension indicater
		*pwsz = L'[';
		pwsz++;
	}
	
	//Calculate the total number of inner items...
	//This is easy, all dimensions will have the same number "inner" items
	//IE:  rg[y][x] - all y arrays have x elements.
	//IE:  rg[z][y][x] - all z arrays, have y arrays, which have x elements
	ulInnerElements = pSafeArray->rgsabound[0].cElements;
	while(!bDone && (pwsz < pwszEnd))
	{	
		//Dimension[0] always goes through a complete cycle every time
		//Just do this part of the ripple counter seperately...
		for(i=0; i<ulInnerElements; i++)
		{
			//Initialize Variant
			cVariant.Init();
			
			//Obtain the data from the safe array...
			switch(wType)
			{
				case VT_EMPTY:
				case VT_NULL:
				case VT_I2:
				case VT_I4:
				case VT_R4:
				case VT_R8:
				case VT_CY:
				case VT_DATE:
				case VT_BSTR:
				case VT_DISPATCH:
				case VT_ERROR:
				case VT_BOOL:
				case VT_UNKNOWN:
				case VT_I1:
				case VT_UI1:
				case VT_UI2:
				case VT_UI4:
				case VT_I8:
				case VT_UI8:
				case VT_INT:
				case VT_UINT:
					//Otherwise if its a valid variant type, we can place it within
					//our variant as a subtype and delegate to VariantToString to
					//do the converion
					V_VT(&cVariant) = wType;
					TESTC(hr = SafeArrayGetElement(pSafeArray, rgIndices, &V_I4(&cVariant)));
					break;

				case VT_VARIANT:
					//just place directly into our variant.
					TESTC(hr = SafeArrayGetElement(pSafeArray, rgIndices, &cVariant));
					break;
				
				case VT_DECIMAL:
					//DECIMAL is not part of the VARIANT union
					V_VT(&cVariant)		= wType;
					TESTC(hr = SafeArrayGetElement(pSafeArray, rgIndices, &V_DECIMAL(&cVariant)));
					break;

				default:
					//Unable to handle this type...
					TESTC(hr = E_FAIL);
			}
			
			rgIndices[0]++;

			//Convert VARIANT To String
			TESTC(hr = VariantToString(&cVariant, pwsz, pwszEnd - pwsz, CONV_NONE));
			pwsz += wcslen(pwsz);
			
			//Array Seperator
			if(i<ulInnerElements-1 && (pwsz < pwszEnd))
			{
				*pwsz = L',';
				pwsz++;
			}

			//Clear the Variant, (could be outofline memory...)
			XTEST(cVariant.Clear());

			if(!(pwsz < pwszEnd))
				break;
		}

		//Adjust the other Dimensions of the ripple counter
		for(iDim=0; iDim<cDimensions; iDim++)
		{
			LONG lUpperBound = 0;
			SafeArrayGetUBound(pSafeArray, iDim+1, &lUpperBound);
			
			//Increment this ripple if below max bound, and exit out (break)
			if(rgIndices[iDim] < lUpperBound)
			{
				rgIndices[iDim]++;

				//Need to add opening braces...
				for(ULONG j=iDim; j>0 && (pwsz < pwszEnd); j--)
				{
					*pwsz = L'[';
					pwsz++;
				}
				break;
			}
			//Otherwise reset this one and move onto the 
			//next Dimension (ie: Don't break...)
			else if(iDim != cDimensions-1)
			{
				LONG lLowerBound = 0;
				SafeArrayGetLBound(pSafeArray, iDim+1, &lLowerBound);
				rgIndices[iDim] = lLowerBound;
				
				if(pwsz < pwszEnd)
				{
					*pwsz = L']';
					pwsz++;
				}
			}
			//If we have hit the last Dimension and its over the value
			//This means were done...
			else
			{
				bDone = TRUE;
			}
		}
	}

	//Display Right outer braces
	for(iDim=0; iDim<cDimensions && (pwsz < pwszEnd); iDim++)
	{
		*pwsz = L']';
		pwsz++;
	}

CLEANUP:
	SAFE_FREE(rgIndices);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToSafeArray
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToSafeArray(WCHAR* pwszBuffer, DBTYPE wType, SAFEARRAY** ppSafeArray)
{
	ASSERT(ppSafeArray);
	if(!pwszBuffer)
		return E_INVALIDARG;

	//We already know we need to create an array
	wType &= ~VT_ARRAY;

	//This method is capable of handling N-Dimenstions of Data!!
	//We need to a String of N Dimensions and turn it into a SafeArray

	//For example:  We have a 3D data, where z-dim has 2 elements, y-Dim has 3 ele
	//and x-dim has 2 elements we end up with the following matrix of points
	
	//	(z, y, x)  => value	[[[
	//	(1, 1, 1)  => 1
	//	(1, 1, 2)  => 2
	//	(1, 2, 1)  => 3		][
	//	(1, 2, 2)  => 4
	//	(1, 3, 1)  => 5		][
	//	(1, 3, 2)  => 6
	//	(2, 1, 1)  => 7		]][[
	//	(2, 1, 2)  => 8
	//	(2, 2, 1)  => 9		][
	//	(2, 2, 2)  => A
	//	(2, 3, 1)  => B		][
	//	(2, 3, 2)  => C
	//						]]]

	//So we could be passed a string of:
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]

	//The algorythm we are using is bascially based upon a "ripple" counter.
	//We keep a counter for each dimension.  So when CounterDim[0] hits the Upper
	//Limit, we increment CounterDim[1] and set CounterDim[0] back to LowerLimit.  
	//This continues until all have reached the upper limit together:
	//{CounterDim[n-1]==UpperLimt, ... Dim[0]==UpperLimit}

	//The braces are determined by rising/falling edges of the ripple counter.
	//Everytime a dimension restarts its value from Upper->Lower we see a "][".
	//So we have a pre and a post set of - "[[[" braces "]]]" for the number of dimensions.
	//You'll notices the set of braces in the above example on the rising/falling
	//edges of the ripple counter....

	HRESULT hr = S_OK;
	CVariant cVariant;

	WCHAR wszBuffer[MAX_COL_SIZE];
	wszBuffer[0] = wEOL;
	WCHAR* pwsz = pwszBuffer;

	ULONG i,iDim, ulInnerElements = 0;

	//Determine the Number of Dimensions...
	ULONG cDimensions = 0;
	while(pwsz[0]==L'[')
	{
		cDimensions++;
		pwsz++;
		wcscat_s(wszBuffer, NUMELE(wszBuffer), L"]");
	}

	//Find the End of the Data (where everever "]...") is...
	WCHAR* pwszNext = pwsz;
	WCHAR* pwszEnd = pwsz;
	WCHAR* pwszCurEnd = wcschr(pwsz, L']');

	//No-op
	if(cDimensions < 1)
		return E_FAIL;

	//Create SafeArray
	SAFEARRAY* pSafeArray = NULL;
	*ppSafeArray = NULL;

	LONG* rgIndices = NULL;
	SAFEARRAYBOUND* rgSafeArrayBounds = NULL;

	//Indices array...
	SAFE_ALLOC(rgIndices, LONG, cDimensions);
	memset(rgIndices, 0, cDimensions * sizeof(LONG));

	//SafeArrayBounds array...
	SAFE_ALLOC(rgSafeArrayBounds, SAFEARRAYBOUND, cDimensions);
	memset(rgSafeArrayBounds, 0, cDimensions*sizeof(SAFEARRAYBOUND));

	//Need to find out how many elements are in Dim[0]
	//NOTE: <= 'Allow empty string safearrays'
	while(pwszNext && pwszNext <= pwszCurEnd)
	{
		ulInnerElements++;
		rgIndices[0]++;
		pwszNext = wcschr(pwszNext, L',');
		if(pwszNext)
			pwszNext++;
	}

	//Now from the [] notation find out how many elements in the other dimenstions
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]
	
	//The algorythm we will use is:
	//Everytime we see a "]" we need to increment the next Dimension elements
	//Everytime we see a "[" we need to reset the previous Dimension elements
	
	//TODO
	//Currently this algorythm only handles 1 dimension.
	//No reason it couldn't use the above method and work for more, just on
	//a time constraint, and the only provider we have supports max 1 dim...

	ASSERT(cDimensions == 1);
	
	//Create the SafeArrayBounds
	for(iDim=0; iDim<cDimensions; iDim++)
	{
		rgSafeArrayBounds[iDim].lLbound = 0;
		rgSafeArrayBounds[iDim].cElements = rgIndices[0];
	}
	
	//Now actually create the SafeArray
	pSafeArray = SafeArrayCreate(wType, cDimensions, rgSafeArrayBounds);
	ASSERT(pSafeArray);
	
	pwszCurEnd = wcschr(pwsz, L']');
	rgIndices[0] = 0;
	pwszNext = pwsz;
	for(i=0; i<ulInnerElements; i++)
	{
		//Obtain the start and end of the value
		pwszEnd = wcschr(pwszNext, L',');
		if(pwszEnd==NULL || pwszEnd>pwszCurEnd)
			pwszEnd = pwszCurEnd;

		//Setup rgIndicaes
		rgIndices[0] = i;

		//Convert Value to a Variant
		void* pValue = NULL;
		StringCopy(wszBuffer, pwszNext, pwszEnd-pwszNext+1);
		TESTC(hr = StringToVariant(wszBuffer, wType, &cVariant, CONV_NONE));

		//Add this Value to the SafeArray
		//NOTE: According to the spec for SafeArrayPutElement
		//VT_DISPATCH, VT_UNKNOWN, and VT_BSTR are pointers, and do not require another level of indirection. 
		switch(wType)
		{
			case VT_DISPATCH:
			case VT_UNKNOWN:
			case VT_BSTR:
				pValue = V_UNKNOWN(&cVariant);
				break;
				
			case VT_VARIANT:
				pValue = &cVariant;
				break;

			case VT_DECIMAL:
				//DECIMAL is not part of the VARIANT union
				pValue = &V_DECIMAL(&cVariant);
				break;
			
			default:
				pValue = &V_I4(&cVariant);
				break;
		};

		//Add this Value to the SafeArray
		TESTC(hr = SafeArrayPutElement(pSafeArray, rgIndices, pValue));
		XTEST(cVariant.Clear());

		//Incement to next value...
		pwszNext = wcschr(pwszNext, L',');
		if(pwszNext)
			pwszNext += 1;
	}

	//Everything complete successfully...
	*ppSafeArray = pSafeArray;

CLEANUP:
	SAFE_FREE(rgIndices);
	SAFE_FREE(rgSafeArrayBounds);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT VectorToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VectorToString(DBVECTOR* pVector, DBTYPE wType, WCHAR* pwszBuffer, DBLENGTH ulMaxSize)
{
	ASSERT(pVector);
	ASSERT(pwszBuffer);
	HRESULT hr = S_OK;

	//No-op
	if(!ulMaxSize)
		return S_OK;
	
	VARIANT Variant;
	VARIANT* pVariant = NULL;
	WCHAR* pwsz = pwszBuffer;
	WCHAR* pwszEnd = pwsz + ulMaxSize;
	pwsz[0] = wEOL;

	//Make sure we are dealing with the base type...
	wType &= ~DBTYPE_VECTOR;

	//Loop over the vector...
	for(ULONG iEle=0; iEle<pVector->size; iEle++)
	{
		//Initialize Variant
		pVariant = &Variant;
		pVariant->vt = VT_EMPTY;

		//NOTE: The pVariant is really just a pointer to the data.  We don't free the data
		//since the vector data doesn't belong to us, we are just convering the given data to
		//a string.  The simplest way to do this is to dump into a variant and let our helper
		//function VariantToString deal with this...

		//Obtain the data from the vector...
		switch(wType)
		{
			case VT_EMPTY:
			case VT_NULL:
				V_VT(pVariant) = wType;
				break;
		
			case VT_I2:
			case VT_I4:
			case VT_R4:
			case VT_R8:
			case VT_CY:
			case VT_DATE:
			case VT_BSTR:
			case VT_DISPATCH:
			case VT_ERROR:
			case VT_BOOL:
			case VT_UNKNOWN:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_I8:
			case VT_UI8:
			case VT_INT:
			case VT_UINT:
			{
				DBLENGTH ulTypeSize = 0;
				if(SUCCEEDED(GetDBTypeMaxSize(wType, &ulTypeSize)))
				{
					V_VT(pVariant) = wType;
					memcpy(&V_I4(pVariant), (BYTE*)pVector->ptr + (ulTypeSize*iEle), (size_t)ulTypeSize);
				}
				break;
			}

			case VT_VARIANT:
				//just place directly into our variant.
				pVariant = (VARIANT*)((BYTE*)pVector->ptr + (sizeof(VARIANT)*iEle));
				break;

			case VT_DECIMAL:
				//DECIMAL is not part of the VARIANT union
				V_VT(pVariant)		= wType;
				V_DECIMAL(pVariant) = *(DECIMAL*)((BYTE*)pVector->ptr + (sizeof(DECIMAL)*iEle));
				break;
			
			default:
				//Unable to handle this type...
				TESTC(hr = E_FAIL);
		}

		//Convert VARIANT To String
		TESTC(hr = VariantToString(pVariant, pwsz, pwszEnd - pwsz, CONV_NONE));
		pwsz += wcslen(pwsz);

		//Vector Seperator
		if(iEle<pVector->size-1 && (pwsz < pwszEnd))
		{
			*pwsz = L',';
			pwsz++;
		}
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToVector
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToVector(WCHAR* pwszBuffer, DBTYPE wType, DBVECTOR* pVector)
{
	ASSERT(pVector);
	if(!pwszBuffer)
		return E_INVALIDARG;

	//Make sure we are dealing with the base type...
	wType &= ~DBTYPE_VECTOR;

	HRESULT hr = S_OK;
	VARIANT Variant;
	VariantInitFast(&Variant);

	WCHAR wszBuffer[MAX_COL_SIZE] = {0};
	ULONG iEle = 0;

	//Determine the Number of Elements...
	pVector->size = 1;
	WCHAR* pwsz = pwszBuffer;
	while(pwsz = wcschr(pwsz, L','))
	{
		pVector->size++;
		pwsz++;
	}

	//Obtain the size of each element
	DBLENGTH ulTypeSize = 0;
	if(FAILED(GetDBTypeMaxSize(wType, &ulTypeSize)))
		return hr;
	
	//Alloc the vector...
	SAFE_ALLOC(pVector->ptr, BYTE, ulTypeSize*pVector->size);

	pwsz = pwszBuffer;
	for(iEle=0; iEle<pVector->size; iEle++)
	{
		//Obtain the start and end of the value
		WCHAR* pwszEnd = wcschr(pwsz, L',');
		void* pElement = (BYTE*)pVector->ptr + (ulTypeSize*iEle);

		//Convert Value to a Variant
		StringCopy(wszBuffer, pwsz, pwszEnd ? pwszEnd-pwsz+1 : MAX_COL_SIZE);
		TESTC(hr = StringToVariant(wszBuffer, wType, &Variant, CONV_NONE));

		//Add this to the vector...
		switch(wType)
		{
			case VT_EMPTY:
			case VT_NULL:
				break;
		
			case VT_I2:
				*(SHORT*)pElement	= V_I2(&Variant);
				break;

			case VT_I4:
			case VT_R4:
			case VT_R8:
			case VT_CY:
			case VT_DATE:
			case VT_BSTR:
			case VT_DISPATCH:
			case VT_ERROR:
			case VT_BOOL:
			case VT_UNKNOWN:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_I8:
			case VT_UI8:
			case VT_INT:
			case VT_UINT:
				memcpy(pElement, &V_I4(&Variant), (size_t)ulTypeSize);
				break;

			case VT_VARIANT:
				//just place directly into our variant.
				*(VARIANT*)pElement = Variant;
				break;

			case VT_DECIMAL:
				*(DECIMAL*)pElement	= V_DECIMAL(&Variant);
				break;
			
			
			default:
				//Unable to handle this type...
				TESTC(hr = E_NOTIMPL);
		};

		//Incement to next value...
		pwsz = wcschr(pwsz, L',');
		if(pwsz)
			pwsz += 1;
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CHAR* strDuplicate
//
/////////////////////////////////////////////////////////////////////////////
CHAR* strDuplicate(LPCSTR psz)
{
	HRESULT hr = S_OK;

	//no-op case
	if(!psz)
		return NULL;
	
	size_t cLen	= strlen(psz);

	//Allocate space for the string
	CHAR* pszBuffer = NULL;
	SAFE_ALLOC(pszBuffer, CHAR, cLen+1);

	//Now copy the string
	strcpy_s(pszBuffer, cLen+1, psz);

CLEANUP:
	return pszBuffer;
}



/////////////////////////////////////////////////////////////////////////////
// WCHAR* wcsDuplicate
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* wcsDuplicate(LPCWSTR pwsz)
{
	HRESULT hr = S_OK;

	//no-op case
	if(!pwsz)
		return NULL;
	
	size_t cLen	= wcslen(pwsz);

	//Allocate space for the string
	WCHAR* pwszBuffer = NULL;
	SAFE_ALLOC(pwszBuffer, WCHAR, cLen+1);

	//Now copy the string
	StringCopy(pwszBuffer, pwsz, cLen+1);

CLEANUP:
	return pwszBuffer;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT DBIDToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DBIDToString(const DBID* pDBID, WCHAR* pwsz, ULONG ulMaxLen)
{
	//No-op
	if(!ulMaxLen)
		return S_OK;

	ASSERT(pwsz);
	WCHAR wszBuffer[MAX_NAME_LEN] = {0};
	pwsz[0] = wEOL;

	//No-op
	if(pDBID == NULL)
	{
		StringCopy(pwsz, L"NULL", MAX_NAME_LEN);
		return S_OK;
	}

	//ColumnID (SubItem)  DBID
	switch(pDBID->eKind)
	{
		case DBKIND_NAME:
			StringFormat(pwsz, ulMaxLen, L"{\"%s\"}", pDBID->uName.pwszName);
			break;
		
		case DBKIND_PROPID:
			StringFormat(pwsz, ulMaxLen, L"{%ld}", pDBID->uName.ulPropid);
			break;
		
		case DBKIND_GUID:
			StringFromGUID2(pDBID->uGuid.guid, pwsz, ulMaxLen);
			break;

		case DBKIND_GUID_NAME:
			StringFromGUID2(pDBID->uGuid.guid, wszBuffer, MAX_NAME_LEN);
			StringFormat(pwsz, ulMaxLen, L"{%s,\"%s\"}", wszBuffer, pDBID->uName.pwszName);
			break;
		
		case DBKIND_GUID_PROPID:
		{	
			//Special Case:  Maybe this is a defined RowCol DBID
			WCHAR* pwszColName = GetRowColName(pDBID);
			if(pwszColName)
			{
				StringCopy(pwsz, pwszColName, ulMaxLen);
			}
			else
			{
				StringFromGUID2(pDBID->uGuid.guid, wszBuffer, MAX_NAME_LEN);
				StringFormat(pwsz, ulMaxLen, L"{%s,%ld}", wszBuffer, pDBID->uName.ulPropid);
			}
			break;
		}

		case DBKIND_PGUID_NAME:
			if(pDBID->uGuid.pguid)
				StringFromGUID2(*pDBID->uGuid.pguid, wszBuffer, MAX_NAME_LEN);
			StringFormat(pwsz, ulMaxLen, L"{&%s,\"%s\"}", wszBuffer, pDBID->uName.pwszName);
			break;
		
		case DBKIND_PGUID_PROPID:
			if(pDBID->uGuid.pguid)
				StringFromGUID2(*pDBID->uGuid.pguid, wszBuffer, MAX_NAME_LEN);
			StringFormat(pwsz, ulMaxLen, L"{&%s,%ld}", wszBuffer, pDBID->uName.ulPropid);
			break;
		
		default:
			ASSERT(!"Unhandled Type!");
			break;
	};

	//Make sure the resulting string is NULL terminated
	pwsz[ulMaxLen ? ulMaxLen-1 : 0] = wEOL;
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// DBIDFree
//
/////////////////////////////////////////////////////////////////////////////
void  DBIDFree(DBID* pDBID)
{
	if(pDBID)
	{
		switch(pDBID->eKind)
		{
			case DBKIND_PGUID_NAME:
				SAFE_FREE(pDBID->uGuid.pguid);
				SAFE_FREE(pDBID->uName.pwszName);
				break;

			case DBKIND_GUID_NAME:
			case DBKIND_NAME: 
				SAFE_FREE(pDBID->uName.pwszName);
				break;
			
			case DBKIND_PGUID_PROPID:
				SAFE_FREE(pDBID->uGuid.pguid);
				break;
		};
	}
}
	

/////////////////////////////////////////////////////////////////////////////
// HRESULT DBIDCopy
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	 DBIDCopy(DBID* pDst, const DBID* pSrc)
{
	ASSERT(pDst && pSrc);
	HRESULT hr = S_OK;

	//Simple case, no out-of-line data...
	memcpy(pDst, pSrc, sizeof(DBID));
	
	//Now copy any outofline data...
	switch(pDst->eKind)
	{
		case DBKIND_PGUID_PROPID:
			pDst->uGuid.pguid = NULL;
			if(pSrc->uGuid.pguid)
			{
				SAFE_ALLOC(pDst->uGuid.pguid, GUID, 1);
				memcpy(pDst->uGuid.pguid, pSrc->uGuid.pguid, sizeof(GUID));
			}
			break;

		case DBKIND_PGUID_NAME:
			pDst->uGuid.pguid = NULL;
			if(pSrc->uGuid.pguid)
			{
				SAFE_ALLOC(pDst->uGuid.pguid, GUID, 1);
				memcpy(pDst->uGuid.pguid, pSrc->uGuid.pguid, sizeof(GUID));
			}
			pDst->uName.pwszName = wcsDuplicate(pDst->uName.pwszName);
			break;

		case DBKIND_GUID_NAME:
		case DBKIND_NAME: 
			pDst->uName.pwszName = wcsDuplicate(pDst->uName.pwszName);
			break;
	};

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT DBIDEqual
//
/////////////////////////////////////////////////////////////////////////////
BOOL DBIDEqual(const DBID* pDst, const DBID* pSrc)
{
	ASSERT(pDst && pSrc);
	BOOL bEqual = FALSE;

	if(pDst->eKind == pSrc->eKind)
	{
		//Now compare sub-parts...
		switch(pDst->eKind)
		{
			case DBKIND_PGUID_NAME:
				if(bEqual = pDst->uGuid.pguid && pSrc->uGuid.pguid ? memcmp(pDst->uGuid.pguid, pSrc->uGuid.pguid, sizeof(GUID))==0 : pDst->uGuid.pguid == pSrc->uGuid.pguid)
					bEqual = StringCompare(pDst->uName.pwszName, pSrc->uName.pwszName);
				break;
			
			case DBKIND_PGUID_PROPID:
				if(bEqual = pDst->uGuid.pguid && pSrc->uGuid.pguid ? memcmp(pDst->uGuid.pguid, pSrc->uGuid.pguid, sizeof(GUID))==0 : pDst->uGuid.pguid == pSrc->uGuid.pguid)
					bEqual = (pDst->uName.ulPropid == pSrc->uName.ulPropid);	
				break;
			
			case DBKIND_PROPID:
				bEqual = (pDst->uName.ulPropid == pSrc->uName.ulPropid);	
				break;
				
			case DBKIND_GUID_PROPID:
				bEqual = (pDst->uName.ulPropid == pSrc->uName.ulPropid);	
				break;
			
			case DBKIND_GUID:
				bEqual = memcmp(&pDst->uGuid.guid, &pSrc->uGuid.guid, sizeof(GUID))==0;
				break;

			case DBKIND_GUID_NAME:
				if(bEqual = memcmp(&pDst->uGuid.guid, &pSrc->uGuid.guid, sizeof(GUID))==0)
					bEqual = StringCompare(pDst->uName.pwszName, pSrc->uName.pwszName);
				break;

			case DBKIND_NAME: 
				bEqual = StringCompare(pDst->uName.pwszName, pSrc->uName.pwszName);
				break;

			default:
				break;
		};

	}

	//Otherwise
	return bEqual;
}



/////////////////////////////////////////////////////////////////////////////
// IsVariableType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsVariableType(DBTYPE wType)
{
	//According to OLE DB Spec Appendix A (Variable-Length Data Types)
	switch(wType) 
	{
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
		case DBTYPE_VARNUMERIC:
			return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// IsFixedType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsFixedType(DBTYPE wType)
{
	return !IsVariableType(wType);
}

/////////////////////////////////////////////////////////////////////////////
// IsNumericType
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsNumericType(DBTYPE wType)
{
	//According to OLE DB Spec Appendix A (Numeric Data Types)
	switch(wType) 
	{
		case DBTYPE_I1:
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
		case DBTYPE_UI1:
		case DBTYPE_UI2:
		case DBTYPE_UI4:
		case DBTYPE_UI8:
		case DBTYPE_R4:
		case DBTYPE_R8:
		case DBTYPE_CY:
		case DBTYPE_DECIMAL:
		case DBTYPE_NUMERIC:
			return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT GetDBTypeMaxSize
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetDBTypeMaxSize(DBTYPE wType, DBLENGTH* pulMaxSize, BYTE* pbPrecision, BYTE* pbScale)
{
	HRESULT		hr = S_OK;

	//On failure we will set the column size to the max, so callers
	//don't have to condition this for normal unknown types...
	DBLENGTH	ulMaxSize	= MAX_COL_SIZE;
	BYTE		bPrecision	= 0;
	BYTE		bScale		= 0;

	//Values taken from OLE DB Spec, Appendix A
	//In some situations we need to know ulMaxSize, Prec, Scale, as defaults or creating
	//an accessor before we actually have ColumnsInfo.  Useful info, but mainly used
	//only for defaults for dialogs...

	switch(wType)
	{
		case DBTYPE_EMPTY:
		case DBTYPE_NULL:
			ulMaxSize = 0;
			break;
		
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
		case DBTYPE_VARNUMERIC:
			ulMaxSize = MAX_COL_SIZE;
			break;

		case DBTYPE_BSTR:
			ulMaxSize = sizeof(BSTR);
			break;
		
		case DBTYPE_I1:
		case DBTYPE_UI1:
			ulMaxSize = 1;
			bPrecision = 3;
			break;

		case DBTYPE_I2:
		case DBTYPE_UI2:
			ulMaxSize = 2;
			bPrecision = 5;
			break;

		case DBTYPE_I4:
		case DBTYPE_UI4:
			ulMaxSize = 4;
			bPrecision = 10;
			break;

		case DBTYPE_I8:
			ulMaxSize = 8;
			bPrecision = 19;
			break;

		case DBTYPE_UI8:
			ulMaxSize = 8;
			bPrecision = 20;
			break;

		case DBTYPE_R4:
			ulMaxSize = sizeof(float);
			bPrecision = 7;
			break;

		case DBTYPE_R8:
			ulMaxSize = sizeof(double);
			bPrecision = 16;
			break;

		case DBTYPE_CY:
			ulMaxSize = 8;
			bPrecision = 19;
			break;

		case DBTYPE_NUMERIC:
			ulMaxSize = sizeof(DB_NUMERIC);
			bPrecision = 38;
			break;

		case DBTYPE_DATE:
			ulMaxSize = sizeof(double);
			break;

		case DBTYPE_BOOL:
			ulMaxSize = 2;
			break;

		case DBTYPE_VARIANT:
			ulMaxSize = sizeof(VARIANT);
			break;

		case DBTYPE_IDISPATCH:
		case DBTYPE_IUNKNOWN:
			ulMaxSize = sizeof(IUnknown*);
			break;

		case DBTYPE_GUID:
			ulMaxSize = sizeof(GUID);
			break;

		case DBTYPE_ERROR:
			ulMaxSize = sizeof(SCODE);
			break;

		case DBTYPE_DBDATE:
			ulMaxSize = sizeof(DBDATE);
			break;

		case DBTYPE_DBTIME:
			ulMaxSize = sizeof(DBTIME);
			break;

		case DBTYPE_DBTIMESTAMP:
			ulMaxSize = sizeof(DBTIMESTAMP);
			break;

		case DBTYPE_DECIMAL:
			ulMaxSize = sizeof(DECIMAL);
			bPrecision = 28;
			break;

		default:
			//DBTYPE_BYREF
			if(wType & DBTYPE_BYREF)
			{
				ulMaxSize = sizeof(void*);
			}			
			//DBTYPE_ARRAY
			else if(wType & DBTYPE_ARRAY)
			{
				ulMaxSize = sizeof(SAFEARRAY*);
			}			
			//DBTYPE_VECTOR
			else if(wType & DBTYPE_VECTOR)
			{
				ulMaxSize = sizeof(DBVECTOR*);
			}			
			else
			{
				//On failure we will set the column size to the max (already done above), so callers
				//don't have to condition this for normal unknown types...
				hr = E_FAIL;
			}
			break;
	};

	if(pulMaxSize)
		*pulMaxSize = ulMaxSize;
	if(pbPrecision)
		*pbPrecision = bPrecision;
	if(pbScale)
		*pbScale = bScale;
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// WCHAR* GetDBTypeName
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* GetDBTypeName(DBTYPE wType)
{
	return GetBitMapName(wType, g_cDBTypes, g_rgDBTypes, DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_BYREF | DBTYPE_RESERVED/*dwBitStart*/);
}


/////////////////////////////////////////////////////////////////////////////
// DBTYPE GetDBType
//
/////////////////////////////////////////////////////////////////////////////
DBTYPE GetDBType(WCHAR* pwsz)
{
	return (DBTYPE)GetMapName(pwsz, g_cDBTypes, g_rgDBTypes);
}


/////////////////////////////////////////////////////////////////////////////
// WCHAR* GetVariantTypeName
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* GetVariantTypeName(VARTYPE vt)
{
	return GetBitMapName(vt, g_cVariantTypes, g_rgVariantTypes, VT_VECTOR | VT_ARRAY | VT_BYREF | VT_RESERVED/*dwBitStart*/);
}


/////////////////////////////////////////////////////////////////////////////
// WCHAR* GetVariantTypeName
//
/////////////////////////////////////////////////////////////////////////////
DBTYPE GetVariantType(WCHAR* pwsz)
{
	return (DBTYPE)GetMapName(pwsz, g_cVariantTypes, g_rgVariantTypes);
}



///////////////////////////////////////////////////////////////
// FreeData
//
///////////////////////////////////////////////////////////////
HRESULT FreeData(DBTYPE wType, DBLENGTH cData, void* rgData)
{
	//no-op
	if(!rgData)
		return S_OK;

	//Loop over all elements of the array
	for(ULONG i=0; i<cData; i++)
	{
		//Free any "out-of-line" memory
		switch(wType)
		{
			case DBTYPE_BSTR:
			{
				BSTR* pbstr = &((BSTR*)rgData)[i];
				SAFE_BSTRFREE(*pbstr);
				break;
			}

			case DBTYPE_VARIANT:
			{
				VARIANT* pVariant = &((VARIANT*)rgData)[i];
				VariantClearFast(pVariant);
				break;
			}

			case DBTYPE_IDISPATCH:
			case DBTYPE_IUNKNOWN:
			{
				IUnknown* pIUnknown = ((IUnknown**)rgData)[i];
				SAFE_RELEASE(pIUnknown);
				break;
			}

			case DBTYPE_VARNUMERIC:
			case DBTYPE_PROPVARIANT:
				//TODO:
				break;

			default:
			{
				//NOTE: All the modifiers are mutually exclusive.
				
				//DBTYPE_BYREF
				if(wType & DBTYPE_BYREF)
				{
					void* pByRef	= ((void**)rgData)[i];

					//Free any outofline sub elements, (recursive)
					FreeData(wType & ~DBTYPE_BYREF, 1, pByRef);

					//Free the allocated block
					SAFE_FREE(pByRef);
				}

				//DBTYPE_VECTOR
				else if(wType & DBTYPE_VECTOR)
				{
					DBVECTOR* pVector = &((DBVECTOR*)rgData)[i];
					
					//Free any outofline sub elements, (recursive)
					FreeData(wType & ~DBTYPE_VECTOR, pVector->size, pVector->ptr);
					
					//Free the block of memory
					SAFE_FREE(pVector->ptr);
				}
				
				//DBTYPE_ARRAY
				else if(wType & DBTYPE_ARRAY)
				{
					SAFEARRAY** ppSafeArray = &((SAFEARRAY**)rgData)[i];
					if(*ppSafeArray)
						SafeArrayDestroy(*ppSafeArray);
					*ppSafeArray = NULL;
				}
				
				break;
			}
		};	
	}
	
	return S_OK;
}


///////////////////////////////////////////////////////////////
// FreeBindingData
//
///////////////////////////////////////////////////////////////
HRESULT FreeBindingData(DBCOUNTITEM cBindings, const DBBINDING* rgBindings, void* pData, BOOL fSetData)
{
	//Need to walk the array and free any other alloc memory
	for(ULONG i=0; i<cBindings; i++)
	{
		//First make sure we have valid data...
		if(!VALUE_IS_BOUND(rgBindings[i]) || !STATUS_IS_BOUND(rgBindings[i]))
			continue;
		
		//Free any "out-of-line" memory
		DBSTATUS dbStatus = BINDING_STATUS(rgBindings[i], pData);
		switch(dbStatus)
		{
			case DBSTATUS_S_OK:
			case DBSTATUS_S_TRUNCATED:
			case DBSTATUS_S_ALREADYEXISTS:
				//Always free in successful status (either Getting or Setting).
				if(rgBindings[i].dwMemOwner == DBMEMOWNER_CLIENTOWNED)
					FreeData(rgBindings[i].wType, 1, &BINDING_VALUE(rgBindings[i], pData));
				break;

			case DBSTATUS_S_ISNULL:
			case DBSTATUS_S_DEFAULT:
			case DBSTATUS_S_IGNORE:	
				//Nothing to do in either case Getting or Setting.
				//In Getting data in undefined, in Setting the value was never setup...
				break;
			
			default:
				//Otherwise data is undefined for retrieval from the provider (GetData, GetColumns, etc)
				//But for Setting (SetData, InsertRow, SetColumns, etc), we created the data
				//which has to be free'd even if the provider failed.
				
				//NOTE: For IUnknown columns the provider releases on Setting, but instead of
				//having special logic all over, we AddRef the Stream an extra time before
				//setting, so it can always be released like the rest of the data and doesn't 
				//have to be special cased...

//				if(fSetData && rgBindings[i].dwMemOwner == DBMEMOWNER_CLIENTOWNED)
//					FreeData(rgBindings[i].wType, 1, &BINDING_VALUE(rgBindings[i], pData));
				break;
		}
	}

	return S_OK;
}


///////////////////////////////////////////////////////////////
// FreeBindings
//
///////////////////////////////////////////////////////////////
HRESULT CopyBinding(DBBINDING* pDest, DBBINDING* pSource)
{
	ASSERT(pDest);
	ASSERT(pSource);
	HRESULT hr = S_OK;

	//First just memcopy the inline info
	memcpy(pDest, pSource, sizeof(DBBINDING));
	
	//Now we need to allocate any out-of-line info
	if(pSource->pObject)
	{
		SAFE_ALLOC(pDest->pObject, DBOBJECT, 1);
		memcpy(pDest->pObject, pSource->pObject, sizeof(DBOBJECT));
	}

CLEANUP:
	return hr;
}


///////////////////////////////////////////////////////////////
// FreeBindings
//
///////////////////////////////////////////////////////////////
HRESULT FreeBindings(DBCOUNTITEM* pcBindings, DBBINDING** prgBindings)
{
	ASSERT(pcBindings);
	ASSERT(prgBindings);

	//Need to walk the array and free any other alloc memory
	for(ULONG i=0; i<*pcBindings; i++)
	{
		//Free any pObjects
		if(*prgBindings)
			SAFE_FREE((*prgBindings)[i].pObject);
	}

	//Now we can free the outer struct
	*pcBindings = 0;
	SAFE_FREE(*prgBindings);
	return S_OK;
}


///////////////////////////////////////////////////////////////
// FreeColAccess
//
///////////////////////////////////////////////////////////////
HRESULT FreeColAccess(ULONG* pcColAccess, DBCOLUMNACCESS** prgColAccess, void** ppData)
{
	ASSERT(pcColAccess);
	ASSERT(prgColAccess);

	//Now we can free the outer struct
	*pcColAccess = 0;
	SAFE_FREE(*prgColAccess);
	SAFE_FREE(*ppData);
	return S_OK;
}



///////////////////////////////////////////////////////////////
// FreeConstraintDesc
//
///////////////////////////////////////////////////////////////
HRESULT FreeConstraintDesc(
	DBORDINAL*			pcConsDesc, 
	DBCONSTRAINTDESC**	prgConsDesc, 
	BOOL				fFree
)
{
	DBORDINAL			indexCol;
	DBCONSTRAINTDESC	*rgConsDesc;

	ASSERT(pcConsDesc);
	ASSERT(prgConsDesc);

	rgConsDesc = *prgConsDesc;
	for(DBORDINAL index = 0; index<*pcConsDesc; index++)
	{
		// release the current element
		DBIDFree(rgConsDesc[index].pConstraintID);
		SAFE_FREE(rgConsDesc[index].pConstraintID);

		DBIDFree(rgConsDesc[index].pReferencedTableID);
		SAFE_FREE(rgConsDesc[index].pReferencedTableID);

		SAFE_FREE(rgConsDesc[index].pwszConstraintText);

		for (indexCol = 0; indexCol < rgConsDesc[index].cColumns; indexCol++)
			DBIDFree(&rgConsDesc[index].rgColumnList[indexCol]);
		
		SAFE_FREE(rgConsDesc[index].rgColumnList);

		for (indexCol = 0; indexCol < rgConsDesc[index].cForeignKeyColumns; indexCol++)
			DBIDFree(&rgConsDesc[index].rgForeignKeyColumnList[indexCol]);
		
		SAFE_FREE(rgConsDesc[index].rgForeignKeyColumnList);
	}

	//Now we can free the outer struct
	*pcConsDesc = 0;
	if(fFree)
		SAFE_FREE(*prgConsDesc);
	return S_OK;
}



/////////////////////////////////////////////////////////////////
// WCHAR* GetColName
//
/////////////////////////////////////////////////////////////////
WCHAR* GetColName(const DBCOLUMNINFO* pColInfo)
{
	static WCHAR* pwszNullName = L"<NULL>";
	static WCHAR* pwszEmptyName = L"<Empty>";
	static WCHAR* pwszBookmarkName = L"<Bookmark>";

	//Providers are not required to return a Column Name for any column
	//Useful information, but some columns (ie: Bookmarks) and others
	//May not have a column name.  So this method will either just return
	//whatever the provider returned or will generate an appropiate one if not...
	//We really need a column name to help for the GUI headers

	if(pColInfo == NULL)
		return NULL;
	
	WCHAR* pwszColName = pColInfo->pwszName;
	if(pColInfo->iOrdinal == 0)
	{
		//Bookmark
		if(!pwszColName || !pwszColName[0])
			pwszColName = pwszBookmarkName;
	}
	else
	{
		//otherwise
		if(!pwszColName || !pwszColName[0])
		{
			//If this is a DBID defined column
			if(pColInfo->columnid.eKind == DBKIND_GUID_PROPID)
				pwszColName = GetRowColName(&pColInfo->columnid);
		}
	}

	//Handle Boundaries
	if(!pwszColName)
		pwszColName = pwszNullName;
	else if(!pwszColName[0])
		pwszColName = pwszEmptyName;

	ASSERT(pwszColName);
	return pwszColName;
}



/////////////////////////////////////////////////////////////////
// GetMaxDisplaySize
//
/////////////////////////////////////////////////////////////////
DBLENGTH GetMaxDisplaySize(DBTYPE wBindingType, DBTYPE wBackendType, DBLENGTH ulColumnSize, DBLENGTH ulMaxVarSize)
{
	//Obtain the correct number of bytes to respresent the data in the wBindingType format.
	//NOTE: For variable types, the bytes will be bounded by ulMaxVarSize (cbMaxLen), for 
	//fixed length type bindings, the size will be bounded by the type, (ie: NOT ulMaxVarSize), 
	//since the provider ignores cbMaxLen for fixed length types and assumes their is enough 
	//storage for the type...
	DBLENGTH cbMaxLen = 0;
	
	//May need to adjust the MaxLen, depending upon what the BindingType is
	switch(wBindingType)
	{
		//Strings are kind of a pain.  Although we get the luxury of 
		//Having the provider coerce the type, we need to allocate a buffer 
		//large enough for the provider to store the type in "string" format
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		{
			switch(wBackendType)
			{
				case DBTYPE_NULL:
				case DBTYPE_EMPTY:
					//Don't need much room for these...
					cbMaxLen = 0 + 1;
					break;

				case DBTYPE_I1:
				case DBTYPE_I2:
				case DBTYPE_I4:
				case DBTYPE_UI1:
				case DBTYPE_UI2:
				case DBTYPE_UI4:
				case DBTYPE_R4:
				case DBTYPE_HCHAPTER:
					//All of the above fit well into 15 characters of display size
					cbMaxLen = 15 + 1;
					break;

				case DBTYPE_I8:
				case DBTYPE_UI8:
				case DBTYPE_R8:
				case DBTYPE_CY:
				case DBTYPE_ERROR:
				case DBTYPE_BOOL:
					//All of the above fit well into 25 characters of display size
					cbMaxLen = 25 + 1;
					break;

				case DBTYPE_DECIMAL:
				case DBTYPE_NUMERIC:
				case DBTYPE_DATE:
				case DBTYPE_DBDATE:
				case DBTYPE_DBTIMESTAMP:
				case DBTYPE_GUID:
					//All of the above fit well into 50 characters of display size
					cbMaxLen = 50 + 1;
					break;
				
				case DBTYPE_BYTES:
					//Bytes -> String, 1 byte = 2 Ascii chars. (0xFF == "FF")
					cbMaxLen = min(ulColumnSize, ulMaxVarSize) * 2 + 1;
					break;

				case DBTYPE_STR:
				case DBTYPE_WSTR:
					//String -> String
					//ulColumnSize already contains the length
					cbMaxLen = min(ulColumnSize, ulMaxVarSize) + 1;
					break;

				case DBTYPE_IUNKNOWN:
				case DBTYPE_IDISPATCH:
					cbMaxLen = sizeof(IUnknown*);
					break;

				default:
 					//For everything else
					//Just default to our largest buffer size
					cbMaxLen = ulMaxVarSize;
					break;
			};

			//Adjust for Unicode strings
			if(wBindingType == DBTYPE_WSTR)
				cbMaxLen *= 2;

			//Make sure variable types are bounded by the maximum variable size,
			//since we are binding this is a string (STR/WSTR)
			cbMaxLen = min(cbMaxLen, ulMaxVarSize);	
			break;
		}
		
		default:
		{
			//Otherwise were not binding it as an inline string, so we don't need to know the 
			//total formating string length, we can simply specify the size of the type...

			//NOTE: On failure this function already sets the maxlen to the max for unrecognized types.
			GetDBTypeMaxSize(wBindingType, &cbMaxLen);

			//Make sure variable types are bounded by the maximum variable size.
			//We can't do this for fixed length types since the provider assumes this memory
			//is at least the size of the data type...
			if(IsVariableType(wBindingType))
				cbMaxLen = min(cbMaxLen, ulMaxVarSize);
			break;
		}
	};

	return cbMaxLen;
}

		
/////////////////////////////////////////////////////////////////////
// Helper Functions
//
/////////////////////////////////////////////////////////////////////
void*  SetThis(HWND hWnd, void* pThis)
{
	ASSERT(hWnd);
	void* pPrev = (void*)SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
	
	//NOTE: We need to help prevent the case of overwriting the "this" pointer
	//in a nested senario.  If you hit this assert a subwindow has subclassed the window
	//as well.  By replacing the this pointer, the previous window will lose the correct
	//class assoicted with it.
	ASSERT(!pPrev || !pThis || pPrev == pThis);

	//Return the current value...
	return pThis;
}

void*  GetThis(HWND hWnd)
{
	//Return the current value...
	ASSERT(hWnd);
	return (void*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}


//////////////////////////////////////////////////////////////////
// SyncSibling
//
//////////////////////////////////////////////////////////////////
void SyncSibling(HWND hToWnd, HWND hFromWnd)
{
	ASSERT(hToWnd && hFromWnd);

	//Make both windows synched, 
	//Get the current selection from the Source
	INDEX iItem = (INDEX)SendMessage(hFromWnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
	
	//Tell the Target to select the same selection
	if(iItem != LVM_ERR)
	{
		//Get the current selection from the Target and Unselect it
		INDEX iOldItem = (INDEX)SendMessage(hToWnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
		if(iItem != iOldItem)
		{
			//Unselect previous one
			LV_SetItemState(hToWnd, iOldItem, 0, 0, LVIS_SELECTED);

			//Select the new one
			LV_SetItemState(hToWnd, iItem, 0, LVIS_SELECTED, LVNI_SELECTED);

			//Ensure that it is visible
			SendMessage(hToWnd, LVM_ENSUREVISIBLE, (WPARAM)iItem, (LPARAM)FALSE);
		}
	}
}                



typedef struct _SynchInfo
{
	WNDPROC	wpProc;
	HWND	hWndSynch;
} SYNCHINFO;


////////////////////////////////////////////////////////////////
// SynchSubProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI SynchSubProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SYNCHINFO* pSynchInfo = (SYNCHINFO*)GetThis(hWnd);
	static BOOL fInsideSend = FALSE;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			//SubClass
			pSynchInfo = new SYNCHINFO;
			pSynchInfo->wpProc		= (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
			pSynchInfo->hWndSynch	= (HWND)lParam;
			
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)SynchSubProc);
			SetThis(hWnd, (void*)pSynchInfo);
			return 0;
		}

		case WM_DESTROY:
			SetThis(hWnd, NULL);
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pSynchInfo->wpProc); 
			SAFE_DELETE(pSynchInfo);
			break;

		case WM_VSCROLL:
			if(!fInsideSend)
			{
				//Send a synch message to the other control
				fInsideSend = TRUE;
				SendMessage(pSynchInfo->hWndSynch, message, wParam, lParam);
				fInsideSend = FALSE;
			}
			break;
	}
 
	if(pSynchInfo && pSynchInfo->wpProc)
		return CallWindowProc(pSynchInfo->wpProc, hWnd, message, wParam, lParam);
	return TRUE;
}



//////////////////////////////////////////////////////////////////
// InternalTrace
//
//////////////////////////////////////////////////////////////////
void InternalTrace(LPCWSTR pwszText)
{
	//NOTE:  We have a fixed argument version and a variable version (...), since
	//we have problems with calls to a variable length version and trying to use the '%'
	//operator as a real character and not as a format specifier...
	
	if(pwszText)
	{
		if(IsUnicodeOS())
		{
			OutputDebugStringW(pwszText);
		}
		else
		{
			//Output to the DebugWindow
			CHAR szBuffer[MAX_QUERY_LEN];
			ConvertToMBCS(pwszText, szBuffer, MAX_QUERY_LEN);
			OutputDebugStringA(szBuffer);
		}
	}
}


//////////////////////////////////////////////////////////////////
// InternalTraceFmt
//
//////////////////////////////////////////////////////////////////
void InternalTraceFmt(LPCWSTR	pwszFmt, ...)
{
	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[MAX_QUERY_LEN-1] = wEOL;
	
	//Delegate
	InternalTrace(wszBuffer);
}


//////////////////////////////////////////////////////////////////
// CenterDialog
//
//////////////////////////////////////////////////////////////////
BOOL CenterDialog(HWND hdlg)
{
	RECT  rcParent;                         // Parent window client rect
	RECT  rcDlg;                            // Dialog window rect
	int   nLeft, nTop;                      // Top-left coordinates
	int   cWidth, cHeight;                  // Width and height
	HWND	hwnd;

	// Get frame window client rect in screen coordinates
	hwnd = GetParent(hdlg);
	if(hwnd == NULL || hwnd == GetDesktopWindow()) 
	{
		rcParent.top = rcParent.left = 0;
		rcParent.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rcParent.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	}
	else 
	{
		GetWindowRect(hwnd, &rcParent);
	}

	// Determine the top-left point for the dialog to be centered
	GetWindowRect(hdlg, &rcDlg);
	cWidth  = rcDlg.right  - rcDlg.left;
	cHeight = rcDlg.bottom - rcDlg.top;
	nLeft   = rcParent.left + 
            (((rcParent.right  - rcParent.left) - cWidth ) / 2);
	nTop    = rcParent.top  +
            (((rcParent.bottom - rcParent.top ) - cHeight) / 2);
	if (nLeft < 0) nLeft = 0;
	if (nTop  < 0) nTop  = 0;

	// Place the dialog
	return MoveWindow(hdlg, nLeft, nTop, cWidth, cHeight, TRUE);
}


//////////////////////////////////////////////////////////////////
// MoveWindow
//
//////////////////////////////////////////////////////////////////
BOOL MoveWindow(HWND hWnd, ULONG x, ULONG y)
{
	RECT  rcParent;                         // Parent window client rect
	RECT  rcDlg;                            // Dialog window rect
	int   nLeft, nTop;                      // Top-left coordinates
	int   cWidth, cHeight;                  // Width and height
	HWND  hWndParent;

	// Get frame window client rect in screen coordinates
	hWndParent = GetParent(hWnd);
	if(hWndParent == NULL || hWndParent == GetDesktopWindow()) 
	{
		rcParent.top = rcParent.left = 0;
		rcParent.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rcParent.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	}
	else 
		GetWindowRect(hWndParent, &rcParent);

	// Determine the top-left point for the dialog to be centered
	GetWindowRect(hWnd, &rcDlg);
	cWidth  = rcDlg.right  - rcDlg.left;
	cHeight = rcDlg.bottom - rcDlg.top;
	nLeft   = rcParent.left + x;
	nTop    = rcParent.top + y;

	// Place the dialog
	return MoveWindow(hWnd, nLeft, nTop, cWidth, cHeight, TRUE);
}


//////////////////////////////////////////////////////////////////
// GetWindowPos
//
//////////////////////////////////////////////////////////////////
BOOL GetWindowPos(HWND hWnd, POINTS* pPTS)
{
	ASSERT(pPTS);

	RECT rect;
	if(GetWindowRect(hWnd, &rect))
	{
		pPTS->x = (SHORT)rect.left;
		pPTS->y = (SHORT)rect.top;
	}
	
	return FALSE;
}


//////////////////////////////////////////////////////////////////
// GetWindowSize
//
//////////////////////////////////////////////////////////////////
SIZE GetWindowSize(HWND hWnd)
{
	RECT rect = { 0,0,0,0 };
	SIZE size = { 0,0 };

	//Obtain window cordinates.
	if(GetWindowRect(hWnd, &rect))
	{
		//Fillin SIZE struct
		size.cx = rect.right - rect.left;
		size.cy = rect.bottom - rect.top;
	}
	return size;
}


//////////////////////////////////////////////////////////////////
// GetClientSize
//
//////////////////////////////////////////////////////////////////
SIZE GetClientSize(HWND hWnd)
{
	RECT rect = { 0,0,0,0 };
	SIZE size = { 0,0 };

	//Obtain window cordinates.
	if(GetClientRect(hWnd, &rect))
	{
		//Fillin SIZE struct
		size.cx = rect.right - rect.left;
		size.cy = rect.bottom - rect.top;
	}
	return size;
}


//////////////////////////////////////////////////////////////////
// GetClientCoords
//
//////////////////////////////////////////////////////////////////
RECT GetClientCoords(HWND hWndParent, HWND hWnd)
{
	//Need the Boundary Rectangle in Client Coordinates, with
	//respect to the parent window...
	RECT rect, rectClientCoords;
	GetWindowRect(hWnd, &rect);
	
	POINT ptTopLeft = { rect.left, rect.top };
	ScreenToClient(hWndParent, &ptTopLeft);
	POINT ptBottomRight = { rect.right, rect.bottom };
	ScreenToClient(hWndParent, &ptBottomRight);
	
	rectClientCoords.top		= ptTopLeft.y;
	rectClientCoords.left		= ptTopLeft.x;
	rectClientCoords.bottom		= ptBottomRight.y;
	rectClientCoords.right		= ptBottomRight.x;
	return rectClientCoords;
}

//////////////////////////////////////////////////////////////////
// wMessageBox
//
//////////////////////////////////////////////////////////////////
INT wMessageBox(
	HWND hwnd,							// Parent window for message display
	UINT uiStyle,						// Style of message box
	WCHAR* pwszTitle,					// Title for message
	WCHAR* pwszFmt,						// Format string
	...									// Substitution parameters
	)
{
	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
	va_end(marker);
   
	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[MAX_QUERY_LEN-1] = wEOL;

	//Unicode version is supported on both Win95/WinNT
	return MessageBoxW(hwnd, wszBuffer, pwszTitle, uiStyle);
}


//////////////////////////////////////////////////////////////////
// wSendMessage
//
//////////////////////////////////////////////////////////////////
LRESULT wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszText)
{
	LRESULT lResult = 0;
	HRESULT hr = S_OK;

	if(IsUnicodeOS())
	{
		//WinNT - Unicode, no need for conversions...
		lResult = SendMessageW(hWnd, Msg, (WPARAM)wParam, (LPARAM)pwszText);
	}
	else
	{
		//Win95 - Unicode not supported...
		CHAR szBuffer[MAX_QUERY_LEN] = { 0, 0, 0, 0 };
		ULONG ulDestSize = MAX_QUERY_LEN;
		CHAR* psz = szBuffer;

		//All messages that have [input] strings
		switch(Msg)
		{
			case EM_GETLINE:
				((WORD*)psz)[0] = MAX_QUERY_LEN;
				break;

			case WM_GETTEXT:
	 			//We may need to dynamically allocate this...
				if(wParam > MAX_QUERY_LEN)
					SAFE_ALLOC(psz, CHAR, wParam);
				break;
					
			case EM_GETSELTEXT:
				ASSERT(wParam && "wSendMessage EM_GETSELTEXT requires a length so the conversion buffer so it doesn't overflow...");
	 			ulDestSize = (ULONG)wParam;
					
				//We may need to dynamically allocate this...
				if(wParam > MAX_QUERY_LEN)
					SAFE_ALLOC(psz, CHAR, wParam);
				wParam = 0;
				break;

			case SB_SETTEXTW:
				Msg = SB_SETTEXT;

			case WM_SETTEXT:
			case EM_REPLACESEL:
			case CB_FINDSTRINGEXACT:
			case CB_FINDSTRING:
			case CB_ADDSTRING:
			case LB_ADDSTRING:
			case SB_SETTEXTA:
				//Convert to ANSI before sending
	 			//We may need to dynamically allocate this...
				if(!ConvertToMBCS(pwszText, psz, MAX_QUERY_LEN))
					psz = ConvertToMBCS(pwszText);
				break;
		};

		//Send the message with an ANSI Buffer 
		lResult = SendMessageA(hWnd, Msg, (WPARAM)wParam, (LPARAM)psz);
		
		//All messages that have [output] messages (return strings)
		switch(Msg)
		{
			case WM_GETTEXT:
				//Now convert the result into the users WCHAR buffer
				//wParam indicates the max length
				ConvertToWCHAR(psz, pwszText, (INT)wParam);
				break;

			case EM_GETLINE:
				//Now convert the result into the users WCHAR buffer
				ASSERT(pwszText);
				ConvertToWCHAR(psz, pwszText, ((WORD*)pwszText)[0]);
				break;

			case CB_GETLBTEXT:
			case EM_GETSELTEXT:
			case LB_GETTEXT:
				//Now convert the result into the users WCHAR buffer
				ConvertToWCHAR(psz, pwszText, ulDestSize);
				break;
		};

		//Free any dynamically allocated memory
		if(psz != szBuffer)
			SAFE_FREE(psz);
	}
		
CLEANUP:
	return lResult;
}


//////////////////////////////////////////////////////////////////
// wSendMessageFmt
//
//////////////////////////////////////////////////////////////////
LRESULT wSendMessageFmt(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszFmt, ...)
{
	ASSERT(pwszFmt);
	ASSERT(wParam != WM_GETTEXT);
	
	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[MAX_QUERY_LEN-1] = wEOL;

	//Delegate
	return wSendMessage(hWnd, Msg, wParam, wszBuffer);
}


//////////////////////////////////////////////////////////////////
// WCHAR wGetWindowText
//
//////////////////////////////////////////////////////////////////
WCHAR* wGetWindowText(HWND hWnd)
{
	//Dynamic Memory Version of the the WinAPI GetWindowText
	WCHAR* pwsz = NULL;
	HRESULT hr = S_OK;

	//First obtain the length of the string
	LRESULT iLength = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	if(iLength>0)
	{
		LRESULT iActLength = 0;

		//Allocate our string
		SAFE_ALLOC(pwsz, WCHAR, iLength+1);
		
		//Now get the text from the window...
		iActLength = wSendMessage(hWnd, WM_GETTEXT, iLength+1, pwsz);
		
		//Ensure NULL terminated...
		pwsz[iActLength>0 ? min(iActLength, iLength) : 0] = wEOL;
	}

CLEANUP:
	return pwsz;
}


//////////////////////////////////////////////////////////////////
// ConvertToLONG
//
//////////////////////////////////////////////////////////////////
BOOL ConvertToLONG(LPCWSTR pwszText, LONG* plValue, LONG lMin, LONG lMax, INT iBase)
{
	ASSERT(plValue);
	if(!pwszText || !pwszText[0])
		return FALSE;

	errno = 0;
	WCHAR*	pwszEnd = NULL;
			
	//Convert to LONG
	LONG lValue = wcstol(pwszText, &pwszEnd, iBase);
	if(errno==ERANGE || !pwszText[0] || lValue<lMin || lValue>lMax || pwszEnd==NULL || pwszEnd[0]!=wEOL) 
		return FALSE;

	//Only change the input value on success...
	*plValue = lValue;
	return TRUE;
}


//////////////////////////////////////////////////////////////////
// GetEditBoxValue
//
//////////////////////////////////////////////////////////////////
BOOL GetEditBoxValue(HWND hEditWnd, LONG* plValue, LONG lMin, LONG lMax, BOOL fAllowEmpty)
{
	ASSERT(hEditWnd);
	ASSERT(plValue);
	WCHAR	wszBuffer[MAX_QUERY_LEN] = {0};
	
	//Get the EditText
	wSendMessage(hEditWnd, WM_GETTEXT, MAX_QUERY_LEN, wszBuffer);
	if(wszBuffer[0] || !fAllowEmpty)
	{
		//Delegate
		if(!wszBuffer[0] || !ConvertToLONG(wszBuffer, plValue, lMin, lMax))
		{
			wMessageBox(hEditWnd, MB_TASKMODAL | MB_ICONERROR | MB_OK,  wsz_ERROR, 
				wsz_INVALID_VALUE_, wszBuffer, lMin, lMax);
			SetFocus(hEditWnd);
			return FALSE;
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////
// SetMenuData
//
/////////////////////////////////////////////////////////////////
BOOL SetMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition, ULONG_PTR dwItemData)
{
	//Setup our Menu Structure...
	//NOTE: HBITMAP was added in IE5, and may not work on older controls...
	MENUITEMINFOA menuinfo = { offsetof(MENUITEMINFOA, cch) + sizeof(UINT), MIIM_DATA, 0, 0, 0, NULL, NULL, NULL, dwItemData, NULL, 0};

#ifndef _WIN64
	//Set this menu's item data
	//NOTE: HBITMAP was added in IE5, and may not work on older controls...
	//So we will first try with the smaller size, since this should be backward compatible...
	if(SetMenuItemInfoA(hMenu, uItem, fByPosition, &menuinfo))
		return TRUE;
#endif //_WIN64
	
	//Retry Logic
	//On some OS's, (Win64), it only assumes the full NT5 structure size
	//thus for a new OS doesn't have backward compatability issues...
	menuinfo.cbSize = sizeof(MENUITEMINFOA);
	return SetMenuItemInfoA(hMenu, uItem, fByPosition, &menuinfo);
}


/////////////////////////////////////////////////////////////////////////////
// SetSubMenuData
//
/////////////////////////////////////////////////////////////////////////////
BOOL SetSubMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition, ULONG_PTR dwItemData)
{
	//NOTE: Having every menu item in all objects menu have unique ids, makes adding the same interface
	//to another object very painful, and causes manually editing the menu resources, making sure its
	//added to OnUpdateCommand for enabling/disabling the menu item, as well as adding another
	//case in OnCommand for the same interface! 

	//Instead we have devised a design where we can intitally set the item data of every menu
	//to contain the object identifier.  This way we can always obtain the stored data of the menu
	//and instantly know the type of object this interface/method is called from.  Saves hundreads 
	//of lines of code, and almost ellimiates all places to remember to add code to...

	//First set this menu's item data
	if(SetMenuData(hMenu, 0, TRUE/*fByPosition*/, dwItemData))
	{
		//Now that we have successfully set the data, we should loop through any child 
		//menus and also set that data, since there is no current way to traverse backward in a menu...
		INT iItems = GetMenuItemCount(hMenu);
		for(INT i=0; i<iItems; i++)
		{
			ULONG ulMenuID = GetMenuItemID(hMenu, i);
			if(ulMenuID == ULONG_MAX)
			{
				//Recursive Alogorytm
				HMENU hSubMenu = GetSubMenu(hMenu, i);
				if(hSubMenu)
					SetSubMenuData(hSubMenu, 0, TRUE/*fByPosition*/, dwItemData);
			}
		}

		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////
// GetMenuData
//
/////////////////////////////////////////////////////////////////
ULONG_PTR GetMenuData(HMENU hMenu, UINT uItem, BOOL fByPosition)
{
	//Setup our Menu Structure...
	//NOTE: HBITMAP was added in IE5, and may not work on older controls...
	MENUITEMINFOA menuinfo = { offsetof(MENUITEMINFOA, cch) + sizeof(UINT),	MIIM_DATA, 0, 0, 0, NULL, NULL, NULL, 0, NULL, 0 };

#ifndef _WIN64
	//First set this menu's item data
	//NOTE: HBITMAP was added in IE5, and may not work on older controls...
	//So we will first try with the smaller size, since this should be backward compatible...
	if(GetMenuItemInfoA(hMenu, uItem, fByPosition, &menuinfo))
		return menuinfo.dwItemData;
#endif //_WIN64

	//Retry Logic
	//On some OS's, (Win64), it only assumes the full NT5 structure size
	//thus for a new OS doesn't have backward compatability issues...
	menuinfo.cbSize = sizeof(MENUITEMINFOA);
	if(GetMenuItemInfoA(hMenu, uItem, fByPosition, &menuinfo))
		return menuinfo.dwItemData;
	return 0;
}


///////////////////////////////////////////////////////////////
// BrowseOpenFileName
//
///////////////////////////////////////////////////////////////
HRESULT BrowseOpenFileName(HINSTANCE hInstance, HWND hWnd, WCHAR* pwszTitle, WCHAR* pwszFileName, ULONG ulMaxSize, WCHAR* pwszExtension, WCHAR* pwszFilter, DWORD* pdwFileOffset, DWORD dwFlags)
{
	static ULONG ulFilterIndex = 1;  //Default to first filter
	HRESULT hr = E_FAIL;

	if(IsUnicodeOS())
	{
		//Setup OPENFILENAME struct...
		OPENFILENAMEW    ofn;
		memset( &ofn, 0, sizeof( ofn ));
		ofn.lStructSize       = sizeof( OPENFILENAMEW );
		ofn.hwndOwner         = hWnd;
		ofn.hInstance         = hInstance;
		ofn.nMaxFile          = ulMaxSize;
		ofn.nMaxCustFilter	  = MAX_NAME_LEN;
		ofn.nFilterIndex	  = ulFilterIndex;
		ofn.lpstrFile         = pwszFileName;
		ofn.lpstrTitle		  = pwszTitle;
		ofn.lpstrFilter		  = pwszFilter;
		ofn.lpstrDefExt		  = pwszExtension;
		ofn.Flags             = dwFlags;

		//Display Common Dialog to obtain File To Load...
		TESTC_(GetOpenFileNameW(&ofn),TRUE);

		//Save the Choosen Filter Index
		ulFilterIndex = ofn.nFilterIndex;
		hr = S_OK;

		//Save the first FileName offset (if multiselet)
		if(pdwFileOffset)
			*pdwFileOffset = ofn.nFileOffset;
	}
	else
	{
		CHAR szFileName[MAX_NAME_LEN];
		ConvertToMBCS(pwszFileName, szFileName, MAX_NAME_LEN);
		CHAR szTitle[MAX_NAME_LEN];
		ConvertToMBCS(pwszTitle, szTitle, MAX_NAME_LEN);
		CHAR szExtension[MAX_NAME_LEN];
		ConvertToMBCS(pwszExtension, szExtension, MAX_NAME_LEN);

		//NOTE:  pwszFilter is a double null terminated string, (containing an array of singurally
		//null terminated sub strings).
		CHAR szFilter[MAX_QUERY_LEN] = {0};
		CHAR* pszFilter = szFilter;
		while(pwszFilter && *pwszFilter)
		{
			ConvertToMBCS(pwszFilter, pszFilter, MAX_QUERY_LEN-(INT)(pszFilter-szFilter));
			pwszFilter += wcslen(pwszFilter) + 1;
			pszFilter += strlen(pszFilter) + 1;
		}

		//Setup OPENFILENAME struct...
		OPENFILENAMEA    ofn;
		memset( &ofn, 0, sizeof( ofn ));
		ofn.lStructSize       = sizeof( OPENFILENAMEA );
		ofn.hwndOwner         = hWnd;
		ofn.hInstance         = hInstance;
		ofn.nMaxFile          = ulMaxSize;
		ofn.nMaxCustFilter	  = MAX_NAME_LEN;
		ofn.nFilterIndex	  = ulFilterIndex;
		ofn.lpstrFile         = szFileName;
		ofn.lpstrTitle		  = szTitle;
		ofn.lpstrFilter		  = szFilter;
		ofn.lpstrDefExt		  = szExtension;
		ofn.Flags             = dwFlags;

		//Display Common Dialog to obtain File To Load...
		TESTC_(GetOpenFileNameA(&ofn), TRUE)
		ConvertToWCHAR(szFileName, pwszFileName, MAX_NAME_LEN);

		//Save the Choosen Filter Index
		ulFilterIndex = ofn.nFilterIndex;
		hr = S_OK;

		//Save the first FileName offset (if multiselet)
		if(pdwFileOffset)
			*pdwFileOffset = ofn.nFileOffset;
	}

CLEANUP:
	return hr;
}


///////////////////////////////////////////////////////////////
// CreateDefFileName
//
///////////////////////////////////////////////////////////////
HRESULT CreateDefFileName(LPCWSTR pwszFileName, WCHAR* pwszFullPath, ULONG ulMaxSize, BOOL fCurrentDir)
{
	if(pwszFileName == NULL || pwszFullPath == NULL || ulMaxSize == 0)
		return E_INVALIDARG;
	
	//Obtain the current Directory
	DWORD ulStrLen = 0;
	if(fCurrentDir)
	{
		if(IsUnicodeOS())
		{
			ulStrLen = GetCurrentDirectoryW(ulMaxSize, pwszFullPath);
		}
		else
		{
			CHAR szBuffer[MAX_PATH+1];
			ulStrLen = GetCurrentDirectoryA(MAX_PATH, szBuffer);
			ConvertToWCHAR(szBuffer, pwszFullPath, ulMaxSize);
		}
	}
	else
	{
		if(IsUnicodeOS())
		{
			ulStrLen = GetSystemDirectoryW(pwszFullPath, ulMaxSize);
		}
		else
		{
			CHAR szBuffer[MAX_PATH+1];
			ulStrLen = GetSystemDirectoryA(szBuffer, MAX_PATH);
			ConvertToWCHAR(szBuffer, pwszFullPath, ulMaxSize);
		}
	}
	
	if(ulStrLen)
	{
		//We can't just strcat a "\filename" since depending upon the current
		//directory sometimes it returns "c:\" or "c:\oledb", etc...
		if(pwszFullPath[ulStrLen-1] != L'\\')
			wcscat_s(pwszFullPath, ulMaxSize, L"\\");


		//Now tack on the suggested filename...
		wcscat_s(pwszFullPath, ulMaxSize, pwszFileName);
		return S_OK;
	}	
	
	return E_FAIL;
}


///////////////////////////////////////////////////////////////
// BrowseSaveFileName
//
///////////////////////////////////////////////////////////////
HRESULT BrowseSaveFileName(HINSTANCE hInstance, HWND hWnd, WCHAR* pwszTitle, WCHAR* pwszFileName, ULONG ulMaxSize, WCHAR* pwszExtension, WCHAR* pwszFilter, DWORD* pdwFileOffset, DWORD dwFlags)
{
	HRESULT hr = E_FAIL;
	static ULONG ulFilterIndex = 1;  //Default to first filter

	if(IsUnicodeOS())
	{
		//Setup OPENFILENAME struct...
		OPENFILENAMEW    ofn;
		memset( &ofn, 0, sizeof( ofn ));
		ofn.lStructSize       = sizeof( OPENFILENAMEW );
		ofn.hwndOwner         = hWnd;
		ofn.hInstance         = hInstance;
		ofn.nMaxFile          = ulMaxSize;
		ofn.lpstrFile         = pwszFileName;
		ofn.lpstrTitle		  = pwszTitle;
		ofn.lpstrFilter		  = pwszFilter;
		ofn.nMaxCustFilter	  = MAX_NAME_LEN;
		ofn.nFilterIndex	  = ulFilterIndex;
		ofn.lpstrDefExt		  = pwszExtension;
		ofn.Flags             = dwFlags;
		
		//Display Common Dialog to obtain File To Save...
		if(GetSaveFileNameW(&ofn))
		{
			ulFilterIndex = ofn.nFilterIndex;

			//Save the first FileName offset (if multiselet)
			if(pdwFileOffset)
				*pdwFileOffset = ofn.nFileOffset;
			
			hr = S_OK;
		}
	}
	else
	{
		CHAR szFileName[MAX_NAME_LEN];
		ConvertToMBCS(pwszFileName, szFileName, MAX_NAME_LEN);
		CHAR szTitle[MAX_NAME_LEN];
		ConvertToMBCS(pwszTitle, szTitle, MAX_NAME_LEN);
		CHAR szExtension[MAX_NAME_LEN];
		ConvertToMBCS(pwszExtension, szExtension, MAX_NAME_LEN);

		//NOTE:  pwszFilter is a double null terminated string, (containing an array of singurally
		//null terminated sub strings).
		CHAR szFilter[MAX_QUERY_LEN] = {0};
		CHAR* pszFilter = szFilter;
		while(pwszFilter && *pwszFilter)
		{
			ConvertToMBCS(pwszFilter, pszFilter, MAX_QUERY_LEN-(INT)(pszFilter-szFilter));
			pwszFilter += wcslen(pwszFilter) + 1;
			pszFilter += strlen(pszFilter) + 1;
		}

		//Setup OPENFILENAME struct...
		OPENFILENAMEA    ofn;
		memset( &ofn, 0, sizeof( ofn ));
		ofn.lStructSize       = sizeof( OPENFILENAMEA );
		ofn.hwndOwner         = hWnd;
		ofn.hInstance         = hInstance;
		ofn.nMaxFile          = ulMaxSize;
		ofn.lpstrFile         = szFileName;
		ofn.lpstrTitle		  = szTitle;
		ofn.lpstrFilter		  = szFilter;
		ofn.nMaxCustFilter	  = MAX_NAME_LEN;
		ofn.nFilterIndex	  = ulFilterIndex;
		ofn.lpstrDefExt		  = szExtension;
		ofn.Flags             = dwFlags;

		//Display Common Dialog to obtain File To Save...
		if(GetSaveFileNameA(&ofn))
		{
			ConvertToWCHAR(szFileName, pwszFileName, MAX_NAME_LEN);
			ulFilterIndex = ofn.nFilterIndex;

			//Save the first FileName offset (if multiselet)
			if(pdwFileOffset)
				*pdwFileOffset = ofn.nFileOffset;

			hr = S_OK;
		}
	}

	return hr;
}


////////////////////////////////////////////////////////
// HRESULT CreateRegKey
//
////////////////////////////////////////////////////////
HRESULT CreateRegKey(HKEY hRootKey, WCHAR* pwszKeyName, HKEY* phKey, REGSAM samDesired)
{
	HRESULT hr = E_FAIL;
	ULONG dwDisposition = 0;

	//Need the name of the key to open
	if(!pwszKeyName)
		return E_FAIL;
	
	if(IsUnicodeOS())
	{
		hr = RegCreateKeyExW(hRootKey, pwszKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, phKey, &dwDisposition);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);

		hr = RegCreateKeyExA(hRootKey, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, phKey, &dwDisposition);
	}

	if(hr != S_OK)
		hr = E_FAIL;
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT OpenRegKey
//
////////////////////////////////////////////////////////
HRESULT OpenRegKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD ulOptions, REGSAM samDesired, HKEY* phKey)
{
	HRESULT hr = E_FAIL;

	if(IsUnicodeOS())
	{
		//Obtain the Key for HKEY_CLASSES_ROOT\"SubKey"
		hr = RegOpenKeyExW(hRootKey, pwszKeyName, ulOptions, samDesired, phKey);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);

		//Obtain the Key for HKEY_CLASSES_ROOT\"SubKey"
		hr = RegOpenKeyExA(hRootKey, szBuffer, ulOptions, samDesired, phKey);
	}

	if(hr != S_OK)
		return E_FAIL;
	return hr;
}


////////////////////////////////////////////////////////
// WCHAR* GetProgID
//
////////////////////////////////////////////////////////
WCHAR* GetProgID(REFCLSID clsid)
{
	WCHAR* pwszProgID = NULL;
	WCHAR wszBuffer[MAX_NAME_LEN];
	wszBuffer[0] = wEOL;

	//ProgID From the Sprecified CLSID
	if(FAILED(ProgIDFromCLSID(clsid, &pwszProgID)))
	{
		//If that does work, we will just return the 
		//String representation of the GUID
		StringFromGUID2(clsid, wszBuffer, MAX_NAME_LEN);
		pwszProgID = wcsDuplicate(wszBuffer);
	}

	return pwszProgID;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumKey
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszSubKeyName, ULONG cBytes)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//Need some place to put the key name returned!
	if(!pwszSubKeyName)
		return E_FAIL;
	
	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}
	
	if(IsUnicodeOS())
	{
		//Obtain the specified RegItem at the index specified
		hr = RegEnumKeyW(hKey ? hKey : hRootKey, dwIndex, pwszSubKeyName, cBytes);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};
		
		//Obtain the specified RegItem at the index specified
		hr = RegEnumKeyA(hKey ? hKey : hRootKey, dwIndex, szBuffer, MAX_NAME_LEN);
		ConvertToWCHAR(szBuffer, pwszSubKeyName, cBytes);
	}
	

CLEANUP:
	if(hr != S_OK && hr != ERROR_NO_MORE_ITEMS)
		hr = E_FAIL;
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumValue
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR** ppwszValueName)
{
	HRESULT hr = S_OK;
	ULONG cBytes = 0;
	HKEY hKey = NULL;
	ULONG cMaxValueChars = 0;

	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}

	//First obtain the length of the Value...
	if(S_OK == RegQueryInfoKey(hKey ? hKey : hRootKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cMaxValueChars, NULL, NULL, NULL))
	{
		//Alloc a buffer large enough...
		SAFE_ALLOC(*ppwszValueName, WCHAR, cMaxValueChars+1);
		(*ppwszValueName)[0] = wEOL;

		//Now obtain the data...
		cBytes = (cMaxValueChars+1)*sizeof(WCHAR);
		hr = GetRegEnumValue(hRootKey, pwszKeyName, dwIndex, *ppwszValueName, &cBytes);
	}
	
CLEANUP:
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumValue
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszValueName, ULONG* pcBytes)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;
	ASSERT(pcBytes);

	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}

	if(IsUnicodeOS())
	{
		//Obtain the specified RegItem at the index specified
		hr = RegEnumValueW(hKey ? hKey : hRootKey, dwIndex, pwszValueName, pcBytes, 0, NULL, NULL, 0);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};
		ULONG cTotal = sizeof(szBuffer);
		
		//Obtain the specified RegItem at the index specified
		hr = RegEnumValueA(hKey ? hKey : hRootKey, dwIndex, pwszValueName ? szBuffer : NULL, &cTotal, 0, NULL, NULL, 0);
		
		if(pwszValueName)
			ConvertToWCHAR(szBuffer, pwszValueName, *pcBytes);
		*pcBytes = cTotal;
	}
	
CLEANUP:
	if(hr != S_OK && hr != ERROR_NO_MORE_ITEMS)
		hr = E_FAIL;
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue, ULONG cBytes)
{
	if(IsUnicodeOS())
	{
		return GetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue, cBytes, NULL, REG_SZ);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		HRESULT hr = GetRegEntry(hRootKey, pwszKeyName, pwszValueName, szBuffer, MAX_NAME_LEN, NULL, REG_SZ);
		if(SUCCEEDED(hr))
			ConvertToWCHAR(szBuffer, pwszValue, cBytes);

		return hr;
	}
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR** ppwszValue)
{
	HRESULT hr = S_OK;
	ASSERT(ppwszValue);
	ULONG cBytes = 0;
	CHAR* pszBuffer = NULL;

	//Obtain the length first (value = NULL)
	TESTC(hr = GetRegEntry(hRootKey, pwszKeyName, pwszValueName, NULL, 0, &cBytes, REG_SZ));

	//Allocate the string
	//NOTE: The length returned from RegQueryValueEx also includes the NULL terminator
	SAFE_ALLOC(*ppwszValue, WCHAR, cBytes);

	if(IsUnicodeOS())
	{
		//Now obtain the data
		TESTC(hr = GetRegEntry(hRootKey, pwszKeyName, pwszValueName, *ppwszValue, cBytes, &cBytes, REG_SZ));
	}
	else
	{
		//Allocate the string
		SAFE_ALLOC(pszBuffer, CHAR, cBytes);
		
		//Now obtain the data
		TESTC(hr = GetRegEntry(hRootKey, pwszKeyName, pwszValueName, pszBuffer, cBytes, &cBytes, REG_SZ));
		ConvertToWCHAR(pszBuffer, *ppwszValue, cBytes);
		return hr;
	}

CLEANUP:
	SAFE_FREE(pszBuffer);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG* pulValue)
{
	return GetRegEntry(hRootKey, pwszKeyName, pwszValueName, pulValue, sizeof(ULONG), NULL, REG_DWORD);
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cBytes, ULONG* pcActualBytes, ULONG dwType)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//NOTE: pStruct can be NULL if the user is just obtaining the length...

	//Obtain the Data for the above key
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}
		
	if(IsUnicodeOS())
	{
		hr = RegQueryValueExW(hKey ? hKey : hRootKey, pwszValueName, NULL, &dwType, (BYTE*)pStruct, &cBytes);
	}
	else
	{
		//Obtain the Data for the above key
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValueName, szBuffer, MAX_NAME_LEN);
		
		hr = RegQueryValueExA(hKey ? hKey : hRootKey, szBuffer, NULL, &dwType, (BYTE*)pStruct, &cBytes);
	}
	
	if(hr != S_OK)
		hr = E_FAIL;

CLEANUP:
	if(pcActualBytes)
		*pcActualBytes = cBytes;

	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue)
{
	if(IsUnicodeOS())
	{
		return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue ? pwszValue : L"", pwszValue ? (ULONG)((wcslen(pwszValue)+1)*sizeof(WCHAR)) : (ULONG)sizeof(WCHAR), REG_SZ);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValue, szBuffer, MAX_NAME_LEN);
		return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue ? szBuffer : "", pwszValue ? (ULONG)((strlen(szBuffer)+1)*sizeof(CHAR)) : (ULONG)sizeof(CHAR), REG_SZ);
	}
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG ulValue)
{
	return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, &ulValue, sizeof(ULONG), REG_DWORD);
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cBytes, DWORD dwType)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//Obtain the Data for the above key
	if(pwszKeyName)
	{
		if(FAILED(hr = CreateRegKey(hRootKey, pwszKeyName, &hKey)))
			goto CLEANUP;
	}
	
	if(IsUnicodeOS())
	{
		//Set the data for the above key (or the root key...)
		hr = RegSetValueExW(hKey ? hKey : hRootKey, pwszValueName, 0, dwType, (BYTE*)pStruct, cBytes);
	}
	else
	{
		//Set the data for the above key (or the root key...)
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValueName, szBuffer, MAX_NAME_LEN);
		hr = RegSetValueExA(hKey ? hKey : hRootKey, szBuffer, 0, dwType, (BYTE*)pStruct, cBytes);
	}

	if(hr != S_OK)
		hr = E_FAIL;

CLEANUP:
	CloseRegKey(hKey);
	return hr;
}



////////////////////////////////////////////////////////
// HRESULT DelRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName)
{
	HRESULT hr;

	//DelRegEntry
	if(IsUnicodeOS())
	{
		hr = RegDeleteKeyW(hRootKey, pwszKeyName);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);
		hr = RegDeleteKeyA(hRootKey, szBuffer);
	}

	//Entry successfully deleted - return S_OK
	if(hr==S_OK) 
		return S_OK;

	//Entry not found - return S_FALSE
	if(hr==ERROR_FILE_NOT_FOUND)
		return S_FALSE;

	return E_FAIL;
}


////////////////////////////////////////////////////////
// HRESULT DelRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, BOOL fSubKeys)
{
	HKEY hKey = NULL;
	WCHAR wszBuffer[MAX_NAME_LEN];
	HRESULT hr = S_OK;
	
	//RegDeleteKey only deletes the key if there are no subkeys
	if(SUCCEEDED(hr = OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ | KEY_WRITE, &hKey)))
	{
		//This is a pain to always have to delete the subkeys to remove the key...
		if(fSubKeys)
		{
			//Loop over all subkeys...
			//NOTE: GetRegEnum requires KEY_ENUMERATE_SUB_KEYS which is found in KEY_READ.
			while((hr = GetRegEnumKey(hKey, NULL, 0, wszBuffer, MAX_NAME_LEN))==S_OK)
			{
				//Recurse and delete the sub key...
				if(FAILED(hr = DelRegEntry(hKey, wszBuffer, fSubKeys)))
					break;
			}
		}
		
		//Now we can delete the root key
		hr = DelRegEntry(hRootKey, pwszKeyName);
		CloseRegKey(hKey);
	}

	return hr;
}

	
////////////////////////////////////////////////////////
// HRESULT CloseRegKey
//
////////////////////////////////////////////////////////
HRESULT CloseRegKey(HKEY hKey)
{
	HRESULT hr = S_OK;
	
	//RegCloseKey
	if(hKey)
		hr = RegCloseKey(hKey);
	
	if(hr != S_OK)
		hr = E_FAIL;
	return hr;
}


///////////////////////////////////////////////////////////////
// Static Strings Messages
//
///////////////////////////////////////////////////////////////

extern WCHAR wsz_SUCCESS[]				= L"Microsoft OLE DB RowsetViewer - Success";
extern WCHAR wsz_WARNING[]				= L"Microsoft OLE DB RowsetViewer - Warning";
extern WCHAR wsz_INFO[]					= L"Microsoft OLE DB RowsetViewer - Info";
extern WCHAR wsz_ERROR[]				= L"Microsoft OLE DB RowsetViewer - Error";
extern WCHAR wsz_EXCEPTION[]			= L"Microsoft OLE DB RowsetViewer - Exception";
extern WCHAR wsz_ERRORINFO[]			= L"Microsoft OLE DB RowsetViewer - IErrorInfo";

//General String Values
extern WCHAR wsz_INVALID_VALUE_[]		= L"Invalid Value '%s' specified.  Please specify a value >= %lu and <= %lu.";


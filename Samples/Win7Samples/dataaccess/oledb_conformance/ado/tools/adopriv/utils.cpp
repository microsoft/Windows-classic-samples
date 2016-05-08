#include "stdafx.h"
#include "resource.h"
#include <privlib.h>
#include "Utils.h"

#ifdef __USEPRIVLIB
	fCreateModInfo*	pf;
#else
/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//		Previously allocated memory
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cStrLen)
{
//	ASSERT(pwsz && psz);

	//Convert the string to MBCS
	INT iResult = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cStrLen, NULL, NULL);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
CHAR* ConvertToMBCS(WCHAR* pwsz)
{
	//no-op case
	if(!pwsz)
		return NULL;
	
	ULONG cLen	= wcslen(pwsz);

	//Allocate space for the string
	CHAR* pszBuffer = new CHAR[((cLen+1)*sizeof(CHAR))];
	if(pszBuffer==NULL)
		goto CLEANUP;

	//Now convert the string
	WideCharToMultiByte(CP_ACP, 0, pwsz, -1, pszBuffer, cLen+1, NULL, NULL);

CLEANUP:
	return pszBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//		Previously allocated memory
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cStrLen)
{
	//ASSERT(psz && pwsz);

	//Convert the string to MBCS
	INT iResult = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cStrLen);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
WCHAR* ConvertToWCHAR(CHAR* psz)
{
	//no-op case
	if(!psz)
		return NULL;
	
	ULONG cLen	= strlen(psz);

	//Allocate space for the string
	WCHAR* pwszBuffer = new WCHAR[((cLen+1)*sizeof(WCHAR))];
	if(pwszBuffer==NULL)
		goto CLEANUP;

	//Now convert the string
	MultiByteToWideChar(CP_ACP, 0, psz, -1, pwszBuffer, cLen+1);

CLEANUP:
	return pwszBuffer;
}


//--------------------------------------------------------------------
// @func BOOL | IsFixedLength
//
// Returns TRUE is type is fixed length 
//
//--------------------------------------------------------------------
BOOL IsFixedLength(DBTYPE dbtype)
{
	if ((DBTYPE_STR == (~(DBTYPE_BYREF) & dbtype))    ||
		(DBTYPE_STR == (~(DBTYPE_VECTOR) & dbtype))   ||
		(DBTYPE_STR == (~(DBTYPE_ARRAY) & dbtype))    ||
		(DBTYPE_WSTR == (~(DBTYPE_BYREF) & dbtype))   ||
		(DBTYPE_WSTR == (~(DBTYPE_VECTOR) & dbtype))  ||
		(DBTYPE_WSTR == (~(DBTYPE_ARRAY) & dbtype))   ||
		(DBTYPE_BSTR == (~(DBTYPE_BYREF) & dbtype))   ||
		(DBTYPE_BSTR == (~(DBTYPE_VECTOR) & dbtype))  ||
		(DBTYPE_BSTR == (~(DBTYPE_ARRAY) & dbtype))   ||
		(DBTYPE_BYTES == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_BYTES == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_BYTES == (~(DBTYPE_ARRAY) & dbtype)))
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------
// @func BOOL | IsNumericType
//
// Returns TRUE is type is numeric
//
//--------------------------------------------------------------------
BOOL  IsNumericType(DBTYPE dbtype)
{
	if ((DBTYPE_I1 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_I1 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_I1 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_I2 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_I2 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_I2 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_I4 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_I4 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_I4 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_I8 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_I8 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_I8 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_UI1 == (~(DBTYPE_BYREF) & dbtype)) ||
		(DBTYPE_UI1 == (~(DBTYPE_VECTOR) & dbtype))||
		(DBTYPE_UI1 == (~(DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_UI2 == (~(DBTYPE_BYREF) & dbtype)) ||
		(DBTYPE_UI2 == (~(DBTYPE_VECTOR) & dbtype))||
		(DBTYPE_UI2 == (~(DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_UI4 == (~(DBTYPE_BYREF) & dbtype)) ||
		(DBTYPE_UI4 == (~(DBTYPE_VECTOR) & dbtype))||
		(DBTYPE_UI4 == (~(DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_UI8 == (~(DBTYPE_BYREF) & dbtype)) ||
		(DBTYPE_UI8 == (~(DBTYPE_VECTOR) & dbtype))||
		(DBTYPE_UI8 == (~(DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_R4 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_R4 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_R4 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_R8 == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_R8 == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_R8 == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_CY == (~(DBTYPE_BYREF) & dbtype))  ||
		(DBTYPE_CY == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_CY == (~(DBTYPE_ARRAY) & dbtype))  ||
		(DBTYPE_DECIMAL == (~(DBTYPE_BYREF) & dbtype))	||
		(DBTYPE_DECIMAL == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_DECIMAL == (~(DBTYPE_ARRAY) & dbtype))	||
		(DBTYPE_NUMERIC == (~(DBTYPE_BYREF) & dbtype))	||
		(DBTYPE_NUMERIC == (~(DBTYPE_VECTOR) & dbtype)) ||
		(DBTYPE_NUMERIC == (~(DBTYPE_ARRAY) & dbtype)))
		return TRUE;
	else 
		return FALSE;
}

//--------------------------------------------------------------------
// @func BOOL | AddCharToNumericVal
//
// Uses 64-bit arithmetic
// Returns TRUE if successfull.
//
//--------------------------------------------------------------------
BOOL AddCharToNumericVal(
	WCHAR wLetter,
	DB_NUMERIC * pNumeric
)
{
	//ASSERT(iswdigit(wLetter) && pNumeric);
	
	// Check parameters
	if (!pNumeric)
		return FALSE;

	// Convert WCHAR to ULONG
	WCHAR pwszBuf[2];
	pwszBuf[0]  = wLetter;
	pwszBuf[1]  = L'\0';;
	LONG lDigit = _wtol(pwszBuf);

	// If operation won't overflow
	if (pNumeric->precision <= sizeof(DWORDLONG) - 1)
	{
		*(UNALIGNED DWORDLONG *)pNumeric->val *= 10;
		*(UNALIGNED DWORDLONG *)pNumeric->val += lDigit;
	}
	else
	{
		DWORDLONG dwlAccum = lDigit;

		for(ULONG ul = 0; ul < sizeof(pNumeric->val) / sizeof(ULONG); ul++ )
		{
			dwlAccum +=(DWORDLONG)(*(((UNALIGNED ULONG *)pNumeric->val) + ul)) * 10;
			*(((UNALIGNED ULONG *)pNumeric->val) + ul) = (ULONG)dwlAccum;
			dwlAccum >>= sizeof(ULONG) * 8;
		}
	}

	//	Adjust length if overflow into next byte
	if (pNumeric->precision < sizeof(pNumeric->val) && *(pNumeric->val + pNumeric->precision) != 0)
		pNumeric->precision++;

	return TRUE;
}

#endif //__USEPRIVLIB
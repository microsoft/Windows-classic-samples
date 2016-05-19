//-----------------------------------------------------------------------------
// Microsoft Local Test Manager (LTM)
// Copyright (C) 1997 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module MODERROR.HPP
//
//-----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////
#ifndef __MODERROR_HPP_
#define __MODERROR_HPP_

/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "ModuleCore.h"
#include <assert.h>


/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
#define __LONGSTRING(string) L##string
#define LONGSTRING(string) __LONGSTRING(string)
#define	CHECK(func, scode)  m_pError->Validate(func, LONGSTRING(__FILE__), __LINE__,ResultFromScode(scode))
#define COMPARE(obj1,obj2) m_pError->Compare(obj1==obj2, LONGSTRING(__FILE__), __LINE__)

#define	SUCCEED	S_OK  
#define FAIL	FALSE

// Unfortunatly these were defined originally and now there
// are plently of tests and privlib using these.  Don't remove
typedef ULONG UDWORD;
typedef USHORT UWORD;

const WCHAR wszReceivedHR[]			= L"Received: ";
const WCHAR wszExpectedHR[]			= L"Expected: ";
const WCHAR wszFile[]				= L"File:";
const WCHAR wszLine[]				= L"Line:";
const WCHAR wszNewLine[]			= L"\r\n";

const WCHAR wszExpectedSuccess[]	= L"Expected a Successful HRESULT";
const WCHAR wszDashes[]				= L"------------";
const WCHAR wszCompareFailed[]		= L"The items compared are not equal.";


/////////////////////////////////////////////////////////////////////////////
// CError
//
/////////////////////////////////////////////////////////////////////////////
class CError
{
public:
	//Constructors
	CError(ERRORLEVEL eLevel = HR_STRICT);
	virtual ~CError();

	//Interface
	HRESULT	SetErrorInterface(IError* pIError);
	HRESULT GetErrorInterface(IError** ppIError);
	
	//Error Level
	void SetErrorLevel(ERRORLEVEL eLevel);
	ERRORLEVEL GetErrorLevel(void);
	HRESULT	GetActualHr(void);
	
	//Validation
	BOOL Validate(HRESULT ActualHr, WCHAR* pwszFile, DWORD ulLine, HRESULT hrExpected = S_OK);
	BOOL Compare(BOOL fEqual, WCHAR* pwszFile, DWORD ulLine);

	//Logging
	void LogExpectedHr(HRESULT hrExpected);
	void LogReceivedHr(HRESULT hrReceived, WCHAR* pwszFile, DWORD ulLine);
	
	//Error Counts
	void ResetModErrors();
	void ResetCaseErrors();
	void ResetVarErrors();
	DWORD GetModErrors();
	DWORD GetCaseErrors();
	DWORD GetVarErrors();
	CError & operator ++(int);

private:
	BOOL		m_fTriedErrorAlready;
	IError*		m_pIError;
	IError*		pIError(); //Internal use Only

};	

#endif  //__MODERROR_HPP_

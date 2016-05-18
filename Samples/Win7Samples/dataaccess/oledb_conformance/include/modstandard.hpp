#ifndef _MODSTANDARD_HPP_
#define _MODSTANDARD_HPP_

//suppress warnings about calling "unsecure" string functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif	//_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif	//_CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <ocidl.h>


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "MODMacros.hpp"
#include "CSuperLog.hpp"
#include "MODClasses.hpp"
#include "MODuleCore.h"
#include <crtdbg.h>

/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
//These inlines are to output the filename and line number before every allocation...
inline	void* CoTaskMemAlloc_Trace(SIZE_T cbSize, CHAR* pszFileName, ULONG ulLine)
{		
#ifdef _DEBUG
	_CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "CoTaskMemAlloc(%d) - File: %s, Line: %Iu\n", cbSize, pszFileName, ulLine);
#endif	//_DEBUG
	return CoTaskMemAlloc(cbSize);
}

inline	void* CoTaskMemRealloc_Trace(void* pv, SIZE_T cbSize, CHAR* pszFileName, ULONG ulLine)
{
#ifdef _DEBUG
	_CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "CoTaskMemRealloc(0x%08x, %Iu) - File: %s, Line: %d\n", pv, cbSize, pszFileName, ulLine);
#endif	//_DEBUG
	return CoTaskMemRealloc(pv, cbSize);
}

inline	void CoTaskMemFree_Trace(void* pv, CHAR* pszFileName, ULONG ulLine)
{
#ifdef _DEBUG
	_CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "CoTaskMemFree(0x%08x) - File: %s, Line: %u\n", pv, pszFileName, ulLine);
#endif	//_DEBUG
	CoTaskMemFree(pv);
}

//Macros
#define LTMALLOCSPY(cbSize)				CoTaskMemAlloc_Trace(cbSize, __FILE__, __LINE__)
#define LTMREALLOCSPY(pv, cbSize)		CoTaskMemRealloc_Trace(pv, cbSize, __FILE__, __LINE__)
#define LTMFREESPY(pv)					CoTaskMemFree_Trace(pv, __FILE__, __LINE__)



#endif // _MODSTANDARD_HPP_

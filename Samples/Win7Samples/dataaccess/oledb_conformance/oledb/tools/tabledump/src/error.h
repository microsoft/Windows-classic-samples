//-----------------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// @doc
//
// @module ERROR.H
//
//-----------------------------------------------------------------------------------

#ifndef _ERROR_H_
#define _ERROR_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "common.h"


////////////////////////////////////////////////////////////////////////////
// Extened Error Info
//
////////////////////////////////////////////////////////////////////////////
//Get ISQLErrorInfo
HRESULT GetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr);

//Get IErrorRecords
HRESULT GetErrorRecords(HWND hWnd, ULONG* pcRecords, IErrorRecords** ppIErrorRecords);
HRESULT DisplayErrorRecords(HWND hWnd, ULONG cRecords, IErrorRecords* pIErrorRecords, CHAR* pszFile = "Unknown", ULONG ulLine = 0);
HRESULT DisplayAllErrors(HWND hWnd, HRESULT hrActual, CHAR* pszFile = "Unknown", ULONG ulLine = 0);
HRESULT DisplayAllErrors(HWND hWnd, HRESULT hrActual, HRESULT hrExpected, CHAR* pszFile = "Unknown", ULONG ulLine = 0);

//Property Errors
HRESULT DisplayPropErrors(HWND hWnd, REFIID riid, IUnknown* pIUnknown);
HRESULT DisplayPropErrors(HWND hWnd, ULONG cPropSets, DBPROPSET* rgPropSets);
HRESULT DisplayAllProps(HWND hWnd, ULONG cPropSets, DBPROPSET* rgPropSets);

//Binding Errors
HRESULT DisplayBindingErrors(HWND hWnd, DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData);
HRESULT DisplayAccessorErrors(HWND hWnd, DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBBINDSTATUS* rgStatus);


#endif	//_ERROR_H_

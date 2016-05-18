//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module ERROR.H
//
//-----------------------------------------------------------------------------
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
WCHAR* GetErrorName(HRESULT hr);

//Get ISQLErrorInfo
HRESULT GetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr);

//Get IErrorRecords
HRESULT GetErrorRecords(ULONG* pcRecords, IErrorRecords** ppIErrorRecords);
HRESULT DisplayErrorRecords(HWND hWnd, ULONG cRecords, IErrorRecords* pIErrorRecords, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0);

HRESULT DisplayAllErrors(HWND hWnd, HRESULT hr, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0);

#endif	//_ERROR_H_

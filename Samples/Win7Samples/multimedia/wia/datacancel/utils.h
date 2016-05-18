#ifndef __WIA_COMMON_UTILS
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------
#define __WIA_COMMON_UTILS

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wia.h>
#include <oleauto.h>
#include <shlwapi.h>
#include <strsafe.h>


// Helper function to display an error message and an optional HRESULT
void ReportError( LPCTSTR pszMessage, HRESULT hr = S_OK );

HRESULT ReadPropertyGuid(IWiaItem2* pWiaItem2, PROPID propid , GUID* guid);

HRESULT ReadPropertyBSTR(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , BSTR* pbstr);

HRESULT ReadPropertyLong(IWiaItem2* pWiaItem2, PROPID propid , LONG* lVal);    

HRESULT WritePropertyGuid(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , GUID guid);

HRESULT WritePropertyLong(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , LONG lVal);

HRESULT ReadWiaPropsAndGetDeviceID( IWiaPropertyStorage *pWiaPropertyStorage ,BSTR* pbstrDeviceID );

HRESULT PrintItemName( IWiaItem2 *pIWiaItem2 );

#endif

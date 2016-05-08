//+--------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File:        module.cpp
//
// Contents:    Cert Server Exit Module implementation
//
//---------------------------------------------------------------------------

#include "pch.cpp"
#pragma hdrstop

#include "celib.h"
#pragma warning(push)
#pragma warning(disable : 4996) // to disable SDK warning from using deprecated APIs with ATL 7.0 and greater
#include "exit.h"
#include "module.h"
#pragma warning(pop)


STDMETHODIMP
CCertManageExitModuleSample::GetProperty(
    /* [in] */ const BSTR /* strConfig */ ,
    /* [in] */ BSTR /* strStorageLocation */ ,
    /* [in] */ BSTR strPropertyName,
    /* [in] */ LONG /* Flags */ ,
    /* [retval][out] */ VARIANT __RPC_FAR *pvarProperty)
{
    LPWSTR szStr = NULL;

    if (pvarProperty == NULL)
        return E_POINTER;

    VariantInit(pvarProperty);

    if (IsNullOrEmptyBStr(strPropertyName))
        return S_FALSE;

    if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_NAME))
        szStr = wsz_SAMPLE_NAME;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_DESCRIPTION))
        szStr = wsz_SAMPLE_DESCRIPTION;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_COPYRIGHT))
        szStr = wsz_SAMPLE_COPYRIGHT;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_FILEVER))
        szStr = wsz_SAMPLE_FILEVER;
    else if (0 == _wcsicmp(strPropertyName, wszCMM_PROP_PRODUCTVER))
        szStr = wsz_SAMPLE_PRODUCTVER;
    else
        return S_FALSE;  

    pvarProperty->bstrVal = SysAllocString(szStr);
    if (NULL == pvarProperty->bstrVal)
        return E_OUTOFMEMORY;

    pvarProperty->vt = VT_BSTR;

    return S_OK;
}
        
STDMETHODIMP 
CCertManageExitModuleSample::SetProperty(
    /* [in] */ const BSTR /* strConfig */ ,
    /* [in] */ BSTR /* strStorageLocation */ ,
    /* [in] */ BSTR /* strPropertyName */ ,
    /* [in] */ LONG /* Flags */ ,
    /* [in] */ VARIANT const __RPC_FAR * /* pvarProperty */ )
{
     // no settable properties supported
     return S_FALSE;
}
        
STDMETHODIMP
CCertManageExitModuleSample::Configure( 
    /* [in] */ const BSTR /* strConfig */ ,
    /* [in] */ BSTR /* strStorageLocation */ ,
    /* [in] */ LONG /* Flags */ )
{
    MessageBox(NULL, L"No Configurable Options", NULL, MB_OK|MB_ICONINFORMATION);

    return S_OK;
}

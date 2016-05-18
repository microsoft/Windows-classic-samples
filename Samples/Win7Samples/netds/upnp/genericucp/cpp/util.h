// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


void ReleaseObj(IUnknown* pInterface);

void PrintErrorText(HRESULT hr);

void SafeStrCopy(TCHAR* src, TCHAR* dst, size_t srcBufLen);

// For Invoke Action 
HRESULT HrCreateSafeArray(VARTYPE vt, int nArgs, SAFEARRAY **ppsa);
HRESULT HrCreateArgVariants(DWORD dwArgs, VARIANT*** pppVars);
HRESULT HrDestroyArgVariants(DWORD dwArgs, VARIANT*** pppVars);
void VariantSetVar(VARIANT* pvarToSet, VARIANT& va);
void VariantSetArray(SAFEARRAY* psa, VARIANT& va);
HRESULT HrGetSafeArrayBounds(SAFEARRAY *psa, long* plLBound, long* plUBound);
HRESULT HrGetVariantElement(SAFEARRAY *psa, int lPosition, VARIANT* pvar);

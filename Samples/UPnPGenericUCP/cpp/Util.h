// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


void ReleaseObj(_In_ IUnknown* pInterface);

void PrintErrorText(_In_ HRESULT hr);

void SafeStrCopy(_Out_writes_(srcBufLen) LPWSTR src, _In_ LPCWSTR dst, _In_ size_t srcBufLen);

// For Invoke Action 
HRESULT HrCreateSafeArray(_In_ VARTYPE vt, _In_ int nArgs, _Outptr_ SAFEARRAY **ppsa);
_When_(dwArgs == 0, _At_(*pppVars, _Post_null_))
_When_(dwArgs > 0, _At_(*pppVars, _Post_notnull_))
HRESULT HrCreateArgVariants(_In_ DWORD dwArgs, _Outptr_result_buffer_maybenull_(dwArgs) VARIANT*** pppVars);
HRESULT HrDestroyArgVariants(_In_ DWORD dwArgs, _Inout_ _At_(*pppVars, _Pre_writable_size_(dwArgs) _Post_maybenull_) VARIANT*** pppVars);
void VariantSetVar(_In_ VARIANT* pvarToSet, _Inout_ VARIANT& va);
void VariantSetArray(_In_ SAFEARRAY* psa, _Inout_ VARIANT& va);
HRESULT HrGetSafeArrayBounds(_In_ SAFEARRAY *psa, _Out_ long* plLBound, _Out_ long* plUBound);
HRESULT HrGetVariantElement(_In_ SAFEARRAY *psa, _In_ int lPosition, _Out_ VARIANT* pvar);

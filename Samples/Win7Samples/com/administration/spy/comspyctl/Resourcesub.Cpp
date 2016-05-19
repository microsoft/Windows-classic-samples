// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <comsvcs.h>
#include "ComSpyCtl.h"
#include "ComSpyAudit.h"
#include "CCOMSpy.h"
#include "SysLCESub.h"
#include "ResourceSub.h"
#include <strsafe.h>

/////////////////////////////////////////////////////////////////////////////
// CResourceSub

STDMETHODIMP CResourceSub::OnResourceCreate(
    COMSVCSEVENTINFO * pInfo, 
	ULONG64 ObjectID,
    LPCWSTR pwszType,
    ULONG64 resId,
	BOOL enlisted)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnResourceCreate", GuidToBstr(pInfo->guidApp));
	m_pSpy->AddParamValueToList(L"Type", pwszType);

	WCHAR szObjectID[32];
	StringCchPrintfW(szObjectID, ARRAYSIZE(szObjectID), L"%#016I64X", ObjectID);
	m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

	WCHAR szResID[32];
	StringCchPrintfW(szResID, ARRAYSIZE(szResID), L"%#016I64X", resId);
	m_pSpy->AddParamValueToList(L"ResourceID", szResID);

	WCHAR szEnlisted[10];
	if (enlisted)
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted), L"TRUE");
	else
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted),L"FALSE");
	m_pSpy->AddParamValueToList(L"Enlisted", szEnlisted);
		
	IF_AUDIT_DO(OnResourceCreate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szObjectID, pwszType, szResID,enlisted);

	return S_OK;
}


STDMETHODIMP CResourceSub::OnResourceAllocate(
	COMSVCSEVENTINFO * pInfo, 
	ULONG64 ObjectID,
    LPCWSTR pwszType,
    ULONG64 resId,
	BOOL enlisted,
	DWORD NumRated,
	DWORD Rating)
{
	
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnResourceAllocate",GuidToBstr(pInfo->guidApp));
	m_pSpy->AddParamValueToList(L"Type", pwszType);

	WCHAR szObjectID[32];
	StringCchPrintfW(szObjectID, ARRAYSIZE(szObjectID), L"%#016I64X", ObjectID);
	m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

	WCHAR szResID[32];
	StringCchPrintfW(szResID, ARRAYSIZE(szResID), L"%#016I64X", resId);
	m_pSpy->AddParamValueToList(L"ResourceID", szResID);

	WCHAR szEnlisted[10];
	if (enlisted)
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted), L"TRUE");
	else
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted), L"FALSE");
	m_pSpy->AddParamValueToList(L"Enlisted", szEnlisted);

	WCHAR szNumRated[16];
    StringCchPrintfW(szNumRated, ARRAYSIZE(szNumRated), L"%#08X", NumRated);
	m_pSpy->AddParamValueToList(L"NumRated", szNumRated);

	WCHAR szRating[16];
    StringCchPrintfW(szRating, ARRAYSIZE(szRating), L"%#08X", Rating);
	m_pSpy->AddParamValueToList(L"Rating", szRating);

	IF_AUDIT_DO(OnResourceAllocate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szObjectID, pwszType, szResID,enlisted,szNumRated,szRating);
	return S_OK;
}


STDMETHODIMP CResourceSub::OnResourceRecycle(
	COMSVCSEVENTINFO * pInfo, 
	ULONG64 ObjectID,
    LPCWSTR pwszType,
    ULONG64 resId)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnResourceRecycle",GuidToBstr(pInfo->guidApp));
	m_pSpy->AddParamValueToList(L"Type", pwszType);

	WCHAR szObjectID[32];
    StringCchPrintfW(szObjectID, ARRAYSIZE(szObjectID), L"%#016I64X", ObjectID);
	m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

	WCHAR szResID[32];
    StringCchPrintfW(szResID, ARRAYSIZE(szResID), L"%#016I64X", resId);
	m_pSpy->AddParamValueToList(L"ResourceID", szResID);

	IF_AUDIT_DO(OnResourceRecycle)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szObjectID, pwszType, szResID);
	return S_OK;
}


STDMETHODIMP CResourceSub::OnResourceDestroy(
    COMSVCSEVENTINFO * pInfo, 
	ULONG64 ObjectID,
	HRESULT hr,
    LPCWSTR pwszType,
    ULONG64 resId)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnResourceDestroy",GuidToBstr(pInfo->guidApp));
	
	m_pSpy->AddParamValueToList(L"Type", pwszType);

	WCHAR szObjectID[32];
	StringCchPrintfW(szObjectID, ARRAYSIZE(szObjectID), L"%#016I64X", ObjectID);
	m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

	WCHAR szResID[32];
	StringCchPrintfW(szResID, ARRAYSIZE(szResID), L"%#016I64X", resId);
	m_pSpy->AddParamValueToList(L"ResourceID", szResID);

	WCHAR szHResult[16];
	StringCchPrintfW(szHResult, ARRAYSIZE(szHResult), L"%#016I64X", hr);
	m_pSpy->AddParamValueToList(L"Destroy HR", szHResult);

	IF_AUDIT_DO(OnResourceDestroy)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), szObjectID, hr,pwszType, szResID);
	return S_OK;
}

STDMETHODIMP CResourceSub::OnResourceTrack(
											COMSVCSEVENTINFO * pInfo, 
											ULONG64 ObjectID,
											LPCWSTR pwszType,
											ULONG64 resId,
											BOOL enlisted)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnResourceTrack", GuidToBstr(pInfo->guidApp));

	m_pSpy->AddParamValueToList(L"Type", pwszType);

	WCHAR szObjectID[32];
	StringCchPrintfW(szObjectID, ARRAYSIZE(szObjectID), L"%#016I64X", ObjectID);
	m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

	WCHAR szResID[32];
	StringCchPrintfW(szResID, ARRAYSIZE(szResID), L"%#016I64X", resId);
	m_pSpy->AddParamValueToList(L"ResourceID", szResID);

	WCHAR szEnlisted[10];
	if (enlisted)
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted), L"TRUE");
	else
		StringCchPrintfW(szEnlisted, ARRAYSIZE(szEnlisted), L"FALSE");
	m_pSpy->AddParamValueToList(L"Enlisted", szEnlisted);
		
	IF_AUDIT_DO(OnResourceTrack)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szObjectID, pwszType, szResID,enlisted);


	return S_OK;
}


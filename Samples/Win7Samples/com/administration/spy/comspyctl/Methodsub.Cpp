// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ComSpyCtl.h"
#include <comsvcs.h>
#include "SysLCESub.h"
#include "ComSpyAudit.h"

#include "CComSpy.h"
#include "MethodSub.h"
#include <strsafe.h>

extern LONGLONG PerformanceFrequency;

STDMETHODIMP  CMethodSub::OnMethodCall(COMSVCSEVENTINFO * pInfo, ULONG64 oid,REFCLSID guidCid, REFIID guidRid, ULONG iMeth)
{
	TimeStack * pStack = m_map[oid];
	if (!pStack)
	{
		pStack = new TimeStack;
		m_map[oid] = pStack;
	}
	_ASSERTE(pStack);
	pStack->push_front(pInfo->perfCount);

	m_pSpy->AddEventToList(pInfo->perfCount, L"OnMethodCall", GuidToBstr(pInfo->guidApp));
	WCHAR id[32];
    StringCchPrintfW(id,ARRAYSIZE(id),L"%#016I64X", oid);
	m_pSpy->AddParamValueToList(L"ObjectID", id);

    CComBSTR bstrCLSID = GuidToBstr(guidCid);
    CComBSTR bstrIID = GuidToBstr(guidRid);

	m_pSpy->AddParamValueToList(L"CLSID", bstrCLSID);	
	m_pSpy->AddParamValueToList(L"riid", bstrIID);

	LPWSTR pwszGuidName = NULL;
	HRESULT hr;
	hr = GetMethodName(guidRid, iMeth, &pwszGuidName) ;
    if(hr == S_OK) 
	{ 
		m_pSpy->AddParamValueToList(L"Method Name", pwszGuidName);
	}
	else
	{
        static const size_t cchIndexMax = 8 + 1;
		pwszGuidName = new WCHAR[cchIndexMax] ; 
		StringCchPrintf(pwszGuidName, cchIndexMax, L"%X", iMeth) ; 
		m_pSpy->AddParamValueToList(L"v-table index", pwszGuidName);
	}
   	IF_AUDIT_DO(OnMethodCall)(pInfo->perfCount,
							  GuidToBstr(pInfo->guidApp),
							  id,						
							  bstrCLSID,						
							  bstrIID,			
							  pwszGuidName);



    delete [] pwszGuidName ;
    pwszGuidName = NULL ;
	return S_OK;
}

STDMETHODIMP  CMethodSub::OnMethodReturn(COMSVCSEVENTINFO * pInfo, 
										  ULONG64 oid,
										  REFCLSID guidCid, 
										  REFIID guidRid, 
										  ULONG iMeth, 
										  HRESULT hrRet)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnMethodReturn", GuidToBstr(pInfo->guidApp));
	WCHAR id[32];
	StringCchPrintfW(id,ARRAYSIZE(id),L"%#016I64X", oid);
	m_pSpy->AddParamValueToList(L"ObjectID", id);

    CComBSTR bstrCLSID = GuidToBstr(guidCid);
    CComBSTR bstrIID = GuidToBstr(guidRid);

	m_pSpy->AddParamValueToList(L"CLSID", bstrCLSID);
	m_pSpy->AddParamValueToList(L"riid", bstrIID);

	LPWSTR pwszGuidName   =   NULL ;
	HRESULT hr;
	hr = GetMethodName(guidRid, iMeth, &pwszGuidName) ;
    if(hr != S_OK) 
	{
        static const size_t cchIndexMax = 8 + 1;
		pwszGuidName = new WCHAR[cchIndexMax] ; 
		StringCchPrintf(pwszGuidName, cchIndexMax,L"%X", iMeth) ; 
		m_pSpy->AddParamValueToList(L"v-table index", pwszGuidName);
	}
	else
	{
		m_pSpy->AddParamValueToList(L"Method Name", pwszGuidName);
	}
    
	WCHAR wcRet[16];
    StringCchPrintfW(wcRet, ARRAYSIZE(wcRet), L"%#08X", hrRet);
    m_pSpy->AddParamValueToList(L"Return Value", wcRet);


	WCHAR sTime[16];
	TimeStack * pStack = m_map[oid];
    if (pStack)
    {
	    LONGLONG oldTime  = pStack->front();
	    pStack->pop_front();
	    _ASSERTE(oldTime);

        DWORD timeDiff = (DWORD)((1000*(pInfo->perfCount-oldTime))/PerformanceFrequency);
	    StringCchPrintf(sTime, ARRAYSIZE(sTime),L"%d", timeDiff);

	    m_pSpy->AddParamValueToList(L"Call time (ms)", sTime);
    }
	

	IF_AUDIT_DO(OnMethodReturn)(pInfo->perfCount,	
								GuidToBstr(pInfo->guidApp),
								  id,						
								  bstrCLSID,						
								  bstrIID,			
								  pwszGuidName,
								  hrRet,
								  sTime);
	delete [] pwszGuidName ;
    pwszGuidName = NULL ;


	if (pStack && pStack->empty())
	{
		delete pStack;
		m_map.erase(oid);
	}
	return S_OK;
}

STDMETHODIMP  CMethodSub::OnMethodException(COMSVCSEVENTINFO * pInfo, ULONG64 oid,REFCLSID guidCid, REFIID guidRid, ULONG iMeth)
{
	m_pSpy->AddEventToList(pInfo->perfCount, L"OnMethodException", GuidToBstr(pInfo->guidApp));
	
	WCHAR id[32];
    StringCchPrintfW(id, ARRAYSIZE(id), L"%#016I64X", oid);
	m_pSpy->AddParamValueToList(L"ObjectID", id);

    CComBSTR bstrCLSID = GuidToBstr(guidCid);
    CComBSTR bstrIID = GuidToBstr(guidRid);

	m_pSpy->AddParamValueToList(L"CLSID", bstrCLSID);
	m_pSpy->AddParamValueToList(L"riid", bstrIID);

	LPWSTR pwszGuidName   =   NULL ;
	HRESULT hr;
	hr = GetMethodName(guidRid, iMeth, &pwszGuidName) ;
    if(hr != S_OK) 
	{
	pwszGuidName = new WCHAR[64] ; 
	StringCchCopy(pwszGuidName, 64, L"Unknown (no TypeLibrary)"); 
	}
    m_pSpy->AddParamValueToList(L"Method Name", pwszGuidName);

   	IF_AUDIT_DO(OnMethodException)(pInfo->perfCount,
								   GuidToBstr(pInfo->guidApp),
								   id,						
								   bstrCLSID,						
								   bstrIID,			
								   pwszGuidName);


    delete [] pwszGuidName ;
    pwszGuidName = NULL ;
	return S_OK;
}

HRESULT CMethodSub::GetClsidOfTypeLib2 (IID * piid, UUID * puuidClsid)
{
	LPWSTR	pwszBuffIID				= NULL;
	HRESULT			hr						= S_OK;
	WCHAR			rgBufferIid [40];
	WCHAR			szBufferClsid[40];
	RPC_STATUS		RpcStatus;
	WCHAR			rgBufferKey [256];
	long			lRetVal;
	HKEY			hkey;
	
	//-------------------------------------------------------------------------
	//Convert the iid to it's string form
	RpcStatus = UuidToString ( piid,(unsigned short **)&pwszBuffIID);


	StringCchCopy(rgBufferIid,  ARRAYSIZE(rgBufferIid), pwszBuffIID);
	RpcStringFree ((unsigned short **)&pwszBuffIID);


	//-------------------------------------------------------------------------
	//Create the Registry key to do the lookup with
	StringCchCopy(rgBufferKey, ARRAYSIZE(rgBufferKey), L"Interface\\{");
	StringCchCat(rgBufferKey, ARRAYSIZE(rgBufferKey), rgBufferIid);
	StringCchCat(rgBufferKey, ARRAYSIZE(rgBufferKey), L"}\\TypeLib");

	//UNDONE -- gaganc Perhaps should handle the \interface\<iid1>\forward\<iid2|
	//case also.

	//-------------------------------------------------------------------------
	//Open the appropriate registry key
	//HKEY_CLASSES_ROOT\Interface\{<...interface iid ...>}\Typelib

	lRetVal = RegOpenKey	(
								HKEY_CLASSES_ROOT,
								rgBufferKey,
								&hkey								
							);

	if (lRetVal != ERROR_SUCCESS)
	{
		return E_FAIL;
	}



	//-------------------------------------------------------------------------
	//Obain the clsid from the open key
    DWORD cbBufferClsid = sizeof(szBufferClsid);
	lRetVal = RegQueryValueEx	(
									hkey,
									NULL,
									NULL,
									NULL,
									(LPBYTE)szBufferClsid,
									&cbBufferClsid
								);
	
	
    if(lRetVal != ERROR_SUCCESS)
    {
        RegCloseKey(hkey) ;
        return E_FAIL ;
    }


	//-------------------------------------------------------------------------
	//Get rid of the braces in the begining and the end.

	//Remove the trailing brace
	szBufferClsid[37] = '\0';

	//convert the string form of the clsid to a binary form. Plus 1 will 
	//remove the first brace.
	lRetVal = UuidFromString ((RPC_WSTR)(szBufferClsid + 1), puuidClsid);

    if(lRetVal != ERROR_SUCCESS)
    {
        RegCloseKey(hkey) ;
        return E_UNEXPECTED ;
    }        

	//-------------------------------------------------------------------------
	//Prepare to return

	lRetVal = RegCloseKey (hkey);

	return hr;
} //end GetClsidOfTypeLib


//----------------------------------------------------------------------------
// Function:	GetMethodName (INTERNAL FUNCTION)
// Description:	Takes an IID and a method index returns the corresponding 
//              method name.
//           
// Exceptions:	n/a
// Process:		
// Notes:		ppwszMethodName should be freed by the client using standard
//				C++ delete.  If hr != S_OK, ppwszMethodName will be set to
//				NULL.
//----------------------------------------------------------------------------
HRESULT CMethodSub::GetMethodName (REFIID riid, int iMeth, LPWSTR* ppwszMethodName)
{
	HRESULT			hr;
	UUID			uuidClsid;
	LCID			lcid		= 0x0;
	ITypeLib	*	pITypeLib	= NULL;
	ITypeInfo	*	pITypeInfo	= NULL;
    FUNCDESC    *   pFuncDesc   = 0x0;
    LPWSTR   pwszMethod   = NULL;
    BSTR            bstrName ;
    UINT            lcNames     = 0 ;

    IID * piid = (IID *) &riid;

	//Obtain the clsid of the typelib for this interface
	hr = GetClsidOfTypeLib2 (piid, &uuidClsid);

	if (hr != S_OK) goto errExit ;

	//-------------------------------------------------------------------------
	//Obtain the ITypeLib interface on the typelib
	hr = LoadRegTypeLib (
							uuidClsid,		//CLSID
							1,				//Major version number
							0,				//Minor version number
							0,			//the locale id
							&pITypeLib
						);

	// if the default system lcid (0) doesn't work, we try
	// LCID 9 (standard english) and if that doesn't work, we failfast
	if(FAILED(hr))
	{
		hr = LoadRegTypeLib (uuidClsid,	1,0,9,&pITypeLib);
	}


    if(FAILED(hr)) goto errExit ;

	hr = pITypeLib->GetTypeInfoOfGuid (*piid, &pITypeInfo);

    if(FAILED(hr)) goto errExit ;

	hr = pITypeInfo->GetFuncDesc(iMeth, &pFuncDesc) ;

    if(FAILED(hr)) goto errExit ;

    hr = pITypeInfo->GetNames(pFuncDesc->memid, &bstrName, 1, &lcNames) ;

    if(FAILED(hr)) goto errExit ;

    pwszMethod = new WCHAR[lstrlen(bstrName)+1] ;

    if(! pwszMethod)
    {
        hr = E_OUTOFMEMORY ;
        goto errExit ;
    }

    StringCchCopy(pwszMethod, lstrlen(bstrName)+1, bstrName) ;

    SysFreeString(bstrName) ;
    
errExit:    	
    
    if (pFuncDesc)
	{
		pITypeInfo->ReleaseFuncDesc(pFuncDesc) ;
	}


	if (pITypeInfo)
	{
		pITypeInfo->Release();
	}

    if (pITypeLib)
    {
        pITypeLib->Release() ;
    }

    *ppwszMethodName = pwszMethod ;

    return hr ;
} 



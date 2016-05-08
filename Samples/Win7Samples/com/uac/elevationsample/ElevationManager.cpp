// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// ElevationManager.cpp
//	-Implements functionality for elevation including using 
//	the elevation moniker and setting a button shield icon.
//


#include "stdafx.h"
#include "ElevationManager.h"
#include "..\RegisterServer\RegisterServer.h"
#include "..\RegisterServer\RegisterServer_i.c"

//
//  FUNCTION: ElevationManager(__out hr *)
//
//	PURPOSE: Constructor for elevation manager.  Calls CoInitializeEx.
//
//	COMMENTS:
//		Calling code should check the out parameter for Failure and cancel
//		further operation on class.
//
ElevationManager::ElevationManager(__out HRESULT * hr)
{
	CInitHR = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	*hr = CInitHR;
}

ElevationManager::~ElevationManager(void)
{
	if(SUCCEEDED(CInitHR))
		CoUninitialize();
}

//
//  FUNCTION: private RegisterAsDll(const TCHAR *, HWND, BOOL)
//
//  PURPOSE: Creates an instance of the specified IID in an elevated process.
//
//  COMMENTS:
//		tszFileName	- The filename to be registered
//		hwndMain	- The handle to the main window.
//		fRegister	- TRUE: register server.
//					- FALSE: unregister server.
//		
//		Uses CoCreateInstanceAsAdmin to load the IDllRegister Interface in an elevated process.
//		
//		Succeeds if registration is successful, fails otherwise.
//		Over The Shoulder (OTS) elevation will succeed in CoCreateInstanceAsAdmin with the correct credentials, but fail
//		with E_ACCESSDENIED if the registry key 'AppId/{CLSID}/AccessPermission' is not correctly set to allow the 
//		Interactive user. (See Register.cpp function SetOTSRegValue)
//		
HRESULT ElevationManager::RegisterAsDll(const TCHAR * tszFileName, HWND hwndMain, BOOL fRegister){
	
	HRESULT hr;
	IDllRegister * pDllRegister = NULL;
	TCHAR *tszFullFileName = NULL;
	BSTR bstrFileName = NULL;
	int bufLen = GetFullPathName(tszFileName, 0, NULL, NULL);

	tszFullFileName = (TCHAR *)malloc(sizeof(TCHAR) * bufLen);
	if(!tszFullFileName){
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	if(!GetFullPathName(tszFileName, bufLen, tszFullFileName, NULL)){
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto cleanup;
	}
	
	hr = CoCreateInstanceAsAdmin(hwndMain, CLSID_RegistrationClass, IID_IDllRegister, (void **) &pDllRegister);
	if(FAILED(hr)){
		goto cleanup;
	}

	bstrFileName = SysAllocString(tszFullFileName);
	
	if(!bstrFileName){
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	if(fRegister)
		hr = pDllRegister->RegisterDll(bstrFileName);
	else
		hr = pDllRegister->UnRegisterDll(bstrFileName);
	

cleanup:	
	free(tszFullFileName);
	SysFreeString(bstrFileName);
	if(pDllRegister)
		pDllRegister->Release();
	pDllRegister=NULL;
	
	return hr;
}

//
//  FUNCTION: private CoCreateInstanceAsAdmin(HWND, REFCLSID, REFIID, __out void**)
//
//  PURPOSE: Creates an instance of the specified IID in an elevated process us the COM Elevation Moniker.
//
//  COMMENTS:
//		hwndMain	- The Window associated with this call.
//		rclsid		- CLSID associated with the data and code that will be used to create the object. 
//		riid		- Reference to the identifier of the interface to be used to communicate with the object.
//		ppv			- [out] Address of pointer variable that receives the interface pointer requested in riid. 
//						Upon successful return, *ppv contains the requested interface pointer. 
//						Upon failure, *ppv contains NULL.
//
//		Fails if:	-User declines Elevation
//					-{CLSID}/Elevation/Enabled != 1 (See Register.cpp)
//					-{CLSID}/LocalizedString not set or invalid (See Register.cpp)
//
HRESULT ElevationManager::CoCreateInstanceAsAdmin(HWND hwndMain, __in REFCLSID rclsid, __in REFIID riid, __deref_out void ** ppv)
{
    BIND_OPTS3 bo;
	WCHAR  wszCLSID[CLSIDSize];
	WCHAR  wszMonikerName[MonikerSize+CLSIDSize];

    StringFromGUID2(rclsid, wszCLSID, ARRAYSIZE(wszCLSID)); 
    HRESULT hr = StringCchPrintf(wszMonikerName, ARRAYSIZE(wszMonikerName), MonikerName, wszCLSID);
    if (FAILED(hr))
        return hr;
    memset(&bo, 0, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.hwnd = hwndMain;
	bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
    hr = CoGetObject(wszMonikerName, &bo, riid, ppv);
	return hr;
}

//
//  FUNCTION: private RegisterAsExe(const TCHAR *, HWND, BOOL)
//
//  PURPOSE: Registers the file provided as an executable.
//
//  COMMENTS:
//		tszFileName - The file to be registered.
//		hwndMain - The Window associated with this call.
//		fRegister - TRUE: Register server
//					FALSE: Unregister server.
//		
//		This function does not get an elevated instance of the server, instead the server is responsible 
//			to elevate the executable using shell execute.
//
HRESULT ElevationManager::RegisterAsExe(const TCHAR * tszFileName, HWND hwndMain, BOOL fRegister){

	HRESULT hr;
	IExeRegister * pExeRegister = NULL;
	BSTR bstrFileName = NULL;

	hr = CoCreateInstance(CLSID_RegistrationClass, NULL, CLSCTX_INPROC, IID_IExeRegister, (void **) &pExeRegister);

	if(FAILED(hr))
		goto cleanup;
	

	bstrFileName = SysAllocString(tszFileName);

	if(!bstrFileName){
		hr = E_OUTOFMEMORY;
		goto cleanup;
	};

	if(fRegister)
		hr = pExeRegister->RegisterExe(bstrFileName, hwndMain);
	else
		hr = pExeRegister->UnregisterExe(bstrFileName, hwndMain);

cleanup:
	SysFreeString(bstrFileName);
	if(pExeRegister)
		pExeRegister->Release();
	pExeRegister = NULL;
	return hr;
}

//
//  FUNCTION: SetButtonShield(HWND)
//
//  PURPOSE: Places the Shield Icon on the button referenced by hwnd.
//
//  COMMENTS:
//		hwndButton - handle to the button that should recieve the shield icon.
//		This call is in it's own function here for clarity.
//		An application must be manifested to use comctl32.dll version 6 to gain this functionality:
//			In VS, add to ProjectProperties->Linker->Manifest File->Additional Manifest Dependencies:
//			"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'"
//
LRESULT ElevationManager::SetButtonShield(HWND hwndButton){

	return Button_SetElevationRequiredState(hwndButton, TRUE);

}

//
//  FUNCTION: UnregisterServer(const TCHAR *, HWND)
//
//  PURPOSE: Public entry point to Unregister the server provided.
//
//  COMMENTS:
//		tszFileName - The server to unregister.
//		hwndMain - the window associated with this call.
//		
//		Filename can be an exe or a library image.
//		
HRESULT ElevationManager::UnRegisterServer(const TCHAR * tszFileName, HWND hwndMain){

	BOOL isExe = IsFileExe(tszFileName);
	
	if(isExe)
		return RegisterAsExe(tszFileName, hwndMain, FALSE);
	else
		return RegisterAsDll(tszFileName, hwndMain, FALSE);


}

//
//  FUNCTION: RegisterServer(const TCHAR *, HWND)
//
//  PURPOSE: Public entry point to register the server provided.
//
//  COMMENTS:
//		tszFileName - The server to register.
//		hwndMain - the window associated with this call.
//		
//		Filename can be an exe or a library image.
//	
HRESULT ElevationManager::RegisterServer(const TCHAR * tszFileName, HWND hwndMain){
	
	BOOL isExe = IsFileExe(tszFileName);
	
	if(isExe)
		return RegisterAsExe(tszFileName, hwndMain, TRUE);
	else
		return RegisterAsDll(tszFileName, hwndMain, TRUE);

}

//
//  FUNCTION: private IsFileExe(const TCHAR *)
//
//  PURPOSE: Checks the file extension of the given file for '.exe'
//  COMMENTS:
//		tszFileName - The file name to check.		
//		Returns - TRUE if file is EXE, FALSE Otherwise.
BOOL ElevationManager::IsFileExe(const TCHAR * tszFileName){
	size_t len = _tcslen(tszFileName);
	size_t min = 5;
	const TCHAR * tszTemp = NULL;

	if(len < min)
		return FALSE;
	tszTemp = tszFileName + (len - 4);
	if(_tcscmp(tszTemp, _T(".exe"))==0)
		return TRUE;
	return FALSE;	
}



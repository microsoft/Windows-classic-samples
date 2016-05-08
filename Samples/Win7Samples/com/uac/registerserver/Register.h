// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once
#include "Windows.h"
#include "RegisterServer.h"
#include "RegisterServer_i.c"
#include <tchar.h>
#include <olectl.h>
#include <shellapi.h>
#include <sddl.h>

#define FILE_NAME			-1	//Registry value should be the dll file name.
#define OTS_SEC_DESC		-2  //Registry value should be a REG_BINARY security descriptor.
#define LOCALIZED_STRING	-3  //Registry value should be localized string recource (filename,-resourceID).
#define ELEVPROMPT_ICON		-4  //Registry value should be localized icon recource (filename,-resourceID).
#define DISPLAY_ICON_VALUE	-IDI_ICON
#define LOCALIZED_STRING_VALUE	-IDS_LOCALIZED
//provide byte address for elevated flag.
int Elevated = 1;

//Forward declarations for DllRegisterServer helper functions.
HRESULT SetOTSRegValue(HKEY, LPCWSTR);
BOOL GetAccessPermissionsForUACServer(SECURITY_DESCRIPTOR **);
BOOL SetAccessPermissions(HKEY, PSECURITY_DESCRIPTOR);

class RegistrationClass : public IClassFactory
{
public:
	RegistrationClass(void);
	~RegistrationClass(void);

	//IUnkown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	
	//IClassFactory
	STDMETHODIMP  CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
	STDMETHODIMP LockServer(BOOL bLock) { return S_FALSE; }

};

class RegisterImpl :	
	public IDllRegister,
	public IExeRegister{
	LONG m_ref;
public:
	RegisterImpl(void);
	~RegisterImpl(void);

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID, void **);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IDllRegister
	//RequiresElevation
	STDMETHODIMP RegisterDll(BSTR);
	STDMETHODIMP UnRegisterDll(BSTR);
	//IExeRegister
	STDMETHODIMP RegisterExe(BSTR, HWND);
	STDMETHODIMP UnregisterExe(BSTR, HWND);
private:
	STDMETHODIMP OpenAndExecuteDll(BSTR, LPCSTR);
	STDMETHODIMP VerifyAndExecuteExe(BSTR, HWND, const LPCTSTR);
};

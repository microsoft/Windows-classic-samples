// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Register.cpp		
//	-Implementation of IRegisterExe, IRegisterDll, and RegistrationClass, demonstrates elevation through ShellExecute in function VerifyAndExecuteExe
//	-Demonstrates registry keys required for elevation including:
//							{CLSID}\LocalizedString,	 (required)
//							{CLSID}\Elevation\Enabled,	 (required)
//							{CLSID}\Elevation\IconReference, (optional)
//							{AppId}\AccessPermission	 (optional, required for Over The Shoulder Elevation)  	

#include "Register.h"
#include "resource.h"



//Table of registry keys.
const struct{
	LPCTSTR tszKeyName;		//Registry Key Name
	LPCTSTR tszValueName;	//Registry Value Name
	byte * lpbValue;		//Value
	int regType;			//Registry type
} regEntries []= {
	//Register Class HelloWorld
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}"), 0 , (byte *)_T("ElevationSample"), REG_SZ}, 
	//Location of DLL, use LocalServer32 to register .exe Classes
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}\\InprocServer32"), 0, (byte *)FILE_NAME, REG_SZ},
	//Threading Model
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}\\InprocServer32"), _T("ThreadingModel"), (byte *)_T("Free"), REG_SZ},
	//Register CLSID to ProgID
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}\\ProgID"), 0, (byte *)_T("Elevation.RegServer.1"), REG_SZ},
	//Register ProgID to CLSID
	{_T("SOFTWARE\\CLASSES\\Test.HelloWorld.1"), 0,(byte *) _T("ElevationSample"), REG_SZ},
	{_T("SOFTWARE\\CLASSES\\Test.HelloWorld.1\\CLSID"), 0, (byte *)_T("{64009241-4306-4971-B6E9-184CA1208F06}"), REG_SZ},
	//Enable Elevation
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}\\Elevation") 
	, _T("Enabled"), (byte *)&Elevated, REG_DWORD},
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}")
	, _T("LocalizedString"), (byte *)LOCALIZED_STRING, REG_EXPAND_SZ},
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}\\Elevation")
	, _T("IconReference"), (byte *)ELEVPROMPT_ICON, REG_EXPAND_SZ},
	//Register for DLL Surrogate
	{_T("SOFTWARE\\CLASSES\\CLSID\\{64009241-4306-4971-B6E9-184CA1208F06}"), _T("AppID"), (byte *)_T("{64009241-4306-4971-B6E9-184CA1208F06}"), REG_SZ},
	{_T("SOFTWARE\\CLASSES\\AppID\\{64009241-4306-4971-B6E9-184CA1208F06}"), _T("DllSurrogate"), (byte *)_T(""), REG_SZ},
	//Register for Over the Shoulder (OTS) Activation
	{_T("SOFTWARE\\CLASSES\\AppID\\{64009241-4306-4971-B6E9-184CA1208F06}"), _T("AccessPermission"), (byte *)OTS_SEC_DESC, REG_BINARY}
};


//
//   FUNCTION:  DllRegisterServer(void)
//
//   PURPOSE: Exported for server self registration.	
//
STDAPI DllRegisterServer(void)
 
 {
	HRESULT hr = S_OK;
	HKEY hkey = NULL;
	TCHAR szFileName[MAX_PATH];
	TCHAR szResources[MAX_PATH + 7]; //PATH,-00000
	const TCHAR * tszKeyName;
	const TCHAR * tszValueName;
	byte * tszValue;
	int iRegType,
		numEntries = RTL_NUMBER_OF(regEntries),
		i = 0;
	DWORD err = 0;
	HMODULE phModule;

	if(		!GetModuleHandleEx(0, _T("RegisterServer"), &phModule)
		||	!GetModuleFileName(phModule, szFileName, MAX_PATH)){
		return SELFREG_E_CLASS;
	}
	//Register Entries
	
	for ( i = 0; i < numEntries  && SUCCEEDED(hr) ; i++){
		tszKeyName = regEntries[i].tszKeyName;
		tszValueName = regEntries[i].tszValueName;
		tszValue = regEntries[i].lpbValue;
		iRegType = regEntries[i].regType;

		if(tszValue == (byte *)FILE_NAME){
			//value should be the filepath.
			tszValue = (byte *)szFileName;
		}else if(tszValue == (byte *) ELEVPROMPT_ICON){
			//value should be localized icon.
			_sntprintf_s(szResources, MAX_PATH + 7, _T("@%s,%d"), szFileName, DISPLAY_ICON_VALUE);
			tszValue = (byte *)szResources;
		}else if(tszValue == (byte *) LOCALIZED_STRING){
			//value should be localized string
			_sntprintf_s(szResources, MAX_PATH + 7, _T("@%s,%d"), szFileName, LOCALIZED_STRING_VALUE);
			tszValue = (byte *)szResources;
		}else if(tszValue == (byte *) OTS_SEC_DESC){
			//value should be security descriptor for OTS
			hr = SetOTSRegValue(HKEY_LOCAL_MACHINE, tszKeyName);
			continue;
		}

		err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, tszKeyName ,0, NULL,  
								REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 
									NULL,   &hkey,  NULL);
		if(err == ERROR_SUCCESS){
			err = RegSetValueEx(hkey, tszValueName,    0,  iRegType, tszValue, (DWORD)(wcslen((const wchar_t*)tszValue) + 1)*2);
			RegCloseKey(hkey);
		
		}
		
		if(err != ERROR_SUCCESS){
			hr = HRESULT_FROM_WIN32(err);
			
		}
		
	}
	
	return hr;
}


//
//   FUNCTION:  SetOTSRegValue(HKEY, LPCWSTR)
//
//   PURPOSE: Helper function for enabling Over the shoulder elevation at registration.
//
//   COMMENTS:
//		hKey - A handle to an open registry key as in RegCreateKeyEx.
//		tszKeyName	- The subkey location for AccessPermission
//		Registers the security descritpor for GetAccessPermissionsForUACServer,
//			opens Registry Key and calls SetAccessPermisions to set it.
//			
//
HRESULT SetOTSRegValue(HKEY hKey, LPCWSTR tszKeyName){
	HRESULT hr = S_OK;
	SECURITY_DESCRIPTOR * psd = NULL;
	HKEY hSubKey = NULL;
	DWORD err = 0;

	if(!GetAccessPermissionsForUACServer(&psd)){
		hr =  E_ACCESSDENIED;
		goto cleanup;
	}
	
	err =  RegCreateKeyEx(hKey, tszKeyName,0, NULL,  
									REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, 
										NULL,   &hSubKey,  NULL); 
	if(err != ERROR_SUCCESS){
		hr = HRESULT_FROM_WIN32(err);
		goto cleanup;
	}

	if(!SetAccessPermissions(hSubKey, psd)){
		hr = E_ACCESSDENIED;
		goto cleanup;
	}
cleanup:
	RegCloseKey(hSubKey);
	LocalFree(psd);
	return hr;
}


//
//   FUNCTION: GetAccessPermissionsForUACServer(SECURITY_DESCRIPTOR **)
//
//   PURPOSE: Helper function for Getting a security descriptor that allows
//		access to the Interactive user.
//
//   COMMENTS:
//		 ppSD [out]	- a reference to the SD pointer that will contain the SD upon SUCCESS.  
//
//		As per ConvertStringSecurityDescriptorToSecurityDescriptor, the returned SD must
//			be released with LocalFree
//		Returns	- TRUE on success, FALSE otherwise
//
BOOL GetAccessPermissionsForUACServer(SECURITY_DESCRIPTOR **ppSD)
{
// Local call permissions to IU, SY
	LPTSTR lptszSDDL = _T("O:BAG:BAD:(A;;0x3;;;IU)(A;;0x3;;;SY)");
	SECURITY_DESCRIPTOR *pSD = NULL;
	*ppSD = NULL;

	if (ConvertStringSecurityDescriptorToSecurityDescriptor(lptszSDDL, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&pSD, NULL))
	{
		*ppSD = pSD;
		return TRUE;
	}

	return FALSE;
}


//
//   FUNCTION: SetAccessPermissions(HKEY, PSECURITY_DESCRIPTOR)
//
//   PURPOSE: Helper function for setting the registry key AccessPermission with
//			a binary representation of the Security descriptor retrieved by 
//			GetAccessPermissionsForUACServer.
//
//   COMMENTS:
//		hkey	- The handle to an open registry key where access permissions will be set		 
//		pSD		- a reference to the SD to be written to the registry.  
//
//		Returns - TRUE on success, FALSE otherwise.	
//
BOOL SetAccessPermissions(HKEY hKey, PSECURITY_DESCRIPTOR pSD)
{
	BOOL bResult = FALSE;
	DWORD dwLen = GetSecurityDescriptorLength(pSD);
	LONG lResult;
	lResult = RegSetValueEx(hKey, 
		_T("AccessPermission"),
		0,
		REG_BINARY,
		(BYTE*)pSD,
		dwLen);
	
	if (lResult != ERROR_SUCCESS) goto done;
	bResult = TRUE;
done:
	return bResult;
}

//
//   FUNCTION:  DllUnregisterServer(void)
//
//   PURPOSE: Exported for server self unregistration.	
//
STDAPI DllUnregisterServer(void){
	HRESULT hr = S_OK;
	DWORD err = 0;
	int numEntries = RTL_NUMBER_OF(regEntries);
	HKEY hkey;

	for(int i = numEntries - 1; i > -1 && SUCCEEDED(hr) ; i -- ){
		err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCWSTR)regEntries[i].tszKeyName, NULL, DELETE, &hkey);
		if(err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND){
			hr = HRESULT_FROM_WIN32(err);
			break;
		}
		err = RegDeleteKey(HKEY_LOCAL_MACHINE, (LPCWSTR)regEntries[i].tszKeyName);
		if(err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND){
			hr = HRESULT_FROM_WIN32(err);
			break;
		}
	}
	
	return hr;
	
}

//
//   FUNCTION: DllGetClassObject(REFCLSID, REFIID, LPVOID* ppv)
//
//   PURPOSE: Exported to get class objects hosted by this server.
//
//	 COMMENTS:
//		This server only supports CLSID_RegistrationClass.
//
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if(rclsid != CLSID_RegistrationClass)
		return CLASS_E_CLASSNOTAVAILABLE;
	
	static RegistrationClass pRegClass;

	return pRegClass.QueryInterface (riid, ppv);
}

//
//   FUNCTION:  DllMain( HINSTANCE, DWORD, LPVOID)
//
//   PURPOSE: Exported main function for DLL.
//
//	 COMMENTS: Does nothing for this DLL.
//
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved
){return true;};


/*** RegisterImpl ***/
RegisterImpl::RegisterImpl(void): m_ref(0){}

RegisterImpl::~RegisterImpl(void){}


/*** IDllRegister ***/
//
//   FUNCTION: RegisterDll(void)
//
//   PURPOSE: Public entrypoint into the interface for registering a DLL.  
//				Uses OpenAndExecuteDll with the standard DLL Export 
//				function DllRegisterServer to register the DLL.
//
//	 COMMENTS:
//		bstrFileName - the filepath of the DLL to unregister.
//		
//		Requires Interface to be created elevated if DllRegisterServer
//		expects administrative rights.
//
//		Returns - E_INVALIDARG if the file does not exist, is not a valid DLL image
//					or 'DllRegisterServer is not exported by the DLL.
//				  S_FALSE if the DLL is not unloaded correctly, to get extended error information
//						call GetLastError.
//				  The HRESULT of DllRegisterServer otherwise.
//
STDMETHODIMP RegisterImpl::RegisterDll(BSTR bstrFileName){
	return OpenAndExecuteDll(bstrFileName, "DllRegisterServer");
}

//
//   FUNCTION: UnRegisterDll(void)
//
//   PURPOSE: Public entrypoint into the interface for unregistering a DLL.  
//				Uses OpenAndExecuteDll with the standard DLL Export 
//				function DllUnregisterServer to register the DLL.
//
//	 COMMENTS:
//		bstrFileName - the filepath of the DLL to unregister.
//		
//		Requires Interface to be created elevated if DllUnregisterServer
//		expects administrative rights.
//		
//		Returns - E_INVALIDARG if the file does not exist, is not a valid DLL image
//					or 'DllUnregisterServer is not exported by the DLL.
//				  S_FALSE if the DLL is not unloaded correctly, to get extended error information
//						call GetLastError.
//				  The HRESULT of DllUnregisterServer otherwise.
//
STDMETHODIMP RegisterImpl::UnRegisterDll(BSTR bstrFileName){
	return OpenAndExecuteDll(bstrFileName, "DllUnregisterServer");
}


//
//   FUNCTION: private OpenAndExecuteDll(BSTR, LPCSTR)
//
//   PURPOSE: Loads the library and calls the function name passed if it is exported 
//			by the library.
//
//	 COMMENTS:
//		See (Un)RegisterDll for expected return values.
//	
//
STDMETHODIMP RegisterImpl::OpenAndExecuteDll(BSTR bstrFileName, LPCSTR function){
	HMODULE hmodDll = LoadLibraryEx(bstrFileName, NULL, 0);
	FARPROC fprocDllReg;
	HRESULT hr;
	DWORD dw;

	if(!hmodDll)
		return HRESULT_FROM_WIN32(GetLastError());
	fprocDllReg = GetProcAddress(hmodDll, function);
	if(!fprocDllReg){
		dw = GetLastError();
		FreeLibrary(hmodDll);
		return HRESULT_FROM_WIN32(dw);	
	}

	hr = fprocDllReg();
	
	FreeLibrary(hmodDll);
		
	return hr;
}

/*** IExeRegister ***/

//
//   FUNCTION: RegisterExe(void)
//
//   PURPOSE: Public entrypoint into the interface for registering an EXE.  
//				Uses VerifyAndExecuteExe with the command line argument /RegServer to
//				register the exe.
//
//	 COMMENTS:
//		bstrFileName - the filepath of the EXE to unregister.
//		hwndWindow - the window associated with execution, can be NULL.
//
//		Does not require an elevated instance of the interface. Instead Shell Execute will 
//			start the EXE in an elevated process.
//		
//		Returns - E_INVALIDARG if the file does not exist, or is not a valid EXE image.
//				  E_FAIL if the EXE cannot be executed, call GetLastError for more information.
//				  S_OK if the EXE is executed.
//
STDMETHODIMP RegisterImpl::RegisterExe(BSTR bstrFileName, HWND hwndWindow){
	return VerifyAndExecuteExe(bstrFileName, hwndWindow, _T("/RegServer"));
}

//
//   FUNCTION: UnregisterExe(void)
//
//   PURPOSE: Public entrypoint into the interface for registering an EXE.  
//				Uses VerifyAndExecuteExe with the command line argument /UnregServer to
//				unregister the exe.
//
//	 COMMENTS:
//		bstrFileName - the filepath of the EXE to unregister.
//		hwndWindow - the window associated with execution, can be NULL.
//
//		Does not require an elevated instance of the interface. Instead Shell Execute will 
//			start the EXE in an elevated process.
//		
//		Returns - E_INVALIDARG if the file does not exist, or is not a valid EXE image.
//				  E_FAIL if the EXE cannot be executed, call GetLastError for more information.
//				  S_OK if the EXE is executed.
//
STDMETHODIMP RegisterImpl::UnregisterExe(BSTR bstrFileName, HWND hwndWindow){
	return VerifyAndExecuteExe(bstrFileName, hwndWindow, _T("/UnregServer"));
	
}

//
//   FUNCTION: private VerifyAndExecuteExe(BSTR, HWND, LPCSTR)
//
//   PURPOSE: Runs the given file name as an executable, requesting elevated permissions using ShellExecuteEx.
//
//
STDMETHODIMP RegisterImpl::VerifyAndExecuteExe(BSTR bstrFileName, HWND hwndWindow, const LPCTSTR lpParam){
	SHELLEXECUTEINFO sei;
	ZeroMemory(&sei, sizeof(sei));

	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_FLAG_NO_UI;
	sei.hwnd = hwndWindow;
	sei.lpVerb = _T("runas");
	sei.lpFile = bstrFileName;
	sei.lpParameters = lpParam;
	sei.nShow = SW_HIDE;
	sei.lpDirectory = NULL;
	
	if(!ShellExecuteEx(&sei))
		return HRESULT_FROM_WIN32(GetLastError());
	else
		return S_OK;
}

/*** IUnknown ***/
//
//   FUNCTION: QueryInterface(REFIID, void **)
//
//   PURPOSE: Standard IUnknown implementation.
//
//	 COMMENTS: This classes supports the following interfaces:
//				IUnknown
//				IExeRegister
//				IDllRegister
//
STDMETHODIMP  RegisterImpl::QueryInterface(REFIID riid, void ** ppv){

	*ppv = NULL;

	if(riid == IID_IUnknown || riid == IID_IExeRegister)
		*ppv = static_cast<IExeRegister*>(this);
	else if (riid == IID_IDllRegister)
		*ppv = static_cast<IDllRegister*>(this);

	if(*ppv == NULL){
			return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv) -> AddRef();
	return S_OK;
}

//
//   FUNCTION: AddRef(void)
//
//   PURPOSE: Standard IUnknown implementation.
//
STDMETHODIMP_(ULONG)  RegisterImpl::AddRef(void){
	return InterlockedIncrement(&m_ref);
}

//
//   FUNCTION: Release(void)
//
//   PURPOSE: Standard IUnknown implementation.
//
STDMETHODIMP_(ULONG) RegisterImpl::Release(void){
	LONG count = InterlockedDecrement(&m_ref);
	if(count == 0){
		delete this;
	}
	return count;
}

/*** end-RegisterImpl ***/

/*** RegistrationClass ***/
RegistrationClass::RegistrationClass(void)
{
}

RegistrationClass::~RegistrationClass(void)
{
}

//
//   FUNCTION:  QueryInterface(REFIID, void **)
//
//   PURPOSE: Standard IUnknown implementation.
//
//	 COMMENTS:
//		This server only supports CLSID_RegistrationClass.
//
STDMETHODIMP RegistrationClass::QueryInterface(REFIID riid, void **ppv){
		if(riid == IID_IClassFactory || riid ==IID_IUnknown)
			*ppv =  static_cast<IClassFactory*>(this);
		else{
			*ppv = 0;
			return E_NOINTERFACE;
		}

		reinterpret_cast<IUnknown*>(*ppv) -> AddRef();
		return S_OK;
	}

//
//   FUNCTION:  AddRef(void)
//
//   PURPOSE: Standard IUnknown implementation.
//
STDMETHODIMP_(ULONG) RegistrationClass::AddRef(void){
		return 1; 
	}

//
//   FUNCTION:  Release(void)
//
//   PURPOSE: Standard IUnknown implementation.
//
STDMETHODIMP_(ULONG) RegistrationClass::Release(void){
		return 2;
	}

//
//   FUNCTION:  CreateInstance(IUnknown *, REFIID, void **)
//
//   PURPOSE: Standard IClassFactory implementation.
//
STDMETHODIMP  RegistrationClass::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv){
		*ppv =  NULL;
		if(pUnkOuter !=NULL)
			return CLASS_E_NOAGGREGATION;

		RegisterImpl *hwp = new RegisterImpl();
		if(hwp == NULL)
			return E_OUTOFMEMORY;
		hwp->AddRef();
		
		HRESULT hr = hwp->QueryInterface(riid, ppv);

		hwp->Release();

		return hr;
	}

/*** end-RegistrationClass ***/


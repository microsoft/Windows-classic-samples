//--------------------------------------------------------------------
// Microsoft OLE DB Test
//	
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module Modstub.cpp | Provides common DLL implementation
//


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "MODStandard.hpp"
#include <stdio.h>			//sprintf



/////////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////////
void SetGlobalModuleData(void);
CThisTestModule* g_pThisTestModule = NULL;

static DWORD sg_dwProcCount = 0;
static HMODULE g_hInstance = NULL;

#define MAX_NAME_LEN 4096



/////////////////////////////////////////////////////////////////////////////
// DllMain
//
/////////////////////////////////////////////////////////////////////////////
extern "C" int APIENTRY DllMain(

	HINSTANCE hInstance, 			
	DWORD dwReason, 				
	LPVOID lpReserved)				

{
	switch(dwReason) 
	{
		case DLL_PROCESS_ATTACH:
			g_hInstance = hInstance;
			SetGlobalModuleData();
			sg_dwProcCount++;
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			if(--sg_dwProcCount == 0)
			{
				if(g_pThisTestModule)
					delete g_pThisTestModule;
			}
			break;

		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// DllGetClassObject
//
/////////////////////////////////////////////////////////////////////////////
STDAPI  DllGetClassObject(

	REFCLSID rclsid, 			
	REFIID riid, 				
	LPVOID * ppv)				

{
	*ppv = NULL;
	GlobalModuleData* pModuleData = &g_pThisTestModule->m_gmd;

	//Validate that this dll can support the requested CLSID
	//We allow CLSID_NULL so LTM can load this dll and find the CLSID to store into
	//the registry to register it with LTM...
	if(rclsid != *pModuleData->m_pguidModuleCLSID && rclsid != CLSID_NULL)
		return REGDB_E_CLASSNOTREG;

    //Validate that this can support the requested interface which can be
    //IUnknown or IClassFactory
	if(riid != IID_IUnknown && riid != IID_IClassFactory)
    	return E_NOINTERFACE;
    
    //Instantiate the class factory. 
	*ppv = new CTestModuleClassFactory(&g_pThisTestModule->m_gmd);
	if(!*ppv)
    	return E_OUTOFMEMORY;
    
	((IUnknown *)*ppv)->AddRef();
	return S_OK;
} 



/////////////////////////////////////////////////////////////////////////////
// DllCanUnloadNow
//
/////////////////////////////////////////////////////////////////////////////
STDAPI DllCanUnloadNow(void)
{  
	return sg_dwProcCount ? S_FALSE : S_OK;
} 



/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer
//
/////////////////////////////////////////////////////////////////////////////
STDAPI DllUnregisterServer(void)
{
	HKEY hRootKey = NULL;
	HKEY hModuleKey = NULL;

	HKEY hLTMKey = NULL;
	HKEY hLTMModuleKey = NULL;
	
	WCHAR* pwszCLSID = NULL;
	ULONG cBufferSize = MAX_NAME_LEN;
	GlobalModuleData* pModuleData = &g_pThisTestModule->m_gmd;
	CHAR szGuid[MAX_NAME_LEN];
	CHAR szBuffer[MAX_NAME_LEN];
	CHAR szBuffer2[MAX_NAME_LEN];
	
	//{...Guid...}
	StringFromCLSID(*pModuleData->m_pguidModuleCLSID, &pwszCLSID);
	WideCharToMultiByte(CP_ACP, 0, pwszCLSID, -1, szGuid, MAX_NAME_LEN, NULL, NULL);

	// Step 1: Remove our CLSID as an OLE Server
	//HKEY_CLASSES_ROOT\CLSID
	if(ERROR_SUCCESS == RegOpenKeyExA(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_WRITE, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\CLSID\{..Guid..}
		if(ERROR_SUCCESS == RegOpenKeyExA(hRootKey, szGuid, 0, KEY_WRITE, &hModuleKey))
		{
			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\InprocServer32
			RegDeleteKeyA(hModuleKey, "InprocServer32");
			RegDeleteKeyA(hModuleKey, "ProgID");
		}
		RegDeleteKeyA(hRootKey, szGuid);
		RegCloseKey(hRootKey);
	}

	//HKEY_CLASSES_ROOT\ProgID
	WideCharToMultiByte(CP_ACP, 0, pModuleData->m_wszModuleName, -1, szBuffer2, MAX_NAME_LEN, NULL, NULL);
	strcpy(szBuffer, "LTMTest.");
	strcat(szBuffer, szBuffer2);
	if(ERROR_SUCCESS == RegOpenKeyExA(HKEY_CLASSES_ROOT, szBuffer, 0, KEY_WRITE, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\ProgID\CLSID
		RegDeleteKeyA(hRootKey, "CLSID");
		RegDeleteKey(hRootKey, NULL);
	}

	// Step 2: Remove LTM-specific registery entries 
	//Obtain the Key for HKEY_LOCAL_MACHINE\"SOFTWARE\Microsoft\LTM\Test Modules"
	if(ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\LTM\\Test Modules", 0, KEY_WRITE, &hLTMKey))
	{
		//Obtain the Key for "...\{Guid}
		if(ERROR_SUCCESS == RegOpenKeyExA(hLTMKey, szGuid, 0, KEY_WRITE | KEY_READ, &hLTMModuleKey))
		{
			while(RegEnumKeyExA(hLTMModuleKey, 0, szBuffer, &cBufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				HKEY hLTMCase = NULL;
				if(ERROR_SUCCESS == RegOpenKeyExA(hLTMModuleKey, szBuffer, 0, KEY_WRITE | KEY_READ, &hLTMCase))
				{
					cBufferSize = MAX_NAME_LEN;
					while(RegEnumKeyExA(hLTMCase, 0, szBuffer2, &cBufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
					{
						RegDeleteKeyA(hLTMCase, szBuffer2);
						cBufferSize = MAX_NAME_LEN;
					}

					RegCloseKey(hLTMCase); hLTMCase = NULL;
					RegDeleteKeyA(hLTMModuleKey, szBuffer);
					cBufferSize = MAX_NAME_LEN;
				}
			}
			RegDeleteKeyA(hLTMKey, szGuid);
		}
	}

	RegCloseKey(hLTMKey);
	RegCloseKey(hLTMModuleKey);

	RegCloseKey(hRootKey);
	RegCloseKey(hModuleKey);
	CoTaskMemFree(pwszCLSID);
	return S_OK;
}



/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer
//
/////////////////////////////////////////////////////////////////////////////
STDAPI DllRegisterServer(void)
{
	HRESULT hr = E_FAIL;

	HKEY hLTMKey = NULL;
	HKEY hLTMModuleKey = NULL;
	
	HKEY hRootKey = NULL;
	HKEY hModuleKey = NULL;
	HKEY hInprocServerKey = NULL;
	HKEY hProgIDKey = NULL;
	
	WCHAR* pwszCLSID = NULL;
	GlobalModuleData* pModuleData = &g_pThisTestModule->m_gmd;
	CHAR szModuleFileName[MAX_NAME_LEN];
	CHAR szProgID[MAX_NAME_LEN];
	CHAR szModuleName[MAX_NAME_LEN];
	CHAR szGuid[MAX_NAME_LEN];

	//Unregister First
	DllUnregisterServer();
	
	//{...Guid...}
	StringFromCLSID(*pModuleData->m_pguidModuleCLSID, &pwszCLSID);
	WideCharToMultiByte(CP_ACP, 0, pwszCLSID, -1, szGuid, MAX_NAME_LEN, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, pModuleData->m_wszModuleName, -1, szModuleName, MAX_NAME_LEN, NULL, NULL);

	strcpy(szProgID, "LTMTest.");
	strcat(szProgID, szModuleName);

	// Step 1: Register our CLSID as an OLE Server
	//HKEY_CLASSES_ROOT\CLSID
	if(ERROR_SUCCESS == RegCreateKeyA(HKEY_CLASSES_ROOT, "CLSID", &hRootKey))
	{
		//HKEY_CLASSES_ROOT\CLSID\{..Guid..}
		if(ERROR_SUCCESS == RegCreateKeyA(hRootKey, szGuid, &hModuleKey))
		{
			//{Guid}\Value=ModuleName
			RegSetValueEx(hModuleKey, NULL, 0, REG_SZ, (BYTE*)szModuleName, ((DWORD)strlen(szModuleName)) * sizeof(CHAR));

			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\InprocServer32
			if(ERROR_SUCCESS == RegCreateKeyA(hModuleKey, "InprocServer32", &hInprocServerKey))
			{
				//InprocServer32\<>\FileName.dll
				GetModuleFileNameA(g_hInstance, szModuleFileName, MAX_NAME_LEN);
				RegSetValueExA(hInprocServerKey, NULL, 0, REG_SZ, (BYTE*)szModuleFileName, ((DWORD)strlen(szModuleFileName)) * sizeof(CHAR));
				
				//InprocServer32\ThreadingModel\Both
				RegSetValueExA(hInprocServerKey, "ThreadingModel", 0, REG_SZ, (BYTE*)"Both", ((DWORD)strlen("Both")) * sizeof(CHAR));
				RegCloseKey(hInprocServerKey);
				hr = S_OK;
			}

			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\ProgID
			if(ERROR_SUCCESS == RegCreateKeyA(hModuleKey, "ProgID", &hProgIDKey))
			{
				RegSetValueExA(hProgIDKey, NULL, 0, REG_SZ, (BYTE*)szProgID, ((DWORD)strlen(szProgID)) * sizeof(CHAR));
				RegCloseKey(hProgIDKey);
			}
			RegCloseKey(hModuleKey);
		}
		RegCloseKey(hRootKey);
	}


	//HKEY_CLASSES_ROOT\ProgID
	if(ERROR_SUCCESS == RegCreateKeyA(HKEY_CLASSES_ROOT, szProgID, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\ProgID\CLSID
		if(ERROR_SUCCESS == RegCreateKeyA(hRootKey, "CLSID", &hModuleKey))
		{
			//ProgID\CLSID\{Guid}
			RegSetValueExA(hModuleKey, NULL, 0, REG_SZ, (BYTE*)szGuid, ((DWORD)strlen(szGuid)) * sizeof(CHAR));
			RegCloseKey(hModuleKey);
			hr = S_OK;
		}
		RegCloseKey(hRootKey);
	}
	

	//Step 2: Register our test in the LTM Test Location
	//So LTM is aware of a new test module is available for running...
	
	//Obtain the Key for HKEY_LOCAL_MACHINE\"SOFTWARE\Microsoft\LTM\Test Modules"
	if(ERROR_SUCCESS == RegCreateKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\LTM\\Test Modules", &hLTMKey))
	{

		//Obtain the Key for "...\{Guid}
		if(ERROR_SUCCESS == RegCreateKeyA(hLTMKey, szGuid, &hLTMModuleKey))
		{
			//ModuleName
			RegSetValueExA(hLTMModuleKey, NULL, 0, REG_SZ, (BYTE*)szModuleName, ((DWORD)strlen(szModuleName)) * sizeof(CHAR));
		}
	}

	RegCloseKey(hLTMKey);
	RegCloseKey(hLTMModuleKey);
	CoTaskMemFree(pwszCLSID);
	return hr;
}




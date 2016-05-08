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
	HRESULT				hr = S_OK;
	HKEY				hRootKey = NULL;
	HKEY				hModuleKey = NULL;

	HKEY				hLTMKey = NULL;
	HKEY				hLTMModuleKey = NULL;
	
	WCHAR				*pwszCLSID = NULL;
	ULONG				cBufferSize = MAX_NAME_LEN;
	CHAR				szGuid[MAX_NAME_LEN];
	CHAR				szBuffer[MAX_NAME_LEN];
	CHAR				szBuffer2[MAX_NAME_LEN];

	CHAR				strFileName[MAX_PATH + 1];
	WCHAR				wszFileName[MAX_PATH + 1];

	GlobalModuleData	*pModuleData = &g_pThisTestModule->m_gmd;
	
	ITypeLib			*pITypeLib = NULL;
	TLIBATTR			*pTLibAttr = NULL;

	// The instance handle of a DLL can be used as module handle (see DllMain doc)
	if(!GetModuleFileName(g_hInstance, strFileName, sizeof(strFileName)/sizeof(CHAR)))
		return E_FAIL;
	
	//{...Guid...}
	StringFromCLSID(*pModuleData->m_pguidModuleCLSID, &pwszCLSID);
	WideCharToMultiByte(CP_ACP, 0, pwszCLSID, -1, szGuid, MAX_NAME_LEN, NULL, NULL);

	// Step 1: Remove our CLSID as an OLE Server
	//HKEY_CLASSES_ROOT\CLSID
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_WRITE, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\CLSID\{..Guid..}
		if(ERROR_SUCCESS == RegOpenKeyEx(hRootKey, szGuid, 0, KEY_WRITE, &hModuleKey))
		{
			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\InprocServer32
			RegDeleteKey(hModuleKey, "InprocServer32");
			RegDeleteKey(hModuleKey, "ProgID");
		}
		RegDeleteKey(hRootKey, szGuid);
		RegCloseKey(hRootKey);
	}

	//HKEY_CLASSES_ROOT\ProgID
	WideCharToMultiByte(CP_ACP, 0, pModuleData->m_wszModuleName, -1, szBuffer2, MAX_NAME_LEN, NULL, NULL);
	strcpy(szBuffer, "LTMTest.");
	strcat(szBuffer, szBuffer2);
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, KEY_WRITE, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\ProgID\CLSID
		RegDeleteKey(hRootKey, "CLSID");
		RegDeleteKey(hRootKey, NULL);
	}

	// Step 2: Remove LTM-specific registery entries 
	//Obtain the Key for HKEY_LOCAL_MACHINE\"SOFTWARE\Microsoft\LTM\Test Modules"
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\LTM\\Test Modules", 0, KEY_WRITE, &hLTMKey))
	{
		//Obtain the Key for "...\{Guid}
		if(ERROR_SUCCESS == RegOpenKeyEx(hLTMKey, szGuid, 0, KEY_WRITE | KEY_READ, &hLTMModuleKey))
		{
			while(RegEnumKeyEx(hLTMModuleKey, 0, szBuffer, &cBufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				HKEY hLTMCase = NULL;
				if(ERROR_SUCCESS == RegOpenKeyEx(hLTMModuleKey, szBuffer, 0, KEY_WRITE | KEY_READ, &hLTMCase))
				{
					cBufferSize = MAX_NAME_LEN;
					while(RegEnumKeyEx(hLTMCase, 0, szBuffer2, &cBufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
					{
						RegDeleteKey(hLTMCase, szBuffer2);
						cBufferSize = MAX_NAME_LEN;
					}

					RegCloseKey(hLTMCase); hLTMCase = NULL;
					RegDeleteKey(hLTMModuleKey, szBuffer);
					cBufferSize = MAX_NAME_LEN;
				}
			}
			RegDeleteKey(hLTMKey, szGuid);
		}
	}

	RegCloseKey(hLTMKey);
	RegCloseKey(hLTMModuleKey);

	RegCloseKey(hRootKey);
	RegCloseKey(hModuleKey);
	CoTaskMemFree(pwszCLSID);

	//convert to wide char
	if(!MultiByteToWideChar(CP_ACP, NULL, strFileName, -1, wszFileName, MAX_PATH))
	{
		return E_FAIL;
	}

	if (SUCCEEDED(hr = LoadTypeLib(wszFileName, &pITypeLib)))
	{
		hr = pITypeLib->GetLibAttr(&pTLibAttr);
		if (S_OK == hr)
		{
			hr = UnRegisterTypeLib(pTLibAttr->guid, 
					pTLibAttr->wMajorVerNum, pTLibAttr->wMinorVerNum, 
					pTLibAttr->lcid, pTLibAttr->syskind);
			pITypeLib->ReleaseTLibAttr(pTLibAttr);
		}
	}
	else
	{
		return E_FAIL;
	}

	if(pITypeLib)
	{
		pITypeLib->Release();
		pITypeLib = NULL;
	}
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer
//
/////////////////////////////////////////////////////////////////////////////
STDAPI DllRegisterServer(void)
{
	HRESULT				hr = E_FAIL;

	HKEY				hLTMKey = NULL;
	HKEY				hLTMModuleKey = NULL;
	
	HKEY				hRootKey = NULL;
	HKEY				hModuleKey = NULL;
	HKEY				hInprocServerKey = NULL;
	HKEY				hProgIDKey = NULL;
	
	WCHAR				*pwszCLSID = NULL;
	CHAR				szModuleFileName[MAX_NAME_LEN];
	CHAR				szProgID[MAX_NAME_LEN];
	CHAR				szModuleName[MAX_NAME_LEN];
	CHAR				szGuid[MAX_NAME_LEN];

	CHAR				strFileName[MAX_PATH + 1];
	WCHAR				wszFileName[MAX_PATH + 1];

	GlobalModuleData	*pModuleData = &g_pThisTestModule->m_gmd;
	ITypeLib			*pITypeLib = NULL;

	// The instance handle of a DLL can be used as module handle (see DllMain doc)
	if(!GetModuleFileName(g_hInstance, strFileName, sizeof(strFileName)/sizeof(CHAR)))
		return E_FAIL;

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
	if(ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, "CLSID", &hRootKey))
	{
		//HKEY_CLASSES_ROOT\CLSID\{..Guid..}
		if(ERROR_SUCCESS == RegCreateKey(hRootKey, szGuid, &hModuleKey))
		{
			//{Guid}\Value=ModuleName
			RegSetValueEx(hModuleKey, NULL, 0, REG_SZ, (BYTE*)szModuleName, (strlen(szModuleName)) * sizeof(CHAR));

			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\InprocServer32
			if(ERROR_SUCCESS == RegCreateKey(hModuleKey, "InprocServer32", &hInprocServerKey))
			{
				//InprocServer32\<>\FileName.dll
				GetModuleFileName(g_hInstance, szModuleFileName, MAX_NAME_LEN);
				RegSetValueEx(hInprocServerKey, NULL, 0, REG_SZ, (BYTE*)szModuleFileName, (strlen(szModuleFileName)) * sizeof(CHAR));
				
				//InprocServer32\ThreadingModel\Both
				RegSetValueEx(hInprocServerKey, "ThreadingModel", 0, REG_SZ, (BYTE*)"Both", (strlen("Both")) * sizeof(CHAR));
				RegCloseKey(hInprocServerKey);
				hr = S_OK;
			}

			//HKEY_CLASSES_ROOT\CLSID\{..Guid..}\ProgID
			if(ERROR_SUCCESS == RegCreateKey(hModuleKey, "ProgID", &hProgIDKey))
			{
				RegSetValueEx(hProgIDKey, NULL, 0, REG_SZ, (BYTE*)szProgID, (strlen(szProgID)) * sizeof(CHAR));
				RegCloseKey(hProgIDKey);
			}
			RegCloseKey(hModuleKey);
		}
		RegCloseKey(hRootKey);
	}


	//HKEY_CLASSES_ROOT\ProgID
	if(ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szProgID, &hRootKey))
	{
		//HKEY_CLASSES_ROOT\ProgID\CLSID
		if(ERROR_SUCCESS == RegCreateKey(hRootKey, "CLSID", &hModuleKey))
		{
			//ProgID\CLSID\{Guid}
			RegSetValueEx(hModuleKey, NULL, 0, REG_SZ, (BYTE*)szGuid, (strlen(szGuid)) * sizeof(CHAR));
			RegCloseKey(hModuleKey);
			hr = S_OK;
		}
		RegCloseKey(hRootKey);
	}
	

	//Step 2: Register our test in the LTM Test Location
	//So LTM is aware of a new test module is available for running...
	
	//Obtain the Key for HKEY_LOCAL_MACHINE\"SOFTWARE\Microsoft\LTM\Test Modules"
	if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\LTM\\Test Modules", &hLTMKey))
	{

		//Obtain the Key for "...\{Guid}
		if(ERROR_SUCCESS == RegCreateKey(hLTMKey, szGuid, &hLTMModuleKey))
		{
			//ModuleName
			RegSetValueEx(hLTMModuleKey, NULL, 0, REG_SZ, (BYTE*)szModuleName, (strlen(szModuleName)) * sizeof(CHAR));
		}
	}

	RegCloseKey(hLTMKey);
	RegCloseKey(hLTMModuleKey);
	CoTaskMemFree(pwszCLSID);


	//convert to wide char
	if(!MultiByteToWideChar(CP_ACP, NULL, strFileName, -1, wszFileName, MAX_PATH))
	{
		return E_FAIL;
	}

	if (SUCCEEDED(hr = LoadTypeLib(wszFileName, &pITypeLib)))
		hr = RegisterTypeLib(pITypeLib, wszFileName, NULL);
	else
	{
		return E_FAIL;
	}

	if(pITypeLib)
	{
		pITypeLib->Release();
		pITypeLib = NULL;
	}

	return hr;
}




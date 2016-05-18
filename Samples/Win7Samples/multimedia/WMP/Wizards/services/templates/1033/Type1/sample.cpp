//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "stdafx.h"
#include "[!output root].h"

CONTENT_PARTNER_GLOBALS g;

OBJECT_ENTRY_AUTO(__uuidof(C[!output Safe_root]), C[!output Safe_root])

C[!output Safe_root]::C[!output Safe_root]()
{
   ZeroMemory( g.credentialsFile, MAX_PATH*sizeof(g.credentialsFile[1]) );
   g.totalDownloadFailures = 0;
   g.userLoggedIn = 0;
   g.haveCachedCredentials = 0;
   m_downloadThreadHandle = 0;
   m_downloadThreadId = 0;
   m_msgDownloadBatch = 0;
   m_buyThreadHandle = 0;
   m_buyThreadId = 0;
   m_msgBuy = 0;
   m_refreshLicenseThreadHandle = 0;
   m_refreshLicenseThreadId = 0;
   m_msgRefreshLicense = 0;
   m_loginThreadHandle = 0;
   m_loginThreadId = 0;
   m_msgLogin = 0;
   m_msgLogout = 0;
   m_msgAuthenticate = 0;
   m_msgVerifyPermission = 0;
   m_sendMessageThreadHandle = 0;
   m_sendMessageThreadId = 0;
   m_msgSendMessage = 0;
   m_updateDeviceThreadHandle = 0;
   m_updateDeviceThreadId = 0;
   m_msgUpdateDevice = 0;
   m_listThreadHandle = 0;
   m_listThreadId = 0;
   m_msgGetListContents = 0;
   m_msgExitMessageLoop = 0;  
}


HRESULT C[!output Safe_root]::FinalConstruct()
{
   HRESULT hr = S_OK;

   ATLTRACE2("%x: FinalConstruct\n", GetCurrentThreadId());

   m_msgDownloadBatch = RegisterWindowMessage( L"DownloadBatch" );

   if(0 == m_msgDownloadBatch)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgBuy = RegisterWindowMessage( L"Buy" );

   if(0 == m_msgBuy)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgRefreshLicense = RegisterWindowMessage( L"RefreshLicense" );

   if(0 == m_msgRefreshLicense)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgLogin = RegisterWindowMessage( L"Login" );

   if(0 == m_msgLogin)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgAuthenticate = RegisterWindowMessage( L"Authenticate" );

   if(0 == m_msgAuthenticate)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgLogout = RegisterWindowMessage( L"Logout" );

   if(0 == m_msgLogout)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgVerifyPermission = RegisterWindowMessage( L"VerifyPermission" );

   if(0 == m_msgVerifyPermission)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgSendMessage = RegisterWindowMessage( L"SendMessage" );

   if(0 == m_msgSendMessage)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgGetListContents = RegisterWindowMessage( L"GetListContents" );

   if(0 == m_msgGetListContents)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   m_msgExitMessageLoop = RegisterWindowMessage( L"ExitMessageLoop" );

   if(0 == m_msgExitMessageLoop)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ATLTRACE2("FinalConstruct: RegisterWindowMessage failed. %x\n", hr);
      goto cleanup;
   }

   hr = this->CreateCredentialsFilePath();

   if(FAILED(hr))
   {
      ATLTRACE2("FinalConstruct: CreateCredentialsFilePath failed. %x\n", hr);
      goto cleanup;
   }

   // Determine whether we have cached credentials.

   HANDLE hInfoFile = INVALID_HANDLE_VALUE;

   hInfoFile = CreateFile(
      g.credentialsFile,
      0,
      0,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL);

   if(INVALID_HANDLE_VALUE != hInfoFile)
   {
      // The file exists.
      // We have cached credentials.
      g.haveCachedCredentials = 1;
   }
    
   CloseHandle(hInfoFile);

cleanup:
   return hr;
}

C[!output Safe_root]::~C[!output Safe_root]()
{
   ATLTRACE2("%x: [!output Safe_root] destructor: Total download failures: %d\n", 
      GetCurrentThreadId(), 
      g.totalDownloadFailures); 
}


class C[!output Safe_root]Module : public CAtlDllModuleT< C[!output Safe_root]Module >
{};

C[!output Safe_root]Module _AtlModule;

BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
   return _AtlModule.DllMain(dwReason, lpReserved); 
}

STDAPI DllCanUnloadNow(void)
{
   return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
   // There is no type library, so pass
   // FALSE to _AtlModule.DllRegistreServer
   HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
    return hr;
}

STDAPI DllUnregisterServer(void)
{
   // There is no type library, so pass
   // FALSE to _AtlModule.DllUnRegistreServer
    HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
    return hr;
}
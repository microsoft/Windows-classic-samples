//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#include "RoutingExtension.h"
#include <faxcomex_i.c>

//+---------------------------------------------------------------------------
//
//  function:   IsOSVersionCompatible
//
//  Synopsis:   finds whether the target OS supports this functionality.
//
//  Arguments:  [dwVersion] - Minimum Version of the OS required for the Sample to run.
//
//  Returns:    bool - true if the Sample can run on this OS
//
//----------------------------------------------------------------------------

bool IsOSVersionCompatible(DWORD dwVersion)
{
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
        if( !bOsVersionInfoEx  )
        {
                osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
                if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
                        return false;
        }
        bOsVersionInfoEx = (osvi.dwMajorVersion >= dwVersion );
        return (bOsVersionInfoEx == TRUE);
}


//+---------------------------------------------------------------------------
//
//  function:   IsServiceAuthSystem.
//
//  Synopsis:   Checks whether the fax service runs under the Local System Account or Network Service Account
//
//  Arguments:  None
//
//  Returns:    TRUE - if service runs under Local System account
//              FALSE - otherwise
//
//----------------------------------------------------------------------------
BOOL IsServiceAuthSystem()          
{
        SC_HANDLE hScmHandle,hSvcHandle;
        LPQUERY_SERVICE_CONFIG lpqscBuf = NULL;
        BOOL bResult = TRUE;
        DWORD dwBytesNeeded = 0;
        DWORD dwBytesCopied = 0;
        DWORD dwLastError = ERROR_SUCCESS;        
        hScmHandle = hSvcHandle = NULL;

        //Open Service Manager
        hScmHandle = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
        if(NULL == hScmHandle)
        {
                bResult = FALSE;
                goto Error;
        }

        //Open Fax Service
        hSvcHandle = OpenService(hScmHandle,TEXT("Fax"),SERVICE_ALL_ACCESS);
        if(NULL == hSvcHandle)
        {      
                bResult = FALSE;
                goto Error;
        }

        //Query Fax Service
        if (!QueryServiceConfig(hSvcHandle,NULL,0,&dwBytesNeeded) ) 
        {
                dwLastError = GetLastError();
                if(ERROR_INSUFFICIENT_BUFFER!=dwLastError)
                {
                        bResult = FALSE;
                        goto Error;
                }
        }    

        lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, dwBytesNeeded); 
        if (lpqscBuf == NULL) 
        {
                bResult = FALSE;
                goto Error;
        }
        if (! QueryServiceConfig(hSvcHandle, lpqscBuf, dwBytesNeeded, &dwBytesCopied) ) 
        {
                bResult = FALSE;
                goto Error;
        }
        if ( NULL == lpqscBuf->lpServiceStartName) 
        {
                bResult = FALSE;
                goto Error;
        }

        //Check the service Start Account Name
        if(_tcsnicmp(lpqscBuf->lpServiceStartName,TEXT("LocalSystem"),_tcslen(lpqscBuf->lpServiceStartName)))
                bResult = FALSE;

Error:
        if(hScmHandle)
                CloseServiceHandle(hScmHandle);
        if(hSvcHandle)
                CloseServiceHandle(hSvcHandle);
        if(lpqscBuf)
                LocalFree(lpqscBuf);


        return bResult;
}

//+---------------------------------------------------------------------------
//
//  function:   DllEntry
//
//  Synopsis:   dll entrypoint 
//
//  Arguments:  hInstance   - Module handle
//              Reason      - Reason for being called
//              Context     - Register context
//
//  Returns:    TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------


extern "C" DWORD DllEntry(HINSTANCE hInstance, DWORD dwReason, LPVOID lpContext)
{
        BOOL bRet = 0;
        switch (dwReason) 
        {
                case DLL_PROCESS_ATTACH:
                        g_hModule = hInstance;
                        bRet = DisableThreadLibraryCalls(hInstance);
                        if(bRet == 0)
                        {
                                ROUTEDEBUG(( L"DisableThreadLibraryCalls failed, ec = %d\n", GetLastError() ));
                                return FALSE;
                        }
                        break;
                case DLL_PROCESS_DETACH:

                        //
                        // cleanup
                        //
                        if (hReceiveEvent) CloseHandle(hReceiveEvent);
                        break;
        }
        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   DllRegisterServer
//
//  Synopsis:   Function for the in-process server to create its registry entries
//
//  Arguments:  VOID
//
//  Returns:    S_OK on success
//
//  Notes:
//          We leverage the DllRegisterServer entrypoint as an easy way to configure
//          our routing extension for use on the system.  Note that the extension doesn't
//          have any COM code in it per se, but this makes installation much simpler since
//          the setup code doesn't have to use custom code to setup the routing extension.
//
//----------------------------------------------------------------------------
STDAPI DllRegisterServer( VOID )      
{
        HRESULT hr = S_OK;   
        IFaxServer2* pFaxServer = NULL;
        bool bConnected = false;

        BSTR bstrFriendlyName= SysAllocString(EXTENSIONFRIENDLYNAME);
        BSTR bstrImage=SysAllocString(EXTENSIONPATH);
        BSTR bstrExtensionName=SysAllocString(EXTENSIONNAME);

        SAFEARRAY * pSafeArray;
        SAFEARRAYBOUND aDim[1];    
        BSTR* pbstrArray = NULL;

        VARIANT vVariant;

        //check for OS version
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                ROUTEDEBUG((_T("This sample is compatible with Windows Vista \n")));   
                hr = ERROR_NOT_SUPPORTED;
                goto Exit1;
        }


        // we assume that the routing extension has already been installed into the 
        // proper location by the setup code.

        
        if(bstrFriendlyName == NULL || bstrImage == NULL || bstrExtensionName == NULL)
        {
                //SysAllocString Failed
                ROUTEDEBUG((_T("SysAllocString Failed.\n")));
                goto Exit;
        }
        //initialize COM
        hr = CoInitialize(NULL);
        if(FAILED(hr))
        {
                //failed to init com
                ROUTEDEBUG((_T("Failed to init com. Error 0x%x \n"), hr));                
                goto Exit;
        }

        hr = CoCreateInstance (CLSID_FaxServer, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxServer), 
                            (void **)&pFaxServer);
        if(FAILED(hr))
        {
                //CoCreateInstance failed.
                ROUTEDEBUG((_T("CoCreateInstance failed. Error 0x%x \n"), hr));
                goto Exit;
        }
         

        //connect to local fax server.
        hr = pFaxServer->Connect(L"");
        if(FAILED(hr))
        {
                ROUTEDEBUG((_T("Connect failed. Error 0x%x \n"), hr));
                goto Exit;
        }
        bConnected = true;

        FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
        hr = pFaxServer->get_APIVersion(&enumFaxAPIVersion);
        if(FAILED(hr))
        {
                //get_APIVersion failed.
                ROUTEDEBUG((_T("get_APIVersion failed. Error 0x%x \n"), hr));                
                goto Exit;
        }

        if (enumFaxAPIVersion < fsAPI_VERSION_3) 
        {
                ROUTEDEBUG((_T("This sample is compatible with Windows Vista")));
                goto Exit;
        }         


        VariantInit(&vVariant); 

        aDim[0].lLbound = 0;
        aDim[0].cElements = 1;  

        vVariant.vt = VT_ARRAY | VT_BSTR;
        pSafeArray = SafeArrayCreate(VT_BSTR, 1, aDim);   
        SafeArrayAccessData(pSafeArray, (void**)&pbstrArray);   


        //Create the Variant String MethodName;FriendlyName;FunctionName;GUID
        WCHAR strMethodInfo[STR_SIZE] = {0};
        hr = StringCchCopy(strMethodInfo, 1024, FAXROUTEMETHOD);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCopy for FAXROUTEMETHOD failed. Error = 0x%x\n" ,hr));
                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, L";");
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for ; failed. Error = 0x%x\n" ,hr));
                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, FAXROUTEFRIENDLYNAME);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for FAXROUTEFRIENDLYNAME failed. Error = 0x%x\n" ,hr));
                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, L";");
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for ; failed. Error = 0x%x\n" ,hr));
                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, FAXROUTEFUNCTION);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for FAXROUTEFUNCTION failed. Error = 0x%x\n" ,hr));
                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, L";");
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for ; failed. Error = 0x%x\n" ,hr));

                goto Exit;
        }    
        hr = StringCchCat(strMethodInfo, STR_SIZE, ROUTEITGUID);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCat for ROUTEITGUID failed. Error = 0x%x\n" ,hr));
                                goto Exit;
        }    

        *pbstrArray = SysAllocString(strMethodInfo);
        if(*pbstrArray == NULL )
        {
                //SysAllocString Failed
                ROUTEDEBUG((_T("SysAllocString Failed for pbstrArray.\n")));
                goto Exit;
        }
        SafeArrayUnaccessData(pSafeArray);
        vVariant.parray = pSafeArray;    

        hr = pFaxServer->RegisterInboundRoutingExtension(bstrExtensionName,bstrFriendlyName,bstrImage,vVariant);
        if(FAILED(hr))
        {
                //RegisterInboundRoutingExtension failed.
                ROUTEDEBUG((_T("RegisterInboundRoutingExtension failed. Error 0x%x \n"), hr));                
                goto Exit;
        }


Exit:    
        if(*pbstrArray != NULL)
                SysFreeString(*pbstrArray);
        if(bstrExtensionName)
                SysFreeString(bstrExtensionName);
        if(bstrImage)
                SysFreeString(bstrImage);
        if(bstrFriendlyName)
                SysFreeString(bstrFriendlyName);
        if(bConnected)
        {
                pFaxServer->Disconnect();
        }

        CoUninitialize();   
Exit1:
        return hr;
}


//+---------------------------------------------------------------------------
//
//  function:   DllUnregisterServer
//
//  Synopsis:   Function for the in-process server to delete its registry entries
//
//  Arguments:  VOID
//
//  Returns:    S_OK on success
//
//----------------------------------------------------------------------------

STDAPI DllUnregisterServer( VOID )       
{
        HRESULT hr = S_OK;
        IFaxServer2* pFaxServer = NULL;
        bool bConnected = false;        
        BSTR bstrExtensionName = SysAllocString(EXTENSIONNAME);

        //check for OS version
        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                ROUTEDEBUG((_T("This sample is compatible with Windows Vista \n")));   
                hr = ERROR_NOT_SUPPORTED;
                goto Exit1;
        }

        if(bstrExtensionName == NULL)
        {
                //SysAllocString Failed
                ROUTEDEBUG((_T("SysAllocString Failed.\n")));
                goto Exit;
        }

        // we assume that the routing extension has already been installed into the 
        // proper location by the setup code.


        //initialize COM
        hr = CoInitialize(NULL);
        if(FAILED(hr))
        {
                //failed to init com
                ROUTEDEBUG((_T("Failed to init com. Error 0x%x \n"), hr));
                goto Exit;
        }

        hr = CoCreateInstance (CLSID_FaxServer, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxServer), 
                            (void **)&pFaxServer);
        if(FAILED(hr))
        {
                //CoCreateInstance failed.
                ROUTEDEBUG((_T("CoCreateInstance failed. Error 0x%x \n"), hr));
                goto Exit;
        }

        //connect to local fax server.
        hr = pFaxServer->Connect(L"");
        if(FAILED(hr))
        {
                ROUTEDEBUG((_T("Connect failed. Error 0x%x \n"), hr));
                goto Exit;
        }
        bConnected = true;

        FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
        hr = pFaxServer->get_APIVersion(&enumFaxAPIVersion);
        if(FAILED(hr))
        {
                //get_APIVersion failed.
                ROUTEDEBUG((_T("get_APIVersion failed. Error 0x%x \n"), hr));
                goto Exit;
        }

        if (enumFaxAPIVersion < fsAPI_VERSION_3) 
        {
                ROUTEDEBUG((_T("This sample is compatible with Windows Vista")));
                goto Exit;
        }         


      
        hr = pFaxServer->UnregisterInboundRoutingExtension(bstrExtensionName);
        if(FAILED(hr))
        {
                //UnregisterInboundRoutingExtension failed.
                ROUTEDEBUG((_T("UnregisterInboundRoutingExtension failed. Error 0x%x \n"), hr));
                goto Exit;
        }


Exit:    
        if(bstrExtensionName)
                SysFreeString(bstrExtensionName);
        if(bConnected)
        {
                pFaxServer->Disconnect();
        }

        if(LogFile != NULL)
            HeapFree(hHeap, 0, LogFile);
        if(IniFile != NULL)
            HeapFree(hHeap, 0,IniFile);
   

        CoUninitialize();   
Exit1:
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxRouteInitialize
//
//  Synopsis:   This functions is called by the fax service to initialize the routing extension.  This function
//              should only be called once per instantiation of the fax service
//
//  Arguments:  hHeapHandle               - Heap handle for memory all allocations
//              pFaxRouteCallbackRoutines - structure containing callback functions    
//              
//  Returns:    TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------

BOOL WINAPI FaxRouteInitialize(IN HANDLE hHeapHandle, IN PFAX_ROUTE_CALLBACKROUTINES pFaxRouteCallbackRoutines )
{
        DWORD dwNeeded =0;
        SECURITY_ATTRIBUTES SA;
        TCHAR* ptstrSD = NULL;    // SDDL string
        ULONG uSDSize = 0;
        BOOL bLocalSystemAct = TRUE;
        BOOL bResult        = FALSE;


        // make sure we can understand the structure
        if ( pFaxRouteCallbackRoutines->SizeOfStruct < sizeof(FAX_ROUTE_CALLBACKROUTINES) )
        {
                ROUTEDEBUG ((L"The passed in SizeOfStruct (%d) is smaller than expected (%d) ", 
                                        pFaxRouteCallbackRoutines->SizeOfStruct,
                                        sizeof(FAX_ROUTE_CALLBACKROUTINES)));
                bResult = FALSE;
                goto Exit;
        }

        hHeap = hHeapHandle;
        FaxRouteAddFile = pFaxRouteCallbackRoutines->FaxRouteAddFile;
        FaxRouteDeleteFile = pFaxRouteCallbackRoutines->FaxRouteDeleteFile;
        FaxRouteGetFile = pFaxRouteCallbackRoutines->FaxRouteGetFile;
        FaxRouteEnumFiles = pFaxRouteCallbackRoutines->FaxRouteEnumFiles;
        FaxRouteModifyRoutingData = pFaxRouteCallbackRoutines->FaxRouteModifyRoutingData;

        InitializeCriticalSection( &csRoute );

        bLocalSystemAct =  IsServiceAuthSystem();

        // If bLocalSystemAct is true, then the fax service is running under the Local System Account
        // hence we don't need create a security descriptor.
        // 
        // If bLocalSystemAct is false, then the fax Service is runnung with run with Network Service 
        // account, so we must setup a security descriptor.        
        if(!bLocalSystemAct)
        {
                // we must setup a security descriptor so other account (and other desktops) may access
                // to Event or Mutex                
                ptstrSD =   TEXT("O:NS")                // owner Network Service
                        TEXT("G:NS")                    // group Network Service
                        TEXT("D:")                      // DACL
                        TEXT("(A;;GA;;;NS)")            // give Network Service full access
                        TEXT("(A;;0x00100000;;;AU)");   // give Authenticated users SYNCHRONIZE access

                SA.nLength = sizeof(SECURITY_ATTRIBUTES);
                SA.bInheritHandle = FALSE;
                SA.lpSecurityDescriptor = NULL;

                if(!ConvertStringSecurityDescriptorToSecurityDescriptor(
                                        ptstrSD,
                                        SDDL_REVISION_1,
                                        &(SA.lpSecurityDescriptor),
                                        &uSDSize
                                        ))
                {
                        ROUTEDEBUG(( L"ConvertStringSecurityDescriptorToSecurityDescriptor failed, ec = %d\n", 
                                                GetLastError() ));
                        bResult = FALSE;                
                        goto Exit;
                }
        }

        hReceiveEvent = CreateEvent(&SA,FALSE,FALSE,FAXROUTE);
        if (!hReceiveEvent)
        {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                        ROUTEDEBUG (( L"CreateEvent failed, ec = %d\n", GetLastError() ));
                        bResult = FALSE;                
                        goto Exit;
                }
        }
        LocalFree(SA.lpSecurityDescriptor);

        // get path to files for logging, etc.        
        dwNeeded = ExpandEnvironmentStrings(ININAME,IniFile,0);    
        IniFile  = (LPWSTR) HeapAlloc(hHeap,HEAP_ZERO_MEMORY,(dwNeeded)*sizeof(WCHAR));
        if (!IniFile)
        {
                ROUTEDEBUG((L"HeapAlloc failed, ec = %d\n", GetLastError() ));
                bResult = FALSE;
                goto Exit;
        }
        DWORD dwSuccess = ExpandEnvironmentStrings(ININAME,IniFile,dwNeeded);
        if (dwSuccess == 0)
        {
                ROUTEDEBUG(( L"ExpandEnvironmentStrings for LogFile failed, ec = %d\n", GetLastError() ));
                bResult = FALSE;
                goto Exit;
        }
        dwNeeded = ExpandEnvironmentStrings(LOGNAME,LogFile,0);
        LogFile  = (LPWSTR) HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(WCHAR)*(dwNeeded));
        if (!LogFile)
        {                
                ROUTEDEBUG(( L"HeapAlloc failed, ec = %d\n", GetLastError() ));
                bResult = FALSE;
                goto Exit;
        }
        dwSuccess = ExpandEnvironmentStrings(LOGNAME,LogFile,dwNeeded);
        if (dwSuccess == 0)
        {
                ROUTEDEBUG(( L"ExpandEnvironmentStrings for LogFile failed, ec = %d\n", GetLastError() ));
                bResult = FALSE;
                goto Exit;
        }

        ROUTEDEBUG (( L"Logfile : %s\n", LogFile ));
        ROUTEDEBUG (( L"Inifile : %s\n", IniFile ));
        bResult = TRUE;
Exit:
        if((bResult==FALSE) && (LogFile != NULL))
                HeapFree(hHeap, 0, LogFile);
        if((bResult==FALSE) && (IniFile != NULL))
                HeapFree(hHeap, 0,IniFile);
        return bResult;

}

//+---------------------------------------------------------------------------
//
//  function:   FaxRouteGetRoutingInfo
//
//  Synopsis:   This functions is called by the fax service to get routing configuration data.
//
//  Arguments:  lpcwstrRoutingGuid         - Unique identifier for the requested routing method
//              dwDeviceId            - Device that is being configured
//              lpbRoutingInfo         - Routing info buffer
//              lpdwRoutingInfoSize     - Size of the buffer (in bytes)
//              
//  Returns:    TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------

BOOL WINAPI FaxRouteGetRoutingInfo(IN  LPCWSTR lpcwstrRoutingGuid, IN  DWORD dwDeviceId, 
                IN  LPBYTE lpbRoutingInfo, OUT LPDWORD lpdwRoutingInfoSize )
{        
        //Here we read for each device if the Routing Method is enabled.

        DWORD dwDataSize = sizeof (DWORD);
        ReadConfiguration(dwDeviceId);
        BOOL bMethodEnabled = g_dwFlags;
        if (NULL == lpbRoutingInfo)
        {
                // Caller just wants to know the data size            
                *lpdwRoutingInfoSize = dwDataSize;
                return TRUE;
        }
        if (dwDataSize > *lpdwRoutingInfoSize)
        {        
                // Caller supplied too small a buffer        
                *lpdwRoutingInfoSize = dwDataSize;
                SetLastError (ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
        }
        // First DWORD tells if this method is enabled or not        
        *((LPDWORD)lpbRoutingInfo) = bMethodEnabled;

        // Set actual size used
        *lpdwRoutingInfoSize = dwDataSize;
        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxRouteSetRoutingInfo
//
//  Synopsis:   This functions is called by the fax service to
//              set routing configuration data.
//
//  Arguments:  lpwstrRoutingGuid         - Unique identifier for the requested routing method
//              dwDeviceId            - Device that is being configured
//              lpbRoutingInfo         - Routing info buffer
//              dwRoutingInfoSize     - Size of the buffer (in bytes)
//
//  Returns:    TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------

BOOL WINAPI
FaxRouteSetRoutingInfo(
                IN  LPWSTR lpwstrRoutingGuid,
                IN  DWORD dwDeviceId,
                IN  LPBYTE lpbRoutingInfo,
                IN  DWORD dwRoutingInfoSize
                )
{
        DWORD dwRes= ERROR_SUCCESS;
        BOOL bMethodEnabled = FALSE;
        LPCWSTR lpcwstrMethodConfig = LPCWSTR(&lpbRoutingInfo[sizeof (DWORD)]);
        if (dwRoutingInfoSize < sizeof (DWORD))
        {
                ROUTEDEBUG(( TEXT("Data size is too small (%ld)"),
                                        dwRoutingInfoSize));
                SetLastError (ERROR_INVALID_PARAMETER);
                return FALSE;
        }
        // First DWORD tells if method is enabled.
        bMethodEnabled = *((LPDWORD)(lpbRoutingInfo)) ? TRUE : FALSE;
        // Store new value in the extension data storage        
        dwRes = g_pFaxExtSetData (g_hModule,
                        dwDeviceId,
                        DEV_ID_SRC_FAX, // We always use the Fax Device Id
                        REGVAL_RM_FLAGS_GUID,
                        (LPBYTE)&bMethodEnabled,
                        sizeof (DWORD)
                        );
        if (ERROR_SUCCESS != dwRes)
        {
                ROUTEDEBUG((L"g_pFaxExtSetData failed., ec = %d\n", GetLastError() ));
                SetLastError (ERROR_INVALID_PARAMETER);
                return FALSE;
        }
        return TRUE;
}


//+---------------------------------------------------------------------------
//
//  function:   ReadConfiguration
//
//  Synopsis:   Reads the routing configuration from the storage.
//              If the storage doesn't contain configuration, default values are used.
//
//  Arguments:  dwDeviceId            - Device that is being configured
//
//  Returns:    Standard Win23 error code.
//
//----------------------------------------------------------------------------

DWORD ReadConfiguration (DWORD dwDevId)  
{
        DWORD   dwRes = ERROR_SUCCESS;
        LPBYTE  lpData = NULL;
        DWORD   dwDataSize =0;

        // Start by reading the flags data
        dwRes = g_pFaxExtGetData ( dwDevId,
                        DEV_ID_SRC_FAX, // We always use the Fax Device Id
                        REGVAL_RM_FLAGS_GUID,
                        &lpData,
                        &dwDataSize
                        );
        if (ERROR_SUCCESS != dwRes)
        {
                if (ERROR_FILE_NOT_FOUND == dwRes)
                {                        
                        // Data does not exist for this device. Try to read default values from unassociated data.
                        dwRes = g_pFaxExtGetData ( 0,        // unassociated data
                                        DEV_ID_SRC_FAX, // We always use the Fax Device Id
                                        REGVAL_RM_FLAGS_GUID,
                                        &lpData,
                                        &dwDataSize
                                        );
                        if (ERROR_FILE_NOT_FOUND == dwRes)
                        {                                
                                // Data does not exist for this device. Use default values.
                                ROUTEDEBUG( ( TEXT("No routing flags configuration - using defaults")));
                                g_dwFlags = DEFAULT_FLAGS;
                        }
                }
                if (ERROR_SUCCESS != dwRes &&
                                ERROR_FILE_NOT_FOUND != dwRes)
                {
                        // Can't read configuration
                        ROUTEDEBUG( ( TEXT("Error reading routing flags (ec = %ld)"), dwRes));
                        return dwRes;
                }
        }   

        if (NULL != lpData)
        {
                // Data read successfully
                if (sizeof (DWORD) != dwDataSize)
                {
                        // We're expecting a single DWORD here
                        ROUTEDEBUG((TEXT("Routing flags configuration has bad size (%ld) - expecting %ld"), dwDataSize, sizeof (DWORD)));
                        g_pFaxExtFreeBuffer (lpData);
                        return ERROR_BADDB; // The configuration registry database is corrupt.
                }
                g_dwFlags = DWORD (*lpData);
                g_pFaxExtFreeBuffer (lpData);
        }   
        return ERROR_SUCCESS;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxRouteDeviceEnable
//
//  Synopsis:    This functions is called by the fax service to determine if a routing extension is enabled or
//               to enable a routing extension
//
//  Arguments:    lpcwstrRoutingGuid         - Unique identifier for the requested routing method
//                dwDeviceId            - Device that is being configured
//                lEnabled             - meaning differs based on context (see FAXROUTE_ENABLE enumerated type)
//
//  Returns:    if lEnable == QUERY_STATUS then it returns whether the method iebaled or disbled.
//              else the method had to be set to a state and whether that operation was successful or not. 
//
//----------------------------------------------------------------------------

BOOL WINAPI FaxRouteDeviceEnable(
                IN  LPWSTR lpcwstrRoutingGuid,
                IN  DWORD dwDeviceId,
                IN  LONG lEnable
                )
{
        // Note that this isn't thread safe
        DWORD dwRes = ERROR_SUCCESS;
        DWORD dwValue = g_dwFlags;

        // make sure that we're dealing with our routing method
        if (_tcscmp(CharLower(lpcwstrRoutingGuid),ROUTEITGUID) != 0)
        {
                ROUTEDEBUG (( L"Passed a GUID (%s) for a method not in this extension!\n", lpcwstrRoutingGuid ));
                return FALSE;
        }

        // Just read the status of the method
        if (QUERY_STATUS == lEnable)
        {
                ReadConfiguration(dwDeviceId);
                return (g_dwFlags == TRUE);
        }

        //Set the State of the Method for the DEvice
        if(lEnable == STATUS_ENABLE )
                dwValue = TRUE;
        else         
                dwValue = FALSE;


        // Store new value in the extension data storage
        dwRes = g_pFaxExtSetData (g_hModule,
                        dwDeviceId,
                        DEV_ID_SRC_FAX, // We always use the Fax Device Id
                        REGVAL_RM_FLAGS_GUID,
                        (LPBYTE)&dwValue,
                        sizeof (DWORD)
                        );
        if (ERROR_SUCCESS == dwRes)
        {
                // Registry store successful - Update flags value in memory with new value.
                g_dwFlags = dwValue;
        }    
        return dwRes;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxRouteDeviceChangeNotification
//
//  Synopsis:    This functions is called by the fax service to alert the routing extension that a device 
//               has changed
//
//  Arguments:    dwDeviceId            - Device that has changed 
//                bNewDevice           - TRUE means device was added, FALSE means a device was removed 
//
//  Returns:      TRUE for success
//
//----------------------------------------------------------------------------

BOOL WINAPI FaxRouteDeviceChangeNotification( IN  DWORD dwDeviceId, IN  BOOL  bNewDevice )
{
        //
        // We don't have any per device routing data, so this is just stubbed out
        //
        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:   FaxExtInitializeConfig
//
//  Synopsis:    This functions is called by the fax service to alert the routing extension that a device 
//               has changed
//
//  Arguments:    pFaxExtGetData               [in] - Pointer to FaxExtGetData
//                pFaxExtSetData               [in] - Pointer to FaxExtSetData
//                pFaxExtRegisterForEvents     [in] - Pointer to FaxExtRegisterForEvents
//                pFaxExtUnregisterForEvents   [in] - Pointer to FaxExtUnregisterForEvents
//                pFaxExtFreeBuffer            [in] - Pointer to FaxExtFreeBuffer
//                bNewDevice           - TRUE means device was added, FALSE means a device was removed 
//
//  Returns:      Standard HRESULT code
//
//----------------------------------------------------------------------------

HRESULT FaxExtInitializeConfig ( 
                PFAX_EXT_GET_DATA               pFaxExtGetData,
                PFAX_EXT_SET_DATA               pFaxExtSetData,
                PFAX_EXT_REGISTER_FOR_EVENTS    pFaxExtRegisterForEvents,
                PFAX_EXT_UNREGISTER_FOR_EVENTS  pFaxExtUnregisterForEvents,
                PFAX_EXT_FREE_BUFFER            pFaxExtFreeBuffer
                )
{    
        g_pFaxExtGetData = pFaxExtGetData;
        g_pFaxExtSetData = pFaxExtSetData;
        g_pFaxExtRegisterForEvents = pFaxExtRegisterForEvents;
        g_pFaxExtUnregisterForEvents = pFaxExtUnregisterForEvents;
        g_pFaxExtFreeBuffer = pFaxExtFreeBuffer;
        return S_OK;
}   // FaxExtInitializeConfig



// Routing Method(s)

//+---------------------------------------------------------------------------
//
//  function:   RouteIt
//
//  Synopsis:    This functions is called by the fax service to alert the routing extension that a device 
//               has changed
//
//  Arguments:    pFaxRoute            - Routing information
//                pFailureData         - Failure data buffer
//                lpdwFailureDataSize     - Size of failure data buffer
//
//  Returns:      TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------
BOOL WINAPI RouteIt(
                PFAX_ROUTE pFaxRoute,
                PVOID *pFailureData,
                LPDWORD lpdwFailureDataSize
                )
{
        //Source File Name
        WCHAR strTiffFileName[MAX_PATH] = {0};
        //Drive to copy to
        WCHAR strDrive[MAX_PATH ] = {0}; 
        //Destination File Complete Path
        WCHAR strCopyOfTiff[MAX_PATH+1] = {0};
        //Directory to copy to.
        WCHAR strCopyDir[MAX_PATH+1] = {0};

        //szFileExt is the filename with the extension        
        LPWSTR lpwstrFilePart  = NULL;
        LPWSTR lpwstrOcc = NULL;  

        //lpwstrPath is the path of the file
        WCHAR lpwstrPath [MAX_PATH + 1] = {0} ;
        BOOL bRetVal = FALSE;
        HRESULT hr = S_OK;

        DWORD dwRetVal = ReadConfiguration(pFaxRoute->DeviceId);
        if(dwRetVal != ERROR_SUCCESS)
        {
                ROUTEDEBUG(( L"ReadConfiguration failed. ec = %d", dwRetVal));
                goto Exit;         
        }       

        if(g_dwFlags == FALSE)    
        {
                ROUTEDEBUG(( L"Routing Method RouteIt is disabled for device %ld", pFaxRoute->DeviceId));
                bRetVal = TRUE;
                goto Exit;
        }

        DWORD Size = sizeof(strTiffFileName);

        // serialize access to this function so that data is written into the logfile accurately
        EnterCriticalSection( &csRoute );

        if (!FaxRouteGetFile(
                                pFaxRoute->JobId,
                                1,
                                strTiffFileName,
                                &Size))
        {
                ROUTEDEBUG(( L"Couldn't FaxRouteGetFile, ec = %d", GetLastError() ));
                bRetVal = FALSE;
                goto Exit;
        }

        ROUTEDEBUG ((
                                L"Received fax %s\n\tCSID :%s\n\t Name : %s\n\t #: %s\n\tDevice: %s\n", 
                                strTiffFileName,
                                ValidString ( pFaxRoute->Csid ),
                                ValidString ( pFaxRoute->ReceiverName),
                                ValidString ( pFaxRoute->ReceiverNumber),
                                ValidString ( pFaxRoute->DeviceName ) 
                    ));

        //Separate path and file name
        dwRetVal = GetFullPathName(strTiffFileName,MAX_PATH + 1, lpwstrPath, &lpwstrFilePart); 
        if(dwRetVal == 0 )
        {
                ROUTEDEBUG(( L"GetFullPathName failed, ec = %d", GetLastError() ));
                bRetVal = FALSE;
                goto Exit;
        }

        //Find the Drive
        lpwstrOcc = wcschr(strTiffFileName, '\\');
        hr = StringCchCopyN(strDrive, MAX_PATH, strTiffFileName, lstrlen(strTiffFileName) - lstrlen(lpwstrOcc) + 1);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchCopyN failed for strDrive, hr = 0x%x", hr ));
                bRetVal = FALSE;
                goto Exit;
        }

        //Make the Path of the file
        hr = StringCchPrintf(strCopyDir,MAX_PATH+1,L"%s%s",strDrive,ROUTEDIR);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchPrintf failed, hr = 0x%x for strCopyDir", hr ));
                bRetVal = FALSE;
                goto Exit;
        }
        //Make the Path + File NAme
        hr = StringCchPrintf(strCopyOfTiff,MAX_PATH+1,L"%s%s\\%s",strDrive,ROUTEDIR,lpwstrFilePart);
        if(hr != S_OK)
        {
                ROUTEDEBUG(( L"StringCchPrintf failed, hr = 0x%x for strCopyOfTiff", hr ));
                bRetVal = FALSE;
                goto Exit;
        }

        ROUTEDEBUG((L"Location of the tif file: %s \n", strCopyOfTiff));        

        // copy the tiff so it persists after this routine exits
        BOOL bRet = CreateDirectory(strCopyDir,NULL);
        if(bRet == FALSE)
        {
                dwRetVal= GetLastError();
                if(dwRetVal!= ERROR_ALREADY_EXISTS)
                {
                        ROUTEDEBUG((L"CreateDirectory failed. Error  %d \n", dwRetVal));                            
                        bRetVal = FALSE;
                        goto Exit;
                }
        }

        bRet = CopyFile(strTiffFileName,strCopyOfTiff,TRUE);
        if(bRet == FALSE)
        {
                ROUTEDEBUG((L"CopyFile Failed. Error  %d \n", GetLastError()));    
                ROUTEDEBUG((L"Src File: %s Dest File: %s \n", strTiffFileName, strCopyOfTiff));    
                bRetVal = FALSE;
                goto Exit;
        }

        //
        // write some logging data
        // 
        WriteRoutingInfoIntoIniFile(strCopyOfTiff,pFaxRoute);

        //
        // signal event -- another application could use this named event to do something 
        // with the file that was just copied into this directory 
        // (note that the INI file isn't thread-safe accross applications, we could have the routing data 
        // overwritten by another fax being received)
        //

        SetEvent(hReceiveEvent);

        //
        // service needs to be able to interact with the current desktop for this to work
        //
        MessageBeep(MB_ICONEXCLAMATION);
        bRetVal = TRUE;

Exit:
        if(lpwstrFilePart)
                free(lpwstrFilePart);
        LeaveCriticalSection( &csRoute );
        return bRetVal;
}

// Utility Functions

//+---------------------------------------------------------------------------
//
//  function:   WriteRoutingInfoIntoIniFile
//
//  Synopsis:    This functions writes each routing info member into ini file
//
//  Arguments:    lpwstrFileName         - The dest File 
//                pFaxRoute         - Fax Route Object
//
//  Returns:      TRUE for success, otherwise FALSE.
//
//----------------------------------------------------------------------------

BOOL WriteRoutingInfoIntoIniFile(LPWSTR lpwstrTiffFileName,PFAX_ROUTE pFaxRoute) 
{
        WCHAR Buffer[MAX_PATH*2] = {0};
        ULONG ulBufferSize = sizeof(Buffer);
        ULONG ulArraySize = ulBufferSize/sizeof(Buffer[0]);

        if(pFaxRoute == NULL)
        {
            ROUTEDEBUG(( L"WriteRoutingInfoIntoIniFile has pFaxRoute as NULL\n"));
            return FALSE;
        }
        //
        // write each routing info member into ini file
        // 

        //filename
        ZeroMemory(Buffer,ulBufferSize);     
        StringCchCopyN(Buffer,ulBufferSize,ValidString (lpwstrTiffFileName), ulArraySize - 1); 
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"FileName",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );


        //jobid
        ZeroMemory(Buffer,ulBufferSize); 
        StringCchPrintf(Buffer, ulBufferSize, L"%u",pFaxRoute->JobId);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"JobId",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //elapsedtime
        ZeroMemory(Buffer,ulBufferSize); 
        StringCchPrintf(Buffer, ulBufferSize, L"%u",pFaxRoute->ElapsedTime);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"ElapsedTime",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //receivetime
        ZeroMemory(Buffer,ulBufferSize);
        StringCchPrintf(Buffer, ulBufferSize, L"%u",pFaxRoute->ReceiveTime);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"ReceiveTime",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //pagecount
        ZeroMemory(Buffer,ulBufferSize);
        StringCchPrintf(Buffer, ulBufferSize, L"%u",pFaxRoute->PageCount);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"PageCount",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //Csid
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer,ulBufferSize, ValidString (pFaxRoute->Csid), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"Csid",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //CallerId
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer,ulBufferSize, ValidString (pFaxRoute->CallerId ), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"CallerId",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //RoutingInfo
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer, ulBufferSize,ValidString (pFaxRoute->RoutingInfo ), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"RoutingInfo",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //ReceiverName
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer, ulBufferSize, ValidString (pFaxRoute->ReceiverName ), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"ReceiverName",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //ReceiverNumber
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer, ulBufferSize, ValidString (pFaxRoute->ReceiverNumber ), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"ReceiverNumber",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //DeviceName
        ZeroMemory(Buffer,ulBufferSize);
        StringCchCopyN(Buffer, ulBufferSize, ValidString (pFaxRoute->DeviceName ), ulArraySize - 1);
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"DeviceName",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        //DeviceId
        ZeroMemory(Buffer,ulBufferSize);
        StringCchPrintf(Buffer, ulBufferSize, L"%u",pFaxRoute->DeviceId );
        WritePrivateProfileString(
                        L"RoutingInfo",// pointer to section name 
                        L"DeviceId",// pointer to key name 
                        Buffer,   // pointer to string to add 
                        IniFile // pointer to initialization filename 
                        );

        return TRUE;
}

//+---------------------------------------------------------------------------
//
//  function:    DebugPrint
//
//  Synopsis:    Prints a debug string
//
//  Arguments:   format      - wsprintf() format string
//               ...         - Variable data
//
//  Returns:     None.
//
//----------------------------------------------------------------------------

void DebugPrint(LPTSTR Format, ...)
{
        TCHAR Buffer[1024*3] ={0};
        TCHAR AppName[MAX_PATH] ={0};
        SYSTEMTIME CurrentTime;
        int len=0;
        DWORD dwRetVal = ERROR_SUCCESS;
        LPTSTR lptstrFilePart = NULL;
        TCHAR strPath[MAX_PATH] ={0};

        va_list marker;

        ZeroMemory(AppName,MAX_PATH);

        if (  GetModuleFileName(
                                NULL, // handle to module to find filename for 
                                AppName,
                                MAX_PATH)
           )
        {
                //Separate path and file name
                dwRetVal = GetFullPathName(AppName,MAX_PATH, strPath, &lptstrFilePart); 
        }

        ZeroMemory(&CurrentTime,sizeof(SYSTEMTIME));

        GetLocalTime(&CurrentTime);

        StringCchPrintf(
                        Buffer, 1024,
                        TEXT ("%02d.%02d.%02d.%03d %s: "),
                        CurrentTime.wHour,
                        CurrentTime.wMinute,
                        CurrentTime.wSecond,
                        CurrentTime.wMilliseconds,
                        lptstrFilePart
                       );

        // init arg list
        va_start(marker,Format);

        // point to rest of blank buffer
        len = lstrlen(Buffer);

        StringCchVPrintf(&Buffer[len], sizeof(Buffer) - len - 3, Format, marker);

        len = lstrlen(Buffer);
        if (Buffer[len-1] == L'\n' )
        {   
                Buffer[len-1] = L'\r';
                Buffer[len] = L'\n';
                Buffer[len+1] = 0;
        }
        else
        {
                Buffer[len] = L'\r';
                Buffer[len+1] = L'\n';
                Buffer[len+2] = 0;
        }
        OutputDebugString(Buffer);
        va_end(marker);
}

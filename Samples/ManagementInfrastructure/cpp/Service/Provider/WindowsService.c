//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "WindowsService.h"
#include "util.h"

#define ID_PROMPT_USER_STRING 500

MI_Char g_startMode[5][25] = { L"Service boot start",
                           L"Service system start",
                           L"Service auto start",
                           L"Service demand start",
                           L"Service disabled" };

MI_Result SetService(
    _Out_ MSFT_WindowsService* self,
    _In_ SC_HANDLE *phSvcCtlMgr,
    _In_ ENUM_SERVICE_STATUS *lpService,
    _In_ MI_Context* context)
{
    MI_Result result;

    if( NULL == self || NULL == phSvcCtlMgr || NULL == lpService || NULL == context)
        return MI_RESULT_INVALID_PARAMETER;

    result = MSFT_WindowsService_Construct(self, context);
    if(result != MI_RESULT_OK)
        return result;

    // Setting a dummy value in this example for CSCreationClassName key property.
    result = MSFT_WindowsService_Set_SystemCreationClassName(self, SYSTEM_CREATION_CLASS_NAME);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }
    
    // Setting scoping system name
    {
        MI_Char buf[MAX_PATH];
        DWORD bufCharCount = MAX_PATH;
        memset(buf, 0, sizeof(buf));
        if(GetComputerNameW(buf, &bufCharCount))
        {
            result = MSFT_WindowsService_Set_SystemName(self, buf);
            if(result != MI_RESULT_OK)
            {
                MSFT_WindowsService_Destruct(self);
                return result;
            }
        }
    }

    // Setting the CreationClassName
    result = MSFT_WindowsService_Set_CreationClassName(self, CLASS_CREATION_NAME);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }

    // Setting service name
    result = MSFT_WindowsService_Set_Name(self, lpService->lpServiceName);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }

    // Setting the caption as display name
    result = MSFT_WindowsService_Set_Caption(self, lpService->lpDisplayName);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }

    result = MSFT_WindowsService_Set_Started(self, lpService->ServiceStatus.dwCurrentState == SERVICE_RUNNING);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }

    result = MSFT_WindowsService_Set_Name(self, lpService->lpServiceName);
    if(result != MI_RESULT_OK)
    {
        MSFT_WindowsService_Destruct(self);
        return result;
    }

    // Setting start type of service
    {
        LPQUERY_SERVICE_CONFIG lpsc = NULL;
        SC_HANDLE service = NULL;
        DWORD bytesNeeded, cbBufSize = 0;
		
        service = OpenService(*phSvcCtlMgr, lpService->lpServiceName, GENERIC_READ );
        if(NULL == service)
        {
            //Can't open this service.Do the cleanup and return error
            MSFT_WindowsService_Destruct(self);
            return ResultFromWin32Error(GetLastError());
        }

        if( !QueryServiceConfig(service, NULL, 0, &bytesNeeded))
        {
            DWORD dwError = GetLastError();
            if( ERROR_INSUFFICIENT_BUFFER == dwError )
            {
                cbBufSize = bytesNeeded;
                lpsc = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, bytesNeeded);
                if(NULL == lpsc)
                {
                    MSFT_WindowsService_Destruct(self);
                    CloseServiceHandle(service);
                    return ResultFromWin32Error(GetLastError());
                }
            }
            else
            {
                MSFT_WindowsService_Destruct(self);
                CloseServiceHandle(service);
                return ResultFromWin32Error(dwError);
            }
        }

        // Query for service configuration with increased buffer size
        if( !QueryServiceConfig( service, lpsc, cbBufSize, &bytesNeeded) ) 
        {
            MSFT_WindowsService_Destruct(self);
            CloseServiceHandle(service);
            LocalFree(lpsc);
            return ResultFromWin32Error(GetLastError());
        }

        // Setting the service start type
        if(lpsc && lpsc->dwStartType < 5) // valid start type
        {
            result = MSFT_WindowsService_Set_StartMode(self, g_startMode[lpsc->dwStartType]);
            if(result != MI_RESULT_OK)
            {
                MSFT_WindowsService_Destruct(self);
                CloseServiceHandle(service);
                LocalFree(lpsc);
                return ResultFromWin32Error(GetLastError());
            }
        }

        // closing the service handle
        CloseServiceHandle(service);
        LocalFree(lpsc);
    }
    return result;
}

// Validating instance as non NULL
// Validating the existance of all key properties. And making sure that key properties values are same as the one set by provider except for service name.
MI_Result IsValidInstance(_In_ const MSFT_WindowsService* instanceName)
{
    MI_Result result = MI_RESULT_OK;
    // Check to make sure that instance is not null and instance has all the key properties.
    if(instanceName && 
        instanceName->SystemCreationClassName.exists == MI_TRUE &&
        instanceName->CreationClassName.exists == MI_TRUE &&
        instanceName->SystemName.exists == MI_TRUE &&
        instanceName->Name.exists == MI_TRUE)
    {
        // Making sure that key properties are same as the one set by the provider
        if( (_wcsicmp(instanceName->SystemCreationClassName.value, SYSTEM_CREATION_CLASS_NAME) != 0) ||
            (_wcsicmp(instanceName->CreationClassName.value, CLASS_CREATION_NAME) != 0) )
        {
            // The instance with the user passed in key is not found.
            return MI_RESULT_NOT_FOUND;
        }
        else
        {
            MI_Char buf[MAX_PATH];
            DWORD bufCharCount = MAX_PATH;

            // Checking to see SystemName key propertye is same as the one set by the provider
            //memset(buf, 0, sizeof(buf));
            if(GetComputerNameW(buf, &bufCharCount))
            {
                if(_wcsicmp(instanceName->SystemName.value, buf) != 0)
                {
                    return MI_RESULT_NOT_FOUND;
                }
            }
            else
            {
                return MI_RESULT_FAILED;
            }
        }
    }
    else
    {
        result = MI_RESULT_INVALID_PARAMETER;
    }

    return result;
}

MI_Result EnumerateServices(
    _In_ MI_Context* context,
    _In_ MI_Boolean keysOnly)
{
    SC_HANDLE hSvcCtlMgr;
    DWORD dwServiceIndex, dwBytesNeeded, dwServiceCount, dwResumeHandle = 0;
    ENUM_SERVICE_STATUS * lpServiceArray = NULL;
    BOOL returnValue;
    MI_Result result =  MI_RESULT_OK;

    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(context);

    hSvcCtlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (hSvcCtlMgr == NULL)
    {
        // Cannot get access to SCManager object.
        return ResultFromWin32Error(GetLastError());
    }
        
    returnValue = EnumServicesStatus(
        hSvcCtlMgr,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &dwBytesNeeded,
        &dwServiceCount,
        &dwResumeHandle);

    if (!returnValue)
    {
        DWORD lastError = GetLastError();
        if ((lastError == ERROR_INSUFFICIENT_BUFFER) || (lastError == ERROR_MORE_DATA))
        {
            lpServiceArray = (ENUM_SERVICE_STATUS *)AllocateMemory(dwBytesNeeded);
            if (lpServiceArray == NULL)
            {
                CloseServiceHandle(hSvcCtlMgr);
                return MI_RESULT_SERVER_LIMITS_EXCEEDED;
            }

            dwResumeHandle = 0;
            returnValue = EnumServicesStatus(
                hSvcCtlMgr,
                SERVICE_WIN32,
                SERVICE_STATE_ALL,
                lpServiceArray,
                dwBytesNeeded,
                &dwBytesNeeded,
                &dwServiceCount,
                &dwResumeHandle);
            if (!returnValue)
            {
                FreeMemory(lpServiceArray);
                CloseServiceHandle(hSvcCtlMgr);
                return ResultFromWin32Error(GetLastError());
            }
        }
        else
        {
            CloseServiceHandle(hSvcCtlMgr);
            return ResultFromWin32Error(lastError);
        }
    }
    else
    {
        CloseServiceHandle(hSvcCtlMgr);
        return MI_RESULT_FAILED;
    }

    // Enumerating through all the services and posting the instance to wmi service.
    for(dwServiceIndex = 0; dwServiceIndex < dwServiceCount; dwServiceIndex++)
    {
        MSFT_WindowsService serviceInstance;
        
        //Setting service instance properties
        result = SetService(&serviceInstance, &hSvcCtlMgr, &lpServiceArray[dwServiceIndex], context);
        if(result == MI_RESULT_OK)
        {
            result = MSFT_WindowsService_Post(&serviceInstance, context);
            MSFT_WindowsService_Destruct(&serviceInstance);
            if(result != MI_RESULT_OK)
            {
                break;
            }
        }
        else
        {
            // Notifying the user of the failure to query particular service information. 
            // And also requesting for resonse whether to continue or stop processing further
            MI_Boolean bContinue = FALSE;
            MI_Char errMsg[MAX_PATH];
            StringCchPrintfW(errMsg,MAX_PATH,L"Error Querying the  service config %s", lpServiceArray[dwServiceIndex].lpServiceName);
            MI_Context_WriteError(context, result, MI_RESULT_TYPE_MI,errMsg, &bContinue);
		    
            if(!bContinue)
            {
                // The user asked to cancel the operation
                break;
            }
            else
            {
                // Continue with the next service
            }
        }
    }

    FreeMemory(lpServiceArray);
    CloseServiceHandle(hSvcCtlMgr);
    return result;
}

MI_Result GetServiceInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName)
{
    SC_HANDLE hSvcCtlMgr;
    DWORD dwServiceIndex, dwBytesNeeded, dwServiceCount, dwResumeHandle = 0;
    ENUM_SERVICE_STATUS * lpServiceArray = NULL;
    BOOL returnValue;
    MI_Result result =  MI_RESULT_OK;

    result = IsValidInstance(instanceName);
    if(result != MI_RESULT_OK)
    {    
        return result;
    }   

    hSvcCtlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (hSvcCtlMgr == NULL)
    {
        // Cannot get access to SCManager object.
        return ResultFromWin32Error(GetLastError());
    }
        
    returnValue = EnumServicesStatus(
        hSvcCtlMgr,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &dwBytesNeeded,
        &dwServiceCount,
        &dwResumeHandle);

    if (!returnValue)
    {
        DWORD lastError = GetLastError();
        if ((lastError == ERROR_INSUFFICIENT_BUFFER) || (lastError == ERROR_MORE_DATA))
        {
            lpServiceArray = (ENUM_SERVICE_STATUS *)AllocateMemory(dwBytesNeeded);
            if (lpServiceArray == NULL)
            {
                CloseServiceHandle(hSvcCtlMgr);
                return MI_RESULT_SERVER_LIMITS_EXCEEDED;
            }

            dwResumeHandle = 0;
            returnValue = EnumServicesStatus(
                hSvcCtlMgr,
                SERVICE_WIN32,
                SERVICE_STATE_ALL,
                lpServiceArray,
                dwBytesNeeded,
                &dwBytesNeeded,
                &dwServiceCount,
                &dwResumeHandle);
            if (!returnValue)
            {
                FreeMemory(lpServiceArray);
                CloseServiceHandle(hSvcCtlMgr);
                return ResultFromWin32Error(GetLastError());
            }
        }
        else
        {
            CloseServiceHandle(hSvcCtlMgr);
            return ResultFromWin32Error(lastError);
        }
    }
    else
    {
        CloseServiceHandle(hSvcCtlMgr);
        return MI_RESULT_FAILED;
    }

    // Enumerating through all the services and posting the instance to wmi service.
    for(dwServiceIndex = 0; dwServiceIndex < dwServiceCount; dwServiceIndex++)
    {
        // Checking if the name of the service instance requested is present in the list of services
        if(_wcsicmp(instanceName->Name.value, lpServiceArray[dwServiceIndex].lpServiceName) == 0)
        {
            MSFT_WindowsService serviceInstance;     
        
            //Setting service instance properties
            result = SetService(&serviceInstance, &hSvcCtlMgr, &lpServiceArray[dwServiceIndex], context);
            if(result == MI_RESULT_OK)
            {
                result = MSFT_WindowsService_Post(&serviceInstance, context);
                MSFT_WindowsService_Destruct(&serviceInstance);
            }
            break;
        }
    }

    if(dwServiceIndex == dwServiceCount)
    {
        // Could not find the service instance requested.
        result = MI_RESULT_NOT_FOUND;
    }

    FreeMemory(lpServiceArray);

    
    CloseServiceHandle(hSvcCtlMgr);
    return result;
}

void Invoke_StartService(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName)
{
    MI_Char PromptMsg[MAX_PATH];
    MI_Char PromptMsgTemplate[MAX_PATH];
    MI_Boolean bContinue = FALSE;
    SC_HANDLE sc = NULL;
    SC_HANDLE service = NULL;
    MI_Result result;
    LPWSTR lpOrgUILang = NULL;

    result = IsValidInstance(instanceName);
    if(result != MI_RESULT_OK)
    {    
        MI_Context_PostError(context, result, MI_RESULT_TYPE_MI, L"Not a valid instance");
        return;
    }

    sc = OpenSCManager (NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE);
    if (sc == NULL)
    {
        MI_Context_PostError(context, GetLastError(), MI_RESULT_TYPE_WIN32, L"Unable to Start service; OpenSCManager failed");		
        return;
    }
    service = OpenService(sc, instanceName->Name.value, SERVICE_START);
    if (service == NULL)
    {
        // Opening service failed - cannot continue with the operation
        MI_Context_PostError(context, GetLastError(), MI_RESULT_TYPE_WIN32, L"");
        goto CleanupAndExit;
    }

    // SetUILocale reads preferred UI Locale settings set by client
    // and set the UI locale to the current thread preferred UI Languages,
    // following LoadString API will search the installed language packages
    // of <service.dll> (I.E, service.dll.mui) based on the sequence of
    // current thread preferred UI languages setting.
    SetUILocale(context, &lpOrgUILang);
    LoadString(GetModuleHandle(L"service.dll"),
        ID_PROMPT_USER_STRING,
        PromptMsgTemplate,
        sizeof(PromptMsgTemplate)/sizeof(wchar_t));
    // ResetUILocale will reset the current thread preferred UI Languages back
    // to the original setting
    ResetUILocale(lpOrgUILang);
    {
        DWORD_PTR pArgs[1];
        pArgs[0] = (DWORD_PTR)(instanceName->Name.value);
        FormatMessageW(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            (LPCVOID)PromptMsgTemplate,
            0,
            0,
            PromptMsg,
            MAX_PATH - 1,
            (va_list*)pArgs);
        PromptMsg[MAX_PATH - 1] = L'\0';
    }
    result = MI_Context_PromptUser(context, PromptMsg, MI_PROMPTTYPE_NORMAL, &bContinue);
    if(result != MI_RESULT_OK)
    {
        // Something went wrong while prompting the user.
        // Report the error to client and abort the operation.
        MI_Context_PostError(context,result, MI_RESULT_TYPE_MI,L"Failed to Send Prompt to user - Start operation can't continue");		
        goto CleanupAndExit;
    }
    else 
    if(!bContinue)
    {
        result = MI_Context_WriteVerbose(context,L"User said NO to start service prompt");
        MI_Context_PostResult(context, result);
        goto CleanupAndExit;

    }

    // Try to start the service
    if(!StartService(service, 0, NULL))
    {
        DWORD errCode = GetLastError();
        if(errCode == ERROR_SERVICE_ALREADY_RUNNING)
        {
            result = MI_Context_WriteVerbose(context, L"Requested service is already started");
            if(result != MI_RESULT_OK)
            {
                MI_Context_PostError(context, result,  MI_RESULT_TYPE_MI, L"Failed to send verbose message");
                goto CleanupAndExit;
            }
        }
        else
        {
            // Starting service failed.
            MI_Context_PostError(context, errCode, MI_RESULT_TYPE_WIN32, L"Failed to start the service; Failure in StartService");
            goto CleanupAndExit;
        }
    }
    else
    {
        result = MI_RESULT_OK;
    }

    if(result == MI_RESULT_OK)
    {
        // Returning the result for the method.
        MSFT_WindowsService_StartService outInstance;
	
        result = MSFT_WindowsService_StartService_Construct(&outInstance, context);
        if(result != MI_RESULT_OK)
        {
            MI_Context_PostError(context, result, MI_RESULT_TYPE_MI, L"");
            goto CleanupAndExit;
        }

        // MIReturn is special name for the return value from the method.
        result = MSFT_WindowsService_StartService_Set_MIReturn(&outInstance, 0); 
        if(result != MI_RESULT_OK)
        {
            MSFT_WindowsService_StartService_Destruct(&outInstance);
            MI_Context_PostError(context, result, MI_RESULT_TYPE_MI, L"");
            goto CleanupAndExit;
        }

        // Posting the output instance for the method with the output properties and return values set (here there are no output properties except for return value)
        result = MSFT_WindowsService_StartService_Post(&outInstance, context);

        // Output instance is either successfuly posted to wmi service or failed. In either case we need to free up this instance.
        MSFT_WindowsService_StartService_Destruct(&outInstance);
        if(result != MI_RESULT_OK)
        {
            MI_Context_PostError(context, result, MI_RESULT_TYPE_MI, L"");
            goto CleanupAndExit;
        }
        MI_Context_PostResult(context, MI_RESULT_OK);
    }

CleanupAndExit:
    if(service)
        CloseServiceHandle(service);
    if(sc)
        CloseServiceHandle(sc);
}

void Invoke_StopService(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName)
{
    MI_Boolean bContinue = FALSE;
    SC_HANDLE sc = NULL;
    SC_HANDLE service = NULL;
    SERVICE_STATUS stt;
    MI_Char PromptMsg[MAX_PATH];
    HRESULT hr;
    MI_Result result = MI_RESULT_FAILED;

    {
        // Example way to get the custom option passed by client in the provider.
        MI_Type type;
        MI_Value value;
        MI_GetCustomOption(context, L"Force", &type, &value);
        hr = StringCchPrintf(PromptMsg,MAX_PATH,L"Recieved Flag Force %d", value.boolean );
        if(FAILED(hr))
        {
            MI_Context_PostError(context,GetLastError(),MI_RESULT_TYPE_HRESULT, L"Unable to Stop service; StringCchPrintf failed");
            return;
        }
    }

    MI_Context_WriteVerbose(context, PromptMsg);

    sc = OpenSCManager (NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE);
    if (sc == NULL) {
        MI_Context_PostError(context,GetLastError(),MI_RESULT_TYPE_WIN32, L"Unable to Stop service; OpenSCManager failed");		
        return;
    }

    service = OpenService(sc, instanceName->Name.value, SERVICE_STOP);
    if (service == NULL) {
        MI_Context_PostError(context, GetLastError(), MI_RESULT_TYPE_WIN32, L"Failed in OpenService");
        goto CleanupAndExit;
    }

    hr = StringCchPrintf(PromptMsg,MAX_PATH,L"Stopping Service %s",instanceName->Name.value );
    if(FAILED(hr))
    {
        MI_Context_PostError(context,GetLastError(),MI_RESULT_TYPE_HRESULT, L"Unable to Stop service; StringCchPrintf failed");
        goto CleanupAndExit;
    }

    // The following will promot the user even without -Confirm parameter as the prompt type is critical
    if(MI_Context_PromptUser(context, PromptMsg, MI_PROMPTTYPE_NORMAL, &bContinue) != MI_RESULT_OK || (bContinue == FALSE ))
    {
        MI_Context_WriteVerbose(context,L"User said NO to Stop service prompt");
        MI_Context_PostResult(context, MI_RESULT_OK);
        goto CleanupAndExit;
    }

    // Try to stop the service.
    if(!ControlService(service, SERVICE_CONTROL_STOP, &stt))
    {
        DWORD errCode = GetLastError();
        if(errCode == ERROR_SERVICE_NOT_ACTIVE)
        {
            result = MI_Context_WriteVerbose(context, L"Requested service has not been started");
            if(result != MI_RESULT_OK)
            {
                MI_Context_PostError(context, result,  MI_RESULT_TYPE_MI, L"Failure to send verbose message");
                goto CleanupAndExit;
            }
        }
        else
        {
            MI_Context_PostError(context, errCode, MI_RESULT_TYPE_WIN32, L"Failed to stop the service; Failure in ControlService");
            goto CleanupAndExit;
        }
    }
    else
    {
        result = MI_RESULT_OK;
    }

    if(result == MI_RESULT_OK)
    {
        // Setting the output object to be posted.
        MSFT_WindowsService_StopService outInstance;
        result = MSFT_WindowsService_StopService_Construct(&outInstance, context);
        if(result != MI_RESULT_OK)
        {
            MI_Context_PostResult(context, result);
            goto CleanupAndExit;
        }

        result = MSFT_WindowsService_StopService_Set_MIReturn(&outInstance, MI_RESULT_OK);
        if(result != MI_RESULT_OK)
        {
            MSFT_WindowsService_StopService_Destruct(&outInstance);
            MI_Context_PostResult(context, result);
            goto CleanupAndExit;
        }

        result = MSFT_WindowsService_StopService_Post(&outInstance, context);

        // Output instance is either successfuly posted to wmi service or failed. In either case we need to free up this instance.
        MSFT_WindowsService_StopService_Destruct(&outInstance);
        if(result != MI_RESULT_OK)
        {
            MI_Context_PostResult(context, result);
            goto CleanupAndExit;
        }

        MI_Context_PostResult(context, MI_RESULT_OK);
    }

CleanupAndExit:
    if(service)
        CloseServiceHandle(service);
    if(sc)
        CloseServiceHandle(sc);
}

// Converting the win32 failure code to suitable MI_Result code.
MI_Result ResultFromWin32Error(
    DWORD error)
{
    MI_Result result = MI_RESULT_FAILED;
    switch(error)
    {
    case ERROR_FILE_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case ERROR_PATH_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case ERROR_ACCESS_DENIED: 
        result = MI_RESULT_ACCESS_DENIED;
        break;
    case ERROR_INVALID_HANDLE : 
        result = MI_RESULT_INVALID_PARAMETER;
        break; 
    case ERROR_NOT_ENOUGH_MEMORY : 
        result = MI_RESULT_SERVER_LIMITS_EXCEEDED;
        break;     
    case ERROR_INVALID_DATA : 
        result = MI_RESULT_INVALID_PARAMETER;
        break; 
    case ERROR_NOT_SUPPORTED : 
        result = MI_RESULT_NOT_SUPPORTED;
        break;
    case ERROR_INVALID_PARAMETER : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;     
    case ERROR_INSUFFICIENT_BUFFER : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;    
    case ERROR_PROC_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_BAD_PATHNAME : 
        result = MI_RESULT_INVALID_PARAMETER;
        break;        
    case ERROR_ALREADY_EXISTS : 
        result = MI_RESULT_ALREADY_EXISTS;
        break;    
    case ERROR_NO_DATA : 
        result = MI_RESULT_NOT_FOUND;
        break;  
    case ERROR_NOINTERFACE : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_OBJECT_NAME_EXISTS : 
        result = MI_RESULT_ALREADY_EXISTS;
        break; 
    case ERROR_SERVICE_DOES_NOT_EXIST : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case ERROR_NOT_FOUND : 
        result = MI_RESULT_NOT_FOUND;
        break;      
    case ERROR_NO_SUCH_USER : 
        result = MI_RESULT_NOT_FOUND;
        break;       
    case ERROR_NO_SUCH_GROUP : 
        result = MI_RESULT_NOT_FOUND;
        break;   
    case DNS_ERROR_RCODE_NAME_ERROR : 
        result = MI_RESULT_NOT_FOUND;
        break;
    case DNS_INFO_NO_RECORDS : 
        result = MI_RESULT_NOT_FOUND;
        break; 
    default : 
        result = MI_RESULT_FAILED;
        break;        
    }
    return result;
}
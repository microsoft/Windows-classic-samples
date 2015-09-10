//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <Mi.h>
#include "WindowsService.h"
#include "MSFT_WindowsServiceManager.h"

// Streaming instances
MI_Result StreamInstance(_In_ MI_Context* context, _In_z_ MI_Char *propertyName, _In_count_(size) MI_Instance** arrInstances, MI_Uint32 size)
{
    MI_Result result = MI_RESULT_FAILED;
    MI_Value val;
    MI_Uint32 i;

    if(arrInstances == NULL)
        return result;
	
    if(size > 1)
    {
        //Streaming more than one instance at a time.
        val.instancea.data = arrInstances;
        val.instancea.size = size;

        result = MI_Context_WriteStreamParameter(context, propertyName, &val, MI_INSTANCEA, MI_FLAG_STREAM);
    }
    else
    {
        val.instance = *arrInstances;
        result = MI_Context_WriteStreamParameter(context, propertyName, &val, MI_INSTANCE, MI_FLAG_STREAM);
    }

    for( i = 0 ; i < size; i++)
    {	
        MI_Instance_Destruct(arrInstances[i]);
    }

    return result;
}

MI_Result Invoke_GetWindowsServices(
    _In_ MI_Context* context,
    _In_opt_ const MSFT_WindowsServiceManager_GetWindowsServices* in)
{
    SC_HANDLE hSvcCtlMgr;
    DWORD dwServiceIndex, dwBytesNeeded, dwServiceCount, dwResumeHandle = 0;
    ENUM_SERVICE_STATUS * lpServiceArray = NULL;
    BOOL returnValue;
    MI_Result result =  MI_RESULT_OK;
    MI_Boolean bRequiredAllServices = TRUE;

    // If value is zero this method will post all stopped services instances.
    // If the value is 1 this method will return all running service instances.
    // For all other values this method will return all services instances.
    if(in->status.exists == TRUE)
    {
        if(in->status.value < 2)
        {
            bRequiredAllServices = FALSE;
        }
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
        MSFT_WindowsService serviceInstance;

        if(!bRequiredAllServices)
        {
            if( (in->status.value == 0 && lpServiceArray[dwServiceIndex].ServiceStatus.dwCurrentState == SERVICE_STOPPED) ||
                 (in->status.value == 1 && lpServiceArray[dwServiceIndex].ServiceStatus.dwCurrentState == SERVICE_RUNNING) )
            {
                // Desired service instance, process further
            }
            else
            {
                // Proceeding to next service as client is not interest in this service
                continue;
            }
        }
        
        //Setting service instance properties
        result = SetService(&serviceInstance, &hSvcCtlMgr, &lpServiceArray[dwServiceIndex], context);
        if(result == MI_RESULT_OK)
        {
            MI_Instance *instance;
            
            instance = &(serviceInstance.__instance);
            result = StreamInstance(context, L"services", &instance, 1); //Streaming one instance at a time
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

    if(result == MI_RESULT_OK)
    {
        //Posting the output instance with return value
        MSFT_WindowsServiceManager_GetWindowsServices outputInstance;
        result = MSFT_WindowsServiceManager_GetWindowsServices_Construct(&outputInstance, context);
        if(result == MI_RESULT_OK)
        {
            result = MSFT_WindowsServiceManager_GetWindowsServices_Set_MIReturn(&outputInstance, MI_RESULT_OK);
            if(result == MI_RESULT_OK)
            {
                result = MSFT_WindowsServiceManager_GetWindowsServices_Post(&outputInstance, context);
            }
            MSFT_WindowsServiceManager_GetWindowsServices_Destruct(&outputInstance);
        }
    }

    return result;
}
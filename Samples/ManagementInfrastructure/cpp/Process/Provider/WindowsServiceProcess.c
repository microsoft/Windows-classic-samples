//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "WindowsServiceProcess.h"

#define MSFT_WINDOWS_PROCESS L"MSFT_WindowsProcess"
#define MSFT_WINDOWS_SERVICE L"MSFT_WindowsService"


void GetServiceInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace, _In_ MI_Uint32 handleID,
                            _In_opt_ const MI_Instance *existingEndPoint)
{
    SC_HANDLE hSvcCtlMgr;
    BOOL returnValue;
    LPENUM_SERVICE_STATUS_PROCESS pServiceList = NULL ;
    MI_Uint32 dwByteCount = 0, hEnumHandle = 0 , dwEntryCount = 0 , iCount = 0;
    MI_Char queryString[MAX_PATH*4], tempQueryString[MAX_PATH];
    MI_Boolean bFirst = MI_TRUE;
    HRESULT hr = StringCchPrintf(queryString, MAX_PATH*4,L"Select * from MSFT_WindowsService");
    if( FAILED(hr) )
    {
        MI_Context_PostResult(context, ResultFromWin32Error(HRESULT_TO_WIN32(hr)));
        return;
    }
    hSvcCtlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (hSvcCtlMgr == NULL)
    {
        // Cannot get access to SCManager object.
        MI_Context_PostResult(context, ResultFromWin32Error(GetLastError()));
        return;
    }    

    returnValue = EnumServicesStatusEx(
        hSvcCtlMgr,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_ACTIVE | SERVICE_INACTIVE,
        ( LPBYTE ) pServiceList,
        dwByteCount,
        (LPDWORD)&dwByteCount,
        (LPDWORD)&dwEntryCount,
        (LPDWORD)&hEnumHandle,
        NULL );
    
    if (!returnValue)
    {
        MI_Uint32 lastError = GetLastError();
        if ((lastError == ERROR_INSUFFICIENT_BUFFER) || (lastError == ERROR_MORE_DATA))
        {
            pServiceList = (LPENUM_SERVICE_STATUS_PROCESS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwByteCount);
            if( pServiceList == NULL )
            {
                CloseServiceHandle(hSvcCtlMgr);
                MI_Context_PostResult(context, MI_RESULT_FAILED);
                return;                  
            }
            
            memset(pServiceList, 0, dwByteCount) ;
            returnValue = EnumServicesStatusEx(
                hSvcCtlMgr,
                SC_ENUM_PROCESS_INFO,
                SERVICE_WIN32,
                SERVICE_ACTIVE | SERVICE_INACTIVE,
                ( LPBYTE ) pServiceList,
                dwByteCount,
                (LPDWORD)&dwByteCount,
                (LPDWORD)&dwEntryCount,
                (LPDWORD)&hEnumHandle,
                NULL );

            if( !returnValue)
            {
                HeapFree(GetProcessHeap(), 0, pServiceList);
                CloseServiceHandle(hSvcCtlMgr);
                MI_Context_PostResult(context, MI_RESULT_FAILED);
                return;                  
            }
        }
        else
        {
            CloseServiceHandle(hSvcCtlMgr);
            MI_Context_PostResult(context, MI_RESULT_FAILED);
            return;            
        }
    }
    else
    {
        CloseServiceHandle(hSvcCtlMgr);
        MI_Context_PostResult(context, MI_RESULT_FAILED);
        return;
    }
        
    // Enumerating through all services and finding the ones which has process ID = handleID
    for( iCount = 0 ; iCount< dwEntryCount ; iCount++)
    {
        if( pServiceList[iCount].ServiceStatusProcess.dwProcessId == handleID )
        {
            if( bFirst )
            {
                hr = StringCchPrintfW(tempQueryString, MAX_PATH,L" where Name=\"%s\"", pServiceList[iCount].lpServiceName);
                bFirst = MI_FALSE;
            }
            else
            {
                hr = StringCchPrintfW(tempQueryString, MAX_PATH,L" or Name=\"%s\"", pServiceList[iCount].lpServiceName);
            }
            if( FAILED(hr) )
            {
                HeapFree(GetProcessHeap(), 0, pServiceList);
                CloseServiceHandle(hSvcCtlMgr);
                MI_Context_PostResult(context, MI_RESULT_FAILED);
                return;                
            }
            
            hr = StringCchCatW(queryString, MAX_PATH*4,tempQueryString);
            if( FAILED(hr) )
            {
                HeapFree(GetProcessHeap(), 0, pServiceList);
                CloseServiceHandle(hSvcCtlMgr);
                MI_Context_PostResult(context, MI_RESULT_FAILED);
                return;
            }            
        }
    }

    if( bFirst ) // no matching instance
    {
        MI_Context_PostResult(context, MI_RESULT_OK);
    }
    else
    {
        GetInstances(context, nameSpace, queryString, existingEndPoint , MI_TRUE);  
    }
    
    HeapFree(GetProcessHeap(), 0, pServiceList);
    CloseServiceHandle(hSvcCtlMgr);    
}

void GetProcessInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace, _In_z_ const MI_Char* serviceName,
                             _In_opt_ const MI_Instance *existingEndPoint)
{
    SC_HANDLE hSvcCtlMgr, hSvcHandle;
    BOOL returnValue;
    SERVICE_STATUS_PROCESS StatusInfo ;
    MI_Uint32 dwBytesNeeded = 0;
    MI_Char queryString[MAX_PATH], tempQueryString[MAX_PATH];
    HRESULT hr = StringCchPrintf(queryString, MAX_PATH,L"Select * from MSFT_WindowsProcess");
    if( FAILED(hr) )
    {
        MI_Context_PostResult(context, ResultFromWin32Error(HRESULT_TO_WIN32(hr)));
        return;
    }
    hSvcCtlMgr = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (hSvcCtlMgr == NULL)
    {
        // Cannot get access to SCManager object.
        MI_Context_PostResult(context, ResultFromWin32Error(GetLastError()));
        return;
    } 

    hSvcHandle = OpenService(hSvcCtlMgr, serviceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_INTERROGATE );
    if (hSvcHandle == NULL)
    {
        // Cannot get access to SCManager object.
        CloseServiceHandle(hSvcCtlMgr); 
        MI_Context_PostResult(context, ResultFromWin32Error(GetLastError()));
        return;
    } 

    returnValue = QueryServiceStatusEx( hSvcHandle,
                       SC_STATUS_PROCESS_INFO,
                       ( UCHAR * ) &StatusInfo ,
                       sizeof ( StatusInfo ) ,
                       (LPDWORD)&dwBytesNeeded );

    if( !returnValue)
    {
        CloseServiceHandle(hSvcCtlMgr);
        CloseServiceHandle(hSvcHandle);
        MI_Context_PostResult(context, ResultFromWin32Error(GetLastError()));
        return;
    }
    
    hr = StringCchPrintfW(tempQueryString, MAX_PATH,L" where Handle=\"%d\"", StatusInfo.dwProcessId);
    if( FAILED(hr) )
    {
        CloseServiceHandle(hSvcCtlMgr);
        CloseServiceHandle(hSvcHandle);
        MI_Context_PostResult(context, MI_RESULT_FAILED);
        return;                
    }
    
    hr = StringCchCatW(queryString, MAX_PATH,tempQueryString);
    if( FAILED(hr) )
    {
        CloseServiceHandle(hSvcCtlMgr);
        CloseServiceHandle(hSvcHandle);
        MI_Context_PostResult(context, MI_RESULT_FAILED);
        return;
    }                                                  
    
    GetInstances(context, nameSpace, queryString, existingEndPoint, MI_FALSE);
    CloseServiceHandle(hSvcCtlMgr);
    CloseServiceHandle(hSvcHandle);
    
}

void GetInstances(_In_ MI_Context* context, _In_opt_z_ const MI_Char* nameSpace, _In_z_ const MI_Char* queryString,
                    _In_opt_ const MI_Instance *existingEndPoint, _In_ MI_Boolean bExistingEndPointIsProcessObject)
{
    MI_Session localSession = MI_SESSION_NULL;
    MI_Result result = MI_RESULT_OK;
    MI_Boolean moreResults = MI_FALSE;
    MI_Operation miOperation = MI_OPERATION_NULL;
    
    result = MI_Context_GetLocalSession(context ,&localSession);
    if( result != MI_RESULT_OK)
    {
        MI_Context_PostResult(context, result);
        return ;
    }
    MI_Session_QueryInstances(&localSession, 0, NULL, nameSpace, L"WQL", queryString, NULL, &miOperation);

        /* Must loop through all results until moreResults == MI_FALSE */
    do
    {
        const MI_Instance *miInstance;
        MI_Result _miResult;
        const MI_Char *errorString = NULL;
        const MI_Instance *errorDetails = NULL;            

        /* Retrieve a single instance result */
        _miResult = MI_Operation_GetInstance(&miOperation, &miInstance, &moreResults, &result, &errorString, &errorDetails);
        if (_miResult != MI_RESULT_OK)
        {
            if( errorDetails )
            {
                MI_Context_PostCimError(context, errorDetails);
            }
            else
            {
                MI_Context_PostResult(context, result);
            }
            MI_Operation_Close(&miOperation);
            return;
        }
        
        if (miInstance)
        {
            if( bExistingEndPointIsProcessObject )
            {
                result = PostInstance(context, existingEndPoint, miInstance, bExistingEndPointIsProcessObject);
            }
            else
            {
                result = PostInstance(context, miInstance, existingEndPoint, bExistingEndPointIsProcessObject);
            }
        }

    } while (result == MI_RESULT_OK && moreResults == MI_TRUE);
    MI_Operation_Close(&miOperation);
    MI_Context_PostResult(context, result);
}

MI_Result PostInstance( _In_ MI_Context* context, _In_opt_ const MI_Instance *miInstanceProcess, _In_opt_ const MI_Instance *miInstanceService, 
                        _In_ MI_Boolean bExistingEndPointIsProcessObject)
{
    MI_Result result = MI_RESULT_OK;
    MSFT_WindowsServiceProcess associationObject;
    MI_Value valueProcess, valueService;
    if( bExistingEndPointIsProcessObject && !miInstanceProcess && miInstanceService)
    {
        return MI_Context_PostInstance(context, miInstanceService);
    }
    else if( !bExistingEndPointIsProcessObject && !miInstanceService && miInstanceProcess)
    {
        return MI_Context_PostInstance(context, miInstanceProcess);
    }
    
    //Need to send association object
    result = MSFT_WindowsServiceProcess_Construct(&associationObject,context);
    if( result != MI_RESULT_OK)
    {
        return result;
    }
    
    valueProcess.reference = (MI_Instance*)miInstanceProcess;
    result = MI_Instance_SetElement(&associationObject.__instance, L"Process", &valueProcess ,MI_REFERENCE,0);
    if( result != MI_RESULT_OK)
    {
        MSFT_WindowsServiceProcess_Destruct(&associationObject);
        return result;
    }    

    valueService.reference = (MI_Instance*)miInstanceService;
    result = MI_Instance_SetElement(&associationObject.__instance, L"Service", &valueService ,MI_REFERENCE,0);
    if( result != MI_RESULT_OK)
    {
        MSFT_WindowsServiceProcess_Destruct(&associationObject);
        return result;
    }   

    result = MI_Context_PostInstance(context, &associationObject.__instance);
    MSFT_WindowsServiceProcess_Destruct(&associationObject);
    return result;
}


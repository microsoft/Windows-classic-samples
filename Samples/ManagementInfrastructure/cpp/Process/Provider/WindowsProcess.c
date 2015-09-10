//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <windows.h>
#include <stdlib.h>
#include <assert.h>
#include <combaseapi.h>
#include "helper.h"
#include "MSFT_WindowsProcess.h"
#include "WindowsProcess.h"

MI_Result GetOSName(
    __out_ecount(nSize) LPWSTR buf,
    __in size_t nSize)
{
    MI_Result result = MI_RESULT_FAILED;
    OSVERSIONINFO osvi;
    assert(buf != NULL);
    buf[0] = L'\0';
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning( push )
#pragma warning( disable : 4996 ) // 'GetVersionEx': was declared deprecated
    if(GetVersionEx(&osvi))
    {
        if(SUCCEEDED(StringCchPrintfW(buf, nSize, L"Microsoft Windows Version %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber)))
        {
            result = MI_RESULT_OK;
        }
        else
        {
            result = ResultFromWin32Error(GetLastError());
        }
    }
    else
    {
        result = ResultFromWin32Error(GetLastError());
    }
    return result;
#pragma warning( pop ) // 'GetVersionEx': was declared deprecated
}

BOOL EnablePrivilege()
{
    LUID PrivilegeRequired ;
    DWORD dwLen = 0, iCount = 0;
    BOOL bRes = FALSE;
    HANDLE hToken = NULL;
    BYTE *pBuffer = NULL;
    TOKEN_PRIVILEGES* pPrivs = NULL;

    bRes = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &PrivilegeRequired);
    if( !bRes) return FALSE;
    
    bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken); 
    if(!bRes) return FALSE;

    bRes = GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLen);
    if (TRUE == bRes)
    {
        CloseHandle(hToken);
        return FALSE;
    }
    pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLen);
    if(NULL == pBuffer) return FALSE;
    
    if (!GetTokenInformation(hToken, TokenPrivileges, pBuffer, dwLen, &dwLen)) 
    {
        CloseHandle(hToken);
        HeapFree(GetProcessHeap(), 0, pBuffer);
        return FALSE;
    }

    // Iterate through all the privileges and enable the one required
    bRes = FALSE;
    pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
    for(iCount = 0; iCount < pPrivs->PrivilegeCount; iCount++)
    {
        if (pPrivs->Privileges[iCount].Luid.LowPart == PrivilegeRequired.LowPart &&
          pPrivs->Privileges[iCount].Luid.HighPart == PrivilegeRequired.HighPart )
        {
            pPrivs->Privileges[iCount].Attributes |= SE_PRIVILEGE_ENABLED;
            // here it's found
            bRes = AdjustTokenPrivileges(hToken, FALSE, pPrivs, dwLen, NULL, NULL);
            break;
        }
    }

    CloseHandle(hToken);
    HeapFree(GetProcessHeap(), 0, pBuffer);    
    return bRes;
}

MI_Result ConvertFileTimeToDateTime(
    _In_ LPFILETIME pfTime, 
    _Out_ MI_Datetime *pdTime)
{
    SYSTEMTIME sTime;
    assert(pfTime != NULL);
    assert(pdTime != NULL);
    memset(pdTime, 0, sizeof(MI_Datetime));
    if(FileTimeToSystemTime(pfTime, &sTime))
    {
        pdTime->isTimestamp = TRUE;
        pdTime->u.timestamp.year = sTime.wYear;
        pdTime->u.timestamp.month = sTime.wMonth;
        pdTime->u.timestamp.day = sTime.wDay;
        pdTime->u.timestamp.hour = sTime.wHour;
        pdTime->u.timestamp.minute = sTime.wMinute;
        pdTime->u.timestamp.second = sTime.wSecond;
        pdTime->u.timestamp.microseconds = sTime.wMilliseconds * 1000;
        pdTime->u.timestamp.utc = 0;
    }
    else
    {
        return ResultFromWin32Error(GetLastError());
    }

    return MI_RESULT_OK;
}

//
// Helper function to set properties of MSFT_WindowsProcess instance,
// All the key properties must be set, while other properties could
// be optional and depends on the provider.
//
MI_Result SetInstance(
    _Out_ MSFT_WindowsProcess* self,
    DWORD processId,
    _In_ MI_Context* context
    )
{
    HANDLE hProcess = NULL;
    MI_Result result = MSFT_WindowsProcess_Construct(self, context);
    if(result != MI_RESULT_OK)
    {
        return result;
    }

    // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        processId);
    if(NULL != hProcess)
    {
        // Set process handle in the instance - putting the PID of the process in string format
        MI_Char PIDInString[15];
        if(! _i64tow_s(processId, PIDInString, sizeof(PIDInString)/sizeof(MI_Char), 10) )
        {
            result = MSFT_WindowsProcess_Set_Handle(self, PIDInString);
            if(result != MI_RESULT_OK)
            {
                CloseHandle( hProcess );
                MSFT_WindowsProcess_Destruct(self);
                return result;
            }
        }
        else
        {
            CloseHandle( hProcess );
            MSFT_WindowsProcess_Destruct(self);
            return MI_RESULT_FAILED;
        }

        // Setting the name of the process
        {
            HMODULE hMod;
            DWORD cbNeeded;
            MI_Char szProcessName[MAX_PATH] = L"";

            if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
                    &cbNeeded) )
            {
                GetModuleBaseName( hProcess, hMod, szProcessName, 
                                    sizeof(szProcessName)/sizeof(TCHAR) );
            }
            
            result = MSFT_WindowsProcess_Set_Name(self, szProcessName);
            if( result != MI_RESULT_OK )
            {
                CloseHandle( hProcess );
                MSFT_WindowsProcess_Destruct(self);
                return result;
            }

            // Setting the process name as caption too
            result = MSFT_WindowsProcess_Set_Caption(self, szProcessName);
            if(result != MI_RESULT_OK)
            {
                CloseHandle( hProcess );
                MSFT_WindowsProcess_Destruct(self);
                return result;
            }
        }

        // Setting the execution state to running
        result = MSFT_WindowsProcess_Set_ExecutionState(self, 3);
        if(result != MI_RESULT_OK)
        {
            CloseHandle( hProcess );
            MSFT_WindowsProcess_Destruct(self);
            return result;
        }

        // Setting the priority of the process
        {
            DWORD priority;
            priority = GetPriorityClass(hProcess);
            if(priority)
            {
                result = MSFT_WindowsProcess_Set_Priority(self, priority);
                if(result != MI_RESULT_OK)
                {
                    CloseHandle(hProcess);
                    MSFT_WindowsProcess_Destruct(self);
                    return result;
                }
            }
        }

        // Setting the WorkingSetSize property with the minimum working set size of the process.
        {
            SIZE_T minimumWorkingSetSize, maximumWorkingSetSize;

            if(GetProcessWorkingSetSize(hProcess, &minimumWorkingSetSize, &maximumWorkingSetSize))
            {
                result = MSFT_WindowsProcess_Set_WorkingSetSize(self, minimumWorkingSetSize);
                if(result != MI_RESULT_OK)
                {
                    CloseHandle(hProcess);
                    MSFT_WindowsProcess_Destruct(self);
                    return result;
                }
            }
        }

        // Setting CreationDate, KernelModeTime, UserModeTime
        {
            FILETIME fCreationTime, fKernelTime, fUserTime, fExitTime;
            if(GetProcessTimes(hProcess, &fCreationTime, &fExitTime, &fKernelTime, &fUserTime))
            {
                MI_Datetime dTime;
                result = ConvertFileTimeToDateTime(&fCreationTime, &dTime);
                if(result == MI_RESULT_OK)
                {
                    result = MSFT_WindowsProcess_Set_CreationDate(self, dTime);
                    if(result != MI_RESULT_OK)
                    {
                        CloseHandle(hProcess);
                        MSFT_WindowsProcess_Destruct(self);
                        return result;
                    }
                }
                
                {
                    // Setting KernelModeTime in milliseconds
                    // FILETIME contains two 32-bit values that combine to form a 64 bit count of 100-nanosecond time units.

                    UINT64 temp, timeInMilliseconds;
                    temp = fKernelTime.dwHighDateTime;
                    temp = temp << 32;
                    temp += fKernelTime.dwLowDateTime;
                    //temp = (UINT64) (((UINT32)fKernelTime.dwHighDateTime << 32) | (UINT32)fKernelTime.dwLowDateTime); 
                    timeInMilliseconds = temp / 10000; // Converting 100-nanosecond time units to milliseconds.

                    result = MSFT_WindowsProcess_Set_KernelModeTime (self, timeInMilliseconds);
                    if(result != MI_RESULT_OK)
                    {
                        CloseHandle(hProcess);
                        MSFT_WindowsProcess_Destruct(self);
                        return result;
                    }

                    // Setting UserModeTime in milliseconds
                    temp = fUserTime.dwHighDateTime;
                    temp = temp << 32;
                    temp += fUserTime.dwLowDateTime;
                    timeInMilliseconds = temp / 10000; // Converting 100-nanosecond time units to milliseconds.

                    result = MSFT_WindowsProcess_Set_UserModeTime (self, timeInMilliseconds);
                    if(result != MI_RESULT_OK)
                    {
                        CloseHandle(hProcess);
                        MSFT_WindowsProcess_Destruct(self);
                        return result;
                    }
                }
                
                result = ConvertFileTimeToDateTime(&fCreationTime, &dTime);
                if(result == MI_RESULT_OK)
                {
                    result = MSFT_WindowsProcess_Set_CreationDate(self, dTime);
                    if(result != MI_RESULT_OK)
                    {
                        CloseHandle(hProcess);
                        MSFT_WindowsProcess_Destruct(self);
                        return result;
                    }
                }
            }
        }

        // Release the handle to the process.
        CloseHandle( hProcess );

        // Setting a dummy value in this example for CSCreationClassName key property.
        result = MSFT_WindowsProcess_Set_CSCreationClassName(self, CS_CREATION_CLASS_NAME);
        if(result != MI_RESULT_OK)
        {
            MSFT_WindowsProcess_Destruct(self);
            return result;
        }
          
        result = MSFT_WindowsProcess_Set_OSCreationClassName(self, OS_CREATION_CLASS_NAME);
        if(result != MI_RESULT_OK)
        {
            MSFT_WindowsProcess_Destruct(self);
            return result;
        }

        // Current creation class is MSFT_WindowsProcess
        result = MSFT_WindowsProcess_Set_CreationClassName(self, CLASS_CREATION_NAME);
        if(result != MI_RESULT_OK)
        {
            MSFT_WindowsProcess_Destruct(self);
            return result;
        }
                        
        //OSName 
        {
            MI_Char buf[INFO_BUFFER_SIZE];

            result = GetOSName(buf, INFO_BUFFER_SIZE);
            if(result == MI_RESULT_OK)
            {
                result = MSFT_WindowsProcess_Set_OSName(self, buf);
                if(result != MI_RESULT_OK)
                {
                    MSFT_WindowsProcess_Destruct(self);
                    return result;
                }
            }
        }

        //CSName - host name
        {
            MI_Char buf[INFO_BUFFER_SIZE];
            DWORD bufCharCount = INFO_BUFFER_SIZE;
            memset(buf, 0, sizeof(buf));
            if(GetComputerNameW(buf, &bufCharCount))
            {
                result = MSFT_WindowsProcess_Set_CSName(self, buf);
                if(result != MI_RESULT_OK)
                {
                    MSFT_WindowsProcess_Destruct(self);
                    return result;
                }
            }
        }
    }
    else
    {
        // Converting the failure code to suitable MI_Result code.
        result = ResultFromWin32Error(GetLastError());
        MSFT_WindowsProcess_Destruct(self);
    }

    return result;
}

//
// Validating the given instance is non NULL, and
// all key properties having same value as set by the provider,
// check handle property exists as well.
//
MI_Result IsValidInstance(_In_ const MSFT_WindowsProcess* instanceName)
{
    MI_Result result = MI_RESULT_OK;
    // Check to make sure that instance is not null and instance has all the key properties.
    if(instanceName && 
        instanceName->CSCreationClassName.exists == MI_TRUE &&
        instanceName->CSName.exists == MI_TRUE &&
        instanceName->OSCreationClassName.exists == MI_TRUE &&
        instanceName->OSName.exists == MI_TRUE &&
        instanceName->CreationClassName.exists == MI_TRUE &&
        instanceName->Handle.exists == MI_TRUE)
    {
        // Making sure that key properties are same as the one set by the provider
        if( (_wcsicmp(instanceName->CSCreationClassName.value, CS_CREATION_CLASS_NAME) != 0) ||
            (_wcsicmp(instanceName->OSCreationClassName.value, OS_CREATION_CLASS_NAME) != 0) ||
            (_wcsicmp(instanceName->CreationClassName.value, CLASS_CREATION_NAME) != 0) )
        {
            // The instance with the user passed in key is not found.
            return MI_RESULT_NOT_FOUND;
        }
        else
        {
            MI_Char buf[INFO_BUFFER_SIZE];
            DWORD bufCharCount = INFO_BUFFER_SIZE;

            // Checking to see OSName key property is same as the one set by the provider
            result = GetOSName(buf, INFO_BUFFER_SIZE);
            if(result == MI_RESULT_OK)
            {
                if(_wcsicmp(instanceName->OSName.value, buf) != 0)
                {
                    return MI_RESULT_NOT_FOUND;
                }
            }
            else
            {
                return result;
            }

            // Checking to see CSName key propertye is same as the one set by the provider
            //memset(buf, 0, sizeof(buf));
            if(GetComputerNameW(buf, &bufCharCount))
            {
                if(_wcsicmp(instanceName->CSName.value, buf) != 0)
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

MI_Result EnumerateProcesses(
    _In_ MI_Context* context,
    _In_ MI_Boolean keysOnly)
{
    DWORD aProcesses[1024]; 
    DWORD cbNeeded; 
    DWORD cProcesses;
    unsigned int i;
    MI_Result result = MI_RESULT_OK;

    // -*- use it later
    MI_UNREFERENCED_PARAMETER(keysOnly);

    // Here enabling privileges will help to list processes from different users if the caller has not enabled it already.
    EnablePrivilege( );

    // Get the list of process identifiers.
    // -*- change the size to operate dynamically
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return MI_RESULT_FAILED;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.

    for ( i = 0; i < cProcesses; i++ )
    {
        MSFT_WindowsProcess instance;

        result = SetInstance(&instance, aProcesses[i], context);
        if(result != MI_RESULT_OK)
        {
            // Ignoring the failure to open specific processes as the caller may not have permission to open all processes.
            continue;
        }

        // Post instance to wmi server
        result = MSFT_WindowsProcess_Post(&instance, context);
            
        // Now we can free the instance which will free the resources allocated as part of setting the properties.
        MSFT_WindowsProcess_Destruct(&instance);
        if(result != MI_RESULT_OK)
            break;
    }

    return result;
}


MI_Result GetProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName)
{
    MI_Result result = MI_RESULT_OK;
    MSFT_WindowsProcess instance;

    EnablePrivilege();
    
    result = IsValidInstance(instanceName);
  
    if(result == MI_RESULT_OK)
    {
        DWORD processId;
        // Retrieving the process id from handle parameter
        processId = (DWORD)_wtoi64(instanceName->Handle.value);
        
        result = SetInstance(&instance, processId, context);
        if(result == MI_RESULT_OK)
        {
            // Post instance to wmi server
            result = MSFT_WindowsProcess_Post(&instance, context);
            
            // Now we can free the instance which will free the resources allocated as part of setting the properties.
            MSFT_WindowsProcess_Destruct(&instance);
        }
    }
    return result;
}

MI_Result DeleteProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName)
{
    MI_Result result = MI_RESULT_OK;
    MI_UNREFERENCED_PARAMETER(context);

    EnablePrivilege();
   
    result = IsValidInstance(instanceName);
   
    if(result == MI_RESULT_OK)
    {
        DWORD processId;
        HANDLE hProcess;
        // Retrieving the process id from handle parameter
        processId = (DWORD)_wtoi64(instanceName->Handle.value);
        
        hProcess = OpenProcess( PROCESS_TERMINATE,
                        FALSE, processId );
        if(NULL != hProcess)
        {
            if(! TerminateProcess(hProcess, 0) )
            {
                result = ResultFromWin32Error(GetLastError());
            }

            CloseHandle(hProcess);
        }
        else
        {
            // Converting the failure code to suitable MI_Result code.
            result = ResultFromWin32Error(GetLastError());
        }

    }
    return result;
}

MI_Result ModifyProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* modifiedInstance)
{
    MI_Result result = MI_RESULT_OK;
    MI_UNREFERENCED_PARAMETER(context);

    EnablePrivilege();
    
    // Modify has to find the instance with the key properties passed in and modify non-key properties passed by client.
    // And then post the modified instance.
    result = IsValidInstance(modifiedInstance);
   
    if(result == MI_RESULT_OK)
    {
        DWORD processId;
        HANDLE hProcess;
        // Retrieving the process id from handle parameter
        processId = (DWORD)_wtoi64(modifiedInstance->Handle.value);
        
        hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processId );
        if(NULL != hProcess)
        {
            // In this sample provider modifying only Priority property of the instance if it is changed by client.
            if(modifiedInstance->Priority.exists == MI_TRUE)
            {
                if(SetPriorityClass(hProcess, modifiedInstance->Priority.value))
                {
                    MSFT_WindowsProcess instance;
                    
                    // Retrieving the modified process instance       
                    result = SetInstance(&instance, processId, context);
                    if(result == MI_RESULT_OK)
                    {
                        // Post modified instance to wmi server
                        result = MSFT_WindowsProcess_Post(&instance, context);
            
                        // Now we can free the instance which will free the resources allocated as part of setting the properties.
                        MSFT_WindowsProcess_Destruct(&instance);
                    }
                }
                else
                {
                    result = ResultFromWin32Error(GetLastError());
                }
            }

            CloseHandle(hProcess);
        }
        else
        {
            // Converting the failure code to suitable MI_Result code.
            result = ResultFromWin32Error(GetLastError());
        }

    }
    
    return result;
}

MI_Result Invoke_SetPriority(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_SetPriority* in)
{
    MI_Result result = MI_RESULT_OK;
    MI_UNREFERENCED_PARAMETER(context);

    // Checking to see if the priority value is passed in by client.
    if(NULL == in || in->Priority.exists == FALSE)
    {
        return MI_RESULT_INVALID_PARAMETER;
    }

    EnablePrivilege();
    
    // Instance method has to find the instance with the key properties passed in and modify non-key properties passed by client.
    // And then post the output instance.
    result = IsValidInstance(instanceName);
   
    if(result == MI_RESULT_OK)
    {
        DWORD processId;
        HANDLE hProcess;
        // Retrieving the process id from handle parameter
        processId = (DWORD)_wtoi64(instanceName->Handle.value);
        
        hProcess = OpenProcess( PROCESS_SET_INFORMATION,
                        FALSE, processId );
        if(NULL != hProcess)
        {
            // Setting the new priority
            if(SetPriorityClass(hProcess, in->Priority.value))
            {
                MSFT_WindowsProcess_SetPriority outputInstance;

                result = MSFT_WindowsProcess_SetPriority_Construct(&outputInstance, context);
                if(result == MI_RESULT_OK)
                {
                    result = MSFT_WindowsProcess_SetPriority_Set_MIReturn(&outputInstance, MI_RESULT_OK);
                    if(result == MI_RESULT_OK)
                    {
                        // Posting the output instance with the return value set
                        result = MSFT_WindowsProcess_SetPriority_Post(&outputInstance, context);
                    }
                    MSFT_WindowsProcess_SetPriority_Destruct(&outputInstance);
                }
            }
            else
            {
                result = ResultFromWin32Error(GetLastError());
            }

            CloseHandle(hProcess);
        }
        else
        {
            // Converting the failure code to suitable MI_Result code.
            result = ResultFromWin32Error(GetLastError());
        }

    }
    
    return result;
}


//
// Helper function of allocating memory from process heap
// 
// Argument:
//      dwBytes     number of bytes to allocate.
//  
// Return value:
//      allocated memory address
//
LPVOID AllocateMemory(SIZE_T dwBytes)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);
}

//
// Helper function of freeing memory
// 
// Argument:
//      lpMem       memory address to free.
//
void FreeMemory(LPVOID lpMem)
{
    HeapFree(GetProcessHeap(), 0, lpMem);
}

//
// Create a process based on given commandline.
// The newly created process will be posted back to client
// upon successfully creation.
//
MI_Result CreateProcessHelper(
    _In_ MI_Context* context,
    _In_z_ const MI_Char* commandLine,
    _Out_ MSFT_WindowsProcess* pProcess)
{
    PROCESS_INFORMATION processInformation;
    STARTUPINFO startupInfo;
    BOOL creationResult;
    size_t nLength = wcslen(commandLine) + 1;
    LPWSTR cmdLine;
    LPWSTR cmdLineTemp;
    HRESULT revertResult;
    ZeroMemory(pProcess, sizeof(MSFT_WindowsProcess));

    // Revert to host account, otherwise CreateProcess does not work
    revertResult = CoRevertToSelf();
    if (revertResult != S_OK)
    {
        return ResultFromWin32Error(revertResult);
    }

    cmdLine = (LPWSTR)AllocateMemory(nLength * 2 * sizeof(wchar_t));
    cmdLineTemp = cmdLine;
    if (commandLine[0] != L'"')
    {
        cmdLineTemp[0] = L'"';
        cmdLineTemp ++;
    }
    CopyMemory(cmdLineTemp, commandLine, nLength * sizeof(wchar_t));
    if (commandLine[0] != L'"')
    {
        cmdLineTemp += (nLength - 1);
        cmdLineTemp[0] = L'"';
        cmdLineTemp[1] = L'\0';
    }
    ZeroMemory(&processInformation, sizeof(processInformation));
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    // The target process will be created in session of the
    // provider host process. If the provider was hosted in
    // wmiprvse.exe process, the target process will be launched
    // in session 0 and UI is invisible to the logged on user,
    // but the process can be found through task manager.
    creationResult = CreateProcess(
        NULL,                   // No module name (use command line)
        cmdLine,                // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, // creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory 
        &startupInfo,           // Pointer to STARTUPINFO structure
        &processInformation);   // Pointer to PROCESS_INFORMATION structure
    FreeMemory(cmdLine);
    if (!creationResult)
    {
        // create process failed
        DWORD errorCode = GetLastError();
        return ResultFromWin32Error(errorCode);
    }

    // close process and thread handles
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);

    // create process successfully,
    // read the MSFT_WindowsProcess instance and post back to client
    return SetInstance(pProcess, processInformation.dwProcessId, context);
}

//
// Create a process based on given instance.
// Only CommandLine property will be used to create a process.
// The newly created process will be posted back to client
// upon successfully creation.
//
MI_Result InvokeIntrisicCreateMethod(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* newInstance)
{
    if (newInstance->CommandLine.exists == MI_TRUE)
    {
        MSFT_WindowsProcess newProcess;
        MI_Result r = CreateProcessHelper(context, newInstance->CommandLine.value, &newProcess);
        if (r == MI_RESULT_OK)
        {
            // Posting the created instance is necessary, this is the way client will know the key properties of instance created.
            r = MSFT_WindowsProcess_Post(&newProcess, context);
            MSFT_WindowsProcess_Destruct(&newProcess);
        }
        return r;
    }
    return MI_RESULT_INVALID_PARAMETER;
}

//
// Create a process based on given argument,
// CommandLine argument will be used here.
// The reference to newly created process will be posted back to client
// upon successfully creation.
//
MI_Result InvokeExtrinsicCreateMethod(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess_Create* createArg)
{
    if (createArg->CommandLine.exists == MI_TRUE)
    {
        MSFT_WindowsProcess_Create createResult;
        MSFT_WindowsProcess newProcess;
        MI_Result r = CreateProcessHelper(context, createArg->CommandLine.value, &newProcess);
        if (r == MI_RESULT_OK)
        {
            // build creation result object
            r = MSFT_WindowsProcess_Create_Construct(&createResult, context);
            if (r == MI_RESULT_OK)
            {
                // set the newly created process object to result object
                r = MSFT_WindowsProcess_Create_Set_Process(&createResult, (CIM_Process *)&newProcess);
                if (r == MI_RESULT_OK)
                {
                    r = MSFT_WindowsProcess_Create_Set_MIReturn(&createResult, 0);
                }
                if (r == MI_RESULT_OK)
                {
                    r = MSFT_WindowsProcess_Create_Post(&createResult, context);
                }
                MSFT_WindowsProcess_Create_Destruct(&createResult);
            }
            MSFT_WindowsProcess_Destruct(&newProcess);
        }
        return r;
    }
    return MI_RESULT_INVALID_PARAMETER;
}

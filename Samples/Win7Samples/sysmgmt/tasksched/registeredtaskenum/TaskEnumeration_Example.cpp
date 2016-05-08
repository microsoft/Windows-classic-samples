// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This sample enumerates all the tasks registered in the root task 
// folder on the local computer and displays their name and status.
//

#define _WIN32_DCOM

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
//  Include the task header file.
#include "rpcsal.h"
#include "taskschd.h"
# pragma comment(lib, "taskschd.lib")
# pragma comment(lib, "comsupp.lib")

using namespace std;

void main(void)
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if( FAILED(hr) )
    {
        printf("\nCoInitializeEx failed: %x", hr );
        return;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);
    if( FAILED(hr) )
    {
        printf("\nCoInitializeSecurity failed: %x", hr );
        return;
    }

    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService *pService = NULL;
    hr = CoCreateInstance( CLSID_TaskScheduler,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_ITaskService,
                           (void**)&pService );  
    if( FAILED(hr) )
    {
          printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
          CoUninitialize();
          return;
    }
        
    //  Connect to the local task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if( FAILED(hr) )
    {
        printf("ITaskService::Connect failed: %x", hr );
        pService->Release();
        CoUninitialize();
        return;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.
    ITaskFolder *pRootFolder = NULL;
    hr = pService->GetFolder( _bstr_t( L"\\") , &pRootFolder );

    pService->Release();
    if( FAILED(hr) )
    {
        printf("Cannot get Root Folder pointer: %x", hr );
        CoUninitialize();
        return;
    }
    
    //  -------------------------------------------------------
    //  Get the registered tasks in the folder.
    IRegisteredTaskCollection* pTaskCollection = NULL;
    hr = pRootFolder->GetTasks( NULL, &pTaskCollection );

    pRootFolder->Release();
    if( FAILED(hr) )
    {
        printf("Cannot get the registered tasks.: %x", hr);
        CoUninitialize();
        return;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);
    if( FAILED(hr) )
    {
        printf("Cannot get the task collection.: %x", hr);
        CoUninitialize();
        return;
    }

    if( numTasks == 0 )
    {
        printf("\nNo Tasks are currently running");
        pTaskCollection->Release();
        CoUninitialize();
        return;
    }

    printf("\nNumber of Tasks : %d", numTasks );

    //  -------------------------------------------------------
    //  Visit each task in the folder.
    for(LONG i=0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        _bstr_t taskName;
        TASK_STATE taskState;
        _bstr_t taskStateStr;

        hr = pTaskCollection->get_Item( _variant_t(i+1), &pRegisteredTask );
        if( FAILED(hr) )
        {
            printf("Cannot get the registered task: %x", hr);
            continue;
        }
        
        hr = pRegisteredTask->get_Name(taskName.GetAddress());
        if( FAILED(hr) )
        {
            printf("Cannot get the registered task name: %x", hr);
            pRegisteredTask->Release();
            continue;
        }

        hr = pRegisteredTask->get_State(&taskState);
        if( FAILED(hr) )
        {
            printf("Cannot get the registered task state: %x", hr);
        }
        else
        {
            printf("\n\nTask Name: %S", (LPCWSTR)taskName);

            switch(taskState)
            {
            case TASK_STATE_DISABLED:
                taskStateStr = "disabled";
                break;
            case TASK_STATE_QUEUED:
                taskStateStr = "queued";
                break;
            case TASK_STATE_READY:
                taskStateStr = "ready";
                break;
            case TASK_STATE_RUNNING:
                taskStateStr = "running";
                break;
            default:
                taskStateStr = "unknown";
                break;
            }
            printf("\n\tState: %s", (LPCSTR)taskStateStr);
        }

        pRegisteredTask->Release();
    }

    pTaskCollection->Release();
    CoUninitialize();
    return;
}


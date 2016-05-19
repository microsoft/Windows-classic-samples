// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Implement class TClientNotificationWork.  A simple class to store data
//      that represents work to be performed by the Threadpool to make
//      callbacks into the Function Discovery client.

#include "stdafx.h"

TClientNotificationWork::TClientNotificationWork():
    WorkType(OnUpdate)
{
    ZeroMemory(&WorkData, sizeof(WorkData));
}  // TClientNotificationWork::TClientNotificationWork

TClientNotificationWork* TClientNotificationWork::CreateClientOnErrorWork(HRESULT hr)
{
    TClientNotificationWork* pClientNotificationWork = new(std::nothrow) TClientNotificationWork;
    
    if (pClientNotificationWork)
    {
        pClientNotificationWork->WorkType = OnError;
        pClientNotificationWork->WorkData.OnErrorWork.hr = hr;
    }

    return pClientNotificationWork;
}  // TClientNotificationWork::CreateClientOnErrorWork

TClientNotificationWork* TClientNotificationWork::CreateClientOnEventWork(DWORD EventId)
{
    TClientNotificationWork* pClientNotificationWork = new(std::nothrow) TClientNotificationWork;
    
    if (pClientNotificationWork)
    {
        pClientNotificationWork->WorkType = OnEvent;
        pClientNotificationWork->WorkData.OnEventWork.EventId = EventId;
    }

    return pClientNotificationWork;
}  // TClientNotificationWork::CreateClientOnEventWork

TClientNotificationWork* TClientNotificationWork::CreateClientOnUpdateWork(
    QueryUpdateAction QueryUpdateAction, 
    __in TFunctionInstanceInfo* pFunctionInstanceInfo)
{
    TClientNotificationWork* pClientNotificationWork = new(std::nothrow) TClientNotificationWork;
    
    if (pClientNotificationWork)
    {
        pFunctionInstanceInfo->AddRef();

        pClientNotificationWork->WorkType = OnUpdate;
        pClientNotificationWork->WorkData.OnUpdateWork.QueryUpdateAction = QueryUpdateAction;
        pClientNotificationWork->WorkData.OnUpdateWork.pFunctionInstanceInfo = pFunctionInstanceInfo;
    }

    return pClientNotificationWork;
}  // TClientNotificationWork::CreateClientOnUpdateWork

TClientNotificationWork::~TClientNotificationWork()
{
    if (OnUpdate == WorkType)
    {
        WorkData.OnUpdateWork.pFunctionInstanceInfo->Release();
        WorkData.OnUpdateWork.pFunctionInstanceInfo = NULL;
    }

}  // TClientNotificationWork::~TClientNotificationWork
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Define class TClientNotificationWork.  A class to store data
//      that represents work to be performed by the threadpool to make
//      callbacks into the Function Discovery client.

#pragma once

enum TClientNotificationWorkType
{
    OnError,
    OnEvent,
    OnUpdate
};  // TClientNotificationWorkType

class TClientNotificationWork:
    public TList<TClientNotificationWork>::TListEntry
{
public:
    static TClientNotificationWork* CreateClientOnErrorWork(
        HRESULT hr);

    static TClientNotificationWork* CreateClientOnEventWork(
        DWORD EventId);

    static TClientNotificationWork* CreateClientOnUpdateWork(
        QueryUpdateAction QueryUpdateAction, 
        __in TFunctionInstanceInfo* pFunctionInstanceInfo);

    ~TClientNotificationWork();

protected:
    TClientNotificationWork();

public:

    TClientNotificationWorkType WorkType;
    union TWorkData
    {
        struct TOnErrorWork
        {
            HRESULT hr;
        } OnErrorWork;

        struct TOnEventWork
        {
            DWORD EventId;
        } OnEventWork;

        struct TOnUpdateWork
        {
            QueryUpdateAction QueryUpdateAction;
            TFunctionInstanceInfo* pFunctionInstanceInfo;
        } OnUpdateWork;

    } WorkData;
};  // TClientNotificationWork
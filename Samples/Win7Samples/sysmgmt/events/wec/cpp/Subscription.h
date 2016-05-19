// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include <windows.h>
#include <EvColl.h>
#include <vector>
#include <string>
#include <strsafe.h>


//Track properties of the Subscription
typedef struct _SUBSCRIPTION
{
    std::wstring Name;
    std::wstring Description;
    std::wstring URI;
    std::wstring Query;
    std::wstring DestinationLog;

    std::wstring EventSource;
    BOOL EventSourceStatus;
    std::wstring EventSourceUserName;
    std::wstring EventSourcePassword;
    std::wstring CommonUserName;
    std::wstring CommonPassword;


    EC_SUBSCRIPTION_CONFIGURATION_MODE ConfigMode;
    EC_SUBSCRIPTION_DELIVERY_MODE DeliveryMode;
    DWORD MaxItems;
    DWORD MaxLatencyTime;
    DWORD HeartbeatInerval;

    EC_SUBSCRIPTION_CONTENT_FORMAT ContentFormat;
    EC_SUBSCRIPTION_CREDENTIALS_TYPE CredentialsType;

    BOOL SubscriptionStatus;
    BOOL ReadExistingEvents;
} SUBSCRIPTION;

//Track Runtime Status
typedef struct _RUNTIME_STATUS
{
    std::wstring ActiveStatus;
    DWORD LastError;
    std::wstring LastErrorMessage;
    std::wstring NextRetryTime;

} RUNTIME_STATUS;

//Subscription Management Functions
DWORD CreateSubscription ( const SUBSCRIPTION& sub);
DWORD EnumerateSubscriptions ( void );

DWORD GetRuntimeStatus ( LPCWSTR subscriptionName);
DWORD RetrySubscription(LPCWSTR subscriptionName);
DWORD GetSubscription(LPCWSTR subscriptionName);


//Subscription Information
DWORD GetProperty(EC_HANDLE hSubscription,  EC_SUBSCRIPTION_PROPERTY_ID propID, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vProperty);
DWORD SetProperty(EC_HANDLE hSubscription, EC_SUBSCRIPTION_PROPERTY_ID propID, DWORD flags, PEC_VARIANT& propertyValue);
DWORD GetArrayProperty(EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray, EC_SUBSCRIPTION_PROPERTY_ID propID, DWORD arrayIndex, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vProperty);

DWORD GetStatus(LPCWSTR subscriptionName, LPCWSTR eventSource, EC_SUBSCRIPTION_RUNTIME_STATUS_INFO_ID statusInfoID, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vStatus);

//Event Source Management Functions
DWORD AddEventSource (  EC_HANDLE hSubscription,  std::wstring eventSource, BOOL status, std::wstring eventSourceUserName, std::wstring eventSourcePassword );


//Helpers - Conversion Functions - Utils.cpp
std::wstring ConvertEcConfigurationMode( DWORD code );
std::wstring ConvertEcDeliveryMode( DWORD code );
std::wstring ConvertEcContentFormat( DWORD code );
std::wstring ConvertEcCredentialsType( DWORD code );
std::wstring ConvertEcDateTime( ULONGLONG code );


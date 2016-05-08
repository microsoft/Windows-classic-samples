// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "Subscription.h"

DWORD GetRuntimeStatus ( LPCWSTR subscriptionName)
{

    DWORD dwEventSourceCount, dwRetVal = ERROR_SUCCESS;

    std::vector<BYTE> buffer;
    std::vector<BYTE> eventSourceBuffer;
    PEC_VARIANT vStatus, vProperty, vEventSource;
    EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray =NULL;
    EC_HANDLE hSubscription;

    RUNTIME_STATUS runtimeStatus;

    //Open the subscription for query
    hSubscription = EcOpenSubscription(subscriptionName, EC_READ_ACCESS, EC_OPEN_EXISTING);
    if (!hSubscription)
    {
        return GetLastError();
    }

    dwRetVal = GetProperty(hSubscription, EcSubscriptionEventSources, 0,buffer, vProperty);
    
    if ( ERROR_SUCCESS != dwRetVal )
        goto Cleanup;
        

   //Ensure that we have obtained handle to the Array Property
    if ( vProperty->Type != EcVarTypeNull && vProperty->Type!= EcVarObjectArrayPropertyHandle)
    {
        dwRetVal = ERROR_INVALID_DATA;
        goto Cleanup;
    }
    
    hArray = (vProperty->Type == EcVarTypeNull) ? NULL: vProperty->PropertyHandleVal ;

    if( !hArray)
    {
		dwRetVal = ERROR_INVALID_DATA;
		goto Cleanup;
    }
	
    // Get the EventSources array size (number of elements) 
    if ( !EcGetObjectArraySize(hArray,
                                        &dwEventSourceCount ) )
    {
        dwRetVal = GetLastError();
        goto Cleanup;
    }
    
    for ( DWORD i = 0; i < dwEventSourceCount ; i++)
    {

        dwRetVal = GetArrayProperty( hArray, 
                                                 EcSubscriptionEventSourceAddress,
                                                 i,
                                                 0,
                                                 eventSourceBuffer,
                                                 vEventSource);

        if (ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }

        if (vEventSource->Type != EcVarTypeNull && vEventSource->Type != EcVarTypeString)
        {
            dwRetVal = ERROR_INVALID_DATA;
            goto Cleanup;
        }

        LPCWSTR eventSource = (vEventSource->Type == EcVarTypeNull) ? NULL: vEventSource->StringVal;

        if (!eventSource)
            continue;
        
        
        dwRetVal = GetStatus( subscriptionName, 
                                        eventSource,
                                        EcSubscriptionRunTimeStatusActive, 
                                        0, 
                                        buffer, 
                                        vStatus);
    
        if( ERROR_SUCCESS != dwRetVal )
            goto Cleanup;


        if( vStatus->Type != EcVarTypeUInt32 )
        {
            dwRetVal = ERROR_INVALID_DATA;
            goto Cleanup;
        }

        switch (vStatus->UInt32Val)
        {
            case EcRuntimeStatusActiveStatusActive:
                  runtimeStatus.ActiveStatus = L"Active";
                  break;
            case EcRuntimeStatusActiveStatusDisabled:
                  runtimeStatus.ActiveStatus = L"Disabled";
                  break;
           case EcRuntimeStatusActiveStatusInactive:
                  runtimeStatus.ActiveStatus = L"Inactive";
                  break;
            case EcRuntimeStatusActiveStatusTrying:
                  runtimeStatus.ActiveStatus = L"Trying";
                  break;
            default:
                  runtimeStatus.ActiveStatus = L"Unknown Status";
                  break;
       }

        //Get Subscription Last Error
        dwRetVal = GetStatus( subscriptionName, 
                                        eventSource, 
                                        EcSubscriptionRunTimeStatusLastError , 
                                        0, 
                                        buffer, 
                                        vStatus);

        if( ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }

        if ( vStatus->Type != EcVarTypeUInt32 )
        {
            dwRetVal = ERROR_INVALID_DATA;
            goto Cleanup;
        }
        
        runtimeStatus.LastError = vStatus->UInt32Val;

        //Obtain the associated Error Message
        dwRetVal = GetStatus( subscriptionName, 
                                        eventSource, 
                                        EcSubscriptionRunTimeStatusLastErrorMessage , 
                                        0, 
                                        buffer, 
                                        vStatus);

        if( ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }
         
         if ( vStatus->Type != EcVarTypeNull && vStatus->Type!= EcVarTypeString)
         {
             dwRetVal = ERROR_INVALID_DATA;
             goto Cleanup;
         }
          
         if( vStatus->Type != EcVarTypeNull)
         {
            runtimeStatus.LastErrorMessage = vStatus->StringVal;
         }
         else
         {
            runtimeStatus.LastErrorMessage = L"";
         }

        //Obtain the Next Retry Time
        dwRetVal = GetStatus( subscriptionName, 
                                        eventSource, 
                                        EcSubscriptionRunTimeStatusNextRetryTime, 
                                        0, 
                                        buffer, 
                                        vStatus);

        if( ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }
         
         if ( vStatus->Type != EcVarTypeNull && vStatus->Type!= EcVarTypeDateTime)
         {
             dwRetVal = ERROR_INVALID_DATA;
             goto Cleanup;
         }
          
         if( vStatus->Type != EcVarTypeNull)
         {
            runtimeStatus.NextRetryTime = ConvertEcDateTime(vStatus->DateTimeVal);
         }
         else
         {
            runtimeStatus.NextRetryTime = L"";
         }

	
         wprintf(L"\nEventSource[%u]\n",  i);
         wprintf(L"    Address: %s\n", eventSource);
         wprintf(L"    Runtime Status: %s\n", runtimeStatus.ActiveStatus.c_str());
         wprintf(L"    Last Error: %u\n", runtimeStatus.LastError);
         
         if( 0 != runtimeStatus.LastError )
         {
            wprintf(L"    Last Error Message: %s\n", runtimeStatus.LastErrorMessage.c_str());
         }
         else
         {
            wprintf(L"    Last Error Message: No Error\n");
         }
		 
         wprintf(L"    Next Retry Time: %s\n", runtimeStatus.NextRetryTime.c_str());
		 
    }

Cleanup:

   if(hArray)
   	EcClose(hArray);
   
   EcClose(hSubscription);
   return dwRetVal;
}

//Helper function to obtain the actual data for the specified EC_SUBSCRIPTION_RUNTIME_STATUS_INFO_ID
DWORD GetStatus(LPCWSTR subscriptionName, LPCWSTR eventSource, EC_SUBSCRIPTION_RUNTIME_STATUS_INFO_ID statusInfoID, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vStatus)
{

    DWORD dwBufferSize, dwRetVal = ERROR_SUCCESS;

    buffer.clear();
    buffer.resize(sizeof(EC_VARIANT));
    
    if ( !EcGetSubscriptionRunTimeStatus( subscriptionName,
                                                        statusInfoID,
                                                        eventSource,
                                                        flags,
                                                        (DWORD) buffer.size(),
                                                        (PEC_VARIANT) &buffer[0],
                                                        &dwBufferSize))
    {
        dwRetVal = GetLastError();

        if( ERROR_INSUFFICIENT_BUFFER ==  dwRetVal)
        {
            dwRetVal = ERROR_SUCCESS;
            buffer.resize(dwBufferSize);
            if(!EcGetSubscriptionRunTimeStatus( subscriptionName,
                                                              statusInfoID,
                                                              eventSource,
                                                              flags,
                                                              (DWORD) buffer.size(),
                                                              (PEC_VARIANT) &buffer[0],
                                                              &dwBufferSize))
            {
                dwRetVal = GetLastError();
            }
        }
    }

    if ( ERROR_SUCCESS == dwRetVal)
    {
        vStatus = (PEC_VARIANT) &buffer[0];
    }
    else
    {
        vStatus = NULL;
    }

    return dwRetVal;

}

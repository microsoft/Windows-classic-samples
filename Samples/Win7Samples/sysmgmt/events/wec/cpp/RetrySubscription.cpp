// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Subscription.h"

//
//
// Note:
// If you want to retry the entire subscription, EcSaveSubscription()
// without any value changes will do the same work.
//
//
DWORD RetrySubscription(LPCWSTR subscriptionName)
{

    DWORD dwRetVal, dwEventSourceCount;
    
    EC_HANDLE hSubscription;
    EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray=NULL;

    PEC_VARIANT vProperty, vEventSource;
    std::vector<BYTE> buffer, eventSourceBuffer;

      //Open an existing subscription for reading
    hSubscription = EcOpenSubscription( subscriptionName, 
                                                      EC_READ_ACCESS, 
                                                      EC_OPEN_EXISTING);
    if (!hSubscription)
        return GetLastError();

    //Get the event sources collection
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionEventSources, 
                                      0,
                                      buffer, 
                                      vProperty);

     if ( ERROR_SUCCESS != dwRetVal )
    {
	 goto Cleanup;
    }

   //Ensure that we have obtained handle to the Array Property
    if ( vProperty->Type != EcVarTypeNull && vProperty->Type!= EcVarObjectArrayPropertyHandle)
    {
        dwRetVal =  ERROR_INVALID_DATA;
	 goto Cleanup;
    }

    hArray = (vProperty->Type == EcVarTypeNull) ? NULL: vProperty->PropertyHandleVal ;

    if(!hArray)
    {
	dwRetVal = ERROR_INVALID_DATA;
	goto Cleanup;
    }
	
    // Get the EventSources array size (number of elements) 
    if ( !EcGetObjectArraySize( hArray,
                                         &dwEventSourceCount ) )
    {
        dwRetVal = GetLastError();
	 goto Cleanup;
    }

    //Retry for all the event sources in the subcription
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
            dwRetVal =  ERROR_INVALID_DATA;
	     goto Cleanup;
        }

        LPCWSTR eventSource = (vEventSource->Type == EcVarTypeNull) ? NULL: vEventSource->StringVal;

        if (!eventSource)
            continue;

        if(!EcRetrySubscription( subscriptionName, 
                                        eventSource, 
                                        0))
        {
            dwRetVal =  GetLastError();
	     goto Cleanup;
        }
    }

Cleanup:

    if(hArray)
		EcClose(hArray);
	
    EcClose(hSubscription);
    return dwRetVal;
}

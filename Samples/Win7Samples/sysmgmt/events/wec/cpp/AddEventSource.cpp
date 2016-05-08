// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Subscription.h"

DWORD AddEventSource ( EC_HANDLE hSubscription, std::wstring eventSource, BOOL status, std::wstring eventSourceUserName, std::wstring eventSourcePassword )
{


    DWORD dwEventSourceCount, dwRetVal = ERROR_SUCCESS;
    PEC_VARIANT vProperty = NULL;
    std::vector<BYTE> buffer;

    //Used to access the event source collection
    EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray = NULL;


    if (!hSubscription)
        return ERROR_INVALID_PARAMETER;

  
    // Get the EventSources array so a new event source can be added for the specified target.
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionEventSources, 
                                      0,
                                      buffer, 
                                      vProperty);
    
    if ( ERROR_SUCCESS != dwRetVal )
    {
        return dwRetVal;
    }
    
    //Event Sources is a collection
    //Ensure that we have obtained handle to the Array Property 
    if ( vProperty->Type != EcVarTypeNull  && vProperty->Type != EcVarObjectArrayPropertyHandle)
    {
        dwRetVal = ERROR_INVALID_DATA;
	 goto Cleanup;
    }

    hArray = (vProperty->Type == EcVarTypeNull)? NULL: vProperty->PropertyHandleVal;

    if(!hArray)
    {
	dwRetVal = ERROR_INVALID_DATA;
	goto Cleanup;

    }
        
    if ( !EcGetObjectArraySize( hArray,
                                        &dwEventSourceCount ) )
    {
	dwRetVal = GetLastError();
	goto Cleanup;
        
    }


    // Add a new EventSource to the EventSources array object.
    if ( !EcInsertObjectArrayElement( hArray, 
                                                 dwEventSourceCount) )
    {
	dwRetVal = GetLastError();
	goto Cleanup;

    }


    // Set the new EventSource's address property
    EC_VARIANT vPropertyAddress;

    vPropertyAddress.Type = EcVarTypeString;
    vPropertyAddress.StringVal = eventSource.c_str();

    if ( !EcSetObjectArrayProperty( hArray,
                                               EcSubscriptionEventSourceAddress, 
                                               dwEventSourceCount,
                                               0,
                                               &vPropertyAddress ) )
    {
		dwRetVal = GetLastError();
		goto Cleanup;

    }
    
    // Set the EventSource's UserName/Password property 
    if( eventSourceUserName.length() > 0 )
    {
        EC_VARIANT vPropertyCredentials;

        vPropertyCredentials.Type = EcVarTypeString;
        vPropertyCredentials.StringVal = eventSourceUserName.c_str();

        if( !EcSetObjectArrayProperty(hArray, 
                                                 EcSubscriptionEventSourceUserName, 
                                                 0, 
                                                 dwEventSourceCount,
                                                 &vPropertyCredentials ) )
        {

		dwRetVal = GetLastError();
		goto Cleanup;

        }

        //Set Password to empty ("") if Password is not specified - blank password(?)
        vPropertyCredentials.StringVal = (eventSourcePassword.length() > 0) ? eventSourcePassword.c_str() : L"";

        if( !EcSetObjectArrayProperty(hArray,
                                                 EcSubscriptionEventSourcePassword, 
                                                 0, 
                                                 dwEventSourceCount, 
                                                 &vPropertyCredentials ) )
        {

		dwRetVal = GetLastError();
		goto Cleanup;

        }
    }

    // Set the new EventSource's Enabled property

    vPropertyAddress.Type = EcVarTypeBoolean;
    vPropertyAddress.BooleanVal = status;

    if ( !EcSetObjectArrayProperty(hArray, 
                                              EcSubscriptionEventSourceEnabled,
                                              dwEventSourceCount, 
                                              0, 
                                              &vPropertyAddress ) )
    {
       dwRetVal = GetLastError();
	goto Cleanup;

    }

Cleanup:

    if(hArray)
        EcClose(hArray);
    
    return dwRetVal;
}

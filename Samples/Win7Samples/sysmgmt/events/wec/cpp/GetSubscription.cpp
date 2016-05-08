// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Subscription.h"




typedef enum _SUB_TYPE
{
    SubTypeConfigurationMode =0,
    SubTypeCredentialsType,
    SubTypeDeliveryMode,
    SubTypeContentFormat,
    SubTypeNone
} SUB_TYPE;


void  PrintProperty(PEC_VARIANT vProperty, EC_VARIANT_TYPE varType, SUB_TYPE subType);



DWORD GetSubscription(LPCWSTR subscriptionName)
{

    DWORD dwRetVal = ERROR_SUCCESS;
    EC_HANDLE hSubscription;
    PEC_VARIANT vProperty;
    std::vector<BYTE> buffer;
   
    
    //Event Source property is a collection 

    PEC_VARIANT vEventSource;
    EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray = NULL;
    DWORD dwEventSourceCount;


    if (!subscriptionName ||  0 == wcslen(subscriptionName) )
        return ERROR_INVALID_PARAMETER;
    
    //Open Subscription:
    hSubscription = EcOpenSubscription( subscriptionName,
                                                      EC_READ_ACCESS, 
                                                      EC_OPEN_EXISTING);
    if (!hSubscription)
        return GetLastError();

    wprintf(L"\n\n");

    //Subscription ID
    wprintf(L"Subscription ID: %s\n", subscriptionName); 

    //Description
    wprintf(L"Description: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionDescription, 
                                      0, 
                                      buffer, 
                                      vProperty);

    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);

    //URI
    wprintf(L"URI: ");
    dwRetVal = GetProperty(hSubscription, EcSubscriptionURI, 0, buffer, vProperty);

    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);


    //Subscription Enabled/Disabled
    wprintf(L"Enabled: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionEnabled, 
                                      0, 
                                      buffer, 
                                      vProperty);

    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeBoolean, SubTypeNone);

    //Subscription Configuration Mode
    wprintf(L"Configuration Mode: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionConfigurationMode, 
                                      0, 
                                      buffer, 
                                      vProperty);

    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeConfigurationMode);

    //Query
    wprintf(L"Query: ");
    dwRetVal = GetProperty( hSubscription,
                                      EcSubscriptionQuery, 
                                      0, 
                                      buffer, 
                                      vProperty);

    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);

    //DeliveryMode
    wprintf(L"Delivery Mode: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionDeliveryMode, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeDeliveryMode);


    //Max Items
    wprintf(L"Max Items: ");
    dwRetVal = GetProperty( hSubscription,
                                      EcSubscriptionDeliveryMaxItems, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeNone);
    
    //Max Latency Time
    wprintf(L"Max Latency Time: ");
    dwRetVal = GetProperty( hSubscription,
                                      EcSubscriptionDeliveryMaxLatencyTime, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeNone);

    //Heartbeat Interval
    wprintf(L"Heartbeat Interval: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionHeartbeatInterval, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeNone);

    //Locale
    wprintf(L"Locale: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionLocale, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);

    //Content Format
    wprintf(L"Content Format: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionContentFormat, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeContentFormat);

    //Log File
    wprintf(L"Log File: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionLogFile, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);
    
    //Credentials Type
    wprintf(L"Credentials Type: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionCredentialsType, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeUInt32, SubTypeCredentialsType);

    //Common UserName
    wprintf(L"Common User Name: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionCommonUserName, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeString, SubTypeNone);

    //CommonUserPassword
    wprintf(L"Common Password: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionCommonPassword, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    wprintf(L"*\n");

    //Read Existing Events
    wprintf(L"Read Existing Events: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionReadExistingEvents, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeBoolean, SubTypeNone);
    

    //Expiry
    wprintf(L"Expiry: ");
    dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionExpires, 
                                      0, 
                                      buffer, 
                                      vProperty);
    
    if (ERROR_SUCCESS != dwRetVal )
        goto Cleanup;

    PrintProperty(vProperty, EcVarTypeDateTime, SubTypeNone);

     //Event Source
     dwRetVal = GetProperty( hSubscription, 
                                      EcSubscriptionEventSources, 
                                      0, 
                                      buffer, 
                                      vProperty);

    if (ERROR_SUCCESS != dwRetVal)
        goto Cleanup;
    
    if ( vProperty->Type != EcVarTypeNull && vProperty->Type!= EcVarObjectArrayPropertyHandle)
    {
	    dwRetVal = ERROR_INVALID_DATA;
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
        wprintf(L"\nEventSource [%u]: \n", i);

        wprintf(L"         Address: ");
        dwRetVal = GetArrayProperty( hArray, 
                                                 EcSubscriptionEventSourceAddress,
                                                 i,
                                                 0,
                                                 buffer,
                                                 vEventSource);

        if (ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }

        PrintProperty(vEventSource, EcVarTypeString, SubTypeNone);

        wprintf(L"         Enabled: ");
        dwRetVal = GetArrayProperty( hArray, 
                                                 EcSubscriptionEventSourceEnabled, 
                                                 i,
                                                 0,
                                                 buffer,
                                                 vEventSource);

        if (ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }

        PrintProperty(vEventSource, EcVarTypeBoolean, SubTypeNone);

       wprintf(L"         UserName: ");
       dwRetVal = GetArrayProperty( hArray, 
                                                EcSubscriptionEventSourceUserName, 
                                                i,
                                                0,
                                                buffer,
                                                vEventSource);

        if (ERROR_SUCCESS != dwRetVal)
        {
            goto Cleanup;
        }

        PrintProperty(vEventSource, EcVarTypeString, SubTypeNone);

       wprintf(L"         Password: ");
       dwRetVal = GetArrayProperty( hArray, 
                                                EcSubscriptionEventSourcePassword, 
                                                i,
                                                0,
                                                buffer,
                                                vEventSource);

       // By Default Reading of the Password is not allowed. An ACCESS DENIED Error will be returned
        if (ERROR_SUCCESS != dwRetVal && ERROR_ACCESS_DENIED!= dwRetVal)
        {
            wprintf(L"Error Obtaining Data\n");
            continue;
        }
        else
        {
            dwRetVal = ERROR_SUCCESS;
            wprintf(L"*\n");
        }

    }

Cleanup:

	if(hArray)
		EcClose(hArray);
	
    EcClose(hSubscription);
    
    return dwRetVal;
    
}


void  PrintProperty(PEC_VARIANT vProperty, EC_VARIANT_TYPE varType, SUB_TYPE subType)
{

    if ( ( !vProperty ) || ( ( vProperty->Type!= (DWORD) EcVarTypeNull ) && ( vProperty->Type != (DWORD) varType ) ) )
    {
        wprintf(L"Failed to Obtain Data\n");
        return;
    }

    switch (vProperty->Type)
    {
        case EcVarTypeString:
            wprintf(L"%s\n", vProperty->StringVal);
            break;

        case EcVarTypeUInt32:
            switch (subType)
            {
                case SubTypeConfigurationMode:
                    wprintf(L"%s\n", ConvertEcConfigurationMode(vProperty->UInt32Val).c_str());
                    break;

                case SubTypeCredentialsType:
                    wprintf(L"%s\n", ConvertEcCredentialsType( vProperty->UInt32Val).c_str());
                    break;

                case SubTypeContentFormat:
                    wprintf(L"%s\n", ConvertEcContentFormat( vProperty->UInt32Val).c_str());
                    break;

                case SubTypeDeliveryMode:
                    wprintf(L"%s\n", ConvertEcDeliveryMode( vProperty->UInt32Val).c_str());
                    break;

                default:
                    wprintf(L"%u\n", vProperty->UInt32Val);
            }
            break;

        case EcVarTypeBoolean:
            wprintf(L"%s\n", (vProperty->BooleanVal) ? L"True": L"False");
            break;

        case EcVarTypeDateTime:
            wprintf(L"%s\n", ConvertEcDateTime( vProperty->DateTimeVal).c_str());
            break;

        case EcVarTypeNull:
        default:
            wprintf(L" - \n");

    }

    return;
}



    


    



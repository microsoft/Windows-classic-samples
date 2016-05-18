// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "Subscription.h"


DWORD CreateSubscription(const SUBSCRIPTION& sub)
{
    DWORD dwRetVal = ERROR_SUCCESS;

    EC_HANDLE hSubscription;

    if ( sub.Name.empty() || sub.URI.empty() || sub.EventSource.empty() || sub.Query.empty() )
    {
        return ERROR_INVALID_PARAMETER;
    }


    hSubscription = EcOpenSubscription( sub.Name.c_str(), 	
	                                                   EC_READ_ACCESS | EC_WRITE_ACCESS, 
       	                                            EC_CREATE_NEW);

    if ( !hSubscription)
    {
        return GetLastError();
    }

    //Set the properties for the subscription
    EC_VARIANT vPropertyValue;

    //Property: Description (EcSubscriptionDescription)
    vPropertyValue.Type = EcVarTypeString;
    vPropertyValue.StringVal = sub.Description.c_str();

    if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionDescription, NULL, &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }


    //Property: URI (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionURI)
    vPropertyValue.Type = EcVarTypeString;
    vPropertyValue.StringVal = sub.URI.c_str();

    if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionURI, NULL, &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }


    //Property: Query (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionQuery)
    vPropertyValue.Type = EcVarTypeString;
    vPropertyValue.StringVal = sub.Query.c_str();

    if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionQuery, NULL, &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }

    //Property: Destination Log File (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionLogFile)
    vPropertyValue.Type = EcVarTypeString;
    vPropertyValue.StringVal = sub.DestinationLog.c_str();

    if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionLogFile, NULL, &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }

    //Property: Configuration Mode (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionConfigurationMode)
    vPropertyValue.Type = EcVarTypeUInt32;
    vPropertyValue.UInt32Val = sub.ConfigMode;

    if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionConfigurationMode, NULL, &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }

    //Accept Delivery Mode, MaxItems, HearbeatInterval and MaxLatency values only if 
    // Configuration Mode is Custom (EcConfigurationModeCustom)
    // If it is other configuration modes (Normal, MinLatency or MinBandwidth) they will be ignored 

    if ( sub.ConfigMode == EcConfigurationModeCustom)
    {

        //Property: Delivery Mode (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionDeliveryMode)
        vPropertyValue.Type = EcVarTypeUInt32;
        vPropertyValue.UInt32Val = sub.DeliveryMode;
        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionDeliveryMode, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }

        //Property: MaxItems (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionDeliveryMaxItems)
        vPropertyValue.Type = EcVarTypeUInt32;
        vPropertyValue.UInt32Val = sub.MaxItems;
        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionDeliveryMaxItems, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }

         //Property: HearbeatInterval (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionHeartbeatInterval)
        vPropertyValue.Type = EcVarTypeUInt32;
        vPropertyValue.UInt32Val = sub.HeartbeatInerval;
        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionHeartbeatInterval, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }
         //Property: MaxLatency ( EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionDeliveryMaxLatencyTime)
        vPropertyValue.Type = EcVarTypeUInt32;
        vPropertyValue.UInt32Val = sub.MaxLatencyTime;
        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionDeliveryMaxLatencyTime, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }

    }

    //Property: Content Format (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionContentFormat)
    vPropertyValue.Type = EcVarTypeUInt32;
    vPropertyValue.UInt32Val = sub.ContentFormat;

    if (!EcSetSubscriptionProperty(hSubscription, 
                                             EcSubscriptionContentFormat, 
                                             0, 
                                             &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }
    
    //Property: Credentials Type (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionCredentialsType)
    vPropertyValue.Type = EcVarTypeUInt32;
    vPropertyValue.UInt32Val = sub.CredentialsType;

    if (!EcSetSubscriptionProperty(hSubscription,
                                             EcSubscriptionCredentialsType, 
                                             0, 
                                             &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }   

    //Enable the Subscription
    //Property: Enable/Disable Subscription (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionEnabled)

    vPropertyValue.Type = EcVarTypeBoolean;
    vPropertyValue.BooleanVal = sub.SubscriptionStatus;

    if (!EcSetSubscriptionProperty(hSubscription, 
                                             EcSubscriptionEnabled, 
                                             0, 
                                             &vPropertyValue))
    {
            dwRetVal = GetLastError();
            goto Cleanup;
    }

    
    //Property: Common Username (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionCommonUserName)
    // This property is used across all the event sources available for this subscription
    
    if ( sub.CommonUserName.length() > 0)
    {
        vPropertyValue.Type = EcVarTypeString;
        vPropertyValue.StringVal = sub.CommonUserName.c_str();

        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionCommonUserName, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }

        //Property: Common Password (EC_SUBSCRIPTION_PROPERTY_ID::EcSubscriptionCommonPassword)
        //Set Password to empty (""/blank password) if Password is not specified.
        //NOTE: Please use Credential Manager Functions to handle Password information
        
        vPropertyValue.Type = EcVarTypeString;
        vPropertyValue.StringVal = (sub.CommonPassword.length() > 0) ? sub.CommonPassword.c_str(): L"" ;

        if (!EcSetSubscriptionProperty(hSubscription, EcSubscriptionCommonPassword, NULL, &vPropertyValue))
        {
            dwRetVal = GetLastError();
            goto Cleanup;
        }
         
        //Add the event source - so it uses the common user's credentials
        dwRetVal = AddEventSource( hSubscription, 
	                                                sub.EventSource, 
        	                                         sub.EventSourceStatus, 
               	                                  L"", 
                      	                           L"");
    }
    else
    {
        dwRetVal = AddEventSource( hSubscription, 
	                                                sub.EventSource, 
       	                                         sub.EventSourceStatus, 
              	                                  sub.EventSourceUserName, 
                     	                           sub.EventSourcePassword);
    }

    if ( ERROR_SUCCESS != dwRetVal)
    {
        goto Cleanup;
    }

    //Save the subscription with the associated properties
    // This will create the subscription and store it in the 
    // subscription repository 
    
    if( !EcSaveSubscription(hSubscription, NULL) )
    {
        dwRetVal = GetLastError();
        goto Cleanup;
    }

Cleanup:
    EcClose(hSubscription);
    return dwRetVal;

}



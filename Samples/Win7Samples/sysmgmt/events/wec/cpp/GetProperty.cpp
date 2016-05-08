// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "Subscription.h"

DWORD GetProperty(EC_HANDLE hSubscription, EC_SUBSCRIPTION_PROPERTY_ID propID, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vProperty)
{

    DWORD  dwBufferSize, dwRetVal = ERROR_SUCCESS;
    buffer.resize(sizeof(EC_VARIANT));

    if (!hSubscription)
        return ERROR_INVALID_PARAMETER;
    
    //Obtain Property Value for the specified Property 
    if( !EcGetSubscriptionProperty(  hSubscription,
                                               propID, 
                                               flags, 
                                               (DWORD) buffer.size(), 
                                               (PEC_VARIANT)&buffer[0], 
                                               &dwBufferSize) )
    {
        
        dwRetVal = GetLastError();

        if( ERROR_INSUFFICIENT_BUFFER == dwRetVal)
        {
            dwRetVal = ERROR_SUCCESS;
            buffer.resize(dwBufferSize);

            if(!EcGetSubscriptionProperty( hSubscription,
                                                     propID,
                                                     flags,
                                                     (DWORD) buffer.size(),
                                                     (PEC_VARIANT)&buffer[0],
                                                     &dwBufferSize) )
            {
                dwRetVal = GetLastError();
            }
        }
    }

    if (dwRetVal == ERROR_SUCCESS)
    {
        vProperty = (PEC_VARIANT) &buffer[0];
    }
    else
    {
        vProperty = NULL;
    }

    return dwRetVal;
}
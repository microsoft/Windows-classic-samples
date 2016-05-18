// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "Subscription.h"




DWORD GetArrayProperty(EC_OBJECT_ARRAY_PROPERTY_HANDLE hArray, EC_SUBSCRIPTION_PROPERTY_ID propID, DWORD arrayIndex, DWORD flags, std::vector<BYTE>& buffer, PEC_VARIANT& vProperty)
{
    
    DWORD dwRetVal = ERROR_SUCCESS;
    DWORD dwBufferSizeUsed;

    buffer.resize(sizeof(EC_VARIANT));
    
    if (!EcGetObjectArrayProperty( hArray, 
                                              propID,
                                              arrayIndex,
                                              flags,
                                              (DWORD) buffer.size(),
                                              (PEC_VARIANT) &buffer[0],
                                              &dwBufferSizeUsed))
    {
        dwRetVal = GetLastError();
       
        if (ERROR_INSUFFICIENT_BUFFER == dwRetVal)
        {
            buffer.resize(dwBufferSizeUsed);
            dwRetVal = ERROR_SUCCESS;
            if (!EcGetObjectArrayProperty( hArray, 
                                                      propID,
                                                      arrayIndex,
                                                      flags,
                                                      (DWORD) buffer.size(),
                                                      (PEC_VARIANT) &buffer[0],
                                                      &dwBufferSizeUsed))
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
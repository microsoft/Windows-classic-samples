// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Subscription.h"

DWORD EnumerateSubscriptions()
{


    //Enumerates Subscriptions Available on the machine

    DWORD dwBufferSizeUsed, dwError = ERROR_SUCCESS;
    BOOL bRetVal = true;

    std::vector<WCHAR> buffer(MAX_PATH);

    EC_HANDLE hEnumerator;

    hEnumerator = EcOpenSubscriptionEnum(NULL);

    if ( !hEnumerator)
        return GetLastError();

    while( bRetVal )
    {

        bRetVal = EcEnumNextSubscription( hEnumerator, 
                                                         (DWORD) buffer.size(),
                                                         (LPWSTR) &buffer[0], 
                                                         &dwBufferSizeUsed);

        dwError = GetLastError();

        if (!bRetVal && ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            dwError = ERROR_SUCCESS;
            buffer.resize(dwBufferSizeUsed);

            bRetVal = EcEnumNextSubscription( hEnumerator,
                                                            (DWORD) buffer.size(),
                                                            (LPWSTR) &buffer[0],
                                                            &dwBufferSizeUsed);
            dwError = GetLastError();
        }


        if( !bRetVal && ERROR_NO_MORE_ITEMS == dwError)
        { 
            dwError = ERROR_SUCCESS;
            break;
        }

	 if (bRetVal && ERROR_SUCCESS != dwError)
	 {
	 	break;
	 }	
	 
         wprintf(L"%s\n", (LPCWSTR) &buffer[0]);
            
    }

    EcClose(hEnumerator);
    return dwError;
}
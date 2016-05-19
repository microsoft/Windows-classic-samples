/******************************************************************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>

/*****************************************************************************/
void _cdecl main(void)
{
    INT iReturn = 0;
    HCRYPTPROV hProv = 0;
    LPSTR pszContainerName = NULL;
    DWORD cbContainerName = 0;
    HCRYPTKEY hKey;

    // Attempt to acquire a handle to the default key container.
    if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0)) {
        
        if(GetLastError() != NTE_BAD_KEYSET) {
            // Some sort of error occured.
            printf("Error opening default key container!\n");
            goto Error;
        }
        
        // Create default key container.
        if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
            
            printf("Error creating default key container!\n");
            goto Error;
        }

        // Get size of the name of the default key container name.
        if(CryptGetProvParam(hProv, PP_CONTAINER, NULL, &cbContainerName, 0)) {
            
            // Allocate buffer to receive default key container name.
            pszContainerName = malloc(cbContainerName);

            if(pszContainerName) {
                // Get name of default key container name.
                if(!CryptGetProvParam(hProv, PP_CONTAINER, (PBYTE)pszContainerName, &cbContainerName, 0)) {
                    // Error getting default key container name.
                    pszContainerName[0] = 0;
                }
            }
        }

        printf("Create key container '%s'\n", pszContainerName ? pszContainerName : "");

        // Free container name buffer (if created)
        if(pszContainerName) {
            free(pszContainerName);
        }
    }

    // Attempt to get handle to signature key.
    if(!CryptGetUserKey(hProv, AT_SIGNATURE, &hKey)) {
        
        if(GetLastError() != NTE_NO_KEY) {
            printf("Error %x during CryptGetUserKey!\n", GetLastError());
            goto Error;
        }

        // Create signature key pair.
        printf("Create signature key pair\n");

        if(!CryptGenKey(hProv, AT_SIGNATURE, 0, &hKey)) {
            printf("Error %x during CryptGenKey!\n", GetLastError());
            goto Error;
        }
    }
    
    // Close key handle
    CryptDestroyKey(hKey);

    // Attempt to get handle to exchange key.
    if(!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hKey)) {

        if(GetLastError() != NTE_NO_KEY) {
            printf("Error %x during CryptGetUserKey!\n", GetLastError());
            goto Error;
        }

        // Create key exchange key pair.
        printf("Create key exchange key pair\n");

        if(!CryptGenKey(hProv, AT_KEYEXCHANGE, 0, &hKey)) {
            printf("Error %x during CryptGenKey!\n", GetLastError());
            goto Error;
        }
    }

    // Close key handle
    CryptDestroyKey(hKey);

    printf("OK\n");

Exit:

    // Close the context (if open)
    if(hProv) {
        CryptReleaseContext(hProv, 0);
    }

    exit(iReturn);

Error:

    iReturn = 1;
    goto Exit;
}

/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Reg.c

Abstract:

    This C file includes sample code for registration portion of
    an nspv2 naming provider

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#include "Reg.h"

CRITICAL_SECTION g_cs;
RegInfo g_Registrations[MAX_REGISTRATIONS];
ULONG g_cRegistrations = 0;

// Utility routine to get the number of bytes in a string
//
ULONG StringSize(PCWSTR wz)
{
    if (wz == NULL)
        return 0;
    else
        return (ULONG) ((wcslen(wz) + 1) * sizeof(WCHAR));
}


// Utility routine which allocates memory and copies a string
//
INT CopyString(PCWSTR wzSrc, __deref_out PWSTR *pwzDest)
{
    INT err = NO_ERROR;
    ULONG cb = StringSize(wzSrc);

    *pwzDest = malloc(cb);
    if (*pwzDest == NULL)
    {
        err = WSA_NOT_ENOUGH_MEMORY;
    }
    else
    {
        memcpy(*pwzDest, wzSrc, cb);
    }

    return err;
}


// Utility routine to allocate memory and copy an array of CSADDR_INFO
//
INT CopyAddrs(__in_ecount(cAddrs) CSADDR_INFO *rgAddrsSrc, 
                                  DWORD cAddrs,
                      __deref_out CSADDR_INFO **prgAddrsDest)
{
    INT err = NO_ERROR;
    ULONG i = 0;

    CSADDR_INFO *rgAddrsDest = (CSADDR_INFO *)malloc(cAddrs * sizeof(CSADDR_INFO));

    if (rgAddrsDest == NULL)
    {
        return WSA_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(rgAddrsDest, cAddrs * sizeof(CSADDR_INFO));

    for (i = 0; i < cAddrs; i++)
    {
        // copy static members
        rgAddrsDest[i] = rgAddrsSrc[i];

        // copy embedded memory
        if (rgAddrsSrc[i].LocalAddr.lpSockaddr != NULL)
        {
            rgAddrsDest[i].LocalAddr.lpSockaddr =
                (LPSOCKADDR) malloc(rgAddrsDest[i].LocalAddr.iSockaddrLength);

            if (rgAddrsDest[i].LocalAddr.lpSockaddr == NULL)
            {
                err = WSA_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            // copy address byte-for-byte
            memcpy(rgAddrsDest[i].LocalAddr.lpSockaddr,
                   rgAddrsSrc[i].LocalAddr.lpSockaddr,
                   rgAddrsDest[i].LocalAddr.iSockaddrLength);
        }

        // copy embedded memory
        if (rgAddrsSrc[i].RemoteAddr.lpSockaddr != NULL)
        {
            rgAddrsDest[i].RemoteAddr.lpSockaddr =
                (LPSOCKADDR)malloc(rgAddrsDest[i].RemoteAddr.iSockaddrLength);

            if (rgAddrsDest[i].RemoteAddr.lpSockaddr == NULL)
            {
                err = WSA_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }

            // copy address byte-for-byte
            memcpy(rgAddrsDest[i].RemoteAddr.lpSockaddr,
                   rgAddrsSrc[i].RemoteAddr.lpSockaddr,
                   rgAddrsDest[i].RemoteAddr.iSockaddrLength);

        }
    }

    // success
    *prgAddrsDest = rgAddrsDest;
    return NO_ERROR;

    // failure
cleanup:
    for (i = 0; i < cAddrs; i++)
    {
        if (rgAddrsDest[i].LocalAddr.lpSockaddr != NULL)
            free(rgAddrsDest[i].LocalAddr.lpSockaddr);
        if (rgAddrsDest[i].RemoteAddr.lpSockaddr != NULL)
            free(rgAddrsDest[i].RemoteAddr.lpSockaddr);
    }

    free(rgAddrsDest);
    return err;
}

// Note:  Caller must take g_cs lock in order to call this safely
//
BOOL FindRegistration(PCWSTR pcwzEmailAddress, PCWSTR pcwzServiceName, __out RegInfo **ppRegInfo)
{
    ULONG i = 0;

    for (i = 0; i < g_cRegistrations; i++)
    {
        if ( (_wcsicmp(pcwzEmailAddress, g_Registrations[i].pwzEmailAddress) == 0) &&
             (_wcsicmp(pcwzServiceName,  g_Registrations[i].pwzServiceName) == 0)
           )
        {
            break;
        }
    }

    if (i == g_cRegistrations)
    {
        return FALSE;
    }
    else
    {
        *ppRegInfo = &g_Registrations[i];
        return TRUE;
    }
}

// Add a registration to the cache
//
INT AddRegistration(__in LPWSAQUERYSET2 pQuerySet)
{
    INT err = NO_ERROR;

    EnterCriticalSection(&g_cs);

    // If it already exists, we remove it
    RemoveRegistration(pQuerySet);

    if (g_cRegistrations == MAX_REGISTRATIONS)
    {
        err = WSA_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    
    err = CopyString(pQuerySet->lpszServiceInstanceName, 
                     &g_Registrations[g_cRegistrations].pwzEmailAddress);

    if (err != NO_ERROR)
    {
        goto cleanup;
    }

    err = CopyString(pQuerySet->lpszContext,
                     &g_Registrations[g_cRegistrations].pwzServiceName);
    if (err != NO_ERROR)
    {
        goto cleanup;
    }

    err = CopyAddrs(pQuerySet->lpcsaBuffer, 
                    pQuerySet->dwNumberOfCsAddrs, 
                    &g_Registrations[g_cRegistrations].prgAddresses);
            
    if (err != NO_ERROR)
    {
        goto cleanup;
    }

    printf("Registration for %S (service = %S) added.\n", 
            pQuerySet->lpszServiceInstanceName,
            pQuerySet->lpszContext);
    g_Registrations[g_cRegistrations].cAddresses = pQuerySet->dwNumberOfCsAddrs;
    g_cRegistrations++;
    
    LeaveCriticalSection(&g_cs);

    return err;

cleanup:

    free(g_Registrations[g_cRegistrations].pwzEmailAddress);
    free(g_Registrations[g_cRegistrations].pwzServiceName);

    LeaveCriticalSection(&g_cs);

    return err;
}

void RemoveAllRegistrations()
{
    ULONG i;

    EnterCriticalSection(&g_cs);

    while (g_cRegistrations > 0)
    {
        free(g_Registrations[g_cRegistrations-1].pwzEmailAddress);

        for (i = 0; i < g_Registrations[g_cRegistrations-1].cAddresses; i++)
        {
            if (g_Registrations[g_cRegistrations-1].prgAddresses[i].LocalAddr.lpSockaddr != NULL)
                free(g_Registrations[g_cRegistrations-1].prgAddresses[i].LocalAddr.lpSockaddr);
            if (g_Registrations[g_cRegistrations-1].prgAddresses[i].RemoteAddr.lpSockaddr != NULL)
                free(g_Registrations[g_cRegistrations-1].prgAddresses[i].RemoteAddr.lpSockaddr);
        }

        free(g_Registrations[g_cRegistrations-1].prgAddresses);
        g_cRegistrations--;
    }

    LeaveCriticalSection(&g_cs);

    printf("All registrations removed.\n");
}

INT RemoveRegistration(__in LPWSAQUERYSET2 pQuerySet)
{
    ULONG i, j;
    int err;

    err = WSAHOST_NOT_FOUND;

    EnterCriticalSection(&g_cs);

    for (i = 0; i < g_cRegistrations; i++)
    {
        if ( (wcscmp(pQuerySet->lpszServiceInstanceName, g_Registrations[i].pwzEmailAddress) == 0) &&
             (wcscmp(pQuerySet->lpszContext, g_Registrations[i].pwzServiceName) == 0)
           )
        {
            // found registration to remove
            free(g_Registrations[i].pwzEmailAddress);

            for (j = 0; j < g_Registrations[i].cAddresses; j++)
            {
                if (g_Registrations[i].prgAddresses[j].LocalAddr.lpSockaddr != NULL)
                    free(g_Registrations[i].prgAddresses[j].LocalAddr.lpSockaddr);
                if (g_Registrations[i].prgAddresses[j].RemoteAddr.lpSockaddr != NULL)
                    free(g_Registrations[i].prgAddresses[j].RemoteAddr.lpSockaddr);
            }

            free(g_Registrations[i].prgAddresses);

            g_Registrations[i] = g_Registrations[g_cRegistrations-1];
            g_cRegistrations--;
            printf("Registration for %S removed.\n", pQuerySet->lpszServiceInstanceName);

            err = NO_ERROR;
            break;
        }
    }

    LeaveCriticalSection(&g_cs);

    return err;
}

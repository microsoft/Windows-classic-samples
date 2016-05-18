/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SampleClient.c

Abstract:

    This C file includes sample code for registering a name with
    the nspv2 naming apis

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef celems
#define celems(x) (sizeof(x)/sizeof((x)[0]))    
#endif

#define MAX_ADDR        16

void PrintIPV6Addr(
        __in                SOCKADDR_IN6 *             pSockAddr)
{
    WORD * pwAddr = (WORD *)(&(pSockAddr->sin6_addr));
    printf("[%x:%x:%x:%x%x:%x:%x:%x]:%d\n",
        htons(pwAddr[0]),  htons(pwAddr[1]), htons(pwAddr[2]), 
        htons(pwAddr[3]),  htons(pwAddr[4]),  htons(pwAddr[5]), 
        htons(pwAddr[6]),  htons(pwAddr[7]),
        htons(pSockAddr->sin6_port));
}

//
// Publish or Delete an e-mail address via the WSASetService API
// This sample only accepts ipv6 addresses.
//
HRESULT Publish(BOOL    fPublish,
                PCWSTR  pwzEmailName, 
                PCWSTR  pwzServiceName,
                ULONG   cIPV6Addrs,
                __in_ecount(cIPV6Addrs) SOCKADDR_IN6 *pIPV6Sockaddr)
{
    HRESULT hr                                    = S_OK;
    INT iErr                                      = 0;
    WSAQUERYSETW querySet                         = {0};        
    CSADDR_INFO  rgAddrs[MAX_ADDR]                = {0};
    ULONG i                                       = 0;

    // Initialize the array of CSADDR_INFO
    //
    for(i = 0; i < cIPV6Addrs && i < celems(rgAddrs); i++)
    {
        rgAddrs[i].iProtocol                   = IPPROTO_TCP;
        rgAddrs[i].iSocketType                 = SOCK_STREAM;
        rgAddrs[i].LocalAddr.iSockaddrLength   = sizeof(*pIPV6Sockaddr);
        rgAddrs[i].LocalAddr.lpSockaddr        = (SOCKADDR*) pIPV6Sockaddr + i;
        rgAddrs[i].RemoteAddr.iSockaddrLength  = sizeof(*pIPV6Sockaddr);
        rgAddrs[i].RemoteAddr.lpSockaddr       = (SOCKADDR*) pIPV6Sockaddr + i;
    }

    querySet.dwSize                  = sizeof(WSAQUERYSETW);
    querySet.lpszServiceInstanceName = (PWSTR) pwzEmailName; 
    querySet.lpszContext             = (PWSTR) pwzServiceName;
    querySet.lpNSProviderId          = NULL;
    querySet.lpServiceClassId        = NULL;
    querySet.dwNameSpace             = NS_EMAIL;
    querySet.dwNumberOfCsAddrs       = MIN(celems(rgAddrs), cIPV6Addrs);
    querySet.lpcsaBuffer             = (LPCSADDR_INFO) rgAddrs;

    iErr = WSASetService(&querySet, fPublish ? RNRSERVICE_REGISTER : RNRSERVICE_DELETE, 0);
    if (SOCKET_ERROR == iErr)
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
    }

    return hr;
}

void Usage(PCWSTR wzProgram)
{
    printf("Usage: %S [-u] email_address\n\n", wzProgram);
    printf("\nOptions:\t-u : unpublish (default: publish)\n");
    printf("\n");
    printf("This sample will \"publish\" the {email address, ip address} pair\n");
    printf("with all naming providers registered to accept the specified domain.\n");
    printf("For use with the sample provider, use @sampleprovider.net domain\n");
}

HRESULT ParseCmdLine(
    int argc, 
    __in_ecount(argc) LPWSTR *argv, 
    __out BOOL *pfPublish, 
    ULONG cchMailAddress, 
    __out_ecount(cchMailAddress) WCHAR *wzEmailAddress,
    ULONG cchServiceName, 
    __out_ecount(cchServiceName) WCHAR *wzServiceName)
{
    int   index = 1;
    PWSTR pwzPos = argv[index];
    HRESULT hr = S_OK;

    if (argc < 2)
    {
        Usage(argv[0]);
        return E_FAIL;
    }

    *pfPublish = TRUE;

    if ('-' == *pwzPos || '/' == *pwzPos)
    {
        pwzPos++;
        if (0 == lstrcmpiW(pwzPos, L"u"))
        {
            *pfPublish = FALSE;
        }
        else
        {
            Usage(argv[0]);
            return E_FAIL;
        }

        pwzPos = argv[++index];
    }

    if (NULL == wcschr(pwzPos,'@'))
    {
        Usage(argv[0]);
        return E_FAIL;
    }
    else
    {
        hr = StringCchCopy(wzEmailAddress, cchMailAddress, pwzPos);
        pwzPos = argv[++index];
    }

    if (index < argc)
    {
        hr = StringCchCopy(wzServiceName, cchServiceName, pwzPos);
    }

    return hr;
}

//
// Get the ipv6 addresses for the local machine
//
HRESULT GetAddresses(__inout ULONG *pcAddrs, 
                     __out_ecount(*pcAddrs) SOCKADDR_IN6 *rgAddrs)
{
    HRESULT hr = S_OK;
    CHAR szHostName[MAX_PATH] = {0};
    DWORD cchHostName = celems(szHostName);
    struct addrinfo aiHints = {0};
    struct addrinfo *pResults = NULL;
    struct addrinfo *pCurrent = NULL;
    ULONG cAddrs = 0;

    if (!GetComputerNameA(szHostName, &cchHostName))
    {
        DWORD dwErr = GetLastError();            
        hr = HRESULT_FROM_WIN32(dwErr);
        fprintf(stderr, "GetComputerName failed error = %u\n", dwErr);
    }
    else
    {
        aiHints.ai_family = PF_INET6;
        if (getaddrinfo(szHostName, NULL, &aiHints, &pResults) == SOCKET_ERROR)
        {
            int iErr = WSAGetLastError();
            hr = HRESULT_FROM_WIN32(iErr);
            fprintf (stderr, "getaddrinfo(local machine name) failed error = %d\n", iErr);
        }
    }

    if (SUCCEEDED(hr))
    {
        pCurrent = pResults;
        while (pCurrent && cAddrs < *pcAddrs)
        {
            if (AF_INET6 == pCurrent->ai_family)
            {
                CopyMemory(rgAddrs + cAddrs, pCurrent->ai_addr, sizeof(SOCKADDR_IN6));
                cAddrs++;
            }

            pCurrent = pCurrent->ai_next;
        }
    }

    *pcAddrs = cAddrs;
    freeaddrinfo(pResults);
    return hr;
}

int __cdecl wmain(int argc, __in_ecount(argc) LPWSTR * argv)
{
    HRESULT hr              = S_OK;
    int iErr                = 0;
    BOOL fPublish           = TRUE;
    WORD wVersionRequested  = MAKEWORD( 2, 2 );
    WSADATA wsaData         = { 0 };
    WCHAR wzServiceName[MAX_PATH]  = {0};
    WCHAR wzEmailAddress[MAX_PATH] = {0};
    SOCKADDR_IN6 rgAddrs[MAX_ADDR] = { 0 };
    ULONG cAddrs            = celems(rgAddrs);
    ULONG i ;

    iErr = WSAStartup(wVersionRequested, &wsaData);
    if (iErr != 0)
    {
        hr = HRESULT_FROM_WIN32(iErr);
    }

    if (SUCCEEDED(hr))
    {
        hr = ParseCmdLine(argc, argv, &fPublish, celems(wzEmailAddress), wzEmailAddress,
                celems(wzServiceName), wzServiceName);
    }

    if (SUCCEEDED(hr))
    {
        hr = GetAddresses(&cAddrs, rgAddrs);

        if (SUCCEEDED(hr) && cAddrs == 0)
        {
            hr = E_UNEXPECTED;
            fprintf (stderr, "No address available to register/unregister\n");
        }
    }

    if (SUCCEEDED(hr))
    {
        printf ("----------------------------------------------------------------------\n");
        printf ("%s the following addresses:\n", fPublish ? "Publishing" : "Unpublishing");
        for (i = 0; i < cAddrs; i++)
        {
            PrintIPV6Addr(rgAddrs + i);
        }
        printf ("----------------------------------------------------------------------\n");
        printf ("\n");

        hr = Publish(fPublish, wzEmailAddress, wzServiceName, cAddrs, rgAddrs);
        
        fprintf (stderr, "%s %s - hr = %x\n", 
                 fPublish ? "Publishing" : "Unpublishing",
                 SUCCEEDED(hr) ? "SUCCEEDED" : "FAILED",
                 hr);
    }

    iErr = WSACleanup();
    if (0 != iErr)
    {
        iErr = WSAGetLastError();
        fprintf (stderr, "WSACleanup failed WSAError = 0x%x\n", iErr);
    }    

    return hr;
}



/*++
 Copyright (c) 2002 - 2002 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT. 

--*/

#include "precomp.h"
//#pragma prefast(disable:24002)
#pragma hdrstop

//
// Private functions.
//

/***************************************************************************++

Routine Description:
    Prints a record in the IP Listen store.

Arguments:
    pOutput - A pointer to HTTP_SERVICE_CONFIG_URLACL_SET

Return Value:
    None.

--***************************************************************************/
void
PrintIpListenRecords(
    IN PUCHAR pOutput
    )
{
    PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY pListenQuery;
    ULONG                                i;
    DWORD                                Status = NO_ERROR;

    pListenQuery = (PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY) pOutput;

    for(i=0; i<pListenQuery->AddrCount; i++)
    {
        DWORD        dwSockAddrLength;
        WCHAR        IpAddr[MAX_PATH];
        DWORD        dwIpAddrLen;
        PSOCKADDR_IN pSockAddrIn;

        dwIpAddrLen = sizeof(IpAddr)/sizeof(WCHAR);

        pSockAddrIn = (PSOCKADDR_IN) 
                    &pListenQuery->AddrList[i];

        // Convert address to string.
        //

        if(pSockAddrIn->sin_family == AF_INET)
        {
            dwSockAddrLength = sizeof(SOCKADDR_IN);
        }
        else if(pSockAddrIn->sin_family == AF_INET6)
        {
            dwSockAddrLength = sizeof(SOCKADDR_IN6);
        }
        else
        {
            Status = ERROR_REGISTRY_CORRUPT;
            break; 
        }

        Status = WSAAddressToString(
                            (LPSOCKADDR)&pListenQuery->AddrList[i],
                            dwSockAddrLength,
                            NULL,
                            (LPWSTR) IpAddr,
                            &dwIpAddrLen
                            );

        if(SOCKET_ERROR == Status)
        {
            Status = WSAGetLastError();
            break;
        }
        else
        {
            wprintf(L"IP         :%s.", IpAddr);

            wprintf(L"------------------------------------------------------------------------------");
        }
    }

    return;
}

/***************************************************************************++

Routine Description:
    Sets an IP Listen entry

Arguments:
    pIp - IP address to set.

Return Value:
    Success/Failure.

--***************************************************************************/
int DoIpSet(
    __in_opt PWSTR pIp
    )
{
    HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM SetParam;
    DWORD                               Status;
    SOCKADDR_STORAGE                    TempSockAddr;

    // convert IP to a SOCKADDR.
    if((Status = GetAddress(pIp, 
                            &TempSockAddr,
                            sizeof(TempSockAddr)
                            )) != NO_ERROR)
    {
        wprintf(L"%s is not a valid IP address.", pIp);
        return Status;
    }

    SetParam.AddrLength = sizeof(TempSockAddr); 
    SetParam.pAddress   = (LPSOCKADDR)&TempSockAddr;

    // Call the API
    Status = HttpSetServiceConfiguration(
                NULL,
                HttpServiceConfigIPListenList,
                (PVOID)&SetParam,
                sizeof(SetParam),
                NULL
                );

    wprintf(L"HttpSetServiceConfiguration completed with %d", Status);

              
    return Status;
}

/***************************************************************************++

Routine Description:
    Queries for a URL ACL entry.

Arguments:
    None.

Return Value:
    Success/Failure.

--***************************************************************************/
int DoIpQuery(
    )
{
    DWORD    Status;
    PUCHAR   pOutput = NULL;
    DWORD    OutputLength = 0;
    DWORD    ReturnLength = 0;

    for(;;)
    {
        // First get the size of the output buffer that is required.
        //
        Status = HttpQueryServiceConfiguration(
                    NULL,
                    HttpServiceConfigIPListenList,
                    NULL,
                    0,
                    pOutput,
                    OutputLength,
                    &ReturnLength,
                    NULL
                    );

        if(ERROR_INSUFFICIENT_BUFFER == Status)
        {
            // If the API completes with ERROR_INSUFFICIENT_BUFFER, we'll
            // allocate memory for it & continue with the loop where we'll
            // call it again.
            
            if(pOutput)
            {
                // If there was an existing buffer, free it.
                LocalFree(pOutput);
            }

            // 
            // Allocate a new buffer.
            //
            pOutput = LocalAlloc(LMEM_FIXED, ReturnLength);
            if(!pOutput)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            OutputLength = ReturnLength;
        }
        else if(NO_ERROR == Status)
        {
            PrintIpListenRecords(pOutput);
            break;
        }
        else
        {
            //
            // Some other error, so we are done
            //
            wprintf(L"HttpQueryServiceConfiguration completed with %d", Status);
            break;
        }
    }

    if(pOutput)
    {
        LocalFree(pOutput);
    }

    return Status;
}

int DoIpDelete(
    __in_opt PWSTR pIp
    )
{
    HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM SetParam;
    DWORD                               Status;
    SOCKADDR_STORAGE                    TempSockAddr;

    if((Status = GetAddress(pIp, 
                            &TempSockAddr,
                            sizeof(TempSockAddr)
                            )) != NO_ERROR)
    {
        wprintf(L"%s is not a valid IP address. ", pIp);
        return Status;
    }

    SetParam.AddrLength = sizeof(TempSockAddr); 
    SetParam.pAddress   = (LPSOCKADDR) &TempSockAddr;

    Status = HttpDeleteServiceConfiguration(
                NULL,
                HttpServiceConfigIPListenList,
                (PVOID)&SetParam,
                sizeof(SetParam),
                NULL
                );
	
    wprintf(L"HttpDeleteServiceConfiguration completed with %d", Status);       
    return Status;
}

//
// Public function.
//

int DoIpListen(
    int   argc, 
    __in_ecount(argc) WCHAR      **argv, 
    HTTPCFG_TYPE Type
    )
{
    PWSTR   pIp = NULL;
    WCHAR   **argvSaved = argv; 

    while(argc>=2 && (argv[0][0] == '-' || argv[0][0]== '/'))
    {
        switch(toupper(argv[0][1]))
        {
            case 'I':
                pIp = argv[1];
                break;
    
            default:
  	         wprintf(L"%s is not a valid command. ", argv[0]);

                return ERROR_INVALID_PARAMETER;
        }
        argc -=2;
        argv +=2;
    }

    switch(Type)
    {
        case HttpCfgTypeSet:
            return DoIpSet(pIp);

        case HttpCfgTypeQuery:
            return DoIpQuery();

        case HttpCfgTypeDelete:
            return DoIpDelete(pIp);

        default: 
	     wprintf(L"%s is not a valid command. ", argvSaved[0]);
            return ERROR_INVALID_PARAMETER;
    }
}

/*++
 Copyright (c) 2002 - 2002 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT. 

--*/

#include "precomp.h"
#pragma hdrstop

VOID
Usage()
{

        wprintf(
			L"httpcfg ACTION STORENAME [OPTIONS] \n"
			L"ACTION                             -set | query  | delete \n" 
			L"STORENAME                       -ssl | urlacl | iplisten \n"
			L"[OPTIONS]                         -See Below\n"
			L"\n"
			L"Options for ssl:\n"
			L"    -i IP-Address               - IP:port for the SSL certificate (record key)\n"
			L"\n"
			L"    -h SslHash                   - Hash of the Certificate.\n"
			L"\n"
			L"    -g GUID                       - GUID to identify the owning application.\n"
			L"\n"
			L"    -c CertStoreName           - Store name for the certificate. Defaults to\n"
			L"                                MY. Certificate must be stored in the\n"
			L"                                LOCAL_MACHINE context.\n"
			L"\n"
			L"    -m CertCheckMode           - Bit Flag\n"
			L"                                    0x00000001 - Client certificate will not be\n"
			L"                                                 verified for revocation.\n"
			L"                                    0x00000002 - Only cached client certificate\n"
			L"                                                 revocation will be used.\n"
			L"                                    0x00000004 - Enable use of the Revocation\n"
			L"                                                 freshness time setting.\n"
			L"                                    0x00010000 - No usage check.\n"
			L"\n"
			L"    -r RevocationFreshnessTime - How often to check for an updated certificate\n"
			L"                                 revocation list (CRL). If this value is 0,\n"
			L"                                 then the new CRL is updated only if the\n"
			L"                                 previous one expires. Time is specified in\n"
			L"                                 seconds.\n"
			L"\n"
			L"    -x UrlRetrievalTimeout     - Timeout on attempt to retrieve certificate\n"
			L"                                 revocation list from the remote URL.\n"
			L"                                 Timeout is specified in Milliseconds.\n"
			L"\n"
			L"    -t SslCtlIdentifier        - Restrict the certificate issuers that can be\n"
			L"                                 trusted. Can be a subset of the certificate\n"
			L"                                 issuers that are trusted by the machine.\n"
			L"\n"
			L"    -n SslCtlStoreName         - Store name under LOCAL_MACHINE where\n"
			L"                                 SslCtlIdentifier is stored.\n"
			L"\n"
			L"    -f Flags                   - Bit Field\n"
			L"                                    0x00000001 - Use DS Mapper.\n"
			L"                                    0x00000002 - Negotiate Client certificate.\n"
			L"                                    0x00000004 - Do not route to Raw ISAPI\n"
			L"                                                 filters.\n"
			L"\n"
			L"Options for urlacl:\n"
			L"    -u Url                     - Fully Qualified URL. (record key)\n"
			L"    -a ACL                     - ACL specified as a SDDL string.\n"
			L"\n"
			L"Options for iplisten:\n"
			L"    -i IPAddress               - IPv4 or IPv6 address. (for set/delete only)\n");

}

/***************************************************************************++

Routine Description:
    main routine.

Arguments:
    argc - # of command line arguments.
    argv - Arguments.

Return Value:
    Success/Failure.

--***************************************************************************/
int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    DWORD           Status = NO_ERROR;
    HTTPCFG_TYPE    Type;
    HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;
    WORD            wVersionRequested;
    WSADATA         wsaData;

    // Parse command line parameters.

    if(argc < 3)
    {
	 Usage();
        return 0;
    }

    argv++; argc --;

    //
    // First parse the type of operation.
    //
   
    if(_wcsicmp(argv[0], L"set") == 0)
    {
        Type = HttpCfgTypeSet;
    }
    else if(_wcsicmp(argv[0], L"query") == 0)
    {
        Type = HttpCfgTypeQuery;
    }
    else if(_wcsicmp(argv[0], L"delete") == 0)
    {
        Type = HttpCfgTypeDelete;
    }
    else if(_wcsicmp(argv[0], L"?") == 0)
    {
	 Usage();
        return 0;
    }
    else
    {
	 wprintf(L"%s is an invalid command", argv[0]);
        return ERROR_INVALID_PARAMETER;
    }
    argv++; argc--;

    //
    // Call HttpInitialize.
    //

    if((Status = HttpInitialize(
                    HttpApiVersion, 
                    HTTP_INITIALIZE_CONFIG, 
                    NULL)) != NO_ERROR)
    {
    	 wprintf(L"HttpInitialize failed with %d", Status);
        return Status;
    }

    //
    // Call WSAStartup as we are using some winsock functions.
    //
    wVersionRequested = MAKEWORD( 2, 2 );

    if(WSAStartup( wVersionRequested, &wsaData ) != 0)
    {
        HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);
        return GetLastError();
    }

    //
    // Call the corresponding API.
    //

    if(_wcsicmp(argv[0], L"ssl") == 0)
    {
        argv++; argc--;
        Status = DoSsl(argc, argv, Type);
    }
    else if(_wcsicmp(argv[0], L"urlacl") == 0)
    {
        argv++; argc--;
        Status = DoUrlAcl(argc, argv, Type);
    }
    else if(_wcsicmp(argv[0], L"iplisten") == 0)
    {
        argv++; argc--;
        Status = DoIpListen(argc, argv, Type);
    }
    else 
    {
	 wprintf(L"%s is an invalid command", argv[0]);
        Status = ERROR_INVALID_PARAMETER;
    }

    WSACleanup();
    HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);

    return Status;
}


/***************************************************************************++

Routine Description:
    Given a WCHAR IP, this routine converts it to a SOCKADDR. 

Arguments:
    pIp     - IP address to covert.
    pBuffer - Buffer, must be == sizeof(SOCKADDR_STORAGE)
    Length  - Length of buffer

Return Value:
    Success/Failure.

--***************************************************************************/
DWORD
GetAddress(
    __in_opt PWSTR  pIp, 
    PVOID   pBuffer,
    ULONG   Length
    )
{
    DWORD Status;
    DWORD TempStatus;

    if(pIp == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // The address could be a v4 or a v6 address. First, let's try v4.
    //

    Status = WSAStringToAddress(
                pIp,
                AF_INET,
                NULL,
                pBuffer,
                (LPINT)&Length
                );

    if(Status != NO_ERROR)
    {
        //
        // Now, try v6
        //

        Status = WSAGetLastError();

        TempStatus = WSAStringToAddress(
                        pIp,
                        AF_INET6,
                        NULL,
                        pBuffer,
                        (LPINT)&Length
                        );

        //
        // If IPv6 also fails, then we want to return the original 
        // error.
        //
        // If it succeeds, we want to return NO_ERROR.
        //

        if(TempStatus == NO_ERROR)
        {
            Status = NO_ERROR;
        }
    }

    return Status;
}

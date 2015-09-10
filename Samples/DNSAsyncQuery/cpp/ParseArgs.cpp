//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Abstract:
//
//  This file abstracts parsing input arguments for DnsQuery Sample
//
//     DnsQuery -q <QueryName> [-t QueryType] [-o QueryOptions] [-s server]
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<Strsafe.h>
#include <windns.h>

VOID
PrintHelp()
{
    wprintf(L"Usage: DnsQuery -q <QueryName> [-t QueryType] [-s DnsServerIP] [-o QueryOptions]\n");
    wprintf(L"<QueryName>\t\tInput query Name\n");
    wprintf(L"<QueryType>\t\tInput query type: A, PTR, NS, AAAA, TXT....\n");
    wprintf(L"<DnsServerIP>\t\tDNS Server IP address\n");
    wprintf(L"<QueryOptions>\t\tInput query flags (use combination of following numerics or one of the below option string):\n");
    wprintf(L"\t\t\t\t0x0      DNS_QUERY_STANDARD\n");
    wprintf(L"\t\t\t\t0x1      DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE\n");
    wprintf(L"\t\t\t\t0x2      DNS_QUERY_USE_TCP_ONLY\n");
    wprintf(L"\t\t\t\t0x4      DNS_QUERY_NO_RECURSION\n");
    wprintf(L"\t\t\t\t0x8      DNS_QUERY_BYPASS_CACHE\n");
    wprintf(L"\t\t\t\t0x10     DNS_QUERY_NO_WIRE_QUERY\n");
    wprintf(L"\t\t\t\t0x20     DNS_QUERY_NO_LOCAL_NAME\n");
    wprintf(L"\t\t\t\t0x40     DNS_QUERY_NO_HOSTS_FILE\n");
    wprintf(L"\t\t\t\t0x100    DNS_QUERY_WIRE_ONLY\n");
    wprintf(L"\t\t\t\t0x400    DNS_QUERY_MULTICAST_ONLY\n");
    wprintf(L"\t\t\t\t0x800    DNS_QUERY_NO_MULTICAST\n");
    wprintf(L"\t\t\t\t0x1000   DNS_QUERY_TREAT_AS_FQDN\n");
    wprintf(L"\t\t\t\t0x200000 DNS_QUERY_DISABLE_IDN_ENCODING\n");
    wprintf(L"\n");
    return;
}

typedef struct _USER_FRIENDLY_QUERY_TYPE
{
    PWSTR String;
    WORD Value;
}USER_FRIENDLY_QUERY_TYPE;

USER_FRIENDLY_QUERY_TYPE TypeTable[] =
{
    L"ZERO"      , 0                 ,
    L"A"         , DNS_TYPE_A        ,
    L"NS"        , DNS_TYPE_NS       ,
    L"MD"        , DNS_TYPE_MD       ,
    L"MF"        , DNS_TYPE_MF       ,
    L"CNAME"     , DNS_TYPE_CNAME    ,
    L"SOA"       , DNS_TYPE_SOA      ,
    L"MB"        , DNS_TYPE_MB       ,
    L"MG"        , DNS_TYPE_MG       ,
    L"MR"        , DNS_TYPE_MR       ,
    L"NULL"      , DNS_TYPE_NULL     ,
    L"WKS"       , DNS_TYPE_WKS      ,
    L"PTR"       , DNS_TYPE_PTR      ,
    L"HINFO"     , DNS_TYPE_HINFO    ,
    L"MINFO"     , DNS_TYPE_MINFO    ,
    L"MX"        , DNS_TYPE_MX       ,
    L"TXT"       , DNS_TYPE_TEXT     ,
    L"RP"        , DNS_TYPE_RP       ,
    L"AFSDB"     , DNS_TYPE_AFSDB    ,
    L"X25"       , DNS_TYPE_X25      ,
    L"ISDN"      , DNS_TYPE_ISDN     ,
    L"RT"        , DNS_TYPE_RT       ,
    L"NSAP"      , DNS_TYPE_NSAP     ,
    L"NSAPPTR"   , DNS_TYPE_NSAPPTR  ,
    L"SIG"       , DNS_TYPE_SIG      ,
    L"KEY"       , DNS_TYPE_KEY      ,
    L"PX"        , DNS_TYPE_PX       ,
    L"GPOS"      , DNS_TYPE_GPOS     ,
    L"AAAA"      , DNS_TYPE_AAAA     ,
    L"LOC"       , DNS_TYPE_LOC      ,
    L"NXT"       , DNS_TYPE_NXT      ,
    L"EID"       , DNS_TYPE_EID      ,
    L"NIMLOC"    , DNS_TYPE_NIMLOC   ,
    L"SRV"       , DNS_TYPE_SRV      ,
    L"ATMA"      , DNS_TYPE_ATMA     ,
    L"NAPTR"     , DNS_TYPE_NAPTR    ,
    L"KX"        , DNS_TYPE_KX       ,
    L"CERT"      , DNS_TYPE_CERT     ,
    L"A6"        , DNS_TYPE_A6       ,
    L"DNAME"     , DNS_TYPE_DNAME    ,
    L"SINK"      , DNS_TYPE_SINK     ,
    L"OPT"       , DNS_TYPE_OPT      ,
    L"DS"        , DNS_TYPE_DS       ,
    L"RRSIG"     , DNS_TYPE_RRSIG    ,
    L"NSEC"      , DNS_TYPE_NSEC     ,
    L"DNSKEY"    , DNS_TYPE_DNSKEY   ,
    L"DHCID"     , DNS_TYPE_DHCID    ,
    L"NSEC3"     , DNS_TYPE_NSEC3    ,
    L"NSEC3PARAM", DNS_TYPE_NSEC3PARAM ,
    L"ALL"       , DNS_TYPE_ALL      ,
     NULL,   0,
};

WORD
ExtractUserFriendlyQueryType(
    _In_ PWSTR QueryType
    )
{
    INT Index = 0;
    WORD ReturnValue = 0;
    PWSTR EndPtr = NULL;

    while(TypeTable[Index].String != NULL)
    {
        if (_wcsicmp(QueryType, TypeTable[Index].String) == 0)
        {
            ReturnValue = TypeTable[Index].Value;
            goto exit;
        }
        Index++;
    }

    ReturnValue = (WORD)wcstoul(QueryType, &EndPtr, 0);
   
exit:
    return ReturnValue;
}

typedef struct _USER_FRIENDLY_QUERY_OPTIONS
{
    PWSTR String;
    ULONG Value;
}USER_FRIENDLY_QUERY_OPTIONS;

USER_FRIENDLY_QUERY_OPTIONS TableOption[] =
{
    L"DNS_QUERY_STANDARD",                          DNS_QUERY_STANDARD,
    L"DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE",         DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE,
    L"DNS_QUERY_USE_TCP_ONLY",                      DNS_QUERY_USE_TCP_ONLY,
    L"DNS_QUERY_NO_RECURSION",                      DNS_QUERY_NO_RECURSION,
    L"DNS_QUERY_BYPASS_CACHE",                      DNS_QUERY_BYPASS_CACHE,
    L"DNS_QUERY_NO_WIRE_QUERY",                     DNS_QUERY_NO_WIRE_QUERY,
    L"DNS_QUERY_NO_LOCAL_NAME",                     DNS_QUERY_NO_LOCAL_NAME,
    L"DNS_QUERY_NO_HOSTS_FILE",                     DNS_QUERY_NO_HOSTS_FILE,
    L"DNS_QUERY_WIRE_ONLY",                         DNS_QUERY_WIRE_ONLY,
    L"DNS_QUERY_MULTICAST_ONLY",                    DNS_QUERY_MULTICAST_ONLY,
    L"DNS_QUERY_NO_MULTICAST",                      DNS_QUERY_NO_MULTICAST,
    L"DNS_QUERY_TREAT_AS_FQDN",                     DNS_QUERY_TREAT_AS_FQDN,
    L"DNS_QUERY_DISABLE_IDN_ENCODING",              DNS_QUERY_DISABLE_IDN_ENCODING,
    NULL,   0,
};

ULONG
ExtractUserFriendlyQueryOptions(
    _In_ PWSTR QueryOptions
    )
{
    INT Index = 0;
    ULONG ReturnValue = 0;
    PWSTR EndPtr = NULL;

    while(TableOption[Index].String != NULL)
    {
        if (_wcsicmp(QueryOptions, TableOption[Index].String) == 0)
        {
            ReturnValue = TableOption[Index].Value;
            goto exit;
        }
        Index++;
    }

    ReturnValue = (ULONG)wcstoul(QueryOptions, &EndPtr, 0);

exit:
    return ReturnValue;
}

DWORD
ParseArguments(
    _In_ int Argc,
    _In_reads_(Argc) PWCHAR Argv[],
    _Out_writes_(DNS_MAX_NAME_BUFFER_LENGTH) PWSTR QueryName,
    _Out_ WORD *QueryType,
    _Out_ ULONG *QueryOptions,
    _Out_writes_(MAX_PATH) PWSTR ServerIp
    )
{
    int CurrentIndex = 1;
    BOOL ArgContainsQueryName = FALSE;
    BOOL Error = ERROR_INVALID_PARAMETER;

    *QueryType = DNS_TYPE_A; // default type
    *QueryOptions = 0; // default query options
    *QueryName = L'\0';
    *ServerIp = L'\0';

    while (CurrentIndex < Argc)
    {
        //
        //  Query Name:
        //

        #pragma prefast(suppress:6385, "CurrentIndex is guaranteed less than Argc")
        if (_wcsicmp(Argv[CurrentIndex], L"-q") == 0)
        {
            CurrentIndex++;
            if (CurrentIndex < Argc)
            {
                StringCchCopy(QueryName, DNS_MAX_NAME_BUFFER_LENGTH, Argv[CurrentIndex]);
                ArgContainsQueryName = TRUE;
            }
            else
            {
                goto exit;
            }
        }

        //
        //  Query Type
        //

        else if (_wcsicmp(Argv[CurrentIndex], L"-t") == 0)
        {

            CurrentIndex++;
            if (CurrentIndex < Argc)
            {
                *QueryType = ExtractUserFriendlyQueryType(Argv[CurrentIndex]);
                if (*QueryType == 0)
                {
                    goto exit;
                }
            }
            else
            {
                goto exit;
            }
        }

        //
        //  Query Options
        //

        else if (_wcsicmp(Argv[CurrentIndex], L"-o") == 0)
        {

            CurrentIndex++;
            if (CurrentIndex < Argc)
            {
                *QueryOptions = ExtractUserFriendlyQueryOptions(Argv[CurrentIndex]);
            }
            else
            {
                goto exit;
            }
        }

        //
        // Server List
        //

        else if (_wcsicmp(Argv[CurrentIndex], L"-s") == 0)
        {
            CurrentIndex++;
            if (CurrentIndex < Argc)
            {
                StringCchCopy(ServerIp, MAX_PATH, Argv[CurrentIndex]);
            }
            else
            {
                goto exit;
            }
        }

        #pragma prefast(suppress:6385, "CurrentIndex is guaranteed less than Argc")
        else
        {
            goto exit;
        }
        CurrentIndex++;
    }

    if (ArgContainsQueryName)
    {
        Error = ERROR_SUCCESS;
    }

exit:
    if (Error != ERROR_SUCCESS)
    {
        PrintHelp();
    }

    return Error;

}


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
//  This file abstracts printing various types of DNS records.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <windns.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>

#define MAX_ADDRESS_STRING_LENGTH   64

VOID
ARecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    struct in_addr Ipv4address;
    WCHAR Ipv4String[MAX_ADDRESS_STRING_LENGTH] = L"\0"; 

    Ipv4address.S_un.S_addr = DnsRecord->Data.A.IpAddress;
    RtlIpv4AddressToString(&Ipv4address, Ipv4String);

    wprintf(L"\tIP address     = %s\n",Ipv4String);
}

VOID
PtrRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tHostName       = %s\n", DnsRecord->Data.PTR.pNameHost);
}

VOID
SoaRecordPrint(
    PDNS_RECORD DnsRecord
    )
{

    wprintf(L"n\tPrimary        = %s\n"
            L"\tAdmin          = %s\n"
            L"\tSerial         = %d\n"
            L"\tRefresh        = %d\n"
            L"\tRetry          = %d\n"
            L"\tExpire         = %d\n"
            L"\tDefault TTL    = %d\n",
            DnsRecord->Data.SOA.pNamePrimaryServer,
            DnsRecord->Data.SOA.pNameAdministrator,
            DnsRecord->Data.SOA.dwSerialNo,
            DnsRecord->Data.SOA.dwRefresh,
            DnsRecord->Data.SOA.dwRetry,
            DnsRecord->Data.SOA.dwExpire,
            DnsRecord->Data.SOA.dwDefaultTtl);

}
VOID
TxtRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    INT Index = 0;
    INT Count = 0;
    LPTSTR *StringArray;
    

    Count = DnsRecord->Data.TXT.dwStringCount;
    StringArray = DnsRecord->Data.TXT.pStringArray;

    for( Index = 1; Index <= Count; Index++ )
    {
        wprintf(L"\tString[%d]      = %s\n", Index,*StringArray);
        StringArray++;
    }

}
VOID
MxRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tPreference     = %d\n"
            L"\tExchange       = %s\n",
            DnsRecord->Data.MX.wPreference,
            DnsRecord->Data.MX.pNameExchange);
}

VOID
AaaaRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    IN6_ADDR Ipv6address;
    WCHAR Ipv6String[MAX_ADDRESS_STRING_LENGTH] = L"\0"; 

    CopyMemory(&Ipv6address, 
               &DnsRecord->Data.AAAA.Ip6Address, 
               sizeof(Ipv6address));

    RtlIpv6AddressToString(&Ipv6address, Ipv6String);
    wprintf(L"\tIP address     = %s\n",Ipv6String);
}

VOID
SrvRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tPriority       = %d\n"
            L"\tWeight         = %d\n"
            L"\tPort           = %d\n"
            L"\tTarget Host    = %s\n",
            DnsRecord->Data.SRV.wPriority,
            DnsRecord->Data.SRV.wWeight,
            DnsRecord->Data.SRV.wPort,
            DnsRecord->Data.SRV.pNameTarget);
}

VOID
SigRecordPrint(
    PDNS_RECORD DnsRecord
    )
{

    wprintf(L"\tType Covered         = %d\n"
            L"\tAlgorithm            = %d\n"
            L"\tLabels               = %d\n"
            L"\tOriginal TTL         = %d\n"
            L"\tSignature Expiration = %d\n"
            L"\tSignature Inception  = %d\n"
            L"\tKey Tag              = %d\n"
            L"\tSigner's Name        = %s\n",
            DnsRecord->Data.SIG.wTypeCovered,
            DnsRecord->Data.SIG.chAlgorithm,
            DnsRecord->Data.SIG.chLabelCount,
            DnsRecord->Data.SIG.dwOriginalTtl,
            DnsRecord->Data.SIG.dwExpiration,
            DnsRecord->Data.SIG.dwTimeSigned,
            DnsRecord->Data.SIG.wKeyTag,
            DnsRecord->Data.SIG.pNameSigner);
}


VOID
KeyRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tFlags                = 0x%04x\n"
            L"\tProtocol             = %d\n"
            L"\tAlgorithm            = %d\n",
            DnsRecord->Data.KEY.wFlags,
            DnsRecord->Data.KEY.chProtocol,        
            DnsRecord->Data.KEY.chAlgorithm);

}

VOID
DsRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tKey Tag              = %d\n"
            L"\tAlgorithm            = %d\n"
            L"\tDigest Type          = %d\n",
            DnsRecord->Data.DS.wKeyTag,
            DnsRecord->Data.DS.chAlgorithm,
            DnsRecord->Data.DS.chDigestType);
}

VOID
NsecRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tNext Domain Name     = %s\n",
            DnsRecord->Data.NSEC.pNextDomainName);

}

VOID
Nsec3RecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tHashAlgorithm                   = %d\n"
            L"\tFlags                           = 0x%02x\n"
            L"\tIterations                      = %d\n",
            DnsRecord->Data.NSEC3.chAlgorithm,
            DnsRecord->Data.NSEC3.bFlags,
            DnsRecord->Data.NSEC3.wIterations);
}

VOID
Nsec3ParamRecordPrint(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"\tHashAlgorithm                   = %d\n"
            L"\tFlags                           = 0x%02x\n"
            L"\tIterations                      = %d\n",
            DnsRecord->Data.NSEC3PARAM.chAlgorithm,
            DnsRecord->Data.NSEC3PARAM.bFlags,
            DnsRecord->Data.NSEC3PARAM.wIterations);
}

typedef VOID (* PRINT_DNS_RECORD_DATA_FUNCTION)(
            PDNS_RECORD DnsRecord
    );

//
//  This sample prints record data for most commonly used DNS TYPES.
//

PRINT_DNS_RECORD_DATA_FUNCTION   PrintDataTable[] =
{
    NULL,               //  ZERO
    ARecordPrint,       //  A
    PtrRecordPrint,     //  NS
    PtrRecordPrint,     //  MD
    PtrRecordPrint,     //  MF
    PtrRecordPrint,     //  CNAME
    SoaRecordPrint,     //  SOA
    PtrRecordPrint,     //  MB
    PtrRecordPrint,     //  MG
    PtrRecordPrint,     //  MR
    NULL,               //  NULL
    NULL,               //  WKS
    PtrRecordPrint,     //  PTR
    TxtRecordPrint,     //  HINFO
    NULL,               //  MINFO
    MxRecordPrint,      //  MX
    TxtRecordPrint,     //  TXT
    NULL,               //  RP
    MxRecordPrint,      //  AFSDB
    TxtRecordPrint,     //  X25
    TxtRecordPrint,     //  ISDN
    MxRecordPrint,      //  RT
    NULL,               //  NSAP
    NULL,               //  NSAPPTR
    SigRecordPrint,     //  SIG
    KeyRecordPrint,     //  KEY
    NULL,               //  PX
    NULL,               //  GPOS
    AaaaRecordPrint,    //  AAAA
    NULL,               //  LOC
    NULL,               //  NXT
    NULL,               //  EID   
    NULL,               //  NIMLOC
    SrvRecordPrint,     //  SRV   
    NULL,               //  ATMA  
    NULL,               //  NAPTR 
    NULL,               //  KX    
    NULL,               //  CERT  
    NULL,               //  A6    
    NULL,               //  DNAME 
    NULL,               //  SINK  
    NULL,               //  OPT   
    NULL,               //  42
    DsRecordPrint,      //  DS
    NULL,               //  44
    NULL,               //  45
    SigRecordPrint,     //  RRSIG
    NsecRecordPrint,    //  NSEC
    KeyRecordPrint,     //  DNSKEY
    NULL,               //  DHCID    
    Nsec3RecordPrint,   //  NSEC3
    Nsec3ParamRecordPrint  //  NSEC3PARAM
};

VOID
PrintRecord(
    PDNS_RECORD DnsRecord
    )
{
    wprintf(L"  Record:\n"
            L"\tPtr            = %p, pNext = %p\n"
            L"\tOwner          = %s\n"
            L"\tType           = %d\n"
            L"\tFlags          = %08x\n"
            L"\t\tSection      = %d\n"
            L"\t\tDelete       = %d\n"
            L"\t\tCharSet      = %d\n"
            L"\tTTL            = %d\n"
            L"\tReserved       = %d\n"
            L"\tDataLength     = %d\n",
            DnsRecord,
            DnsRecord->pNext,
            DnsRecord->pName,
            DnsRecord->wType,
            DnsRecord->Flags.DW,
            DnsRecord->Flags.S.Section,
            DnsRecord->Flags.S.Delete,
            DnsRecord->Flags.S.CharSet,
            DnsRecord->dwTtl,
            DnsRecord->dwReserved,
            DnsRecord->wDataLength );

    if (DnsRecord->wType < ARRAYSIZE(PrintDataTable) &&
        PrintDataTable[DnsRecord->wType] != NULL)
    {
        PrintDataTable[DnsRecord->wType](DnsRecord);
    }
}

VOID
PrintDnsRecordList(  
    PDNS_RECORD DnsRecord
    )
{
    while(DnsRecord)
    {
        PrintRecord(DnsRecord);
        DnsRecord = DnsRecord->pNext;
    }
}


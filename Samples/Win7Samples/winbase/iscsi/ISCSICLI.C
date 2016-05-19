//***************************************************************************
//
//  iscsicli.c
// 
//  Module: iscsi discovery command line tool
//
//  Purpose: Contains iscsi discovery command line tool
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved
//***************************************************************************

#define WIN32_LEAN_AND_MEAN
#include <initguid.h>
#include <windows.h>
#include <shellapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>

#include "iscsierr.h"
#include "iscsidsc.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <setupapi.h>

#include <strsafe.h>

//
// This struct carries around information about a volume including the
// disk extents that comprise the volume and the volume path names
//
typedef struct
{
    union
    {
        VOLUME_DISK_EXTENTS VolumeDiskExtents;
        UCHAR VolumeDiskExtentsBuffer[sizeof(VOLUME_DISK_EXTENTS) + 1024 * sizeof(DISK_EXTENT)];
    };
    TCHAR VolumePathNames[1];
    
} VOLUMEMOREINFO, *PVOLUMEMOREINFO;


typedef struct
{
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    SP_DEVINFO_DATA DeviceInfoData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;
    PVOID MoreInfo;
} DEVICEINTERFACEENTRY, *PDEVICEINTERFACEENTRY;


typedef ISDSC_STATUS (*ENUMDEVICEINTERFACECALLBACK)(
    IN PVOID Context,
    IN LPGUID Guid,
    IN HDEVINFO DevInfo,
    IN OUT PDEVICEINTERFACEENTRY DevEntry
    );


#define OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)(Base) + (Offset)))


#define Alloc(size)   malloc(size)
#define Free(p)   free(p)

void Usage(ULONG Code)
{
    if (Code == 0)
    {
#ifdef UNICODE
        printf("iscsicli\n");
        printf("\n");
#endif
    }

    if ((Code == 0) || (Code == 1))
    {
        printf("iscsicli AddTarget <TargetName> <TargetAlias> <TargetPortalAddress>\n");
        printf("                   <TargetPortalSocket> <Target flags>\n");
        printf("                   <Persist> <Login Flags> <Header Digest> <Data Digest> \n");
        printf("                   <Max Connections> <DefaultTime2Wait>\n");
        printf("                   <DefaultTime2Retain> <Username> <Password> <AuthType>\n");
        printf("                   <Mapping Count> <Target Lun> <OS Bus> <Os Target> \n");
        printf("                   <OS Lun> ...\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 2))
    {
        printf("iscsicli RemoveTarget <TargetName> \n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 3))
    {
        printf("iscsicli AddTargetPortal <TargetPortalAddress> <TargetPortalSocket> \n");
        printf("                         [HBA Name] [Port Number]\n");
        printf("                         <Security Flags>\n");
        printf("                         <Login Flags> <Header Digest> <Data Digest> \n");
        printf("                         <Max Connections> <DefaultTime2Wait>\n");
        printf("                         <DefaultTime2Retain> <Username> <Password> <AuthType>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 4))
    {
        printf("iscsicli RemoveTargetPortal <TargetPortalAddress> <TargetPortalSocket> [HBA Name] [Port Number]\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 5))
    {
        printf("iscsicli RefreshTargetPortal <TargetPortalAddress> <TargetPortalSocket> [HBA Name] [Port Number]\n");
        printf("\n");
    }

    if ((Code == 0) || (Code == 6))
    {
        printf("iscsicli ListTargets [ForceUpdate]\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 7))
    {
        printf("iscsicli ListTargetPortals\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 8))
    {
        printf("iscsicli TargetInfo <TargetName> [Discovery Mechanism]\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 9))
    {
        printf("iscsicli LoginTarget <TargetName> <ReportToPNP>\n");    
        printf("                     <TargetPortalAddress> <TargetPortalSocket>\n");    
        printf("                     <InitiatorInstance> <Port number> <Security Flags>\n");    
        printf("                     <Login Flags> <Header Digest> <Data Digest> \n");    
        printf("                     <Max Connections> <DefaultTime2Wait>\n");  
        printf("                     <DefaultTime2Retain> <Username> <Password> <AuthType> <Key>\n");  
        printf("                     <Mapping Count> <Target Lun> <OS Bus> <Os Target> \n");    
        printf("                     <OS Lun> ...\n");  
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 10))
    {
        printf("iscsicli LogoutTarget <SessionId>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 11))
    {
        printf("iscsicli PersistentLoginTarget <TargetName> <ReportToPNP>\n");    
        printf("                     <TargetPortalAddress> <TargetPortalSocket>\n");    
        printf("                     <InitiatorInstance> <Port number> <Security Flags>\n");    
        printf("                     <Login Flags> <Header Digest> <Data Digest> \n");    
        printf("                     <Max Connections> <DefaultTime2Wait>\n");  
        printf("                     <DefaultTime2Retain> <Username> <Password> <AuthType> <Key>\n");  
        printf("                     <Mapping Count> <Target Lun> <OS Bus> <Os Target> \n");    
        printf("                     <OS Lun> ...\n");  
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 12))
    {
        printf("iscsicli ListPersistentTargets\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 13))
    {
        printf("iscsicli RemovePersistentTarget <Initiator Name> <TargetName> \n");
        printf("                                <Port Number> \n");
        printf("                                <Target Portal Address> \n");
        printf("                                <Target Portal Socket> \n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 14))
    {
        printf("iscsicli AddConnection <SessionId> <Initiator Instance>\n");
        printf("                       <Port Number> <Target Portal Address>\n");   
        printf("                       <Target Portal Socket> <Security Flags>\n"); 
        printf("                     <Login Flags> <Header Digest> <Data Digest> \n");    
        printf("                     <Max Connections> <DefaultTime2Wait>\n");  
        printf("                     <DefaultTime2Retain> <Username> <Password> <AuthType> <Key>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 15))
    {
        printf("iscsicli RemoveConnection <SessionId> <ConnectionId> \n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 16))
    {
        printf("iscsicli ScsiInquiry <SessionId> <LUN> <EvpdCmddt> <PageCode>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 17))
    {
        printf("iscsicli ReadCapacity <SessionId> <LUN>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 18))
    {
        printf("iscsicli ReportLUNs <SessionId>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 19))
    {
        printf("iscsicli ReportTargetMappings\n");  
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 20))
    {
        printf("iscsicli ListInitiators\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 21))
    {
        printf("iscsicli AddiSNSServer <iSNS Server Address>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 22))
    {
        printf("iscsicli RemoveiSNSServer <iSNS Server Address>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 23))
    {
        printf("iscsicli RefreshiSNSServer <iSNS Server Address>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 24))
    {
        printf("iscsicli ListiSNSServers\n");       
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 25))
    {
        printf("iscsicli NodeName <node name>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 26))
    {
        printf("iscsicli SessionList <Show Session Info>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 27))
    {
        printf("iscsicli CHAPSecret <chap secret>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 28))
    {
        printf("iscsicli TunnelAddr <Initiator Name> <InitiatorPort> <Destination Address> <Tunnel Address> <Persist>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 29))
    {
        printf("iscsicli GroupKey <Key> <Persist>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 31))
    {
        printf("iscsicli BindPersistentVolumes\n\n");
        printf("iscsicli BindPersistentDevices\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 37))
    {
        printf("iscsicli ReportPersistentDevices\n");
        printf("\n");
    }
        
    if ((Code == 0) || (Code == 38))
    {
        printf("iscsicli AddPersistentDevice <Volume or Device Path>\n");
        printf("\n");
    }
        
    if ((Code == 0) || (Code == 39))
    {
        printf("iscsicli RemovePersistentDevice <Volume or Device Path>\n");
        printf("\n");
    }
        
    if ((Code == 0) || (Code == 40))
    {
        printf("iscsicli ClearPersistentDevices\n");
        printf("\n");
    }


    if ((Code == 0) || (Code == 42))
    {
        printf("iscsicli GetPSKey <Initiator Name> <initiator Port> <Id Type> <Id>\n");
        printf("\n");
    }
    
    if ((Code == 0) || (Code == 30))
    {
        printf("iscsicli PSKey <Initiator Name> <initiator Port> <Security Flags> <Id Type> <Id> <Key> <persist>\n");
        printf("\n");
    }


    if (Code == 0)
    {
        printf("Quick Commands\n\n");
    }

    if ((Code == 0) || (Code == 33))
    {
        printf("iscsicli QLoginTarget <TargetName>  [CHAP Username] [CHAP Password]\n");
        printf("\n");
    }

    if ((Code == 0) || (Code == 34))
    {
        printf("iscsicli QAddTarget <TargetName> <TargetPortalAddress>\n");
        printf("\n");
    }


    if ((Code == 0) || (Code == 35))
    {
        printf("iscsicli QAddTargetPortal <TargetPortalAddress>\n");
        printf("                          [CHAP Username] [CHAP Password]\n");
        printf("\n");
    }

    if ((Code == 0) || (Code == 36))
    {
        printf("iscsicli QAddConnection <SessionId> <Initiator Instance>\n");
        printf("                        <Target Portal Address>\n");
        printf("                        [CHAP Username] [CHAP Password]\n");
        printf("\n");
    }

    if ((Code == 0) ||
        (Code == 1) ||
        (Code == 9) ||
        (Code == 11))
    {
        printf("Target Mappings:\n");
        printf("    <Target Lun> is the LUN value the target uses to expose the LUN.\n");
        printf("                 It must be in the form 0x0123456789abcdef\n");
        printf("    <OS Bus> is the bus number the OS should use to surface the LUN\n");
        printf("    <OS Target> is the target number the OS should use to surface the LUN\n");
        printf("    <OS LUN> is the LUN number the OS should use to surface the LUN\n");
        printf("\n");
    }

    if ((Code == 0) ||
        (Code == 30) ||
        (Code == 42))
    {
        printf("Payload Id Type:\n");
        printf("    ID_IPV4_ADDR is      1 - Id format is 1.2.3.4\n");
        printf("    ID_FQDN is           2 - Id format is ComputerName\n");
        printf("    ID_IPV6_ADDR is      5 - Id form is IPv6 Address\n");
        printf("\n");
    }
        
    if ((Code == 0) ||
        (Code == 3) ||
        (Code == 9) ||
        (Code == 11) ||
        (Code == 14) ||
        (Code == 30))
    {
        printf("Security Flags:\n");
        printf("    TunnelMode is          0x00000040\n");
        printf("    TransportMode is       0x00000020\n");
        printf("    PFS Enabled is         0x00000010\n");
        printf("    Aggressive Mode is     0x00000008\n");
        printf("    Main mode is           0x00000004\n");
        printf("    IPSEC/IKE Enabled is   0x00000002\n");
        printf("    Valid Flags is         0x00000001\n");
        printf("\n");
    }
    
    if ((Code == 0) ||
        (Code == 1) ||
        (Code == 3) ||
        (Code == 9) ||
        (Code == 11) ||
        (Code == 14))
    {
        printf("Login Flags:\n");
        printf("    ISCSI_LOGIN_FLAG_REQUIRE_IPSEC                0x00000001\n");
        printf("        IPsec is required for the operation\n\n");
        printf("    ISCSI_LOGIN_FLAG_MULTIPATH_ENABLED            0x00000002\n");
        printf("        Multipathing is enabled for the target on this initiator\n");
        printf("\n");
    }
    
    if ((Code == 0) ||
        (Code == 1) ||
        (Code == 3) ||
        (Code == 9) ||
        (Code == 11) ||
        (Code == 14))
    {
        printf("AuthType:\n");
        printf("    ISCSI_NO_AUTH_TYPE = 0,\n");
        printf("        No iSCSI in-band authenticiation is used\n\n");
        printf("    ISCSI_CHAP_AUTH_TYPE = 1,\n");
        printf("        One way CHAP (Target authenticates initiator is used)\n\n");
        printf("    ISCSI_MUTUAL_CHAP_AUTH_TYPE = 2\n");
        printf("        Mutual CHAP (Target and Initiator authenticate each other is used)\n");
        printf("\n");
    }
    
    if ((Code == 0) ||
        (Code == 1))
    {
        printf("Target Flags:\n");
        printf("    ISCSI_TARGET_FLAG_HIDE_STATIC_TARGET            0x00000002\n");
        printf("        If this flag is set then the target will never be reported unless it\n");
        printf("        is also discovered dynamically.\n\n");
        
        printf("    ISCSI_TARGET_FLAG_MERGE_TARGET_INFORMATION      0x00000004\n");
        printf("        If this flag is set then the target information passed will be\n");
        printf("        merged with any target information already statically configured for\n");
        printf("        the target\n");
        printf("\n");
    }

    if ((Code == 0) ||
        (Code == 1) ||
        (Code == 3) ||
        (Code == 9) ||
        (Code == 11) ||
        (Code == 14) ||
        (Code == 27) ||
        (Code == 30) ||
        (Code == 33) ||
        (Code == 35) ||
        (Code == 36))
    {
        printf("CHAP secrets, CHAP passwords and IPSEC preshared keys can be specified as\n");
        printf("a text string or as a sequence of hexadecimal values. The value specified on\n");
        printf("the command line is always considered a string unless the first two characters\n");
        printf("0x in which case it is considered a hexadecimal value.\n");
        printf("\n");
        printf("For example 0x12345678 specifies a 4 byte secret\n");
        printf("\n");
    }
    
    printf("All numerical values are assumed decimal unless preceeded by 0x. If\n");
    printf("preceeded by 0x then value is assumed to be hex\n");
    printf("\n");
    
    if (Code == 0)
    {
#ifdef UNICODE
        printf("iscsicli can also be run in command line mode where iscsicli commands\n");
        printf("can be entered directly from the console. To enter command line\n");
        printf("mode, just run iscsicli without any parameters\n");
        printf("\n");
#endif
    }
}


PTCHAR GetiSCSIMessageText(
    __out_ecount(MessageLen) PTCHAR Message,
    ULONG MessageLen,
    ISDSC_STATUS Status
    )
{
    ULONG d;
    HRESULT hr;
    
    //
    // first check for os error code
    //
    d = FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_FROM_HMODULE,
                  GetModuleHandle(TEXT("iscsidsc.dll")),
                  Status,
                  0,                                 // langid
                  Message,
                  FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  NULL);
    if (d == 0)
    {
#ifdef UNICODE
        hr = StringCchPrintfW(Message,
                              MessageLen,
                              L"Status: 0x%x",
                              Status);
#else
        hr = StringCchPrintfA(Message, MessageLen, "Status: 0x%x", Status);
#endif
    } 

    return(Message);
}

ISDSC_STATUS DiscpUnicodeToAnsiSize(
    IN __in PWCHAR UnicodeString,
    OUT ULONG *AnsiSizeInBytes
    )
/*++

Routine Description:

    This routine will return the length needed to represent the unicode
    string as ANSI

Arguments:

    UnicodeString is the unicode string whose ansi length is returned

    *AnsiSizeInBytes is number of bytes needed to represent unicode
        string as ANSI

Return Value:

    ERROR_SUCCESS or error code

--*/
{
    _try
    {
        *AnsiSizeInBytes = WideCharToMultiByte(CP_ACP,
                                               0,
                                               UnicodeString,
                                               -1,
                                               NULL,
                                               0, NULL, NULL);
    } _except(EXCEPTION_EXECUTE_HANDLER) {
        return(ERROR_NOACCESS);
    }
    return((*AnsiSizeInBytes == 0) ? GetLastError() : ERROR_SUCCESS);
}


ULONG DiscpUnicodeToAnsi(
    IN __in_opt LPWSTR pszW,
    OUT __deref_out LPSTR *ppszA,
    IN ULONG MaxLen
    )
/*++

Routine Description:

    Convert Unicode string into its ansi equivalent

Arguments:

    pszW is unicode string to convert

    *ppszA on entry has a pointer to buffer to write ansi string or
        NULL. If NULL then a buffer is allocated

Return Value:

    Error code

--*/
{
    ULONG cCharacters;
    ULONG Status;
    ULONG cbAnsiUsed;
    BOOLEAN AllocMemory;

    //
    // If input is null then just return empty
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return(ERROR_SUCCESS);
    }


    if (*ppszA == NULL)
    {
        Status = DiscpUnicodeToAnsiSize(pszW, &MaxLen);
        if (Status == ERROR_SUCCESS)
        {
            *ppszA = Alloc(MaxLen);
            if (*ppszA == NULL)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                AllocMemory = FALSE;
            } else {
                AllocMemory = TRUE;
            }
        } else {
            AllocMemory = FALSE;
        }
    } else {
        AllocMemory = FALSE;
        Status = ERROR_SUCCESS;
    }

    if (Status == ERROR_SUCCESS)
    {
        cCharacters = (ULONG)wcslen(pszW)+1;


        // Convert to ANSI.
        cbAnsiUsed = WideCharToMultiByte(CP_ACP,
                                         0,
                                         pszW,
                                         cCharacters,
                                         *ppszA,
                                         MaxLen,
                                         NULL,
                                         NULL);

        if (0 == cbAnsiUsed)
        {
            Status = GetLastError();
            if (AllocMemory)
            {
                Free(*ppszA);
            }

        } else {
            Status = ERROR_SUCCESS;
        }
    }

    return(Status);

}
void PrintSecurityFlags(
    __in PCHAR Indent,
    ISCSI_SECURITY_FLAGS SecurityFlags
                       )
{
    printf("%sSecurity Flags        : 0x%I64x\n",
           Indent,
           SecurityFlags
          );

    if (SecurityFlags & ISCSI_SECURITY_FLAG_TUNNEL_MODE_PREFERRED)
    {
        printf("%s    Tunnel Mode Preferred\n",
               Indent
              );
    }

    if (SecurityFlags & ISCSI_SECURITY_FLAG_TRANSPORT_MODE_PREFERRED)
    {
        printf("%s    Transport Mode Preferred\n",
               Indent
              );
    }

    if (SecurityFlags & ISCSI_SECURITY_FLAG_PFS_ENABLED)
    {
        printf("%s    PFS Enabled\n",
               Indent
              );
    }

    if (SecurityFlags & ISCSI_SECURITY_FLAG_AGGRESSIVE_MODE_ENABLED)
    {
        printf("%s    Aggressive Mode Enabled\n",
               Indent
              );
    }

    if (SecurityFlags & ISCSI_SECURITY_FLAG_IKE_IPSEC_ENABLED)
    {
        printf("%s    IPSEC Enabled\n",
               Indent
              );
    }

    if (SecurityFlags & ISCSI_SECURITY_FLAG_VALID)
    {
        printf("%s    Security Flags are Valid\n",
               Indent
              );
    }
}

void PrintTargetMapping(
    PISCSI_TARGET_MAPPING Mapping
    )
{
    ULONG j;
    
#ifdef UNICODE
    printf("    Session Id             : %I64x-%I64x\n"        
           "    Target Name            : %ws\n"
           "    Initiator              : %ws\n"
           "    Initiator Scsi Device  : %ws\n"
           "    Initiator Bus          : %d\n"
           "    Initiator Target Id    : %d\n",
#else                   
    printf("    Session Id             : %I64x-%I64x\n"        
           "    Target Name            : %s\n"
           "    Initiator              : %s\n"
           "    Initiator Scsi Device  : %s\n"
           "    Initiator Bus          : %d\n"
           "    Initiator Target Id    : %d\n",
#endif
                           Mapping->SessionId.AdapterUnique,
                           Mapping->SessionId.AdapterSpecific,
                           Mapping->TargetName,
                           Mapping->InitiatorName,
                           Mapping->OSDeviceName,
                           Mapping->OSBusNumber,
                           Mapping->OSTargetNumber
                           );
    
    for (j = 0; j < Mapping->LUNCount; j++)
    {
        printf("        Target LUN: 0x%I64x <--> OS Lun: 0x%x\n",
               Mapping->LUNList[j].TargetLUN,
               Mapping->LUNList[j].OSLUN);
    }
    printf("\n");
}

void PrintStringList(
    __in PSTR Title,
    __in PSTR Spacer,
    __in PTSTR Buffer,
    ULONG SizeInBytes
    )
{
    printf("%s\n", Title);
    
    while (*Buffer != 0)
    {
#ifdef UNICODE
        printf("%s\"%ws\"\n", Spacer, Buffer);
#else
        printf("%s\"%s\"\n", Spacer, Buffer);
#endif
        if (SizeInBytes > 0)
        {
            while (*Buffer != 0)
            {
                Buffer++;
                SizeInBytes -= sizeof(TCHAR);
            }
            if (SizeInBytes > 0)
            {
                Buffer++;
                SizeInBytes -= sizeof(TCHAR);
            } else {
                printf("StringList error2\n");
                break;
            }
        } else {
            printf("StringList error\n");
            break;
        }
    }
}

void PrintBuffer(
    __in PSTR Spacer,
    __in_ecount(Size) PUCHAR Buffer,
    ULONG Size
    )
{
    ULONG i, n;
    
    while (Size > 0)
    {
        if (Size >= 0x10)
        {
            n = 0x10;
        } else {
            n = Size;
        }

        printf("%s", Spacer);
        for (i = 0; i < 0x10; i++)
        {
            if (i < n)
            {
                printf("%02x ", Buffer[i]);
            } else {
                printf("   ");
            }
        }
        printf("    ");
        for (i = 0; i < n; i++)
        {
            if ((Buffer[i] > 0x20) && (Buffer[i] < 0x7f))
            {
                printf("%c", Buffer[i]);
            } else {
                printf(".");
            }
        }
        printf("\n");
        
        Buffer += n;
        Size -= n;
    }
}

void PrintLoginOptions(
    __in CHAR *Header,
    PISCSI_LOGIN_OPTIONS LoginOptions
    )
{
    printf("%sVersion              : %d\n", Header, LoginOptions->Version);
    printf("%sInformation Specified: 0x%x\n",
           Header, 
           LoginOptions->InformationSpecified);

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_HEADER_DIGEST) == ISCSI_LOGIN_OPTIONS_HEADER_DIGEST)
    {
        printf("%sHeader Digest        : ", Header);
        if (LoginOptions->HeaderDigest == ISCSI_DIGEST_TYPE_NONE)
        {
            printf("None\n");
        } else if (LoginOptions->HeaderDigest == ISCSI_DIGEST_TYPE_CRC32C) {
            printf("CRC-32C\n");
        }
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_DATA_DIGEST) == ISCSI_LOGIN_OPTIONS_DATA_DIGEST)
    {
        printf("%sData Digest          : ", Header);
        if (LoginOptions->DataDigest == ISCSI_DIGEST_TYPE_NONE)
        {
            printf("None\n");
        } else if (LoginOptions->DataDigest == ISCSI_DIGEST_TYPE_CRC32C) {
            printf("CRC-32C\n");
        }
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_MAXIMUM_CONNECTIONS) == ISCSI_LOGIN_OPTIONS_MAXIMUM_CONNECTIONS)
    {
        printf("%sMaximum Connections  : %d\n",
               Header,
              LoginOptions->MaximumConnections);
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_WAIT) == ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_WAIT)
    {
        printf("%sDefault Time 2 Wait  : %d\n",
               Header,
              LoginOptions->DefaultTime2Wait);
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_RETAIN) == ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_RETAIN)
    {
        printf("%sDefault Time 2 Retain: %d\n",
               Header,
              LoginOptions->DefaultTime2Retain);
    }

    printf("%sLogin Flags          : 0x%x\n",
               Header,
           LoginOptions->LoginFlags);

    if ((LoginOptions->LoginFlags &
          ISCSI_LOGIN_FLAG_REQUIRE_IPSEC) == ISCSI_LOGIN_FLAG_REQUIRE_IPSEC)
    {
        printf("%s                       Require IPsec\n", Header);
    }

    if ((LoginOptions->LoginFlags &
          ISCSI_LOGIN_FLAG_MULTIPATH_ENABLED) == ISCSI_LOGIN_FLAG_MULTIPATH_ENABLED)
    {
        printf("%s                       Multipath Enabled\n", Header);
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_AUTH_TYPE) == ISCSI_LOGIN_OPTIONS_AUTH_TYPE)
    {
        printf("%sAuthentication Type  : ", Header);
        if (LoginOptions->AuthType == ISCSI_NO_AUTH_TYPE)
        {
            printf("None\n");
        } else if (LoginOptions->AuthType == ISCSI_CHAP_AUTH_TYPE) {
            printf("CHAP\n");
        } else if (LoginOptions->AuthType == ISCSI_MUTUAL_CHAP_AUTH_TYPE) {
            printf("Mutual CHAP\n");
        } else {
            printf("Unknown - %d\n", LoginOptions->AuthType);
        }
    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_USERNAME) == ISCSI_LOGIN_OPTIONS_USERNAME)
    {
        printf("%sUsername             : \n", Header);
        PrintBuffer("                          ",
                    LoginOptions->Username,
                    LoginOptions->UsernameLength);

    }

    if ((LoginOptions->InformationSpecified &
          ISCSI_LOGIN_OPTIONS_PASSWORD) == ISCSI_LOGIN_OPTIONS_PASSWORD)
    {
        printf("%sPassword             : <is established>\n", Header);
    }               
}

//+-------------------------------------------------------------------------
//
//  Function:   HexStringToULONGLONG   (private)
//
//  Synopsis:   scan lpsz for a number of hex digits (at most 8); update lpsz
//              return value in Value; check for chDelim;
//
//  Arguments:  [lpsz]    - the hex string to convert
//              [Value]   - the returned value
//              [cDigits] - count of digits
//
//  Returns:    TRUE for success
//
//--------------------------------------------------------------------------

BOOL HexStringToULONGLONG(
    __in PTSTR lpsz,
    ULONGLONG * RetValue,
    int cDigits,
    __in TCHAR chDelim
    )
{
    int Count;
    ULONGLONG Value;

    Value = 0;
    for (Count = 0; (Count < cDigits) && (*lpsz != 0); Count++, lpsz++)
    {
        if (*lpsz >= TEXT('0') && *lpsz <= TEXT('9'))
            Value = (Value << 4) + *lpsz - TEXT('0');
        else if (*lpsz >= TEXT('A') && *lpsz <= TEXT('F'))
            Value = (Value << 4) + *lpsz - TEXT('A') + 10;
        else if (*lpsz >= TEXT('a') && *lpsz <= TEXT('f'))
            Value = (Value << 4) + *lpsz - TEXT('a') + 10;
        else
            return(FALSE);
    }

    *RetValue = Value;
    
    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}

BOOLEAN StringToSessionId(
    IN __in PTSTR String,
    OUT PISCSI_UNIQUE_SESSION_ID SessionId
    )
{
    PTCHAR s, x;
    TCHAR c;
    
    //
    // Session id is in the form of
    // 0x1234567812345678-0x1234567812345678
    //

    //
    // skip over '0x'
    //
    if ((*String == TEXT('0')) &&
        ((String[1] == TEXT('x')) || (String[1] == TEXT('X'))))
    {
        String += 2;
    }
    s = String;

    while ((*String != TEXT('-') && (String[1] != 0)))
        String++;

    if (*String == 0)
    {
        return(FALSE);
    }

    x = String;
    c = *x;
    *String++ = 0;

    if (! HexStringToULONGLONG(s,
                               &SessionId->AdapterUnique,
                               sizeof(ULONGLONG)*2, 0))
    {
        *x = c;
        return(FALSE);
    }
    *x = c;

    if ((*String == TEXT('0')) &&
        ((String[1] == TEXT('x') ) || (String[1] == TEXT('X'))))
    {
        String += 2;
    }
    
    if (! HexStringToULONGLONG(String,
                               &SessionId->AdapterSpecific,
                               sizeof(ULONGLONG)*2, 0))
    {
        return(FALSE);
    }

    return(TRUE);
}

ULONG xtoi_(LPCTSTR lpsz)
{
    ULONG Count;
    ULONG Value;
    ULONG cDigits = (ULONG)_tcslen(lpsz);

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= TEXT('0') && *lpsz <= TEXT('9'))
            Value = (Value << 4) + *lpsz - TEXT('0');
        else if (*lpsz >= TEXT('A') && *lpsz <= TEXT('F'))
            Value = (Value << 4) + *lpsz - TEXT('A') + 10;
        else if (*lpsz >= TEXT('a') && *lpsz <= TEXT('f'))
            Value = (Value << 4) + *lpsz - TEXT('a') + 10;
        else
            return(0);
    }

    return(Value);
    
}

ISDSC_STATUS ParseHexString(
    IN LPCTSTR s,
    OUT PUCHAR *BufPtr,
    OUT ULONG *BufLen
    )
{
    ISDSC_STATUS Status;
    TCHAR temp[3];
    PUCHAR b;
    ULONG slen, blen;
    ULONG i,j;
    
    slen = (ULONG)_tcslen(s);
    blen = slen/2;

    if ((blen*2) == slen)
    {
        *BufLen = blen;
        b = Alloc(blen);
        if (b != NULL)
        {
            for (i = 0, j = 0; i < slen; i += 2, j++)
            {
                temp[0] = s[i];
                temp[1] = s[i+1];
                temp[2] = 0;
                b[j] = (UCHAR)xtoi_(temp);
            }
            *BufPtr = b;
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        //
        // string must have 2 characters for each binary value
        //
        Status = ERROR_INVALID_PARAMETER;
    }

    return(Status);
}



ULONG stoi(
    __in PTCHAR x
    )
{
    ULONG r;
    
    if ((_tcslen(x) > 2) &&
        (x[0] == TEXT('0')) &&
        ((x[1] == TEXT('x')) || (x[1] == TEXT('X'))))
    {
        r = xtoi_(x+2);
    } else {
#ifdef UNICODE
        r = _wtoi(x);
#else
        r = atoi(x);
#endif
    }
    return(r);
}

ULONGLONG stoiD(
    __in PTCHAR x
    )
{
    ULONGLONG r;
    ULONG base;
    
    if ((_tcslen(x) > 2) &&
        (x[0] == TEXT('0')) &&
        ((x[1] == TEXT('x')) || (x[1] == TEXT('X'))))
    {
        base = 16;
        x += 2;
    } else {
        base = 10;
    }
    
    r = _tcstoui64(x,
                   NULL,
                   base);
    return(r);
}

BOOLEAN stoiDForLogicalUnit(
    __in PTCHAR x,
    ULONGLONG *Value
    )
{
    if ((_tcslen(x) != 18) ||
        (x[0] != TEXT('0')) ||
        ((x[1] != TEXT('x')) && (x[1] == TEXT('X'))))
    {
        return(FALSE);
    }

    *Value = stoiD(x);
    
    return(TRUE);
}

void ParseLoginOptions(
    PISCSI_LOGIN_OPTIONS LoginOptions,
    __in_ecount(ArgC) PTSTR *ArgV,
    ULONG  ArgC,
    ULONG ArgCIndex
    )
{
    ISDSC_STATUS Status;
    PTCHAR Secret;
    UNREFERENCED_PARAMETER(ArgC);
    memset(LoginOptions, 0, sizeof(ISCSI_LOGIN_OPTIONS));
    
    LoginOptions->InformationSpecified = 0;

    LoginOptions->LoginFlags = 0;
    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->LoginFlags = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    
    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_HEADER_DIGEST;
        LoginOptions->HeaderDigest = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_DATA_DIGEST;
        LoginOptions->DataDigest = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_MAXIMUM_CONNECTIONS;
        LoginOptions->MaximumConnections = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_WAIT;
        LoginOptions->DefaultTime2Wait = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_DEFAULT_TIME_2_RETAIN;
        LoginOptions->DefaultTime2Retain = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_USERNAME;
        
        if (*ArgV[ArgCIndex] == TEXT('-'))
        {
            LoginOptions->Username = NULL;
            LoginOptions->UsernameLength = 0;
        } else {
#ifdef UNICODE
            LoginOptions->Username = NULL;
            DiscpUnicodeToAnsi(
                               ArgV[ArgCIndex],
                               (LPSTR *)&LoginOptions->Username,
                               0);
#else
            LoginOptions->Username = ArgV[ArgCIndex];
#endif
            LoginOptions->UsernameLength = (ULONG)strlen((LPCSTR)LoginOptions->Username);
        }
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_PASSWORD;
        if (*ArgV[ArgCIndex] == TEXT('-'))
        {
            LoginOptions->Password = NULL;
            LoginOptions->PasswordLength = 0;
        } else {
            Secret = ArgV[ArgCIndex];
            if ((Secret[0] == TEXT('0')) &&
                ((Secret[1] == TEXT('X')) ||
                 (Secret[1] == TEXT('x'))))
            {
                Status = ParseHexString(Secret+2,
                                        &LoginOptions->Password,
                                        &LoginOptions->PasswordLength);
            } else {
            
#ifdef UNICODE
                LoginOptions->Password = NULL;
                DiscpUnicodeToAnsi(
                                   ArgV[ArgCIndex],
                                   (LPSTR *)&LoginOptions->Password,
                                   0);
#else
                LoginOptions->Password = ArgV[ArgCIndex];
#endif
                LoginOptions->PasswordLength = (ULONG)strlen((LPCSTR)LoginOptions->Password);
            }
        }
    }
    ArgCIndex++;

    if (*ArgV[ArgCIndex] != TEXT('*'))
    {
        LoginOptions->InformationSpecified |= ISCSI_LOGIN_OPTIONS_AUTH_TYPE;
        LoginOptions->AuthType = stoi(ArgV[ArgCIndex]);
    }
    ArgCIndex++;
    
}


BOOLEAN IsTrue(
    __in PTCHAR s,
    BOOLEAN Default             
    )
{
    if (*s == TEXT('*'))
    {
        return(Default);
    }
    
    if ( (*s == TEXT('t')) || (*s == TEXT('T')))
    {
        return(TRUE);
    }
    return(FALSE);
}


ISDSC_STATUS TunnelAddress(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli TunnelAddr <Initiator Name> <InitiatorPort> <Destination
// Address> <Tunnel Address> <Persist>
{
    ISDSC_STATUS Status;
    PTCHAR Initiator, DestAddress, TunnelAddress;
    BOOLEAN Persist;
    ULONG InitiatorPort;

    if (ArgC != 7)
    {
        Usage(28);
        return(ERROR_SUCCESS);
    }

    Initiator = ArgV[2];
    if (*Initiator == TEXT('*'))
    {
        Initiator = NULL;
    }
    if (*ArgV[3] == TEXT('*'))
    {
        InitiatorPort = ISCSI_ALL_INITIATOR_PORTS;
    } else {
        InitiatorPort = stoi(ArgV[3]);
    }
    
    DestAddress = ArgV[4];
    if (*DestAddress == TEXT('*'))
    {
        DestAddress = NULL;
    }
    
    TunnelAddress = ArgV[5];
    if (*TunnelAddress == TEXT('*'))
    {
        TunnelAddress = NULL;
    }
    
    Persist = IsTrue(ArgV[6], TRUE);

    Status = SetIScsiTunnelModeOuterAddress(Initiator,
                                             InitiatorPort,
                                             DestAddress,
                                             TunnelAddress,
                                             Persist);

    return(Status);
}

ISDSC_STATUS GroupKey(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli GroupKey <Key> <Persist>
{
    ISDSC_STATUS Status;
    PCHAR Key;
    ULONG KeyLength = 0;
    BOOLEAN Persist;

    if (ArgC != 4)
    {
        Usage(29);
        return(ERROR_SUCCESS);
    }

    if (*ArgV[2] == TEXT('*'))
    {
        Key = NULL;
        KeyLength = 0;
        Status = ERROR_SUCCESS;
    } else {    
#ifdef UNICODE
        Key = NULL;
        Status = DiscpUnicodeToAnsi(ArgV[2],
                                    &Key,
                                    0); 
#else
        Key = ArgV[2];
        Status = ERROR_SUCCESS;
#endif
        if (Status == ERROR_SUCCESS)
        {
            KeyLength = (ULONG)strlen(Key) + 1;     
        }
    }

    if (Status == ERROR_SUCCESS)
    {
        Persist = IsTrue(ArgV[3], TRUE);

        Status = SetIScsiGroupPresharedKey(KeyLength,
                                           (PUCHAR)Key,
                                           Persist);

#ifdef UNICODE
        if (Key != NULL)
        {
            Free(Key);
        }
#endif
    }
    
    return(Status);
    
}



ISDSC_STATUS CHAPSecret(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli CHAPSecret <secret>
{
    ISDSC_STATUS Status = ERROR_SUCCESS;
    PUCHAR Key;
    ULONG KeyLength = 0;
    PTCHAR Secret;

    if (ArgC != 3)
    {
        Usage(27);
        return(Status);
    }
    
    Secret = ArgV[2];
    if (*Secret == TEXT('*'))
    {
        Key = NULL;
        KeyLength = 0;      
    } else {
        if ((Secret[0] == TEXT('0')) &&
            ((Secret[1] == TEXT('X')) ||
             (Secret[1] == TEXT('x'))))
        {
            Status = ParseHexString(Secret+2,
                                    &Key,
                                    &KeyLength);
        } else {
#ifdef UNICODE
            Key = NULL;
            Status = DiscpUnicodeToAnsi(ArgV[2],
                                        (LPSTR *)&Key,
                                        0); 
#else
            Key = ArgV[2];
            Status = ERROR_SUCCESS;
#endif
            if (Status == ERROR_SUCCESS)
            {
                KeyLength = (ULONG)strlen((LPCSTR)Key);
            }
        }
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = SetIScsiInitiatorCHAPSharedSecret(KeyLength,
                                          Key);
        
        if ((Key != NULL) && (Key != (PUCHAR)ArgV[2]))
        {
            Free(Key);
        }
    }
                                      
    return(Status);    
}

ISDSC_STATUS BindPeristentVolumes(
    int  ArgC ,
    __in_ecount(ArgC) PTCHAR *ArgV 
)
//    printf("iscsicli BindPersistentVolumes\n");
{
    ISDSC_STATUS Status;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    Status = SetupPersistentIScsiVolumes();
                                      
    return(Status);
    
}

ISDSC_STATUS ClearPersistentVolumes(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
//    printf("iscsicli ClearPersistentVolumes\n");
{
    ISDSC_STATUS Status;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    Status = ClearPersistentIScsiDevices();
                                      
    return(Status);
    
}

ISDSC_STATUS AddPersistentVolume(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
//        printf("iscsicli AddPersistentVolume <Volume Path>\n");
{
    ISDSC_STATUS Status;

    if (ArgC != 3)
    {
        Usage(38);
        Status = ERROR_SUCCESS;
    } else {    
        Status = AddPersistentIScsiDevice(ArgV[2]);
    }
                                      
    return(Status);
    
}

ISDSC_STATUS RemovePersistentVolume(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
//        printf("iscsicli RemovePersistentVolume <Volume Path>\n");
{
    ISDSC_STATUS Status;

    if (ArgC != 3)
    {
        Usage(39);
        Status = ERROR_SUCCESS;
    } else {    
        Status = RemovePersistentIScsiDevice(ArgV[2]);
    }
                                      
    return(Status);
    
}

ISDSC_STATUS ReportPersistentVolumes(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
//    printf("iscsicli ReportPersistentVolumes\n");
{
    ISDSC_STATUS Status;
    ULONG SizeNeeded;
    PTCHAR Buffer;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    SizeNeeded = 0;
    Status = ReportPersistentIScsiDevices(&SizeNeeded,
                                          NULL);
    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        Buffer = Alloc(SizeNeeded * sizeof(TCHAR));
        if (Buffer != NULL)
        {
            Status = ReportPersistentIScsiDevices(&SizeNeeded,
                                                  Buffer);

            if (Status == ERROR_SUCCESS)
            {
                PrintStringList("Persistent Volumes",
                                "",
                                Buffer,
                                SizeNeeded);
                printf("\n");
            }           
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
                                      
    return(Status);
    
}



ISDSC_STATUS NodeName(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli NodeName <node name>
{
    ISDSC_STATUS Status;
    PTCHAR NodeName;

    if (ArgC != 3)
    {
        Usage(25);
        return(ERROR_SUCCESS);
    }

    if (*ArgV[2] == TEXT('*'))
    {
        NodeName = NULL;
    } else {
        NodeName = ArgV[2];
    }
    
    Status = SetIScsiInitiatorNodeName(NodeName);

    return(Status);
    
}


BOOLEAN DiscpIsDeviceNumberInVolume(
    IN ULONG DeviceNumber,
    IN PVOLUME_DISK_EXTENTS Volume
    )
/*
Description:

    This routine will determine if the disk represented by a disk
    device number is part of a volume

Arguments:

    DeviceNumber is the disk device number

    Volume is the VOLUME_DISK_EXTENTS for the volume


Return Values:

    Status

*/
{
    ULONG i;

    for (i = 0; i < Volume->NumberOfDiskExtents; i++)
    {
        if (Volume->Extents[i].DiskNumber == DeviceNumber)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

PTCHAR DiscpIsStringInList(
    IN PTCHAR List,
    IN PTCHAR String
    )
/*
Description:

    This routine will determine if a string is already in a list of
    REG_MULTI_SZ. If so a pointer to where the string is located is
    returned.

Arguments:

    List has a pointer to a REG_MULTI_SZ list

    String is the string to find in the list

Return Values:

    Pointer to the string in the list or NULL if string is not in the
    list

*/
{
    PTCHAR p;

    p = List;

    if (*p == 0)
    {
        //
        // Empty list
        //
        return(NULL);
    }

    do
    {
        if (_tcsicmp(p, String) == 0)
        {
            //
            // We found our string in the list
            //
            return(p);
        }

        //
        // Advance to next string
        //
        p += (_tcslen(p) + 1);

    } while (*p != 0);

    return(NULL);
}

ISDSC_STATUS DiscpAddStringToMultiSzList(
    IN OUT PTCHAR *List,
    IN OUT PULONG ListSizeInBytes,
    IN PTCHAR String
    )
/*
Description:

    This routine will add a string to the end of a REG_MULTI_SZ list.
    It will reallocate the list buffer and copy over the existing
    contents and add the new string to the end of the list. If the
    string is already in the list then the string is not added

Arguments:

    *List on entry has a pointer to the REG_MULTI_SZ list. The memory
        used by the list should be allocated via DiscpAllocMemory. On
        return it has a pointer to the newly allocated buffer or NULL.

    *ListSizeInBytes on entry has the number of bytes contained in the
        REG_MULTI_SZ buffer. On return it is updated with the new
        number of bytes in the buffer.

    String is the string to add to the end of the buffer

Return Values:

    Status

*/
{
    ISDSC_STATUS Status;
    size_t NewListSizeInBytes;
    PTCHAR NewList;
    PTCHAR p;
    ULONG Len;
    HRESULT hr;

    //
    // First see if the string is already in the list
    //
    if ((DiscpIsStringInList(*List,
                            String)) == NULL)
    {
        if (*ListSizeInBytes != sizeof(TCHAR))
        {
            NewListSizeInBytes = *ListSizeInBytes +
                                 ((_tcslen(String)+1) * sizeof(TCHAR));
        } else {
            NewListSizeInBytes = ((_tcslen(String)+2) * sizeof(TCHAR));
        }

        NewList = Alloc(NewListSizeInBytes);
        if (NewList != NULL)
        {
            if (*ListSizeInBytes != sizeof(TCHAR))
            {
                //
                // Copy over existing list except for second nul terminator
                // at end
                //
                Len = (*ListSizeInBytes) - sizeof(TCHAR);
                memcpy(NewList, *List, Len);
            } else {
                Len = 0;
            }

            //
            // Now copy new string to the end of the list and add the
            // extra nul terminator
            //
            p = (PTCHAR)OffsetToPtr(NewList, Len);
            hr = StringCchCopy(p,
                                (NewListSizeInBytes - Len) / sizeof(WCHAR),
                                String);
            p += (_tcslen(String) + 1);
            *p = 0;

            //
            // return new list pointer and length
            //
            Free(*List);
            *List = NewList;
            *ListSizeInBytes = (ULONG)NewListSizeInBytes;
            Status = ERROR_SUCCESS;
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        //
        // String already in the list, nothing to do
        //
        Status = ERROR_SUCCESS;
    }

    return(Status);
}

ISDSC_STATUS DiscpVolumeMountList(
    IN PTCHAR Name,
    IN PTCHAR VolumeNameToFind,
    IN OUT PTCHAR *VolumePath,
    IN OUT ULONG *VolumePathLen
    )
/*++

Routine Description:

    This routine will see if the volume passed or any volumes mounted
    on the volume passed are the volume that we are looking for and if
    so then add the path to the volume to the list

Arguments:

    Name is the volume name for the volume on which to look for volume
        paths that are for VolumeNameToFind

    VolumeNameToFind is the name of the volume we are searching for 

    *VolumePath returns with a pointer to the list of volume paths.
        Each path in the list is nul terminated with the last path
        double nul terminated. The caller must free this buffer. 

    *VolumePathLen returns with the number of bytes in the
        VolumePath buffer

Return Value:

    Status

--*/
{
    ISDSC_STATUS Status;
    PTCHAR VolumeName;
    BOOL b;
    TCHAR c1, c2;
    PTCHAR LinkName1, LinkName2;
    PTCHAR VolumeMountPoint;
    PTCHAR MountPointPath;
    HANDLE h;
    HRESULT hr;

    VolumeName = Alloc(5 * (MAX_PATH * sizeof(TCHAR)));
    if (VolumeName != NULL)
    {
        Status = ERROR_SUCCESS;
        LinkName1 = (PTCHAR)OffsetToPtr(VolumeName,
                                        MAX_PATH * sizeof(TCHAR));
        LinkName2 = (PTCHAR)OffsetToPtr(LinkName1,
                                        MAX_PATH * sizeof(TCHAR));
        VolumeMountPoint = (PTCHAR)OffsetToPtr(LinkName2,
                                               MAX_PATH * sizeof(TCHAR));
        
        MountPointPath = (PTCHAR)OffsetToPtr(VolumeMountPoint,
                                               MAX_PATH * sizeof(TCHAR));
        
        //
        // See if the mount point is for our volume name
        //
        b = GetVolumeNameForVolumeMountPoint(Name,
                                             VolumeName,
                                             MAX_PATH);
        if (b)
        {
            
            if (_tcsicmp(VolumeName,
                         VolumeNameToFind) == 0)
            {
                //
                // we found a mountpoint for our volume name,
                // lets add it to the list
                //
                Status = DiscpAddStringToMultiSzList(VolumePath,
                                                     VolumePathLen,
                                                     Name);
            } else {
                c1 = VolumeName[48];
                c2 = VolumeNameToFind[48];
                VolumeName[48] = 0;
                VolumeNameToFind[48] = 0;

                if (QueryDosDevice(&VolumeName[4],
                                   LinkName1,
                                   MAX_PATH) &&
                    QueryDosDevice(&VolumeNameToFind[4],
                                   LinkName2,
                                   MAX_PATH))
                {
                    if (_tcscmp(LinkName1, LinkName2) == 0)
                    {
                        Status = DiscpAddStringToMultiSzList(VolumePath,
                                                             VolumePathLen,
                                                             Name);
                    }
                }

                VolumeName[48] = c1;
                VolumeNameToFind[48] = c2;
            }

            h = FindFirstVolumeMountPoint(VolumeName,
                                          VolumeMountPoint,
                                          MAX_PATH);
            if (h != INVALID_HANDLE_VALUE)
            {
                do
                {
                    hr = StringCchCopy(MountPointPath,
                                        MAX_PATH,
                                        Name);
                    hr = StringCchCat(MountPointPath,
                                       MAX_PATH,
                                       VolumeMountPoint);

                    Status = DiscpVolumeMountList(MountPointPath,
                                                  VolumeNameToFind,
                                                  VolumePath,
                                                  VolumePathLen);

                    b = FindNextVolumeMountPoint(h,
                                                 VolumeMountPoint,
                                                 MAX_PATH);

                } while ((Status == ERROR_SUCCESS) && b);

                FindVolumeMountPointClose(h);
            }
        }
        
        Free(VolumeName);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    return(Status);
}

ISDSC_STATUS DiscpGetVolumePathNamesForVolumeName(
    IN PTCHAR VolumeNameToFind,
    OUT PTCHAR *VolumePath,
    OUT ULONG *VolumePathLen
    )
/*++

Routine Description:

    My implementation of the GetVolumePathNamesForVolumeName
    functionality, but for W2K. What we need to do is to find the
    volume name for every drive letter and then the volume name for
    every mount point and if any of them match our volume name then
    we've got a mapping. Note there could be multiple paths for a
    volume name.

Arguments:

    VolumeNameToFind is the name of the volume

    *VolumePath returns with a pointer to the list of volume paths.
        Each path in the list is nul terminated with the last path
        double nul terminated. The caller must free this buffer. 

    *VolumePathLen returns with the number of characters in the
        VolumePath buffer

Return Value:

    Status

--*/
{
    ISDSC_STATUS Status;
    TCHAR c;
    TCHAR Drive[4];
    
    //
    // Initialize output volume path list to double nul
    //
    *VolumePathLen = sizeof(TCHAR);
    *VolumePath = (PTCHAR)Alloc(*VolumePathLen);
    if (*VolumePath != NULL)
    {
        (*VolumePath)[0] = 0;

        //
        // Loop through all drive letters looking for mount points that
        // match our volume name
        //
        Drive[1] = L':';
        Drive[2] = L'\\';
        Drive[3] = 0;

        for (c = L'C', Status = ERROR_SUCCESS;
             ((c < (L'Z' + 1)) && (Status == ERROR_SUCCESS));
             c++)
        {
            Drive[0] = c;
            Status = DiscpVolumeMountList(Drive,
                                          VolumeNameToFind,
                                          VolumePath,
                                          VolumePathLen);

        }
        
        //
        // COnvet VolumePathLen from bytes to characters
        //
        *VolumePathLen = *VolumePathLen / sizeof(TCHAR);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    return(Status);
}


typedef BOOL (*GETVOLUMEPATHNAMESFORVOLUMENAMEAPI)(
  LPCTSTR lpszVolumeName,
  LPTSTR lpszVolumePathNames,
  DWORD cchBufferLength,
  PDWORD lpcchReturnLength
);



ISDSC_STATUS DiscpVolumeNameToVolumePath(
    IN PTCHAR VolumeName,
    OUT PTCHAR *VolumePath,
    OUT ULONG *VolumePathLen
    )
/*++

Routine Description:

    This routine will map a volume name to the volume paths for it. XP
    and W2003 have a nifty function that does this easily, but it is
    not available on W2K. So this routine will figure out if we are on
    W2K or not and do it the hard way on W2K or the easy way on XP and
    W2003. This api should behave in a functionally identical way to
    the GetVolumePathNamesForVolumeName api.

Arguments:

    VolumeName is the name of the volume

    *VolumePath returns with a pointer to the list of volume paths.
        Each path in the list is nul terminated with the last path
        double nul terminated. The caller must free this buffer. 

    *VolumePathLen returns with the number of characters in the
        VolumePath buffer

Return Value:

    Status

--*/
{
    ISDSC_STATUS Status;
    OSVERSIONINFOEX VersionInfo;
    BOOL b;
    HMODULE Module;
    GETVOLUMEPATHNAMESFORVOLUMENAMEAPI GetVolumePathNamesForVolumeNameApi;
    PTCHAR p;
    ULONG CharNeeded;

    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO)&VersionInfo))
    {
        if ((VersionInfo.dwMajorVersion == 5) &&
            (VersionInfo.dwMinorVersion == 0))
        {
            //
            // We are on W2K so we need to do the mapping from volume
            // name to VolumePath the hard way
            //
            Status = DiscpGetVolumePathNamesForVolumeName(VolumeName,
                                                          VolumePath,
                                                          VolumePathLen);
        } else {
            //
            // Since we are on XP or W2003 then we can use the
            // advanced API so load it up from kernel32.dll
            //
            Module = GetModuleHandle(TEXT("Kernel32.Dll"));
            GetVolumePathNamesForVolumeNameApi =
                (GETVOLUMEPATHNAMESFORVOLUMENAMEAPI)GetProcAddress(Module,
                                                                   "GetVolumePathNamesForVolumeNameW");
            if (GetVolumePathNamesForVolumeNameApi != NULL)
            {               
                b = (*GetVolumePathNamesForVolumeNameApi)(VolumeName,
                                                          NULL,
                                                          0,
                                                          &CharNeeded);

                Status = b ? ERROR_SUCCESS : GetLastError();

                if (Status == ERROR_MORE_DATA)
                {
                    p = Alloc(CharNeeded * sizeof(TCHAR));
                    if (p != NULL)
                    {
                        b = (*GetVolumePathNamesForVolumeNameApi)(VolumeName,
                                                                  p,
                                                                  CharNeeded,
                                                                  &CharNeeded);

                        Status = b ? ERROR_SUCCESS : GetLastError();
                        if (Status == ERROR_SUCCESS)
                        {
                            *VolumePath = p;
                            *VolumePathLen = CharNeeded;
                        } else {
                            Free(p);
                        }
                     } else {
                        Status = ERROR_NOT_ENOUGH_MEMORY;
                    }
                } else if (Status == ERROR_SUCCESS) {
                    p = Alloc(2 * sizeof(TCHAR));
                    if (p != NULL)
                    {
                        p[0] = 0;
                        p[1] = 0;
                        *VolumePath = p;
                        *VolumePathLen = 2;
                    } else {
                        Status = ERROR_NOT_ENOUGH_MEMORY;
                    }                       
                } else {
                    Status = GetLastError();
                }
            } else {
                Status = GetLastError();
            }
        }           
    } else {
        Status = GetLastError();
    }       

    return(Status);
}

ISDSC_STATUS DiscpEnumerateDeviceInterfaces(
    IN LPGUID Guid,
    IN ENUMDEVICEINTERFACECALLBACK Callback,
    IN PVOID Context,
    OUT ULONG *CountPtr,
    OUT PDEVICEINTERFACEENTRY *ListPtr
    )
{
    ISDSC_STATUS Status;
    HDEVINFO DevInfo;
    BOOL b;
    ULONG i, Count;
    SP_DEVICE_INTERFACE_DATA   DeviceInterfaceData;
    PDEVICEINTERFACEENTRY List, e;
    ULONG DeviceInterfaceDetailDataSize;

    //
    // get info on all exsiting disk devices
    //
    DevInfo = SetupDiGetClassDevs(
                Guid,
                NULL, // IN PCTSTR  Enumerator,  OPTIONAL
                NULL, // IN HWND  hwndParent,  OPTIONAL
                DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (DevInfo != INVALID_HANDLE_VALUE)
    {
        //
        // First step is to get the count of disks so we can build our
        // list
        //

        Count = 0;
        do
        {
            DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            b =  SetupDiEnumDeviceInterfaces(
                                             DevInfo,
                                             NULL, // IN PSP_DEVINFO_DATA  DeviceInfoData,  OPTIONAL
                                             Guid,
                                             Count,
                                             &DeviceInterfaceData
                                            );
            Count++;
        } while (b);

        Status = GetLastError();

        if (Status == ERROR_NO_MORE_ITEMS)
        {
            Count--;

            //
            // Now that we've got a rough count, lets loop over all of the
            // disks again and build up out data structs
            //
            *CountPtr = 0;
            if (Count > 0)
            {
                //
                // Allocate space for all of the entries
                //
                List = Alloc(Count * sizeof(DEVICEINTERFACEENTRY));
                if (List != NULL)
                {
                    *ListPtr = List;

                    memset(List, 0, Count * sizeof(DEVICEINTERFACEENTRY));

                    i = 0;
                    while (i < Count)
                    {
                        //
                        // Call setupdi to get the information needed for
                        // each interface
                        //
                        e = &List[(*CountPtr)];

                        e->DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
                        b =  SetupDiEnumDeviceInterfaces(
                                                         DevInfo,
                                                         NULL, // IN PSP_DEVINFO_DATA  DeviceInfoData,  OPTIONAL
                                                         Guid,
                                                         i++,
                                                         &e->DeviceInterfaceData
                                                        );
                        if (b)
                        {
                            DeviceInterfaceDetailDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                                                            (MAX_PATH * sizeof(TCHAR));
                            e->DeviceInterfaceDetailData = Alloc(DeviceInterfaceDetailDataSize);
                            if (e->DeviceInterfaceDetailData != NULL)
                            {
                                e->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                                e->DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                                b =  SetupDiGetDeviceInterfaceDetail(
                                                                DevInfo,
                                                                &e->DeviceInterfaceData,
                                                                e->DeviceInterfaceDetailData,
                                                                DeviceInterfaceDetailDataSize,
                                                                &DeviceInterfaceDetailDataSize,
                                                                &e->DeviceInfoData);
                                if ((b == FALSE) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                                {
                                    //
                                    // If buffer was too small for
                                    // DeviceInterfaceDetailData then try
                                    // the call again
                                    //
                                    Free(e->DeviceInterfaceDetailData);
                                    e->DeviceInterfaceDetailData = Alloc(DeviceInterfaceDetailDataSize);
                                    if (e->DeviceInterfaceDetailData != NULL)
                                    {
                                        e->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                                        e->DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                                        b =  SetupDiGetDeviceInterfaceDetail(
                                                                        DevInfo,
                                                                        &e->DeviceInterfaceData,
                                                                        e->DeviceInterfaceDetailData,
                                                                        DeviceInterfaceDetailDataSize,
                                                                        &DeviceInterfaceDetailDataSize,
                                                                        &e->DeviceInfoData);
                                    }
                                }

                                if (b)
                                {
                                    //
                                    // if we've successfully gathered all
                                    // of our info, then we do the callout
                                    // for the specific work
                                    //
                                    if (Callback != NULL)
                                    {
                                        Status = (*Callback)(Context,
                                                             Guid,
                                                             DevInfo,
                                                             e);
                                    } else {
                                        Status = ERROR_SUCCESS;
                                    }

                                    if (Status == ERROR_SUCCESS)
                                    {
                                        (*CountPtr)++;
                                    } else {
                                        Free(e->DeviceInterfaceDetailData);
                                    }
                                } else {
                                    if (e->DeviceInterfaceDetailData != NULL)
                                    {
                                        Free(e->DeviceInterfaceDetailData);
                                    }
                                }
                            }
                        }
                    }
                    Status = ERROR_SUCCESS;
                } else {
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }

        SetupDiDestroyDeviceInfoList(DevInfo);
    } else {
        Status = GetLastError();
    }
    return(Status);
}

void DiscpFreeDeviceInterfaceList(
    IN ULONG Count,
    IN PDEVICEINTERFACEENTRY List
    )
/*++
Description:

    This routine will free all of the memory allocate for a device
    interface list

Arguments:

    Count is the number of entries in the list

    List is the list of device entries


Return Values:

    ERROR_SUCCESS or error code

--*/
{
    ULONG i;

    for (i = 0; i < Count; i++)
    {
        Free(List[i].DeviceInterfaceDetailData);
        if (List[i].MoreInfo != NULL)
        {
            Free(List[i].MoreInfo);
        }
    }
    Free(List);
}


ISDSC_STATUS DiscpVolumeDeviceInterfaceCallback(
    IN PVOID Context,
    IN LPGUID Guid,
    IN HDEVINFO DevInfo,
    IN OUT PDEVICEINTERFACEENTRY DevEntry
    )
/*++

Routine Description:

    This routine is the volume device interface callback. It gets
    additional information about each volume that is found. Information
    includes the disk extents and the volume path names.

Arguments:

    Context is not used

    Guid is the guid for the volume device interface

    DevInfo is the DevInfo set for the enumeration

    DevEntry is the device interface information structure. This
        routine will insert a pointer to the VOLUME_DISK_EXTENTS for
        the device interface

Return Value:

    Status

--*/
{
    HANDLE Handle;
    ULONG Status;
    BOOL b;
    ULONG Returned;
    ULONG ExtentSize = sizeof(VOLUME_DISK_EXTENTS) + 1024 * sizeof(DISK_EXTENT);
    ULONG VolumeSize, Size;
    PVOLUMEMOREINFO VolumeMoreInfo;
    PTCHAR VolumeName, VolumeMP;
    PTCHAR VolumePath;
    size_t VolumeMPLen;
    HRESULT hr;
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Guid);
    UNREFERENCED_PARAMETER(DevInfo);
    
    VolumeMPLen = _tcslen(DevEntry->DeviceInterfaceDetailData->DevicePath) +2;
    VolumeName = Alloc((VolumeMPLen + MAX_PATH) *
                                  sizeof(TCHAR));
    if (VolumeName != NULL)
    {
        //
        // Append a \ to the end of the device interface name because that
        // is what GetVolumeNameForVolumeMountPoint wants
        //
        VolumeMP = (PTCHAR)OffsetToPtr(VolumeName, MAX_PATH * sizeof(TCHAR));
        hr = StringCchCopy(VolumeMP,
                           VolumeMPLen,
                           DevEntry->DeviceInterfaceDetailData->DevicePath);
        hr = StringCchCat(VolumeMP,
                          VolumeMPLen,
                          TEXT("\\"));

        b = GetVolumeNameForVolumeMountPoint(VolumeMP,
                                            VolumeName,
                                            MAX_PATH);

        if (b)
        {
            //
            // Now we have the volume name, we can get the volume paths
            // that can access it. First figure out the size needed for
            // the pathname buffer
            //
            Status = DiscpVolumeNameToVolumePath(VolumeName,
                                                 &VolumePath,
                                                  &VolumeSize);
            
            if (Status == ERROR_SUCCESS)
            {
                //
                // Get the disk extents for the volume
                //
                Size = ExtentSize + (VolumeSize * sizeof(TCHAR));
                Handle = CreateFile(DevEntry->DeviceInterfaceDetailData->DevicePath,
                                    GENERIC_READ,                      // access mode
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,                          // share mode
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,                 // file attributes
                                    NULL);

                if (Handle != INVALID_HANDLE_VALUE)
                {
                    DevEntry->MoreInfo = Alloc(Size);

                    if (DevEntry->MoreInfo != NULL)
                    {
                        VolumeMoreInfo = (PVOLUMEMOREINFO)DevEntry->MoreInfo;
                        b = DeviceIoControl(
                                            Handle,
                                            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,       // operation
                                            NULL,           // input data buffer
                                            0,         // size of input data buffer
                                            VolumeMoreInfo->VolumeDiskExtentsBuffer,          // output data buffer
                                            ExtentSize,        // size of output data buffer
                                            &Returned,     // byte count
                                            NULL    // overlapped information
                                           );

                        if (b)
                        {
                            //
                            // And if all goes well, get the volume
                            // path names for the volume
                            //
                            memcpy(VolumeMoreInfo->VolumePathNames,
                                   VolumePath,
                                   VolumeSize * sizeof(TCHAR));
                                            
                            Status = ERROR_SUCCESS;
                        } else {
                            Free(DevEntry->MoreInfo);
                            DevEntry->MoreInfo = NULL;
                            Status = GetLastError();
                        }
                    } else {
                        Status = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    CloseHandle(Handle);
                } else {
                    Status = GetLastError();
                }
                Free(VolumePath);
            }
        } else {
            Status = GetLastError();
        }
        Free(VolumeName);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(Status);
}


PCHAR DeviceTypeFromGuid(
    LPGUID Guid
    )
{
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_DISK,
               sizeof(GUID)) == 0)
    {
        return("Disk");
    }

    if (memcmp(Guid,
               &GUID_DEVINTERFACE_TAPE,
               sizeof(GUID)) == 0)
    {
        return("Tape");
    }
    
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_CDROM,
               sizeof(GUID)) == 0)
    {
        return("CDRom");
    }
    
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_WRITEONCEDISK,
               sizeof(GUID)) == 0)
    {
        return("Write Once Disk");
    }
    
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_CDCHANGER,
               sizeof(GUID)) == 0)
    {
        return("CD Changer");
    }
    
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_MEDIUMCHANGER,
               sizeof(GUID)) == 0)
    {
        return("Medium Changer");
    }
    
    if (memcmp(Guid,
               &GUID_DEVINTERFACE_FLOPPY,
               sizeof(GUID)) == 0)
    {
        return("Floppy");
    }

    return("Unknown");
}



ISDSC_STATUS GetMountPointsFromDeviceNumber(
    IN ULONG DeviceNumber,
    IN ULONG VolumeCount,
    IN PDEVICEINTERFACEENTRY VolumeList,
    IN OUT ULONG *VolumeIndex
    )
/*
Description:

    This routine will return the index of the volume that the device
    belongs. 

Arguments:

    DeviceNumber has the disk device number

    VolumeCount has the number of volumes

    VolumeList has information about the voluem device interfaces

    *VoluemIndex on entry has the next index in the list to being the
        search. On return it has the index into volume list for the
        next match. 

Return Values:

    Status

*/
{
    ULONG j;
    ISDSC_STATUS Status = ERROR_INVALID_PARAMETER;
    PVOLUMEMOREINFO VolumeMoreInfo;

    for (j = *VolumeIndex; j < VolumeCount; j++)
    {
        if (VolumeList[j].MoreInfo != NULL)
        {
            VolumeMoreInfo = (PVOLUMEMOREINFO)VolumeList[j].MoreInfo;
            if (DiscpIsDeviceNumberInVolume(DeviceNumber,
                                            &VolumeMoreInfo->VolumeDiskExtents))
            {
                *VolumeIndex = j;
                Status = ERROR_SUCCESS;
                break;
            }
        }
    }
    return(Status);
}


ISDSC_STATUS SessionList(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli SessionList
{
    ISDSC_STATUS Status;
    PISCSI_SESSION_INFO SessionInfo;
    PISCSI_CONNECTION_INFO ConnectionInfo;
    ULONG i,j;
    ULONG SessionCount, SizeNeeded;
    BOOLEAN ShowDeviceInfo = TRUE;
    ISDSC_STATUS StatusDontCare;
    BOOLEAN HeaderPrinted;

    if ((ArgC < 2) || (ArgC > 3))
    {
        Usage(26);
        return(ERROR_SUCCESS);
    }

    if (ArgC == 3)
    {
        ShowDeviceInfo = IsTrue(ArgV[2],
                                TRUE);
    }

    SizeNeeded = 0;
    Status = GetIScsiSessionList(
                                 &SizeNeeded,
                                 &SessionCount,
                                 NULL);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        SessionInfo = (PISCSI_SESSION_INFO)Alloc(SizeNeeded);
        if (SessionInfo == NULL)
        {
            printf("Couldn't alloc for session list\n");
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Status = GetIScsiSessionList(
                                     &SizeNeeded,
                                     &SessionCount,
                                     SessionInfo);


    } else {
        SessionCount = 0;
        SessionInfo = NULL;
    }

    if (Status == ERROR_SUCCESS)
    {
        printf("Total of %d sessions\n\n", SessionCount);

                                          
        for (i = 0; i < SessionCount; i++, SessionInfo++)
        {
#if UNICODE
            printf("Session Id             : %I64x-%I64x\n"
                   "Initiator Node Name    : %ws\n"
                   "Target Node Name       : %ws\n"
                   "Target Name            : %ws\n"
                   "ISID                   : %02x %02x %02x %02x %02x %02x\n"
                   "TSID                   : %02x %02x\n"
                   "Number Connections     : %d\n",
                   SessionInfo->SessionId.AdapterUnique,
                   SessionInfo->SessionId.AdapterSpecific,
                   SessionInfo->InitiatorName,
                   SessionInfo->TargetNodeName,
                   SessionInfo->TargetName,
                   SessionInfo->ISID[0],
                   SessionInfo->ISID[1],
                   SessionInfo->ISID[2],
                   SessionInfo->ISID[3],
                   SessionInfo->ISID[4],
                   SessionInfo->ISID[5],
                   SessionInfo->TSID[0],
                   SessionInfo->TSID[1],
                   SessionInfo->ConnectionCount);
#else
            printf("Session Id             : %I64x-%I64x\n"
                   "Initiator Node Name    : %s\n"
                   "Target Node Name       : %s\n"
                   "Target Name            : %s\n"
                   "ISID                   : %02x %02x %02x %02x %02x %02x\n"
                   "TSID                   : %02x %02x\n"
                   "Number Connections     : %d\n",
                   SessionInfo->SessionId.AdapterUnique,
                   SessionInfo->SessionId.AdapterSpecific,
                   SessionInfo->InitiatorName,
                   SessionInfo->TargetNodeName,
                   SessionInfo->TargetName,
                   SessionInfo->ISID[0],
                   SessionInfo->ISID[1],
                   SessionInfo->ISID[2],
                   SessionInfo->ISID[3],
                   SessionInfo->ISID[4],
                   SessionInfo->ISID[5],
                   SessionInfo->TSID[0],
                   SessionInfo->TSID[1],
                   SessionInfo->ConnectionCount);
#endif
            if (SessionInfo->ConnectionCount > 0)
            {
                printf("\n    Connections:\n");
            }
            
            for (j = 0; j < SessionInfo->ConnectionCount; j++)
            {
                ConnectionInfo = &SessionInfo->Connections[j];
#ifdef UNICODE
                printf("        Connection Id     : %I64x-%I64x\n"
                       "        Initiator Portal  : %ws/%d\n"
                       "        Target Portal     : %ws/%d\n"
                       "        CID               : %02x %02x\n",
                       ConnectionInfo->ConnectionId.AdapterUnique,
                       ConnectionInfo->ConnectionId.AdapterSpecific,
                       ConnectionInfo->InitiatorAddress,
                       ConnectionInfo->InitiatorSocket,
                       ConnectionInfo->TargetAddress,
                       ConnectionInfo->TargetSocket,
                       ConnectionInfo->CID[0],
                       ConnectionInfo->CID[1]);
#else
                printf("        Connection Id     : %I64x-%I64x\n"
                       "        Initiator Portal  : %s/%d\n"
                       "        Target Portal     : %s/%d\n"
                       "        CID               : %02x %02x\n",
                       ConnectionInfo->ConnectionId.AdapterUnique,
                       ConnectionInfo->ConnectionId.AdapterSpecific,
                       ConnectionInfo->InitiatorAddress,
                       ConnectionInfo->InitiatorSocket,
                       ConnectionInfo->TargetAddress,
                       ConnectionInfo->TargetSocket,
                       ConnectionInfo->CID[0],
                       ConnectionInfo->CID[1]);
#endif
            }
            
            printf("\n");

            if (ShowDeviceInfo)
            {
                ULONG DeviceCount;
                PISCSI_DEVICE_ON_SESSION DeviceList;
                ULONG VolumeCount;
                PDEVICEINTERFACEENTRY VolumeList;
                ULONG k;
                PTCHAR p;
                PVOLUMEMOREINFO VolumeMoreInfo;
                ULONG VolumeIndex;
                
                Status = GetDevicesForIScsiSession(&SessionInfo->SessionId,
                                                   &DeviceCount,
                                                   NULL);
                if (Status == ERROR_INSUFFICIENT_BUFFER)
                {
                    DeviceList = Alloc(DeviceCount *
                                       sizeof(ISCSI_DEVICE_ON_SESSION));
                    if (DeviceList != NULL)
                    {
                        Status = GetDevicesForIScsiSession(&SessionInfo->SessionId,
                                                           &DeviceCount,
                                                           DeviceList);
                        if ((Status == ERROR_SUCCESS) &&
                            (DeviceCount > 0))
                        {
                            Status = DiscpEnumerateDeviceInterfaces((LPGUID)&GUID_DEVINTERFACE_VOLUME,
                                                                    DiscpVolumeDeviceInterfaceCallback,
                                                                    NULL,
                                                                    &VolumeCount,
                                                                    &VolumeList);
                            if (Status != ERROR_SUCCESS)
                            {
                                VolumeCount = 0;
                                VolumeList = NULL;
                            }

                            printf("    Devices:\n");
                            for (k = 0; k < DeviceCount; k++)
                            {
                                printf("        Device Type            : %s\n",
                                       DeviceTypeFromGuid(&DeviceList[k].DeviceInterfaceType));

                                printf("        Device Number          : %d\n",
                                       DeviceList[k].StorageDeviceNumber.DeviceNumber);

                                printf("        Storage Device Type    : %d\n",
                                       DeviceList[k].StorageDeviceNumber.DeviceType);

                                printf("        Partition Number       : %d\n",
                                       DeviceList[k].StorageDeviceNumber.PartitionNumber);

                                
                                if (memcmp(&DeviceList[k].DeviceInterfaceType,
                                           &GUID_DEVINTERFACE_DISK,
                                           sizeof(GUID)) == 0)
                                {
                                    HeaderPrinted = FALSE;
                                    VolumeIndex = 0;
                                    for( ; ;)
                                    {
                                        StatusDontCare = GetMountPointsFromDeviceNumber(DeviceList[k].StorageDeviceNumber.DeviceNumber,
                                                                        VolumeCount,
                                                                        VolumeList,
                                                                        &VolumeIndex);
                                        if (StatusDontCare == ERROR_SUCCESS)
                                        {
                                            if (! HeaderPrinted)
                                            {
                                                printf("        Volume Path Names      : \n");
                                                HeaderPrinted = TRUE;
                                            }
                                            VolumeMoreInfo = (PVOLUMEMOREINFO)(VolumeList[VolumeIndex].MoreInfo);
                                            p = VolumeMoreInfo->VolumePathNames;
                                            while (*p != 0)
                                            {
#ifdef UNICODE
                                                printf("                                 %ws\n", p);
#else
                                                printf("                                 %s\n", p);
#endif
                                                p += _tcslen(p)+1;
                                            }
                                            VolumeIndex++;
                                        } else {
                                            break;
                                        }
                                    } 
                                }

                                printf("\n");
                            }

                            if (VolumeList != NULL)
                            {
                                DiscpFreeDeviceInterfaceList(VolumeCount,
                                                             VolumeList);
                            }
                            
                        }
                        Free(DeviceList);
                    }
                }
                Status = ERROR_SUCCESS;
            }           
        }
    }
    
    return(Status);
    
}

ISDSC_STATUS GetPSKey(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
//        printf("iscsicli GetPSKey <Initiator Name> <initiator Port>
//        <Id Type> <Id>\n");
{
    ISDSC_STATUS Status;
    PTCHAR Initiator;
    PCHAR Id;
    ULONG IdLen;
    IKE_IDENTIFICATION_PAYLOAD_TYPE IdType;
    ULONG Addr;
    PCHAR IdText;
    ULONG InitiatorPort;
    struct sockaddr_storage sockaddr;
    int sockaddrlen;
    struct sockaddr_in6 *sockaddr6;
    struct sockaddr_in *sockaddr4;
    IKE_AUTHENTICATION_INFORMATION AuthInfo;
    
    if (ArgC != 6)
    {
        Usage(42);
        return(ERROR_SUCCESS);
    }

    Initiator = ArgV[2];
    if (*Initiator == TEXT('*'))
    {
        Initiator = NULL;
    }
    if (*ArgV[3] == TEXT('*'))
    {
        InitiatorPort = ISCSI_ALL_INITIATOR_PORTS;
    } else {
        InitiatorPort = stoi(ArgV[3]);
    }
    
    IdType = (IKE_IDENTIFICATION_PAYLOAD_TYPE)stoi(ArgV[4]);
    
#ifdef UNICODE
    IdText = NULL;
    Status = DiscpUnicodeToAnsi(ArgV[5],
                                &IdText,
                                0);
    if (IdText == NULL)
    {
        Usage(42);
        return(ERROR_SUCCESS);      
    }
#else
    IdText = ArgV[5];
    Status = ERROR_SUCCESS;
#endif

    if (Status == ERROR_SUCCESS)
    {
        switch(IdType)
        {
            case ID_IPV4_ADDR:
            {
                sockaddrlen = sizeof(sockaddrlen);
                Status = WSAStringToAddressA(IdText,
                                            PF_INET,
                                            NULL,
                                            (LPSOCKADDR)&sockaddr,
                                            &sockaddrlen);
                if (Status != ERROR_SUCCESS)
                {
#ifdef UNICODE
                    Free(IdText);
#endif
                    
                    return(Status);
                }

                sockaddr4 = (struct sockaddr_in *)&sockaddr;
                Addr = sockaddr4->sin_addr.S_un.S_addr;
                Id = (PCHAR)&Addr;
                IdLen = 4;
                break;
            }

            case ID_IPV6_ADDR:
            {
                sockaddrlen = sizeof(sockaddrlen);
                Status = WSAStringToAddressA(IdText,
                                            PF_INET6,
                                            NULL,
                                            (LPSOCKADDR)&sockaddr,
                                            &sockaddrlen);
                if (Status != ERROR_SUCCESS)
                {
#ifdef UNICODE
                    Free(IdText);
#endif
                    
                    return(Status);
                }

                sockaddr6 = (struct sockaddr_in6 *)&sockaddr;
                Id = (PCHAR)&sockaddr6->sin6_addr;
                IdLen = 16;
                break;
            }

            case ID_FQDN:
            case ID_USER_FQDN:
            {
                Id = IdText;
                IdLen = (ULONG)strlen(IdText);
                break;
            }

            default:
            {           
                printf("Error, only id types ID_IPV4_ADDR (1), ID_FQDN (2), ID_USER_FQDN supported (3)\n");
#ifdef UNICODE
                Free(IdText);
#endif
                return(ERROR_SUCCESS);
            }

        }

        memset(&AuthInfo, 0, sizeof(IKE_AUTHENTICATION_INFORMATION));
        AuthInfo.AuthMethod = IKE_AUTHENTICATION_PRESHARED_KEY_METHOD;
        AuthInfo.PsKey.IdType = IdType;
        AuthInfo.PsKey.IdLengthInBytes = IdLen;
        AuthInfo.PsKey.Id = (PUCHAR)Id;
                               
        Status = GetIScsiIKEInfo(Initiator,
                                 InitiatorPort,
                                 NULL,
                                 &AuthInfo);

        if (Status == ERROR_SUCCESS)
        {
            PrintSecurityFlags("    ",
                               AuthInfo.PsKey.SecurityFlags);
        }
        
    }
    return(Status); 
}

ISDSC_STATUS PSKey(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
)
// iscsicli PSKey <Initiator Name> <initiator Port> <Security Flags>
//                <Id Type> <Id> <Key> <persist>
{
    ISDSC_STATUS Status;
    PTCHAR Initiator;
    ISCSI_SECURITY_FLAGS SecurityFlags;
    PCHAR Id;
    ULONG IdLen;
    IKE_IDENTIFICATION_PAYLOAD_TYPE IdType;
    PCHAR Key;
    ULONG KeyLength;
    ULONG Addr;
    PCHAR IdText;
    IKE_AUTHENTICATION_INFORMATION IKEAuthInfo;
    ULONG InitiatorPort;
    BOOLEAN Persist;
    struct sockaddr_storage sockaddr;
    int sockaddrlen;
    struct sockaddr_in6 *sockaddr6;
    struct sockaddr_in *sockaddr4;

    if (ArgC != 9)
    {
        Usage(30);
        return(ERROR_SUCCESS);
    }

    Initiator = ArgV[2];
    if (*Initiator == TEXT('*'))
    {
        Initiator = NULL;
    }
    if (*ArgV[3] == TEXT('*'))
    {
        InitiatorPort = ISCSI_ALL_INITIATOR_PORTS;
    } else {
        InitiatorPort = stoi(ArgV[3]);
    }
    
    SecurityFlags = stoi(ArgV[4]);
    IdType = (IKE_IDENTIFICATION_PAYLOAD_TYPE)stoi(ArgV[5]);

#ifdef UNICODE
    IdText = NULL;
    Status = DiscpUnicodeToAnsi(ArgV[6],
                                &IdText,
                                0); 
#else
    IdText = ArgV[6];
    Status = ERROR_SUCCESS;
#endif

    if (Status == ERROR_SUCCESS)
    {
        switch(IdType)
        {
            case ID_IPV4_ADDR:
            {
                sockaddrlen = sizeof(sockaddr);
                Status = WSAStringToAddressA(IdText,
                                            PF_INET,
                                            NULL,
                                            (LPSOCKADDR)&sockaddr,
                                            &sockaddrlen);
                if (Status != ERROR_SUCCESS)
                {
#ifdef UNICODE
                    Free(IdText);
#endif
                    
                    return(Status);
                }

                sockaddr4 = (struct sockaddr_in *)&sockaddr;
                Addr = sockaddr4->sin_addr.S_un.S_addr;
                Id = (PCHAR)&Addr;
                IdLen = 4;
                break;
            }

            case ID_IPV6_ADDR:
            {
                sockaddrlen = sizeof(sockaddrlen);
                Status = WSAStringToAddressA(IdText,
                                            PF_INET6,
                                            NULL,
                                            (LPSOCKADDR)&sockaddr,
                                            &sockaddrlen);
                if (Status != ERROR_SUCCESS)
                {
#ifdef UNICODE
                    Free(IdText);
#endif
                    
                    return(Status);
                }

                sockaddr6 = (struct sockaddr_in6 *)&sockaddr;
                Id = (PCHAR)&sockaddr6->sin6_addr;
                IdLen = 16;
                break;
            }

            case ID_FQDN:
            case ID_USER_FQDN:
            {
                Id = IdText;
                IdLen = (ULONG)strlen(IdText);
                break;
            }

            default:
            {           
                printf("Error, only id types ID_IPV4_ADDR (1), ID_FQDN (2), ID_USER_FQDN supported (3)\n");
#ifdef UNICODE
                Free(IdText);
#endif
                return(ERROR_SUCCESS);
            }

        }

        if (*ArgV[7] == TEXT('*'))
        {
            Key = NULL;
            KeyLength = 0;
        } else {
#ifdef UNICODE
            Key = NULL;
            Status = DiscpUnicodeToAnsi(ArgV[7],
                                        &Key,
                                        0); 
#else
            Key = ArgV[7];
#endif
            KeyLength = (ULONG)strlen(Key);
        }

        if (Status == ERROR_SUCCESS)
        {
            Persist = IsTrue(ArgV[8], TRUE);

            IKEAuthInfo.AuthMethod = IKE_AUTHENTICATION_PRESHARED_KEY_METHOD;
            IKEAuthInfo.PsKey.SecurityFlags = SecurityFlags;
            IKEAuthInfo.PsKey.IdType = IdType;
            IKEAuthInfo.PsKey.IdLengthInBytes = IdLen;
            IKEAuthInfo.PsKey.Id = (PUCHAR)Id;
            IKEAuthInfo.PsKey.KeyLengthInBytes = KeyLength;
            IKEAuthInfo.PsKey.Key = (PUCHAR)Key;

            Status = SetIScsiIKEInfo(Initiator,
                                     InitiatorPort,
                                     &IKEAuthInfo,
                                     Persist);
#ifdef UNICODE
            if (Key != NULL)
            {
                Free(Key);
            }
#endif
        }

#ifdef UNICODE
        Free(IdText);
#endif
    }
    
    return(Status);
}



ISDSC_STATUS ReportLUNs(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ReportLUNs <SessionId>
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    PUCHAR Response;
    UCHAR Sense[18];
    ULONG SenseSize, ResponseSize;
    UCHAR ScsiStatus = 0;

    if (ArgC != 3)
    {
        Usage(18);
        Status = ERROR_SUCCESS;     
    } else {
        if (StringToSessionId(ArgV[2], &SessionId))
        {            
            SenseSize = 18;
            Response = NULL;
            ResponseSize = 0;
            Status = SendScsiReportLuns(&SessionId,
                                          &ScsiStatus,
                                          &ResponseSize,
                                          Response,
                                          &SenseSize,
                                          Sense);
            if (Status == ERROR_INSUFFICIENT_BUFFER)
            {
                Response = Alloc(ResponseSize);
                if (Response != NULL)
                {
                    Status = SendScsiReportLuns(&SessionId,
                                                  &ScsiStatus,
                                                  &ResponseSize,
                                                  Response,
                                                  &SenseSize,
                                                  Sense);
                    if (Status == ERROR_SUCCESS)
                    {
                        printf("    ScsiStatus             : 0x%x\n"
                               "    Response Buffer Size   : 0x%x\n",
                               ScsiStatus,
                               ResponseSize);

                        PrintBuffer("        ",Response, ResponseSize);
                    }
                    Free(Response);
                }
            }

            if (Status ==  ISDSC_SCSI_REQUEST_FAILED)
            {
                printf("    ScsiStatus             : 0x%x\n"
                       "    Sense Buffer Size      : 0x%x\n",
                       ScsiStatus,
                       SenseSize);
                PrintBuffer("    ", Sense, SenseSize);                      
            }           
        } else {
#ifdef UNICODE
            printf("Invalid sessionid: %ws\n", ArgV[2]);
#else
            printf("Invalid sessionid: %s\n", ArgV[2]);
#endif
            Status = ERROR_INVALID_PARAMETER;
        }
        
    }
    
    return(Status);
}


ISDSC_STATUS ReadCapacity(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ReadCapacity <SessionId> <LUN>
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ULONGLONG LUN;
    PUCHAR  Response;
    UCHAR Sense[18];
    ULONG SenseSize, ResponseSize;
    UCHAR ScsiStatus = 0;

    if (ArgC != 4)
    {
        Usage(17);
        Status = ERROR_SUCCESS;     
    } else {
        if (StringToSessionId(ArgV[2], &SessionId))
        {
            
            if (HexStringToULONGLONG(ArgV[3], &LUN, sizeof(ULONGLONG)*2, 0))
            {
                SenseSize = 18;
                Response = NULL;
                ResponseSize = 0;
                Status = SendScsiReadCapacity(&SessionId,
                                              LUN,
                                              &ScsiStatus,
                                              &ResponseSize,
                                              Response,
                                              &SenseSize,
                                              Sense);
                if (Status == ERROR_INSUFFICIENT_BUFFER)
                {
                    Response = Alloc(ResponseSize);
                    if (Response != NULL)
                    {
                        Status = SendScsiReadCapacity(&SessionId,
                                                      LUN,
                                                      &ScsiStatus,
                                                      &ResponseSize,
                                                      Response,
                                                      &SenseSize,
                                                      Sense);
                        if (Status == ERROR_SUCCESS)
                        {
                            printf("    ScsiStatus             : 0x%x\n"
                                   "    Response Buffer Size   : 0x%x\n",
                                   ScsiStatus,
                                   ResponseSize);

                            PrintBuffer("        ",Response, ResponseSize);
                        }
                        Free(Response);
                    }
                }
                
                if (Status ==  ISDSC_SCSI_REQUEST_FAILED)
                {
                    printf("    ScsiStatus             : 0x%x\n"
                           "    Sense Buffer Size      : 0x%x\n",
                           ScsiStatus,
                           SenseSize);
                    PrintBuffer("    ", Sense, SenseSize);                      
                }           
            } else {
#ifdef UNICODE
                printf("Invalid LUN: %ws\n", ArgV[3]);
#else
                printf("Invalid LUN: %s\n", ArgV[3]);
#endif
                Status = ERROR_INVALID_PARAMETER;
            }
        } else {
#ifdef UNICODE
            printf("Invalid sessionid: %ws\n", ArgV[2]);
#else
            printf("Invalid sessionid: %s\n", ArgV[2]);
#endif
            Status = ERROR_INVALID_PARAMETER;
        }
        
    }
    
    return(Status);
}

ISDSC_STATUS DoScsiInquiry(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ScsiInquiry <SessionId> <LUN> <EvpdCmddt> <PageCode>
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ULONGLONG LUN;
    UCHAR EvpdCmdt, PageCode;
    PUCHAR Response;
    UCHAR Sense[18];
    ULONG SenseSize, ResponseSize;
    UCHAR ScsiStatus;

    if (ArgC != 6)
    {
        Usage(16);
        Status = ERROR_SUCCESS;     
    } else {
        if (StringToSessionId(ArgV[2], &SessionId))
        {
            
            if (HexStringToULONGLONG(ArgV[3], &LUN, sizeof(ULONGLONG)*2, 0))
            {
                EvpdCmdt = (UCHAR)stoi(ArgV[4]);
                PageCode = (UCHAR)stoi(ArgV[5]);
                
                SenseSize = 18;
                Response = NULL;
                ResponseSize = 0;
                Status = SendScsiInquiry(&SessionId,
                                              LUN,
                                              EvpdCmdt,
                                              PageCode,
                                              &ScsiStatus,
                                              &ResponseSize,
                                              Response,
                                              &SenseSize,
                                              Sense);
                if (Status == ERROR_INSUFFICIENT_BUFFER)
                {
                    Response = Alloc(ResponseSize);
                    if (Response != NULL)
                    {
                        Status = SendScsiInquiry(&SessionId,
                                                      LUN,
                                                      EvpdCmdt,
                                                      PageCode,
                                                      &ScsiStatus,
                                                      &ResponseSize,
                                                      Response,
                                                      &SenseSize,
                                                      Sense);
                        if (Status == ERROR_SUCCESS)
                        {
                            printf("    ScsiStatus             : 0x%x\n"
                                   "    Response Buffer Size   : 0x%x\n",
                                   ScsiStatus,
                                   ResponseSize);

                            PrintBuffer("        ",Response, ResponseSize);
                        }
                        Free(Response);
                    }
                }
                
                if (Status ==  ISDSC_SCSI_REQUEST_FAILED)
                {
                    printf("    ScsiStatus             : 0x%x\n"
                           "    Sense Buffer Size      : 0x%x\n",
                           ScsiStatus,
                           SenseSize);
                    PrintBuffer("    ", Sense, SenseSize);                      
                }           
            } else {
#ifdef UNICODE
                printf("Invalid LUN: %ws\n", ArgV[3]);
#else
                printf("Invalid LUN: %s\n", ArgV[3]);
#endif
                Status = ERROR_INVALID_PARAMETER;
            }
        } else {
#ifdef UNICODE
            printf("Invalid sessionid: %ws\n", ArgV[2]);
#else
            printf("Invalid sessionid: %s\n", ArgV[2]);
#endif
            Status = ERROR_INVALID_PARAMETER;
        }
        
    }
    
    return(Status);
}


ISDSC_STATUS DoLogoutTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli LogoutTarget <SessionId>
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;

    if (ArgC != 3)
    {
        Usage(10);
        Status = ERROR_SUCCESS;
    } else {
        if (StringToSessionId(ArgV[2], &SessionId))
        {
            printf("Logout Target 0x%I64x-0x%I64x\n",
                   SessionId.AdapterUnique,
                   SessionId.AdapterSpecific);

            Status = LogoutIScsiTarget(&SessionId);
        } else {
#ifdef UNICODE
            printf("Invalid sessionid: %ws\n", ArgV[2]);
#else
            printf("Invalid sessionid: %s\n", ArgV[2]);
#endif
            Status = ERROR_INVALID_PARAMETER;
        }
    }

    return(Status);
}


ISDSC_STATUS DoLoginToTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV,
    BOOLEAN IsPersistent                             
    )
{
    ISDSC_STATUS Status;
    BOOLEAN ReportToPNP;
    PTCHAR TargetName;
    PTCHAR TargetPortalAddress;
    USHORT TargetPortalSocket;
    PTCHAR InitiatorInstance;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ISCSI_UNIQUE_SESSION_ID ConnectionId;
    ISCSI_TARGET_PORTAL TargetPortal = { 0 };
    ISCSI_LOGIN_OPTIONS LoginOptions;
    ISCSI_SECURITY_FLAGS SecurityFlags;
    ULONG MappingCount;
    PISCSI_TARGET_MAPPING Mapping;
    ULONG i, ArgCIndex;
    ULONG PortNumber;
    int ArgCExpected;
    PCHAR Key;
    ULONG KeyLength;
    ULONG SizeNeeded;
    ULONG x;
    BOOLEAN b;
    HRESULT hr;

    if (ArgC < 20)
    {
        Usage(IsPersistent ? 11 : 9);
        Status = ERROR_SUCCESS;
    } else {
        TargetName = ArgV[2];
        ReportToPNP = TRUE;
        TargetPortalAddress = NULL;
        TargetPortalSocket = 0;
        InitiatorInstance = NULL;
        
        ReportToPNP = IsTrue(ArgV[3], TRUE);
        
        if (*ArgV[4] != TEXT('*'))
        {
            TargetPortalAddress = ArgV[4];
            if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
            {
                return(ERROR_INVALID_PARAMETER);
            }
        }       

        if ((TargetPortalAddress != NULL) && (*ArgV[5] != TEXT('*')))
        {
            TargetPortalSocket = (USHORT)stoi(ArgV[5]);
            hr = StringCchCopy(TargetPortal.Address,
                               MAX_ISCSI_PORTAL_ADDRESS_LEN,
                               TargetPortalAddress);
                     
            TargetPortal.Socket = TargetPortalSocket;
            *TargetPortal.SymbolicName = 0;
        } else if (! ((TargetPortalAddress ==  NULL) &&
                      (*ArgV[5] == TEXT('*')))) {
            printf("Portal address and socket must be specified\n");
            return(ERROR_INVALID_PARAMETER);
        }

        if (*ArgV[6] != TEXT('*'))
        {
            InitiatorInstance = ArgV[6];
        }

        if (*ArgV[7] != TEXT('*'))
        {
            PortNumber = stoi(ArgV[7]);
        } else {
            PortNumber = ISCSI_ANY_INITIATOR_PORT;
        }

        SecurityFlags = stoi(ArgV[8]);

        ParseLoginOptions(&LoginOptions,
                          ArgV,
                          ArgC,
                          9);

        if (*ArgV[18] == TEXT('*'))
        {
            Key = NULL;
            KeyLength = 0;
        } else {
#ifdef UNICODE          
            Key = NULL;
            DiscpUnicodeToAnsi(ArgV[18],
                               &Key,
                               0);
#else
            Key = ArgV[18];
#endif
            KeyLength = (ULONG)strlen(Key) + 1;
        }
        
        MappingCount = stoi(ArgV[19]);
        ArgCExpected = 20 + (MappingCount * 4);
        if (ArgC != ArgCExpected)
        {
            Usage(IsPersistent ? 11 : 9);
            return(ERROR_SUCCESS);
        }

        if (MappingCount != 0)
        {
            SizeNeeded = sizeof(ISCSI_TARGET_MAPPING) +
                         MappingCount * sizeof(SCSI_LUN_LIST);
            
            Mapping = (PISCSI_TARGET_MAPPING)Alloc(SizeNeeded) ;
            if (Mapping == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            ArgCIndex = 20;
            *Mapping->InitiatorName = 0;
            *Mapping->TargetName = 0;
            *Mapping->OSDeviceName = 0;
            Mapping->OSBusNumber = stoi(ArgV[ArgCIndex+1]);
            Mapping->OSTargetNumber = stoi(ArgV[ArgCIndex+2]);
            
            Mapping->LUNCount = MappingCount;
            Mapping->LUNList = (PSCSI_LUN_LIST)OffsetToPtr(Mapping,
                                               sizeof(ISCSI_TARGET_MAPPING));
            
            for (i = 0; i < MappingCount; i++)
            {
                b = stoiDForLogicalUnit(ArgV[ArgCIndex],
                                       &Mapping->LUNList[i].TargetLUN);

                if (b == FALSE)
                {
                    printf("Target LUN must be in 0x0123456789abcdef format\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                
                ArgCIndex++;        // bus

                x = stoi(ArgV[ArgCIndex]);
                if (x != Mapping->OSBusNumber)
                {
                    printf("OSBus number must be the same for all LUNs\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                
                ArgCIndex++;        // target
                x = stoi(ArgV[ArgCIndex]);
                if (x != Mapping->OSTargetNumber)
                {
                    printf("OSTarget number must be the same for all LUNs\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                
                ArgCIndex++;                
                Mapping->LUNList[i].OSLUN = stoi(ArgV[ArgCIndex]);
                ArgCIndex++;                
            }
        } else {
            Mapping = NULL;
        }
        

#ifdef UNICODE      
        printf("LoginTarget to %ws on %ws to %ws/%d\n",
               TargetName,
               InitiatorInstance ? InitiatorInstance : L"<no init instance>",
               TargetPortalAddress ? TargetPortal.Address : L"<no portal>",
               TargetPortalAddress ? TargetPortal.Socket : 0
              );
#else
        printf("LoginTarget to %s on %s to %s/%d\n",
               TargetName,
               InitiatorInstance ? InitiatorInstance : "<no init instance>",
               TargetPortalAddress ? TargetPortal.Address : "<no portal>",
               TargetPortalAddress ? TargetPortal.Socket : 0
              );
#endif      

        Status = LoginIScsiTarget(TargetName,
                              ReportToPNP ? FALSE : TRUE,
                              InitiatorInstance,
                              PortNumber,                   // PortNUmber
                              TargetPortalAddress ? &TargetPortal : NULL,
                              SecurityFlags,                   
                              Mapping,                // Mappings
                              &LoginOptions,                // LoginOptions
                              KeyLength,
                              Key,
                               IsPersistent,
                              &SessionId,
                                 &ConnectionId);

        if (! IsPersistent)
        {
            if (Status == ERROR_SUCCESS)
            {
                printf("Session Id is 0x%I64x-0x%I64x\n",
                       SessionId.AdapterUnique,
                       SessionId.AdapterSpecific);
                printf("Connection Id is 0x%I64x-0x%I64x\n",
                       ConnectionId.AdapterUnique,
                       ConnectionId.AdapterSpecific);
            }
        }
        
        if (Mapping != NULL)
        {
            Free(Mapping);
        }
    }

    return(Status);
}

ISDSC_STATUS TryLoginToTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli LoginTarget <TargetName> <ReportToPNP>
//                       <TargetPortalAddress> <TargetPortalSocket>
//                       <InitiatorInstance> <Port number> <Security Flags>
//                     <Header Digest> <Data Digest> 
//                     <Max Connections> <DefaultTime2Wait>
//                     <DefaultTime2Retain> <Username> <Password>
//                     <AuthType> <Key>
//                     <Mapping Count> <Target Lun> <OS Bus> <Os Target> 
//                     <OS Lun> ...
//                       
{
    ISDSC_STATUS Status;
    Status = DoLoginToTarget(ArgC,
                             ArgV,
                             FALSE);
    return(Status);
}


ISDSC_STATUS PersistentLoginTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli PersistentLoginTarget <TargetName> <ReportToPNP>
//                       <TargetPortalAddress> <TargetPortalSocket>
//                       <InitiatorInstance> <Port number> <Security Flags>
//                     <Header Digest> <Data Digest> 
//                     <Max Connections> <DefaultTime2Wait>
//                     <DefaultTime2Retain> <Username> <Password>
//                     <AuthType> <Key>
//                     <Mapping Count> <Target Lun> <OS Bus> <Os Target> 
//                     <OS Lun> ...
//                       
{
    ISDSC_STATUS Status;
    Status = DoLoginToTarget(ArgC,
                             ArgV,
                             TRUE);
    return(Status);
}

ISDSC_STATUS RemovePersistentTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  printf("iscsicli RemovePersistentTarget <Initiator Name> <TargetName>\n"
//    printf("                                <Port Number> \n");
//  printf("                                <Target Portal Address> \n");
//  printf("                                <Target Portal Socket> \n");
{
    ISDSC_STATUS Status;
    PTCHAR I;
    ISCSI_TARGET_PORTAL TargetPortal;
    PISCSI_TARGET_PORTAL TargetPortalPtr;
    ULONG PortNumber;
    HRESULT hr;

    if (ArgC != 7)
    {
        Usage(13);
        Status = ERROR_SUCCESS;     
    } else {
        if (*ArgV[2] == TEXT('*'))
        {
            I = NULL;
        } else {
            I = ArgV[2];
        }

        if (*ArgV[4] == TEXT('*'))
        {
            PortNumber = ISCSI_ALL_INITIATOR_PORTS;
        } else {
            PortNumber = stoi(ArgV[4]);
        }

        if (*ArgV[5] == TEXT('*'))
        {
            //
            // Both empty and NULL portal have the same effect
            //
            TargetPortalPtr = NULL;
#if 0
            TargetPortalPtr = &TargetPortal;
            memset(TargetPortalPtr, 0, sizeof(ISCSI_TARGET_PORTAL));
#endif
        } else {
            if (_tcslen(ArgV[5]) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
            {
                return(ERROR_INVALID_PARAMETER);
            }
            
            hr = StringCchCopy(TargetPortal.Address,
                               MAX_ISCSI_PORTAL_ADDRESS_LEN,
                               ArgV[5]);
            
            TargetPortal.Socket = (USHORT)stoi(ArgV[6]);
            *TargetPortal.SymbolicName = 0;
            TargetPortalPtr = &TargetPortal;
        }

        
        Status = RemoveIScsiPersistentTarget(I,
                                             PortNumber,
                                             ArgV[3],
                                             TargetPortalPtr);
    }
    
    return(Status);
}


ISDSC_STATUS ListPersistentTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  printf("iscsicli ListPersistentTargets\n");
{
    ISDSC_STATUS Status;
    ULONG Count;
    PPERSISTENT_ISCSI_LOGIN_INFO LoginInfo, LoginInfoArray;
    ULONG i;
    ULONG SizeNeeded;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    Count = 0;
    SizeNeeded = 0;
    Status = ReportIScsiPersistentLogins(&Count,
                                          NULL,
                                        &SizeNeeded);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        printf("Total of %d peristent targets\n",
               Count
              );
        
        LoginInfoArray = Alloc(SizeNeeded);
        if (LoginInfoArray != NULL)
        {
            Status = ReportIScsiPersistentLogins(&Count,
                                                 LoginInfoArray,
                                                &SizeNeeded);
            if (Status == ERROR_SUCCESS)
            {
                for (i = 0; i < Count; i++)
                {
                    LoginInfo = &LoginInfoArray[i];
#ifdef UNICODE
                    printf("    Target Name           : %ws\n"
                           "    Address and Socket    : %ws %d\n"
                           "    Session Type          : %s\n"
                           "    Initiator Name        : %ws\n",                        
#else
                    printf("    Target Name           : %s\n"
                           "    Address and Socket    : %s %d\n" 
                           "    Session Type          : %s\n"
                           "    Initiator Name        : %s\n",
#endif
                           LoginInfo->TargetName,
                           *(LoginInfo->TargetPortal.Address) == 0 ? TEXT("*") : LoginInfo->TargetPortal.Address,
                           LoginInfo->TargetPortal.Socket,
                           LoginInfo->IsInformationalSession ? "Informational" : "Data",
                           LoginInfo->InitiatorInstance
                          );

                    if (LoginInfo->InitiatorPortNumber == ISCSI_ANY_INITIATOR_PORT)
                    {
                        printf("    Port Number           : <Any Port>\n");
                    } else {
                        printf("    Port Number           : %d\n",
                               LoginInfo->InitiatorPortNumber
                              );
                    }

                    PrintSecurityFlags("    ", LoginInfo->SecurityFlags);
                            
                    PrintLoginOptions("    ",
                                      &LoginInfo->LoginOptions);
                    if (LoginInfo->Mappings != NULL)
                    {
                        PrintTargetMapping(LoginInfo->Mappings);
                    }
                                        
                    printf("\n");
                }
            }
            Free(LoginInfoArray);
            
        }
    }   

    return(Status); 
}


ISDSC_STATUS GetTargetInfo(
    IN __in PTCHAR TargetName,
    IN __in_opt PTCHAR DiscoveryMechanism,
    IN TARGET_INFORMATION_CLASS InfoClass,
    IN OUT PULONG BufferSize,
    OUT PVOID *Buffer
   )
{
    ISDSC_STATUS Status;
    PVOID b;
    ULONG SizeNeeded;

    SizeNeeded = 0;
    
    Status = GetIScsiTargetInformation(TargetName,
                                  DiscoveryMechanism,
                                  InfoClass,
                                  &SizeNeeded,
                                  NULL);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        b = Alloc(SizeNeeded);
        if (b == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Status = GetIScsiTargetInformation(TargetName,
                                      DiscoveryMechanism,
                                      InfoClass,
                                      &SizeNeeded,
                                      b);

        if (Status == ERROR_SUCCESS)
        {
            *Buffer = b;
            *BufferSize = SizeNeeded;
        } else {
            Free(b);
        }
    }
    return(Status);
}

void PrintPortalGroups(
    ULONG Count,
    PISCSI_TARGET_PORTAL_GROUP PortalGroups,
    ULONG Size
    )
{
    PISCSI_TARGET_PORTAL_GROUP p;
    ULONG SizeUsed, i, j;
    
    p = PortalGroups;
    for (i = 0; i < Count; i++)
    {
        printf("        Group %d has %d portals\n",
               i,
               p->Count
              );

        if (p->Count == 0)
        {
            SizeUsed = (FIELD_OFFSET(ISCSI_TARGET_PORTAL_GROUP,
                                    Portals) +3) & ~3;
        } else {
            SizeUsed = ((sizeof(ISCSI_TARGET_PORTAL_GROUP) +
                       (p->Count-1) * sizeof(ISCSI_TARGET_PORTAL))+3) & ~3;
        }

        if (SizeUsed <= Size)
        {
            for (j = 0; j < p->Count; j++)
            {
#ifdef UNICODE
                printf("            Address and Socket    : %ws %d\n"
                       "            Symbolic Name         : %ws\n",
#else
                printf("            Address and Socket    : %s %d\n"
                       "            Symbolic Name         : %s\n",
#endif
                       p->Portals[j].Address,
                       p->Portals[j].Socket,
                       p->Portals[j].SymbolicName
                      );
            }
            Size -= SizeUsed;
            p = (PISCSI_TARGET_PORTAL_GROUP)((PUCHAR)p + SizeUsed);
        } else {
            break;
        }
    }
}

ISDSC_STATUS TargetInfo(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli TargetInfo  <TargetName> [DiscoveryMechanism]
{
    PTCHAR DiscoveryMechanism;
    ULONG Count;
    ISDSC_STATUS Status;
    ULONG Size;
    PVOID Buffer;
    TCHAR m[MAX_PATH];
    
    if ((ArgC != 3) && (ArgC != 4))
    {
        Usage(8);
        return(ERROR_SUCCESS);
    } else {
        if (ArgC == 3)
        {
            DiscoveryMechanism = NULL;
        } else if (ArgV[3][0] == TEXT('*')) {
            DiscoveryMechanism = NULL;
        } else {
            DiscoveryMechanism = ArgV[3];
        }
    }

#ifdef UNICODE
    printf("Get Target information for %ws discovered by %ws\n",
#else
    printf("Get Target information for %s discovered by %s\n",
#endif
           ArgV[2],
           DiscoveryMechanism != NULL ? DiscoveryMechanism : TEXT("<all mechanisms>"));

    Status = GetTargetInfo(ArgV[2],
                           DiscoveryMechanism,                        
                           DiscoveryMechanisms,
                           &Size,
                           &Buffer);

    if (Status == ERROR_SUCCESS)
    {
        if (Size > 0)
        {           
            PrintStringList("    Discovery Mechanisms :",
                            "         ",
                            Buffer,
                            Size);
        } else {
            printf("    Discovery Mechanisms: <This List is Empty>\n");
        }
        Free(Buffer);
    }

    if (DiscoveryMechanism != NULL)
    {
        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               ProtocolType,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            PCHAR ProtocolList[] = { "iSCSI TCP Protocol", "Unknown" };
            TARGETPROTOCOLTYPE ProtocolType;

            ProtocolType = *((PTARGETPROTOCOLTYPE)Buffer);

            if (ProtocolType != ISCSI_TCP_PROTOCOL_TYPE)
            {
                ProtocolType = 1;
            }
            
            printf("    Protocol Type        : %s\n",
                   ProtocolList[ProtocolType]);
            
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Protocol Type Failed : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Protocol Type Failed : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }

        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               TargetAlias,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
#ifdef UNICODE      
            printf("    Target Alias         : %ws\n",
#else
            printf("    Target Alias         : %s\n",
#endif
                   (PTCHAR)Buffer);
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Target Alias Failed  : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Target Alias Failed  : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }    

        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               PortalGroups,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            if (Size > 0)
            {
                PISCSI_TARGET_PORTAL_GROUP p;

                Count = *((PULONG)Buffer);
                p = (PISCSI_TARGET_PORTAL_GROUP)((PUCHAR)Buffer + sizeof(ULONG));
                   printf("    PortalGroups         : %d portal groups\n",
                          Count
                         );
                   PrintPortalGroups(
                                  Count,
                                  p,
                                  Size);
            } else {
                   printf("    PortalGroups         : <This List Is Empty>\n");
            }
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Portal Groups Failed : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Portal Groups Failed : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }
        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               InitiatorName,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            if (Size > 0)
            {           
#ifdef UNICODE
                printf("    Initiator Name       : %ws\n",
                       (PWSTR)Buffer);
#else
                printf("    Initiator Name       : %s\n",
                       (PSTR)Buffer);
#endif
            } else {
                printf("    Initiator Name       : <Empty>\n");
            }
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Initiator List Failed: %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Initiator List Failed: %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }
        
        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               TargetFlags,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            if (Size == sizeof(ISCSI_TARGET_FLAGS))
            {
                ISCSI_TARGET_FLAGS TargetFlags;

                TargetFlags = *((PISCSI_TARGET_FLAGS)Buffer);
                
                printf("    Target Flags         : 0x%x\n",
                       TargetFlags);
                
                if (TargetFlags & ISCSI_TARGET_FLAG_HIDE_STATIC_TARGET)
                {
                    printf("                          Target is hidden until dynamically discovered\n");
                }
            }
            
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Target Flags Failed  : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Target Flags Failed  : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }

        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               LoginOptions,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            if (Size >= sizeof(ISCSI_LOGIN_OPTIONS))
            {
                PISCSI_LOGIN_OPTIONS LoginOptions;

                LoginOptions = (PISCSI_LOGIN_OPTIONS)Buffer;

                PrintLoginOptions("    ",
                                  LoginOptions);
            }
            
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Login Options Failed : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Login Options Failed : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }


        Status = GetTargetInfo(ArgV[2],
                               DiscoveryMechanism,                        
                               PersistentTargetMappings,
                               &Size,
                               &Buffer);

        if (Status == ERROR_SUCCESS)
        {
            if (Size >= sizeof(ISCSI_TARGET_MAPPING))
            {
                PISCSI_TARGET_MAPPING Mapping;

                Mapping = (PISCSI_TARGET_MAPPING)Buffer;

                PrintTargetMapping(
                                  Mapping);
            }
            
            Free(Buffer);
        } else {
#ifdef UNICODE
            printf("    Target Mappings Failed : %ws\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#else
            printf("    Target Mappings Failed : %s\n",
                   GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
        }

        
        Status = ERROR_SUCCESS;
    }
    printf("\n");
    
    return(Status);
}

ISDSC_STATUS DoAddConnection(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli AddConnection <SessionId> <initiator instance>
//                         <Port Number> <Target Portal Address>
//                         <Target Portal Socket> <Security Flags>
//                         <Header Digest> <Data Digest>
//                         <Max Connections> <DefaultTime2Wait>
//                         <DefaultTime2Retain> <Username> <Password>
//                         <AuthType> <Key>
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR TargetPortalAddress;
    ULONG PortNumber;
    USHORT TargetPortalSocket;
    ISCSI_LOGIN_OPTIONS LoginOptions;
    ISCSI_SECURITY_FLAGS SecurityFlags;
    PCHAR Key;
    ULONG KeyLength = 0;
    ISCSI_UNIQUE_CONNECTION_ID ConnectionId;
    PTCHAR InitiatorName;
    HRESULT hr;

    if (ArgC != 18)
    {
        Usage(14);
        return(ERROR_SUCCESS);
    }

    if (! StringToSessionId(ArgV[2], &SessionId))
    {
        Usage(14);
        return(ERROR_SUCCESS);
    }

    if (*ArgV[3] != TEXT('*'))
    {
        InitiatorName = ArgV[3];
    } else {
        InitiatorName = NULL;
    }

    if (*ArgV[4] == TEXT('*'))
    {
        PortNumber = ISCSI_ANY_INITIATOR_PORT;
    } else {
        PortNumber = stoi(ArgV[4]);
    }

    if (*ArgV[5] != TEXT('*'))
    {
        TargetPortalAddress = ArgV[5];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }
    } else {
        TargetPortalAddress = NULL;
    }

    if ((TargetPortalAddress != NULL) && ((*ArgV[6] != TEXT('*'))))
    {
        TargetPortalSocket = (USHORT)stoi(ArgV[6]);
        hr = StringCchCopy(TargetPortal.Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);
        
        TargetPortal.Socket = TargetPortalSocket;
        *TargetPortal.SymbolicName = 0;
    } else {
        TargetPortalAddress = NULL;
    }

    SecurityFlags = stoi(ArgV[7]);
    
    ParseLoginOptions(&LoginOptions,
                      ArgV,
                      ArgC,
                      8);
    if (*ArgV[17] == TEXT('*'))
    {
        Key = NULL;
        KeyLength = 0;
        Status = ERROR_SUCCESS;
    } else {
#ifdef UNICODE          
        Key = NULL;
        Status = DiscpUnicodeToAnsi(ArgV[17],
                           &Key,
                           0);
#else
        Key = ArgV[17];
        Status = ERROR_SUCCESS;
#endif
        if (Status == ERROR_SUCCESS)
        {
            KeyLength = (ULONG)strlen(Key) + 1;
        }
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = AddIScsiConnection(&SessionId,
                               InitiatorName,
                               PortNumber,
                               TargetPortalAddress ? &TargetPortal : NULL,
                               SecurityFlags,
                               &LoginOptions,
                               KeyLength,
                                Key,
                               &ConnectionId);
#ifdef UNICODE
        if (Key != NULL)
        {
            Free(Key);
        }   
#endif
    }   

    return(Status);
}    

ISDSC_STATUS DoRemoveConnection(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//    printf("iscsicli RemoveConnection <SessionId> <ConnectionId> \n");
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ISCSI_UNIQUE_CONNECTION_ID ConnectionId;

    if (ArgC != 4)
    {
        Usage(15);
        return(ERROR_SUCCESS);
    }

    if (! StringToSessionId(ArgV[2], &SessionId))
    {
        Usage(15);
        return(ERROR_SUCCESS);
    }

    if (! StringToSessionId(ArgV[3], &ConnectionId))
    {
        Usage(15);
        return(ERROR_SUCCESS);
    }

    Status = RemoveIScsiConnection(&SessionId,
                                   &ConnectionId);

    return(Status);
}    

ISDSC_STATUS DoReportInitiatorList(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ListInitiators 
{
    ISDSC_STATUS Status;
    PTCHAR Buffer,b = NULL;
    ULONG BufferSize;
    UNREFERENCED_PARAMETER(ArgV);    
    
    if (ArgC != 2)
    {
        Usage(20);
        Status = ERROR_SUCCESS;
    } else {
        BufferSize = 0;
        Buffer = NULL;
        Status = ReportIScsiInitiatorList(
                                   &BufferSize,
                                   NULL);

        if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            Buffer = Alloc(BufferSize * sizeof(TCHAR));
            if (Buffer == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            b = Buffer;
            Status = ReportIScsiInitiatorList(
                                       &BufferSize,
                                       Buffer);
            
        }

        if (Status == ERROR_SUCCESS)
        {
            printf("Initiators List:\n");
            while (*Buffer != 0)
            {
#ifdef UNICODE
                printf("    %ws\n", Buffer);
#else
                printf("    %s\n", Buffer);
#endif
                while (*Buffer != 0)
                {
                    Buffer++;
                }
                Buffer++;
            }
        }
        
        if (b != NULL)
        {
            Free(b);
        }
    }
    return(Status);
}

ISDSC_STATUS DoReportActiveIScsiTargetMappings(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ReportTargetMappings
{
    ISDSC_STATUS Status;
    ULONG MappingCount, BufferSize;
    PISCSI_TARGET_MAPPING Mapping, MappingX;
    ULONG i;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    BufferSize = 0;
    Status = ReportActiveIScsiTargetMappings(&BufferSize,
                                             &MappingCount,
                                             NULL);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        MappingX = (PISCSI_TARGET_MAPPING)Alloc(BufferSize);
        if (MappingX != NULL)
        {
            Status = ReportActiveIScsiTargetMappings(&BufferSize,
                                                     &MappingCount,
                                                     MappingX);
            if (Status == ERROR_SUCCESS)
            {
                printf("Total of %d mappings returned\n", MappingCount);
                Mapping = MappingX;
                for (i = 0; i < MappingCount; i++)
                {
                    PrintTargetMapping(Mapping);
                    Mapping++;
                }
            }
            Free(MappingX);
        }
    } else if (Status == ERROR_SUCCESS) {
        printf("No mappings\n");
    }

    return(Status);
}

ISDSC_STATUS ListTargets(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli ListTargets <ForceUpdate>
{
    ISDSC_STATUS Status;
    BOOLEAN ForceUpdate;
    PTCHAR Buffer,b = NULL;
    ULONG BufferSize;
    
    if ((ArgC != 3) && (ArgC != 2))
    {
        Usage(6);
        Status = ERROR_SUCCESS;
    } else {
        if (ArgC == 3)
        {
            ForceUpdate = IsTrue(ArgV[2], FALSE);
        } else {
            ForceUpdate = FALSE;
        }

        BufferSize = 0;
        Buffer = NULL;
        Status = ReportIScsiTargets(ForceUpdate,
                                   &BufferSize,
                                   NULL);

        if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            Buffer = Alloc(BufferSize * sizeof(TCHAR));
            if (Buffer == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            
            b = Buffer;
            Status = ReportIScsiTargets(ForceUpdate,
                                       &BufferSize,
                                       Buffer);
            
        }

        if (Status == ERROR_SUCCESS)
        {
            printf("Targets List:\n");
            while (*Buffer != 0)
            {
#ifdef UNICODE
                printf("    %ws\n", Buffer);
#else
                printf("    %s\n", Buffer);
#endif
                while (*Buffer != 0)
                {
                    Buffer++;
                }
                Buffer++;
            }
                    
        }
    }

    if (b != NULL)
    {
        Free(b);
    }
    
    return(Status);
}


ISDSC_STATUS RefreshTargetPortal(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli RefreshTargetPortal <TargetPortalAddress>
//                               <TargetPortalSocket> [HBAName]
//                               [PortNumber]
{
    ISDSC_STATUS Status;
    PTCHAR TargetPortalAddress;
    PTCHAR TargetPortalSocket;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR HBAName;
    ULONG PortNumber;
    HRESULT hr;

    if ((ArgC < 4) || (ArgC > 6))
    {
        Usage(5);
        Status = ERROR_SUCCESS;
    } else {
        TargetPortalAddress = ArgV[2];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }
        TargetPortalSocket = ArgV[3];

        if (ArgC > 4)
        {
            HBAName = ArgV[4];
            if (*HBAName == TEXT('*'))
            {
                HBAName = NULL;
            }

            if (ArgC > 5)
            {
                if (*ArgV[5] == TEXT('*'))
                {
                    PortNumber = ISCSI_ALL_INITIATOR_PORTS;
                } else {
                    PortNumber = stoi(ArgV[5]);
                }
            } else {
                PortNumber = ISCSI_ALL_INITIATOR_PORTS;
            }
        } else {
            HBAName = NULL;
            PortNumber = ISCSI_ALL_INITIATOR_PORTS;
        }

        hr = StringCchCopy(TargetPortal.Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);
        
        TargetPortal.Socket = (USHORT)stoi(TargetPortalSocket);

        Status = RefreshIScsiSendTargetPortal(HBAName,
                                              PortNumber,
                                               &TargetPortal);
    }

    return(Status);
}


ISDSC_STATUS ListTargetPortals(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//    iscsicli ListTargetPortals
{
    ISDSC_STATUS Status;
    ULONG Count;
    PISCSI_TARGET_PORTAL_INFO_EX PortalInfoArray, PortalInfo;
    ULONG i;
    ULONG SizeNeeded;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    Count = 0;
    Status = ReportIScsiSendTargetPortalsEx(&Count,
                                            &SizeNeeded,
                                            NULL);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        PortalInfoArray = Alloc(SizeNeeded);
        if (PortalInfoArray != NULL)
        {
            Status = ReportIScsiSendTargetPortalsEx(&Count,
                                                    &SizeNeeded,
                                                    PortalInfoArray);
            if (Status == ERROR_SUCCESS)
            {
                printf("Total of %d portals are persisted:\n\n", Count);
                for (i = 0; i < Count; i++)
                {
                    PortalInfo = &PortalInfoArray[i];
#ifdef UNICODE
                    printf("    Address and Socket   : %ws %d\n"
                           "    Symbolic Name        : %ws\n"
                           "    Initiator Name       : %ws\n",
#else
                    printf("    Address and Socket   : %s %d\n"
                           "    Symbolic Name        : %s\n"
                           "    Initiator Name       : %s\n",
#endif
                           PortalInfo->Address,
                           PortalInfo->Socket,
                           PortalInfo->SymbolicName,
                           PortalInfo->InitiatorName
                          );
                    if (PortalInfo->InitiatorPortNumber == ISCSI_ANY_INITIATOR_PORT)
                    {
                        printf("    Port Number          : <Any Port>\n");
                    } else {
                        printf("    Port Number          : %d\n",
                               PortalInfo->InitiatorPortNumber);
                    }

                    PrintSecurityFlags("    ",
                                       PortalInfo->SecurityFlags);
                    
                            
                    PrintLoginOptions("    ",
                                      &PortalInfo->LoginOptions);
                    printf("\n");
                }
            }
            Free(PortalInfoArray);            
        }
    }   

    return(Status); 
}


ISDSC_STATUS RemoveTargetPortal(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli RemoveTargetPortal <TargetPortalAddress> <TargetPortalSocket>
//                           [HBA Name] [PortNumber]
//
{
    ISDSC_STATUS Status;
    PTCHAR TargetPortalAddress;
    PTCHAR TargetPortalSocket;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR HBAName;
    ULONG PortNumber;
    HRESULT hr;

    if ((ArgC < 4) || (ArgC > 6))
    {
        Usage(4);
        Status = ERROR_SUCCESS;
    } else {
        TargetPortalAddress = ArgV[2];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }
        TargetPortalSocket = ArgV[3];

        if (ArgC > 4)
        {
            HBAName = ArgV[4];
            if (*HBAName == TEXT('*'))
            {
                HBAName = NULL;
            }

            if (ArgC > 5)
            {
                if (*ArgV[5] == TEXT('*'))
                {
                    PortNumber = ISCSI_ALL_INITIATOR_PORTS;
                } else {
                    PortNumber = stoi(ArgV[5]);
                }
            } else {
                PortNumber = ISCSI_ALL_INITIATOR_PORTS;
            }
        } else {
            HBAName = NULL;
            PortNumber = ISCSI_ALL_INITIATOR_PORTS;
        }

        hr = StringCchCopy(TargetPortal.Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);
        
        TargetPortal.Socket = (USHORT)stoi(TargetPortalSocket);

        Status = RemoveIScsiSendTargetPortal(HBAName,
                                             PortNumber,
                                             &TargetPortal);
    }

    return(Status);
}

ISDSC_STATUS AddTargetPortal(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli AddTargetPortal <TargetPortalAddress> <TargetPortalSocket>
//                           [HBA Name] [PortNumber]
//  printf("                         <Security Flags>\n");
//    printf("                         <Login Flags> <Header Digest> <Data Digest> \n");
//    printf("                         <Max Connections> <DefaultTime2Wait>\n");
//    printf("                         <DefaultTime2Retain> <Username>
//                                     <Password> <AuthType>\n");
{   
    ISDSC_STATUS Status;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR TargetPortalAddress, TargetPortalSocket, HBAName;
    ULONG PortNumber;
    PISCSI_LOGIN_OPTIONS LoginOptions = NULL;
    ISCSI_LOGIN_OPTIONS LO;
    ISCSI_SECURITY_FLAGS SecurityFlags = 0;
    HRESULT hr;

    if ((ArgC != 4) && (ArgC < 16))
    {
        Usage(3);
        Status = ERROR_SUCCESS;
    } else {
        TargetPortalAddress = ArgV[2];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }
        
        TargetPortalSocket = ArgV[3];

        if (ArgC > 4)
        {
            HBAName = ArgV[4];
            if (*HBAName == TEXT('*'))
            {
                HBAName = NULL;
            }

            if (ArgC > 5)
            {
                if (*ArgV[5] == TEXT('*'))
                {
                    PortNumber = ISCSI_ALL_INITIATOR_PORTS;
                } else {
                    PortNumber = stoi(ArgV[5]);
                }

                SecurityFlags = stoi(ArgV[6]);

                
                if (ArgC > 7)
                {
                    ParseLoginOptions(&LO,
                                      ArgV,
                                      ArgC,
                                      7);
                    LoginOptions = &LO;
                } else {
                    LoginOptions = NULL;
                }
            } else {
                PortNumber = ISCSI_ALL_INITIATOR_PORTS;
            }
        } else {
            HBAName = NULL;
            PortNumber = ISCSI_ALL_INITIATOR_PORTS;
        }
        
        hr = StringCchCopy(TargetPortal.Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);
        
        TargetPortal.Socket = (USHORT)stoi(TargetPortalSocket);
        *TargetPortal.SymbolicName = 0;

        Status = AddIScsiSendTargetPortal(HBAName,
                                          PortNumber,
                                          LoginOptions,
                                          SecurityFlags,
                                           &TargetPortal);

    }
    
    return(Status);
}

ISDSC_STATUS RemoveTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli RemoveTarget <TargetName>
{
    ISDSC_STATUS Status;
    PTCHAR TargetName;

    if (ArgC != 3)
    {
        Usage(2);
        Status = ERROR_SUCCESS;
        return(Status);
    } else {
        TargetName = ArgV[2];
    }

    Status = RemoveIScsiStaticTarget(TargetName);

    return(Status);
}

    
ISDSC_STATUS AddTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli AddTarget <TargetName> <TargetAlias> <TargetPortalAddress>
//                     <TargetPortalSocket> <Target flags>
//                     <Persist> <Header Digest> <Data Digest> 
//                     <Max Connections> <DefaultTime2Wait>
//                     <DefaultTime2Retain> <Username> <Password>
//                     <AuthType>
//                     <Mapping Count> <Target Lun> <OS Bus> <Os Target> 
//                     <OS Lun> ...
{
    ISDSC_STATUS Status;
    ISCSI_TARGET_PORTAL_GROUP PortalGroup;
    PTCHAR TargetName, TargetAlias, TargetPortalAddress;
    PTCHAR TargetPortalSocket;
    ISCSI_LOGIN_OPTIONS LoginOptions;
    BOOLEAN Persist;
    ULONG MappingCount;
    PISCSI_TARGET_MAPPING Mapping;
    int ArgCExpected, ArgCIndex;
    ISCSI_TARGET_FLAGS TargetFlags;
    ULONG i, SizeNeeded;
    ULONG x;
    BOOLEAN b;
    HRESULT hr;
            
    if (ArgC < 18)
    {
        Usage(1);
        Status = ERROR_SUCCESS;
    } else {
        TargetName = ArgV[2];
        if (*ArgV[3] == TEXT('*'))
        {
            TargetAlias = NULL;
        } else {
            TargetAlias = ArgV[3];
        }

        if ((*ArgV[4] != TEXT('*')) && (*ArgV[5] != TEXT('*')))
        {
            TargetPortalAddress = ArgV[4];
            if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
            {
                return(ERROR_INVALID_PARAMETER);
            }
            TargetPortalSocket = ArgV[5];

            PortalGroup.Count = 1;
            *PortalGroup.Portals[0].SymbolicName = 0;
            hr = StringCchCopy(PortalGroup.Portals[0].Address,
                               MAX_ISCSI_PORTAL_ADDRESS_LEN,
                               TargetPortalAddress);
            
            PortalGroup.Portals[0].Socket = (USHORT)stoi(TargetPortalSocket);
        } else {
            PortalGroup.Count = 0;
            *PortalGroup.Portals[0].Address = 0;
        }

        TargetFlags = stoi(ArgV[6]);

        Persist = IsTrue(ArgV[7], TRUE);

        ParseLoginOptions(&LoginOptions,
                          ArgV,
                          ArgC,
                          8);
                
        MappingCount = stoi(ArgV[17]);
        ArgCExpected = 18 + (MappingCount * 4);
        if (ArgC != ArgCExpected)
        {
            Usage(1);
            return(ERROR_SUCCESS);
        }

        if (MappingCount != 0)
        {
            SizeNeeded = sizeof(ISCSI_TARGET_MAPPING) +
                         MappingCount * sizeof(SCSI_LUN_LIST);
            
            Mapping = (PISCSI_TARGET_MAPPING)Alloc(SizeNeeded) ;
            if (Mapping == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            ArgCIndex = 18;
            *Mapping->InitiatorName = 0;
            *Mapping->TargetName = 0;
            *Mapping->OSDeviceName = 0;
            Mapping->OSBusNumber = stoi(ArgV[ArgCIndex+1]);
            Mapping->OSTargetNumber = stoi(ArgV[ArgCIndex+2]);
            Mapping->LUNCount = MappingCount;
            Mapping->LUNList = (PSCSI_LUN_LIST)OffsetToPtr(Mapping,
                                               sizeof(ISCSI_TARGET_MAPPING));
            
            for (i = 0; i < MappingCount; i++)
            {
                b = stoiDForLogicalUnit(ArgV[ArgCIndex],
                                       &Mapping->LUNList[i].TargetLUN);

                if (b == FALSE)
                {
                    printf("Target LUN must be in 0x0123456789abcdef format\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                ArgCIndex++;

                x = stoi(ArgV[ArgCIndex]);
                if (x != Mapping->OSBusNumber)
                {
                    printf("OSBus number must be the same for all LUNs\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                
                ArgCIndex++;        // target
                x = stoi(ArgV[ArgCIndex]);
                if (x != Mapping->OSTargetNumber)
                {
                    printf("OSTarget number must be the same for all LUNs\n");
                    return(ERROR_INVALID_PARAMETER);
                }
                
                ArgCIndex++;                
                Mapping->LUNList[i].OSLUN = stoi(ArgV[ArgCIndex]);
                ArgCIndex++;                
            }
        } else {
            Mapping = NULL;
        }
                
        Status = AddIScsiStaticTarget(TargetName,
                                       TargetAlias,
                                      TargetFlags,               // TargetFlags
                                       Persist,           // Persist
                                      Mapping,            // Mappings
                                      &LoginOptions,            // LoginOptions
                                      PortalGroup.Count == 0 ?
                                          NULL :
                                          &PortalGroup);
        if (Mapping != NULL)
        {
            Free(Mapping);
        }
    }
    return(Status);
}


ISDSC_STATUS AddiSNSServerX(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli AddiSNSServer <Server name>
{
    ISDSC_STATUS Status;
    PTCHAR ServerName;

    if (ArgC != 3)
    {
        Usage(21);
        return(ERROR_SUCCESS);
    }

    ServerName = ArgV[2];

    Status = AddISNSServer(ServerName);

    return(Status);
}


ISDSC_STATUS RemoveiSNSServerX(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli RemoveiSNSServer <Server name>
{
    ISDSC_STATUS Status;
    PTCHAR ServerName;

    if (ArgC != 3)
    {
        Usage(22);
        return(ERROR_SUCCESS);
    }

    ServerName = ArgV[2];

    Status = RemoveISNSServer(ServerName);
    return(Status);
}

ISDSC_STATUS RefreshiSNSServer(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  iscsicli RefreshiSNSServer <Server name>
{
    ISDSC_STATUS Status;
    PTCHAR ServerName;

    if (ArgC != 3)
    {
        Usage(23);
        return(ERROR_SUCCESS);
    }

    if (*ArgV[2] == TEXT('*'))
    {
        ServerName = NULL;
    } else {
        ServerName = ArgV[2];
    }

    Status = RefreshISNSServer(ServerName);
    return(Status);
}


ISDSC_STATUS ListiSNSServers(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//    printf("iscsicli ListiSNSServer\n");
{
    ISDSC_STATUS Status;
    ULONG SizeNeeded, Size, Len;
    PTCHAR Buffer, b;
    UNREFERENCED_PARAMETER(ArgC);
    UNREFERENCED_PARAMETER(ArgV);

    SizeNeeded = 0;
    Status = ReportISNSServerList(&SizeNeeded,
                                  NULL);

    if (Status == ERROR_INSUFFICIENT_BUFFER)
    {
        Buffer = (PTCHAR)Alloc(SizeNeeded * sizeof(TCHAR));
        if (Buffer != NULL)
        {
            Status = ReportISNSServerList(&SizeNeeded,
                                          Buffer);
            if (Status == ERROR_SUCCESS)
            {
                Len = 0;
                b = (PTCHAR)Buffer;
                while (Len < SizeNeeded)
                {
                    b = &Buffer[Len];
                    Size = (ULONG)(_tcslen(b) + 1);
#ifdef UNICODE
                    printf("    %ws\n", b);
#else
                    printf("    %s\n", b);;
#endif                  
                    Len += Size;
                }
            }
            Free(Buffer);
        }
    } else if (Status == ERROR_SUCCESS) {
        printf("No SNS Servers\n");
    }

    return(Status);
}

ISDSC_STATUS BuildLoginOptionsForCHAP(
    IN __in PTSTR CHAPUsername,
    IN __in PTSTR CHAPPassword,
    OUT PISCSI_LOGIN_OPTIONS *LoginOptionsPtr
    )
/*++

Routine Description:

    This routine will allocate a login options structure and build it
    to contain the CHAP username and password needed for one way CHAP

Arguments:

    CHAPUsername is the chap username to use

    CHAPPassword is the chap password to use

    *LoginOptionsPtr returns with the filled in login option structure

Return Value:

    ERROR_SUCCESS or error code

--*/
{
    ISDSC_STATUS Status;
    PISCSI_LOGIN_OPTIONS LoginOptions;
    PTCHAR Secret;
    
    LoginOptions = Alloc(sizeof(ISCSI_LOGIN_OPTIONS));
    if (LoginOptions != NULL)
    {
        memset(LoginOptions, 0, sizeof(ISCSI_LOGIN_OPTIONS));

        LoginOptions->Version = ISCSI_LOGIN_OPTIONS_VERSION;
        LoginOptions->InformationSpecified = ISCSI_LOGIN_OPTIONS_USERNAME |
                                             ISCSI_LOGIN_OPTIONS_PASSWORD |
                                             ISCSI_LOGIN_OPTIONS_AUTH_TYPE;
        LoginOptions->AuthType = ISCSI_CHAP_AUTH_TYPE;
        
        if ((*CHAPUsername == TEXT('-')) || (*CHAPUsername == TEXT('*')))
        {
            LoginOptions->Username = NULL;
            LoginOptions->UsernameLength = 0;
            Status = ERROR_SUCCESS;
        } else {
#ifdef UNICODE
            LoginOptions->Username = NULL;
            Status = DiscpUnicodeToAnsi(
                                        CHAPUsername,
                                        (LPSTR *)&LoginOptions->Username,
                                        0);
#else
            LoginOptions->Username = CHAPUsername;
            Status = ERROR_SUCCESS;
#endif
            if (Status == ERROR_SUCCESS)
            {
                LoginOptions->UsernameLength = (ULONG)strlen((LPCSTR)LoginOptions->Username);
            }
        }

        if (Status == ERROR_SUCCESS)
        {
            if ((*CHAPPassword == TEXT('-')) || *CHAPPassword == TEXT('*'))
            {
                LoginOptions->Password = NULL;
                LoginOptions->PasswordLength = 0;
                Status = ERROR_SUCCESS;
            } else {
                Secret = CHAPPassword;
                if ((Secret[0] == TEXT('0')) &&
                    ((Secret[1] == TEXT('X')) ||
                     (Secret[1] == TEXT('x'))))
                {
                    Status = ParseHexString(Secret+2,
                                            &LoginOptions->Password,
                                            &LoginOptions->PasswordLength);                 
                } else {
#ifdef UNICODE
                    LoginOptions->Password = NULL;
                    Status = DiscpUnicodeToAnsi(
                                       CHAPPassword,
                                       (LPSTR *)&LoginOptions->Password,
                                       0);
#else
                    LoginOptions->Password = CHAPPassword;
                    Status = ERROR_SUCCESS;
#endif
                    if (Status == ERROR_SUCCESS)
                    {
                        LoginOptions->PasswordLength = (ULONG)strlen((LPCSTR)LoginOptions->Password);
                    }
                }
            }

            if (Status == ERROR_SUCCESS)
            {
                *LoginOptionsPtr = LoginOptions;
            }
        }
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(Status);
}
ISDSC_STATUS QLoginTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  printf("iscsicli QLoginTarget <TargetName>  [CHAP Username] [CHAP Password]\n");
{
    ISDSC_STATUS Status;
    PTCHAR TargetName, CHAPUsername, CHAPPassword;
    PISCSI_LOGIN_OPTIONS LoginOptions;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ISCSI_UNIQUE_SESSION_ID ConnectionId;

    if ((ArgC != 3) && (ArgC != 5))
    {
        Usage(33);
        return(ERROR_SUCCESS);
    }

    TargetName = ArgV[2];

    if (ArgC == 5)
    {
        CHAPUsername = ArgV[3];
        CHAPPassword = ArgV[4];

        Status = BuildLoginOptionsForCHAP(CHAPUsername,
                                          CHAPPassword,
                                          &LoginOptions);
    } else {
        LoginOptions = NULL;
        Status = ERROR_SUCCESS;
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = LoginIScsiTarget(TargetName,
                                  FALSE,         // IsInformationalSession
                                  NULL,         // InitiatorInstance,
                                  ISCSI_ANY_INITIATOR_PORT,
                                  NULL,         // TargetPortal
                                  0,            // SecurityFlags,
                                  NULL,         // Mappings,
                                  LoginOptions,
                                  0,            // KeySize,
                                  NULL,         // Key
                                  FALSE,        // IsPersistent,
                                  &SessionId,
                                  &ConnectionId);
        
        if (Status == ERROR_SUCCESS)
        {
            printf("Session Id is 0x%I64x-0x%I64x\n",
                   SessionId.AdapterUnique,
                   SessionId.AdapterSpecific);
            printf("Connection Id is 0x%I64x-0x%I64x\n",
                   ConnectionId.AdapterUnique,
                   ConnectionId.AdapterSpecific);
        }
    }

    if (LoginOptions != NULL)
    {
#ifdef UNICODE
        if (LoginOptions->Username != NULL)
        {
            Free(LoginOptions->Username);
        }

        if (LoginOptions->Password != NULL)
        {
            Free(LoginOptions->Password);
        }
#endif
        Free(LoginOptions);
    }
    
    return(Status);
}

ISDSC_STATUS QAddTarget(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//  printf("iscsicli QAddTarget <TargetName> <TargetPortalAddress>\n");
{
    ISDSC_STATUS Status;
    ISCSI_TARGET_PORTAL_GROUP PortalGroup;
    PTCHAR TargetName, TargetPortalAddress;
    HRESULT hr;

    if (ArgC != 4)
    {
        Usage(34);
        return(ERROR_SUCCESS);
    }

    TargetName = ArgV[2];
    
    if (*ArgV[3] != TEXT('*')) 
    {
        TargetPortalAddress = ArgV[3];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }

        PortalGroup.Count = 1;
        *PortalGroup.Portals[0].SymbolicName = 0;
        hr = StringCchCopy(PortalGroup.Portals[0].Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);

        PortalGroup.Portals[0].Socket = 3260;
    } else {
        PortalGroup.Count = 0;
        *PortalGroup.Portals[0].Address = 0;
    }

    Status = AddIScsiStaticTarget(TargetName,
                                  NULL,             // TargetAlias,
                                  0,                // TargetFlags
                                  TRUE,             // Persist
                                  NULL,             // Mappings
                                  NULL,             // LoginOptions
                                  &PortalGroup);
    
    
    
    return(Status);
}

ISDSC_STATUS QAddTargetPortal(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//      printf("iscsicli QAddTargetPortal <TargetPortalAddress> 
//      printf("                          [CHAP Username] [CHAP Password]\n"
{
    ISDSC_STATUS Status;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR TargetPortalAddress;
    PTCHAR CHAPUsername, CHAPPassword;
    PISCSI_LOGIN_OPTIONS LoginOptions;
    HRESULT hr;

    if ((ArgC != 3) && (ArgC != 5))
    {
        Usage(35);
        return(ERROR_SUCCESS);
    }

    TargetPortalAddress = ArgV[2];
    if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
    {
        return(ERROR_INVALID_PARAMETER);
    }
    
    hr = StringCchCopy(TargetPortal.Address,
                       MAX_ISCSI_PORTAL_ADDRESS_LEN,
                       TargetPortalAddress);
    
    TargetPortal.Address[MAX_ISCSI_PORTAL_ADDRESS_LEN-1] = 0;
    TargetPortal.Socket = 3260;
    *TargetPortal.SymbolicName = 0;
    
    if (ArgC == 5)
    {
        CHAPUsername = ArgV[3];
        CHAPPassword = ArgV[4];

        Status = BuildLoginOptionsForCHAP(CHAPUsername,
                                          CHAPPassword,
                                          &LoginOptions);
    } else {
        LoginOptions = NULL;
        Status = ERROR_SUCCESS;
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = AddIScsiSendTargetPortal(NULL,     // InitiatorInstance,
                                          ISCSI_ANY_INITIATOR_PORT,
                                          LoginOptions,
                                          0,        // SecurityFlags,
                                          &TargetPortal);
    }   

    if (LoginOptions != NULL)
    {
#ifdef UNICODE
        if (LoginOptions->Username != NULL)
        {
            Free(LoginOptions->Username);
        }

        if (LoginOptions->Password != NULL)
        {
            Free(LoginOptions->Password);
        }
#endif
        Free(LoginOptions);
    }
    
    return(Status);
}

ISDSC_STATUS QAddConnection(
    int ArgC,
    __in_ecount(ArgC) PTCHAR *ArgV
    )
//      printf("iscsicli QAddConnection <SessionId> <Initiator Instance>\n");
//      printf("                        <Target Portal Address>
//      printf("                        [CHAP Username] [CHAP Password]\n");
{
    ISDSC_STATUS Status;
    ISCSI_UNIQUE_SESSION_ID SessionId;
    ISCSI_TARGET_PORTAL TargetPortal;
    PTCHAR TargetPortalAddress;
    ISCSI_UNIQUE_CONNECTION_ID ConnectionId;
    PTCHAR InitiatorName;
    PISCSI_LOGIN_OPTIONS LoginOptions;
    PTCHAR CHAPUsername, CHAPPassword;
    HRESULT hr;

    if ((ArgC != 5) && (ArgC != 7))
    {
        Usage(36);
        return(ERROR_SUCCESS);
    }

    if (! StringToSessionId(ArgV[2], &SessionId))
    {
        Usage(36);
        return(ERROR_SUCCESS);
    }

    InitiatorName = ArgV[3];
    
    if (*ArgV[4] != TEXT('*'))
    {
        TargetPortalAddress = ArgV[4];
        if (_tcslen(TargetPortalAddress) > (MAX_ISCSI_PORTAL_ADDRESS_LEN-1))
        {
            return(ERROR_INVALID_PARAMETER);
        }

        hr = StringCchCopy(TargetPortal.Address,
                           MAX_ISCSI_PORTAL_ADDRESS_LEN,
                           TargetPortalAddress);
        
        TargetPortal.Socket = 3260;
        *TargetPortal.SymbolicName = 0;
        
    } else {
        TargetPortalAddress = TEXT("");
    }
    
    if (ArgC == 7)
    {
        CHAPUsername = ArgV[5];
        CHAPPassword = ArgV[6];

        Status = BuildLoginOptionsForCHAP(CHAPUsername,
                                          CHAPPassword,
                                          &LoginOptions);
    } else {
        LoginOptions = NULL;
        Status = ERROR_SUCCESS;
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = AddIScsiConnection(&SessionId,
                                    InitiatorName,
                                    ISCSI_ANY_INITIATOR_PORT,
                                    *TargetPortalAddress == 0 ?
                                                    NULL : &TargetPortal,
                                    0,              // SecurityFlags,
                                    LoginOptions,
                                    0,              // KeySize,
                                    NULL,           // Key
                                    &ConnectionId); 

        if (Status == ERROR_SUCCESS)
        {
            printf("Connection Id is 0x%I64x-0x%I64x\n",
                   ConnectionId.AdapterUnique,
                   ConnectionId.AdapterSpecific);
        }
    }

    if (LoginOptions != NULL)
    {
#ifdef UNICODE
        if (LoginOptions->Username != NULL)
        {
            Free(LoginOptions->Username);
        }

        if (LoginOptions->Password != NULL)
        {
            Free(LoginOptions->Password);
        }
#endif
        Free(LoginOptions);
    }
    
    return(Status);
}


VOID __cdecl PrintErrorMessage(
    DWORD dwError,
    __in_opt LPTSTR szFmt,
    ...
    )
{
    LPTSTR  szT;
    va_list arglist;
    LPTSTR  szErrMessage = NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwError, 0/*LANG_USER_DEFAULT*/, (LPTSTR)&szErrMessage, 0, NULL);
    if(szFmt && szErrMessage)
    {
        for(szT = szErrMessage; *szT; szT++)
        {
            if(*szT == '\r' || *szT == '\n')
                *szT = 0;
        }
    }

#ifdef UNICODE
    printf("Error 0x%08lx. %ws", dwError, szErrMessage ? szErrMessage : L"");
#else
    printf("Error 0x%08lx. %s", dwError, szErrMessage ? szErrMessage : "");
#endif

    if(szFmt)
    {
        va_start(arglist, szFmt);
#ifdef UNICODE
        vwprintf(szFmt, arglist);
#else
        vprintf(szFmt, arglist);
#endif
        va_end(arglist);
    }

    if(szErrMessage)
        LocalFree((HLOCAL)szErrMessage);
}

ISDSC_STATUS PerformCommandLine(
#ifdef UNICODE
    __in PTCHAR CommandLine
#else
    int argc,
    __in_ecount(argc) char *argv[]
#endif
    )
{
    int ArgC;
    PTCHAR *ArgV;
    ISDSC_STATUS Status = ERROR_SUCCESS;
    TCHAR Message[FORMAT_MESSAGE_MAX_WIDTH_MASK];

#ifdef UNICODE
    ArgV = CommandLineToArgvW(CommandLine, &ArgC);
#else
    ArgC = argc;
    ArgV = argv;
#endif

    if (ArgV != NULL)
    {
        if (ArgC > 1)
        {
            // AddIScsiStaticTarget
            if (_tcsicmp(ArgV[1], TEXT("AddTarget")) == 0)
            {
                Status = AddTarget(ArgC, ArgV);

            // RemoveIScsiStaticTarget          
            } else if (_tcsicmp(ArgV[1], TEXT("RemoveTarget")) == 0) {
                Status = RemoveTarget(ArgC, ArgV);

            // AddIScsiSendTargetPortal         
            } else if (_tcsicmp(ArgV[1], TEXT("AddTargetPortal")) == 0) {
                Status = AddTargetPortal(ArgC, ArgV);

            // RemoveIScsiSendTargetPortal          
            } else if (_tcsicmp(ArgV[1], TEXT("RemoveTargetPortal")) == 0) {
                Status = RemoveTargetPortal(ArgC,ArgV);

            // RefreshIScsiSendTargetPortal         
            } else if (_tcsicmp(ArgV[1], TEXT("RefreshTargetPortal")) == 0) {
                Status = RefreshTargetPortal(ArgC,ArgV);

            // ReportIScsiSendTargetPortals         
            } else if (_tcsicmp(ArgV[1], TEXT("ListTargetPortals")) == 0) {
                Status = ListTargetPortals(ArgC,ArgV);

            // ReportTargets            
            } else if (_tcsicmp(ArgV[1], TEXT("ListTargets")) == 0) {
                Status = ListTargets(ArgC,ArgV);

            // GetTargetInformation         
            } else if (_tcsicmp(ArgV[1], TEXT("TargetInfo")) == 0) {
                Status = TargetInfo(ArgC,ArgV);

            // LoginTarget          
            } else if (_tcsicmp(ArgV[1], TEXT("LoginTarget")) == 0) {
                Status = TryLoginToTarget(ArgC,ArgV);

            // PersistentLoginTarget          
            } else if (_tcsicmp(ArgV[1], TEXT("PersistentLoginTarget")) == 0) {
                Status = PersistentLoginTarget(ArgC,ArgV);

            // RemovePersistentTarget
            } else if (_tcsicmp(ArgV[1], TEXT("RemovePersistentTarget")) == 0) {
                Status = RemovePersistentTarget(ArgC,ArgV);

            // ListPersistentTarget
            } else if (_tcsicmp(ArgV[1], TEXT("ListPersistentTargets")) == 0) {
                Status = ListPersistentTarget(ArgC,ArgV);

            // LogoutTarget         
            } else if (_tcsicmp(ArgV[1], TEXT("LogoutTarget")) == 0) {
                Status = DoLogoutTarget(ArgC,ArgV);

            // ReportInitiatorList          
            } else if (_tcsicmp(ArgV[1], TEXT("ListInitiators")) == 0) {
                Status = DoReportInitiatorList(ArgC,ArgV);

            // ReportActiveIScsiTargetMappings
            } else if (_tcsicmp(ArgV[1], TEXT("ReportTargetMappings")) == 0) {
                Status = DoReportActiveIScsiTargetMappings(ArgC,ArgV);

            // AddConnection            
            } else if (_tcsicmp(ArgV[1], TEXT("AddConnection")) == 0) {
                Status = DoAddConnection(ArgC,ArgV);

            // RemoveConnection
            } else if (_tcsicmp(ArgV[1], TEXT("RemoveConnection")) == 0) {
                Status = DoRemoveConnection(ArgC,ArgV);

            // SendScsiInquiry          
            } else if (_tcsicmp(ArgV[1], TEXT("ScsiInquiry")) == 0) {
                Status = DoScsiInquiry(ArgC,ArgV);

            // SendScsiReadCapacity         
            } else if (_tcsicmp(ArgV[1], TEXT("ReadCapacity")) == 0) {
                Status = ReadCapacity(ArgC,ArgV);

            // SendScsiReportLuns           
            } else if (_tcsicmp(ArgV[1], TEXT("ReportLUNs")) == 0) {
                Status = ReportLUNs(ArgC,ArgV);

            // AddiSNSServer
            } else if (_tcsicmp(ArgV[1], TEXT("AddiSNSServer")) == 0) {
                Status = AddiSNSServerX(ArgC,ArgV);

            // RemoveiSNSServer
            } else if (_tcsicmp(ArgV[1], TEXT("RemoveiSNSServer")) == 0) {
                Status = RemoveiSNSServerX(ArgC,ArgV);

            // ListiSNSServers
            } else if (_tcsicmp(ArgV[1], TEXT("ListiSNSServers")) == 0) {
                Status = ListiSNSServers(ArgC,ArgV);

            // RefreshiSNSServer
            } else if (_tcsicmp(ArgV[1], TEXT("RefreshiSNSServer")) == 0) {
                Status = RefreshiSNSServer(ArgC,ArgV);

            // TunnelAddr
            } else if (_tcsicmp(ArgV[1], TEXT("TunnelAddr")) == 0) {
                Status = TunnelAddress(ArgC,ArgV);

            // GroupKey
            } else if (_tcsicmp(ArgV[1], TEXT("GroupKey")) == 0) {
                Status = GroupKey(ArgC,ArgV);

            // PSKey
            } else if (_tcsicmp(ArgV[1], TEXT("PSKey")) == 0) {
                Status = PSKey(ArgC,ArgV);

            // CHAPSecret
            } else if (_tcsicmp(ArgV[1], TEXT("CHAPSecret")) == 0) {
                Status = CHAPSecret(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("NodeName")) == 0) {
                Status = NodeName(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("SessionList")) == 0) {
                Status = SessionList(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("BindPersistentVolumes")) == 0) {
                Status = BindPeristentVolumes(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("BindPersistentDevices")) == 0) {
                Status = BindPeristentVolumes(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("AddPersistentDevice")) == 0) {
                Status = AddPersistentVolume(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("RemovePersistentDevice")) == 0) {
                Status = RemovePersistentVolume(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("ClearPersistentDevices")) == 0) {
                Status = ClearPersistentVolumes(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("ReportPersistentDevices")) == 0) {
                Status = ReportPersistentVolumes(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("GetPSKey")) == 0) {
                Status = GetPSKey(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("QLoginTarget")) == 0) {
                Status = QLoginTarget(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("QAddTarget")) == 0) {
                Status = QAddTarget(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("QAddTargetPortal")) == 0) {
                Status = QAddTargetPortal(ArgC,ArgV);

            } else if (_tcsicmp(ArgV[1], TEXT("QAddConnection")) == 0) {
                Status = QAddConnection(ArgC,ArgV);


            } else {
                Usage(0);
                Status = ERROR_SUCCESS;
            }
        } else {
            Usage(0);
            Status = ERROR_SUCCESS;
        }

    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
#ifdef UNICODE
    printf("%ws\n", GetiSCSIMessageText(Message,
                                        FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                        Status));
#else
    printf("%s\n", GetiSCSIMessageText(Message,
                                       FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                       Status));
#endif
    return(Status);
}



#define INPUT_BUFFER_SIZE 4096
TCHAR s[INPUT_BUFFER_SIZE];
TCHAR s1[INPUT_BUFFER_SIZE];
int _cdecl main(int argc, __in_ecount(argc) char *argv[])
{
    ISDSC_STATUS Status;
#ifdef UNICODE
    ULONG Len;
#endif
    TCHAR m[MAX_PATH];
    TCHAR NodeName[MAX_ISCSI_NAME_LEN+1];
    ISCSI_VERSION_INFO iSCSIVer;
    HRESULT hr;
    WORD wVersionRequested;
    WSADATA DiscpWsaData;   
    UNREFERENCED_PARAMETER(argv);

    Status = GetIScsiVersionInformation(&iSCSIVer);
    if (Status == ERROR_SUCCESS)
    {
        printf("Microsoft iSCSI Initiator Version %d.%d\n\n",
               iSCSIVer.MajorVersion,
               iSCSIVer.MinorVersion  
              );
    }

    wVersionRequested = MAKEWORD( 1, 1 );
    Status = WSAStartup( wVersionRequested, &DiscpWsaData );

    if (Status == ERROR_SUCCESS)
    {
        if (argc == 1)
        {
#ifdef UNICODE    
            while (! feof(stdin))
            {
                Status = GetIScsiInitiatorNodeName(NodeName);
                if (Status == ERROR_SUCCESS)
                {
                    printf("[%ws] Enter command or ^C to exit\n", NodeName);
                    if (_fgetts(s, INPUT_BUFFER_SIZE, stdin) != NULL) 
                    {
                        hr = StringCchCopy(s1,
                                           INPUT_BUFFER_SIZE,
                                           TEXT("iscsicli "));
                        hr = StringCchCat(s1,
                                          INPUT_BUFFER_SIZE,
                                          s);
                        Len = ((ULONG)_tcslen(s1)-1);
                        if (s1[Len] == 0xa)
                        {
                            s1[Len] = 0;
                        }
                        Status = PerformCommandLine(s1);
                    }
                } else {
#ifdef UNICODE
                    printf("Error getting node name: %ws\n", 
                           GetiSCSIMessageText(m, MAX_PATH, Status));
#else
                    printf("Error getting node name: %s\n",
                           GetiSCSIMessageText(m, MAX_PATH, Status));
#endif
                    break;
                }
            }
#else
            Usage(0);
            Status = GetIScsiInitiatorNodeName(NodeName);
            if (Status == ERROR_SUCCESS)
            {
                printf("Running on node name %s\n", NodeName);
            }
#endif
        } else {
#ifdef UNICODE
            Status = PerformCommandLine(GetCommandLineW());
#else
            Status = PerformCommandLine(argc, argv);
#endif
        }
        WSACleanup();
    } else {
        printf("Error setting up Windows sockets: %ws\n",
               GetiSCSIMessageText(m, MAX_PATH, Status));              
    }
    
    return(Status);
}



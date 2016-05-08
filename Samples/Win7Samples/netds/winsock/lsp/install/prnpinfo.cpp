// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: prnpinfo.cpp
//
// Description:
//
//    This file contains a single routine for printing the contents of a
//    WSAPROTOCOL_INFOW structure.
//
#include <winsock2.h>
#include <ws2spi.h>
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT
#endif

#include <af_irda.h>
#include <wsrm.h>
#include <ws2atm.h>
#include <wsipx.h>

#pragma warning(push)
#pragma warning(disable:4201)

#include <atalkwsh.h>

#pragma warning(pop)

//
// Function: PrintProtocolInfo
//
// Description:
//    Decode a WSAPROTOCOL_INFO entry and print the member fields
//    in a readable format. There's no secret voodoo here, just 
//    interpret all the fields of the structure.
//
void 
PrintProtocolInfo(
    WSAPROTOCOL_INFOW *wsapi
    )
{
    WCHAR       szGuidString[MAX_PATH],
                wszProviderPath[MAX_PATH];
    INT         dwProviderPathLen=MAX_PATH-1;
    int		    rc, error, i;

    rc = WSCGetProviderPath(
           &wsapi->ProviderId,
            wszProviderPath,
           &dwProviderPathLen,
           &error
            );
    if ( 0 != rc )
    {
        fprintf(stderr, "WSCGetProviderPath failed: %d\n", error);
        lstrcpyW(wszProviderPath, L"(error)");
    }

    //
    // Display address family and protocol information
    //

    printf("\nProtocol: %S\n", wsapi->szProtocol);
    printf("\n    Path: %S\n", wszProviderPath);
    printf("           Address Family: ");
    switch ( wsapi->iAddressFamily )
    {
        case AF_INET:
        case AF_INET6:
            printf("%s\n", (wsapi->iAddressFamily == AF_INET ? "AF_INET" : "AF_INET6"));
            printf("                 Protocol: ");      
            switch (wsapi->iProtocol)
            {
                case IPPROTO_IP:
                    printf("IPROTO_IP\n");
                    break;
                case IPPROTO_ICMP:
                    printf("IPROTO_ICMP\n");
                    break;
                case IPPROTO_IGMP:
                    printf("IPROTO_IGMP\n");
                    break;
                case IPPROTO_GGP:
                    printf("IPROTO_GGP\n");
                    break;
                case IPPROTO_TCP:
                    printf("IPROTO_TCP\n");
                    break;
                case IPPROTO_PUP:
                    printf("IPROTO_PUP\n");
                    break;
                case IPPROTO_UDP:
                    printf("IPROTO_UDP\n");
                    break;
                case IPPROTO_IDP:
                    printf("IPROTO_IDP\n");
                    break;
                case IPPROTO_ND:
                    printf("IPROTO_ND\n");
                    break;
                case IPPROTO_RM:
                    printf("IPPROTO_RM\n");
                    break;
                case IPPROTO_RAW:
                    printf("IPROTO_RAW\n");
                    break;
            }
            break;

        case AF_UNSPEC:
            printf("AF_UNSPEC\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_UNIX:
            printf("AF_UNIX\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_IMPLINK:
            printf("AF_IMPLINK\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_PUP:
            printf("AF_PUP\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_CHAOS:
            printf("AF_CHAOS\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_NS:
            printf("AF_NS or AF_IPX\n");
            printf("                 Protocol: ");      

            switch ( wsapi->iProtocol )
            {
                case NSPROTO_IPX:
                    printf("NSPROTO_IPX\n");
                    break;
                case NSPROTO_SPX:
                    printf("NSPROTO_SPX\n");
                    break;
                case NSPROTO_SPXII:
                    printf("NSPROTO_SPXII\n");
                    break;
            }
            break;

        case AF_ISO:
            printf("AF_ISO or AF_OSI\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_ECMA:
            printf("AF_ECMA\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_DATAKIT:
            printf("AF_DATAKIT\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_CCITT:
            printf("AF_CCITT\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_SNA:
            printf("AF_SNA\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_DECnet:
            printf("AF_DECnet\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_DLI:
            printf("AF_DLI\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_LAT:
            printf("AF_LAT\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_HYLINK:
            printf("AF_HYLINK\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_APPLETALK:
            printf("AF_APPLETALK\n");
            printf("                 Protocol: ");      

            switch ( wsapi->iProtocol )
            {
                case DDPPROTO_RTMP:
                    printf("DDPPROTO_RTMP\n");
                    break;
                case DDPPROTO_NBP:
                    printf("DDPPROTO_NBP\n");
                    break;
                case DDPPROTO_ATP:
                    printf("DDPROTO_ATP\n");
                    break;
                case DDPPROTO_AEP:
                    printf("DDPPROTO_AEP\n");
                    break;
                case DDPPROTO_RTMPRQ:
                    printf("DDPPROTO_RTMPRQ\n");
                    break;
                case DDPPROTO_ZIP:
                    printf("DDPPROTO_ZIP\n");
                    break;
                case DDPPROTO_ADSP:
                    printf("DDPPROTO_ADSP\n");
                    break;
                case ATPROTO_ADSP:
                    printf("ATPROTO_ADSP\n");
                    break;
                case ATPROTO_ATP:
                    printf("ATPROTO_ATP\n");
                    break;
                case ATPROTO_ASP:
                    printf("ATPROTO_ASP\n");
                    break;
                case ATPROTO_PAP:
                    printf("ATPROTO_PAP\n");
                    break;
            }
            break;

        case AF_NETBIOS:
            printf("AF_NETBIOS\n");
            printf("                 Protocol: ");      
            printf("NetBIOS LANA %d\n", ((wsapi->iProtocol == 0x80000000) ? 0: abs(wsapi->iProtocol)));
            break;

        case AF_VOICEVIEW:
            printf("AF_VOICEVIEW\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_FIREFOX:
            printf("AF_FIREFOX\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_UNKNOWN1:
            printf("AF_UNKNOWN1\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_BAN:
            printf("AF_BAN\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;

        case AF_ATM:
            printf("AF_ATM\n");
            printf("                 Protocol: ");      

            switch ( wsapi->iProtocol )
            {
                case ATMPROTO_AALUSER:
                    printf("ATMPROTO_AALUSER\n");
                    break;
                case ATMPROTO_AAL1:
                    printf("ATMPROTO_AAL1\n");
                    break;
                case ATMPROTO_AAL2:
                    printf("ATMPROTO_AAL2\n");
                    break;
                case ATMPROTO_AAL34:
                    printf("ATMPROTO_AAL34\n");
                    break;
                case ATMPROTO_AAL5:
                    printf("ATMPROTO_AAL5\n");
                    break;
            }
            break;

        case AF_CLUSTER:
            printf("AF_CLUSTER\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;
        case AF_12844:
            printf("AF_12844\n");
            printf("                 Protocol: UNKNOWN: %d", wsapi->iProtocol);      
            break;
        case AF_IRDA:
            printf("AF_IRDA\n");
            printf("                 Protocol: ");      

            switch (wsapi->iProtocol)
            {
                case IRDA_PROTO_SOCK_STREAM:
                    printf("IRDA_PROTO_SOCK_STREAM\n");
                    break;
            }
            break;

        default:
            printf("Unknown: %d\n", wsapi->iAddressFamily);
    }

    //
    // Display socket type information
    //

    printf("              Socket Type: ");
    switch (wsapi->iSocketType)
    {
        case SOCK_STREAM:
            printf("SOCK_STREAM\n");
            break;
        case SOCK_DGRAM:
            printf("SOCK_DGRAM\n");
            break;
        case SOCK_RAW:
            printf("SOCK_RAW\n");
            break;
        case SOCK_RDM:
            printf("SOCK_RDM\n");
            break;
        case SOCK_SEQPACKET:
            printf("SOCK_SEQPACKET\n");
            break;
    }

    //
    // Display the various provider flags for this entry
    //

    printf("           Connectionless: ");
    if (wsapi->dwServiceFlags1 & XP1_CONNECTIONLESS)
        printf("YES\n");
    else
        printf("NO\n");
    printf("      Guaranteed Delivery: ");
    if (wsapi->dwServiceFlags1 & XP1_GUARANTEED_DELIVERY)
        printf("YES\n");
    else
        printf("NO\n");
    printf("         Guaranteed Order: ");
    if (wsapi->dwServiceFlags1 & XP1_GUARANTEED_ORDER)
        printf("YES\n");
    else
        printf("NO\n");
    printf("         Message Oriented: ");
    if (wsapi->dwServiceFlags1 & XP1_MESSAGE_ORIENTED)
        printf("YES\n");
    else
        printf("NO\n");
    printf("            Pseudo Stream: ");
    if (wsapi->dwServiceFlags1 & XP1_PSEUDO_STREAM)
        printf("YES\n");
    else
        printf("NO\n");
    printf("           Graceful Close: ");
    if (wsapi->dwServiceFlags1 & XP1_GRACEFUL_CLOSE)
        printf("YES\n");
    else
        printf("NO\n");
    printf("           Expedited Data: ");
    if (wsapi->dwServiceFlags1 & XP1_EXPEDITED_DATA)
        printf("YES\n");
    else
        printf("NO\n");
    printf("             Connect Data: ");
    if (wsapi->dwServiceFlags1 & XP1_CONNECT_DATA)
        printf("YES\n");
    else
        printf("NO\n");
    printf("          Disconnect Data: ");
    if (wsapi->dwServiceFlags1 & XP1_DISCONNECT_DATA)
        printf("YES\n");
    else
        printf("NO\n");
    printf("       Supports Broadcast: ");
    if (wsapi->dwServiceFlags1 & XP1_SUPPORT_BROADCAST)
        printf("YES\n");
    else
        printf("NO\n");
    printf("      Supports Multipoint: ");
    if (wsapi->dwServiceFlags1 & XP1_SUPPORT_MULTIPOINT)
        printf("YES\n");
    else
        printf("NO\n");
    printf(" Multipoint Control Plane: ");
    if (wsapi->dwServiceFlags1 & XP1_MULTIPOINT_CONTROL_PLANE)
        printf("ROOTED\n");
    else
        printf("NON-ROOTED\n");
    printf("    Multipoint Data Plane: ");
    if (wsapi->dwServiceFlags1 & XP1_MULTIPOINT_DATA_PLANE)
        printf("ROOTED\n");
    else
        printf("NON-ROOTED\n");
    printf("            QoS Supported: ");
    if (wsapi->dwServiceFlags1 & XP1_QOS_SUPPORTED)
        printf("YES\n");
    else
        printf("NO\n");
    printf("     Unidirectional Sends: ");
    if (wsapi->dwServiceFlags1 & XP1_UNI_SEND)
        printf("YES\n");
    else
        printf("NO\n");
    printf("    Unidirection Receives: ");
    if (wsapi->dwServiceFlags1 & XP1_UNI_RECV)
        printf("YES\n");
    else
        printf("NO\n");
    printf("              IFS Handles: ");
    if (wsapi->dwServiceFlags1 & XP1_IFS_HANDLES)
        printf("YES\n");
    else
        printf("NO\n");
    printf("         Partial Messages: ");
    if (wsapi->dwServiceFlags1 & XP1_PARTIAL_MESSAGE)
        printf("YES\n");
    else
        printf("NO\n");
    printf("           Provider Flags: ");
    if (wsapi->dwProviderFlags & PFL_MULTIPLE_PROTO_ENTRIES)
    {
        printf("PFL_MULTIPLE_PROTO_ENTRIES ");
    }
    if (wsapi->dwProviderFlags & PFL_RECOMMENDED_PROTO_ENTRY)
    {
        printf("PFL_RECOMMENDED_PROT_ENTRY ");
    }
    if (wsapi->dwProviderFlags & PFL_HIDDEN)
    {
        printf("PFL_HIDDEN ");
    }
    if (wsapi->dwProviderFlags & PFL_MATCHES_PROTOCOL_ZERO)
    {
        printf("PFL_MATCHES_PROTOCOL_ZERO ");
    }
    if (wsapi->dwProviderFlags == 0)
    {
        printf("NONE");
    }
    printf("\n");

    //
    // Display provider ID and catalog ID as well as LSP chain information
    //

    StringFromGUID2( wsapi->ProviderId, szGuidString, MAX_PATH-1 );

    printf("              Provider Id: %S\n", szGuidString);
    printf("         Catalog Entry Id: %ld\n", wsapi->dwCatalogEntryId);
    printf("  Number of Chain Entries: %d   {", 
        wsapi->ProtocolChain.ChainLen);

    for(i=0; i < wsapi->ProtocolChain.ChainLen ;i++)
        printf("%ld ", wsapi->ProtocolChain.ChainEntries[i]);

    printf("}\n");

    //
    // Display the remaining information
    //

    printf("                  Version: %d\n", wsapi->iVersion);
    printf("Max Socket Address Length: %d\n", wsapi->iMaxSockAddr);
    printf("Min Socket Address Length: %d\n", wsapi->iMinSockAddr);
    printf("      Protocol Max Offset: %d\n", wsapi->iProtocolMaxOffset);

    printf("       Network Byte Order: ");
    if (wsapi->iNetworkByteOrder == 0)
        printf("BIG-ENDIAN\n");
    else
        printf("LITLE-ENDIAN\n");

    printf("          Security Scheme: ");
    if (wsapi->iSecurityScheme == SECURITY_PROTOCOL_NONE)
        printf("NONE\n");
    else
        printf("%d\n", wsapi->iSecurityScheme);

    printf("             Message Size: ");
    if (wsapi->dwMessageSize == 0)
        printf("N/A (Stream Oriented)\n");
    else if (wsapi->dwMessageSize == 1)
        printf("Depended on underlying MTU\n");
    else if (wsapi->dwMessageSize == 0xFFFFFFFF)
        printf("No limit\n");
    else
        printf("%ld\n", wsapi->dwMessageSize);

    printf("        Provider Reserved: %d\n", wsapi->dwProviderReserved);
}

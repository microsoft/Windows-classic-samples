// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"
#include "VpnConnection.h"

#define EAP_TLS             13      // Smartcard or other certificate (TLS)
#define EAP_PEAP            25      // PEAP
#define EAP_MSCHAPv2        26      // EAP Mschapv2

#define RDT_Tunnel_Pptp (0x8)
#define RDT_Tunnel_L2tp (0x9)
#define RDT_Tunnel_Sstp (0xe)
#define RDT_Tunnel_Ikev2 (0xf)


//********************************************************************************************
// Function: EnumerateVpnConnections
//
// Description: Enumarates and displays all the VPN connections connected to the Remote access server.
//
//********************************************************************************************
VOID EnumerateVpnConnections(
    _In_opt_ LPWSTR serverName
    )
{
    DWORD status = ERROR_SUCCESS;
    MPR_SERVER_HANDLE serverHandleAdmin = NULL;
    HANDLE serverHandleConfig = NULL;
    RAS_CONNECTION_4* vpnConnectionList = NULL;
    DWORD entriesRead   = 0;
    DWORD totalEntries  = 0;
    DWORD resumeHandle    = 0;
    
    wprintf(L"---------------------------------------------------------\n\n");
    wprintf(L"Executing EnumerateVpnConnections on '%s'\n", (serverName == NULL) ? L"Current machine" : serverName);

    status = RemoteAccessServerConenct(serverName, &serverHandleAdmin, &serverHandleConfig);
    if ((ERROR_SUCCESS != status) || 
        (NULL == serverHandleAdmin))
    {
        wprintf(L"RemoteAccessServerConenct failed. \
            The RemoteAccess service might be stopped - Try after starting the RemoteAccess service.\n");
        DisplayError(status);
        goto Done;
    }
    
    status = g_pMprAdminConnectionEnum(serverHandleAdmin,            // hRasServer
                                     4,                 // dwLevel
                                     (LPBYTE *) &vpnConnectionList, // lplpbBuffer
                                     0xffffffff,        // dwPrefMaxLen
                                     &entriesRead,    // lpdwEntriesRead
                                     &totalEntries,   // lpdwTotalEntries
                                     &resumeHandle);    // lpdwResumeHandle
    if (ERROR_SUCCESS != status)
    {
        wprintf(L"MprAdminConnectionEnum failed.\n");
        DisplayError(status);
        goto Done;
    }
    
    if (0 == entriesRead)
    {
        wprintf(L"No VPN clients are connected to the server at this moment.\n");
    }
    else
    {
        wprintf(L"%u VPN clients are connected to the server at this moment.\n", entriesRead);
    }

    for (DWORD index = 0; index < entriesRead; index++)
    {
        wprintf(L"Connection - %u\n", (index + 1));
        PrintVPNConnectionDetails(&(vpnConnectionList[index])); 
    }
    
Done:
    if (vpnConnectionList)
    {
        g_pMprAdminBufferFree(vpnConnectionList);
    }
    RemoteAccessServerDisconenct(serverHandleAdmin, serverHandleConfig);
    
    if (status != ERROR_SUCCESS)
    {
         wprintf(L"EnumerateVpnConnections failed\n");
         DisplayError(status);
    }
    wprintf(L"---------------------------------------------------------\n\n");
}


//********************************************************************************************
// Function: PrintVPNConnectionDetails
//
// Description: Prints various fields of the speficied RAS_CONNECTION_4 structure in string format 
//
//********************************************************************************************
VOID PrintVPNConnectionDetails(
    _In_ RAS_CONNECTION_4* vpnConnection
    )
{
    wprintf(L"\t Connection Start time: ");
    PrintFileTime(&(vpnConnection->connectionStartTime));
    wprintf(L"\n");

    // Username
    //
    wprintf(L"\t Remote username: ");
    if (0 != vpnConnection->wszLogonDomain[0])
    {
        // Domain name is specified
        //
        wprintf(L"%s\\%s\n", vpnConnection->wszLogonDomain, vpnConnection->wszUserName);
    }
    else if (0 != vpnConnection->wszUserName[0])
    {
        wprintf(L"%s\n", vpnConnection->wszUserName);
    }
    else
        wprintf(L"UnKnown\n");
        

    
    wprintf(L"\t Interface type: ");
    PrintInterfaceType(vpnConnection->dwInterfaceType);
    wprintf(L"\n");
    
    // print tunnel type
    //
    wprintf(L"\t VPN type: ");
    switch (vpnConnection->dwDeviceType)
    {
        case RDT_Tunnel_Pptp:
            wprintf(L"PPTP\n");
            break;
        case RDT_Tunnel_L2tp:
            wprintf(L"L2TP\n");
            break;
        case RDT_Tunnel_Sstp:
            wprintf(L"SSTP\n");
            break;
        case RDT_Tunnel_Ikev2:
            wprintf(L"IKEv2\n");
            break;
        default:
            wprintf(L"UnKnown\n");
    }

    // Authentication method
    //
    wprintf(L"\t Authentication method: ");
    PrintAuthMethod(&(vpnConnection->ProjectionInfo));
    wprintf(L"\n");
    
    wprintf(L"\t Total bytes Received: %I64u\n", vpnConnection->ullBytesRcved);
    wprintf(L"\t Total bytes Transmitted: %I64u\n", vpnConnection->ullBytesXmited);
    
}

//********************************************************************************************
// Function: PrintAuthMethod
//
// Description: Retrieve the authentication method from the speficied PROJECTION_INFO2 structure 
// and prints it in string format 
//
//********************************************************************************************
VOID PrintAuthMethod(
    _In_ PROJECTION_INFO2* projectionInfo
    )
{
    DWORD authProtocol = 0;
    DWORD eapTypeID = 0;
    DWORD embeddedEapTypeID = 0;
    DWORD authenticationData = 0;

    if (projectionInfo->projectionInfoType == MPRAPI_IKEV2_PROJECTION_INFO_TYPE)
    {
        // This is an IKEv2 tunnel.
        //
        authProtocol = projectionInfo->Ikev2ProjectionInfo.dwAuthenticationProtocol;
        eapTypeID = projectionInfo->Ikev2ProjectionInfo.dwEapTypeId;
        embeddedEapTypeID = projectionInfo->Ikev2ProjectionInfo.dwEmbeddedEAPTypeId;
    }
    else
    {
        // This is a PPP based tunnell.
        //
        authProtocol = projectionInfo->PppProjectionInfo.dwAuthenticationProtocol;
        eapTypeID = projectionInfo->PppProjectionInfo.dwEapTypeId;
        embeddedEapTypeID = projectionInfo->PppProjectionInfo.dwEmbeddedEAPTypeId;
        authenticationData = projectionInfo->PppProjectionInfo.dwAuthenticationData;
    }
    if ((authProtocol != PPP_LCP_EAP) && 
        (authProtocol != MPRAPI_IKEV2_AUTH_USING_EAP))
    {
        // if auth protocol is not EAP, the auth protocol ID is same as the auth method used
        //
        switch (authProtocol)
        {
        case PPP_LCP_PAP:
            wprintf(L"PAP");
            break;
        case MPRAPI_IKEV2_AUTH_USING_CERT:
            wprintf(L"Machine Certificate");
            break;
        case PPP_LCP_CHAP:
            if (authenticationData == PPP_LCP_CHAP_MSV2)
                wprintf(L"MS-CHAPv2");
            else
                wprintf(L"CHAP");
            break;
        default:
            wprintf(L"UnKnown(%u)", authProtocol);
        }
    }
    else
    {
        // In case of EAP auth protocol, get the auth method from the eap method type
        //
        switch (eapTypeID)
        {
        case EAP_TLS:
            wprintf(L"EAP-TLS");
            break;
        case EAP_PEAP:
            wprintf(L"EAP-PEAP");
            break;
        case EAP_MSCHAPv2:
            wprintf(L"EAP-MSCHAPv2");
            break;
        default:
            wprintf(L"EAP(MethodID: %u)", eapTypeID);
            break;
        }

        // Get the inner auth method from the embedded eap method type
        //
        switch (embeddedEapTypeID)
        {
        case EAP_TLS:
            wprintf(L" (Inner EAP method: EAP-TLS)");
            break;
        case EAP_MSCHAPv2:
            wprintf(L" (Inner EAP method: EAP-MSCHAPv2)");
            break;
        }
    }
}


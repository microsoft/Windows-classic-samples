// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once


#include <MprApi.h>

//
// Configures custom IPSec policies on the remote access server that would be applied to 
// all the VPN clients.
//
VOID SetCustomIpsecConfigurationOnServer(
    _In_opt_ LPWSTR serverName
    );

//
// Removes the custom IPSec configuration (if configured) from the specified remote access server.
//
VOID RemoveCustomIpsecConfigurationFromServer(
    _In_opt_ LPWSTR serverName
    );

//
// Configures custom IPSec configuration, to be used applied to the IKEv2 
// tunnel based VPN clients, on the RRAS server 
//
DWORD ConfigureCustomIPSecPolicyOnServer(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig
    );

//
//  Retrieves and displays the custom IPSec policies configured on the RRAS server
//
DWORD DisplayCustomIPSecConfigurationOnServer(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig
    );

//
// Releases the various fields of the MPR_SERVER_EX structure,
// those are allocated by the remote access server.
//
VOID FreeServerConfigObject(
    _In_ BOOL useAdminApi,
    _In_ MPR_SERVER_EX* serverConfigInfo
    );

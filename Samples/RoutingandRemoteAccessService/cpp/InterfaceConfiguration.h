// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <MprApi.h>
//
// Creates a demand dial interface and configures custom IPSec policies on it.
//
VOID SetCustomIpsecConfigurationOnInterface(
    _In_opt_ LPWSTR serverName
    );

//
// Removes custom IPSec configuration from a demand dial interface.
//
VOID RemoveCustomIpsecConfigurationFromInterface(
    _In_opt_ LPWSTR serverName
    );

//
// Creates a new demand dial interface on the specified remote access server and 
// returns the handle to the newly created interface 
//
DWORD CreateNewDoDInterface(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _Out_ HANDLE* interfaceHandleAdmin,
    _Out_ HANDLE* interfaceHandleConfig
    );

//
// Configures custom IPSec policies on the specified interface 
//
DWORD ConfigureCustomIPSecPolicyOnDoDInterface(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _In_ HANDLE interfaceHandleAdmin,
    _In_ HANDLE interfaceHandleConfig
    );

//
//  Update the custom configuration of the speficied interface with 
//  the supplied custom configuration information 'mprInterfaceCustomInfo'.
//
DWORD UpdateInterfaceCustomConfiguration(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig,
    _In_ HANDLE interfaceHandleAdmin,
    _In_ HANDLE interfaceHandleConfig,
    _In_ MPR_IF_CUSTOMINFOEX* mprInterfaceCustomInfo
    );

//
//  Retrieves and displays the custom IPSec policies configured on the specified interface
//
DWORD DisplayDoDInterfaceConfiguration(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE interfaceHandleAdmin
    );

//
// Prints various fields of the ROUTER_IKEv2_IF_CUSTOM_CONFIG0 structure in string format
//
VOID PrintInterfaceCustomConfiguration(
    _In_ ROUTER_IKEv2_IF_CUSTOM_CONFIG* customIkev2Config
    );

//
// Prints various fields of the speficied MPR_INTERFACE_3 structure in string format 
//
VOID PrintInterfaceInfo(
    _In_ MPR_INTERFACE_3* ifInfo
    );

//
// Releases the various fields of the MPR_IF_CUSTOMINFOEX0 
// structure, those are allocated by the remote access server.
//
VOID FreeInterfaceCustomConfigObject(
    _In_ MPR_IF_CUSTOMINFOEX* customConfig
    );

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <MprApi.h>

//
// Enumarates and displays all the VPN connections connected to the Remote access server.
//
VOID EnumerateVpnConnections(
    _In_opt_ LPWSTR serverName
    );

//
// Prints various fields of the speficied RAS_CONNECTION_4 structure in string format 
//
VOID PrintVPNConnectionDetails(
    _In_ RAS_CONNECTION_4* vpnConnection
    );

//
// Retrieve the authentication method from the speficied PROJECTION_INFO2 structure 
// and prints it in string format 
//
VOID PrintAuthMethod(
    _In_ PROJECTION_INFO2* projectionInfo
    );


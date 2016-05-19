// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    Tcpcommon.h

Abstract:

    This file defines common shared routines between the sample secure TCP
    client and secure TCP server. These routines mainly contain sample code for
    managing custom IPsec policy and querying IPsec SA information using the 
    FWP API.

--*/

#ifndef _TCP_COMMON_H_
#define _TCP_COMMON_H_
#pragma once

#include <windows.h>
#include <fwpmu.h>
#include <Winsock2.h>
#include <nldef.h>

DWORD
AddCustomIPsecPolicy(
   OUT HANDLE* fwpHandle,
   OUT GUID* qmPolicyKey
);

VOID
RemoveCustomIPsecPolicy(
   IN HANDLE fwpHandle,
   IN const GUID* qmPolicyKey
);

DWORD
MatchIPsecSAsForConnectedSocket(
   IN SOCKET sock
);

#define RECV_DATA_BUF_SIZE 256

#endif //_TCP_COMMON_H_


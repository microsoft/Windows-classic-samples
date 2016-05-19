// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module defines the CDiscoveryProtocol class.
//      This class implements a very ridimentary discovery protocol for
//      sample purposes only.  

#pragma once

HRESULT StartDiscoveryQuery(
    __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
    __in_opt PCWSTR pszDeviceCategory,
    __in_opt PCWSTR pszInstanceQueryId,
    __deref_out PHANDLE phQuery);

VOID CloseDiscoveryQuery(
    HANDLE hQuery);

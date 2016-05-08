/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PNRP.h

Abstract:

    This C header file declares PNRP wrapper functions for use in the
    Peer-To-Peer Graphing API sample application.

--********************************************************************/

#pragma once

HRESULT PnrpStartup();
HRESULT PnrpShutdown();
HRESULT PnrpRegister(PCWSTR pwzIdentity, PCWSTR pwzName, PCWSTR pwzCloud, __in PEER_NODE_INFO* pNodeInfo);
HRESULT PnrpUnregister();
HRESULT PnrpResolve(PCWSTR pwzName, PCWSTR pwzCloud, __out ULONG *pcEndpoints, __out PPEER_PNRP_ENDPOINT_INFO *pEndpoints);
HRESULT GetLocalCloudInfo(DWORD cchCloudName, 
                          __out_ecount_opt(cchCloudName) PWSTR pwzCloudName, 
                          __out_opt DWORD* pdwLocalScopeID);

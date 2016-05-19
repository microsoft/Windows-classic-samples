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

HRESULT PnrpRegister(PCWSTR pwzIdentity, PCWSTR pwzName, PCWSTR pwzCloud, __in PEER_NODE_INFO* pNodeInfo);
HRESULT PnrpUnregister(PCWSTR pwzIdentity, PCWSTR pwzName, PCWSTR pwzCloud);
HRESULT PnrpResolve(PCWSTR pwzName, PCWSTR pwzCloud, __out SOCKADDR_IN6* pAddr);
HRESULT GetLocalCloudInfo(DWORD cchCloudName, 
                          __out_ecount_opt(cchCloudName) PWSTR pwzCloudName, 
                          __out_opt DWORD* pdwLocalScopeID);

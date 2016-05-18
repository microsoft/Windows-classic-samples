/**********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    GraphChat.c

Abstract:

    This C file includes sample code which wraps winsock calls for
    using PNRP functionality.

Feedback:
    If you have any questions or feedback, please contact us using 
    any of the mechanisms below:

    Email: peerfb@microsoft.com 
    Newsgroup: Microsoft.public.win32.programmer.networks 
    Website: http://www.microsoft.com/p2p 

--********************************************************************/
   
#pragma warning(disable:4201)   // nameless struct/union

#include "graphchat.h"
#include <initguid.h>
#include <pnrpns.h>
#include "pnrp.h"


const DWORD REGISTRATION_LIFETIME = 60 * 60 * 8; // 8 hours

HANDLE              g_hRegistration = NULL;

HRESULT PnrpStartup()
{
    HRESULT hr = S_OK;

    hr = PeerPnrpStartup(PNRP_VERSION);

    if (FAILED(hr))
    {
        DisplayHrError(L"PnrpStartup failed", hr);

    }

    return hr;
}

HRESULT PnrpShutdown()
{
    HRESULT hr = S_OK;

    hr = PeerPnrpShutdown();

    if (FAILED(hr))
    {
        DisplayHrError(L"PeerPnrpShutdown failed", hr);
    }

    return hr;
}

//-------------------------------------------------------------------------
// Function: PnrpRegister
//
// Purpose:  Register the given name in the PNRP cloud
//
// Arguments:
//   pwzIdentity : identity string created using PeerIdentityCreate
//   pwzName     : name to register in PNRP, generally the graph id
//   pwzCloud    : name of cloud to register in, NULL = all clouds
//   pNodeInfo   : local node info returned from PeerGraphGetNodeInfo
//
// Returns:  HRESULT
//
HRESULT PnrpRegister(PCWSTR pwzIdentity, PCWSTR pwzName, 
                     PCWSTR pwzCloud, __in PEER_NODE_INFO* pNodeInfo)
{
    HRESULT hr = S_OK;
    PEER_PNRP_REGISTRATION_INFO registrationInfo = {0};
    ULONG           nAddrs = 0;
    ULONG           i = 0;

    if (pwzIdentity == NULL || pwzName == NULL || pNodeInfo == NULL ||
        (pNodeInfo->cAddresses!=PEER_PNRP_AUTO_ADDRESSES && pwzCloud == NULL))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (g_hRegistration != NULL)
    {
        hr = PnrpUnregister();
        if (FAILED(hr))
        {
            goto exit;
        }
    }

    // do not attempt to register more addresses than PNRP allows
    if (pNodeInfo->cAddresses!=PEER_PNRP_AUTO_ADDRESSES && pNodeInfo->cAddresses > PNRP_MAX_ENDPOINT_ADDRESSES)
    {
        nAddrs = PNRP_MAX_ENDPOINT_ADDRESSES;
    }
    else
    {
        nAddrs = pNodeInfo->cAddresses;
    }

    registrationInfo.pwzCloudName = (PWSTR) pwzCloud;
    registrationInfo.pwzPublishingIdentity = (PWSTR) pwzIdentity;
    registrationInfo.cAddresses = nAddrs;
    if (pNodeInfo->cAddresses == 0 || pNodeInfo->cAddresses == PEER_PNRP_AUTO_ADDRESSES)
    {
        registrationInfo.ppAddresses = NULL;
    }
    else
    {
        registrationInfo.ppAddresses = (SOCKADDR**)malloc(sizeof(SOCKADDR*) * nAddrs);

        if (registrationInfo.ppAddresses == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        // copy the addresses from PEER_NODE_INFO into CSADDR_INFO format
        for (i = 0; i < nAddrs; i++)
        {
            registrationInfo.ppAddresses[i] = (LPSOCKADDR) &pNodeInfo->pAddresses[i].sin6;
        }
    }

    registrationInfo.pwzComment = L"GraphChatMember";

    //Insert our node id as the payload.  Other nodes can then easily tell if they are already connected
    //to the node associated with this PNRP result
    registrationInfo.payload.cbData = sizeof(pNodeInfo->ullNodeId);
    registrationInfo.payload.pbData = (PBYTE)&pNodeInfo->ullNodeId;

    hr = PeerPnrpRegister(pwzName, &registrationInfo, &g_hRegistration);

exit:
    free(registrationInfo.ppAddresses);

    return hr;
}

//-------------------------------------------------------------------------
// Function: PnrpResolve
//
// Purpose:  Resolve the given name within a PNRP cloud
//
// Arguments:
//   pwzName  : name to resolve in PNRP, generally the graph id
//   pwzCloud : name of cloud to resolve in, NULL = global cloud
//   pAddr    : pointer to result buffer
//
// Returns:  HRESULT
//
HRESULT PnrpResolve(PCWSTR pwzName, PCWSTR pwzCloud, __out ULONG *pcEndpoints, __out PPEER_PNRP_ENDPOINT_INFO *pEndpoints)
{
    HRESULT         hr = S_OK;

    if (pEndpoints == NULL)
    {
        return E_INVALIDARG;
    }

    //get up to five arbitary endpoints to connect to, just in case some fail to respond
    //(once we make contact with one graph member the infrastructure handles discovering other nodes)
    *pcEndpoints = 5;
                       
    hr = PeerPnrpResolve(pwzName, pwzCloud, pcEndpoints, pEndpoints);
   
    return hr;
}

//-------------------------------------------------------------------------
// Function: PnrpUnregister
//
// Purpose:  Unregister the graph from the PNRP cloud
//
// Returns:  HRESULT
//
HRESULT PnrpUnregister()
{
    HRESULT         hr = S_OK;

    hr = PeerPnrpUnregister(g_hRegistration);
    g_hRegistration = NULL;

    return hr;
}

//-------------------------------------------------------------------------
// Function: GetLocalCloudInfo
//
// Purpose:  Retrieve scope ID and/or local cloud name for use in creating
//           and connecting to link local graphs
//
// Arguments:
//   pdwLocalScopeID[out]: Identifier of local connection
//   pwzCloudName[out]   : Name of local connection 
//                         (NULL results in no value returned)
//   cchCloudNameSize[in]: number of characters in pwzCloudName 
//                         (usually MAX_CLOUD_NAME)
//
// Returns:  HRESULT
//
HRESULT GetLocalCloudInfo(DWORD cchCloudName, 
                          __out_ecount_opt(cchCloudName) PWSTR pwzCloudName, 
                          __out_opt DWORD* pdwLocalScopeID)
{
    HRESULT         hr = S_OK;
    PNRPCLOUDINFO   CloudInfo = {0};
    BLOB            blPnrpData = {0};
    WSAQUERYSET     querySet = {0};
    WSAQUERYSET*    pResults = NULL;
    WSAQUERYSET     tempResultSet = {0};
    HANDLE          hLookup = NULL;
    INT             iErr;
    DWORD           dwErr;
    DWORD           dwResultSize;

    // Fill out information for WSA query
    CloudInfo.dwSize = sizeof(CloudInfo);
    CloudInfo.Cloud.Scope = PNRP_LINK_LOCAL_SCOPE;
    
    blPnrpData.cbSize = sizeof(CloudInfo);
    blPnrpData.pBlobData = (LPBYTE)&CloudInfo;

    querySet.dwSize = sizeof(querySet);
    querySet.dwNameSpace = NS_PNRPCLOUD;
    querySet.lpServiceClassId = (LPGUID)&SVCID_PNRPCLOUD;
    querySet.lpBlob = &blPnrpData;

    iErr = WSALookupServiceBegin(
            &querySet, 
            LUP_RETURN_NAME|LUP_RETURN_BLOB, 
            &hLookup);

    if (iErr != 0)
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
    }

    if (SUCCEEDED(hr))
    {
        dwResultSize = sizeof(tempResultSet);

        // Get size of results
        iErr = WSALookupServiceNext(hLookup, 0, &dwResultSize, &tempResultSet);

        if (iErr != 0)
        {
            dwErr = WSAGetLastError();
            if (dwErr == WSAEFAULT)
            {
                // allocate space for results
                pResults = (WSAQUERYSET*) malloc(dwResultSize);

                if (pResults == NULL)
                {
                    hr = HRESULT_FROM_WIN32(WSAGetLastError());
                    goto exit;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwErr);
                goto exit;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            goto exit;
        }
    }

    if (SUCCEEDED(hr))
    {
        // retrieve the local cloud information
        iErr = WSALookupServiceNext(hLookup, 0, &dwResultSize, pResults);
        if (iErr != 0)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Copy the cloud name (if applicable) and scope ID
    if (SUCCEEDED(hr))
    {
        if (pwzCloudName)
        {
            hr = StringCchCopy(pwzCloudName, cchCloudName, pResults->lpszServiceInstanceName);                
            if (FAILED(hr))
            {
                DisplayHrError(L"Failed to copy cloud name", hr);
            }
        }
        if (pdwLocalScopeID)
        {
            *pdwLocalScopeID = ((PNRPCLOUDINFO*)pResults->lpBlob->pBlobData)->Cloud.ScopeId;  
        }
    }

exit:
    if (hLookup != NULL)
    {
        (void) WSALookupServiceEnd(hLookup);
    }

    free(pResults);
    return hr;
}



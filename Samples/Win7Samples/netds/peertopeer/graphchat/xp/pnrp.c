/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PNRP.c

Abstract:

    This C file includes sample code which wraps winsock calls for
    using PNRP functionality.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include "graphchat.h"
#include <initguid.h>
#include <pnrpns.h>
#include "pnrp.h"


const DWORD REGISTRATION_LIFETIME = 60 * 60 * 8; // 8 hours

//-------------------------------------------------------------------------
// Function: PnrpRegister
//
// Purpose:  Register the given name in the PNRP cloud
//
// Arguments:
//   pwzIdentity : identity string created using PeerIdentityCreate
//   pwzName     : name to register in PNRP, generally the graph id
//   pwzCloud    : name of cloud to register in, NULL = global cloud
//   pNodeInfo   : local node info returned from PeerGraphGetNodeInfo
//
// Returns:  HRESULT
//
HRESULT PnrpRegister(PCWSTR pwzIdentity, PCWSTR pwzName, 
                     PCWSTR pwzCloud, __in PEER_NODE_INFO* pNodeInfo)
{
    HRESULT         hr = S_OK;
    CSADDR_INFO*    pCsaAddr = NULL;
    PNRPINFO        pnrpInfo = {0};
    BLOB            blPnrpData = {0};
    WSAQUERYSET     querySet = {0};
    ULONG           nAddrs = 0;
    ULONG           i;
    INT             iRet;

    
    //
    // build a CSADDR_INFO array from the PEER_NODE_INFO
    //
    if (pNodeInfo->cAddresses < 1)
    {
        // need at least one address to register
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        // do not attempt to register more addresses than PNRP allows
        if (pNodeInfo->cAddresses > PNRP_MAX_ENDPOINT_ADDRESSES)
        {
            nAddrs = PNRP_MAX_ENDPOINT_ADDRESSES;
        }
        else
        {
            nAddrs = pNodeInfo->cAddresses;
        }

        pCsaAddr = (CSADDR_INFO*)malloc(sizeof(CSADDR_INFO) * nAddrs);
        if (pCsaAddr != NULL)
        {
            // copy the addresses from PEER_NODE_INFO into CSADDR_INFO format
            for (i = 0; i < nAddrs; i++)
            {
                pCsaAddr[i].iProtocol = IPPROTO_TCP;
                pCsaAddr[i].iSocketType = SOCK_STREAM;
                pCsaAddr[i].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_IN6);
                pCsaAddr[i].LocalAddr.lpSockaddr = (LPSOCKADDR) &pNodeInfo->pAddresses[i].sin6;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        //
        // build the WSAQUERYSET required to register
        //
        pnrpInfo.dwSize = sizeof(pnrpInfo);
        pnrpInfo.dwLifetime = REGISTRATION_LIFETIME;
        pnrpInfo.lpwszIdentity = (PWSTR) pwzIdentity;

        blPnrpData.cbSize = sizeof(pnrpInfo);
        blPnrpData.pBlobData = (BYTE*)&pnrpInfo;

        querySet.dwSize = sizeof(querySet);
        querySet.dwNameSpace = NS_PNRPNAME;
        querySet.dwNumberOfCsAddrs = nAddrs;
        querySet.lpServiceClassId = (LPGUID)&SVCID_PNRPNAME;
        querySet.lpszServiceInstanceName = (PWSTR) pwzName;
        querySet.lpszContext = (PWSTR) pwzCloud;
        querySet.lpszComment = L"GraphChatMember";
        querySet.lpcsaBuffer = pCsaAddr;
        querySet.lpBlob = &blPnrpData;

        // register the name with PNRP
        iRet = WSASetService(&querySet, RNRSERVICE_REGISTER, 0);
        if (iRet != 0)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    if (pCsaAddr != NULL)
    {
        free(pCsaAddr);
    }

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
HRESULT PnrpResolve(PCWSTR pwzName, PCWSTR pwzCloud, __out SOCKADDR_IN6* pAddr)
{
    HRESULT         hr = S_OK;
    PNRPINFO        pnrpInfo = {0};
    BLOB            blPnrpData = {0};
    WSAQUERYSET     querySet = {0};
    WSAQUERYSET*    pResults = NULL;
    WSAQUERYSET     tempResultSet = {0};
    HANDLE          hLookup = NULL;
    BOOL            fFound = FALSE;
    DWORD           dwError;
    INT             iRet;
    ULONG           i;
    DWORD           dwSize = 0;

    //
    // fill in the WSAQUERYSET
    //
    pnrpInfo.dwSize = sizeof(pnrpInfo);
    pnrpInfo.nMaxResolve = 1;
    pnrpInfo.dwTimeout = 30;
    pnrpInfo.enResolveCriteria = PNRP_RESOLVE_CRITERIA_NON_CURRENT_PROCESS_PEER_NAME;

    blPnrpData.cbSize = sizeof(pnrpInfo);
    blPnrpData.pBlobData = (BYTE*)&pnrpInfo;

    querySet.dwSize = sizeof(querySet);
    querySet.dwNameSpace = NS_PNRPNAME;
    querySet.lpServiceClassId = (LPGUID)&SVCID_PNRPNAME;
    querySet.lpszServiceInstanceName = (PWSTR) pwzName;
    querySet.lpszContext = (PWSTR) pwzCloud;
    querySet.lpBlob = &blPnrpData;
    
    // start resolve
    iRet = WSALookupServiceBegin(
            &querySet,
            LUP_RETURN_NAME | LUP_RETURN_ADDR | LUP_RETURN_COMMENT,
            &hLookup);

    if (iRet != 0)
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
    }

    if (SUCCEEDED(hr))
    {
        dwSize = sizeof(tempResultSet);

        // retrieve the required size
        iRet = WSALookupServiceNext(hLookup, 0, &dwSize, &tempResultSet);
        dwError = WSAGetLastError();

        if (dwError == WSAEFAULT)
        {
            // allocate space for the results
            pResults = (WSAQUERYSET*)malloc(dwSize);
            if (pResults == NULL)
            {
                hr = HRESULT_FROM_WIN32(WSAGetLastError());
            }
        }
        else
        {        
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

    if (SUCCEEDED(hr))
    {
        // retrieve the addresses
        iRet = WSALookupServiceNext(hLookup, 0, &dwSize, pResults);
        if (iRet != 0)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        // return the first IPv6 address found
        for (i = 0; i < pResults->dwNumberOfCsAddrs; i++)
        {
            if (pResults->lpcsaBuffer[i].iProtocol == IPPROTO_TCP &&
                pResults->lpcsaBuffer[i].RemoteAddr.iSockaddrLength == sizeof(SOCKADDR_IN6))
            {
                CopyMemory(pAddr, pResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr, sizeof(SOCKADDR_IN6));
                fFound = TRUE;
                break;
            }
        }

        if (!fFound)
        {
            // unable to find an IPv6 address
            hr = HRESULT_FROM_WIN32(WSA_E_NO_MORE);
        }
    }

    if (hLookup != NULL)
    {
        WSALookupServiceEnd(hLookup);
    }
    
    free(pResults);
    return hr;
}

//-------------------------------------------------------------------------
// Function: PnrpUnregister
//
// Purpose:  Unregister the given name from the PNRP cloud
//
// Arguments:
//   pwzIdentity : identity string originally used to register the name
//   pwzName     : name to unregister from PNRP
//   pwzCloud    : name of cloud to unregister from, NULL = global cloud
//
// Returns:  HRESULT
//
HRESULT PnrpUnregister(PCWSTR pwzIdentity, PCWSTR pwzName, PCWSTR pwzCloud)
{
    HRESULT         hr = S_OK;
    PNRPINFO        pnrpInfo = {0};
    BLOB            blPnrpData = {0};
    WSAQUERYSET     querySet = {0};
    INT             iRet;

    //
    // build the WSAQUERYSET required to unregister
    //
    pnrpInfo.dwSize = sizeof(pnrpInfo);
    pnrpInfo.dwLifetime = REGISTRATION_LIFETIME;
    pnrpInfo.lpwszIdentity = (PWSTR) pwzIdentity;

    blPnrpData.cbSize = sizeof(pnrpInfo);
    blPnrpData.pBlobData = (BYTE*)&pnrpInfo;

    querySet.dwSize = sizeof(querySet);
    querySet.dwNameSpace = NS_PNRPNAME;
    querySet.lpServiceClassId = (LPGUID)&SVCID_PNRPNAME;
    querySet.lpszServiceInstanceName = (PWSTR) pwzName;
    querySet.lpszContext = (PWSTR) pwzCloud;
    querySet.lpBlob = &blPnrpData;

    // unregister the name with PNRP
    iRet = WSASetService(&querySet, RNRSERVICE_DELETE, 0);
    if (iRet != 0)
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
    }

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
				}
			}
			else
			{
				hr = HRESULT_FROM_WIN32(dwErr);
			}
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

    if (hLookup != NULL)
    {
        (void) WSALookupServiceEnd(hLookup);
    }

    free(pResults);
    return hr;
}


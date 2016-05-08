/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

#define UNICODE

#include <windows.h>
#include <winbase.h>
#include <mprapi.h>
#include <rtinfo.h>

#include "ipsamplerm.h" // This include file is shipped with the Sample Routing protocol

// Define the Protocol relevant information here
IPSAMPLE_GLOBAL_CONFIG    igc = {0};
#define YOUR_PROTOCOL_INFO_BLOCK_DATA    (&igc)
#define YOUR_PROTOCOL_ID                        0x00c8
#define YOUR_PROTOCOL_BLOCK_SIZE            0x0004



HRESULT AddProtocolToServer(LPCWSTR pswzServerName)
{
    HRESULT     hr = S_OK;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwErrT = ERROR_SUCCESS;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    HANDLE   hMprConfig = NULL;
    LPBYTE      pByte = NULL;
    LPVOID        pHeader = NULL;
    LPVOID        pNewHeader = NULL;
    DWORD       dwSize = 0;
    HANDLE      hTransport = NULL;
    

    // Connect to the server
    // ----------------------------------------------------------------
    dwErr = MprAdminServerConnect((LPWSTR) pswzServerName, &hMprServer);
    if (dwErr == ERROR_SUCCESS)
    {
        // Ok, get the infobase from the server
        // ------------------------------------------------------------
        dwErr = MprAdminTransportGetInfo(hMprServer,
                                         PID_IP,
                                         &pByte,
                                         &dwSize,
                                         NULL,
                                         NULL);

        if (dwErr == ERROR_SUCCESS)
        {
            // Call MprInfoDuplicate to create a duplicate of
            // the infoblock
            // --------------------------------------------------------
            MprInfoDuplicate(pByte, &pHeader);
            MprAdminBufferFree(pByte);
            pByte = NULL;
            dwSize = 0;
        }
    }

    // We also have to open the hMprConfig, but we can ignore the error
    // ----------------------------------------------------------------
    dwErrT = MprConfigServerConnect((LPWSTR) pswzServerName, &hMprConfig);
    if (dwErrT == ERROR_SUCCESS)
    {
        dwErrT = MprConfigTransportGetHandle(hMprConfig, PID_IP, &hTransport);
    }

    if (dwErr != ERROR_SUCCESS)
    {
        // Ok, try to use the MprConfig calls.
        // ------------------------------------------------------------
        MprConfigTransportGetInfo(hMprConfig,
                                  hTransport,
                                  &pByte,
                                  &dwSize,
                                  NULL,
                                  NULL,
                                  NULL);
        
        // Call MprInfoDuplicate to create a duplicate of
        // the infoblock
        // ------------------------------------------------------------
        MprInfoDuplicate(pByte, &pHeader);
        MprConfigBufferFree(pByte);
        pByte = NULL;
        dwSize = 0;
    }

    // Call MprInfoBlockRemove to remove the old protocol block
    MprInfoBlockRemove(pHeader, YOUR_PROTOCOL_ID, &pNewHeader);

    // Did we remove the block?
    if (pNewHeader != NULL)
    {
        // The block was found and removed, so use the new header.
        MprInfoDelete(pHeader);
        pHeader = pNewHeader;
        pNewHeader = NULL;
    }

    // Add protocol to the infoblock here!    
    MprInfoBlockAdd(pHeader,
                    YOUR_PROTOCOL_ID,
                    YOUR_PROTOCOL_BLOCK_SIZE,
                    1,
                    (LPBYTE) YOUR_PROTOCOL_INFO_BLOCK_DATA,
                    &pNewHeader);
    MprInfoDelete(pHeader);
    pHeader = NULL;
        
    
    if (hMprServer)
    {
        
        MprAdminTransportSetInfo(hMprServer, 
                                 PID_IP, 
                                 (BYTE*)pNewHeader, 
                                 MprInfoBlockQuerySize(pNewHeader),
                                 NULL,
                                 0);
    }
    
    if (hMprConfig && hTransport)
    {
        
        MprConfigTransportSetInfo(hMprConfig,
                                  hTransport,
                                  (BYTE*)pNewHeader,
                                  MprInfoBlockQuerySize(pNewHeader),
                                  NULL,
                                  0,
                                  NULL);
    }


    if (pHeader)
        MprInfoDelete(pHeader);
    
    if (pNewHeader)
        MprInfoDelete(pNewHeader);
    
    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);
        
    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    return hr;
}

void main()
{
    //The first parameter is the name of the server. to execute on the local machine pass in NULL
    // as shown
    AddProtocolToServer(NULL);
}

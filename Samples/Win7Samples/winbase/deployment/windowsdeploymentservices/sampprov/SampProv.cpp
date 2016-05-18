/*++

    Copyright (c) 2005 Microsoft Corporation

Module Name:

    SampProv.cpp

Abstract:

    Implements a WDS PXE Provider which is capable is servicing clients 
    requesting PXE boot.

Environment:

    User Mode

--*/

#include "SampProv.h"

//
// Handle to Sample Provider DLL.
//
HANDLE g_hInstance = NULL;

//
// Handle to Sample Provider (provided by WDS PXE).
//
HANDLE g_hSampleProvider = NULL;

//
// Name of configuration file which is used to service clients.
//
WCHAR g_wszConfigurationFile[MAX_PATH] = { 0 };

//
// Default Boot Program.
//
char g_szDefaultBootProgram[PXE_DHCP_FILE_SIZE] = { 0 };

//
// Default BCD File.
//
char g_szDefaultBcdFile[PXE_DHCP_FILE_SIZE] = { 0 };

//
// Server Name.
//
char g_szServerName[PXE_DHCP_SERVER_SIZE] = { 0 };

BOOL 
WINAPI 
DllMain(
  __in HANDLE hInstance, 
  __in DWORD dwReason, 
  __in LPVOID pReserved
)
/*++

Routine Description:

    It is called by the system when processes and threads are initialized and 
    terminated, or on calls to the LoadLibrary and FreeLibrary functions. 

Arguments:

    hInstance   -   The value is the base address of the DLL.
    dwReason    -   Specifies a flag indicating why the DLL entry-point function 
                    is being called. 
    pReserved   -   Specifies further aspects of DLL initialization and cleanup.                   

Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        //
        // Save module instance.
        //
        g_hInstance = hInstance;
    }

    return TRUE;
}


DWORD
PXEAPI
PxeProviderInitialize(
    __in HANDLE hProvider,
    __in HKEY hProviderKey
)
/*++

Routine Description:

     This function is called by WDS PXE to initialize Sample Provider.

Arguments:

    hProvider       -   Handle to Provider.
    hProviderKey    -   Handle to registry store where Provider should store
                        its configuration information.

Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    ULONG uFilter = PXE_PROV_FILTER_PXE_ONLY;

    UNREFERENCED_PARAMETER(hProviderKey);

    //
    // Read Policy Settings.
    //
    dwError = InitializeConfiguration();
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Register Shutdown Callback.
    //
    dwError = PxeRegisterCallback(hProvider,
                                  PXE_CALLBACK_SHUTDOWN,
                                  PxeProviderShutdown,
                                  NULL);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Register Request Processing Callback.
    //
    dwError = PxeRegisterCallback(hProvider,
                                  PXE_CALLBACK_RECV_REQUEST,
                                  PxeProviderRecvRequest,
                                  NULL);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);
    
    //
    // Define filter to only receive requests which are valid Dhcp Packets and
    // contain Option 60 'PXEClient'.
    //
    dwError = PxeProviderSetAttribute(hProvider,
                                      PXE_PROV_ATTR_FILTER,
                                      &uFilter,
                                      sizeof(uFilter));
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Save Provider Handle.
    //
    g_hSampleProvider = hProvider;
    
Cleanup:
    return dwError;
}

DWORD
PXEAPI
PxeProviderShutdown(
    __in PVOID pContext
)
/*++

Routine Description:

    This function is registered as callback for PXE_CALLBACK_SHUTDOWN by 
    Provider and is called by WDSPXE when it needs to shutdown the Provider.

Arguments:

    pContext    -   Context which was passed to WDSPXE when callback was 
                    registered.

Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    UNREFERENCED_PARAMETER(pContext);
    return ERROR_SUCCESS;
}

DWORD
PXEAPI
PxeProviderRecvRequest(
    __in HANDLE hClientRequest,
    __in PVOID pPacket,
    __in ULONG uPacketLen,
    __in PXE_ADDRESS *pLocalAddress,
    __in PXE_ADDRESS *pRemoteAddress,
    __out PXE_BOOT_ACTION *pAction,
    __in PVOID pContext
)
/*++

Routine Description:

    This function is called by WDS PXE when a request is received. All requests
    are processed synchronously by Sample Provider.

Arguments:

    hClientRequest  -   Handle to received request.
    pPacket         -   Pointer to received data.
    uPacketLen      -   Length, in bytes, of pPacket.
    pLocalAddress   -   Local Address on which this request was received.
    pRemoteAddress  -   Remote Address of client who sent the request.
    pAction         -   [out] Contains the next action that should be taken by
                        WDS PXE.
    pContext        -   Context which was passed to WDSPXE when callback was 
                        registered.

Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    WCHAR wszMacAddress[MAX_MAC_ADDR_STRING_LEN];
    WCHAR wszBootProgram[PXE_DHCP_FILE_SIZE];
    char szBootProgram[PXE_DHCP_FILE_SIZE];
    WCHAR wszBcdFile[PXE_DHCP_FILE_SIZE];
    char szBcdFile[PXE_DHCP_FILE_SIZE];
    LPCSTR pszBootProgram = NULL;
    LPCSTR pszBcdFile = NULL;
    PVOID pReplyPacket = NULL;
    PPXE_DHCP_MESSAGE pReplyMessage = NULL;
    ULONG uReplyPacketLen = 0;
    BYTE bOptionValue = 0;
    PXE_ADDRESS DestinationAddr = { 0 };
    HANDLE hOptionHandle = NULL;
    ULONG uBufferLenActual = 0;
    PVOID pBuffer = NULL;
    size_t cchOptionLen = 0;
    UNREFERENCED_PARAMETER(pContext);

    //
    // set PXE_BOOT_ACTION as ignore by default
    //
    
    *pAction=PXE_BA_IGNORE;

    //
    // Convert client MAC address to string.
    //
    
    dwError = GetClientMacAddress(pPacket, 
                                  wszMacAddress);
    
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Check if the MAC address exists.
    //
    
    if (GetPrivateProfileString(DEVICES_SECTION,
                                wszMacAddress,
                                NULL,
                                wszBootProgram,
                                NUMELEM(wszBootProgram),
                                g_wszConfigurationFile))
    {
        if (!WideCharToMultiByte(CP_ACP,
                                 WC_NO_BEST_FIT_CHARS,
                                 wszBootProgram,
                                 -1,
                                 szBootProgram,
                                 NUMELEM(szBootProgram),
                                 NULL,
                                 NULL))
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        pszBootProgram = szBootProgram;
    }
    else
    {
        pszBootProgram = g_szDefaultBootProgram;
    }

    //
    // Check if the BCD file is specified for the given MAC address
    //
    
    if (GetPrivateProfileString(BCD_SECTION,
                                wszMacAddress,
                                NULL,
                                wszBcdFile,
                                NUMELEM(wszBcdFile),
                                g_wszConfigurationFile))
    {
        if (!WideCharToMultiByte(CP_ACP,
                                 WC_NO_BEST_FIT_CHARS,
                                 wszBcdFile,
                                 -1,
                                 szBcdFile,
                                 NUMELEM(szBcdFile),
                                 NULL,
                                 NULL))
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        pszBcdFile = szBcdFile;
    }
    else
    {
        pszBcdFile = g_szDefaultBcdFile;
    }

    //
    // Allocate Reply Packet.
    //
    
    pReplyPacket = PxePacketAllocate(g_hSampleProvider,
                                     hClientRequest,
                                     DHCP_REPLY_PACKET_SIZE);
    if (pReplyPacket == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Initialize Reply Packet using the contents of packet which was received
    // from client.
    //
    
    dwError = PxeDhcpInitialize(pPacket,
                                uPacketLen,
                                pReplyPacket,
                                DHCP_REPLY_PACKET_SIZE,
                                &uReplyPacketLen);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Update Dhcp Packet Header with Boot Program, Server Name and Server Ip
    // Address.
    //
    
    pReplyMessage = (PPXE_DHCP_MESSAGE) pReplyPacket;

    //
    // Server Address.
    //

    MoveMemory(&pReplyMessage->BootstrapServerAddress,
               pLocalAddress->bAddress,
               pLocalAddress->uAddrLen);

    //
    // Boot Program.
    //

    hr = StringCchCopyA((LPSTR) pReplyMessage->BootFileName,
                        PXE_DHCP_FILE_SIZE,
                        pszBootProgram);
    if (FAILED(hr))
    {
        dwError = ERROR_BUFFER_OVERFLOW;
        goto Cleanup;
    }

    //
    // Server Name.
    //
    
    hr = StringCchCopyA((LPSTR) pReplyMessage->HostName,
                        PXE_DHCP_SERVER_SIZE,
                        g_szServerName);
    if (FAILED(hr))
    {
        dwError = ERROR_BUFFER_OVERFLOW;
        goto Cleanup;
    }

    //
    // Append Dhcp Message Type.
    //
    
    bOptionValue = DHCP_OPTION_VALUE_MESSAGE_TYPE_ACK;

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_MESSAGE_TYPE,
                                  1,
                                  &bOptionValue);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Append Client Class Info Option.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_PXE_CLIENT,
                                  NUMELEM(DHCP_OPTION_VALUE_PXE_CLIENT) - 1,
                                  DHCP_OPTION_VALUE_PXE_CLIENT);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Server Identifier. Place the address of the local interface on which
    // the request was received.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_SERVER_IDENTIFIER,
                                  (BYTE) pLocalAddress->uAddrLen,
                                  pLocalAddress->bAddress);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Construct BCD Option
    //

    dwError = WdsBpInitialize(WDSBP_PK_TYPE_BCD, 
                              &hOptionHandle);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    hr = StringCchLengthA((STRSAFE_LPCSTR)pszBcdFile, 
                           DHCP_REPLY_PACKET_SIZE, 
                           & cchOptionLen);
    if (FAILED(hr))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    dwError = WdsBpAddOption(hOptionHandle, 
                             WDSBP_OPT_BCD_FILE_PATH, 
                             (ULONG)cchOptionLen, 
                             (PVOID)pszBcdFile);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    dwError = WdsBpGetOptionBuffer(hOptionHandle, 
                                   0, 
                                   pBuffer, 
                                   &uBufferLenActual);

    if (dwError != ERROR_INSUFFICIENT_BUFFER)
    {
        W32_CLEANUP_ON_FAILURE(dwError, Cleanup);
    }

    pBuffer = new BYTE[uBufferLenActual];
    
    if (pBuffer == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    dwError = WdsBpGetOptionBuffer(hOptionHandle, 
                                   uBufferLenActual, 
                                   pBuffer, 
                                   &uBufferLenActual);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Add option to DHCP Packet.
    //

    dwError = PxeDhcpAppendOptionRaw(pReplyPacket, 
                                     DHCP_REPLY_PACKET_SIZE, 
                                     &uReplyPacketLen, 
                                     (BYTE) uBufferLenActual, 
                                     pBuffer);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);


    //
    // End Option.
    //
    
    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_END,
                                  0,
                                  NULL);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Send Reply to Client.
    //
    
    DestinationAddr.uFlags = PXE_ADDR_USE_DHCP_RULES;
    
    dwError = PxeSendReply(hClientRequest,
                           pReplyPacket,
                           uReplyPacketLen,
                           &DestinationAddr);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // update PXE_BOOT_ACTION to indicate that the provider has answered the client
    //
    
    *pAction = PXE_BA_NBP;
	
Cleanup:
    if (pReplyPacket!= NULL)
    {
        PxePacketFree(g_hSampleProvider,
                      hClientRequest,
                      pReplyPacket);
    }

    if (hOptionHandle!= NULL)
    {
         WdsBpCloseHandle(hOptionHandle);
    }

    if (pBuffer!= NULL)
    {        
         delete[] pBuffer;
    }
    
    return dwError;
}


DWORD
InitializeConfiguration()
/*++

Routine Description:

    This function constructs path to Configuration File and reads the policy.

Arguments:

    None.

Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    WCHAR wszDefaultBootProgram[PXE_DHCP_FILE_SIZE];
    WCHAR wszDefaultBcdFile[PXE_DHCP_FILE_SIZE];
    LPWSTR pwszServerName = NULL;
    ULONG uServerNameLen = 0;

    //
    // Create path to configuration file. Configuration file is placed in the
    // same directory as the Sample Provider DLL, and its name is always
    // <name of dll>.sampprov.ini.
    //
    if (!GetModuleFileName((HMODULE) g_hInstance,
                           g_wszConfigurationFile,
                           NUMELEM(g_wszConfigurationFile)))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    //
    // Append suffix to get the Configuration File name.
    //
    hr = StringCchCat(g_wszConfigurationFile,
                      NUMELEM(g_wszConfigurationFile),
                      CONFIG_FILE_SUFFIX);
    if (FAILED(hr))
    {
        dwError = ERROR_BUFFER_OVERFLOW;
        goto Cleanup;
    }

    //
    // Make sure the configuration file exists.
    //
    
    if (GetFileAttributes(g_wszConfigurationFile) == INVALID_FILE_ATTRIBUTES)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    //
    // Read the value for Default Boot Program.
    //
    if (GetPrivateProfileString(CONFIGURATION_SECTION,
                                CONFIGURATION_DEFAULT_BOOT_PROGRAM,
                                NULL,
                                wszDefaultBootProgram,
                                NUMELEM(wszDefaultBootProgram),
                                g_wszConfigurationFile) == 0 ||
        wszDefaultBootProgram[0] == 0)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

    //
    // Convert Boot Program Path to ANSI.
    //
    
    if (!WideCharToMultiByte(CP_ACP,
                             WC_NO_BEST_FIT_CHARS,
                             wszDefaultBootProgram,
                             -1,
                             g_szDefaultBootProgram,
                             NUMELEM(g_szDefaultBootProgram),
                             NULL,
                             NULL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    PxeTrace(g_hSampleProvider,
             PXE_TRACE_INFO,
             L"Default Boot Program: %S",
             g_szDefaultBootProgram);

    //
    // Read the value for Default BCD File.
    //
    
    if ((GetPrivateProfileString(CONFIGURATION_SECTION,
                                CONFIGURATION_DEFAULT_BCD_FILE,
                                NULL,
                                wszDefaultBcdFile,
                                NUMELEM(wszDefaultBcdFile),
                                g_wszConfigurationFile) == 0) ||
        (wszDefaultBootProgram[0] == 0))
    {
        dwError = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }


    //
    // Convert BCD File Path to ANSI.
    //
    if (!WideCharToMultiByte(CP_ACP,
                             WC_NO_BEST_FIT_CHARS,
                             wszDefaultBcdFile,
                             -1,
                             g_szDefaultBcdFile,
                             NUMELEM(g_szDefaultBcdFile),
                             NULL,
                             NULL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    PxeTrace(g_hSampleProvider,
             PXE_TRACE_INFO,
             L"Default BCD File: %s",
             g_szDefaultBcdFile);


    //
    // Get Server Name.
    //
    GetComputerNameEx(ComputerNameDnsFullyQualified,
                      NULL,
                      &uServerNameLen);
    if (uServerNameLen == 0)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    pwszServerName = new WCHAR[uServerNameLen];
    if (pwszServerName == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (!GetComputerNameEx(ComputerNameDnsFullyQualified,
                           pwszServerName,
                           &uServerNameLen))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    //
    // Truncate length if longer than what can fit in packet.
    //
    if (wcslen(pwszServerName) >= PXE_DHCP_SERVER_SIZE)
        pwszServerName[PXE_DHCP_SERVER_SIZE - 1] = 0;

    //
    // Convert to ansi.
    //
    if (!WideCharToMultiByte(CP_ACP,
                             WC_NO_BEST_FIT_CHARS,
                             pwszServerName,
                             -1,
                             g_szServerName,
                             NUMELEM(g_szServerName),
                             NULL,
                             NULL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }
    
Cleanup:
    if (pwszServerName)
        delete [] pwszServerName;
    
    return dwError;
}

DWORD
GetClientMacAddress(
    __in PVOID pPacket,
    __inout_ecount(MAX_MAC_ADDR_STRING_LEN) LPWSTR pwszMacAddress
)
/*++

Routine Description:

    Converts client Mac Address stored in Dhcp Packet to String. The string
    is padded with zeros to make it full 16 characeters.

Arguments:

    pPacket         -   Pointer to received packet.
    pwszMacAddress  -   [out] Contains mac address.
    
Return Value:

    ERROR_SUCCESS on success. On failure appropriate Win32 Error Code is 
    returned.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    PXE_DHCP_MESSAGE *pDhcpMessage = (PXE_DHCP_MESSAGE*) pPacket;
    BYTE bMacAddress[PXE_DHCP_HWAADR_SIZE] = { 0 };
    LPWSTR pwszPtr = pwszMacAddress;
    BYTE bValue = 0;

    //
    // Copy mac address with appropriate padded zeros.
    //
    MoveMemory(bMacAddress + PXE_DHCP_HWAADR_SIZE - pDhcpMessage->HardwareAddressLength,
               pDhcpMessage->HardwareAddress,
               pDhcpMessage->HardwareAddressLength);

    //
    // Now convert the Mac Address to string.
    //
    for(ULONG i = 0; i < PXE_DHCP_HWAADR_SIZE; i++)
    {
        bValue = bMacAddress[i] >> 4;
        *pwszPtr++ = (bValue < 10) ? (bValue + L'0') : ((bValue - 10) + L'A');

        bValue = bMacAddress[i] & 0x0F;
        *pwszPtr++ = (bValue < 10) ? (bValue + L'0') : ((bValue - 10) + L'A');
    }

    *pwszPtr = L'\0';

    return ERROR_SUCCESS;
}


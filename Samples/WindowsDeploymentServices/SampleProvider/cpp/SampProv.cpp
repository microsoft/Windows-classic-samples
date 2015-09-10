/*++

    Copyright (c) Microsoft Corporation

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
// Handle to the Sample Provider DLL.
//

HANDLE g_hInstance = NULL;

//
// Handle to the Sample Provider object provided by WDSPXE.
//

HANDLE g_hSampleProvider = NULL;

//
// The name of the configuration file which is used to determine which clients
// are recognized, as well as their boot configuration.
//

WCHAR g_wszConfigurationFile[MAX_PATH] = { 0 };

//
// The path to the default boot program.
//

char g_szDefaultBootProgram[PXE_DHCP_FILE_SIZE] = { 0 };

//
// The path to the default BCD file.
//

char g_szDefaultBcdFile[PXE_DHCP_FILE_SIZE] = { 0 };

//
// The server name string to use in reply packets.
//

char g_szServerName[PXE_DHCP_SERVER_SIZE] = { 0 };

BOOL
WINAPI
DllMain (
    _In_ HANDLE hInstance,
    _In_ DWORD dwReason,
    _In_ LPVOID pReserved
    )
/*++

Routine Description:

    This routine is called by the system when processes and threads are
    initialized and terminated, or on calls to the LoadLibrary and FreeLibrary
    functions.

Arguments:

    hInstance - The value is the base address of the DLL.

    dwReason - Specifies a flag indicating why the DLL entry-point function is
        being called.

    pReserved - Specifies further aspects of DLL initialization and cleanup.

Return Value:

    When dwReason is DLL_PROCESS_ATTACH, the return value is a boolean: TRUE to
    indicate success initializing the DLL and FALSE to indicate failure
    initializing the DLL.  For any other value of dwReason, the system ignores
    the return value.

--*/
{
    UNREFERENCED_PARAMETER(pReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        //
        // Save the DLL module handle.
        //

        g_hInstance = hInstance;
    }

    return TRUE;
}


DWORD
PXEAPI
PxeProviderInitialize (
    _In_ HANDLE hProvider,
    _In_ HKEY hProviderKey
    )
/*++

Routine Description:

     This function is called by WDS PXE to initialize Sample Provider.

Arguments:

    hProvider - Handle to Provider.

    hProviderKey - Handle to registry store where Provider should store its
        configuration information.

Return Value:

    A Win32 status code indicating the status of provider initialization.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
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
    // Define filter to only receive requests which are valid DHCP packets and
    // contain option 60 'PXEClient'.
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
PxeProviderShutdown (
    _In_ PVOID pContext
    )
/*++

Routine Description:

    This routine is registered for the PXE_CALLBACK_SHUTDOWN callback, and is
    called by WDSPXE when it needs to shutdown this provider.

Arguments:

    pContext - The context pointer which was passed to WDSPXE when the callback
        was registered.  For this implementation, the context pointer is
        ignored.

Return Value:

    A Win32 status code indicating the status of the callback.

--*/
{
    UNREFERENCED_PARAMETER(pContext);
    return ERROR_SUCCESS;
}

DWORD
PXEAPI
PxeProviderRecvRequest (
    _In_ HANDLE hClientRequest,
    _In_ PVOID pPacket,
    _In_ ULONG uPacketLen,
    _In_ PXE_ADDRESS *pLocalAddress,
    _In_ PXE_ADDRESS *pRemoteAddress,
    _Out_ PXE_BOOT_ACTION *pAction,
    _In_ PVOID pContext
    )
/*++

Routine Description:

    This routine is called by WDSPXE when a request is received. All requests
    are processed synchronously by this provider.

Arguments:

    hClientRequest - Handle to received request.

    pPacket - Pointer to received data.

    uPacketLen - Length, in bytes, of pPacket.

    pLocalAddress - Local Address on which this request was received.

    pRemoteAddress - Remote Address of client who sent the request. For this
        implementation, the remote address is ignored.

    pAction - An out parameter which contains the next action that should be
        taken by WDSPXE in order to process the request.

    pContext - The context pointer which was passed to WDSPXE when the callback
        was registered.  For this implementation, the context pointer is
        ignored.

Return Value:

    A Win32 status code indicating the status of the callback.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    WCHAR wszMacAddress[MAX_MAC_ADDR_STRING_LEN] = { 0 };
    WCHAR wszBootProgram[PXE_DHCP_FILE_SIZE] = { 0 };
    char szBootProgram[PXE_DHCP_FILE_SIZE] = { 0 };
    WCHAR wszBcdFile[PXE_DHCP_FILE_SIZE] = { 0 };
    char szBcdFile[PXE_DHCP_FILE_SIZE] = { 0 };
    LPCSTR pszBootProgram = NULL;
    LPCSTR pszBcdFile = NULL;
    PVOID pReplyPacket = NULL;
    PPXE_DHCP_MESSAGE pReplyMessage = NULL;
    ULONG uReplyPacketLen = 0;
    PVOID pOption = NULL;
    BYTE bRequestMessageType = 0;
    BYTE bResponseMessageType = 0;
    BYTE bOptionLen = 0;
    PXE_ADDRESS DestinationAddr = { 0 };
    HANDLE hOptionHandle = NULL;
    ULONG uBufferLenActual = 0;
    PVOID pBuffer = NULL;
    size_t cchOptionLen = 0;

    UNREFERENCED_PARAMETER(pRemoteAddress);
    UNREFERENCED_PARAMETER(pContext);

    //
    // Use a default action of PXE_BA_REJECTED while the message is validated
    // for purposes of this provider.  The return action PXE_BA_REJECTED is
    // used to indicate that the request was not processed by this provider but
    // WDSPXE may pass the request on to any other registered providers.
    //

    *pAction = PXE_BA_REJECTED;

    //
    // Determine the message type of the request.  Only DISCOVER and REQUEST
    // packets are processed by this provider.
    //

    dwError = PxeDhcpGetOptionValue(pPacket,
                                    uPacketLen,
                                    1,
                                    DHCP_OPTION_MESSAGE_TYPE,
                                    &bOptionLen,
                                    &pOption);

    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Validate the message option length.  It is expected to be precisely 1
    // byte.
    //

    if (bOptionLen != 1)
    {
        dwError = ERROR_INVALID_DATA;
        W32_CLEANUP_ON_FAILURE(dwError, Cleanup);
    }

    //
    // Validate the message type is one of the expected values.  The message
    // type of the response is determined as a result.
    //

    bRequestMessageType = *(PBYTE)pOption;
    switch (bRequestMessageType)
    {
    case DHCP_OPTION_VALUE_MESSAGE_TYPE_DISCOVER:
        bResponseMessageType = DHCP_OPTION_VALUE_MESSAGE_TYPE_OFFER;
        break;

    case DHCP_OPTION_VALUE_MESSAGE_TYPE_REQUEST:
        bResponseMessageType = DHCP_OPTION_VALUE_MESSAGE_TYPE_ACK;
        break;

    default:

        //
        // This provider does not recognize the message type.  Return
        // PXE_BA_REJECTED.
        //

        goto Cleanup;
    }

    //
    // Now that the message is validated, set PXE_BOOT_ACTION to ignore the
    // request by default.  This indicates that WDSPXE should ignore the
    // request without passing it on to any other registered providers.
    //
    // This default return value will be changed below if appropriate per the
    // options in the configuration file.
    //

    *pAction = PXE_BA_IGNORE;

    //
    // Convert the client MAC address in the packet to a string that is used as
    // an index into the configuration file.
    //

    dwError = GetClientMacAddress(pPacket,
                                  wszMacAddress);

    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Check if the MAC address exists in the configuration file.
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
    // Check the configuration file to determine if a BCD file is specified for
    // the given MAC address.
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
    // Allocate a reply packet.
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
    // Initialize the contents of the reply packet using the contents of the
    // packet that was received from the client.
    //

    dwError = PxeDhcpInitialize(pPacket,
                                uPacketLen,
                                pReplyPacket,
                                DHCP_REPLY_PACKET_SIZE,
                                &uReplyPacketLen);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Update the DHCP header of the reply packet with the correct boot
    // program, server name and server IP address.
    //

    pReplyMessage = (PPXE_DHCP_MESSAGE) pReplyPacket;

    //
    // Update the server address in the header.
    //

    MoveMemory(&pReplyMessage->BootstrapServerAddress,
               pLocalAddress->bAddress,
               pLocalAddress->uAddrLen);

    //
    // Update the boot program in the header.
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
    // Update the server name in the header.
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
    // Append a DHCP message type option to the option payload of the reply
    // packet.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_MESSAGE_TYPE,
                                  1,
                                  &bResponseMessageType);

    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Append the Vendor Class Identifier option which indicates a PXE server is
    // responding to the request.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_PXE_CLIENT,
                                  NUMELEM(DHCP_OPTION_VALUE_PXE_CLIENT) - 1,
                                  DHCP_OPTION_VALUE_PXE_CLIENT);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Append the Server Identifier option.  This is a value which is unique to
    // this PXE server.  The address of the local interface on which the
    // request was received is used for this purpose.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_SERVER_IDENTIFIER,
                                  (BYTE) pLocalAddress->uAddrLen,
                                  pLocalAddress->bAddress);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Construct the encapsulated BCD boot program options using the WDSBP API.
    //

    dwError = WdsBpInitialize(WDSBP_PK_TYPE_BCD,
                              &hOptionHandle);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    hr = StringCchLengthA((STRSAFE_LPCSTR)pszBcdFile,
                           DHCP_REPLY_PACKET_SIZE,
                           &cchOptionLen);
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
    // Append the encapsulated BCD options to the reply packet.
    //

    dwError = PxeDhcpAppendOptionRaw(pReplyPacket,
                                     DHCP_REPLY_PACKET_SIZE,
                                     &uReplyPacketLen,
                                     (BYTE) uBufferLenActual,
                                     pBuffer);

    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Any valid DHCP packet must have an End Option to terminate the option
    // payload of the packet.
    //

    dwError = PxeDhcpAppendOption(pReplyPacket,
                                  DHCP_REPLY_PACKET_SIZE,
                                  &uReplyPacketLen,
                                  DHCP_OPTION_END,
                                  0,
                                  NULL);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Send the reply packet to the client.
    //

    DestinationAddr.uFlags = PXE_ADDR_USE_DHCP_RULES;

    dwError = PxeSendReply(hClientRequest,
                           pReplyPacket,
                           uReplyPacketLen,
                           &DestinationAddr);
    W32_CLEANUP_ON_FAILURE(dwError, Cleanup);

    //
    // Update PXE_BOOT_ACTION to indicate that this provider has processed the
    // response and answered the client.
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
InitializeConfiguration (
    VOID
    )
/*++

Routine Description:

    This function constructs path to Configuration File and reads the policy.

Arguments:

    None.

Return Value:

    A Win32 status code that indicates the status of the operation.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    WCHAR wszDefaultBootProgram[PXE_DHCP_FILE_SIZE];
    WCHAR wszDefaultBcdFile[PXE_DHCP_FILE_SIZE];
    LPWSTR pwszServerName = NULL;
    ULONG uServerNameLen = 0;

    //
    // Create the path to the configuration file. The configuration file is
    // placed in the same directory as the Sample Provider DLL, and its name is
    // always <name of dll>.sampprov.ini.
    //

    if (!GetModuleFileName((HMODULE) g_hInstance,
                           g_wszConfigurationFile,
                           NUMELEM(g_wszConfigurationFile)))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    //
    // Append the suffix to get the full configuration file name.
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
    // Read the value for the default boot program.
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
    // Convert the boot program path to ANSI, the format expected in reply
    // packets.
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
    // Read the value for the default BCD file.
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
    // Convert the BCD file path to ANSI, the format expected in reply packets.
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
    // Get the server name from the configuration file.
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
    // Truncate the length of the server name if it is longer than what can fit
    // in a valid reply packet.
    //

    if (wcslen(pwszServerName) >= PXE_DHCP_SERVER_SIZE)
        pwszServerName[PXE_DHCP_SERVER_SIZE - 1] = 0;

    //
    // Convert the server name to ANSI, the format expected in valid reply
    // packets.
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
    delete [] pwszServerName;

    return dwError;
}

DWORD
GetClientMacAddress (
    _In_ PVOID pPacket,
    _Inout_updates_(MAX_MAC_ADDR_STRING_LEN) LPWSTR pwszMacAddress
    )
/*++

Routine Description:

    Converts the client MAC address stored in the specified DHCP packet to a
    string. The string is prepended with zeros to make it 16 characters in
    length.

Arguments:

    pPacket - Pointer to received packet.

    pwszMacAddress - An out parameter which is set to the MAC address.

Return Value:

    A Win32 status code indicating the status of the operation.

--*/
{
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
    // Now convert the MAC address to a string.
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


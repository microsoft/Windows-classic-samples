/*++

    Copyright (c) 2005 Microsoft Corporation

Module Name:

    FilterProv.cpp

Abstract:

    Implements a WDS PXE Provider which filters requests that are passed to
    lower-level WDS PXE Providers.

Environment:

    User Mode

--*/

#include "FilterProv.h"

//
// Handle to Filter Provider DLL.
//
HANDLE g_hInstance = NULL;

//
// Handle to Filter Provider.
//
HANDLE g_hFilterProvider = NULL;

//
// Name of configuration file which is used to filter clients.
//
WCHAR g_wszConfigurationFile[MAX_PATH] = { 0 };

//
// Policy (Include or Exclude).
//
ULONG g_uFilterPolicy = VAL_FILTER_POLICY_DENY;

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

    This function is called by WDS PXE to initialize Filter Provider.

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
    g_hFilterProvider = hProvider;
    
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
    are processed synchrnously by Filter Provider.

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
    WCHAR wszMacAddress[MAX_MAC_ADDR_STRING_LEN];
    WCHAR wszValue[8] = { 0 };
    ULONG uDevicePolicy = VAL_DEVICE_POLICY_DENY;
    BOOL bDeviceFound = FALSE;
    
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
                                wszValue,
                                NUMELEM(wszValue),
                                g_wszConfigurationFile))
    {
        uDevicePolicy = _wtoi(wszValue);

        switch(uDevicePolicy)
        {
        case VAL_DEVICE_POLICY_ALLOW:
        case VAL_DEVICE_POLICY_DENY:
            break;

        default:
            uDevicePolicy = VAL_DEVICE_POLICY_DENY;
            break;
        }

        bDeviceFound = TRUE;
    }

    //
    // If device was found, then use Per-Device policy to decide.
    //
    if (bDeviceFound)
    {
        if (uDevicePolicy == VAL_DEVICE_POLICY_ALLOW)
            *pAction = PXE_BA_REJECTED;
        else
            *pAction = PXE_BA_IGNORE;
    }
    //
    // Device was not found, so use global settings to decide.
    //
    else
    {
        if (g_uFilterPolicy == VAL_FILTER_POLICY_ALLOW)
            *pAction = PXE_BA_REJECTED;
        else
            *pAction = PXE_BA_IGNORE;
    }
    
Cleanup:
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
    ULONG uFilter = PXE_PROV_FILTER_PXE_ONLY;
    WCHAR wszPolicy[8] = { 0 };

    //
    // Create path to configuration file. Configuration file is placed in the
    // same directory as the Filter Provider DLL, and its name is always
    // <name of dll>.filter.ini.
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
                      FILTER_CONFIG_FILE_SUFFIX);
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
    // Read the Policy. This is defined in the 'Configuration' section, using 
    // the 'Policy' key.
    //
    if (GetPrivateProfileString(CONFIGURATION_SECTION,
                                CONFIGURATION_POLICY_KEY,
                                NULL,
                                wszPolicy,
                                NUMELEM(wszPolicy),
                                g_wszConfigurationFile) == 0 ||
        wszPolicy[0] == 0)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

    //
    // Validate Policy.
    //
    g_uFilterPolicy = _wtoi(wszPolicy);

    switch(g_uFilterPolicy)
    {
    case VAL_FILTER_POLICY_DENY:
    case VAL_FILTER_POLICY_ALLOW:
        break;

    default:
        dwError = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    //
    // Log Policy.
    //
    PxeTrace(g_hFilterProvider,
             PXE_TRACE_INFO,
             L"Filter Policy: %s",
             (g_uFilterPolicy == VAL_FILTER_POLICY_DENY) ? 
                L"Exclude" : L"Include");

Cleanup:
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

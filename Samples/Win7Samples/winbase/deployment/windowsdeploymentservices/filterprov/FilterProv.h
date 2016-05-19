/*++

    Copyright (c) 2005 Microsoft Corporation

Module Name:

    FilterProv.h

Abstract:

    Includes required header files and defines other helper data structures.

Environment:

    User Mode

--*/

#include <windows.h>
#include <wdspxe.h>
#include <stdlib.h>
#include <strsafe.h>

//
// Dhcp Options.
//
#define DHCP_OPTION_MESSAGE_TYPE             53

//
// Filter Policy.
//
#define VAL_FILTER_POLICY_DENY              0
#define VAL_FILTER_POLICY_ALLOW             1

//
// Per Device Policy.
//
#define VAL_DEVICE_POLICY_ALLOW             1
#define VAL_DEVICE_POLICY_DENY              0

//
// This string is appended to DLL name to get the file name of configuration 
// file.
//
#define FILTER_CONFIG_FILE_SUFFIX           L".filter.ini"

//
// Configuration File Sections and Keys.
//
#define CONFIGURATION_SECTION               L"Configuration"
#define DEVICES_SECTION                     L"Devices"

#define CONFIGURATION_POLICY_KEY            L"Policy"

//
// Maximum length of buffer required to store MAC address as string.
//
#define MAX_MAC_ADDR_STRING_LEN             (PXE_DHCP_HWAADR_SIZE * 2 + 1)

//
// Macros for handling errors.
//
#define W32_CLEANUP_ON_FAILURE(_Condition, _Label)                          \
                            do {                                            \
                                if ((_Condition) != (ERROR_SUCCESS))        \
                                    goto _Label;                            \
                            } while(0)

//
// Helper macros.
//
#define NUMELEM(p)                          (sizeof(p)/sizeof((p)[0]))

//
// Forward references.
//
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
);

DWORD
PXEAPI
PxeProviderShutdown(
    __in PVOID pContext
);

DWORD
InitializeConfiguration();

DWORD
GetClientMacAddress(
    __in PVOID pPacket,
    __inout_ecount(MAX_MAC_ADDR_STRING_LEN) LPWSTR pwszMacAddress
);


/*++

    Copyright (c) Microsoft Corporation

Module Name:

    SampProv.h

Abstract:

    Includes required header files and defines other helper data structures.

Environment:

    User Mode

--*/

#include <windows.h>
#include <wdspxe.h>
#include <stdlib.h>
#include <strsafe.h>
#include <wdsbp.h>

//
// DHCP option codes.  See RFC 2132.
//
#define DHCP_OPTION_MESSAGE_TYPE            53
#define DHCP_OPTION_SERVER_IDENTIFIER       54
#define DHCP_OPTION_PXE_CLIENT              60
#define DHCP_OPTION_END                     255

//
// DHCP option VALUES FOR DHCP_OPTION_MESSAGE_TYPE.  See RFC 2132 section 9.6.
//
#define DHCP_OPTION_VALUE_MESSAGE_TYPE_DISCOVER 1
#define DHCP_OPTION_VALUE_MESSAGE_TYPE_OFFER    2
#define DHCP_OPTION_VALUE_MESSAGE_TYPE_REQUEST  3
#define DHCP_OPTION_VALUE_MESSAGE_TYPE_ACK      5

//
// PXEClient Vendor Class Identifier, see PXE specfication v2.1.
//
#define DHCP_OPTION_VALUE_PXE_CLIENT        "PXEClient"

//
// Size of Reply Packet.
//
#define DHCP_REPLY_PACKET_SIZE              300

//
// This string is appended to DLL name to get the file name of configuration 
// file.
//
#define CONFIG_FILE_SUFFIX                  L".sampprov.ini"

//
// Configuration File Sections and Keys.
//
#define CONFIGURATION_SECTION               L"Configuration"
#define DEVICES_SECTION                     L"Devices"
#define BCD_SECTION                         L"BCDFiles"

#define CONFIGURATION_DEFAULT_BOOT_PROGRAM  L"DefaultBootProgram"
#define CONFIGURATION_DEFAULT_BCD_FILE      L"DefaultBcdFile"

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
// Forward declarations.
//
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
    );

DWORD
PXEAPI
PxeProviderShutdown (
    _In_ PVOID pContext
    );

DWORD
InitializeConfiguration (
    VOID
    );

DWORD
GetClientMacAddress (
    _In_ PVOID pPacket,
    _Inout_updates_(MAX_MAC_ADDR_STRING_LEN) LPWSTR pwszMacAddress
    );


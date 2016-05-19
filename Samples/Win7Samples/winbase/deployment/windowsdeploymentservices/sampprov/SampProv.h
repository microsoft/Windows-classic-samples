/*++

    Copyright (c) 2005 Microsoft Corporation

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
// Dhcp Options.
//
#define DHCP_OPTION_MESSAGE_TYPE            53
#define DHCP_OPTION_SERVER_IDENTIFIER       54
#define DHCP_OPTION_PXE_CLIENT              60
#define DHCP_OPTION_END                     255

//
// Dhcp Option Values.
//
#define DHCP_OPTION_VALUE_MESSAGE_TYPE_ACK  5
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


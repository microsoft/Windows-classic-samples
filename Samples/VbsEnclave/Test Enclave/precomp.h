/*++

Copyright (c) Microsoft Corporation

Module Name:

    precomp.h

Abstract:

    Contians imports and function prototypes for enclave.c

Author:

    Akash Trehan (aktrehan@microsoft.com) 01-30-2024

--*/

#pragma once

#include <winenclave.h>
#include <wchar.h>

// VBS enclave configuration

const IMAGE_ENCLAVE_CONFIG __enclave_config = {
    sizeof(IMAGE_ENCLAVE_CONFIG),
    IMAGE_ENCLAVE_MINIMUM_CONFIG_SIZE,
    IMAGE_ENCLAVE_POLICY_DEBUGGABLE,    // DO NOT SHIP DEBUGGABLE ENCLAVES TO PRODUCTION
    0,
    0,
    0,
    { 0xFE, 0xFE },    // family id
    { 0x01, 0x01 },    // image id
    0,                 // version
    0,                 // SVN
    0x10000000,        // size
    16,                // number of threads
    IMAGE_ENCLAVE_FLAG_PRIMARY_IMAGE
};

ULONG InitialCookie;

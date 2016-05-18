/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Install.h

Abstract:

    This h file includes sample code for installation of an nspv2
    naming provider

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#pragma once

#include <windows.h>
#include <stdlib.h>
#include "SampleProvider.h"
#include "ws2spi.h"

typedef enum _AuthLevel
{
    None = 0,
    Secondary = 1,
    Primary = 2,
} AuthLevel;

typedef struct _NAPI_DOMAIN_BLOB
{
    AuthLevel level;
    ULONG     cchDomainName;
    ULONG     OffsetNextDomainBlob;
    ULONG     OffsetThisDomainName;
} NAPI_DOMAIN_BLOB;

typedef struct _NAPI_INSTALL_BLOB
{
    ULONG   dwVersion;
    ULONG   dwReserved;
    BOOL    fSupportsWildCard;
    ULONG   cDomains;
    ULONG   OffsetFirstDomain;
} NAPI_INSTALL_BLOB;

INT DoInstall(GUID ProviderId);
VOID DoUninstall(GUID ProviderId);

ULONG GetTotalCch(
        ULONG cStrings, 
        __in_ecount(cStrings) PCWSTR *prgStrings);

INT CreateInstallationBlob(
        BOOL  fWildcard,
        ULONG cDomains,
        __in_ecount(cDomains)  PCWSTR *prgwzDomain,
        __in_ecount(cDomains)  BOOL  *prgfPrimary,
        __out ULONG *pcbBuffer,
        __out PBYTE *ppbBuffer);


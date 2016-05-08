/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Install.c

Abstract:

    This C file includes sample code for installation of an nspv2
    naming provider

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#include "install.h"

// Utility routine to calculate the number of characters in an array of strings
//
ULONG GetTotalCch(ULONG cStrings, __in_ecount(cStrings) PCWSTR *prgStrings)
{
    ULONG cch = 0;
    ULONG i = 0;

    for (i = 0; i < cStrings; i++)
    {
        cch += (ULONG) wcslen(prgStrings[i]);
    }
    return cch;
}

// Installation of a naming provider requires a "blob" of a specific format to be created.
// This function will create this blob.  This function should be reusable by all naming providers.
//
// Layout:  [All offsets are from beginning of buffer]
//
//  | INSTALL_BLOB | DOMAIN_BLOB1 | DOMAIN_BLOB2 | DomainName1 | DomainName2 |
//
INT CreateInstallationBlob(
        BOOL  fWildcard,
        ULONG cDomains,
        __in_ecount(cDomains)  PCWSTR *prgwzDomain,
        __in_ecount(cDomains)  BOOL  *prgfPrimary,
        __out ULONG *pcbBuffer,
        __out PBYTE *ppbBuffer)

{
    ULONG i = 0;
    ULONG cchDomains = GetTotalCch(cDomains, prgwzDomain);
    NAPI_INSTALL_BLOB  InstallBlob = {0};
    BYTE *pCurrent = NULL;
    ULONG cbBlob = sizeof(NAPI_INSTALL_BLOB) + cDomains * sizeof(NAPI_DOMAIN_BLOB) + 
                   cchDomains * sizeof(WCHAR);

    BYTE *pBlob = malloc(cbBlob);
    if (pBlob == NULL)
    {
        return WSA_NOT_ENOUGH_MEMORY;
    }

    // Copy install blob fields
    //
    InstallBlob.dwVersion = 1;
    InstallBlob.dwReserved = 0;
    InstallBlob.fSupportsWildCard = fWildcard;
    InstallBlob.cDomains = cDomains;
    InstallBlob.OffsetFirstDomain = sizeof(InstallBlob);

    CopyMemory(pBlob, &InstallBlob, sizeof(InstallBlob));
    pCurrent = pBlob + sizeof(InstallBlob);

    cchDomains = 0;
    
    // Serialize information about each domain supported
    //
    for (i = 0; i < cDomains; i++)
    {
        NAPI_DOMAIN_BLOB DomainBlob = {0};
        ULONG cchThisDomain = (ULONG) wcslen(prgwzDomain[i]);

        // Copy domain blob fields
        //
        DomainBlob.level = prgfPrimary[i] ? Primary : Secondary;
        DomainBlob.cchDomainName = cchThisDomain;
        DomainBlob.OffsetNextDomainBlob = sizeof(InstallBlob) + (i+1) * sizeof(NAPI_DOMAIN_BLOB);
        DomainBlob.OffsetThisDomainName = sizeof(InstallBlob) + cDomains * sizeof(NAPI_DOMAIN_BLOB) 
                                          + cchDomains * sizeof(WCHAR);

        // Copy domain blob to current buffer location
        //
        CopyMemory(pCurrent, &DomainBlob, sizeof(DomainBlob));
        pCurrent += sizeof(DomainBlob);

        // copy domain name 
        //
        CopyMemory(pBlob + DomainBlob.OffsetThisDomainName, prgwzDomain[i], cchThisDomain * sizeof(WCHAR));
        cchDomains += cchThisDomain;
    }

    *pcbBuffer = cbBlob;
    *ppbBuffer = pBlob;

    return NO_ERROR;
}

// A sample routine demonstrating how a naming provider would install itself.  Typically this would
// happen in an applications setup logic.
//
INT DoInstall(GUID ProviderId)
{
    INT err = NO_ERROR;
    ULONG cbBlob = 0;
    BYTE  *pbBlob = NULL;
    
    // Setup domain arrays such that we are Primary for the domain "sampleprovider.net"
    //
    PWSTR rgDomains[] = { L"sampleprovider.net" };
    BOOL  rgAuthoritative[] = { TRUE };

    // Create the blob
    //
    err = CreateInstallationBlob(FALSE,
                                 celems(rgDomains),
                                 rgDomains,
                                 rgAuthoritative,
                                 &cbBlob,
                                 &pbBlob);

    if (err == NO_ERROR)
    {
        // Call into winsock to install our naming provider
        //
        BLOB blob = { 0 };
        blob.cbSize = cbBlob;
        blob.pBlobData = pbBlob;
        err = WSCInstallNameSpaceEx(L"Sample Provider", L"", NS_EMAIL, 1, &ProviderId, &blob);

        free(pbBlob);
    }

    return err;
}


// Simple wrapper function to the winsock call to uninstall the namespace provider
VOID DoUninstall(GUID ProviderId)
{
    WSCUnInstallNameSpace(&ProviderId);
}


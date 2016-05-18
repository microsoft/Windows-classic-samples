/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Reg.h

Abstract:

    This h file includes sample code for registration portion of
    an nspv2 naming provider

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/
#pragma once

#include "SampleProvider.h"

#define MAX_REGISTRATIONS  100
extern CRITICAL_SECTION g_cs;

typedef struct _RegInfo
{
    PWSTR pwzEmailAddress;
    PWSTR pwzServiceName;
    ULONG cAddresses;
    CSADDR_INFO *prgAddresses;
} RegInfo;
 
INT  AddRegistration(__in LPWSAQUERYSET2 pQuerySet);
INT  RemoveRegistration(__in LPWSAQUERYSET2 pQuerySet);
INT  FindRegistration(PCWSTR pcwzEmailAddress, PCWSTR pcwzServiceName, RegInfo **ppRegInfo);
void RemoveAllRegistrations();

ULONG StringSize(PCWSTR wz);
INT CopyString(PCWSTR wzSrc, __deref_out PWSTR *pwzDest);
INT CopyAddrs(__in_ecount(dwAddrs) CSADDR_INFO *rgAddrsSrc, 
                                   DWORD dwAddrs,
              __deref_out          CSADDR_INFO **prgAddrsDest);


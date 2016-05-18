// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    wfpeventutil.h

Abstract:

    This file declares routines for managing program input through command 
    line arguments and printing program output in form of diagnostic event 
    details.

--*/


#ifndef _WFP_EVENT_UTIL_H_
#define _WFP_EVENT_UTIL_H_
#pragma once

#include <fwpmu.h>
#include <fwpmtypes.h>

DWORD
ParseArguments(
   __in int argc, 
   __in_ecount(argc) const wchar_t* const argv[],
   __out int* mins,
   __out ADDRESS_FAMILY* version,
   __out SOCKADDR_STORAGE* remoteIpAddr,
   __out BOOL* stop
);

VOID
PrintEvents(   
   __in FWPM_NET_EVENT0** matchedEvents,
   __in UINT32 numMatchedEvents
);

#endif   //_PRINT_WFP_EVENTS_H_

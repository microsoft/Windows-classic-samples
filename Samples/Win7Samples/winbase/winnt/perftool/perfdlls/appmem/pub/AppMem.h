/*++

Copyright 1995 - 2000 Microsoft Corporation

Module Name:

    appmem.h

Abstract:

    function prototypes and definitions for applications using the 
    application memory performance counters

Revision History

    30 Aug 1995     Bob Watson (a-robw)     Created

--*/
#ifndef _APPMEM_H_
#define _APPMEM_H_

#ifdef __cplusplus
extern "C" {
#endif

HGLOBAL
GlobalAllocP (
    UINT    fuFlags,
    DWORD   cbBytes
);

HGLOBAL
GlobalFreeP (
    HGLOBAL hglbMem
);

HGLOBAL
GlobalReAllocP (
    HGLOBAL hglbMem,
    DWORD   cbBytes,
    UINT    fuFlags
);

#ifdef __cplusplus
}
#endif

#endif // _APPMEM_H_


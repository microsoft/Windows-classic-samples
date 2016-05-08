/*++

Copyright (c) 1997-1999 Microsoft Corporation

Module Name:

    MSPutils.cpp

Abstract:
    
    data for msputils.h

--*/
#include "precomp.h"
#pragma hdrstop

#include "mspbase.h"
#include "msputils.h"

CMSPCritSection CMSPObjectSafetyImpl::s_CritSection;


BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
    PLIST_ENTRY pCurrent = ListHead;
    while(pCurrent->Flink != Entry)
    {
        pCurrent = pCurrent->Flink;
        if(pCurrent == 0)
        {
            return FALSE;
        }
    }
    return TRUE;
}

// eof
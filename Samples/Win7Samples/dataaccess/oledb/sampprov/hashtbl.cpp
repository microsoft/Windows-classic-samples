//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @module hashtbl.cpp | Hashing routines for row manipulation.
//
//
#include "headers.h"
#include "hashtbl.h"



//--------------------------------------------------------------------
// GetNextSlots
// 
// @func Allocates a contiguous block of the required number of slots.
//
// @rdesc Returns one of the following values:
//      @flag S_OK          | slot allocate succeeded
// 		@flag E_OUTOFMEMORY | slot allocation failed because of memory allocation
//							  problem 
///
HRESULT GetNextSlots
    (
    PLSTSLOT plstslot,  //@parm IN | slot list
    ULONG cslot,        //@parm IN | needed block size (in slots)
    ULONG* pislot       //@parm IN | handle of the first slot in the returned block
    )
{
    ULONG   islot, dslot;
    PSLOT   pslot, pslotTmp;
    ULONG   cbCommit;
    HRESULT hr;

    if (plstslot->islotRov)
        plstslot->islotRov = ((PSLOT) & plstslot->rgslot[(plstslot->islotRov * plstslot->cbSlot)-sizeof(SLOT)])->islotNext;
    else
        plstslot->islotRov = plstslot->islotFirst;

    islot = plstslot->islotRov;
    while (islot)
        {
        if (((PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)])->cslot >= cslot)
            break;
        islot = ((PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)])->islotNext;
        }
    if (islot == 0)
        {
        islot = plstslot->islotFirst;
        while (islot != plstslot->islotRov)
            {
            if (((PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)])->cslot >= cslot)
                break;
            islot = ((PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)])->islotNext;
            }
        if (islot == plstslot->islotRov)
            islot = 0;
        }


    if (islot == 0)
        {
        cbCommit = ((cslot *plstslot->cbSlot) / plstslot->cbPage + 1) *plstslot->cbPage;
        if ((plstslot->cbCommitCurrent + cbCommit) > plstslot->cbCommitMax
            || VirtualAlloc((VOID *) ((BYTE *) plstslot + plstslot->cbCommitCurrent),
                                    cbCommit,
                                    MEM_COMMIT,
                                    PAGE_READWRITE ) == NULL)
            return ResultFromScode( E_OUTOFMEMORY );

        islot = (ULONG) ((plstslot->cbCommitCurrent + plstslot->cbExtra) / plstslot->cbSlot);
        dslot = ((cbCommit + plstslot->cbslotLeftOver) / plstslot->cbSlot);
        if ((plstslot->pbitsSlot)->IsSlotSet( islot - 1 ) != NOERROR)
            {
            if ((plstslot->pbitsSlot)->FindSet( islot - 1, plstslot->islotMin, &islot ) == NOERROR)
                islot++;
            else
                islot = plstslot->islotMin;
            pslot = (PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)];
            pslot->cslot += dslot;
            DecoupleSlot( plstslot, islot, pslot );
            }
        else
            {
            pslot = (PSLOT) ((BYTE *) plstslot + plstslot->cbCommitCurrent - plstslot->cbslotLeftOver);
            pslot->cslot = dslot;
            }

        pslot->islotNext = plstslot->islotFirst;
        pslot->islotPrev = 0;

        plstslot->islotMax += dslot;
        plstslot->islotFirst       = islot;
        plstslot->cbslotLeftOver   = (cbCommit + plstslot->cbslotLeftOver) % plstslot->cbSlot;
        plstslot->cbCommitCurrent += cbCommit;

        if (pslot->islotNext)
            ((PSLOT) & plstslot->rgslot[(pslot->islotNext *plstslot->cbSlot)-sizeof(SLOT)])->islotPrev = islot;
        islot = plstslot->islotFirst;
        }

    pslot = (PSLOT) & plstslot->rgslot[(islot *plstslot->cbSlot)-sizeof(SLOT)];
    DecoupleSlot( plstslot, islot, pslot );
    if (pslot->cslot > cslot)
        {
        pslotTmp = (PSLOT) & plstslot->rgslot[ ((islot + cslot) *plstslot->cbSlot)-sizeof(SLOT)];
        pslotTmp->cslot = pslot->cslot - cslot;
        AddSlotToList( plstslot, islot + cslot, pslotTmp );
        }

    if (FAILED( hr = (plstslot->pbitsSlot)->SetSlots( islot, islot + cslot - 1 )))
        return hr;

    if (pislot)
        *pislot = islot;
    return ResultFromScode( S_OK );
}



//--------------------------------------------------------------------
// DecoupleSlot
//
// @func Decouples a slot from the list of free slots
//
// @rdesc NONE
//
VOID DecoupleSlot
    (
    PLSTSLOT plstslot,  //@parm IN | slot list
    ULONG islot,        //@parm IN | slot handle to decouple
    PSLOT pslot         //@parm IN | pointer to the slot header
    )
{
    if (pslot->islotNext)
        ((PSLOT) & plstslot->rgslot[(pslot->islotNext *plstslot->cbSlot)-sizeof(SLOT)])->islotPrev = pslot->islotPrev;
    if (pslot->islotPrev)
        ((PSLOT) & plstslot->rgslot[(pslot->islotPrev *plstslot->cbSlot)-sizeof(SLOT)])->islotNext = pslot->islotNext;
    else
        plstslot->islotFirst = pslot->islotNext;
    if (islot == plstslot->islotRov)
        plstslot->islotRov = pslot->islotNext;
}


//--------------------------------------------------------------------
// AddSlotToList
//
// @func Adds a slot to the list of free slots
//
// @rdesc NONE
//
VOID AddSlotToList
    (
    PLSTSLOT plstslot,  //@parm IN | slot list
    ULONG islot,        //@parm IN | slot handle
    PSLOT pslot         //@parm IN | pointer to the slot header
    )
{
    pslot->islotPrev = 0;
    pslot->islotNext = plstslot->islotFirst;
    plstslot->islotFirst = islot;
    if (pslot->islotNext)
        ((PSLOT) & plstslot->rgslot[(pslot->islotNext *plstslot->cbSlot)-sizeof(SLOT)])->islotPrev = islot;
}




//--------------------------------------------------------------------
// ReleaseSlots
//
// @func Releases a contiguous block of slots.
//
// @rdesc Returns one of the following values:
//      @flag   S_OK | method succeeded
//
HRESULT ReleaseSlots
    (
    PLSTSLOT plstslot,  //@parm IN | slot list
    ULONG    islot,     //@parm IN | handle of first slot to release 
    ULONG    cslot      //@parm IN | count of slots to release
    )
{
    PSLOT pslot, pslotTmp;

    (plstslot->pbitsSlot)->ResetSlots( islot, islot + cslot - 1 );
    pslot = (PSLOT) & plstslot->rgslot[(islot * plstslot->cbSlot)-sizeof(SLOT)];
    pslot->cslot = cslot;

    if (islot > plstslot->islotMin && (plstslot->pbitsSlot)->IsSlotSet( islot - 1 ) != NOERROR)
        {
        if ((plstslot->pbitsSlot)->FindSet( islot - 1, plstslot->islotMin, &islot ) == NOERROR)
            islot++;
        else
            islot = plstslot->islotMin;
        pslot = (PSLOT) & plstslot->rgslot[(islot * plstslot->cbSlot)-sizeof(SLOT)];
        pslot->cslot += cslot;
        DecoupleSlot( plstslot, islot, pslot );
        }

    if ((islot + cslot) <= plstslot->islotMax && (plstslot->pbitsSlot)->IsSlotSet( islot + cslot ) != NOERROR)
        {
        pslotTmp = (PSLOT) & plstslot->rgslot[ ((islot + cslot) *plstslot->cbSlot)-sizeof(SLOT)];
        pslot->cslot += pslotTmp->cslot;
        DecoupleSlot( plstslot, (islot + cslot), pslotTmp );
        }

    AddSlotToList( plstslot, islot, pslot );
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// InitializeSlotList
//
// @func Initializes the Slot List object
//
// @rdesc Did the initialization succeed
//      @flag S_OK          | method succeeded
//      @flag E_OUTOFMEMORY | failed, out of memory
//           
//
HRESULT InitializeSlotList
    (
    ULONG cslotMax,         //@parm IN | max number of slots
    ULONG cbSlot,           //@parm IN | slot size (row buffer size)
    ULONG cbPage,           //@parm IN | page size
    LPBITARRAY pbits,       //@parm IN | 
    PLSTSLOT* pplstslot,    //@parm OUT | pointer to slot list
    BYTE** prgslot          //@parm OUT | 
    )
{
    ULONG    cbReserve;
    BYTE     *pbAlloc;
    ULONG    cbCommitFirst;
    PLSTSLOT plstslot;
    ULONG    cslot, islotFirst;
    PSLOT    pslot;


    if (cbPage == 0)
        {
        SYSTEM_INFO sysinfo;

        GetSystemInfo( &sysinfo );
        cbPage = sysinfo.dwPageSize;
        }

    // Add in the LSTSLOT and SLOT
	cbSlot = cbSlot + (sizeof( LSTSLOT ) + sizeof( SLOT ));
	
	cbReserve = ((cslotMax *(cbSlot + (sizeof( LSTSLOT ) + sizeof( SLOT )))) / cbPage + 1) *cbPage;

    pbAlloc = (BYTE *) VirtualAlloc( NULL, cbReserve, MEM_RESERVE, PAGE_READWRITE );
    if (pbAlloc == NULL)
        return ResultFromScode( E_OUTOFMEMORY );

    cbCommitFirst = ((sizeof( LSTSLOT ) + sizeof( SLOT )) / cbPage + 1) * cbPage;
    plstslot = (PLSTSLOT) VirtualAlloc( pbAlloc, cbCommitFirst, MEM_COMMIT, PAGE_READWRITE );
    if (plstslot == NULL)
        {
        VirtualFree((VOID *) pbAlloc, 0, MEM_RELEASE );
        return ResultFromScode( E_OUTOFMEMORY );
        }

    plstslot->cbSlot          = cbSlot;
    plstslot->cbPage          = cbPage;
    plstslot->cbCommitCurrent = cbCommitFirst;
    plstslot->cbCommitMax     = cbReserve;
    plstslot->pbitsSlot       = pbits;


    if (cbSlot <= 2*(sizeof( LSTSLOT ) + sizeof( SLOT )))
        {
        islotFirst        = (sizeof( LSTSLOT ) + sizeof( SLOT )) / cbSlot + (((sizeof( LSTSLOT ) + sizeof( SLOT )) % cbSlot) ? 1 : 0);
        plstslot->cbExtra = 0;
        cslot = (ULONG) ((cbCommitFirst / cbSlot) - islotFirst);
        plstslot->cbslotLeftOver = cbCommitFirst - cbSlot * (cslot + islotFirst);
        }
    else
        {
        islotFirst        = 1;
        plstslot->cbExtra = cbSlot - (sizeof( LSTSLOT ) + sizeof( SLOT ));
        cslot = (cbCommitFirst - (sizeof( LSTSLOT ) + sizeof( SLOT ))) / cbSlot;
        plstslot->cbslotLeftOver = cbCommitFirst - (sizeof( LSTSLOT ) + sizeof( SLOT )) - cslot*cbSlot;
        }
    plstslot->rgslot = ((BYTE *) plstslot - plstslot->cbExtra);
    if (cslot)
        {
        plstslot->islotFirst = islotFirst;
        pslot = (PSLOT) & plstslot->rgslot[(islotFirst *plstslot->cbSlot)-sizeof(SLOT)];
        pslot->cslot     = cslot;
        pslot->islotNext = 0;
        pslot->islotPrev = 0;
        }
    else
        plstslot->islotFirst = 0;
    plstslot->islotMin   = islotFirst;
    plstslot->islotMax   = islotFirst + cslot - 1;
    plstslot->islotRov = plstslot->islotFirst;

    *pplstslot = plstslot;
    *prgslot   = plstslot->rgslot;
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
//  ResetSlotList
//
// @func Restore slot list to newly-initiated state
//
// @rdesc 
//  @flag S_OK  | method succeeded
//
HRESULT ResetSlotList
    (
    PLSTSLOT plstslot           //@parm IN | slot list
    )
{
    ULONG   cslot;
    PSLOT   pslot;

    cslot = (plstslot->islotMax >= plstslot->islotMin) ? (plstslot->islotMax - plstslot->islotMin + 1) : 0;
    if (cslot)
        {
        plstslot->islotFirst = plstslot->islotMin;
        pslot = (PSLOT) & plstslot->rgslot[(plstslot->islotFirst *plstslot->cbSlot)-sizeof(SLOT)];
        pslot->cslot     = cslot;
        pslot->islotNext = 0;
        pslot->islotPrev = 0;
        }
    else
        plstslot->islotFirst = 0;

    plstslot->islotRov = plstslot->islotFirst;
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// ReleaseSlotList
//
// @func Free slot list's memory
//
// @rdesc 
//  @flag S_OK  | method succeeded
//
HRESULT ReleaseSlotList
    (
    PLSTSLOT plstslot           //@parm IN | slot list 
    )
{
    if (plstslot == NULL)
        return NOERROR;

    if (plstslot->cbCommitCurrent)
        VirtualFree((VOID *) plstslot, plstslot->cbCommitCurrent, MEM_DECOMMIT );

    VirtualFree((VOID *) plstslot, 0, MEM_RELEASE );
    return ResultFromScode( S_OK );
}



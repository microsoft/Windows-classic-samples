//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module BITARRAY.CPP | This contains an implementation of a bit array
// class currently used by the Internal Buffer to mark released or
// unreleased rows.
//
//
#include "headers.h"
#include "bitarray.h"


//--------------------------------------------------------------------
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CBitArray::CBitArray
    (
    void
    )
{
    m_rgbBit       = NULL;
    m_cPageMax     = 0;
    m_cPageCurrent = 0;
    m_cslotCurrent = 0;
}


//--------------------------------------------------------------------
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CBitArray:: ~CBitArray
    (
    void
    )
{
    if (m_rgbBit)
        {
        if (m_cPageCurrent)
            VirtualFree((VOID *) m_rgbBit, m_cPageCurrent *m_cbPage, MEM_DECOMMIT );
        VirtualFree((VOID *) m_rgbBit, 0, MEM_RELEASE );
        }
}


//--------------------------------------------------------------------
// @mfunc Allocate and Initialize the array of bits
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK          | Initialization succeeded
//      @flag  E_OUTOFMEMORY | Not enough memory to allocate bit array
//
STDMETHODIMP CBitArray::FInit
    (
    ULONG cslotMax,     //@parm IN  | Maximum number of slot
    ULONG cbPage        //@parm IN  | Count of bytes per page
    )
{
    ULONG cPage;
    ULONG ib;

    cPage = (cslotMax / 8 + 1) / cbPage + 1;
    m_rgbBit = (BYTE *) VirtualAlloc( NULL, cbPage *cPage, MEM_RESERVE, PAGE_READWRITE );

    if (m_rgbBit == NULL)
        return ResultFromScode( E_OUTOFMEMORY );

    m_cPageMax = cPage;

    m_cbPage = cbPage;

    for (ib =0; ib < 8; ib++)
        m_rgbBitMask[ib] = (1 << ib);
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Set a range of bit slots
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK          | Initialization succeeded
//      @flag  E_OUTOFMEMORY | Not enough memory to allocate bit array
//
STDMETHODIMP CBitArray::SetSlots
    (
    ULONG islotFirst,   //@parm IN | First slot in range to set
    ULONG islotLast     //@parm IN | Last slot in range to set
    )
{
    ULONG islot;

    if (islotLast >=    m_cslotCurrent)
        {
        ULONG cPageAdd;

        cPageAdd = ((islotLast - m_cslotCurrent + 1) / 8 + 1) / m_cbPage + 1;

        if ((cPageAdd + m_cPageCurrent) > m_cPageMax
            || VirtualAlloc( m_rgbBit + m_cPageCurrent*m_cbPage, cPageAdd *m_cbPage, MEM_COMMIT, PAGE_READWRITE ) == NULL)
            return ResultFromScode( E_OUTOFMEMORY );

        memset( m_rgbBit + m_cPageCurrent*m_cbPage, 0x00, cPageAdd *m_cbPage );
        m_cPageCurrent += cPageAdd;
        m_cslotCurrent += cPageAdd *m_cbPage *8;
        }

    // Only do this top section
    // if we have at least 2 byte's worth of bits to set.
    // Although no real speedup until we have 3 byte's worth.
    // Note really ought to be ((ilast-ifirst+1) >= 2*8).
    // (Note could use CHAR_BIT, num bits in a char.)
    // Also optimized end cases, so nothing is done
    // if the start or end is byte aligned.
    // Need this copied into ResetSlots.

    //if((islotLast -islotFirst) > 2*sizeof(BYTE))
    if (islotLast - islotFirst > 2 * 8)
        {
        ULONG ibFirst, ibLast;
        int iFixFirst, iFixLast;

        ibFirst = islotFirst / 8;
        ibLast  = islotLast / 8;
        iFixFirst = (islotFirst % 8 != 0);  // set to 1 if first byte not totally set
        iFixLast  = (islotLast % 8 != 7);   // set to 1 if last  byte not totally set

        if (iFixFirst)
            for (islot = islotFirst; (islot / 8) == ibFirst; islot++)
                m_rgbBit[islot / 8] |= m_rgbBitMask[islot % 8];

        memset( &m_rgbBit[ibFirst + iFixFirst], 0xff, ibLast - ibFirst + 1 - iFixFirst - iFixLast );

        if (iFixLast)
            for (islot = islotLast; (islot / 8) == ibLast; islot--)
                m_rgbBit[islot / 8] |= m_rgbBitMask[islot % 8];
        }
    else
        {
        for (islot = islotFirst; islot <= islotLast; islot++)
            m_rgbBit[islot / 8] |= m_rgbBitMask[islot % 8];
        }

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Reset a range of slots
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK | Reset Succeeded
//
STDMETHODIMP CBitArray::ResetSlots
    (
    ULONG islotFirst,   //@parm IN | First slot in range to reset
    ULONG islotLast     //@parm IN | Last slot in range to reset
    )
{
    ULONG ibFirst, ibLast, islot;

    if (islotFirst < m_cslotCurrent)
        {
        if (islotLast >=    m_cslotCurrent)
            islotLast = m_cslotCurrent - 1;

        if ((islotLast - islotFirst) > 2*8)
            {
            ibFirst = islotFirst / 8;
            ibLast  = islotLast / 8;
            for (islot = islotFirst; (islot / 8) == ibFirst; islot++)
                m_rgbBit[islot / 8] &= ~m_rgbBitMask[islot % 8];
            memset( &m_rgbBit[ibFirst + 1], 0x00, (ULONG)(ibLast - ibFirst - 1) );
            for (islot = islotLast; (islot / 8) == ibLast; islot--)
                m_rgbBit[islot / 8] &= ~m_rgbBitMask[islot % 8];
            }
        else
            {
            for (islot = islotFirst; islot <= islotLast; islot++)
                m_rgbBit[islot / 8] &= ~m_rgbBitMask[islot % 8];
            }
        }

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Determines if any bits are set
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK     | Array is Empty
//      @flag  S_FALSE  | Array contains set bits
//
STDMETHODIMP CBitArray::ArrayEmpty
    (
    void
    )
{
    if (m_cPageCurrent)
        {
        ULONG idw, cdw, *rgdw;

        cdw = m_cPageCurrent * (m_cbPage / sizeof( ULONG ));
        rgdw = (ULONG *) m_rgbBit;
        for (idw =0; idw < cdw; idw++)
            if (rgdw[idw])
                return ResultFromScode( S_FALSE );
        }
    return ResultFromScode( S_OK );
}



//--------------------------------------------------------------------
// @mfunc Determine if a particular bit slot is set
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK          | Slot is set
//      @flag  E_OUTOFMEMORY | Slot is not set
//
STDMETHODIMP CBitArray::IsSlotSet
    (
    ULONG islot   //@parm IN | Bit slot to check
    )
{
    if (islot >= m_cslotCurrent || (m_rgbBit[islot / 8] & m_rgbBitMask[islot % 8]) == 0x00)
        return ResultFromScode( S_FALSE );  // not set
    else
        return ResultFromScode( S_OK ); // is set
}


//--------------------------------------------------------------------
// @mfunc Find the first set slot within the bit array given a starting
// position
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK          | Initialization succeeded
//      @flag  E_OUTOFMEMORY | Not enough memory to allocate bit array
//
STDMETHODIMP CBitArray::FindSet
    (
    ULONG islotStart,   //@parm IN | Starting slot to search from
    ULONG islotLimit,   //@parm IN | Number of slots to check
    ULONG *pislot       //@parm OUT | Index of first set slot
    )
{
    ULONG ibStart, ibLimit, idwStart, idwLimit, ibEnd, ib, islot, islotEnd, idw, *pdw;

    if (islotStart > islotLimit)
        {
        ibStart  = islotStart / 8;
        ibLimit  = islotLimit / 8;
        if ((ibStart - ibLimit) > 1)
            {
            islotEnd = ibStart*8;
            for (islot = islotStart; islot >= islotEnd; islot--)
                if (m_rgbBit[islot / 8] & m_rgbBitMask[islot % 8])
                    {
                    *pislot = islot;
                    return ResultFromScode( S_OK );
                    }
            idwStart = islotStart / 32;
            idwLimit = islotLimit / 32;
            if (idwStart - idwLimit > 1)
                {
                ibEnd = idwStart*4;
                for (ib = ibStart - 1; ib >= ibEnd; ib--)
                    if (m_rgbBit[ib])
                        {
                        islot = ib*8 + 7;
                        goto Found1;
                        }
                for (pdw = (ULONG *) & m_rgbBit[ (idwStart - 1) *4], idw = idwStart - 1; idw > idwLimit; idw--, pdw--)
                    if (*pdw)
                        {
                        islot = idw*32 + 31;
                        goto Found1;
                        }
                ib = (idwLimit*4 + 3);
                }
            else
                ib = ibStart - 1;
            for (; ib > ibLimit; ib--)
                if (m_rgbBit[ib])
                    {
                    islot = ib*8 + 7;
                    goto Found1;
                    }
            islot = (ibLimit*8 + 7);
            }
        else
            islot = islotStart;

        Found1:
        for (; islot >= islotLimit; islot--)
            if (m_rgbBit[islot / 8] & m_rgbBitMask[islot % 8])
                {
                *pislot = islot;
                return ResultFromScode( S_OK );
                }
        return ResultFromScode( S_FALSE );  // not found
        }
    else
        return ResultFromScode( E_FAIL );
}

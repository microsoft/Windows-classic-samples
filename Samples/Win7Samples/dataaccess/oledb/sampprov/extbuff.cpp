//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module EXTBUFF.CPP | This class provides an array-based access to
// a big block of memory. You pass in the fixed size of each element
// to the constructor. Get (read)  an element with GetItemOfExtBuffer.
// Set (write) an element with InsertIntoExtBuffer.
// You must also pass in the system page size (found with GetSystemInfo).
// 'hItem' is the index into the array, beginning with 1.
// The array always contains m_cItem elements.
//
// GetItemOfExtBuffer returns element hItem, where hItem=1 is the first element.
// (Copies block to caller's memory.)
//
// InsertIntoExtBuffer always appends elements to the tail of the array,
// and returns the index of the newly appended element.
// (So adding first element will return 1.)
//
//
#include "headers.h"
#include "extbuff.h"


//--------------------------------------------------------------------
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CExtBuffer::CExtBuffer
    (
    void
    )
{
    m_cItem   = 0;
    m_cbAlloc = 0;
    m_rgItem  = NULL;
}


//--------------------------------------------------------------------
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CExtBuffer:: ~CExtBuffer
    (
    void
    )
{
    if (m_cbAlloc)
        VirtualFree((VOID *) m_rgItem, m_cbAlloc, MEM_DECOMMIT );

    if (m_rgItem)
        VirtualFree((VOID *) m_rgItem, 0, MEM_RELEASE );
}


//--------------------------------------------------------------------
// @mfunc Allocate and Initialize Buffer
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK | Initialization succeeded
//      @flag  E_OUTOFMEMORY | Not enough memory to allocate buffer
//
STDMETHODIMP CExtBuffer::FInit
    (
    ULONG cItemMax,     //@parm IN | Maximum number of items ever
    ULONG cbItem,       //@parm IN | Size of each item, in bytes
    ULONG cbPage        //@parm IN | Size of system page size (from SysInfo)
    )
{
    BYTE  *pb;

    m_cbReserved = ((cbItem *cItemMax) / cbPage + 1) *cbPage;
    m_rgItem = (BYTE *) VirtualAlloc( NULL, m_cbReserved, MEM_RESERVE, PAGE_READWRITE );

    if (m_rgItem == NULL)
        return ResultFromScode( E_OUTOFMEMORY );

    m_cbItem  = cbItem;
    m_dbAlloc = (cbItem / cbPage + 1) *cbPage;
    pb = (BYTE *) VirtualAlloc( m_rgItem, m_dbAlloc, MEM_COMMIT, PAGE_READWRITE );
    if (pb == NULL)
        {
        VirtualFree((VOID *) m_rgItem, 0, MEM_RELEASE );
        m_rgItem = NULL;
        return ResultFromScode( E_OUTOFMEMORY );
        }

    m_cbAlloc = m_dbAlloc;
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Retrieves a pointer to the value at given index
//
// @rdesc If index is within the range of 1 to m_cItems then a valid
// pointer is returned, else NULL is returned.
//
void* CExtBuffer::operator[]
    (
    DBCOUNTITEM hItem          //@parm IN | Index of element in buffer
    )
{
    // Return ptr to element [n], where n = 1...m_cItem.
    // Returns NULL if 'n' is out of range.
    //
    // You must use InsertIntoExtBuffer to add new elements.
    // Thereafter you can use this operator to retrieve the address of the item.
    // (You can't delete an element, but you can overwrite its space.)

    if (1 <= hItem && hItem <= m_cItem)
        return m_rgItem + (hItem - 1) *m_cbItem;
    else
        return NULL;
}


//--------------------------------------------------------------------
// @mfunc Add Data to the fixed buffers and return the index it was
// added at.
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK | Data copied successfully
//      @flag  E_OUTOFMEMORY | Not enough memory to allocate buffer
//
STDMETHODIMP CExtBuffer::InsertIntoExtBuffer
    (
    VOID* pvItem,       //@parm IN | Pointer to buffer to copy
    HACCESSOR &hItem    //@parm OUT | Index of where data was placed
    )
{
    ULONG cbOffset;

    cbOffset = m_cItem*m_cbItem;
    if ((cbOffset + m_cbItem) > m_cbAlloc)
        {
        BYTE *pb;

        if ((m_cbAlloc + m_dbAlloc) > m_cbReserved)
            pb = NULL;
        else
            pb = (BYTE *) VirtualAlloc( m_rgItem + m_cbAlloc,
                                        m_dbAlloc,
                                        MEM_COMMIT,
                                        PAGE_READWRITE );
        if (pb == NULL)
            {
            return ResultFromScode( E_OUTOFMEMORY );
            }
        m_cbAlloc += m_dbAlloc;
        }

    memcpy((m_rgItem + cbOffset), (BYTE *) pvItem, m_cbItem );
    m_cItem++;
    hItem = m_cItem;

    return ResultFromScode( S_OK );
}



//--------------------------------------------------------------------
// @mfunc Obtain a pointer to the data at a given index into the buffer
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK | pvItem contains a pointer to the data requested
//      @flag  E_INVALIDARG | Invalid Index passed in
//
STDMETHODIMP CExtBuffer::GetItemOfExtBuffer
    (
    HACCESSOR hItem,        //@parm IN | Index of item to get
    VOID*     pvItem        //@parm OUT | Pointer to block at index
    )
{
    if ((hItem > m_cItem) || (hItem == 0) ) {
        *((PACCESSOR *)pvItem) = NULL;
		return ResultFromScode( DB_E_BADACCESSORHANDLE );
	}

    memcpy((BYTE *) pvItem, (m_rgItem + (hItem - 1) *m_cbItem), m_cbItem );
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Get the extents of the currently allocated buffers
//
// @rdesc HRESULT indicating routines status
//      @flag  S_OK | Extents were obtained successfuly
//
STDMETHODIMP CExtBuffer::GetFirstLastItemH
    (
    HACCESSOR &hItemFirst,      //@parm OUT | First item allocated
    HACCESSOR &hItemLast        //@parm OUT | Last item allocated
    )
{
    hItemFirst = 1;
    hItemLast  = m_cItem;
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module FILEIDX.CPP | This module contains the index array code
// for a Comma Seperated Value (CSV) Simple Provider.
//
//
#include "headers.h"
#include "fileidx.h"
#include <stdlib.h>

static const int ARRAY_INIT_SIZE = 1000;
static const int DELETED_ROW = 1;

//--------------------------------------------------------------------
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CFileIdx::CFileIdx
    (
    void
    )
{
    m_rgDex = NULL;
    m_ulDexCnt = 0;
}


//--------------------------------------------------------------------
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CFileIdx:: ~CFileIdx()
{
	SAFE_FREE(m_rgDex);
}


//--------------------------------------------------------------------
// @mfunc Initialization routine
//
// @rdesc BOOLEAN value
//      @flag TRUE | Succeeded
//      @flag FALSE | Failed to Initialize
//
BOOL CFileIdx::fInit
    (
    void
    )
{
    //Allocate Offset Array
    m_rgDex = (LPFILEDEX) PROVIDER_ALLOC( ARRAY_INIT_SIZE * sizeof( FILEDEX ) );
    if (NULL == m_rgDex)
        return FALSE;

    m_ulDexCnt = ARRAY_INIT_SIZE;
    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc ReAllocation of Current Index Array.  This can be called to
// increase the array of index structures.
//
// @rdesc BOOLEAN value
//      @flag TRUE | Succeeded
//      @flag FALSE | Failed to Initialize
//
BOOL CFileIdx::ReAlloc
    (
    DBCOUNTITEM ulRows                //@parm IN | Number of rows to increase array for.
    )
{
    VOID* pDex;

    // Change the array size
    pDex = PROVIDER_REALLOC( m_rgDex, ((m_ulDexCnt + ulRows) * sizeof( FILEDEX )));
    if( !pDex )
        return FALSE;

	m_ulDexCnt += ulRows;
    m_rgDex = (FILEDEX*) pDex;
    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Set the offset into the file in bytes for a particular row.
// If the Index is beyond current allocation, reallocation is attempted.
//
// @rdesc BOOLEAN value
//      @flag TRUE | Succeeded
//      @flag FALSE | Failed to Initialize
//
BOOL CFileIdx::SetIndex
    (
    DBCOUNTITEM	ulDex,        //@parm IN | Row Index value
    size_t		ulOffset      //@parm IN | Offset of Row in the File
    )
{
    // Check index and realloc if beyond our current range
    if (m_ulDexCnt <= ulDex)
        if (FALSE == ReAlloc( ARRAY_INIT_SIZE ))
            return FALSE;

    m_rgDex[ulDex].ulOffset = ulOffset;
    m_rgDex[ulDex].bStatus = FALSE;

    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Mark the row as deleted.
//
// @rdesc BOOLEAN value
//      @flag TRUE | Succeeded
//      @flag FALSE | Failed to Initialize
//
BOOL CFileIdx::DeleteRow
    (
    DBCOUNTITEM ulDex                 //@parm IN | Row Index value
    )
{
    // Index should alway be valid
    assert(m_ulDexCnt > ulDex);

    // Mark Row
    m_rgDex[ulDex].bStatus = DELETED_ROW;

    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Has the row been deleted or not
//
// @rdesc BOOLEAN value
//      @flag TRUE | Row Already Deleted
//      @flag FALSE | Row has not been Deleted
//
BOOL CFileIdx::IsDeleted
    (
    DBCOUNTITEM ulDex                 //@parm IN | Row Index value
    )
{
    // Index should alway be valid and Check deletion status
    if ( (ulDex >= m_ulDexCnt) ||
		 (DELETED_ROW != m_rgDex[ulDex].bStatus) )
        return FALSE;

    return TRUE;
}


//--------------------------------------------------------------------
// @mfunc Return the File Offset that the row starts
//
// @rdesc Offset from Beginning of File
//
size_t CFileIdx::GetRowOffset
    (
    DBCOUNTITEM ulDex                 //@parm IN | Row Index value
    )
{
    // Index should alway be valid
    assert(m_ulDexCnt > ulDex);

    return m_rgDex[ulDex].ulOffset;
}


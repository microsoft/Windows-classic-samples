/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiBuffer.cpp

Abstract:

    A simple buffer class

Author:

    ISAPI developer (Microsoft employee), November 2002

--*/

#include <IsapiTools.h>

#define SIZE_ENCODED_CHUNK      4
#define SIZE_UNENCODED_CHUNK    3

//
// Local prototypes
//

BOOL
Base64DecodeChunk(
    CHAR *  szChunk,
    BYTE *  pData,
    DWORD * pcbData
    );

//
// Method implementations
//

ISAPI_BUFFER::ISAPI_BUFFER(
    DWORD dwMaxAlloc
    ) : _pData( _InlineData ),
        _cbData( 0 ),
        _cbBuffer( INLINE_BUFFER_SIZE ),
        _dwMaxAlloc( dwMaxAlloc ? dwMaxAlloc : DEFAULT_MAX_ALLOC_SIZE )
/*++

Purpose:

    Constructor for the ISAPI_BUFFER object

Arguments:

    dwMaxAlloc - Initial value of max allocation size

Returns:

    None

--*/
{
    return;
}

ISAPI_BUFFER::~ISAPI_BUFFER()
/*++

Purpose:

    Destructory for the ISAPI_BUFFER object

Arguments:

    None

Returns:

    None

--*/
{
    //
    // Reset the object.  Ensure that we
    // deallocate any associated heap.
    //

    Reset( TRUE );
}

BOOL
ISAPI_BUFFER::SetData(
    VOID *  pData,
    DWORD   cbData
    )
/*++

Purpose:

    Sets the specified data into the ISAPI_BUFFER object

Arguments:

    pData  - Pointer to the data to set
    cbData - Size of the data to set

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // Ensure that the buffer is sufficient to
    // store the data.
    //

    if ( cbData > _cbBuffer &&
         !Resize( cbData ) )
    {
        return FALSE;
    }

    //
    // Copy the data into the buffer and
    // set the new size
    //

    CopyMemory( _pData, pData, cbData );

    _cbData = cbData;

    return TRUE;
}

BOOL
ISAPI_BUFFER::AppendData(
    VOID *  pData,
    DWORD   cbData
    )
/*++

Purpose:

    Appends data to the ISAPI_BUFFER object

Arguments:

    pData  - Pointer to the data to append
    cbData - Size of the data to append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbNewSize;

    //
    // Ensure that the buffer is sufficient to
    // store the data.
    //

    cbNewSize = _cbData + cbData;

    if ( cbNewSize > _cbBuffer &&
         !Resize( cbNewSize ) )
    {
        return FALSE;
    }

    //
    // Copy the data into the buffer, beginning
    // at the specified offset, and set the new size.
    //

    CopyMemory( (BYTE*)_pData + _cbData, pData, cbData );

    _cbData = cbNewSize;

    return TRUE;
}

BOOL
ISAPI_BUFFER::Resize(
    DWORD   cbNewSize
    )
/*++

Purpose:

    Ensures that the storage for ISAPI_BUFFER is sufficient to
    contain the specified number of bytes

Arguments:

    cbNewSize - The target size

Returns:

    TRUE on success, FALSE on failure

--*/
{
    HANDLE  hNew;
    DWORD   cbAdjustedNewSize;

    //
    // If the buffer is already large enough, then
    // just return TRUE.
    //

    if ( cbNewSize <= _cbBuffer )
    {
        return TRUE;
    }

    //
    // If the new requested size is larger than the
    // maximum we will allocate, then fail the call
    //

    if ( cbNewSize > _dwMaxAlloc )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        
        goto Failed;
    }

    //
    // Adjust the new size so that we allocate blocks
    // using a granularity of ISAPILIB_BUFFER_INLINE_SIZE.
    // Also, make sure that this adjustment does not
    // exceed _dwMaxAlloc.
    //

    cbAdjustedNewSize = cbNewSize / INLINE_BUFFER_SIZE * INLINE_BUFFER_SIZE;

    if ( cbAdjustedNewSize < cbNewSize )
    {
        cbAdjustedNewSize += INLINE_BUFFER_SIZE;
    }

    if ( cbAdjustedNewSize > _dwMaxAlloc )
    {
        cbAdjustedNewSize = _dwMaxAlloc;
    }

    //
    // Allocate the new storage.
    //

    if ( _pData == _InlineData )
    {
        //
        // If _pData is currently pointed at the new buffer,
        // then we need to allocate some heap storage.
        //

        hNew = LocalAlloc( NONZEROLPTR, cbAdjustedNewSize );

        if ( hNew == NULL )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );

            goto Failed;
        }

        //
        // If there was data in the old buffer, copy
        // it to the new buffer.
        //

        if ( _cbData )
        {
            CopyMemory( (BYTE*)hNew, _pData, _cbData );
        }
    }
    else
    {
        //
        // If _pData is already on the heap, then reallocate it.
        //

        hNew = LocalReAlloc( _pData, cbAdjustedNewSize, LMEM_MOVEABLE );

        if ( hNew == NULL )
        {
            goto Failed;
        }
    }

    //
    // Adjust the pointer and size of the new buffer
    //

    _pData = (BYTE*)hNew;
    _cbBuffer = cbAdjustedNewSize;

    return TRUE;

Failed:

    //
    // No special cleanup to be done
    //

    return FALSE;
}

VOID
ISAPI_BUFFER::Reset(
    BOOL    fDealloc
    )
/*++

Purpose:

    Resets the ISAPI_BUFFER object

Arguments:

    fDealloc - If TRUE, then any heap associated with the
               object is freed.

Returns:

    None

--*/
{
    //
    // If we are supposed to dealloc some
    // heap memory, do so now.
    //

    if ( fDealloc &&
         _pData != _InlineData )
    {
        LocalFree( _pData );
        _pData = _InlineData;
        _cbBuffer = INLINE_BUFFER_SIZE;
    }

    //
    // Reset the data size
    //

    _cbData = 0;
}

BOOL
ISAPI_BUFFER::SetDataSize(
    DWORD   cbNewSize
    )
/*++

Purpose:

    Resets the size member ISAPI_BUFFER object.

Arguments:

    cbNewSize - The new size to set

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // Don't allow it to be set to something bigger
    // than the buffer
    //

    if ( cbNewSize > _cbBuffer )
    {
        return FALSE;
    }

    _cbData = cbNewSize;

    return TRUE;
}

VOID
ISAPI_BUFFER::SetMaxAlloc(
    DWORD   dwMaxAlloc
    )
/*++

Purpose:

    Sets the max alloc size of the ISAPI_BUFFER object.

Arguments:

    dwMaxAlloc - The new size to set

Returns:

    None

--*/
{
    _dwMaxAlloc = dwMaxAlloc ? dwMaxAlloc : DEFAULT_MAX_ALLOC_SIZE;
}

DWORD
ISAPI_BUFFER::QueryMaxAlloc(
    VOID
    )
/*++

Purpose:

    Returns the max alloc size of the ISAPI_BUFFER object.

Arguments:

    None

Returns:

    The max alloc size

--*/
{
    return _dwMaxAlloc;
}

VOID *
ISAPI_BUFFER::QueryPtr(
    VOID
    )
/*++

Purpose:

    returns a pointer to the data member ISAPI_BUFFER object.

Arguments:

    None

Returns:

    A pointer to the data member

--*/
{
    return _pData;
}

DWORD
ISAPI_BUFFER::QueryDataSize(
    VOID
    )
/*++

Purpose:

    returns the data size of the ISAPI_BUFFER object.

Arguments:

    None

Returns:

    The data size

--*/
{
    return _cbData;
}

DWORD
ISAPI_BUFFER::QueryBufferSize(
    VOID
    )
/*++

Purpose:

    Returns the buffer size of the ISAPI_BUFFER object.

Arguments:

    None

Returns:

    The size of the buffer

--*/
{
    return _cbBuffer;
}

BOOL
ISAPI_BUFFER::Base64Decode(
    CHAR *  szString
    )
/*++

Purpose:

    Base64 decodes a string into ISAPI_BUFFER object.

Arguments:

    szString - The string to decode

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BYTE    pData[SIZE_UNENCODED_CHUNK];
    CHAR *  pCursor;
    DWORD   cchRemaining;
    DWORD   cchString;
    DWORD   cbData;
    BOOL    fResult;

    //
    // Validate the input.  Make sure that size the input data
    // is evenly divisible by 4
    //

    cchString = strlen( szString );

    if ( cchString % 4 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    //
    // Decode the data in 4 byte chunks
    //

    pCursor = szString;
    cchRemaining = cchString;

    while ( cchRemaining )
    {
        fResult = Base64DecodeChunk( pCursor,
                                     pData,
                                     &cbData );

        if ( !fResult )
        {
            goto Failed;
        }

        fResult = AppendData( pData, cbData );

        if ( !fResult )
        {
            goto Failed;
        }

        cchRemaining -= SIZE_ENCODED_CHUNK;
        pCursor += SIZE_ENCODED_CHUNK;
    }

    return TRUE;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

VOID
ISAPI_BUFFER::ZeroBuffer(
    VOID
    )
/*++

Purpose:

    Fills the ISAPI_BUFFER data buffer with zeros.

Arguments:

    None

Returns:

    None

--*/
{
    ZeroMemory( _pData, _cbBuffer );
}

BOOL
Base64DecodeChunk(
    CHAR *  szChunk,
    BYTE *  pData,
    DWORD * pcbData
    )
/*++

Purpose:

    Decodes a 4 byte base64 encoded chunk into 3 decoded bytes

Arguments:

    szChunk - The chunk to decode
    pData   - The buffer to store the chunk
    pcbData - Upon return, the number of bytes in the return buffer

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   x;
    DWORD   cbChunk;

    BYTE szReverseTable[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
                              0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
                              0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                              0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
                              0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                              0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                              0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                              0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                              0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    //
    // Clear only the necessary output bytes.
    //
    // *** WARNING ***
    // We are making two assumptions here.  First, we assume
    // that pData is big enough to do this.  Second, we assume
    // that szChunk is always 4 bytes.  If either of these
    // assumptions are bad, we'll fail in a bad way.
    //

    if ( szChunk[2] == '=' )
    {
        *pcbData = 1;
        cbChunk = 2;
    }
    else if ( szChunk[3] == '=' )
    {
        *pcbData = 2;
        cbChunk = 3;
    }
    else
    {
        *pcbData = 3;
        cbChunk = 4;
    }

    ZeroMemory( pData, *pcbData );

    //
    // Reverse lookup the character mappings
    //

    for ( x = 0; x < cbChunk; x++ )
    {
        szChunk[x] = szReverseTable[szChunk[x]];

        if ( szChunk[x] == 0xff )
        {
            //
            // Hmmm.  This isn't a valid character
            // for a base64 encoded buffer.
            //

            SetLastError( ERROR_INVALID_PARAMETER );

            return FALSE;
        }
    }

    //
    // Shift bits per Base64 rules (originally defined in
    // RFC 1341, section 5.2)
    //

    for ( x = 0; x < cbChunk; x++ )
    {
        switch ( x )
        {
        case 0:

            pData[0] = szChunk[0] << 2;
            break;

        case 1:

            pData[0] |= ( szChunk[1] & 0x30 ) >> 4;
            pData[1] = ( szChunk[1] & 0xf ) << 4;
            break;

        case 2:

            pData[1] |= ( szChunk[2] & 0x3c ) >> 2;
            pData[2] = ( szChunk[2] & 0x3 ) << 6;
            break;

        case 3:

            pData[2] |= szChunk[3];
            break;
        }
    }

    return TRUE;
}


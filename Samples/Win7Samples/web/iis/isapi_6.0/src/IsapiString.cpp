/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiString.cpp

Abstract:

    A simple string class build on
    the ISAPI_BUFFER class

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>
#include <stdio.h>

//
// Local definitions
//

#define SIZE_ENCODED_CHUNK      4
#define SIZE_UNENCODED_CHUNK    3

#define UPCASE_ANSI_CHAR(c) ((((unsigned char)c & 0xdf) < 'A' || \
                              ((unsigned char)c & 0xdf) > 'Z') ? \
                              c : \
                              (c & 0xdf))

//
// Local functions
//

VOID
Base64EncodeChunk(
    BYTE *  pData,
    DWORD   cbData,
    CHAR    szOutput[5]
    );

BOOL
IsEscapeSequence(
    CHAR *  str
    );

BOOL
ShouldEscape(
    BYTE    c
    );

//
// Member function implementations
//

ISAPI_STRING::ISAPI_STRING(
    DWORD   dwMaxAlloc
    )
/*++

Purpose:

    Constructor for ISAPI_STRING object

Arguments:

    dwMaxAlloc - The max allocation for the storage of this object

Returns:

    None

--*/
{
    _Buffer.SetMaxAlloc( dwMaxAlloc );
}

ISAPI_STRING::~ISAPI_STRING()
/*++

Purpose:

    Destructor for the ISAPI_STRING object

Arguments:

    None

Returns:

    None

--*/
{
}

BOOL
ISAPI_STRING::Copy(
    CHAR *  szString,
    DWORD   cchString
    )
/*++

Purpose:

    Copies the specified NULL terminated string into the
    ISAPI_STRING object.

Arguments:

    szString  - The string to copy
    cchString - The number of characters to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data.
    //

    if ( cchString == 0 )
    {
        cchString = strlen( szString );
    }

    return CopyToOffset( szString,
                         cchString,
                         0 );
}

BOOL
ISAPI_STRING::CopyW(
    WCHAR * szStringW,
    DWORD   cchStringW
    )
/*++

Purpose:

    Copies the specified NULL terminated Unicode string into the
    ISAPI_STRING object.

Arguments:

    szStringW  - The Unicode string to copy
    cchStringW - The number of characters to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data
    //

    if ( cchStringW == 0 )
    {
        cchStringW = wcslen( szStringW );
    }

    return CopyWToOffset( szStringW,
                          cchStringW,
                          0 );
}

BOOL
ISAPI_STRING::Append(
    CHAR *  szString,
    DWORD   cchString
    )
/*++

Purpose:

    Appends the specified NULL terminated string into the
    ISAPI_STRING object.

Arguments:

    szString  - The string to append
    cchString - The number of characters to append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data.
    //

    if ( cchString == 0 )
    {
        cchString = strlen( szString );
    }

    return CopyToOffset( szString,
                         cchString,
                         QueryCCH() );
}

BOOL
ISAPI_STRING::AppendW(
    WCHAR * szStringW,
    DWORD   cchStringW
    )
/*++

Purpose:

    Appends the specified NULL terminated Unicode string into the
    ISAPI_STRING object.

Arguments:

    szStringW  - The Unicode string to append
    cchStringW - The number of characters to append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data
    //

    if ( cchStringW == 0 )
    {
        cchStringW = wcslen( szStringW );
    }

    return CopyWToOffset( szStringW,
                          cchStringW,
                          QueryCCH() );
}

BOOL
ISAPI_STRING::Printf(
    CHAR *  szString,
    ...
    )
/*++

Purpose:

    Sets the specified printf-style formatted string
    into the ISAPI_STRING object using a variable number
    of arguments.

Arguments:

    szString - The string format
    ...      - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BOOL    fResult;

    //
    // Let ISAPI_BUFFER::vsprintf_s do the work
    //

    va_list args;

    va_start( args, szString );

    fResult = vsprintf_s( szString,
                        args );

    va_end( args );

    return fResult;
}

BOOL
ISAPI_STRING::vsprintf_s(
    CHAR *  szFormat,
    va_list args
    )
/*++

Purpose:

    Sets the specified printf-style formatted string
    into the ISAPI_STRING object using a va_list of args.

Arguments:

    szFormat - The string format
    ...      - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cchWritten;
    DWORD   cbBuffer;
    BOOL    fResult;

    //
    // Build the formatted string
    //

    do
    {
        cbBuffer = QueryBufferSize();

        cchWritten = (DWORD)_vsnprintf_s( QueryStr(),
										sizeof( QueryStr() ),
                                        cbBuffer,
                                        szFormat,
                                        args );

        if ( cchWritten == (DWORD)-1 )
        {
            //
            // If we just failed, and the buffer
            // is already at its maximum size,
            // then fail.
            //

            if ( cbBuffer >= QueryMaxAlloc() )
            {
                goto Failed;
            }

            //
            // Grow the buffer and try again
            //

            cbBuffer *= 2;

            //
            // Don't exceed the max buffer size
            //

            if ( cbBuffer > QueryMaxAlloc() )
            {
                cbBuffer = QueryMaxAlloc();
            }

            fResult = ResizeBuffer( cbBuffer );

            if ( !fResult )
            {
                goto Failed;
            }
        }

    } while ( cchWritten == (DWORD)-1 );

    SetLen( cchWritten );

    return TRUE;

Failed:

    //
    // No special cleanup needed
    //

    return FALSE;
}

VOID
ISAPI_STRING::CalcLen(
    VOID
    )
/*++

Purpose:

    Calculates the size in characters of the ISAPI_STRING data by
    doing a strlen.

Arguments:

    None

Returns:

    None

--*/
{
    _Buffer.SetDataSize( strlen( QueryStr() ) + 1 );
}

DWORD
ISAPI_STRING::QueryCB(
    VOID
    )
/*++

Purpose:

    Returns the number of bytes (not including NULL terminator)
    in the ISAPI_STRING data

Arguments:

    None

Returns:

    The number of bytes

--*/
{
    return QueryCCH();
}

DWORD
ISAPI_STRING::QueryCCH(
    VOID
    )
/*++

Purpose:

    Returns the number of characters (not including NULL terminator)
    in the ISAPI_STRING data

Arguments:

    None

Returns:

    The number of characters

--*/
{
    DWORD   cbBuffer = _Buffer.QueryDataSize();

    return cbBuffer == 0 ? 0 : cbBuffer - 1;
}

BOOL
ISAPI_STRING::SetLen(
    DWORD   cchNewLength
    )
/*++

Purpose:

    Sets the length of the data in the ISAPI_STRING object,
    truncating the current data if necessary.

Arguments:

    cchNewLength - The new data length.

Returns:

    TRUE on success, FALSE on failure.

--*/
{
    CHAR *  pCursor;

    //
    // Ensure that we don't set the length to
    // something longer than the current buffer
    //

    if ( cchNewLength > QueryBufferSize() - 1 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pCursor = QueryStr();

    pCursor[cchNewLength] = '\0';

    _Buffer.SetDataSize( cchNewLength + sizeof(CHAR) );

    return TRUE;
}

DWORD
ISAPI_STRING::QueryBufferSize(
    VOID
    )
/*++

Purpose:

    Returns the number of bytes in the ISAPI_STRING data buffer

Arguments:

    None

Returns:

    The number of bytes

--*/
{
    return _Buffer.QueryBufferSize();
}

BOOL
ISAPI_STRING::ResizeBuffer(
    DWORD   cbSize
    )
/*++

Purpose:

    Guarantees that the data storage buffer of the ISAPI_STRING
    object is at least the number of specified bytes.

Arguments:

    cbSize - The target size

Returns:

    TRUE on success, FALSE on failure

--*/
{
    return _Buffer.Resize( cbSize );
}

DWORD
ISAPI_STRING::QueryMaxAlloc(
    VOID
    )
/*++

Purpose:

    Returns the max allocation size for the ISAPI_STRING's buffer

Arguments:

    None

Returns:

    The max allocation size

--*/
{
    return _Buffer.QueryMaxAlloc();
}

VOID
ISAPI_STRING::SetMaxAlloc(
    DWORD   dwMaxAlloc
    )
/*++

Purpose:

    Sets the max allocation size for the ISAPI_STRING's buffer

Arguments:

    The max allocation size

Returns:

    None

--*/
{
    _Buffer.SetMaxAlloc( dwMaxAlloc );
}

CHAR *
ISAPI_STRING::FindStr(
    CHAR *  szSubString,
    BOOL    fCaseInsensitive,
    DWORD   dwStartingOffset
    )
/*++

Purpose:

    Finds the specified sub-string in the ISAPI_STRING's
    data.

Arguments:

    szSubString      - The sub-string to find
    fCaseInsensitive - If TRUE, does a case insensitive search
    dwStartingOffset - The starting offset in the ISAPI_STRING to start search

Returns:

    Pointer to the instance of the sub-string if found, NULL if not found

--*/
{
    CHAR *  pCursor;
    CHAR *  pEnd;
    CHAR *  szFound;
    CHAR *  szString;
    CHAR    c1;
    CHAR    c2;
    DWORD   cchString;
    DWORD   cchSubString;
    DWORD   cchData;
    DWORD   n;

    //
    // If we are trying to search beyond the length
    // of the string, then we aren't going to find it
    //

    cchData = QueryCCH();

    if ( dwStartingOffset > cchData )
    {
        goto NotFound;
    }

    szString = QueryStr() + dwStartingOffset;

    cchString = cchData - dwStartingOffset;

    //
    // If szString is smaller than szSubString, then
    // we aren't going to find it.
    //

    cchSubString = strlen( szSubString );

    if ( cchSubString > cchString )
    {
        goto NotFound;
    }

    //
    // Set boundaries for the search
    //

    pEnd = szString + cchString - cchSubString;

    //
    // Search now
    //

    szFound = NULL;

    for ( pCursor = szString; pCursor <= pEnd; pCursor++ )
    {
        c1 = fCaseInsensitive ? UPCASE_ANSI_CHAR( *pCursor ) : *pCursor;
        c2 = fCaseInsensitive ? UPCASE_ANSI_CHAR( *szSubString ) : *szSubString;

        if ( c1 == c2 )
        {
            szFound = pCursor;

            for ( n = 1; n < cchSubString; n++ )
            {
                c1 = fCaseInsensitive ? UPCASE_ANSI_CHAR( *(pCursor+n) ) : *(pCursor+n);
                c2 = fCaseInsensitive ? UPCASE_ANSI_CHAR( *(szSubString+n) ) : *(szSubString+n);

                if ( c1 != c2 )
                {
                    szFound = NULL;
                    break;
                }
            }

            if ( szFound )
            {
                return szFound;
            }
        }
    }

NotFound:

    SetLastError( ERROR_INVALID_INDEX );

    return NULL;
}

BOOL
ISAPI_STRING::Base64Encode(
    VOID *  pBuffer,
    DWORD   cbBuffer
    )
/*++

Purpose:

    Base64 encodes the specified buffer into the ISAPI_STRING

Arguments:

    pBuffer  - The buffer to encode
    cbBuffer - The size of the buffer

Returns:

    TRUE on success, FALSE on failure

--*/
{
    CHAR    szChunk[SIZE_ENCODED_CHUNK+1];
    BYTE *  pCursor;
    DWORD   cbRemaining;
    DWORD   cbThisTime;
    BOOL    fResult;

    //
    // Encode the data in chunks
    //

    pCursor = (BYTE*)pBuffer;
    cbRemaining = cbBuffer;

    while ( cbRemaining )
    {
        cbThisTime = cbRemaining > SIZE_UNENCODED_CHUNK ?
                     SIZE_UNENCODED_CHUNK :
                     cbRemaining;

        Base64EncodeChunk( pCursor,
                           cbThisTime,
                           szChunk );

        fResult = Append( szChunk, SIZE_ENCODED_CHUNK );

        if ( !fResult )
        {
            goto Failed;
        }

        cbRemaining -= cbThisTime;
        pCursor += cbThisTime;
    }

    return TRUE;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_STRING::Escape(
    BOOL    fAllowDoubleEscaping
    )
/*++

Purpose:

    Escape encodes the ISAPI_STRING's data per RFC2396

Arguments:

    fAllowDoubleEscaping - If FALSE, then this function will not encode
                           any '%' characters that are part of an escape
                           sequence.  If TRUE, all '%' characters will
                           be encoded.

Returns:

    TRUE on success, FALSE on failure

--*/
{
    ISAPI_STRING    strCooked;
    DWORD           dwNumEscapes = 0;
    CHAR            szHex[3] = {0};
    BYTE *          pRead;
    BYTE *          pWrite;

    //
    // Walk through the string once.  If there
    // are no escapes, then we can just return.
    //

    pRead = (BYTE*)_Buffer.QueryPtr();

    while ( *pRead != '\0' )
    {
        if ( ( fAllowDoubleEscaping ||
               !IsEscapeSequence( (CHAR*)pRead ) ) &&
             ShouldEscape( *pRead ) )
        {
            dwNumEscapes++;
        }

        pRead++;
    }

    if ( dwNumEscapes == 0 )
    {
        return TRUE;
    }

    //
    // Make sure that our cooked string buffer is big enough, so
    // we can manipulate its pointer directly.
    //

    if ( strCooked.ResizeBuffer( QueryCB() + dwNumEscapes * 2 + 1 ) == FALSE )
    {
        goto Failed;
    }

    pRead = (BYTE*)_Buffer.QueryPtr();
    pWrite = (BYTE*)strCooked.QueryStr();

    while ( *pRead != '\0' )
    {
        if ( ( fAllowDoubleEscaping ||
               !IsEscapeSequence( (CHAR*)pRead ) ) &&
             ShouldEscape( *pRead ) )
        {
            itoa( *pRead, szHex, 16 );

            *pWrite = '%';
            *(pWrite+1) = szHex[0];
            *(pWrite+2) = szHex[1];

            pRead++;
            pWrite += 3;

            continue;
        }

        *pWrite = *pRead;

        pRead++;
        pWrite++;
    }

    *pWrite = '\0';

    if ( Copy( strCooked.QueryStr() ) == FALSE )
    {
        goto Failed;
    }

    return TRUE;

Failed:

    return FALSE;
}

VOID
ISAPI_STRING::Unescape(
    VOID
    )
/*++

Purpose:

    Decodes escape sequences in the ISAPI_STRING's data

Arguments:

    None

Returns:

    None

--*/
{
    CHAR *  pRead;
    CHAR *  pWrite;
    CHAR    szHex[3] = {0};
    BYTE    c;

    pRead = (CHAR*)_Buffer.QueryPtr();
    pWrite = pRead;

    while ( *pRead )
    {
        if ( IsEscapeSequence( pRead ) )
        {
            szHex[0] = *(pRead+1);
            szHex[1] = *(pRead+2);

            c = (BYTE)strtoul( szHex, NULL, 16 );

            *pWrite = c;

            pRead += 3;
            pWrite++;

            continue;
        }

        *pWrite = *pRead;

        pRead++;
        pWrite++;
    }

    *pWrite = '\0';

    CalcLen();

    return;
}

CHAR *
ISAPI_STRING::QueryStr(
    VOID
    )
/*++

Purpose:

    Returns a pointer to the ISAPI_STRING's data

Arguments:

    None

Returns:

    The data pointer

--*/
{
    return (CHAR*)_Buffer.QueryPtr();
}

VOID
ISAPI_STRING::ZeroBuffer(
    VOID
    )
/*++

Purpose:

    Fills the ISAPI_STRING's buffer with zeros

Arguments:

    None

Returns:

    None

--*/
{
    _Buffer.ZeroBuffer();
    _Buffer.SetDataSize( 0 );
}

BOOL
ISAPI_STRING::CopyToOffset(
    CHAR *  szString,
    DWORD   cchString,
    DWORD   dwOffset
    )
/*++

Purpose:

    Copies a string into the ISAPI_STRING's data, starting at the
    specified offset.

Arguments:

    szString  - The string to copy
    cchString - The number of characters to copy
    dwOffset  - The offset at which to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbData;

    //
    // Ensure that we have enough storage available
    //

    cbData = cchString + sizeof(CHAR);

    if ( ResizeBuffer( cbData + dwOffset ) == FALSE )
    {
        return FALSE;
    }

    //
    // Copy the data and ensure proper termination
    //

    CopyMemory( QueryStr() + dwOffset, szString, cbData );

    SetLen( QueryCCH() + cbData - sizeof(CHAR) );

    return TRUE;
}

BOOL
ISAPI_STRING::CopyWToOffset(
    WCHAR * szStringW,
    DWORD   cchStringW,
    DWORD   dwOffset
    )
/*++

Purpose:

    Copies a Unicode string into the ISAPI_STRING's data,
    starting at the specified offset.

Arguments:

    szStringW  - The Unicode string to copy
    cchStringW - The number of characters to copy
    dwOffset   - The offset at which to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    CHAR *  pCursor;
    DWORD   cbData;
    DWORD   cbCopied;
    DWORD   cchData;

    cchData = QueryCCH();

    //
    // Ensure that we have enough storage available
    //

    cbData = cchStringW + sizeof(CHAR);

    if ( QueryBufferSize() < cbData + dwOffset &&
         ResizeBuffer( cbData + dwOffset ) == FALSE )
    {
        goto Failed;
    }

    //
    // Set the point into which the copy will occur
    //

    pCursor = QueryStr() + cchData;

    //
    // Convert the wide string into ANSI using
    // the default system code page.
    //

    cbCopied = WideCharToMultiByte( CP_ACP,
                                    0,
                                    szStringW,
                                    cchStringW,
                                    pCursor,
                                    cbData,
                                    NULL,
                                    NULL );

    if ( cbCopied == 0 )
    {
        goto Failed;
    }

    SetLen( cchData + cbCopied );

    return TRUE;

Failed:

    //
    // No special cleanup to be done
    //

    return FALSE;
}

VOID
Base64EncodeChunk(
    BYTE *  pData,
    DWORD   cbData,
    CHAR    szOutput[5]
    )
/*++

Purpose:

    Base64 encodes up to 3 bytes of data into a 4 byte encoded chunk

Arguments:

    pData    - The data to encode
    cbData   - The number of bytes to encode
    szOutput - On return, contains the encoded chunk

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   x;
    BYTE    szTable[] = "ABCDEFGH"
                        "IJKLMNOP"
                        "QRSTUVWX"
                        "YZabcdef"
                        "ghijklmn"
                        "opqrstuv"
                        "wxyz0123"
                        "456789+/";

    //
    // *** Warning ***
    //
    // This function assumes that the output buffer
    // is at least 5 bytes.  If this is not true,
    // then this function will overrun the output
    // buffer.
    //

    ZeroMemory( szOutput, 5 );

    //
    // Handle invalid input data
    //

    if ( cbData > 3 )
    {
        cbData = 3;
    }

    if ( !cbData )
    {
        return;
    }

    //
    // Do the bit shift shuffle...
    //

    for ( x = 0; x < cbData; x++ )
    {
        switch ( x )
        {
        case 0:

            szOutput[0] = pData[0] >> 2;
            szOutput[1] = ( pData[0] & 0x3 ) << 4;
            break;

        case 1:

            szOutput[1] |= ( pData[1] ) >> 4;
            szOutput[2] = ( pData[1] & 0xf ) << 2;
            break;

        case 2:

            szOutput[2] |= ( pData[2] >> 6 );
            szOutput[3] = pData[2] & 0x3f;
            break;
        }
    }

    //
    // Handle padding
    //

    if ( x < 3 )
    {
        szOutput[3] = '=';
    }

    if ( x < 2 )
    {
        szOutput[2] = '=';
    }

    //
    // Convert bits to mapped printable characters
    //

    do
    {
        szOutput[x] = szTable[szOutput[x]];
    } while ( x-- );

    //
    // Done
    //

    return;
}

BOOL
IsEscapeSequence(
    CHAR *  str
    )
/*++

Purpose:

    Determines whether the specified NULL terminated
    sequence of characters is an escape sequence (ie. %00).

Arguments:

    str - The NULL terminated string to consider

Returns:

    TRUE if yes, FALSE if no

--*/
{
    if ( *str == '%' &&
         isxdigit( *(str+1) ) &&
         isxdigit( *(str+2) ) )
    {
        return TRUE;
    }

    return FALSE;
}

BOOL
ShouldEscape(
    BYTE    c
    )
/*++

Purpose:

    Determines whether the specified character should
    be escape encoded, per RFC2396

Arguments:

    c - The character to consider

Returns:

    TRUE if yes, FALSE if no

--*/
{
    //
    // If the character is listed in RFC2396, section
    // 2.4.3 as control, space, delims or unwise, we
    // should escape it.  Also, we should escape characters
    // with the high bit set.
    //

    if ( c <= 0x1f ||
         c == 0x7f )
    {
        //
        // Control character
        //

        goto ReturnTrue;
    }

    if ( c >= 0x80 )
    {
        //
        // High bit set
        //

        goto ReturnTrue;
    }

    switch ( c )
    {

    //
    // space
    //
    case ' ':

    //
    // delims
    //
    case '<':
    case '>':
    case '#':
    case '%':
    case '\"':

    //
    // unwise
    //
    case '{':
    case '}':
    case '|':
    case '\\':
    case '^':
    case '[':
    case ']':
    case '`':

        goto ReturnTrue;
    }

    //
    // If we get here, then the character should not be
    // escaped
    //

    return FALSE;

ReturnTrue:

    return TRUE;
}


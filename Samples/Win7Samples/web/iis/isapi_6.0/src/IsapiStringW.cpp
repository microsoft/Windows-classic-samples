/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiStringW.cpp

Abstract:

    A simple string class build on
    the ISAPI_BUFFER class

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>
#include <stdio.h>

ISAPI_STRINGW::ISAPI_STRINGW(
    DWORD   dwMaxAlloc
    )
/*++

Purpose:

    Constructor for ISAPI_STRINGW object

Arguments:

    dwMaxAlloc - The max allocation for the storage of this object

Returns:

    None

--*/
{
    _Buffer.SetMaxAlloc( dwMaxAlloc );
}

ISAPI_STRINGW::~ISAPI_STRINGW()
/*++

Purpose:

    Destructor for ISAPI_STRING object

Arguments:

    None

Returns:

    None

--*/
{
}

BOOL
ISAPI_STRINGW::Copy(
    WCHAR * szStringW,
    DWORD   cchStringW
    )
/*++

Purpose:

    Copies a Unicode string to the ISAPI_STRINGW object

Arguments:

    szStringW  - The Unicode string to copy
    cchStringW - The number of characters to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data.
    //

    if ( cchStringW == 0 )
    {
        cchStringW = wcslen( szStringW );
    }

    return CopyToOffset( szStringW,
                         cchStringW,
                         0 );
}

BOOL
ISAPI_STRINGW::CopyA(
    CHAR *  szString,
    DWORD   cchString
    )
/*++

Purpose:

    Copies an ANSI string to the ISAPI_STRINGW object

Arguments:

    szString  - The ANSI string to copy
    cchString - The number of characters to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data
    //

    if ( cchString == 0 )
    {
        cchString = strlen( szString );
    }

    return CopyAToOffset( szString,
                          cchString,
                          0 );
}

BOOL
ISAPI_STRINGW::Append(
    WCHAR * szStringW,
    DWORD   cchStringW
    )
/*++

Purpose:

    Appends a Unicode string to the ISAPI_STRINGW object

Arguments:

    szStringW  - The Unicode string to append
    cchStringW - The number of characters to append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data.
    //

    if ( cchStringW == 0 )
    {
        cchStringW = wcslen( szStringW );
    }

    return CopyToOffset( szStringW,
                         cchStringW,
                         QueryCCH() );
}

BOOL
ISAPI_STRINGW::AppendA(
    CHAR *  szString,
    DWORD   cchString
    )
/*++

Purpose:

    Appends an ANSI string to the ISAPI_STRINGW object

Arguments:

    szString  - The ANSI string to append
    cchString - The number of characters to Append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    //
    // If necessary, calculate the length
    // of the new data
    //

    if ( cchString == 0 )
    {
        cchString = strlen( szString );
    }

    return CopyAToOffset( szString,
                          cchString,
                          QueryCCH() );
}

BOOL
ISAPI_STRINGW::Printf(
    WCHAR *  szStringW,
    ...
    )
/*++

Purpose:

    Sets the specified printf-style formatted Unicode string
    into the ISAPI_STRINGW object using a variable number
    of arguments.

Arguments:

    szString - The Unicode string format
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

    va_start( args, szStringW );

    fResult = vsprintf_s( szStringW,
                        args );

    va_end( args );

    return fResult;
}

BOOL
ISAPI_STRINGW::vsprintf_s(
    WCHAR * szFormatW,
    va_list args
    )
/*++

Purpose:

    Sets the specified printf-style formatted Unicode string
    into the ISAPI_STRINGW object using a va_list of args.

Arguments:

    szFormat - The Unicode string format
    ...      - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    INT     cchWritten;
    DWORD   cbBuffer;
    BOOL    fResult;

    //
    // Build the formatted string
    //

    do
    {
        cbBuffer = QueryBufferSize();

        cchWritten = (DWORD)_vsnwprintf_s( QueryStr(),
										sizeof(QueryStr()),
                                         cbBuffer,
                                         szFormatW,
                                         args );

        if ( cchWritten < 0 )
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

    } while ( cchWritten < 0 );

    SetLen( cchWritten );

    return TRUE;

Failed:

    //
    // No special cleanup needed
    //

    return FALSE;
}

VOID
ISAPI_STRINGW::CalcLen(
    VOID
    )
/*++

Purpose:

    Calculates the size in characters of the ISAPI_STRINGW data by
    doing a wcslen.

Arguments:

    None

Returns:

    None

--*/
{
    _Buffer.SetDataSize( wcslen( QueryStr() ) + 1 );
}

DWORD
ISAPI_STRINGW::QueryCB(
    VOID
    )
/*++

Purpose:

    Returns the size, in bytes, of the ISAPI_STRINGW data (not
    including the NULL terminator).

Arguments:

    None

Returns:

    The size in bytes

--*/
{
    DWORD   cbBuffer = _Buffer.QueryDataSize();

    return cbBuffer == 0 ? 0 : ( cbBuffer - 1 );
}

DWORD
ISAPI_STRINGW::QueryCCH(
    VOID
    )
/*++

Purpose:

    Returns the size, in characters, of the ISAPI_STRINGW data (not
    including the NULL terminator).

Arguments:

    None

Returns:

    The size in characters

--*/
{
    return QueryCB() / sizeof(WCHAR);
}

BOOL
ISAPI_STRINGW::SetLen(
    DWORD   cchNewLength
    )
/*++

Purpose:

    Sets the length of the data, in characters, in the ISAPI_STRINGW
    object, truncating the current data if necessary.

Arguments:

    cchNewLength - The new data length in characters.

Returns:

    TRUE on success, FALSE on failure.

--*/
{
    WCHAR * pCursor;
    DWORD   cbNewLength = cchNewLength * sizeof(WCHAR);
    BOOL    fResult;

    fResult = _Buffer.SetDataSize( cbNewLength + sizeof(WCHAR) );

    if ( !fResult )
    {
        return FALSE;
    }

    pCursor = (WCHAR*)_Buffer.QueryPtr() + cchNewLength;

    *pCursor = L'\0';
    
    return TRUE;
}

DWORD
ISAPI_STRINGW::QueryBufferSize(
    VOID
    )
/*++

Purpose:

    Returns the size, in bytes, of the ISAPI_STRINGW buffer

Arguments:

    None

Returns:

    The buffer size, in bytes

--*/
{
    return _Buffer.QueryBufferSize();
}

BOOL
ISAPI_STRINGW::ResizeBuffer(
    DWORD   cbSize
    )
/*++

Purpose:

    Guarantees that the data storage buffer of the ISAPI_STRINGW
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
ISAPI_STRINGW::QueryMaxAlloc(
    VOID
    )
/*++

Purpose:

    Returns the max allocation size for the ISAPI_STRINGW's buffer

Arguments:

    None

Returns:

    The max allocation size

--*/
{
    return _Buffer.QueryMaxAlloc();
}

VOID
ISAPI_STRINGW::SetMaxAlloc(
    DWORD   dwMaxAlloc
    )
/*++

Purpose:

    Sets the max allocation size for the ISAPI_STRINGW's buffer

Arguments:

    The max allocation size

Returns:

    None

--*/
{
    _Buffer.SetMaxAlloc( dwMaxAlloc );
}

WCHAR *
ISAPI_STRINGW::QueryStr(
    VOID
    )
/*++

Purpose:

    Returns the data pointer for the ISAPI_STRINGW

Arguments:

    None

Returns:

    The data pointer

--*/
{
    return (WCHAR*)_Buffer.QueryPtr();
}

BOOL
ISAPI_STRINGW::CopyToOffset(
    WCHAR * szStringW,
    DWORD   cchStringW,
    DWORD   cchOffset
    )
/*++

Purpose:

    Copies a Unicode string into the ISAPI_STRINGW's data, starting at the
    specified offset, in characters.

Arguments:

    szStringW  - The Unicode string to copy
    cchStringW - The number of characters to copy
    cchOffset  - The offset, in chracters, at which to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbData;

    //
    // Ensure that we have enough storage available
    //

    cbData = ( cchStringW + 1 ) * sizeof(WCHAR);

    if ( ResizeBuffer( cbData + cchOffset * sizeof(WCHAR) ) == FALSE )
    {
        return FALSE;
    }

    //
    // Copy the data and ensure proper termination
    //

    CopyMemory( QueryStr() + cchOffset, szStringW, cbData );

    SetLen( QueryCCH() + cchStringW );

    return TRUE;
}

BOOL
ISAPI_STRINGW::CopyAToOffset(
    CHAR *  szString,
    DWORD   cchString,
    DWORD   cchOffset
    )
/*++

Purpose:

    Copies an ANSI string into the ISAPI_STRINGW's data, starting at the
    specified offset, in characters.

Arguments:

    szString  - The ANSI string to copy
    cchString - The number of characters to copy
    cchOffset - The offset, in chracters, at which to copy

Returns:

    TRUE on success, FALSE on failure

--*/
{
    WCHAR * pCursor;
    DWORD   cbString;
    DWORD   cchCopied;
    DWORD   cchData;
    DWORD   cbData;

    cchData = QueryCCH();
    cbData = QueryCB();

    //
    // Ensure that we have enough storage available
    //

    cbString = ( cchString + 1 ) * sizeof(WCHAR);

    if ( ResizeBuffer( cbString + cchOffset * sizeof(WCHAR) ) == FALSE )
    {
        goto Failed;
    }

    //
    // Set the point into which the copy will occur
    //

    pCursor = QueryStr() + cchData;

    //
    // Convert the ANSI string into Unicode using
    // the default system code page.
    //

    cchCopied = MultiByteToWideChar( CP_ACP,
                                     MB_PRECOMPOSED,
                                     szString,
                                     cchString,
                                     pCursor,
                                     cbData );

    if ( cchCopied == 0 )
    {
        goto Failed;
    }

    SetLen( cchData + cchCopied );

    return TRUE;

Failed:

    //
    // No special cleanup to be done
    //

    return FALSE;
}


/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiRequest.h

Abstract:

    A class to do common ISAPI extension
    processing tasks

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>

//
// Define the entity for a generic 500 server error response
//

#define SERVER_ERROR    \
    "<html>\r\n"                                         \
    "<head> <title>500 Server Error</title> </head>\r\n" \
    "<h1> Server Error </h1>\r\n"                        \
    "<hr>\r\n"                                           \
    "The server was unable to process your request.\r\n" \
    "</html>"

//
// Declare instance of static OS major version variable
//

DWORD   ISAPI_REQUEST::_dwOsMajorVersion = 0;

ISAPI_REQUEST::ISAPI_REQUEST(
    EXTENSION_CONTROL_BLOCK *   pEcb
    )
    : _pEcb( pEcb ),
      _dwMaxSyncWriteSize( 0 ), // Use ISAPI_STRING default
      _fNeedDoneWithSession( FALSE ),
      _fClientIsConnected( TRUE ),
      _hImpersonationToken( NULL )
/*++

Purpose:

    Initializes the ISAPI_REQUEST object

    During initialization, this object will determine
    the IIS version.

Arguments:

    pEcb - The ECB for the request to associate with this object

Returns:

    None

--*/
{
    //
    // Set the max response buffer size
    //

    _ResponseBuffer.SetMaxAlloc( DEFAULT_RESPONSE_BUFFER_SIZE );

    //
    // Get the IIS version info from the ECB.
    //

    _dwIISMajorVersion = pEcb->dwVersion >> 16;
    _dwIISMinorVersion = pEcb->dwVersion & 0xffff;

    //
    // Note that not all versions of IIS return
    // their version correctly.
    //
    // In particular, IIS version 4.0 and 5.0 will
    // both return 4.  IIS versions 5.1 and 6.0 do
    // correctly set this value in the ECB.
    //
    // Therefore, if we see a major version of 4,
    // we'll look at the OS version to break the tie.
    //

    if ( _dwIISMajorVersion == 4 &&
         _dwIISMinorVersion == 0 )
    {
        _dwIISMajorVersion = QueryOsMajorVersion();
    }
}

ISAPI_REQUEST::~ISAPI_REQUEST(
    VOID
    )
/*++

Purpose:

    Destructor for the ISAPI_REQUEST object.

    If the extension called AsyncTransmitBufferedResponse, then
    this code will take responsibility for notifying IIS that
    the request has been completed.

Arguments:

    None

Returns:

    None

--*/
{
    DWORD   dwState;

    //
    // If we send the response back via this object's
    // buffer, then we need to let IIS know that we
    // are done with the session.
    //

    if ( _fNeedDoneWithSession )
    {
        //
        // Since buffered responses always send a content-length,
        // we should go ahead and keep the connection open
        //

        dwState = HSE_STATUS_SUCCESS_AND_KEEP_CONN;

        _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                      HSE_REQ_DONE_WITH_SESSION,
                                      &dwState,
                                      NULL,
                                      NULL );
    }
}

BOOL
ISAPI_REQUEST::SyncSendStatusAndHeaders(
    CHAR *  szStatus,
    CHAR *  szHeaders
    )
/*++

Purpose:

    Synchronously sends the specified HTTP status codes
    and response headers to the client.

Arguments:

    szStatus  - The status to send (ie. "200 OK")
    szHeaders - The response headers to send

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cchHeaders;
    DWORD   dwStatus;
    BOOL    fResult;

    //
    // Set defaults if needed
    //

    if ( szStatus == NULL )
    {
        szStatus = DEFAULT_STATUS;
    }

    if ( szHeaders == NULL )
    {
        szHeaders = DEFAULT_HEADERS;
    }

    //
    // Make sure that headers are properly terminated
    //

    cchHeaders = strlen( szHeaders );

    if ( strcmp( szHeaders, DEFAULT_HEADERS ) != 0 )
    {
        //
        // If we are not using the default value, szHeaders
        // must end with "\r\n\r\n" (the first "\r\n" pair is
        // to mark the end of the last header value, and the
        // second is to mark the end of all headers.
        //

        if ( cchHeaders < 4 )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            goto Failed;
        }

        if ( strcmp( szHeaders + cchHeaders - 4, "\r\n\r\n" ) != 0 )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            goto Failed;
        }
    }

    //
    // To ensure proper logging, we need to set the dwHttpStatusCode
    // member on the ECB to reflect the proper value.  We'll get
    // this from the szStatus string, and ensure that it is in the
    // range of 100 to 999.
    //

    dwStatus = atoi( szStatus );

    if ( dwStatus < 100 ||
         dwStatus > 999 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    _pEcb->dwHttpStatusCode = dwStatus;

    //
    // Send the response now.
    //
    // Note that HSE_REQ_SEND_RESPONSE header will take care
    // of managing proper keep-alive behavior.  Basically, if
    // the headers contains a content-length, then IIS will
    // cause the response to honor keep-alive.  If not, then
    // IIS will close the connection when the ISAPI extension
    // is done processing.
    //

    fResult =  _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                             HSE_REQ_SEND_RESPONSE_HEADER,
                                             szStatus,
                                             NULL,
                                             (DWORD*)szHeaders );

    if ( !fResult )
    {
        //
        // Since the call failed, we should assume
        // that the client is no longer connected.
        //

        _fClientIsConnected = FALSE;

        goto Failed;
    }

    return TRUE;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::SyncWriteClientArgs(
    CHAR *  szFormat,
    va_list args
    )
/*++

Purpose:

    Synchronously sends a string to the client using
    vsprintf_s-style formatting and a va_list of arguments.

Arguments:

    szFormat - The string format to send
    args     - A va_list of additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    ISAPI_STRING    IsapiString( _dwMaxSyncWriteSize );
    DWORD           cbData;
    BOOL            fResult;

    //
    // Build the formatted data
    //

    fResult = IsapiString.vsprintf_s( szFormat,
                                    args );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Send it
    //

    cbData = IsapiString.QueryCCH();

    fResult = _pEcb->WriteClient( _pEcb->ConnID,
                                  IsapiString.QueryStr(),
                                  &cbData,
                                  HSE_IO_SYNC | HSE_IO_NODELAY );

    if ( !fResult )
    {
        //
        // Since the call failed, we should assume
        // that the client is no longer connected.
        //

        _fClientIsConnected = FALSE;
    }

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::SyncWriteClientString(
    CHAR *  szFormat,
    ...
    )
/*++

Purpose:

    Synchronously sends a string to the client using
    printf-style formatting an a variable number of
    arguments

Arguments:

    szFormat - The string format
    ...      - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BOOL    fResult;
    va_list args;

    //
    // Let SyncWriteClientArgs do the work
    //

    va_start( args, szFormat );

    fResult = SyncWriteClientArgs( szFormat,
                                   args );

    va_end( args );

    return fResult;
}

BOOL
ISAPI_REQUEST::SyncWriteCompleteResponse(
    CHAR *  szStatus,
    CHAR *  szHeaders,
    CHAR *  szFormat,
    ...
    )
/*++

Purpose:

    Synchronously sends a complete response to the client
    using the specified status and headers, with a formatted
    string with variable arguments as the entity.

Arguments:

    szStatus  - The status to send (ie. "200 OK")
    szHeaders - The response headers to send
    szFormat  - The string format
    ...       - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    va_list args;
    BOOL    fResult;

    //
    // Send the status and headers
    //

    fResult = SyncSendStatusAndHeaders( szStatus,
                                        szHeaders );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Send the entity
    //

    va_start( args, szFormat );

    fResult = SyncWriteClientArgs( szFormat,
                                   args );

    va_end( args );

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::GetServerVariable(
    CHAR *          szVariable,
    ISAPI_STRING *  pIsapiString
    )
/*++

Purpose:

    Gets a server variable value.

Arguments:

    szVariable   - The variable to get
    pIsapiString - ISAPI_STRING object to store the value on return

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbData;
    BOOL    fResult;

    //
    // If the caller is trying to get a Unicode value,
    // the this is the wrong function to call.  They
    // should pass an ISAPI_STRINGW instead of an
    // ISAPI_STRING to get the correct overloaded function.
    //

    if ( strnicmp( szVariable,
                   "UNICODE_",
                   sizeof("UNICODE_") - 1 ) == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    //
    // Try and get the variable with the
    // buffer "as is".
    //

    cbData = pIsapiString->QueryBufferSize();

    fResult = _pEcb->GetServerVariable( _pEcb->ConnID,
                                        szVariable,
                                        pIsapiString->QueryStr(),
                                        &cbData );

    if ( !fResult )
    {
        //
        // If the failure was not due to the size of the
        // buffer, then just fail.
        //

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            goto Failed;
        }

        //
        // Resize the buffer and try again
        //

        if ( !pIsapiString->ResizeBuffer( cbData ) )
        {
            goto Failed;
        }

        fResult = _pEcb->GetServerVariable( _pEcb->ConnID,
                                            szVariable,
                                            pIsapiString->QueryStr(),
                                            &cbData );
    }

    if ( fResult )
    {
        //
        // Adjust string size
        //

        pIsapiString->SetLen( cbData - sizeof(CHAR) );
    }

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::GetServerVariable(
    CHAR *  szVariable,
    ISAPI_STRINGW * pIsapiStringW
    )
/*++

Purpose:

    Gets a Unicode server variable value.

Arguments:

    szVariable    - The variable to get
    pIsapiStringW - ISAPI_STRINGW object to store the value on return

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbData;
    BOOL    fResult;

    //
    // If the caller is not trying to get a Unicode value,
    // the this is the wrong function to call.  They
    // should pass an ISAPI_STRING instead of an
    // ISAPI_STRINGW to get the correct overloaded function.
    //

    if ( strnicmp( szVariable,
                   "UNICODE_",
                   sizeof("UNICODE_") - 1 ) != 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    //
    // Try and get the variable with the
    // buffer "as is".
    //

    cbData = pIsapiStringW->QueryBufferSize();

    fResult = _pEcb->GetServerVariable( _pEcb->ConnID,
                                        szVariable,
                                        pIsapiStringW->QueryStr(),
                                        &cbData );

    if ( !fResult )
    {
        //
        // If the failure was not due to the size of the
        // buffer, then just fail.
        //

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            goto Failed;
        }

        //
        // Resize the buffer and try again
        //

        if ( !pIsapiStringW->ResizeBuffer( cbData ) )
        {
            goto Failed;
        }

        fResult = _pEcb->GetServerVariable( _pEcb->ConnID,
                                            szVariable,
                                            pIsapiStringW->QueryStr(),
                                            &cbData );
    }

    if ( fResult )
    {
        //
        // Adjust string size
        //

        pIsapiStringW->SetLen( cbData / sizeof(WCHAR) - 1 );
    }

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::ReadAllEntity(
    ISAPI_BUFFER *  pBuffer
    )
/*++

Purpose:

    Reads all request entity into the specified ISAPI_BUFFER.

    Note that this function will fail if the max allocation
    size of the target ISAPI_BUFFER is less than the size of
    the request's entity data.

Arguments:

    pBuffer - The ISAPI_BUFFER to store the result on return

Returns:

    TRUE on success, FALSE on failure

--*/
{
    ISAPI_STRING    TransferEncoding;
    BYTE            pInline[INLINE_READ_SIZE];
    BYTE *          pNewData;
    DWORD           cbRemaining;
    DWORD           cbReadThisTime;
    BOOL            fChunked = FALSE;

    //
    // Is this a chunked request?
    //

    if ( GetServerVariable( "HTTP_TRANSFER_ENCODING",
                            &TransferEncoding ) )
    {
        if ( stricmp( TransferEncoding.QueryStr(), "chunked" ) == 0 )
        {
            fChunked = TRUE;
        }
    }

    //
    // If the request is not chunked, go ahead and resize
    // the buffer now, so that no new allocations will be
    // done while reading stuff in.
    //

    if ( !fChunked && !pBuffer->Resize( _pEcb->cbTotalBytes ) )
    {
        return FALSE;
    }

    //
    // Copy the already available bytes into the buffer
    //

    if ( !pBuffer->SetData( _pEcb->lpbData, _pEcb->cbAvailable ) )
    {
        return FALSE;
    }

    //
    // Loop on ReadClient until we've got all the data
    //

    cbRemaining = _pEcb->cbTotalBytes - _pEcb->cbAvailable;
    pNewData = pInline;

    while ( fChunked || cbRemaining )
    {
        //
        // If the request is not chunked, then read directly
        // into the buffer
        //

        if ( !fChunked )
        {
            pNewData = (BYTE*)pBuffer->QueryPtr() +
                       _pEcb->cbTotalBytes -
                       cbRemaining;
        }

        //
        // Read the data
        //

        cbReadThisTime = cbRemaining;

        if ( fChunked && cbRemaining > INLINE_READ_SIZE )
        {
            cbReadThisTime = INLINE_READ_SIZE;
        }

        if ( !_pEcb->ReadClient( _pEcb->ConnID,
                                 pNewData,
                                 &cbReadThisTime ) )
        {
            //
            // Since the call failed, we should assume
            // that the client is no longer connected.
            //

            _fClientIsConnected = FALSE;

            return FALSE;
        }

        //
        // If the request is not chunked, then we just need
        // to decrement the remaining byte count, else we
        // need to do some extra processing to handle the
        // chunk.
        //

        if ( !fChunked )
        {
            cbRemaining -= cbReadThisTime;
        }
        else
        {
            //
            // If we successfully read zero bytes, then we
            // are done.
            //

            if ( cbReadThisTime == 0 )
            {
                return TRUE;
            }

            //
            // We got some data.  Append it to the buffer.
            //

            if ( !pBuffer->AppendData( pInline, cbReadThisTime ) )
            {
                return FALSE;
            }
        }
    }

    //
    // We need to update the buffer to reflect the data
    // placed there in the read loop.
    //

    if ( !pBuffer->SetDataSize( _pEcb->cbTotalBytes ) )
    {
        return FALSE;
    }

    //
    // Done.
    //

    return TRUE;
}

BOOL
ISAPI_REQUEST::SetBufferedResponseStatus(
    CHAR *  szStatus
    )
/*++

Purpose:

    Sets the HTTP status code to be used in a buffered response

Arguments:

    szStatus - The status (ie. "200 OK") to be sent with the response

Returns:

    TRUE on success, FALSE on failure

--*/
{
    return _StatusBuffer.Copy( szStatus );
}

BOOL
ISAPI_REQUEST::AddHeaderToBufferedResponse(
    CHAR *  szName,
    CHAR *  szValue
    )
/*++

Purpose:

    Adds a response header to be used in a buffered response

Arguments:

    szName  - The name of the header to add
    szValie - The value of the header to add

Returns:

    TRUE on success, FALSE on failure

--*/
{
    ISAPI_STRING    CookedHeader;
    BOOL            fResult;

    //
    // Build a cooked header in the form of
    // "name: value\r\n"
    //

    fResult = CookedHeader.Printf( "%s: %s\r\n",
                                   szName,
                                   szValue );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Append the cooked header into the buffer
    //

    fResult = _HeaderBuffer.Append( CookedHeader.QueryStr() );

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::PrintfToResponseBuffer(
    CHAR *  szFormat,
    ...
    )
/*++

Purpose:

    Appends string data to the response buffer using printf-style
    formatting and a variable number of arguments.

    Note that if this data causes the size of the buffer to exceed
    its max allocation, then this function will fail.

Arguments:

    szFormat - The string format
    ...      - Zero or more additional arguments

Returns:

    TRUE on success, FALSE on failure

--*/
{
    ISAPI_STRING    CurrentChunk( _dwMaxSyncWriteSize );
    va_list         args;
    BOOL            fResult;

    //
    // Build a formatted chunk to append to the response
    // buffer.  We'll use the max sync write size value
    // as the limit to the size that the formatted chunk
    // can be.
    //

    va_start( args, szFormat );

    fResult = CurrentChunk.vsprintf_s( szFormat,
                                     args );

    va_end( args );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Append the chunk to the response buffer.
    //

    fResult = _ResponseBuffer.AppendData( CurrentChunk.QueryStr(),
                                          CurrentChunk.QueryCCH() );

    return fResult;

Failed:

    //
    // No special cleanup required.
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::AddDataToResponseBuffer(
    VOID *  pData,
    DWORD   cbData
    )
/*++

Purpose:

    Adds data to the response buffer.

    Note that if this data causes the size of the buffer to exceed
    its max allocation, then this function will fail.

Arguments:

    pData  - The data to append
    cbData - The size of the data to append

Returns:

    TRUE on success, FALSE on failure

--*/
{
    return _ResponseBuffer.AppendData( pData,
                                       cbData );
}

BOOL
ISAPI_REQUEST::AsyncTransmitBufferedResponse(
    PFN_HSE_IO_COMPLETION   pfnCompletion,
    DWORD                   dwExpireSeconds
    )
/*++

Purpose:

    Asynchronously sends the complete, buffered response
    to the client.
    
    This function automatically calculates and adds an "Expires"
    header, based on the expiration specified by the caller. It
    also calculates and adds a content-length header, and
    keep-alive will be supported if the client requested it.

    Note that various versions of IIS have different capabilities
    for sending asynchronous responses.  This function will
    check the IIS version and call a function to send the data
    in the most appropriate way.

Arguments:

    pfnCompletion   - The function to call on completion of the send
    dwExpireSeconds - If NO_EXPIRATION, then no expiration header will
                      be added to the response.  If 0, then an
                      "Expires: 0" header will be added to the
                      response.  Otherwise a valid expiration time
                      will be calculated and added as an Expires
                      header.  On IIS 6, this will mark the request
                      as eligible for the http.sys response cache.

Returns:

    TRUE on success, FALSE on failure

--*/
{
    CHAR            szContentLength[32];
    ISAPI_STRING    strExpires;
    BOOL            fIIS6Cacheable = FALSE;
    BOOL            fResult;

    //
    // If a status has not been set, use the default
    //

    if ( _StatusBuffer.QueryCCH() == 0 )
    {
        fResult = _StatusBuffer.Copy( DEFAULT_STATUS );

        if ( !fResult )
        {
            goto Failed;
        }
    }

    //
    // Calculate the content-length and add the header.
    // This is required for keep-alive.
    //

    itoa( _ResponseBuffer.QueryDataSize(),
          szContentLength,
          10 );

    fResult = AddHeaderToBufferedResponse( "Content-Length",
                                           szContentLength );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Add an expires header if specified
    //

    if ( dwExpireSeconds != NO_EXPIRATION )
    {
        if ( dwExpireSeconds == IMMEDIATE_EXPIRATION )
        {
            fResult = strExpires.Copy( "0" );
        }
        else
        {
            FILETIME        ft;

            if ( GetCurrentTimeAsFileTime( &ft ) == FALSE )
            {
                goto Failed;
            }

            //
            // Add the cache milliseconds.  Note that FILETIME
            // is a count of 100 nanosecond invervals, thus we
            // need to multiply by 10000000 to get seconds
            //

            ((LARGE_INTEGER*)&ft)->QuadPart += dwExpireSeconds * 10000000;

            fResult = GetFileTimeAsString( &ft, &strExpires );

            //
            // On IIS 6, this response should be marked as
            // cacheable.
            //

            fIIS6Cacheable = TRUE;
        }

        if ( !fResult )
        {
            goto Failed;
        }

        if ( AddHeaderToBufferedResponse( "Expires",
                                          strExpires.QueryStr() ) == FALSE )
        {
            goto Failed;
        }
    }

    //
    // Terminate the header string
    //

    fResult = _HeaderBuffer.Append( "\r\n" );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Call the private member function appropriate to
    // the IIS version being used.
    //

    if ( _dwIISMajorVersion <= 4 )
    {
        fResult = AsyncSendResponseIIS4( pfnCompletion );
    }
    else if ( _dwIISMajorVersion == 5 )
    {
        fResult = AsyncSendResponseIIS5( pfnCompletion );
    }
    else
    {
        fResult = AsyncSendResponseIIS6( pfnCompletion,
                                         fIIS6Cacheable );
    }

    return fResult;


Failed:

    //
    // No special cleanup required
    //

    return FALSE;

}

DWORD
ISAPI_REQUEST::QueryMaxSyncWriteSize(
    VOID
    )
/*++

Purpose:

    Returns the maximum write size supported in a single call to
    one of the following member functions:

      SyncWriteClientArgs
      SyncWriteClientString
      SyncWriteCompleteResponse
      PrintfToResponseBuffer

Arguments:

    None

Returns:

    The maximum write size

--*/
{
    return _dwMaxSyncWriteSize;
}

VOID
ISAPI_REQUEST::SetMaxSyncWriteSize(
    DWORD   dwMaxSyncWriteSize
    )
/*++

Purpose:

    Sets the maximum write size supported in a single call to
    one of the following member functions:

      SyncWriteClientArgs
      SyncWriteClientString
      SyncWriteCompleteResponse
      PrintfToResponseBuffer

Arguments:

    The maximum write size

Returns:

    None

--*/
{
    _dwMaxSyncWriteSize = dwMaxSyncWriteSize;
}

DWORD
ISAPI_REQUEST::QueryMaxResponseBufferSize(
    VOID
    )
/*++

Purpose:

    Returns the maximum size of the response buffer

Arguments:

    None

Returns:

    The maximum response buffer size

--*/
{
    return _ResponseBuffer.QueryMaxAlloc();
}

VOID
ISAPI_REQUEST::SetMaxResponseBufferSize(
    DWORD   dwMaxResponseBufferSize
    )
/*++

Purpose:

    Sets the maximum size of the response buffer

Arguments:

    The maximum buffer size

Returns:

    None

--*/
{
    _ResponseBuffer.SetMaxAlloc( dwMaxResponseBufferSize );
}

EXTENSION_CONTROL_BLOCK *
ISAPI_REQUEST::QueryEcb(
    VOID
    )
/*++

Purpose:

    Returns the ECB associated with this ISAPI_REQUEST object

Arguments:

    None

Returns:

    The EXTENSION_CONTROL_BLOCK

--*/
{
    return _pEcb;
}

DWORD
ISAPI_REQUEST::QueryIISMajorVersion(
    VOID
    )
/*++

Purpose:

    Returns the IIS major version

Arguments:

    None

Returns:

    The IIS major version

--*/
{
    return _dwIISMajorVersion;
}

DWORD
ISAPI_REQUEST::QueryIISMinorVersion(
    VOID
    )
/*++

Purpose:

    Returns the IIS minor version

Arguments:

    None

Returns:

    The IIS minor version

--*/
{
    return _dwIISMinorVersion;
}

BOOL
ISAPI_REQUEST::QueryIsClientConnected(
    VOID
    )
/*++

Purpose:

    Determines if the client is still connected

Arguments:

    None

Returns:

    TRUE if the client is connected, FALSE if not.

--*/
{
    //
    // Check to see if the client is connected
    //
    // Note that this call is somewhat expensive in the
    // OOP case, since an RPC call must occur between the
    // dllhost and inetinfo processes.  It should be used
    // sparingly.
    //
    // In the spirit of this, we will only make the call
    // if we believe that the client is still connected.
    //

    if ( _fClientIsConnected == TRUE )
    {
        if ( _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                           HSE_REQ_IS_CONNECTED,
                                           &_fClientIsConnected,
                                           NULL,
                                           NULL ) == FALSE )
        {
            //
            // If the call fails, we should assume that the
            // client is really still connected.
            //

            _fClientIsConnected = TRUE;
        }
    }

    //
    // If the client has been disconnected, set the last
    // error to WSAECONNRESET, as this is generally the
    // error that will result if an extension attempts to
    // do IO with a client that has disconnected.
    //

    if ( !_fClientIsConnected )
    {
        SetLastError( WSAECONNRESET );
    }

    return _fClientIsConnected;
}

BOOL
ISAPI_REQUEST::UnimpersonateClient(
    VOID
    )
/*++

Purpose:

    Removes an impersonation token from the calling
    thread.  On the first call per object, the authenticated
    client token will be cached so that it can be easily
    reapplied by ImpersonateClient.

Arguments:

    None

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BOOL    fResult = TRUE;

    //
    // If we haven't already got the client's impersonation
    // token, then we need to get it now.
    //

    if ( _hImpersonationToken == NULL )
    {
        fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                                HSE_REQ_GET_IMPERSONATION_TOKEN,
                                                &_hImpersonationToken,
                                                NULL,
                                                NULL );

        if ( !fResult )
        {
            goto Failed;
        }
    }

    return RevertToSelf();

Failed:

    return FALSE;
}

BOOL
ISAPI_REQUEST::ImpersonateClient(
    VOID
    )
/*++

Purpose:

    Sets an impersonation token for the authenticated client
    on the calling thread.  If the token has been previously
    cached, use it.  Otherwise, get the correct token from ISAPI.

Arguments:

    None

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BOOL    fResult;

    if ( _hImpersonationToken == NULL )
    {
        //
        // Get the token from the ISAPI
        //

        fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                                HSE_REQ_GET_IMPERSONATION_TOKEN,
                                                &_hImpersonationToken,
                                                NULL,
                                                NULL );

        if ( !fResult )
        {
            goto Failed;
        }
    }

    return SetThreadToken( NULL, _hImpersonationToken );

Failed:

    return FALSE;
}


BOOL
ISAPI_REQUEST::AsyncSendResponseIIS4(
    PFN_HSE_IO_COMPLETION   pfnCompletion
    )
/*++

Purpose:

    Asynchronously sends the buffered response using calls
    that will work on IIS 4.

Arguments:

    pfnCompletion - The function to call on completion of the I/O

Returns:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   cbResponse;
    BOOL    fResult;

    //
    // Before doing an asynchronous write, we need to set
    // the completion routine.  We'll use a this pointer
    // as the completion context.
    //

    fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                            HSE_REQ_IO_COMPLETION,
                                            pfnCompletion,
                                            NULL,
                                            (DWORD*)this );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // There is no mechanism in IIS 4 and earlier to send
    // status and headers asynchronously, unless you are using
    // TRANSMIT_FILE with a valid file handle.  As a result,
    // we need to synchronously send the status and headers.
    //

    fResult = SyncSendStatusAndHeaders( _StatusBuffer.QueryStr(),
                                        _HeaderBuffer.QueryStr() );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Now, we can send the entity body asynchronously
    //
    // We'll need to properly set the state of this
    // object so that the destructor knows to do
    // DONE_WITH_SESSION.
    //

    cbResponse = _ResponseBuffer.QueryDataSize();

    _fNeedDoneWithSession = TRUE;

    fResult = _pEcb->WriteClient( _pEcb->ConnID,
                                  _ResponseBuffer.QueryPtr(),
                                  &cbResponse,
                                  HSE_IO_ASYNC | HSE_IO_NODELAY );

    if ( !fResult )
    {
        //
        // Reset the state so that this object knows not
        // to do DONE_WITH_SESSION in the destructor
        //

        _fNeedDoneWithSession = FALSE;

        goto Failed;
    }

    return TRUE;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
ISAPI_REQUEST::AsyncSendResponseIIS5(
    PFN_HSE_IO_COMPLETION   pfnCompletion
    )
/*++

Purpose:

    Asynchronously sends the buffered response using calls
    that will work on IIS 5.

Arguments:

    pfnCompletion - The function to call on completion of the I/O

Returns:

    TRUE on success, FALSE on failure

--*/
{
    HSE_TF_INFO TfInfo;
    BOOL        fResult;

    TfInfo.pfnHseIO = pfnCompletion;
    TfInfo.pszStatusCode = _StatusBuffer.QueryStr();

    //
    // We'll send the buffered response headers in the
    // pHead member.
    //

    TfInfo.pHead = _HeaderBuffer.QueryStr();
    TfInfo.HeadLength = _HeaderBuffer.QueryCCH() + sizeof(CHAR);

    //
    // Since we're not sending a file, we'll use a NULL
    // file handle and set the offset and length to zero
    //

    TfInfo.hFile = NULL;
    TfInfo.Offset = 0;
    TfInfo.BytesToWrite = 0;

    //
    // We'll send the buffered response entity in the tail
    //

    TfInfo.pTail = _ResponseBuffer.QueryPtr();
    TfInfo.TailLength = _ResponseBuffer.QueryDataSize();

    //
    // Pass a pointer to this object as the context.
    //
    // Note that it is very important that this object
    // was allocated via "new" and not declared as an
    // object on HttpExtensionProc's stack.  If it was
    // declared on the stack, intermittent failures may
    // result, depending on whether HttpExtensionProc
    // returns before or after the completion.
    //

    TfInfo.pContext = this;

    //
    // Set the following flags:
    //
    // HSE_IO_ASYNC        - Always required for TRANSMIT_FILE
    // HSE_IO_SEND_HEADERS - Have TRANSMIT_FILE send the response headers
    // HSE_IO_NODELAY      - Dont nagle (IIS 5 only. This no-ops on IIS 6)
    //

    TfInfo.dwFlags = HSE_IO_ASYNC | HSE_IO_SEND_HEADERS | HSE_IO_NODELAY;

    //
    // Transmit the data
    //
    // Note that there is some state housekeeping that we need to do
    // here.  After HttpExtensionProc calls this function, it will
    // need to return HSE_STATUS_PENDING.  Cleanup of the request,
    // including this object, will then be done in the completion
    // routine, which will generally just delete this object. The
    // destructor for this object will call HSE_REQ_DONE_WITH_SESSION
    // if the below flag is so set.
    //
    // It is important to consider that the completion routine
    // might get called before any code following ServerSupportFunction
    // has a chance to run.  If this happens, the _fNeedDoneWithSession
    // flag needs to be already set for the destructor to work
    // properly.  Also, any attempts to touch members of this object
    // will result in an access violation.  Such failures may be
    // difficult to debug.
    //

    _fNeedDoneWithSession = TRUE;

    fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                            HSE_REQ_TRANSMIT_FILE,
                                            &TfInfo,
                                            NULL,
                                            NULL );

    if ( !fResult )
    {
        // 
        // Since the ServerSupportFunction failed, we know that no
        // completion will occur.  It is therefore safe for us to
        // touch members of this object.
        //
        // We will do so now, to reset the below flag back to the
        // state where HSE_REQ_DONE_WITH_SESSION is not called by
        // the destructor.
        //

        _fNeedDoneWithSession = FALSE;
    }

    return fResult;
}

BOOL
ISAPI_REQUEST::AsyncSendResponseIIS6(
    PFN_HSE_IO_COMPLETION   pfnCompletion,
    BOOL                    fCacheResponse
    )
/*++

Purpose:

    Asynchronously sends the buffered response using calls
    that will work on IIS 6

Arguments:

    pfnCompletion  - The function to call on completion of the I/O
    fCacheResponse - If TRUE, then the response will be marked as
                     cacheable in the http.sys response cache.  Note
                     that this is just a "suggestion" to http.sys.
                     If something about the request disqualifies it
                     for caching, then http.sys is not required to
                     add this response to the cache.

Returns:

    TRUE on success, FALSE on failure

--*/
{
    HSE_VECTOR_ELEMENT  Element;
    HSE_RESPONSE_VECTOR Vector;
    BOOL                fResult;

    //
    // Declare a single element containing the contents
    // of the response buffer.
    //

    Element.ElementType = HSE_VECTOR_ELEMENT_TYPE_MEMORY_BUFFER;
    Element.pvContext = _ResponseBuffer.QueryPtr();
    Element.cbOffset = 0;
    Element.cbSize = _ResponseBuffer.QueryDataSize();

    //
    // Declare our response vector containing a single
    // element in the array, and using the ISAPI_REQUEST's
    // status and headers.
    //
    // We will set the following IO flags for all calls:
    //
    //   HSE_IO_ASYNC        - Do an asynchronous send
    //   HSE_IO_FINAL_SEND   - Optimization, since we know
    //                         that this send is the complete
    //                         response (note that this is
    //                         required to be able to cache
    //                         the response in http.sys)
    //   HSE_IO_SEND_HEADERS - Specifies that we are sending
    //                         response headers in this call
    //

    Vector.dwFlags = HSE_IO_ASYNC | HSE_IO_FINAL_SEND | HSE_IO_SEND_HEADERS;

    //
    // If the response is cacheable, then we should add
    // HSE_IO_CACHE_RESPONSE
    //

    if ( fCacheResponse )
    {
        Vector.dwFlags |= HSE_IO_CACHE_RESPONSE;
    }

    //
    // Set the status and headers
    //

    Vector.pszStatus = _StatusBuffer.QueryStr();
    Vector.pszHeaders = _HeaderBuffer.QueryStr();

    //
    // Finally, we have a single element array.  Note
    // that the elements of this array need to stay valid
    // until the completion occurs.  This will be taken
    // care of automatically by the ISAPI_REQUEST object,
    // since it doesn't free the response buffer until
    // the completion.
    //

    Vector.nElementCount = 1;
    Vector.lpElementArray = &Element;

    //
    // Before doing an asynchronous vector send, we need to set
    // the completion routine.  We'll use a this pointer
    // as the completion context.
    //

    fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                            HSE_REQ_IO_COMPLETION,
                                            pfnCompletion,
                                            NULL,
                                            (DWORD*)this );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Send the data
    //
    // Note that there is some state housekeeping that we need to do
    // here.  After HttpExtensionProc calls this function, it will
    // need to return HSE_STATUS_PENDING.  Cleanup of the request,
    // including this object, will then be done in the completion
    // routine, which will generally just delete this object. The
    // destructor for this object will call HSE_REQ_DONE_WITH_SESSION
    // if the below flag is so set.
    //
    // It is important to consider that the completion routine
    // might get called before any code following ServerSupportFunction
    // has a chance to run.  If this happens, the _fNeedDoneWithSession
    // flag needs to be already set for the destructor to work
    // properly.  Also, any attempts to touch members of this object
    // will result in an access violation.  Such failures may be
    // difficult to debug.
    //

    _fNeedDoneWithSession = TRUE;

    fResult = _pEcb->ServerSupportFunction( _pEcb->ConnID,
                                            HSE_REQ_VECTOR_SEND,
                                            &Vector,
                                            NULL,
                                            NULL );

    if ( !fResult )
    {
        // 
        // Since the ServerSupportFunction failed, we know that no
        // completion will occur.  It is therefore safe for us to
        // touch members of this object.
        //
        // We will do so now, to reset the below flag back to the
        // state where HSE_REQ_DONE_WITH_SESSION is not called by
        // the destructor.
        //

        _fNeedDoneWithSession = FALSE;
    }

    return fResult;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

DWORD
ISAPI_REQUEST::QueryOsMajorVersion(
    VOID
    )
/*++

Purpose:

    Returns the major version of the OS.

    Since this requires a system call, the result will be
    stored in a static member of this function so that we
    only need to do it once for all ISAPI_REQUEST instances.

Arguments:

    None

Returns:

    The OS major version

--*/
{
    OSVERSIONINFO   OsVersionInfo;
    BOOL            fResult;

    //
    // If we've already determined the major version,
    // use the data already acquired.
    //
    // Note that _dwOsMajorVersion is static, so we don't
    // do the below system call for every instance of this
    // object.
    //

    if ( _dwOsMajorVersion != 0 )
    {
        goto Done;
    }

    //
    // Otherwise, get it
    //

    fResult = GetVersionEx( &OsVersionInfo );

    if ( !fResult )
    {
        //
        // Hmmm.  Better to assume 4, I guess.
        //

        _dwOsMajorVersion = 4;

        goto Done;
    }

    _dwOsMajorVersion = OsVersionInfo.dwMajorVersion;

Done:

    return _dwOsMajorVersion;
}

DWORD
SyncSendGenericServerError(
    EXTENSION_CONTROL_BLOCK *   pecb
    )
/*++

Purpose:

    Synchronously sends a generic "500 Server Error" response

Arguments:

    pecb - The EXTENSION_CONTROL_BLOCK of this request

Returns:

    HSE_STATUS_ERROR

--*/
{
    ISAPI_REQUEST   Request( pecb );

    Request.SyncWriteCompleteResponse( "500 Server Error",
                                       "Content-Type: text/html\r\n\r\n",
                                       SERVER_ERROR );

    return HSE_STATUS_ERROR;
}



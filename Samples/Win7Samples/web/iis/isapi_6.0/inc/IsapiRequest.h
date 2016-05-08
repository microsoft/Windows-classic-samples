/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiRequest.h

Abstract:

    A class to do common ISAPI extension
    processing tasks

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapirequest_h
#define _isapirequest_h

#include <IsapiTools.h>
#include <httpext.h>

#define DEFAULT_STATUS                  "200 OK"
#define DEFAULT_HEADERS                 "\r\n"
#define INLINE_READ_SIZE                2048
#define DEFAULT_RESPONSE_BUFFER_SIZE    32 * 1024

//
// Values for use with AsyncTransmitBufferedResponse
// cache expiration times.
//

#define NO_EXPIRATION                   0xffffffff
#define IMMEDIATE_EXPIRATION            0

class ISAPI_REQUEST
{
public:

    ISAPI_REQUEST(
        EXTENSION_CONTROL_BLOCK *   pEcb
        );

    virtual
    ~ISAPI_REQUEST(
        VOID
        );

    BOOL
    SyncSendStatusAndHeaders(
        CHAR *  szStatus = NULL,
        CHAR *  szHeaders = NULL
        );

    BOOL
    SyncWriteClientArgs(
        CHAR *  szFormat,
        va_list args
        );

    BOOL
    SyncWriteClientString(
        CHAR *  szFormat,
        ...
        );

    BOOL
    SyncWriteCompleteResponse(
        CHAR *  szStatus,
        CHAR *  szHeaders,
        CHAR *  szFormat,
        ...
        );

    BOOL
    GetServerVariable(
        CHAR *          szVariable,
        ISAPI_STRING *  pIsapiString
        );

    BOOL
    GetServerVariable(
        CHAR *  szVariable,
        ISAPI_STRINGW * pIsapiStringW
        );

    BOOL
    ReadAllEntity(
        ISAPI_BUFFER *  pBuffer
        );

    BOOL
    SetBufferedResponseStatus(
        CHAR *  szStatus
        );

    BOOL
    AddHeaderToBufferedResponse(
        CHAR *  szName,
        CHAR *  szValue
        );

    BOOL
    PrintfToResponseBuffer(
        CHAR *  szFormat,
        ...
        );

    BOOL
    AddDataToResponseBuffer(
        VOID *  pData,
        DWORD   cbData
        );

    BOOL
    AsyncTransmitBufferedResponse(
        PFN_HSE_IO_COMPLETION   pfnCompletion,
        DWORD                   dwExpireSeconds = NO_EXPIRATION
        );

    DWORD
    QueryMaxSyncWriteSize(
        VOID
        );

    VOID
    SetMaxSyncWriteSize(
        DWORD   dwMaxSyncWriteSize
        );

    DWORD
    QueryMaxResponseBufferSize(
        VOID
        );

    VOID
    SetMaxResponseBufferSize(
        DWORD   dwMaxResponseBufferSize
        );

    EXTENSION_CONTROL_BLOCK *
    QueryEcb(
        VOID
        );

    DWORD
    QueryIISMajorVersion(
        VOID
        );

    DWORD
    QueryIISMinorVersion(
        VOID
        );

    BOOL
    QueryIsClientConnected(
        VOID
        );

    BOOL
    UnimpersonateClient(
        VOID
        );

    BOOL
    ImpersonateClient(
        VOID
        );

private:

    EXTENSION_CONTROL_BLOCK *   _pEcb;
    ISAPI_STRING                _StatusBuffer;
    ISAPI_STRING                _HeaderBuffer;
    ISAPI_BUFFER                _ResponseBuffer;
    DWORD                       _dwMaxSyncWriteSize;
    DWORD                       _dwIISMajorVersion;
    DWORD                       _dwIISMinorVersion;
    static DWORD                _dwOsMajorVersion;
    BOOL                        _fClientIsConnected;
    BOOL                        _fNeedDoneWithSession;
    HANDLE                      _hImpersonationToken;

    BOOL
    AsyncSendResponseIIS4(
        PFN_HSE_IO_COMPLETION   pfnCompletion
        );

    BOOL
    AsyncSendResponseIIS5(
        PFN_HSE_IO_COMPLETION   pfnCompletion
        );

    BOOL
    AsyncSendResponseIIS6(
        PFN_HSE_IO_COMPLETION   pfcCompletion,
        BOOL                    fCacheResponse
        );

    DWORD
    QueryOsMajorVersion(
        VOID
        );
};

DWORD
SyncSendGenericServerError(
    EXTENSION_CONTROL_BLOCK *   pecb
    );

#endif  // _isapirequest_h
/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiBuffer.h

Abstract:

    A simple buffer class

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapibuffer_h
#define _isapibuffer_h

#include <IsapiTools.h>

//
// Set some default values for the buffer object
//
// DEFAULT_MAX_ALLOC_SIZE - The maximum value allowed by
//                          the ISABUFFER::Resize method.
//
// INLINE_BUFFER_SIZE     - The size of the inline storage
//                          buffer for the ISABUFFER object
//
//

#define DEFAULT_MAX_ALLOC_SIZE       16384
#define INLINE_BUFFER_SIZE             256

class ISAPI_BUFFER
{
public:
    
    ISAPI_BUFFER(
        DWORD   dwMaxAlloc = 0 // 0 defaults to DEFAULT_MAX_ALLOC_SIZE
        );

    virtual
    ~ISAPI_BUFFER();

    BOOL
    SetData(
        VOID *  pData,
        DWORD   cbData
        );

    BOOL
    AppendData(
        VOID *  pData,
        DWORD   cbData
        );

    BOOL
    Resize(
        DWORD   cbNewSize
        );

    VOID
    Reset(
        BOOL    fDealloc = FALSE
        );

    BOOL
    SetDataSize(
        DWORD   cbNewSize
        );

    VOID
    SetMaxAlloc(
        DWORD   dwMaxAlloc
        );

    DWORD
    QueryMaxAlloc(
        VOID
        );

    VOID *
    QueryPtr(
        VOID
        );

    DWORD
    QueryDataSize(
        VOID
        );

    DWORD
    QueryBufferSize(
        VOID
        );

    BOOL
    Base64Decode(
        CHAR *  szString
        );

    VOID
    ZeroBuffer(
        VOID
        );

private:

    VOID *  _pData;
    BYTE    _InlineData[INLINE_BUFFER_SIZE];
    DWORD   _cbData;
    DWORD   _cbBuffer;
    DWORD   _dwMaxAlloc;
};

#endif  // _isapibuffer_h
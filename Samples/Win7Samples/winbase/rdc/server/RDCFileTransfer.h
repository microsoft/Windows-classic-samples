// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// RdcFileTransfer.h : Declaration of the CRdcFileTransfer

#pragma once
#include "resource.h"

#include "RdcSdkTestServer.h"


// CRdcFileTransfer
/*
    Implement the main (only at the moment) COM interface.
 
    This interface allows the client to open files, then
    read the signature and file data from them.
 
    RdcOpenFile() returns an "RdcfileHandle" which must be
    used when calling ReadData() and Close(). This allows the client
    to have several files open at a time using only one RdcFileTransfer object.
 
 
    RdcFileOpen() prepares for signature generation (or validation
    of existing signature files).
    Actual signature generation is delayed until ReadData() is used to
    try to read the signatures. This design allows for the signature generation
    to be done in a seperate thread - started by RdcOpenFile().
 
    LIMITATIONS:
        The signature files are *ALWAYS* created as "filename_%d.sig" in the same
        directory as the file being transferred. The signature files do not get deleted.
 
        The signature files are *NOT* checked for integrity. If for some reason, the
        signature files are corrupted in anyway, this server will not detect this
        problem.  The sample should write a custom header after the signatures are
        generated to be used as a check to ensure integrity.
 
        Also, the filedates of the signature files should be checked.
 
        Individual RdcFileHandles are not thread safe -- do not try to issue multiple
        reads against a single filehandle from different threads.
 
 
        This RDC SDK COM server cannot be marshalled - therefore it must be
        run inproc, and in the same apartment.
        (The client app must use CoInitializeEx(0, COINIT_MULTITHREADED).
 
 */
class RdcFileHandleImpl;

class ATL_NO_VTABLE CRdcFileTransfer :
            public CComObjectRootEx<CComMultiThreadModel>,
            public CComCoClass<CRdcFileTransfer, &CLSID_RdcFileTransfer>,
            public IRdcFileTransfer
{
public:
    CRdcFileTransfer()
    {}

    DECLARE_REGISTRY_RESOURCEID ( IDR_RDCFILETRANSFER )


    BEGIN_COM_MAP ( CRdcFileTransfer )
    COM_INTERFACE_ENTRY ( IRdcFileTransfer )
    END_COM_MAP()


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    DebugHresult FinalConstruct();

    void FinalRelease();

public:

    STDMETHODIMP RdcOpenFile (
        LPCWSTR fileName,
        RdcFileTransferInfo * fileInfo,
        RdcFileHandle * fileHandle,
        BOOL deleteSigs,
        ULONG horizonSize1,
        ULONG horizonSizeN,
        ULONG hashWindowSize1,
        ULONG hashWindowSizeN );

    STDMETHOD ( ReadData ) (
        RdcFileHandle * fileHandle,
        ULONG signatureLevel,
        ULONGLONG fileOffset,
        ULONG bytesToRead,
        ULONG *bytesActuallyRead,
        BYTE * data );

    STDMETHOD ( GetFileSize ) (
        RdcFileHandle * fileHandle,
        ULONG signatureLevel,
        ULONGLONG * fileSize );

    STDMETHOD ( GetSimilarityData ) (
        SimilarityData * similarityData );

    STDMETHOD ( Close ) (
        RdcFileHandle * fileHandle );

private:
    // Ensure thread-safe access
    CComAutoCriticalSection m_HandlesLock;

    // The list of open file handles.
    RdcSmartArray<RdcFileHandleImpl * > m_RdcFileHandles;
};

OBJECT_ENTRY_AUTO ( __uuidof ( RdcFileTransfer ), CRdcFileTransfer )

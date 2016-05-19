//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Rostream.h
//
// Abstract:            Definition for class CROStream, which implements
//                      the IStream interface.
//
//*****************************************************************************

#ifndef ROSTREAM_H_INCLUDED
#define ROSTREAM_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <objidl.h>

//////////////////////////////////////////////////////////////////////////////
class CROStream : public IStream
{
public:

    CROStream();

    //
    // IUnknown methods
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */  REFIID riid,
                                              /* [out] */ void **ppvObject );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    //
    // Methods of IStream
    //
    HRESULT STDMETHODCALLTYPE Read( void *pv, ULONG cb, ULONG *pcbRead );
    HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition );
    HRESULT STDMETHODCALLTYPE Stat( STATSTG *pstatstg, DWORD grfStatFlag );

    //
    // Non-implemented methods of IStream
    //
    HRESULT STDMETHODCALLTYPE Write( void const *pv, ULONG cb, ULONG *pcbWritten )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER libNewSize )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE CopyTo( IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE Commit( DWORD grfCommitFlags )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE Revert()
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE UnlockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE Clone( IStream **ppstm )
    {
        return( E_NOTIMPL );
    }

    //
    // CROStream method
    //
    HRESULT Open( /* [in] */ LPCTSTR ptszURL );

protected:

    ~CROStream();

    HANDLE  m_hFile;
    LONG    m_cRef;
};

#endif  // ROSTREAM_H_INCLUDED

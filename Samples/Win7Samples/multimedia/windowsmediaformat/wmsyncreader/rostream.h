//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            rostream.h
//
// Abstract:            Declaration of the CROStream class
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
    ~CROStream();

    HRESULT Open( LPCTSTR pwszURL );

    //
    // Methods of IStream
    //
    HRESULT STDMETHODCALLTYPE Read( void *pv, ULONG cb, ULONG *pcbRead );
    HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition );
    HRESULT STDMETHODCALLTYPE Stat( STATSTG *pstatstg, DWORD grfStatFlag );

    //
    // IUnknown methods
    //
    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppv );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE  Release();


    //
    // Unimplemented methods of IStream
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

protected:
    HANDLE  m_hFile;
    LONG    m_cRefs;
};


#endif  // ROSTREAM_H_INCLUDED

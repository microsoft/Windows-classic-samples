//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Rostream.cpp
//
// Abstract:            Implementation of class CROStream which implements IStream.
//
//*****************************************************************************
#include "ROStream.h"


//------------------------------------------------------------------------------
// Name: CROStream::CROStream()
// Desc: Constructor.
//------------------------------------------------------------------------------
CROStream::CROStream() :
    m_cRef( 1 ),
    m_hFile( INVALID_HANDLE_VALUE )
{
}

//------------------------------------------------------------------------------
// Name: ~CROStream()
// Desc: Destructor.
//------------------------------------------------------------------------------
CROStream::~CROStream()
{
    if( INVALID_HANDLE_VALUE != m_hFile )
    {
        CloseHandle( m_hFile );
    }
}

//------------------------------------------------------------------------------
// Implementation of IUnknown methods.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CROStream::QueryInterface( /* [in] */ REFIID riid,
                                                     /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject ) 
{
    if( ( IID_IStream == riid ) ||
        ( IID_IUnknown == riid ) )
    {
        *ppvObject = static_cast< IStream* >( this );
        AddRef();
        return( S_OK );
    }

    *ppvObject = NULL;
    return( E_NOINTERFACE );
}

ULONG STDMETHODCALLTYPE CROStream::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}

ULONG STDMETHODCALLTYPE CROStream::Release()
{
    if( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return( 0 );
    }

    return( m_cRef );
}

//------------------------------------------------------------------------------
// Name: CROStream::Open()
// Desc: Opens the file.
//------------------------------------------------------------------------------
HRESULT CROStream::Open( LPCTSTR ptszURL )
{
    //
    // Open the file
    //
    m_hFile = CreateFile(
                    ptszURL,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

    if( INVALID_HANDLE_VALUE == m_hFile )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    //
    // Make sure the file is a disk file, not some device.
    //
    if( FILE_TYPE_DISK != GetFileType( m_hFile ) )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
        return( E_FAIL );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CROStream::Read()
// Desc: Implementation of IStream method.
//------------------------------------------------------------------------------
HRESULT CROStream::Read( void *pv, ULONG cb, ULONG *pcbRead )
{
    if( INVALID_HANDLE_VALUE == m_hFile )
    {
        return( E_UNEXPECTED );
    }

    if( !ReadFile( m_hFile, pv, cb, pcbRead, NULL ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CROStream::Seek()
// Desc: Implementation of IStream method.
//------------------------------------------------------------------------------

HRESULT CROStream::Seek( LARGE_INTEGER dlibMove,
                         DWORD dwOrigin,
                         ULARGE_INTEGER *plibNewPosition )
{
    DWORD dwMoveMethod;

    if( INVALID_HANDLE_VALUE == m_hFile )
    {
        return( E_UNEXPECTED );
    }

    switch( dwOrigin )
    {
        case STREAM_SEEK_SET:
            dwMoveMethod = FILE_BEGIN;
            break;

        case STREAM_SEEK_CUR:
            dwMoveMethod = FILE_CURRENT;
            break;

        case STREAM_SEEK_END:
            dwMoveMethod = FILE_END;
            break;

        default:
            return( E_INVALIDARG );
    };

    DWORD dwPos = SetFilePointer( m_hFile, dlibMove.LowPart, NULL, dwMoveMethod );

    if( 0xffffffff == dwPos )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if( NULL != plibNewPosition )
    {
        plibNewPosition->LowPart = dwPos;
        plibNewPosition->HighPart = 0;
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CROStream::Stat()
// Desc: Implementation of IStream method.
//------------------------------------------------------------------------------
HRESULT CROStream::Stat( STATSTG *pstatstg, DWORD grfStatFlag )
{
    if( ( NULL == pstatstg ) || ( STATFLAG_NONAME != grfStatFlag ) )
    {
        return( E_INVALIDARG );
    }

    if( INVALID_HANDLE_VALUE == m_hFile )
    {
        return( E_UNEXPECTED );
    }

    DWORD dwFileSize = GetFileSize( m_hFile, NULL );

    if( 0xffffffff == dwFileSize )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    memset( pstatstg, 0, sizeof( STATSTG ) );

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.LowPart = dwFileSize;

    return( S_OK );
}

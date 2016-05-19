//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            rostream.cpp
//
// Abstract:            Implementation of CROStream, which is an IStream-
//						derived data sourcing class. The IWMSyncReader 
//						interface is capable of sourcing through a custom object 
//						supporting the IStream interface. The CROStream class is 
//						used in the sample to demonstrate this capability.
//
//*****************************************************************************

#include <malloc.h>
#include "ROStream.h"


//////////////////////////////////////////////////////////////////////////////
//////
CROStream::CROStream() :
    m_cRefs( 1 ),
    m_hFile( INVALID_HANDLE_VALUE )
{
}

//////////////////////////////////////////////////////////////////////////////
/////
CROStream::~CROStream()
{
    if( INVALID_HANDLE_VALUE != m_hFile )
    {
        CloseHandle( m_hFile );
    }
}

//////////////////////////////////////////////////////////////////////////////
/////
HRESULT CROStream::Open( LPCTSTR ptszURL )
{

    HRESULT hr = S_OK;
    LPSTR pszURL = NULL;

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

    if( FILE_TYPE_DISK != GetFileType( m_hFile ) )
    {
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
        return( E_FAIL );
    }

    return( hr );
}

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Read( void *pv, ULONG cb, ULONG *pcbRead )
{
    if( !ReadFile( m_hFile, pv, cb, pcbRead, NULL ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    return( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Seek(
                                    LARGE_INTEGER dlibMove,
                                    DWORD dwOrigin,
                                    ULARGE_INTEGER *plibNewPosition )
{
    DWORD dwMoveMethod;

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

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Stat( STATSTG *pstatstg, DWORD grfStatFlag )
{
    if( ( NULL == pstatstg ) || ( STATFLAG_NONAME != grfStatFlag ) )
    {
        return( E_INVALIDARG );
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

//////////////////////////////////////////////////////////////////////////////
//  IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT CROStream::QueryInterface( REFIID riid, void **ppv )
{
    if( ( IID_IUnknown == riid ) || ( IID_IStream == riid ) )
    {
        *ppv = this;
        AddRef();

        return( S_OK );
    }
        
    *ppv = NULL;
    return( E_NOINTERFACE );
}


//////////////////////////////////////////////////////////////////////////////
ULONG CROStream::AddRef()
{
    return( InterlockedIncrement( &m_cRefs ) );
}


//////////////////////////////////////////////////////////////////////////////
ULONG CROStream::Release()
{
    if( 0 == InterlockedDecrement( &m_cRefs ) )
    {
        delete this;
        return( 0 );
    }
    
    return( 0xbad );
}


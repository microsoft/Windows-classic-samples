//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            SDKSampleStorageSystem.cpp
//
// Abstract:
//
//*****************************************************************************

#include "stdafx.h"
#include "SDKSampleStoragePlugin.h"
#include "SDKSampleStorageSystem.h"

#include <float.h>
/////////////////////////////////////////////////////////////////////////////
//
// [CSDKSampleStorageSystem]
//
/////////////////////////////////////////////////////////////////////////////
CSDKSampleStorageSystem::CSDKSampleStorageSystem()
{
    m_pServerContext = NULL;
    m_pClassFactory = NULL;
} // End of CSDKSampleStorageSystem.

/////////////////////////////////////////////////////////////////////////////
CSDKSampleStorageSystem::~CSDKSampleStorageSystem()
{
    if ( m_pServerContext )
    {
        m_pServerContext->Release();
        m_pServerContext = NULL;
    }
    if ( m_pClassFactory )
    {
        m_pClassFactory->Release();
        m_pClassFactory = NULL;
    }
}





/////////////////////////////////////////////////////////////////////////////
//
// [OpenDataContainer]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::OpenDataContainer( 
                            IWMSCommandContext *pCommandContext,
                            IWMSContext *pUserContext,
                            IWMSContext *pPresentationContext,
                            LPWSTR pszContainerName,
                            DWORD dwFlags,
                            IWMSBufferAllocator *pBufferAllocator,
                            IWMSDataSourcePluginCallback *pCallback,
                            QWORD qwContext
                            )
{
    HRESULT hr = S_OK;
    CSampleDataContainer *pDataContainer = NULL;

    if ( ( NULL == pszContainerName )
        || ( NULL == pBufferAllocator )
        || ( NULL == pCallback ) )
    {
        // We are returning an error, so we cannot call the callback.
        return( E_INVALIDARG );
    }

    pDataContainer = new CSampleDataContainer;
    
    if ( NULL == pDataContainer )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }
    
    hr = pDataContainer->Initialize(
                            pUserContext,
                            pszContainerName, 
                            dwFlags,
                            pBufferAllocator,
                            this,
                            pCallback,
                            qwContext
                            );
    if ( FAILED( hr ) )
    {
        goto abort;
    }

abort:
    if( NULL != pDataContainer )
    {
        pDataContainer->Release();
        pDataContainer = NULL;
    }

    return( hr );

    
} // End of OpenDataContainer.






/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::OpenDirectory( 
                            IWMSCommandContext *pCommandContext,
                            IWMSContext *pUserContext,
                            IWMSContext *pPresentationContext,
                            LPWSTR pszContainerName,
                            DWORD dwFlags,
                            IWMSBufferAllocator *pBufferAllocator,
                            IWMSDataSourcePluginCallback *pCallback,
                            QWORD qwContext
                            )
{
    HRESULT hr = S_OK;
    CSampleDirectory *pDirectory = NULL;

    if( ( NULL == pszContainerName )
        || ( NULL == pCallback ) )
    {
        hr = E_INVALIDARG;
        goto abort;
    }

    pDirectory = new CSampleDirectory;
    if( NULL == pDirectory )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    hr = pDirectory->Initialize( 
                            pUserContext,
                            pszContainerName,
                            this,
                            pCallback,
                            qwContext
                            );
    if( FAILED( hr ) )
    {
        goto abort;
    }

abort:
    if( NULL != pDirectory )
    {
        pDirectory->Release();
        pDirectory = NULL;
    }

    return( hr );
} // OpenDirectory





/////////////////////////////////////////////////////////////////////////////
//
// [DeleteDataContainer]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::DeleteDataContainer( 
                        LPWSTR pszContainerName,
                        DWORD dwFlags,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext
                        )
{
    HRESULT hr = S_OK;
    BOOL fSuccess = TRUE;

    if ( ( NULL == pszContainerName )
        || ( NULL == pCallback ) )
    {
        hr = E_INVALIDARG;
        goto abort;
    }

    fSuccess = DeleteFile( pszContainerName );
    if ( !fSuccess )
    {            
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }

abort:
    if ( NULL != pCallback )
    {
        // Ignore any error returned by the callback.
        pCallback->OnDeleteDataContainer( hr, qwContext );
        hr = S_OK; // any error is passed to the callback
    }

    return( hr );
    
} // End of DeleteDataContainer.




/////////////////////////////////////////////////////////////////////////////
//
// [GetDataContainerVersion]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::GetDataContainerVersion( 
                        IWMSCommandContext *pCommandContext,
                        IWMSContext *pUserContext,
                        IWMSContext *pPresContext,
                        LPWSTR pszContainerName,
                        DWORD dwFlags,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext
                        )
{
    
    HRESULT hr = S_OK;
    DATE dateOriginLastModifiedTime;
    WIN32_FILE_ATTRIBUTE_DATA FileAttributeData;
    WIN32_FILE_ATTRIBUTE_DATA * pFileAttributeData;
    SYSTEMTIME stLastModifiedTime;
    IWMSDataContainerVersion *pVersion = NULL;
    QWORD qwFileSize = 0;
    BSTR bstrFileSize = NULL;
    WCHAR pszFileSize[65];

    if ( ( NULL == pszContainerName )
        || ( NULL == pCallback ) )
    {
        // We are returning an error, so we cannot call the callback.
        return( E_INVALIDARG );
    }

    if( !GetFileAttributesEx( pszContainerName + URL_SCHEME_LENGTH, GetFileExInfoStandard, &FileAttributeData ) )
    {
    
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }

    // can't get the DataContainerVersion for a directory
    if( FILE_ATTRIBUTE_DIRECTORY & FileAttributeData.dwFileAttributes )
    {
        hr = E_FAIL;
        goto abort;
    }

    pFileAttributeData = &FileAttributeData;

    //
    // Convert the last modified time from FILETIME to DATE format.
    //
    FileTimeToSystemTime( &pFileAttributeData->ftLastWriteTime, &stLastModifiedTime );
    SystemTimeToVariantTime( &stLastModifiedTime, &dateOriginLastModifiedTime );

    //
    // A null pVersion means (a) we got an error early on or (b) the caller
    // only wants to get the current version.
    //
    hr = m_pClassFactory->CreateInstance( IID_IWMSDataContainerVersion,
                                          (void **) &pVersion );
    if( FAILED( hr ) )
    {
        goto abort;
    }

    hr = pVersion->SetLastModifiedTime( dateOriginLastModifiedTime );
    if( FAILED( hr ) )
    {
        goto abort;
    }

    hr = pVersion->SetExpirationTime( (double) DBL_MAX );
    if( FAILED( hr ) )
    {
        goto abort;
    }

    //
    // everything allowed (this will be replaced with the VRoot setting)
    //
    hr = pVersion->SetCacheFlags( 0xffffffff );
    if( FAILED( hr ) )
    {
        goto abort;
    }

    hr = pVersion->SetContentSize( pFileAttributeData->nFileSizeLow, pFileAttributeData->nFileSizeHigh );
    if( FAILED( hr ) )
    {
        goto abort;
    }

    qwFileSize = MAKEQWORD( pFileAttributeData->nFileSizeLow, pFileAttributeData->nFileSizeHigh );
    _ui64tow_s( qwFileSize, (LPWSTR) pszFileSize,65, 10 );
    bstrFileSize = SysAllocString( (LPWSTR) pszFileSize );
    if( NULL == bstrFileSize )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    hr = pVersion->SetEntityTag( bstrFileSize );
    if( FAILED(hr) )
    {
        goto abort;
    }
abort:
    // The HRESULT returned by the callback should be ignored.
    pCallback->OnGetDataContainerVersion(
                                        hr,
                                        pVersion,
                                        qwContext
                                        );
    hr = S_OK; // any error is passed to the callback
     
    if( NULL != pVersion )
    {
        pVersion->Release();
        pVersion = NULL;
    }
    if( NULL != bstrFileSize )
    {
        SysFreeString( bstrFileSize );
        bstrFileSize = NULL;
    }
    // We called the callback, so we MUST return S_OK.
    return( S_OK );
} // End of GetDataContainerVersion.






/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::GetRootDirectories( 
                            LPWSTR *pstrRootDirectoryList,
                            DWORD dwMaxRoots,
                            IWMSDataSourcePluginCallback *pCallback,
                            QWORD qwContext
                            )
{
    HRESULT hr = S_OK;
    DWORD dwRootNum = 0;
    DWORD dwTotalNumRoots = 0;
    DWORD dwBufferSize = 0;
    WCHAR tempBuffer[3];
    WCHAR *pFullBuffer = NULL;
    WCHAR *pEndBuffer;
    WCHAR *pStartPath;
    WCHAR *pStopPath;
    WCHAR *pstrFile;
    DWORD dwPathNameLength;
    DWORD dwIndex;


    if( ( NULL == pstrRootDirectoryList ) 
        || ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    // Find out how large the buffer should be.
    dwBufferSize = GetLogicalDriveStringsW( 1, tempBuffer );
    if( 0 == dwBufferSize )
    {
        hr = E_FAIL;
        goto abort;
    }

    // Allocate a buffer.
    pFullBuffer = new WCHAR[ dwBufferSize + 2 ];
    if( NULL == pFullBuffer )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    ZeroMemory( pFullBuffer, ( dwBufferSize + 2 ) * sizeof( WCHAR ) );

    // Get the actual data.
    dwBufferSize = GetLogicalDriveStringsW( dwBufferSize, pFullBuffer );
    if( 0 == dwBufferSize )
    {
        hr = E_FAIL;
        goto abort;
    }

    // Each iteration extracts one root pathname.
    pStartPath = pFullBuffer;
    pEndBuffer = pFullBuffer + dwBufferSize;
    dwRootNum = 0;
    dwTotalNumRoots = 0;
    while ( pStartPath < pEndBuffer )
    {
        // Find the end of the current path. This is the next
        // NULL character.
        pStopPath = pStartPath;
        while ( ( *pStopPath ) && ( pStopPath < pEndBuffer ) )
        {
            pStopPath++;
        }

        // If we found a non-empty path, then record it.
        if( pStopPath > pStartPath )
        {
            if( dwRootNum < dwMaxRoots )
            {
                dwPathNameLength = (DWORD)(pStopPath - pStartPath);
                
                pstrFile = (LPWSTR) CoTaskMemAlloc( sizeof(WCHAR) * ( dwPathNameLength + URL_SCHEME_LENGTH + 1 ) );
                if( NULL == pstrFile )
                {
                    hr = E_OUTOFMEMORY;
                    goto abort;
                }

                pstrFile[ URL_SCHEME_LENGTH + dwPathNameLength ] = L'\0';
                
                wcsncpy_s( pstrFile,( dwPathNameLength + URL_SCHEME_LENGTH + 1 ) , URL_SCHEME, URL_SCHEME_LENGTH );
                wcsncpy_s( pstrFile + URL_SCHEME_LENGTH, ( dwPathNameLength + URL_SCHEME_LENGTH + 1 )-URL_SCHEME_LENGTH , pStartPath, dwPathNameLength );
                
                if( L'\0' != pstrFile[ URL_SCHEME_LENGTH + dwPathNameLength ] )
                {
                    hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
                    goto abort;
                }
                
                pstrRootDirectoryList[dwRootNum] = pstrFile;

                dwRootNum++;
            } // copying a string.

            dwTotalNumRoots++;
        } // recording a path.

        // The next path starts just after the previous path.
        pStartPath = pStopPath + 1;
        if( ( NULL == *pStartPath ) || ( pStartPath >= pEndBuffer ) )
        {
            break;
        }
    } // extracting every path.


    // The HRESULT returned by the callback should be ignored.
    ( void ) pCallback->OnGetRootDirectories( 
                            hr,
                            dwRootNum,
                            dwTotalNumRoots,
                            qwContext
                            );
    hr = S_OK; // any error is passed to the callback

abort:
    if( FAILED( hr ) )
    {
        for ( dwIndex = 0; dwIndex < dwRootNum; dwIndex++ )
        {
            if( NULL != pstrRootDirectoryList[dwIndex] )
            {
                CoTaskMemFree( pstrRootDirectoryList[dwIndex] );
                pstrRootDirectoryList[dwIndex] = NULL;
            }
        }
    }
    if( NULL != pFullBuffer )
    {
        delete [] pFullBuffer;
    }

    return( hr );
} // GetRootDirectories






/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::GetDataSourceAttributes( 
                            DWORD *pdwFlags
                            )
{
    if ( NULL == pdwFlags )
    {
        return( E_INVALIDARG );
    }

    *pdwFlags = WMS_DATA_CONTAINER_SUPPORTS_ENUMERATION;
    return( S_OK );
} // GetDataSourceAttributes




/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::CreateDataSourceDirectory( 
                        IWMSCommandContext *pCommandContext,
                        LPWSTR pszContainerName,
                        DWORD dwFlags,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext
                        )
{
    return( E_NOTIMPL );
}




/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::DeleteDirectory( 
                        LPWSTR pszContainerName,
                        DWORD dwFlags,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext
                        )
{
    return( E_NOTIMPL );
}





/////////////////////////////////////////////////////////////////////////////
//
// [InitializePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::InitializePlugin( 
                        IWMSContext *pServerContext,
                        IWMSNamedValues *pNamedValues,
                        IWMSClassObject *pClassFactory
                        )
{
    HRESULT hr = S_OK;
    SYSTEM_INFO sysInfo;

    if ( ( NULL == pServerContext )
        || ( NULL == pClassFactory ) )
    {
        
        return( E_INVALIDARG );
    }

    // TODO: Initialize all storage system state
    GetSystemInfo( &sysInfo );
    m_dwPageSize = sysInfo.dwPageSize;
    

    m_pServerContext = pServerContext;
    m_pServerContext->AddRef();

    m_pNamedValues = pNamedValues;
    m_pNamedValues->AddRef();

    m_pClassFactory = pClassFactory;
    m_pClassFactory->AddRef();

    return( hr );
} // End of InitializePlugin.


/////////////////////////////////////////////////////////////////////////////
//
// [ShutdownPlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
CSDKSampleStorageSystem::ShutdownPlugin()
{
    if( NULL != m_pServerContext )
    {
        m_pServerContext->Release();
        m_pServerContext = NULL;
    }

    if( NULL != m_pNamedValues )
    {
        m_pNamedValues->Release();
        m_pNamedValues = NULL;
    }

    if( NULL != m_pClassFactory )
    {
        m_pClassFactory->Release();
        m_pClassFactory = NULL;
    }

    return( S_OK );
} // End Of ShutdownPlugin


/////////////////////////////////////////////////////////////////////////////
//
// [EnablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSDKSampleStorageSystem::EnablePlugin( long *pdwFlags, long *pdwHeartbeatPeriod )
{
    if ( NULL == pdwFlags || NULL == pdwHeartbeatPeriod )
    {
        return ( E_POINTER );
    }

    *pdwFlags = 0;
    *pdwHeartbeatPeriod = 0;

    return ( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [GetCustomAdminInterface]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::GetCustomAdminInterface( IDispatch **ppValue )
{
    if ( NULL == ppValue )
    {
        return( E_INVALIDARG );
    }

    *ppValue = NULL;

    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [OnHeartbeat]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSampleStorageSystem::OnHeartbeat()
{
    return( S_OK );
} // End of OnHeartbeat.


/////////////////////////////////////////////////////////////////////////////
//
// [DisablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSDKSampleStorageSystem::DisablePlugin()
{
    return ( S_OK );
}







/////////////////////////////////////////////////////////////////////////////
//
//                              DATA CONTAINER
//
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//
// [CSampleDataContainer]
//
/////////////////////////////////////////////////////////////////////////////
CSampleDataContainer::CSampleDataContainer()
{
    m_cRef = 1;
    m_pszPathName = NULL;
    m_hFile = INVALID_HANDLE_VALUE;
    m_pOwnerStorageSystem = NULL;
    InitializeCriticalSection( &m_CriticalSection );
} // End of CSampleDataContainer.




/////////////////////////////////////////////////////////////////////////////
//
// [~CSampleDataContainer]
//
/////////////////////////////////////////////////////////////////////////////
CSampleDataContainer::~CSampleDataContainer()
{
    Shutdown();
    DeleteCriticalSection( &m_CriticalSection );
} // End of ~CSampleDataContainer.



/////////////////////////////////////////////////////////////////////////////

HRESULT CSampleDataContainer::Initialize(
                        IWMSContext *pUserContext,
                        LPWSTR pszContainerName,
                        DWORD dwFlags,
                        IWMSBufferAllocator *pBufferAllocator,
                        CSDKSampleStorageSystem *pOwnerStorageSystem,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext
                        )
{
    
    HRESULT hr = S_OK;
    DWORD dwSDKSampleStorageSystemFlags = 0;
    CHAR *pResult =  NULL;
    DWORD cbDriveNameLen = 0;
    BOOL fSuccess = TRUE;
    DWORD dwOpenStyle = 0;
    CHAR *pszSDKSampleStorageSystemPathname = NULL;
    DWORD dwFileAccess = 0;
    DWORD dwSizeHigh = 0, dwError = 0, dwSizeLow = 0;

    EnterCriticalSection( &m_CriticalSection );

    // If this data container is already open, then don't open it again.
    if ( INVALID_HANDLE_VALUE != m_hFile )
    {
        hr = E_UNEXPECTED;
        goto abort;
    }

    if ( ( NULL == pszContainerName )
        || ( NULL == pCallback ) 
        || ( NULL == pOwnerStorageSystem ) )
    {
        hr = E_INVALIDARG;
        goto abort;
    }

    int cchNeeded = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        pszContainerName,
                        -1,
                        NULL,
                        0,
                        NULL,
                        NULL
                        );
    if( 0 >= cchNeeded )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }

    m_pszPathName = new CHAR[ cchNeeded ];
    if( NULL == m_pszPathName )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    int cchConverted =  WideCharToMultiByte(
                            CP_ACP,
                            0,
                            pszContainerName,
                            -1,
                            m_pszPathName,
                            cchNeeded,
                            NULL,
                            NULL
                            );
    if( cchConverted != cchNeeded )
    {
        delete [] m_pszPathName;
        m_pszPathName = NULL;

        hr = E_UNEXPECTED;
        goto abort;
    }

    m_pOwnerStorageSystem = pOwnerStorageSystem;
    pOwnerStorageSystem->AddRef();

    if ( dwFlags & WMS_DATA_CONTAINER_CREATE_NEW_CONTAINER )
    {
        dwOpenStyle = OPEN_ALWAYS;
    }
    else
    {
        dwOpenStyle = OPEN_EXISTING;
    }

    dwFileAccess = GENERIC_READ;
    if ( ( dwFlags & WMS_DATA_CONTAINER_WRITE_ACCESS )
        || ( dwFlags & WMS_DATA_CONTAINER_CREATE_NEW_CONTAINER ) )
    {
        dwFileAccess |= GENERIC_WRITE;
    }

    pszSDKSampleStorageSystemPathname = m_pszPathName + URL_SCHEME_LENGTH;

    m_hFile = CreateFileA( 
                    pszSDKSampleStorageSystemPathname, 
                    dwFileAccess, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    dwOpenStyle, 
                    dwSDKSampleStorageSystemFlags,
                    NULL 
                    );
    if ( INVALID_HANDLE_VALUE == m_hFile )
    {   
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }
    else
    {
        // Get the file type to see if we should open this or not.
        DWORD dwFileType = GetFileType( m_hFile );

        if( FILE_TYPE_DISK != dwFileType )
        {
            hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
            goto abort;
        }
    }

    // Get the file size.
    dwSizeLow = GetFileSize( m_hFile, & dwSizeHigh ) ; 
 
    // If we failed ... 
    if( ( dwSizeLow == 0xFFFFFFFF ) && 
        ( ( dwError = GetLastError() ) != NO_ERROR ) )
    {
        hr = HRESULT_FROM_WIN32( dwError );
        goto abort;
    }
    m_qwFileSize = MAKEQWORD( dwSizeLow, dwSizeHigh ); 

    m_dwAlignment = 1;
    m_dwAlignmentLog2 = 0;

abort:
    if( FAILED(hr) && ( INVALID_HANDLE_VALUE != m_hFile ) )
    {
        // In a failure condition, ensure the file is closed
        CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }

    LeaveCriticalSection( &m_CriticalSection );

    if ( NULL != pCallback )
    {
        // The HRESULT returned by the callback should be ignored.
        pCallback->OnOpenDataContainer( hr, this, qwContext );
        hr = S_OK; // any error is passed to the callback
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
//
// [Shutdown]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT
CSampleDataContainer::Shutdown()
{
    HRESULT hr = S_OK;
    BOOL fSuccess;

    EnterCriticalSection( &m_CriticalSection );
    
    if ( NULL != m_pszPathName )
    {
        delete m_pszPathName;
        m_pszPathName = NULL;
    }

    if ( INVALID_HANDLE_VALUE != m_hFile )
    {
        fSuccess = CloseHandle( m_hFile );
        if ( !fSuccess )
        {
            DWORD nErr = GetLastError();
        }

        m_hFile = INVALID_HANDLE_VALUE;
    }

    if( NULL != m_pOwnerStorageSystem )
    {
        m_pOwnerStorageSystem->Release();
        m_pOwnerStorageSystem = NULL;
    }

    LeaveCriticalSection( &m_CriticalSection );
    return( hr );
} // End of Shutdown.






/////////////////////////////////////////////////////////////////////////////
inline
STDMETHODIMP_( ULONG ) 
CSampleDataContainer::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}





/////////////////////////////////////////////////////////////////////////////
inline 
STDMETHODIMP_( ULONG ) 
CSampleDataContainer::Release()
{
    if ( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return( 0 );
    }

    return( 0xbad );
} // End of Release.





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSampleDataContainer::QueryInterface( REFIID riid, void **ppvObject )
{
    if ( IID_IWMSDataContainer == riid )
    {
        *ppvObject = ( IWMSDataContainer * ) this;
        AddRef();
        return( S_OK );
    }
    else if ( IID_IUnknown == riid )
    {
        *ppvObject = ( IUnknown * ) this;
        AddRef();
        return( S_OK );
    }

    *ppvObject = NULL;
    return( E_NOINTERFACE );
} // End of QueryInterface.





/////////////////////////////////////////////////////////////////////////////
//
// [GetContainerFormat]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::GetContainerFormat( GUID *pFormat )
{
    if ( NULL == pFormat )
    {
        return( E_INVALIDARG );
    }

    // Sample never knows the format of a file.
    *pFormat = IID_IWMSUnknownFormat;
    return( S_OK );
} // End of GetContainerFormat.






/////////////////////////////////////////////////////////////////////////////
//
// [GetDataSourcePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::GetDataSourcePlugin( 
                        IWMSDataSourcePlugin **ppDataSource
                        )
{
    HRESULT hr = S_OK;
    
    if ( NULL == ppDataSource )
    {
        
        return( E_INVALIDARG );
    }

    *ppDataSource = NULL;

    EnterCriticalSection( &m_CriticalSection );
    
    if ( NULL != m_pOwnerStorageSystem )
    {
        *ppDataSource = m_pOwnerStorageSystem;
        m_pOwnerStorageSystem->AddRef();
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    LeaveCriticalSection( &m_CriticalSection );
    
    return( hr );
} // End of GetDataSourcePlugin.






/////////////////////////////////////////////////////////////////////////////
//
// [GetInfo]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::GetInfo( 
                        DWORD dwInfoValueId,
                        IWMSDataContainerCallback *pCallback,
                        QWORD qwContext
                        )
{
    HRESULT hr = S_OK;

    if ( NULL == pCallback )
    {        
        return( E_INVALIDARG );
    }
    if ( WMS_DATA_CONTAINER_SIZE != dwInfoValueId )
    {
        return( E_NOTIMPL );
    }

    EnterCriticalSection( &m_CriticalSection );
    
    // If this data container is not open, then abort.
    if ( INVALID_HANDLE_VALUE != m_hFile )
    {        
        // Ignore any error returned by the callback.
        ( void ) pCallback->OnGetInfo( 
                        S_OK,
                        WMS_SEEKABLE_CONTAINER | WMS_LOCAL_DATA_CONTAINER,
                        m_qwFileSize,
                        qwContext
                        );
        hr = S_OK; // any error is passed to the callback
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    LeaveCriticalSection( &m_CriticalSection );
    
    return( hr );
} // End of GetInfo.

/////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::GetTransferParameters( 
                                QWORD qwDesiredOffset,
                                DWORD dwDesiredMinSize,
                                DWORD dwDesiredMaxSize,
                                QWORD *pqwOffset,
                                DWORD *pdwSize,
                                DWORD *pdwBufferAlignment
                                )
{
    HRESULT hr = S_OK;

    if( ( NULL == pqwOffset )
        || ( NULL == pdwSize ) )
    {
        return( E_POINTER );
    }

    if( ( dwDesiredMaxSize != 0 ) && ( dwDesiredMinSize != 0 ) && ( dwDesiredMaxSize < dwDesiredMinSize ) )
    {
        return( E_INVALIDARG );
    }

    if( ( qwDesiredOffset == ~0 ) && ( dwDesiredMinSize == 0 ) )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );
    
    if( pdwBufferAlignment != NULL )
    {
        *pdwBufferAlignment = m_dwAlignment;
    }

    if( dwDesiredMinSize != 0 )
    {
        // Round the offset down to the nearest sector boundary.
        *pqwOffset = ( qwDesiredOffset >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;

        // We may have backed the offset up to the nearest sector.
        // Make sure we will still read all of the desired bytes.
        *pdwSize = dwDesiredMinSize + (DWORD) ( qwDesiredOffset - *pqwOffset ); 
        *pdwSize = ( ( *pdwSize + m_dwAlignment - 1 ) >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;

        if( dwDesiredMaxSize != 0 ) 
        {
            if( dwDesiredMaxSize > *pdwSize )
            {
                DWORD dwTmp = ( dwDesiredMaxSize >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;
                *pdwSize = dwTmp;
            }
            else
            {
                *pdwSize = 0;
                hr = E_FAIL;
                goto abort;
            }
        }
    }
    else if( dwDesiredMaxSize != 0 ) // and dwDesiredMinSize == 0
    {
        *pdwSize = ( dwDesiredMaxSize >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;
        if( qwDesiredOffset >= *pdwSize )
        {
            *pqwOffset = qwDesiredOffset - dwDesiredMaxSize;
            *pqwOffset = ( ( *pqwOffset + m_dwAlignment - 1 ) >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;

            if( *pqwOffset > qwDesiredOffset )
            {
                *pdwSize = 0;
                hr = E_FAIL;
                goto abort;
            }
        }
        else
        {
            // we could make pdwSize smaller now if necessary, so that we don't read 
            // too much information            
            *pqwOffset = 0;
            *pdwSize = ( ( (DWORD) qwDesiredOffset + m_dwAlignment - 1 ) >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2; 
        }
    }
    else
    {
        *pdwSize = 0;
        *pqwOffset = ( ( qwDesiredOffset + m_dwAlignment - 1 ) >> m_dwAlignmentLog2 ) << m_dwAlignmentLog2;
    }

abort:
    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
}




/////////////////////////////////////////////////////////////////////////////
//
// [Read]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::Read( 
                        BYTE *pbBuffer,
                        QWORD qwReadPosition,
                        DWORD dwMaxDataSize,
                        DWORD dwFlags,
                        IWMSDataContainerCallback *pCallback,
                        QWORD qwContext
                        )
{
    

    // The dwChangeType and qwChangeParameter values allow a storage
    // system to send stream signals to the server when a read completes.
    // For example, a storage system that reads from a network feed or
    // a live encoder will use this to pass along stream signals such
    // as playlist changes.

    // Do some work and eventually call the callback. Remember the rules:
    //
    // 1. You may call the callback from any thread. You may even call the
    //      callback inside this function before this function returns.
    // 
    // 2. If you call the callback, then this method MUST return S_OK. 
    //       This tells the server that the callback will not be called.
    //
    //       Similarly, if this function returns S_OK, then you must call
    //       the callback eventually, either from inside this method or
    //       later as part of an asynchronous action.
    //
    // 3. If this method returns an error, then it must NOT call the 
    //       callback. The error tells the server that the callback 
    //       will not be called.

    HRESULT hr = S_OK;
    DWORD dwOffsetLow = 0, dwOffsetHigh = 0;
    DWORD dwError = NO_ERROR;
    BOOL fSuccess = TRUE;
    DWORD dwBytesTransferred = 0;


    if ( ( NULL == pbBuffer )
        || ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );
    
    // If this data container is not open, then abort.
    if ( ( INVALID_HANDLE_VALUE == m_hFile )
        || ( NULL == m_pOwnerStorageSystem ) )
    {
        hr = E_UNEXPECTED;
        goto abort;
    }

    dwOffsetLow = LODWORD( qwReadPosition );
    dwOffsetHigh = HIDWORD( qwReadPosition );

    dwOffsetLow = SetFilePointer( m_hFile, dwOffsetLow, (long*)&dwOffsetHigh, FILE_BEGIN );

    if( ( dwOffsetLow == 0xFFFFFFFF ) && ( ( dwError = GetLastError() ) != NO_ERROR ) )
    {
        hr = HRESULT_FROM_WIN32( dwError );
        goto abort;
    }
 
    fSuccess = ReadFile( 
                    m_hFile, 
                    pbBuffer,
                    dwMaxDataSize, 
                    &dwBytesTransferred,
                    NULL
                    );
    if ( !fSuccess )
    {
        dwError = GetLastError();
        if ( ERROR_HANDLE_EOF != dwError )
        {
            hr = HRESULT_FROM_WIN32( dwError );
            goto abort;
        }
    }

    OnIoCompletion(
            dwError,
            CSampleDataContainer::IO_READ,
            qwReadPosition,
            dwBytesTransferred,
            pbBuffer,
            pCallback,
            qwContext
            );

    // We called the callback, so we MUST return S_OK.
    hr = S_OK;

abort:

    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
} // End of Read.





/////////////////////////////////////////////////////////////////////////////
//
// [Write]
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSampleDataContainer::Write( 
                        BYTE *pbBuffer,
                        DWORD dwDataSize,
                        QWORD qwWritePosition,
                        IWMSDataContainerCallback *pCallback,
                        QWORD qwContext
                        )
{
    
    HRESULT hr = S_OK;
    BOOL fSuccess = TRUE;
    DWORD dwOffsetLow = 0, dwOffsetHigh = 0;
    DWORD dwError = NO_ERROR;
    DWORD dwBytesTransferred = 0;

    if ( ( NULL == pbBuffer )
        || ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );

    // If this data container is not open, then abort.
    if ( ( INVALID_HANDLE_VALUE == m_hFile )
        || ( NULL == m_pOwnerStorageSystem ) )
    {
        hr = E_UNEXPECTED;
        goto abort;
    }

    // Do some work and eventually call the callback. Remember the rules:
    //
    // 1. You may call the callback from any thread. You may even call the
    //      callback inside this function before this function returns.
    // 
    // 2. If you call the callback, then this method MUST return S_OK. 
    //       This tells the server that the callback will not be called.
    //
    //       Similarly, if this function returns S_OK, then you must call
    //       the callback eventually, either from inside this method or
    //       later as part of an asynchronous action.
    //
    // 3. If this method returns an error, then it must NOT call the 
    //       callback. The error tells the server that the callback 
    //       will not be called.

    dwOffsetLow = LODWORD( qwWritePosition );
    dwOffsetHigh = HIDWORD( qwWritePosition );

    dwOffsetLow = SetFilePointer( m_hFile, dwOffsetLow, (long*)&dwOffsetHigh, FILE_BEGIN );

    if( ( dwOffsetLow == 0xFFFFFFFF ) && ( ( dwError = GetLastError() ) != NO_ERROR ) )
    {
        hr = HRESULT_FROM_WIN32( dwError );
        goto abort;
    }

    fSuccess = WriteFile( 
                    m_hFile, 
                    pbBuffer, 
                    dwDataSize, 
                    &dwBytesTransferred,
                    NULL
                    );
    if ( !fSuccess )
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }

    OnIoCompletion( dwError,
            CSampleDataContainer::IO_WRITE,
            qwWritePosition,
            dwBytesTransferred,
            pbBuffer,
            pCallback,
            qwContext );
    hr = S_OK;

abort:

    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
void CSampleDataContainer::OnIoCompletion( 
                        DWORD dwError,
                        DWORD dwIoType,
                        QWORD qwOffset,
                        DWORD cbTransferred,
                        BYTE *pbBuffer,
                        IWMSDataContainerCallback *pCallback,
                        QWORD qwContext
                        )
{
    HRESULT hr = HRESULT_FROM_WIN32( dwError );
    

    if ( CSampleDataContainer::IO_READ == dwIoType )
    {
        if ( ( SUCCEEDED( hr ) ) 
            && ( qwOffset >= m_qwFileSize ) 
            && ( 0 == cbTransferred ) )
        {
            hr = HRESULT_FROM_WIN32( ERROR_HANDLE_EOF );
        }

        if ( NULL != pCallback )
        {
            // The HRESULT returned by the callback should be ignored.
            pCallback->OnRead( hr,
                               cbTransferred,
                               0,
                               0,
                               qwContext
                               );
            hr = S_OK; // any error is passed to the callback
        }
    }
    else if ( CSampleDataContainer::IO_WRITE == dwIoType )
    {
        if ( NULL != pCallback )
        {
            // The HRESULT returned by the callback should be ignored.
            pCallback->OnWrite( hr,
                                cbTransferred,
                                qwContext );
            hr = S_OK; // any error is passed to the callback
        }
    }
}








/////////////////////////////////////////////////////////////////////////////
//
// [DoDataContainerExtendedCommand]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::DoDataContainerExtendedCommand( 
                        LPWSTR szCommandName,
                        IWMSCommandContext *pCommand,
                        DWORD dwCallFlags,
                        IWMSDataContainerCallback *pCallback,
                        QWORD qwContext
                        )
{
    if ( NULL == pCallback )
    {
        // We are returning an error, so we cannot call the callback.
        return( E_INVALIDARG );
    }

    // TODO: Implement any special commands that may be sent from the 
    // client or server to plugins that are not part of the current command 
    // set. Currently, this method is not used, and is designed for future
    // growth. 

    // Ignore the result.
    pCallback->OnDoDataContainerExtendedCommand( S_OK, qwContext );

    // We called the callback, so we MUST return S_OK.
    return( S_OK );
} // End of DoDataContainerExtendedCommand.





/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSampleDataContainer::FinishParsingPacketlist( 
                        IWMSPacketList *pPacketList
                        )
{
    HRESULT hr = S_OK;

    if ( NULL == pPacketList )
    {
        return( E_INVALIDARG );
    }

    return( S_OK );
} // FinishParsingPacketlist





/////////////////////////////////////////////////////////////////////////////
//
// [CSampleDirectory]
//
/////////////////////////////////////////////////////////////////////////////
CSampleDirectory::CSampleDirectory()
{
    m_cRef = 1;
    m_pOwnerStorageSystem = NULL;
    InitializeCriticalSection( &m_CriticalSection );
    m_pPathName = NULL;
    m_dwPathNameLength = 0;
    m_pSearchDirName = NULL;
    m_pChildren = NULL;
    m_pRecentChild = NULL;
    m_dwItemNum = 0;    
} //End of CSampleDirectory

CSampleDirectory::~CSampleDirectory()
{
    Shutdown();
    DeleteCriticalSection( &m_CriticalSection );
} // End of ~CSampleDirectory

/////////////////////////////////////////////////////////////////////////////
//
// [Initialize]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT
CSampleDirectory::Initialize(
                        IWMSContext *pUserContext,
                        LPWSTR pszContainerName,
                        CSDKSampleStorageSystem *pOwnerStorageSystem,
                        IWMSDataSourcePluginCallback *pCallback,
                        QWORD qwContext )
{
    HRESULT hr = S_OK;
    WCHAR *pszNtfsPathname = NULL;
    size_t sFileName = 0;
    DWORD dwFileAttributes = 0;
    DWORD dwIndex = 0;
    HANDLE hFileEnum = INVALID_HANDLE_VALUE;
    BOOL fSuccess = FALSE;
    WIN32_FIND_DATA FileInfo;
    CSampleDirectoryInfo *pDirInfo = NULL;
    DWORD dwUrlLength = 0;
    DWORD dwFileNameLength = 0;
    DWORD dwFileNameStartPosition =0;
    WCHAR cLastCharInPath = L'\0';

    EnterCriticalSection( &m_CriticalSection );

    //
    // Check arguments
    //

    if( ( NULL == pszContainerName )||
        ( NULL == pCallback ) ||
        ( NULL == pOwnerStorageSystem ) )
    {
        hr = E_INVALIDARG;
        goto abort;
    }

    if( 0 != _wcsnicmp( pszContainerName, URL_SCHEME, URL_SCHEME_LENGTH ) )
    {
        // The container name doesn't start with the expected scheme
        hr = E_INVALIDARG;
        goto abort;
    }

    m_pOwnerStorageSystem = pOwnerStorageSystem;
    pOwnerStorageSystem->AddRef();

    m_dwItemNum = 0;

    //
    // Copy the full path
    //

    m_dwPathNameLength = (DWORD)wcslen( pszContainerName );
    m_pPathName = new WCHAR[ m_dwPathNameLength + 1 ];
    if( NULL == m_pPathName )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    m_pPathName[m_dwPathNameLength] = L'\0';

    wcsncpy_s( m_pPathName,m_dwPathNameLength + 1, pszContainerName, m_dwPathNameLength );

    if( L'\0' != m_pPathName[m_dwPathNameLength] )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        goto abort;
    }

    //
    // Make an auxiliary copy of the path, without the scheme
    // 

    pszNtfsPathname = pszContainerName + URL_SCHEME_LENGTH;
    sFileName = wcslen( pszNtfsPathname );
    m_pSearchDirName = new WCHAR[ sFileName + DIR_MASK_LENGTH + 1 ];
    if( NULL == m_pSearchDirName )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    m_pSearchDirName[sFileName] = L'\0';

    wcsncpy_s( m_pSearchDirName,sFileName + DIR_MASK_LENGTH + 1, pszNtfsPathname, sFileName );

    if( L'\0' != m_pSearchDirName[sFileName] )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        goto abort;
    }

    //
    // Remove any trailing '\' or '/' from the working path
    //

    if( 0 < sFileName )
    {
        cLastCharInPath = m_pSearchDirName[sFileName - 1];

        if( ( L'\\' == cLastCharInPath )
            || ( L'/' == cLastCharInPath ) )
        {
            --sFileName;
            m_pSearchDirName[sFileName] = L'\0';
        }
    }

    //
    // Check if it's really a directory
    //

    dwFileAttributes = GetFileAttributes( m_pSearchDirName );
    if( -1 == dwFileAttributes )
    {
        // Unable to get the FileAttributes
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto abort;
    }
    else if( !( FILE_ATTRIBUTE_DIRECTORY & dwFileAttributes ) )
    {
        // This "file" is not a directory
        hr = HRESULT_FROM_WIN32( ERROR_DIRECTORY );
        goto abort;
    }

    //
    // Append a '\*' mask to the working path and walk through the directory
    //
    
    m_pSearchDirName[ sFileName + DIR_MASK_LENGTH ] = L'\0';

    wcsncpy_s( &m_pSearchDirName[sFileName],DIR_MASK_LENGTH +1, DIR_MASK, DIR_MASK_LENGTH );

    if( L'\0' != m_pSearchDirName[ sFileName + DIR_MASK_LENGTH ] )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        goto abort;
    }

    hFileEnum = FindFirstFileW( m_pSearchDirName, &FileInfo );
    if( INVALID_HANDLE_VALUE == hFileEnum )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        if( HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) == hr )
        {
            // This means we hit the end of the directory, which is
            // not an error.  Signify S_FALSE for the end of the
            // enumeration.
            hr = S_FALSE;
            goto abort;
        }
        goto abort;
    }

    fSuccess = TRUE;

    // Advance to the desired position in the directory.
    dwIndex = 0;
    while ( TRUE )
    {
        if( !fSuccess )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            if( HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES ) != hr  )
            {
                goto abort;
            }

            // This means we hit the end of the directory, which is
            // not an error. 
            hr = S_OK;
            break;
        }

        if( ( '\0' == FileInfo.cFileName[0] )
            || ( ( '.' == FileInfo.cFileName[0] ) && ('\0' == FileInfo.cFileName[1] ) )
            || ( ( '.' == FileInfo.cFileName[0] ) && ( '.' == FileInfo.cFileName[1] ) && ( '\0' == FileInfo.cFileName[2] ) )
            || ( FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) )
            
        {
            // if the item is empty or '.' or '..' or hidden file then skip it
            // and grab the next file and start the loop again
            fSuccess = FindNextFileW( hFileEnum, &FileInfo );
            continue;
        }

        pDirInfo = new CSampleDirectoryInfo;
        if( NULL == pDirInfo )
        {
            hr = E_OUTOFMEMORY;
            goto abort;
        }

        //
        // Compose the item's URL
        //

        dwFileNameLength = (DWORD)wcslen( FileInfo.cFileName );
        dwUrlLength = m_dwPathNameLength + dwFileNameLength + 1;   // 1 for the '\' separator

        pDirInfo->m_pszwName = ( LPOLESTR ) CoTaskMemAlloc( sizeof( OLECHAR ) * ( dwUrlLength + 1 ) );
        if( NULL == pDirInfo->m_pszwName )
        {
            hr = E_OUTOFMEMORY;
            goto abort;
        }

        pDirInfo->m_pszwName[m_dwPathNameLength] = L'\0';

        wcsncpy_s( pDirInfo->m_pszwName, ( dwUrlLength + 1 ) , m_pPathName, m_dwPathNameLength );

        if( L'\0' != pDirInfo->m_pszwName[m_dwPathNameLength] )
        {
            hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
            goto abort;
        }

        dwFileNameStartPosition = m_dwPathNameLength;
        cLastCharInPath = m_pPathName[m_dwPathNameLength-1];
        if( ( L'\\' != cLastCharInPath ) && ( L'/' != cLastCharInPath ) )
        {
            ( pDirInfo->m_pszwName )[m_dwPathNameLength] = L'\\';
            ( pDirInfo->m_pszwName )[m_dwPathNameLength + 1] = L'\0';
            dwFileNameStartPosition++;
        }

        pDirInfo->m_pszwName[ dwFileNameStartPosition + dwFileNameLength ] = L'\0';

        wcsncpy_s( &pDirInfo->m_pszwName[dwFileNameStartPosition],dwFileNameLength+1, FileInfo.cFileName, dwFileNameLength );

        if( L'\0' != pDirInfo->m_pszwName[ dwFileNameStartPosition + dwFileNameLength ] )
        {
            hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
            goto abort;
        }

        //
        // Set the item's flags
        //

        pDirInfo->m_dwFlags = 0;
        if( FileInfo.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY )
        {
            pDirInfo->m_dwFlags |= WMS_DIRECTORY_ENTRY_IS_DIRECTORY;
        }
        pDirInfo->m_qwSize = MAKEQWORD( FileInfo.nFileSizeLow, FileInfo.nFileSizeHigh );

        //
        // Insert item's info in list
        //

        pDirInfo->m_pNext = m_pChildren;
        m_pChildren = pDirInfo;
        pDirInfo = NULL;

        dwIndex++;

        fSuccess = FindNextFileW( hFileEnum, &FileInfo );
    } // Enumerate every file in the directory.

    m_dwItemNum = 1;
    m_pRecentChild = m_pChildren;

abort:
    LeaveCriticalSection ( &m_CriticalSection );

    if( NULL != pDirInfo )
    {
        delete pDirInfo;
    }
    
    if( INVALID_HANDLE_VALUE != hFileEnum )
    {
        fSuccess = FindClose( hFileEnum );
        hFileEnum = INVALID_HANDLE_VALUE;
    }
    
    if( NULL != pCallback )
    {
        // The HRESULT returned by the callback should be ignored.
        pCallback->OnOpenDirectory( hr, this, qwContext );
        hr = S_OK; // any error is passed to the callback
    }

    // Any error is passed to the callback.
    return( S_OK );
} // End of Initialize


/////////////////////////////////////////////////////////////////////////////
//
// [Shutdown]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT
CSampleDirectory::Shutdown()
{
    CSampleDirectoryInfo *pChild = NULL;
    
    if( NULL != m_pOwnerStorageSystem )
    {
        m_pOwnerStorageSystem->Release();
        m_pOwnerStorageSystem = NULL;
    }

    if( NULL != m_pPathName )
    {
        delete [] m_pPathName;
        m_pPathName = NULL;
    }

    if( NULL != m_pSearchDirName )
    {
        delete [] m_pSearchDirName;
        m_pSearchDirName = NULL;
    }

    // delete children
    while( NULL != m_pChildren )
    {
        pChild = m_pChildren;
        m_pChildren = m_pChildren->m_pNext;
        delete pChild;
    }
    
    return( S_OK );
} // End of Shutdown


/////////////////////////////////////////////////////////////////////////////
//
// [AddRef]
//
/////////////////////////////////////////////////////////////////////////////
inline
STDMETHODIMP_( ULONG ) 
CSampleDirectory::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
} // End of AddRef


/////////////////////////////////////////////////////////////////////////////
//
// [Release]
//
/////////////////////////////////////////////////////////////////////////////
inline 
STDMETHODIMP_( ULONG ) 
CSampleDirectory::Release()
{
    if ( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return( 0 );
    }

    return( 0xbad );
} // End of Release.


/////////////////////////////////////////////////////////////////////////////
//
// [QueryInterface]
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSampleDirectory::QueryInterface( REFIID riid, void **ppvObject )
{
    if ( IID_IWMSDirectory == riid )
    {
        *ppvObject = ( IWMSDirectory * ) this;
        AddRef();
        return( S_OK );
    }
    else if ( IID_IUnknown == riid )
    {
        *ppvObject = ( IUnknown * ) this;
        AddRef();
        return( S_OK );
    }

    *ppvObject = NULL;
    return( E_NOINTERFACE );
} // End of QueryInterface.


/////////////////////////////////////////////////////////////////////////////
//
// [GetDataSourcePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
CSampleDirectory::GetDataSourcePlugin( 
                    IWMSDataSourcePlugin **ppDataSource
                    )
{
    HRESULT hr = S_OK;

    if( NULL == ppDataSource )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );

    if( NULL == m_pOwnerStorageSystem )
    {
        goto abort;
    }

    *ppDataSource = m_pOwnerStorageSystem;
    m_pOwnerStorageSystem->AddRef();

abort:
    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
} // End of GetDataSourcePlugin

/////////////////////////////////////////////////////////////////////////////
//
// [GetName]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
CSampleDirectory::GetName( 
                    LPOLESTR *pstrValue
                    )
{
    HRESULT hr = S_OK;

    if( NULL == pstrValue )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );

    *pstrValue = new WCHAR[ m_dwPathNameLength + 1 ];
    if( NULL == *pstrValue )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    if( NULL != m_pPathName )
    {
        *pstrValue[m_dwPathNameLength] = L'\0';

        wcsncpy_s( *pstrValue,m_dwPathNameLength + 1, m_pPathName, m_dwPathNameLength );

        if( L'\0' != *pstrValue[m_dwPathNameLength] )
        {
            hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
            goto abort;
        }
    }
    else
    {
        ( *pstrValue )[ 0 ] = L'\0';
    }
    
abort:
    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
} // End of GetName

/////////////////////////////////////////////////////////////////////////////
//
// [GetChildInfo]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
CSampleDirectory::GetChildInfo( 
                    DWORD dwIndex,
                    WMSDirectoryEntryInfo *pInfo
                    )
{
    HRESULT hr = S_OK;
    DWORD dwUrlLength = 0;

    if( NULL == pInfo )
    {
        return( E_INVALIDARG );
    }

    EnterCriticalSection( &m_CriticalSection );

    // We cannot go backwards. If the client asked for a previous
    // directory entry, then reset the state to the beginning and
    // then go forward to the desired position.
    if( ( dwIndex < m_dwItemNum ) || ( NULL == m_pRecentChild ) )
    {
        m_dwItemNum = 1;
        m_pRecentChild = m_pChildren;
    } // resetting the count.


    // Advance to the desired position in the directory.
    while ( ( m_dwItemNum < dwIndex ) && ( NULL != m_pRecentChild ) )
    {
        m_pRecentChild = m_pRecentChild->m_pNext;
        m_dwItemNum++;
    } // advancing to the desired position.
    

    if( NULL == m_pRecentChild )
    {
        // This means we hit the end of the directory, which is
        // not an error. Signify S_FALSE for the end of the enumeration.
        hr = S_FALSE;
        goto abort;
    }

    dwUrlLength = (DWORD)wcslen( m_pRecentChild->m_pszwName );
    pInfo->pstrName = ( LPOLESTR ) CoTaskMemAlloc( sizeof(OLECHAR) * ( dwUrlLength + 1 ) );
    if( NULL == pInfo->pstrName )
    {
        hr = E_OUTOFMEMORY;
        goto abort;
    }

    pInfo->pstrName[dwUrlLength] = L'\0';

    wcsncpy_s( pInfo->pstrName,( dwUrlLength + 1 ) , m_pRecentChild->m_pszwName, dwUrlLength );

    if( L'\0' != pInfo->pstrName[dwUrlLength] )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
        goto abort;
    }

    pInfo->dwFlags = m_pRecentChild->m_dwFlags;
    pInfo->qwSize = m_pRecentChild->m_qwSize;

abort:
    LeaveCriticalSection( &m_CriticalSection );

    return( hr );
} // End of GetChildInfo



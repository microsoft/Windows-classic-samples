//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ExtensionData.cpp
//
// Abstract:            Implementation of the CExtensionData class
//
//*****************************************************************************

//////////////////////////////////////////////////////////////////////
//  This file contains the following helper classes used to retrieve and
//  display data unit extensions for all streams in the profile :
//   CExtensionData - the data structure containing all extension data
//   CExtDataList - list of extensions defined in the stream
//
//  It also contains a function which saves a profile to a file.
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ExtensionData.h"
#include <stdlib.h>
#include <strsafe.h>


//////////////////////////////////////////////////////////////////////

CExtensionData::CExtensionData( GUID guidDUExt, BYTE *pbExtensionSystemInfo,
                    WORD cbExtensionDataSize, DWORD cbExtensionSystemInfo, WORD wStreamNum )
                 :  m_guidDUExt( guidDUExt ),
                    m_cbExtensionDataSize( cbExtensionDataSize ),
                    m_pbExtensionSystemInfo( pbExtensionSystemInfo ),
                    m_cbExtensionSystemInfo( cbExtensionSystemInfo ),
                    m_pNext( NULL ), m_pValue( NULL ), m_wStreamNum( wStreamNum )
{
};

CExtensionData::~CExtensionData()
    {
        if (NULL != m_pbExtensionSystemInfo)
		{
			delete [] m_pbExtensionSystemInfo;	
			m_pbExtensionSystemInfo = NULL;
		}
        if (NULL != m_pValue)
		{
			delete [] m_pValue;
			m_pValue = NULL;
		}
    };

//////////////////////////////////////////////////////////////////////
// Display extension data
//////////////////////////////////////////////////////////////////////
HRESULT CExtensionData::DisplayData()
{
    HRESULT hr = S_OK;
    WCHAR *pwszGuidExtensionType = NULL;
    WCHAR *pwszValue = NULL;
    WCHAR *pwszSystemInfo = NULL;
    WCHAR *pszIter = NULL;
	WORD i = 0;

    do
    {
        if( m_cbExtensionSystemInfo > 0 && NULL == m_pbExtensionSystemInfo )
        {
            hr = E_FAIL;
            break;
        }

        hr = StringFromCLSID( m_guidDUExt, &pwszGuidExtensionType );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        //  converts extension data bytes into ASCII
        //
        pwszValue = new WCHAR[ m_cbExtensionDataSize * 3 + 1 ];
        if( NULL == pwszValue )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pszIter = pwszValue;

        for( i = 0; i < m_cbExtensionDataSize; i++ )
        {
            BYTE t = *( ( BYTE * )m_pValue + i );
            (void)StringCchPrintfW( pszIter, 4, L" %2.2X", t);
            pszIter += 3;
        }

        *pszIter = L'\0';

        //
        //  Convert extension system info to ASCII
        //
        pwszSystemInfo = new WCHAR[ m_cbExtensionSystemInfo * 3 + 1 ];
        if( NULL == pwszSystemInfo )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pszIter = pwszSystemInfo;

        for( i = 0; i < m_cbExtensionSystemInfo; i++ )
        {
            BYTE t = *( ( BYTE * )m_pbExtensionSystemInfo + i );
            (void)StringCchPrintfW( pszIter, 4, L" %2.2X", t);
            pszIter += 3;
        }

        *pszIter = L'\0';

        //
        //  Print the data
        //
        wprintf( L" ExtGUID : %ws \tExtDataSize : %u \tExtData[0x] : %ws  \tExtSystInfoSize : %u \tExtSystInfo[0x] : %ws\n",
                    pwszGuidExtensionType,
                    m_cbExtensionDataSize,
                    pwszValue, 
                    m_cbExtensionSystemInfo,
                    pwszSystemInfo );
    }
    while( FALSE );

	if( NULL != pwszValue )
	{              
		delete [] pwszValue;
		pwszValue = NULL;   
	}
	if( NULL != pwszSystemInfo )
	{
		delete [] pwszSystemInfo;
		pwszSystemInfo = NULL;
	}

    if ( NULL != pwszGuidExtensionType )
    {
        CoTaskMemFree( pwszGuidExtensionType );
    }

    return( hr );
}

//////////////////////////////////////////////////////////////////////
// Create a list of extensions defined in all streams in the profile
//////////////////////////////////////////////////////////////////////
HRESULT CExtDataList::Create( IWMProfile *pProfile )
{
    if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    DWORD dwStreamCount = 0;
    IWMStreamConfig2 *pIWMStreamConfig2 = NULL;
    IWMStreamConfig  *pIWMStreamConfig = NULL;
    HRESULT hr = S_OK;

    hr = pProfile->GetStreamCount( &dwStreamCount );
    if( FAILED( hr ) )
    {
        return( E_INVALIDARG );
    }

    for( DWORD cnt = 0; cnt < dwStreamCount; cnt++ )
    {
        SAFE_RELEASE( pIWMStreamConfig2 );
        SAFE_RELEASE( pIWMStreamConfig );

        hr = pProfile->GetStream( cnt , &pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->QueryInterface( IID_IWMStreamConfig2, (void**)&pIWMStreamConfig2 );
        if( FAILED( hr ) )
        {
            break;
        }

        CExtensionData *pExtData = NULL;


        WORD cDUExts = 0;
        hr = pIWMStreamConfig2->GetDataUnitExtensionCount( &cDUExts );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        //
        WORD wStreamNum = 0;
        hr = pIWMStreamConfig->GetStreamNumber( &wStreamNum );
        if ( FAILED( hr ) )
        {
            break;
        }

        for ( WORD i = 0; i < cDUExts; i++ )
        {


            pExtData = new CExtensionData( GUID_NULL, NULL, 0, 0, wStreamNum );
            if( NULL == pExtData )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIWMStreamConfig2->GetDataUnitExtension( i, &pExtData->m_guidDUExt,
                &pExtData->m_cbExtensionDataSize, NULL, &pExtData->m_cbExtensionSystemInfo );

            if ( FAILED( hr ) )
            {
                break;
            }

            pExtData->m_pbExtensionSystemInfo = new BYTE[ pExtData->m_cbExtensionSystemInfo ];
            if( NULL == pExtData->m_pbExtensionSystemInfo )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            pExtData->m_pValue = new BYTE[ pExtData->m_cbExtensionDataSize ];
            if( NULL == pExtData->m_pValue )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIWMStreamConfig2->GetDataUnitExtension( i, &pExtData->m_guidDUExt,
                            &pExtData->m_cbExtensionDataSize,
                            pExtData->m_pbExtensionSystemInfo,
                            &pExtData->m_cbExtensionSystemInfo );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // add extension info to the list
            //
            Append( pExtData );
        }
        if( FAILED( hr ) )
        {
            if (NULL != pExtData)
			{
                delete pExtData;
				pExtData = NULL;
			}
        }
    }

    SAFE_RELEASE( pIWMStreamConfig2 );
    SAFE_RELEASE( pIWMStreamConfig );

    return hr;
}

//////////////////////////////////////////////////////////////////////
// Add extension data to the lists
//////////////////////////////////////////////////////////////////////
bool CExtDataList::Append( CExtensionData *pCExtensionData )
{


    if( NULL == pCExtensionData )
    {
        return FALSE;
    }

    m_pCur = m_pEnd;
    m_pEnd = pCExtensionData;
    m_pEnd->m_pNext = NULL;
    (m_pCur ? m_pCur->m_pNext : m_pStart ) = m_pEnd;
    m_wSize++;
    return( TRUE );
}
//////////////////////////////////////////////////////////////////////
// Iterate through extension data for the given stream.
// Usage :
//     -first call with wStreamNum containing actual stream number
//      retrieves first extension data for that stream 
//     -next call with wStreamNum = 0 retrieves next extension data for
//      previously passed stream number
//     Function returns true if there is extension data, otherwise false
//////////////////////////////////////////////////////////////////////
bool CExtDataList::Find( WORD wStreamNum, CExtensionData **pExtensionData )
{

    if( NULL == pExtensionData || ( 0 == wStreamNum && 0 == m_wSearchStreamNum ) )
    {
        return false;
    }

    if( wStreamNum > 0 )
    {
        m_wSearchStreamNum = wStreamNum;
        m_pIter = GetStart();
    }

    if( NULL != m_pIter )
    {
        for( ; NULL != m_pIter; m_pIter = m_pIter->m_pNext )
        {
            if( m_pIter->m_wStreamNum == m_wSearchStreamNum )
            {
                *pExtensionData = m_pIter;
                m_pIter = m_pIter->m_pNext;
                return true;
            }
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTIONS USED TO SAVE PROFILE TO FILE
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Helper function - save profile into memory
///////////////////////////////////////////////////////////////////////////////

HRESULT SaveProfileToMemory( IWMProfile *pIWMProfile, 
                                  __deref_out_ecount(*pdwLen) WCHAR **ppwszBuffer, 
                                  DWORD *pdwLen )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   *pIWMProfileMgr = NULL;

    do
    {
        hr = WMCreateProfileManager( &pIWMProfileMgr );        
        if( FAILED( hr ) )
        {
            _tprintf( _T( "WMCreateProfileManager failed: (hr=0x%08x)\n" ), hr );
            break;
        }
        
        hr = pIWMProfileMgr->SaveProfile( pIWMProfile, NULL, pdwLen );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "pIWMProfileMgr::SaveProfile failed: (hr=0x%08x)\n" ), hr );
            break;
        }

        *ppwszBuffer = new WCHAR[ *pdwLen ];
        if( NULL == *ppwszBuffer )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMProfileMgr->SaveProfile( pIWMProfile, *ppwszBuffer, pdwLen );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "pIWMProfileMgr::SaveProfile failed: (hr=0x%08x)\n" ), hr );
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pIWMProfileMgr );
    
    return( hr );
}

///////////////////////////////////////////////////////////////////////////////
// Save profile to file
///////////////////////////////////////////////////////////////////////////////

HRESULT SaveProfileToFile( const TCHAR *pszFileName, IWMProfile *pIWMProfile )
{
    if( ( NULL == pszFileName ) || 
        ( NULL == pIWMProfile ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    DWORD   dwLength = 0;
    WCHAR   *pBuffer = NULL;
    HANDLE  fd = INVALID_HANDLE_VALUE;
    DWORD   dwBytesToWrite = 0, dwBytesWritten = 0;

    do
    {
        hr = SaveProfileToMemory( pIWMProfile, &pBuffer, &dwLength );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "SaveProfileToMemory failed: (hr=0x%08x)\n" ), hr );
            break;
        }
        
        fd = CreateFile( pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( INVALID_HANDLE_VALUE == fd )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() ); 
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( fd ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        dwBytesToWrite = dwLength * sizeof( WCHAR );
        
        if( !WriteFile( fd, pBuffer, dwBytesToWrite, &dwBytesWritten, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() ); 
            break;
        }
        
        if( dwBytesToWrite != dwBytesWritten )
        {
            hr = E_FAIL; 
            break;
        }
    }
    while( FALSE );

	if( NULL != pBuffer )
    {
        delete [] pBuffer;
        pBuffer = NULL;
    }
    
    if( INVALID_HANDLE_VALUE != fd ) 
    {                               
        CloseHandle(fd);            
        fd = INVALID_HANDLE_VALUE;   
    }

    return( hr );
}

//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Streamdata.cpp
//
// Abstract:            Implementation of CStreamData class 
//
//*****************************************************************************
#include "stdafx.h"
#include "StreamData.h"


//------------------------------------------------------------------------------
// Name: CStreamData::CStreamData()
// Desc: Constructor.
//------------------------------------------------------------------------------
CStreamData::CStreamData( DWORD dwStreamCount )
{
    m_dwStreamCount     = dwStreamCount;
    m_ptrStreamNumArray = NULL;
    m_ptrMediaArray     = NULL;
    m_ptrStreamBufferWindow = NULL;
    m_pfVBRStream = NULL;
}

//------------------------------------------------------------------------------
// Name: CStreamData::~CStreamData()
// Desc: Destructor.
//------------------------------------------------------------------------------
CStreamData::~CStreamData()
{
    if( m_ptrMediaArray )
    {
        for( DWORD i = 0; i < m_dwStreamCount; i++ )
        {
			if( m_ptrMediaArray[i] )
			{
				delete[] (BYTE*)m_ptrMediaArray[i];
			}
        }
    }

    SAFE_ARRAYDELETE( m_ptrMediaArray );
	SAFE_ARRAYDELETE( m_ptrStreamNumArray );
	SAFE_ARRAYDELETE( m_ptrStreamBufferWindow );
	SAFE_ARRAYDELETE( m_pfVBRStream );
}

//------------------------------------------------------------------------------
// Name: CStreamData::SetAllStreamData()
// Desc: Sets the stream numbers and media types in the member variables 
//       for the specified profile.
//------------------------------------------------------------------------------
HRESULT CStreamData::SetAllStreamData( IWMProfile * pProfile )
{
    if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }
    //
    // Make an array of pointers to WM_MEDIA_TYPE
    //
    m_ptrMediaArray = new WM_MEDIA_TYPE* [m_dwStreamCount];

    m_ptrStreamNumArray = new WORD [m_dwStreamCount];

    m_ptrStreamBufferWindow = new DWORD [m_dwStreamCount];

    m_pfVBRStream = new BOOL[m_dwStreamCount];
    
	if( NULL == m_ptrMediaArray || NULL == m_ptrStreamNumArray || NULL == m_ptrStreamBufferWindow )
    {
        _tprintf( _T( "Internal Error: Out of memory\n" ) );
        return(  E_OUTOFMEMORY );
    }
    
    ZeroMemory( m_ptrMediaArray, m_dwStreamCount * sizeof( WM_MEDIA_TYPE * ) );

    ZeroMemory( m_ptrStreamNumArray, m_dwStreamCount * sizeof( WORD ) );

    ZeroMemory( m_ptrStreamBufferWindow, m_dwStreamCount * sizeof( DWORD ) );

    HRESULT hr = S_OK;
    DWORD   cbType = 0;

    IWMMediaProps *     pProps  = NULL;
    IWMStreamConfig *   pStream = NULL;
    IWMPropertyVault *  pPropertyVault = NULL;
    WMT_ATTR_DATATYPE   enumAttrType;
    DWORD               dwAttrSize;

    //
    // Get all the stream numbers and the WM_MEDIA_TYPEs in the arrays
    //
    for( DWORD i = 0; i < m_dwStreamCount; i++ )
    {
        
	    hr = pProfile->GetStream( i, &pStream );
        if( FAILED( hr ) )
        {
		    _tprintf( _T( "Could not get Stream %d from IWMProfile (hr=0x%08x).\n" ), i, hr );
            break;
        }
        
        pStream->GetStreamNumber( &m_ptrStreamNumArray[i] );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Could not GetStreamNumber from IWMStreamConfig %d (hr=0x%08x).\n" ), i, hr );
            break;
        }
        
        hr = pStream->QueryInterface( IID_IWMMediaProps, ( VOID ** )&pProps );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMMediaProps (hr=0x%08x).\n" ), hr );
            break;
        }
        
        hr = pStream->GetBufferWindow( &m_ptrStreamBufferWindow[ i ] );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not get Buffer Window from IWMStreamConfig (hr=0x%08x).\n" ), hr );
            break;
        }

        hr = pStream->QueryInterface( IID_IWMPropertyVault, ( VOID ** )&pPropertyVault );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not QI for IWMMediaProps (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Check to see if this stream is a VBR stream
        //
        dwAttrSize = sizeof( m_pfVBRStream[i] );
        hr = pPropertyVault->GetPropertyByName( g_wszVBREnabled, &enumAttrType, (BYTE *)&m_pfVBRStream[ i ], &dwAttrSize );
        if( FAILED( hr ) )
        {
            m_pfVBRStream[ i ] = FALSE;
            hr = S_OK;
        }
        
        //
        // Get the memory required for WM_MEDIA_TYPE of this stream
        //
        hr = pProps->GetMediaType( NULL, &cbType );
		if ( FAILED( hr ) )
		{
            _tprintf( _T( "Could not Get Media Type (hr=0x%08x).\n" ), hr );
			break;
		}

		m_ptrMediaArray[i] = (WM_MEDIA_TYPE*) new BYTE[ cbType ];
		
		if( m_ptrMediaArray[i] == NULL )
		{
			hr = E_OUTOFMEMORY;			
            _tprintf( _T( "Internal Error: Out of memory\n" ) );
			break;
		}

        hr = pProps->GetMediaType( m_ptrMediaArray[i], &cbType );
		if ( FAILED( hr ) )
		{
			_tprintf( _T( "Could not Get Media Type (hr=0x%08x).\n" ), hr );
			break;
		}

        SAFE_RELEASE( pStream );
        SAFE_RELEASE( pPropertyVault );
		SAFE_RELEASE( pProps );
        cbType = 0;
    }

    SAFE_RELEASE( pProps );
    SAFE_RELEASE( pStream );
    SAFE_RELEASE( pPropertyVault );
    
    return(  hr );
}

//------------------------------------------------------------------------------
// Name: CStreamData::SetAllStreamsBufferWindow()
// Desc: Sets the buffer window for each stream in the profile to the value
//       stored in m_ptrStreamBufferWindow[].
//------------------------------------------------------------------------------
HRESULT CStreamData::SetAllStreamsBufferWindow( IWMProfile * pProfile )
{

    HRESULT             hr = S_OK;
    IWMStreamConfig *   pStream = NULL;

	if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    for( DWORD i = 0; i < m_dwStreamCount; i++ )
    {
        
        if( 0 != m_ptrStreamNumArray[i] )
        {
	        hr = pProfile->GetStreamByNumber( m_ptrStreamNumArray[i], &pStream );
            if( FAILED( hr ) )
            {
		        _tprintf( _T( "Could not get Stream %d from IWMProfile (hr=0x%08x).\n" ), m_ptrStreamNumArray[i], hr );
                break;
            }

            hr = pStream->SetBufferWindow( m_ptrStreamBufferWindow[ i ] );
            if( FAILED( hr ) )
            {
		        _tprintf( _T( "Could not set Buffer Window for Stream %d (hr=0x%08x).\n" ), m_ptrStreamNumArray[i], hr );
                break;
            }

            hr = pProfile->ReconfigStream( pStream );
            if( FAILED( hr ) )
            {
		        _tprintf( _T( "Could not reconfig stream %d (hr=0x%08x).\n" ), m_ptrStreamNumArray[i], hr );
                break;
            }

            SAFE_RELEASE( pStream );
        }
    }

	SAFE_RELEASE( pStream );

    return( hr );
}

//
// Map the stream numbers of this instance of the class
// with that of another instance given by the calling function.
// The mapped stream numbers are filled up in an array of DWORDs.
//
//------------------------------------------------------------------------------
// Name: CStreamData::MapStreamNums()
// Desc: Maps the stream numbers of this instance of the class with those of 
//       another instance given by the calling function.
//------------------------------------------------------------------------------
BOOL CStreamData::MapStreamNums( CStreamData& data2, WORD ** ptrNumMap )
{
    if( NULL == ptrNumMap || NULL == m_ptrMediaArray || NULL == data2.m_ptrMediaArray )
    {
        return( FALSE );
    }

    WORD wNum = 0;
    //
    // Allot memory for 2 * m_dwStreamCount number of WORDs.
    // The first half of the WORDs will be the stream number for first profile.
    // The second half of the WORDs will be the corresponding stream numbers
    // from the second profile. Corresponding streams of both the halves
    // will be of the same type.
    //
    (*ptrNumMap) = new WORD[2 * m_dwStreamCount];    
    if( NULL == (*ptrNumMap) )
    {
        _tprintf( _T( "Internal Error: Out of memory\n" ) );
        return( FALSE );
    }

    ZeroMemory( (*ptrNumMap), 2 * m_dwStreamCount * sizeof( WORD ) );

    for( WORD i = 0; i < m_dwStreamCount; i++ )
    {
        (*ptrNumMap)[i] = m_ptrStreamNumArray[i];
		//
		// Get the same media in the second profile
		//

        DWORD dwBufferWindow = 0;

        wNum = GetSameMediaType( *ptrNumMap, m_ptrMediaArray[i], m_pfVBRStream[i], 
                                 data2, &dwBufferWindow );        
        if( wNum == (WORD) -1 )
        {
            _tprintf( _T( "Mismatch in Media types of both the Streams.\n" ) );
            return( FALSE );
        }

        //
        //  We will be concatenating the contents of two different ASF files.
        //  Each was encoded so that the stream data falls within the bitrate
        //  and buffer window specified for that stream in that file.
        //  However, concatenated together, Stream N of File 1 and Stream N of File 2
        //  may no longer stay within the bitrate/buffer window specified for that stream
        //  (since the resultant stream might require extra buffer window
        //  around the juncture point).  To be safe, we increase the buffer window size
        //  to be the sum of the buffer window size in the two files.
        //  While this might be excessive, it does ensure that the output file will stay
        //  within the bitrate/buffer window numbers specified in its profile.
        //
        //  The following assumes that this CStreamData will be used to set up the
        //  writer's profile. Let's store the sum of the buffer window values of matching streams
        //  from input profiles here.
        //
        m_ptrStreamBufferWindow[ i ] += dwBufferWindow;
        (*ptrNumMap)[i + m_dwStreamCount] = wNum;
    }
    
    return( TRUE );
}

//------------------------------------------------------------------------------
// Name: CStreamData::GetSameMediaType()
// Desc: Finds the buffer window for a stream of the specified media type.
//------------------------------------------------------------------------------
WORD CStreamData::GetSameMediaType( WORD * ptrNumMap, 
                                    WM_MEDIA_TYPE * pMediaToFind, 
                                    BOOL fVBR, 
                                    CStreamData& data2, 
                                    DWORD * pdwBufferWindow )
{
    if( NULL == ptrNumMap || NULL == pMediaToFind || NULL == pdwBufferWindow )
    {
        return( -1 );
    }

    DWORD dwCount = m_dwStreamCount * 2;
    for( DWORD i =0; i < m_dwStreamCount; i++ )
    {
        if( fVBR == m_pfVBRStream[i]
            && CompareMediaTypes( pMediaToFind, data2.m_ptrMediaArray[i], fVBR ) )
        {
            //
            //  Skip stream if already chosen
            //
            DWORD j;
            for( j = m_dwStreamCount; j < dwCount; j++ )
            {
                if( ptrNumMap[j] == data2.m_ptrStreamNumArray[i] )
                    break;
            }
            if( j == dwCount )
            {
                *pdwBufferWindow = data2.m_ptrStreamBufferWindow[i];
                return( data2.m_ptrStreamNumArray[i] );
            }
        }
    }
    return( -1 );
}

//------------------------------------------------------------------------------
// Name: CStreamData::CompareMediaTypes()
// Desc: Returns TRUE if two media types are identical.
//------------------------------------------------------------------------------
BOOL CStreamData::CompareMediaTypes(WM_MEDIA_TYPE * pMedia1, 
                                    WM_MEDIA_TYPE * pMedia2, 
                                    BOOL fVBR )
{
    if( pMedia1->majortype != pMedia2->majortype )
        return( FALSE );

    if( pMedia1->subtype != pMedia2->subtype )
        return( FALSE );

    if( pMedia1->bFixedSizeSamples != pMedia2->bFixedSizeSamples )
        return( FALSE );
    
    if( pMedia1->bTemporalCompression != pMedia2->bTemporalCompression )
        return( FALSE );

    if( pMedia1->lSampleSize != pMedia2->lSampleSize )
        return( FALSE );

    if( pMedia1->formattype != pMedia2->formattype )
        return( FALSE );

    if( pMedia1->cbFormat != pMedia2->cbFormat )
        return( FALSE );

    if ( fVBR )
    {
        //
        // VBR streams may have different average bitrate even if they're created 
	// using the same profile. We should ignore this difference.
        //
        if ( pMedia1->formattype == WMFORMAT_VideoInfo )
        {
            WMVIDEOINFOHEADER * pVideoFormat1 = (WMVIDEOINFOHEADER *)pMedia1->pbFormat;
            WMVIDEOINFOHEADER * pVideoFormat2 = (WMVIDEOINFOHEADER *)pMedia2->pbFormat;

            pVideoFormat1->dwBitRate = 0; 
            pVideoFormat2->dwBitRate = 0;
        }
        else if ( pMedia1->formattype == WMFORMAT_WaveFormatEx )
        {
            WAVEFORMATEX * pWaveFormat1 = (WAVEFORMATEX *)pMedia1->pbFormat;
            WAVEFORMATEX * pWaveFormat2 = (WAVEFORMATEX *)pMedia2->pbFormat;

            pWaveFormat1->nAvgBytesPerSec = 0; 
            pWaveFormat2->nAvgBytesPerSec = 0;
        }
    }

    if( 0 != memcmp( pMedia1->pbFormat, pMedia2->pbFormat, pMedia1->cbFormat ) )
        return( FALSE );

    return( TRUE );
}

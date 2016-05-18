//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            WMVRecompress.cpp
//
// Abstract:            Implementation of CWMVRecompress class. 
//
//*****************************************************************************

#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <windows.h>
#include <mmsystem.h>

#include "WMVRecompress.h"

//------------------------------------------------------------------------------
// Name: CWMVRecompress::CWMVRecompress()
// Desc: Constructor.
//------------------------------------------------------------------------------
CWMVRecompress::CWMVRecompress()
{
    //
    // Set the reference count to 1 when creating this class.
    //
    m_cRef = 1;
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::~CWMVRecompress()
// Desc: Destructor.
//------------------------------------------------------------------------------
CWMVRecompress::~CWMVRecompress()
{
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::CreateReader()
// Desc: Creates a reader and opens the source file using this reader. .
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::CreateReader( const WCHAR * pwszInputFile )
{
    HRESULT hr = S_OK;

    _tprintf( _T( "Creating the Reader...\n" ) );

    //
    // Create a reader
    //
    hr = WMCreateReader( NULL, 0, &m_pReader );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the IWMReaderAdvanced interface
    //
    hr = m_pReader->QueryInterface( IID_IWMReaderAdvanced, (void **)&m_pReaderAdvanced );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the IWMHeaderInfo interface of the reader
    //
    hr = m_pReader->QueryInterface( IID_IWMHeaderInfo, (void **)&m_pReaderHeaderInfo );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the profile of the reader
    //
    hr = m_pReader->QueryInterface( IID_IWMProfile, (void **)&m_pReaderProfile );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    m_hr = S_OK;
    ResetEvent( m_hEvent );

    //
    // Open the reader, using "this" as the callback interface.
    //
    hr = m_pReader->Open( pwszInputFile, this, NULL );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Wait until the WMT_OPENED status message is received in OnStatus()
    //
    hr = WaitForCompletion();
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the duration of the source file
    //
    WORD wStreamNum = 0;
    WMT_ATTR_DATATYPE enumType;
    WORD cbLength = sizeof( m_qwDuration );

    hr = m_pReaderHeaderInfo->GetAttributeByName( &wStreamNum, 
                                                  g_wszWMDuration, 
                                                  &enumType, 
                                                  (BYTE *)&m_qwDuration, 
                                                  &cbLength );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    if( m_qwDuration == 0 )
    {
        hr = E_INVALIDARG;
        return( hr );
    }

    //
    // Turn on the user clock
    //
    hr = m_pReaderAdvanced->SetUserProvidedClock( TRUE );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    return( hr );
} 

//------------------------------------------------------------------------------
// Name: CWMVRecompress::CreateWriter()
// Desc: Creates a writer and sets the profile and output of this writer.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::CreateWriter( const WCHAR * pwszOutputFile, 
                                      IWMProfile * pProfile )
{
    HRESULT             hr = S_OK;

    _tprintf( _T( "Creating the Writer...\n" ) );

    //
    // Create a writer
    //
    hr = WMCreateWriter( NULL, &m_pWriter );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the IWMHeaderInfo interface of the writer
    //
    hr = m_pWriter->QueryInterface( IID_IWMHeaderInfo, (void **)&m_pWriterHeaderInfo );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Set the profile of the writer
    //
    hr = m_pWriter->SetProfile( pProfile );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Set the output file of the writer
    //
    hr = m_pWriter->SetOutputFilename( pwszOutputFile );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    return( hr );
} 

//------------------------------------------------------------------------------
// Name: CWMVRecompress::GetOutputMap()
// Desc: Creates the map from the reader ouputs to the writer inputs and 
//       the map from the reader outputs to the reader streams.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::GetOutputMap()
{
    HRESULT             hr = S_OK;
    IWMInputMediaProps  * pWriterInputProps = NULL;
    IWMOutputMediaProps * pReaderOutputProps = NULL;
    IWMStreamConfig     * pReaderStreamConfig = NULL;
    DWORD               cReaderStream = 0;
    GUID                guidReaderOutput;
    GUID                guidWriterInput;
    WCHAR               * pwszOutputConnectionName = NULL;
    WCHAR               * pwszStreamConnectionName = NULL;
    WORD                cbConnectionName = 0;
    BOOL                fMatch = FALSE;

    do
    {
        //
        // Get the output count of the reader
        //
        hr = m_pReader->GetOutputCount( &m_cReaderOutput );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the input count of the writer
        //
        hr = m_pWriter->GetInputCount( &m_cWriterInput );
        if( FAILED( hr ) )
        {
            break;
        }
         
        //
        // Get the stream count of the reader
        //
        hr = m_pReaderProfile->GetStreamCount( &cReaderStream );
        if( FAILED( hr ) )
        {
            break;
        }
         
        //
        // Allocate memory for the map
        //
        m_pdwOutputToInput = new DWORD[ m_cReaderOutput ];
        m_pdwOutputToStream = new DWORD[ m_cReaderOutput ];

        if( NULL == m_pdwOutputToInput || NULL == m_pdwOutputToStream )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Iterate all outputs of the reader
        //
        for( DWORD i = 0; i < m_cReaderOutput; i ++ )
        {
            //
            // Get the output property interface of the reader
            //
            hr = m_pReader->GetOutputProps( i, &pReaderOutputProps );
            if( FAILED( hr ) )
            {
                break;
            }                       

            //
            // Get the type of this output property
            //
            hr = pReaderOutputProps->GetType( &guidReaderOutput );
            if( FAILED( hr ) )
            {
                break;
            }                       

            //
            // Get the connection name of this output
            //
            hr = pReaderOutputProps->GetConnectionName( NULL, &cbConnectionName );
            if( FAILED( hr ) )
            {
                break;
            }           
            
            pwszOutputConnectionName = new WCHAR[ cbConnectionName ];
            if( NULL == pwszOutputConnectionName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pReaderOutputProps->GetConnectionName( pwszOutputConnectionName, &cbConnectionName );
            if( FAILED( hr ) )
            {
                break;
            }           
            
            //
            // This loop tries to find a matching input of the writer for this output:
            // 1. The input must not be matched by other outputs of the reader.
            // 2. The input must have the same type as this output.
            //
            DWORD j;
            for( j = 0; j < m_cWriterInput; j ++ )
            {
                //
                // Seach for the current reader outputs to writer inputs map
                // 
                DWORD k;
                for( k = 0; k < i; k ++ )
                {
                    if( m_pdwOutputToInput[ k ] == j )
                    {
                        break;
                    }
                }

                //
                // If this input has been matched by other outputs of the reader, 
                // then don't use this input. 
                //
                if( k < i )
                {
                    continue;
                }

                //
                // Get the input property of the writer
                //
                hr = m_pWriter->GetInputProps( j, &pWriterInputProps );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                //
                // Get the type of this input property
                //
                hr = pWriterInputProps->GetType( &guidWriterInput );
                if( FAILED( hr ) )
                {
                    break;
                }
                
                SAFE_RELEASE( pWriterInputProps );

                //
                // If this input type of the writer is equal to the output 
                // type of the reader, we found a match. 
                //
                if( guidReaderOutput == guidWriterInput )
                {
                    fMatch = TRUE;
                    m_pdwOutputToInput[ i ] = j;
                    _tprintf( _T( "Output %d of the reader is mapped to input %d of the writer.\n" ), i, j );
                    break;
                }
            }

            if( FAILED( hr ) )
            {
                break;
            }

            //
            // If we cannot find a match for this output, mark it in the map. 
            //
            if( j >= m_cWriterInput )
            {
                m_pdwOutputToInput[ i ] = 0xFFFFFFFF;
            }

            //
            // In a multiple bitrate(MBR) file, several streams could have the 
            // same output number. When playing the MBR file, the reader 
            // determines the best stream to use based on the available resource.
            //
            // This loop finds the actual stream from which this reader output 
            // comes. This information will be used to set up multichannel 
            // encoding and smart recompression.
            //
            for( j = 0; j < cReaderStream; j ++ )
            {
                //
                // Get the stream configuration from the reader's profile
                //
                hr = m_pReaderProfile->GetStream( j, &pReaderStreamConfig );
                if( FAILED( hr ) )
                {
                    break;
                }

                //
                // Get the connection name of this stream configuration
                //
                hr = pReaderStreamConfig->GetConnectionName( NULL, &cbConnectionName );
                if( FAILED( hr ) )
                {
                    break;
                }           
                
                pwszStreamConnectionName = new WCHAR[ cbConnectionName ];
                if( NULL == pwszStreamConnectionName )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = pReaderStreamConfig->GetConnectionName( pwszStreamConnectionName, &cbConnectionName );
                if( FAILED( hr ) )
                {
                    break;
                }   
                
                SAFE_RELEASE( pReaderStreamConfig );

                //
                // If this output comes from this stream, they should have the 
                // same conneciton name
                //
                if( 0 == _wcsicmp( pwszStreamConnectionName, pwszOutputConnectionName ) )
                {
                    m_pdwOutputToStream[ i ] = j;
                    SAFE_ARRAYDELETE( pwszStreamConnectionName );
                    break;
                }

                SAFE_ARRAYDELETE( pwszStreamConnectionName );
            }

            if( FAILED( hr ) )
            {
                break;
            }

            SAFE_RELEASE( pReaderOutputProps );
            SAFE_ARRAYDELETE( pwszOutputConnectionName );
        }

        if( FAILED( hr ) )
        {
            break;
        }

        //
        // At least one output of the reader should be mapped to the input
        // of the writer. 
        //
        if( !fMatch )
        {
            hr = E_FAIL;
            _tprintf( _T( "Could not map any stream from the input file to the output file.\n" ) );
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszStreamConnectionName );
    SAFE_ARRAYDELETE( pwszOutputConnectionName );
    SAFE_RELEASE( pReaderStreamConfig );
    SAFE_RELEASE( pReaderOutputProps );
    SAFE_RELEASE( pWriterInputProps );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::SetWriterInput()
// Desc: Sets the input properties of the writer. The method will set up 
//       multichannel source and smart recompression if it's necessary.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::SetWriterInput( BOOL fMultiChannel, 
                                        BOOL fSmartRecompression )
{
    HRESULT             hr = S_OK;
    IWMReaderAdvanced2  * pReaderAdvanced2;
    IWMOutputMediaProps * pReaderOutputProps = NULL;
    IWMStreamConfig     * pReaderStreamConfig = NULL;
    IWMMediaProps       * pReaderStreamProps = NULL;
    IWMInputMediaProps  * pWriterInputProps = NULL;
    GUID                enumType;
    WM_MEDIA_TYPE       * pOutputMediaType = NULL;
    WM_MEDIA_TYPE       * pStreamMediaType = NULL;
    IWMPropertyVault    * pPropertyVault = NULL;
    DWORD               cbMediaType = 0;

    _tprintf( _T( "Setting the inputs of the writer...\n" ) );

    do
    {
        //
        // Get IWMReaderAdvanced2 interface from the reader
        //
        hr = m_pReader->QueryInterface( IID_IWMReaderAdvanced2, (void **)&pReaderAdvanced2 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Iterate all outputs of the reader
        //
        for( DWORD i = 0; i < m_cReaderOutput; i ++ )
        {
            //
            // If this output has a matching input in the writer, set up the input
            //
            if( 0xFFFFFFFF != m_pdwOutputToInput[ i ] )
            {
                //
                // Get the output property of the reader
                //
                hr = m_pReader->GetOutputProps( i, &pReaderOutputProps );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                // Get the input media properties of the writer
                hr = m_pWriter->GetInputProps( m_pdwOutputToInput[ i ], 
                                               &pWriterInputProps );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                // Get the media type
                hr = pWriterInputProps->GetType( &enumType );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                //
                // We need do more work if the stream type is audio and 
                // multichannel output or smart recompression is enabled.
                //
                if( ( fMultiChannel || fSmartRecompression ) && 
                    WMMEDIATYPE_Audio == enumType )
                {
                    //
                    // Get the stream from which this output comes 
                    //
                    hr = m_pReaderProfile->GetStream( m_pdwOutputToStream[ i ], 
                                                      &pReaderStreamConfig );
                    if( FAILED( hr ) )
                    {
                        break;
                    }

                    //
                    // Get the media property interface of this stream
                    //
                    hr = pReaderStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                                              (void **)&pReaderStreamProps );
                    if( FAILED( hr ) )
                    {
                        break;
                    }

                    //
                    // Get the media type of this stream
                    //
                    hr = pReaderStreamProps->GetMediaType( NULL, &cbMediaType );
                    if( FAILED( hr ) )
                    {
                        break;
                    }                       

                    pStreamMediaType = (WM_MEDIA_TYPE *)new BYTE[ cbMediaType ];
                    if( NULL == pStreamMediaType )
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    hr = pReaderStreamProps->GetMediaType( pStreamMediaType, 
                                                           &cbMediaType );
                    if( FAILED( hr ) )
                    {
                        break;
                    }       

                    //
                    // Enable multichannel output for the reader if it's required.
                    // We'll only enable multichannel output when the source has 
                    // more than 2 channels.
                    //
                    // Multichannel output only works on Windodws XP, and it's 
                    // not necessary unless a multichannel profile is used in 
                    // the writer. 
                    //

                    WAVEFORMATEX * pWFX = (WAVEFORMATEX *)pStreamMediaType->pbFormat;

                    if( fMultiChannel && 2 < pWFX->nChannels )
                    {
                        _tprintf( _T( "Multichannel output is enabled for output %d of the reader.\n" ), i );

                        BOOL fEnableDiscreteOutput = TRUE;
                        hr = pReaderAdvanced2->SetOutputSetting( i,
                                                                 g_wszEnableDiscreteOutput,
                                                                 WMT_TYPE_BOOL,
                                                                 (BYTE *)&fEnableDiscreteOutput,
                                                                 sizeof( BOOL ) );
                        if( FAILED( hr ) )
                        {
                            break;
                        }

                        DWORD dwSpeakerConfig = pWFX->nChannels;
                        hr = pReaderAdvanced2->SetOutputSetting( i,
                                                                 g_wszSpeakerConfig,
                                                                 WMT_TYPE_DWORD,
                                                                 (BYTE *)&dwSpeakerConfig,
                                                                 sizeof( DWORD ) );
                        if( FAILED( hr ) )
                        {
                            break;
                        }

                        //
                        // We need to call SetOutputProps() to change output properties accordingly
                        //
                        SAFE_RELEASE( pReaderOutputProps );
                        hr = m_pReader->GetOutputFormat( i, 0, &pReaderOutputProps );
                        if( FAILED( hr ) )
                        {
                            break;
                        }

                        hr = m_pReader->SetOutputProps( i, pReaderOutputProps );
                        if( FAILED( hr ) )
                        {
                            break;
                        }

                    }

                    //
                    // Set smart recompression if it's required.
                    //
                    if( fSmartRecompression )
                    {
                        _tprintf( _T( "Smart recompression is enabled for input %d of the writer.\n" ), 
                                  m_pdwOutputToInput[ i ] );

                        hr = pWriterInputProps->QueryInterface( IID_IWMPropertyVault, 
                                                                (void **)&pPropertyVault );
                        if( FAILED( hr ))
                        {
                            break;
                        }

                        hr = pPropertyVault->SetProperty( g_wszOriginalWaveFormat, 
                                                          WMT_TYPE_BINARY, 
                                                          pStreamMediaType->pbFormat, 
                                                          pStreamMediaType->cbFormat );
                        if( FAILED( hr ))
                        {
                            break;
                        }
                    }                       
                }

                //
                // Get the media type of the output property
                //
                hr = pReaderOutputProps->GetMediaType( NULL, &cbMediaType );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                pOutputMediaType = (WM_MEDIA_TYPE *)new BYTE[ cbMediaType ];
                if( NULL == pOutputMediaType )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = pReaderOutputProps->GetMediaType( pOutputMediaType, 
                                                       &cbMediaType );
                if( FAILED( hr ) )
                {
                    break;
                }       

                //
                // Apply the media type of the output to the input property 
                // of the writer
                //
                hr = pWriterInputProps->SetMediaType( pOutputMediaType );
                if( FAILED( hr ) )
                {
                    break;
                }

                //
                // Apply the changes of input properties to the writer
                //
                hr = m_pWriter->SetInputProps( m_pdwOutputToInput[ i ], pWriterInputProps );
                if( FAILED( hr ) )
                {
                    break;
                }                       

                //
                // Release all resources
                //
                SAFE_ARRAYDELETE( pStreamMediaType );
                SAFE_ARRAYDELETE( pOutputMediaType );

                SAFE_RELEASE( pPropertyVault );
                SAFE_RELEASE( pReaderStreamProps );
                SAFE_RELEASE( pReaderStreamConfig );
                SAFE_RELEASE( pWriterInputProps );
                SAFE_RELEASE( pReaderOutputProps );
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }

    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pStreamMediaType );
    SAFE_ARRAYDELETE( pOutputMediaType );

    SAFE_RELEASE( pPropertyVault );
    SAFE_RELEASE( pWriterInputProps );
    SAFE_RELEASE( pReaderOutputProps );
    SAFE_RELEASE( pReaderStreamProps );
    SAFE_RELEASE( pReaderStreamConfig );
    SAFE_RELEASE( pReaderAdvanced2 );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::Preprocess()
// Desc: Preprocesses the samples from the reader.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::Preprocess()
{
    HRESULT     hr = S_OK;

    //
    // Get the IWMWriterPreprocess interface from the writer
    // 
    hr = m_pWriter->QueryInterface( IID_IWMWriterPreprocess, 
                                    (void **)&m_pWriterPreprocess );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Allocate memroy for an array to store the preprocessing passes
    //
    m_pdwPreprocessPass = new DWORD[ m_cWriterInput ];
    if( NULL == m_pdwPreprocessPass )
    {
        hr = E_OUTOFMEMORY;
        return( hr );
    }

    //
    // Before preprocessing, we need to iterate all inputs of the writer
    // and get the preprocessing passes. Different inputs of the writer 
    // could have different preprocessing passes. 
    //
    DWORD i;
    for( i = 0; i < m_cWriterInput; i ++ )
    {
        //
        // If this input doesn't have a matching input in the reader, 
        // we don't need to do preprocessing for this input. 
        //
        DWORD j;
        for( j = 0; j < m_cReaderOutput; j ++ )
        {
            if( m_pdwOutputToInput[ j ] == i )
            {
                break;
            }
        }

        if( j < m_cReaderOutput )
        {
            //
            // Get the maximum number of preprocessing passes for this input
            //
            hr = m_pWriterPreprocess->GetMaxPreprocessingPasses( i, 
                                                                 0, 
                                                                 &m_pdwPreprocessPass[ i ] );
            if( FAILED( hr ) )
            {
                return( hr );
            }

            //
            // We do preprocessing only if the number of preprocessing passes is not 0
            //
            if( 0 != m_pdwPreprocessPass[ i ] )
            {
                //
                // Use the maximum number of preprocessing passes
                //
                hr = m_pWriterPreprocess->SetNumPreprocessingPasses( i, 
                                                                     0, 
                                                                     m_pdwPreprocessPass[ i ] );
                if( FAILED( hr ) )
                {
                    return( hr );
                }

                //
                // Begin preprocessing for this input
                // 
                hr = m_pWriterPreprocess->BeginPreprocessingPass( i, 0 );
                if( FAILED( hr ) )
                {
                    return( hr );
                }
            }
        }
        else
        {
            //
            // Set the preprocessing pass to 0 if the input doesn't have 
            // a matching output in the reader
            //
            m_pdwPreprocessPass[ i ] = 0;
        }
    }

    while( TRUE )
    {
        //
        // We do preprocessing only if at least one input has more than zero
        // preprocessing passes.
        //
        for( i = 0; i < m_cWriterInput; i ++ )
        {
            if( 0 < m_pdwPreprocessPass[ i ] )
            {
                break;
            }
        }

        if( i == m_cWriterInput )
        {
            //
            // We don't need to do preprocessing again, because the preprocessing
            // passes of all writer inputs are zero now
            //
            break;
        }

        //
        // Set m_bPreprocessing flag to true, so OnSample() will deliver the 
        // sample to m_pWriterPreprocess, not to m_pWriter.
        //
        m_bPreprocessing = TRUE;

        m_hr = S_OK;
        m_fEOF = FALSE;
        m_qwReaderTime = 0;
        m_dwProgress = 0;
        ResetEvent( m_hEvent );

        _tprintf( _T( "            0%%-------20%%-------40%%-------60%%-------80%%-------100%%\n" ) );
        _tprintf( _T( "Preprocess: " ) );

        //
        // Start the reader
        //
        hr = m_pReader->Start( 0, 0, 1.0, 0 );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        //
        // Wait until the reading is finished
        //
        hr = WaitForCompletion();
        _tprintf( _T( "\n" ) );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        //
        // Stop the reader
        //
        hr = m_pReader->Stop( );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        //
        // Decrease the preprocessing passes of each input
        //
        for( i = 0; i < m_cWriterInput; i ++ )
        {
            if( 0 < m_pdwPreprocessPass[ i ] )
            {
                m_pdwPreprocessPass[ i ] --;

                if( 0 == m_pdwPreprocessPass[ i ] )
                {
                    //
                    // If we don't need to do preprocessing for this input again,
                    // call EndPreprocessingPass() to stop preprocessing.
                    //
                    hr = m_pWriterPreprocess->EndPreprocessingPass( i, 0 );
                    if( FAILED( hr ) )
                    {
                        return( hr );
                    }
                }
            }
        }
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::Process()
// Desc: Processes the samples from the reader.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::Process()
{
    HRESULT hr = S_OK;

    //
    // Set m_bPreprocessing flag to false, so OnSample() will deliver the 
    // sample to m_pWriter, not to m_pWriterPreprocess.
    //
    m_bPreprocessing = FALSE;

    m_hr = S_OK;
    m_fEOF = FALSE;
    m_qwReaderTime = 0;
    m_dwProgress = 0;
    ResetEvent( m_hEvent );

    _tprintf( _T( "            0%%-------20%%-------40%%-------60%%-------80%%-------100%%\n" ) );
    _tprintf( _T( "Process:    " ) );

    //
    // Start the reader
    //
    hr = m_pReader->Start( 0, 0, 1.0, 0 );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Wait until the reading is finished
    //
    hr = WaitForCompletion();
    _tprintf( _T( "\n" ) );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Stop the reader
    //
    hr = m_pReader->Stop( );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::WaitForCompletion()
// Desc: Waits until the event is signaled.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::WaitForCompletion()
{
    HRESULT hr = S_OK;

    DWORD dwResult = WaitForSingleObject( m_hEvent, INFINITE );
    if( WAIT_OBJECT_0 == dwResult )
    {
        hr = m_hr;
    }
    else
    {
        hr = HRESULT_FROM_WIN32( dwResult );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::Recompress()
// Desc: Copies the input file to the output file, using the specified profile.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::Recompress( const WCHAR * pwszInputFile, 
                                    const WCHAR * pwszOutputFile,
                                    IWMProfile * pProifle,
                                    BOOL fMultiPass,
                                    BOOL fMultiChannel,
                                    BOOL fSmartRecompression )
{
    HRESULT hr = S_OK;

    //
    // Initialize the pointers
    //
    m_hEvent = NULL;
    m_pReader = NULL;
    m_pReaderAdvanced = NULL;
    m_pReaderHeaderInfo = NULL;
    m_pReaderProfile = NULL;
    m_pWriter = NULL;
    m_pWriterPreprocess = NULL;
    m_pWriterHeaderInfo = NULL;
    m_pdwPreprocessPass = NULL;
    m_pdwOutputToInput = NULL;
    m_pdwOutputToStream = NULL;

    if( NULL == pwszInputFile || NULL == pwszOutputFile || NULL == pProifle )
    {
        return E_INVALIDARG;
    }

    //
    // Dummy do-while loop to do cleanup operation before exiting
    //
    do
    {
        //
        // Event for the asynchronous calls
        //
        m_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if( NULL == m_hEvent )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            _tprintf( _T( "Create event failed (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Create the reader
        //
        hr = CreateReader( pwszInputFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create the Reader for file %ws (hr=0x%08x).\n" ), pwszInputFile, hr );
            break;
        }

        //
        // Create the writer
        //
        hr = CreateWriter( pwszOutputFile, pProifle );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create the Writer for file %ws (hr=0x%08x).\n" ), pwszOutputFile, hr );
            break;
        }

        //
        // Get the map from the reader ouputs to the writer inputs and 
        // the map from the reader outputs to the reader streams
        //
        hr = GetOutputMap();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not get the output map (hr=0x%08x).\n" ), hr );
            break;
        }
        
        //
        // Set the properties of writer inputs
        //
        hr = SetWriterInput( fMultiChannel, fSmartRecompression );
        if( FAILED( hr ) )
        {
            if( NS_E_INVALID_INPUT_FORMAT == hr )
            {
                _tprintf( _T( "Set writer input failed: The input format is invalid (hr=0x%08x).\n" ), hr );
            }
            else
            {
                _tprintf( _T( "Set writer input failed: Error (hr=0x%08x).\n" ), hr );
            }

            break;
        }

        //
        // Start the encoding
        //
        hr = m_pWriter->BeginWriting( );
        if( FAILED( hr ) )
        {
            if( NS_E_VIDEO_CODEC_NOT_INSTALLED == hr )
            {
                _tprintf( _T( "BeginWriting failed: Video codec not installed\n" ) );
            }
            if( NS_E_AUDIO_CODEC_NOT_INSTALLED == hr )
            {
                _tprintf( _T( "BeginWriting failed: Audio codec not installed\n" ) );
            }
            else if( NS_E_INVALID_OUTPUT_FORMAT == hr )
            {
                _tprintf( _T( "BeginWriting failed: Invalid output format \n" ) );
            }
            else if( NS_E_VIDEO_CODEC_ERROR == hr )
            {
                _tprintf( _T( "BeginWriting failed: An unexpected error occurred with the video codec \n" ) );
            }
            else if( NS_E_AUDIO_CODEC_ERROR == hr )
            {
                _tprintf( _T( "BeginWriting failed: An unexpected error occurred with the audio codec \n" ) );
            }
            else
            {
                _tprintf( _T( "BeginWriting failed: Error (hr=0x%08x)\n" ), hr );
            }

            break;
        }

        //
        // Preprocess the samples if multi-pass is enabled
        //
        if( fMultiPass )
        {
            hr = Preprocess();
            if(FAILED(hr ))
            {
                if( NS_E_INVALID_NUM_PASSES == hr )
                {
                    _tprintf( _T( "Preprocessing samples failed: Invalid preprocessing passes. Don't use -m option\n" ) );
                }
                else
                {
                    _tprintf( _T( "Preprocessing samples failed: Error (hr=0x%08x)\n" ), hr );
                }

                break;
            }
        }

        //
        // Process all samples from the reader
        //
        hr = Process();
        if(FAILED(hr ))
        {
            if( NS_E_INVALID_NUM_PASSES == hr )
            {
                _tprintf( _T( "Processing samples failed: Invalid preprocessing passes. Use -m option\n" ) );
            }
            else 
            {
                _tprintf( _T( "Processing samples failed: Error (hr=0x%08x)\n" ), hr );
            }

            break;
        }

        //
        // End writing
        //
        hr = m_pWriter->EndWriting( );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not end writing (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Close the reader
        //
        hr = m_pReader->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the reader (hr=0x%08x).\n" ), hr );
            break;
        }

        _tprintf( _T( "Recompression finished.\n" ) );
        
        //
        // Note: The output file is indexed automatically.
        // You can use IWMWriterFileSink3::SetAutoIndexing(FALSE) to disable 
        // auto indexing. 
        //
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( m_pdwOutputToStream );
    SAFE_ARRAYDELETE( m_pdwOutputToInput );
    SAFE_ARRAYDELETE( m_pdwPreprocessPass );
    SAFE_RELEASE( m_pWriterPreprocess );
    SAFE_RELEASE( m_pWriterHeaderInfo );
    SAFE_RELEASE( m_pWriter );
    SAFE_RELEASE( m_pReaderProfile );
    SAFE_RELEASE( m_pReaderHeaderInfo );
    SAFE_RELEASE( m_pReaderAdvanced );
    SAFE_RELEASE( m_pReader );
    SAFE_CLOSEHANDLE( m_hEvent );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::OnSample()
// Desc: Implementation of IWMReaderCallback::OnSample.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::OnSample( /* [in] */ DWORD dwOutputNum,
                                  /* [in] */ QWORD qwSampleTime,
                                  /* [in] */ QWORD qwSampleDuration,
                                  /* [in] */ DWORD dwFlags,
                                  /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                  /* [in] */ void __RPC_FAR * pvContext )
{
    HRESULT hr = S_OK;

    //
    // Get the input of the writer that matches this output.
    //
    DWORD dwInput = m_pdwOutputToInput[ dwOutputNum ];

    //
    // If this output of the reader doesn't have a matching input of the writer,
    // we'll not pass this sample to the writer.
    //
    if( 0xFFFFFFFF != dwInput )
    {
        //
        // Display the progress information to the user
        //
        while( m_dwProgress <= qwSampleTime * 50 / m_qwDuration )
        {
            m_dwProgress ++;
            _tprintf( _T( "*" ) );
        }
        
        //
        // If it's preprocessing, we pass the sample to m_pWriterPreprocess,
        // otherwise we pass the sample to the m_pWriter.
        //
        if( m_bPreprocessing )
        {
            //
            // If the preprocessing passes of the input is 0, we'll not
            // preprocess this sample.
            //
            if( m_pdwPreprocessPass[ dwInput ] > 0 )
            {
                hr = m_pWriterPreprocess->PreprocessSample( dwInput,        // input number
                                                            qwSampleTime,   // presentation time
                                                            dwFlags,        // flags
                                                            pSample );      // the data
            }
        }
        else
        {
            hr = m_pWriter->WriteSample( dwInput,       // input number
                                         qwSampleTime,  // presentation time
                                         dwFlags,       // flags
                                         pSample );     // the data
        }

        //
        // If an error occured in Reader, save this error code and set the event.
        //
        if( FAILED( hr ) )
        {
            m_hr = hr;
            SetEvent( m_hEvent );
        }
    }

    return(S_OK);
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::OnStatus()
// Desc: Implementation of IWMStatusCallback::OnStatus.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::OnStatus( /* [in] */ WMT_STATUS Status,
                                  /* [in] */ HRESULT hr,
                                  /* [in] */ WMT_ATTR_DATATYPE dwType,
                                  /* [in] */ BYTE __RPC_FAR * pValue,
                                  /* [in] */ void __RPC_FAR * pvContext)
{
    //
    // If an error code already exists, just set the event and return.
    //
    if( FAILED( m_hr ) )
    {
        SetEvent( m_hEvent );
        return(S_OK);
    }

    //
    // If an error occurred in the reader, save this error code and set the event.
    //
    if( FAILED(hr) )
    {
        m_hr = hr;
        SetEvent( m_hEvent );
        return(S_OK);
    }

    //
    // Status gives the current status of the reading of the input file
    //
    switch ( Status )
    {
    case WMT_OPENED:
        _tprintf( _T( "Reader Callback: File is opened.\n" ) );

        m_hr = S_OK;
        SetEvent( m_hEvent );

        break;

    case WMT_STARTED:
        //
        // Ask for 1 second of the stream to be delivered
        //
        m_qwReaderTime = 10000000;
        hr = m_pReaderAdvanced->DeliverTime( m_qwReaderTime );
        if( FAILED( hr ) )
        {
            m_hr = hr;
            SetEvent( m_hEvent );
        }

        break;

    case WMT_EOF:
        m_hr = S_OK;
        m_fEOF = TRUE;
        SetEvent( m_hEvent );
        
        break;
    }

    return(S_OK);
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::OnTime()
// Desc: Implementation of IWMReaderCallbackAdvanced::OnTime.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::OnTime( /* [in] */ QWORD qwCurrentTime,
                                /* [in] */ void __RPC_FAR * pvContext)
{
    //
    // Keep asking for 1 second of the stream till EOF
    //
    if( !m_fEOF )
    {
        m_qwReaderTime += 10000000;

        HRESULT hr = m_pReaderAdvanced->DeliverTime( m_qwReaderTime );
        if( FAILED( hr ) )
        {
            //
            // If an error occurred in the reader, save this error code and set the event.
            //
            m_hr = hr;
            SetEvent( m_hEvent );
        }
    }

    return(S_OK);
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::OnStreamSample()
// Desc: Implementation of IWMReaderCallbackAdvanced::OnStreamSample.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::OnStreamSample( /* [in] */ WORD wStreamNum,
                                        /* [in] */ QWORD cnsSampleTime,
                                        /* [in] */ QWORD cnsSampleDuration,
                                        /* [in] */ DWORD dwFlags,
                                        /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                        /* [in] */ void __RPC_FAR * pvContext )
{
    //
    // The samples are expected in OnSample
    //
    m_hr = E_UNEXPECTED;
    _tprintf( _T( "Reader Callback: Received a compressed sample (hr=0x%08x).\n" ), m_hr );
    SetEvent( m_hEvent );

    return(S_OK);
}

//------------------------------------------------------------------------------
// Implementation of other IWMReaderCallbackAdvanced methods.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::OnStreamSelection( /* [in] */ WORD wStreamCount,
                                           /* [in] */ WORD __RPC_FAR * pStreamNumbers,
                                           /* [in] */ WMT_STREAM_SELECTION __RPC_FAR * pSelections,
                                           /* [in] */ void __RPC_FAR * pvContext )
{
    return(S_OK);
}

HRESULT CWMVRecompress::OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
                                              /* [in] */ WM_MEDIA_TYPE __RPC_FAR * pMediaType,
                                              /* [in] */ void __RPC_FAR * pvContext )
{
    return(S_OK);
}

HRESULT CWMVRecompress::AllocateForOutput( /* [in] */ DWORD dwOutputNum,
                                           /* [in] */ DWORD cbBuffer,
                                           /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                           /* [in] */ void __RPC_FAR * pvContext)
{
    return E_NOTIMPL;
}

HRESULT CWMVRecompress::AllocateForStream( /* [in] */ WORD wStreamNum,
                                           /* [in] */ DWORD cbBuffer,
                                           /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                           /* [in] */ void __RPC_FAR * pvContext)
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------------
// Implementation of IUnknown methods.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::QueryInterface( /* [in] */ REFIID riid,
                                        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR * ppvObject)
{
    if( NULL == ppvObject )
    {
        return( E_INVALIDARG );
    }

    if( riid == IID_IWMStatusCallback )
    {
        *ppvObject = static_cast< IWMStatusCallback * >( this );
        AddRef();
    }
    else if( riid == IID_IWMReaderCallback )
    {
        *ppvObject = static_cast< IWMReaderCallback * >( this );
        AddRef();
    }
    else if( riid == IID_IWMReaderCallbackAdvanced )
    {
        *ppvObject = static_cast< IWMReaderCallbackAdvanced * >( this );
        AddRef();
    }
    else if( riid == IID_IUnknown )
    {
        *ppvObject = static_cast< IWMReaderCallback * >( this );
        AddRef();
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    return(S_OK);
}

ULONG CWMVRecompress::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}

ULONG CWMVRecompress::Release()
{
    if( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
    }

    return( m_cRef );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::ListSystemProfile()
// Desc: Enumerates all system profiles (version 8.0), and displays their 
//       indexes and names.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::ListSystemProfile()
{
    HRESULT             hr = S_OK;
    DWORD               dwIndex = 0;
    DWORD               cProfiles = 0;
    IWMProfileManager   * pIWMProfileManager = NULL;
    IWMProfileManager2  * pIWMProfileManager2 = NULL;
    IWMProfile          * pIWMProfile = NULL;
    LPWSTR              pwszName = NULL;
    DWORD               cchName = 0;

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2, 
                                                 ( void ** )&pIWMProfileManager2 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Set system profile version to 8.0
        //
        hr = pIWMProfileManager2->SetSystemProfileVersion( WMT_VER_8_0 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->GetSystemProfileCount( &cProfiles );
        if( FAILED( hr ) )
        {
            break;
        }
    
        _tprintf( _T( "Profile Indexes are as follows:\n" ) );

        //
        // Iterate all system profiles
        //
        for( dwIndex = 0; dwIndex < cProfiles; dwIndex++ )
        {
            hr = pIWMProfileManager->LoadSystemProfile( dwIndex, &pIWMProfile );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = pIWMProfile->GetName( NULL, &cchName );
            if ( FAILED( hr ) )
            {
                break;
            }

            pwszName = new WCHAR[ cchName ];
            if( NULL == pwszName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIWMProfile->GetName( pwszName, &cchName );
            if ( FAILED( hr ) )
            {
                break;
            }
    
            //
            // Display the system profile index and name
            //
            _tprintf( _T( "   %d - %ws \n" ), dwIndex + 1, pwszName );

            SAFE_ARRAYDELETE( pwszName );
            SAFE_RELEASE( pIWMProfile );
        }

        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszName );
    SAFE_RELEASE( pIWMProfile );
    SAFE_RELEASE( pIWMProfileManager2 );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::LoadSystemProfile()
// Desc: Loads a system profile (version 8.0) by the index.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::LoadSystemProfile( DWORD dwProfileIndex, 
                                           IWMProfile ** ppIWMProfile )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   * pIWMProfileManager = NULL;
    IWMProfileManager2  * pIWMProfileManager2 = NULL;

    if( NULL == ppIWMProfile )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2, 
                                                 ( void ** )&pIWMProfileManager2 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Set system profile version to 8.0
        //
        hr = pIWMProfileManager2->SetSystemProfileVersion( WMT_VER_8_0 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Load the system profile by index
        //
        hr = pIWMProfileManager->LoadSystemProfile( dwProfileIndex, 
                                                    ppIWMProfile );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_RELEASE( pIWMProfileManager2 );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVRecompress::LoadCustomProfile()
// Desc: Loads a custom profile from file.
//------------------------------------------------------------------------------
HRESULT CWMVRecompress::LoadCustomProfile( const WCHAR * pwszProfileFile, 
                                           IWMProfile ** ppIWMProfile )
{
    HRESULT             hr = S_OK;
    DWORD               dwLength = 0;
    DWORD               dwBytesRead = 0;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    IWMProfileManager   * pProfileManager = NULL;
    WCHAR               * pProfile = NULL;

    if( NULL == ppIWMProfile || NULL == pwszProfileFile )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Open the profile file
        //
        hFile = CreateFileW( pwszProfileFile, 
                             GENERIC_READ, 
                             FILE_SHARE_READ, 
                             NULL, 
                             OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, 
                             NULL );
        if( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( hFile ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        dwLength = GetFileSize( hFile, NULL );
        if( -1 == dwLength )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Allocate memory for profile buffer
        //
        pProfile = (WCHAR *) new BYTE[ dwLength + sizeof(WCHAR) ];
        if( NULL == pProfile )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // The buffer must be NULL terminated.
        memset( pProfile, 0, dwLength + sizeof(WCHAR) );

        //
        // Read the profile to a buffer
        //
        if( !ReadFile( hFile, pProfile, dwLength, &dwBytesRead, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Load the profile from the buffer
        //
        hr = pProfileManager->LoadProfileByData( pProfile, 
                                                 ppIWMProfile );
        if( FAILED(hr) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pProfile );
    SAFE_CLOSEFILEHANDLE( hFile );
    SAFE_RELEASE( pProfileManager );

    return( hr );
}

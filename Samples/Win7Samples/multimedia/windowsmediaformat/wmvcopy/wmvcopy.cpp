//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            WMVCopy.cpp
//
// Abstract:            Implementation of CWMVCopy class. 
//
//*****************************************************************************

#include <stdio.h>
#include <tchar.h>

#include "wmvcopy.h"

//------------------------------------------------------------------------------
// Name: CWMVCopy::CWMVCopy()
// Desc: Constructor.
//------------------------------------------------------------------------------
CWMVCopy::CWMVCopy()
{
    //
    // Set the reference count to 1 when creating this class.
    //
    m_cRef = 1;
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::~CWMVCopy()
// Desc: Destructor.
//------------------------------------------------------------------------------
CWMVCopy::~CWMVCopy()
{
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::CreateReader()
// Desc: Creates a reader and opens the source file using this reader.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CreateReader( const WCHAR * pwszInputFile )
{
    HRESULT             hr = S_OK;

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

    m_hr = S_OK;

    //
    // Open the reader; use "this" as the callback interface.
    //
    hr = m_pReader->Open( pwszInputFile, this, NULL );
    if( FAILED ( hr ) )
    {
        return( hr );
    }

    //
    // Wait until WMT_OPENED status message is received in OnStatus()
    //
    hr = WaitForCompletion();
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get the duration of the source file
    //
    WORD wStreamNumber = 0;
    WMT_ATTR_DATATYPE enumType;
    WORD cbLength = sizeof( m_qwDuration );

    hr = m_pReaderHeaderInfo->GetAttributeByName( &wStreamNumber, 
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

    //
    // Turn on manual stream selection, so we get all streams.
    //
    hr = m_pReaderAdvanced->SetManualStreamSelection( TRUE );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    return( hr );
} 

//------------------------------------------------------------------------------
// Name: CWMVCopy::GetProfileInfo()
// Desc: Gets the profile information from the reader.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::GetProfileInfo()
{
    HRESULT             hr = S_OK;
    IWMStreamConfig     * pStreamConfig = NULL;
    IWMMediaProps       * pMediaProperty = NULL;

    //
    // Get the profile of the reader
    //
    hr = m_pReader->QueryInterface( IID_IWMProfile, (void **)&m_pReaderProfile );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Get stream count
    //
    hr = m_pReaderProfile->GetStreamCount( &m_dwStreamCount );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Allocate memory for the stream type array and stream number array
    //
    m_pguidStreamType = new GUID[ m_dwStreamCount ];
    m_pwStreamNumber = new WORD[ m_dwStreamCount ];
    if( NULL == m_pwStreamNumber || NULL == m_pguidStreamType )
    {
        hr = E_OUTOFMEMORY;
        return ( hr );
    }

    for( DWORD i = 0; i < m_dwStreamCount; i++ )
    {
        hr = m_pReaderProfile->GetStream( i, &pStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the stream number of the current stream
        //
        hr = pStreamConfig->GetStreamNumber( &m_pwStreamNumber[i] );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the stream to be received in compressed mode
        //
        hr = m_pReaderAdvanced->SetReceiveStreamSamples( m_pwStreamNumber[i], TRUE );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void **)&pMediaProperty );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the stream type of the current stream
        //
        hr = pMediaProperty->GetType( &m_pguidStreamType[i] );
        if( FAILED( hr ) )
        {
            break;
        }

        SAFE_RELEASE( pMediaProperty );
        SAFE_RELEASE( pStreamConfig );
    }

    SAFE_RELEASE( pMediaProperty );
    SAFE_RELEASE( pStreamConfig );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::CreateWriter()
// Desc: Creates a writer and sets the profile and output of this writer.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CreateWriter( const WCHAR * pwszOutputFile )
{
    HRESULT             hr = S_OK;
    DWORD               dwInputCount = 0;

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
    // Get the IWMWriterAdvanced interface of the writer
    //
    hr = m_pWriter->QueryInterface( IID_IWMWriterAdvanced, (void **)&m_pWriterAdvanced );
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
    // Set the profile of the writer to the reader's profile
    //
    hr = m_pWriter->SetProfile( m_pReaderProfile );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = m_pWriter->GetInputCount( &dwInputCount );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Set the input property to NULL so the SDK knows we're going to 
    // send compressed samples to the inputs of the writer.
    //
    for ( DWORD i = 0; i < dwInputCount; i ++ )
    {
        hr = m_pWriter->SetInputProps( i, NULL );
        if( FAILED( hr ) )
        {
            return( hr );
        }
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
// Name: CWMVCopy::CopyAttribute()
// Desc: Copies all the header attributes from the reader file to the writer.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CopyAttribute()
{
    HRESULT             hr = S_OK;
    WORD                wStreamNumber;
    WORD                wAttributeCount;
    WMT_ATTR_DATATYPE   enumType;
    WORD                cbNameLength = 0;
    WORD                cbValueLength = 0;
    WCHAR               * pwszName = NULL;
    BYTE                * pbValue = NULL;

    for( DWORD i = 0; i <= m_dwStreamCount; i ++ )
    {
        if( i == m_dwStreamCount )
        {
            //
            // When the stream number is 0, the attribute is a file-level attribute
            //
            wStreamNumber = 0;
        }
        else
        {
            wStreamNumber = m_pwStreamNumber[i];
        }

        //
        // Get the attribute count 
        //
        hr = m_pReaderHeaderInfo->GetAttributeCount( wStreamNumber,
                                                     &wAttributeCount );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Copy all attributes of this stream
        //
        for( WORD j = 0; j < wAttributeCount; j ++ )
        {
            //
            // Get the attribute name and value length
            //
            hr = m_pReaderHeaderInfo->GetAttributeByIndex( j,
                                                           &wStreamNumber,
                                                           NULL,
                                                           &cbNameLength,
                                                           &enumType,
                                                           NULL,
                                                           &cbValueLength );
            if( FAILED( hr ) )
            {
                break;
            }

            pwszName = new WCHAR[ cbNameLength ];
            pbValue = new BYTE[ cbValueLength ];
            if( NULL == pwszName || NULL == pbValue )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            //
            // Get the attribute name and value
            //
            hr = m_pReaderHeaderInfo->GetAttributeByIndex( j,
                                                           &wStreamNumber,
                                                           pwszName,
                                                           &cbNameLength,
                                                           &enumType,
                                                           pbValue,
                                                           &cbValueLength );
            if( FAILED( hr ) )
            {
                break;
            }

            //
            // Set the attribute for the writer
            //
            hr = m_pWriterHeaderInfo->SetAttribute( wStreamNumber,
                                                    pwszName,
                                                    enumType,
                                                    pbValue,
                                                    cbValueLength );
            if( E_INVALIDARG == hr )            
            {
                //
                // Some attributes are read-only; we cannot set them.
                // They'll be set automatically by the writer.
                //
                hr = S_OK;
            }

            if( FAILED( hr ) )
            {                
                break;
            }

            SAFE_ARRAYDELETE( pwszName );
            SAFE_ARRAYDELETE( pbValue );
        }

        if( FAILED( hr ) )
        {
            break;
        }
    }

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszName );
    SAFE_ARRAYDELETE( pbValue );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy:CopyCodecInfo()
// Desc: Copies codec information from the reader header to the writer header.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CopyCodecInfo()
{
    HRESULT             hr = S_OK;
    DWORD               cCodecInfo;
    WCHAR               * pwszName = NULL;
    WCHAR               * pwszDescription = NULL;
    BYTE                * pbCodecInfo = NULL;
    IWMHeaderInfo3      * pReaderHeaderInfo3 = NULL;
    IWMHeaderInfo3      * pWriterHeaderInfo3 = NULL;

    do
    {
        hr = m_pReaderHeaderInfo->QueryInterface( IID_IWMHeaderInfo3,
                                                  (void **)&pReaderHeaderInfo3 );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = m_pWriterHeaderInfo->QueryInterface( IID_IWMHeaderInfo3,
                                                  (void **)&pWriterHeaderInfo3 );
        if ( FAILED( hr ) )
        {
            break;
        }
    
        hr = pReaderHeaderInfo3->GetCodecInfoCount( &cCodecInfo );
        if ( FAILED( hr ) )
        {
            break;
        }

        for( DWORD i = 0; i < cCodecInfo; i++ )
        {
            WMT_CODEC_INFO_TYPE enumCodecType;
            WORD        cchName = 0;
            WORD        cchDescription = 0;
            WORD        cbCodecInfo = 0;

            //
            // Get codec info from the source
            //
            hr = pReaderHeaderInfo3->GetCodecInfo( i, &cchName, NULL,
                           &cchDescription, NULL, &enumCodecType, 
                           &cbCodecInfo, NULL );
            if( FAILED( hr ) )
            {
                break;
            }

            pwszName = new WCHAR [ cchName ];
            pwszDescription = new WCHAR [ cchDescription ];
            pbCodecInfo = new BYTE [ cbCodecInfo ];

            if( NULL == pwszName || NULL == pwszDescription || NULL == pbCodecInfo )
            {
    	        hr = E_OUTOFMEMORY;
                break;
            }

            hr = pReaderHeaderInfo3->GetCodecInfo( i, &cchName, pwszName, 
                            &cchDescription, pwszDescription, &enumCodecType, 
                            &cbCodecInfo, pbCodecInfo );
            if( FAILED( hr ) )
            {
                break;
            }

            //
            // Add the codec info to the writer
            //
            hr = pWriterHeaderInfo3->AddCodecInfo( pwszName, pwszDescription, enumCodecType,
                                                   cbCodecInfo, pbCodecInfo );
            if( FAILED( hr ) )
            {
                break;
            }

            SAFE_ARRAYDELETE( pwszName );
            SAFE_ARRAYDELETE( pwszDescription );
            SAFE_ARRAYDELETE( pbCodecInfo );
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pwszName );
    SAFE_ARRAYDELETE( pwszDescription );
    SAFE_ARRAYDELETE( pbCodecInfo );
    SAFE_RELEASE( pReaderHeaderInfo3 );
    SAFE_RELEASE( pWriterHeaderInfo3 );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::CopyScriptInHeader()
// Desc: Copies the script in the header of the source file to the header 
//       of the destination file .
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CopyScriptInHeader()
{
    HRESULT     hr = S_OK;
    WORD        cScript = 0;
    WCHAR       * pwszType = NULL;
    WCHAR       * pwszCommand = NULL;
    QWORD       cnsScriptTime = 0;

    hr = m_pReaderHeaderInfo->GetScriptCount( &cScript );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    for( WORD i = 0; i < cScript; i++ )
    {
        WORD        cchTypeLen = 0;
        WORD        cchCommandLen = 0;

        //
        // Get the memory size for this script
        //
        hr = m_pReaderHeaderInfo->GetScript( i,
                                             NULL,
                                             &cchTypeLen,
                                             NULL,
                                             &cchCommandLen,
                                             &cnsScriptTime );
        if( FAILED( hr ) )
        {
            break;
        }

        pwszType = new WCHAR[cchTypeLen];
        pwszCommand = new WCHAR[cchCommandLen];
        if( pwszType == NULL || pwszCommand == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Get the script
        //
        hr = m_pReaderHeaderInfo->GetScript( i,
                                             pwszType,
                                             &cchTypeLen,
                                             pwszCommand,
                                             &cchCommandLen,
                                             &cnsScriptTime );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the script to the writer
        //
        hr = m_pWriterHeaderInfo->AddScript( pwszType,
                                             pwszCommand,
                                             cnsScriptTime );
        if( FAILED( hr ) )
        {
            break;
        }

        SAFE_ARRAYDELETE( pwszType );
        SAFE_ARRAYDELETE( pwszCommand );
    }

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszType );
    SAFE_ARRAYDELETE( pwszCommand );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::Process()
// Desc: Processes the samples from the reader..
//------------------------------------------------------------------------------
HRESULT CWMVCopy::Process()
{
    HRESULT hr = S_OK;

    //
    // Begin writing
    //
    hr = m_pWriter->BeginWriting( );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    m_hr = S_OK;
    m_fEOF = FALSE;
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
    // End writing
    //
    hr = m_pWriter->EndWriting( );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::CopyMarker()
// Desc: Copies the markers from the source file to the destination file.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CopyMarker( const WCHAR * pwszOutputFile )
{
    HRESULT             hr = S_OK;
    WORD                cMarker = 0;
    IWMMetadataEditor   * pEditor = NULL;
    IWMHeaderInfo       * pWriterHeaderInfo = NULL;
    WCHAR               * pwszMarkerName = NULL;

    do
    {
        hr = m_pReaderHeaderInfo->GetMarkerCount( &cMarker );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Markers can be copied only by the metadata editor.
        // Create an editor
        //
        hr = WMCreateEditor( &pEditor );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Open the output using the editor
        //
        hr = pEditor->Open( pwszOutputFile );
        if( FAILED ( hr ) )
        {
            break;
        }

        hr = pEditor->QueryInterface( IID_IWMHeaderInfo, (void **) &pWriterHeaderInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        for( WORD i = 0; i < cMarker; i++)
        {
            WORD    cchMarkerNameLen = 0;
            QWORD   cnsMarkerTime = 0;

            //
            // Get the memory size for this marker
            //
            hr = m_pReaderHeaderInfo->GetMarker( i,
                                                 NULL,
                                                 &cchMarkerNameLen,
                                                 &cnsMarkerTime );
            if( FAILED( hr ) )
            {
                break;
            }

            pwszMarkerName = new WCHAR[cchMarkerNameLen];
            if( pwszMarkerName == NULL )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = m_pReaderHeaderInfo->GetMarker( i,
                                                 pwszMarkerName,
                                                 &cchMarkerNameLen,
                                                 &cnsMarkerTime );
            if( FAILED( hr ) )
            {
                break;
            }

            //
            // Add marker to the writer
            //
            hr = pWriterHeaderInfo->AddMarker( pwszMarkerName,
                                               cnsMarkerTime );
            if( FAILED( hr ) )
            {
                break;
            }

            SAFE_ARRAYDELETE( pwszMarkerName );
        }

        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Close and release the editor
        //
        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pwszMarkerName );
    SAFE_RELEASE( pWriterHeaderInfo );
    SAFE_RELEASE( pEditor );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CWMVCopy::CopyScriptInList()
// Desc: Copies scripts in m_ScriptList to the header of the writer.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::CopyScriptInList( const WCHAR * pwszOutputFile )
{
    HRESULT             hr = S_OK;
    CScript             * pScript = NULL;
    IWMMetadataEditor   * pEditor = NULL;
    IWMHeaderInfo       * pWriterHeaderInfo = NULL;

    do
    {
        //
        // Scripts can be added by the metadata editor.
        // Create an editor
        //
        hr = WMCreateEditor( &pEditor );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Open the output using the editor
        //
        hr = pEditor->Open( pwszOutputFile );
        if( FAILED ( hr ) )
        {
            break;
        }

        hr = pEditor->QueryInterface( IID_IWMHeaderInfo, (void **) &pWriterHeaderInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        while( m_ScriptList.GetScript( &pScript )  )
        {
            //
            // Add the script to the writer
            //
            hr = pWriterHeaderInfo->AddScript( pScript->GetType( ),
                                               pScript->GetParameter( ),
                                               pScript->GetTime( ) ) ;
            SAFE_DELETE( pScript );

            if( FAILED( hr ) )
            {
                break;
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Close and release the editor
        //
        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pWriterHeaderInfo );
    SAFE_RELEASE( pEditor );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CWMVCopy::WaitForCompletion()
// Desc: Waits until the event is signaled.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::WaitForCompletion()
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
// Name: CWMVCopy::Copy()
// Desc: Copies the input file to the output file. The script stream is moved 
//       to the header if fMoveScriptStream is TRUE.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::Copy( const WCHAR * pwszInputFile, 
                        const WCHAR * pwszOutputFile,
                        QWORD qwMaxDuration,
                        BOOL fMoveScriptStream )
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
    m_pWriterAdvanced = NULL;
    m_pWriterHeaderInfo = NULL;

    m_dwStreamCount = 0;
    m_pguidStreamType = NULL;
    m_pwStreamNumber = NULL;

    if( NULL == pwszInputFile || NULL == pwszOutputFile )
    {
        return( E_INVALIDARG );
    }

    //
    // Dummy do-while loop to do clean up operation before exiting
    //
    do
    {
        m_fMoveScriptStream = fMoveScriptStream;
        m_qwMaxDuration = qwMaxDuration;

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
        // Create the Reader
        //
        hr = CreateReader( pwszInputFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create the Reader (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Get profile information
        //
        hr = GetProfileInfo();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not update profile information (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Create the Writer
        //
        hr = CreateWriter( pwszOutputFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create the Writer (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Copy all attributes
        //
        hr = CopyAttribute();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not copy attributes (hr=0x%08x).\n" ), hr );
            break;
        }


        //
        // Copy codec info
        //
        hr = CopyCodecInfo();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not copy codec info (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Copy all scripts in the header
        //
        hr = CopyScriptInHeader();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not copy scripts (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Process samples: read from the reader, and write to the writer
        //
        hr = Process();
        if( FAILED( hr ) )
        {
            //
            // Output some common error messages.
            //
            if( NS_E_VIDEO_CODEC_NOT_INSTALLED == hr )
            {
                _tprintf( _T( "Processing samples failed: Video codec not installed\n" ) );
            }
            if( NS_E_AUDIO_CODEC_NOT_INSTALLED == hr )
            {
                _tprintf( _T( "Processing samples failed: Audio codec not installed\n" ) );
            }
            else if( NS_E_INVALID_OUTPUT_FORMAT == hr )
            {
                _tprintf( _T( "Processing samples failed: Invalid output format \n" ) );
            }
            else if( NS_E_VIDEO_CODEC_ERROR == hr )
            {
                _tprintf( _T( "Processing samples failed: An unexpected error occurred with the video codec \n" ) );
            }
            else if( NS_E_AUDIO_CODEC_ERROR == hr )
            {
                _tprintf( _T( "Processing samples failed: An unexpected error occurred with the audio codec \n" ) );
            }
            else
            {
                _tprintf( _T( "Processing samples failed: Error (hr=0x%08x)\n" ), hr );
            }

            break;
        }

        //
        // Copy all scripts in m_ScriptList
        //
        hr = CopyScriptInList( pwszOutputFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not copy scripts (hr=0x%08x).\n" ), hr );
            break;
        }

        //
        // Copy marker information
        //
        hr = CopyMarker( pwszOutputFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not copy marker (hr=0x%08x).\n" ), hr );
            break;
        }

        _tprintf( _T( "Copy finished.\n" ) );

        //
        // Note: The output file is indexed automatically.
        // You can use IWMWriterFileSink3::SetAutoIndexing(FALSE) to disable 
        // auto indexing. 
        //
    }
    while( FALSE );

    SAFE_RELEASE( m_pReaderProfile );
    SAFE_RELEASE( m_pReaderHeaderInfo );
    SAFE_RELEASE( m_pReaderAdvanced );
    SAFE_RELEASE( m_pReader );    
    SAFE_RELEASE( m_pWriterHeaderInfo );
    SAFE_RELEASE( m_pWriterAdvanced );
    SAFE_RELEASE( m_pWriter );    
    SAFE_ARRAYDELETE( m_pguidStreamType );
    SAFE_ARRAYDELETE( m_pwStreamNumber );
    SAFE_CLOSEHANDLE( m_hEvent );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::OnSample()
// Desc: Implementation of IWMReaderCallback::OnSample.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::OnSample( /* [in] */ DWORD dwOutputNum,
                            /* [in] */ QWORD qwSampleTime,
                            /* [in] */ QWORD qwSampleDuration,
                            /* [in] */ DWORD dwFlags,
                            /* [in] */ INSSBuffer __RPC_FAR * pSample,
                            /* [in] */ void __RPC_FAR * pvContext )
{
    //
    // The samples are expected in OnStreamSample
    //
    m_hr = E_UNEXPECTED;
    _tprintf( _T( "Reader Callback: Received a decompressed sample (hr=0x%08x).\n" ), m_hr );
    SetEvent( m_hEvent );

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CWMVCopy::OnStatus()
// Desc: Implementation of IWMStatusCallback::OnStatus.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::OnStatus( /* [in] */ WMT_STATUS Status,
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
        return( S_OK );
    }

    //
    // If an error occurred in the reader, save this error code and set the event.
    //
    if( FAILED(hr) )
    {
        m_hr = hr;
        SetEvent( m_hEvent );
        return( S_OK );
    }

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

    return( S_OK );
}


//------------------------------------------------------------------------------
// Name: CWMVCopy::OnTime()
// Desc: Implementation of IWMReaderCallbackAdvanced::OnTime.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::OnTime( /* [in] */ QWORD qwCurrentTime,
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

    return( S_OK );
}

HRESULT CWMVCopy::OnStreamSample( /* [in] */ WORD wStreamNum,
                                  /* [in] */ QWORD cnsSampleTime,
                                  /* [in] */ QWORD cnsSampleDuration,
                                  /* [in] */ DWORD dwFlags,
                                  /* [in] */ INSSBuffer __RPC_FAR * pSample,
                                  /* [in] */ void __RPC_FAR * pvContext)
{
    HRESULT     hr = S_OK;
    BOOL        fMoveScript = FALSE;

    while( m_dwProgress <= cnsSampleTime * 50 / m_qwDuration )
    {
        m_dwProgress ++;
        _tprintf( _T( "*" ) );
    }

    if ( m_qwMaxDuration > 0 && cnsSampleTime > m_qwMaxDuration )
    {
        SetEvent( m_hEvent );
        return( hr );
    }

    if( m_fMoveScriptStream )
    {
        //
        // We may have multiple script streams in this file. 
        //
        for( DWORD i = 0; i < m_dwStreamCount; i ++ )
        {
            if( m_pwStreamNumber[i] == wStreamNum )
            {
                if( WMMEDIATYPE_Script == m_pguidStreamType[ i ] )
                {
                    fMoveScript = TRUE;
                }

                break;
            }
        }
    }

    if( fMoveScript )
    {
        DWORD   dwBufferLength;
        int     nStringLength;
        WCHAR   * pwszType = NULL;
        WCHAR   * pwszCommand = NULL;

        do
        {
            //
            // Read the buffer and length of the script stream sample
            //
            hr = pSample->GetBufferAndLength( (BYTE **)&pwszType, &dwBufferLength );
            if( FAILED( hr ) )
            {
                break;
            }

            nStringLength = dwBufferLength / 2;
            if( nStringLength > 0 )
            {
                //
                // Set the NULL terminator of this sample in case this sample 
                // doesn't have NULL terminator.
                //
                pwszType[ nStringLength - 1 ] = 0;

                //
                // Get the command string of this script
                //
                pwszCommand = wcschr( pwszType, NULL );
                if( pwszCommand - pwszType < nStringLength - 1 )
                {
                    pwszCommand ++;
                }
                else
                {
                    pwszCommand = L" ";
                }

                //
                // Add the script to the script list. We cannot write scripts 
                // directly to the writer after writing has begun.
                //
                hr = m_ScriptList.AddScript( pwszType, pwszCommand, cnsSampleTime );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
        } 
        while( FALSE );
    }
    else
    {
        hr = m_pWriterAdvanced->WriteStreamSample( wStreamNum,
                                                   cnsSampleTime,
                                                   0,
                                                   cnsSampleDuration,
                                                   dwFlags,
                                                   pSample );
    }

    //
    // Set the event if an error occurred
    //
    if( FAILED( hr ) )
    {
        m_hr = hr;
        SetEvent( m_hEvent );
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Implementation of other IWMReaderCallbackAdvanced methods.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::OnStreamSelection( /* [in] */ WORD wStreamCount,
                                     /* [in] */ WORD __RPC_FAR * pStreamNumbers,
                                     /* [in] */ WMT_STREAM_SELECTION __RPC_FAR * pSelections,
                                     /* [in] */ void __RPC_FAR * pvContext)
{
    return( S_OK );
}

HRESULT CWMVCopy::OnOutputPropsChanged( /* [in] */ DWORD dwOutputNum,
                                        /* [in] */ WM_MEDIA_TYPE __RPC_FAR * pMediaType,
                                        /* [in] */ void __RPC_FAR * pvContext )
{
    return( S_OK );
}

HRESULT CWMVCopy::AllocateForOutput( /* [in] */ DWORD dwOutputNum,
                                     /* [in] */ DWORD cbBuffer,
                                     /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                     /* [in] */ void __RPC_FAR * pvContext)
{
    return( E_NOTIMPL );
}

HRESULT CWMVCopy::AllocateForStream( /* [in] */ WORD wStreamNum,
                                     /* [in] */ DWORD cbBuffer,
                                     /* [out] */ INSSBuffer __RPC_FAR *__RPC_FAR * ppBuffer,
                                     /* [in] */ void __RPC_FAR * pvContext)
{
    return( E_NOTIMPL );
}

//------------------------------------------------------------------------------
// Implementation of IUnknown methods.
//------------------------------------------------------------------------------
HRESULT CWMVCopy::QueryInterface( /* [in] */ REFIID riid,
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
        return( E_NOINTERFACE );
    }

    return( S_OK );
}

ULONG CWMVCopy::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}

ULONG CWMVCopy::Release()
{
    if( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
    }

    return( m_cRef );
}

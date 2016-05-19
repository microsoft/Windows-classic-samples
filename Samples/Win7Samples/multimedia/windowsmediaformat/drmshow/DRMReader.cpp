//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DRMReader.cpp
//
// Abstract:            This file contains the implementation for the
//                      CLicenseViewer class, which can show the DRM-related
//                      properties of the file, including information about any
//                      licenses the user may have for the content.
//
//                      For examples about how to query the DRM reader for
//                      properties, see the ShowRights method
//
//*****************************************************************************

#include "stdafx.h"
#include "DRMReader.h"
#include "assert.h"

///////////////////////////////////////////////////////////////////////////////
typedef struct tag_DRM_Properties
{
    const WCHAR*        pwszName;           // Name of the DRM property to test
    WMT_ATTR_DATATYPE   wmtExpectedType;    // Its expected type - we'll break on mismatch
    WORD                cbExpectedLength;   // Its expected length - we'll break on mismatch
}
DRM_Properties;

static const DRM_Properties g_BoolProperties[] =
{
    { g_wszWMDRM_IsDRM,                             WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if the file is DRM-protected
    { g_wszWMDRM_IsDRMCached,                       WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if license was created locally
    { g_wszWMDRM_ActionAllowed_Playback,            WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if license allows playback
    { g_wszWMDRM_ActionAllowed_CopyToCD,            WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if license allows writing to a CD
    { g_wszWMDRM_ActionAllowed_CopyToNonSDMIDevice, WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if license allows tranfer to a non-SDMI compliant device
    { g_wszWMDRM_ActionAllowed_CopyToSDMIDevice,    WMT_TYPE_BOOL, sizeof( BOOL ) },    // True if license allows tranfer to a SDMI compliant device
    { g_wszWMDRM_ActionAllowed_Backup,              WMT_TYPE_BOOL, sizeof( BOOL ) }     // True if license allows itself to be backed-up
};

#define NUM_BOOL_PROPERTIES ( sizeof( g_BoolProperties ) / sizeof(g_BoolProperties[ 0 ] ) )

static const DRM_Properties g_LicenseProperties[] =
{
    { g_wszWMDRM_LicenseState_Playback,             WMT_TYPE_BINARY, sizeof( WM_LICENSE_STATE_DATA ) }, // License data related to playback
    { g_wszWMDRM_LicenseState_CopyToCD,             WMT_TYPE_BINARY, sizeof( WM_LICENSE_STATE_DATA ) }, // License data related to writing content to CD
    { g_wszWMDRM_LicenseState_CopyToNonSDMIDevice,  WMT_TYPE_BINARY, sizeof( WM_LICENSE_STATE_DATA ) }, // License data related to transfer to non-SDMI compliant devices
    { g_wszWMDRM_LicenseState_CopyToSDMIDevice,     WMT_TYPE_BINARY, sizeof( WM_LICENSE_STATE_DATA ) }  // License data related to transfer to SDMI compliant devices
};

#define NUM_LICENSE_PROPERTIES ( sizeof( g_LicenseProperties ) / sizeof( g_LicenseProperties[ 0 ] ) )

static const DRM_Properties g_HeaderProperties[] =
{
    { L"DRMHeader.KID", WMT_TYPE_STRING, 0 },   // The Key ID for the content
    { L"DRMHeader.LAINFO", WMT_TYPE_STRING, 0 } // The license acquisition URL for the content
};

#define NUM_HEADER_PROPERTIES ( sizeof( g_HeaderProperties ) / sizeof( g_HeaderProperties[ 0 ] ) )


//------------------------------------------------------------------------------
// Name: CLicenseViewer::CLicenseViewer()
// Desc: Constructor.
//------------------------------------------------------------------------------
CLicenseViewer::CLicenseViewer()
{
    m_pIWMReader = NULL;
    m_pIWMDRMReader = NULL;
    m_hEvent = NULL;
    m_cRef = 1;
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::~CLicenseViewer()
// Desc: Destructor.
//------------------------------------------------------------------------------
CLicenseViewer::~CLicenseViewer()
{
    Cleanup();
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::Initialize()
// Desc: Initialization.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CLicenseViewer::Initialize()
{
    HRESULT hr = S_OK;

    do
    {
        //
        // Reset the m_hr variable
        //
        m_hr = S_OK;

        //
        // Create callback event, if it doesn't already exist
        //
        if ( NULL == m_hEvent )
        {
            m_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if ( !m_hEvent )
            {
                hr = HRESULT_FROM_WIN32( ::GetLastError() );
                _tprintf( _T( "CreateEvent failed 0x%08lX\n" ), hr );
                break;
            }
        }
    
        //
        // Create a Reader and get the DRM reader interface
        //
        if ( NULL == m_pIWMReader )
        {
            hr = WMCreateReader( NULL, 0, &m_pIWMReader ); 
            if ( FAILED( hr ) || NULL == m_pIWMReader )
            {
                hr = E_FAIL;
                _tprintf( _T( "Failed to create WMReader 0x%08lX\n" ), hr );
                break;
            }

            SAFE_RELEASE( m_pIWMDRMReader );
            assert( m_pIWMReader );
        	hr = m_pIWMReader->QueryInterface( IID_IWMDRMReader, ( void **)&m_pIWMDRMReader );
            if ( FAILED( hr ) )
            {
                _tprintf( _T( "Failed to QI for DRM Reader interface 0x%08lX\n" ), hr );
                break;
            }
        }
    }
    while ( FALSE );
    
    return hr;
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::OnSample()
// Desc: Implementation of IWMReaderCallback::OnSample.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CLicenseViewer::OnSample( DWORD dwOutputNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer __RPC_FAR *pSample, void *pvContext )
{
    //
    // No samples are expected, but a value must be returned
    //
    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::OnStatus()
// Desc: Implementation of IWMStatusCallback::OnStatus.
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CLicenseViewer::OnStatus( WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE *pValue, void *pvContext )
{
    //
    // If the file has been opened (even if there was an error), then set the event so that
    // the other thread can stop waiting, since OnStatus is called on the WMFSDK's thread
    //
    switch ( Status )
    {
    case WMT_OPENED:
        _tprintf( _T( "WMT_OPENED 0x%08lX\n" ), hr );
        if ( FAILED( hr ) )
        {
            m_hr = hr; // Set the m_hr member variable so the error can be addressed later
        }
        assert( m_hEvent );
        SetEvent( m_hEvent );
        break;
    }
    
    return( S_OK );
}

//------------------------------------------------------------------------------
// Implementation of IUnknown methods
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CLicenseViewer::QueryInterface( REFIID riid, void **ppvObject )
{
    HRESULT hr = S_OK;
    
	if ( riid == IID_IWMReaderCallback )
    {
		AddRef();
		*ppvObject = ( IWMReaderCallback* )this;
    }
    else
	{
        *ppvObject = NULL;
        return( E_NOINTERFACE );
    }

    return( hr );
}
	
///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CLicenseViewer::AddRef( void )
{
    return( InterlockedIncrement( &m_cRef ) );
}

///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CLicenseViewer::Release( void )
{
    if ( 0 == InterlockedDecrement( &m_cRef ) )
    {
        delete this;
        return 0;
    }
    
    return( m_cRef );
}


//------------------------------------------------------------------------------
// Name: CLicenseViewer::Close()
// Desc: Close the reader.
//------------------------------------------------------------------------------
HRESULT CLicenseViewer::Close( void )
{
    HRESULT hr = S_OK;

    if ( NULL != m_pIWMReader )
    {
        hr = m_pIWMReader->Close();
    }

    return ( hr );
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::Open()
// Desc: Create and open the reader.
//------------------------------------------------------------------------------
HRESULT CLicenseViewer::Open( __in LPWSTR pwszInFile )
{
    static const DWORD OPEN_WAIT_LENGTH = 30000;
    HRESULT hr = S_OK;

    do 
    {
        //
        // Create the reader and callback event, if not already done
        //
        hr = Initialize();
        if ( FAILED( hr ) )
            break;
            
        //
        // Validate input parameter
        //
        if ( NULL == pwszInFile )
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Filename should not be NULL.\n" ) );
            break;
        }

        //
        // Open the input file
        //
        assert( m_pIWMReader );
	    hr = m_pIWMReader->Open( pwszInFile, this, NULL );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to open file 0x%08lX\n" ), hr );
            break;
        }

        //
        // Wait for the WMT_OPENED message to be sent, signaling the file has been opened.  This
        // message is received in the OnStatus method
        //
        assert( m_hEvent );
        if ( WAIT_TIMEOUT == WaitForSingleObject( m_hEvent, OPEN_WAIT_LENGTH ) )
        {
            _tprintf( _T( "Failed to open file in a reasonable amount of time.\n" ) );
            break;
        }

        //
        // Check the m_hr variable, which will get set if an error occured while opening the file.
        // m_hr will get set in the OnStatus method.
        //
        if ( FAILED( m_hr ) )
        {
            hr = m_hr;

            switch( m_hr )
            {
            case NS_E_LICENSE_REQUIRED:
                _tprintf( _T( "A license is required to open the given file.\n" ) );
                break;

            default:
                _tprintf( _T( "An error occured while opening file \"%ws\": %0.8x.\n" ), pwszInFile, hr );
            }

            break;
        }
    }
    while( FALSE );

    if ( FAILED( hr ) )
    {
        Cleanup();
    }
    
    return( hr );
}


//------------------------------------------------------------------------------
// Name: CLicenseViewer::PrintLicenseStateData()
// Desc: Display license data.
//------------------------------------------------------------------------------
HRESULT CLicenseViewer::PrintLicenseStateData( LPCTSTR tszOutputPrefix, DRM_LICENSE_STATE_DATA* pDRMLicenseStateData )
{
    HRESULT hr = S_OK;
    DWORD dwCountIndex;
    DWORD dwDateIndex;
    
    assert( tszOutputPrefix );
    assert( pDRMLicenseStateData );
    
    do
    {
        //
        // Print the type of right given by aggregating all the licenses for the content
        //
        _tprintf( _T( "%sDRM_LICENSE_DATA.dwStreamId: %ld\n" ), tszOutputPrefix, pDRMLicenseStateData->dwStreamId );
        switch( pDRMLicenseStateData->dwCategory )
        {
        case WM_DRM_LICENSE_STATE_NORIGHT: // No rights to perform this action
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_NORIGHT\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_UNLIM: // Unlimited rights to perform this action
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_UNLIM\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_COUNT: // Action may only be performed a certain number of times
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_COUNT\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_FROM: // Action cannot be performed until a specific date
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_FROM\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_UNTIL: // Action cannot be performed after a certain date
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_UNTIL\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_FROM_UNTIL: // Action can only be performed within a specific range of dates
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_FROM_UNTIL\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_COUNT_FROM: // Action can only be performed a certain number of times, starting from a specific date
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_COUNT_FROM\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_COUNT_UNTIL: // Action can be performed a certain number of times until a specific date
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_COUNT_UNTIL\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_COUNT_FROM_UNTIL: // Action can only be performed a certain number of times, and only is a specific range of dates
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_COUNT_FROM_UNTIL\n" ), tszOutputPrefix );
            break;
    
        case WM_DRM_LICENSE_STATE_EXPIRATION_AFTER_FIRSTUSE: // License restrictions don't occur until after the first use
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: WM_DRM_LICENSE_STATE_EXPIRATION_AFTER_FIRSTUSE\n" ), tszOutputPrefix );
            break;
            
        default:
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCategory: Unknown! - %d\n" ), tszOutputPrefix, pDRMLicenseStateData->dwCategory );
        }
        
        //
        // If count limited, print the number of times the action can be performed
        //
        if ( 0 != pDRMLicenseStateData->dwNumCounts )
        {
            _tprintf( _T( "%sDRM_LICENSE_DATA.dwCount:" ), tszOutputPrefix );
            for( dwCountIndex = 0; dwCountIndex < pDRMLicenseStateData->dwNumCounts; dwCountIndex++ )
            {
                _tprintf( _T( "  %ld" ), pDRMLicenseStateData->dwCount[ dwCountIndex ] );
            }
            _tprintf( _T( "\n" ) );
        }
        
        //
        // If the action is date limited, print the date restriction(s)
        //
        if ( 0 != pDRMLicenseStateData->dwNumDates )
        {
            _tprintf( _T( "%sDRM_LICENSE_DATA.datetime:" ), tszOutputPrefix );
            for( dwDateIndex = 0; dwDateIndex < pDRMLicenseStateData->dwNumDates; dwDateIndex++ )
            {
                SYSTEMTIME  SystemTime;
                WCHAR       wszOn[128];
                WCHAR       wszAt[128];
    
                //
                // Convert the FILETIME to SYSTEMTIME and format into strings
                //
                FileTimeToSystemTime( &(pDRMLicenseStateData->datetime[ dwDateIndex ]), &SystemTime );
    			GetDateFormatW( LOCALE_USER_DEFAULT, 0, &SystemTime, L"ddd',' MMM dd yyyy", wszOn, 128 );
    			GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &SystemTime, L"hh mm ss tt", wszAt, 128 );
    
                _tprintf( _T( "  On %ws at %ws" ), wszOn, wszAt );
            }
            _tprintf( _T( "\n" ) );
        }
        
        //
        // If the aggregate license data cannot easily be represented, the "vague" value will be set.
        // This will happen when two or more licenses with different right types (like COUNT and
        // and FROM_UNTIL) make it impossible to represent the information simply.  For instance,
        // if license 1 allows 5 plays and license 2 allows unlimited playback during the current
        // month, then the user is guaranteed at least 5 plays, but possibly an unlimited number,
        // if done within the current month.
        //
        _tprintf( _T( "%sDRM_LICENSE_DATA.dwVague: %ld \n" ), tszOutputPrefix, pDRMLicenseStateData->dwVague );
    }
    while ( FALSE );
    
    return hr;
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::ShowRights()
// Desc: Show DRM rights properties.
//------------------------------------------------------------------------------
HRESULT CLicenseViewer::ShowRights()
{
    HRESULT                 hr = S_OK;
    DWORD                   dwPropertyIndex;
    WMT_ATTR_DATATYPE       wmtType;
    BOOL                    fBoolProperty;
    WORD                    cbLength;
    WM_LICENSE_STATE_DATA   LicenseStateData;
    WCHAR                   wszTemp[500];
    LPCWSTR                 wszPropertyName;
    const DRM_Properties*   pCurrentProperty;
    LPBYTE                  pbProperty = NULL;
    DWORD                   dwStateIndex;
    
    //
    // Validate current state
    //
    if ( !m_pIWMDRMReader )
    {
        _tprintf( _T( "ShowRights: m_pIWMDRMReader is NULL." ) );
        return E_FAIL;
    }
    
    _tprintf( _T( "ShowRights 0x%08lX\n" ), hr );

    //
    // Print the boolean properties
    //
    _tprintf( _T( "  Basic rights information:\n" ) );

    for ( dwPropertyIndex = 0; dwPropertyIndex < NUM_BOOL_PROPERTIES; dwPropertyIndex++ )
    {
        pCurrentProperty = &(g_BoolProperties[ dwPropertyIndex ]);
        
        //
        // Get the next property from the DRM reader
        //
        wszPropertyName = pCurrentProperty->pwszName;
        cbLength = sizeof( BOOL );
        assert( m_pIWMDRMReader );
        assert( pCurrentProperty );
    	hr = m_pIWMDRMReader->GetDRMProperty( wszPropertyName, &wmtType, (BYTE *)&fBoolProperty, &cbLength );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "    Couldn't get value for property %ws (hr = 0x%08lX)\n" ), wszPropertyName, hr );
            hr = S_OK; // Reset the HRESULT, since it was "handled"
        }
        else
        {
            //
            // Check to make sure the property is the expected type and size
            //
            if ( wmtType != pCurrentProperty->wmtExpectedType )
            {
                _tprintf( _T( "    Invalid returned property type\n" ) );
                continue;
            }

            if ( sizeof( BOOL ) != cbLength )
            {
                _tprintf( _T( "    Invalid returned property length\n" ) );
                continue;
            }
            
            //
            // Print the value of the property
            //
            _tprintf( _T( "    %ws: %Ls\n" ), wszPropertyName, fBoolProperty ? _T( "Yes" ) : _T( "No" ) );
        }
    }

    //
    // Get the license state properties
    //
    _tprintf( _T( "  Content rights information:\n" ) );

    for( dwPropertyIndex = 0; dwPropertyIndex < NUM_LICENSE_PROPERTIES; dwPropertyIndex++ )
    {
        pCurrentProperty = &(g_LicenseProperties[ dwPropertyIndex ]);

        //
        // Get property from DRM reader
        //        
        cbLength = pCurrentProperty->cbExpectedLength;
        assert( m_pIWMDRMReader );
        assert( pCurrentProperty );
    	hr = m_pIWMDRMReader->GetDRMProperty( pCurrentProperty->pwszName, &wmtType, (BYTE *)&LicenseStateData, &cbLength );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "    Couldn't get value for property %ws (hr = 0x%08lX)\n" ), pCurrentProperty->pwszName, hr );
            hr = S_OK; // Reset the HRESULT, since it was "handled"
        }
        else
        {
            //
            // Check that the property type and size are the expected values
            //
            if ( wmtType != pCurrentProperty->wmtExpectedType )
            {
                _tprintf( _T( "  Invalid returned property type\n" ) );
                continue;
            }

            if ( cbLength != pCurrentProperty->cbExpectedLength )
            {
                _tprintf( _T( "  Invalid returned property length\n" ) );
                continue;
            }
            
            //
            // Print each license state
            //
            _tprintf( _T( "    %ws:\n" ), pCurrentProperty->pwszName );
            for( dwStateIndex = 0; dwStateIndex < LicenseStateData.dwNumStates; dwStateIndex++ )
            {
                hr = PrintLicenseStateData( _T("      "), &(LicenseStateData.stateData[ dwStateIndex ]) );
                if ( FAILED( hr ) )
                {
                    break;
                }
            }
            if ( FAILED( hr ) )
            {
                break;
            }
        }
    }

    //
    // Test the Header properties
    //
    _tprintf( _T( "  Header Properies:\n" ) );

    for( dwPropertyIndex = 0; dwPropertyIndex < NUM_HEADER_PROPERTIES; dwPropertyIndex++ )
    {
        pCurrentProperty = &(g_HeaderProperties[ dwPropertyIndex ]);
        
        //
        // Get the property from the DRM reader
        //
        cbLength = sizeof( wszTemp );
        assert( m_pIWMDRMReader );
        assert( pCurrentProperty );
    	hr = m_pIWMDRMReader->GetDRMProperty( pCurrentProperty->pwszName, &wmtType, (BYTE *)&wszTemp[ 0 ], &cbLength );
        if ( FAILED( hr ) )
        {
            _tprintf( _T( "    Couldn't get value for property %ws (hr = 0x%08lX)\n" ), pCurrentProperty->pwszName, hr );
            hr = S_OK; // Reset the HRESULT, since it was "handled"
        }
        else
        {
            //
            // Check that the type returned is the expected type
            //
            if ( wmtType != pCurrentProperty->wmtExpectedType )
            {
                _tprintf( _T( "    Invalid returned property type\n" ) );
                continue;
            }
            
            //
            // Print the value of the property
            //
            _tprintf( _T( "    %ws: %ws\n" ), pCurrentProperty->pwszName, wszTemp );
        }
    }
    
    SAFE_ARRAYDELETE( pbProperty );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CLicenseViewer::Cleanup()
// Desc: Release resources.
//------------------------------------------------------------------------------
HRESULT CLicenseViewer::Cleanup()
{
    HRESULT hr = S_OK;

    //
    // Release all resources held in member variables
    //    
    SAFE_RELEASE( m_pIWMDRMReader );
    SAFE_RELEASE( m_pIWMReader );
    SAFE_CLOSEHANDLE( m_hEvent );

    return( hr );
}


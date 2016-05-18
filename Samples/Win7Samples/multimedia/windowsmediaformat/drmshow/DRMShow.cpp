//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DRMShow.cpp
//
// Abstract:            This file contains the entry point to the sample,
//                      parsing the command line and calling into the
//                      CLicenseViewer class to print out the file's DRM-
//                      related properties
//
//*****************************************************************************

#include "stdafx.h"
#include "DRMReader.h"
#include <string.h>


//------------------------------------------------------------------------------
// Forward declarations.
//------------------------------------------------------------------------------//
void Usage();
#ifndef UNICODE
HRESULT ConvertStrToUnicode( LPCTSTR    ptszInput, __out LPWSTR* pwszInput );
#endif
HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR* ptszInput );

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int  __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    HRESULT         hr              = S_OK;
    CLicenseViewer  *pLicenseViewer = NULL;
    TCHAR           *ptszInput      = NULL;

    //
    // Get the name of the file for which to get the DRM properties
    //
    hr = CommandLineParser( argc, argv, &ptszInput );
    if ( FAILED( hr ) )
    {
        return hr;
    }

#ifndef UNICODE 
    WCHAR* pwszInput = NULL;

    hr = ConvertStrToUnicode( ptszInput, &pwszInput );
    if ( FAILED( hr ) )
    {
        return hr;
    }
#endif // UNICODE

    hr = CoInitialize( NULL );
	if( FAILED( hr ) )
	{
		_tprintf( _T( "CoInitialize failed" ) ) ;
		return( 1 );
	}

    do 
    {
        pLicenseViewer = new CLicenseViewer();
        if ( pLicenseViewer == NULL )
        {
            _tprintf( _T( "Unable to create license viewer object." ) );
            break;
        }

        //
        // Open the file
        //
#ifndef UNICODE 
        hr = pLicenseViewer->Open( pwszInput );
#else
        hr = pLicenseViewer->Open( ptszInput );
#endif
        if( FAILED( hr ) )
        {
            _tprintf( _T( "License viewer failed to open the file. hr = 0x%08lX\n" ), hr );
            break;
        }
        
        //
        // Print out the DRM info for the file
        //
        hr = pLicenseViewer->ShowRights();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "License viewer failed to ShowRights hr = 0x%08lX\n" ), hr );
            break;
        }
        
        //
        // Close the file
        //
        hr = pLicenseViewer->Close();
        if( FAILED( hr ) )
        {
            break;
        }
        
        //
        // Free the license viewer member variables, so that the reader will release
        // its reference count on the license viewer.  Otherwise the license viewer
        // will never be successfully deleted, and will leak memory
        //
        hr = pLicenseViewer->Cleanup();
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    //
    // Clean up
    //
    SAFE_RELEASE( pLicenseViewer );

#ifndef UNICODE 
    SAFE_ARRAYDELETE( pwszInput );
#endif

	CoUninitialize( );

    return 0;
}

//------------------------------------------------------------------------------
// Name: Usage()
// Desc: Shows correct command-line usage.
//------------------------------------------------------------------------------
void Usage()
{
   _tprintf( _T( "drmshow -i <infile>\n" ) );
   _tprintf( _T( "\tinfile\t = ASF file name\n" ) );
   _tprintf( _T( "\n" ) );

}

#ifndef UNICODE

//------------------------------------------------------------------------------
// Name: ConvertStrToUnicode()
// Desc: Converts a string to wide characters.
//------------------------------------------------------------------------------
HRESULT ConvertStrToUnicode( LPCTSTR    ptszInput, __out LPWSTR* pwszInput )
{
    HRESULT hr = S_OK;
    int nSizeCount = 0;
    
    if ( NULL == ptszInput || NULL == pwszInput)
    {
        return( E_INVALIDARG );
    }

    //
    // Make wide character string of the file name
    //
    nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 ) ;
    if( 0 ==  nSizeCount )
    {
        _tprintf( _T( "Internal error %lu\n" ), GetLastError() );
        return ( E_UNEXPECTED );
    }

    *pwszInput = new WCHAR[ nSizeCount ];
    if( NULL == *pwszInput)
    {
        _tprintf( _T( "Internal Error %lu\n" ), GetLastError() ) ;
        SAFE_ARRAYDELETE( *pwszInput );
        return ( E_UNEXPECTED );
    }

    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *pwszInput, nSizeCount ) )
    {
        _tprintf( _T( "Internal error %lu\n" ), GetLastError() );
        SAFE_ARRAYDELETE( *pwszInput );
        return ( E_UNEXPECTED );
    }        

    return( hr );
}
#endif

//------------------------------------------------------------------------------
// Name: CommandLineParser()
// Desc: Gets arguments from the command line and checks for validity.
//------------------------------------------------------------------------------
HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR* ptszInput )
{
    TCHAR*    ptszMode    = NULL;
    HRESULT hr            = S_OK;
    *ptszInput            = NULL;    


    if ( argc < 1 || NULL == argv || NULL == ptszInput)
    {
        return( E_INVALIDARG );
    }

    for( int i = 1; i < argc; i ++ )
    {
        if ( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++;

            if ( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }

            *ptszInput = argv[i];
            continue;
        }
    }

    if ( NULL == *ptszInput )
    {
        Usage();
        return( E_INVALIDARG );
    }

    return S_OK;
}

//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//
// Abstract:            Parse the parameters and call CWMVCopy::Copy
//
//*****************************************************************************

#include <tchar.h>
#include <stdio.h>

#include "wmvcopy.h"


//------------------------------------------------------------------------------
// Name: ConvertTCharToWChar()
// Desc: Converts a TCHAR string to a WCHAR string. The caller must release
//       the memory of pwszOutput by calling delete[] pwszOutput.
//------------------------------------------------------------------------------
HRESULT ConvertTCharToWChar( LPCTSTR ptszInput, __out LPWSTR * pwszOutput )
{
    int cchOutput = 0;
    
    if( NULL == ptszInput || NULL == pwszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get output buffer size
    //
#ifdef UNICODE
    cchOutput = wcslen( ptszInput ) + 1;
#else //UNICODE
    cchOutput = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#endif // UNICODE

    *pwszOutput = new WCHAR[ cchOutput ];
    if( NULL == *pwszOutput)
    {
        return( E_OUTOFMEMORY );
    }

#ifdef UNICODE
    wcsncpy_s( *pwszOutput, cchOutput, ptszInput, cchOutput - 1);

#else //UNICODE
    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *pwszOutput, cchOutput ) )
    {
        SAFE_ARRAYDELETE( *pwszOutput );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#endif // UNICODE

    return( S_OK );
}


//------------------------------------------------------------------------------
// Name: Usage()
// Desc: Shows the command-line usage if the user gives incorrect arguments.
//------------------------------------------------------------------------------
void Usage()
{
    _tprintf( _T( "Usage: wmvcopy  -i <INPUT_FILE> -o <OUTPUT_FILE> -d <TIME> [-s]\n" ) );
    _tprintf( _T( "  -i <INPUT_FILE> = Input Windows Media file name\n" ) );
    _tprintf( _T( "  -o <OUTPUT_FILE> = Output Windows Media file name\n" ) );
    _tprintf( _T( "  -d <TIME> = specify the max duration in seconds of the output file\n" ) );
    _tprintf( _T( "  -s = Move scripts in script stream to header\n" ) );
    _tprintf( _T( "\n" ) );
    _tprintf( _T( "  Note: this program will not copy the image stream in the source file." ) );
}

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    HRESULT         hr = S_OK;
    WCHAR           * pwszInFile = NULL;
    WCHAR           * pwszOutFile = NULL;
    QWORD           qwMaxDuration = 0;  // Maximum duration of output file 
    BOOL            fMoveScriptStream = FALSE;
    BOOL            fValidArgument = FALSE;
    CWMVCopy        * pWMVCopy = NULL;
        
    //
    // Initialize COM library
    //
    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: hr = 0x%08x\n" ), hr );
        return( hr );
    }

    do
    {
        //
        // Parse the command line
        //
        int i;
        for( i = 1; i < argc; i ++ )
        {
            fValidArgument = FALSE;

            if( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
            {
                i++;
                if( i >= argc || NULL != pwszInFile)
                {
                    break;
                }

                hr = ConvertTCharToWChar( argv[i], &pwszInFile );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-o" ) ) )
            {
                i++;
                if( i >= argc || NULL != pwszOutFile )
                {
                    break;
                }

                hr = ConvertTCharToWChar( argv[i], &pwszOutFile );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-d" ) ) )
            {
                i++;
                if( i >= argc )
                {
                    break;
                }

                int nMaxDuration = _ttoi( argv[i] );
                if ( nMaxDuration <= 0 )
                {
                    break;
                }

                qwMaxDuration = nMaxDuration * 10000000;
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-s" ) ) )
            {
                _tprintf( _T( "Move scripts in script stream to header.\n" ) );
                fMoveScriptStream = TRUE;
            }
            else
            {
                break;
            }

            fValidArgument = TRUE;
        }

        if( !fValidArgument || i < argc )
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Invalid argument.\n" ) );
            Usage();
            break;
        }

        if( NULL == pwszInFile || NULL == pwszOutFile || 0 == _wcsicmp( pwszInFile, pwszOutFile ) )
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "An input file and an output file must be specified.\n" ) );
            Usage();
            break;
        }

        pWMVCopy = new CWMVCopy();
        if( NULL == pWMVCopy )
        {
            hr = E_OUTOFMEMORY;
            _tprintf( _T( "Could not allocate memory for CWMVRecompress: (hr=0x%08x)\n" ), hr );
            break;
        }

        //
        // Copy this file
        //
        hr = pWMVCopy->Copy( pwszInFile, pwszOutFile, qwMaxDuration, fMoveScriptStream );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszInFile );
    SAFE_ARRAYDELETE( pwszOutFile );
    SAFE_RELEASE( pWMVCopy );

    CoUninitialize();

    return  hr;
}
